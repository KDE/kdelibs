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

#include <math.h>

#include <kdb/exception.h>
#include <kdb/dbengine.h>

HandlerImpl::HandlerImpl(MYSQL_RES * result)
    :res(result)
{
    //kdDebug(20012) << "HandlerImpl::HandlerImpl" << endl;

    m_fields = mysql_fetch_fields(res);
    numFields = mysql_num_fields(res);
}

HandlerImpl::~HandlerImpl()
{
    //kdDebug(20012) << "HandlerImpl::~HandlerImpl" << endl;
    mysql_free_result(res);
}


KDB_ULONG
HandlerImpl::count() const
{
    //kdDebug(20012) << "HandlerImpl::count" << endl;
    return mysql_num_rows(res);
}

KDB::Row
HandlerImpl::record(KDB_ULONG pos) const
{
    //kdDebug(20012) << "KDB::RowHandlerImpl::record" << endl;

    // TODO: transform it into a direct request to the backend if possible
    if (m_rows.isEmpty())
        rows(); //load the rows if not already done
    return m_rows[pos];
}

KDB::RowList
HandlerImpl::rows() const
{
    //kdDebug(20012) << "KDB::RowListHandlerImpl::rows" << endl;
    if ( m_rows.isEmpty() ) {
        MYSQL_ROW row;

        while ((row = mysql_fetch_row(res))) {
            KDB::Row f;
            unsigned long *len = mysql_fetch_lengths(res);
            MYSQL_FIELD *fld = mysql_fetch_fields(res);
            for (unsigned int i = 0; i < numFields; i++ ) {
                
                Value v(QString::fromLocal8Bit(row[i],len[i]));

                switch (fld[i].type) {
                case FIELD_TYPE_TINY:
                case FIELD_TYPE_SHORT:
                    v = QVariant(v.toString().toInt());
                    //kdDebug(20012) << "To int: "<< v.toInt() << endl;
                    break;
                case FIELD_TYPE_LONG:
                case FIELD_TYPE_INT24:
                    //kdDebug(20012) << "To Long: (string) "<< v.toString() << endl;
                    v.cast(Value::Long);
                    //kdDebug(20012) << "To Long: "<< v.toLong() << endl;
                case FIELD_TYPE_LONGLONG:
                    // ???
                    break;
                case FIELD_TYPE_DECIMAL:
                case FIELD_TYPE_DOUBLE:
                case FIELD_TYPE_FLOAT:
                    {
                        QStringList lst = QStringList::split(".",v.toString());
                        double d = lst[0].toDouble();
                        if (lst.count() > 1) {
                            d += (lst[1].toDouble() / pow(10,lst[1].length()));
                        }
                        v = QVariant(d);
                        break;
                    }
                case FIELD_TYPE_TIMESTAMP:
                    // must be rewritten
                    v.asDateTime();
                    break;
                case FIELD_TYPE_DATE:
                    {
                        QString str = v.toString();
                        QStringList lst = QStringList::split("-",str);
                        if (lst.count() == 3) { // date has correct format                            
                            int y = lst[0].toInt();
                            int m = lst[1].toInt();
                            int d = lst[2].toInt();
                            v = Value(QDate(y,m,d));
                        } else {
                            v = Value(QString());
                        }
                        break;
                    }
                case FIELD_TYPE_TIME:
                    // must be rewritten
                    v.asTime();
                    break;
                case FIELD_TYPE_DATETIME:
                    // must be rewritten
                    v.asDateTime();
                    break;
                case FIELD_TYPE_YEAR:
                    v.asInt();
                    break;
                case FIELD_TYPE_STRING:
                case FIELD_TYPE_BLOB:
                    break;
                case FIELD_TYPE_SET:
                    // must be rewritten
                    v.asStringList();
                    break;
                case FIELD_TYPE_ENUM:
                    // must be rewritten
                    v.asStringList();
                    break;
                default:
                    break;
                }
                
                f << v;
            }
            m_rows << f;
        }
    }
    return m_rows ;
}

QStringList
HandlerImpl::fields() const
{
    //kdDebug(20012) << "HandlerImpl::fields" << endl;
    QStringList f;

    for (unsigned int i = 0; i < numFields; i++) {
        f << m_fields[i].name;
    }
    return f;
}

QString
HandlerImpl::nativeType(const QString &fieldName) const
{
    //kdDebug(20012) << "HandlerImpl::nativeType" << " fieldName=" << fieldName << endl;
    unsigned int i = 0;
    for ( ; i < numFields; ++i) {
        if (fieldName == m_fields[i].name)
            break;
    }

    if (i == numFields) {
        DBENGINE->pushError( new KDB::ObjectNotFound(fieldName));
        return QString::null;
    }

    QString res;

    switch (m_fields[i].type) {
    case FIELD_TYPE_TINY:
        res = "TINYINT";
        break;
    case FIELD_TYPE_SHORT:
        res = "SMALLINT";
        break;
    case FIELD_TYPE_LONG:
        res = "INTEGER";
        break;
    case FIELD_TYPE_INT24:
        res = "MEDIUMINT";
        break;
    case FIELD_TYPE_LONGLONG:
        res = "BIGINT";
        break;
    case FIELD_TYPE_DECIMAL:
        res = "NUMERIC";
        break;
    case FIELD_TYPE_FLOAT:
        res = "FLOAT";
        break;
    case FIELD_TYPE_DOUBLE:
        res = "DOUBLE";
        break;
    case FIELD_TYPE_TIMESTAMP:
        res = "TIMESTAMP";
        break;
    case FIELD_TYPE_DATE:
        res = "DATE";
        break;
    case FIELD_TYPE_TIME:
        res = "TIME";
        break;
    case FIELD_TYPE_DATETIME:
        res = "DATETIME";
        break;
    case FIELD_TYPE_YEAR:
        res = "YEAR";
        break;
    case FIELD_TYPE_STRING:
        // which kind of string???
        res = "VARCHAR";
        break;
    case FIELD_TYPE_BLOB:
        res = "BLOB";
        break;
    case FIELD_TYPE_SET:
        res = "SET";
        break;
    case FIELD_TYPE_ENUM:
        res = "ENUM";
        break;
    default:
        break;
    }

    return res;
}

KDB::DataType
HandlerImpl::kdbDataType(const QString &fieldName) const
{
    //kdDebug(20012) << "KDB::DataTypeHandlerImpl::kdbDataType" << " fieldName=" << fieldName << endl;

    return ConnectorImpl::NToK(nativeType(fieldName));
}
