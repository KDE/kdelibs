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
#include <khtmlstring.h>


HTMLString::HTMLString(QChar *str, uint len) 
{ 
    s = str, l = len; 
}

// assign and copy
HTMLString::HTMLString(const HTMLString &str) 
{ 
    s = str.unicode(), l = str.length();
}

HTMLString &HTMLString::operator =(const HTMLString &str) 
{
    s = str.unicode();
    l = str.length();
    return *this;
}

HTMLString *HTMLString::operator++() {
    if(l > 0 ) 
	s++, l--;
    return this;
}

HTMLString *HTMLString::operator += (int i) {
    if((int)l < i)
	s = 0, l = 0;
    else
	s += i, l -= i;
    return this;
}    
HTMLString HTMLString::operator + (int i) {
    if((int)l < i)
	return HTMLString();
    else
	return HTMLString(s + i, l - i);
}
bool HTMLString::percentage(int &_percentage) const 
{
    if ( *(s+l-1) != QChar('%'))
       return false;

    _percentage = QConstString(s, l-1).string().toInt();
    return true;
}


bool strncmp( const HTMLString &a, const QChar *b, int len)
{ 
    HTMLString c((QChar *)b, len);
    return (a == c);
}


int ustrlen( const QChar *c )
{
    int l = 0;
    while( *c++ != QChar::null ) l++;
    return l;
}

QChar *ustrchr( const QChar *c, const QChar s )
{
    while( *c != QChar::null )
    {
	if( *c == s ) return (QChar *)c; 
	c++;
    }
    return 0L;
}


int ustrncasecmp( const QChar *a, const QChar *b, int l )
{
    while ( l-- && a->lower() == b->lower() )
	a++,b++;
    if ( l==-1 )
	return 0;
    return a->lower() - b->lower();
}

// just decides if they are equal or not!!!!!
int ustrcasecmp( const HTMLString &a, const HTMLString &b )
{
    if( a.length() != b.length() ) return -1;
    int l = a.length();
    return ustrncasecmp(a.unicode(), b.unicode(), l);
}

int ustrcasecmp( const HTMLString &a, const QString &b )
{
    if( a.length() != b.length() ) return -1;
    int l = a.length();
    return ustrncasecmp(a.unicode(), b.unicode(), l);
}


//-----------------------------------------------------------------------------


