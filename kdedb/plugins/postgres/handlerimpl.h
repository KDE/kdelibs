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

#ifndef PGSQL_HANDLERIMPL_H
#define PGSQL_HANDLERIMPL_H "$Id$"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libpq-fe.h>
#include <kdb/handler.h>

class ConnectorImpl;

class HandlerImpl: public KDB::Handler {

public:
    HandlerImpl(PGresult *result, ConnectorImpl *conn);
    virtual ~HandlerImpl();

    KDB_ULONG count() const;
    KDB::Row record(KDB_ULONG pos) const;
    KDB::RowList rows() const;
    QStringList fields() const; 
    QString nativeType(const QString &fieldName) const;
    KDB::DataType kdbDataType(const QString &fieldName) const ;

    bool append(KDB::Row row);
    bool update(KDB_ULONG pos, KDB::Row row);
    bool remove(KDB_ULONG pos, KDB::Row row);

private:

    PGresult *res;
    int numFields;

    ConnectorImpl *m_conn;
    mutable KDB::RowList m_rows;

};

#endif
