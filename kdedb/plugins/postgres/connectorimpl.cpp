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
#include "connectorimpl.h"

#include <kdebug.h>

#include <kdb/table.h>
#include <kdb/dbengine.h>
#include <kdb/exception.h>

ConnectorImpl::ConnectorImpl()
{
}

ConnectorImpl::ConnectorImpl(const ConnectorImpl &c)
    : Connector(c), conn(c.conn)
{
    setHost( c.host() );
    setPort( c.port() );
    setUser( c.user() );
    setPassword( c.password() );
    setConnected( c.isConnected() );

}

ConnectorImpl::~ConnectorImpl()
{
}

bool ConnectorImpl::connect()
{
    if (conn) {
        PQfinish(conn);
        conn = 0;
    }
    
    QString connString = QString("host=%1 user=%2 password=%3");
    
    connString = connString.arg(host()).arg(user()).arg(password());

    conn = PQconnectdb(connString.ascii());                               
    
    return false;
}

void ConnectorImpl::close()
{
}

KDB::Connector *
ConnectorImpl::clone()
{
    return new ConnectorImpl(*this);
}

QStringList
ConnectorImpl::databases()
{
}

QStringList
ConnectorImpl::tables()
{
}

KDB::RowList
ConnectorImpl::fields(const QString & tableName)
{
}

bool
ConnectorImpl::createDatabase(const QString & name)
{
}

KDB::RowList
ConnectorImpl::resultQuery(const QString &sql)
{
}

bool
ConnectorImpl::setCurrentDatabase(const QString &name)
{
}

KDB_ULONG
ConnectorImpl::execute(const QString &sql)
{
}

KDB::Handler *
ConnectorImpl::query(const QString &SQL)
{
}

KDB::DataType
ConnectorImpl::nativeToKDB(const QString &type)
{
    KDB::DataType ret = KDB::CHAR; 
    return ret;
}

QString
ConnectorImpl::KDBToNative(KDB::DataType type)
{
    QString ret = "CHAR"; 
    return ret;
}

bool
ConnectorImpl::createTable(const KDB::Table &tab)
{
    return true;
}

bool
ConnectorImpl::dropDatabase(const QString & name)
{
    return true;
}

bool
ConnectorImpl::dropTable(const QString & name)
{
    return true;
}



