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
#include "field.h"

#include <kdebug.h>

#include "field.moc"

using namespace KDB;

Field::Field(const QString &name, QObject *parent)
    : Object ( parent, name.utf8() ),
      m_type(UNKNOWN),
      m_size(0),
      m_precision(0)
{
    //kdDebug(20000) << "Field::Field" << endl;
}

Field::Field(HandlerPtr h,  const QString &name, QObject *parent )
    : Object ( parent, name.utf8() ), m_handler( h ),
      m_type(UNKNOWN),
      m_size(0),
      m_precision(0)

{
    //kdDebug(20000) << "Field::Field <2>" << endl;
    m_type = m_handler->kdbDataType(name);
    m_native = m_handler->nativeType(name);
}

Field::~Field()
{
    //kdDebug(20000) << "Field::~Field" << endl;
}

DataType
Field::type()
{
    return m_type;
}
    
QString
Field::nativeType()
{
    return m_native;
}

void
Field::setConstraint(const QString &val)
{
    m_constraint = val;
}

QString 
Field::constraint()  
{
    return m_constraint;
}
    
void
Field::setSize(unsigned int size)
{
    //TODO: check if the type allow sizing
    m_size = size;
}

unsigned int
Field::size()
{
    return m_size;
}

void
Field::setType(DataType t)
{
    m_type = t;
}

void
Field::setNativeType(const QString & type)
{
    m_native = type;
}

void
Field::setPrecision(unsigned int precision)
{
    //TODO: check if the type allow precision
    m_precision = precision;
}

unsigned int
Field::precision()
{
    return m_precision;
}

void
Field::setAcceptNull(bool acceptNull)
{
    //TODO: check if the type allow acceptNull
    m_acceptNull = acceptNull;
}

bool
Field::acceptNull()
{
    return m_acceptNull;
}


void 
Field::setDefaultValue(const Value &val)
{
    m_default = val;
}

Value
Field::defaultValue()
{
    return m_default;
}

void 
Field::setComment(const QString &val)
{
    m_comment = val;
}

QString 
Field::comment()
{
    return m_comment;
}

Value *
Field::operator ->()
{
    if (isNull()) {
        return new Value("NULL");
    }
    
    return &m_val;
    
}

Value
Field::value()
{
    if (isNull()) {
        kdDebug(20000) << "field " << name() << " is NULL" << endl;
        return Value("NULL");
    }
    
    return m_val;
}

void
Field::setValue(const Value &v, bool init)
{
    if (!init)
        m_changed = (m_val != v);
    m_val = v;
}

bool
Field::changed()
{
    return m_changed;
}

Field &
Field::operator =(const Value &v)
{
    setValue(v);
    return *this;
}

bool
Field::isNull()
{
    return (m_val.type() == Value::Invalid);
}

