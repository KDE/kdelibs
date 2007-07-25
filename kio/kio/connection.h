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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KIO_CONNECTION_H
#define KIO_CONNECTION_H

#if defined(MAKE_KIO_LIB) || defined(MAKE_KLAUNCHER)
# define KIO_CONNECTION_EXPORT  KIO_EXPORT
#else
# define KIO_CONNECTION_EXPORT  KIO_EXPORT_DEPRECATED
#endif

#include "kio_export.h"

#include <QtCore/QObject>
#include <QtCore/QString>

namespace KIO {

    class ConnectionPrivate;
    class ConnectionServer;
    /**
     * @private
     *
     * This class provides a simple means for IPC between two applications
     * via a pipe.
     * It handles a queue of commands to be sent which makes it possible to
     * queue data before an actual connection has been established.
     */
    class KIO_CONNECTION_EXPORT Connection : public QObject
    {
	Q_OBJECT
    public:
	/**
	 * Creates a new connection.
	 * @see connectToRemote, listenForRemote
	 */
	explicit Connection(QObject *parent = 0);
	virtual ~Connection();

        /**
         * Connects to the remote address.
         */
        void connectToRemote(const QString &address);

        /// Closes the connection.
	void close();

        QString errorString() const;

        bool isConnected() const;

        /**
	 * Checks whether the connection has been initialized.
	 * @return true if the initialized
	 * @see init()
	 */
	bool inited() const;

        /**
	 * Sends/queues the given command to be sent.
	 * @param cmd the command to set
	 * @param arr the bytes to send
	 * @return true if successful, false otherwise
	 */
	bool send(int cmd, const QByteArray &arr = QByteArray());

        /**
	 * Sends the given command immediately.
	 * @param _cmd the command to set
	 * @param data the bytes to send
	 * @return true if successful, false otherwise
	 */
	bool sendnow( int _cmd, const QByteArray &data );

        /**
         * Returns true if there are packets to be read immediately,
         * false if waitForIncomingTask must be called before more data
         * is available.
         */
        bool hasTaskAvailable() const;

        /**
         * Waits for one more command to be handled and ready.
         *
         * @param ms   the time to wait in milliseconds
         * @returns true if one command can be read, false if we timed out
         */
        bool waitForIncomingTask(int ms = 30000);

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
        bool suspended() const;

    Q_SIGNALS:
        void readyRead();

    private:
        Q_PRIVATE_SLOT(d, void dequeue())
        Q_PRIVATE_SLOT(d, void commandReceived(Task))
        Q_PRIVATE_SLOT(d, void disconnected())
        friend class ConnectionPrivate;
        friend class ConnectionServer;
	class ConnectionPrivate* const d;
    };

    class ConnectionServerPrivate;
    /**
     * @private
     *
     * This class provides a way to obtaining KIO::Connection connections.
     */
    class KIO_EXPORT ConnectionServer : public QObject
    {
        Q_OBJECT
    public:
        ConnectionServer(QObject *parent = 0);
        ~ConnectionServer();

        /**
         * Sets this connection to listen mode. Use address() to obtain the
         * address this is listening on.
         */
        void listenForRemote();
        bool isListening() const;
        /// Closes the connection.
	void close();

        /**
         * Returns the address for this connection if it is listening, an empty
         * string if not.
         */
        QString address() const;

        Connection *nextPendingConnection();
        void setNextPendingConnection(Connection *conn);
    Q_SIGNALS:
        void newConnection();

    private:
        friend class ConnectionServerPrivate;
        ConnectionServerPrivate * const d;
    };

}

#undef KIO_CONNECTION_EXPORT

#endif
