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
    /**
     * Constructor for DCOP connection.
     * @param conn ICE protocol connection to use for this DCOP connection
     */
    DCOPConnection( IceConn conn );

		/**
		 * Destructor for DCOP connection.
		 */
    ~DCOPConnection();

    /**
     * List of signal connections.
     */
    DCOPSignalConnectionList *signalConnectionList();

    /**
     * Add the data from offset @p start in @p _data to the output
     * buffer and schedule it for later transmission.
     */
    void waitForOutputReady(const QByteArray &_data, int start);

    /**
     * Called from DCOPServer::slotOutputReady()
     * Flush the output buffer.
     */
    void slotOutputReady();

    /**
     * Application identifier, eg. "konsole-10140"
     */
    DCOPCString appId;
    /**
     * Application identifier without '-' and the number, eg. "konsole"
     */
    DCOPCString plainAppId;
    /** 
     * Inter Client Exchange connection
     */
    IceConn iceConn;
    /**
     * If > 0, application registration will be broadcasted.
     */
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
    /**
     * List of ICE connections waiting for reply.
     */
    QList <IceConn> waitingForReply;
		/** 
		 * List of ICE connections waiting for delayed (after DCOPReplyWait) reply.
		 */ 
    QList <IceConn> waitingForDelayedReply;
		/**
		 * List of signal connections.
		 */
    DCOPSignalConnectionList *_signalConnectionList;
		/**
		 * Determines whether the application runs as a daemon.
		 */
    bool daemon;
		/**
		 * Determines whether output of the application is blocked. If so, the
		 * data are stored in outputBuffer instead of being immediately sent.
		 * @see outputBuffer
		 */
    bool outputBlocked;
		/**
		 * Output buffer to store ICE data when output is blocked.
		 */
    QList <QByteArray> outputBuffer;
		/**
		 * Index of first valid element in outputBuffer.
		 */
    unsigned long outputBufferStart;
    /**
		 * Used in waitForOutputReady to be notified when output is ready.
		 */
    QSocketNotifier *outputBufferNotifier;
};


/**
 * @internal
 */
class DCOPServer : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates new server.
     * @param _suicide Determines whether the server should quit if no clients
     * connect in certain time period.
     */
    DCOPServer(bool _suicide);

    /**
     * Destroys the server.
     */
    ~DCOPServer();

    /**
     * Adds a connection to watch (process data when some activity occurs)
     * @param iceConn connection to watch
     */
    void* watchConnection( IceConn iceConn );
    /**
     * Removes a connection and deletes it.
     * @param data DCOPConnection to be removed
     */
    void removeConnection( void* data );    
    /**
     * Handles incoming DCOP message - reads it from sender, processes and sends
     * to the receiver.
     * @param iceConn ICE connection for the message
     * @param opcode Type of the message
     * @param length Length of the message
     * @param swap Unused?
     */
    void processMessage( IceConn iceConn, int opcode, unsigned long length, Bool swap);
    
    void ioError( IceConn iceConn );

    bool receive(const DCOPCString &app, const DCOPCString &obj,
                 const DCOPCString &fun, const QByteArray& data,
                 DCOPCString& replyType, QByteArray &replyData, IceConn iceConn);

    /**
     * Finds an application for given identificator.
     * @param appId application identificator to look for
     * @return Connection to this application if found, null otherwise
     */
    DCOPConnection *findApp(const DCOPCString &appId);
    /**
     * Find a DCOP connection for given ICE connection.
     * @param iceConn ICE connection to find DCOP connection for
     * @return DCOP connection for given ICE connection
     */
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
