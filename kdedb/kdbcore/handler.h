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

#ifndef KDB_HANDLER_H
#define KDB_HANDLER_H "$Id$"

#include <ksharedptr.h>

#include <kdb/connector.h>

namespace KDB {

/**
  * Will contain the handler to a set of records.
  */
class Handler : public Object, public KShared {

public:
    virtual ~Handler() {};

    virtual KDB_ULONG count() const = 0;
    virtual Row record(KDB_ULONG pos) const = 0;
    virtual RowList rows() const = 0;

    virtual QStringList fields() const = 0;

    virtual bool append(Row row) = 0;
    virtual bool update(KDB_ULONG pos, Row row) = 0;
    virtual bool remove(KDB_ULONG pos, Row row) = 0;
    
    virtual QString nativeType(const QString &fieldName) const = 0;
    virtual DataType kdbDataType(const QString &fieldName) const = 0;

};

typedef KSharedPtr<Handler> HandlerPtr;
}

#endif
