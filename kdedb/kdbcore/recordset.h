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
#ifndef RECORDSET_H
#define RECORDSET_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>
#include <qlist.h>

#include "dataobject.h"
#include "field.h"
#include "record.h"
#include "recordsetiterator.h"
#include "handler.h"

namespace KDB {

class Connector;

class Recordset;

typedef KSharedPtr<Recordset> RecordsetPtr;

/**
 * A set of records, that can be traversed through iterators.
 *
 * A recordset can be obtained directly by the database (through
 * @ref KDB::Database::openRecordset ("Select * from blah") ) or from an
 * existing table or query.
 *
 * There are two types of recordsets: read only or updatable. the type can
 * be queried through @ref updatable
 *
 * an updatable recordset can be asked to build new records, as well as delete
 * existing ones. Updates are allowed in the @ref KDB::Record class
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @author Michael Koch  <m_kock@bigfoot.de>
 * @version kdbcore 0.0.2
 */
class Recordset : public DataObject {
    friend class Table;
    friend class Query;
    friend class Database;

    Q_OBJECT

 public:

    ~Recordset();

    /**
     * returns an iterator that points to the first record of the
     * recordset.
     */
    RecordsetIterator begin(); 

    /**
     * returns the list of all fields for the current recordset.
     */
    FieldList fields();

    /**
     * returns true if the recordset is updatable, i.e. addRecord can be
     * called and Record s can be modified and updated
     */
    bool updatable();

    /**
     * creates a new record. The programmer must call Record::update to actually
     * insert the record in the database.
     */
    RecordPtr addRecord();

    /**
     * reloads all data from the database.
     */
    void requery();

    /**
     * returns the amount of records in the current database
     */
    KDB_ULONG count();
    
 signals:
    void recordAdded( KDB::RecordPtr );
    void recordRemoved( KDB::RecordPtr );
    void recordUpdated( KDB::RecordPtr );
    void changed();
    
 private:

    Connector *connector;
    Recordset(Connector * conn, const QString &baseQuery, QObject *parent = 0);

    QString m_sql;
    FieldList m_fields;

    HandlerPtr m_handle;
    bool m_changed;
    
};
 
}

#endif




