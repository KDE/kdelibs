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
#include <kprotocolmanager.h>

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include <qdir.h>


static bool
isRelativeURL(const QString &_url)
{
  // A relative URL has no ':' at least not before the first '/'.
  int colonPos = _url.find(':');
  if (colonPos == -1)
      return true; // No ':' found, this is a relative URL

  int slashPos = _url.find('/');
  if ((slashPos != -1) && (colonPos > slashPos))
      return true; // We have a ':' but it comes after a '/' so it is a relative URL

  return false; // It's not a relative URL.
}

// Reference: RFC 1738 Uniform Resource Locators

KURL::KURL()
{
  reset();
  m_bIsMalformed = true;
}

KURL::KURL( const QString &_url )
{
  reset();
  /*The following two variables have already been
    initialized by reset()! Why Re-initialize again ?? (Dawit A.)
  m_strProtocol = "file";
  m_iPort = 0; */
  parse( _url );
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

KURL::KURL( const KURL& _u, const QString& _rel_url )
{
  if ( _rel_url[0] == '#' )
  {
    *this = _u;
    QString tmp = _rel_url.mid(1);
    decode( tmp );
    setHTMLRef( tmp );
  }
  else if ( isRelativeURL(_rel_url))
  {
    *this = _u;
    m_strQuery_encoded = QString::null;
    m_strRef_encoded = QString::null;
    if (_rel_url[0] == '/')
    {
        m_strPath = QString::null;
    }
    else 
    {
       int pos = m_strPath.findRev( '/' );
       if (pos >= 0)
          m_strPath.truncate(pos);
       m_strPath += '/';
    }
    *this = url() + _rel_url;
    cleanPath();
  }
  else
  {
    *this = _rel_url;
  }
}

void KURL::reset()
{
  m_strProtocol = "file";
  m_strUser = QString::null;
  m_strPass = QString::null;
  m_strHost = QString::null;
  m_strPath = QString::null;
  m_strRef_encoded = QString::null;
  m_bIsMalformed = false;
  m_iPort = 0;
}

void KURL::parse( const QString& _url )
{
  if ( _url.isEmpty() )
    {
      m_bIsMalformed = true;
      return;
    }

  m_bIsMalformed = false;

  QString port;
  int start = 0;
  uint len = _url.length();
  QChar* buf = new QChar[ len + 1 ];
  QChar* orig = buf;
  memcpy( buf, _url.unicode(), len * sizeof( QChar ) );

  uint pos = 0;

  // Node 1: Accept alpha or slash
  QChar x = buf[pos++];
  if ( x == '/' )
    goto Node9;
  if ( !isalpha( (int)x ) )
    goto NodeErr;

  // Node 2: Accept any amount of alphas
  // Proceed with :// :/ or :
  while( isalpha( buf[pos] ) && pos < len ) pos++;
  if ( pos == len )
    goto NodeErr;
  if (buf[pos] == ':' && buf[pos+1] == '/' && buf[pos+2] == '/' )
    {
      m_strProtocol = QString( orig, pos ).lower();
      pos += 3;
    }
  else if (buf[pos] == ':' && buf[pos+1] == '/' )
    {
      m_strProtocol = QString( orig, pos ).lower();
      pos++;
      start = pos;
      goto Node9;
    }
  else if ( buf[pos] == ':' )
    {
      m_strProtocol = QString( orig, pos ).lower();
      pos++;
      goto Node11;
    }
  else
    goto NodeErr;

  //Node 3: We need at least one character here
  if ( pos == len )
    goto NodeOk;
  //    goto NodeErr;
#if 0
  start = pos++;
#else
  start = pos;
#endif

  // Node 4: Accept any amount of characters.
  // Terminate or / or @
  while( buf[pos] != ':' && buf[pos] != '@' && buf[pos] != '/' && pos < len ) pos++;
  if ( pos == len )
    {
      m_strHost = QString( buf + start, pos - start );
      goto NodeOk;
    }
  x = buf[pos];
  if ( x == '@' )
    {
      m_strUser = QString( buf + start, pos - start );
      pos++;
      goto Node7;
    }
  /* else if ( x == ':' )
     {
     m_strHost = QString( buf + start, pos - start );
     pos++;
     goto Node8a;
     } */
  else if ( x == '/' )
    {
      m_strHost = QString( buf + start, pos - start );
      start = pos++;
      goto Node9;
    }
  else if ( x != ':' )
    goto NodeErr;
  m_strUser = QString( buf + start, pos - start );
  pos++;

  // Node 5: We need at least one character
  if ( pos == len )
    goto NodeErr;
  start = pos++;

  // Node 6: Read everything until @
  while( buf[pos] != '@' && pos < len ) pos++;
  if ( pos == len )
    {
      // Ok the : was used to separate host and port
      m_strHost = m_strUser;
      m_strUser = QString::null;
      QString tmp( buf + start, pos - start );
      char *endptr;
      m_iPort = (unsigned short int)strtol(tmp.ascii(), &endptr, 10);
      if ((pos == len) && (strlen(endptr) == 0))
	goto NodeOk;
      // there is more after the digits
      pos -= strlen(endptr);
      start = pos++;
      goto Node9;
    }
  m_strPass = QString( buf + start, pos - start);
  pos++;

  // Node 7: We need at least one character
 Node7:
  if ( pos == len )
    goto NodeErr;
  start = pos++;

  // Node 8: Read everything until / : or terminate
  while( buf[pos] != '/' && buf[pos] != ':' && pos < len ) pos++;
  if ( pos == len )
    {
      m_strHost = QString( buf + start, pos - start );
      goto NodeOk;
    }
  x = buf[pos];
  m_strHost = QString( buf + start, pos - start );
  if ( x == '/' )
    {
      start = pos++;
      goto Node9;
    }
  else if ( x != ':' )
    goto NodeErr;
  pos++;

  // Node 8a: Accept at least one digit
  if ( pos == len )
    goto NodeErr;
  start = pos;
  if ( !isdigit( buf[pos++] ) )
    goto NodeErr;

  // Node 8b: Accept any amount of digits
  while( isdigit( buf[pos] ) && pos < len ) pos++;
  port = QString( buf + start, pos - start );
  m_iPort = port.toUShort();
  if ( pos == len )
    goto NodeOk;
  start = pos++;

  // Node 9: Accept any character and # or terminate
 Node9:
  while( buf[pos] != '#' && pos < len ) pos++;
  if ( pos == len )
    {
      QString tmp( buf + start, len - start );
      setEncodedPathAndQuery( tmp );
      // setEncodedPathAndQuery( QString( buf + start, pos - start ) );
      goto NodeOk;
    }
  else if ( buf[pos] != '#' )
    goto NodeErr;
  setEncodedPathAndQuery( QString( buf + start, pos - start ) );
  pos++;

  // Node 10: Accept all the rest
  m_strRef_encoded = QString( buf + pos, len - pos );
  goto NodeOk;

  // Node 11 We need at least one character
 Node11:
  start = pos;
  if ( pos++ == len )
    goto NodeOk;
  //    goto NodeErr;

  // Node 12: Accept the res
  setEncodedPathAndQuery( QString( buf + start, len - start ) );
  goto NodeOk;

 NodeOk:
  delete []orig;
  //debug("Prot=%s\nUser=%s\nPass=%s\nHost=%s\nPath=%s\nQuery=%s\nRef=%s\nPort=%i\n",
  //m_strProtocol.ascii(), m_strUser.ascii(), m_strPass.ascii(),
  //m_strHost.ascii(), m_strPath.ascii(), m_strQuery_encoded.ascii(),
  //m_strRef_encoded.ascii(), m_iPort );
  if (m_strProtocol == "file") 
  {
    if (!m_strHost.isEmpty())
    {
      // File-protocol has a host name..... hmm?
      if (m_strHost.lower() == "localhost")
      {
        m_strHost = QString::null; // We can ignore localhost
      }
      else {
        // What to do with a remote file?
        QString remoteProtocol( KProtocolManager::self().remoteFileProtocol() );
        if (remoteProtocol.isEmpty())
        {
           // Pass the hostname as part of the path. Perhaps system calls
           // just handle it.
           m_strPath = "//"+m_strHost+m_strPath;
           m_strHost = QString::null;
        }
        else
        {
           // Use 'remoteProtocol' as protocol.
           m_strProtocol = remoteProtocol;
         }
      }
    }
  }
  return;

 NodeErr:
  debug("Error in parsing \"%s\"",_url.ascii());
  delete []orig;
  m_bIsMalformed = true;
}

KURL& KURL::operator=( const QString& _url )
{
  reset();
  parse( _url );

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

bool KURL::operator==( const QString& _u ) const
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

void KURL::setFileName( const QString& _txt )
{
  int i = 0;
  while( _txt[i] == '/' ) ++i;
  QString tmp;
  if ( i )
    tmp = _txt.mid( i );
  else
    tmp = _txt;

  QString path = m_strPath;
  if ( path.isEmpty() )
    path = "/";
  else
  {
    int lastSlash = path.findRev( '/' );
    if ( lastSlash == -1)
    {
      // The first character is not a '/' ???
      // This looks strange ...
      path = "/";
    }
    else if ( path.right(1) != "/" )
      path.truncate( lastSlash+1 ); // keep the "/"
  }

  path += tmp;
  setEncodedPathAndQuery( path );
  cleanPath();
}

void KURL::cleanPath() // taken from the old KURL
{
  if ( m_strPath.isEmpty() )
    return;

  // Did we have a trailing '/'
  int len = m_strPath.length();
  bool slash = false;
  if ( len > 0 && m_strPath.right(1)[0] == '/' )
    slash = true;

  m_strPath = QDir::cleanDirPath( m_strPath );

  // Restore the trailing '/'
  len = m_strPath.length();
  if ( len > 0 && m_strPath.right(1)[0] != '/' && slash )
    m_strPath += "/";
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

void KURL::setEncodedPathAndQuery( const QString& _txt )
{
  int pos = _txt.find( '?' );
  if ( pos == -1 )
  {
    m_strPath = _txt;
    m_strQuery_encoded = QString::null;
  }
  else
  {
    m_strPath = _txt.left( pos );
    m_strQuery_encoded = _txt.right(_txt.length() - pos - 1);
    // WABA: Make sure to distinguish between an empty query and no query!
    if (m_strQuery_encoded.isNull())
       m_strQuery_encoded = "";
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
      result = QString::null;
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
  return ( ( m_strProtocol == "file" ) && ( m_strHost.isEmpty()) );
}

bool KURL::hasSubURL() const
{
  return ( KProtocolManager::self().isFilterProtocol( m_strProtocol ) && !m_strRef_encoded.isEmpty() );
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
    if ( m_iPort != 0 ) {
      char buffer[ 100 ];
      sprintf( buffer, ":%u", m_iPort );
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

  // !!WABA!! There is a small but subtle difference between not
  // having a query-string and having an empty query string!
  // Therefor we check against isNull() and not against isEmpty()!
  if ( !m_strQuery_encoded.isNull() ) 
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

KURL::List KURL::split( const KURL& _url )
{
  return split( _url.url() );
}

KURL::List KURL::split( const QString& _url )
{
  KURL::List lst;
  QString tmp = _url;

  do
  {
    KURL u( tmp );
    if ( u.isMalformed() )
      return KURL::List();

    // Continue with recursion ?
    if ( u.hasSubURL() )
    {
      debug("Has SUB URL %s", u.ref().local8Bit().data() );
      tmp = u.ref();
      u.setRef( QString::null );
      lst.append( u );
    }
    // A HTML style reference finally ?
    else if ( u.hasRef() )
    {
      tmp = u.ref();
      u.setRef( QString::null );
      lst.append( u );
      // Append the HTML style reference to the
      // first URL.
      (*lst.begin()).setRef( tmp );
      return lst;
    }
    // No more references and suburls
    else
    {
      lst.append( u );
      return lst;
    }
  } while( 1 );

  // Never reached
  return lst;
}

QString KURL::join( const KURL::List & lst )
{
  QString dest = QString::null;
  QString ref;

  KURL::List::ConstIterator it;
  for( it = lst.begin() ; it != lst.end(); ++it )
  {
    if ( it == lst.begin() )
      ref = (*it).ref();
    else
      ASSERT( !(*it).hasRef() );

    QString tmp = (*it).url();
    dest += tmp;
    if ( it != lst.fromLast() )
      dest += "#";
  }

  if ( !ref.isEmpty() )
  {
    dest += "#";
    dest += ref;
  }

  return dest;
}

QString KURL::filename( bool _strip_trailing_slash ) const
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

void KURL::addPath( const QString& _txt )
{
  if ( _txt.isEmpty() )
    return;

  int len = m_strPath.length();
  // Add the trailing '/' if it is missing
  if ( _txt[0] != '/' && ( len == 0 || m_strPath[ len - 1 ] != '/' ) )
    m_strPath += "/";

  // No double '/' characters
  int i = 0;
  if ( len != 0 && m_strPath[ len - 1 ] == '/' )
  {
    while( _txt[i] == '/' )
      ++i;
  }

  m_strPath += _txt.mid( i );
}

QString KURL::directory( bool _strip_trailing_slash_from_result,
			 bool _ignore_trailing_slash_in_path ) const
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
  // Does not handle Unicode contents in _url.

  int old_length = _url.length();

  if ( !old_length )
    return;

  // a worst case approximation
  QChar *new_url = new QChar[ old_length * 3 + 1 ];
  int new_length = 0;

  // The (char)(QChar) casts below should use QChar::latin1() instead.

  for ( int i = 0; i < old_length; i++ )
  {
    // 'unsave' and 'reserved' characters
    // according to RFC 1738,
    // 2.2. URL Character Encoding Issues (pp. 3-4)
    // Torben: Added the space characters
    if ( strchr("<>#@\"&%$:,;?={}|^~[]\'`\\ \n\t\r", _url[i].unicode()) )
    {
      new_url[ new_length++ ] = '%';

      unsigned int c = _url[ i ].unicode() / 16;
      c += (c > 9) ? ('A' - 10) : '0';
      new_url[ new_length++ ] = c;

      c = _url[ i ].unicode() % 16;
      c += (c > 9) ? ('A' - 10) : '0';
      new_url[ new_length++ ] = c;
	
    }
    else
      new_url[ new_length++ ] = _url[i].unicode();
  }

  new_url[new_length] = 0;
  _url = QString(new_url, new_length);
  delete [] new_url;
}

char KURL::hex2int( unsigned int _char )
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
  // Does not handle Unicode in _url.

  int old_length = _url.length();
  if ( !old_length )
    return;

  int new_length = 0;

  // make a copy of the old one
  QChar *new_url = new QChar[ old_length + 1];

  // The (char)(QChar) casts below should use QChar::latin1() instead.

  int i = 0;
  while( i < old_length )
  {
    unsigned int character = _url[ i++ ].unicode();
    if ( character == '%' )
    {
      character = hex2int( _url[i].unicode() ) * 16
		+ hex2int( _url[i+1].unicode() );
      i += 2;
    }
    new_url [ new_length++ ] = character;
  }
  new_url [ new_length ] = 0;
  _url = QString(new_url, new_length);;
  delete [] new_url;
}

// implemented by David, faure@kde.org
// Modified by Torben, weis@kde.org
bool KURL::cd( const QString& _dir, bool zapRef )
{
  if ( _dir.isEmpty() )
    return false;

  // absolute path ?
  if ( _dir[0] == '/' )
  {
    m_strPath = _dir;
    if ( zapRef )
      setHTMLRef( QString::null );
    return true;
  }

  // Users home directory on the local disk ?
  if ( ( _dir[0] == '~' ) && ( m_strProtocol == "file" ))
  {
    m_strPath = QDir::homeDirPath().copy();
    m_strPath += "/";
    m_strPath += _dir.right(m_strPath.length() - 1);
    if ( zapRef )
      setHTMLRef( QString::null );
    return true;
  }

  // relative path
  // we always work on the past of the first url.
  // Sub URLs are not touched.

  // append '/' if necessary
  QString p = path(1);

  p += _dir;
  p = QDir::cleanDirPath( p );
  setPath( p );

  if ( zapRef )
    setHTMLRef( QString::null );

  return true;
}

KURL KURL::upURL( bool _zapRef ) const
{
  QString old = m_strPath;

  KURL u( *this );
  u.cd("..");

  // Did we change the directory ? => job done
  if ( u.path() != old )
  {
    if ( _zapRef )
      u.setHTMLRef( QString::null );

    return u;
  }

  // So we have to strip protocols.
  // Example: tar:/#gzip:/decompress#file:/home/weis/test.tgz will be changed
  // to file:/home/weis/
  KURL::List lst = split( u );

  QString ref = (*lst.begin()).ref();

  // Remove first protocol
  lst.remove( lst.begin() );

  // Remove all stream protocols
  while( lst.begin() != lst.end() )
  {
    if ( KProtocolManager::self().inputType( (*lst.begin()).protocol() ) == KProtocolManager::T_STREAM )
      lst.remove( lst.begin() );
    else
      break;
  }

  // No source protocol at all ?
  if ( lst.begin() == lst.end() )
    return KURL();

  // Remove the filename. Example: We start with
  // tar:/#gzip:/decompress#file:/home/x.tgz
  // and end with file:/home/x.tgz until now. Yet we
  // just strip the filename at the end of the leftmost
  // url.
  (*lst.begin()).setPath( (*lst.begin()).directory( false ) );

  if ( !_zapRef )
    (*lst.begin()).setRef( ref );

  return join( lst );
}

QString KURL::htmlRef() const
{
  if ( !hasSubURL() )
  {
    QString tmp = ref();
    decode( tmp );
    return tmp;
  }

  List lst = split( *this );
  QString tmp = (*lst.begin()).ref();
  decode( tmp );

  return tmp;
}

void KURL::setHTMLRef( const QString& _ref )
{
  if ( !hasSubURL() )
  {
    m_strRef_encoded = _ref;
    encode( m_strRef_encoded );
    return;
  }

  List lst = split( *this );
  QString tmp = _ref;
  encode( tmp );
  (*lst.begin()).setRef( tmp );

  *this = join( lst );
}

bool KURL::hasHTMLRef() const
{
  if ( !hasSubURL() )
  {
    return hasRef();
  }

  List lst = split( *this );
  return (*lst.begin()).hasRef();
}

bool urlcmp( const QString& _url1, const QString& _url2 )
{
  // Both empty ?
  if ( _url1.isEmpty() && _url2.isEmpty() )
    return true;
  // Only one empty ?
  if ( _url1.isEmpty() || _url2.isEmpty() )
    return false;

  KURL::List list1 = KURL::split( _url1 );
  KURL::List list2 = KURL::split( _url2 );

  // Malformed ?
  if ( list1.isEmpty() || list2.isEmpty() )
    return false;

  return ( list1 == list2 );
}

bool urlcmp( const QString& _url1, const QString& _url2, bool _ignore_trailing, bool _ignore_ref )
{
  // Both empty ?
  if ( _url1.isEmpty() && _url2.isEmpty() )
    return true;
  // Only one empty ?
  if ( _url1.isEmpty() || _url2.isEmpty() )
    return false;

  KURL::List list1 = KURL::split( _url1 );
  KURL::List list2 = KURL::split( _url2 );

  // Malformed ?
  if ( list1.isEmpty() || list2.isEmpty() )
    return false;

  unsigned int size = list1.count();
  if ( list2.count() != size )
    return false;

  if ( _ignore_ref )
  {
    (*list1.begin()).setRef(QString::null);
    (*list2.begin()).setRef(QString::null);
  }

  KURL::List::Iterator it1 = list1.begin();
  KURL::List::Iterator it2 = list2.begin();
  for( ; it1 != list1.end() ; ++it1, ++it2 )
    if ( !(*it1).cmp( *it2, _ignore_trailing ) )
      return false;

  return true;
}
