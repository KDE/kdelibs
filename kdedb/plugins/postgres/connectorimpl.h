/*
 *  This file is part of the KDB libraries
 *  Copyright (c) 2000 Praduroux Alessandro <pradu@thekompany.com>
 *
 *  $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */     

#ifndef PGSQL_CONNECTORIMPL_H
#define PGSQL_CONNECTORIMPL_H "$Id$"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libpq-fe.h>
#include <kdb/connector.h>

#include <ksharedptr.h>

#include <qvaluelist.h>
#include <qstringlist.h>
#include <qintcache.h>

struct connRep
	: public KShared {
public:
	PGconn *conn;

	~connRep() {
		PQfinish(conn);
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

    QStringList databases();
    QStringList tables();
    KDB::RowList fields(const QString &tableName);

    KDB_ULONG execute(const QString &sql);

    KDB::DataType nativeToKDB(const QString &type);
    QString KDBToNative(KDB::DataType type);

    static KDB::DataType _nativeToKDB(const QString &type);
    static QString _KDBToNative(KDB::DataType type);

    bool createDatabase(const QString &name);
    bool dropDatabase(const QString &name);
    bool createTable(const KDB::Table &tab);
    bool dropTable(const QString &name);

    bool appendField(const QString &table, KDB::Field *f);
    bool removeField(const QString &table, const QString &field);
    bool changeField(const QString &table, KDB::Field *f);

    bool setCurrentDatabase(const QString &name);

    KDB::Handler *query(const QString &SQL);

    void beginTransaction();
    void commit();
    void rollback();

    QString oidToTypeName(Oid oid);

protected:

    PGconn * connection() { return conn->conn; };

    QString m_database;
    
    KDB::RowList resultQuery(const QString &sql);
    KSharedPtr<connRep> conn;

    QIntCache<QString> m_typeCache;
};


#endif
