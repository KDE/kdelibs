/* This file is part of the KDE libraries
    Copyright (C) 1999 Torben Weis <weis@kde.org>
 
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

#include "kurl.h"

#include <stdio.h>
#include <assert.h>
#include <qdir.h>

// Reference: RFC 1738 Uniform Resource Locators

bool kurl_parse( KURL *_url, const char *_txt );

KURL::KURL()
{
  reset();
  m_bIsMalformed = true;
}

KURL::KURL( const char *_url )
{
  reset();
  m_strProtocol = "file";
  m_iPort = -1;
  parse( _url );
}

KURL::KURL( QString &_url )
{
  reset();
  m_strProtocol = "file";
  m_iPort = -1;
  parse( _url.data() );
}

KURL::KURL( const KURL& _u )
{
  m_strProtocol = _u.m_strProtocol;
  m_strUser = _u.m_strUser;
  m_strPass = _u.m_strPass;
  m_strHost = _u.m_strHost;
  m_strPath = _u.m_strPath;
  m_strQuery_encoded = _u.m_strQuery_encoded;
  m_strRef_encoded = _u.m_strRef_encoded;
  m_bIsMalformed = _u.m_bIsMalformed;
  m_iPort = _u.m_iPort;
}

KURL::KURL( const KURL& _u, const char *_rel_url )
{
  if ( _rel_url[0] == '/' )
  {
    *this = _u;
    setEncodedPathAndQuery( _rel_url );
  }
  else if ( _rel_url[0] == '#' )
  {
    *this = _u;
    setRef( _rel_url + 1 );
  }
  else if ( strstr( _rel_url, ":/" ) != 0 )
  {
    *this = _rel_url;
  }
  else
  {
    *this = _u;
    QString tmp;
    decode( tmp );
    setFileName( tmp.data() );
  }
}

void KURL::reset()
{
  m_strProtocol = "file";
  m_strUser = "";
  m_strPass = "";
  m_strHost = "";
  m_strPath = "";
  m_strQuery_encoded = "";
  m_strRef_encoded = "";
  m_bIsMalformed = false;
  m_iPort = -1;
}

void KURL::parse( const char *_url )
{
  m_bIsMalformed = !kurl_parse( this, _url );
}

KURL& KURL::operator=( const char* _url )
{
  reset();  
  parse( _url );

  return *this;
}

KURL& KURL::operator=( QString& _url )
{
  reset();
  parse( _url.data() );

  return *this;
}

KURL& KURL::operator=( const KURL& _u )
{
  m_strProtocol = _u.m_strProtocol;
  m_strUser = _u.m_strUser;
  m_strPass = _u.m_strPass;
  m_strHost = _u.m_strHost;
  m_strPath = _u.m_strPath;
  m_strQuery_encoded = _u.m_strQuery_encoded;
  m_strRef_encoded = _u.m_strRef_encoded;
  m_bIsMalformed = _u.m_bIsMalformed;
  m_iPort = _u.m_iPort;

  return *this;
}

bool KURL::operator==( const KURL& _u ) const
{
  if ( isMalformed() || _u.isMalformed() )
    return false;
  
  if ( m_strProtocol == _u.m_strProtocol &&
       m_strUser == _u.m_strUser &&
       m_strPass == _u.m_strPass &&
       m_strHost == _u.m_strHost &&
       m_strPath == _u.m_strPath &&
       m_strQuery_encoded == _u.m_strQuery_encoded &&
       m_strRef_encoded == _u.m_strRef_encoded &&
       m_bIsMalformed == _u.m_bIsMalformed &&
       m_iPort == _u.m_iPort )
    return true;
  
  return false;
}

bool KURL::operator==( const char* _u ) const
{
  KURL u( _u );
  return ( *this == u );
}

bool KURL::cmp( KURL &_u, bool _ignore_trailing )
{
  if ( _ignore_trailing )
  {
    QString path1 = path(1);
    QString path2 = _u.path(1);
    if ( path1 != path2 )
      return false;

    if ( m_strProtocol == _u.m_strProtocol &&
	 m_strUser == _u.m_strUser &&
	 m_strPass == _u.m_strPass &&
	 m_strHost == _u.m_strHost &&
	 m_strQuery_encoded == _u.m_strQuery_encoded &&
	 m_strRef_encoded == _u.m_strRef_encoded &&
	 m_bIsMalformed == _u.m_bIsMalformed &&
	 m_iPort == _u.m_iPort )
      return true;

    return false;
  }
  
  return ( *this == _u );
}

void KURL::setFileName( const char *_txt )
{
  // TODO: clean path at the end
  while( *_txt == '/' ) _txt++;

  if ( m_strPath.isEmpty() )
  {
    m_strPath = "/";
    m_strPath += _txt;
    return;
  }    
  
  if ( m_strPath.right(1) == "/")
  {
    m_strPath += _txt;
    return;
  }
  
  int i = m_strPath.findRev( '/' );
  // If ( i == -1 ) => The first character is not a '/' ???
  // This looks strange ...
  if ( i == -1 )
  {
    m_strPath = "/";
    m_strPath += _txt;
    return;
  }
  
  m_strPath.truncate( i+1 ); // keep the "/"
  m_strPath += _txt;
}

QString KURL::encodedPathAndQuery( int _trailing, bool _no_empty_path )
{
  QString tmp = path( _trailing );
  if ( _no_empty_path && tmp.isEmpty() )
    tmp = "/";
  
  encode( tmp );
  if ( !m_strQuery_encoded.isEmpty() )
  {
    tmp += "?";
    tmp += m_strQuery_encoded;
  }
  
  return tmp;
}

void KURL::setEncodedPathAndQuery( const char *_txt )
{
  QString tmp = _txt;
  int pos = tmp.find( '?' );
  if ( pos == -1 )
  {
    m_strPath = tmp;
    m_strQuery_encoded = "";
  }
  else
  { 
    m_strPath = tmp.left( pos );
    m_strQuery_encoded = _txt + pos + 1;
  }

  decode( m_strPath );
}

QString KURL::path( int _trailing ) const
{
  QString result = path();

  if ( _trailing == 0 )
    return result;
  else if ( _trailing == 1 )
  {
    int len = result.length();
    if ( len == 0 )
      result = "";
    else if ( result[ len - 1 ] != '/' )
      result += "/";
    return result;
  }
  else if ( _trailing == -1 )
  {
    if ( result == "/" )
      return result;
    int len = result.length();
    if ( len != 0 && result[ len - 1 ] == '/' )
      result.truncate( len - 1 );
    return result;
  }
  else
    assert( 0 );
}

bool KURL::isLocalFile() const
{
  if ( m_strProtocol != "file" )
    return false;
  
  if ( m_strRef_encoded.isEmpty() )
    return true;
  
  KURL u( m_strRef_encoded.data() );
  if ( u.isMalformed() )
    return true;
  
  return false;
}

bool KURL::hasSubURL() const
{
  if ( m_strRef_encoded.isEmpty() )
    return false;
  
  KURL u( m_strRef_encoded.data() );
  if ( u.isMalformed() )
    return false;

  return true;
}

QString KURL::url() const
{
  return url( 0 );
}

QString KURL::url( int _trailing ) const
{
  // HACK encode parts here!

  QString u = m_strProtocol.copy();
  if ( hasHost() )
  {
    u += "://";
    if ( hasUser() )
    {
      u += m_strUser;
      if ( hasPass() )
      {
	u += ":";
	u += m_strPass;
      }
      u += "@";
    }
    u += m_strHost;
    if ( m_iPort != -1 )
    {
      char buffer[ 100 ];
      sprintf( buffer, ":%i", m_iPort );
      u += buffer;
    }
  }
  else
    u += ":";
  QString tmp;
  if ( _trailing == 0 )
    tmp = m_strPath;
  else
    tmp = path( _trailing );
  encode( tmp );
  u += tmp;
    
  if ( !m_strQuery_encoded.isEmpty() )
  {
    u += "?";
    u += m_strQuery_encoded;
  }
  
  if ( hasRef() )
  {
    u += "#";
    u += m_strRef_encoded;
  }
  
  return u;
}

bool KURL::split( const char *_url, QList<KURL>& lst )
{
  lst.setAutoDelete(true);
  QString tmp;
  
  do
  {
    KURL * u = new KURL ( _url );
    if ( u->isMalformed() )
      return false;
    
    if ( u->hasSubURL() )
    {
      tmp = u->ref();
      _url = tmp.data();
      u->setRef( "" );
      lst.append( u );
    }
    else
    {
      lst.append( u );
      return true;
    }
  } while( 1 );
}

void KURL::join( KURLList & lst, QString& _dest )
{
  _dest = "";
  KURL * it;
  for( it = lst.first() ; it ; it = lst.next() )
  {
    QString tmp = it->url();
    _dest += tmp;
    if ( it != lst.getLast() )
      _dest += "#";
  }
}

QString KURL::filename( bool _strip_trailing_slash )
{
  QString fname;

  int len = m_strPath.length();
  if ( len == 0 )
    return fname;
  
  if ( _strip_trailing_slash )
  {    
    while ( len >= 1 && m_strPath[ len - 1 ] == '/' )
      len--;
  }
  else if ( m_strPath[ len - 1 ] == '/' )
    return fname;
  
  // Does the path only consist of '/' characters ?
  if ( len == 1 && m_strPath[ 1 ] == '/' )
    return fname;
  
  int i = m_strPath.findRev( '/', len - 1 );
  // If ( i == -1 ) => The first character is not a '/' ???
  // This looks like an error to me.
  if ( i == -1 )
    return fname;
  
  fname = m_strPath.mid( i + 1, len - i - 1 ); // TO CHECK
  // fname.assign( m_strPath, i + 1, len - i - 1 );
  return fname;
}

void KURL::addPath( const char *_txt )
{
  if ( *_txt == 0 )
    return;
  
  int len = m_strPath.length();
  // Add the trailing '/' if it is missing
  if ( _txt[0] != '/' && ( len == 0 || m_strPath[ len - 1 ] != '/' ) )
    m_strPath += "/";
    
  // No double '/' characters
  if ( len != 0 && m_strPath[ len - 1 ] == '/' )
    while( *_txt == '/' )
      _txt++;
  
  m_strPath += _txt;
}

QString KURL::directory( bool _strip_trailing_slash_from_result, bool _ignore_trailing_slash_in_path )
{
  QString result;
  if ( _ignore_trailing_slash_in_path )
    result = path( -1 );
  else
    result = m_strPath;
 
  if ( result.isEmpty() || result == "/" )
    return result;
    
  int i = result.findRev( "/" );
  if ( i == -1 )
    return result;
  
  if ( i == 0 )
  {
    result = "/";
    return result;
  }
  
  if ( _strip_trailing_slash_from_result )
    result = m_strPath.left( i );
  else
    result = m_strPath.left( i + 1 );

  return result;
}

void KURL::encode( QString& _url )
{
  int old_length = _url.length();

  if ( !old_length )
    return;
   
  // a worst case approximation
  char *new_url = new char[ old_length * 3 + 1 ];
  int new_length = 0;
     
  for ( int i = 0; i < old_length; i++ )
  {
    // 'unsave' and 'reserved' characters
    // according to RFC 1738,
    // 2.2. URL Character Encoding Issues (pp. 3-4)
    // Torben: Added the space characters
    if ( strchr("<>#@\"&%$:,;?={}|^~[]\'`\\ \n\t\r", _url[i]) )
    {
      new_url[ new_length++ ] = '%';

      char c = _url[ i ] / 16;
      c += (c > 9) ? ('A' - 10) : '0';
      new_url[ new_length++ ] = c;

      c = _url[ i ] % 16;
      c += (c > 9) ? ('A' - 10) : '0';
      new_url[ new_length++ ] = c;
	    
    }
    else
      new_url[ new_length++ ] = _url[i];
  }

  new_url[new_length] = 0;
  _url = new_url;
  delete [] new_url;
}

char KURL::hex2int( char _char )
{
  if ( _char >= 'A' && _char <='F')
    return _char - 'A' + 10;
  if ( _char >= 'a' && _char <='f')
    return _char - 'a' + 10;
  if ( _char >= '0' && _char <='9')
    return _char - '0';
  return 0;
}

void KURL::decode( QString& _url )
{
  int old_length = _url.length();
  if ( !old_length )
    return;
    
  int new_length = 0;

  // make a copy of the old one
  char *new_url = new char[ old_length + 1];

  int i = 0;
  while( i < old_length )
  {
    char character = _url[ i++ ];
    if ( character == '%' )
    {
      character = hex2int( _url[i] ) * 16 + hex2int( _url[i+1] );
      i += 2;
    }
    new_url [ new_length++ ] = character;
  }
  new_url [ new_length ] = 0;
  _url = new_url;
  delete [] new_url;
}

// Compatibility with old KURL. Added by David Faure <faure@kde.org>
bool KURL::cd( const QString& _dir, bool zapRef)
{
    if ( _dir.isNull() )
        return false;
 
    if( _dir[0] == '/' )
    {
        m_strPath = _dir;
    }
    else if (( _dir[0] == '~' ) && ( m_strProtocol == "file" ))
    {
        m_strPath = QDir::homeDirPath().copy();
        m_strPath += "/";
        m_strPath += _dir + 1;
    }
    else
    {
        if ( m_strPath.right(1)[0] != '/' && _dir[0] != '/' )
            m_strPath += "/";
        m_strPath += _dir;
    }

    m_strPath = QDir::cleanDirPath( m_strPath );
    if ( zapRef )
        setRef( QString::null );
 
    return true;
}

bool urlcmp( KURLList& _url1, KURLList& _url2 )
{
  unsigned int size = _url1.count();
  if ( _url2.count() != size )
    return false;
  
  KURL* it1 = _url1.first();
  KURL* it2 = _url2.first();
  for( ; it1 && it2 ; it1 = _url1.next(), it2 = _url2.next() )
    if ( it1->url() != it2->url() )
      return false;
  
  return true;
}

bool urlcmp( const char *_url1, const char *_url2 )
{
  KURLList list1;
  KURLList list2;

  bool res1 = KURL::split( _url1, list1 );
  bool res2 = KURL::split( _url2, list2 );

  if ( !res1 || !res2 )
    return false;

  return urlcmp( list1, list2 );
}

bool urlcmp( const char *_url1, const char *_url2, bool _ignore_trailing, bool _ignore_ref )
{
  KURLList list1;
  KURLList list2;

  bool res1 = KURL::split( _url1, list1 );
  bool res2 = KURL::split( _url2, list2 );

  if ( !res1 || !res2 )
    return false;

  unsigned int size = list1.count();
  if ( list2.count() != size )
    return false;

  if ( _ignore_ref )
  {    
    list1.getLast()->setRef("");
    list2.getLast()->setRef("");
  }
  
  KURL* it1 = list1.first();
  KURL* it2 = list2.first();
  for( ; it1 && it2 ; it1 = list1.next(), it2 = list2.next() )
    if ( !it1->cmp( *it2, _ignore_ref ) )
      return false;

  return true;
}
