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
#ifndef KDB_TABLE_H
#define KDB_TABLE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>
#include <qlist.h>

#include "field.h"
#include "index.h"
#include "dataobject.h"

namespace KDB {

class Table;

typedef KSharedPtr<Table> TablePtr;
typedef QValueList<TablePtr> TableList;
typedef QValueListIterator<TablePtr> TableIterator;

class Recordset;
class Connector;

typedef KSharedPtr<Recordset> RecordsetPtr;

/**
 * Representation of a database table.
 *
 * Trough this object we access the fields and
 * indices of a given table (when available). This object will allow building of
 * new fields and indexes, as well as editing of the table data using
 * @ref openRecordset
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @version kdbcore 0.0.2
 */
class Table :public DataObject {

    friend class Database;

    Q_OBJECT

public:

    ~Table();

    /**
     * creates a new field for the current table.
     * If the table is not create()d, the field is appended
     * to the list of fields and created with the table itself.
     * else no modification will occur. To apply the changes to the
     * table, you must use @ref appendField
     */
    Field * newField(const QString &name);

    /**
     * appends the field to the list of fields and issue an
     * alter table to the underlying DBMS.
     */
    bool appendField(Field *);

    /**
     * appends the field to the list of fields and issue an
     * alter table to the underlying DBMS.
     */
    bool removeField(const QString &name);

    /**
     * changes a field definition and issue an
     * alter table to the underlying DBMS.
     * the field must be already present in the list of fields
     */
    bool changeField(Field *);

    /**
     * returns an existing field by name, or 0L if the
     * field does not exists
     */
    Field * getField(const QString &name) const;

    /**
     * return the list of available fields
     */
    FieldList fields() const;

    /**
     * return the list of names of available fields
     */
    QStringList fieldNames() const;

    /**
     * creates a new index
     *
     * @param name this is the name of the index
     */
    Index * newIndex(const QString &name);

    /**
     * return an index by name
     */
    Index * getIndex(const QString &name) const;

    /**
     * return the list of available indexes
     */
    IndexList indexes() const;

    /**
     * creates a recordset based on this table.
     */
    RecordsetPtr openRecordset();

    /**
     * actually create the table.
     * returns true if the table has been successfully created,
     * false otherwise.
     */
    bool create();

    /**
     * returns true if the table is in design mode, i.e. it
     * is not created
     */
    bool designMode() const;

 signals:

    void fieldAdded( KDB::Field * );
    void fieldRemoved( KDB::Field * );
    void changed();
    void indexAdded( KDB::Index * );
    void indexRemoved( KDB::Index * );
    void created( KDB::Table * );

 private:

    Table(Connector *conn, QObject *parent = 0L, const char *name = 0L, bool design = false);

    Connector * connector;
    FieldList m_fields;
    IndexList m_indexes;

    bool m_design;

};

}

#endif
