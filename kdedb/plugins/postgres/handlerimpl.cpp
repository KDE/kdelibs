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

#include <kdb/exception.h>
#include <kdb/dbengine.h>

#include "handlerimpl.h"
#include "connectorimpl.h"

HandlerImpl::HandlerImpl(PGresult *result, ConnectorImpl *conn)
	:res(result),
	 m_conn(conn)
{
	//    kdDebug(20012) << "HandlerImpl::HandlerImpl" << endl;
	numFields = PQnfields(res);
}

HandlerImpl::~HandlerImpl()
{
	//    kdDebug(20012) << "HandlerImpl::HandlerImpl" << endl;
	PQclear(res);
}


KDB_ULONG HandlerImpl::count() const
{
	//    kdDebug(20012) << "HandlerImpl::count" << endl;
	return PQntuples(res);
}

KDB::Row HandlerImpl::record(KDB_ULONG pos) const
{
	//    kdDebug(20012) << "KDB::RowHandlerImpl::record" << endl;
	if (m_rows.isEmpty())
		rows();
	return m_rows[pos];
}

KDB::RowList HandlerImpl::rows() const
{
	if (m_rows.isEmpty()) {
		for (int i = 0; i < PQntuples(res); i++) {
			KDB::Row f;
			for (int j = 0; j < numFields; j++) {
				Value v;
				QString type = m_conn->oidToTypeName(PQftype(res,j)); 
				if (type == "bpchar" || type == "name") {
					v = Value(QString::fromLocal8Bit(PQgetvalue(res, i, j),
									 PQgetlength(res, i, j) + 1));
				} else if ( type == "int4" || type == "int2" ) {
					v = Value(QString::fromLocal8Bit(PQgetvalue(res, i, j), PQgetlength(res, i, j) + 1).toInt());
				} else {
					/*
					  v = QVariant(QByteArray().duplicate(PQgetvalue(res, i, j),
					  PQgetlength(res, i, j) + 1));
					*/
					// temporary. need to implement qbytearray too
					v = Value(QString::fromLocal8Bit(PQgetvalue(res, i, j), PQgetlength(res, i, j) + 1));
				}
				f << v;
			}
			m_rows << f;
		}
	}
	return m_rows;
}

QStringList HandlerImpl::fields() const
{
	//    kdDebug(20012) << "HandlerImpl::fields" << endl;
	QStringList f;
	for (int i = 0; i < numFields; i++) 
		f << PQfname(res, i);
	return f;
}

QString HandlerImpl::nativeType(const QString &fieldName) const
{
	QString type;
	for (int i = 0; i < numFields; i++) 
		if (PQfname(res, i) == fieldName)
			type = m_conn->oidToTypeName(PQftype(res,i));
	return type;
}

KDB::DataType HandlerImpl::kdbDataType(const QString &fieldName) const
{
	return KDB::UNKNOWN;
}

bool HandlerImpl::append(KDB::Row row)
{
	return false;
}

bool HandlerImpl::update(KDB_ULONG pos, KDB::Row row)
{
	return false;
}

bool HandlerImpl::remove(KDB_ULONG pos, KDB::Row row)
{
	return false;
}


