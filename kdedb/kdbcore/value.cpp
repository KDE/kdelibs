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
#include "value.h"

#include <qnamespace.h>
#include <qdatetime.h>

#include <klocale.h>
#include <kdebug.h>

Value::Value()
{
    t = Invalid;
    ptr = 0;
}

Value::~Value()
{
    clearPtr();
}

Value::Value (const Value &v)
    :QVariant()
{
    if (v.type() == Date ||
        v.type() == Time ||
        v.type() == DateTime) {   
        this->t = v.type();
        this->ptr = v.ptr;
    } else {
        this->t = Invalid;
        this->ptr = 0;
        *this = v;
    }
}

Value::Value (const QVariant &v)
    : QVariant(v)
{
    this->t = Invalid;
    this->ptr = 0;    
}

Value::Value (const QDate &d )
{
    t = Date;
    ptr = new QDate(d);
}

Value::Value (const QTime& tm )
{
    t = Time;
    ptr = new QTime(tm);
}

Value::Value (const QDateTime& dt )
{
    t = DateTime;
    ptr = new QDateTime(dt);
}

Value::Value ( long l )
{
    //kdDebug(20000) << k_funcinfo << l << endl;
    t = Long;
    ptr = new long(l);
}

Value::Value (KDB_ULONG l)
{
    //kdDebug(20000) << k_funcinfo << l << endl;
    t = ULong;
    ptr = new KDB_ULONG(l);
}

Value &
Value::operator = (const Value &v)
{
    if (v.type() == Date ||
        v.type() == Time ||
        v.type() == DateTime ||
        v.type() == Long ||
        v.type() == ULong){
        clear();
        this->t = v.type();
        this->ptr = v.ptr;
    } else {
        this->t = Invalid;
        clearPtr();
        QVariant::operator =(v);
    }
    return *this;
}

bool 
Value::operator==( const Value &v ) const
{
    bool res = false;
    switch (v.type()) {
    case Date: 
        res = toDate() == v.toDate();
        break;
    case Time:
        res = toTime() == v.toTime();
        break;
    case DateTime:
        res = toDateTime() == v.toDateTime();
        break;
    case Long:
        res = toLong() == v.toLong();
        break;
    case ULong:
        res = toULong() == v.toULong();
        break;
    default:
        res = QVariant::operator ==(v);
        break;
    }
    return res;
}

bool 
Value::operator!=( const Value &v ) const
{
    return !(*this == v);
}

Value::Type
Value::type() const
{
    if (QVariant::type() == QVariant::Invalid)
        return t;
    else
        return (Value::Type)QVariant::type();
}
const char* 
Value::typeName() const
{
    switch (type()) {
    case Date:
        return "Date";
        break;
    case Time:
        return "Time";
        break;
    case DateTime:
        return "DateTime";
        break;
    case Long:
        return "Long";
        break;
    case ULong:
        return "ULong";
        break;
    default:
        return QVariant::typeName();
        break;
    }
          
}

bool 
Value::canCast( Type toCast ) const
{
    if (type() == Date && toCast == String)
        return true;
    if (type() == DateTime && toCast == String)
        return true;
    if (type() == Time && toCast == String)
        return true;

    if (toCast == Date && type() == String) {
        // test if the string can be converted?
        return true;
    }
    if (toCast == Time && type() == String) {
        // test if the string can be converted?
        return true;
    }
    if (toCast == Time && type() == String) {
        // test if the string can be converted?
        return true;
    }
    // cases for Long && ULong
    return QVariant::canCast((QVariant::Type)(toCast));
}

bool 
Value::cast( Type toCast )
{
    switch (toCast) {
    case Long:
        asLong();
        break;
    case ULong:
        asULong();
        break;
    case Date:
        asDate();
        break;
    case DateTime:
        asDateTime();
        break;
    case Time:
        asTime();
        break;
    default:
        // WARNING: QT 2.x does not have QVariant::cast
        // QVariant::cast((QVariant::Type)(toCast));
        break;
    }
    return canCast(toCast);
}

const QDate 
Value::toDate() const
{
    if ( type() == Date )
	return *((QDate*)ptr);
    if ( type() == String )
	return KLocale().readDate(toString());
    return QDate();
}

const QTime 
Value::toTime() const
{
    if ( type() == Time )
	return *((QTime*)ptr);
    if ( type() == String )
	return KLocale().readTime(toString());
    return QTime();
}

const QString 
Value::toString() const
{
    //kdDebug(20000) << k_funcinfo << typeToName(type()) << endl;
    if ( type() == DateTime ) {
        QString d = KLocale().formatDate(((QDateTime*)ptr)->date(),true);
        QString t = KLocale().formatTime(((QDateTime*)ptr)->time());
	return QString("%1 %2").arg(d).arg(t);
    }
    if ( type() == Date )
	return KLocale().formatDate(((QDateTime*)ptr)->date(),true);
    if ( type() == Time )
        return KLocale().formatTime(((QDateTime*)ptr)->time());
    if ( type() == Long )
        return QString("%1").arg(*(long*)ptr);
    if ( type() == ULong )
        return QString("%1").arg(*(KDB_ULONG*)ptr);
    
    return QVariant::toString();
}

const QDateTime 
Value::toDateTime() const
{
    if ( type() == DateTime )
	return *((QDateTime*)ptr);
    if ( type() == String )
        // WARNING: converting to a QDateTime not implemented
	return QDateTime();
    return QDateTime();
}

const long 
Value::toLong() const
{
    //kdDebug(20000) << k_funcinfo << typeToName(type()) << endl;

    if (type() == String)
        return toString().toLong();
    if (type() == Int )
        return (long) toInt();
    if ( type() == UInt )
        return (long) toUInt();
    if ( type() == Long ||
         type() == ULong )
        return *((long *)ptr);   
    if (type() == Double )
        return (long) toDouble();

    return 0L;
}

const KDB_ULONG
Value::toULong() const
{
    if (type() == String)
        return toString().toULong();
    if (type() == Int )
        return (KDB_ULONG) toInt();
    if ( type() == UInt )
        return (KDB_ULONG) toUInt();
    if ( type() == Long ||
         type() == ULong )
        return *((KDB_ULONG *)ptr);   
    if (type() == Double )
        return (KDB_ULONG) toDouble();

    return 0L;
}


QString& 
Value::asString()
{
    *this = QVariant(toString());
    return QVariant::asString();
}

QDate& 
Value::asDate()
{
    clearPtr();
    t = Date;
    ptr = new QDate(toDate());
    clear();
    return *((QDate*)ptr);
}

QTime& 
Value::asTime()
{
    clearPtr();
    t = Time;
    ptr = new QTime(toTime());
    clear();
    return *((QTime *)ptr);
}

QDateTime& 
Value::asDateTime()
{
    clearPtr();
    t = DateTime;
    ptr = new QDateTime(toDateTime());
    clear();
    return *((QDateTime*) ptr);
}

long &
Value::asLong()
{
    clearPtr();
    t = Long;
    ptr = new long(toLong());
    clear();
    return *((long*) ptr);
}

KDB_ULONG &
Value::asULong()
{
    clearPtr();
    t = ULong;
    ptr = new KDB_ULONG(toULong());
    clear();
    return *((KDB_ULONG*) ptr);
}

const char* 
Value::typeToName( Type typ )
{
    switch (typ) {
    case Long:
        return "Long";
        break;
    case ULong:
        return "ULong";
        break;
    case Date:
        return "Date";
        break;
    case Time:
        return "Time";
        break;
    case DateTime:
        return "DateTime";
        break;
    default:
        return QVariant::typeToName((QVariant::Type)typ);
        break;
    }

    return ""; //make the compiler happy
}

Value::Type
Value::nameToType( const char* name )
{
    if (strcmp(name,"Long") == 0)
        return Long;
    if (strcmp(name,"ULong") == 0)
        return ULong;
    if (strcmp(name,"Date") == 0)
        return Date;
    if (strcmp(name,"Time") == 0)
        return Time;
    if (strcmp(name,"DateTime") == 0)
        return DateTime;
    return (Value::Type)QVariant::nameToType(name);
}


void
Value::clearPtr()
{
    //if (ptr) {
    if (false) {
        switch (type()) {
        case Date:
            delete (QDate *) ptr;
            break;
        case Time:
            delete (QTime *) ptr;
            break;          
        case DateTime:
            delete (QDateTime *) ptr;
            break;
        case Long:
            delete (long *) ptr;
            break;
        case ULong:
            delete (KDB_ULONG *) ptr;
            break;
        default:
            break;
        }
        ptr = 0L;
    }
}
