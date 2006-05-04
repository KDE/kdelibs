/*  -*- c++ -*-
    Copyright (c) 2005 Ingo Kloecker <kloecker@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kascii.h"

int kasciistricmp( const char *str1, const char *str2 )
{
    const unsigned char *s1 = (const unsigned char *)str1;
    const unsigned char *s2 = (const unsigned char *)str2;
    int res;
    unsigned char c1, c2;

    if ( !s1 || !s2 )
        return s1 ? 1 : (s2 ? -1 : 0);
    if ( !*s1 || !*s2 )
        return *s1 ? 1 : (*s2 ? -1 : 0);
    for (;*s1; ++s1, ++s2) {
        c1 = *s1; c2 = *s2;
        if (c1 >= 'A' && c1 <= 'Z')
            c1 += 'a' - 'A';
        if (c2 >= 'A' && c2 <= 'Z')
            c2 += 'a' - 'A';

        if ((res = c1 - c2))
            break;
    }
    return *s1 ? res : (*s2 ? -1 : 0);
}

/** Convert a single ASCII character to lowercase.
    @param ch Character to convert
    @internal
*/
static unsigned char ASCIIToLower( unsigned char ch )
{
  if ( ch >= 'A' && ch <= 'Z' )
    return ch - 'A' + 'a';
  else
    return ch;
}

char * kAsciiToLower( char *s )
{
  if ( !s )
    return 0;
  for ( unsigned char *p = (unsigned char *) s; *p; ++p )
    *p = ASCIIToLower( *p );
  return s;
}

/** Convert a single ASCII character to uppercase.
    @param ch Character to convert
    @internal
*/
static unsigned char ASCIIToUpper( unsigned char ch )
{
  if ( ch >= 'a' && ch <= 'z' )
    return ch - 'a' + 'A';
  else
    return ch;
}

char * kAsciiToUpper( char *s )
{
  if ( !s )
    return 0;
  for ( unsigned char *p = (unsigned char *) s; *p; ++p )
    *p = ASCIIToUpper( *p );
  return s;
}

