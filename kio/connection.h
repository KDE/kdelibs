// -*- c++ -*-

#ifndef __connection_h__
#define __connection_h__ "$Id$"

#include <sys/types.h>

#include <stdio.h>
#include <kprocess.h>

class KSocket;
class QSocketNotifier;

namespace KIO {

    struct Task {
	int cmd;
	QByteArray data;
    };

    /**
     * This class provides a simple means for IPC between two applications
     * via a pipe.
     * It handles a queue of commands to be sent, and has a internal signal
     * called after a command has been sent, to send the next one (FIFO).
     */
    class Connection : public QObject
    {
	Q_OBJECT
    public:
	Connection();
	virtual ~Connection();
	
	void init(KSocket *sock);
	void connect(QObject *receiver = 0, const char *member = 0);
	void close();
	
	int fd_from() const { return fd_in; }
	
	void init(int fd_in, int fd_out);
	
	bool inited() const { return (fd_in != 0) && (f_out != 0); }
	
	// send (queues the command to be sent)
	void send(int cmd, const QByteArray &arr = QByteArray());

	// send (without queue)
	bool sendnow( int _cmd, const QByteArray &data );

	/**
	 * Receive data
	 *
	 * @return >=0 indicates the received data size upon success
	 *         -1  indicates error
	 */
	int read( int* _cmd, QByteArray & );

        /**
         * Don't handle incoming data until resumed
         */
        void suspend();

        /**
         * Resume handling of incoming data
         */
        void resume();

	void queueOnly(bool queue);

    protected slots:
        void dequeue();
	
    protected:
	
	
    private:
	bool queueonly;
	int fd_in;
	FILE *f_out;
	KSocket *socket;
	QSocketNotifier *notifier;
	QObject *receiver;
	const char *member;
	QList<Task> tasks;
    };

};

#endif
