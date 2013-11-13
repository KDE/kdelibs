/*
 * Copyright (C) 2000 Alex Zepeda <zipzippy@sonic.net>
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
 * Copyright (C) 2001 Dawit Alemayehu <adawit@kde.org>
 * Copyright (C) 2007,2008 Andreas Hartmetz <ahartmetz@gmail.com>
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

#include <kio/kiocore_export.h>
#include <kio/slavebase.h>

class QIODevice;

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
class KIOCORE_EXPORT TCPSlaveBase : public SlaveBase
{
public:
    /**
     * Constructor.
     *
     * @param autoSsl if true, will automatically invoke startSsl() right after
     *                connecting. In the absence of errors the use of SSL will
     *                therefore be transparent to higher layers.
     */
    TCPSlaveBase(const QByteArray &protocol,
                 const QByteArray &poolSocket, const QByteArray &appSocket,
                 bool autoSsl = false);

    virtual ~TCPSlaveBase();

protected:
    enum SslResultDetail {
        ResultOk = 1,
        ResultOverridden = 2,
        ResultFailed = 4,
        ResultFailedEarly = 8
    };
    friend class QFlags<KIO::TCPSlaveBase::SslResultDetail>;
public:
    Q_DECLARE_FLAGS(SslResult, SslResultDetail)
protected:

    /**
     * Send data to the remote host.
     *
     * @param data data to be sent to remote machine
     * @param len the length (in bytes) of the data to be sent
     *
     * @return the actual size of the data that was sent
     */
    using SlaveBase::write; //Silence incompatible virtual override warning
    ssize_t write(const char *data, ssize_t len);

    /**
     * Read incoming data from the remote host.
     *
     * @param data storage for the data read from server
     * @param len length of the data (in bytes) to read from the server
     *
     * @return the actual size of data that was obtained
     */
    using SlaveBase::read;
    ssize_t read(char *data, ssize_t len);

    /**
     * Same as read() except it reads data one line at a time.
     */
    ssize_t readLine(char *data, ssize_t len);

    /**
     * Performs the initial TCP connection stuff and/or
     * SSL handshaking as necessary.
     *
     * @param protocol the protocol being used
     * @param host hostname
     * @param port port number
     *
     * @return on succes, true is returned.
     *         on failure, false is returned and an appropriate
     *         error message is sent to the application.
     */
    bool connectToHost(const QString &protocol, const QString& host, quint16 port);

    /**
     * Connects to the specified host and port.
     *
     * @param host host name
     * @param port port number
     * @param errorString if not NULL, this string will contain error information
     *                    on why the connection request failed.
     *
     * @return  on success, 0 is returned. on failure, a KIO::Error code is returned.
     *          @ref errorString, if not NULL, will contain the appropriate error message
     *          that can be sent back to the client.
     *
     * @since 4.7.2
     */
    int connectToHost(const QString& host, quint16 port, QString* errorString = 0);

    /**
     * the current port for this service
     *
     */
    quint16 port() const;

    /**
     * Will start SSL after connecting?
     *
     * @return if so, true is returned.
     *         if not, false is returned.
     */
    bool isAutoSsl() const;

    /**
     * Is the current connection using SSL?
     *
     * @return if so, true is returned.
     *         if not, false is returned.
     */
    bool isUsingSsl() const;

    /**
     * Start using SSL on the connection. You can use it right after connecting
     * for classic, transparent to the protocol SSL. Calling it later can be
     * used to implement e.g. SMTP's STARTTLS feature.
     *
     * @return on success, true is returned.
     *         on failure, false is returned.
     */
    bool startSsl();

    /**
     * Close the connection and forget non-permanent data like the peer host.
     */
    void disconnectFromHost();

    /**
     * Returns true when end of data is reached.
     */
    bool atEnd() const;

    /**
     * Determines whether or not we are still connected
     * to the remote machine.
     *
     * return @p true if the socket is still active or
     *           false otherwise.
     */
    bool isConnected() const;

    /**
     * Wait for incoming data on the socket
     * for the period specified by @p t.
     *
     * @param t  length of time in seconds that we should monitor the
     *           socket before timing out.
     *
     * @return true if any data arrived on the socket before the
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
    void setBlocking( bool b );

    /**
     * Return the socket object, if the class ever needs to do anything to it
     */
    QIODevice *socket() const;

protected:
    virtual void virtual_hook( int id, void* data );

private:
    // For the certificate verification code
    SslResult verifyServerCertificate();

    // For prompting for the client certificate to use
    void selectClientCertificate();

    class TcpSlaveBasePrivate;
    TcpSlaveBasePrivate* const d;
};

}

#endif
