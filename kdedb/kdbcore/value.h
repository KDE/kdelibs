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

#ifndef KDB_VALUE_H
#define KDB_VALUE_H "$Id$"

#include <qvariant.h>

#include <kdb/kdb.h>

class QDate;
class QTime;
class QDateTime;

class Value;
class ValuePrivate; // not sure it is useful at all, but who knows...

/**
 * Representation of a value object.
 *
 * It stores a value and a value type. mostly is a QVariant, with some more
 * datatypes when QT 3.0 will be out, probably all implementation will go
 * away and we will have only a QVariant here. Actually, at my best
 * knowledge, this is source compatible with QT 3.0 QVariant.
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @version kdbcore 0.0.2
 */

class Value :public QVariant {
        
 public:
    
    enum Type {
	Invalid,
	Map,
	List,
	String,
	StringList,
	Font,
	Pixmap,
	Brush,
	Rect,
	Size,
	Color,
	Palette,
	ColorGroup,
	IconSet,
	Point,
	Image,
	Int,
	UInt,
	Bool,
	Double,
	CString,
	PointArray,
	Region,
	Bitmap,
	Cursor,
	SizePolicy,
	Date,
	Time,
	DateTime,
	ByteArray,
        Long,
        ULong
    };
    
    Value();
    ~Value();
    Value (const Value &);
    Value (const QVariant &);
    Value (const QDate& );
    Value (const QTime& );
    Value (const QDateTime& );
    Value (const QString&);
    Value (const char *);
    Value (long);
    Value (int);
    Value (KDB_ULONG);
        
    Value & operator = (const Value &);
    bool operator==( const Value& ) const;
    bool operator!=( const Value& ) const;

    Type type() const;
    const char* typeName() const;

    bool canCast( Type ) const;
    bool cast( Type );

    const QString toString() const;
    const QDate toDate() const;
    const QTime toTime() const;
    const QDateTime toDateTime() const;
    const long toLong() const;
    const KDB_ULONG toULong() const;

    QString& asString();
    QDate& asDate();
    QTime& asTime();
    QDateTime& asDateTime();
    long& asLong();
    KDB_ULONG& asULong();

    static const char* typeToName( Type typ );
    static Type nameToType( const char* name );

 private:

    void clearPtr();
    
    Type t;
    void *ptr;
    ValuePrivate *d;
};
 

#endif
