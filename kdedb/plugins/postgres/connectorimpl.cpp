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
	setHost(c.host());
	setPort(c.port());
	setUser(c.user());
	setPassword (c.password());
	setConnected(c.isConnected());
	m_database = c.m_database; // keep always the same database

	m_typeCache.setAutoDelete(true);
}

ConnectorImpl::~ConnectorImpl()
{
}

bool ConnectorImpl::connect()
{
	QString connString(QString::fromLatin1("host=%1 user=%2 password=%3"));
    
	connString = connString.arg(host()).arg(user()).arg(password());

	if (port() != 0)
		connString += QString::fromLatin1(" port=%1").arg(port());

	if (m_database.isEmpty()) {
		// we do not have a database name to connect to
		// grab the default database from the config file, if it exists
		KConfigBase *conf = DBENGINE->config();
		KConfigGroupSaver(conf, "KDBPostgresPlugin");
		conf->setGroup("KDBPostgresPlugin");

		int numHost = conf->readNumEntry("NumHosts", 0);

		QString db;
		while (numHost--) {
			QString prefix(QString::fromLatin1("%1_").arg(numHost));
			QString h = conf->readEntry(prefix + "host");
			if (h == host()) { 
				db = conf->readEntry(prefix + "db");
				break;
			}
		}

		if (!db.isEmpty())
			connString += QString::fromLatin1(" dbname=%1").arg(db);
		else // try using the default 'template1', works 99%
			connString += QString::fromLatin1(" dbname=template1");
	} else {
		connString += QString::fromLatin1(" dbname=%1").arg(m_database);
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
	//PQfinish(conn->conn);
	conn = 0L;
}

KDB::Connector *ConnectorImpl::clone()
{
	return new ConnectorImpl(*this);
}

QStringList ConnectorImpl::databases()
{
	KDB::RowList l = resultQuery(QString::fromLatin1("SELECT datname FROM pg_database"));

	QStringList lst;
	KDB::RowList::Iterator it;
	for (it = l.begin(); it != l.end(); ++it) {
		lst << (*it)[0].asString();
	}
	return lst;
}

QStringList ConnectorImpl::tables()
{
	KDB::RowList l = resultQuery(QString::fromLatin1("SELECT tablename FROM pg_tables WHERE tablename !~'^pg_'"));

	QStringList lst;
	KDB::RowList::Iterator it;
	for (it = l.begin(); it != l.end(); ++it) {
		lst << (*it)[0].asString();
	}

	return lst;
}

KDB::RowList ConnectorImpl::fields(const QString &tableName)
{
	QString sql(QString::fromLatin1("SELECT a.attname, t.typname, t.typlen, a.atttypmod, a.attnotnull, a.atthasdef, a.attnum FROM pg_class c, pg_attribute a, pg_type t %1 AND a.attnum > 0 AND a.attrelid = c.oid AND a.atttypid = t.oid ORDER BY a.attnum"));
	sql = sql.arg(QString::fromLatin1("WHERE c.relname = '%1'").arg(tableName));

	KDB::RowList lst = resultQuery(sql);

	KDB::RowList res;

	for (KDB::RowList::Iterator i = lst.begin(); i != lst.end(); i++) {
		KDB::Row row, orig = *i;

		row << orig[0]; //name
		row << orig[1]; //type

		// decode typname e attypmod
		if (orig[2].toInt() == -1) {
			// calculate the length based on attypmod
			Q_INT32 len = orig[3].toInt();
			QString colTyp = orig[1].toString();
			if (colTyp == "bpchar" || colTyp == "varchar") {
				row << Value(len - VARHDR);
				row << Value(0);
			} else if (colTyp == "numeric") {
				int size = ((len - VARHDR) >> 16) & 0xffff;
				int prec = (len - VARHDR) & 0xffff;
				row << Value(size);
				row << Value(prec);
			} else if (colTyp.startsWith("_")) {
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

bool ConnectorImpl::createDatabase(const QString &name)
{
	QString sql(QString::fromLatin1("CREATE DATABASE %1").arg(name));
	execute(sql);
	return (!DBENGINE->error());
}

KDB::RowList ConnectorImpl::resultQuery(const QString &sql)
{
	KDB::Handler *res = query(sql);
	KDB::RowList l;
	if (res) {
		l = res->rows();
		delete res;
	}

	return l;
}

bool ConnectorImpl::setCurrentDatabase(const QString &name)
{
	m_database = name;
	return connect();
}

KDB_ULONG ConnectorImpl::execute(const QString &sql)
{
	PGresult *res = PQexec(connection(), sql.latin1());

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

	QString nTuples(QString::fromLatin1("%1").arg(PQcmdTuples(res)));
	PQclear(res);
	return nTuples.toULong();
}

KDB::Handler *ConnectorImpl::query(const QString &sql)
{
	PGresult *res = PQexec(connection(), sql.latin1());
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

	return new HandlerImpl(res, this);
}

KDB::DataType ConnectorImpl::nativeToKDB(const QString &t)
{
	return _nativeToKDB(t);
}

KDB::DataType ConnectorImpl::_nativeToKDB(const QString &t)
{
	if (t == "CHAR")
		return KDB::CHAR;
	if (t == "VARCHAR")
		return KDB::VARCHAR;
	if (t == "INT2")
		return KDB::SMALLINT;
	if (t == "INT4")
		return KDB::INT;
	if (t == "INT8")
		return KDB::BIGINT;
	if (t == "FLOAT4")
		return KDB::FLOAT;
	if (t == "FLOAT8")
		return KDB::DOUBLE;
	if (t == "DATE")
		return KDB::DATE;
	if (t == "TIME")
		return KDB::TIME;
	if (t == "TIMESTAMP")
		return KDB::TIMESTAMP;

	// Everything else.. we need a KDB::ERROR type.
	return KDB::CHAR;
}

QString ConnectorImpl::KDBToNative(KDB::DataType type)
{
	return _KDBToNative(type);
}

QString ConnectorImpl::_KDBToNative(KDB::DataType type)
{
	QString ret;
	switch (type) {
	case KDB::CHAR:
		ret = "CHAR";
		break;
	case KDB::VARCHAR:
		ret = "VARCHAR";
		break;
	case KDB::SMALLINT:
		ret = "INT2";
		break;
	case KDB::INT:
		ret = "INT4";
		break;
	case KDB::BIGINT:
		ret = "INT8";
		break;
	case KDB::FLOAT:
		ret = "FLOAT4"; // How do we check the precision?!
		break;
	case KDB::DOUBLE:
		ret = "FLOAT8"; // Maybe this is how?
	case KDB::DATE:
		ret = "DATE";
		break;
	case KDB::TIME:
		ret = "TIME";
		break;
	case KDB::TIMESTAMP:
		ret = "TIMESTAMP";
		break;
	// The following are all unimplemented
	case KDB::SET:
	case KDB::ENUM:
	case KDB::ARRAY:
	case KDB::BLOB:
	default:
		ret = "UHOHFIXMEPLEASE";
		break;
	}
	return ret;
}

QString constructTypeDef(KDB::Field *f)
{
	QString ret = ConnectorImpl::_KDBToNative(f->type());

	if (f->size()) {
		ret += QString::fromLatin1("(%1");
		ret = ret.arg(f->size());
		if (f->precision()) {
			ret += QString::fromLatin1(",%1");
			ret = ret.arg(f->precision());
		}
		ret += QString::fromLatin1(")");
	}

	if (!f->acceptNull()) {
		ret += QString::fromLatin1(" NOT NULL");
	}

	return ret;
}

bool ConnectorImpl::createTable(const KDB::Table &t)
{
	QString cmd(QString::fromLatin1("CREATE TABLE %1 ("));
	cmd = cmd.arg(t.name());

	KDB::FieldList fl = t.fields();
	KDB::FieldIterator fit(fl);

	KDB::Field *f;
	QString s_field;

	fit.toFirst();
	for (fit.toFirst(); fit.current(); ++fit) {
		f = fit.current();
		s_field = QString::fromLatin1("%1 %2, ");
		cmd = cmd.arg(f->name()).arg(constructTypeDef(f));
	}

	cmd = cmd.mid(0, cmd.length()-2);
	cmd += ")";

	execute(cmd);
	return (!DBENGINE->error());
}

bool ConnectorImpl::dropDatabase(const QString &name)
{
	QString sql(QString::fromLatin1("DROP DATABASE %1").arg(name));
	execute(sql);
	return (!DBENGINE->error());
}

bool ConnectorImpl::dropTable(const QString &name)
{
	QString sql(QString::fromLatin1("DROP TABLE %1").arg(name));
	execute(sql);
	return (!DBENGINE->error());
}

QString ConnectorImpl::oidToTypeName(Oid oid)
{
	QString *s = m_typeCache.find((long int)oid);
	if (s) {
		//    kdDebug(20012) << "Found in cache " << *s << endl;
		return *s;
	} else {
		QString sql(QString::fromLatin1("SELECT typname FROM pg_type WHERE oid = %1").arg((unsigned int)oid));

		// we must do everything here to avoid recursive calls from HandlerImpl
		PGresult *res = PQexec(connection(), sql.latin1());
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

		if (m_typeCache.insert(static_cast<long int>(oid), newOid)) {
			return *newOid;
		} else {
			delete newOid;
			return QString::null;
		}
	}
}

void ConnectorImpl::beginTransaction()
{
	execute(QString::fromLatin1("BEGIN TRANSACTION"));
}

void ConnectorImpl::commit()
{
	execute(QString::fromLatin1("COMMIT"));
}

void ConnectorImpl::rollback() 
{
	execute(QString::fromLatin1("ROLLBACK"));
}

bool ConnectorImpl::appendField(const QString &table, KDB::Field *f)
{
	QString cmd(QString::fromLatin1("ALTER TABLE %1 ADD COLUMN %2 %3"));
	cmd = cmd.arg(table).arg(f->name()).arg(f->type());

	execute(cmd);
	return (!DBENGINE->error());
}

bool ConnectorImpl::removeField(const QString &/*table*/, const QString &/*field*/)
{
	// With postgresql right now you need to copy the table, and then recreate it without said field.  Yikes.  Soon.
	return false;
}

bool ConnectorImpl::changeField(const QString &/*table*/, KDB::Field */*field*/)
{
	// Not quite sure what is wanted here anyways.  Change field how?
	return false;
}
