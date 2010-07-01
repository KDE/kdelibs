/*
    Copyright (C) 1999 Torben Weis <weis@kde.org>
    Copyright (C) 2005-2006 David Faure <faure@kde.org>

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

#include "kurl.h"

#include <kdebug.h>
#include <kglobal.h>
#include <kshell.h>
#include <kstringhandler.h>

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include <QtCore/QDir>
#include <QtCore/QMutableStringListIterator>
#include <QtCore/QRegExp>
#include <QtCore/QMimeData>
#include <QtCore/QTextCodec>

#ifdef DEBUG_KURL
static int kurlDebugArea() { static int s_area = KDebug::registerArea("kdecore (KUrl)"); return s_area; }
#endif

static QString cleanpath( const QString &_path, bool cleanDirSeparator, bool decodeDots )
{
  if (_path.isEmpty())
      return QString();

  if (QFileInfo(_path).isRelative())
     return _path; // Don't mangle mailto-style URLs

  QString path = _path;

  int len = path.length();

  if (decodeDots)
  {
     static const QString &encodedDot = KGlobal::staticQString("%2e");
     if (path.indexOf(encodedDot, 0, Qt::CaseInsensitive) != -1)
     {
        static const QString &encodedDOT = KGlobal::staticQString("%2E"); // Uppercase!
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
  if (orig_pos >= 2 && path[0].isLetter() && path[1] == QLatin1Char(':') ) {
    result.prepend(QString(path[0]) + QLatin1Char(':') );
  }
#endif

  if ( result.isEmpty() )
    result = '/';
  else if ( slash && result[result.length()-1] != QLatin1Char('/') )
       result.append(QChar('/'));

  return result;
}

#ifdef Q_WS_WIN

// returns true if provided arguments desinate letter+colon or double slash
#define IS_DRIVE_OR_DOUBLESLASH(isletter, char1, char2, colon, slash) \
  ((isletter && char2 == colon) || (char1 == slash && char2 == slash))

// Removes file:/// or file:// or file:/ or / prefix assuming that str
// is (nonempty) Windows absolute path with a drive letter or double slash.
// If there was file protocol, the path is decoded from percent encoding
static QString removeSlashOrFilePrefix(const QString& str)
{
  // FIXME this should maybe be replaced with some (faster?)/nicer logic
  const int len = str.length();
  if (str[0]=='f') {
    if ( len > 10 && str.startsWith( QLatin1String( "file:///" ) )
         && IS_DRIVE_OR_DOUBLESLASH(str[8].isLetter(), str[8], str[9], QLatin1Char(':'), QLatin1Char('/')) )
      return QUrl::fromPercentEncoding( str.toLatin1() ).mid(8);
    else if ( len > 9 && str.startsWith( QLatin1String( "file://" ) )
              && IS_DRIVE_OR_DOUBLESLASH(str[7].isLetter(), str[7], str[8], QLatin1Char(':'), QLatin1Char('/')) )
      return QUrl::fromPercentEncoding( str.toLatin1() ).mid(7);
    else if ( len > 8 && str.startsWith( QLatin1String( "file:/" ) )
              && IS_DRIVE_OR_DOUBLESLASH(str[6].isLetter(), str[6], str[7], QLatin1Char(':'), QLatin1Char('/')) )
      return QUrl::fromPercentEncoding( str.toLatin1() ).mid(6);
  }
  /* No 'else' here since there can be "f:/" path. */

  /* '/' + drive letter or // */
  if ( len > 2 && str[0] == QLatin1Char('/')
       && IS_DRIVE_OR_DOUBLESLASH(str[1].isLetter(), str[1], str[2], QLatin1Char(':'), QLatin1Char('/')) )
    return str.mid(1);
  /* drive letter or // */
  else if ( len >= 2 && IS_DRIVE_OR_DOUBLESLASH(str[0].isLetter(), str[0], str[1], QLatin1Char(':'), QLatin1Char('/')) )
    return str;
  return QString();
}
#endif

bool KUrl::isRelativeUrl(const QString &_url)
{
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

KUrl::List::List(const KUrl &url)
{
    append( url );
}

KUrl::List::List(const QList<KUrl> &list)
    : QList<KUrl>(list)
{
}

KUrl::List::List(const QStringList &list)
{
  for (QStringList::ConstIterator it = list.begin();
       it != list.end();
       ++it)
    {
      append( KUrl(*it) );
    }
}

QStringList KUrl::List::toStringList() const
{
   QStringList lst;
   for( KUrl::List::ConstIterator it = begin();
        it != end();
        ++it)
   {
      lst.append( (*it).url() );
   }
   return lst;
}

static QByteArray uriListData(const KUrl::List& urls)
{
    QList<QByteArray> urlStringList;
    KUrl::List::ConstIterator uit = urls.constBegin();
    const KUrl::List::ConstIterator uEnd = urls.constEnd();
    for (; uit != uEnd ; ++uit) {
        // Get each URL encoded in utf8 - and since we get it in escaped
        // form on top of that, .toLatin1() is fine.
        urlStringList.append((*uit).toMimeDataString().toLatin1());
    }

    QByteArray uriListData;
    for (int i = 0, n = urlStringList.count(); i < n; ++i) {
      uriListData += urlStringList.at(i);
        if (i < n-1)
          uriListData += "\r\n";
    }
    return uriListData;
}

static const char s_kdeUriListMime[] = "application/x-kde4-urilist";

void KUrl::List::populateMimeData( QMimeData* mimeData,
                                   const KUrl::MetaDataMap& metaData,
                                   MimeDataFlags flags ) const
{
    mimeData->setData("text/uri-list", uriListData(*this));

    if ( ( flags & KUrl::NoTextExport ) == 0 )
    {
        QStringList prettyURLsList;
        KUrl::List::ConstIterator uit = constBegin();
        const KUrl::List::ConstIterator uEnd = constEnd();
        for ( ; uit != uEnd ; ++uit ) {
            QString prettyURL = (*uit).prettyUrl();
            if ( (*uit).protocol() == "mailto" ) {
                prettyURL = (*uit).path(); // remove mailto: when pasting into konsole
            }
            prettyURLsList.append( prettyURL );
        }

        QByteArray plainTextData = prettyURLsList.join( "\n" ).toLocal8Bit();
        if( count() > 1 ) // terminate last line, unless it's the only line
            plainTextData.append( "\n" );
        mimeData->setData( "text/plain", plainTextData );
    }

    if ( !metaData.isEmpty() )
    {
        QByteArray metaDataData; // :)
        for( KUrl::MetaDataMap::const_iterator it = metaData.begin(); it != metaData.end(); ++it )
        {
            metaDataData += it.key().toUtf8();
            metaDataData += "$@@$";
            metaDataData += it.value().toUtf8();
            metaDataData += "$@@$";
        }
        mimeData->setData( "application/x-kio-metadata", metaDataData );
    }
}


void KUrl::List::populateMimeData(const KUrl::List& mostLocalUrls,
                                  QMimeData* mimeData,
                                  const KUrl::MetaDataMap& metaData,
                                  MimeDataFlags flags) const
{
    // Export the most local urls as text/uri-list and plain text.
    mostLocalUrls.populateMimeData(mimeData, metaData, flags);

    mimeData->setData(s_kdeUriListMime, uriListData(*this));
}

bool KUrl::List::canDecode( const QMimeData *mimeData )
{
    return mimeData->hasFormat("text/uri-list") ||
        mimeData->hasFormat(s_kdeUriListMime);
}

QStringList KUrl::List::mimeDataTypes()
{
    return QStringList() << s_kdeUriListMime << "text/uri-list";
}


KUrl::List KUrl::List::fromMimeData(const QMimeData *mimeData,
                                    DecodeOptions decodeOptions,
                                    KUrl::MetaDataMap* metaData)
{

    KUrl::List uris;
    const char* firstMimeType = s_kdeUriListMime;
    const char* secondMimeType = "text/uri-list";
    if (decodeOptions == PreferLocalUrls) {
        qSwap(firstMimeType, secondMimeType);
    }
    QByteArray payload = mimeData->data(firstMimeType);
    if (payload.isEmpty())
        payload = mimeData->data(secondMimeType);
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
                uris.append( KUrl::fromMimeDataByteArray( s ) );
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
            QString str = QString::fromUtf8( metaDataPayload );
            Q_ASSERT( str.endsWith( "$@@$" ) );
            str.truncate( str.length() - 4 );
            const QStringList lst = str.split( "$@@$" );
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

KUrl::List KUrl::List::fromMimeData( const QMimeData *mimeData, KUrl::MetaDataMap* metaData )
{
    return fromMimeData(mimeData, PreferKdeUrls, metaData);
}

KUrl::List::operator QVariant() const
{
  return qVariantFromValue(*this);
}

///

KUrl::KUrl()
    : QUrl(), d(0)
{
}

KUrl::~KUrl()
{
}


KUrl::KUrl( const QString &str )
  : QUrl(), d(0)
{
  if ( !str.isEmpty() ) {
#ifdef Q_WS_WIN
#ifdef DEBUG_KURL
    kDebug(kurlDebugArea()) << "KUrl::KUrl ( const QString &str = " << str.toAscii().data() << " )";
#endif
    QString pathToSet;
    // when it starts with file:// it's a url and must be valid. we don't care if the
    // path exist/ is valid or not
    if (!str.startsWith(QLatin1String("file://")))
      pathToSet = removeSlashOrFilePrefix( QDir::fromNativeSeparators(str) );
    if ( !pathToSet.isEmpty() ) {
      // we have a prefix indicating this is a local URL
      // remember the possible query using _setEncodedUrl(), then set up the correct path without query protocol part
      int index = pathToSet.lastIndexOf('?');
      if (index == -1)
        setPath( pathToSet );
      else {
        setPath( pathToSet.left( index ) );
        _setQuery( pathToSet.mid( index + 1 ) );
      }
      return;
    }
#endif
    if ( str[0] == QLatin1Char('/') || str[0] == QLatin1Char('~') )
      setPath( str );
    else {
      _setEncodedUrl( str.toUtf8() );
    }
  }
}

KUrl::KUrl( const char * str )
  : QUrl(), d(0)
{
#ifdef Q_WS_WIN
  // true if @a c is letter
  #define IS_LETTER(c) \
    ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))

  // like IS_DRIVE_OR_DOUBLESLASH, but slash is prepended
  #define IS_SLASH_AND_DRIVE_OR_DOUBLESLASH_0 \
    ( str[0] == '/' && IS_DRIVE_OR_DOUBLESLASH(IS_LETTER(str[1]), str[1], str[2], ':', '/') )

  // like IS_DRIVE_OR_DOUBLESLASH, with characters == str[0] and str[1]
  #define IS_DRIVE_OR_DOUBLESLASH_0 \
    ( IS_DRIVE_OR_DOUBLESLASH(IS_LETTER(str[0]), str[0], str[1], ':', '/') )

#if defined(DEBUG_KURL)
  kDebug(kurlDebugArea()) << "KUrl::KUrl " << " " << str;
#endif
  if ( str && str[0] && str[1] && str[2] ) {
    if ( IS_SLASH_AND_DRIVE_OR_DOUBLESLASH_0 )
      setPath( QString::fromUtf8( str+1 ) );
    else if ( IS_DRIVE_OR_DOUBLESLASH_0 )
      setPath( QString::fromUtf8( str ) );
  }
#endif
  if ( str && str[0] ) {
    if ( str[0] == '/' || str[0] == '~' )
      setPath( QString::fromUtf8( str ) );
    else
      _setEncodedUrl( str );
  }
}

KUrl::KUrl( const QByteArray& str )
   : QUrl(), d(0)
{
  if ( !str.isEmpty() ) {
#ifdef Q_WS_WIN
#ifdef DEBUG_KURL
    kDebug(kurlDebugArea()) << "KUrl::KUrl " << " " << str.data();
#endif
    if ( IS_SLASH_AND_DRIVE_OR_DOUBLESLASH_0 )
      setPath( QString::fromUtf8( str.mid( 1 ) ) );
    else if ( IS_DRIVE_OR_DOUBLESLASH_0 )
      setPath( QString::fromUtf8( str ) );
#else
    if ( str[0] == '/' || str[0] == '~' )
      setPath( QString::fromUtf8( str ) );
#endif
    else
      _setEncodedUrl( str );
  }
}

KUrl::KUrl( const KUrl& _u )
    : QUrl( _u ), d(0)
{
#if defined(Q_WS_WIN) && defined(DEBUG_KURL)
    kDebug(kurlDebugArea()) << "KUrl::KUrl(KUrl) " << " path " << _u.path() << " toLocalFile " << _u.toLocalFile();
#endif
}

KUrl::KUrl( const QUrl &u )
    : QUrl( u ), d(0)
{
#if defined(Q_WS_WIN) && defined(DEBUG_KURL)
    kDebug(kurlDebugArea()) << "KUrl::KUrl(Qurl) " << " path " << u.path() << " toLocalFile " << u.toLocalFile();
#endif
}

KUrl::KUrl( const KUrl& _u, const QString& _rel_url )
   : QUrl(), d(0)
{
#if defined(Q_WS_WIN) && defined(DEBUG_KURL)
    kDebug(kurlDebugArea()) << "KUrl::KUrl(KUrl,QString rel_url) " << " path " << _u.path() << " toLocalFile " << _u.toLocalFile();
#endif
#if 0
  if (_u.hasSubUrl()) // Operate on the last suburl, not the first
  {
    KUrl::List lst = split( _u );
    KUrl u(lst.last(), _rel_url);
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
    QByteArray strRef_encoded = rUrl.mid(1).toLatin1();
    if ( strRef_encoded.isNull() )
        strRef_encoded = ""; // we know there was an (empty) html ref, we saw the '#'
    setEncodedFragment( strRef_encoded );
  }
  else if ( isRelativeUrl( rUrl ) )
  {
    *this = _u;
    setFragment( QString() );
    setEncodedQuery( QByteArray() );
    QString strPath = path();
    if ( rUrl[0] == QLatin1Char('/') )
    {
        if ((rUrl.length() > 1) && (rUrl[1] == QLatin1Char('/')))
        {
            setHost( QString() );
            setPort( -1 );
            // File protocol returns file:/// without host, strip // from rUrl
            if ( _u.isLocalFile() )
                rUrl.remove(0, 2);
        }
        strPath.clear();
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
    //kDebug(kurlDebugArea()) << "url()=" << url() << " rUrl=" << rUrl;
    KUrl tmp( url() + rUrl);
    //kDebug(kurlDebugArea()) << "assigning tmp=" << tmp.url();
    *this = tmp;
    cleanPath(KeepDirSeparators);
  }
  else
  {
    KUrl tmp( rUrl );
    //kDebug(kurlDebugArea()) << "not relative; assigning tmp=" << tmp.url();
    *this = tmp;
    // Preserve userinfo if applicable.
    if (!_u.userInfo().isEmpty() && userInfo().isEmpty()
        && (_u.host() == host()) && (_u.scheme() == scheme()))
    {
       setUserInfo( _u.userInfo() );
    }
    cleanPath(KeepDirSeparators);
  }
}

KUrl& KUrl::operator=( const KUrl& _u )
{
  QUrl::operator=( _u );
  return *this;
}

bool KUrl::operator==( const KUrl& _u ) const
{
  return QUrl::operator==( _u );
}

bool KUrl::operator==( const QString& _u ) const
{
  KUrl u( _u );
  return ( *this == u );
}

KUrl::operator QVariant() const
{
  return qVariantFromValue(*this);
}

bool KUrl::cmp( const KUrl &u, bool ignore_trailing ) const
{
  return equals( u, ignore_trailing ? CompareWithoutTrailingSlash : EqualsOptions(0) );
}

bool KUrl::equals( const KUrl &_u, const EqualsOptions& options ) const
{
  if ( !isValid() || !_u.isValid() )
    return false;

  if ( options & CompareWithoutTrailingSlash || options & CompareWithoutFragment )
  {
    QString path1 = path((options & CompareWithoutTrailingSlash) ? RemoveTrailingSlash : LeaveTrailingSlash);
    QString path2 = _u.path((options & CompareWithoutTrailingSlash) ? RemoveTrailingSlash : LeaveTrailingSlash);

    if (options & AllowEmptyPath) {
        if (path1 == QLatin1String("/"))
            path1.clear();
        if (path2 == QLatin1String("/"))
            path2.clear();
    }

#ifdef Q_WS_WIN
    const bool bLocal1 = isLocalFile();
    const bool bLocal2 = _u.isLocalFile();
    if ( !bLocal1 && bLocal2 || bLocal1 && !bLocal2 )
      return false;
    // local files are case insensitive
    if ( bLocal1 && bLocal2 && 0 != QString::compare( path1, path2, Qt::CaseInsensitive ) )
      return false;
#endif
    if ( path1 != path2 )
      return false;

    if ( scheme() == _u.scheme() &&
         authority() == _u.authority() && // user+pass+host+port
         encodedQuery() == _u.encodedQuery() &&
         (fragment() == _u.fragment() || options & CompareWithoutFragment )    )
      return true;

    return false;
  }

  return ( *this == _u );
}

QString KUrl::protocol() const
{
    return scheme().toLower();
}

void KUrl::setProtocol( const QString& proto )
{
    setScheme( proto );
}

QString KUrl::user() const
{
    return userName();
}

void KUrl::setUser( const QString& user )
{
    setUserName( user );
}

bool KUrl::hasUser() const
{
    return !userName().isEmpty();
}

QString KUrl::pass() const
{
    return password();
}

void KUrl::setPass( const QString& pass )
{
    setPassword( pass );
}

bool KUrl::hasPass() const
{
    return !password().isEmpty();
}

bool KUrl::hasHost() const
{
    return !host().isEmpty();
}

bool KUrl::hasPath() const
{
    return !path().isEmpty();
}

KUrl KUrl::fromPath( const QString& text )
{
    KUrl u;
    u.setPath( text );
    return u;
}

void KUrl::setFileName( const QString& _txt )
{
  setFragment( QString() );
  int i = 0;
  while( i < _txt.length() && _txt[i] == QLatin1Char('/') )
      ++i;
  QString tmp = i ? _txt.mid( i ) : _txt;

  QString path = this->path();
  if ( path.isEmpty() )
#ifdef Q_OS_WIN
    path = isLocalFile() ? QDir::rootPath() : QLatin1String("/");
#else
    path = QDir::rootPath();
#endif
  else
  {
    int lastSlash = path.lastIndexOf( QLatin1Char('/') );
    if ( lastSlash == -1)
      path.clear(); // there's only the file name, remove it
    else if ( !path.endsWith( QLatin1Char('/') ) )
      path.truncate( lastSlash+1 ); // keep the "/"
  }

  path += tmp;
  setPath( path );

  cleanPath();
}

void KUrl::cleanPath( const CleanPathOption& options )
{
  //if (m_iUriMode != URL) return;
  const QString newPath = cleanpath(path(), !(options & KeepDirSeparators), false);
  if ( path() != newPath )
      setPath( newPath );
  // WABA: Is this safe when "/../" is encoded with %?
  //m_strPath_encoded = cleanpath(m_strPath_encoded, cleanDirSeparator, true);
}

static QString trailingSlash( KUrl::AdjustPathOption trailing, const QString &path )
{
  if ( trailing == KUrl::LeaveTrailingSlash ) {
    return path;
  }

  QString result = path;

  if ( trailing == KUrl::AddTrailingSlash )
  {
    int len = result.length();
    if ( (len == 0) || (result[ len - 1 ] != QLatin1Char('/')) )
      result += QLatin1Char('/');
    return result;
  }
  else if ( trailing == KUrl::RemoveTrailingSlash )
  {
    if ( result == QLatin1String("/") )
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
    return result;
  }
}

void KUrl::adjustPath( AdjustPathOption trailing )
{
#if 0
  if (!m_strPath_encoded.isEmpty())
  {
     m_strPath_encoded = trailingSlash( _trailing, m_strPath_encoded );
  }
#endif
  const QString newPath = trailingSlash( trailing, path() );
  if ( path() != newPath )
      setPath( newPath );
}


QString KUrl::encodedPathAndQuery( AdjustPathOption trailing , const EncodedPathAndQueryOptions &options) const
{
    QString encodedPath;
#ifdef Q_OS_WIN
    // see KUrl::path()
    if (isLocalFile()) {
        // ### this is probably broken
        encodedPath = trailingSlash(trailing, QUrl::toLocalFile());
        encodedPath = QString::fromLatin1(QUrl::toPercentEncoding(encodedPath, "!$&'()*+,;=:@/"));
    } else {
        encodedPath = trailingSlash(trailing, QUrl::encodedPath());
    }
#else
    encodedPath = trailingSlash(trailing, QUrl::encodedPath());
#endif

    if ((options & AvoidEmptyPath) && encodedPath.isEmpty()) {
        encodedPath.append('/');
    }

    if (hasQuery()) {
        return encodedPath + '?' + encodedQuery();
    } else {
        return encodedPath;
    }
}

#if 0
void KUrl::setEncodedPath( const QString& _txt, int encoding_hint )
{
  m_strPath_encoded = _txt;

  decode( m_strPath_encoded, m_strPath, m_strPath_encoded, encoding_hint );
  // Throw away encoding for local files, makes file-operations faster.
  if (m_strProtocol == "file")
     m_strPath_encoded.clear();

  if ( m_iUriMode == Auto )
    m_iUriMode = URL;
}
#endif

void KUrl::setEncodedPathAndQuery( const QString& _txt )
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

QString KUrl::path( AdjustPathOption trailing ) const
{
#ifdef Q_WS_WIN
  kWarning() << (isLocalFile() ? "converted to local file - the related call should be converted to toLocalFile()" : "") << QUrl::path();
  return trailingSlash( trailing, isLocalFile() ? QUrl::toLocalFile() : QUrl::path() );
#else
  return trailingSlash( trailing, QUrl::path() );
#endif
}

QString KUrl::toLocalFile( AdjustPathOption trailing ) const
{
    if (hasHost() && isLocalFile()) {
        KUrl urlWithoutHost(*this);
        urlWithoutHost.setHost(QString());
        return trailingSlash(trailing, urlWithoutHost.toLocalFile());
    }
    return trailingSlash(trailing, QUrl::toLocalFile());
}

inline static bool hasSubUrl( const QUrl& url );

static inline bool isLocalFile( const QUrl& url )
{
  if ( ( url.scheme() != QLatin1String("file") ) || hasSubUrl( url ) )
     return false;

  if (url.host().isEmpty() || (url.host() == QLatin1String("localhost")))
     return true;

  char hostname[ 256 ];
  hostname[ 0 ] = '\0';
  if (!gethostname( hostname, 255 ))
     hostname[sizeof(hostname)-1] = '\0';

  for(char *p = hostname; *p; p++)
     *p = tolower(*p);

  return (url.host() == QString::fromLatin1( hostname ));
}

bool KUrl::isLocalFile() const
{
  return ::isLocalFile( *this );
}

void KUrl::setFileEncoding(const QString &encoding)
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
     _setQuery(QString());
  else
     _setQuery(args.join("&"));
}

QString KUrl::fileEncoding() const
{
  if (!isLocalFile())
     return QString();

  QString q = query();

  if (q.isEmpty())
     return QString();

  if (q[0] == '?')
     q = q.mid(1);

  const QStringList args = q.split('&', QString::SkipEmptyParts);
  for(QStringList::ConstIterator it = args.begin();
      it != args.end();
      ++it)
  {
      QString s = QUrl::fromPercentEncoding((*it).toLatin1());
      if (s.startsWith("charset="))
         return s.mid(8);
  }
  return QString();
}

inline static bool hasSubUrl( const QUrl& url )
{
  // The isValid call triggers QUrlPrivate::validate which needs the full encoded url,
  // all this takes too much time for isLocalFile()
  if ( url.scheme().isEmpty() /*|| !isValid()*/ )
    return false;
  const QByteArray ref( url.fragment().toLatin1() );
  if (ref.isEmpty())
     return false;
  switch ( ref.data()[0] ) {
  case 'g':
    if ( ref.startsWith("gzip:") )
      return true;
    break;
  case 'b':
    if ( ref.startsWith("bzip:") || ref.startsWith("bzip2:") )
      return true;
    break;
  case 'l':
    if ( ref.startsWith("lzma:") )
      return true;
    break;
  case 'x':
    if ( ref.startsWith("xz:") )
      return true;
    break;
  case 't':
    if ( ref.startsWith("tar:") )
      return true;
    break;
  case 'a':
    if ( ref.startsWith("ar:") )
      return true;
    break;
  case 'z':
    if ( ref.startsWith("zip:") )
      return true;
    break;
  default:
    break;
  }
  if ( url.scheme() == "error" ) // anything that starts with error: has suburls
     return true;
  return false;
}

bool KUrl::hasSubUrl() const
{
  return ::hasSubUrl( *this );
}

QString KUrl::url( AdjustPathOption trailing ) const
{
    if (QString::compare(scheme(), QLatin1String("mailto"), Qt::CaseInsensitive) == 0) {
        // mailto urls should be prettified, see the url183433 testcase.
        return prettyUrl(trailing);
    }
  if ( trailing == AddTrailingSlash && !path().endsWith( QLatin1Char('/') ) ) {
      // -1 and 0 are provided by QUrl, but not +1, so that one is a bit tricky.
      // To avoid reimplementing toEncoded() all over again, I just use another QUrl
      // Let's hope this is fast, or not called often...
      QUrl newUrl( *this );
      newUrl.setPath( path() + QLatin1Char('/') );
      return QString::fromLatin1(newUrl.toEncoded());
  }
  else if ( trailing == RemoveTrailingSlash && path() == "/" ) {
      return QLatin1String(toEncoded(None));
  }
  return QString::fromLatin1(toEncoded(trailing == RemoveTrailingSlash ? StripTrailingSlash : None));
}

static QString toPrettyPercentEncoding(const QString &input, bool forFragment)
{
  QString result;
  for (int i = 0; i < input.length(); ++i) {
    QChar c = input.at(i);
    register ushort u = c.unicode();
    if (u < 0x20
        || (!forFragment && u == '?') // don't escape '?' in fragments, not needed and wrong (#173101)
        || u == '#' || u == '%'
        || (u == ' ' && (i+1 == input.length() || input.at(i+1) == ' '))) {
      static const char hexdigits[] = "0123456789ABCDEF";
      result += QLatin1Char('%');
      result += QLatin1Char(hexdigits[(u & 0xf0) >> 4]);
      result += QLatin1Char(hexdigits[u & 0xf]);
    } else {
      result += c;
    }
  }

  return result;
}

QString KUrl::prettyUrl( AdjustPathOption trailing ) const
{
  // reconstruct the URL in a "pretty" form
  // a "pretty" URL is NOT suitable for data transfer. It's only for showing data to the user.
  // however, it must be parseable back to its original state, since
  // notably Konqueror displays it in the Location address.

  // A pretty URL is the same as a normal URL, except that:
  // - the password is removed
  // - the hostname is shown in Unicode (as opposed to ACE/Punycode)
  // - the pathname and fragment parts are shown in Unicode (as opposed to %-encoding)
  QString result = scheme();
  if (!result.isEmpty())
  {
    if(!authority().isEmpty() || result == QLatin1String("file"))
        result += QLatin1String("://");
    else
        result += QLatin1String(":");
  }

  QString tmp = userName();
  if (!tmp.isEmpty()) {
    result += QUrl::toPercentEncoding(tmp);
    result += QLatin1Char('@');
  }

  // Check if host is an ipv6 address
  tmp = host();
  if (tmp.contains(':'))
    result += QLatin1Char('[') + tmp + QLatin1Char(']');
  else
    result += tmp;

  if (port() != -1) {
    result += QLatin1Char(':');
    result += QString::number(port());
  }

  tmp = path();
  result += toPrettyPercentEncoding(tmp, false);

  // adjust the trailing slash, if necessary
  if (trailing == AddTrailingSlash && !tmp.endsWith(QLatin1Char('/')))
    result += QLatin1Char('/');
  else if (trailing == RemoveTrailingSlash && tmp.length() > 1 && tmp.endsWith(QLatin1Char('/')))
    result.chop(1);

  if (hasQuery()) {
    result += QLatin1Char('?');
    result += encodedQuery();
  }

  if (hasFragment()) {
    result += QLatin1Char('#');
    result += toPrettyPercentEncoding(fragment(), true);
  }

  return result;
}

#if 0
QString KUrl::prettyUrl( int _trailing, AdjustementFlags _flags) const
{
  QString u = prettyUrl(_trailing);
  if (_flags & StripFileProtocol && u.startsWith("file://")) {
    u.remove(0, 7);
#ifdef Q_WS_WIN
    return QDir::convertSeparators(u);
#endif
  }
  return u;
}
#endif

QString KUrl::pathOrUrl() const
{
    return pathOrUrl(LeaveTrailingSlash);
}

QString KUrl::pathOrUrl(AdjustPathOption trailing) const
{
  if ( isLocalFile() && fragment().isNull() && encodedQuery().isNull() ) {
    return toLocalFile(trailing);
  } else {
    return prettyUrl(trailing);
  }
}

// Used for text/uri-list in the mime data
QString KUrl::toMimeDataString() const // don't fold this into populateMimeData, it's also needed by other code like konqdrag
{
  if ( isLocalFile() )
  {
#if 1
    return url();
#else
    // According to the XDND spec, file:/ URLs for DND must have
    // the hostname part. But in really it just breaks many apps,
    // so it's disabled for now.
    const QString s = url( 0, KGlobal::locale()->fileEncodingMib() );
    if( !s.startsWith( QLatin1String ( "file://" ) ))
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

  if (hasPass()) {
    KUrl safeUrl(*this);
    safeUrl.setPassword(QString());
    return safeUrl.url();
  }
  return url();
}

KUrl KUrl::fromMimeDataByteArray( const QByteArray& str )
{
  if ( str.startsWith( "file:" ) )
    return KUrl( str /*, QTextCodec::codecForLocale()->mibEnum()*/ );

  return KUrl( str /*, 106*/ ); // 106 is mib enum for utf8 codec;
}

KUrl::List KUrl::split( const KUrl& _url )
{
  QString ref;
  bool hasRef;
  KUrl::List lst;
  KUrl url = _url;

  while(true)
  {
     KUrl u = url;
     u.setFragment( QString() );
     lst.append(u);
     if (url.hasSubUrl())
     {
        url = KUrl(url.fragment());
     }
     else
     {
        ref = url.fragment();
        hasRef = url.hasFragment();
        break;
     }
  }

  if ( hasRef )
  {
    // Set HTML ref in all URLs.
    KUrl::List::Iterator it;
    for( it = lst.begin() ; it != lst.end(); ++it )
    {
      (*it).setFragment( ref );
    }
  }

  return lst;
}

KUrl::List KUrl::split( const QString& _url )
{
  return split(KUrl(_url));
}

KUrl KUrl::join( const KUrl::List & lst )
{
  if (lst.isEmpty()) return KUrl();
  KUrl tmp;

  bool first = true;
  QListIterator<KUrl> it(lst);
  it.toBack();
  while (it.hasPrevious())
  {
     KUrl u(it.previous());
     if (!first) {
         u.setEncodedFragment(tmp.url().toLatin1() /* TODO double check encoding */);
     }
     tmp = u;

     first = false;
  }

  return tmp;
}

QString KUrl::fileName( const DirectoryOptions& options ) const
{
  Q_ASSERT( options != 0 ); //Disallow options == false
  QString fname;
  if (hasSubUrl()) { // If we have a suburl, then return the filename from there
    const KUrl::List list = KUrl::split(*this);
    return list.last().fileName(options);
  }
  const QString path = this->path();

  int len = path.length();
  if ( len == 0 )
    return fname;

  if (!(options & ObeyTrailingSlash) )
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

void KUrl::addPath( const QString& _txt )
{
  if (hasSubUrl())
  {
     KUrl::List lst = split( *this );
     KUrl &u = lst.last();
     u.addPath(_txt);
     *this = join( lst );
     return;
  }

  //m_strPath_encoded.clear();

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
  //kDebug(kurlDebugArea())<<"addPath: resultpath="<<path();
}

QString KUrl::directory( const DirectoryOptions& options ) const
{
  Q_ASSERT( options != 0 ); //Disallow options == false
  QString result = path(); //m_strPath_encoded.isEmpty() ? m_strPath : m_strPath_encoded;
  if ( !(options & ObeyTrailingSlash) )
    result = trailingSlash( RemoveTrailingSlash, result );

  if ( result.isEmpty() || result == QLatin1String ( "/" ) )
    return result;

  int i = result.lastIndexOf( '/' );
  // If ( i == -1 ) => the first character is not a '/'
  // So it's some URL like file:blah.tgz, with no path
  if ( i == -1 )
    return QString();

  if ( i == 0 )
  {
    return QLatin1String( "/" );
  }

#ifdef Q_WS_WIN
  if ( i == 2 && result[1] == QLatin1Char(':') )
  {
    return result.left(3);
  }
#endif

  if ( options & AppendTrailingSlash )
    result = result.left( i + 1 );
  else
    result = result.left( i );

  //if (!m_strPath_encoded.isEmpty())
  //  result = decode(result);

  return result;
}


bool KUrl::cd( const QString& _dir )
{
  if ( _dir.isEmpty() || !isValid() )
    return false;

  if (hasSubUrl())
  {
     KUrl::List lst = split( *this );
     KUrl &u = lst.last();
     u.cd(_dir);
     *this = join( lst );
     return true;
  }

  // absolute path ?
#ifdef Q_OS_WIN
  if ( !QFileInfo(_dir).isRelative() )
#else
  if ( _dir[0] == QLatin1Char('/') )
#endif
  {
    //m_strPath_encoded.clear();
    setPath( _dir );
    setHTMLRef( QString() );
    setEncodedQuery( QByteArray() );
    return true;
  }

  // Users home directory on the local disk ?
  if ( ( _dir[0] == '~' ) && ( scheme() == QLatin1String ( "file" ) ))
  {
    //m_strPath_encoded.clear();
    QString strPath = QDir::homePath();
    strPath += QLatin1Char('/');
    strPath += _dir.right( strPath.length() - 1 );
    setPath( strPath );
    setHTMLRef( QString() );
    setEncodedQuery( QByteArray() );
    return true;
  }

  // relative path
  // we always work on the past of the first url.
  // Sub URLs are not touched.

  // append '/' if necessary
  QString p = path(AddTrailingSlash);
  p += _dir;
  p = cleanpath( p, true, false );
  setPath( p );

  setHTMLRef( QString() );
  setEncodedQuery( QByteArray() );

  return true;
}

KUrl KUrl::upUrl( ) const
{
  if (!isValid() || isRelative())
    return KUrl();

  if (!encodedQuery().isEmpty())
  {
     KUrl u(*this);
     u.setEncodedQuery(QByteArray());
     return u;
  }

  if (!hasSubUrl())
  {
     KUrl u(*this);

     u.cd("../");

     return u;
  }

  // We have a subURL.
  KUrl::List lst = split( *this );
  if (lst.isEmpty())
      return KUrl(); // Huh?
  while (true)
  {
     KUrl &u = lst.last();
     const QString old = u.path();
     u.cd("../");
     if (u.path() != old)
         break; // Finished.
     if (lst.count() == 1)
         break; // Finished.
     lst.removeLast();
  }
  return join( lst );
}

QString KUrl::htmlRef() const
{
  if ( !hasSubUrl() )
  {
      return QUrl::fromPercentEncoding( ref().toLatin1() );
  }

  const List lst = split( *this );
  return QUrl::fromPercentEncoding( (*lst.begin()).ref().toLatin1() );
}

QString KUrl::encodedHtmlRef() const
{
  if ( !hasSubUrl() )
  {
    return ref();
  }

  const List lst = split( *this );
  return (*lst.begin()).ref();
}

void KUrl::setHTMLRef( const QString& _ref )
{
  if ( !hasSubUrl() )
  {
    setFragment( _ref );
    return;
  }

  List lst = split( *this );

  (*lst.begin()).setFragment( _ref );

  *this = join( lst );
}

bool KUrl::hasHTMLRef() const
{
  if ( !hasSubUrl() )
  {
    return hasRef();
  }

  const List lst = split( *this );
  return (*lst.begin()).hasRef();
}

void KUrl::setDirectory( const QString &dir)
{
  if ( dir.endsWith(QLatin1Char('/')))
     setPath(dir);
  else
     setPath(dir + QLatin1Char('/'));
}

void KUrl::setQuery( const QString &_txt )
{
  if (!_txt.isEmpty() && _txt[0] == '?')
    _setQuery( _txt.length() > 1 ? _txt.mid(1) : "" /*empty, not null*/ );
  else
    _setQuery( _txt );
}

void KUrl::_setQuery( const QString& query )
{
    if ( query.isNull() ) {
        setEncodedQuery( QByteArray() );
    } else if ( query.isEmpty() ) {
        setEncodedQuery( "" );
    } else {
        setEncodedQuery( query.toLatin1() ); // already percent-escaped, so toLatin1 is ok
    }
}

QString KUrl::query() const
{
  if (!hasQuery()) {
    return QString();
  }
  return QString( QChar( '?' ) ) + QString::fromAscii( encodedQuery() );
}

void KUrl::_setEncodedUrl(const QByteArray& url)
{
  setEncodedUrl(url, QUrl::TolerantMode);
  if (!isValid()) // see unit tests referring to N183630/task 183874
    setUrl(url, QUrl::TolerantMode);
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

  KUrl::List list1 = KUrl::split( _url1 );
  KUrl::List list2 = KUrl::split( _url2 );

  // Malformed ?
  if ( list1.isEmpty() || list2.isEmpty() )
    return false;

  return ( list1 == list2 );
#endif
}

bool urlcmp( const QString& _url1, const QString& _url2, const KUrl::EqualsOptions& _options )
{
    // Both empty ?
    if (_url1.isEmpty() && _url2.isEmpty())
        return true;
    // Only one empty ?
    if (_url1.isEmpty() || _url2.isEmpty())
        return false;

    KUrl u1(_url1);
    KUrl u2(_url2);
    return u1.equals(u2, _options);

#if 0 // kde3 code that supported nested urls

  KUrl::List list1 = KUrl::split( _url1 );
  KUrl::List list2 = KUrl::split( _url2 );

  // Malformed ?
  if ( list1.isEmpty() || list2.isEmpty() )
    return false;

  int size = list1.count();
  if ( list2.count() != size )
    return false;

  if ( _ignore_ref )
  {
    (*list1.begin()).setRef(QString());
    (*list2.begin()).setRef(QString());
  }

  KUrl::List::Iterator it1 = list1.begin();
  KUrl::List::Iterator it2 = list2.begin();
  for( ; it1 != list1.end() ; ++it1, ++it2 )
    if ( !(*it1).equals( *it2, _ignore_trailing ) )
      return false;
  return true;
#endif
}

// static
KUrl KUrl::fromPathOrUrl( const QString& text )
{
    KUrl url;
    if ( !text.isEmpty() )
    {
        if (!QDir::isRelativePath(text) || text[0] == '~')
            url.setPath( text );
        else
            url = KUrl( text );
    }

    return url;
}

static QString _relativePath(const QString &base_dir, const QString &path, bool &isParent)
{
   QString _base_dir(QDir::cleanPath(base_dir));
   QString _path(QDir::cleanPath(path.isEmpty() || (path[0] != QLatin1Char('/')) ? _base_dir+'/'+path : path));

   if (_base_dir.isEmpty())
      return _path;

   if (_base_dir[_base_dir.length()-1] != QLatin1Char('/'))
      _base_dir.append(QLatin1Char('/') );

   const QStringList list1 = _base_dir.split(QLatin1Char('/'), QString::SkipEmptyParts);
   const QStringList list2 = _path.split(QLatin1Char('/'), QString::SkipEmptyParts);

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

QString KUrl::relativePath(const QString &base_dir, const QString &path, bool *isParent)
{
   bool parent = false;
   QString result = _relativePath(base_dir, path, parent);
   if (parent)
      result.prepend("./");

   if (isParent)
      *isParent = parent;

   return result;
}


QString KUrl::relativeUrl(const KUrl &base_url, const KUrl &url)
{
   if ((url.protocol() != base_url.protocol()) ||
       (url.host() != base_url.host()) ||
       (url.port() && url.port() != base_url.port()) ||
       (url.hasUser() && url.user() != base_url.user()) ||
       (url.hasPass() && url.pass() != base_url.pass()))
   {
      return url.url();
   }

   QString relURL;

   if ((base_url.path() != url.path()) || (base_url.query() != url.query()))
   {
      bool dummy;
      QString basePath = base_url.directory(KUrl::ObeyTrailingSlash);
      relURL = _relativePath(basePath, url.path(), dummy); // was QUrl::toPercentEncoding() but why?
      relURL += url.query();
   }

   if ( url.hasRef() )
   {
      relURL += '#';
      relURL += url.ref();
   }

   if ( relURL.isEmpty() )
      return "./";

   return relURL;
}

void KUrl::setPath( const QString& _path )
{
#if defined(Q_WS_WIN) && defined(DEBUG_KURL)
    kDebug(kurlDebugArea()) << "KUrl::setPath " << " " << _path.toAscii().data();
#endif
    if ( scheme().isEmpty() )
        setScheme( QLatin1String( "file" ) );
    QString path = KShell::tildeExpand( _path );
#ifdef Q_WS_WIN
    const int len = path.length();
    if( len == 2 && IS_LETTER(path[0]) && path[1] == QLatin1Char(':') )
        path += QLatin1Char('/');
    //This is necessary because QUrl has the "path" part including the first slash
    //Without this QUrl doesn't understand that this is a path, and some operations fail
    //e.g. C:/blah needs to become /C:/blah
    else
    if( len > 0 && path[0] != QLatin1Char('/') && scheme() == QLatin1String( "file" ) )
        path = QLatin1Char('/') + path;
#endif
    QUrl::setPath( path );
}

#if 0 // this would be if we didn't decode '+' into ' '
QMap< QString, QString > KUrl::queryItems( int options ) const {
  QMap< QString, QString > result;
  const QList<QPair<QString, QString> > items = QUrl::queryItems();
  QPair<QString, QString> item;
  Q_FOREACH( item, items ) {
      result.insert( options & CaseInsensitiveKeys ? item.first.toLower() : item.first, item.second );
  }
  return result;
}
#endif

QMap< QString, QString > KUrl::queryItems( const QueryItemsOptions &options ) const {
  const QString strQueryEncoded = encodedQuery();
  if ( strQueryEncoded.isEmpty() )
    return QMap<QString,QString>();

  QMap< QString, QString > result;
  const QStringList items = strQueryEncoded.split( '&', QString::SkipEmptyParts );
  for ( QStringList::const_iterator it = items.begin() ; it != items.end() ; ++it ) {
    const int equal_pos = (*it).indexOf( '=' );
    if ( equal_pos > 0 ) { // = is not the first char...
      QString name = (*it).left( equal_pos );
      if ( options & CaseInsensitiveKeys )
	name = name.toLower();
      QString value = (*it).mid( equal_pos + 1 );
      if ( value.isEmpty() )
        result.insert( name, QString::fromLatin1("") );
      else {
	// ### why is decoding name not necessary?
	value.replace( '+', ' ' ); // + in queries means space
	result.insert( name, QUrl::fromPercentEncoding( value.toLatin1() ) );
      }
    } else if ( equal_pos < 0 ) { // no =
      QString name = (*it);
      if ( options & CaseInsensitiveKeys )
	name = name.toLower();
      result.insert( name, QString() );
    }
  }

  return result;
}

QString KUrl::queryItem( const QString& _item ) const
{
  const QString strQueryEncoded = encodedQuery();
  const QString item = _item + '=';
  if ( strQueryEncoded.length() <= 1 )
    return QString();

  const QStringList items = strQueryEncoded.split( '&', QString::SkipEmptyParts );
  const int _len = item.length();
  for ( QStringList::ConstIterator it = items.begin(); it != items.end(); ++it )
  {
    if ( (*it).startsWith( item ) )
    {
      if ( (*it).length() > _len )
      {
        QString str = (*it).mid( _len );
        str.replace( '+', ' ' ); // + in queries means space.
        return QUrl::fromPercentEncoding( str.toLatin1() );
      }
      else // empty value
        return QString::fromLatin1("");
    }
  }

  return QString();
}

void KUrl::addQueryItem( const QString& _item, const QString& _value )
{
  QString item = _item + '=';
  QString value = QUrl::toPercentEncoding( _value );

  QString strQueryEncoded = encodedQuery();
  if (!strQueryEncoded.isEmpty())
     strQueryEncoded += '&';
  strQueryEncoded += item + value;
  setEncodedQuery( strQueryEncoded.toLatin1() );
}

void KUrl::populateMimeData( QMimeData* mimeData,
                             const MetaDataMap& metaData,
                             MimeDataFlags flags ) const
{
  KUrl::List lst( *this );
  lst.populateMimeData( mimeData, metaData, flags );
}

bool KUrl::hasRef() const
{
  return hasFragment();
}

void KUrl::setRef( const QString& fragment )
{
  if ( fragment.isNull() )
    setFragment( fragment ); // pass null, not empty
  else
    setFragment( QUrl::fromPercentEncoding( fragment.toLatin1() ) );
}

QString KUrl::ref() const
{
  if ( fragment().isNull() )
    return QString();
  else
    return QString::fromLatin1( QUrl::toPercentEncoding( fragment() ) );
}

bool KUrl::isParentOf( const KUrl& u ) const
{
  return QUrl::isParentOf( u ) || equals( u, CompareWithoutTrailingSlash );
}

uint qHash(const KUrl& kurl)
{
  // qHash(kurl.url()) was the worse implementation possible, since QUrl::toEncoded()
  // had to concatenate the bits of the url into the full url every time.

  return qHash(kurl.protocol()) ^ qHash(kurl.path()) ^ qHash(kurl.fragment()) ^ qHash(kurl.query());
}
