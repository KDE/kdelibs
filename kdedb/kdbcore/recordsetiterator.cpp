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

#include "recordsetiterator.h"
#include "recordset.h"

#include <kdebug.h>

#include "recordsetiterator.moc"

using namespace KDB;

RecordsetIterator::RecordsetIterator()
    : Object(0L), m_handler(0), m_rset(0), m_pos(0), m_bof(false)
{
}

RecordsetIterator::RecordsetIterator(const RecordsetIterator & iter)
    : Object(0L), m_handler(iter.m_handler), m_rset(iter.m_rset), m_pos(iter.m_pos), m_bof(iter.m_bof)
{
    connect(m_rset, SIGNAL(changed()),SLOT(recordsetChanged()));
    initFields();
}

RecordsetIterator::RecordsetIterator(Recordset *rec, HandlerPtr handler)
    : Object(0L), m_handler(handler), m_rset(rec), m_pos(0), m_bof(false)
{
    connect(m_rset, SIGNAL(changed()),SLOT(recordsetChanged()));
    initFields();
}

RecordsetIterator::~RecordsetIterator()
{
    m_fields.setAutoDelete(true);
    m_fields.clear();
}


void
RecordsetIterator::initFields()
{
    QStringList fld = m_handler->fields();
    QStringList::Iterator it = fld.begin();

    while (it != fld.end() ) {
        m_fields.append(new Field(m_handler, (*it), this));
        ++it;
    }

}
void
RecordsetIterator::recordsetChanged()
{
    //kdDebug(20000) << "RecordsetIterator::recordsetChanged" << endl;
    // when added or deleted a record, nothing really changes

    //m_handler = 0L;
}

RecordsetIterator &
RecordsetIterator::operator =(const RecordsetIterator &iter)
{
    disconnect();
    m_pos = iter.m_pos;
    m_handler = iter.m_handler;
    m_rset = iter.m_rset;
    connect(m_rset, SIGNAL(changed()),SLOT(recordsetChanged()));

    return *this;
}


RecordPtr
RecordsetIterator::current() const
{

    if (m_bof)
        return 0L;
    
    if (m_pos >= m_handler->count())
        return 0L;
        //throw new InvalidRequest("Cannot advance past EOF");

    if (m_handler) {
        Record * rec = new Record(m_rset, m_fields, m_handler->record(m_pos), m_pos);
        connect(rec, SIGNAL(updated(KDB::Record *, bool)), m_rset, SLOT(slotRecordUpdated(KDB::Record *, bool)));
        connect(rec, SIGNAL(deleted(KDB::Record *)), m_rset, SLOT(slotRecordDeleted(KDB::Record *)));
        return RecordPtr(rec);
    } else {
        Object::pushError( new InvalidRequest("Iterator has been invalidated"));
        return 0L;
    }
}

RecordPtr
RecordsetIterator::operator ->() const
{
    return current();
}

/*
  RecordPtr
  RecordsetIterator::operator *() const // this is not correct! fix it, please
  {
  return current();
  }
*/

RecordPtr
RecordsetIterator::operator ++(int)
{
    m_bof = false;
    m_pos++;
    // EOF is not an error condition for an iterator!
    //if (m_pos == m_handler->count() )
    //    throw new InvalidRequest("Cannot advance past EOF");
    return current();

}

RecordPtr
RecordsetIterator::operator ++()
{
    m_bof = false;
    RecordPtr ret = current();
    m_pos++;
    return ret;
}

void
RecordsetIterator::moveFirst()
{
    m_bof = false;
    m_pos = 0;
}

void
RecordsetIterator::moveLast()
{
    m_bof = false;
    m_pos = m_handler->count() - 1;
}

void
RecordsetIterator::moveTo(KDB_ULONG pos)
{
    m_bof = false;
    if (pos > (m_handler->count() - 1)) {
        // EOF is not an error condition for an iterator!
        //throw new InvalidRequest("Cannot advance past EOF");
        // put the pointer at count() (one past EOF)
        pos = m_handler->count();
    }

    m_pos = pos;
}

RecordPtr
RecordsetIterator::operator --(int)
{
    if (m_pos == 0)
        m_bof = true;
    else
        m_pos--;
    // EOF is not an error condition for an iterator!
    //if (m_pos == m_handler->count() )
    //    throw new InvalidRequest("Cannot advance past EOF");
    return current();
}

RecordPtr
RecordsetIterator::operator --()
{
    RecordPtr ret = current();
    if (m_pos == 0)
        m_bof = true;
    else
        m_pos--;
    return ret;
}

RecordPtr
RecordsetIterator::findFirst(const QString &field, const QString &val)
{
    //kdDebug(20000) << "RecordsetIterator::findFirst" << " field=" << field << " val=" << val << endl;
    moveFirst();    
    RecordPtr p = current();
    do {
        if (val == p->field(field)->toString()) {
            break;
        }
    } while (p = operator++());

    return p;
}

RecordPtr
RecordsetIterator::findNext(const QString &field, const QString &val)
{
    //kdDebug(20000) << "RecordsetIterator::findNext" << " field=" << field << " val=" << val << endl;
    RecordPtr p;
    while (p = operator++()) {
        if (val == p->field(field)->toString()) {
            break;
        }
    } 

    return p;

}

RecordPtr
RecordsetIterator::findPrevious(const QString &field, const QString &val)
{
    //kdDebug(20000) << "RecordsetIterator::findPrevious" << " field=" << field << " val=" << val << endl;
    RecordPtr p;
    while (p = operator--()) {
        if (val == p->field(field)->toString()) {
            break;
        }
    } 
    
    return p;

}

RecordPtr
RecordsetIterator::findLast(const QString &field, const QString &val)
{
    //kdDebug(20000) << "RecordsetIterator::findLast" << " field=" << field << " val=" << val << endl;
    moveLast();    
    RecordPtr p = current();
    do {
        if (val == p->field(field)->toString()) {
            break;
        }
    } while (p = operator--());

    return p;

}




