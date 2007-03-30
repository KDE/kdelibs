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

#ifndef KASCII_H
#define KASCII_H

#include <kdecore_export.h>

/**
 * Locale-independent qstricmp. Use this for comparing ascii keywords
 * in a case-insensitive way.
 * qstricmp fails with e.g. the Turkish locale where 'I'.toLower() != 'i'
 */
KDECORE_EXPORT int kasciistricmp( const char *str1, const char *str2 );

/**
  Locale-independent function to convert ASCII strings to lower case ASCII
  strings. This means that it affects @em only the ASCII characters A-Z.

  @param str  pointer to the string which should be converted to lower case
  @return     pointer to the converted string (same as @a str)
*/
KDECORE_EXPORT char * kAsciiToLower( char *str );

/**
  Locale-independent function to convert ASCII strings to upper case ASCII
  strings. This means that it affects @em only the ASCII characters a-z.

  @param str  pointer to the string which should be converted to upper case
  @return     pointer to the converted string (same as @a str)
*/
KDECORE_EXPORT char * kAsciiToUpper( char *str );

#endif
