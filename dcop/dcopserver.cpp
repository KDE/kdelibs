/*****************************************************************

Copyright (c) 1999,2000 Preston Brown <pbrown@kde.org>
Copyright (c) 1999,2000 Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#define QT_CLEAN_NAMESPACE 1
#include <qfile.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qstack.h>

#include <dcopserver.h>
#include <dcopglobal.h>

template class QDict<DCOPConnection>;
template class QList<_IceConn>;
template class QPtrDict<DCOPConnection>;
template class QList<DCOPListener>;

static Bool HostBasedAuthProc ( char* /*hostname*/)
{
    return false; // no host based authentication
}

// SM DUMMY
#include <X11/SM/SMlib.h>

static Status NewClientProc ( SmsConn, SmPointer, unsigned long*, SmsCallbacks*, char** )
{
    return 0;
};

static void registerXSM()
{
    char 	errormsg[256];
    if (!SmsInitialize ((char *)"SAMPLE-SM", (char *)"1.0",
			NewClientProc, NULL,
			HostBasedAuthProc, 256, errormsg))
	{
	    qFatal("register xsm failed");
	}
}



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
    notifyRegister = false;
    time = 0;
}

    QCString appId;
    IceConn iceConn;
    bool notifyRegister;
    CARD32 time;
    QList < _IceConn> waitingForReply;
    QList < _IceConn> waitingForDelayedReply;
};

IceAuthDataEntry *authDataEntries = 0;
static char *addAuthFile = 0;
static char *remAuthFile = 0;

static IceListenObj *listenObjs = 0;
int numTransports = 0;
static int ready[2];


/* for printing hex digits */
static void fprintfhex (FILE *fp, unsigned int len, char *cp)
{
    static char hexchars[] = "0123456789abcdef";

    for (; len > 0; len--, cp++) {
	unsigned char s = *cp;
	putc(hexchars[s >> 4], fp);
	putc(hexchars[s & 0x0f], fp);
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


#ifndef HAVE_MKSTEMP
static char *unique_filename (const char *path, const char *prefix)
#else
static char *unique_filename (const char *path, const char *prefix, int *pFd)
#endif
{
#ifndef HAVE_MKSTEMP
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
#ifdef HAVE_MKSTEMP
    int         fd;
#endif

    original_umask = umask (0077);      /* disallow non-owner access */

    path = getenv ("DCOP_SAVE_DIR");
    if (!path)
	path = "/tmp";
#ifndef HAVE_MKSTEMP
    if ((addAuthFile = unique_filename (path, "dcop")) == NULL)
	goto bad;

    if (!(addfp = fopen (addAuthFile, "w")))
	goto bad;

    if ((remAuthFile = unique_filename (path, "dcop")) == NULL)
	goto bad;

    if (!(removefp = fopen (remAuthFile, "w")))
	goto bad;
#else
    if ((addAuthFile = unique_filename (path, "dcop", &fd)) == NULL)
	goto bad;

    if (!(addfp = fdopen(fd, "wb")))
	goto bad;

    if ((remAuthFile = unique_filename (path, "dcop", &fd)) == NULL)
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
    case DCOPReplyDelayed:
	{
	    DCOPMsg *pMsg = 0;
	    IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
	    QByteArray ba( length );
	    IceReadData(iceConn, length, ba.data() );
	    QDataStream ds( ba, IO_ReadOnly );
	    QCString appFrom, app;
	    ds >> appFrom >> app;
	    DCOPConnection* target = appIds.find( app );
	    DCOPConnection* conn = clients.find( iceConn );
	    if ( opcode == DCOPReplyDelayed ) {
// 		qDebug("DCOPServer::Got DCOPReplyDelayed (from: \"%s\" to: \"%s\")",
// 		       appFrom.data(), app.data());
		if ( !target )
		    qWarning("DCOPServer::DCOPReplyDelayed for unknown connection.");
		else if ( !conn )
		    qWarning("DCOPServer::DCOPReplyDelayed from unknown connection.");
		else if (!conn->waitingForDelayedReply.remove( target->iceConn ))
		    qWarning("DCOPServer::DCOPReplyDelayed from/to does not match. (#2)");
	    }
	    if ( target ) {
		IceGetHeader( target->iceConn, majorOpcode, opcode,
			      sizeof(DCOPMsg), DCOPMsg, pMsg );
		pMsg->time = ++time;
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
	    } else if ( app[app.length()-1] == '*') {
		// handle a multicast.
		QDictIterator<DCOPConnection> aIt(appIds);
		int l = app.length()-1;
		for ( ; aIt.current(); ++aIt) {
		    DCOPConnection *client = aIt.current();
		    if (!l || (strncmp(client->appId.data(), app.data(), l) == 0))
			{
			    IceGetHeader(client->iceConn, majorOpcode, DCOPSend,
					 sizeof(DCOPMsg), DCOPMsg, pMsg);
			    pMsg->time = ++time;
			    int datalen = ba.size();
			    pMsg->length += datalen;
			    IceSendData(client->iceConn, datalen, (char *) ba.data());
			}
		}
	    }
	}
	break;
    case DCOPCall:
    case DCOPFind:
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
	
	    DCOPConnection* conn = clients.find( iceConn );
	    if ( (!conn->waitingForReply.isEmpty() && conn->time > pMsg->time)
		 || ( target && !target->waitingForReply.isEmpty() && target->time > pMsg->time ) ) {
		//qDebug("reject call  %ld < %ld", pMsg->time, conn->time);
		IceGetHeader( iceConn, majorOpcode, DCOPCallRejected,
			      sizeof(DCOPMsg), DCOPMsg, pMsg );
		pMsg->time = ++time; // oldtime;
		IceFlush( iceConn );
		return;
	    }
	
	    if ( target ) {
		target->waitingForReply.append( iceConn );
		IceGetHeader( target->iceConn, majorOpcode, opcode,
			      sizeof(DCOPMsg), DCOPMsg, pMsg );
		pMsg->time = ++time;
		target->time = pMsg->time;
		pMsg->length += datalen;
		IceSendData(target->iceConn, datalen, (char *) ba.data());
	    } else {
		QCString replyType;
		QByteArray replyData;
		bool b = FALSE;
                // DCOPServer itself does not do DCOPFind.
		if ( (opcode == DCOPCall) && (app == "DCOPServer") ) {
		    QCString obj, fun;
		    QByteArray data;
		    ds >> obj >> fun >> data;
		    b = receive( app, obj, fun, data, replyType, replyData, iceConn );
		    if ( !b )
			qWarning("%s failure: object '%s' has no function '%s'", app.data(), obj.data(), fun.data() );
		}

		if (b) {
		    QByteArray reply;
		    QDataStream replyStream( reply, IO_WriteOnly );
		    replyStream << replyType << replyData.size();
		    int datalen = reply.size() + replyData.size();
		    IceGetHeader( iceConn, majorOpcode, DCOPReply,
				  sizeof(DCOPMsg), DCOPMsg, pMsg );
		    pMsg->time = ++time;
		    pMsg->length += datalen;
		    IceSendData( iceConn, reply.size(), (char *) reply.data());
		    IceSendData( iceConn, replyData.size(), (char *) replyData.data());
		} else {
		    QByteArray reply;
		    IceGetHeader( iceConn, majorOpcode, DCOPReplyFailed,
				  sizeof(DCOPMsg), DCOPMsg, pMsg );
		    pMsg->time = ++time;
		    pMsg->length += reply.size();
		    IceSendData( iceConn, reply.size(), (char *) reply.data());
		}
	    }
	}
	break;
    case DCOPReply:
    case DCOPReplyFailed:
    case DCOPReplyWait:
	{
	    DCOPMsg *pMsg = 0;
	    IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
	    QByteArray ba( length );
	    IceReadData(iceConn, length, ba.data() );
	    DCOPConnection* conn = clients.find( iceConn );
	    if ( !conn ) {
		qWarning("DCOPServer::DCOPReply from unknown connection.");
		break;
	    }
	    DCOPConnection* connreply = clients.find( conn->waitingForReply.take(0) );
	    if ( !connreply )
		qWarning("DCOPServer::DCOPReply for unknown connection.");
	    else {
		if ( opcode == DCOPReplyWait )
		    conn->waitingForDelayedReply.append( connreply->iceConn );
		IceGetHeader( connreply->iceConn, majorOpcode, opcode,
			      sizeof(DCOPMsg), DCOPMsg, pMsg );
		pMsg->time = ++time;
		int datalen = ba.size();
		pMsg->length += datalen;
		IceSendData(connreply->iceConn, datalen, (char *) ba.data());
	    }
	}
	break;
    default:
	qWarning("DCOPServer::processMessage unknown message");
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


static void sighandler(int sig)
{
    if (sig == SIGHUP) {
	signal(SIGHUP, sighandler);
	return;
    }

    delete the_server;
    exit(0);
}

DCOPServer::DCOPServer()
    : QObject(0,0), appIds(263), clients(263)
{
    time = 0; // the beginning of time....

    extern int _IceLastMajorOpcode; // from libICE
    if (_IceLastMajorOpcode < 1 )
	registerXSM();

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
	    qWarning("Could not register DCOP protocol with ICE");
	}

    char errormsg[256];
    if (!IceListenForConnections (&numTransports, &listenObjs,
				  256, errormsg))
	{
	    fprintf (stderr, "%s\n", errormsg);
	    exit (1);
	} else {
	    // publish available transports.
	    QCString fName = ::getenv("HOME");
	    fName += "/.DCOPserver";
	    FILE *f;
	    f = ::fopen(fName.data(), "w+");
	    fprintf(f, IceComposeNetworkIdList(numTransports, listenObjs));
	    fprintf(f, "\n%i\n", getpid());
	    fclose(f);
	}

    if (!SetAuthentication(numTransports, listenObjs, &authDataEntries))
	qFatal("DCOPSERVER: authentication setup failed.");

    IceAddConnectionWatch (DCOPWatchProc, (IcePointer) this);

    listener.setAutoDelete( TRUE );
    DCOPListener* con;
    for ( int i = 0; i < numTransports; i++) {
	con = new DCOPListener( listenObjs[i] );
	listener.append( con );
	connect( con, SIGNAL( activated(int) ), this, SLOT( newClient(int) ) );
    }
    char c;
    (void) write(ready[1], &c, 1); // dcopserver is started
    close(ready[1]);
}

DCOPServer::~DCOPServer()
{
    IceFreeListenObjs (numTransports, listenObjs);

    QCString fName;
    fName = ::getenv("DCOPSERVER");
    if (fName.isNull()) {
	fName = ::getenv("HOME");
	fName += "/.DCOPserver";
	unlink(fName.data());
    }

    FreeAuthenticationData(numTransports, authDataEntries);
}


/*!
  Called from our IceIoErrorHandler
 */
void DCOPServer::ioError( IceConn iceConn )
{
    IceSetShutdownNegotiation (iceConn, False);
    IceCloseConnection( iceConn );
}


void DCOPServer::processData( int /*socket*/ )
{
    (void ) IceProcessMessages( ((DCOPConnection*)sender())->iceConn, 0, 0 );
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
    if (cstatus != IceConnectAccepted) {
	if (cstatus == IceConnectIOError)
	    qWarning ("IO error opening ICE Connection!\n");
	else
	    qWarning ("ICE Connection rejected!\n");
	IceCloseConnection (iceConn);
    }
}

void* DCOPServer::watchConnection( IceConn iceConn )
{
    DCOPConnection* con = new DCOPConnection( iceConn );
    connect( con, SIGNAL( activated(int) ), this, SLOT( processData(int) ) );

    clients.insert(iceConn, con );

    return (void*) con;
}

void DCOPServer::removeConnection( void* data )
{
    DCOPConnection* conn = (DCOPConnection*)data;
    clients.remove(conn->iceConn );

    // Send DCOPReplyFailed to all in conn->waitingForReply
    while (!conn->waitingForReply.isEmpty()) {
	IceConn iceConn = conn->waitingForReply.take(0);
	if (iceConn) {
	    DCOPConnection* target = clients.find( iceConn );
	    qWarning("DCOP aborting call from '%s' to '%s'", target->appId.data(), conn->appId.data() );
	    QByteArray reply;
	    DCOPMsg *pMsg;
	    IceGetHeader( iceConn, majorOpcode, DCOPReplyFailed,
			  sizeof(DCOPMsg), DCOPMsg, pMsg );
	    pMsg->time = ++time;
	    pMsg->length += reply.size();
	    IceSendData( iceConn, reply.size(), (char *) reply.data());
	}
    }

    // Send DCOPReplyFailed to all in conn->waitingForDelayedReply
    while (!conn->waitingForDelayedReply.isEmpty()) {
	IceConn iceConn = conn->waitingForDelayedReply.take();
	if (iceConn) {
	    DCOPConnection* target = clients.find( iceConn );
	    qWarning("DCOP aborting (delayed) call from '%s' to '%s'", target->appId.data(), conn->appId.data() );
	    QByteArray reply;
	    DCOPMsg *pMsg;
	    IceGetHeader( iceConn, majorOpcode, DCOPReplyFailed,
			  sizeof(DCOPMsg), DCOPMsg, pMsg );
	    pMsg->time = ++time;
	    pMsg->length += reply.size();
	    IceSendData( iceConn, reply.size(), (char *) reply.data());
	}
    }

    // Send DCOPReplyDelayed with 0 seq to all in conn->waitingDelayedReply
    if ( !conn->appId.isNull() ) {
	qDebug("DCOP:  unregister '%s'", conn->appId.data() );
	appIds.remove( conn->appId );

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
	    if ( c->notifyRegister && (c != conn) ) {
		IceGetHeader( c->iceConn, majorOpcode, DCOPSend,
			      sizeof(DCOPMsg), DCOPMsg, pMsg );
		pMsg->time = ++time;
		pMsg->length += datalen;
		IceSendData(c->iceConn, datalen, (char *) ba.data());
	    }
	}
    }
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
		}

		if ( conn->appId.isNull() )
		    qDebug("DCOP: register '%s'", app2.data() );
		else
		    qDebug("DCOP:  '%s' now known as '%s'", conn->appId.data(), app2.data() );

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
		    if ( c->notifyRegister && (c != conn) ) {
			IceGetHeader( c->iceConn, majorOpcode, DCOPSend,
				      sizeof(DCOPMsg), DCOPMsg, pMsg );
			pMsg->time = ++time;
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
    } else if ( fun == "setNotifications(bool)" ) {
	QDataStream args( data, IO_ReadOnly );
	if (!args.atEnd()) {
	    Q_INT8 notifyActive;
	    args >> notifyActive;
	    DCOPConnection* conn = clients.find( iceConn );
	    if ( conn )
		conn->notifyRegister = (notifyActive != 0);
	    replyType = "void";
	    return TRUE;
	}
    }

    return FALSE;
}

#if defined(X_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE X_POSIX_C_SOURCE
#include <setjmp.h>
#undef _POSIX_C_SOURCE
#elif defined(X_NOT_POSIX) || defined(_POSIX_SOURCE)
#include <setjmp.h>
#else
#define _POSIX_SOURCE
#include <setjmp.h>
#undef _POSIX_SOURCE
#endif
jmp_buf JumpHere;

void IoErrorHandler ( IceConn iceConn)
{
    the_server->ioError( iceConn );
    longjmp (JumpHere, 1);
}

const char* const ABOUT =
"Usage: dcopserver [--nofork] [--nosid] [--help]\n"
"\n"
"DCOP is KDE's Desktop Communications Protocol. It is a lightweight IPC/RPC\n"
"mechanism built on top of the X Consortium's Inter Client Exchange protocol.\n"
"It enables desktop applications to communicate reliably with low overhead.\n"
"\n"
"Copyright (C) 1999-2000, The KDE Developers <http://www.kde.org>\n"
;


int main( int argc, char* argv[] )
{
    bool nofork = false;
    bool nosid = false;
    for(int i = 1; i < argc; i++) {
	if (strcmp(argv[i], "--nofork") == 0)
	    nofork = true;
	else if (strcmp(argv[i], "--nosid") == 0)
	    nosid = true;
	else {
	    fprintf(stdout, ABOUT );
	    exit(0);
	}
    }

    // check if we are already running
    QCString fName = ::getenv("HOME");
    fName += "/.DCOPserver";
    if (::access(fName.data(), R_OK) == 0) {
	QFile f(fName);
	f.open(IO_ReadOnly);
	QTextStream t(&f);
	t.readLine(); // skip over connection list
	bool ok = false;
	pid_t pid = t.readLine().toUInt(&ok);
	f.close();
	if (ok && (kill(pid, SIGHUP) == 0)) {
	    qWarning( "---------------------------------\n"
		      "It looks like dcopserver is already running. If you are sure\n"
		      "that it is not already running, remove %s\n"
		      "and start dcopserver again.\n"
		      "---------------------------------\n",
		      fName.data() );

	    // lock file present, die silently.
	    exit(0);
	} else {
	    // either we couldn't read the PID or kill returned an error.
	    // remove lockfile and continue
	    unlink(fName.data());
	}
    }

    pipe(ready);

    if (!nofork)
	{
	    if (fork() > 0)
		{
		    char c;
		    close(ready[1]);
		    (void) read(ready[0], &c, 1); // Wait till dcopserver is started
		    close(ready[0]);
		    exit(0); // I am the parent
		}
	    close(ready[0]);

	    if (!nosid)
		setsid();

	    if (fork() > 0)
		exit(0); // get rid of controlling terminal
	}

    signal(SIGHUP, sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGPIPE, SIG_IGN);

    putenv("SESSION_MANAGER=");

    QApplication a( argc, argv, false );

    IceSetIOErrorHandler (IoErrorHandler );
    DCOPServer *server = new DCOPServer();

    setjmp (JumpHere);
    a.exec();
    delete server;    
}

#include "dcopserver.moc"
