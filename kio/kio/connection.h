// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>

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

#ifndef __connection_h__
#define __connection_h__ "$Id$"

#include <sys/types.h>

#include <stdio.h>
#include <qptrlist.h>

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
     * It handles a queue of commands to be sent which makes it possible to
     * queue data before an actual connection has been established.
     */
    class Connection : public QObject
    {
	Q_OBJECT
    public:
	/**
	 * Creates a new connection.
	 * @see init()
	 */
	Connection();
	virtual ~Connection();

        /**
	 * Initialize this connection to use the given socket.
	 * @param sock the socket to use
	 * @see inited()
	 */
	void init(KSocket *sock);
        /**
	 * Initialize the connection to use the given file
	 * descriptors.
	 * @param fd_in the input file descriptor to use
	 * @param fd_out the output file descriptor to use
	 * @see inited()
	 */
	void init(int fd_in, int fd_out); // Used by KDENOX
	void connect(QObject *receiver = 0, const char *member = 0);
        /// Closes the connection.
	void close();
	
        /**
	 * Returns the input file descriptor.
	 * @return the input file descriptor
	 */
	int fd_from() const { return fd_in; }
        /**
	 * Returns the output file descriptor.
	 * @return the output file descriptor
	 */
        int fd_to() const { return fileno( f_out ); }

        /**
	 * Checks whether the connection has been initialized.
	 * @return true if the initialized
	 * @see init()
	 */
	bool inited() const { return (fd_in != -1) && (f_out != 0); }
	
        /** 
	 * Sends/queues the given command to be sent.
	 * @param cmd the command to set
	 * @param arr the bytes to send
	 */
	void send(int cmd, const QByteArray &arr = QByteArray());

        /** 
	 * Sends the given command immediately.
	 * @param cmd the command to set
	 * @param arr the bytes to send
	 * @return true if successful, false otherwise
	 */
	bool sendnow( int _cmd, const QByteArray &data );

	/**
	 * Receive data.
	 *
	 * @param _cmd the received command will be written here
	 * @param data the received data will be written here
	 * @return >=0 indicates the received data size upon success
	 *         -1  indicates error
	 */
	int read( int* _cmd, QByteArray &data );

        /**
         * Don't handle incoming data until resumed.
         */
        void suspend();

        /**
         * Resume handling of incoming data.
         */
        void resume();

        /**
         * Returns status of connection.
	 * @return true if suspended, false otherwise
         */
        bool suspended() const { return m_suspended; }

    protected slots:
        void dequeue();
	
    protected:
	
	
    private:
	int fd_in;
	FILE *f_out;
	KSocket *socket;
	QSocketNotifier *notifier;
	QObject *receiver;
	const char *member;
	QPtrList<Task> tasks;
        bool m_suspended;
    private:
	class ConnectionPrivate* d;
    };

};

#endif
