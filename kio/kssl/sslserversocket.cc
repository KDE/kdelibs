/* -*- C++ -*-

   This file implements a server socket that initiates SSL encrypted
   connections.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2003, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Id$
*/

extern "C" {
#include <openssl/err.h>
#include <unistd.h>
}

#include "sslserversocket.h"

bool SSLServerSocket::Initialized=false;

SSLServerSocket::SSLServerSocket(SSL_METHOD *meth_, SSL_CTX *ctx_,
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

SSLServerSocket::~SSLServerSocket()
{
    if(sd!=0) delete sd;
}

bool SSLServerSocket::Init()
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

bool SSLServerSocket::ok() const
{
    return !!sd;
}

void SSLServerSocket::incomingConnection(int)
{
    int fd;
    // -----
    fd=sd->accept();
    if(fd>0)
    {
        newConnection(fd);
    }
}

int SSLServerSocket::port() const
{
    if(sd==0)
    {
        return -1;
    } else {
        return sd->port();
    }
}

int SSLServerSocket::socket() const
{
    if(sd==0)
    {
        return -1;
    } else {
        return sd->socket();
    }
}

QHostAddress SSLServerSocket::address() const
{
    if(sd==0)
    {
        return QHostAddress();
    } else {
        return sd->address();
    }
}

void SSLServerSocket::setSocket(int sock)
{
    if(sd!=0) { delete sd; sd=0; }
    if(sn!=0) { delete sn; sn=0; }

    sd=new QSocketDevice(sock, QSocketDevice::Stream);
    sn=new QSocketNotifier(sd->socket(), QSocketNotifier::Read,
                           this, "Accepting new connections");
    connect(sn, SIGNAL(activated(int)), SLOT(incomingConnection(int)));
}

QSocketDevice *SSLServerSocket::socketDevice()
{
    return sd; // might be zero!
}

