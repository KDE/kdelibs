/* This file is part of the KDE project
 *
 * Copyright (C) 2001 - 2003 Mirko Boehm <mirko@kde.org>
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
 
 * This file implements a server socket that initiates SSL encrypted
 * connections.

 * $ Author: Mirko Boehm $
 * $ Copyright: (C) 2003, Mirko Boehm $
 * $ Contact: mirko@kde.org
 *            http://www.kde.org
 *            http://www.hackerbuero.org $
 *
 * $Id$
*/

extern "C" {
#include <openssl/err.h>
#include <unistd.h>
}

#include "sslserversocket.h"

using namespace KDESSL;

bool ServerSocket::Initialized=false;

ServerSocket::ServerSocket(SSL_METHOD *meth_, SSL_CTX *ctx_,
				 int port, int backlog,
				 QObject *parent, const char *name)
    : QObject(parent, name),
      meth(meth_),
      ctx(ctx_),
      ssl(0),
      sd(0),
      sn(0)
{
    Init();
    sd=new QSocketDevice;
    sd->setAddressReusable(true);
    if(sd->bind(QHostAddress(), port))
    {
        if(sd->listen(backlog))
	{
            sn=new QSocketNotifier(sd->socket(), QSocketNotifier::Read,
                                   this, "Accepting new connections");
            connect(sn, SIGNAL(activated(int)), SLOT(incomingConnection(int)));
	} else {
            delete sd; sd=0;
	}
    } else {
        delete sd; sd=0;
    }
}

ServerSocket::~ServerSocket()
{
    if(sd!=0) delete sd;
}

bool ServerSocket::Init()
{ // maybe this is obsolete since I decided not to initialize the SSL library in here
    if(Initialized)
    {
        return false;
    } else {
        Initialized=true;
        if(!ctx)
	{
            return false;
	} else {
            return true;
	}
    }
}

bool ServerSocket::ok() const
{
    return !!sd;
}

void ServerSocket::incomingConnection(int)
{
    int fd;
    // -----
    fd=sd->accept();
    if(fd>0)
    {
        newConnection(fd);
    }
}

int ServerSocket::port() const
{
    if(sd==0)
    {
        return -1;
    } else {
        return sd->port();
    }
}

int ServerSocket::socket() const
{
    if(sd==0)
    {
        return -1;
    } else {
        return sd->socket();
    }
}

QHostAddress ServerSocket::address() const
{
    if(sd==0)
    {
        return QHostAddress();
    } else {
        return sd->address();
    }
}

void ServerSocket::setSocket(int sock)
{
    if(sd!=0) { delete sd; sd=0; }
    if(sn!=0) { delete sn; sn=0; }

    sd=new QSocketDevice(sock, QSocketDevice::Stream);
    sn=new QSocketNotifier(sd->socket(), QSocketNotifier::Read,
                           this, "Accepting new connections");
    connect(sn, SIGNAL(activated(int)), SLOT(incomingConnection(int)));
}

QSocketDevice *ServerSocket::socketDevice()
{
    return sd; // might be zero!
}


#include "sslserversocket.moc"
