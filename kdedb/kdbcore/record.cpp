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
#include "record.h"
#include "field.h"
#include "recordset.h"
#include "handler.h"

#include <kdebug.h>

#include "record.moc"

using namespace KDB;

Record::Record(Recordset *r, FieldList fields, Row values, KDB_ULONG pos )
    : DataObject( r, "Record"), m_fields(fields), m_absPos(pos)
{
    //kdDebug(20000) << "Record::Record" << endl;
    if (values.count() == 0) {
        m_new = true;
    } else {
        m_new = false;
        FieldIterator it = begin();
        Row::Iterator itv = values.begin();
        
        while (it.current() ) {
            it.current()->setValue(*itv,true);
            ++it;
            ++itv;
        }
    }
}

Record::Record(const Record &r)
    : DataObject(r.parent(), "Record"), m_fields(r.m_fields), m_absPos(r.m_absPos)
{
}

Record::~Record()
{
    //kdDebug(20000) << "Record::~Record" << endl;
}

FieldIterator 
Record::begin() 
{
    FieldIterator it(m_fields);
    return it;
}

Field & 
Record::operator[](const QString& name) 
{
    return field(name);
}

Field & 
Record::operator[](int index) 
{
    return *(m_fields.at(index));
}

Field &
Record::field(int index)
{
    return *(m_fields.at(index));
}

Field &
Record::field(const QString &name)
{
    FieldIterator it = begin();

    while ( it.current() ) {
        if (it.current()->name() == name)
            break;
        ++it;
    }

    if (!it.current()) {
        pushError(new ObjectNotFound(name));
    }
    
    return *(it.current());

}

void
Record::update()
{
    kdDebug(20000) << "Record::update" << endl;

    // emit updated if at least one field has changed
    FieldIterator it = begin();
    while ( it.current() ) {
        if (it.current()->changed()) {
            kdDebug(20000) << "changed !" << endl;
            emit updated(this, m_new);
            m_new = false;
            break;
        }
        ++it;
    }
}

Record &
Record::operator =(const Record &r)
{
    m_fields = r.m_fields;
    return *this;
}
    

KDB_ULONG
Record::absolutePosition()
{
    return m_absPos;
}

void
Record::remove()
{
    emit deleted(this);
}
