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
#include "handlerimpl.h"

#include <mysql/errmsg.h>
#include <mysql/mysqld_error.h>

#include <kdb/exception.h>
#include <kdb/dbengine.h>
#include <kdb/kdb.h>
#include <kdb/value.h>

#include <kdebug.h>


/* static QByteArray strToBA(const QString &s)
   {
   QByteArray ret = s.utf8();
   return ret;
   }
*/

ConnectorImpl::ConnectorImpl()
{
    kdDebug(20012) << "ConnectorImpl::ConnectorImpl" << endl;
    conn = 0L;
}

ConnectorImpl::ConnectorImpl(const ConnectorImpl &c)
    : Connector(c)
{
    kdDebug(20012) << "ConnectorImpl::ConnectorImpl (const ConnectorImpl &c)" << endl;
    conn = c.conn;

    setHost( c.host() );
    setPort( c.port() );
    setUser( c.user() );
    setPassword( c.password() );
    setConnected( c.isConnected() );
}

ConnectorImpl::~ConnectorImpl()
{
#ifndef NDEBUG
    qDebug("ConnectorImpl::~ConnectorImpl");
#endif
}

MYSQL *
ConnectorImpl::connection()
{
    kdDebug(20012) << "ConnectorImpl::connection" << endl;
    return conn->conn;
}

bool
ConnectorImpl::connect()
{
    kdDebug(20012) << "ConnectorImpl::connect" << endl;
    bool ret = true;

    if (isConnected()) {
        DBENGINE->pushError(new KDB::InvalidRequest("Already connected to server"));
        return false;
    }

    connRep *c = new connRep;
    c->conn = mysql_init(0L);
    conn = c;
    
    if (!mysql_real_connect(connection(),
                            host().latin1(),
                            user().latin1(),
                            password().latin1(),
                            0,
                            port(),
                            0,
                            0 )) {
        int error = mysql_errno(connection());
        switch (error) {
        case CR_CONN_HOST_ERROR:
        case CR_CONNECTION_ERROR:
        case ER_ACCESS_DENIED_ERROR:
            DBENGINE->pushError( new KDB::InvalidLogin(mysql_error(connection())));
            ret = false;
            break;
        case CR_IPSOCK_ERROR:
        case CR_OUT_OF_MEMORY:
        case CR_SOCKET_CREATE_ERROR:
        case CR_VERSION_ERROR:
        case CR_NAMEDPIPEOPEN_ERROR:
            DBENGINE->pushError( new KDB::ServerError(mysql_error(connection())));
            ret = false;
            break;
        case CR_UNKNOWN_HOST:
            DBENGINE->pushError( new KDB::HostNotFound(mysql_error(connection())));
            ret = false;
            break;
        default:
            DBENGINE->pushError(new KDB::ServerError(mysql_error(connection())));
            ret = false;
            break;
        }
    }

    setConnected(ret);
    return ret;
}

void ConnectorImpl::close()
{
    kdDebug(20012) << "ConnectorImpl::close" << endl;
    if ( isConnected() ) {
        kdDebug(20012) << k_funcinfo << endl;
        conn = 0L;
        setConnected( false );
    }
}

KDB::Connector *
ConnectorImpl::clone()
{
    kdDebug(20012) << "ConnectorImpl::clone" << endl;
    ConnectorImpl * ci = new ConnectorImpl(*this);
    return ci;
}

QStringList
ConnectorImpl::databases()
{
    kdDebug(20012) << "ConnectorImpl::databases" << endl;
    KDB::RowList l = resultQuery("SHOW databases");
    QStringList lst;
    KDB::RowList::Iterator it;
    for( it = l.begin(); it != l.end(); ++it ) {
        lst << (*it)[0].asString();
    }
    return lst;
}

QStringList
ConnectorImpl::tables()
{
    kdDebug(20012) << "ConnectorImpl::tables" << endl;
    KDB::RowList l = resultQuery("SHOW tables");
    QStringList lst;
    KDB::RowList::Iterator it;
    for( it = l.begin(); it != l.end(); ++it ) {
        lst << (*it)[0].asString();
    }
    return lst;
}


KDB::RowList
ConnectorImpl::fields(const QString & tableName)
{
    kdDebug(20012) << "ConnectorImpl::fields" << " tableName=" << tableName << endl;

    //KDB::RowList l = resultQuery(QString("SHOW fields from %1").arg(tableName));

    HandlerImpl *h = static_cast<HandlerImpl *> (query(QString("SHOW fields from %1").arg(tableName)));
    KDB_ULONG nFields = h->count();
    KDB::RowList res;
    KDB::Row cur;
    KDB::Row r;

    for (KDB_ULONG i = 0; i < nFields; i++) {
        r.clear();
        cur = h->record(i);
        r << cur[0]; //name

        // datatype (native) 
        // let's normalize the type, since in show fields is reported an e.g. varchar(50)
        // and we need to pass VARCHAR to the NToK call.
        QString norm;
        QString type = cur[1].asString();
        int idx;
                
        kdDebug(20012) << "Start type = " << type << endl;

        if ((idx = type.find('(') ) != -1 )
            norm = type.left(idx).upper();
        else
            norm = type.upper();
        
        kdDebug(20012) << "Normalized type = " << norm << endl;

        r << Value(norm);
        
        res << r;
    }

    delete h;
    return res;

}

bool
ConnectorImpl::createDatabase(const QString & name)
{
    kdDebug(20012) << "ConnectorImpl::createDatabase" << " name=" << name << endl;
    QString sql = QString("create database %1").arg(name);
    execute(sql);

    if (DBENGINE->error())
        return false;

    return true;
}


KDB::RowList
ConnectorImpl::resultQuery(const QString &sql)
{
    kdDebug(20012) << "KDB::RowListConnectorImpl::resultQuery" << " sql=" << sql << endl;

    HandlerImpl *res = static_cast<HandlerImpl *> (query(sql));

    KDB::RowList l = res->rows();

    delete res;

    return l;
}

bool
ConnectorImpl::setCurrentDatabase(const QString &name)
{
    kdDebug(20012) << "ConnectorImpl::setCurrentDatabase" << " name=" << name << endl;
    // pretend to be closed, but don't close the connection
    // it may be used by other connectors
    if (isConnected())
        setConnected(false);

    connect(); // create a new connection
    if ( mysql_select_db(connection(), name.utf8()) != 0 ) {
        DBENGINE->pushError( new KDB::ServerError(mysql_error(connection())));
        return false;
    }
    return true;
}

KDB_ULONG
ConnectorImpl::execute(const QString &sql)
{
    kdDebug(20012) << "ConnectorImpl::execute" << " sql=" << sql << endl;
    int len = sql.length();

    if ( (mysql_real_query(connection(), sql.latin1(), len )) != 0 ) {
        //here test for different types of errors???
        DBENGINE->pushError( new KDB::ServerError(mysql_error(connection())) );
        return 0;
    }

    return mysql_affected_rows(connection());
}

KDB::Handler *
ConnectorImpl::query(const QString &SQL)
{
    kdDebug(20012) << "ConnectorImpl::query" << " SQL=" << SQL << endl;
    execute(SQL);
    if (DBENGINE->error()) {
        return 0L;
    }
    HandlerImpl *res = new HandlerImpl(mysql_store_result( connection() ));
    return res;
}

KDB::DataType
ConnectorImpl::nativeToKDB(const QString &type)
{    
    kdDebug(20012) << "KDB::DataTypeConnectorImpl::nativeToKDB" << " type=" << type << endl;
    return NToK(type);
}

KDB::DataType
ConnectorImpl::NToK(const QString &type)
{
    kdDebug(20012) << "KDB::DataType ConnectorImpl::NToK" << " type=" << type << endl;
    KDB::DataType ret = KDB::UNKNOWN;

    if (type == "CHAR")
        ret = KDB::CHAR;

    if (type == "TEXT")
        ret = KDB::BLOB;

    if (type == "VARCHAR")
        ret = KDB::VARCHAR;

    if (type == "SMALLINT")
        ret = KDB::SMALLINT;

    if (type == "INT")
        ret = KDB::INT;

    if (type == "BIGINT")
        ret = KDB::BIGINT;

    if (type == "FLOAT")
        ret = KDB::FLOAT;

    if (type == "DOUBLE")
        ret = KDB::DOUBLE;

    if (type == "DATE")
        ret = KDB::DATE;

    if (type == "TIME")
        ret = KDB::TIME;

    if (type == "TIMESTAMP")
        ret = KDB::TIMESTAMP;

    if (type == "SET")
        ret = KDB::SET;

    if (type == "ENUM")
        ret = KDB::ENUM;

    if (type == "BLOB")
        ret = KDB::BLOB;

    return ret;
}



QString
ConnectorImpl::KDBToNative(KDB::DataType type)
{
    kdDebug(20012) << "ConnectorImpl::KDBToNative" << endl;
    return KToN(type);
}

QString
ConnectorImpl::KToN(KDB::DataType type)
{
    kdDebug(20012) << "ConnectorImpl::KToN" << endl;
    QString ret = QString::null;

    switch (type) {
    case KDB::CHAR:
        ret = "CHAR";
        break;
    case KDB::VARCHAR:
        ret = "VARCHAR";
        break;
    case KDB::SMALLINT:
        ret = "SMALLINT";
        break;
    case KDB::INT:
        ret = "INT";
        break;
    case KDB::BIGINT:
        ret = "BIGINT";
        break;
    case KDB::FLOAT:
        ret = "FLOAT";
        break;
    case KDB::DOUBLE:
        ret = "DOUBLE";
        break;
    case KDB::DATE:
        ret = "DATE";
        break;
    case KDB::TIME:
        ret = "TIME";
        break;
    case KDB::TIMESTAMP:
        ret = "TIMESTAMP";
        break;
    case KDB::SET:
        ret = "SET";
        break;
    case KDB::ENUM:
        ret = "ENUM";
        break;
    case KDB::ARRAY:
        ret = "BLOB";
        break;
    case KDB::BLOB:
        ret = "BLOB";
        break;
    default:
        break;
    }
    return ret;
}


bool
ConnectorImpl::createTable(const KDB::Table &tab)
{
    kdDebug(20012) << "ConnectorImpl::createTable" << endl;
    QString sql = QString("Create table %1 (\n").arg(tab.name());

    KDB::FieldList f = tab.fields();
    KDB::FieldIterator itf(f);

    bool first = true;
    while (itf.current()) {
        if (!first) {
            sql += ",";
        }
        first = false;

        KDB::Field *field = itf.current();
        sql += QString("%1\t%2\t").arg(field->name()).arg(KToN(field->type()));

        if (field->size() > 0)
            sql += QString("(%1) ").arg(field->size());

        if (!field->acceptNull()) {
            sql += "NOT NULL";
        }
        sql += "\n";
        ++itf;
    }

    sql += ")\n";

    kdDebug(20012) << k_funcinfo << sql << endl;
    execute(sql);

    if (DBENGINE->error())
        return false;

    return true;
}

bool
ConnectorImpl::dropDatabase(const QString & name)
{
    kdDebug(20012) << "ConnectorImpl::dropDatabase" << " name=" << name << endl;
    execute(QString("drop database %1").arg(name));
    if (DBENGINE->error())
        return false;

    return true;
}

bool
ConnectorImpl::dropTable(const QString & name)
{
    kdDebug(20012) << "ConnectorImpl::dropTable" << " name=" << name << endl;
    execute(QString("drop table %1").arg(name));
    if (DBENGINE->error())
        return false;

    return true;
}

/*
  TINYINT
  SMALLINT
  MEDIUMINT
  INT
  INTEGER
  BIGINT
  FLOAT
  DOUBLE
  REAL
  DECIMAL
  NUMERIC
  DATE
  DATETIME
  TIMESTAMP
  TIME
  YEAR
  CHAR
  VARCHAR
  TINYBLOB
  TINYTEXT
  BLOB
  TEXT
  MEDIUMBLOB
  MEDIUMTEXT
  LONGBLOB
  LONGTEXT
  ENUM
  SET





  FIELD_TYPE_TINY
  FIELD_TYPE_SHORT
  FIELD_TYPE_LONG
  FIELD_TYPE_INT24
  FIELD_TYPE_LONGLONG
  FIELD_TYPE_DECIMAL
  FIELD_TYPE_FLOAT
  FIELD_TYPE_DOUBLE
  FIELD_TYPE_TIMESTAMP
  FIELD_TYPE_DATE
  FIELD_TYPE_TIME
  FIELD_TYPE_DATETIME
  FIELD_TYPE_YEAR
  FIELD_TYPE_STRING
  FIELD_TYPE_BLOB
  FIELD_TYPE_SET
  FIELD_TYPE_ENUM
  FIELD_TYPE_NULL
  FIELD_TYPE_CHAR

*/
