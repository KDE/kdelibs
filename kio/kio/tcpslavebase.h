/*
 * $Id$
 *
 * Copyright (C) 2000 Alex Zepeda <zipzippy@sonic.net>
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
 * Copyright (C) 2001 Dawit Alemayehu <adawit@kde.org>
 *
 * This file is part of the KDE project
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _TCP_SLAVEBASE_H
#define _TCP_SLAVEBASE_H "$Id$"

#include <sys/types.h>

#include <stdio.h>

#include <kextsock.h>
#include <kio/slavebase.h>


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
class TCPSlaveBase : public SlaveBase
{
public:
    TCPSlaveBase(unsigned short int defaultPort, const QCString &protocol,
                 const QCString &poolSocket, const QCString &appSocket);

    TCPSlaveBase(unsigned short int defaultPort, const QCString &protocol,
                 const QCString &poolSocket, const QCString &appSocket,
                 bool useSSL);

    virtual ~TCPSlaveBase();

protected:

#ifndef KDE_NO_COMPAT
    /**
     * @deprecated Due to inconsistency with KDE naming convention.
     */
    ssize_t Write(const void *data, ssize_t len) { return write( data, len ); }

    /**
     * @deprecated Due to inconsistency with KDE naming convention.
     */
    ssize_t Read(void *data, ssize_t len) { return read( data, len ); }

    /**
     * @deprecated Due to inconsistency with KDE naming convention.
     */
    ssize_t ReadLine(char *data, ssize_t len) { return readLine( data, len ); }

    /**
     * @deprecated Due to inconsistency with KDE naming convention.
     */
    unsigned short int GetPort(unsigned short int p) { return port(p); }

    /**
     * @deprecated Due to inconsistency with KDE naming convention.
     */
    bool ConnectToHost( const QString &host, unsigned int port,
                        bool sendError ) { return connectToHost( host, port, sendError ); }

    /**
     * @deprecated Due to inconsistency with KDE naming convention.
     */
    void CloseDescriptor() { closeDescriptor(); }

    /**
     * @deprecated Due to inconsistency with KDE naming convention.
     */
    bool AtEOF() { return atEnd(); }

    /**
     * @deprecated Due to inconsistency with KDE naming convention.
     */
    bool InitializeSSL() { return initializeSSL(); }

    /**
     * @deprecated Due to inconsistency with KDE naming convention.
     */
    void CleanSSL() { cleanSSL(); }
#endif

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
    ssize_t write(const void *data, ssize_t len);

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
    ssize_t read(void *data, ssize_t len);

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
     * Determines the appropiate port to use.
     *
     * This functions attempts to discover the appropriate port.
     *
     * @param port
     */
    unsigned short int port(unsigned short int);

    /**
     * Performs the initial TCP connection stuff and/or
     * SSL handshaking as necessary.
     *
     * Please note that unlike its deprecated counterpart, this
     * function allows you to disable any error message from being
     * sent back to the calling application!  You can then use the
     * @ref connectResult() function to determine the result of the
     * request for connection.
     *
     * @param host hostname
     * @param port port number
     * @param sendError if true sends error message to calling app.
     *
     * @return on succes, true is returned.
     *         on failure, false is returned and an appropriate
     *         error message is send to the application.
     */
    bool connectToHost( const QString &host, unsigned int port,
                        bool sendError = true );

    /**
     * Are we using TLS?
     *
     * @return if so, true is returned.
     *         if not, true isn't returned.
     */
    bool usingTLS();

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
     * metadata restored.  This is particularily important for SSL
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
     * return @p true if the socket is still active or
     *           false otherwise.
     */
    bool isConnectionValid();

    /**
     * Returns the status of the connection.
     *
     * This function allows you to invoke @ref ConnectToHost
     * with the @p sendError flag set to false so that you
     * can send the appropriate error message back to the
     * calling io-slave.
     *
     * @return the status code as returned by KExtendedSocket.
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
     * Be sure to call this function before calling @ref ConnectToHost.
     * Otherwise, this setting will not have any effect until the next
     * @p ConnectToHost.
     *
     * @param b true to make the connection a blocking one, false otherwise.
     */
    void setBlockConnection( bool b );

    /**
     * Sets how long to wait for orignally connecting to
     * the requested before timinig out.
     *
     * Be sure to call this function before calling @ref ConnectToHost,
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

protected:
    int m_iSock;
    bool m_bIsSSL;
    unsigned short int m_iPort;
    unsigned short int m_iDefaultPort;
    QCString m_sServiceName;
    FILE *fp;

private:
    bool doSSLHandShake( bool sendError );

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class TcpSlaveBasePrivate;
    TcpSlaveBasePrivate *d;
};

};

#endif
