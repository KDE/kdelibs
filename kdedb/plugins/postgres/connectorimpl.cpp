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

#include <kconfig.h>
#include <kdebug.h>

#include <kdb/kdb.h>
#include <kdb/table.h>
#include <kdb/dbengine.h>
#include <kdb/exception.h>
#include <kdb/value.h>

const Q_INT32 VARHDR = 4;

ConnectorImpl::ConnectorImpl()
{
    m_typeCache.setAutoDelete(true);
}

ConnectorImpl::ConnectorImpl(const ConnectorImpl &c)
    : Connector(c),
      conn(c.conn)
{
    setHost( c.host() );
    setPort( c.port() );
    setUser( c.user() );
    setPassword( c.password() );
    setConnected( c.isConnected() );
    m_database = c.m_database; // keep always the same database

    m_typeCache.setAutoDelete(true);

}

ConnectorImpl::~ConnectorImpl()
{
}

bool ConnectorImpl::connect()
{
    QString connString = QString("host=%1 user=%2 password=%3");
    
    connString = connString.arg(host()).arg(user()).arg(password());

    if (port() != 0)
        connString += QString(" port=%1").arg(port());

    if (m_database.isEmpty()) {
        // we do not have a database name to connect to
        // grab the default database from the config file, if it exists
        KConfigBase *conf = DBENGINE->config();
        KConfigGroupSaver(conf, "KDBPostgresPlugin");
        conf->setGroup("KDBPostgresPlugin");
        
        int numHost = conf->readNumEntry("NumHosts",0);
        
        QString db;
        while (numHost--) {
            QString prefix = QString("%1_").arg(numHost);
            QString h = conf->readEntry(prefix + "host");
            if (h == host()) { 
                db = conf->readEntry(prefix + "db");
                break;
            }
        }
        
        if (!db.isEmpty())
            connString += QString(" dbname=%1").arg(db);
        else // try using the default 'template1', works 99%
            connString += QString(" dbname=template1");
    } else {
        connString += QString(" dbname=%1").arg(m_database);
    }
    
    connRep *c = new connRep;
    c->conn = PQconnectdb(connString.ascii());                               
    conn = c;
    
    if (PQstatus(connection()) == CONNECTION_BAD) {
        DBENGINE->pushError(new KDB::InvalidLogin(this, PQerrorMessage(connection())));
        conn = 0L;
        setConnected(false);
        return false;
    } else {
        setConnected(true);
        return true;
    }
}

void ConnectorImpl::close()
{
    conn = 0L;
}

KDB::Connector *
ConnectorImpl::clone()
{
    return new ConnectorImpl(*this);
}

QStringList
ConnectorImpl::databases()
{
    KDB::RowList l = resultQuery("select datname from pg_database");

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
    KDB::RowList l = resultQuery("select tablename from pg_tables where tablename !~'^pg_'");

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
    QString sql = "Select a.attname, ";
    sql += "t.typname, ";
    sql += "t.typlen, ";
    sql += "a.atttypmod, ";
    sql += "a.attnotnull, ";
    sql += "a.atthasdef, ";
    sql += "a.attnum ";
    sql += "FROM pg_class c, pg_attribute a, pg_type t ";
    sql += QString("WHERE c.relname = '%1'").arg(tableName);
    sql += " AND a.attnum > 0 ";
    sql += " AND a.attrelid = c.oid";
    sql += " AND a.atttypid = t.oid";
    sql += " ORDER BY a.attnum";

    KDB::RowList lst = resultQuery(sql);

    KDB::RowList res;

    for (KDB::RowList::Iterator i = lst.begin();i != lst.end(); i++) {
        KDB::Row orig = *i;

        KDB::Row row;
        row << orig[0]; //name
        row << orig[1]; //type

        // decode typname e attypmod
        if (orig[2].toInt() == -1 ) {
            // calculate the length based on attypmod
            Q_INT32 len = orig[3].toInt();
            QString colTyp = orig[1].toString();
            if (colTyp == "bpchar" || colTyp == "varchar") {
                row << Value(len - VARHDR);
                row << Value(0);
            } else if (colTyp == "numeric" ) {
                int size = ((len - VARHDR) >> 16) & 0xffff;
                int prec = (len - VARHDR) & 0xffff;
                row << Value(size);
                row << Value(prec);
            } else if (colTyp.startsWith("_") ) {
                row << Value(len + 1);
                row << Value(0);
            } else { // dunno! set it to 0
                row << Value(0);
                row << Value(0);
            }
        } else {
            // we already have the size here
            row << orig[2];
            row << Value(0);
        }
    
        if (orig[4].toString() == "f")
            row << Value(QVariant("Y"));
        else
            row << Value(QVariant("N"));

        //TODO: these aren't mandatory, but useful
        // load default value

        // load comment
        res << row;
    }
    return res;
}

bool
ConnectorImpl::createDatabase(const QString & name)
{
    QString sql = QString("Create database %1").arg(name);
    execute(sql);
    return !DBENGINE->error();
}

KDB::RowList
ConnectorImpl::resultQuery(const QString &sql)
{
    KDB::Handler *res = query(sql);
    KDB::RowList l;
    if (res) {
        l = res->rows();
        delete res;
    }
    
    return l;    
}

bool
ConnectorImpl::setCurrentDatabase(const QString &name)
{
    m_database = name;
    return connect();
}

KDB_ULONG
ConnectorImpl::execute(const QString &sql)
{
    PGresult * res = PQexec(connection(),sql.latin1());
    if (!res){
        DBENGINE->pushError(new KDB::ServerError(this, PQerrorMessage(connection())));
        return 0L;
    }
    
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        if (PQresultStatus(res) == PGRES_TUPLES_OK)
            DBENGINE->pushError(new KDB::SQLError(this, "not a command query!"));
        else
            DBENGINE->pushError(new KDB::SQLError(this, PQresultErrorMessage(res)));
        return 0;
    }

    QString nTuples = QString("%1").arg(PQcmdTuples(res));
    PQclear(res);
    return nTuples.toULong();
    
}

KDB::Handler *
ConnectorImpl::query(const QString &SQL)
{
    PGresult * res = PQexec(connection(),SQL.latin1());
    if (!res){
        DBENGINE->pushError(new KDB::ServerError(this, PQerrorMessage(connection())));
        return 0L;
    }
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        if (PQresultStatus(res) == PGRES_COMMAND_OK)
            DBENGINE->pushError(new KDB::SQLError(this, "not a select query!"));
        else
            DBENGINE->pushError(new KDB::SQLError(this, PQresultErrorMessage(res)));
        return 0;
    }

    return new HandlerImpl(res,this);

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
    return false;
}

bool
ConnectorImpl::dropDatabase(const QString & name)
{
    QString sql = QString("Drop database %1").arg(name);
    execute(sql);
    return !DBENGINE->error();
}

bool
ConnectorImpl::dropTable(const QString & name)
{
    QString sql = QString("Drop table %1").arg(name);
    execute(sql);
    return !DBENGINE->error();
}

QString
ConnectorImpl::oidToTypeName(Oid oid)
{
    //    kdDebug(20012) << k_funcinfo << oid << endl;
    
    QString *s = m_typeCache.find((long int)oid);
    if (s) {
        //    kdDebug(20012) << "Found in cache " << *s << endl;
        return *s;
    } else {
        QString sql = QString("Select typname from pg_type where oid = %1").arg((unsigned int)oid);

        // we must do everything here to avoid recursive calls from HandlerImpl
        PGresult * res = PQexec(connection(),sql.latin1());
        if (!res){
            DBENGINE->pushError(new KDB::ServerError(this, PQerrorMessage(connection())));
            return QString::null;
        }
        
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            DBENGINE->pushError(new KDB::SQLError(this, PQresultErrorMessage(res)));
            return QString::null;
        }

        QString *newOid = new QString(QString::fromLocal8Bit(PQgetvalue(res, 0, 0),
                                                             PQgetlength(res, 0, 0) + 1));
        //kdDebug(20012) << "Loaded from pg_type " << *newOid << endl;

        if (m_typeCache.insert((long int)oid, newOid)) {
            return *newOid;
        } else {
            delete newOid;
            return QString::null;
        }
    }

}


void 
ConnectorImpl::beginTransaction()
{
    execute("BEGIN TRANSACTION");
}

void 
ConnectorImpl::commit()
{
    execute("COMMIT");
}

void 
ConnectorImpl::rollback() 
{
    execute("ROLLBACK");
}
