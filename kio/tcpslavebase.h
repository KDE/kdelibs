/*
 * $Id$
 *
 * Copyright (C) 2000 Alex Zepeda <jazepeda@pacbell.net>
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
#define _TCP_SLAVEBASE_H "$Id"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#undef HAVE_SSL

#include <sys/types.h>

#include <stdio.h>

#ifdef HAVE_SSL
extern "C" {
	#include <openssl/ssl.h>
}
#endif

#include "kio/slavebase.h"


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
    virtual ~TCPSlaveBase();
protected:

    // These two act like the standard syscalls except they will
    // decipher SSL stuff if needed
    ssize_t Write(const void *data, ssize_t len);
    ssize_t Read(void *data, ssize_t len);

    ssize_t ReadLine(char *data, ssize_t len);

    // This determines the appropiate port to use.
    unsigned short int GetPort(unsigned short int port);

    // This does the initial TCP connection stuff and/or SSL handshaking
    bool ConnectToHost(const QCString &host, unsigned short int port);

    // The proper way to close up a socket here, as it closes the stdio
    // fstream stuff, as well as sets the socket back to -1
    void CloseDescriptor();

    // Initializs the SSL variables, called from the constructor
    bool InitializeSSL();
    void CleanSSL();

    bool AtEOF();

    bool m_bIsSSL;
    unsigned short int m_iDefaultPort, m_iPort;
    int m_iSock;
    QCString m_sServiceName;
    FILE *fp;

#ifdef HAVE_SSL
    SSL_CTX *ssl_context;
    SSL *ssl;
    X509 *server_cert;
    SSL_METHOD *meth;
#endif
};

};

#endif
