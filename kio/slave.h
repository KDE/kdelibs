// -*- c++ -*-

#ifndef KIO_SLAVE_H
#define KIO_SLAVE_H

#include <qobject.h>
#include "kio/slaveinterface.h"
#include "kio/connection.h"
#include <kurl.h>
#include <time.h>

class KServerSocket;
class KSocket;

namespace KIO {

    class Slave : public KIO::SlaveInterface
    {
	Q_OBJECT
	
    public:
	Slave(KServerSocket *unixdomain, const QString &protocol);
	void resume() {}
	void suspend() {}

	/**
	 * Force termination
	 */
	void kill();

        /**
         * @return true if the slave survided the last mission.
         */
        bool isAlive() { return !dead; }

        /**
         * Open connection for url
         * @param host to connect to.
         * @param port to connect to.
         * @param user to login as
         * @param passwd to login with
         */
        void openConnection( const QString &host, int port,
                             const QString &user, const QString &passwd);

        /**
         * Close connection (forced).
         */
        void closeConnection();

        /**
         * @return Protocol handled by this slave
         */
        QString protocol() { return m_protocol; }

        /**
         * @return Host this slave is (was?) connected to
         */
        QString host() { return m_host; }

        /**
         * @return port this slave is (was?) connected to
         */
        int port() { return m_port; }

        /**
         * @return User this slave is (was?) logged in as
         */
        QString user() { return m_user; }

        /**
         * @return Passwd used to log in
         */
        QString passwd() { return m_passwd; }

	/**
	 * Creates a new slave if the @ref KIOSlavePool has no matching one.
	 * @ref m_pSlave and @ref m_strSlaveProtocol are set accordingly on success.
	 *
	 * @param _error is the error code on failure and undefined else.
	 * @param _error_text is the error text on failure and undefined else.
	 *
	 * @return @ref m_pSlave on success or 0L on failure.
	 */
	static Slave* createSlave( const KURL& url, int& error, QString& error_text );
	
    public slots:
        void accept(KSocket *socket);
	void gotInput(int socket);
	void gotAnswer(int socket);
    signals:
        void slaveDied(KIO::Slave *slave);
	
    private:
        QString m_protocol;
        QString m_host;
        int m_port;
        QString m_user;
        QString m_passwd;
	KServerSocket *serv;
	bool contacted;
	bool dead;
	time_t contact_started;
	KIO::Connection slaveconn;
    };

};

#endif
