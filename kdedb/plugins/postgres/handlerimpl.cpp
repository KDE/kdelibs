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
#include "handlerimpl.h"
#include "connectorimpl.h"
#include <kdb/exception.h>
#include <kdb/dbengine.h>

HandlerImpl::HandlerImpl(PGresult *result)
    :res(result)
{
    kdDebug(20012) << "HandlerImpl::HandlerImpl" << endl;
    numFields = PQnfields(res);
}

HandlerImpl::~HandlerImpl()
{
    kdDebug(20012) << "HandlerImpl::HandlerImpl" << endl;
    PQclear(res);
}


KDB_ULONG
HandlerImpl::count()
{
    kdDebug(20012) << "HandlerImpl::count" << endl;
    return PQnfields(res);
}

KDB::Row
HandlerImpl::record(KDB_ULONG pos)
{
    kdDebug(20012) << "KDB::RowHandlerImpl::record" << endl;
    if (m_rows.isEmpty())
        rows();
    return m_rows[pos];
}

KDB::RowList
HandlerImpl::rows()
{
    kdDebug(20012) << "KDB::RowHandlerImpl::rows" << endl;
    if (m_rows.isEmpty()) {
        for (int i = 0; i < PQntuples(res); i++)
            {
                KDB::Row f;
                for (int j = 0; j < numFields; j++)
                    {
                        Value v(QString::fromLocal8Bit(PQgetvalue(res, i, j),PQgetlength(res, i, j) + 1));
                        f << v;
                    }
                m_rows << f;
            }
    }
    return m_rows;
}

QStringList
HandlerImpl::fields()
{
    
    kdDebug(20012) << "HandlerImpl::fields" << endl;
    QStringList f;
    for (int i = 0; i < numFields; i++) 
        f << PQfname(res, i);
    return f;
}

QString
HandlerImpl::nativeType(const QString &fieldName)
{
}

KDB::DataType
HandlerImpl::kdbDataType(const QString &fieldName)
{
}
