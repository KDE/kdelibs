/*
   This file is part of the KDB libraries
   Copyright (c) 2000 Praduroux Alessandro <pradu@thekompany.com>
 
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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/     
#include "connector.h"
#include "exception.h"

#include <kdebug.h>

using namespace KDB;

Connector::Connector()
    : _connected(false)
{
}

Connector::Connector(const Connector &)
    : _connected(false)
{
}

Connector::~Connector()
{
    kdDebug(20000) << "Connector::~Connector " << this << endl;
    /*
      if ( _connected )
      this->close();
    */
}

void
Connector::setHost(const QString & host)
{
    _host = host;
}

void
Connector::setPort(int port)
{
    _port = port;
}

void
Connector::setUser(const QString & user)
{
    _user = user;
}

void
Connector::setPassword(const QString & pwd)
{
    _pwd = pwd;
}

QString
Connector::host() const
{
    return _host;
}

int
Connector::port() const
{
    return _port;
}

QString
Connector::user() const
{
    return _user;
}

QString
Connector::password() const
{
    return _pwd;
}

bool
Connector::isConnected() const
{
    return _connected;
}


void
Connector::setConnected(bool conn)
{
    _connected = conn;
}

void
Connector::close()
{
    setConnected(false);
}



