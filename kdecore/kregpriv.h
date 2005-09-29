/* This file is part of the KDE libraries
   Copyright (c) 1999 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef __kreg_private_h__
#define __kreg_private_h__

extern "C" { // bug with some libc5 distributions
#include <regex.h>
}

class KRegExpDataPtr;

/**
 * Used internally by KRegExp.
 * @internal
 *
 * @author Torben Weis <weis@kde.org>
 */
class KRegExpPrivate
{
public:
  KRegExpPrivate();
  KRegExpPrivate( const char *_pattern, const char *_mode = "" );
  ~KRegExpPrivate();

  bool compile( const char *_pattern, const char *_mode = "" );

  bool match( const char *_string );
  const char *group( int _grp );
  int groupStart( int _grp );
  int groupEnd( int _grp );
    
protected:
  regex_t m_pattern;
  regmatch_t m_matches[ 10 ];
  char* m_strMatches[10];
  bool m_bInit;

private:
  KRegExpDataPtr *d;
};

#endif
