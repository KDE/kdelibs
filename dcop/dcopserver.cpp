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

#include <stdlib.h>
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


IceAuthDataEntry*	authDataEntries;
static IceListenObj *listenObjs = 0;
int numTransports = 0;

#define MAGIC_COOKIE_LEN 16



static IceIOErrorHandler prev_handler;

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
	//IceWriteData( target->iceConn, datalen, (char *) ba.data());
	IceSendData(target->iceConn, datalen, (char *) ba.data());
	//IceFlush( target->iceConn );
      } else if ( app == "DCOPServer" ) {
	QCString obj, fun;
	QByteArray data;
	ds >> obj >> fun >> data;
	QCString replyType;
	QByteArray replyData;
	if ( !receive( app, obj, fun, data, replyType, replyData, iceConn ) ) {
	    qWarning("%s failure: object '%s' has no function '%s'", app.data(), obj.data(), fun.data() );
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
	//IceWriteData( target->iceConn, datalen, (char *) ba.data());
	IceSendData(target->iceConn, datalen, (char *) ba.data());
	//IceFlush( target->iceConn );
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



static Bool HostBasedAuthProc ( char* /*hostname*/)
{

//   qDebug("HostBasedAuthProc for %s", hostname);
  return TRUE; // do no security at all (for now)
}

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
     * Allocate new SmsConn.
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

//   if (!IceListenForConnections (&numTransports, &listenObjs,
// 				256, errormsg))
  if (!IceListenForWellKnownConnections ((char *) "5432", 
					 &numTransports, &listenObjs,
					 256, errormsg))
    {
      fprintf (stderr, "%s\n", errormsg);
      exit (1);
    }

    atexit(CloseListeners);

 authDataEntries = new IceAuthDataEntry[ numTransports * 2  ];


 for ( int i = 0; i < numTransports * 2; i += 2)
    {
	authDataEntries[i].network_id =
	    IceGetListenConnectionString (listenObjs[i/2]);
	authDataEntries[i].protocol_name = (char *) "ICE";
	authDataEntries[i].auth_name = (char *) "MIT-MAGIC-COOKIE-1";

	authDataEntries[i].auth_data =
	    IceGenerateMagicCookie (MAGIC_COOKIE_LEN);
	authDataEntries[i].auth_data_length = MAGIC_COOKIE_LEN;

	authDataEntries[i+1].network_id =
	    IceGetListenConnectionString (listenObjs[i/2]);
	authDataEntries[i+1].protocol_name = (char *) "DCOP";
	authDataEntries[i+1].auth_name = (char *) "MIT-MAGIC-COOKIE-1";

	authDataEntries[i+1].auth_data =
	    IceGenerateMagicCookie (MAGIC_COOKIE_LEN);
	authDataEntries[i+1].auth_data_length = MAGIC_COOKIE_LEN;

//#### 	write_iceauth (addfp, removefp, authDataEntries[i]);
// 	write_iceauth (addfp, removefp, authDataEntries[i+1]);

 	IceSetPaAuthData (2, &authDataEntries[i]);

	IceSetHostBasedAuthProc (listenObjs[i/2], HostBasedAuthProc);
    }


 IceAddConnectionWatch (DCOPWatchProc, (IcePointer) this);



 listener.setAutoDelete( TRUE );
 DCOPListener* con;
    for ( int i = 0; i < numTransports; i++) {
      con = new DCOPListener( listenObjs[i] );
      listener.append( con );
      connect( con, SIGNAL( activated(int) ), this, SLOT( newClient(int) ) );
    }

    char * networkIds = IceComposeNetworkIdList (numTransports, listenObjs);
    qDebug("networkids: %s", networkIds );

    free ( networkIds );
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

bool DCOPServer::receive(const QCString &app, const QCString &obj,
			 const QCString &fun, const QByteArray& data,
			 QCString& replyType, QByteArray &replyData,
			 IceConn iceConn)
{
  if ( fun == "registerAs(QCString)" ) {
    QDataStream args( data, IO_ReadOnly );
    if (!args.atEnd()) {
      QCString app;
      args >> app;
      QDataStream reply( replyData, IO_WriteOnly );
      DCOPConnection* conn = clients.find( iceConn );
      if ( conn && !app.isEmpty() ) {
	  if ( !conn->appId.isNull() &&
	       appIds.find( conn->appId ) == conn ) {
	      appIds.remove( conn->appId );
	      qDebug("remove '%s', will be reregistered", conn->appId.data() );
	  }
	
	  conn->appId = app;
	  if ( appIds.find( app ) != 0 ) {
	      // we already have this application, unify
	      int n = 1;
	      QCString tmp;
	      do {
		  n++;
		  tmp.setNum( n );
		  tmp.prepend("-");
		  tmp.prepend( app );
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

  return a.exec();
}
