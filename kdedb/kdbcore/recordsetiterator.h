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
#ifndef RECORDSETITERATOR_H
#define RECORDSETITERATOR_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "object.h"
#include "record.h"
#include "handler.h"
#include "field.h"
#include "value.h"

namespace KDB {

class Recordset;

/**
 * This is an iterator over a recordset.
 *
 * It can be used to access all records of a recordset
 * either sequentialy or randomly.
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @author Michael Koch  <m_kock@bigfoot.de>
 * @version KDB 0.0.2
 */
class RecordsetIterator: public Object {

    Q_OBJECT

 public:
    RecordsetIterator();
    RecordsetIterator( Recordset *rec, HandlerPtr handler);
    RecordsetIterator( const RecordsetIterator &iter);

    ~RecordsetIterator();

    RecordsetIterator& operator =(const RecordsetIterator &iter);
    RecordPtr operator ->() const;
    RecordPtr current() const;
    //    RecordPtr operator*() const;

    RecordPtr operator ++(int);
    RecordPtr operator ++();
    RecordPtr operator --(int);
    RecordPtr operator --();

    RecordPtr findFirst(const QString &field, const Value &val);
    RecordPtr findNext(const QString &field, const Value &val);
    RecordPtr findPrevious(const QString &field, const Value &val); 
    RecordPtr findLast(const QString &field, const Value &val);
    void moveFirst();
    void moveLast();
    void moveTo(KDB_ULONG pos);

 protected slots:
    void recordsetChanged();

 private:

    void initFields();

    FieldList m_fields;
    HandlerPtr m_handler;
    Recordset *m_rset;
    KDB_ULONG m_pos;
    bool m_bof;
    
};
 
} 
#endif

