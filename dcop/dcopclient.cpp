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

//Added by the Qt porting tool:
#include <Q3CString>
#include <Q3StrList>
#include <Q3ValueList>
#include <q3asciidict.h>


// qt <-> dcop integration
#include <QMetaObject>
#include <QMetaProperty>
#include <QVariant>
#include <QTimer>
#include <Q3IntDictIterator>
#include <QEventLoop>
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
#include <Qt/qpointer.h>
#include <Qt/qtextstream.h>
#include <Qt/qfile.h>
#include <Qt/qdir.h>
#include <Qt/qapplication.h>
#include <Qt/qsocketnotifier.h>
#include <Qt/qregexp.h>

//#include <private/qucomextra_p.h>

#include <dcopglobal.h>
#include <dcopclient.h>
#include <dcopobject.h>

#if defined Q_WS_X11 && ! defined K_WS_QTONLY
#include <X11/Xmd.h> 
#endif
extern "C" {
#ifdef Q_OS_UNIX
#include <KDE-ICE/ICElib.h>
#include <KDE-ICE/ICEutil.h>
#include <KDE-ICE/ICEmsg.h>
#include <KDE-ICE/ICEproto.h>
#endif
}

// #define DCOPCLIENT_DEBUG	1

extern QMap<Q3CString, DCOPObject *> * kde_dcopObjMap; // defined in dcopobject.cpp

/*********************************************
 * Keep track of local clients
 *********************************************/
typedef Q3AsciiDict<DCOPClient> client_map_t;
static client_map_t *DCOPClient_CliMap = 0;

static
client_map_t *cliMap()
{
    if (!DCOPClient_CliMap)
        DCOPClient_CliMap = new client_map_t;
    return DCOPClient_CliMap;
}

DCOPClient *DCOPClient::findLocalClient( const Q3CString &_appId )
{
    return cliMap()->find(_appId.data());
}

static
void registerLocalClient( const Q3CString &_appId, DCOPClient *client )
{
    cliMap()->replace(_appId.data(), client);
}

static
void unregisterLocalClient( const Q3CString &_appId )
{
    client_map_t *map = cliMap();
    map->remove(_appId.data());
}
/////////////////////////////////////////////////////////

template class Q3PtrList<DCOPObjectProxy>;
template class Q3PtrList<DCOPClientTransaction>;
#ifdef Q_OS_UNIX
template class Q3PtrList<_IceConn>;
#endif

struct DCOPClientMessage
{
    int opcode;
#ifdef Q_OS_UNIX
    CARD32 key;
#endif
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
    Q3CString* replyType;
    QByteArray* replyData;
    int replyId;
    Q_INT32 transactionId;
    Q3CString calledApp;
    QPointer<QObject> replyObject;
    Q3CString replySlot;
};

class DCOPClientPrivate
{
public:
    DCOPClient *parent;
    Q3CString appId;
#ifdef Q_OS_UNIX
    IceConn iceConn;
#endif
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

    Q3CString senderId;
    Q3CString objId;
    Q3CString function;

    Q3CString defaultObject;
    Q3PtrList<DCOPClientTransaction> *transactionList;
    bool transaction;
    Q_INT32 transactionId;
    int opcode;

#ifdef Q_OS_UNIX
    // Special key values:
    // 0 : Not specified
    // 1 : DCOPSend
    // 2 : Priority
    // >= 42: Normal
    CARD32 key;
    CARD32 currentKey; 
    CARD32 currentKeySaved;
#endif

    QTimer postMessageTimer;
    Q3PtrList<DCOPClientMessage> messages;

    Q3PtrList<DCOPClient::ReplyStruct> pendingReplies;
    Q3PtrList<DCOPClient::ReplyStruct> asyncReplyQueue;

    struct LocalTransactionResult 
    {
        Q3CString replyType;
        QByteArray replyData;
    };

    Q3IntDict<LocalTransactionResult> localTransActionList;
    
    QTimer eventLoopTimer;
};

class DCOPClientTransaction
{
public:
    Q_INT32 id;
#ifdef Q_OS_UNIX
    CARD32 key;
#endif
    Q3CString senderId;
};

Q3CString DCOPClient::iceauthPath()
{
    Q3CString path = ::getenv("PATH");
    if (path.isEmpty())
        path = "/bin:/usr/bin";
    path += ":/usr/bin/X11:/usr/X11/bin:/usr/X11R6/bin";
    Q3CString fPath = strtok(path.data(), ":\b");
    while (!fPath.isNull())
    {
        fPath += "/iceauth";
        if (access(fPath.data(), X_OK) == 0)
        {
            return fPath;
        }
   
        fPath = strtok(NULL, ":\b");
    }
    return 0;
}

static Q3CString dcopServerFile(const Q3CString &hostname, bool old)
{
    Q3CString fName = ::getenv("DCOPAUTHORITY");
    if (!old && !fName.isEmpty())
        return fName;
    
    fName = ::getenv("HOME");
    if (fName.isEmpty())
    {
        fprintf(stderr, "Aborting. $HOME is not set.\n");
        exit(1);
    }
#ifdef Q_WS_X11
    Q3CString disp = getenv("DISPLAY");
#elif defined(Q_WS_QWS)
    Q3CString disp = getenv("QWS_DISPLAY");
#else
    Q3CString disp;
#endif
    if (disp.isEmpty())
        disp = "NODISPLAY";

    int i;
    if((i = disp.findRev('.')) > disp.findRev(KPATH_SEPARATOR) && i >= 0)
        disp.truncate(i);

    if (!old)
    {
        while( (i = disp.find(KPATH_SEPARATOR)) >= 0)
            disp[i] = '_';
    }

    fName += "/.DCOPserver_";
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
Q3CString DCOPClient::dcopServerFile(const Q3CString &hostname)
{
    return ::dcopServerFile(hostname, false);
}


// static
Q3CString DCOPClient::dcopServerFileOld(const Q3CString &hostname)
{
    return ::dcopServerFile(hostname, true);
}


const char* DCOPClientPrivate::serverAddr = 0;

#ifdef Q_OS_UNIX
static void DCOPProcessInternal( DCOPClientPrivate *d, int opcode, CARD32 key, const QByteArray& dataReceived, bool canPost  );
#endif

void DCOPClient::handleAsyncReply(ReplyStruct *replyStruct)
{
    if (replyStruct->replyObject)
    {
        QObject::connect(this, SIGNAL(callBack(int, const Q3CString&, const QByteArray &)),
               replyStruct->replyObject, replyStruct->replySlot);
        emit callBack(replyStruct->replyId, *(replyStruct->replyType), *(replyStruct->replyData));
        QObject::disconnect(this, SIGNAL(callBack(int, const Q3CString&, const QByteArray &)),
               replyStruct->replyObject, replyStruct->replySlot);
    }
    delete replyStruct;
}

#ifdef Q_OS_UNIX
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

    QByteArray dataReceived( length );
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
            Q3CString* t =  replyStruct->replyType;
            replyStruct->status = DCOPClient::ReplyStruct::Ok;
            replyStruct->transactionId = 0;

            Q3CString calledApp, app;
            QDataStream ds( &dataReceived, QIODevice::ReadOnly );
            ds >> calledApp >> app >> *t >> *b;

            *replyWaitRet = True;
            return;
        } else {
            qWarning("Very strange! got a DCOPReply opcode, but we were not waiting for a reply!");
            return;
        }
    case DCOPReplyWait:
        if ( replyStruct ) {
            Q3CString calledApp, app;
            Q_INT32 id;
            QDataStream ds( &dataReceived, QIODevice::ReadOnly );
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
            Q3CString calledApp, app;
            Q_INT32 id;

            ds >> calledApp >> app >> id;
            if (replyStruct && (id == replyStruct->transactionId) && (calledApp == replyStruct->calledApp))
            {
                *replyWaitRet = True;
            }

            for(DCOPClient::ReplyStruct *rs = d->pendingReplies.first(); rs; 
                rs = d->pendingReplies.next())
            {
                if ((rs->transactionId == id) && (rs->calledApp == calledApp))
                {
                    d->pendingReplies.remove();
                    QByteArray* b = rs->replyData;
                    Q3CString* t =  rs->replyType;
                    ds >> *t >> *b;

                    rs->status = DCOPClient::ReplyStruct::Ok;
                    rs->transactionId = 0;
                    if (!rs->replySlot.isEmpty())
                    {
                        d->parent->handleAsyncReply(rs);
                    }
                    return;
                }
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
#endif

void DCOPClient::processPostedMessagesInternal()
{
#ifdef Q_OS_UNIX
    if ( d->messages.isEmpty() )
        return;
    Q3PtrListIterator<DCOPClientMessage> it (d->messages );
    DCOPClientMessage* msg ;
    while ( ( msg = it.current() ) ) {
        ++it;
        if ( d->currentKey && msg->key != d->currentKey )
            continue;
        d->messages.removeRef( msg );
        d->opcode = msg->opcode;
        DCOPProcessInternal( d, msg->opcode, msg->key, msg->data, false );
        delete msg;
    }
    if ( !d->messages.isEmpty() )
        d->postMessageTimer.start( 100, true );
#endif
}

#ifdef Q_OS_UNIX
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

    Q3CString fromApp;
    ds >> fromApp;
    if (fromApp.isEmpty())
        return; // Reserved for local calls

    if (!d->accept_calls)
    {
        QByteArray reply;
        QDataStream replyStream( &reply, QIODevice::WriteOnly );
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

    Q3CString app, objId, fun;
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
        d->postMessageTimer.start( 0, true );
        return;
    }

    d->objId = objId;
    d->function = fun;

    Q3CString replyType;
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

    Q_INT32 id = c->transactionId();
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
#endif


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
    d = new DCOPClientPrivate;
    d->parent = this;
#ifdef Q_OS_UNIX
    d->iceConn = 0L;
    d->key = 0;
    d->currentKey = 0;
#endif
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
    QObject::connect( &d->postMessageTimer, SIGNAL( timeout() ), this, SLOT( processPostedMessagesInternal() ) );
    QObject::connect( &d->eventLoopTimer, SIGNAL( timeout() ), this, SLOT( eventLoopTimeout() ) );

    if ( !mainClient() )
        setMainClient( this );
}

DCOPClient::~DCOPClient()
{
#ifdef DCOPCLIENT_DEBUG
    qWarning("d->messages.count() = %d", d->messages.count());
    Q3PtrListIterator<DCOPClientMessage> it (d->messages );
    DCOPClientMessage* msg ;
    while ( ( msg = it.current() ) ) {
        ++it;
        d->messages.removeRef( msg );
        qWarning("DROPPING UNHANDLED DCOP MESSAGE:");
        qWarning("         opcode = %d key = %d", msg->opcode, msg->key);
        QDataStream ds( &msg->data, QIODevice::ReadOnly );

        Q3CString fromApp, app, objId, fun;
        ds >> fromApp >> app >> objId >> fun;
        qWarning("         from = %s", fromApp.data()); 
        qWarning("         to = %s / %s / %s", app.data(), objId.data(), fun.data());
        delete msg;
    }
#endif
#ifdef Q_OS_UNIX
    if (d->iceConn)
        if (IceConnectionStatus(d->iceConn) == IceConnectAccepted)
            detach();
#endif

    if (d->registered)
        unregisterLocalClient( d->appId );

    delete d->notifier;
    delete d->transactionList;
    d->messages.setAutoDelete(true);
    delete d;

    if ( mainClient() == this )
        setMainClient( 0 );
}

void DCOPClient::setServerAddress(const Q3CString &addr)
{
    Q3CString env = "DCOPSERVER=" + addr;
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
                                          QSocketNotifier::Read, 0, 0);
        QObject::connect(d->notifier, SIGNAL(activated(int)),
                SLOT(processSocketData(int)));
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
}
#endif


bool DCOPClient::attachInternal( bool registerAsAnonymous )
{
#ifdef Q_OS_UNIX
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
        emit attachFailed(QString::fromLatin1( "Communications could not be established." ));
        return false;
    }

    bool bClearServerAddr = false;
    // first, check if serverAddr was ever set.
    if (!d->serverAddr) {
        // here, we obtain the list of possible DCOP connections,
        // and attach to them.
        QString dcopSrv;
        dcopSrv = ::getenv("DCOPSERVER");
        if (dcopSrv.isEmpty()) {
            QString fName = dcopServerFile();
            QFile f(fName);
            if (!f.open(QIODevice::ReadOnly)) {
                emit attachFailed(QString::fromLatin1( "Could not read network connection list.\n" )+fName);
                return false;
            }
            int size = qMin( (qint64)1024, f.size() ); // protection against a huge file
            Q3CString contents( size+1 );
            if ( f.readBlock( contents.data(), size ) != size )
            {
               qDebug("Error reading from %s, didn't read the expected %d bytes", fName.latin1(), size);
               // Should we abort ?
            }
            contents[size] = '\0';
            int pos = contents.find('\n');
            if ( pos == -1 ) // Shouldn't happen
            {
                qDebug("Only one line in dcopserver file !: %s", contents.data());
                dcopSrv = QString::fromLatin1(contents);
            }
            else
            {
                dcopSrv = QString::fromLatin1(contents.left( pos ));
//#ifndef NDEBUG
//                qDebug("dcopserver address: %s", dcopSrv.latin1());
//#endif
            }
        }
        d->serverAddr = qstrdup( const_cast<char *>(dcopSrv.latin1()) );
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
        emit attachFailed(QString::fromLatin1( errBuf ));
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
        emit attachFailed(QString::fromLatin1( errBuf ));
        return false;
    } else if (setupstat == IceProtocolAlreadyActive) {
        if (bClearServerAddr) {
            delete [] d->serverAddr;
            d->serverAddr = 0;
        }
        /* should not happen because 3rd arg to IceOpenConnection was 0. */
        emit attachFailed(QString::fromLatin1( "internal error in IceOpenConnection" ));
        return false;
    }


    if (IceConnectionStatus(d->iceConn) != IceConnectAccepted) {
        if (bClearServerAddr) {
            delete [] d->serverAddr;
            d->serverAddr = 0;
        }
        emit attachFailed(QString::fromLatin1( "DCOP server did not accept the connection." ));
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
#else //!Q_OS_UNIX
    return false;
#endif
}


bool DCOPClient::detach()
{
#ifdef Q_OS_UNIX
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
#else //!Q_OS_UNIX
    return false;
#endif
}

bool DCOPClient::isAttached() const
{
#ifdef Q_OS_UNIX
    if (!d->iceConn)
        return false;

    return (IceConnectionStatus(d->iceConn) == IceConnectAccepted);
#else //!Q_OS_UNIX
    return false;
#endif
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

bool DCOPClient::qtBridgeEnabled()
{
    return d->qt_bridge_enabled;
}

void DCOPClient::setQtBridgeEnabled(bool b)
{
    d->qt_bridge_enabled = b;
}

Q3CString DCOPClient::registerAs( const Q3CString &appId, bool addPID )
{
    Q3CString result;

    Q3CString _appId = appId;

    if (addPID) {
        Q3CString pid;
        pid.sprintf("-%d", getpid());
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
    Q3CString replyType;
    QByteArray data, replyData;
    QDataStream arg( &data, QIODevice::WriteOnly );
    arg << _appId;
    if ( call( "DCOPServer", "", "registerAs(Q3CString)", data, replyType, replyData ) ) {
        QDataStream reply( &replyData, QIODevice::ReadOnly );
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


Q3CString DCOPClient::appId() const
{
    return d->appId;
}


int DCOPClient::socket() const
{
#ifdef Q_OS_UNIX
    if (d->iceConn)
        return IceConnectionNumber(d->iceConn);
#endif //Q_OS_UNIX
    return 0;
}

static inline bool isIdentChar( char x )
{                                                // Avoid bug in isalnum
    return x == '_' || (x >= '0' && x <= '9') ||
         (x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z');
}

Q3CString DCOPClient::normalizeFunctionSignature( const Q3CString& fun ) {
    if ( fun.isEmpty() )                                // nothing to do
        return fun.copy();
    Q3CString result( fun.size() );
    char *from        = strdup( fun.data() );
    char *to        = strdup( result.data() );
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


Q3CString DCOPClient::senderId() const
{
    return d->senderId;
}


bool DCOPClient::send(const Q3CString &remApp, const Q3CString &remObjId,
                      const Q3CString &remFun, const QByteArray &data)
{
#ifdef Q_OS_UNIX
    if (remApp.isEmpty())
       return false;
    DCOPClient *localClient = findLocalClient( remApp );

    if ( localClient  ) {
        bool saveTransaction = d->transaction;
        Q_INT32 saveTransactionId = d->transactionId;
        Q3CString saveSenderId = d->senderId;

        d->senderId = 0; // Local call
        Q3CString replyType;
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
    QDataStream ds( &ba, QIODevice::WriteOnly);
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
#endif //Q_OS_UNIX
    return false;
}

bool DCOPClient::send(const Q3CString &remApp, const Q3CString &remObjId,
                      const Q3CString &remFun, const QString &data)
{
    QByteArray ba;
    QDataStream ds( &ba, QIODevice::WriteOnly);
    ds << data;
    return send(remApp, remObjId, remFun, ba);
}

bool DCOPClient::findObject(const Q3CString &remApp, const Q3CString &remObj,
                            const Q3CString &remFun, const QByteArray &data,
                            Q3CString &foundApp, Q3CString &foundObj,
                            bool useEventLoop)
{
    return findObject( remApp, remObj, remFun, data, foundApp, foundObj, useEventLoop, -1 );
}

bool DCOPClient::findObject(const Q3CString &remApp, const Q3CString &remObj,
                            const Q3CString &remFun, const QByteArray &data,
                            Q3CString &foundApp, Q3CString &foundObj,
                            bool useEventLoop, int timeout)
{
    Q3CStringList appList;
    Q3CString app = remApp;
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
        Q3CStringList apps=registeredApplications();
        for( Q3CStringList::ConstIterator it = apps.begin();
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
      for( Q3CStringList::ConstIterator it = appList.begin();
           it != appList.end();
           ++it)
      {
        Q3CString remApp = *it;
        Q3CString replyType;
        QByteArray replyData;
        bool result = false;
        DCOPClient *localClient = findLocalClient( remApp );

        if ( (phase == 1) && localClient ) {
            // In phase 1 we do all local clients
            bool saveTransaction = d->transaction;
            Q_INT32 saveTransactionId = d->transactionId;
            Q3CString saveSenderId = d->senderId;

            d->senderId = 0; // Local call
            result = localClient->find(  remApp, remObj, remFun, data, replyType, replyData );

            Q_INT32 id = localClient->transactionId();
            if (id) {
                // Call delayed. We have to wait till it has been processed.
                do {
                    QApplication::processEvents( QEventLoop::WaitForMore);
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

bool DCOPClient::process(const Q3CString &, const QByteArray &,
                         Q3CString&, QByteArray &)
{
    return false;
}

bool DCOPClient::isApplicationRegistered( const Q3CString& remApp)
{
    Q3CString replyType;
    QByteArray data, replyData;
    QDataStream arg( &data, QIODevice::WriteOnly );
    arg << remApp;
    int result = false;
    if ( call( "DCOPServer", "", "isApplicationRegistered(Q3CString)", data, replyType, replyData ) ) {
        QDataStream reply( &replyData, QIODevice::ReadOnly );
        reply >> result;
    }
    return result;
}

Q3CStringList DCOPClient::registeredApplications()
{
    Q3CString replyType;
    QByteArray data, replyData;
    Q3CStringList result;
    if ( call( "DCOPServer", "", "registeredApplications()", data, replyType, replyData ) ) {
        QDataStream reply( &replyData, QIODevice::ReadOnly );
        reply >> result;
    }
    return result;
}

Q3CStringList DCOPClient::remoteObjects( const Q3CString& remApp, bool *ok )
{
    Q3CString replyType;
    QByteArray data, replyData;
    Q3CStringList result;
    if ( ok )
        *ok = false;
    if ( call( remApp, "DCOPClient", "objects()", data, replyType, replyData ) ) {
        QDataStream reply( &replyData, QIODevice::ReadOnly );
        reply >> result;
        if ( ok )
            *ok = true;
    }
    return result;
}

Q3CStringList DCOPClient::remoteInterfaces( const Q3CString& remApp, const Q3CString& remObj, bool *ok  )
{
    Q3CString replyType;
    QByteArray data, replyData;
    Q3CStringList result;
    if ( ok )
        *ok = false;
    if ( call( remApp, remObj, "interfaces()", data, replyType, replyData ) && replyType == "Q3CStringList") {
        QDataStream reply( &replyData, QIODevice::ReadOnly );
        reply >> result;
        if ( ok )
            *ok = true;
    }
    return result;
}

Q3CStringList DCOPClient::remoteFunctions( const Q3CString& remApp, const Q3CString& remObj, bool *ok  )
{
    Q3CString replyType;
    QByteArray data, replyData;
    Q3CStringList result;
    if ( ok )
        *ok = false;
    if ( call( remApp, remObj, "functions()", data, replyType, replyData ) && replyType == "Q3CStringList") {
        QDataStream reply( &replyData, QIODevice::ReadOnly );
        reply >> result;
        if ( ok )
            *ok = true;
    }
    return result;
}

void DCOPClient::setNotifications(bool enabled)
{
    QByteArray data;
    QDataStream ds( &data, QIODevice::WriteOnly);
    ds << static_cast<Q_INT8>(enabled);

    Q3CString replyType;
    QByteArray reply;
    if (!call("DCOPServer", "", "setNotifications( bool )", data, replyType, reply))
        qWarning("I couldn't enable notifications at the dcopserver!");
}

void DCOPClient::setDaemonMode( bool daemonMode )
{
    QByteArray data;
    QDataStream ds( &data, QIODevice::WriteOnly);
    ds << static_cast<Q_INT8>( daemonMode );

    Q3CString replyType;
    QByteArray reply;
    if (!call("DCOPServer", "", "setDaemonMode(bool)", data, replyType, reply))
        qWarning("I couldn't enable daemon mode at the dcopserver!");
}



/*
  DCOP <-> Qt bridge

  ********************************************************************************
 */

static void fillQtObjects( Q3CStringList& l, QObject* o, Q3CString path )
{
    if ( !path.isEmpty() )
        path += '/';

    int unnamed = 0;
    //const QObjectList *list = o ? o->children() : QObject::objectTrees();
    const QObjectList list =  o->children(); // ### FIXME
    QObjectList::const_iterator it;
    for ( it = list.constBegin(); it != list.constEnd(); ++it ){
      ++it;
      Q3CString n = (*it)->name();
      if ( n == "unnamed" || n.isEmpty() )
      {
          n.sprintf("%p", (void*)(*it) );
          n = QString("unnamed%1(%2, %3)").arg(++unnamed).arg((*it)->className()).arg(QLatin1String( n ));
      }
      Q3CString fn = path + n;
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
    O ( const Q3CString& str, QObject* obj ):s(str), o(obj){}
    Q3CString s;
    QObject* o;
};
} // namespace

static void fillQtObjectsEx( Q3ValueList<O>& l, QObject* o, Q3CString path )
{
    if ( !path.isEmpty() )
        path += '/';

    int unnamed = 0;
    //const QObjectList *list = o ? o->children() : QObject::objectTrees();
    const QObjectList list =  o->children(); // ### FIXME
    QObjectList::const_iterator it;
    for ( it = list.constBegin(); it != list.constEnd(); ++it ){
      ++it;
      Q3CString n = (*it)->name();
      if ( n == "unnamed" || n.isEmpty() )
      {
          n.sprintf("%p", (void*)(*it) );
          n = QString("unnamed%1(%2, %3)").arg(++unnamed).arg((*it)->className()).arg(QLatin1String( n ));
      }
      Q3CString fn = path + n;
      QObject *obj = *it;
      l.append( O( fn, obj ) );
      if ( !obj->children().isEmpty() )
          fillQtObjectsEx( l, obj , fn );
   }
}


static QObject* findQtObject( Q3CString id )
{
    QRegExp expr( id );
    Q3ValueList<O> l;
    fillQtObjectsEx( l, 0, "qt" );
    // Prefer an exact match, but fall-back on the first that contains the substring
    QObject* firstContains = 0L;
    for ( Q3ValueList<O>::ConstIterator it = l.begin(); it != l.end(); ++it ) {
        if ( (*it).s == id ) // exact match
            return (*it).o;
        if ( !firstContains && QString::fromUtf8((*it).s).contains( expr ) ) {
            firstContains = (*it).o;
        }
    }
    return firstContains;
}

static Q3CStringList  findQtObjects( Q3CString id )
{
    QRegExp expr( id );
    Q3ValueList<O> l;
    fillQtObjectsEx( l, 0, "qt" );
    Q3CStringList result;
    for ( Q3ValueList<O>::ConstIterator it = l.begin(); it != l.end(); ++it ) {
        if ( QString::fromUtf8((*it).s).contains( expr ) )
            result << (*it).s;
    }
    return result;
}

static bool receiveQtObject( const Q3CString &objId, const Q3CString &fun, const QByteArray &data,
                            Q3CString& replyType, QByteArray &replyData)
{
    if  ( objId == "qt" ) {
        if ( fun == "interfaces()" ) {
            replyType = "Q3CStringList";
            QDataStream reply( &replyData, QIODevice::WriteOnly );
            Q3CStringList l;
            l << "DCOPObject";
            l << "Qt";
            reply << l;
            return true;
        } else if ( fun == "functions()" ) {
            replyType = "Q3CStringList";
            QDataStream reply( &replyData, QIODevice::WriteOnly );
            Q3CStringList l;
            l << "Q3CStringList functions()";
            l << "Q3CStringList interfaces()";
            l << "Q3CStringList objects()";
            l << "Q3CStringList find(Q3CString)";
            reply << l;
            return true;
        } else if ( fun == "objects()" ) {
            replyType = "Q3CStringList";
            QDataStream reply( &replyData, QIODevice::WriteOnly );
            Q3CStringList l;
            fillQtObjects( l, 0, "qt" );
            reply << l;
            return true;
        } else if ( fun == "find(Q3CString)" ) {
            QByteArray dataCopy = data;
            QDataStream ds( &dataCopy, QIODevice::ReadOnly );
            Q3CString id;
            ds >> id ;
            replyType = "Q3CStringList";
            QDataStream reply( &replyData, QIODevice::WriteOnly );
            reply << findQtObjects( id ) ;
            return true;
        }
    } else if ( objId.left(3) == "qt/" ) {
        QObject* o = findQtObject( objId );
        if ( !o )
            return false;
        if ( fun == "functions()" ) {
            replyType = "Q3CStringList";
            QDataStream reply( &replyData, QIODevice::WriteOnly );
            Q3CStringList l;
            l << "Q3CStringList functions()";
            l << "Q3CStringList interfaces()";
            l << "Q3CStringList properties()";
            l << "bool setProperty(Q3CString,QVariant)";
            l << "QVariant property(Q3CString)";
/*
            Q3StrList lst = o->metaObject()->slotNames( true ); // ### how to replace?
            int i = 0;
            for ( Q3PtrListIterator<char> it( lst ); it.current(); ++it ) {
                if ( o->metaObject()->slot( i++, true )->access != QMetaData::Public )
                    continue;
                Q3CString slot = it.current();
                if ( slot.contains( "()" ) ) {
                    slot.prepend("void ");
                    l <<  slot;
                }
            }
*/
            reply << l;
            return true;
        } else if ( fun == "interfaces()" ) {
            replyType = "Q3CStringList";
            QDataStream reply( &replyData, QIODevice::WriteOnly );
            Q3CStringList l;
            const QMetaObject *meta = o->metaObject();
            while ( meta ) {
                l.prepend( meta->className() );
                meta = meta->superClass();
            }
            reply << l;
            return true;
        } else if ( fun == "properties()" ) {
            replyType = "Q3CStringList";
            QDataStream reply( &replyData, QIODevice::WriteOnly );
            Q3CStringList l;
            const QMetaObject *mo = o->metaObject();
            int count = mo->propertyCount();
            int offset = mo->propertyOffset();
            for ( int i = offset; i < count; i++) {
                const QMetaProperty p = mo->property( count );
                Q3CString prop = p.typeName();
                prop += ' ';
                prop += p.name();
                if ( !p.isWritable() )
                    prop += " readonly";
                l << prop;
            }
            reply << l;
            return true;
        } else if ( fun == "property(Q3CString)" ) {
            replyType = "QVariant";
            QByteArray dataCopy = data;
            QDataStream ds( &dataCopy, QIODevice::ReadOnly );
            Q3CString name;
            ds >> name ;
            QVariant result = o->property(  name );
            QDataStream reply( &replyData, QIODevice::WriteOnly );
            reply << result;
            return true;
        } else if ( fun == "setProperty(Q3CString,QVariant)" ) {
            QByteArray dataCopy = data;
            QDataStream ds( &dataCopy, QIODevice::ReadOnly );
            Q3CString name;
            QVariant value;
            ds >> name >> value;
            replyType = "bool";
            QDataStream reply( &replyData, QIODevice::WriteOnly );
            reply << (Q_INT8) o->setProperty( name, value );
            return true;
        } else {
            int slot = o->metaObject()->indexOfSlot( fun );
            if ( slot != -1 ) {
                replyType = "void";
                QMetaObject::invokeMember(o, fun );
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


bool DCOPClient::receive(const Q3CString &/*app*/, const Q3CString &objId,
                         const Q3CString &fun, const QByteArray &data,
                         Q3CString& replyType, QByteArray &replyData)
{
    d->transaction = false; // Assume no transaction.
    if ( objId == "DCOPClient" ) {
        if ( fun == "objects()" ) {
            replyType = "Q3CStringList";
            QDataStream reply( &replyData, QIODevice::WriteOnly );
            Q3CStringList l;
            if (d->qt_bridge_enabled)
            {
               l << "qt"; // the Qt bridge object
            }
            if ( kde_dcopObjMap ) {
                QMap<Q3CString, DCOPObject *>::ConstIterator it( kde_dcopObjMap->begin());
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
        if ( fun == "applicationRegistered(Q3CString)" ) {
            QByteArray dataCopy = data;
            QDataStream ds( &dataCopy, QIODevice::ReadOnly );
            Q3CString r;
            ds >> r;
            emit applicationRegistered( r );
            return true;
        } else if ( fun == "applicationRemoved(Q3CString)" ) {
            QByteArray dataCopy = data;
            QDataStream ds( &dataCopy, QIODevice::ReadOnly );
            Q3CString r;
            ds >> r;
            emit applicationRemoved( r );
            return true;
        }

        if ( process( fun, data, replyType, replyData ) )
            return true;
        // fall through and send to defaultObject if available

    } else if (d->qt_bridge_enabled &&
               (objId == "qt" || objId.left(3) == "qt/") ) { // dcop <-> qt bridge
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
        Q3PtrList<DCOPObject> matchList =
            DCOPObject::match(objId.left(objId.length()-1));
        for (DCOPObject *objPtr = matchList.first();
             objPtr != 0L; objPtr = matchList.next()) {
            objPtr->setCallingDcopClient(this);
            if (!objPtr->process(fun, data, replyType, replyData))
                return false;
        }
        return true;
    } else if (!DCOPObject::hasObject(objId)) {
        if ( DCOPObjectProxy::proxies ) {
            for ( Q3PtrListIterator<DCOPObjectProxy> it( *DCOPObjectProxy::proxies ); it.current();  ++it ) {
                // TODO: it.current()->setCallingDcopClient(this);
                if ( it.current()->process( objId, fun, data, replyType, replyData ) )
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
static bool findResultOk(Q3CString &replyType, QByteArray &replyData)
{
    Q_INT8 success; // Tsk.. why is there no operator>>(bool)?
    if (replyType != "bool") return false;

    QDataStream reply( &replyData, QIODevice::ReadOnly );
    reply >> success;

    if (!success) return false;
    return true;
}

// set the function result to DCOPRef pointing to (app,objId) and
// return true.
static bool findSuccess(const Q3CString &app, const Q3CString objId, Q3CString &replyType, QByteArray &replyData)
{
    DCOPRef ref(app, objId);
    replyType = "DCOPRef";

    replyData = QByteArray();
    QDataStream final_reply( &replyData, QIODevice::WriteOnly );
    final_reply << ref;
    return true;
}


bool DCOPClient::find(const Q3CString &app, const Q3CString &objId,
                      const Q3CString &fun, const QByteArray &data,
                      Q3CString& replyType, QByteArray &replyData)
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
        Q3PtrList<DCOPObject> matchList =
            DCOPObject::match(objId.left(objId.length()-1));
        for (DCOPObject *objPtr = matchList.first();
             objPtr != 0L; objPtr = matchList.next())
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


bool DCOPClient::call(const Q3CString &remApp, const Q3CString &remObjId,
                      const Q3CString &remFun, const QByteArray &data,
                      Q3CString& replyType, QByteArray &replyData,
                      bool useEventLoop)
{
    return call( remApp, remObjId, remFun, data, replyType, replyData, useEventLoop, -1 );
}

bool DCOPClient::call(const Q3CString &remApp, const Q3CString &remObjId,
                      const Q3CString &remFun, const QByteArray &data,
                      Q3CString& replyType, QByteArray &replyData,
                      bool useEventLoop, int timeout)
{
    if (remApp.isEmpty())
        return false;
    DCOPClient *localClient = findLocalClient( remApp );

    if ( localClient ) {
        bool saveTransaction = d->transaction;
        Q_INT32 saveTransactionId = d->transactionId;
        Q3CString saveSenderId = d->senderId;

        d->senderId = 0; // Local call
        bool b = localClient->receive(  remApp, remObjId, remFun, data, replyType, replyData );
        
        Q_INT32 id = localClient->transactionId();
        if (id) {
           // Call delayed. We have to wait till it has been processed.
           do {
              QApplication::processEvents( QEventLoop::WaitForMore);
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
        ReplyStruct *replyStruct = d->asyncReplyQueue.take(0);
        handleAsyncReply(replyStruct);
    }
}

int DCOPClient::callAsync(const Q3CString &remApp, const Q3CString &remObjId,
                const Q3CString &remFun, const QByteArray &data,
                QObject *callBackObj, const char *callBackSlot)
{
    Q3CString replyType;
    QByteArray replyData;

    ReplyStruct *replyStruct = new ReplyStruct;
    replyStruct->replyType = new Q3CString;
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

bool DCOPClient::callInternal(const Q3CString &remApp, const Q3CString &remObjId,
                      const Q3CString &remFun, const QByteArray &data,
                      Q3CString& replyType, QByteArray &replyData,
                      bool useEventLoop, int timeout, int minor_opcode)
{
#ifdef Q_OS_UNIX
    ReplyStruct replyStruct;
    replyStruct.replyType = &replyType;
    replyStruct.replyData = &replyData;
    return callInternal(remApp, remObjId, remFun, data, &replyStruct, useEventLoop, timeout, minor_opcode);
#else //!Q_OS_UNIX
    return false;
#endif
}

bool DCOPClient::callInternal(const Q3CString &remApp, const Q3CString &remObjId,
                      const Q3CString &remFun, const QByteArray &data,
                      ReplyStruct *replyStruct,
                      bool useEventLoop, int timeout, int minor_opcode)
{
#ifdef Q_OS_UNIX
    if ( !isAttached() )
        return false;

    DCOPMsg *pMsg;

    CARD32 oldCurrentKey = d->currentKey;
    if ( !d->currentKey )
        d->currentKey = d->key; // no key yet, initiate new call

    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
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
                        d->eventLoopTimer.start(time_left - guiTimeout, true);
                    qApp->enter_loop();
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
             *(replyStruct->replyType) = Q3CString();
             *(replyStruct->replyData) = QByteArray();
             replyStruct->status = ReplyStruct::Failed;
             break;
        }
    }

    // Wake up parent call, maybe it's reply is available already.
    if ( d->non_blocking_call_lock ) {
        qApp->exit_loop();
    }

    d->currentKey = oldCurrentKey;
    return replyStruct->status != ReplyStruct::Failed;
#else //!Q_OS_UNIX
    return false;
#endif
}

void DCOPClient::eventLoopTimeout()
{
    qApp->exit_loop();
}

void DCOPClient::processSocketData(int fd)
{
#ifdef Q_OS_UNIX
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
        qApp->exit_loop();
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
#endif //Q_OS_UNIX
}

void DCOPClient::setDefaultObject( const Q3CString& objId )
{
    d->defaultObject = objId;
}


Q3CString DCOPClient::defaultObject() const
{
    return d->defaultObject;
}

bool
DCOPClient::isLocalTransactionFinished(Q_INT32 id, Q3CString &replyType, QByteArray &replyData)
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
        d->transactionList = new Q3PtrList<DCOPClientTransaction>;

    d->transaction = true;
    DCOPClientTransaction *trans = new DCOPClientTransaction();
    trans->senderId = d->senderId;
    trans->id = ++d->transactionId;
    if (d->transactionId < 0)  // Ensure that ids > 0
        d->transactionId = 0;
#ifdef Q_OS_UNIX
    trans->key = d->currentKey;
#endif

    d->transactionList->append( trans );

    return trans;
}

Q_INT32
DCOPClient::transactionId() const
{
    if (d->transaction)
        return d->transactionId;
    else
        return 0;
}

void
DCOPClient::endTransaction( DCOPClientTransaction *trans, Q3CString& replyType,
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

    if ( !d->transactionList->removeRef( trans ) ) {
        qWarning("Transaction unknown: Not on list of pending transactions!");
        return; // Transaction
    }

    if (trans->senderId.isEmpty()) 
    {
        // Local transaction
        DCOPClientPrivate::LocalTransactionResult *result = new DCOPClientPrivate::LocalTransactionResult();
        result->replyType = replyType;
        result->replyData = replyData;
        
        d->localTransActionList.insert(trans->id, result);
        
        delete trans;

        return;
    }

#ifdef Q_OS_UNIX
    DCOPMsg *pMsg;

    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << d->appId << trans->senderId << trans->id << replyType << replyData;

    IceGetHeader(d->iceConn, d->majorOpcode, DCOPReplyDelayed,
                 sizeof(DCOPMsg), DCOPMsg, pMsg);
    pMsg->key = trans->key;
    pMsg->length += ba.size();

    IceSendData( d->iceConn, ba.size(), const_cast<char *>(ba.data()) );
#endif

    delete trans;
}

void
DCOPClient::emitDCOPSignal( const Q3CString &object, const Q3CString &signal, const QByteArray &data)
{
    // We hack the sending object name into the signal name
    send("DCOPServer", "emit", object+"#"+normalizeFunctionSignature(signal), data);
}

void
DCOPClient::emitDCOPSignal( const Q3CString &signal, const QByteArray &data)
{
    emitDCOPSignal(0, signal, data);
}

bool
DCOPClient::connectDCOPSignal( const Q3CString &sender, const Q3CString &senderObj,
  const Q3CString &signal,
  const Q3CString &receiverObj, const Q3CString &slot, bool Volatile)
{
    Q3CString replyType;
    QByteArray data, replyData;
    Q_INT8 iVolatile = Volatile ? 1 : 0;

    QDataStream args(&data, QIODevice::WriteOnly );
    args << sender << senderObj << normalizeFunctionSignature(signal) << receiverObj << normalizeFunctionSignature(slot) << iVolatile;

    if (!call("DCOPServer", 0,
        "connectSignal(Q3CString,Q3CString,Q3CString,Q3CString,Q3CString,bool)",
        data, replyType, replyData))
    {
        return false;
    }

    if (replyType != "bool")
        return false;

    QDataStream reply(&replyData, QIODevice::ReadOnly );
    Q_INT8 result;
    reply >> result;
    return (result != 0);
}

bool
DCOPClient::connectDCOPSignal( const Q3CString &sender, const Q3CString &signal,
  const Q3CString &receiverObj, const Q3CString &slot, bool Volatile)
{
    return connectDCOPSignal( sender, 0, signal, receiverObj, slot, Volatile);
}

bool
DCOPClient::disconnectDCOPSignal( const Q3CString &sender, const Q3CString &senderObj,
  const Q3CString &signal,
  const Q3CString &receiverObj, const Q3CString &slot)
{
    Q3CString replyType;
    QByteArray data, replyData;

    QDataStream args(&data, QIODevice::WriteOnly );
    args << sender << senderObj << normalizeFunctionSignature(signal) << receiverObj << normalizeFunctionSignature(slot);

    if (!call("DCOPServer", 0,
        "disconnectSignal(Q3CString,Q3CString,Q3CString,Q3CString,Q3CString)",
        data, replyType, replyData))
    {
        return false;
    }

    if (replyType != "bool")
        return false;

    QDataStream reply(&replyData, QIODevice::ReadOnly );
    Q_INT8 result;
    reply >> result;
    return (result != 0);
}

bool
DCOPClient::disconnectDCOPSignal( const Q3CString &sender, const Q3CString &signal,
  const Q3CString &receiverObj, const Q3CString &slot)
{
    return disconnectDCOPSignal( sender, 0, signal, receiverObj, slot);
}

void
DCOPClient::setPriorityCall(bool b)
{
#ifdef Q_OS_UNIX
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
          d->postMessageTimer.start( 0, true ); // Process queued messages
    }
#endif
}



void
DCOPClient::emergencyClose()
{
    Q3PtrList<DCOPClient> list;
    client_map_t *map = DCOPClient_CliMap;
    if (!map) return;
    Q3AsciiDictIterator<DCOPClient> it(*map);
    while(it.current()) {
       list.removeRef(it.current());
       list.append(it.current());
       ++it;
    }
#ifdef Q_OS_UNIX
    for(DCOPClient *cl = list.first(); cl; cl = list.next())
    {
        if (cl->d->iceConn) {
            IceProtocolShutdown(cl->d->iceConn, cl->d->majorOpcode);
            IceCloseConnection(cl->d->iceConn);
            cl->d->iceConn = 0L;
        }
    }
#endif
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

