/*
 * Copyright (C) 2000 Alex Zepeda <zipzippy@sonic.net>
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
 * Copyright (C) 2001 Dawit Alemayehu <adawit@kde.org>
 *
 * This file is part of the KDE project
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _TCP_SLAVEBASE_H
#define _TCP_SLAVEBASE_H

#include <sys/types.h>
#include <stdio.h>

#include <ksocketbase.h>
#include <kio/slavebase.h>

namespace KNetwork { class KStreamSocket; }

namespace KIO {

/**
 * There are two classes that specifies the protocol between application (job)
 * and kioslave. SlaveInterface is the class to use on the application end,
 * SlaveBase is the one to use on the slave end.
 *
 * Slave implementations should simply inherit SlaveBase
 *
 * A call to foo() results in a call to slotFoo() on the other end.
 */
class KIO_EXPORT TCPSlaveBase : public SlaveBase
{
public:
    TCPSlaveBase(unsigned short int defaultPort, const Q3CString &protocol,
                 const Q3CString &poolSocket, const Q3CString &appSocket);

    TCPSlaveBase(unsigned short int defaultPort, const Q3CString &protocol,
                 const Q3CString &poolSocket, const Q3CString &appSocket,
                 bool useSSL);

    virtual ~TCPSlaveBase();

protected:

    /**
     * This function acts like standard write function call
     * except it is also capable of making SSL or SOCKS
     * connections.
     *
     * @param data info to be sent to remote machine
     * @param len the length of the data to be sent
     *
     * @return the actual size of the data that was sent
     */
    ssize_t write(const char *data, ssize_t len);

    /**
     * This function acts like standard read function call
     * except it is also capable of deciphering SSL data as
     * well as handling data over SOCKSified connections.
     *
     * @param data storage for the info read from server
     * @param len lenght of the info to read from the server
     *
     * @return the actual size of data that was obtained
     */
    ssize_t read(char *data, ssize_t len);

    /**
     * Same as above except it reads data one line at a time.
     */
    ssize_t readLine(char *data, ssize_t len);
 
    /**
     * Sets the maximum size of blocks read in during calls to readLine().
     * This allows a slave to optimize for the protocol which it implements.
     * Ideally this should be (common_line_length+1) or so.
     * Making this too large will have adverse effects on performance.
     * Initial/default value is 256(bytes)
     */
    void setBlockSize(int sz);

    /**
     * Determines the appropriate port to use.
     *
     * This functions attempts to discover the appropriate port.
     *
     * @param _port the port to try, if it works, it is returned
     * @return the default port if the given port doesn't work
     */
    QString port(const QString& _port);

    /**
     * Performs the initial TCP connection stuff and/or
     * SSL handshaking as necessary.
     *
     * Please note that unlike its deprecated counterpart, this
     * function allows you to disable any error message from being
     * sent back to the calling application!  You can then use the
     * connectResult() function to determine the result of the
     * request for connection.
     *
     * @param host hostname
     * @param service service name (this is mapped to a port number)
     * @param sendError if true sends error message to calling app.
     *
     * @return on succes, true is returned.
     *         on failure, false is returned and an appropriate
     *         error message is send to the application.
     */
    bool connectToHost( const QString &host, const QString& service,
                        bool sendError = true );

    /**
     * Are we using SSL?
     *
     * @return if so, true is returned.
     *         if not, true isn't returned.
     * @since 3.2
     */
    bool usingSSL() const { return m_bIsSSL; }

    /**
     * Are we using TLS?
     *
     * @return if so, true is returned.
     *         if not, true isn't returned.
     * @since 3.2
     */
    bool usingTLS() const;

    /**
     * Can we use TLS?
     *
     * @return if so, true is returned.
     *         if not, true isn't returned.
     */
    bool canUseTLS();

    /**
     * Start using TLS on the connection.
     *
     * @return on success, 1 is returned.
     *         on failure, 0 is returned.
     *         on TLS init failure, -1 is returned.
     *         on connect failure, -2 is returned.
     *         on certificate failure, -3 is returned.
     */
    int startTLS();

    /**
     * Stop using TLS on the connection.
     */
    void stopTLS();

    /**
     * Closes the current file descriptor.
     *
     * Call this function to properly close up the socket
     * since it also takes care to prroperly close the stdio
     * fstream stuff, as well as sets the socket back to -1
     */
    void closeDescriptor();


    /**
     * Returns true when end of data is reached
     */
    bool atEnd();


    /**
     * Call this if you use persistent connections and want all the
     * metadata restored.  This is particularly important for SSL
     * sessions since the app needs to know the state of connection,
     * certificates, etc.
     */
    void setSSLMetaData();


    /**
     * Initializs all SSL variables
     */
    bool initializeSSL();


    /**
     * Cleans up all SSL settings.
     */
    void cleanSSL();

    /**
     * Determines whether or not we are still connected
     * to the remote machine.
     *
     * This method may fail to detect a closed SSL connection.
     *
     * return @p true if the socket is still active or
     *           false otherwise.
     */
    bool isConnectionValid();

    /**
     * Returns the status of the connection.
     *
     * This function allows you to invoke ConnectToHost
     * with the @p sendError flag set to false so that you
     * can send the appropriate error message back to the
     * calling io-slave.
     *
     * @return the error code after the connection
     */
    int connectResult();

    /**
     * Wait for some type of activity on the socket
     * for the period specified by @p t.
     *
     * @param t  length of time in seconds that we should monitor the
     *           socket before timing out.
     *
     * @return true if any activity was seen on the socket before the
     *              timeout value was reached, false otherwise.
     */
    bool waitForResponse( int t );

    /**
     * Sets the mode of the connection to blocking or non-blocking.
     *
     * Be sure to call this function before calling connectToHost.
     * Otherwise, this setting will not have any effect until the next
     * @p connectToHost.
     *
     * @param b true to make the connection a blocking one, false otherwise.
     */
    void setBlockConnection( bool b );

    /**
     * Sets how long to wait for orignally connecting to
     * the requested before timinig out.
     *
     * Be sure to call this function before calling ConnectToHost,
     * otherwise the setting will not take effect until the next call
     * to @p ConnectToHost.
     *
     * @param t timeout value
     */
    void setConnectTimeout( int t );

    /**
     * Returns true if SSL tunneling is enabled.
     *
     * @see setEnableSSlTunnel
     */
    bool isSSLTunnelEnabled();

    /**
     * Set up SSL tunneling mode.
     *
     * Calling this function with a @p true argument will allow
     * you to temprarly ignore the @p m_bIsSSL flag setting and
     * make a non-SSL connection.  It is mostly useful for making
     * connections to SSL sites through a non-transparent proxy
     * server (i.e. most proxy servers out there).
     *
     * Note that once you have successfully "tunneled" through the
     * proxy server you must call this function with its argument
     * set to false to properly connect to the SSL site.
     *
     * @param enable if true SSL Tunneling will be enabled
     */
    void setEnableSSLTunnel( bool enable );

    /**
     * Sets up the the real hostname for an SSL connection
     * that goes through a proxy server.
     *
     * This function is essential in making sure that the
     * real hostname is used for validating certificates from
     * SSL sites!
     *
     * @param realHost the actual host name we are connecting to
     */
    void setRealHost( const QString& realHost );

    // don't use me!
    void doConstructorStuff();

    // For the certificate verification code
    int verifyCertificate();

    // For prompting for the certificate to use
    void certificatePrompt();

    // Did the user abort (as the reason for connectToHost returning false)
    bool userAborted() const;

    /**
     * Return the socket object, if the class ever needs to do anything to it
     */
    KNetwork::KStreamSocket& socket();

protected:
    bool m_bIsSSL;
    QString m_port;
    unsigned short int m_iDefaultPort;
    Q3CString m_sServiceName;

private:
    bool doSSLHandShake( bool sendError );

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class TcpSlaveBasePrivate;
    TcpSlaveBasePrivate *d;
};

}

#endif
