/*
 * $Id$
 *
 * Copyright (C) 2000 Alex Zepeda <jazepeda@pacbell.net>
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _TCP_SLAVEBASE_H
#define _TCP_SLAVEBASE_H "$Id$"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>

#include <stdio.h>

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
class TCPSlaveBase
	: public SlaveBase
{
public:
    TCPSlaveBase(unsigned short int default_port, const QCString &protocol, const QCString &pool_socket, const QCString &app_socket);
    TCPSlaveBase(unsigned short int default_port, const QCString &protocol, const QCString &pool_socket, const QCString &app_socket, bool useSSL);
    virtual ~TCPSlaveBase();
protected:

    // These two act like the standard syscalls except they will
    // decipher SSL stuff or do SOCKS if needed
    ssize_t Write(const void *data, ssize_t len);
    ssize_t Read(void *data, ssize_t len);

    ssize_t ReadLine(char *data, ssize_t len);

    // This determines the appropiate port to use.
    unsigned short int GetPort(unsigned short int port);

    /**
     * This does the initial TCP connection stuff and/or SSL handshaking
     * 
     * @param host hostname
     * @param port port number
     * 
     * @return on succes, true is returned.
     *         on failure, false is returned and an appropriate error 
     *         message is send to the application.
     */
    bool ConnectToHost(const QCString &host, unsigned short int port);

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
     * @return on success, true is returned.
     *         on failure, true certainly isn't returned.
     */
    bool startTLS();

    /**
     * Stop using TLS on the connection.
     */
    void stopTLS();

    // The proper way to close up a socket here, as it closes the stdio
    // fstream stuff, as well as sets the socket back to -1
    void CloseDescriptor();

    // Initializs the SSL variables, called from the constructor
    // Don't call us
    bool InitializeSSL();
    void CleanSSL();

    bool AtEOF();

    bool m_bIsSSL;
    unsigned short int m_iDefaultPort, m_iPort;
    int m_iSock;
    QCString m_sServiceName;
    FILE *fp;

    class TcpSlaveBasePrivate;
    TcpSlaveBasePrivate *d;

    // don't use me!
    void doConstructorStuff();

};

};

#endif
