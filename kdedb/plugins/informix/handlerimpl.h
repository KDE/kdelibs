/*
   This file is part of the KDB libraries
   Copyright (c) 2000 Scott Newton <sknewton@ihug.co.nz>
 
   Code based on DBD::Informix:
   Portions Copyright 1994-1995 Tim Bunce
   Portions Copyright 1995-1996 Alligator Descartes
   Portions Copyright 1994      Bill Hailes
   Portions Copyright 1996      Terry Nightingale
   Portions Copyright 1996-1999 Jonathan Leffler
   Portions Copyright 2000	    Informix Software Inc.

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

#ifndef HANDLERIMPL_H
#define HANDLERIMPL_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ksharedptr.h>

#include <kdb/handler.h>

#include "informiximpl.h"
#include "connectorimpl.h"


class HandlerImpl: public KDB::Handler {

public:
    HandlerImpl( InformixImpl * ptr );
    virtual ~HandlerImpl();

    KDB_ULONG count() const;
    KDB::Row record( KDB_ULONG pos ) const;
    KDB::RowList rows() const;
    QStringList fields() const;
    QString nativeType( const QString & fieldName ) const;
    KDB::DataType kdbDataType( const QString & fieldName ) const;

private:
    unsigned int numFields;
    mutable KDB::RowList m_rows;
	InformixImpl * ifmx_hdl;

    //MYSQL_RES *res;
    //MYSQL_FIELD *m_fields;
};

#endif

