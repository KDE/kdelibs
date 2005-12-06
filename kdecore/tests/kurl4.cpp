/*
    Copyright (C) 1999 Torben Weis <weis@kde.org>
    Copyright (C) 2005 David Faure <faure@kde.org>

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

/// KDE4 TODO: maybe we should use QUrl::resolved()

/*
 * The currently active RFC for URL/URIs is RFC3986
 * Previous (and now deprecated) RFCs are RFC1738 and RFC2396
 */

#include "kurl4.h"

// KDE_QT_ONLY is first used for dcop/client (e.g. marshalling)
#ifndef KDE_QT_ONLY
#include <kdebug.h>
#include <kglobal.h>
#endif

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include <qdir.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <q3stylesheet.h>
#include <qmimedata.h>
#include <qtextcodec.h>

#ifdef Q_WS_WIN
# define KURL_ROOTDIR_PATH "C:/"
#else
# define KURL_ROOTDIR_PATH "/"
#endif

static const QString fileProt = "file";

static QString cleanpath( const QString &_path, bool cleanDirSeparator, bool decodeDots )
{
  if (_path.isEmpty()) return QString::null;

  if (QDir::isRelativePath(_path))
     return _path; // Don't mangle mailto-style URLs

  QString path = _path;

  int len = path.length();

  if (decodeDots)
  {
#ifndef KDE_QT_ONLY
     static const QString &encodedDot = KGlobal::staticQString("%2e");
#else
     QString encodedDot("%2e");
#endif
     if (path.indexOf(encodedDot, 0, Qt::CaseInsensitive) != -1)
     {
#ifndef KDE_QT_ONLY
        static const QString &encodedDOT = KGlobal::staticQString("%2E"); // Uppercase!
#else
        QString encodedDOT("%2E");
#endif
        path.replace(encodedDot, ".");
        path.replace(encodedDOT, ".");
        len = path.length();
     }
  }

  bool slash = (len && path[len-1] == QLatin1Char('/')) ||
               (len > 1 && path[len-2] == QLatin1Char('/') && path[len-1] == QLatin1Char('.'));

  // The following code cleans up directory path much like
  // QDir::cleanPath() except it can be made to ignore multiple
  // directory separators by setting the flag to false.  That fixes
  // bug# 15044, mail.altavista.com and other similar brain-dead server
  // implementations that do not follow what has been specified in
  // RFC 2396!! (dA)
  QString result;
  int cdUp, orig_pos, pos;

  cdUp = 0;
  pos = orig_pos = len;
  while ( pos && (pos = path.lastIndexOf(QLatin1Char('/'),--pos)) != -1 )
  {
    len = orig_pos - pos - 1;
    if ( len == 2 && path[pos+1] == '.' && path[pos+2] == '.' )
      cdUp++;
    else
    {
      // Ignore any occurrences of '.'
      // This includes entries that simply do not make sense like /..../
      if ( (len || !cleanDirSeparator) &&
           (len != 1 || path[pos+1] != '.' ) )
      {
          if ( !cdUp )
              result.prepend(path.mid(pos, len+1));
          else
              cdUp--;
      }
    }
    orig_pos = pos;
  }

#ifdef Q_WS_WIN // prepend drive letter if exists (js)
  if (orig_pos >= 2 && isalpha(path[0].latin1()) && path[1]==':') {
    result.prepend(QString(path[0])+":");
  }
#endif

  if ( result.isEmpty() )
    result = KURL_ROOTDIR_PATH;
  else if ( slash && result[result.length()-1] != QLatin1Char('/') )
       result.append(QChar('/'));

  return result;
}

bool KURL::isRelativeURL(const QString &_url)
{
#if 0
  // would this work?
  return QUrl( _url ).isRelative();
#endif
  int len = _url.length();
  if (!len) return true; // Very short relative URL.
  const QChar *str = _url.unicode();

  // Absolute URL must start with alpha-character
  if (!isalpha(str[0].toLatin1()))
     return true; // Relative URL

  for(int i = 1; i < len; i++)
  {
     char c = str[i].toLatin1(); // Note: non-latin1 chars return 0!
     if (c == ':')
        return false; // Absolute URL

     // Protocol part may only contain alpha, digit, + or -
     if (!isalpha(c) && !isdigit(c) && (c != '+') && (c != '-'))
        return true; // Relative URL
  }
  // URL did not contain ':'
  return true; // Relative URL
}

KURL::List::List(const KURL &url)
{
    append( url );
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


void KURL::List::populateMimeData( QMimeData* mimeData,
                                const KURL::MetaDataMap& metaData,
                                MimeDataFlags flags )
{
    QList<QByteArray> urlStringList;
    KURL::List::ConstIterator uit = begin();
    const KURL::List::ConstIterator uEnd = end();
    for ( ; uit != uEnd ; ++uit )
    {
        // Get each URL encoded in utf8 - and since we get it in escaped
        // form on top of that, .latin1() is fine.
        urlStringList.append( (*uit).toMimeDataString().latin1() );
    }

    QByteArray uriListData;
    for ( QList<QByteArray>::const_iterator it = urlStringList.begin(), end = urlStringList.end()
                                                 ; it != end ; ++it ) {
        uriListData += (*it);
        uriListData += "\r\n";
    }
    mimeData->setData( "text/uri-list", uriListData );

    if ( ( flags & KURL::NoTextExport ) == 0 )
    {
        QStringList prettyURLsList;
        for ( uit = begin(); uit != uEnd ; ++uit )
            prettyURLsList.append( (*uit).prettyURL() );

        QByteArray plainTextData = prettyURLsList.join( "\n" ).toLocal8Bit();
        if( count() > 1 ) // terminate last line, unless it's the only line
            plainTextData.append( "\n" );
        mimeData->setData( "text/plain", plainTextData );
    }

    if ( !metaData.isEmpty() )
    {
        QByteArray metaDataData; // :)
        for( KURL::MetaDataMap::const_iterator it = metaData.begin(); it != metaData.end(); ++it )
        {
            metaDataData += it.key().toUtf8();
            metaDataData += "$@@$";
            metaDataData += it.data().toUtf8();
            metaDataData += "$@@$";
        }
        mimeData->setData( "application/x-kio-metadata", metaDataData );
    }
}

bool KURL::List::canDecode( const QMimeData *mimeData )
{
    return mimeData->hasFormat( "text/uri-list" ) || mimeData->hasFormat( "application/x-kde-urilist" );
}

KURL::List KURL::List::fromMimeData( const QMimeData *mimeData, KURL::MetaDataMap* metaData )
{
    KURL::List uris;
    // x-kde-urilist is the same format as text/uri-list, but contains
    // KDE-aware urls, like media:/ and system:/, whereas text/uri-list is resolved to
    // local files. So we look at it first for decoding, but we let apps set it when encoding.
    QByteArray payload = mimeData->data( "application/x-kde-urilist" );
    if ( payload.isEmpty() )
        payload = mimeData->data( "text/uri-list" );
    if ( !payload.isEmpty() ) {
        int c = 0;
        const char* d = payload.data();
        while ( c < payload.size() && d[c] ) {
            int f = c;
            // Find line end
            while (c < payload.size() && d[c] && d[c]!='\r'
                   && d[c] != '\n')
                c++;
            QByteArray s( d+f, c-f );
            if ( s[0] != '#' ) // non-comment?
                uris.append( KURL::fromMimeDataByteArray( s ) );
            // Skip junk
            while ( c < payload.size() && d[c] &&
                    ( d[c] == '\n' || d[c] == '\r' ) )
                ++c;
        }
    }
    if ( metaData )
    {
        const QByteArray metaDataPayload = mimeData->data( "application/x-kio-metadata" );
        if ( !metaDataPayload.isEmpty() )
        {
            const QString str = QString::fromUtf8( metaDataPayload );
            const QStringList lst = str.split( "$@@$");
            QStringList::ConstIterator it = lst.begin();
            bool readingKey = true; // true, then false, then true, etc.
            QString key;
            for ( ; it != lst.end(); ++it ) {
                if ( readingKey )
                    key = *it;
                else
                    metaData->insert( key, *it );
                readingKey = !readingKey;
            }
            Q_ASSERT( readingKey ); // an odd number of items would be, well, odd ;-)
        }
    }

    return uris;
}

////

KURL::KURL()
    : QUrl()
{
}

KURL::~KURL()
{
}


KURL::KURL( const QString &url )
    // : QUrl( url )  can't do that
{
    setEncodedUrl( url.latin1(), QUrl::TolerantMode );
}

KURL::KURL( const char * url )
    // : QUrl( QLatin1String(url) )
{
    setEncodedUrl( url, QUrl::TolerantMode );
}

KURL::KURL( const QByteArray& url )
    // : QUrl( QLatin1String(url) )
{
    setEncodedUrl( url, QUrl::TolerantMode );
}

KURL::KURL( const KURL& _u )
    : QUrl( _u )
{
}

KURL::KURL( const QUrl &u )
    : QUrl( u )
{
}

QDataStream & operator<< (QDataStream & s, const KURL & a)
{
#warning TODO: port to QUrl
#if 0 // ###### TODO (must be wire-compatible for DCOP)
  QString QueryForWire=a.m_strQuery_encoded;
  if (!a.m_strQuery_encoded.isNull())
    QueryForWire.prepend("?");

    s << a.m_strProtocol << a.m_strUser << a.m_strPass << a.m_strHost
      << a.m_strPath << a.m_strPath_encoded << QueryForWire << a.m_strRef_encoded
      << qint8(a.m_bIsMalformed ? 1 : 0) << a.m_iPort;
#endif
    return s;
}

QDataStream & operator>> (QDataStream & s, KURL & a)
{
#warning TODO: port to QUrl
#if 0
    qint8 malf;
    QString QueryFromWire;
    s >> a.m_strProtocol >> a.m_strUser >> a.m_strPass >> a.m_strHost
      >> a.m_strPath >> a.m_strPath_encoded >> QueryFromWire >> a.m_strRef_encoded
      >> malf >> a.m_iPort;
    a.m_bIsMalformed = (malf != 0);

    if ( QueryFromWire.isNull() )
      a.m_strQuery_encoded = QString::null;
    else if ( QueryFromWire.length() == 1 ) // empty query
      a.m_strQuery_encoded = "";
    else
      a.m_strQuery_encoded = QueryFromWire.mid(1);

    a.m_iUriMode = KURL::uriModeForProtocol( a.m_strProtocol );
#endif
    return s;
}

KURL::KURL( const KURL& _u, const QString& _rel_url )
{
#if 0
  if (_u.hasSubURL()) // Operate on the last suburl, not the first
  {
    KURL::List lst = split( _u );
    KURL u(lst.last(), _rel_url);
    lst.erase( --lst.end() );
    lst.append( u );
    *this = join( lst );
    return;
  }
#endif
  QString rUrl = _rel_url;

  // WORKAROUND THE RFC 1606 LOOPHOLE THAT ALLOWS
  // http:/index.html AS A VALID SYNTAX FOR RELATIVE
  // URLS. ( RFC 2396 section 5.2 item # 3 )
  int len = _u.scheme().length();
  if ( !_u.host().isEmpty() && !rUrl.isEmpty() &&
       rUrl.indexOf( _u.scheme(), 0, Qt::CaseInsensitive ) == 0 &&
       rUrl[len] == ':' && (rUrl[len+1] != QLatin1Char('/') ||
       (rUrl[len+1] == '/' && rUrl[len+2] != QLatin1Char('/'))) )
  {
    rUrl.remove( 0, rUrl.indexOf( ':' ) + 1 );
  }


  if ( rUrl.isEmpty() )
  {
    *this = _u;
  }
  else if ( rUrl[0] == '#' )
  {
    *this = _u;
    QString strRef_encoded = rUrl.mid(1);
    if ( strRef_encoded.isNull() )
        strRef_encoded = ""; // we know there was an (empty) html ref, we saw the '#'
    setFragment( strRef_encoded );
  }
  else if ( isRelativeURL( rUrl ) )
  {
    *this = _u;
    setFragment( QString::null );
    setEncodedQuery( QByteArray() );
    QString strPath = path();
    if ( rUrl[0] == QLatin1Char('/') )
    {
        if ((rUrl.length() > 1) && (rUrl[1] == QLatin1Char('/')))
        {
            setHost( QString::null );
            // File protocol returns file:/// without host, strip // from rUrl
            if ( _u.isLocalFile() )
                rUrl.remove(0, 2);
        }
        strPath = QString::null;
    }
    else if ( rUrl[0] != '?' )
    {
       int pos = strPath.lastIndexOf( QLatin1Char('/') );
       if (pos >= 0)
          strPath.truncate(pos);
       strPath += QLatin1Char('/');
    }
    else
    {
       if ( strPath.isEmpty() )
          strPath = QLatin1Char('/');
    }
    setPath( strPath );
    KURL tmp( url() + rUrl);
    *this = tmp;
    cleanPath(false);
  }
  else
  {
    KURL tmp( rUrl );
    *this = tmp;
    // Preserve userinfo if applicable.
    if (!_u.userInfo().isEmpty() && userInfo().isEmpty()
        && (_u.host() == host()) && (_u.scheme() == scheme()))
    {
       setUserInfo( _u.userInfo() );
    }
    cleanPath(false);
  }
}

bool KURL::operator==( const KURL& _u ) const
{
  if ( !isValid() || !_u.isValid() )
    return false;
  return QUrl::operator==( _u );;
}

bool KURL::operator==( const QString& _u ) const
{
  KURL u( _u );
  return ( *this == u );
}

bool KURL::cmp( const KURL &u, bool ignore_trailing ) const
{
  return equals( u, ignore_trailing );
}

bool KURL::equals( const KURL &_u, bool ignore_trailing ) const
{
  if ( !isValid() || !_u.isValid() )
    return false;

  if ( ignore_trailing )
  {
    QString path1 = path(1);
    QString path2 = _u.path(1);
    if ( path1 != path2 )
      return false;

    if ( scheme() == _u.scheme() &&
         authority() == _u.authority() && // user+pass+host+port
         encodedQuery() == _u.encodedQuery() &&
         fragment() == _u.fragment() )
      return true;

    return false;
  }

  return ( *this == _u );
}

void KURL::setFileName( const QString& _txt )
{
  setFragment( QString::null );
  int i = 0;
  while( i < _txt.length() && _txt[i] == QLatin1Char('/') )
      ++i;
  QString tmp = i ? _txt.mid( i ) : _txt;

  //QString path = m_strPath_encoded.isEmpty() ? m_strPath : m_strPath_encoded;
  QString path = this->path();
  if ( path.isEmpty() )
    path = "/";
  else
  {
    int lastSlash = path.lastIndexOf( QLatin1Char('/') );
    if ( lastSlash == -1)
    {
      // The first character is not a '/' ???
      // This looks strange ...
      path = QLatin1Char('/');
    }
    else if ( !path.endsWith( QLatin1Char('/') ) )
      path.truncate( lastSlash+1 ); // keep the "/"
  }
#if 0
  if (m_strPath_encoded.isEmpty())
#endif
  {
     path += tmp;
     setPath( path );
  }
#if 0
  else
  {
     path += encode_string(tmp);
     setEncodedPath( path );
  }
#endif
  cleanPath();
}

void KURL::cleanPath( bool cleanDirSeparator )
{
  //if (m_iUriMode != URL) return;
  const QString newPath = cleanpath(path(), cleanDirSeparator, false);
  if ( path() != newPath )
      setPath( newPath );
  // WABA: Is this safe when "/../" is encoded with %?
  //m_strPath_encoded = cleanpath(m_strPath_encoded, cleanDirSeparator, true);
}

static QString trailingSlash( int _trailing, const QString &path )
{
  QString result = path;

  if ( _trailing == 0 )
    return result;
  else if ( _trailing == 1 )
  {
    int len = result.length();
    if ( (len == 0) || (result[ len - 1 ] != QLatin1Char('/')) )
      result += QLatin1Char('/');
    return result;
  }
  else if ( _trailing == -1 )
  {
    if ( result == "/" )
      return result;
    int len = result.length();
    while (len > 1 && result[ len - 1 ] == QLatin1Char('/'))
    {
      len--;
    }
    result.truncate( len );
    return result;
  }
  else {
    assert( 0 );
    return QString::null;
  }
}

void KURL::adjustPath( int _trailing )
{
#if 0
  if (!m_strPath_encoded.isEmpty())
  {
     m_strPath_encoded = trailingSlash( _trailing, m_strPath_encoded );
  }
#endif
  const QString newPath = trailingSlash( _trailing, path() );
  if ( path() != newPath )
      setPath( newPath );
}


QString KURL::encodedPathAndQuery( int _trailing, bool _no_empty_path ) const
{
  QString tmp;
#if 0
  if (!m_strPath_encoded.isEmpty())
  {
     tmp = trailingSlash( _trailing, m_strPath_encoded );
  }
  else
#endif
  {
     tmp = path( _trailing );
     if ( _no_empty_path && tmp.isEmpty() )
        tmp = "/";
#if 0
     if (m_iUriMode == Mailto)
     {
        tmp = encode( tmp, 2 );
     }
     else
     {
        tmp = encode( tmp, 1 );
     }
#endif
     tmp = QLatin1String( QUrl::toPercentEncoding( tmp ) );
  }

  if (!fragment().isNull())
      tmp += '?' + fragment();
  return tmp;
}

#if 0
void KURL::setEncodedPath( const QString& _txt, int encoding_hint )
{
  m_strPath_encoded = _txt;

  decode( m_strPath_encoded, m_strPath, m_strPath_encoded, encoding_hint );
  // Throw away encoding for local files, makes file-operations faster.
  if (m_strProtocol == fileProt)
     m_strPath_encoded = QString::null;

  if ( m_iUriMode == Auto )
    m_iUriMode = URL;
}
#endif

void KURL::setEncodedPathAndQuery( const QString& _txt )
{
  int pos = _txt.indexOf( '?' );
  if ( pos == -1 )
  {
    setPath( QUrl::fromPercentEncoding( _txt.toLatin1() ) );
    setEncodedQuery( QByteArray() );
  }
  else
  {
    setPath( QUrl::fromPercentEncoding( _txt.toLatin1() ).left( pos ) );
    _setQuery( _txt.right( _txt.length() - pos - 1 ) );
  }
}

QString KURL::path( int _trailing ) const
{
  return trailingSlash( _trailing, path() );
}

bool KURL::isLocalFile() const
{
  if ( ( scheme() != fileProt ) || hasSubURL() )
     return false;

  if (host().isEmpty() || (host() == QLatin1String("localhost")))
     return true;

  char hostname[ 256 ];
  hostname[ 0 ] = '\0';
  if (!gethostname( hostname, 255 ))
     hostname[sizeof(hostname)-1] = '\0';

  for(char *p = hostname; *p; p++)
     *p = tolower(*p);

  return (host() == hostname);
}

void KURL::setFileEncoding(const QString &encoding)
{
  if (!isLocalFile())
     return;

  QString q = query();

  if (!q.isEmpty() && (q[0] == '?'))
     q = q.mid(1);

  QStringList args = q.split('&', QString::SkipEmptyParts);
  for(QStringList::Iterator it = args.begin();
      it != args.end();)
  {
      QString s = QUrl::fromPercentEncoding( (*it).toLatin1() );
      if (s.startsWith("charset="))
         it = args.erase(it);
      else
         ++it;
  }
  if (!encoding.isEmpty())
      args.append("charset=" + QUrl::toPercentEncoding(encoding));

  if (args.isEmpty())
     _setQuery(QString::null);
  else
     _setQuery(args.join("&"));
}

QString KURL::fileEncoding() const
{
  if (!isLocalFile())
     return QString::null;

  QString q = query();

  if (q.isEmpty())
     return QString::null;

  if (q[0] == '?')
     q = q.mid(1);

  QStringList args = q.split('&', QString::SkipEmptyParts);
  for(QStringList::ConstIterator it = args.begin();
      it != args.end();
      ++it)
  {
      QString s = QUrl::fromPercentEncoding((*it).toLatin1());
      if (s.startsWith("charset="))
         return s.mid(8);
  }
  return QString::null;
}

bool KURL::hasSubURL() const
{
  if ( scheme().isEmpty() || !isValid() )
    return false;
  const QString ref = fragment();
  if (ref.isEmpty())
     return false;
  if (ref.startsWith("gzip:"))
     return true;
  if (ref.startsWith("bzip:"))
     return true;
  if (ref.startsWith("bzip2:"))
     return true;
  if (ref.startsWith("tar:"))
     return true;
  if (ref.startsWith("ar:"))
     return true;
  if (ref.startsWith("zip:"))
     return true;
  if ( scheme() == "error" ) // anything that starts with error: has suburls
     return true;
  return false;
}

QString KURL::url( int _trailing ) const
{
  if ( _trailing == +1 && !path().endsWith( QLatin1Char('/') ) ) {
      // -1 and 0 are provided by QUrl, but not +1, so that one is a bit tricky.
      // To avoid reimplementing toString() all over again, I just use another QUrl
      // Let's hope this is fast, or not called often...
      QUrl newUrl( *this );
      newUrl.setPath( path() + QLatin1Char('/') );
      return QLatin1String( newUrl.toEncoded() ); // ### check
  }
  return QLatin1String( toEncoded( _trailing == -1 ? StripTrailingSlash : None ) ); // ## check encoding
}

QString KURL::prettyURL( int _trailing ) const
{
  // Can't use toString(), it breaks urls with %23 in them (becomes '#', which is parsed back as a fragment)
  // So prettyURL is just url, with the password removed.
  if ( password().isEmpty() )
    return url( _trailing );

  QUrl newUrl( *this );
  newUrl.setPassword( QString::null );
  if ( _trailing == +1 && !path().endsWith( QLatin1Char('/') ) ) {
      // -1 and 0 are provided by QUrl, but not +1.
      newUrl.setPath( path() + QLatin1Char('/') );
      return QLatin1String( newUrl.toEncoded() );
  }
  return QLatin1String( newUrl.toEncoded(  _trailing == -1 ? StripTrailingSlash : None ) ); // ## check encoding
}

#if 0
QString KURL::prettyURL( int _trailing, AdjustementFlags _flags) const
{
  QString u = prettyURL(_trailing);
  if (_flags & StripFileProtocol && u.startsWith("file://")) {
    u.remove(0, 7);
#ifdef Q_WS_WIN
    return QDir::convertSeparators(u);
#endif
  }
  return u;
}
#endif

QString KURL::pathOrURL() const
{
  if ( isLocalFile() && fragment().isNull() && encodedQuery().isNull() ) {
    return path();
  } else {
    return prettyURL();
  }
}

QString KURL::htmlURL() const
{
  return Q3StyleSheet::escape(prettyURL());
}

QString KURL::toMimeDataString() const // don't fold this into populateMimeData, it's also needed by other code like konqdrag
{
  if ( isLocalFile() )
  {
#if 1
//    return url(0, KGlobal::locale()->fileEncodingMib());
// Can't do that anymore with QUrl....
//      return url( 0, QTextCodec::codecForLocale()->mibEnum() );
    return url();
#else
    // According to the XDND spec, file:/ URLs for DND must have
    // the hostname part. But in really it just breaks many apps,
    // so it's disabled for now.
    const QString s = url( 0, KGlobal::locale()->fileEncodingMib() );
    if( !s.startsWith( "file://" ))
    {
        char hostname[257];
        if ( gethostname( hostname, 255 ) == 0 )
        {
            hostname[256] = '\0';
            return QString( "file://" ) + hostname + s.mid( 5 );
        }
    }
#endif
  }

  if ( protocol() == "mailto" ) {
      return path();
  }

  return url(0 /*, 106*/); // 106 is mib enum for utf8 codec
}

KURL KURL::fromMimeDataByteArray( const QByteArray& str )
{
  if ( str.startsWith( "file:" ) )
    return KURL( str /*, QTextCodec::codecForLocale()->mibEnum()*/ );

  return KURL( str /*, 106*/ ); // 106 is mib enum for utf8 codec;
}

KURL::List KURL::split( const KURL& _url )
{
  QString ref;
  KURL::List lst;
  KURL url = _url;

  while(true)
  {
     KURL u = url;
     u.setFragment( QString::null );
     lst.append(u);
     if (url.hasSubURL())
     {
        url = KURL(url.fragment());
     }
     else
     {
        ref = url.fragment();
        break;
     }
  }

  // Set HTML ref in all URLs.
  KURL::List::Iterator it;
  for( it = lst.begin() ; it != lst.end(); ++it )
  {
     (*it).setFragment( ref );
  }

  return lst;
}

KURL::List KURL::split( const QString& _url )
{
  return split(KURL(_url));
}

KURL KURL::join( const KURL::List & lst )
{
  if (lst.isEmpty()) return KURL();
  KURL tmp;


  bool first = true;
  QListIterator<KURL> it(lst);
  it.toBack();
  while (it.hasPrevious())
  {
     KURL u(it.previous());
     if (!first)
     {
        if (u.fragment().isNull())
            u.setFragment( tmp.url() );
        else
            u.setFragment( u.fragment() + '#' + tmp.url() ); // Support more than one suburl thingy
     }
     tmp = u;

     first = false;
  }

  return tmp;
}

QString KURL::fileName( bool _strip_trailing_slash ) const
{
  QString fname;
  if (hasSubURL()) { // If we have a suburl, then return the filename from there
    KURL::List list = KURL::split(*this);
    return list.last().fileName(_strip_trailing_slash);
  }
  const QString path = this->path();

  int len = path.length();
  if ( len == 0 )
    return fname;

  if ( _strip_trailing_slash )
  {
    while ( len >= 1 && path[ len - 1 ] == QLatin1Char('/') )
      len--;
  }
  else if ( path[ len - 1 ] == QLatin1Char('/') )
    return fname;

  // Does the path only consist of '/' characters ?
  if ( len == 1 && path[ 0 ] == QLatin1Char('/') )
    return fname;

  // Skip last n slashes
  int n = 1;
#if 0
  if (!m_strPath_encoded.isEmpty())
  {
     // This is hairy, we need the last unencoded slash.
     // Count in the encoded string how many encoded slashes follow the last
     // unencoded one.
     int i = m_strPath_encoded.lastIndexOf( QLatin1Char('/'), len - 1 );
     QString fileName_encoded = m_strPath_encoded.mid(i+1);
     n += fileName_encoded.count("%2f", Qt::CaseInsensitive);
  }
#endif
  int i = len;
  do {
    i = path.lastIndexOf( QLatin1Char('/'), i - 1 );
  }
  while (--n && (i > 0));

  // If ( i == -1 ) => the first character is not a '/'
  // So it's some URL like file:blah.tgz, return the whole path
  if ( i == -1 ) {
    if ( len == (int)path.length() )
      fname = path;
    else
      // Might get here if _strip_trailing_slash is true
      fname = path.left( len );
  }
  else
  {
     fname = path.mid( i + 1, len - i - 1 ); // TO CHECK
  }
  return fname;
}

void KURL::addPath( const QString& _txt )
{
  if (hasSubURL())
  {
     KURL::List lst = split( *this );
     KURL &u = lst.last();
     u.addPath(_txt);
     *this = join( lst );
     return;
  }

  //m_strPath_encoded = QString::null;

  if ( _txt.isEmpty() )
    return;

  QString strPath = path();
  int i = 0;
  int len = strPath.length();
  // Add the trailing '/' if it is missing
  if ( _txt[0] != QLatin1Char('/') && ( len == 0 || strPath[ len - 1 ] != QLatin1Char('/') ) )
    strPath += QLatin1Char('/');

  // No double '/' characters
  i = 0;
  const int _txtlen = _txt.length();
  if ( strPath.endsWith( QLatin1Char('/') ) )
  {
    while ( ( i < _txtlen ) && ( _txt[i] == QLatin1Char('/') ) )
      ++i;
  }

  setPath( strPath + _txt.mid( i ) );
  //kdDebug(126)<<"addPath: resultpath="<<path()<<endl;
}

QString KURL::directory( bool _strip_trailing_slash_from_result,
                         bool _ignore_trailing_slash_in_path ) const
{
  QString result = path(); //m_strPath_encoded.isEmpty() ? m_strPath : m_strPath_encoded;
  if ( _ignore_trailing_slash_in_path )
    result = trailingSlash( -1, result );

  if ( result.isEmpty() || result == "/" )
    return result;

  int i = result.lastIndexOf( "/" );
  // If ( i == -1 ) => the first character is not a '/'
  // So it's some URL like file:blah.tgz, with no path
  if ( i == -1 )
    return QString::null;

  if ( i == 0 )
  {
    result = "/";
    return result;
  }

  if ( _strip_trailing_slash_from_result )
    result = result.left( i );
  else
    result = result.left( i + 1 );

  //if (!m_strPath_encoded.isEmpty())
  //  result = decode(result);

  return result;
}


bool KURL::cd( const QString& _dir )
{
  if ( _dir.isEmpty() || !isValid() )
    return false;

  if (hasSubURL())
  {
     KURL::List lst = split( *this );
     KURL &u = lst.last();
     u.cd(_dir);
     *this = join( lst );
     return true;
  }

  // absolute path ?
  if ( _dir[0] == QLatin1Char('/') )
  {
    //m_strPath_encoded = QString::null;
    setPath( _dir );
    setHTMLRef( QString::null );
    setEncodedQuery( QByteArray() );
    return true;
  }

  // Users home directory on the local disk ?
  if ( ( _dir[0] == '~' ) && ( scheme() == fileProt ))
  {
    //m_strPath_encoded = QString::null;
    QString strPath = QDir::homePath();
    strPath += QLatin1Char('/');
    strPath += _dir.right( strPath.length() - 1 );
    setPath( strPath );
    setHTMLRef( QString::null );
    setEncodedQuery( QByteArray() );
    return true;
  }

  // relative path
  // we always work on the past of the first url.
  // Sub URLs are not touched.

  // append '/' if necessary
  QString p = path(1);
  p += _dir;
  p = cleanpath( p, true, false );
  setPath( p );

  setHTMLRef( QString::null );
  setEncodedQuery( QByteArray() );

  return true;
}

KURL KURL::upURL( ) const
{
  if (!encodedQuery().isEmpty())
  {
     KURL u(*this);
     u.setEncodedQuery(QByteArray());
     return u;
  };

  if (!hasSubURL())
  {
     KURL u(*this);

     u.cd("../");

     return u;
  }

  // We have a subURL.
  KURL::List lst = split( *this );
  if (lst.isEmpty())
      return KURL(); // Huh?
  while (true)
  {
     KURL &u = lst.last();
     QString old = u.path();
     u.cd("../");
     if (u.path() != old)
         break; // Finshed.
     if (lst.count() == 1)
         break; // Finished.
     lst.removeLast();
  }
  return join( lst );
}

QString KURL::htmlRef() const
{
  if ( !hasSubURL() )
  {
      return QUrl::fromPercentEncoding( ref().toLatin1() );
  }

  List lst = split( *this );
  return QUrl::fromPercentEncoding( (*lst.begin()).ref().toLatin1() );
}

QString KURL::encodedHtmlRef() const
{
  if ( !hasSubURL() )
  {
    return ref();
  }

  List lst = split( *this );
  return (*lst.begin()).ref();
}

void KURL::setHTMLRef( const QString& _ref )
{
  if ( !hasSubURL() )
  {
    setFragment( _ref );
    return;
  }

  List lst = split( *this );

  (*lst.begin()).setFragment( _ref );

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

void KURL::setDirectory( const QString &dir)
{
  if ( dir.endsWith("/"))
     setPath(dir);
  else
     setPath(dir+"/");
}

void KURL::setQuery( const QString &_txt )
{
   if (!_txt.isEmpty() && _txt[0] == '?')
      _setQuery( _txt.length() > 1 ? _txt.mid(1) : "" /*empty, not null*/ );
   else
      _setQuery( _txt );
}

void KURL::_setQuery( const QString& query )
{
    setEncodedQuery( query.isNull() ? 0 : query.ascii() ); // ### TODO encoding ok?
}

QString KURL::query() const
{
    if (encodedQuery().isNull())
        return QString::null;
    return QString( QChar( '?' ) ) + QString::fromAscii( encodedQuery() );
}

bool urlcmp( const QString& _url1, const QString& _url2 )
{
  return QUrl( _url1, QUrl::TolerantMode ) == QUrl( _url2, QUrl::TolerantMode );
#if 0
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
#endif
}

bool urlcmp( const QString& _url1, const QString& _url2, bool _ignore_trailing, bool _ignore_ref )
{
    QUrl u1( _url1 );
    QUrl u2( _url2 );
    QUrl::FormattingOptions options = QUrl::None;
    if ( _ignore_trailing )
        options |= QUrl::StripTrailingSlash;
    if ( _ignore_ref )
        options |= QUrl::RemoveFragment;
    return u1.toString( options ) == u2.toString( options );

#if 0
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

  int size = list1.count();
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
    if ( !(*it1).equals( *it2, _ignore_trailing ) )
      return false;
  return true;
#endif
}

QMap< QString, QString > KURL::queryItems( int options ) const {
  QMap< QString, QString > result;
  const QList<QPair<QString, QString> > items = QUrl::queryItems();
  QPair<QString, QString> item;
  Q_FOREACH( item, items ) {
      result.insert( options & CaseInsensitiveKeys ? item.first.toLower() : item.first, item.second );
  }
#if 0
  if ( m_strQuery_encoded.isEmpty() )
    return QMap<QString,QString>();

  QMap< QString, QString > result;
  QStringList items = m_strQuery_encoded.split( '&', QString::SkipEmptyParts );
  for ( QStringList::const_iterator it = items.begin() ; it != items.end() ; ++it ) {
    int equal_pos = (*it).indexOf( '=' );
    if ( equal_pos > 0 ) { // = is not the first char...
      QString name = (*it).left( equal_pos );
      if ( options & CaseInsensitiveKeys )
	name = name.toLower();
      QString value = (*it).mid( equal_pos + 1 );
      if ( value.isEmpty() )
	result.insert( name, QLatin1String("") );
      else {
	// ### why is decoding name not necessary?
	value.replace( '+', ' ' ); // + in queries means space
	result.insert( name, decode_string( value, encoding_hint ) );
      }
    } else if ( equal_pos < 0 ) { // no =
      QString name = (*it);
      if ( options & CaseInsensitiveKeys )
	name = name.toLower();
      result.insert( name, QString::null );
    }
  }
#endif
  return result;
}

// static
KURL KURL::fromPathOrURL( const QString& text )
{
    KURL url;
    if ( !text.isEmpty() )
    {
        if (!QDir::isRelativePath(text))
            url.setPath( text );
        else
            url = text;
    }

    return url;
}

static QString _relativePath(const QString &base_dir, const QString &path, bool &isParent)
{
   QString _base_dir(QDir::cleanPath(base_dir));
   QString _path(QDir::cleanPath(path.isEmpty() || (path[0] != QLatin1Char('/')) ? _base_dir+"/"+path : path));

   if (_base_dir.isEmpty())
      return _path;

   if (_base_dir[_base_dir.length()-1] != QLatin1Char('/'))
      _base_dir.append(QLatin1Char('/') );

   QStringList list1 = _base_dir.split(QLatin1Char('/'), QString::SkipEmptyParts);
   QStringList list2 = _path.split(QLatin1Char('/'), QString::SkipEmptyParts);

   // Find where they meet
   int level = 0;
   int maxLevel = qMin(list1.count(), list2.count());
   while((level < maxLevel) && (list1[level] == list2[level])) level++;

   QString result;
   // Need to go down out of the first path to the common branch.
   for(int i = level; i < list1.count(); i++)
      result.append("../");

   // Now up up from the common branch to the second path.
   for(int i = level; i < list2.count(); i++)
      result.append(list2[i]).append("/");

   if ((level < list2.count()) && (path[path.length()-1] != QLatin1Char('/')))
      result.truncate(result.length()-1);

   isParent = (level == list1.count());

   return result;
}

QString KURL::relativePath(const QString &base_dir, const QString &path, bool *isParent)
{
   bool parent = false;
   QString result = _relativePath(base_dir, path, parent);
   if (parent)
      result.prepend("./");

   if (isParent)
      *isParent = parent;

   return result;
}


QString KURL::relativeURL(const KURL &base_url, const KURL &url)
{
   if ((url.protocol() != base_url.protocol()) ||
       (url.host() != base_url.host()) ||
       (url.port() && url.port() != base_url.port()) ||
       (url.hasUser() && url.user() != base_url.user()) ||
       (url.hasPass() && url.pass() != base_url.pass()))
   {
      return url.url(0);
   }

   QString relURL;

   if ((base_url.path() != url.path()) || (base_url.query() != url.query()))
   {
      bool dummy;
      QString basePath = base_url.directory(false, false);
      relURL = _relativePath(basePath, url.path(), dummy); // was QUrl::toPercentEncoding() but why?
      relURL += url.query();
   }

   if ( url.hasRef() )
   {
      relURL += "#";
      relURL += url.ref();
   }

   if ( relURL.isEmpty() )
      return "./";

   return relURL;
}

void KURL::setPath( const QString& path )
{
    if ( scheme().isEmpty() )
        setScheme( "file" );
    QUrl::setPath( path );
}
