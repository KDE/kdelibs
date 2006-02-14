/*****************************************************************

Copyright (c) 1999 Preston Brown <pbrown@kde.org>
Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>

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

// qt <-> dcop integration
#include <qmetaobject.h>
#include <qvariant.h>
#include <qtimer.h>
#include <QHash>
#include <qeventloop.h>
// end of qt <-> dcop integration

#include "config.h"

#include <config.h>
#include <dcopref.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/socket.h>

#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif
#include <QPointer>
#include <QSet>
#include <qtextstream.h>
#include <qfile.h>
#include <qdir.h>
#include <qapplication.h>
#include <qsocketnotifier.h>
#include <qregexp.h>

//#include <private/qucomextra_p.h>

#include <dcopglobal.h>
#include <dcopclient.h>
#include <dcopobject.h>

#if defined Q_WS_X11 && ! defined K_WS_QTONLY
#include <X11/Xmd.h>
#endif
extern "C" {
#include <KDE-ICE/ICElib.h>
#include <KDE-ICE/ICEutil.h>
#include <KDE-ICE/ICEmsg.h>
#include <KDE-ICE/ICEproto.h>
}

// #define DCOPCLIENT_DEBUG	1

extern QMap<DCOPCString, DCOPObject *> * kde_dcopObjMap; // defined in dcopobject.cpp

/*********************************************
 * Keep track of local clients
 *********************************************/
typedef QHash<QByteArray, DCOPClient*> client_map_t;
static client_map_t *DCOPClient_CliMap = 0;


static
client_map_t *cliMap()
{
    if (!DCOPClient_CliMap)
        DCOPClient_CliMap = new client_map_t;
    return DCOPClient_CliMap;
}

DCOPClient *DCOPClient::findLocalClient( const DCOPCString &_appId )
{
    return cliMap()->value(_appId);
}

static
void registerLocalClient( const DCOPCString &_appId, DCOPClient *client )
{
    cliMap()->insert(_appId, client);
}

static
void unregisterLocalClient( const DCOPCString &_appId )
{
    cliMap()->remove(_appId);
}
/////////////////////////////////////////////////////////

template class QList<DCOPObjectProxy*>;
template class QList<DCOPClientTransaction*>;
template class QList<_IceConn*>;

struct DCOPClientMessage
{
    int opcode;
    CARD32 key;
    QByteArray data;
};

class DCOPClient::ReplyStruct
{
public:
    enum ReplyStatus { Pending, Ok, Failed };
    ReplyStruct() {
        status = Pending;
        replyType = 0;
        replyData = 0;
        replyId = -1;
        transactionId = -1;
        replyObject = 0;
    }
    ReplyStatus status;
    DCOPCString* replyType;
    QByteArray* replyData;
    int replyId;
    qint32 transactionId;
    DCOPCString calledApp;
    QPointer<QObject> replyObject;
    DCOPCString replySlot;
};

class DCOPClientPrivate
{
public:
    DCOPClientPrivate( DCOPClient* client )
        : parent( client ),
          postMessageTimer( client ),
          eventLoop( 0 )
    {}
    DCOPClient *parent;
    DCOPCString appId;
    IceConn iceConn;
    int majorOpcode; // major opcode negotiated w/server and used to tag all comms.

    int majorVersion, minorVersion; // protocol versions negotiated w/server

    static const char* serverAddr; // location of server in ICE-friendly format.
    QSocketNotifier *notifier;
    bool non_blocking_call_lock;
    bool registered;
    bool foreign_server;
    bool accept_calls;
    bool accept_calls_override; // If true, user has specified policy.
    bool qt_bridge_enabled;

    DCOPCString senderId;
    DCOPCString objId;
    DCOPCString function;

    DCOPCString defaultObject;
    QList<DCOPClientTransaction*> *transactionList;
    bool transaction;
    qint32 transactionId;
    int opcode;

    // Special key values:
    // 0 : Not specified
    // 1 : DCOPSend
    // 2 : Priority
    // >= 42: Normal
    CARD32 key;
    CARD32 currentKey;
    CARD32 currentKeySaved;

    QTimer postMessageTimer;
    QList<DCOPClientMessage*> messages;

    QList<DCOPClient::ReplyStruct*> pendingReplies;
    QList<DCOPClient::ReplyStruct*> asyncReplyQueue;

    struct LocalTransactionResult
    {
        DCOPCString replyType;
        QByteArray replyData;
    };

    QHash<int, LocalTransactionResult*> localTransActionList;

    QTimer eventLoopTimer;
    QEventLoop* eventLoop;
};

class DCOPClientTransaction
{
public:
    qint32 id;
    CARD32 key;
    DCOPCString senderId;
};

QByteArray DCOPClient::iceauthPath()
{
#ifdef Q_OS_WIN32
	char	szPath[512];
	char *	pszFilePart;
	int		ret;
	ret = SearchPathA(NULL,"iceauth.exe",NULL,sizeof(szPath)/sizeof(szPath[0]),szPath,&pszFilePart);
	if(ret != 0)
		return QByteArray(szPath);
#else
    QByteArray path = ::getenv("PATH");
    if (path.isEmpty())
        path = "/bin:/usr/bin";
    path += ":/usr/bin/X11:/usr/X11/bin:/usr/X11R6/bin";
    QByteArray fPath = strtok(path.data(), ":\b");
    while (!fPath.isNull())
    {
        fPath += "/iceauth";
        if (access(fPath.data(), X_OK) == 0)
        {
            return fPath;
        }

        fPath = strtok(NULL, ":\b");
    }
#endif
    return 0;
}

static QByteArray dcopServerFile(const QByteArray &hostname, bool old)
{
    QByteArray fName = ::getenv("DCOPAUTHORITY");
    if (!old && !fName.isEmpty())
        return fName;

    fName = QFile::encodeName( QDir::homePath() );
//    fName = ::getenv("HOME");
    if (fName.isEmpty())
    {
        fprintf(stderr, "Aborting. $HOME is not set.\n");
        exit(1);
    }
#ifdef Q_WS_X11
    QByteArray disp = getenv("DISPLAY");
#elif defined(Q_WS_QWS)
    QByteArray disp = getenv("QWS_DISPLAY");
#else
    QByteArray disp;
#endif
    if (disp.isEmpty())
        disp = "NODISPLAY";

    int i;
    if((i = disp.lastIndexOf('.')) > disp.lastIndexOf(KPATH_SEPARATOR) && i >= 0)
        disp.truncate(i);

    if (!old)
        disp.replace(KPATH_SEPARATOR, '_');

    fName += "/.DCOP4server_";
    if (hostname.isEmpty())
    {
        char hostName[256];
        hostName[0] = '\0';
        if (gethostname(hostName, sizeof(hostName)))
        {
            fName += "localhost";
        }
        else
        {
            hostName[sizeof(hostName)-1] = '\0';
            fName += hostName;
        }
    }
    else
    {
        fName += hostname;
    }
    fName += "_"+disp;
    return fName;
}


// static
QByteArray DCOPClient::dcopServerFile(const QByteArray &hostname)
{
    return ::dcopServerFile(hostname, false);
}


// static
QByteArray DCOPClient::dcopServerFileOld(const QByteArray &hostname)
{
    return ::dcopServerFile(hostname, true);
}


const char* DCOPClientPrivate::serverAddr = 0;

static void DCOPProcessInternal( DCOPClientPrivate *d, int opcode, CARD32 key, const QByteArray& dataReceived, bool canPost  );

void DCOPClient::handleAsyncReply(ReplyStruct *replyStruct)
{
    if (replyStruct->replyObject)
    {
        QObject::connect(this, SIGNAL(callBack(int, const DCOPCString&, const QByteArray &)),
               replyStruct->replyObject, replyStruct->replySlot);
        emit callBack(replyStruct->replyId, *(replyStruct->replyType), *(replyStruct->replyData));
        QObject::disconnect(this, SIGNAL(callBack(int, const DCOPCString&, const QByteArray &)),
               replyStruct->replyObject, replyStruct->replySlot);
    }
    delete replyStruct;
}

/**
 * Callback for ICE.
 */
static void DCOPProcessMessage(IceConn iceConn, IcePointer clientObject,
                        int opcode, unsigned long length, Bool /*swap*/,
                        IceReplyWaitInfo *replyWait,
                        Bool *replyWaitRet)
{
    DCOPMsg *pMsg = 0;
    DCOPClientPrivate *d = static_cast<DCOPClientPrivate *>(clientObject);
    DCOPClient::ReplyStruct *replyStruct = replyWait ? static_cast<DCOPClient::ReplyStruct*>(replyWait->reply) : 0;

    IceReadMessageHeader(iceConn, sizeof(DCOPMsg), DCOPMsg, pMsg);
    CARD32 key = pMsg->key;
    if ( d->key == 0 )
        d->key = key; // received a key from the server

    QByteArray dataReceived( length, '\0' );
    IceReadData(iceConn, length, dataReceived.data() );

    d->opcode = opcode;
    switch (opcode ) {

    case DCOPReplyFailed:
        if ( replyStruct ) {
            replyStruct->status = DCOPClient::ReplyStruct::Failed;
            replyStruct->transactionId = 0;
            *replyWaitRet = True;
            return;
        } else {
            qWarning("Very strange! got a DCOPReplyFailed opcode, but we were not waiting for a reply!");
            return;
        }
    case DCOPReply:
        if ( replyStruct ) {
            QByteArray* b = replyStruct->replyData;
            DCOPCString* t =  replyStruct->replyType;
            replyStruct->status = DCOPClient::ReplyStruct::Ok;
            replyStruct->transactionId = 0;

            DCOPCString calledApp, app;
            QDataStream ds( &dataReceived, QIODevice::ReadOnly );
            ds.setVersion(QDataStream::Qt_3_1);
            ds >> calledApp >> app >> *t >> *b;

            *replyWaitRet = True;
            return;
        } else {
            qWarning("Very strange! got a DCOPReply opcode, but we were not waiting for a reply!");
            return;
        }
    case DCOPReplyWait:
        if ( replyStruct ) {
            DCOPCString calledApp, app;
            qint32 id;
            QDataStream ds( &dataReceived, QIODevice::ReadOnly );
            ds.setVersion(QDataStream::Qt_3_1);
            ds >> calledApp >> app >> id;
            replyStruct->transactionId = id;
            replyStruct->calledApp = calledApp;
            d->pendingReplies.append(replyStruct);
            *replyWaitRet = True;
            return;
        } else {
            qWarning("Very strange! got a DCOPReplyWait opcode, but we were not waiting for a reply!");
            return;
        }
    case DCOPReplyDelayed:
        {
            QDataStream ds( &dataReceived, QIODevice::ReadOnly );
            ds.setVersion(QDataStream::Qt_3_1);
            DCOPCString calledApp, app;
            qint32 id;

            ds >> calledApp >> app >> id;
            if (replyStruct && (id == replyStruct->transactionId) && (calledApp == replyStruct->calledApp))
            {
                *replyWaitRet = True;
            }

            QList<DCOPClient::ReplyStruct*>::Iterator iter = d->pendingReplies.begin();
            while (iter != d->pendingReplies.end())
            {
                DCOPClient::ReplyStruct* rs = *iter;
                if ((rs->transactionId == id) && (rs->calledApp == calledApp))
                {
                    d->pendingReplies.erase(iter);
                    QByteArray* b = rs->replyData;
                    DCOPCString* t =  rs->replyType;
                    ds >> *t >> *b;

                    rs->status = DCOPClient::ReplyStruct::Ok;
                    rs->transactionId = 0;
                    if (!rs->replySlot.isEmpty())
                    {
                        d->parent->handleAsyncReply(rs);
                    }
                    return;
                }
                ++iter;
            }
        }
        qWarning("Very strange! got a DCOPReplyDelayed opcode, but we were not waiting for a reply!");
        return;
    case DCOPCall:
    case DCOPFind:
    case DCOPSend:
        DCOPProcessInternal( d, opcode, key, dataReceived, true );
    }
}

void DCOPClient::processPostedMessagesInternal()
{
    if ( d->messages.isEmpty() )
        return;
    QList<DCOPClientMessage*>::Iterator it = d->messages.begin();
    DCOPClientMessage* msg;

    while (it != d->messages.end()) {
        msg = *it;
        if ( d->currentKey && msg->key != d->currentKey ) {
            ++it;
            continue;
        }
        it = d->messages.erase(it);
        d->opcode = msg->opcode;
        DCOPProcessInternal( d, msg->opcode, msg->key, msg->data, false );
        delete msg;
    }
    if ( !d->messages.isEmpty() )
        d->postMessageTimer.start( 100 );
}

/**
   Processes DCOPCall, DCOPFind and DCOPSend
 */
void DCOPProcessInternal( DCOPClientPrivate *d, int opcode, CARD32 key, const QByteArray& dataReceived, bool canPost  )
{
    if (!d->accept_calls && (opcode == DCOPSend))
        return;

    IceConn iceConn = d->iceConn;
    DCOPMsg *pMsg = 0;
    DCOPClient *c = d->parent;
    QDataStream ds( (QByteArray*)&dataReceived, QIODevice::ReadOnly );
    ds.setVersion(QDataStream::Qt_3_1);

    DCOPCString fromApp;
    ds >> fromApp;
    if (fromApp.isEmpty())
        return; // Reserved for local calls

    if (!d->accept_calls)
    {
        QByteArray reply;
        QDataStream replyStream( &reply, QIODevice::WriteOnly );
        replyStream.setVersion(QDataStream::Qt_3_1);
        // Call rejected.
        replyStream << d->appId << fromApp;
        IceGetHeader( iceConn, d->majorOpcode, DCOPReplyFailed,
                      sizeof(DCOPMsg), DCOPMsg, pMsg );
        int datalen = reply.size();
        pMsg->key = key;
        pMsg->length += datalen;
        IceSendData( iceConn, datalen, const_cast<char *>(reply.data()));
        return;
    }

    DCOPCString app, objId, fun;
    QByteArray data;
    ds >> app >> objId >> fun >> data;
    d->senderId = fromApp;
    d->objId = objId;
    d->function = fun;

// qWarning("DCOP: %s got call: %s:%s:%s key = %d currentKey = %d", d->appId.data(), app.data(), objId.data(), fun.data(), key, d->currentKey);

    if ( canPost && d->currentKey && key != d->currentKey ) {
        DCOPClientMessage* msg = new DCOPClientMessage;
        msg->opcode = opcode;
        msg->key = key;
        msg->data = dataReceived;
        d->messages.append( msg );
        d->postMessageTimer.start( 0 );
        return;
    }

    d->objId = objId;
    d->function = fun;

    DCOPCString replyType;
    QByteArray replyData;
    bool b;
    CARD32 oldCurrentKey = d->currentKey;
    if ( opcode != DCOPSend ) // DCOPSend doesn't change the current key
        d->currentKey = key;

    if ( opcode == DCOPFind )
        b = c->find(app, objId, fun, data, replyType, replyData );
    else
        b = c->receive( app, objId, fun, data, replyType, replyData );
    // set notifier back to previous state

    if ( opcode == DCOPSend )
        return;

    if ((d->currentKey == key) || (oldCurrentKey != 2))
        d->currentKey = oldCurrentKey;

    QByteArray reply;
    QDataStream replyStream( &reply, QIODevice::WriteOnly );
    replyStream.setVersion(QDataStream::Qt_3_1);

    qint32 id = c->transactionId();
    if (id) {
        // Call delayed. Send back the transaction ID.
        replyStream << d->appId << fromApp << id;

        IceGetHeader( iceConn, d->majorOpcode, DCOPReplyWait,
                      sizeof(DCOPMsg), DCOPMsg, pMsg );
        pMsg->key = key;
        pMsg->length += reply.size();
        IceSendData( iceConn, reply.size(), const_cast<char *>(reply.data()));
        return;
    }

    if ( !b )        {
        // Call failed. No data send back.

        replyStream << d->appId << fromApp;
        IceGetHeader( iceConn, d->majorOpcode, DCOPReplyFailed,
                      sizeof(DCOPMsg), DCOPMsg, pMsg );
        int datalen = reply.size();
        pMsg->key = key;
        pMsg->length += datalen;
        IceSendData( iceConn, datalen, const_cast<char *>(reply.data()));
        return;
    }

    // Call successful. Send back replyType and replyData.
    replyStream << d->appId << fromApp << replyType << replyData.size();


    // we are calling, so we need to set up reply data
    IceGetHeader( iceConn, d->majorOpcode, DCOPReply,
                  sizeof(DCOPMsg), DCOPMsg, pMsg );
    int datalen = reply.size() + replyData.size();
    pMsg->key = key;
    pMsg->length += datalen;
    // use IceSendData not IceWriteData to avoid a copy.  Output buffer
    // shouldn't need to be flushed.
    IceSendData( iceConn, reply.size(), const_cast<char *>(reply.data()));
    IceSendData( iceConn, replyData.size(), const_cast<char *>(replyData.data()));
}



static IcePoVersionRec DCOPClientVersions[] = {
    { DCOPVersionMajor, DCOPVersionMinor,  DCOPProcessMessage }
};


static DCOPClient* dcop_main_client = 0;

DCOPClient* DCOPClient::mainClient()
{
    return dcop_main_client;
}

void DCOPClient::setMainClient( DCOPClient* client )
{
    dcop_main_client = client;
}


DCOPClient::DCOPClient()
{
    d = new DCOPClientPrivate( this );
    d->iceConn = 0L;
    d->key = 0;
    d->currentKey = 0;
    d->majorOpcode = 0;
    d->appId = 0;
    d->notifier = 0L;
    d->non_blocking_call_lock = false;
    d->registered = false;
    d->foreign_server = true;
    d->accept_calls = true;
    d->accept_calls_override = false;
    d->qt_bridge_enabled = true;
    d->transactionList = 0L;
    d->transactionId = 0;
    d->postMessageTimer.setSingleShot( true );
    d->eventLoopTimer.setSingleShot( true );
    QObject::connect( &d->postMessageTimer, SIGNAL( timeout() ), this, SLOT( processPostedMessagesInternal() ) );
    QObject::connect( &d->eventLoopTimer, SIGNAL( timeout() ), this, SLOT( eventLoopTimeout() ) );

    if ( !mainClient() )
        setMainClient( this );
}

DCOPClient::~DCOPClient()
{
#ifdef DCOPCLIENT_DEBUG
    qWarning("d->messages.count() = %d", d->messages.count());
    DCOPClientMessage* msg ;
    while ( !d->messages.isEmpty() ) {
        msg = d->messages.takeFirst();
        qWarning("DROPPING UNHANDLED DCOP MESSAGE:");
        qWarning("         opcode = %d key = %d", msg->opcode, msg->key);
        QDataStream ds( &msg->data, QIODevice::ReadOnly );
        ds.setVersion(QDataStream::Qt_3_1);

        DCOPCString fromApp, app, objId, fun;
        ds >> fromApp >> app >> objId >> fun;
        qWarning("         from = %s", fromApp.data());
        qWarning("         to = %s / %s / %s", app.data(), objId.data(), fun.data());
        delete msg;
    }
#endif
    if (d->iceConn)
        if (IceConnectionStatus(d->iceConn) == IceConnectAccepted)
            detach();

    if (d->registered)
        unregisterLocalClient( d->appId );

    delete d->notifier;
    delete d->transactionList;
    delete d->eventLoop;
    qDeleteAll(d->messages);
/*    while (!d->messages.isEmpty())
        delete d->messages.takeFirst();*/
    delete d;

    if ( mainClient() == this )
        setMainClient( 0 );
}

void DCOPClient::setServerAddress(const QByteArray &addr)
{
    QByteArray env = "DCOPSERVER=" + addr;
    putenv(strdup(env.data()));
    delete [] DCOPClientPrivate::serverAddr;
    DCOPClientPrivate::serverAddr = qstrdup( addr.data() );
}

bool DCOPClient::attach()
{
    if (!attachInternal( true ))
       if (!attachInternal( true ))
          return false; // Try two times!
    return true;
}

void DCOPClient::bindToApp()
{
    // check if we have a qApp instantiated.  If we do,
    // we can create a QSocketNotifier and use it for receiving data.
    if (qApp) {
        if ( d->notifier )
            delete d->notifier;
        d->notifier = new QSocketNotifier(socket(),
                                          QSocketNotifier::Read, 0);
        QObject::connect(d->notifier, SIGNAL(activated(int)),
                SLOT(processSocketData(int)));

        moveToThread( qApp->thread() );
    }
}

void DCOPClient::suspend()
{
#ifdef Q_WS_WIN //TODO: remove (win32 ports sometimes do not create notifiers)
    if (!d->notifier)
        return;
#endif
    assert(d->notifier); // Suspending makes no sense if we didn't had a qApp yet
    d->notifier->setEnabled(false);
}

void DCOPClient::resume()
{
#ifdef Q_WS_WIN //TODO: remove
    if (!d->notifier)
        return;
#endif
    assert(d->notifier); // Should never happen
    d->notifier->setEnabled(true);
}

bool DCOPClient::isSuspended() const
{
#if defined(Q_WS_WIN) || defined(Q_WS_MAC) //TODO: REMOVE
    if (!d->notifier)
        return false;
#endif
    return !d->notifier->isEnabled();
}

#ifdef SO_PEERCRED
// Check whether the remote end is owned by the same user.
static bool peerIsUs(int sockfd)
{
    struct ucred cred;
    socklen_t siz = sizeof(cred);
    if (getsockopt(sockfd, SOL_SOCKET, SO_PEERCRED, &cred, &siz) != 0)
        return false;
    return (cred.uid == getuid());
}
#else
// Check whether the socket is owned by the same user.
static bool isServerSocketOwnedByUser(const char*server)
{
#ifdef Q_OS_WIN
    if (strncmp(server, "tcp/", 4) != 0)
        return false; // Not a local socket -> foreign.
	else
		return true;
#else
    if (strncmp(server, "local/", 6) != 0)
        return false; // Not a local socket -> foreign.
    const char *path = strchr(server, KPATH_SEPARATOR);
    if (!path)
        return false;
    path++;

    struct stat stat_buf;
    if (stat(path, &stat_buf) != 0)
        return false;

    return (stat_buf.st_uid == getuid());
#endif
}
#endif


bool DCOPClient::attachInternal( bool registerAsAnonymous )
{
    char errBuf[1024];

    if ( isAttached() )
        detach();

    if ((d->majorOpcode = IceRegisterForProtocolSetup(const_cast<char *>("DCOP"),
                                                      const_cast<char *>(DCOPVendorString),
                                                      const_cast<char *>(DCOPReleaseString),
                                                      1, DCOPClientVersions,
                                                      DCOPAuthCount,
                                                      const_cast<char **>(DCOPAuthNames),
                                                      DCOPClientAuthProcs, 0L)) < 0) {
        emit attachFailed(QLatin1String( "Communications could not be established." ));
        return false;
    }

    bool bClearServerAddr = false;
    // first, check if serverAddr was ever set.
    if (!d->serverAddr) {
        // here, we obtain the list of possible DCOP connections,
        // and attach to them.
        QByteArray dcopSrv;
        dcopSrv = ::getenv("DCOPSERVER");
        if (dcopSrv.isEmpty()) {
            QString fName = QFile::decodeName( dcopServerFile() );
            QFile f(fName);
            if (!f.open(QIODevice::ReadOnly)) {
                emit attachFailed(QLatin1String( "Could not read network connection list.\n" )+fName);
                return false;
            }
            int size = qMin( qint64(1024), f.size() ); // protection against a huge file
            QByteArray contents( size+1, '\0' );
            if ( f.read( contents.data(), size ) != size )
            {
               qDebug("Error reading from %s, didn't read the expected %d bytes", fName.toLocal8Bit().constData(), size);
               // Should we abort ?
            }
            contents[size] = '\0';
            int pos = contents.indexOf('\n');
            if ( pos == -1 ) // Shouldn't happen
            {
                qDebug("Only one line in dcopserver file !: %s", contents.data());
                dcopSrv = contents;
            }
            else
            {
				if(contents[pos - 1] == '\r')	// check for windows end of line
					pos--;
                dcopSrv = contents.left( pos );
//#ifndef NDEBUG
//                qDebug("dcopserver address: %s", dcopSrv.latin1());
//#endif
            }
        }
        d->serverAddr = qstrdup( const_cast<char *>(dcopSrv.data()) );
        bClearServerAddr = true;
    }

    if ((d->iceConn = IceOpenConnection(const_cast<char*>(d->serverAddr),
                                        static_cast<IcePointer>(this), False, d->majorOpcode,
                                        sizeof(errBuf), errBuf)) == 0L) {
        qDebug("DCOPClient::attachInternal. Attach failed %s", errBuf ? errBuf : "");
        d->iceConn = 0;
        if (bClearServerAddr) {
           delete [] d->serverAddr;
           d->serverAddr = 0;
        }
        emit attachFailed(QLatin1String( errBuf ));
        return false;
    }

    IceSetShutdownNegotiation(d->iceConn, False);

    int setupstat;
    char* vendor = 0;
    char* release = 0;
    setupstat = IceProtocolSetup(d->iceConn, d->majorOpcode,
                                 static_cast<IcePointer>(d),
                                 False, /* must authenticate */
                                 &(d->majorVersion), &(d->minorVersion),
                                 &(vendor), &(release), 1024, errBuf);
    if (vendor) free(vendor);
    if (release) free(release);

    if (setupstat == IceProtocolSetupFailure ||
        setupstat == IceProtocolSetupIOError) {
        IceCloseConnection(d->iceConn);
        d->iceConn = 0;
        if (bClearServerAddr) {
            delete [] d->serverAddr;
            d->serverAddr = 0;
        }
        emit attachFailed(QLatin1String( errBuf ));
        return false;
    } else if (setupstat == IceProtocolAlreadyActive) {
        if (bClearServerAddr) {
            delete [] d->serverAddr;
            d->serverAddr = 0;
        }
        /* should not happen because 3rd arg to IceOpenConnection was 0. */
        emit attachFailed(QLatin1String( "internal error in IceOpenConnection" ));
        return false;
    }


    if (IceConnectionStatus(d->iceConn) != IceConnectAccepted) {
        if (bClearServerAddr) {
            delete [] d->serverAddr;
            d->serverAddr = 0;
        }
        emit attachFailed(QLatin1String( "DCOP server did not accept the connection." ));
        return false;
    }

#ifdef SO_PEERCRED
    d->foreign_server = !peerIsUs(socket());
#else
    d->foreign_server = !isServerSocketOwnedByUser(d->serverAddr);
#endif
    if (!d->accept_calls_override)
        d->accept_calls = !d->foreign_server;

    bindToApp();

    if ( registerAsAnonymous )
        registerAs( "anonymous", true );

    return true;
}


bool DCOPClient::detach()
{
    int status;

    if (d->iceConn) {
        IceProtocolShutdown(d->iceConn, d->majorOpcode);
        status = IceCloseConnection(d->iceConn);
        if (status != IceClosedNow)
            return false;
        else
            d->iceConn = 0L;
    }

    if (d->registered)
        unregisterLocalClient(d->appId);

    delete d->notifier;
    d->notifier = 0L;
    d->registered = false;
    d->foreign_server = true;
    return true;
}

bool DCOPClient::isAttached() const
{
    if (!d->iceConn)
        return false;

    return (IceConnectionStatus(d->iceConn) == IceConnectAccepted);
}

bool DCOPClient::isAttachedToForeignServer() const
{
    return isAttached() && d->foreign_server;
}

bool DCOPClient::acceptCalls() const
{
    return isAttached() && d->accept_calls;
}

void DCOPClient::setAcceptCalls(bool b)
{
    d->accept_calls = b;
    d->accept_calls_override = true;
}

bool DCOPClient::qtBridgeEnabled() const
{
    return d->qt_bridge_enabled;
}

void DCOPClient::setQtBridgeEnabled(bool b)
{
    d->qt_bridge_enabled = b;
}

DCOPCString DCOPClient::registerAs( const DCOPCString &appId, bool addPID )
{
    DCOPCString result;

    DCOPCString _appId = appId;

    if (addPID) {
        DCOPCString pid;
        pid = QString().sprintf("-%d", getpid()).toAscii();
        _appId = _appId + pid;
    }

    if( d->appId == _appId )
        return d->appId;

#if 0 // no need to detach, dcopserver can handle renaming
    // Detach before reregistering.
    if ( isRegistered() ) {
        detach();
    }
#endif

    if ( !isAttached() ) {
        if (!attachInternal( false ))
            if (!attachInternal( false ))
                return result; // Try two times
    }

    // register the application identifier with the server
    DCOPCString replyType;
    QByteArray data, replyData;
    QDataStream arg( &data, QIODevice::WriteOnly );
    arg.setVersion(QDataStream::Qt_3_1);
    arg << _appId;
    if ( call( "DCOPServer", "", "registerAs(QCString)", data, replyType, replyData ) ) {
        QDataStream reply( &replyData, QIODevice::ReadOnly );
        reply.setVersion(QDataStream::Qt_3_1);
        reply >> result;
    }

    d->appId = result;
    d->registered = !result.isNull();

    if (d->registered)
        registerLocalClient( d->appId, this );

    return result;
}

bool DCOPClient::isRegistered() const
{
    return d->registered;
}


DCOPCString DCOPClient::appId() const
{
    return d->appId;
}


int DCOPClient::socket() const
{
    if (d->iceConn)
        return IceConnectionNumber(d->iceConn);
    return 0;
}

static inline bool isIdentChar( char x )
{                                                // Avoid bug in isalnum
    return x == '_' || (x >= '0' && x <= '9') ||
         (x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z');
}

DCOPCString DCOPClient::normalizeFunctionSignature( const DCOPCString& fun ) {
    if ( fun.isEmpty() )                                // nothing to do
        return fun;
    DCOPCString result( fun.size() );
    const char *from = fun.data();
    char *to         = result.data();
    char *first = to;
    char last = 0;
    while ( true ) {
        while ( *from && isspace(*from) )
            from++;
        if ( last && isIdentChar( last ) && isIdentChar( *from ) )
            *to++ = 0x20;
        while ( *from && !isspace(*from) ) {
            last = *from++;
            *to++ = last;
        }
        if ( !*from )
            break;
    }
    if ( to > first && *(to-1) == 0x20 )
        to--;
    *to = '\0';
    result.resize( (int)((long)to - (long)result.data()) + 1 );
    return result;
}


DCOPCString DCOPClient::senderId() const
{
    return d->senderId;
}


bool DCOPClient::send(const DCOPCString &remApp, const DCOPCString &remObjId,
                      const DCOPCString &remFun, const QByteArray &data)
{
    if (remApp.isEmpty())
       return false;
    DCOPClient *localClient = findLocalClient( remApp );

    if ( localClient  ) {
        bool saveTransaction = d->transaction;
        qint32 saveTransactionId = d->transactionId;
        DCOPCString saveSenderId = d->senderId;

        d->senderId = 0; // Local call
        DCOPCString replyType;
        QByteArray replyData;
        (void) localClient->receive(  remApp, remObjId, remFun, data, replyType, replyData );

        d->transaction = saveTransaction;
        d->transactionId = saveTransactionId;
        d->senderId = saveSenderId;
        // send() returns true if the data could be send to the DCOPServer,
        // regardles of receiving the data on the other application.
        // So we assume the data is successfully send to the (virtual) server
        // and return true in any case.
        return true;
    }

    if ( !isAttached() )
        return false;


    DCOPMsg *pMsg;

    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds.setVersion(QDataStream::Qt_3_1);
    ds << d->appId << remApp << remObjId << normalizeFunctionSignature(remFun) << data.size();

    IceGetHeader(d->iceConn, d->majorOpcode, DCOPSend,
                 sizeof(DCOPMsg), DCOPMsg, pMsg);

    pMsg->key = 1; // DCOPSend always uses the magic key 1
    int datalen = ba.size() + data.size();
    pMsg->length += datalen;

    IceSendData( d->iceConn, ba.size(), const_cast<char *>(ba.data()) );
    IceSendData( d->iceConn, data.size(), const_cast<char *>(data.data()) );

    //IceFlush(d->iceConn);

    if (IceConnectionStatus(d->iceConn) == IceConnectAccepted)
        return true;
    return false;
}

bool DCOPClient::send(const DCOPCString &remApp, const DCOPCString &remObjId,
                      const DCOPCString &remFun, const QString &data)
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds.setVersion(QDataStream::Qt_3_1);
    ds << data;
    return send(remApp, remObjId, remFun, ba);
}

bool DCOPClient::findObject(const DCOPCString &remApp, const DCOPCString &remObj,
                            const DCOPCString &remFun, const QByteArray &data,
                            DCOPCString &foundApp, DCOPCString &foundObj,
                            bool useEventLoop, int timeout)
{
    DCOPCStringList appList;
    DCOPCString app = remApp;
    if (app.isEmpty())
        app = "*";

    foundApp = 0;
    foundObj = 0;

    if (app[app.length()-1] == '*')
    {
        // Find all apps that match 'app'.
        // NOTE: It would be more efficient to do the filtering in
        // the dcopserver itself.
        int len = app.length()-1;
        DCOPCStringList apps=registeredApplications();
        for( DCOPCStringList::ConstIterator it = apps.begin();
            it != apps.end();
            ++it)
        {
            if ( strncmp( (*it).data(), app.data(), len) == 0)
                appList.append(*it);
        }
    }
    else
    {
        appList.append(app);
    }

    // We do all the local clients in phase1 and the rest in phase2
    for(int phase=1; phase <= 2; phase++)
    {
      for( DCOPCStringList::ConstIterator it = appList.begin();
           it != appList.end();
           ++it)
      {
        DCOPCString remApp = *it;
        DCOPCString replyType;
        QByteArray replyData;
        bool result = false;
        DCOPClient *localClient = findLocalClient( remApp );

        if ( (phase == 1) && localClient ) {
            // In phase 1 we do all local clients
            bool saveTransaction = d->transaction;
            qint32 saveTransactionId = d->transactionId;
            DCOPCString saveSenderId = d->senderId;

            d->senderId = 0; // Local call
            result = localClient->find(  remApp, remObj, remFun, data, replyType, replyData );

            qint32 id = localClient->transactionId();
            if (id) {
                // Call delayed. We have to wait till it has been processed.
                do {
                    QApplication::processEvents( QEventLoop::WaitForMoreEvents );
                } while( !localClient->isLocalTransactionFinished(id, replyType, replyData));
                result = true;
            }
            d->transaction = saveTransaction;
            d->transactionId = saveTransactionId;
            d->senderId = saveSenderId;
        }
        else if ((phase == 2) && !localClient)
        {
            // In phase 2 we do the other clients
            result = callInternal(remApp, remObj, remFun, data,
                     replyType, replyData, useEventLoop, timeout, DCOPFind);
        }

        if (result)
        {
            if (replyType == "DCOPRef")
            {
                DCOPRef ref;
                QDataStream reply( &replyData, QIODevice::ReadOnly );
                reply.setVersion(QDataStream::Qt_3_1);
                reply >> ref;

                if (ref.app() == remApp) // Consistency check
                {
                    // replyType contains objId.
                    foundApp = ref.app();
                    foundObj = ref.object();
                    return true;
                }
            }
        }
      }
    }
    return false;
}

bool DCOPClient::process(const DCOPCString &, const QByteArray &,
                         DCOPCString&, QByteArray &)
{
    return false;
}

bool DCOPClient::isApplicationRegistered( const DCOPCString& remApp)
{
    DCOPCString replyType;
    QByteArray data, replyData;
    QDataStream arg( &data, QIODevice::WriteOnly );
    arg.setVersion(QDataStream::Qt_3_1);
    arg << remApp;
    int result = false;
    if ( call( "DCOPServer", "", "isApplicationRegistered(QCString)", data, replyType, replyData ) ) {
        QDataStream reply( &replyData, QIODevice::ReadOnly );
        reply.setVersion(QDataStream::Qt_3_1);
        reply >> result;
        qDebug("Call to isApplicationRegistered returned:%d", result);
    } else {
        qDebug("Call to isApplicationRegistered failed");
    }
    return result;
}

DCOPCStringList DCOPClient::registeredApplications()
{
    DCOPCString replyType;
    QByteArray data, replyData;
    DCOPCStringList result;
    if ( call( "DCOPServer", "", "registeredApplications()", data, replyType, replyData ) ) {
        QDataStream reply( &replyData, QIODevice::ReadOnly );
        reply.setVersion(QDataStream::Qt_3_1);
        reply >> result;
    }
    return result;
}

DCOPCStringList DCOPClient::remoteObjects( const DCOPCString& remApp, bool *ok )
{
    DCOPCString replyType;
    QByteArray data, replyData;
    DCOPCStringList result;
    if ( ok )
        *ok = false;
    if ( call( remApp, "DCOPClient", "objects()", data, replyType, replyData ) ) {
        QDataStream reply( &replyData, QIODevice::ReadOnly );
        reply.setVersion(QDataStream::Qt_3_1);
        reply >> result;
        if ( ok )
            *ok = true;
    }
    return result;
}

DCOPCStringList DCOPClient::remoteInterfaces( const DCOPCString& remApp, const DCOPCString& remObj, bool *ok  )
{
    DCOPCString replyType;
    QByteArray data, replyData;
    DCOPCStringList result;
    if ( ok )
        *ok = false;
    if ( call( remApp, remObj, "interfaces()", data, replyType, replyData ) && replyType == "QCStringList") {
        QDataStream reply( &replyData, QIODevice::ReadOnly );
        reply.setVersion(QDataStream::Qt_3_1);
        reply >> result;
        if ( ok )
            *ok = true;
    }
    return result;
}

DCOPCStringList DCOPClient::remoteFunctions( const DCOPCString& remApp, const DCOPCString& remObj, bool *ok  )
{
    DCOPCString replyType;
    QByteArray data, replyData;
    DCOPCStringList result;
    if ( ok )
        *ok = false;
    if ( call( remApp, remObj, "functions()", data, replyType, replyData ) && replyType == "QCStringList") {
        QDataStream reply( &replyData, QIODevice::ReadOnly );
        reply.setVersion(QDataStream::Qt_3_1);
        reply >> result;
        if ( ok )
            *ok = true;
    }
    return result;
}

void DCOPClient::setNotifications(bool enabled)
{
    QByteArray data;
    QDataStream ds(&data, QIODevice::WriteOnly);
    ds.setVersion(QDataStream::Qt_3_1);
    ds << static_cast<qint8>(enabled);

    DCOPCString replyType;
    QByteArray reply;
    if (!call("DCOPServer", "", "setNotifications( bool )", data, replyType, reply))
        qWarning("I couldn't enable notifications at the dcopserver!");
}

void DCOPClient::setDaemonMode( bool daemonMode )
{
    QByteArray data;
    QDataStream ds(&data, QIODevice::WriteOnly);
    ds.setVersion(QDataStream::Qt_3_1);
    ds << static_cast<qint8>( daemonMode );

    DCOPCString replyType;
    QByteArray reply;
    if (!call("DCOPServer", "", "setDaemonMode(bool)", data, replyType, reply))
        qWarning("I couldn't enable daemon mode at the dcopserver!");
}



/*
  DCOP <-> Qt bridge

  ********************************************************************************
 */

static void fillQtObjects( DCOPCStringList& l, QObject* o, DCOPCString path)
{
    if ( !path.isEmpty() )
        path += '/';

    int unnamed = 0;
    //const QObjectList *list = o ? o->children() : QObject::objectTrees();
    const QObjectList list = o ? o->children() : QObjectList(); // ### FIXME
    QObjectList::const_iterator it;
    for ( it = list.constBegin(); it != list.constEnd(); ++it ){
      ++it;
#ifdef _GNUC
#warning might make sense to update to QString as objectName is one now
#endif
      QByteArray n = (*it)->objectName().toLatin1();
      if ( n == "unnamed" || n.isEmpty() )
      {
          n = QString().sprintf("%p", (char*)(*it) ).toAscii();
          n = QString("unnamed%1(%2, %3)").arg(++unnamed).arg((*it)->metaObject()->className()).arg(QLatin1String( n )).toAscii();
      }
      QByteArray fn = path + n;
      l.append( fn );
      QObject *obj = *it;
      if ( !obj->children().isEmpty() )
          fillQtObjects( l, obj, fn );
   }
}

namespace
{
struct O
{
    O(): o(0) {}
    O ( const DCOPCString& str, QObject* obj ):s(str), o(obj){}
    DCOPCString s;
    QObject* o;
};
} // namespace

static void fillQtObjectsEx( QList<O>& l, QObject* o, DCOPCString path )
{
    if ( !path.isEmpty() )
        path += '/';

    int unnamed = 0;
    //const QObjectList *list = o ? o->children() : QObject::objectTrees();
    const QObjectList list = o ? o->children() : QObjectList(); // ### FIXME
    QObjectList::const_iterator it;
    for ( it = list.constBegin(); it != list.constEnd(); ++it ){
      ++it;
#ifdef _GNUC
#warning might make sense to update to QString as objectName is one now
#endif
      QByteArray n = (*it)->objectName().toLatin1();
      if ( n == "unnamed" || n.isEmpty() )
      {
          QString ptr = QString().sprintf("%p", (char*)(*it) );
          n = QString("unnamed%1(%2, %3)").arg(++unnamed).arg((*it)->metaObject()->className()).arg(QLatin1String( ptr.toLatin1() )).toLatin1();
      }
      QByteArray fn = path + n;
      QObject *obj = *it;
      l.append( O( fn, obj ) );
      if ( !obj->children().isEmpty() )
          fillQtObjectsEx( l, obj , fn );
   }
}


static QObject* findQtObject( DCOPCString id )
{
    QRegExp expr( id );
    QList<O> l;
    fillQtObjectsEx( l, 0, "qt" );

    // Prefer an exact match, but fall-back on the first that contains the substring
    QObject* firstContains = 0L;
    for ( QList<O>::ConstIterator it = l.begin(); it != l.end(); ++it ) {
        if ( (*it).s == id ) // exact match
            return (*it).o;
        if ( !firstContains && QString::fromUtf8((*it).s).contains( expr ) ) {
            firstContains = (*it).o;
        }
    }
    return firstContains;
}

static DCOPCStringList  findQtObjects( DCOPCString id )
{
    QRegExp expr( id );
    QList<O> l;
    fillQtObjectsEx( l, 0, "qt" );
    DCOPCStringList result;
    for ( QList<O>::ConstIterator it = l.begin(); it != l.end(); ++it ) {
        if ( QString::fromUtf8((*it).s).contains( expr ) )
            result << (*it).s;
    }
    return result;
}

static bool receiveQtObject( const DCOPCString &objId, const DCOPCString &fun, const QByteArray &data,
                            DCOPCString& replyType, QByteArray &replyData)
{
    if  ( objId == "qt" ) {
        if ( fun == "interfaces()" ) {
            replyType = "QCStringList";
            QDataStream reply( &replyData, QIODevice::WriteOnly );
            reply.setVersion(QDataStream::Qt_3_1);
            DCOPCStringList l;
            l << "DCOPObject";
            l << "Qt";
            reply << l;
            return true;
        } else if ( fun == "functions()" ) {
            replyType = "QCStringList";
            QDataStream reply( &replyData, QIODevice::WriteOnly );
            reply.setVersion(QDataStream::Qt_3_1);
            DCOPCStringList l;
            l << "QCStringList functions()";
            l << "QCStringList interfaces()";
            l << "QCStringList objects()";
            l << "QCStringList find(QCString)";
            reply << l;
            return true;
        } else if ( fun == "objects()" ) {
            replyType = "QCStringList";
            QDataStream reply( &replyData, QIODevice::WriteOnly );
            reply.setVersion(QDataStream::Qt_3_1);
            DCOPCStringList l;
            fillQtObjects( l, 0, "qt" );
            reply << l;
            return true;
        } else if ( fun == "find(QCString)" ) {
            QDataStream ds( data );
            ds.setVersion(QDataStream::Qt_3_1);
            DCOPCString id;
            ds >> id ;
            replyType = "QCStringList";
            QDataStream reply( &replyData, QIODevice::WriteOnly );
            reply.setVersion(QDataStream::Qt_3_1);
            reply << findQtObjects( id ) ;
            return true;
        }
    } else if ( objId.startsWith( "qt/" ) ) {
        QObject* o = findQtObject( objId );
        if ( !o )
            return false;
        if ( fun == "functions()" ) {
            replyType = "QCStringList";
            QDataStream reply( &replyData, QIODevice::WriteOnly );
            reply.setVersion(QDataStream::Qt_3_1);
            DCOPCStringList l;
            l << "QCStringList functions()";
            l << "QCStringList interfaces()";
            l << "QCStringList properties()";
            l << "bool setProperty(QCString,QVariant)";
            l << "QVariant property(QCString)";
/** ### how to replace?
            QStrList lst = o->metaObject()->slotNames( true );
            int i = 0;
            for ( QPtrListIterator<char> it( lst ); it.current(); ++it ) {
                if ( o->metaObject()->slot( i++, true )->access != QMetaData::Public )
                    continue;
                DCOPCString slot = it.current();
                if ( slot.contains( "()" ) ) {
                    slot.prepend("void ");
                    l <<  slot;
                }
            }
*/
            reply << l;
            return true;
        } else if ( fun == "interfaces()" ) {
            replyType = "QCStringList";
            QDataStream reply( &replyData, QIODevice::WriteOnly );
            reply.setVersion(QDataStream::Qt_3_1);
            DCOPCStringList l;
            const QMetaObject *meta = o->metaObject();
            while ( meta ) {
                l.prepend( meta->className() );
                meta = meta->superClass();
            }
            reply << l;
            return true;
        } else if ( fun == "properties()" ) {
            replyType = "QCStringList";
            QDataStream reply( &replyData, QIODevice::WriteOnly );
            reply.setVersion(QDataStream::Qt_3_1);
            DCOPCStringList l;
            const QMetaObject *mo = o->metaObject();
            int count = mo->propertyCount();
            int offset = mo->propertyOffset();
            for ( int i = offset; i < count; i++) {
                const QMetaProperty p = mo->property( count );
                DCOPCString prop = p.typeName();
                prop += ' ';
                prop += p.name();
                if ( !p.isWritable() )
                    prop += " readonly";
                l << prop;
            }
            reply << l;
            return true;
        } else if ( fun == "property(QCString)" ) {
            replyType = "QVariant";
            QDataStream ds( data );
            ds.setVersion(QDataStream::Qt_3_1);
            DCOPCString name;
            ds >> name ;
            QVariant result = o->property(  name );
            QDataStream reply( &replyData, QIODevice::WriteOnly );
            reply.setVersion(QDataStream::Qt_3_1);
            reply << result;
            return true;
        } else if ( fun == "setProperty(QCString,QVariant)" ) {
            QDataStream ds( data );
            ds.setVersion(QDataStream::Qt_3_1);
            DCOPCString name;
            QVariant value;
            ds >> name >> value;
            replyType = "bool";
            QDataStream reply( &replyData, QIODevice::WriteOnly );
            reply.setVersion(QDataStream::Qt_3_1);
            reply << (qint8) o->setProperty( name, value );
            return true;
        } else {
            int slot = o->metaObject()->indexOfSlot( fun );
            if ( slot != -1 ) {
                replyType = "void";
                QMetaObject::invokeMethod( o, fun );
                return true;
            }
        }


    }
    return false;
}


/*
  ********************************************************************************
  End of DCOP <-> Qt bridge
 */


bool DCOPClient::receive(const DCOPCString &/*app*/, const DCOPCString &objId,
                         const DCOPCString &fun, const QByteArray &data,
                         DCOPCString& replyType, QByteArray &replyData)
{
    d->transaction = false; // Assume no transaction.
    if ( objId == "DCOPClient" ) {
        if ( fun == "objects()" ) {
            replyType = "QCStringList";
            QDataStream reply( &replyData, QIODevice::WriteOnly );
            reply.setVersion(QDataStream::Qt_3_1);
            DCOPCStringList l;
            if (d->qt_bridge_enabled)
            {
               l << "qt"; // the Qt bridge object
            }
            if ( kde_dcopObjMap ) {
                QMap<DCOPCString, DCOPObject *>::ConstIterator it( kde_dcopObjMap->begin());
                for (; it != kde_dcopObjMap->end(); ++it) {
                    if ( !it.key().isEmpty() ) {
                        if ( it.key() == d->defaultObject )
                            l << "default";
                        l << it.key();
                    }
                }
            }
            reply << l;
            return true;
        }
    }

    if ( objId.isEmpty() || objId == "DCOPClient" ) {
        if ( fun == "applicationRegistered(QCString)" ) {
            QDataStream ds( data );
            ds.setVersion(QDataStream::Qt_3_1);
            DCOPCString r;
            ds >> r;
            emit applicationRegistered( r );
            return true;
        } else if ( fun == "applicationRemoved(QCString)" ) {
            QDataStream ds( data );
            ds.setVersion(QDataStream::Qt_3_1);
            DCOPCString r;
            ds >> r;
            emit applicationRemoved( r );
            return true;
        }

        if ( process( fun, data, replyType, replyData ) )
            return true;
        // fall through and send to defaultObject if available

    } else if (d->qt_bridge_enabled &&
               (objId == "qt" || objId.startsWith( "qt/") ) ) { // dcop <-> qt bridge
        return receiveQtObject( objId, fun, data, replyType, replyData );
    }

    if ( objId.isEmpty() || objId == "default" ) {
        if ( !d->defaultObject.isEmpty() && DCOPObject::hasObject( d->defaultObject ) ) {
            DCOPObject *objPtr = DCOPObject::find( d->defaultObject );
            objPtr->setCallingDcopClient(this);
            if (objPtr->process(fun, data, replyType, replyData))
                return true;
        }

        // fall through and send to object proxies
    }

    if (!objId.isEmpty() && objId[objId.length()-1] == '*') {
        // handle a multicast to several objects.
        // doesn't handle proxies currently.  should it?
        DCOPCString toMatch;
        toMatch = objId.left(objId.length()-1);
        QList<DCOPObject*> matchList = DCOPObject::match(toMatch);
        Q_FOREACH (DCOPObject *objPtr, matchList) {
            objPtr->setCallingDcopClient(this);
            if (!objPtr->process(fun, data, replyType, replyData))
                return false;
        }
        return true;
    } else if (!DCOPObject::hasObject(objId)) {
        if ( DCOPObjectProxy::proxies ) {
            Q_FOREACH (DCOPObjectProxy* proxy, *DCOPObjectProxy::proxies) {
                // TODO: it.current()->setCallingDcopClient(this);
                if ( proxy->process( objId, fun, data, replyType, replyData ) )
                    return true;
            }
        }
        return false;

    } else {
        DCOPObject *objPtr = DCOPObject::find(objId);
        objPtr->setCallingDcopClient(this);
        if (!objPtr->process(fun, data, replyType, replyData)) {
            // obj doesn't understand function or some other error.
            return false;
        }
    }

    return true;
}

// Check if the function result is a bool with the value "true"
// If so set the function result to DCOPRef pointing to (app,objId) and
// return true. Return false otherwise.
static bool findResultOk(DCOPCString &replyType, QByteArray &replyData)
{
    qint8 success; // Tsk.. why is there no operator>>(bool)?
    if (replyType != "bool") return false;

    QDataStream reply( &replyData, QIODevice::ReadOnly );
    reply.setVersion(QDataStream::Qt_3_1);
    reply >> success;

    if (!success) return false;
    return true;
}

// set the function result to DCOPRef pointing to (app,objId) and
// return true.
static bool findSuccess(const DCOPCString &app, const DCOPCString objId, DCOPCString &replyType, QByteArray &replyData)
{
    DCOPRef ref(app, objId);
    replyType = "DCOPRef";

    replyData = QByteArray();
    QDataStream final_reply( &replyData, QIODevice::WriteOnly );
    final_reply.setVersion(QDataStream::Qt_3_1);
    final_reply << ref;
    return true;
}


bool DCOPClient::find(const DCOPCString &app, const DCOPCString &objId,
                      const DCOPCString &fun, const QByteArray &data,
                      DCOPCString& replyType, QByteArray &replyData)
{
    d->transaction = false; // Transactions are not allowed.
    if ( !app.isEmpty() && app != d->appId && app[app.length()-1] != '*') {
        qWarning("WEIRD! we somehow received a DCOP message w/a different appId");
        return false;
    }

    if (objId.isEmpty() || objId[objId.length()-1] != '*')
    {
        if (fun.isEmpty())
        {
            if (objId.isEmpty() || DCOPObject::hasObject(objId))
               return findSuccess(app, objId, replyType, replyData);
            return false;
        }
        // Message to application or single object...
        if (receive(app, objId, fun, data, replyType, replyData))
        {
            if (findResultOk(replyType, replyData))
                return findSuccess(app, objId, replyType, replyData);
        }
    }
    else {
        // handle a multicast to several objects.
        // doesn't handle proxies currently.  should it?
        DCOPCString toMatch;
        toMatch = objId.left(objId.length()-1);
        QList<DCOPObject*> matchList = DCOPObject::match(toMatch);
        Q_FOREACH (DCOPObject *objPtr, matchList)
        {
            replyType = 0;
            replyData = QByteArray();
            if (fun.isEmpty())
                return findSuccess(app, objPtr->objId(), replyType, replyData);
            objPtr->setCallingDcopClient(this);
            if (objPtr->process(fun, data, replyType, replyData))
                if (findResultOk(replyType, replyData))
                    return findSuccess(app, objPtr->objId(), replyType, replyData);
        }
    }
    return false;
}

bool DCOPClient::call(const DCOPCString &remApp, const DCOPCString &remObjId,
                      const DCOPCString &remFun, const QByteArray &data,
                      DCOPCString& replyType, QByteArray &replyData,
                      bool useEventLoop, int timeout)
{
    if (remApp.isEmpty())
        return false;
    DCOPClient *localClient = findLocalClient( remApp );

    if ( localClient ) {
        bool saveTransaction = d->transaction;
        qint32 saveTransactionId = d->transactionId;
        DCOPCString saveSenderId = d->senderId;

        d->senderId = 0; // Local call
        bool b = localClient->receive(  remApp, remObjId, remFun, data, replyType, replyData );

        qint32 id = localClient->transactionId();
        if (id) {
           // Call delayed. We have to wait till it has been processed.
           do {
              QApplication::processEvents( QEventLoop::WaitForMoreEvents );
           } while( !localClient->isLocalTransactionFinished(id, replyType, replyData));
           b = true;
        }
        d->transaction = saveTransaction;
        d->transactionId = saveTransactionId;
        d->senderId = saveSenderId;
        return b;
    }

    return callInternal(remApp, remObjId, remFun, data,
                        replyType, replyData, useEventLoop, timeout, DCOPCall);
}

void DCOPClient::asyncReplyReady()
{
    while( d->asyncReplyQueue.count() )
    {
        ReplyStruct *replyStruct = d->asyncReplyQueue.first();
        d->asyncReplyQueue.removeFirst();
        handleAsyncReply(replyStruct);
    }
}

int DCOPClient::callAsync(const DCOPCString &remApp, const DCOPCString &remObjId,
                const DCOPCString &remFun, const QByteArray &data,
                QObject *callBackObj, const char *callBackSlot)
{
    DCOPCString replyType;
    QByteArray replyData;

    ReplyStruct *replyStruct = new ReplyStruct;
    replyStruct->replyType = new DCOPCString;
    replyStruct->replyData = new QByteArray;
    replyStruct->replyObject = callBackObj;
    replyStruct->replySlot = callBackSlot;
    replyStruct->replyId = ++d->transactionId;
    if (d->transactionId < 0)  // Ensure that ids > 0
        d->transactionId = 0;

    bool b = callInternal(remApp, remObjId, remFun, data,
                          replyStruct, false, -1, DCOPCall);
    if (!b)
    {
        delete replyStruct->replyType;
        delete replyStruct->replyData;
        delete replyStruct;
        return 0;
    }

    if (replyStruct->transactionId == 0)
    {
        // Call is finished already
        QTimer::singleShot(0, this, SLOT(asyncReplyReady()));
        d->asyncReplyQueue.append(replyStruct);
    }

    return replyStruct->replyId;
}

bool DCOPClient::callInternal(const DCOPCString &remApp, const DCOPCString &remObjId,
                      const DCOPCString &remFun, const QByteArray &data,
                      DCOPCString& replyType, QByteArray &replyData,
                      bool useEventLoop, int timeout, int minor_opcode)
{
    ReplyStruct replyStruct;
    replyStruct.replyType = &replyType;
    replyStruct.replyData = &replyData;
    return callInternal(remApp, remObjId, remFun, data, &replyStruct, useEventLoop, timeout, minor_opcode);
}

bool DCOPClient::callInternal(const DCOPCString &remApp, const DCOPCString &remObjId,
                      const DCOPCString &remFun, const QByteArray &data,
                      ReplyStruct *replyStruct,
                      bool useEventLoop, int timeout, int minor_opcode)
{
    if ( !isAttached() )
        return false;

    DCOPMsg *pMsg;

    CARD32 oldCurrentKey = d->currentKey;
    if ( !d->currentKey )
        d->currentKey = d->key; // no key yet, initiate new call

    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds.setVersion(QDataStream::Qt_3_1);
    ds << d->appId << remApp << remObjId << normalizeFunctionSignature(remFun) << data.size();

    IceGetHeader(d->iceConn, d->majorOpcode, minor_opcode,
                 sizeof(DCOPMsg), DCOPMsg, pMsg);

    pMsg->key = d->currentKey;
    int datalen = ba.size() + data.size();
    pMsg->length += datalen;

// qWarning("DCOP: %s made call %s:%s:%s key = %d", d->appId.data(), remApp.data(), remObjId.data(), remFun.data(), pMsg->key);

    IceSendData(d->iceConn, ba.size(), const_cast<char *>(ba.data()));
    IceSendData(d->iceConn, data.size(), const_cast<char *>(data.data()));

    if (IceConnectionStatus(d->iceConn) != IceConnectAccepted)
        return false;

    IceFlush (d->iceConn);

    IceReplyWaitInfo waitInfo;
    waitInfo.sequence_of_request = IceLastSentSequenceNumber(d->iceConn);
    waitInfo.major_opcode_of_request = d->majorOpcode;
    waitInfo.minor_opcode_of_request = minor_opcode;

    replyStruct->transactionId = -1;
    waitInfo.reply = static_cast<IcePointer>(replyStruct);

    Bool readyRet = False;
    IceProcessMessagesStatus s;

    timeval time_start;
    int time_left = -1;
    if( timeout >= 0 )
    {
        gettimeofday( &time_start, NULL );
        time_left = timeout;
    }
    for(;;) {
        bool checkMessages = true;
        if ( useEventLoop
             ? d->notifier != NULL  // useEventLoop needs a socket notifier and a qApp
             : timeout >= 0 ) {     // !useEventLoop doesn't block only for timeout >= 0
            const int guiTimeout = 100;
            checkMessages = false;

            int msecs = useEventLoop
                ? guiTimeout  // timeout for the GUI refresh
                : time_left; // time remaining for the whole call
            fd_set fds;
            struct timeval tv;
            FD_ZERO( &fds );
            FD_SET( socket(), &fds );
            tv.tv_sec = msecs / 1000;
            tv.tv_usec = (msecs % 1000) * 1000;
            if ( select( socket() + 1, &fds, 0, 0, &tv ) <= 0 ) {
                if( useEventLoop && (timeout < 0 || time_left > guiTimeout)) {
                    // nothing was available, we got a timeout. Reactivate
                    // the GUI in blocked state.
                    bool old_lock = d->non_blocking_call_lock;
                    if ( !old_lock ) {
                        d->non_blocking_call_lock = true;
                        emit blockUserInput( true );
                    }
                    if( timeout >= 0 )
                        d->eventLoopTimer.start(time_left - guiTimeout);
                    if ( !d->eventLoop )
                        d->eventLoop = new QEventLoop;
                    d->eventLoop->exec(QEventLoop::ExcludeUserInputEvents);
                    d->eventLoopTimer.stop();
                    if ( !old_lock ) {
                        d->non_blocking_call_lock = false;
                        emit blockUserInput( false );
                    }
                }
            }
            else
            {
                checkMessages = true;
            }
        }
        if (!d->iceConn)
            return false;

        if( replyStruct->transactionId != -1 )
        {
            if (replyStruct->transactionId == 0)
               break; // Call complete
            if (!replyStruct->replySlot.isEmpty())
               break; // Async call
        }

        if( checkMessages ) { // something is available
            s = IceProcessMessages(d->iceConn, &waitInfo,
                                    &readyRet);
            if (s == IceProcessMessagesIOError) {
                detach();
                d->currentKey = oldCurrentKey;
                return false;
            }
        }

        if( replyStruct->transactionId != -1 )
        {
            if (replyStruct->transactionId == 0)
               break; // Call complete
            if (!replyStruct->replySlot.isEmpty())
               break; // Async call
        }

        if( timeout < 0 )
            continue;
        timeval time_now;
        gettimeofday( &time_now, NULL );
        time_left = timeout -
                        ((time_now.tv_sec - time_start.tv_sec) * 1000) -
                        ((time_now.tv_usec - time_start.tv_usec) / 1000);
        if( time_left <= 0)
        {
             if (useEventLoop)
             {
                // Before we fail, check one more time if something is available
                time_left = 0;
                useEventLoop = false;
                continue;
             }
             *(replyStruct->replyType) = DCOPCString();
             *(replyStruct->replyData) = QByteArray();
             replyStruct->status = ReplyStruct::Failed;
             break;
        }
    }

    // Wake up parent call, maybe it's reply is available already.
    if ( d->non_blocking_call_lock ) {
        d->eventLoop->quit();
    }

    d->currentKey = oldCurrentKey;
    return replyStruct->status != ReplyStruct::Failed;
}

void DCOPClient::eventLoopTimeout()
{
    d->eventLoop->quit();
}

void DCOPClient::processSocketData(int fd)
{
    // Make sure there is data to read!
    fd_set fds;
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    int result = select(fd+1, &fds, 0, 0, &timeout);
    if (result == 0)
        return;

    if ( d->non_blocking_call_lock ) {
        d->eventLoop->quit();
        return;
    }

    if (!d->iceConn) {
        d->notifier->deleteLater();
        d->notifier = 0;
        qWarning("received an error processing data from the DCOP server!");
        return;
    }

    IceProcessMessagesStatus s =  IceProcessMessages(d->iceConn, 0, 0);

    if (s == IceProcessMessagesIOError) {
        detach();
        qWarning("received an error processing data from the DCOP server!");
        return;
    }
}

void DCOPClient::setDefaultObject( const DCOPCString& objId )
{
    d->defaultObject = objId;
}


DCOPCString DCOPClient::defaultObject() const
{
    return d->defaultObject;
}

bool
DCOPClient::isLocalTransactionFinished(qint32 id, DCOPCString &replyType, QByteArray &replyData)
{
    DCOPClientPrivate::LocalTransactionResult *result = d->localTransActionList.take(id);
    if (!result)
        return false;

    replyType = result->replyType;
    replyData = result->replyData;
    delete result;

    return true;
}

DCOPClientTransaction *
DCOPClient::beginTransaction()
{
    if (d->opcode == DCOPSend)
        return 0;
    if (!d->transactionList)
        d->transactionList = new QList<DCOPClientTransaction*>;

    d->transaction = true;
    DCOPClientTransaction *trans = new DCOPClientTransaction;
    trans->senderId = d->senderId;
    trans->id = ++d->transactionId;
    if (d->transactionId < 0)  // Ensure that ids > 0
        d->transactionId = 0;
    trans->key = d->currentKey;

    d->transactionList->append( trans );

    return trans;
}

qint32
DCOPClient::transactionId() const
{
    if (d->transaction)
        return d->transactionId;
    else
        return 0;
}

void
DCOPClient::endTransaction( DCOPClientTransaction *trans, DCOPCString& replyType,
                            QByteArray &replyData)
{
    if ( !trans )
        return;

    if ( !isAttached() )
        return;

    if ( !d->transactionList) {
        qWarning("Transaction unknown: No pending transactions!");
        return; // No pending transactions!
    }

    if ( !d->transactionList->removeAll( trans ) ) {
        qWarning("Transaction unknown: Not on list of pending transactions!");
        return; // Transaction
    }

    if (trans->senderId.isEmpty())
    {
        // Local transaction
        DCOPClientPrivate::LocalTransactionResult *result = new DCOPClientPrivate::LocalTransactionResult;
        result->replyType = replyType;
        result->replyData = replyData;

        d->localTransActionList.insert(trans->id, result);

        delete trans;

        return;
    }

    DCOPMsg *pMsg;

    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds.setVersion(QDataStream::Qt_3_1);
    ds << d->appId << trans->senderId << trans->id << replyType << replyData;

    IceGetHeader(d->iceConn, d->majorOpcode, DCOPReplyDelayed,
                 sizeof(DCOPMsg), DCOPMsg, pMsg);
    pMsg->key = trans->key;
    pMsg->length += ba.size();

    IceSendData( d->iceConn, ba.size(), const_cast<char *>(ba.data()) );

    delete trans;
}

void
DCOPClient::emitDCOPSignal( const DCOPCString &object, const DCOPCString &signal, const QByteArray &data)
{
    // We hack the sending object name into the signal name
    DCOPCString name;
    name = object+"#"+normalizeFunctionSignature(signal);
    send("DCOPServer", "emit", name, data);
}

void
DCOPClient::emitDCOPSignal( const DCOPCString &signal, const QByteArray &data)
{
    emitDCOPSignal(0, signal, data);
}

bool
DCOPClient::connectDCOPSignal( const DCOPCString &sender, const DCOPCString &senderObj,
  const DCOPCString &signal,
  const DCOPCString &receiverObj, const DCOPCString &slot, bool Volatile)
{
    DCOPCString replyType;
    QByteArray data, replyData;
    qint8 iVolatile = Volatile ? 1 : 0;

    QDataStream args(&data, QIODevice::WriteOnly );
    args.setVersion(QDataStream::Qt_3_1);
    args << sender << senderObj << normalizeFunctionSignature(signal) << receiverObj << normalizeFunctionSignature(slot) << iVolatile;

    if (!call("DCOPServer", 0,
        "connectSignal(QCString,QCString,QCString,QCString,QCString,bool)",
        data, replyType, replyData))
    {
        return false;
    }

    if (replyType != "bool")
        return false;

    QDataStream reply(&replyData, QIODevice::ReadOnly );
    reply.setVersion(QDataStream::Qt_3_1);
    qint8 result;
    reply >> result;
    return (result != 0);
}


bool
DCOPClient::disconnectDCOPSignal( const DCOPCString &sender, const DCOPCString &senderObj,
  const DCOPCString &signal,
  const DCOPCString &receiverObj, const DCOPCString &slot)
{
    DCOPCString replyType;
    QByteArray data, replyData;

    QDataStream args (&data, QIODevice::WriteOnly );
    args.setVersion  (QDataStream::Qt_3_1);
    args << sender << senderObj << normalizeFunctionSignature(signal) << receiverObj << normalizeFunctionSignature(slot);

    if (!call("DCOPServer", 0,
        "disconnectSignal(QCString,QCString,QCString,QCString,QCString)",
        data, replyType, replyData))
    {
        return false;
    }

    if (replyType != "bool")
        return false;

    QDataStream reply(&replyData, QIODevice::ReadOnly );
    reply.setVersion(QDataStream::Qt_3_1);
    qint8 result;
    reply >> result;
    return (result != 0);
}


void
DCOPClient::setPriorityCall(bool b)
{
    if (b)
    {
       if (d->currentKey == 2)
          return;
       d->currentKeySaved = d->currentKey;
       d->currentKey = 2;
    }
    else
    {
       if (d->currentKey != 2)
          return;
       d->currentKey = d->currentKeySaved;
       if ( !d->messages.isEmpty() )
          d->postMessageTimer.start( 0 ); // Process queued messages
    }
}



void
DCOPClient::emergencyClose()
{
    QSet<DCOPClient*> clients;
    client_map_t *map = DCOPClient_CliMap;
    if (!map) return;

    Q_FOREACH (DCOPClient* client, map->values()) {
        if (client)
            clients.insert(client);
    }
    Q_FOREACH (DCOPClient *cl, clients)
    {
        if (cl->d->iceConn) {
            IceProtocolShutdown(cl->d->iceConn, cl->d->majorOpcode);
            IceCloseConnection(cl->d->iceConn);
            cl->d->iceConn = 0L;
        }
    }
}

const char *
DCOPClient::postMortemSender()
{
    if (!dcop_main_client)
        return "";
    if (dcop_main_client->d->senderId.isEmpty())
        return "";
    return dcop_main_client->d->senderId.data();
}

const char *
DCOPClient::postMortemObject()
{
    if (!dcop_main_client)
        return "";
    return dcop_main_client->d->objId.data();
}
const char *
DCOPClient::postMortemFunction()
{
    if (!dcop_main_client)
        return "";
    return dcop_main_client->d->function.data();
}

void DCOPClient::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include <dcopclient.moc>

