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
#include <kdebug.h>

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include <qurl.h>
#include <qdir.h>
#include <qstringlist.h>

static
QString encode( const QString& segment )
{
  QCString local = segment.local8Bit();

  int old_length = local.length();

  if ( !old_length )
    return QString::null;

  // a worst case approximation
  QChar *new_segment = new QChar[ old_length * 3 + 1 ];
  int new_length = 0;

  for ( int i = 0; i < old_length; i++ )
  {
    // 'unsave' and 'reserved' characters
    // according to RFC 1738,
    // 2.2. URL Character Encoding Issues (pp. 3-4)
    // WABA: Added non-ascii
    unsigned char character = local[i];
    if ( (character <= 32) || (character >= 127) ||
         strchr("<>#@\"&%$:,;?={}|^~[]\'`\\", character) )
    {
      new_segment[ new_length++ ] = '%';

      unsigned int c = character / 16;
      c += (c > 9) ? ('A' - 10) : '0';
      new_segment[ new_length++ ] = c;

      c = character % 16;
      c += (c > 9) ? ('A' - 10) : '0';
      new_segment[ new_length++ ] = c;
	
    }
    else
      new_segment[ new_length++ ] = local[i];
  }

  QString result = QString(new_segment, new_length);
  delete [] new_segment;
  return result;
}

static char hex2int( unsigned int _char )
{
  if ( _char >= 'A' && _char <='F')
    return _char - 'A' + 10;
  if ( _char >= 'a' && _char <='f')
    return _char - 'a' + 10;
  if ( _char >= '0' && _char <='9')
    return _char - '0';
  return -1;
}

// WABA: The result of lazy_encode isn't usable for a URL which
// needs to satisfies RFC requirements. However, the following
// operation will make it usable again:
//	encode(decode(...))
//
// As a result one can see that url.prettyURL() does not result in
// a RFC compliant URL but that the following sequence does:
//	KURL(url.prettyURL()).url()


static QString lazy_encode( const QString& segment )
{
  int old_length = segment.length();

  if ( !old_length )
    return QString::null;

  // a worst case approximation
  QChar *new_segment = new QChar[ old_length * 3 + 1 ];
  int new_length = 0;

  for ( int i = 0; i < old_length; i++ )
  {
    unsigned int character = segment[i].unicode(); // Don't use latin1()
                                                   // It returns 0 for non-latin1 values
    // Small set of really ambiguous chars
    if ((character < 32) ||  // Low ASCII
        ((character == '%') && // The escape character itself
           (i+2 < old_length) && // But only if part of a valid escape sequence!
          (hex2int(segment[i+1].unicode())!= -1) &&
          (hex2int(segment[i+2].unicode())!= -1)) ||
        (character == '?') || // Start of query delimiter
        (character == '#') || // Start of reference delimiter
        ((character == 32) && (i+1 == old_length))) // A trailing space
    {
      new_segment[ new_length++ ] = '%';

      unsigned int c = character / 16;
      c += (c > 9) ? ('A' - 10) : '0';
      new_segment[ new_length++ ] = c;

      c = character % 16;
      c += (c > 9) ? ('A' - 10) : '0';
      new_segment[ new_length++ ] = c;
    }
    else
    new_segment[ new_length++ ] = segment[i];
  }

  QString result = QString(new_segment, new_length);
  delete [] new_segment;
  return result;
}


static QString decode( const QString& segment, bool *keepEncoded=0 )
{
  bool isUnicode = false;
  bool isLocal = false;
  bool isAscii = true;
  int old_length = segment.length();
  if ( !old_length )
    return QString::null;

  int new_length = 0;

  // make a copy of the old one
  char *new_segment = new char[ old_length + 1];
  QChar *new_usegment = new QChar[ old_length + 1 ];

  int i = 0;
  while( i < old_length )
  {
    unsigned int character = segment[ i++ ].unicode();
    if ( (character == '%' ) &&
         ( i+1 < old_length) ) // Must have at least two chars left!
    {
      char a = hex2int( segment[i].latin1() );
      char b = hex2int( segment[i+1].latin1() );
      if ((a != -1) && (b != -1)) // Only replace if sequence is valid
      {
         character = a * 16 + b; // Replace with value of %dd
         i += 2; // Skip dd
         if (character > 127)
            isLocal = true;
      }
    }
    new_segment [ new_length ] = character;
    new_usegment [ new_length ] = character;
    new_length++;
    if (character > 127)
    {
       isAscii = false;
       if (character > 255)
          isUnicode = true;
    }
  }
  new_segment [ new_length ] = 0;
  QString result;
  // Guess the encoding
  if ((!isAscii && !isUnicode) || isLocal)
  {
     result = QString::fromLocal8Bit(new_segment, new_length);
     if (keepEncoded)
       *keepEncoded = true;
  }
  else
  {
     result = QString( new_usegment, new_length);
     if (keepEncoded)
       *keepEncoded = false;
  }
  delete [] new_segment;
  delete [] new_usegment;
  return result;
}

bool KURL::isRelativeURL(const QString &_url)
{
  int len = _url.length();
  if (!len) return true; // Very short relative URL.
  const QChar *str = _url.unicode();

  // Absolute URL must start with alpha-character
  if (!isalpha(str[0].latin1()))
     return true; // Relative URL

  for(int i = 1; i < len; i++)
  {
     char c = str[i].latin1(); // Note: non-latin1 chars return 0!
     if (c == ':')
        return false; // URL starts with "xxx:" -> absolute URL

     // Protocol part may only contain alpha, digit, + or -
     if (!isalpha(c) && !isdigit(c) && (c != '+') && (c != '-'))
        return true; // Relative URL
  }
  // URL did not contain ':'
  return true; // Relative URL
}

KURL::List::List(const QStringList &list)
{
  for (QStringList::ConstIterator it = list.begin();
       it != list.end();
       it++)
    {
      append( KURL(*it) );
    }
}

QStringList KURL::List::toStringList() const
{
  QStringList lst;
   for( KURL::List::ConstIterator it = begin();
        it != end();
        it++)
   {
      lst.append( (*it).url() );
   }
   return lst;
}


KURL::KURL()
{
  reset();
}

KURL::KURL( const QString &url, int encoding_hint )
{
  reset();
  parse( url, encoding_hint );
}

KURL::KURL( const char * url, int encoding_hint )
{
  reset();
  parse( QString::fromLatin1(url), encoding_hint );
}

KURL::KURL( const KURL& _u )
{
  m_strProtocol = _u.m_strProtocol;
  m_strUser = _u.m_strUser;
  m_strPass = _u.m_strPass;
  m_strHost = _u.m_strHost;
  m_strPath = _u.m_strPath;
  m_strPath_encoded = _u.m_strPath_encoded;
  m_strQuery_encoded = _u.m_strQuery_encoded;
  m_strRef_encoded = _u.m_strRef_encoded;
  m_bIsMalformed = _u.m_bIsMalformed;
  m_iPort = _u.m_iPort;
}

QDataStream & operator<< (QDataStream & s, const KURL & a)
{
    s << a.m_strProtocol << a.m_strUser << a.m_strPass << a.m_strHost
      << a.m_strPath << a.m_strPath_encoded << a.m_strQuery_encoded << a.m_strRef_encoded
      << Q_INT8(a.m_bIsMalformed ? 1 : 0) << a.m_iPort;
    return s;
}

QDataStream & operator>> (QDataStream & s, KURL & a)
{
    Q_INT8 malf;
    s >> a.m_strProtocol >> a.m_strUser >> a.m_strPass >> a.m_strHost
      >> a.m_strPath >> a.m_strPath_encoded >> a.m_strQuery_encoded >> a.m_strRef_encoded
      >> malf >> a.m_iPort;
    a.m_bIsMalformed = (malf != 0);

    if ( a.m_strQuery_encoded.isEmpty() )
      a.m_strQuery_encoded = QString::null;

    return s;
}

KURL::KURL( const QUrl &u )
{
  m_strProtocol = u.protocol();
  m_strUser = u.user();
  m_strPass = u.password();
  m_strHost = u.host();
  m_strPath = u.path( FALSE );
  m_strPath_encoded = QString::null;
  m_strQuery_encoded = u.query();
  m_strRef_encoded = u.ref();
  m_bIsMalformed = !u.isValid();
  m_iPort = u.port();
}

KURL::KURL( const KURL& _u, const QString& _rel_url, int encoding_hint )
{
  // WORKAROUND THE RFC 1606 LOOPHOLE THAT ALLOWS
  // http:/index.html AS A VALID SYNTAX FOR RELATIVE
  // URLS. ( RFC 2396 section 5.2 item # 3 )
  QString rUrl = _rel_url;
  if ( _u.hasHost() && rUrl.length() != 0 &&
       rUrl.find( QRegExp("^[a-zA-Z][a-zA-Z0-9\\+\\.\\-]*:/?[a-zA-Z0-9%_!~'();:@&=$,\\?\\*\\+\\.\\-]") ) == 0 &&
       rUrl.find( _u.m_strProtocol, 0 , false ) == 0 )
  {
  		rUrl.remove( 0, rUrl.find( ':' ) + 1 );
  }

  if ( rUrl[0] == '#' )
  {
    *this = _u;
    setHTMLRef( decode(rUrl.mid(1)) );
  }
  else if ( isRelativeURL( rUrl) )
  {
    *this = _u;
    m_strQuery_encoded = QString::null;
    m_strRef_encoded = QString::null;
    if ( rUrl[0] == '/')
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
    KURL tmp( url() + rUrl, encoding_hint);
    *this = tmp;
    cleanPath();
  }
  else
  {
    KURL tmp( rUrl, encoding_hint);
    *this = tmp;
  }
}

void KURL::reset()
{
  m_strProtocol = QString::null;
  m_strUser = QString::null;
  m_strPass = QString::null;
  m_strHost = QString::null;
  m_strPath = QString::null;
  m_strPath_encoded = QString::null;
  m_strRef_encoded = QString::null;
  m_bIsMalformed = true;
  m_iPort = 0;
}

bool KURL::isEmpty() const
{
  return (m_strPath.isEmpty() && m_strProtocol.isEmpty());
}

void KURL::parse( const QString& _url, int /* encoding_hint */ )
{
  // Return immediately whenever the given url
  // is empty or null.
  if ( _url.length() == 0  )
  {
    m_strProtocol = _url;
    return;
  }

  // This is wrong!! All URLs should be deemed invalid until
  // they have been correctly parsed. Specially now since ::url()
  // returns the given url even if it is malformed. (DA)
  // m_bIsMalformed = false;

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

  // Node 2: Accept any amount of (alpha|digit|'+'|'-')
  // '.' is not currently accepted, because current KURL may be confused.
  // Proceed with :// :/ or :
  while( (isalpha((int)buf[pos]) || isdigit((int)buf[pos]) ||
          buf[pos] == '+' || buf[pos] == '-') &&
         pos < len ) pos++;
  if ( pos == len - 1 ) // Need to always compare length()-1 otherwise KURL passes "http:" as legal!!! (DA)
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
    // goto NodeOk; Wrong!!!  As the above comment states at least one character is required here!!!
      goto NodeErr;
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
      m_strHost = decode(QString( buf + start, pos - start ));
      goto NodeOk;
    }
  x = buf[pos];
  if ( x == '@' )
    {
      m_strUser = decode(QString( buf + start, pos - start ));
      pos++;
      goto Node7;
    }
  /* else if ( x == ':' )
     {
     m_strHost = decode(QString( buf + start, pos - start ));
     pos++;
     goto Node8a;
     } */
  else if ( x == '/' )
    {
      m_strHost = decode(QString( buf + start, pos - start ));
      start = pos++;
      goto Node9;
    }
  else if ( x != ':' )
    goto NodeErr;
  m_strUser = decode(QString( buf + start, pos - start ));
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
  m_strPass = decode(QString( buf + start, pos - start));
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
      m_strHost = decode(QString( buf + start, pos - start ));
      goto NodeOk;
    }
  x = buf[pos];
  m_strHost = decode(QString( buf + start, pos - start ));
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
    goto NodeOk; // Wrong, but since a fix was applied up top it is a non-issue here!!!!
                 // Just for the record an opaque URL such as "mailto:" is always required
                 // to have at least one more character other than a '/' following the colon.
  // Node 12: Accept the res
  setEncodedPathAndQuery( QString( buf + start, len - start ) );
  goto NodeOk;

 NodeOk:
  delete []orig;
  m_bIsMalformed = false; // Valid URL
  if (m_strProtocol.isEmpty())
    m_strProtocol = "file";

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
  kdDebug(126) << "KURL couldn't parse URL \"" << _url << "\"" << endl;
  delete []orig;
  reset();
  m_strProtocol = _url;
}

KURL& KURL::operator=( const QString& _url )
{
  reset();
  parse( _url );

  return *this;
}

KURL& KURL::operator=( const char * _url )
{
  reset();
  parse( QString::fromLatin1(_url) );

  return *this;
}

KURL& KURL::operator=( const QUrl & u )
{
  m_strProtocol = u.protocol();
  m_strUser = u.user();
  m_strPass = u.password();
  m_strHost = u.host();
  m_strPath = u.path( FALSE );
  m_strPath_encoded = QString::null;
  m_strQuery_encoded = u.query();
  m_strRef_encoded = u.ref();
  m_bIsMalformed = !u.isValid();
  m_iPort = u.port();

  return *this;
}

KURL& KURL::operator=( const KURL& _u )
{
  m_strProtocol = _u.m_strProtocol;
  m_strUser = _u.m_strUser;
  m_strPass = _u.m_strPass;
  m_strHost = _u.m_strHost;
  m_strPath = _u.m_strPath;
  m_strPath_encoded = _u.m_strPath_encoded;
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
       m_strPath_encoded == _u.m_strPath_encoded &&
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

bool KURL::cmp( const KURL &_u, bool _ignore_trailing ) const
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
  setPath( path );
  cleanPath();
}

static QString cleanpath(const QString &path)
{
  if (path.isEmpty()) return QString::null;
  // Did we have a trailing '/'
  int len = path.length();
  bool slash = false;
  if ( len > 0 && path.right(1)[0] == '/' )
    slash = true;

  QString result = QDir::cleanDirPath( path );

  // Restore the trailing '/'
  len = result.length();
  if ( len > 0 && result.right(1)[0] != '/' && slash )
    result += "/";
  return result;
}

void KURL::cleanPath() // taken from the old KURL
{
  m_strPath = cleanpath(m_strPath);
  // WABA: Is this safe when "/../" is encoded with %?
  m_strPath_encoded = cleanpath(m_strPath_encoded);
}

static QString trailingSlash( int _trailing, const QString &path )
{
  QString result = path;

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
  else {
    assert( 0 );
    return QString::null;
  }
}


QString KURL::encodedPathAndQuery( int _trailing, bool _no_empty_path, int ) const
{
  QString tmp;
  if (!m_strPath_encoded.isEmpty())
  {
     tmp = trailingSlash( _trailing, m_strPath_encoded);
  }
  else
  {
     tmp = path( _trailing );
     if ( _no_empty_path && tmp.isEmpty() )
        tmp = "/";
     tmp = encode( tmp );
  }

  tmp += m_strQuery_encoded;
  return tmp;
}

void KURL::setEncodedPathAndQuery( const QString& _txt, int )
{
  int pos = _txt.find( '?' );
  if ( pos == -1 )
  {
    m_strPath_encoded = _txt;
    m_strQuery_encoded = QString::null;
  }
  else
  {
    m_strPath_encoded = _txt.left( pos );
    m_strQuery_encoded = _txt.right(_txt.length() - pos);
  }
  bool keepEncoded;
  m_strPath = decode( m_strPath_encoded, &keepEncoded );
  if (!keepEncoded)
     m_strPath_encoded = QString::null;
}

QString KURL::path( int _trailing ) const
{
  return trailingSlash( _trailing, path() );
}

bool KURL::isLocalFile() const
{
  static QString fileProt = QString::fromLatin1( "file" );
  return ( ( m_strProtocol == fileProt ) && ( m_strHost.isEmpty()) );
}

bool KURL::hasSubURL() const
{
  return ( KProtocolManager::self().isFilterProtocol( m_strProtocol ) && !m_strRef_encoded.isEmpty() );
}

QString KURL::url( int _trailing ) const
{
  if( m_bIsMalformed )
  {
    // Return the whole url even when the url is
    // malformed.  Under such conditions the url
    // is stored in m_strProtocol.
    return m_strProtocol;
  }

  QString u = m_strProtocol.copy();
  if ( hasHost() )
  {
    u += "://";
    if ( hasUser() )
    {
      u += encode(m_strUser);
      if ( hasPass() )
      {
	u += ":";
	u += encode(m_strPass);
      }
      u += "@";
    }
    u += encode(m_strHost);
    if ( m_iPort != 0 ) {
      QString buffer;
      buffer.sprintf( ":%u", m_iPort );
      u += buffer;
    }
  }
  else
    u += ":";

  u += encodedPathAndQuery( _trailing );

  if ( hasRef() )
  {
    u += "#";
    u += m_strRef_encoded;
  }

  return u;
}

QString KURL::prettyURL() const
{
  if( m_bIsMalformed )
  {
    // Return the whole url even when the url is
    // malformed.  Under such conditions the url
    // is stored in m_strProtocol.
    return m_strProtocol;
  }

  QString u = m_strProtocol.copy();
  if ( hasHost() )
  {
    u += "://";
    if ( hasUser() )
    {
      u += lazy_encode(m_strUser);
      // Don't show password!
      u += "@";
    }
    u += lazy_encode(m_strHost);
    if ( m_iPort != 0 ) {
      QString buffer;
      buffer.sprintf( ":%u", m_iPort );
      u += buffer;
    }
  }
  else
  {
    u += ":";
  }

  u += lazy_encode( m_strPath );

  u += m_strQuery_encoded;

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
      kdDebug() << "Has SUB URL " << u.ref().local8Bit().data() << endl;
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

QString KURL::filename( bool _ignore_trailing_slash_in_path ) const
{
    kdDebug() << "KURL::filename() is obsolete, use KURL::fileName() instead" << endl;
    return fileName(_ignore_trailing_slash_in_path);
}

QString KURL::fileName( bool _strip_trailing_slash ) const
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
  m_strPath_encoded = QString::null;

  if ( _txt.isEmpty() )
    return;

  int i = 0;
  int len = m_strPath.length();
  // NB: avoid three '/' when building a new path from nothing
  if ( len == 0 ) {
    while( _txt[i] == '/' ) ++i;
  }
  // Add the trailing '/' if it is missing
  else if ( _txt[0] != '/' && ( len == 0 || m_strPath[ len - 1 ] != '/' ) )
    m_strPath += "/";

  // No double '/' characters
  i = 0;
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


// implemented by David, faure@kde.org
// Modified by Torben, weis@kde.org
bool KURL::cd( const QString& _dir, bool zapRef )
{
//TODO
//WABA: Add support for m_strPath_encoded
  if ( _dir.isEmpty() || m_bIsMalformed )
    return false;

  // absolute path ?
  if ( _dir[0] == '/' )
  {
    m_strPath_encoded = QString::null;
    m_strPath = _dir;
    if ( zapRef )
      setHTMLRef( QString::null );
    return true;
  }

  // Users home directory on the local disk ?
  if ( ( _dir[0] == '~' ) && ( m_strProtocol == "file" ))
  {
    m_strPath_encoded = QString::null;
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
//TODO
//WABA: Add better support for m_strPath_encoded
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
  u.cd("..", false /*don't zap ref yet*/);

  // Did we change the directory ? => job done
  if ( ( u.path() != old ) && ( KProtocolManager::self().outputType( u.protocol() ) != KProtocolManager::T_STREAM ) )
  {
    if ( _zapRef )
      u.setHTMLRef( QString::null );

    return u;
  }

  // So we have to strip protocols.
  // Example: tar:/#gzip:/decompress#file:/home/weis/test.tgz will be changed
  // to file:/home/weis/
  KURL::List lst = split( u );
  if (lst.isEmpty())
      return KURL();

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
    return decode( ref() );
  }

  List lst = split( *this );
  return decode( (*lst.begin()).ref() );
}

void KURL::setHTMLRef( const QString& _ref )
{
  if ( !hasSubURL() )
  {
    m_strRef_encoded = encode( _ref );
    return;
  }

  List lst = split( *this );

  (*lst.begin()).setRef( encode( _ref) );

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

void
KURL::setProtocol( const QString& _txt )
{
   m_strProtocol = _txt;
   m_bIsMalformed = false;
}

void
KURL::setUser( const QString& _txt )
{
   m_strUser = _txt;
}

void
KURL::setPass( const QString& _txt )
{
   m_strPass = _txt;
}

void
KURL::setHost( const QString& _txt )
{
   m_strHost = _txt;
}

void
KURL::setPort( unsigned short int _p )
{
   m_iPort = _p;
}

void KURL::setPath( const QString & path )
{
  if (isEmpty())
    m_bIsMalformed = false;
  if (m_strProtocol.isEmpty())
    m_strProtocol = "file";
  m_strPath = path;
  m_strPath_encoded = QString::null;
}

void KURL::setQuery( const QString &_txt, int )
{
   if (_txt.length() && (_txt[0] !='?'))
      m_strQuery_encoded = "?" + _txt;
   else
      m_strQuery_encoded = _txt;
}

QString KURL::decode_string(const QString &str, int)
{
   return decode(str);
}

QString KURL::encode_string(const QString &str, int)
{
   return encode(str);
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
