/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "dcopserver.moc"
#include <dcopglobal.h>
#include <qdatastream.h>
#include <qstack.h>

static DCOPServer* the_server = 0;
class DCOPListener : public QSocketNotifier
{
 public:
  DCOPListener( IceListenObj obj )
    : QSocketNotifier( IceGetListenConnectionNumber( obj ),
		       QSocketNotifier::Read, 0, 0)
    {
      listenObj = obj;
    }

  IceListenObj listenObj;
};

class DCOPConnection : public QSocketNotifier
{
 public:
  DCOPConnection( IceConn conn )
    : QSocketNotifier( IceConnectionNumber( conn ),
		       QSocketNotifier::Read, 0, 0 )
    {
      iceConn = conn;
      status = IceConnectPending;
    }

  QCString appId;
  IceConn iceConn;
  IceConnectStatus status;
  QStack< _IceConn> waitingForReply;
};

IceAuthDataEntry *authDataEntries = 0;
static char *addAuthFile = 0;
static char *remAuthFile = 0;

static IceListenObj *listenObjs = 0;
int numTransports = 0;
    
static IceIOErrorHandler prev_handler;

static Bool HostBasedAuthProc ( char* /*hostname*/)
{
  return false; // no host based authentication
}

static const char *hex_table[] = {            /* for printing hex digits */
    "00", "01", "02", "03", "04", "05", "06", "07",
    "08", "09", "0a", "0b", "0c", "0d", "0e", "0f",
    "10", "11", "12", "13", "14", "15", "16", "17",
    "18", "19", "1a", "1b", "1c", "1d", "1e", "1f",
    "20", "21", "22", "23", "24", "25", "26", "27",
    "28", "29", "2a", "2b", "2c", "2d", "2e", "2f",
    "30", "31", "32", "33", "34", "35", "36", "37",
    "38", "39", "3a", "3b", "3c", "3d", "3e", "3f",
    "40", "41", "42", "43", "44", "45", "46", "47",
    "48", "49", "4a", "4b", "4c", "4d", "4e", "4f",
    "50", "51", "52", "53", "54", "55", "56", "57",
    "58", "59", "5a", "5b", "5c", "5d", "5e", "5f",
    "60", "61", "62", "63", "64", "65", "66", "67",
    "68", "69", "6a", "6b", "6c", "6d", "6e", "6f",
    "70", "71", "72", "73", "74", "75", "76", "77",
    "78", "79", "7a", "7b", "7c", "7d", "7e", "7f",
    "80", "81", "82", "83", "84", "85", "86", "87",
    "88", "89", "8a", "8b", "8c", "8d", "8e", "8f",
    "90", "91", "92", "93", "94", "95", "96", "97",
    "98", "99", "9a", "9b", "9c", "9d", "9e", "9f",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
    "a8", "a9", "aa", "ab", "ac", "ad", "ae", "af",
    "b0", "b1", "b2", "b3", "b4", "b5", "b6", "b7",
    "b8", "b9", "ba", "bb", "bc", "bd", "be", "bf",
    "c0", "c1", "c2", "c3", "c4", "c5", "c6", "c7",
    "c8", "c9", "ca", "cb", "cc", "cd", "ce", "cf",
    "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
    "d8", "d9", "da", "db", "dc", "dd", "de", "df",
    "e0", "e1", "e2", "e3", "e4", "e5", "e6", "e7",
    "e8", "e9", "ea", "eb", "ec", "ed", "ee", "ef",
    "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7",
    "f8", "f9", "fa", "fb", "fc", "fd", "fe", "ff",
};

static void fprintfhex (FILE *fp, unsigned int len, char *cp)
{
  unsigned char *ucp = (unsigned char *) cp;
  
  for (; len > 0; len--, ucp++) {
    const char *s = hex_table[*ucp];
    putc (s[0], fp);
    putc (s[1], fp);
  }
}

/*
 * We use temporary files which contain commands to add/remove entries from
 * the .ICEauthority file.
 */ 
static void
write_iceauth (FILE *addfp, FILE *removefp, IceAuthDataEntry *entry)
{
  fprintf (addfp,
	   "add %s \"\" %s %s ",
	   entry->protocol_name,
	   entry->network_id,
	   entry->auth_name);
  fprintfhex (addfp, entry->auth_data_length, entry->auth_data);
  fprintf (addfp, "\n");
  
  fprintf (removefp,
	   "remove protoname=%s protodata=\"\" netid=%s authname=%s\n",
	   entry->protocol_name,
	   entry->network_id,
	   entry->auth_name);
}

#ifndef HAS_MKSTEMP
static char *unique_filename (const char *path, const char *prefix)
#else
static char *unique_filename (const char *path, const char *prefix, int *pFd)
#endif
{
#ifndef HAS_MKSTEMP
#ifndef X_NOT_POSIX
  return ((char *) tempnam (path, prefix));
#else
  char tempFile[PATH_MAX];
  char *tmp;
  
  sprintf (tempFile, "%s/%sXXXXXX", path, prefix);
  tmp = (char *) mktemp (tempFile);
  if (tmp)
    {
      char *ptr = (char *) malloc (strlen (tmp) + 1);
      strcpy (ptr, tmp);
      return (ptr);
    }
  else
    return (NULL);
#endif
#else
  char tempFile[PATH_MAX];
  char *ptr;
  
  sprintf (tempFile, "%s/%sXXXXXX", path, prefix);
  ptr = (char *)malloc(strlen(tempFile) + 1);
  if (ptr != NULL)
    {
      strcpy(ptr, tempFile);
      *pFd =  mkstemp(ptr);
    }
  return ptr;
#endif
}

#define MAGIC_COOKIE_LEN 16

Status
SetAuthentication (int count, IceListenObj *listenObjs, 
		   IceAuthDataEntry **authDataEntries)
{
  FILE        *addfp = NULL;
  FILE        *removefp = NULL;
  const char  *path;
  int         original_umask;
  char        command[256];
  int         i;
#ifdef HAS_MKSTEMP
  int         fd;
#endif
  
  original_umask = umask (0077);      /* disallow non-owner access */
  
  path = getenv ("DCOP_SAVE_DIR");
  if (!path) {
    path = getenv ("HOME");
    if (!path)
      path = ".";
  }
#ifndef HAS_MKSTEMP
  if ((addAuthFile = unique_filename (path, ".dcop")) == NULL)
    goto bad;
  
  if (!(addfp = fopen (addAuthFile, "w")))
    goto bad;
  
  if ((remAuthFile = unique_filename (path, ".dcop")) == NULL)
    goto bad;
  
  if (!(removefp = fopen (remAuthFile, "w")))
    goto bad;
#else
  if ((addAuthFile = unique_filename (path, ".dcop", &fd)) == NULL)
    goto bad;
  
  if (!(addfp = fdopen(fd, "wb")))
    goto bad;
  
  if ((remAuthFile = unique_filename (path, ".dcop", &fd)) == NULL)
    goto bad;
 
  if (!(removefp = fdopen(fd, "wb")))
    goto bad;
#endif
  
  if ((*authDataEntries = (IceAuthDataEntry *) malloc (
						       count * 2 * sizeof (IceAuthDataEntry))) == NULL)
    goto bad;
 
  for (i = 0; i < numTransports * 2; i += 2) {
    (*authDataEntries)[i].network_id =
      IceGetListenConnectionString (listenObjs[i/2]);
    (*authDataEntries)[i].protocol_name = (char *) "ICE";
    (*authDataEntries)[i].auth_name = (char *) "MIT-MAGIC-COOKIE-1";
 
    (*authDataEntries)[i].auth_data =
      IceGenerateMagicCookie (MAGIC_COOKIE_LEN);
    (*authDataEntries)[i].auth_data_length = MAGIC_COOKIE_LEN;
 
    (*authDataEntries)[i+1].network_id =
      IceGetListenConnectionString (listenObjs[i/2]);
    (*authDataEntries)[i+1].protocol_name = (char *) "DCOP";
    (*authDataEntries)[i+1].auth_name = (char *) "MIT-MAGIC-COOKIE-1";
   
    (*authDataEntries)[i+1].auth_data =
      IceGenerateMagicCookie (MAGIC_COOKIE_LEN);
    (*authDataEntries)[i+1].auth_data_length = MAGIC_COOKIE_LEN;
    
    write_iceauth (addfp, removefp, &(*authDataEntries)[i]);
    write_iceauth (addfp, removefp, &(*authDataEntries)[i+1]);
 
    IceSetPaAuthData (2, &(*authDataEntries)[i]);
    
    IceSetHostBasedAuthProc (listenObjs[i/2], HostBasedAuthProc);
  }
 
  fclose (addfp);
  fclose (removefp);
  
  umask (original_umask);
  
  sprintf (command, "iceauth source %s", addAuthFile);
  system (command);
 
  unlink (addAuthFile);
 
  return (1);
 
 bad:
 
  if (addfp)
    fclose (addfp);
  
  if (removefp)
    fclose (removefp);
  
  if (addAuthFile) {
    unlink(addAuthFile);
    free(addAuthFile);
  }
  if (remAuthFile) {
    unlink(remAuthFile);
    free(remAuthFile);
  }
  
  return (0);
}

/*
 * Free up authentication data.
 */
void
FreeAuthenticationData(int count, IceAuthDataEntry *authDataEntries)
{
  /* Each transport has entries for ICE and XSMP */
  
  char command[256];
  int i;
  
  for (i = 0; i < count * 2; i++) {
    free (authDataEntries[i].network_id);
    free (authDataEntries[i].auth_data);
  }
  
  free (authDataEntries);
   
  sprintf (command, "iceauth source %s", remAuthFile);
  system(command);
  
  unlink(remAuthFile);
  
  free(addAuthFile);
  free(remAuthFile);
}

void
MyIoErrorHandler ( IceConn iceConn )

{
    if (prev_handler)
	(*prev_handler) (iceConn);
}

void
InstallIOErrorHandler ()

{
    IceIOErrorHandler default_handler;

    prev_handler = IceSetIOErrorHandler (NULL);
    default_handler = IceSetIOErrorHandler (MyIoErrorHandler);
    if (prev_handler == default_handler)
	prev_handler = NULL;
}


void DCOPWatchProc ( IceConn iceConn, IcePointer client_data, Bool opening, IcePointer* watch_data)
{
  DCOPServer* ds = ( DCOPServer*) client_data;

  if (opening) {
    *watch_data = (IcePointer) ds->watchConnection( iceConn );
  }
  else  {
    ds->removeConnection( (void*) *watch_data );
  }
}

void DCOPProcessMessage( IceConn iceConn, IcePointer /*clientData*/, 
			 int opcode, unsigned long length, Bool swap)
{
  the_server->processMessage( iceConn, opcode, length, swap );
}

void DCOPServer::processMessage( IceConn iceConn, int opcode, 
				 unsigned long length, Bool /*swap*/)
{
  switch( opcode ) {
  case DCOPSend:
    {
      DCOPMsg *pMsg = 0;
      IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
      QByteArray ba( length );
      IceReadData(iceConn, length, ba.data() );
      QDataStream ds( ba, IO_ReadOnly );
      QCString appFrom, app;
      ds >> appFrom >> app;
      DCOPConnection* target = appIds.find( app );
      if ( target ) {
	IceGetHeader( target->iceConn, majorOpcode, DCOPSend,
		     sizeof(DCOPMsg), DCOPMsg, pMsg );
	int datalen = ba.size();
	pMsg->length += datalen;
	IceSendData(target->iceConn, datalen, (char *) ba.data());
      } else if ( app == "DCOPServer" ) {
	QCString obj, fun;
	QByteArray data;
	ds >> obj >> fun >> data;
	QCString replyType;
	QByteArray replyData;
	if ( !receive( app, obj, fun, data, replyType, replyData, iceConn ) ) {
	    qWarning("%s failure: object '%s' has no function '%s'", app.data(), obj.data(), fun.data() );
	}
      } else if ( app == "*") {
	// handle a broadcast.
	QDictIterator<DCOPConnection> aIt(appIds);
	while (aIt.current()) {
	  IceGetHeader(aIt.current()->iceConn, majorOpcode, DCOPSend,
		       sizeof(DCOPMsg), DCOPMsg, pMsg);
	  int datalen = ba.size();
	  pMsg->length += datalen;
	  IceSendData(aIt.current()->iceConn, datalen, (char *) ba.data());
	}
      }
    }
    break;
  case DCOPCall:
    {
      DCOPMsg *pMsg = 0;
      IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
      QByteArray ba( length );
      IceReadData(iceConn, length, ba.data() );
      QDataStream ds( ba, IO_ReadOnly );
      QCString appFrom, app;
      ds >> appFrom >> app;
      DCOPConnection* target = appIds.find( app );
      int datalen = ba.size();
      if ( target ) {
	target->waitingForReply.push( iceConn );
	IceGetHeader( target->iceConn, majorOpcode, DCOPCall,
		     sizeof(DCOPMsg), DCOPMsg, pMsg );
	pMsg->length += datalen;
	IceSendData(target->iceConn, datalen, (char *) ba.data());
      } else {
	QCString replyType;
	QByteArray replyData;
	bool b = FALSE;
	if ( app == "DCOPServer" ) {
	  QCString obj, fun;
	  QByteArray data;
	  ds >> obj >> fun >> data;
	  b = receive( app, obj, fun, data, replyType, replyData, iceConn );
	  if ( !b )
	      qWarning("%s failure: object '%s' has no function '%s'", app.data(), obj.data(), fun.data() );
	}

	QByteArray reply;
	QDataStream replyStream( reply, IO_WriteOnly );
	replyStream << replyType << replyData.size();
	
	int datalen = reply.size() + replyData.size();
	IceGetHeader( iceConn, majorOpcode, b? DCOPReply : DCOPReplyFailed,
		      sizeof(DCOPMsg), DCOPMsg, pMsg );
	pMsg->length += datalen;
	IceSendData( iceConn, reply.size(), (char *) reply.data());
	IceSendData( iceConn, replyData.size(), (char *) replyData.data());
      }
    }
    break;
  case DCOPReply:
  case DCOPReplyFailed:
    {
      DCOPMsg *pMsg = 0;
      IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
      QByteArray ba( length );
      IceReadData(iceConn, length, ba.data() );
      DCOPConnection* conn = clients.find( iceConn );
      if ( conn ) {
	DCOPConnection* connreply = clients.find( conn->waitingForReply.pop() );
	if ( connreply ) {
	  IceGetHeader( connreply->iceConn, majorOpcode, opcode,
			sizeof(DCOPMsg), DCOPMsg, pMsg );
	  int datalen = ba.size();
	  pMsg->length += datalen;
	  IceSendData(connreply->iceConn, datalen, (char *) ba.data());
	}
      }
    }
    break;
  default:
    qDebug("DCOPServer::processMessage unknown message");
  }
}


IcePaVersionRec DCOPVersions[] = {
  { DCOPVersionMajor, DCOPVersionMinor,  DCOPProcessMessage }
};

typedef struct DCOPServerConnStruct *DCOPServerConn;

struct DCOPServerConnStruct
{
  /*
   * We use ICE to esablish a connection with the client.
   */
  
  IceConn		iceConn;
  
  
  /*
   * Major and minor versions of the XSMP.
   */
  
  int			proto_major_version;
  int			proto_minor_version;
  
  
  QCString clientId;
};


static Status DCOPServerProtocolSetupProc ( IceConn iceConn,
					    int majorVersion, int minorVersion,
					    char* vendor, char* release,
					    IcePointer *clientDataRet, 
					    char **/*failureReasonRet*/)
{
    DCOPServerConn serverConn;
//     unsigned long 	mask;
//     Status		status;

    /*
     * vendor/release are undefined for ProtocolSetup in DCOP
     */

    if (vendor)
	free (vendor);
    if (release)
	free (release);


    /*
     * Allocate new DCOPServerConn.
     */

    serverConn = new DCOPServerConnStruct;

    serverConn->iceConn = iceConn;
    serverConn->proto_major_version = majorVersion;
    serverConn->proto_minor_version = minorVersion;
    //serverConn->clientId already initialized

    *clientDataRet = (IcePointer) serverConn;


    return 1;
}

static void
CloseListeners ()
{
  IceFreeListenObjs (numTransports, listenObjs);

  QCString fName(::getenv("HOME"));
  fName += "/.DCOPserver";
  unlink(fName.data());
  
  FreeAuthenticationData(numTransports, authDataEntries);
}

DCOPServer::DCOPServer()
  : QObject(0,0), appIds(200), clients(200)
{
  the_server = this;
  if (( majorOpcode = IceRegisterForProtocolReply ((char *) "DCOP",
						   (char *) DCOPVendorString, 
						   (char *) DCOPReleaseString,
						   1, DCOPVersions,
						   1, (char **) DCOPAuthNames,
						   DCOPServerAuthProcs,
						   HostBasedAuthProc,
						   DCOPServerProtocolSetupProc,
						   NULL,	/* IceProtocolActivateProc - we don't care about
								   when the Protocol Reply is sent, because the
								   session manager can not immediately send a
								   message - it must wait for RegisterClient. */
						   NULL	/* IceIOErrorProc */
						   )) < 0)
    {
      qDebug("Could not register DCOP protocol with ICE");
    }
  char errormsg[256];

  if (!IceListenForConnections (&numTransports, &listenObjs,
				256, errormsg))
    {
      fprintf (stderr, "%s\n", errormsg);
      exit (1);
    } else {
      // publish available transports.
      QCString fName(::getenv("HOME"));
      fName += "/.DCOPserver";
      FILE *f;
      f = ::fopen(fName.data(), "w+");
      fprintf(f, IceComposeNetworkIdList(numTransports, listenObjs));
      fclose(f);
    }
      

  atexit(CloseListeners);
  
  if (!SetAuthentication(numTransports, listenObjs, &authDataEntries)) {
    qDebug("dcopserver could not set authorization");
    exit(1);
  }

  IceAddConnectionWatch (DCOPWatchProc, (IcePointer) this);
  
  listener.setAutoDelete( TRUE );
  DCOPListener* con;
  for ( int i = 0; i < numTransports; i++) {
    con = new DCOPListener( listenObjs[i] );
    listener.append( con );
    connect( con, SIGNAL( activated(int) ), this, SLOT( newClient(int) ) );
  }
}

DCOPServer::~DCOPServer()
{
}


void DCOPServer::processData( int /*socket*/ )
{
  DCOPConnection* conn = (DCOPConnection*)sender();
  IceProcessMessagesStatus s =  IceProcessMessages( conn->iceConn, 0, 0 );


  if (s == IceProcessMessagesIOError) {
    IceCloseConnection( conn->iceConn );
    return;
  }
  conn->status = IceConnectionStatus(  conn->iceConn );
}

void DCOPServer::newClient( int /*socket*/ )
{
  IceAcceptStatus status;
  IceConn iceConn = IceAcceptConnection( ((DCOPListener*)sender())->listenObj, &status);
  IceSetShutdownNegotiation( iceConn, False );


  IceConnectStatus cstatus;
  while ((cstatus = IceConnectionStatus (iceConn))==IceConnectPending) {
    qApp->processOneEvent();
  }
  if (cstatus == IceConnectAccepted) {
    char* connstr = IceConnectionString (iceConn);
    free (connstr);
  } else {
    if (cstatus == IceConnectIOError)
      qDebug ("IO error opening ICE Connection!\n");
    else
      qDebug ("ICE Connection rejected!\n");
    IceCloseConnection (iceConn);
  }
}

void* DCOPServer::watchConnection( IceConn iceConn )
{
  qDebug("new connection (count=%d)", clients.count() );
  DCOPConnection* con = new DCOPConnection( iceConn );
  connect( con, SIGNAL( activated(int) ), this, SLOT( processData(int) ) );

  clients.insert(iceConn, con );

  return (void*) con;
}

void DCOPServer::removeConnection( void* data )
{
  DCOPConnection* conn = (DCOPConnection*)data;
  clients.remove(conn->iceConn );
  appIds.remove( conn->appId );
  if ( conn->appId.data() ) {
      qDebug("remove connection '%s' (count=%d)", conn->appId.data(), clients.count() );
      QPtrDictIterator<DCOPConnection> it( clients );
      QByteArray data;
      QDataStream datas( data, IO_WriteOnly );
      datas << conn->appId;
      QByteArray ba;
      QDataStream ds( ba, IO_WriteOnly );
      ds << QCString("DCOPServer") << QCString("")
	 << QCString("") << QCString("applicationRemoved(QCString)") << data;
      int datalen = ba.size();
      DCOPMsg *pMsg = 0;
      while ( it.current() ) {
	  DCOPConnection* c = it.current();
	  ++it;
	  if ( c != conn ) {
	      IceGetHeader( c->iceConn, majorOpcode, DCOPSend,
			    sizeof(DCOPMsg), DCOPMsg, pMsg );
	      pMsg->length += datalen;
	      IceSendData(c->iceConn, datalen, (char *) ba.data());
	  }
      }
  }
  else
      qDebug("remove unregistered connection (count=%d)", clients.count() );
  delete conn;
}

bool DCOPServer::receive(const QCString &/*app*/, const QCString &/*obj*/,
			 const QCString &fun, const QByteArray& data,
			 QCString& replyType, QByteArray &replyData,
			 IceConn iceConn)
{
  if ( fun == "registerAs(QCString)" ) {
    QDataStream args( data, IO_ReadOnly );
    if (!args.atEnd()) {
      QCString app2;
      args >> app2;
      QDataStream reply( replyData, IO_WriteOnly );
      DCOPConnection* conn = clients.find( iceConn );
      if ( conn && !app2.isEmpty() ) {
	  if ( !conn->appId.isNull() &&
	       appIds.find( conn->appId ) == conn ) {
	      appIds.remove( conn->appId );
	      qDebug("remove '%s', will be reregistered", conn->appId.data() );
	  }
	
	  conn->appId = app2;
	  if ( appIds.find( app2 ) != 0 ) {
	      // we already have this application, unify
	      int n = 1;
	      QCString tmp;
	      do {
		  n++;
		  tmp.setNum( n );
		  tmp.prepend("-");
		  tmp.prepend( app2 );
	      } while ( appIds.find( tmp ) != 0 );
	      conn->appId = tmp;
	  }
	  qDebug("register '%s'", conn->appId.data() );
	  appIds.insert( conn->appId, conn );
	  QPtrDictIterator<DCOPConnection> it( clients );
	  QByteArray data;
	  QDataStream datas( data, IO_WriteOnly );
	  datas << conn->appId;
	  QByteArray ba;
	  QDataStream ds( ba, IO_WriteOnly );
	  ds <<QCString("DCOPServer") <<  QCString("") << QCString("")
	     << QCString("applicationRegistered(QCString)") << data;
	  int datalen = ba.size();
	  DCOPMsg *pMsg = 0;
	  while ( it.current() ) {
	      DCOPConnection* c = it.current();
	      ++it;
	      if ( c != conn ) {
		  IceGetHeader( c->iceConn, majorOpcode, DCOPSend,
				sizeof(DCOPMsg), DCOPMsg, pMsg );
		  pMsg->length += datalen;
		  IceWriteData( c->iceConn, datalen, (char *) ba.data());
		  IceFlush( c->iceConn );
	      }
	  }
      }
      replyType = "QCString";
      reply << conn->appId;
      return TRUE;
    }
  }
  else if ( fun == "registeredApplications()" ) {
    QDataStream reply( replyData, IO_WriteOnly );
    QCStringList applications;
    QDictIterator<DCOPConnection> it( appIds );
    while ( it.current() ) {
      applications << it.currentKey().ascii();
      ++it;
    }
    replyType = "QCStringList";
    reply << applications;
    return TRUE;
  } else if ( fun == "isApplicationRegistered(QCString)" ) {
    QDataStream args( data, IO_ReadOnly );
    if (!args.atEnd()) {
      QCString s;
      args >> s;
      QDataStream reply( replyData, IO_WriteOnly );
      int b = ( appIds.find( s ) != 0 );
      replyType = "bool";
      reply << b;
      return TRUE;
    }
  }

  return FALSE;
}

int main( int argc, char* argv[] )
{
  QApplication a( argc, argv );
  InstallIOErrorHandler();
  DCOPServer server;

  daemon(1, 0);

  return a.exec();
}
