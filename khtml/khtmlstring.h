/*
    This file is part of the KDE libraries

    Copyright (C) 1999 Lars Knoll (knoll@mpi-hd.mpg.de)

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
//----------------------------------------------------------------------------
//
// KDE HTML Widget -- String class
// $Id$

#ifndef KHTMLSTRING_H
#define KHTMLSTRING_H

#include "qstring.h"
#include <stdlib.h>

class HTMLString
{
public:
    HTMLString() { s = 0, l = 0; }
    /* constructs an empty HTMLString. Like this assignment to 0 works */
    HTMLString(int) { s = 0, l = 0; }
    HTMLString(QChar *str, uint len);
//protected: // sometimes useful, to find unwanted conversions
    HTMLString(const QString &str) { 
	s = (QChar *)str.unicode();
	l = str.length();
    }
public:
    virtual ~HTMLString() { }

    // assign and copy
    HTMLString(const HTMLString &str);
    HTMLString &operator =(const HTMLString &str);

    HTMLString *operator++();
    HTMLString *operator += (int i);
    HTMLString operator + (int i);

    QChar operator [](int i) {
	return *(s+i);
    }

#if 0
    /** return a QConstString cast to a const QString. The caller is 
	responsible for deleting the returned string */
    const QString *string() const {
	QConstString *str = new QConstString(s, l);
	return &(str->string());
    }
#endif

    void find(const QChar c) {
	while( l > 0 )
	{
	    if( *s == c ) return;
	    s++, l--;
	}
    }

    uint length() const { return l; }
    void setLength( int len ) { l = len; }
    QChar *unicode() const { return s; }
    operator const QString() { return QConstString(s, l).string(); }
    const QString string() { return QConstString(s, l).string(); }
    int toInt() const { return QConstString(s, l).string().toInt(); }
    bool percentage(int &_percentage) const;

private:

    uint l;
    QChar *s;
};

inline bool operator==( const HTMLString &a, const HTMLString &b )
{ 
    if( a.length() != b.length() ) return false;

    int l = a.length();
    QChar *s1 = a.unicode();
    QChar *s2 = b.unicode();
     while ( l )
    {
	if( *s1 != *s2 ) return false;
	l--, s1++, s2++;
    }
    return true;
}

inline bool operator==( const HTMLString &a, const QString &b )
{ 
    HTMLString c( (QChar *)b.unicode(), b.length() );
    return (a == c);
}


bool strncmp( const HTMLString &a, const QChar *b, int len);
int ustrlen( const QChar *c );
QChar *ustrchr( const QChar *c, const QChar s );
int ustrncasecmp( const QChar *a, const QChar *b, int l );

// just decides if they are equal or not!!!!!
int ustrcasecmp( const HTMLString &a, const HTMLString &b );
int ustrcasecmp( const HTMLString &a, const QString &b );

//-----------------------------------------------------------------------------

#endif
