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
#ifndef CONNECTORIMPL_H
#define CONNECTORIMPL_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <mysql/mysql.h>
#include <kdb/connector.h>
#include <kdb/table.h>
#include <kdb/field.h>

#include <qvaluelist.h>
#include <qstringlist.h>

#include <ksharedptr.h>
#include <kdebug.h>

struct connRep : public KShared {

    MYSQL *conn;

    ~connRep() {
        qDebug("~connRep");
        mysql_close(conn);
    }

};


class ConnectorImpl: public KDB::Connector {

public:
    ConnectorImpl();
    ConnectorImpl(const ConnectorImpl& c);
    ~ConnectorImpl();

    bool connect();
    void close();

    KDB::Connector *clone();

    QStringList databases() ;
    QStringList tables() ;
    KDB::RowList fields(const QString & tableName) ;

    KDB_ULONG execute(const QString &sql);

    KDB::DataType nativeToKDB(const QString &type);
    QString KDBToNative(KDB::DataType type);

    static KDB::DataType NToK(const QString &type);
    static QString KToN(KDB::DataType type);

    bool createDatabase(const QString & name) ;
    bool dropDatabase(const QString & name) ;
    bool createTable(const KDB::Table &tab) ;
    bool dropTable(const QString & name) ;

    bool setCurrentDatabase(const QString &name);

    KDB::Handler *query(const QString &SQL) ;

protected:

    MYSQL * connection();
    KDB::RowList resultQuery(const QString &sql);

    KSharedPtr<connRep> conn;
};


#endif


