/*****************************************************************

#include "dcopserver.h"

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

#include <config.h>

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
#include <unistd.h>
#include <fcntl.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#define QT_CLEAN_NAMESPACE 1
#include <qfile.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qstack.h>
#include <qtimer.h>

#include <dcopserver.h>
#include <dcopsignals.h>
#include <dcopglobal.h>

template class QDict<DCOPConnection>;
template class QPtrDict<DCOPConnection>;
template class QList<DCOPListener>;

#define _DCOPIceSendBegin(x)	\
   int fd = IceConnectionNumber( x );		\
   long fd_fl = fcntl(fd, F_GETFL, 0);		\
   fcntl(fd, F_SETFL, fd_fl | O_NDELAY);	
#define _DCOPIceSendEnd()	\
   fcntl(fd, F_SETFL, fd_fl);		

static bool only_local = false;

static Bool HostBasedAuthProc ( char* /*hostname*/)
{
    return only_local; // no host based authentication
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
    if (!SmsInitialize (const_cast<char *>("SAMPLE-SM"), const_cast<char *>("1.0"),
			NewClientProc, NULL,
			HostBasedAuthProc, 256, errormsg))
	{
	    qFatal("register xsm failed");
	}
}

QCString dcopServerFile()
{
   QCString fName = ::getenv("HOME");
   if (fName.isEmpty())
   {
      fprintf(stderr, "Aborting. $HOME is not set.\n");
      exit(1);
   }
   QCString disp = getenv("DISPLAY");
   if (disp.isEmpty())
   {
      fprintf(stderr, "Aborting. $DISPLAY is not set.\n");
      exit(1);
   }
   int i;
   if((i = disp.findRev('.')) > disp.findRev(':') && i >= 0)
       disp.truncate(i);

   fName += "/.DCOPserver_";
   char hostName[256];
   if (gethostname(hostName, 255))
      fName += "localhost";
   else
      fName += hostName;
   fName += "_"+disp;
   return fName;
}


DCOPServer* the_server = 0;

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

DCOPConnection::DCOPConnection( IceConn conn )
	: QSocketNotifier( IceConnectionNumber( conn ),
			   QSocketNotifier::Read, 0, 0 )
{
    iceConn = conn;
    notifyRegister = false;
    _signalConnectionList = 0;
    daemon = false;
}

DCOPConnection::~DCOPConnection()
{
    delete _signalConnectionList;
}

DCOPSignalConnectionList *
DCOPConnection::signalConnectionList()
{
    if (!_signalConnectionList)
       _signalConnectionList = new DCOPSignalConnectionList;
    return _signalConnectionList;
}

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
    ptr = static_cast<char *>(malloc(strlen(tempFile) + 1));
    if (ptr != NULL)
	{
	    strcpy(ptr, tempFile);
	    *pFd =  mkstemp(ptr);
	}
    return ptr;
#endif
}

#define MAGIC_COOKIE_LEN 16

Status SetAuthentication_local (int count, IceListenObj *listenObjs)
{
    int i;
    for (i = 0; i < count; i ++) {
	char *prot = IceGetListenConnectionString(listenObjs[i]);
	if (!prot) continue;
	char *host = strchr(prot, '/');
	char *sock = 0;
	if (host) {
	    *host=0;
	    host++;
	    sock = strchr(host, ':');
	    if (sock) {
		*sock = 0;
		sock++;
	    }
	}
#ifndef NDEBUG
	qDebug("DCOPServer: SetAProc_loc: conn %d, prot=%s, file=%s",
		(unsigned)i, prot, sock);
#endif
	if (sock && !strcmp(prot, "local")) {
	    chmod(sock, 0700);
	}
	IceSetHostBasedAuthProc (listenObjs[i], HostBasedAuthProc);
	free(prot);
    }
    return 1;
}

Status
SetAuthentication (int count, IceListenObj *_listenObjs,
		   IceAuthDataEntry **_authDataEntries)
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

    if ((*_authDataEntries = static_cast<IceAuthDataEntry *>(malloc (count * 2 * sizeof (IceAuthDataEntry)))) == NULL)
	goto bad;

    for (i = 0; i < numTransports * 2; i += 2) {
	(*_authDataEntries)[i].network_id =
	    IceGetListenConnectionString (_listenObjs[i/2]);
	(*_authDataEntries)[i].protocol_name = const_cast<char *>("ICE");
	(*_authDataEntries)[i].auth_name = const_cast<char *>("MIT-MAGIC-COOKIE-1");

	(*_authDataEntries)[i].auth_data =
	    IceGenerateMagicCookie (MAGIC_COOKIE_LEN);
	(*_authDataEntries)[i].auth_data_length = MAGIC_COOKIE_LEN;

	(*_authDataEntries)[i+1].network_id =
	    IceGetListenConnectionString (_listenObjs[i/2]);
	(*_authDataEntries)[i+1].protocol_name = const_cast<char *>("DCOP");
	(*_authDataEntries)[i+1].auth_name = const_cast<char *>("MIT-MAGIC-COOKIE-1");

	(*_authDataEntries)[i+1].auth_data =
	    IceGenerateMagicCookie (MAGIC_COOKIE_LEN);
	(*_authDataEntries)[i+1].auth_data_length = MAGIC_COOKIE_LEN;

	write_iceauth (addfp, removefp, &(*_authDataEntries)[i]);
	write_iceauth (addfp, removefp, &(*_authDataEntries)[i+1]);

	IceSetPaAuthData (2, &(*_authDataEntries)[i]);

	IceSetHostBasedAuthProc (_listenObjs[i/2], HostBasedAuthProc);
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

    umask (original_umask);

    return (0);
}

/*
 * Free up authentication data.
 */
void
FreeAuthenticationData(int count, IceAuthDataEntry *_authDataEntries)
{
    /* Each transport has entries for ICE and XSMP */

    char command[256];
    int i;

    if (only_local)
	return;

    for (i = 0; i < count * 2; i++) {
	free (_authDataEntries[i].network_id);
	free (_authDataEntries[i].auth_data);
    }

    free (_authDataEntries);

    sprintf (command, "iceauth source %s", remAuthFile);
    system(command);

    unlink(remAuthFile);

    free(addAuthFile);
    free(remAuthFile);
}

void DCOPWatchProc ( IceConn iceConn, IcePointer client_data, Bool opening, IcePointer* watch_data)
{
    DCOPServer* ds = static_cast<DCOPServer*>(client_data);

    if (opening) {
	*watch_data = static_cast<IcePointer>(ds->watchConnection( iceConn ));
    }
    else  {
	ds->removeConnection( static_cast<void*>(*watch_data) );
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
    DCOPConnection* conn = clients.find( iceConn );
    if ( !conn ) {
	qWarning("DCOPServer::processMessage message from unknown connection. [opcode = %d]", opcode);
	return;
    }
    switch( opcode ) {
    case DCOPSend:
    case DCOPReplyDelayed:
	{
	    DCOPMsg *pMsg = 0;
	    IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
	    CARD32 key = pMsg->key;
	    QByteArray ba( length );
	    IceReadData(iceConn, length, ba.data() );
	    QDataStream ds( ba, IO_ReadOnly );
	    QCString fromApp, toApp;
	    ds >> fromApp >> toApp;
	    DCOPConnection* target = findApp( toApp );
	    int datalen = ba.size();
	    if ( opcode == DCOPReplyDelayed ) {
		if ( !target )
		    qWarning("DCOPServer::DCOPReplyDelayed for unknown connection.");
		else if ( !conn )
		    qWarning("DCOPServer::DCOPReplyDelayed from unknown connection.");
		else if (!conn->waitingForDelayedReply.removeRef( target->iceConn ))
		    qWarning("DCOPServer::DCOPReplyDelayed from/to does not match. (#2)");
                else if (!target->waitingOnReply.removeRef(iceConn))
                       qWarning("DCOPServer::DCOPReplyDelayed for client who wasn't waiting on one!");
	    }
	    if ( target ) {
		IceGetHeader( target->iceConn, majorOpcode, opcode,
			      sizeof(DCOPMsg), DCOPMsg, pMsg );
		pMsg->key = key;
		pMsg->length += datalen;
		_DCOPIceSendBegin( target->iceConn );
		IceSendData(target->iceConn, datalen, (char *) ba.data());
                _DCOPIceSendEnd();
	    } else if ( toApp == "DCOPServer" ) {
		QCString obj, fun;
		QByteArray data;
		ds >> obj >> fun >> data;
		QCString replyType;
		QByteArray replyData;
		if ( !receive( toApp, obj, fun, data, replyType, replyData, iceConn ) ) {
		    qWarning("%s failure: object '%s' has no function '%s'", toApp.data(), obj.data(), fun.data() );
		}
	    } else if ( toApp[toApp.length()-1] == '*') {
		// handle a multicast.
		QAsciiDictIterator<DCOPConnection> aIt(appIds);
		int l = toApp.length()-1;
		for ( ; aIt.current(); ++aIt) {
		    DCOPConnection *client = aIt.current();
		    if (!l || (strncmp(client->appId.data(), toApp.data(), l) == 0))
			{
			    IceGetHeader(client->iceConn, majorOpcode, DCOPSend,
					 sizeof(DCOPMsg), DCOPMsg, pMsg);
			    pMsg->key = key;
			    pMsg->length += datalen;
			    _DCOPIceSendBegin( client->iceConn );
			    IceSendData(client->iceConn, datalen, const_cast<char *>(ba.data()) );
                            _DCOPIceSendEnd();
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
	    CARD32 key = pMsg->key;
	    QByteArray ba( length );
	    IceReadData(iceConn, length, ba.data() );
	    QDataStream ds( ba, IO_ReadOnly );
	    QCString fromApp, toApp;
	    ds >> fromApp >> toApp;
	    DCOPConnection* target = findApp( toApp );
	    int datalen = ba.size();

	    if ( target ) {
		target->waitingForReply.append( iceConn );
                conn->waitingOnReply.append( target->iceConn);

		IceGetHeader( target->iceConn, majorOpcode, opcode,
			      sizeof(DCOPMsg), DCOPMsg, pMsg );
		pMsg->key = key;
		pMsg->length += datalen;
		_DCOPIceSendBegin( target->iceConn );
		IceSendData(target->iceConn, datalen, const_cast<char *>(ba.data()) );
 		IceFlush( target->iceConn );
                _DCOPIceSendEnd();
	    } else {
		QCString replyType;
		QByteArray replyData;
		bool b = FALSE;
		// DCOPServer itself does not do DCOPFind.
		if ( (opcode == DCOPCall) && (toApp == "DCOPServer") ) {
		    QCString obj, fun;
		    QByteArray data;
		    ds >> obj >> fun >> data;
		    b = receive( toApp, obj, fun, data, replyType, replyData, iceConn );
		    if ( !b )
			qWarning("%s failure: object '%s' has no function '%s'", toApp.data(), obj.data(), fun.data() );
		}

		if (b) {
		    QByteArray reply;
		    QDataStream replyStream( reply, IO_WriteOnly );
		    replyStream << toApp << fromApp << replyType << replyData.size();
		    int replylen = reply.size() + replyData.size();
		    IceGetHeader( iceConn, majorOpcode, DCOPReply,
				  sizeof(DCOPMsg), DCOPMsg, pMsg );
		    if ( key != 0 )
			pMsg->key = key;
		    else
			pMsg->key = serverKey++;
		    pMsg->length += replylen;
                    _DCOPIceSendBegin( iceConn );
		    IceSendData( iceConn, reply.size(), const_cast<char *>(reply.data()) );
		    IceSendData( iceConn, replyData.size(), const_cast<char *>(replyData.data()) );
                    _DCOPIceSendEnd();
		} else {
		    QByteArray reply;
		    QDataStream replyStream( reply, IO_WriteOnly );
		    replyStream << toApp << fromApp;
		    IceGetHeader( iceConn, majorOpcode, DCOPReplyFailed,
				  sizeof(DCOPMsg), DCOPMsg, pMsg );
		    if ( key != 0 )
			pMsg->key = key;
		    else
			pMsg->key = serverKey++;
		    pMsg->length += reply.size();
                    _DCOPIceSendBegin( iceConn );
		    IceSendData( iceConn, reply.size(), const_cast<char *>(reply.data()) );
                    _DCOPIceSendEnd();
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
	    CARD32 key = pMsg->key;
	    QByteArray ba( length );
	    IceReadData(iceConn, length, ba.data() );
	    QDataStream ds( ba, IO_ReadOnly );
	    QCString fromApp, toApp;
	    ds >> fromApp >> toApp;

	    DCOPConnection* connreply = findApp( toApp );
	    int datalen = ba.size();

	    if ( !connreply )
		qWarning("DCOPServer::DCOPReply for unknown connection.");
	    else {
		conn->waitingForReply.removeRef( connreply->iceConn );
		if ( opcode == DCOPReplyWait )
                {
		    conn->waitingForDelayedReply.append( connreply->iceConn );
                }
                else
                { // DCOPReply or DCOPReplyFailed
                    if (!connreply->waitingOnReply.removeRef(iceConn))
                       qWarning("DCOPServer::DCOPReply for client who wasn't waiting on one!");
                }
		IceGetHeader( connreply->iceConn, majorOpcode, opcode,
			      sizeof(DCOPMsg), DCOPMsg, pMsg );
		pMsg->key = key;
		pMsg->length += datalen;
                _DCOPIceSendBegin( connreply->iceConn );
		IceSendData(connreply->iceConn, datalen, const_cast<char *>(ba.data()) );
                _DCOPIceSendEnd();
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

    *clientDataRet = static_cast<IcePointer>(serverConn);


    return 1;
}


static void sighandler(int sig)
{
    if (sig == SIGHUP) {
	signal(SIGHUP, sighandler);
	return;
    }

    qApp->quit();
    //exit(0);
}

#ifdef HAVE__ICETRANSNOLISTEN
extern "C" int _IceTransNoListen(const char *protocol);
#endif

DCOPServer::DCOPServer(bool _only_local)
    : QObject(0,0), appIds(263), clients(263), currentClientNumber(0)
{
    serverKey = 42;

    only_local = _only_local;

#ifdef HAVE__ICETRANSNOLISTEN
    if (only_local)
	_IceTransNoListen("tcp");
#else
    only_local = false;
#endif

    dcopSignals = new DCOPSignals;

    extern int _IceLastMajorOpcode; // from libICE
    if (_IceLastMajorOpcode < 1 )
	registerXSM();

    the_server = this;
    if (( majorOpcode = IceRegisterForProtocolReply (const_cast<char *>("DCOP"),
						     const_cast<char *>(DCOPVendorString),
						     const_cast<char *>(DCOPReleaseString),
						     1, DCOPVersions,
						     1, const_cast<char **>(DCOPAuthNames),
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
    int orig_umask = umask(0); /*old libICE's don't reset the umask() they set */
    if (!IceListenForConnections (&numTransports, &listenObjs,
				  256, errormsg))
	{
	    fprintf (stderr, "%s\n", errormsg);
	    exit (1);
	} else {
	    (void) umask(orig_umask);
	    // publish available transports.
	    QCString fName = dcopServerFile();
	    FILE *f;
	    f = ::fopen(fName.data(), "w+");
	    char *idlist = IceComposeNetworkIdList(numTransports, listenObjs);
	    if (idlist != 0) {
	        fprintf(f, idlist);
		free(idlist);
	    }
	    fprintf(f, "\n%i\n", getpid());
	    fclose(f);
            // Create a link named like the old-style (KDE 2.0) naming
            QCString compatName(fName);
            compatName.truncate(compatName.findRev('_'));
            ::symlink(fName,compatName);
	}

    if (only_local) {
	if (!SetAuthentication_local(numTransports, listenObjs))
	    qFatal("DCOPSERVER: authentication setup failed.");
    } else {
	if (!SetAuthentication(numTransports, listenObjs, &authDataEntries))
	    qFatal("DCOPSERVER: authentication setup failed.");
    }

    IceAddConnectionWatch (DCOPWatchProc, static_cast<IcePointer>(this));

    listener.setAutoDelete( TRUE );
    DCOPListener* con;
    for ( int i = 0; i < numTransports; i++) {
	con = new DCOPListener( listenObjs[i] );
	listener.append( con );
	connect( con, SIGNAL( activated(int) ), this, SLOT( newClient(int) ) );
    }
    char c = 0;
    write(ready[1], &c, 1); // dcopserver is started
    close(ready[1]);

    m_timer =  new QTimer(this);
    connect( m_timer, SIGNAL(timeout()), this, SLOT(slotTerminate()) );
    m_deadConnectionTimer = new QTimer(this);
    connect( m_deadConnectionTimer, SIGNAL(timeout()), this, SLOT(slotCleanDeadConnections()) );
}

DCOPServer::~DCOPServer()
{
    IceFreeListenObjs (numTransports, listenObjs);

    QCString fName;
    fName = ::getenv("DCOPSERVER");
    if (fName.isNull()) {
	fName = dcopServerFile();
    }
    unlink(fName.data());

    FreeAuthenticationData(numTransports, authDataEntries);
    delete dcopSignals;
}


DCOPConnection* DCOPServer::findApp( const QCString& appId )
{
    if ( appId.isNull() )
	return 0;
    DCOPConnection* conn = appIds.find( appId );
    if ( !conn && appId.find( '-' ) < 0 ) {
	for ( QAsciiDictIterator<DCOPConnection> it( appIds ); it.current(); ++it )
	    if ( it.current()->plainAppId == appId )
		return it.current();
    }
    return conn;
}

/*!
  Called by timer after write errors.
 */
void DCOPServer::slotCleanDeadConnections()
{
qWarning("DCOP Cleaning up dead connections.");
    while(!deadConnections.isEmpty())
    {
       IceConn iceConn = deadConnections.take(0);
       IceSetShutdownNegotiation (iceConn, False);
       (void) IceCloseConnection( iceConn );
    }
}

/*!
  Called from our IceIoErrorHandler
 */
void DCOPServer::ioError( IceConn iceConn  )
{
    deadConnections.removeRef(iceConn);
    deadConnections.prepend(iceConn);
    m_deadConnectionTimer->start(0, true);
}


void DCOPServer::processData( int /*socket*/ )
{
    IceConn iceConn = static_cast<const DCOPConnection*>(sender())->iceConn;
    IceProcessMessagesStatus status = IceProcessMessages( iceConn, 0, 0 );
    if ( status == IceProcessMessagesIOError ) {
        deadConnections.removeRef(iceConn);
        if (deadConnections.isEmpty())
           m_deadConnectionTimer->stop();
	IceSetShutdownNegotiation (iceConn, False);
	(void) IceCloseConnection( iceConn );
    }
}

void DCOPServer::newClient( int /*socket*/ )
{
    IceAcceptStatus status;
    IceConn iceConn = IceAcceptConnection( static_cast<const  DCOPListener*>(sender())->listenObj, &status);
    IceSetShutdownNegotiation( iceConn, False );

    IceConnectStatus cstatus;
    while ((cstatus = IceConnectionStatus (iceConn))==IceConnectPending) {
	(void) IceProcessMessages( iceConn, 0, 0 );
    }

    if (cstatus != IceConnectAccepted) {
	if (cstatus == IceConnectIOError)
	    qWarning ("IO error opening ICE Connection!\n");
	else
	    qWarning ("ICE Connection rejected!\n");
        deadConnections.removeRef(iceConn);
	(void) IceCloseConnection (iceConn);
    }
}

void* DCOPServer::watchConnection( IceConn iceConn )
{
    DCOPConnection* con = new DCOPConnection( iceConn );
    connect( con, SIGNAL( activated(int) ), this, SLOT( processData(int) ) );

    clients.insert(iceConn, con );

    return static_cast<void*>(con);
}

void DCOPServer::removeConnection( void* data )
{
    DCOPConnection* conn = static_cast<DCOPConnection*>(data);

    dcopSignals->removeConnections(conn);

    clients.remove(conn->iceConn );

    // Send DCOPReplyFailed to all in conn->waitingForReply
    while (!conn->waitingForReply.isEmpty()) {
	IceConn iceConn = conn->waitingForReply.take(0);
	if (iceConn) {
	    DCOPConnection* target = clients.find( iceConn );
	    qWarning("DCOP aborting call from '%s' to '%s'", target ? target->appId.data() : "<unknown>" , conn->appId.data() );
	    QByteArray reply;
	    DCOPMsg *pMsg;
	    IceGetHeader( iceConn, majorOpcode, DCOPReplyFailed,
			  sizeof(DCOPMsg), DCOPMsg, pMsg );
	    pMsg->key = 1;
	    pMsg->length += reply.size();
            _DCOPIceSendBegin( iceConn );
	    IceSendData( iceConn, reply.size(), const_cast<char *>(reply.data()));
            _DCOPIceSendEnd();
            if (!target)
               qWarning("DCOP Error: unknown target in waitingForReply");
            else if (!target->waitingOnReply.removeRef(conn->iceConn))
               qWarning("DCOP Error: client in waitingForReply wasn't waiting on reply");
	}
    }

    // Send DCOPReplyFailed to all in conn->waitingForDelayedReply
    while (!conn->waitingForDelayedReply.isEmpty()) {
	IceConn iceConn = conn->waitingForDelayedReply.take(0);
	if (iceConn) {
	    DCOPConnection* target = clients.find( iceConn );
	    qWarning("DCOP aborting (delayed) call from '%s' to '%s'", target ? target->appId.data() : "<unknown>", conn->appId.data() );
	    QByteArray reply;
	    DCOPMsg *pMsg;
	    IceGetHeader( iceConn, majorOpcode, DCOPReplyFailed,
			  sizeof(DCOPMsg), DCOPMsg, pMsg );
	    pMsg->key = 1;
	    pMsg->length += reply.size();
            _DCOPIceSendBegin( iceConn );
	    IceSendData( iceConn, reply.size(), const_cast<char *>(reply.data()));
            _DCOPIceSendEnd();
            if (!target)
               qWarning("DCOP Error: unknown target in waitingForDelayedReply");
            else if (!target->waitingOnReply.removeRef(conn->iceConn))
               qWarning("DCOP Error: client in waitingForDelayedReply wasn't waiting on reply");
	}
    }
    while (!conn->waitingOnReply.isEmpty())
    {
	IceConn iceConn = conn->waitingOnReply.take(0);
        if (iceConn) {
           DCOPConnection* target = clients.find( iceConn );
           if (!target)
           {
               qWarning("DCOP Error: still waiting for answer from non-existing client.");
               continue;
           }
           qWarning("DCOP aborting while waiting for answer from '%s'", target->appId.data());
           if (!target->waitingForReply.removeRef(conn->iceConn) &&
               !target->waitingForDelayedReply.removeRef(conn->iceConn))
              qWarning("DCOP Error: called client has forgotten about caller");
        }
    }

    if ( !conn->appId.isNull() ) {
#ifndef NDEBUG
	qDebug("DCOP:  unregister '%s'", conn->appId.data() );
#endif
        if ( !conn->daemon )
        {
            currentClientNumber--;
#ifndef NDEBUG
            qDebug("DCOP: number of clients is now down to %d", currentClientNumber );
#endif
        }

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
		pMsg->key = 1;
		pMsg->length += datalen;
                _DCOPIceSendBegin(c->iceConn);
		IceSendData(c->iceConn, datalen, const_cast<char *>(ba.data()));
                _DCOPIceSendEnd();
	    }
	}
    }

    delete conn;

    if ( currentClientNumber == 0 )
    {
        m_timer->start( 10000 ); // if within 10 seconds nothing happens, we'll terminate
    }
}

void DCOPServer::slotTerminate()
{
#ifndef NDEBUG
    fprintf( stderr, "DCOPServer : slotTerminate() -> sending terminateKDE signal.\n" );
#endif
    QByteArray data;
    dcopSignals->emitSignal(0L /* dcopserver */, "terminateKDE()", data, false);
}

bool DCOPServer::receive(const QCString &/*app*/, const QCString &obj,
			 const QCString &fun, const QByteArray& data,
			 QCString& replyType, QByteArray &replyData,
			 IceConn iceConn)
{
    if ( obj == "emit")
    {
        DCOPConnection* conn = clients.find( iceConn );
        if (conn) {
	    //qDebug("DCOPServer: %s emits %s", conn->appId.data(), fun.data());
	    dcopSignals->emitSignal(conn, fun, data, false);
        }
        replyType = "void";
        return true;
    }
    if ( fun == "setDaemonMode(bool)" ) {
        QDataStream args( data, IO_ReadOnly );
        if ( !args.atEnd() ) {
            Q_INT8 iDaemon;
            bool daemon;
            args >> iDaemon;

            daemon = static_cast<bool>( iDaemon );

	    DCOPConnection* conn = clients.find( iceConn );
            if ( conn && !conn->appId.isNull() ) {
                if ( daemon ) {
                    if ( !conn->daemon )
                    {
                        conn->daemon = true;

#ifndef NDEBUG
                        qDebug( "DCOP: new daemon %s", conn->appId.data() );
#endif

                        currentClientNumber--;

// David says it's safer not to do this :-)
//                        if ( currentClientNumber == 0 )
//                            m_timer->start( 10000 );
                    }
                } else
                {
                    if ( conn->daemon ) {
                        conn->daemon = false;

                        currentClientNumber++;

                        m_timer->stop();
                    }
                }
            }

            replyType = "void";
            return true;
        }
    }
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
                {
                    currentClientNumber++;
                    m_timer->stop(); // abort termination if we were planning one
#ifndef NDEBUG
                    qDebug("DCOP: register '%s' -> number of clients is now %d", app2.data(), currentClientNumber );
#endif
                }
#ifndef NDEBUG
		else
		    qDebug("DCOP:  '%s' now known as '%s'", conn->appId.data(), app2.data() );
#endif

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

		int c = conn->appId.find( '-' );
		if ( c > 0 )
		    conn->plainAppId = conn->appId.left( c );
		else
		    conn->plainAppId = conn->appId;

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
			pMsg->length += datalen;
                        _DCOPIceSendBegin(c->iceConn);
			IceWriteData( c->iceConn, datalen, const_cast<char *>(ba.data()));
			IceFlush( c->iceConn );
                        _DCOPIceSendEnd();
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
	QAsciiDictIterator<DCOPConnection> it( appIds );
	while ( it.current() ) {
	    applications << it.currentKey();
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
	    int b = ( findApp( s ) != 0 );
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
    } else if ( fun == "connectSignal(QCString,QCString,QCString,QCString,QCString,bool)") {
        DCOPConnection* conn = clients.find( iceConn );
        if (!conn) return false;
        QCString sender, senderObj, signal, receiverObj, slot;
        Q_INT8 Volatile;
        QDataStream args(data, IO_ReadOnly );
        if (args.atEnd()) return false;
        args >> sender >> senderObj >> signal >> receiverObj >> slot >> Volatile;
        //qDebug("DCOPServer: connectSignal(sender = %s senderObj = %s signal = %s recvObj = %s slot = %s)", sender.data(), senderObj.data(), signal.data(), receiverObj.data(), slot.data());
        bool b = dcopSignals->connectSignal(sender, senderObj, signal, conn, receiverObj, slot, (Volatile != 0));
        replyType = "bool";
        QDataStream reply( replyData, IO_WriteOnly );
        reply << (Q_INT8) (b?1:0);
        return TRUE;
    } else if ( fun == "disconnectSignal(QCString,QCString,QCString,QCString,QCString)") {
        DCOPConnection* conn = clients.find( iceConn );
        if (!conn) return false;
        QCString sender, senderObj, signal, receiverObj, slot;
        QDataStream args(data, IO_ReadOnly );
        if (args.atEnd()) return false;
        args >> sender >> senderObj >> signal >> receiverObj >> slot;
        //qDebug("DCOPServer: disconnectSignal(sender = %s senderObj = %s signal = %s recvObj = %s slot = %s)", sender.data(), senderObj.data(), signal.data(), receiverObj.data(), slot.data());
        bool b = dcopSignals->disconnectSignal(sender, senderObj, signal, conn, receiverObj, slot);
        replyType = "bool";
        QDataStream reply( replyData, IO_WriteOnly );
        reply << (Q_INT8) (b?1:0);
        return TRUE;
    }

    return FALSE;
}

void
DCOPServer::sendMessage(DCOPConnection *conn, const QCString &sApp,
                        const QCString &rApp, const QCString &rObj,
                        const QCString &rFun,  const QByteArray &data)
{
   QByteArray ba;
   QDataStream ds( ba, IO_WriteOnly );
   ds << sApp << rApp << rObj << rFun << data;
   int datalen = ba.size();
   DCOPMsg *pMsg = 0;

   IceGetHeader( conn->iceConn, majorOpcode, DCOPSend,
                 sizeof(DCOPMsg), DCOPMsg, pMsg );
   pMsg->length += datalen;
   pMsg->key = 1; // important!
   _DCOPIceSendBegin( conn->iceConn );
   IceSendData(conn->iceConn, datalen, const_cast<char *>(ba.data()));
   _DCOPIceSendEnd();
}

void IoErrorHandler ( IceConn iceConn)
{
    the_server->ioError( iceConn );
}

const char* const ABOUT =
"Usage: dcopserver [--nofork] [--nosid] [--nolocal] [--help]\n"
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
    bool nolocal = false;
    for(int i = 1; i < argc; i++) {
	if (strcmp(argv[i], "--nofork") == 0)
	    nofork = true;
	else if (strcmp(argv[i], "--nosid") == 0)
	    nosid = true;
	else if (strcmp(argv[i], "--nolocal") == 0)
	    nolocal = true;
	else {
	    fprintf(stdout, ABOUT );
	    return 0;
	}
    }

    // check if we are already running
    QCString fName = dcopServerFile();
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
	    return 0;
	} else {
	    // either we couldn't read the PID or kill returned an error.
	    // remove lockfile and continue
	    unlink(fName.data());
	}
    }

    pipe(ready);

    if (!nofork) {
	if (fork() > 0) {
	    char c = 1;
	    close(ready[1]);
	    read(ready[0], &c, 1); // Wait till dcopserver is started
	    close(ready[0]);
	    return c; // I am the parent
	}
	close(ready[0]);

	if (!nosid)
	    setsid();

	if (fork() > 0)
	    return 0; // get rid of controlling terminal
    }

    signal(SIGHUP, sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGPIPE, SIG_IGN);

    putenv(strdup("SESSION_MANAGER="));

    QApplication a( argc, argv, false );

    IceSetIOErrorHandler (IoErrorHandler );
    DCOPServer *server = new DCOPServer(!nolocal); // this sets the_server

    int ret = a.exec();
    delete server;
    return ret;
}

#include "dcopserver.moc"
