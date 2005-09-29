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

#include <sys/types.h>

#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "kregexp.h"
#include "kregpriv.h"
#include "kdebug.h"

KRegExpPrivate::KRegExpPrivate()
{
  m_bInit = false;

  for ( int i = 0; i < 10; i++ )
    m_strMatches[i] = 0L;
}

KRegExpPrivate::KRegExpPrivate( const char *_pattern, const char *_mode )
{
  m_bInit = false;

  for ( int i = 0; i < 10; i++ )
    m_strMatches[i] = 0L;

  compile( _pattern, _mode );
}

KRegExpPrivate::~KRegExpPrivate()
{
  for ( int i = 0; i < 10; i++ )
    if ( m_strMatches[i] )
      free( m_strMatches[i] );

  if ( m_bInit )
    regfree( &m_pattern );
}

bool KRegExpPrivate::compile( const char *_pattern, const char *_mode )
{
  if ( m_bInit )
    regfree( &m_pattern );

  int res = regcomp( &m_pattern, _pattern, ( strchr( _mode, 'i' ) != 0L ? REG_ICASE : 0 ) | REG_EXTENDED );
  if ( res == 0 )
    m_bInit = true;

  return ( res == 0 );
}

bool KRegExpPrivate::match( const char *_string )
{
  if ( !m_bInit )
  {
    kdDebug(128) << "You must compile a pattern before you can try to match it" << endl;
    assert( 0 );
  }

  for ( int i = 0; i < 10; i++ )
  {
    m_matches[i].rm_so = -1;
    m_matches[i].rm_eo = -1;
    if ( m_strMatches[i] )
    {
      free( m_strMatches[i] );
      m_strMatches[i] = 0L;
    }
  }

  int res = regexec( &m_pattern, _string, 10, m_matches, 0 );
  if ( res != 0 )
    return false;

  int slen = strlen( _string );

  for ( int j = 0; j < 10; j++ )
  {
    if ( m_matches[j].rm_so >= 0 && m_matches[j].rm_eo >= 0 &&
	 m_matches[j].rm_so <= slen && m_matches[j].rm_eo <= slen &&
	 m_matches[j].rm_so <= m_matches[j].rm_eo )
    {
      int len = m_matches[j].rm_eo - m_matches[j].rm_so;
      m_strMatches[j] = ( char* )malloc( len + 1 );
      memcpy( m_strMatches[j], _string + m_matches[j].rm_so, len );
      m_strMatches[j][ len ] = 0;
    }
  }

  return true;
}

const char* KRegExpPrivate::group( int _grp )
{
  if ( _grp < 0 || _grp >= 10 )
  {
    kdDebug(128) << "You may only use a group in the range of 0..9" << endl;
    assert( 0 );
  }

  return m_strMatches[ _grp ];
}

int KRegExpPrivate::groupStart( int _grp )
{
  if ( _grp < 0 || _grp >= 10 )
  {
    kdDebug(128) << "You may only use a group in the range of 0..9" << endl;
    assert( 0 );
  }

  return m_matches[ _grp ].rm_so;
}

int KRegExpPrivate::groupEnd( int _grp )
{
  if ( _grp < 0 || _grp >= 10 )
  {
    kdDebug(128) << "You may only use a group in the range of 0..9" << endl;
    assert( 0 );
  }

  return m_matches[ _grp ].rm_eo;
}

KRegExp::KRegExp()
{
  m_pPrivate = new KRegExpPrivate;
}

KRegExp::KRegExp( const char *_pattern, const char *_mode)
{
  m_pPrivate = new KRegExpPrivate( _pattern, _mode );
}

KRegExp::~KRegExp()
{
  delete m_pPrivate;
}

bool KRegExp::compile( const char *_pattern, const char *_mode)
{
  return m_pPrivate->compile( _pattern, _mode );
}

bool KRegExp::match( const char *_string )
{
  return m_pPrivate->match( _string );
}

const char* KRegExp::group( int _grp )
{
  return m_pPrivate->group( _grp );
}

int KRegExp::groupStart( int _grp )
{
  return m_pPrivate->groupStart( _grp );
}

int KRegExp::groupEnd( int _grp )
{
  return m_pPrivate->groupEnd( _grp );
}

/*
int main( int argc, char **argv )
{
  if ( argc != 3 )
    assert( 0 );

  KRegExp r( argv[1], "" );
  cout << "Compiled" << endl;

  if ( !r.match( argv[2] ) )
  {
    cerr << "Does not match" << endl;
    return 0;
  }

  cout << "Match" << endl;

  for( int i = 0; i < 10; i++ )
  {
    const char *c = r.group( i );
    if ( !c )
      return 0;
    cout << "Group #" << i << ":" << c << endl;
  }

  return 0;
}
*/
