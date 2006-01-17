/*
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
*/
#ifndef DCOPSERVER_H
#define DCOPSERVER_H "$Id$"

#include <qobject.h>

#include <QHash>
#include <QByteArray>
#include <QList>
#include <QTextStream>
#include <qstring.h>
#include <qsocketnotifier.h>
#include <qapplication.h>
#include <kdatastream.h>

#define INT32 QINT32
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xmd.h>
#endif
#include <KDE-ICE/ICElib.h>
extern "C" {
#include <KDE-ICE/ICEutil.h>
#include <KDE-ICE/ICEmsg.h>
#include <KDE-ICE/ICEproto.h>
}

class DCOPConnection;
class DCOPListener;
class DCOPSignalConnectionList;
class DCOPSignals;
class QTimer;

// If you enable the following define DCOP will create
// $HOME/.dcop.log file which will list all signals passing
// through it.
//#define DCOP_LOG
#ifdef DCOP_LOG
class QTextStream;
class QFile;
#endif

/**
 * @internal
 */
class DCOPConnection : public QSocketNotifier
{
public:
    DCOPConnection( IceConn conn );
    ~DCOPConnection();

    DCOPSignalConnectionList *signalConnectionList();

    // Add the data from offset @p start in @p _data to the output
    // buffer and schedule it for later transmission.
    void waitForOutputReady(const QByteArray &_data, int start);

    // Called from DCOPServer::slotOutputReady()
    // Flush the output buffer.
    void slotOutputReady();

    DCOPCString appId;
    DCOPCString plainAppId;
    IceConn iceConn;
    int notifyRegister;
    /**
     * When client A has called client B then for the duration of the call:
     * A->waitingOnReply contains B
     *   and either
     * B->waitingForReply contains A
     *   or
     * B->waitingForDelayedReply contains A
     *
     * This allows us to do proper bookkeeping in case client A, client B
     * or both unregister during the call.
     */
    QList <IceConn> waitingOnReply;
    QList <IceConn> waitingForReply;
    QList <IceConn> waitingForDelayedReply;
    DCOPSignalConnectionList *_signalConnectionList;
    bool daemon;
    bool outputBlocked;
    QList <QByteArray> outputBuffer;
    unsigned long outputBufferStart;
    QSocketNotifier *outputBufferNotifier;
};


/**
 * @internal
 */
class DCOPServer : public QObject
{
    Q_OBJECT
public:
    DCOPServer(bool _suicide);
    ~DCOPServer();

    void* watchConnection( IceConn iceConn );
    void removeConnection( void* data );
    void processMessage( IceConn iceConn, int opcode, unsigned long length, Bool swap);
    void ioError( IceConn iceConn );

    bool receive(const DCOPCString &app, const DCOPCString &obj,
                 const DCOPCString &fun, const QByteArray& data,
                 DCOPCString& replyType, QByteArray &replyData, IceConn iceConn);

    DCOPConnection *findApp(const DCOPCString &appId);
    DCOPConnection *findConn(IceConn iceConn)
       { return clients.value(iceConn); }

    void sendMessage(DCOPConnection *conn, const DCOPCString &sApp,
                     const DCOPCString &rApp, const DCOPCString &rObj,
                     const DCOPCString &rFun, const QByteArray &data);

private Q_SLOTS:
    void newClient( int socket );
    void processData( int socket );
    void slotTerminate();
    void slotSuicide();
    void slotShutdown();
    void slotExit();
    void slotCleanDeadConnections();
    void slotOutputReady(int socket );

private:
    void broadcastApplicationRegistration( DCOPConnection* conn, const DCOPCString type,
        const DCOPCString& data );
    bool suicide;
    bool shutdown;
    int majorOpcode;
    int currentClientNumber;
    CARD32 serverKey;
    DCOPSignals *dcopSignals;
    QTimer *m_timer;
    QTimer *m_deadConnectionTimer;
    QList<DCOPListener*> listener;
    QHash<DCOPCString, DCOPConnection*> appIds; // index on app id
    QHash<IceConn, DCOPConnection*> clients; // index on iceConn
    QHash<int, DCOPConnection*> fd_clients; // index on fd
    QList<IceConn> deadConnections;

#ifdef DCOP_LOG
    QTextStream *m_stream;
    QFile *m_logger;
#endif
};

extern DCOPServer* the_server;

#endif
