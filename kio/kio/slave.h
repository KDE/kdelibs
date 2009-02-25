// -*- c++ -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2000 Waldo Bastian <bastian@kde.org>
 *                2000 Stephan Kulow <coolo@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef KIO_SLAVE_H
#define KIO_SLAVE_H

#define KIO_SLAVE_EXPORT KIO_EXPORT

#include <time.h>
#include <unistd.h>

#include <QtCore/QObject>

#include <kurl.h>

#include "kio/slaveinterface.h"

namespace KIO {

    class SlavePrivate;
    // Attention developers: If you change the implementation of KIO::Slave,
    // do *not* use connection() or slaveconn but the respective KIO::Slave
    // accessor methods. Otherwise classes derived from Slave might break. (LS)
    //
    // Do not use this class directly, outside of KIO. Only use the Slave pointer
    // that is returned by the scheduler for passing it around.
    //
    // TODO: KDE5: Separate public API and private stuff for this better
    class KIO_SLAVE_EXPORT Slave : public KIO::SlaveInterface
    {
	Q_OBJECT
    public:
	explicit Slave(const QString &protocol, QObject *parent = 0);

        virtual ~Slave();

	void setPID(pid_t);

        int slave_pid();

	/**
	 * Force termination
	 */
	void kill();

        /**
         * @return true if the slave survived the last mission.
         */
        bool isAlive();

        /**
         * Set host for url
         * @param host to connect to.
         * @param port to connect to.
         * @param user to login as
         * @param passwd to login with
         */
        virtual void setHost( const QString &host, quint16 port,
                      const QString &user, const QString &passwd);

        /**
         * Clear host info.
         */
        void resetHost();

        /**
         * Configure slave
         */
        virtual void setConfig(const MetaData &config);

        /**
	 * The protocol this slave handles.
	 *
         * @return name of protocol handled by this slave, as seen by the user
         */
        QString protocol();

        void setProtocol(const QString & protocol);
        /**
	 * The actual protocol used to handle the request.
	 *
	 * This method will return a different protocol than
	 * the one obtained by using protocol() if a
	 * proxy-server is used for the given protocol.  This
	 * usually means that this method will return "http"
	 * when the actuall request was to retrieve a resource
	 * from an "ftp" server by going through a proxy server.
	 *
         * @return the actual protocol (io-slave) that handled the request
         */
        QString slaveProtocol();

        /**
         * @return Host this slave is (was?) connected to
         */
        QString host();

        /**
         * @return port this slave is (was?) connected to
         */
        quint16 port();

        /**
         * @return User this slave is (was?) logged in as
         */
        QString user();

        /**
         * @return Passwd used to log in
         */
        QString passwd();

	/**
	 * Creates a new slave.
	 *
	 * @param protocol the protocol
	 * @param url is the url
	 * @param error is the error code on failure and undefined else.
	 * @param error_text is the error text on failure and undefined else.
	 *
	 * @return 0 on failure, or a pointer to a slave otherwise.
	 */
	static Slave* createSlave( const QString &protocol, const KUrl& url, int& error, QString& error_text );

	/**
	 * Requests a slave on hold for ths url, from klauncher, if there is such a job.
	 * See hold()
	 */
	static Slave* holdSlave( const QString &protocol, const KUrl& url );

	// == communication with connected kioslave ==
	// whenever possible prefer these methods over the respective
	// methods in connection()
	/**
	 * Suspends the operation of the attached kioslave.
	 */
        virtual void suspend();
	/**
	 * Resumes the operation of the attached kioslave.
	 */
        virtual void resume();
	/**
	 * Tells whether the kioslave is suspended.
	 * @return true if the kioslave is suspended.
	 */
        virtual bool suspended();
	/**
	 * Sends the given command to the kioslave.
	 * @param cmd command id
	 * @param arr byte array containing data
	 */
        virtual void send(int cmd, const QByteArray &arr = QByteArray());

	// == end communication with connected kioslave ==
	/**
	 * Puts the kioslave associated with @p url at halt, and return it to klauncher, in order
	 * to let another application connect to it and finish the job.
	 * This is for the krunner case: type a URL in krunner, it will start downloading
	 * to find the mimetype (KRun), and then hold the slave, publish the held slave using,
	 * this method, and the final application can continue the same download by requesting
	 * the same URL.
	 */
	virtual void hold(const KUrl &url); // TODO KDE5: no reason to be virtual

	/**
	 * @return The time this slave has been idle.
	 */
	time_t idleTime();

	/**
	 * Marks this slave as idle.
	 */
	void setIdle();

        /*
         * @returns Whether the slave is connected
         * (Connection oriented slaves only)
         */
        bool isConnected();
        void setConnected(bool c);

        void ref();
        void deref();

    public Q_SLOTS:
        void accept();
	void gotInput();
	void timeout();
    Q_SIGNALS:
        void slaveDied(KIO::Slave *slave);

    private:
        Q_DECLARE_PRIVATE(Slave)
    };
}

#undef KIO_SLAVE_EXPORT

#endif
