/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */

#include "dom_string.h"
#include "dom_stringimpl.h"
using namespace DOM;


DOMString::DOMString()
{
    impl = 0;
}

DOMString::DOMString(int)
{
    impl = 0;
}

DOMString::DOMString(QChar *str, uint len, bool copy) 
{ 
    QChar *c;
    if(copy)
    {
	c = new QChar[len];
	memcpy(c, str, len*sizeof(QChar));
    }
    else
    {
	c = str;
    }
    impl = new DOMStringImpl( c, len );
    impl->ref();
}

DOMString::DOMString(const QChar *str, uint len) 
{ 
    QChar *c;
    c = new QChar[len];
    memcpy(c, str, len*sizeof(QChar));
    impl = new DOMStringImpl( c, len );
    impl->ref();
}

DOMString::DOMString(const QString &str)
{ 
    QChar *c;
    c = new QChar[str.length()];
    memcpy(c, str.unicode(), str.length()*sizeof(QChar));
    impl = new DOMStringImpl( c, str.length() );
    impl->ref();
}

DOMString::DOMString(const char *str) 
{ 
    int len = 0;
    const char *p = str;
    while(*p != '\0') p++, len++;
    QChar *m_str = new QChar[len];
    int i = 0;
    while(i<len) 
    {
	m_str[i] = str[i];
	i++;
    }
    impl = new DOMStringImpl( m_str, len );
    impl->ref();
}

DOMString::DOMString(DOMStringImpl *i)
{
    impl = i;
    if(impl) impl->ref();
}

DOMString::DOMString(const DOMString &other)
{ 
    impl = other.impl;
    if(impl) impl->ref();
}

DOMString::~DOMString()
{
    if(impl) impl->deref();
}

DOMString &DOMString::operator =(const DOMString &other) 
{
    if(impl) impl->deref();
    impl = other.impl;
    if(impl) impl->ref();
    return *this;
}

DOMString &DOMString::operator += (const DOMString &str) 
{
    if(!impl) 
    {
	impl = str.impl->copy();
	impl->ref();
	return *this;
    }
    if(str.impl)
    {
	impl->append(str.impl);
    }
    return *this;
}

DOMString DOMString::operator + (const DOMString &str) 
{
    if(!impl) return str.copy();
    if(str.impl)
    {
	DOMString s = copy();
	s += str;
	return s;
    }

    return copy();
}

void DOMString::insert(DOMString str, uint pos)
{
    if(!impl) 
    {
	impl = str.impl->copy();
	impl->ref();
    }
    else
	impl->insert(str.impl, pos);
}


const QChar &DOMString::operator [](unsigned int i)
{
    static const QChar nullChar = 0;

    if(!impl || i > impl->l ) return nullChar;

    return *(impl->s+i);
}

int DOMString::find(const QChar c)
{
    unsigned int l = 0;
    if(!impl) return -1;
    while( l < impl->l )
    {
	if( *(impl->s+l) == c ) return l;
	l++;
    }
    return -1;
}

uint DOMString::length() const
{
    if(!impl) return 0;
    return impl->l;
}

void DOMString::truncate( unsigned int len )
{
    if(impl) impl->truncate(len);
}

bool DOMString::percentage(int &_percentage) const 
{
    if ( *(impl->s+impl->l-1) != QChar('%'))
       return false;

    _percentage = QConstString(impl->s, impl->l-1).string().toInt();
    return true;
}

QChar *DOMString::unicode() const
{
    if(!impl) return 0;
    return impl->s;
}

const QString DOMString::string() const
{
    if(!impl) return QConstString(0, 0).string();

    return QConstString(impl->s, impl->l).string(); 
}

int DOMString::toInt() const
{
    if(!impl) return 0;

    return QConstString(impl->s, impl->l).string().toInt();
}

DOMString DOMString::copy() const
{
    if(!impl) return 0;
    return impl->copy();
}

// ------------------------------------------------------------------------

bool DOM::strncmp( const DOMString &a, const DOMString &b, unsigned int len)
{ 
    if(a.length() < len || b.length() < len) return false;

    if(memcmp(a.unicode(), b.unicode(), len*sizeof(QChar)))
	return true;
    return false;
}

int DOM::strncasecmp( const DOMString &s1, const DOMString &s2, unsigned int l )
{
    const QChar *a = s1.unicode();
    const QChar *b = s2.unicode();

    while ( l-- )
    {
	if( a->lower() != b->lower() )
	    return a->lower() - b->lower();
	a++,b++;
    }
    return 0;
}

int DOM::strcasecmp( const DOMString &a, const DOMString &b )
{
    if( a.length() != b.length() ) return -1;
    unsigned int l = a.length();
    return strncasecmp(a, b, l);
}

//-----------------------------------------------------------------------------

bool DOM::operator==( const DOMString &a, const DOMString &b )
{ 
    if( a.length() != b.length() ) return false;

    int l = a.length();
    if(!memcmp(a.unicode(), b.unicode(), l*sizeof(QChar))) 
	return true;
    return false;
}

bool DOM::operator==( const DOMString &a, const QString &b )
{ 
    if( a.length() != b.length() ) return false;

    int l = a.length();
    if(!memcmp(a.unicode(), b.unicode(), l*sizeof(QChar))) 
	return true;
    return false;
}

bool DOM::operator==( const DOMString &a, const char *b )
{ 
    return a == DOMString(b);
}

bool DOM::operator==( const DOMString &a, int )
{
    return (a.length() == 0);
}

bool DOM::operator!=( const DOMString &a, const DOMString &b )
{
    return !(a==b);
}

bool DOM::operator!=( const DOMString &a, int )
{
    return (a.length() != 0);
}

bool DOM::operator!=( const DOMString &a, const QString &b )
{
    return !(a==b);
}

bool DOM::operator!=( const DOMString &a, const char *b )
{ 
    return a != DOMString(b);
}


