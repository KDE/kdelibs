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
#ifndef RECORD_H
#define RECORD_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qlist.h>
#include <qstring.h>

#include "kdb.h"
#include "field.h"
#include "dataobject.h"
#include "handler.h"

namespace KDB {

class Recordset;
class Record;
class Connector;
class Handler;

typedef KSharedPtr<Record> RecordPtr;

/**
 * A collection of fields with relevant values.
 *
 * The fields should be accessed like an associative array:
 *
 * <pre>
 * 	rec["field1"] = "value";
 * or
 * 	rec[1] = "value";
 * </pre>
 *
 * there are two record types: updatable and read only, depending on the
 * parent recordset
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @author Michael Koch  <m_kock@bigfoot.de>
 * @version kdbcore 0.0.2
 */
class Record : public DataObject {

    Q_OBJECT

 public:
    Record(Recordset *r, FieldList fields, Row values, KDB_ULONG pos);
    Record(const Record &);

    Record & operator = (const Record &);
    
    ~Record();

    FieldIterator begin();

    Field & operator[](const QString& name);
    Field & operator[](int index);

    Field & field(const QString & name);
    Field & field(int index) ;

    KDB_ULONG absolutePosition();

    void update();
    void remove();

 signals:

    void updated(KDB::Record *, bool);
    void deleted(KDB::Record *);
    
 private:
    FieldList m_fields ;
    bool m_new;
    KDB_ULONG m_absPos;
};
 
}

#endif


