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
#include "recordset.h"
#include "record.h"
#include "field.h"
#include "connector.h"
#include "handler.h"

#include <kdebug.h>

#include "recordset.moc"

using namespace KDB;

Recordset::Recordset(Connector *conn, const QString &baseQuery, QObject *parent)
    : DataObject(parent, 0L), connector(conn->clone()), m_sql(baseQuery)
{
    m_fields.setAutoDelete(true);
    requery();
}

Recordset::~Recordset()
{
    delete connector;
}

RecordsetIterator
Recordset::begin()
{
    if (m_changed)
        requery();

    RecordsetIterator it(this, m_handle);
    return it;
}

FieldList
Recordset::fields()
{
    return m_fields;
}

bool
Recordset::updatable()
{
    return false;
}

RecordPtr
Recordset::addRecord()
{
    // TOOD: test if updatable
    // TODO: notification of update from the Record added
    Row r;
    RecordPtr ptr = new Record(this, m_fields, r);
    m_changed = true;
    emit changed();
    emit recordAdded(ptr);
    return ptr;
}

void
Recordset::requery()
{
    m_handle = connector->query(m_sql);
    m_fields.clear();

    // load fields
    QStringList fld = m_handle->fields();
    QStringList::Iterator it = fld.begin();
    
    while (it != fld.end() ) {
        m_fields.append(new Field(m_handle, (*it), this));
        ++it;
    }

    m_changed = false;
    emit changed();
}

KDB_ULONG
Recordset::count()
{
    if (!m_handle)
        return 0;
    return m_handle->count();
}







