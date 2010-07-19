/*
   Copyright (C) 2000-2003 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2000-2002 George Staikos <staikos@kde.org>
   Copyright (C) 2000-2002 Dawit Alemayehu <adawit@kde.org>
   Copyright (C) 2001,2002 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2007      Nick Shaforostoff <shafff@ukr.net>
   Copyright (C) 2007      Daniel Nicoletti <mirttex@users.sourceforge.net>
   Copyright (C) 2008,2009 Andreas Hartmetz <ahartmetz@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// TODO delete / do not save very big files; "very big" to be defined

#define QT_NO_CAST_FROM_ASCII

#include "http.h"

#include <config.h>

#include <fcntl.h>
#include <utime.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h> // must be explicitly included for MacOSX

#include <QtXml/qdom.h>
#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/QDate>
#include <QtDBus/QtDBus>
#include <QtNetwork/QAuthenticator>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostInfo>

#include <kurl.h>
#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kservice.h>
#include <kdatetime.h>
#include <kcomponentdata.h>
#include <krandom.h>
#include <kmimetype.h>
#include <ktoolinvocation.h>
#include <kstandarddirs.h>
#include <kremoteencoding.h>
#include <ktcpsocket.h>

#include <kio/ioslave_defaults.h>
#include <kio/http_slave_defaults.h>

#include <httpfilter.h>

#include <solid/networking.h>

#ifdef HAVE_LIBGSSAPI
#ifdef GSSAPI_MIT
#include <gssapi/gssapi.h>
#else
#include <gssapi.h>
#endif /* GSSAPI_MIT */

// Catch uncompatible crap (BR86019)
#if defined(GSS_RFC_COMPLIANT_OIDS) && (GSS_RFC_COMPLIANT_OIDS == 0)
#include <gssapi/gssapi_generic.h>
#define GSS_C_NT_HOSTBASED_SERVICE gss_nt_service_name
#endif

#endif /* HAVE_LIBGSSAPI */

#include <misc/kntlm/kntlm.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kde_file.h>

//string parsing helpers and HeaderTokenizer implementation
#include "parsinghelpers.cpp"
//authentication handlers
#include "httpauthentication.cpp"


// see filenameFromUrl(): a sha1 hash is 160 bits
static const int s_hashedUrlBits = 160;   // this number should always be divisible by eight
static const int s_hashedUrlNibbles = s_hashedUrlBits / 4;
static const int s_hashedUrlBytes = s_hashedUrlBits / 8;

using namespace KIO;

extern "C" int KDE_EXPORT kdemain( int argc, char **argv )
{
    QCoreApplication app( argc, argv ); // needed for QSocketNotifier
    KComponentData componentData( "kio_http", "kdelibs4" );
    (void) KGlobal::locale();

    if (argc != 4)
    {
        fprintf(stderr, "Usage: kio_http protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }

    HTTPProtocol slave(argv[1], argv[2], argv[3]);
    slave.dispatchLoop();
    return 0;
}

/***********************************  Generic utility functions ********************/

static bool isCrossDomainRequest( const QString& fqdn, const QString& originURL )
{
  //TODO read the RFC
  if (originURL == QLatin1String("true")) // Backwards compatibility
     return true;

  KUrl url ( originURL );

  // Document Origin domain
  QString a = url.host();
  // Current request domain
  QString b = fqdn;

  if (a == b)
    return false;

  QStringList la = a.split(QLatin1Char('.'), QString::SkipEmptyParts);
  QStringList lb = b.split(QLatin1Char('.'), QString::SkipEmptyParts);

  if (qMin(la.count(), lb.count()) < 2) {
      return true;  // better safe than sorry...
  }

  while(la.count() > 2)
      la.pop_front();
  while(lb.count() > 2)
      lb.pop_front();

  return la != lb;
}

/*
  Eliminates any custom header that could potentially alter the request
*/
static QString sanitizeCustomHTTPHeader(const QString& _header)
{
  QString sanitizedHeaders;
  const QStringList headers = _header.split(QRegExp(QLatin1String("[\r\n]")));

  for(QStringList::ConstIterator it = headers.begin(); it != headers.end(); ++it)
  {
    // Do not allow Request line to be specified and ignore
    // the other HTTP headers.
    if (!(*it).contains(QLatin1Char(':')) ||
        (*it).startsWith(QLatin1String("host"), Qt::CaseInsensitive) ||
        (*it).startsWith(QLatin1String("proxy-authorization"), Qt::CaseInsensitive) ||
        (*it).startsWith(QLatin1String("via"), Qt::CaseInsensitive))
      continue;

    sanitizedHeaders += (*it);
    sanitizedHeaders += QLatin1String("\r\n");
  }
  sanitizedHeaders.chop(2);

  return sanitizedHeaders;
}

// for a given response code, conclude if the response is going to/likely to have a response body
static bool canHaveResponseBody(int rCode, KIO::HTTP_METHOD method)
{
/* RFC 2616 says...
    1xx: false
    200: method HEAD: false, otherwise:true
    201: true
    202: true
    203: see 200
    204: false
    205: false
    206: true
    300: see 200
    301: see 200
    302: see 200
    303: see 200
    304: false
    305: probably like 300, RFC seems to expect disconnection afterwards...
    306: (reserved), for simplicity do it just like 200
    307: see 200
    4xx: see 200
    5xx :see 200
*/
    if (rCode >= 100 && rCode < 200) {
        return false;
    }
    switch (rCode) {
    case 201:
    case 202:
    case 206:
        // RFC 2616 does not mention HEAD in the description of the above. if the assert turns out
        // to be a problem the response code should probably be treated just like 200 and friends.
        Q_ASSERT(method != HTTP_HEAD);
        return true;
    case 204:
    case 205:
    case 304:
        return false;
    default:
        break;
    }
    // safe (and for most remaining response codes exactly correct) default
    return method != HTTP_HEAD;
}

static bool isEncryptedHttpVariety(const QByteArray &p)
{
    return p == "https" || p == "webdavs";
}

static bool isValidProxy(const KUrl &u)
{
    return u.isValid() && u.hasHost();
}

static bool isHttpProxy(const KUrl &u)
{
    return isValidProxy(u) && u.protocol() == QLatin1String("http");
}

static QByteArray methodString(HTTP_METHOD m)
{
    switch(m) {
    case HTTP_GET:
        return "GET ";
    case HTTP_PUT:
        return "PUT ";
    case HTTP_POST:
        return "POST ";
    case HTTP_HEAD:
        return "HEAD ";
    case HTTP_DELETE:
        return "DELETE ";
    case HTTP_OPTIONS:
        return "OPTIONS ";
    case DAV_PROPFIND:
        return "PROPFIND ";
    case DAV_PROPPATCH:
        return "PROPPATCH ";
    case DAV_MKCOL:
        return "MKCOL ";
    case DAV_COPY:
        return "COPY ";
    case DAV_MOVE:
        return "MOVE ";
    case DAV_LOCK:
        return "LOCK ";
    case DAV_UNLOCK:
        return "UNLOCK ";
    case DAV_SEARCH:
        return "SEARCH ";
    case DAV_SUBSCRIBE:
        return "SUBSCRIBE ";
    case DAV_UNSUBSCRIBE:
        return "UNSUBSCRIBE ";
    case DAV_POLL:
        return "POLL ";
    case DAV_REPORT:
        return "REPORT ";
    default:
        Q_ASSERT(false);
        return QByteArray();
    }
}

static QString formatHttpDate(qint64 date)
{
    KDateTime dt;
    dt.setTime_t(date);
    QString ret = dt.toString(KDateTime::RFCDateDay);
    ret.chop(6);    // remove " +0000"
    // RFCDate[Day] omits the second if zero, but HTTP requires it; see bug 240585.
    if (!dt.time().second()) {
        ret.append(QString::fromLatin1(":00"));
    }
    ret.append(QString::fromLatin1(" GMT"));
    return ret;
}

static bool shouldDiscardContent(int code)
{
    return ((code == 401) || (code == 407));
}

#define NO_SIZE ((KIO::filesize_t) -1)

#ifdef HAVE_STRTOLL
#define STRTOLL strtoll
#else
#define STRTOLL strtol
#endif


/************************************** HTTPProtocol **********************************************/


HTTPProtocol::HTTPProtocol( const QByteArray &protocol, const QByteArray &pool,
                            const QByteArray &app )
    : TCPSlaveBase(protocol, pool, app, isEncryptedHttpVariety(protocol))
    , m_iSize(NO_SIZE)
    , m_isBusy(false)
    , m_maxCacheAge(DEFAULT_MAX_CACHE_AGE)
    , m_maxCacheSize(DEFAULT_MAX_CACHE_SIZE/2)
    , m_protocol(protocol)
    , m_wwwAuth(0)
    , m_proxyAuth(0)
    , m_socketProxyAuth(0)
    , m_isError(false)
    , m_isLoadingErrorPage(false)
    , m_remoteRespTimeout(DEFAULT_RESPONSE_TIMEOUT)
{
    reparseConfiguration();
    setBlocking(true);
    connect(socket(), SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)),
            this, SLOT(proxyAuthenticationForSocket(const QNetworkProxy &, QAuthenticator *)));
}

HTTPProtocol::~HTTPProtocol()
{
  httpClose(false);
}

void HTTPProtocol::reparseConfiguration()
{
    kDebug(7113);

    delete m_proxyAuth;
    delete m_wwwAuth;
    m_proxyAuth = 0;
    m_wwwAuth = 0;
    m_request.proxyUrl.clear(); //TODO revisit
}

void HTTPProtocol::resetConnectionSettings()
{
  m_isEOF = false;
  m_isError = false;
  m_isLoadingErrorPage = false;
}

quint16 HTTPProtocol::defaultPort() const
{
    return isEncryptedHttpVariety(m_protocol) ? DEFAULT_HTTPS_PORT : DEFAULT_HTTP_PORT;
}

void HTTPProtocol::resetResponseParsing()
{
  m_isRedirection = false;
  m_isChunked = false;
  m_iSize = NO_SIZE;
  clearUnreadBuffer();

  m_responseHeaders.clear();
  m_contentEncodings.clear();
  m_transferEncodings.clear();
  m_contentMD5.clear();
  m_mimeType.clear();

  setMetaData(QString::fromLatin1("request-id"), m_request.id);
}

void HTTPProtocol::resetSessionSettings()
{
  // Do not reset the URL on redirection if the proxy
  // URL, username or password has not changed!
  KUrl proxy ( config()->readEntry("UseProxy") );
  QNetworkProxy::ProxyType proxyType = QNetworkProxy::NoProxy;

#if 0
  if ( m_proxyAuth.realm.isEmpty() || !proxy.isValid() ||
       m_request.proxyUrl.host() != proxy.host() ||
       m_request.proxyUrl.port() != proxy.port() ||
       (!proxy.user().isEmpty() && proxy.user() != m_request.proxyUrl.user()) ||
       (!proxy.pass().isEmpty() && proxy.pass() != m_request.proxyUrl.pass()) )
  {
    m_request.proxyUrl = proxy;

    kDebug(7113) << "Using proxy:" << m_request.useProxy()
                 << "URL: " << m_request.proxyUrl.url()
                 << "Realm: " << m_proxyAuth.realm;
  }
#endif
    m_request.proxyUrl = proxy;
    kDebug(7113) << "Using proxy:" << isValidProxy(m_request.proxyUrl)
                 << "URL: " << m_request.proxyUrl.url();
                 //<< "Realm: " << m_proxyAuth.realm;

  if (isValidProxy(m_request.proxyUrl)) {
      if (m_request.proxyUrl.protocol() == QLatin1String("socks")) {
          // Let Qt do SOCKS because it's already implemented there...
          proxyType = QNetworkProxy::Socks5Proxy;
      } else if (isAutoSsl()) {
          // and for HTTPS we use HTTP CONNECT on the proxy server, also implemented in Qt.
          // This is the usual way to handle SSL proxying.
          proxyType = QNetworkProxy::HttpProxy;
      }
      m_request.proxyUrl = proxy;
  } else {
      m_request.proxyUrl = KUrl();
  }

  QNetworkProxy appProxy(proxyType, m_request.proxyUrl.host(), m_request.proxyUrl.port(),
                         m_request.proxyUrl.user(), m_request.proxyUrl.pass());
  QNetworkProxy::setApplicationProxy(appProxy);

  if (isHttpProxy(m_request.proxyUrl) && !isAutoSsl()) {
    m_request.isKeepAlive = config()->readEntry("PersistentProxyConnection", false);
    kDebug(7113) << "Enable Persistent Proxy Connection: "
                 << m_request.isKeepAlive;
  }

  m_request.redirectUrl = KUrl();
  m_request.useCookieJar = config()->readEntry("Cookies", false);
  m_request.cacheTag.useCache = config()->readEntry("UseCache", true);
  m_request.preferErrorPage = config()->readEntry("errorPage", true);
  m_request.doNotAuthenticate = config()->readEntry("no-auth", false);
  m_strCacheDir = config()->readPathEntry("CacheDir", QString());
  m_maxCacheAge = config()->readEntry("MaxCacheAge", DEFAULT_MAX_CACHE_AGE);
  m_request.windowId = config()->readEntry("window-id");

  kDebug(7113) << "Window Id =" << m_request.windowId;
  kDebug(7113) << "ssl_was_in_use ="
               << metaData(QLatin1String("ssl_was_in_use"));

  m_request.referrer.clear();
  // RFC 2616: do not send the referrer if the referrer page was served using SSL and
  //           the current page does not use SSL.
  if ( config()->readEntry("SendReferrer", true) &&
       (isEncryptedHttpVariety(m_protocol) || metaData(QLatin1String("ssl_was_in_use")) != QLatin1String("TRUE") ) )
  {
     KUrl refUrl(metaData(QLatin1String("referrer")));
     if (refUrl.isValid()) {
        // Sanitize
        QString protocol = refUrl.protocol();
        if (protocol.startsWith(QLatin1String("webdav"))) {
           protocol.replace(0, 6, QLatin1String("http"));
           refUrl.setProtocol(protocol);
        }

        if (protocol.startsWith(QLatin1String("http"))) {
            m_request.referrer = QString::fromLatin1(refUrl.toEncoded(QUrl::RemoveUserInfo | QUrl::RemoveFragment));
        }
     }
  }

  if (config()->readEntry("SendLanguageSettings", true)) {
      m_request.charsets = config()->readEntry( "Charsets", "iso-8859-1" );
      if (!m_request.charsets.isEmpty()) {
          m_request.charsets += QString::fromLatin1(DEFAULT_PARTIAL_CHARSET_HEADER);
      }
      m_request.languages = config()->readEntry( "Languages", DEFAULT_LANGUAGE_HEADER );
  } else {
      m_request.charsets.clear();
      m_request.languages.clear();
  }

  // Adjust the offset value based on the "resume" meta-data.
  QString resumeOffset = metaData(QLatin1String("resume"));
  if (!resumeOffset.isEmpty()) {
     m_request.offset = resumeOffset.toULongLong();
  } else {
     m_request.offset = 0;
  }
  // Same procedure for endoffset.
  QString resumeEndOffset = metaData(QLatin1String("resume_until"));
  if (!resumeEndOffset.isEmpty()) {
     m_request.endoffset = resumeEndOffset.toULongLong();
  } else {
     m_request.endoffset = 0;
  }

  m_request.disablePassDialog = config()->readEntry("DisablePassDlg", false);
  m_request.allowTransferCompression = config()->readEntry("AllowCompressedPage", true);
  m_request.id = metaData(QLatin1String("request-id"));

  // Store user agent for this host.
  if (config()->readEntry("SendUserAgent", true)) {
     m_request.userAgent = metaData(QLatin1String("UserAgent"));
  } else {
     m_request.userAgent.clear();
  }

  m_request.cacheTag.etag.clear();
  // -1 is also the value returned by KDateTime::toTime_t() from an invalid instance.
  m_request.cacheTag.servedDate = -1;
  m_request.cacheTag.lastModifiedDate = -1;
  m_request.cacheTag.expireDate = -1;

  m_request.responseCode = 0;
  m_request.prevResponseCode = 0;

  delete m_wwwAuth;
  m_wwwAuth = 0;
  delete m_socketProxyAuth;
  m_socketProxyAuth = 0;

  // Obtain timeout values
  m_remoteRespTimeout = responseTimeout();

  // Bounce back the actual referrer sent
  setMetaData(QLatin1String("referrer"), m_request.referrer);

  // Follow HTTP/1.1 spec and enable keep-alive by default
  // unless the remote side tells us otherwise or we determine
  // the persistent link has been terminated by the remote end.
  m_request.isKeepAlive = true;
  m_request.keepAliveTimeout = 0;
}

void HTTPProtocol::setHost( const QString& host, quint16 port,
                            const QString& user, const QString& pass )
{
  // Reset the webdav-capable flags for this host
  if ( m_request.url.host() != host )
    m_davHostOk = m_davHostUnsupported = false;

  m_request.url.setHost(host);

  // is it an IPv6 address?
  if (host.indexOf(QLatin1Char(':')) == -1) {
      m_request.encoded_hostname = QString::fromLatin1(QUrl::toAce(host));
  } else  {
      int pos = host.indexOf(QLatin1Char('%'));
      if (pos == -1)
        m_request.encoded_hostname = QLatin1Char('[') + host + QLatin1Char(']');
      else
        // don't send the scope-id in IPv6 addresses to the server
        m_request.encoded_hostname = QLatin1Char('[') + host.left(pos) + QLatin1Char(']');
  }
  m_request.url.setPort((port > 0 && port != defaultPort()) ? port : -1);
  m_request.url.setUser(user);
  m_request.url.setPass(pass);

  //TODO need to do anything about proxying?

  kDebug(7113) << "Hostname is now:" << m_request.url.host()
               << "(" << m_request.encoded_hostname << ")";
}

bool HTTPProtocol::maybeSetRequestUrl(const KUrl &u)
{
  kDebug (7113) << u.url();

  m_request.url = u;
  m_request.url.setPort(u.port(defaultPort()) != defaultPort() ? u.port() : -1);

  if (u.host().isEmpty()) {
     error( KIO::ERR_UNKNOWN_HOST, i18n("No host specified."));
     return false;
  }

  if (u.path().isEmpty()) {
     KUrl newUrl(u);
     newUrl.setPath(QLatin1String("/"));
     redirection(newUrl);
     finished();
     return false;
  }

  return true;
}

void HTTPProtocol::proceedUntilResponseContent( bool dataInternal /* = false */ )
{
  kDebug (7113);
  if (!(proceedUntilResponseHeader() && readBody(dataInternal))) {
      return;
  }

  httpClose(m_request.isKeepAlive);

  // if data is required internally, don't finish,
  // it is processed before we finish()
  if (!dataInternal) {
      if ((m_request.responseCode == 204) &&
          ((m_request.method == HTTP_GET) || (m_request.method == HTTP_POST))) {
          error(ERR_NO_CONTENT, QString());
      } else {
          finished();
      }
  }
}

bool HTTPProtocol::proceedUntilResponseHeader()
{
  kDebug (7113);

  // Retry the request until it succeeds or an unrecoverable error occurs.
  // Recoverable errors are, for example:
  // - Proxy or server authentication required: Ask for credentials and try again,
  //   this time with an authorization header in the request.
  // - Server-initiated timeout on keep-alive connection: Reconnect and try again

  while (true) {
      if (!sendQuery()) {
          return false;
      }
      if (readResponseHeader()) {
          // Success, finish the request.
          break;
      }

      // If not loading error page and the response code requires us to resend the query,
      // then throw away any error message that might have been sent by the server.
      if (!m_isLoadingErrorPage && shouldDiscardContent(m_request.responseCode)) {
          // This gets rid of any error page sent with 401 or 407 response...
          readBody(true);
      }

      // no success, close the cache file so the cache state is reset - that way most other code
      // doesn't have to deal with the cache being in various states.
      cacheFileClose();
      if (m_isError || m_isLoadingErrorPage) {
          // Unrecoverable error, abort everything.
          // Also, if we've just loaded an error page there is nothing more to do.
          // In that case we abort to avoid loops; some webservers manage to send 401 and
          // no authentication request. Or an auth request we don't understand.
          return false;
      }

      if (!m_request.isKeepAlive) {
          httpCloseConnection();
      }
  }

  // Do not save authorization if the current response code is
  // 4xx (client error) or 5xx (server error).
  kDebug(7113) << "Previous Response:" << m_request.prevResponseCode;
  kDebug(7113) << "Current Response:" << m_request.responseCode;

  setMetaData(QLatin1String("responsecode"), QString::number(m_request.responseCode));
  setMetaData(QLatin1String("content-type"), m_mimeType);

  // At this point sendBody() should have delivered any POST data.
  m_POSTbuf.clear();

  return true;
}

void HTTPProtocol::stat(const KUrl& url)
{
  kDebug(7113) << url.url();

  if (!maybeSetRequestUrl(url))
      return;
  resetSessionSettings();

  if ( m_protocol != "webdav" && m_protocol != "webdavs" )
  {
    QString statSide = metaData(QString::fromLatin1("statSide"));
    if (statSide != QLatin1String("source"))
    {
      // When uploading we assume the file doesn't exit
      error( ERR_DOES_NOT_EXIST, url.prettyUrl() );
      return;
    }

    // When downloading we assume it exists
    UDSEntry entry;
    entry.insert( KIO::UDSEntry::UDS_NAME, url.fileName() );
    entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG ); // a file
    entry.insert( KIO::UDSEntry::UDS_ACCESS, S_IRUSR | S_IRGRP | S_IROTH ); // readable by everybody

    statEntry( entry );
    finished();
    return;
  }

  davStatList( url );
}

void HTTPProtocol::listDir( const KUrl& url )
{
  kDebug(7113) << url.url();

  if (!maybeSetRequestUrl(url))
    return;
  resetSessionSettings();

  davStatList( url, false );
}

void HTTPProtocol::davSetRequest( const QByteArray& requestXML )
{
  // insert the document into the POST buffer, kill trailing zero byte
  m_POSTbuf = requestXML;
}

void HTTPProtocol::davStatList( const KUrl& url, bool stat )
{
  UDSEntry entry;

  // check to make sure this host supports WebDAV
  if ( !davHostOk() )
    return;

  // Maybe it's a disguised SEARCH...
  QString query = metaData(QLatin1String("davSearchQuery"));
  if ( !query.isEmpty() )
  {
    QByteArray request = "<?xml version=\"1.0\"?>\r\n";
    request.append( "<D:searchrequest xmlns:D=\"DAV:\">\r\n" );
    request.append( query.toUtf8() );
    request.append( "</D:searchrequest>\r\n" );

    davSetRequest( request );
  } else {
    // We are only after certain features...
    QByteArray request;
    request = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
    "<D:propfind xmlns:D=\"DAV:\">";

    // insert additional XML request from the davRequestResponse metadata
    if ( hasMetaData(QLatin1String("davRequestResponse")) )
      request += metaData(QLatin1String("davRequestResponse")).toUtf8();
    else {
      // No special request, ask for default properties
      request += "<D:prop>"
      "<D:creationdate/>"
      "<D:getcontentlength/>"
      "<D:displayname/>"
      "<D:source/>"
      "<D:getcontentlanguage/>"
      "<D:getcontenttype/>"
      "<D:executable/>"
      "<D:getlastmodified/>"
      "<D:getetag/>"
      "<D:supportedlock/>"
      "<D:lockdiscovery/>"
      "<D:resourcetype/>"
      "</D:prop>";
    }
    request += "</D:propfind>";

    davSetRequest( request );
  }

  // WebDAV Stat or List...
  m_request.method = query.isEmpty() ? DAV_PROPFIND : DAV_SEARCH;
  m_request.url.setQuery(QString());
  m_request.cacheTag.policy = CC_Reload;
  m_request.davData.depth = stat ? 0 : 1;
  if (!stat)
     m_request.url.adjustPath(KUrl::AddTrailingSlash);

  proceedUntilResponseContent( true );

  // Has a redirection already been called? If so, we're done.
  if (m_isRedirection) {
    finished();
    return;
  }

  QDomDocument multiResponse;
  multiResponse.setContent( m_webDavDataBuf, true );

  bool hasResponse = false;

  for ( QDomNode n = multiResponse.documentElement().firstChild();
        !n.isNull(); n = n.nextSibling())
  {
    QDomElement thisResponse = n.toElement();
    if (thisResponse.isNull())
      continue;

    hasResponse = true;

    QDomElement href = thisResponse.namedItem(QLatin1String("href")).toElement();
    if ( !href.isNull() )
    {
      entry.clear();

      QString urlStr = QUrl::fromPercentEncoding(href.text().toUtf8());
#if 0 // qt4/kde4 say: it's all utf8...
      int encoding = remoteEncoding()->encodingMib();
      if ((encoding == 106) && (!KStringHandler::isUtf8(KUrl::decode_string(urlStr, 4).toLatin1())))
        encoding = 4; // Use latin1 if the file is not actually utf-8

      KUrl thisURL ( urlStr, encoding );
#else
      KUrl thisURL( urlStr );
#endif

      if ( thisURL.isValid() ) {
        QString name = thisURL.fileName();

        // base dir of a listDir(): name should be "."
        if ( !stat && thisURL.path(KUrl::AddTrailingSlash).length() == url.path(KUrl::AddTrailingSlash).length() )
          name = QLatin1Char('.');

        entry.insert( KIO::UDSEntry::UDS_NAME, name.isEmpty() ? href.text() : name );
      }

      QDomNodeList propstats = thisResponse.elementsByTagName(QLatin1String("propstat"));

      davParsePropstats( propstats, entry );

      if ( stat )
      {
        // return an item
        statEntry( entry );
        finished();
        return;
      }
      else
      {
        listEntry( entry, false );
      }
    }
    else
    {
      kDebug(7113) << "Error: no URL contained in response to PROPFIND on" << url;
    }
  }

  if ( stat || !hasResponse )
  {
    error( ERR_DOES_NOT_EXIST, url.prettyUrl() );
  }
  else
  {
    listEntry( entry, true );
    finished();
  }
}

void HTTPProtocol::davGeneric( const KUrl& url, KIO::HTTP_METHOD method )
{
  kDebug(7113) << url.url();

  if (!maybeSetRequestUrl(url))
    return;
  resetSessionSettings();

  // check to make sure this host supports WebDAV
  if ( !davHostOk() )
    return;

  // WebDAV method
  m_request.method = method;
  m_request.url.setQuery(QString());
  m_request.cacheTag.policy = CC_Reload;

  proceedUntilResponseContent( false );
}

int HTTPProtocol::codeFromResponse( const QString& response )
{
  const int firstSpace = response.indexOf( QLatin1Char(' ') );
  const int secondSpace = response.indexOf( QLatin1Char(' '), firstSpace + 1 );
  return response.mid( firstSpace + 1, secondSpace - firstSpace - 1 ).toInt();
}

void HTTPProtocol::davParsePropstats( const QDomNodeList& propstats, UDSEntry& entry )
{
  QString mimeType;
  bool foundExecutable = false;
  bool isDirectory = false;
  uint lockCount = 0;
  uint supportedLockCount = 0;

  for ( int i = 0; i < propstats.count(); i++)
  {
    QDomElement propstat = propstats.item(i).toElement();

    QDomElement status = propstat.namedItem(QLatin1String("status")).toElement();
    if ( status.isNull() )
    {
      // error, no status code in this propstat
      kDebug(7113) << "Error, no status code in this propstat";
      return;
    }

    int code = codeFromResponse( status.text() );

    if ( code != 200 )
    {
      kDebug(7113) << "Warning: status code" << code << "(this may mean that some properties are unavailable";
      continue;
    }

    QDomElement prop = propstat.namedItem( QLatin1String("prop") ).toElement();
    if ( prop.isNull() )
    {
      kDebug(7113) << "Error: no prop segment in this propstat.";
      return;
    }

    if ( hasMetaData( QLatin1String("davRequestResponse") ) )
    {
      QDomDocument doc;
      doc.appendChild(prop);
      entry.insert( KIO::UDSEntry::UDS_XML_PROPERTIES, doc.toString() );
    }

    for ( QDomNode n = prop.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
      QDomElement property = n.toElement();
      if (property.isNull())
        continue;

      if ( property.namespaceURI() != QLatin1String("DAV:") )
      {
        // break out - we're only interested in properties from the DAV namespace
        continue;
      }

      if ( property.tagName() == QLatin1String("creationdate") )
      {
        // Resource creation date. Should be is ISO 8601 format.
        entry.insert( KIO::UDSEntry::UDS_CREATION_TIME, parseDateTime( property.text(), property.attribute(QLatin1String("dt")) ) );
      }
      else if ( property.tagName() == QLatin1String("getcontentlength") )
      {
        // Content length (file size)
        entry.insert( KIO::UDSEntry::UDS_SIZE, property.text().toULong() );
      }
      else if ( property.tagName() == QLatin1String("displayname") )
      {
        // Name suitable for presentation to the user
        setMetaData( QLatin1String("davDisplayName"), property.text() );
      }
      else if ( property.tagName() == QLatin1String("source") )
      {
        // Source template location
        QDomElement source = property.namedItem( QLatin1String("link") ).toElement()
                                      .namedItem( QLatin1String("dst") ).toElement();
        if ( !source.isNull() )
          setMetaData( QLatin1String("davSource"), source.text() );
      }
      else if ( property.tagName() == QLatin1String("getcontentlanguage") )
      {
        // equiv. to Content-Language header on a GET
        setMetaData( QLatin1String("davContentLanguage"), property.text() );
      }
      else if ( property.tagName() == QLatin1String("getcontenttype") )
      {
        // Content type (mime type)
        // This may require adjustments for other server-side webdav implementations
        // (tested with Apache + mod_dav 1.0.3)
        if ( property.text() == QLatin1String("httpd/unix-directory") )
        {
          isDirectory = true;
        }
        else
        {
          mimeType = property.text();
        }
      }
      else if ( property.tagName() == QLatin1String("executable") )
      {
        // File executable status
        if ( property.text() == QLatin1String("T") )
          foundExecutable = true;

      }
      else if ( property.tagName() == QLatin1String("getlastmodified") )
      {
        // Last modification date
        entry.insert( KIO::UDSEntry::UDS_MODIFICATION_TIME, parseDateTime( property.text(), property.attribute(QLatin1String("dt")) ) );
      }
      else if ( property.tagName() == QLatin1String("getetag") )
      {
        // Entity tag
        setMetaData( QLatin1String("davEntityTag"), property.text() );
      }
      else if ( property.tagName() == QLatin1String("supportedlock") )
      {
        // Supported locking specifications
        for ( QDomNode n2 = property.firstChild(); !n2.isNull(); n2 = n2.nextSibling() )
        {
          QDomElement lockEntry = n2.toElement();
          if ( lockEntry.tagName() == QLatin1String("lockentry") )
          {
            QDomElement lockScope = lockEntry.namedItem( QLatin1String("lockscope") ).toElement();
            QDomElement lockType = lockEntry.namedItem( QLatin1String("locktype") ).toElement();
            if ( !lockScope.isNull() && !lockType.isNull() )
            {
              // Lock type was properly specified
              supportedLockCount++;
              QString scope = lockScope.firstChild().toElement().tagName();
              QString type = lockType.firstChild().toElement().tagName();

              setMetaData( QString::fromLatin1("davSupportedLockScope%1").arg(supportedLockCount), scope );
              setMetaData( QString::fromLatin1("davSupportedLockType%1").arg(supportedLockCount), type );
            }
          }
        }
      }
      else if ( property.tagName() == QLatin1String("lockdiscovery") )
      {
        // Lists the available locks
        davParseActiveLocks( property.elementsByTagName( QLatin1String("activelock") ), lockCount );
      }
      else if ( property.tagName() == QLatin1String("resourcetype") )
      {
        // Resource type. "Specifies the nature of the resource."
        if ( !property.namedItem( QLatin1String("collection") ).toElement().isNull() )
        {
          // This is a collection (directory)
          isDirectory = true;
        }
      }
      else
      {
        kDebug(7113) << "Found unknown webdav property: " << property.tagName();
      }
    }
  }

  setMetaData( QLatin1String("davLockCount"), QString::number(lockCount) );
  setMetaData( QLatin1String("davSupportedLockCount"), QString::number(supportedLockCount) );

  entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, isDirectory ? S_IFDIR : S_IFREG );

  if ( foundExecutable || isDirectory )
  {
    // File was executable, or is a directory.
    entry.insert( KIO::UDSEntry::UDS_ACCESS, 0700 );
  }
  else
  {
    entry.insert( KIO::UDSEntry::UDS_ACCESS, 0600 );
  }

  if ( !isDirectory && !mimeType.isEmpty() )
  {
    entry.insert( KIO::UDSEntry::UDS_MIME_TYPE, mimeType );
  }
}

void HTTPProtocol::davParseActiveLocks( const QDomNodeList& activeLocks,
                                        uint& lockCount )
{
  for ( int i = 0; i < activeLocks.count(); i++ )
  {
    QDomElement activeLock = activeLocks.item(i).toElement();

    lockCount++;
    // required
    QDomElement lockScope = activeLock.namedItem( QLatin1String("lockscope") ).toElement();
    QDomElement lockType = activeLock.namedItem( QLatin1String("locktype") ).toElement();
    QDomElement lockDepth = activeLock.namedItem( QLatin1String("depth") ).toElement();
    // optional
    QDomElement lockOwner = activeLock.namedItem( QLatin1String("owner") ).toElement();
    QDomElement lockTimeout = activeLock.namedItem( QLatin1String("timeout") ).toElement();
    QDomElement lockToken = activeLock.namedItem( QLatin1String("locktoken") ).toElement();

    if ( !lockScope.isNull() && !lockType.isNull() && !lockDepth.isNull() )
    {
      // lock was properly specified
      lockCount++;
      QString scope = lockScope.firstChild().toElement().tagName();
      QString type = lockType.firstChild().toElement().tagName();
      QString depth = lockDepth.text();

      setMetaData( QString::fromLatin1("davLockScope%1").arg( lockCount ), scope );
      setMetaData( QString::fromLatin1("davLockType%1").arg( lockCount ), type );
      setMetaData( QString::fromLatin1("davLockDepth%1").arg( lockCount ), depth );

      if ( !lockOwner.isNull() )
          setMetaData( QString::fromLatin1("davLockOwner%1").arg( lockCount ), lockOwner.text() );

      if ( !lockTimeout.isNull() )
          setMetaData( QString::fromLatin1("davLockTimeout%1").arg( lockCount ), lockTimeout.text() );

      if ( !lockToken.isNull() )
      {
        QDomElement tokenVal = lockScope.namedItem( QLatin1String("href") ).toElement();
        if ( !tokenVal.isNull() )
            setMetaData( QString::fromLatin1("davLockToken%1").arg( lockCount ), tokenVal.text() );
      }
    }
  }
}

long HTTPProtocol::parseDateTime( const QString& input, const QString& type )
{
  if ( type == QLatin1String("dateTime.tz") )
  {
    return KDateTime::fromString( input, KDateTime::ISODate ).toTime_t();
  }
  else if ( type == QLatin1String("dateTime.rfc1123") )
  {
    return KDateTime::fromString( input, KDateTime::RFCDate ).toTime_t();
  }

  // format not advertised... try to parse anyway
  time_t time = KDateTime::fromString( input, KDateTime::RFCDate ).toTime_t();
  if ( time != 0 )
    return time;

  return KDateTime::fromString( input, KDateTime::ISODate ).toTime_t();
}

QString HTTPProtocol::davProcessLocks()
{
    if ( hasMetaData( QLatin1String("davLockCount") ) )
    {
        QString response = QLatin1String("If:");
        int numLocks = metaData( QLatin1String("davLockCount") ).toInt();
    bool bracketsOpen = false;
    for ( int i = 0; i < numLocks; i++ )
    {
        if ( hasMetaData( QString::fromLatin1("davLockToken%1").arg(i) ) )
      {
          if ( hasMetaData( QString::fromLatin1("davLockURL%1").arg(i) ) )
        {
          if ( bracketsOpen )
          {
            response += QLatin1Char(')');
            bracketsOpen = false;
          }
          response += QLatin1String(" <") + metaData( QString::fromLatin1("davLockURL%1").arg(i) ) + QLatin1Char('>');
        }

        if ( !bracketsOpen )
        {
          response += QLatin1String(" (");
          bracketsOpen = true;
        }
        else
        {
          response += QLatin1Char(' ');
        }

        if ( hasMetaData( QString::fromLatin1("davLockNot%1").arg(i) ) )
          response += QLatin1String("Not ");

        response += QLatin1Char('<') + metaData( QString::fromLatin1("davLockToken%1").arg(i) ) + QLatin1Char('>');
      }
    }

    if ( bracketsOpen )
      response += QLatin1Char(')');

    response += QLatin1String("\r\n");
    return response;
  }

  return QString();
}

bool HTTPProtocol::davHostOk()
{
  // FIXME needs to be reworked. Switched off for now.
  return true;

  // cached?
  if ( m_davHostOk )
  {
    kDebug(7113) << "true";
    return true;
  }
  else if ( m_davHostUnsupported )
  {
    kDebug(7113) << " false";
    davError( -2 );
    return false;
  }

  m_request.method = HTTP_OPTIONS;

  // query the server's capabilities generally, not for a specific URL
  m_request.url.setPath(QLatin1String("*"));
  m_request.url.setQuery(QString());
  m_request.cacheTag.policy = CC_Reload;

  // clear davVersions variable, which holds the response to the DAV: header
  m_davCapabilities.clear();

  proceedUntilResponseHeader();

  if (m_davCapabilities.count())
  {
    for (int i = 0; i < m_davCapabilities.count(); i++)
    {
      bool ok;
      uint verNo = m_davCapabilities[i].toUInt(&ok);
      if (ok && verNo > 0 && verNo < 3)
      {
        m_davHostOk = true;
        kDebug(7113) << "Server supports DAV version" << verNo;
      }
    }

    if ( m_davHostOk )
      return true;
  }

  m_davHostUnsupported = true;
  davError( -2 );
  return false;
}

// This function is for closing proceedUntilResponseHeader(); requests
// Required because there may or may not be further info expected
void HTTPProtocol::davFinished()
{
  // TODO: Check with the DAV extension developers
  httpClose(m_request.isKeepAlive);
  finished();
}

void HTTPProtocol::mkdir( const KUrl& url, int )
{
  kDebug(7113) << url.url();

  if (!maybeSetRequestUrl(url))
    return;
  resetSessionSettings();

  m_request.method = DAV_MKCOL;
  m_request.url.setQuery(QString());
  m_request.cacheTag.policy = CC_Reload;

  proceedUntilResponseHeader();

  if ( m_request.responseCode == 201 )
    davFinished();
  else
    davError();
}

void HTTPProtocol::get( const KUrl& url )
{
  kDebug(7113) << url.url();

  if (!maybeSetRequestUrl(url))
    return;
  resetSessionSettings();

  m_request.method = HTTP_GET;

  QString tmp(metaData(QLatin1String("cache")));
  if (!tmp.isEmpty())
    m_request.cacheTag.policy = parseCacheControl(tmp);
  else
    m_request.cacheTag.policy = DEFAULT_CACHE_CONTROL;

  proceedUntilResponseContent();
  httpClose(m_request.isKeepAlive);
}

void HTTPProtocol::put( const KUrl &url, int, KIO::JobFlags flags )
{
  kDebug(7113) << url.url();

  if (!maybeSetRequestUrl(url))
    return;
  resetSessionSettings();

  // Webdav hosts are capable of observing overwrite == false
  if (!(flags & KIO::Overwrite) && m_protocol.startsWith("webdav")) { // krazy:exclude=strings
    // check to make sure this host supports WebDAV
    if ( !davHostOk() )
      return;

    QByteArray request = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
    "<D:propfind xmlns:D=\"DAV:\"><D:prop>"
      "<D:creationdate/>"
      "<D:getcontentlength/>"
      "<D:displayname/>"
      "<D:resourcetype/>"
      "</D:prop></D:propfind>";

    davSetRequest( request );

    // WebDAV Stat or List...
    m_request.method = DAV_PROPFIND;
    m_request.url.setQuery(QString());
    m_request.cacheTag.policy = CC_Reload;
    m_request.davData.depth = 0;

    proceedUntilResponseContent(true);

    if (m_request.responseCode == 207) {
      error(ERR_FILE_ALREADY_EXIST, QString());
      return;
    }

    m_isError = false;
  }

  m_request.method = HTTP_PUT;
  m_request.url.setQuery(QString());
  m_request.cacheTag.policy = CC_Reload;

  proceedUntilResponseHeader();

  kDebug(7113) << "error = " << m_isError;
  if (m_isError)
    return;

  kDebug(7113) << "responseCode = " << m_request.responseCode;

  httpClose(false); // Always close connection.

  if ( (m_request.responseCode >= 200) && (m_request.responseCode < 300) )
    finished();
  else
    httpPutError();
}

void HTTPProtocol::copy( const KUrl& src, const KUrl& dest, int, KIO::JobFlags flags )
{
  kDebug(7113) << src.url() << "->" << dest.url();

  if (!maybeSetRequestUrl(dest) || !maybeSetRequestUrl(src))
    return;
  resetSessionSettings();

  // destination has to be "http(s)://..."
  KUrl newDest = dest;
  if (newDest.protocol() == QLatin1String("webdavs"))
    newDest.setProtocol(QLatin1String("https"));
  else if (newDest.protocol() == QLatin1String("webdav"))
    newDest.setProtocol(QLatin1String("http"));

  m_request.method = DAV_COPY;
  m_request.davData.desturl = newDest.url();
  m_request.davData.overwrite = (flags & KIO::Overwrite);
  m_request.url.setQuery(QString());
  m_request.cacheTag.policy = CC_Reload;

  proceedUntilResponseHeader();

  // The server returns a HTTP/1.1 201 Created or 204 No Content on successful completion
  if ( m_request.responseCode == 201 || m_request.responseCode == 204 )
    davFinished();
  else
    davError();
}

void HTTPProtocol::rename( const KUrl& src, const KUrl& dest, KIO::JobFlags flags )
{
  kDebug(7113) << src.url() << "->" << dest.url();

  if (!maybeSetRequestUrl(dest) || !maybeSetRequestUrl(src))
    return;
  resetSessionSettings();

  // destination has to be "http://..."
  KUrl newDest = dest;
  if (newDest.protocol() == QLatin1String("webdavs"))
    newDest.setProtocol(QLatin1String("https"));
  else if (newDest.protocol() == QLatin1String("webdav"))
    newDest.setProtocol(QLatin1String("http"));

  m_request.method = DAV_MOVE;
  m_request.davData.desturl = newDest.url();
  m_request.davData.overwrite = (flags & KIO::Overwrite);
  m_request.url.setQuery(QString());
  m_request.cacheTag.policy = CC_Reload;

  proceedUntilResponseHeader();

  // Work around strict Apache-2 WebDAV implementation which refuses to cooperate
  // with webdav://host/directory, instead requiring webdav://host/directory/
  // (strangely enough it accepts Destination: without a trailing slash)
  // See BR# 209508 and BR#187970
  if ( m_request.responseCode == 301) {
    m_request.url = m_request.redirectUrl;
    m_request.method = DAV_MOVE;
    m_request.davData.desturl = newDest.url();
    m_request.davData.overwrite = (flags & KIO::Overwrite);
    m_request.url.setQuery(QString());
    m_request.cacheTag.policy = CC_Reload;
    if (m_wwwAuth) {  // force re-authentication...
        delete m_wwwAuth;
        m_wwwAuth = 0;
    }
    proceedUntilResponseHeader();
  }

  if ( m_request.responseCode == 201 )
    davFinished();
  else
    davError();
}

void HTTPProtocol::del( const KUrl& url, bool )
{
  kDebug(7113) << url.url();

  if (!maybeSetRequestUrl(url))
    return;
  resetSessionSettings();

  m_request.method = HTTP_DELETE;
  m_request.url.setQuery(QString());;
  m_request.cacheTag.policy = CC_Reload;

  proceedUntilResponseHeader();

  // Work around strict Apache-2 WebDAV implementation which refuses to cooperate
  // with webdav://host/directory, instead requiring webdav://host/directory/
  // (strangely enough it accepts Destination: without a trailing slash)
  // See BR# 209508 and BR#187970.
  if (m_request.responseCode == 301) {
    m_request.url = m_request.redirectUrl;
    m_request.method = HTTP_DELETE;
    m_request.url.setQuery(QString());;
    m_request.cacheTag.policy = CC_Reload;
    if (m_wwwAuth) { // force re-authentication...
        delete m_wwwAuth;
        m_wwwAuth = 0;
    }
    proceedUntilResponseHeader();
  }

  // The server returns a HTTP/1.1 200 Ok or HTTP/1.1 204 No Content
  // on successful completion
  if ( m_protocol.startsWith( "webdav" ) ) { // krazy:exclude=strings
    if ( m_request.responseCode == 200 || m_request.responseCode == 204 )
      davFinished();
    else
      davError();
  } else {
    if ( m_request.responseCode == 200 || m_request.responseCode == 204 )
      finished();
    else
      error( ERR_SLAVE_DEFINED, i18n( "The resource cannot be deleted." ) );
  }
}

void HTTPProtocol::post( const KUrl& url )
{
  kDebug(7113) << url.url();

  if (!maybeSetRequestUrl(url))
    return;
  resetSessionSettings();

  m_request.method = HTTP_POST;
  m_request.cacheTag.policy= CC_Reload;

  proceedUntilResponseContent();
}

void HTTPProtocol::davLock( const KUrl& url, const QString& scope,
                            const QString& type, const QString& owner )
{
  kDebug(7113) << url.url();

  if (!maybeSetRequestUrl(url))
    return;
  resetSessionSettings();

  m_request.method = DAV_LOCK;
  m_request.url.setQuery(QString());
  m_request.cacheTag.policy= CC_Reload;

  /* Create appropriate lock XML request. */
  QDomDocument lockReq;

  QDomElement lockInfo = lockReq.createElementNS( QLatin1String("DAV:"), QLatin1String("lockinfo") );
  lockReq.appendChild( lockInfo );

  QDomElement lockScope = lockReq.createElement( QLatin1String("lockscope") );
  lockInfo.appendChild( lockScope );

  lockScope.appendChild( lockReq.createElement( scope ) );

  QDomElement lockType = lockReq.createElement( QLatin1String("locktype") );
  lockInfo.appendChild( lockType );

  lockType.appendChild( lockReq.createElement( type ) );

  if ( !owner.isNull() ) {
    QDomElement ownerElement = lockReq.createElement( QLatin1String("owner") );
    lockReq.appendChild( ownerElement );

    QDomElement ownerHref = lockReq.createElement( QLatin1String("href") );
    ownerElement.appendChild( ownerHref );

    ownerHref.appendChild( lockReq.createTextNode( owner ) );
  }

  // insert the document into the POST buffer
  m_POSTbuf = lockReq.toByteArray();

  proceedUntilResponseContent( true );

  if ( m_request.responseCode == 200 ) {
    // success
    QDomDocument multiResponse;
    multiResponse.setContent( m_webDavDataBuf, true );

    QDomElement prop = multiResponse.documentElement().namedItem( QLatin1String("prop") ).toElement();

    QDomElement lockdiscovery = prop.namedItem( QLatin1String("lockdiscovery") ).toElement();

    uint lockCount = 0;
    davParseActiveLocks( lockdiscovery.elementsByTagName( QLatin1String("activelock") ), lockCount );

    setMetaData( QLatin1String("davLockCount"), QString::number( lockCount ) );

    finished();

  } else
    davError();
}

void HTTPProtocol::davUnlock( const KUrl& url )
{
  kDebug(7113) << url.url();

  if (!maybeSetRequestUrl(url))
    return;
  resetSessionSettings();

  m_request.method = DAV_UNLOCK;
  m_request.url.setQuery(QString());
  m_request.cacheTag.policy= CC_Reload;

  proceedUntilResponseContent( true );

  if ( m_request.responseCode == 200 )
    finished();
  else
    davError();
}

QString HTTPProtocol::davError( int code /* = -1 */, const QString &_url )
{
  bool callError = false;
  if ( code == -1 ) {
    code = m_request.responseCode;
    callError = true;
  }
  if ( code == -2 ) {
    callError = true;
  }

  QString url = _url;
  if ( !url.isNull() )
    url = m_request.url.url();

  QString action, errorString;
  KIO::Error kError;

  // for 412 Precondition Failed
  QString ow = i18n( "Otherwise, the request would have succeeded." );

  switch ( m_request.method ) {
    case DAV_PROPFIND:
      action = i18nc( "request type", "retrieve property values" );
      break;
    case DAV_PROPPATCH:
      action = i18nc( "request type", "set property values" );
      break;
    case DAV_MKCOL:
      action = i18nc( "request type", "create the requested folder" );
      break;
    case DAV_COPY:
      action = i18nc( "request type", "copy the specified file or folder" );
      break;
    case DAV_MOVE:
      action = i18nc( "request type", "move the specified file or folder" );
      break;
    case DAV_SEARCH:
      action = i18nc( "request type", "search in the specified folder" );
      break;
    case DAV_LOCK:
      action = i18nc( "request type", "lock the specified file or folder" );
      break;
    case DAV_UNLOCK:
      action = i18nc( "request type", "unlock the specified file or folder" );
      break;
    case HTTP_DELETE:
      action = i18nc( "request type", "delete the specified file or folder" );
      break;
    case HTTP_OPTIONS:
      action = i18nc( "request type", "query the server's capabilities" );
      break;
    case HTTP_GET:
      action = i18nc( "request type", "retrieve the contents of the specified file or folder" );
      break;
    case DAV_REPORT:
      action = i18nc( "request type", "run a report in the specified folder" );
      break;
    case HTTP_PUT:
    case HTTP_POST:
    case HTTP_HEAD:
    default:
      // this should not happen, this function is for webdav errors only
      Q_ASSERT(0);
  }

  // default error message if the following code fails
  kError = ERR_INTERNAL;
  errorString = i18nc("%1: code, %2: request type", "An unexpected error (%1) occurred "
                      "while attempting to %2.", code, action);

  switch ( code )
  {
    case -2:
      // internal error: OPTIONS request did not specify DAV compliance
      kError = ERR_UNSUPPORTED_PROTOCOL;
      errorString = i18n("The server does not support the WebDAV protocol.");
      break;
    case 207:
      // 207 Multi-status
    {
      // our error info is in the returned XML document.
      // retrieve the XML document

      // there was an error retrieving the XML document.
      // ironic, eh?
      if ( !readBody( true ) && m_isError )
        return QString();

      QStringList errors;
      QDomDocument multiResponse;

      multiResponse.setContent( m_webDavDataBuf, true );

      QDomElement multistatus = multiResponse.documentElement().namedItem( QLatin1String("multistatus") ).toElement();

      QDomNodeList responses = multistatus.elementsByTagName( QLatin1String("response") );

      for (int i = 0; i < responses.count(); i++)
      {
        int errCode;
        QString errUrl;

        QDomElement response = responses.item(i).toElement();
        QDomElement code = response.namedItem( QLatin1String("status") ).toElement();

        if ( !code.isNull() )
        {
          errCode = codeFromResponse( code.text() );
          QDomElement href = response.namedItem( QLatin1String("href") ).toElement();
          if ( !href.isNull() )
            errUrl = href.text();
          errors << davError( errCode, errUrl );
        }
      }

      //kError = ERR_SLAVE_DEFINED;
      errorString = i18nc( "%1: request type, %2: url",
                           "An error occurred while attempting to %1, %2. A "
                           "summary of the reasons is below.", action, url );

      errorString += QLatin1String("<ul>");

      for ( QStringList::const_iterator it = errors.constBegin(); it != errors.constEnd(); ++it )
        errorString += QLatin1String("<li>") + *it + QLatin1String("</li>");

      errorString += QLatin1String("</ul>");
    }
    case 403:
    case 500: // hack: Apache mod_dav returns this instead of 403 (!)
      // 403 Forbidden
      kError = ERR_ACCESS_DENIED;
      errorString = i18nc( "%1: request type", "Access was denied while attempting to %1.",  action );
      break;
    case 405:
      // 405 Method Not Allowed
      if ( m_request.method == DAV_MKCOL )
      {
        kError = ERR_DIR_ALREADY_EXIST;
        errorString = i18n("The specified folder already exists.");
      }
      break;
    case 409:
      // 409 Conflict
      kError = ERR_ACCESS_DENIED;
      errorString = i18n("A resource cannot be created at the destination "
                  "until one or more intermediate collections (folders) "
                  "have been created.");
      break;
    case 412:
      // 412 Precondition failed
      if ( m_request.method == DAV_COPY || m_request.method == DAV_MOVE )
      {
        kError = ERR_ACCESS_DENIED;
        errorString = i18n("The server was unable to maintain the liveness of "
                           "the properties listed in the propertybehavior XML "
                           "element or you attempted to overwrite a file while "
                           "requesting that files are not overwritten. %1",
                             ow );

      }
      else if ( m_request.method == DAV_LOCK )
      {
        kError = ERR_ACCESS_DENIED;
        errorString = i18n("The requested lock could not be granted. %1",  ow );
      }
      break;
    case 415:
      // 415 Unsupported Media Type
      kError = ERR_ACCESS_DENIED;
      errorString = i18n("The server does not support the request type of the body.");
      break;
    case 423:
      // 423 Locked
      kError = ERR_ACCESS_DENIED;
      errorString = i18nc( "%1: request type", "Unable to %1 because the resource is locked.",  action );
      break;
    case 425:
      // 424 Failed Dependency
      errorString = i18n("This action was prevented by another error.");
      break;
    case 502:
      // 502 Bad Gateway
      if ( m_request.method == DAV_COPY || m_request.method == DAV_MOVE )
      {
        kError = ERR_WRITE_ACCESS_DENIED;
        errorString = i18nc( "%1: request type", "Unable to %1 because the destination server refuses "
                           "to accept the file or folder.",  action );
      }
      break;
    case 507:
      // 507 Insufficient Storage
      kError = ERR_DISK_FULL;
      errorString = i18n("The destination resource does not have sufficient space "
                         "to record the state of the resource after the execution "
                         "of this method.");
      break;
  }

  // if ( kError != ERR_SLAVE_DEFINED )
  //errorString += " (" + url + ')';

  if ( callError )
    error( ERR_SLAVE_DEFINED, errorString );

  return errorString;
}

void HTTPProtocol::httpPutError()
{
  QString action, errorString;
  KIO::Error kError;

  switch ( m_request.method ) {
    case HTTP_PUT:
      action = i18nc("request type", "upload %1", m_request.url.prettyUrl());
      break;
    default:
      // this should not happen, this function is for http errors only
      // ### WTF, what about HTTP_GET?
      Q_ASSERT(0);
  }

  // default error message if the following code fails
  kError = ERR_INTERNAL;
  errorString = i18nc("%1: response code, %2: request type",
                      "An unexpected error (%1) occurred while attempting to %2.",
                       m_request.responseCode, action);

  switch ( m_request.responseCode )
  {
    case 403:
    case 405:
    case 500: // hack: Apache mod_dav returns this instead of 403 (!)
      // 403 Forbidden
      // 405 Method Not Allowed
      kError = ERR_ACCESS_DENIED;
      errorString = i18nc( "%1: request type", "Access was denied while attempting to %1.",  action );
      break;
    case 409:
      // 409 Conflict
      kError = ERR_ACCESS_DENIED;
      errorString = i18n("A resource cannot be created at the destination "
                  "until one or more intermediate collections (folders) "
                  "have been created.");
      break;
    case 423:
      // 423 Locked
      kError = ERR_ACCESS_DENIED;
      errorString = i18nc( "%1: request type", "Unable to %1 because the resource is locked.",  action );
      break;
    case 502:
      // 502 Bad Gateway
      kError = ERR_WRITE_ACCESS_DENIED;
      errorString = i18nc( "%1: request type", "Unable to %1 because the destination server refuses "
                         "to accept the file or folder.",  action );
      break;
    case 507:
      // 507 Insufficient Storage
      kError = ERR_DISK_FULL;
      errorString = i18n("The destination resource does not have sufficient space "
                         "to record the state of the resource after the execution "
                         "of this method.");
      break;
  }

  // if ( kError != ERR_SLAVE_DEFINED )
  //errorString += " (" + url + ')';

  error( ERR_SLAVE_DEFINED, errorString );
}

bool HTTPProtocol::sendErrorPageNotification()
{
    if (!m_request.preferErrorPage)
        return false;

    if (m_isLoadingErrorPage)
        kWarning(7113) << "called twice during one request, something is probably wrong.";

    m_isLoadingErrorPage = true;
    SlaveBase::errorPage();
    return true;
}

bool HTTPProtocol::isOffline()
{
  // ### TEMPORARY WORKAROUND (While investigating why solid may
  // produce false positives)
  return false;

  Solid::Networking::Status status = Solid::Networking::status();

  kDebug(7113) << "networkstatus:" << status;

  // on error or unknown, we assume online
  return status == Solid::Networking::Unconnected;
}

void HTTPProtocol::multiGet(const QByteArray &data)
{
    QDataStream stream(data);
    quint32 n;
    stream >> n;

    kDebug(7113) << n;

    HTTPRequest saveRequest;
    if (m_isBusy)
        saveRequest = m_request;

    resetSessionSettings();

    for (unsigned i = 0; i < n; i++) {
        KUrl url;
        stream >> url >> mIncomingMetaData;

        if (!maybeSetRequestUrl(url))
            continue;

        //### should maybe call resetSessionSettings() if the server/domain is
        //    different from the last request!

        kDebug(7113) << url.url();

        m_request.method = HTTP_GET;
        m_request.isKeepAlive = true;   //readResponseHeader clears it if necessary

        QString tmp = metaData(QLatin1String("cache"));
        if (!tmp.isEmpty())
            m_request.cacheTag.policy= parseCacheControl(tmp);
        else
            m_request.cacheTag.policy= DEFAULT_CACHE_CONTROL;

        m_requestQueue.append(m_request);
    }

    if (m_isBusy)
        m_request = saveRequest;
#if 0
    if (!m_isBusy) {
        m_isBusy = true;
        QMutableListIterator<HTTPRequest> it(m_requestQueue);
        while (it.hasNext()) {
            m_request = it.next();
            it.remove();
            proceedUntilResponseContent();
        }
        m_isBusy = false;
    }
#endif
    if (!m_isBusy) {
        m_isBusy = true;
        QMutableListIterator<HTTPRequest> it(m_requestQueue);
        // send the requests
        while (it.hasNext()) {
            m_request = it.next();
            sendQuery();
            // save the request state so we can pick it up again in the collection phase
            it.setValue(m_request);
            kDebug(7113) << "check one: isKeepAlive =" << m_request.isKeepAlive;
            if (m_request.cacheTag.ioMode != ReadFromCache) {
                m_server.initFrom(m_request);
            }
        }
        // collect the responses
        //### for the moment we use a hack: instead of saving and restoring request-id
        //    we just count up like ParallelGetJobs does.
        int requestId = 0;
        Q_FOREACH (const HTTPRequest &r, m_requestQueue) {
            m_request = r;
            kDebug(7113) << "check two: isKeepAlive =" << m_request.isKeepAlive;
            setMetaData(QLatin1String("request-id"), QString::number(requestId++));
            sendAndKeepMetaData();
            if (!(readResponseHeader() && readBody())) {
                return;
            }
            // the "next job" signal for ParallelGetJob is data of size zero which
            // readBody() sends without our intervention.
            kDebug(7113) << "check three: isKeepAlive =" << m_request.isKeepAlive;
            httpClose(m_request.isKeepAlive);  //actually keep-alive is mandatory for pipelining
        }

        finished();
        m_requestQueue.clear();
        m_isBusy = false;
    }
}

ssize_t HTTPProtocol::write (const void *_buf, size_t nbytes)
{
  size_t sent = 0;
  const char* buf = static_cast<const char*>(_buf);
  while (sent < nbytes)
  {
    int n = TCPSlaveBase::write(buf + sent, nbytes - sent);

    if (n < 0) {
      // some error occurred
      return -1;
    }

    sent += n;
  }

  return sent;
}

void HTTPProtocol::clearUnreadBuffer()
{
    m_unreadBuf.clear();
}

// Note: the implementation of unread/readBuffered assumes that unread will only
// be used when there is extra data we don't want to handle, and not to wait for more data.
void HTTPProtocol::unread(char *buf, size_t size)
{
    // implement LIFO (stack) semantics
    const int newSize = m_unreadBuf.size() + size;
    m_unreadBuf.resize(newSize);
    for (size_t i = 0; i < size; i++) {
        m_unreadBuf.data()[newSize - i - 1] = buf[i];
    }
    if (size) {
        //hey, we still have data, closed connection or not!
        m_isEOF = false;
    }
}

size_t HTTPProtocol::readBuffered(char *buf, size_t size, bool unlimited)
{
    size_t bytesRead = 0;
    if (!m_unreadBuf.isEmpty()) {
        const int bufSize = m_unreadBuf.size();
        bytesRead = qMin((int)size, bufSize);

        for (size_t i = 0; i < bytesRead; i++) {
            buf[i] = m_unreadBuf.constData()[bufSize - i - 1];
        }
        m_unreadBuf.truncate(bufSize - bytesRead);

        // If we have an unread buffer and the size of the content returned by the
        // server is unknown, e.g. chuncked transfer, return the bytes read here since
        // we may already have enough data to complete the response and don't want to
        // wait for more. See BR# 180631.
        if (unlimited)
            return bytesRead;
    }
    if (bytesRead < size) {
        int rawRead = TCPSlaveBase::read(buf + bytesRead, size - bytesRead);
        if (rawRead < 1) {
            m_isEOF = true;
            return bytesRead;
        }
        bytesRead += rawRead;
    }
    return bytesRead;
}

//### this method will detect an n*(\r\n) sequence if it crosses invocations.
//    it will look (n*2 - 1) bytes before start at most and never before buf, naturally.
//    supported number of newlines are one and two, in line with HTTP syntax.
// return true if numNewlines newlines were found.
bool HTTPProtocol::readDelimitedText(char *buf, int *idx, int end, int numNewlines)
{
    Q_ASSERT(numNewlines >=1 && numNewlines <= 2);
    char mybuf[64]; //somewhere close to the usual line length to avoid unread()ing too much
    int pos = *idx;
    while (pos < end && !m_isEOF) {
        int step = qMin((int)sizeof(mybuf), end - pos);
        if (m_isChunked) {
            //we might be reading the end of the very last chunk after which there is no data.
            //don't try to read any more bytes than there are because it causes stalls
            //(yes, it shouldn't stall but it does)
            step = 1;
        }
        size_t bufferFill = readBuffered(mybuf, step);

        for (size_t i = 0; i < bufferFill ; i++, pos++) {
            // we copy the data from mybuf to buf immediately and look for the newlines in buf.
            // that way we don't miss newlines split over several invocations of this method.
            buf[pos] = mybuf[i];

            // did we just copy one or two times the (usually) \r\n delimiter?
            // until we find even more broken webservers in the wild let's assume that they either
            // send \r\n (RFC compliant) or \n (broken) as delimiter...
            if (buf[pos] == '\n') {
                bool found = numNewlines == 1;
                if (!found) {   // looking for two newlines
                    found = ((pos >= 1 && buf[pos - 1] == '\n') ||
                             (pos >= 3 && buf[pos - 3] == '\r' && buf[pos - 2] == '\n' &&
                                          buf[pos - 1] == '\r'));
                }
                if (found) {
                    i++;    // unread bytes *after* CRLF
                    unread(&mybuf[i], bufferFill - i);
                    *idx = pos + 1;
                    return true;
                }
            }
        }
    }
    *idx = pos;
    return false;
}

static bool isCompatibleNextUrl(const KUrl &previous, const KUrl &now)
{
    if (previous.host() != now.host() || previous.port() != now.port()) {
        return false;
    }
    if (previous.user().isEmpty() && previous.pass().isEmpty()) {
        return true;
    }
    return previous.user() == now.user() && previous.pass() == now.pass();
}

bool HTTPProtocol::httpShouldCloseConnection()
{
  kDebug(7113) << "Keep Alive:" << m_request.isKeepAlive;

  if (!isConnected()) {
      return false;
  }

  if (m_request.method != HTTP_GET && m_request.method != HTTP_POST) {
      return true;
  }

  // TODO compare current proxy state against proxy needs of next request,
  // *when* we actually have variable proxy settings!

  if (isValidProxy(m_request.proxyUrl))  {
      return !isCompatibleNextUrl(m_server.proxyUrl, m_request.proxyUrl);
  }
  return !isCompatibleNextUrl(m_server.url, m_request.url);
}

bool HTTPProtocol::httpOpenConnection()
{
  kDebug(7113);
  m_server.clear();

  // Only save proxy auth information after proxy authentication has
  // actually taken place, which will set up exactly this connection.
  disconnect(socket(), SIGNAL(connected()),
             this, SLOT(saveProxyAuthenticationForSocket()));

  clearUnreadBuffer();

  bool connectOk = false;
  if (isHttpProxy(m_request.proxyUrl) && !isAutoSsl()) {
      connectOk = connectToHost(m_request.proxyUrl.protocol(), m_request.proxyUrl.host(), m_request.proxyUrl.port());
  } else {
      connectOk = connectToHost(QString::fromLatin1(m_protocol), m_request.url.host(), m_request.url.port(defaultPort()));
  }

  if (!connectOk) {
      return false;
  }

  // Disable Nagle's algorithm, i.e turn on TCP_NODELAY.
  KTcpSocket *sock = qobject_cast<KTcpSocket *>(socket());
  if (sock) {
      // kDebug(7113) << "TCP_NODELAY:" << sock->socketOption(QAbstractSocket::LowDelayOption);
      sock->setSocketOption(QAbstractSocket::LowDelayOption, 1);
  }

  m_server.initFrom(m_request);
  connected();
  return true;
}

bool HTTPProtocol::satisfyRequestFromCache(bool *cacheHasPage)
{
    kDebug(7113);

    if (m_request.cacheTag.useCache) {
        const bool offline = isOffline();

        if (offline && m_request.cacheTag.policy != KIO::CC_Reload) {
            m_request.cacheTag.policy= KIO::CC_CacheOnly;
        }

        const bool isCacheOnly = m_request.cacheTag.policy == KIO::CC_CacheOnly;
        const CacheTag::CachePlan plan = m_request.cacheTag.plan(m_maxCacheAge);

        bool openForReading = false;
        if (plan == CacheTag::UseCached || plan == CacheTag::ValidateCached) {
            openForReading = cacheFileOpenRead();

            if (!openForReading && (isCacheOnly || offline)) {
                // cache-only or offline -> we give a definite answer and it is "no"
                *cacheHasPage = false;
                if (isCacheOnly) {
                    error(ERR_DOES_NOT_EXIST, m_request.url.url());
                } else if (offline) {
                    error(ERR_COULD_NOT_CONNECT, m_request.url.url());
                }
                return true;
            }
        }

        if (openForReading) {
            m_request.cacheTag.ioMode = ReadFromCache;
            *cacheHasPage = true;
            // return false if validation is required, so a network request will be sent
            return m_request.cacheTag.plan(m_maxCacheAge) == CacheTag::UseCached;
        }
    }
    *cacheHasPage = false;
    return false;
}

QString HTTPProtocol::formatRequestUri() const
{
    // Only specify protocol, host and port when they are not already clear, i.e. when
    // we handle HTTP proxying ourself and the proxy server needs to know them.
    // Sending protocol/host/port in other cases confuses some servers, and it's not their fault.
    if (isHttpProxy(m_request.proxyUrl) && !isAutoSsl()) {
        KUrl u;

        QString protocol = QString::fromLatin1(m_protocol);
        if (protocol.startsWith(QLatin1String("webdav"))) {
            protocol.replace(0, strlen("webdav"), QLatin1String("http"));
        }
        u.setProtocol(protocol);

        u.setHost(m_request.url.host());
        // if the URL contained the default port it should have been stripped earlier
        Q_ASSERT(m_request.url.port() != defaultPort());
        u.setPort(m_request.url.port());
        u.setEncodedPathAndQuery(m_request.url.encodedPathAndQuery(
                                    KUrl::LeaveTrailingSlash, KUrl::AvoidEmptyPath));
        return u.url();
    } else {
        return m_request.url.encodedPathAndQuery(KUrl::LeaveTrailingSlash, KUrl::AvoidEmptyPath);
    }
}

/**
 * This function is responsible for opening up the connection to the remote
 * HTTP server and sending the header.  If this requires special
 * authentication or other such fun stuff, then it will handle it.  This
 * function will NOT receive anything from the server, however.  This is in
 * contrast to previous incarnations of 'httpOpen' as this method used to be
 * called.
 *
 * The basic process now is this:
 *
 * 1) Open up the socket and port
 * 2) Format our request/header
 * 3) Send the header to the remote server
 * 4) Call sendBody() if the HTTP method requires sending body data
 */
bool HTTPProtocol::sendQuery()
{
  kDebug(7113);

  // Cannot have an https request without autoSsl!  This can
  // only happen if  the current installation does not support SSL...
  if (isEncryptedHttpVariety(m_protocol) && !isAutoSsl()) {
      error(ERR_UNSUPPORTED_PROTOCOL, QString::fromLatin1(m_protocol));
    return false;
  }

  m_request.cacheTag.ioMode = NoCache;
  m_request.cacheTag.servedDate = -1;
  m_request.cacheTag.lastModifiedDate = -1;
  m_request.cacheTag.expireDate = -1;

  QString header;

  bool hasBodyData = false;
  bool hasDavData = false;

  {
    header = QString::fromLatin1(methodString(m_request.method));
    QString davHeader;

    // Fill in some values depending on the HTTP method to guide further processing
    switch (m_request.method)
    {
    case HTTP_GET: {
        bool cacheHasPage = false;
        if (satisfyRequestFromCache(&cacheHasPage)) {
            kDebug(7113) << "cacheHasPage =" << cacheHasPage;
            return cacheHasPage;
        }
        if (!cacheHasPage) {
            // start a new cache file later if appropriate
            m_request.cacheTag.ioMode = WriteToCache;
        }
    }
    case HTTP_HEAD:
        break;
    case HTTP_PUT:
    case HTTP_POST:
        hasBodyData = true;
        break;
    case HTTP_DELETE:
    case HTTP_OPTIONS:
        break;
    case DAV_PROPFIND:
        hasDavData = true;
        davHeader = QLatin1String("Depth: ");
        if ( hasMetaData( QLatin1String("davDepth") ) )
        {
          kDebug(7113) << "Reading DAV depth from metadata: " << metaData( QLatin1String("davDepth") );
          davHeader += metaData( QLatin1String("davDepth") );
        }
        else
        {
          if ( m_request.davData.depth == 2 )
              davHeader += QLatin1String("infinity");
          else
              davHeader += QString::number( m_request.davData.depth );
        }
        davHeader += QLatin1String("\r\n");
        break;
    case DAV_PROPPATCH:
        hasDavData = true;
        break;
    case DAV_MKCOL:
        break;
    case DAV_COPY:
    case DAV_MOVE:
        davHeader = QLatin1String("Destination: ") + m_request.davData.desturl;
        // infinity depth means copy recursively
        // (optional for copy -> but is the desired action)
        davHeader += QLatin1String("\r\nDepth: infinity\r\nOverwrite: ");
        davHeader += QLatin1Char(m_request.davData.overwrite ? 'T' : 'F');
        davHeader += QLatin1String("\r\n");
        break;
    case DAV_LOCK:
        davHeader = QLatin1String("Timeout: ");
        {
          uint timeout = 0;
          if ( hasMetaData( QLatin1String("davTimeout") ) )
            timeout = metaData( QLatin1String("davTimeout") ).toUInt();
          if ( timeout == 0 )
            davHeader += QLatin1String("Infinite");
          else
              davHeader += QString::fromLatin1("Seconds-%1").arg(timeout);
        }
        davHeader += QLatin1String("\r\n");
        hasDavData = true;
        break;
    case DAV_UNLOCK:
        davHeader = QLatin1String("Lock-token: ") + metaData(QLatin1String("davLockToken")) + QLatin1String("\r\n");
        break;
    case DAV_SEARCH:
    case DAV_REPORT:
        hasDavData = true;
        /* fall through */
    case DAV_SUBSCRIBE:
    case DAV_UNSUBSCRIBE:
    case DAV_POLL:
        break;
    default:
        error (ERR_UNSUPPORTED_ACTION, QString());
        return false;
    }
    // DAV_POLL; DAV_NOTIFY

    header += formatRequestUri() + QLatin1String(" HTTP/1.1\r\n"); /* start header */

    /* support for virtual hosts and required by HTTP 1.1 */
    header += QLatin1String("Host: ") + m_request.encoded_hostname;
    if (m_request.url.port(defaultPort()) != defaultPort()) {
        header += QString::fromLatin1(":%1").arg(m_request.url.port());
    }
    header += QLatin1String("\r\n");

    // Support old HTTP/1.0 style keep-alive header for compatibility
    // purposes as well as performance improvements while giving end
    // users the ability to disable this feature for proxy servers that
    // don't support it, e.g. junkbuster proxy server.
    if (isHttpProxy(m_request.proxyUrl) && !isAutoSsl()) {
        header += QLatin1String("Proxy-Connection: ");
    } else {
        header += QLatin1String("Connection: ");
    }
    if (m_request.isKeepAlive) {
        header += QLatin1String("Keep-Alive\r\n");
    } else {
        header += QLatin1String("close\r\n");
    }

    if (!m_request.userAgent.isEmpty())
    {
        header += QLatin1String("User-Agent: ");
        header += m_request.userAgent;
        header += QLatin1String("\r\n");
    }

    if (!m_request.referrer.isEmpty())
    {
        header += QLatin1String("Referer: "); //Don't try to correct spelling!
        header += m_request.referrer;
        header += QLatin1String("\r\n");
    }

    if ( m_request.endoffset > m_request.offset )
    {
        header += QString::fromLatin1("Range: bytes=%1-%2\r\n").arg(KIO::number(m_request.offset))
                         .arg(KIO::number(m_request.endoffset));
        kDebug(7103) << "kio_http : Range = " << KIO::number(m_request.offset) <<
                        " - "  << KIO::number(m_request.endoffset);
    }
    else if ( m_request.offset > 0 && m_request.endoffset == 0 )
    {
        header += QString::fromLatin1("Range: bytes=%1-\r\n").arg(KIO::number(m_request.offset));
        kDebug(7103) << "kio_http : Range = " << KIO::number(m_request.offset);
    }

    if ( !m_request.cacheTag.useCache || m_request.cacheTag.policy==CC_Reload )
    {
      /* No caching for reload */
      header += QLatin1String("Pragma: no-cache\r\n"); /* for HTTP/1.0 caches */
      header += QLatin1String("Cache-control: no-cache\r\n"); /* for HTTP >=1.1 caches */
    }
    else if (m_request.cacheTag.plan(m_maxCacheAge) == CacheTag::ValidateCached)
    {
      kDebug(7113) << "needs validation, performing conditional get.";
      /* conditional get */
      if (!m_request.cacheTag.etag.isEmpty())
        header += QLatin1String("If-None-Match: ")+m_request.cacheTag.etag+QLatin1String("\r\n");

      if (m_request.cacheTag.lastModifiedDate != -1) {
        QString httpDate = formatHttpDate(m_request.cacheTag.lastModifiedDate);
        header += QLatin1String("If-Modified-Since: ") + httpDate + QLatin1String("\r\n");
        setMetaData(QLatin1String("modified"), httpDate);
      }
    }

    header += QLatin1String("Accept: ");
    const QString acceptHeader = metaData(QLatin1String("accept"));
    if (!acceptHeader.isEmpty())
      header += acceptHeader;
    else
      header += QLatin1String(DEFAULT_ACCEPT_HEADER);
    header += QLatin1String("\r\n");

    if (m_request.allowTransferCompression)
      header += QLatin1String("Accept-Encoding: x-gzip, x-deflate, gzip, deflate\r\n");

    if (!m_request.charsets.isEmpty())
      header += QLatin1String("Accept-Charset: ") + m_request.charsets + QLatin1String("\r\n");

    if (!m_request.languages.isEmpty())
      header += QLatin1String("Accept-Language: ") + m_request.languages + QLatin1String("\r\n");

    QString cookieStr;
    const QString cookieMode = metaData(QLatin1String("cookies")).toLower();

    if (cookieMode == QLatin1String("none"))
    {
      m_request.cookieMode = HTTPRequest::CookiesNone;
    }
    else if (cookieMode == QLatin1String("manual"))
    {
      m_request.cookieMode = HTTPRequest::CookiesManual;
      cookieStr = metaData(QLatin1String("setcookies"));
    }
    else
    {
      m_request.cookieMode = HTTPRequest::CookiesAuto;
      if (m_request.useCookieJar)
        cookieStr = findCookies(m_request.url.url());
    }

    if (!cookieStr.isEmpty())
      header += cookieStr + QLatin1String("\r\n");

    QString customHeader = metaData( QLatin1String("customHTTPHeader") );
    if (!customHeader.isEmpty())
    {
      header += sanitizeCustomHTTPHeader(customHeader);
      header += QLatin1String("\r\n");
    }

    QString contentType = metaData(QLatin1String("content-type"));
    if ((m_request.method == HTTP_POST || m_request.method == HTTP_PUT)
    && !contentType.isEmpty())
    {
      header += contentType;
      header += QLatin1String("\r\n");
    }

    // Remember that at least one failed (with 401 or 407) request/response
    // roundtrip is necessary for the server to tell us that it requires
    // authentication.
    // We proactively add authentication headers if we have cached credentials
    // to avoid the extra roundtrip where possible.
    // (TODO: implement this caching)
    header += authenticationHeader();

    if ( m_protocol == "webdav" || m_protocol == "webdavs" )
    {
      header += davProcessLocks();

      // add extra webdav headers, if supplied
      davHeader += metaData(QLatin1String("davHeader"));

      // Set content type of webdav data
      if (hasDavData)
        davHeader += QLatin1String("Content-Type: text/xml; charset=utf-8\r\n");

      // add extra header elements for WebDAV
      header += davHeader;
    }
  }

  kDebug(7103) << "============ Sending Header:";
  Q_FOREACH (const QString &s, header.split(QLatin1String("\r\n"), QString::SkipEmptyParts)) {
    kDebug(7103) << s;
  }

  // End the header iff there is no payload data. If we do have payload data
  // sendBody() will add another field to the header, Content-Length.
  if (!hasBodyData && !hasDavData)
    header += QLatin1String("\r\n");

  // Check the reusability of the current connection.
  if (httpShouldCloseConnection()) {
    httpCloseConnection();
  }

  // Now that we have our formatted header, let's send it!
  // Create a new connection to the remote machine if we do
  // not already have one...
  // NB: the !m_socketProxyAuth condition is a workaround for a proxied Qt socket sometimes
  // looking disconnected after receiving the initial 407 response.
  // I guess the Qt socket fails to hide the effect of  proxy-connection: close after receiving
  // the 407 header.
  if ((!isConnected() && !m_socketProxyAuth))
  {
    if (!httpOpenConnection())
    {
       kDebug(7113) << "Couldn't connect, oopsie!";
       return false;
    }
  }

  // Clear out per-connection settings...
  resetConnectionSettings();


  // Send the data to the remote machine...
  ssize_t written = write(header.toLatin1(), header.length());
  bool sendOk = (written == (ssize_t) header.length());
  if (!sendOk)
  {
    kDebug(7113) << "Connection broken! (" << m_request.url.host() << ")"
                 << "  -- intended to write" << header.length()
                 << "bytes but wrote" << (int)written << ".";

    // The server might have closed the connection due to a timeout, or maybe
    // some transport problem arose while the connection was idle.
    if (m_request.isKeepAlive)
    {
       httpCloseConnection();
       return true; // Try again
    }

    kDebug(7113) << "sendOk == false. Connection broken !"
                 << "  -- intended to write" << header.length()
                 << "bytes but wrote" << (int)written << ".";
    error( ERR_CONNECTION_BROKEN, m_request.url.host() );
    return false;
  }
  else
    kDebug(7113) << "sent it!";

  bool res = true;
  if (hasBodyData || hasDavData)
    res = sendBody();

  infoMessage(i18n("%1 contacted. Waiting for reply...", m_request.url.host()));

  return res;
}

void HTTPProtocol::forwardHttpResponseHeader()
{
  // Send the response header if it was requested
  if ( config()->readEntry("PropagateHttpHeader", false) )
  {
    setMetaData(QLatin1String("HTTP-Headers"), m_responseHeaders.join(QString(QLatin1Char('\n'))));
    sendMetaData();
  }
}

void HTTPProtocol::parseHeaderFromCache()
{
    kDebug(7113);
    // ### we're not checking the return value, but we actually should
    cacheFileReadTextHeader2();

    Q_FOREACH (const QString &str, m_responseHeaders) {
        QString header = str.trimmed().toLower();
        if (header.startsWith(QLatin1String("content-type: "))) {
            int pos = header.indexOf(QLatin1String("charset="));
            if (pos != -1) {
                QString charset = header.mid(pos+8);
                m_request.cacheTag.charset = charset;
                setMetaData(QLatin1String("charset"), charset);
            }
        } else if (header.startsWith(QLatin1String("content-language: "))) {
            QString language = header.mid(18);
            setMetaData(QLatin1String("content-language"), language);
        } else if (header.startsWith(QLatin1String("content-disposition:"))) {
            parseContentDisposition(header.mid(20));
        }
    }

    if (m_request.cacheTag.lastModifiedDate != -1) {
        setMetaData(QLatin1String("modified"), formatHttpDate(m_request.cacheTag.lastModifiedDate));
    }

    // this header comes from the cache, so the response must have been cacheable :)
    setCacheabilityMetadata(true);
    kDebug(7113) << "Emitting mimeType" << m_mimeType;
    mimeType(m_mimeType);
    forwardHttpResponseHeader();
}

void HTTPProtocol::fixupResponseMimetype()
{
    kDebug(7113) << "before fixup" << m_mimeType;
    // Convert some common mimetypes to standard mimetypes
    if (m_mimeType == QLatin1String("application/x-targz"))
        m_mimeType = QString::fromLatin1("application/x-compressed-tar");
    else if (m_mimeType == QLatin1String("image/x-png"))
        m_mimeType = QString::fromLatin1("image/png");
    else if (m_mimeType == QLatin1String("audio/x-mp3") || m_mimeType == QLatin1String("audio/x-mpeg") || m_mimeType == QLatin1String("audio/mp3"))
        m_mimeType = QString::fromLatin1("audio/mpeg");
    else if (m_mimeType == QLatin1String("audio/microsoft-wave"))
        m_mimeType = QString::fromLatin1("audio/x-wav");

    // Crypto ones....
    else if (m_mimeType == QLatin1String("application/pkix-cert") ||
             m_mimeType == QLatin1String("application/binary-certificate")) {
        m_mimeType = QString::fromLatin1("application/x-x509-ca-cert");
    }

    // Prefer application/x-compressed-tar or x-gzpostscript over application/x-gzip.
    else if (m_mimeType == QLatin1String("application/x-gzip")) {
        if ((m_request.url.path().endsWith(QLatin1String(".tar.gz"))) ||
            (m_request.url.path().endsWith(QLatin1String(".tar"))))
            m_mimeType = QString::fromLatin1("application/x-compressed-tar");
        if ((m_request.url.path().endsWith(QLatin1String(".ps.gz"))))
            m_mimeType = QString::fromLatin1("application/x-gzpostscript");
    }

    // Some webservers say "text/plain" when they mean "application/x-bzip"
    else if ((m_mimeType == QLatin1String("text/plain")) || (m_mimeType == QLatin1String("application/octet-stream"))) {
        QString ext = m_request.url.path().right(4).toUpper();
        if (ext == QLatin1String(".BZ2"))
            m_mimeType = QString::fromLatin1("application/x-bzip");
        else if (ext == QLatin1String(".PEM"))
            m_mimeType = QString::fromLatin1("application/x-x509-ca-cert");
        else if (ext == QLatin1String(".SWF"))
            m_mimeType = QString::fromLatin1("application/x-shockwave-flash");
        else if (ext == QLatin1String(".PLS"))
            m_mimeType = QString::fromLatin1("audio/x-scpls");
        else if (ext == QLatin1String(".WMV"))
            m_mimeType = QString::fromLatin1("video/x-ms-wmv");
    }
    kDebug(7113) << "after fixup" << m_mimeType;
}


void HTTPProtocol::fixupResponseContentEncoding()
{
    // WABA: Correct for tgz files with a gzip-encoding.
    // They really shouldn't put gzip in the Content-Encoding field!
    // Web-servers really shouldn't do this: They let Content-Size refer
    // to the size of the tgz file, not to the size of the tar file,
    // while the Content-Type refers to "tar" instead of "tgz".
    if (!m_contentEncodings.isEmpty() && m_contentEncodings.last() == QLatin1String("gzip")) {
        if (m_mimeType == QLatin1String("application/x-tar")) {
            m_contentEncodings.removeLast();
            m_mimeType = QString::fromLatin1("application/x-compressed-tar");
        } else if (m_mimeType == QLatin1String("application/postscript")) {
            // LEONB: Adding another exception for psgz files.
            // Could we use the mimelnk files instead of hardcoding all this?
            m_contentEncodings.removeLast();
            m_mimeType = QString::fromLatin1("application/x-gzpostscript");
        } else if ((m_request.allowTransferCompression &&
                   m_mimeType == QLatin1String("text/html"))
                   ||
                   (m_request.allowTransferCompression &&
                   m_mimeType != QLatin1String("application/x-compressed-tar") &&
                   m_mimeType != QLatin1String("application/x-tgz") && // deprecated name
                   m_mimeType != QLatin1String("application/x-targz") && // deprecated name
                   m_mimeType != QLatin1String("application/x-gzip") &&
                   !m_request.url.path().endsWith(QLatin1String(".gz")))) {
            // Unzip!
        } else {
            m_contentEncodings.removeLast();
            m_mimeType = QString::fromLatin1("application/x-gzip");
        }
    }

    // We can't handle "bzip2" encoding (yet). So if we get something with
    // bzip2 encoding, we change the mimetype to "application/x-bzip".
    // Note for future changes: some web-servers send both "bzip2" as
    //   encoding and "application/x-bzip[2]" as mimetype. That is wrong.
    //   currently that doesn't bother us, because we remove the encoding
    //   and set the mimetype to x-bzip anyway.
    if (!m_contentEncodings.isEmpty() && m_contentEncodings.last() == QLatin1String("bzip2")) {
        m_contentEncodings.removeLast();
        m_mimeType = QString::fromLatin1("application/x-bzip");
    }
}


/**
 * This function will read in the return header from the server.  It will
 * not read in the body of the return message.  It will also not transmit
 * the header to our client as the client doesn't need to know the gory
 * details of HTTP headers.
 */
bool HTTPProtocol::readResponseHeader()
{
    resetResponseParsing();
    if (m_request.cacheTag.ioMode == ReadFromCache &&
        m_request.cacheTag.plan(m_maxCacheAge) == CacheTag::UseCached) {
        parseHeaderFromCache();
        return true;
    }

try_again:
    kDebug(7113);

    bool upgradeRequired = false;   // Server demands that we upgrade to something
                                    // This is also true if we ask to upgrade and
                                    // the server accepts, since we are now
                                    // committed to doing so
    bool canUpgrade = false;        // The server offered an upgrade //### currently not queried
    bool noHeadersFound = false;

    m_request.cacheTag.charset.clear();
    m_responseHeaders.clear();

    static const int maxHeaderSize = 128 * 1024;

    char buffer[maxHeaderSize];
    bool cont = false;
    bool bCanResume = false;

    if (!isConnected()) {
        kDebug(7113) << "No connection.";
        return false; // Reestablish connection and try again
    }

#if 0
    // NOTE: This is unnecessary since TCPSlaveBase::read does the same exact
    // thing. Plus, if we are unable to read from the socket we need to resend
    // the request as done below, not error out! Do not assume remote server
    // will honor persistent connections!!
    if (!waitForResponse(m_remoteRespTimeout)) {
        kDebug(7113) << "Got socket error:" << socket()->errorString();
        // No response error
        error(ERR_SERVER_TIMEOUT , m_request.url.host());
        return false;
    }
#endif

    int bufPos = 0;
    bool foundDelimiter = readDelimitedText(buffer, &bufPos, maxHeaderSize, 1);
    if (!foundDelimiter && bufPos < maxHeaderSize) {
        kDebug(7113) << "EOF while waiting for header start.";
        if (m_request.isKeepAlive) {
            // Try to reestablish connection.
            httpCloseConnection();
            return false; // Reestablish connection and try again.
        }

        if (m_request.method == HTTP_HEAD) {
            // HACK
            // Some web-servers fail to respond properly to a HEAD request.
            // We compensate for their failure to properly implement the HTTP standard
            // by assuming that they will be sending html.
            kDebug(7113) << "HEAD -> returned mimetype: " << DEFAULT_MIME_TYPE;
            mimeType(QString::fromLatin1(DEFAULT_MIME_TYPE));
            return true;
        }

        kDebug(7113) << "Connection broken !";
        error( ERR_CONNECTION_BROKEN, m_request.url.host() );
        return false;
    }
    if (!foundDelimiter) {
        //### buffer too small for first line of header(!)
        Q_ASSERT(0);
    }

    kDebug(7103) << "============ Received Status Response:";
    kDebug(7103) << QByteArray(buffer, bufPos).trimmed();

    HTTP_REV httpRev = HTTP_None;
    int headerSize = 0;

    int idx = 0;

    if (idx != bufPos && buffer[idx] == '<') {
        kDebug(7103) << "No valid HTTP header found! Document starts with XML/HTML tag";
        // document starts with a tag, assume HTML instead of text/plain
        m_mimeType = QLatin1String("text/html");
        m_request.responseCode = 200; // Fake it
        httpRev = HTTP_Unknown;
        m_request.isKeepAlive = false;
        noHeadersFound = true;
        // put string back
        unread(buffer, bufPos);
        goto endParsing;
    }

    // "HTTP/1.1" or similar
    if (consume(buffer, &idx, bufPos, "ICY ")) {
        httpRev = SHOUTCAST;
        m_request.isKeepAlive = false;
    } else if (consume(buffer, &idx, bufPos, "HTTP/")) {
        if (consume(buffer, &idx, bufPos, "1.0")) {
            httpRev = HTTP_10;
            m_request.isKeepAlive = false;
        } else if (consume(buffer, &idx, bufPos, "1.1")) {
            httpRev = HTTP_11;
        }
    }

    if (httpRev == HTTP_None && bufPos != 0) {
        // Remote server does not seem to speak HTTP at all
        // Put the crap back into the buffer and hope for the best
        kDebug(7113) << "DO NOT WANT." << bufPos;
        unread(buffer, bufPos);
        if (m_request.responseCode) {
            m_request.prevResponseCode = m_request.responseCode;
        }
        m_request.responseCode = 200; // Fake it
        httpRev = HTTP_Unknown;
        m_request.isKeepAlive = false;
        noHeadersFound = true;
        goto endParsing;
    }

    // response code //### maybe wrong if we need several iterations for this response...
    //### also, do multiple iterations (cf. try_again) to parse one header work w/ pipelining?
    if (m_request.responseCode) {
        m_request.prevResponseCode = m_request.responseCode;
    }
    skipSpace(buffer, &idx, bufPos);
    //TODO saner handling of invalid response code strings
    if (idx != bufPos) {
        m_request.responseCode = atoi(&buffer[idx]);
    } else {
        m_request.responseCode = 200;
    }
    // move idx to start of (yet to be fetched) next line, skipping the "OK"
    idx = bufPos;
    // (don't bother parsing the "OK", what do we do if it isn't there anyway?)

    // immediately act on most response codes...

    if (m_request.responseCode != 200 && m_request.responseCode != 304) {
        m_request.cacheTag.ioMode = NoCache;
    }

    if (m_request.responseCode >= 500 && m_request.responseCode <= 599) {
        // Server side errors

        if (m_request.method == HTTP_HEAD) {
            ; // Ignore error
        } else {
            if (!sendErrorPageNotification()) {
                error(ERR_INTERNAL_SERVER, m_request.url.url());
                return false;
            }
        }
    } else if (m_request.responseCode == 401 || m_request.responseCode == 407) {
        // Unauthorized access
    } else if (m_request.responseCode == 416) {
        // Range not supported
        m_request.offset = 0;
        return false; // Try again.
    } else if (m_request.responseCode == 426) {
        // Upgrade Required
        upgradeRequired = true;
    } else if (m_request.responseCode >= 400 && m_request.responseCode <= 499) {
        // Any other client errors
        // Tell that we will only get an error page here.
        if (!sendErrorPageNotification()) {
            if (m_request.responseCode == 403)
                error(ERR_ACCESS_DENIED, m_request.url.url());
            else
                error(ERR_DOES_NOT_EXIST, m_request.url.url());
            return false;
        }
    } else if (m_request.responseCode == 307) {
        // 307 Temporary Redirect
    } else if (m_request.responseCode == 304) {
        // 304 Not Modified
        // The value in our cache is still valid. See below for actual processing.
    } else if (m_request.responseCode >= 301 && m_request.responseCode<= 303) {
        // 301 Moved permanently
        if (m_request.responseCode == 301) {
            setMetaData(QLatin1String("permanent-redirect"), QLatin1String("true"));
        }
        // 302 Found (temporary location)
        // 303 See Other
        if (m_request.method == HTTP_POST) {
            // NOTE: This is wrong according to RFC 2616 (section 10.3.[2-4,8]).
            // However, because almost all client implementations treat a 301/302
            // response as a 303 response in violation of the spec, many servers
            // have simply adapted to this way of doing things! Thus, we are
            // forced to do the same thing. Otherwise, we won't be able to retrieve
            // these pages correctly.
            m_request.method = HTTP_GET; // Force a GET
        }
    } else if ( m_request.responseCode == 207 ) {
        // Multi-status (for WebDav)

    } else if (m_request.responseCode == 204) {
        // No content

        // error(ERR_NO_CONTENT, i18n("Data have been successfully sent."));
        // Short circuit and do nothing!

        // The original handling here was wrong, this is not an error: eg. in the
        // example of a 204 No Content response to a PUT completing.
        // m_isError = true;
        // return false;
    } else if (m_request.responseCode == 206) {
        if (m_request.offset) {
            bCanResume = true;
        }
    } else if (m_request.responseCode == 102) {
        // Processing (for WebDAV)
        /***
         * This status code is given when the server expects the
         * command to take significant time to complete. So, inform
         * the user.
         */
        infoMessage( i18n( "Server processing request, please wait..." ) );
        cont = true;
    } else if (m_request.responseCode == 100) {
        // We got 'Continue' - ignore it
        cont = true;
    }

endParsing:
    bool authRequiresAnotherRoundtrip = false;

    // Skip the whole header parsing if we got no HTTP headers at all
    if (!noHeadersFound) {

        // Auth handling
        {
            const bool wasAuthError = m_request.prevResponseCode == 401 || m_request.prevResponseCode == 407;
            const bool isAuthError = m_request.responseCode == 401 || m_request.responseCode == 407;
            const bool sameAuthError = (m_request.responseCode == m_request.prevResponseCode);
            kDebug(7113) << "wasAuthError=" << wasAuthError << "isAuthError=" << isAuthError
                         << "sameAuthError=" << sameAuthError;
            // Not the same authorization error as before and no generic error?
            // -> save the successful credentials.
            if (wasAuthError && (m_request.responseCode < 400 || (isAuthError && !sameAuthError))) {
                KIO::AuthInfo authinfo;
                bool alreadyCached = false;
                KAbstractHttpAuthentication *auth = 0;
                switch (m_request.prevResponseCode) {
                case 401:
                    auth = m_wwwAuth;
                    alreadyCached = config()->readEntry("cached-www-auth", false);
                    break;
                case 407:
                    auth = m_proxyAuth;
                    alreadyCached = config()->readEntry("cached-proxy-auth", false);
                    break;
                default:
                    Q_ASSERT(false); // should never happen!
                }

                kDebug(7113) << "authentication object:" << auth;

                // Prevent recaching of the same credentials over and over again.
                if (auth && (!auth->realm().isEmpty() || !alreadyCached)) {
                    auth->fillKioAuthInfo(&authinfo);
                    if (auth == m_wwwAuth) {
                        setMetaData(QLatin1String("{internal~currenthost}cached-www-auth"), QLatin1String("true"));
                        if (auth->realm().isEmpty() && !auth->supportsPathMatching())
                            setMetaData(QLatin1String("{internal~currenthost}www-auth-realm"), authinfo.realmValue);
                    } else {
                        setMetaData(QLatin1String("{internal~allhosts}cached-proxy-auth"), QLatin1String("true"));
                        if (auth->realm().isEmpty() && !auth->supportsPathMatching())
                            setMetaData(QLatin1String("{internal~allhosts}proxy-auth-realm"), authinfo.realmValue);
                    }
                    cacheAuthentication(authinfo);
                    kDebug(7113) << "Caching authentication for" << m_request.url;
                }
                // Update our server connection state which includes www and proxy username and password.
                m_server.updateCredentials(m_request);
            }
        }

        // done with the first line; now tokenize the other lines

        // TODO review use of STRTOLL vs. QByteArray::toInt()

        foundDelimiter = readDelimitedText(buffer, &bufPos, maxHeaderSize, 2);
        kDebug(7113) << " -- full response:" << endl << QByteArray(buffer, bufPos).trimmed();
        Q_ASSERT(foundDelimiter);

        //NOTE because tokenizer will overwrite newlines in case of line continuations in the header
        //     unread(buffer, bufSize) will not generally work anymore. we don't need it either.
        //     either we have a http response line -> try to parse the header, fail if it doesn't work
        //     or we have garbage -> fail.
        HeaderTokenizer tokenizer(buffer);
        headerSize = tokenizer.tokenize(idx, sizeof(buffer));

        // Note that not receiving "accept-ranges" means that all bets are off
        // wrt the server supporting ranges.
        TokenIterator tIt = tokenizer.iterator("accept-ranges");
        if (tIt.hasNext() && tIt.next().toLower().startsWith("none")) { // krazy:exclude=strings
            bCanResume = false;
        }

        tIt = tokenizer.iterator("keep-alive");
        while (tIt.hasNext()) {
            if (tIt.next().startsWith("timeout=")) { // krazy:exclude=strings
                m_request.keepAliveTimeout = tIt.current().mid(strlen("timeout=")).trimmed().toInt();
            }
        }

        // get the size of our data
        tIt = tokenizer.iterator("content-length");
        if (tIt.hasNext()) {
            m_iSize = STRTOLL(tIt.next().constData(), 0, 10);
        }

        tIt = tokenizer.iterator("content-location");
        if (tIt.hasNext()) {
            setMetaData(QLatin1String("content-location"), QString::fromLatin1(tIt.next().trimmed()));
        }

        // which type of data do we have?
        QString mediaValue;
        QString mediaAttribute;
        tIt = tokenizer.iterator("content-type");
        if (tIt.hasNext()) {
            QList<QByteArray> l = tIt.next().split(';');
            if (!l.isEmpty()) {
                // Assign the mime-type.
                m_mimeType = QString::fromLatin1(l.first().trimmed().toLower());
                kDebug(7113) << "Content-type: " << m_mimeType;
                l.removeFirst();
            }

            // If we still have text, then it means we have a mime-type with a
            // parameter (eg: charset=iso-8851) ; so let's get that...
            Q_FOREACH (const QByteArray &statement, l) {
                QList<QByteArray> parts = statement.split('=');
                if (parts.count() != 2) {
                    continue;
                }
                mediaAttribute = QString::fromLatin1(parts[0].trimmed().toLower());
                mediaValue = QString::fromLatin1(parts[1].trimmed());
                if (mediaValue.length() && (mediaValue[0] == QLatin1Char('"')) &&
                    (mediaValue[mediaValue.length() - 1] == QLatin1Char('"'))) {
                    mediaValue = mediaValue.mid(1, mediaValue.length() - 2);
                }
                kDebug (7113) << "Encoding-type: " << mediaAttribute
                              << "=" << mediaValue;

                if (mediaAttribute == QLatin1String("charset")) {
                    mediaValue = mediaValue.toLower();
                    m_request.cacheTag.charset = mediaValue;
                    setMetaData(QLatin1String("charset"), mediaValue);
                } else {
                    setMetaData(QLatin1String("media-") + mediaAttribute, mediaValue);
                }
            }
        }

        // content?
        tIt = tokenizer.iterator("content-encoding");
        while (tIt.hasNext()) {
            // This is so wrong !!  No wonder kio_http is stripping the
            // gzip encoding from downloaded files.  This solves multiple
            // bug reports and caitoo's problem with downloads when such a
            // header is encountered...

            // A quote from RFC 2616:
            // " When present, its (Content-Encoding) value indicates what additional
            // content have been applied to the entity body, and thus what decoding
            // mechanism must be applied to obtain the media-type referenced by the
            // Content-Type header field.  Content-Encoding is primarily used to allow
            // a document to be compressed without loosing the identity of its underlying
            // media type.  Simply put if it is specified, this is the actual mime-type
            // we should use when we pull the resource !!!
            addEncoding(QString::fromLatin1(tIt.next()), m_contentEncodings);
        }
        // Refer to RFC 2616 sec 15.5/19.5.1 and RFC 2183
        tIt = tokenizer.iterator("content-disposition");
        if (tIt.hasNext()) {
            parseContentDisposition(QString::fromLatin1(tIt.next()));
        }
        tIt = tokenizer.iterator("content-language");
        if (tIt.hasNext()) {
            QString language = QString::fromLatin1(tIt.next().trimmed());
            if (!language.isEmpty()) {
                setMetaData(QLatin1String("content-language"), language);
            }
        }

        tIt = tokenizer.iterator("proxy-connection");
        if (tIt.hasNext() && isHttpProxy(m_request.proxyUrl) && !isAutoSsl()) {
            QByteArray pc = tIt.next().toLower();
            if (pc.startsWith("close")) { // krazy:exclude=strings
                m_request.isKeepAlive = false;
            } else if (pc.startsWith("keep-alive")) { // krazy:exclude=strings
                m_request.isKeepAlive = true;
            }
        }

        tIt = tokenizer.iterator("link");
        if (tIt.hasNext()) {
            // We only support Link: <url>; rel="type"   so far
            QStringList link = QString::fromLatin1(tIt.next()).split(QLatin1Char(';'), QString::SkipEmptyParts);
            if (link.count() == 2) {
                QString rel = link[1].trimmed();
                if (rel.startsWith(QLatin1String("rel=\""))) {
                    rel = rel.mid(5, rel.length() - 6);
                    if (rel.toLower() == QLatin1String("pageservices")) {
                        //### the remove() part looks fishy!
                        QString url = link[0].remove(QRegExp(QLatin1String("[<>]"))).trimmed();
                        setMetaData(QLatin1String("PageServices"), url);
                    }
                }
            }
        }

        tIt = tokenizer.iterator("p3p");
        if (tIt.hasNext()) {
            // P3P privacy policy information
            QStringList policyrefs, compact;
            while (tIt.hasNext()) {
                QStringList policy = QString::fromLatin1(tIt.next().simplified())
                                     .split(QLatin1Char('='), QString::SkipEmptyParts);
                if (policy.count() == 2) {
                    if (policy[0].toLower() == QLatin1String("policyref")) {
                        policyrefs << policy[1].remove(QRegExp(QLatin1String("[\")\']"))).trimmed();
                    } else if (policy[0].toLower() == QLatin1String("cp")) {
                        // We convert to cp\ncp\ncp\n[...]\ncp to be consistent with
                        // other metadata sent in strings.  This could be a bit more
                        // efficient but I'm going for correctness right now.
                        const QString s = policy[1].remove(QRegExp(QLatin1String("[\")\']")));
                        const QStringList cps = s.split(QLatin1Char(' '), QString::SkipEmptyParts);
                        compact << cps;
                    }
                }
            }
            if (!policyrefs.isEmpty()) {
                setMetaData(QLatin1String("PrivacyPolicy"), policyrefs.join(QLatin1String("\n")));
            }
            if (!compact.isEmpty()) {
                setMetaData(QLatin1String("PrivacyCompactPolicy"), compact.join(QLatin1String("\n")));
            }
        }

        // continue only if we know that we're at least HTTP/1.0
        if (httpRev == HTTP_11 || httpRev == HTTP_10) {
            // let them tell us if we should stay alive or not
            tIt = tokenizer.iterator("connection");
            while (tIt.hasNext()) {
                QByteArray connection = tIt.next().toLower();
                if (!(isHttpProxy(m_request.proxyUrl) && !isAutoSsl())) {
                    if (connection.startsWith("close")) { // krazy:exclude=strings
                        m_request.isKeepAlive = false;
                    } else if (connection.startsWith("keep-alive")) { // krazy:exclude=strings
                        m_request.isKeepAlive = true;
                    }
                }
                if (connection.startsWith("upgrade")) { // krazy:exclude=strings
                    if (m_request.responseCode == 101) {
                        // Ok, an upgrade was accepted, now we must do it
                        upgradeRequired = true;
                    } else if (upgradeRequired) {  // 426
                        // Nothing to do since we did it above already
                    } else {
                        // Just an offer to upgrade - no need to take it
                        canUpgrade = true;
                    }
                }
            }
            // what kind of encoding do we have?  transfer?
            tIt = tokenizer.iterator("transfer-encoding");
            while (tIt.hasNext()) {
                // If multiple encodings have been applied to an entity, the
                // transfer-codings MUST be listed in the order in which they
                // were applied.
                addEncoding(QString::fromLatin1(tIt.next().trimmed()), m_transferEncodings);
            }

            // md5 signature
            tIt = tokenizer.iterator("content-md5");
            if (tIt.hasNext()) {
                m_contentMD5 = QString::fromLatin1(tIt.next().trimmed());
            }

            // *** Responses to the HTTP OPTIONS method follow
            // WebDAV capabilities
            tIt = tokenizer.iterator("dav");
            while (tIt.hasNext()) {
                m_davCapabilities << QString::fromLatin1(tIt.next());
            }
            // *** Responses to the HTTP OPTIONS method finished
        }


        // Now process the HTTP/1.1 upgrade
        QStringList upgradeOffers;
        tIt = tokenizer.iterator("upgrade");
        if (tIt.hasNext()) {
            // Now we have to check to see what is offered for the upgrade
            QString offered = QString::fromLatin1(tIt.next());
            upgradeOffers = offered.split(QRegExp(QLatin1String("[ \n,\r\t]")), QString::SkipEmptyParts);
        }
        Q_FOREACH (const QString &opt, upgradeOffers) {
            if (opt == QLatin1String("TLS/1.0")) {
                if (!startSsl() && upgradeRequired) {
                    error(ERR_UPGRADE_REQUIRED, opt);
                    return false;
                }
            } else if (opt == QLatin1String("HTTP/1.1")) {
                httpRev = HTTP_11;
            } else if (upgradeRequired) {
                // we are told to do an upgrade we don't understand
                error(ERR_UPGRADE_REQUIRED, opt);
                return false;
            }
        }

        // Harvest cookies (mmm, cookie fields!)
        QByteArray cookieStr; // In case we get a cookie.
        tIt = tokenizer.iterator("set-cookie");
        while (tIt.hasNext()) {
            cookieStr += "Set-Cookie: ";
            cookieStr += tIt.next();
            cookieStr += '\n';
        }
        if (!cookieStr.isEmpty()) {
            if ((m_request.cookieMode == HTTPRequest::CookiesAuto) && m_request.useCookieJar) {
                // Give cookies to the cookiejar.
                const QString domain = config()->readEntry("cross-domain");
                if (!domain.isEmpty() && isCrossDomainRequest(m_request.url.host(), domain)) {
                    cookieStr = "Cross-Domain\n" + cookieStr;
                }
                addCookies( m_request.url.url(), cookieStr );
            } else if (m_request.cookieMode == HTTPRequest::CookiesManual) {
                // Pass cookie to application
                setMetaData(QLatin1String("setcookies"), QString::fromUtf8(cookieStr)); // ## is encoding ok?
            }
        }

        // We need to reread the header if we got a '100 Continue' or '102 Processing'
        // This may be a non keepalive connection so we handle this kind of loop internally
        if ( cont )
        {
            kDebug(7113) << "cont; returning to mark try_again";
            goto try_again;
        }

        if (!m_isChunked && (m_iSize == NO_SIZE) && m_request.isKeepAlive &&
            canHaveResponseBody(m_request.responseCode, m_request.method)) {
            kDebug(7113) << "Ignoring keep-alive: otherwise unable to determine response body length.";
            m_request.isKeepAlive = false;
        }

        // TODO cache the proxy auth data (not doing this means a small performance regression for now)

        // we may need to send (Proxy or WWW) authorization data
        authRequiresAnotherRoundtrip = false;
        if (!m_request.doNotAuthenticate &&
            (m_request.responseCode == 401 || m_request.responseCode == 407)) {
            KIO::AuthInfo authinfo;
            KAbstractHttpAuthentication **auth;

            if (m_request.responseCode == 401) {
                auth = &m_wwwAuth;
                tIt = tokenizer.iterator("www-authenticate");
                authinfo.url = m_request.url;
                authinfo.username = m_server.url.user();
                authinfo.prompt = i18n("You need to supply a username and a "
                                       "password to access this site.");
                authinfo.commentLabel = i18n("Site:");
            } else {
                // make sure that the 407 header hasn't escaped a lower layer when it shouldn't.
                // this may break proxy chains which were never tested anyway, and AFAIK they are
                // rare to nonexistent in the wild.
                Q_ASSERT(QNetworkProxy::applicationProxy().type() == QNetworkProxy::NoProxy);
                auth = &m_proxyAuth;
                tIt = tokenizer.iterator("proxy-authenticate");
                authinfo.url = m_request.proxyUrl;
                authinfo.username = m_request.proxyUrl.user();
                authinfo.prompt = i18n("You need to supply a username and a password for "
                                       "the proxy server listed below before you are allowed "
                                       "to access any sites." );
                authinfo.commentLabel = i18n("Proxy:");
            }

            QList<QByteArray> authTokens = tIt.all();
            // Workaround brain dead server responses that violate the spec and
            // incorrectly return a 401/407 without the required WWW/Proxy-Authenticate
            // header fields. See bug 215736...
            if (!authTokens.isEmpty()) {
                authRequiresAnotherRoundtrip = true;
                kDebug(7113) << "parsing authentication request; response code =" << m_request.responseCode;

            try_next_auth_scheme:
                QByteArray bestOffer = KAbstractHttpAuthentication::bestOffer(authTokens);
                if (*auth) {
                    if (!bestOffer.toLower().startsWith((*auth)->scheme().toLower())) {
                        // huh, the strongest authentication scheme offered has changed.
                        kDebug(7113) << "deleting old auth class...";
                        delete *auth;
                        *auth = 0;
                    }
                }

                if (!(*auth)) {
                    *auth = KAbstractHttpAuthentication::newAuth(bestOffer, config());
                }

                kDebug(7113) << "pointer to auth class is now" << *auth;

                if (*auth) {
                    kDebug(7113) << "Trying authentication scheme:" << (*auth)->scheme();

                    // remove trailing space from the method string, or digest auth will fail
                    (*auth)->setChallenge(bestOffer, authinfo.url, methodString(m_request.method));

                    QString username;
                    QString password;
                    bool generateAuthorization = true;
                    if ((*auth)->needCredentials()) {
                        // use credentials supplied by the application if available
                        if (!m_request.url.user().isEmpty() && !m_request.url.pass().isEmpty()) {
                            username = m_request.url.user();
                            password = m_request.url.pass();
                            // don't try this password any more
                            m_request.url.setPass(QString());
                        } else {
                            // try to get credentials from kpasswdserver's cache, then try asking the user.
                            authinfo.verifyPath = false; // we have realm, no path based checking please!
                            authinfo.realmValue = (*auth)->realm();
                            if (authinfo.realmValue.isEmpty() && !(*auth)->supportsPathMatching())
                                authinfo.realmValue = QLatin1String((*auth)->scheme());

                            // Save the current authinfo url because it can be modified by the call to
                            // checkCachedAuthentication. That way we can restore it if the call
                            // modified it.
                            const KUrl reqUrl = authinfo.url;
                            if (!checkCachedAuthentication(authinfo) ||
                                ((*auth)->wasFinalStage() && m_request.responseCode == m_request.prevResponseCode)) {
                                QString errorMsg;
                                if ((*auth)->wasFinalStage()) {
                                    switch (m_request.prevResponseCode) {
                                    case 401:
                                        errorMsg = i18n("Authentication Failed.");
                                        break;
                                    case 407:
                                        errorMsg = i18n("Proxy Authentication Failed.");
                                        break;
                                    default:
                                        break;
                                    }
                                }

                                // Reset url to the saved url...
                                authinfo.url = reqUrl;
                                authinfo.keepPassword = true;
                                authinfo.comment = i18n("<b>%1</b> at <b>%2</b>",
                                                        authinfo.realmValue, authinfo.url.host());

                                if (!openPasswordDialog(authinfo, errorMsg)) {
                                    if (sendErrorPageNotification()) {
                                        generateAuthorization = false;
                                        authRequiresAnotherRoundtrip = false;
                                    } else {
                                        error(ERR_ACCESS_DENIED, reqUrl.host());
                                        return false;
                                    }
                                }
                            }
                            username = authinfo.username;
                            password = authinfo.password;
                        }
                    }

                    if (generateAuthorization) {
                        (*auth)->generateResponse(username, password);

                        kDebug(7113) << "Auth State: isError=" << (*auth)->isError()
                                     << "needCredentials=" << (*auth)->needCredentials()
                                     << "forceKeepAlive=" << (*auth)->forceKeepAlive()
                                     << "forceDisconnect=" << (*auth)->forceDisconnect()
                                     << "headerFragment=" << (*auth)->headerFragment();

                        if ((*auth)->isError()) {
                            authTokens.removeOne(bestOffer);
                            if (!authTokens.isEmpty())
                                goto try_next_auth_scheme;
                            else {
                                error(ERR_UNSUPPORTED_ACTION, i18n("Authorization failed."));
                                return false;
                            }
                            //### return false; ?
                        } else if ((*auth)->forceKeepAlive()) {
                            //### think this through for proxied / not proxied
                            m_request.isKeepAlive = true;
                        } else if ((*auth)->forceDisconnect()) {
                            //### think this through for proxied / not proxied
                            m_request.isKeepAlive = false;
                            httpCloseConnection();
                        }
                    }
                } else {
                    if (sendErrorPageNotification())
                        authRequiresAnotherRoundtrip = false;
                    else {
                        error(ERR_UNSUPPORTED_ACTION, i18n("Unknown Authorization method."));
                        return false;
                    }
                }
            }
        }

        QString locationStr;
        // In fact we should do redirection only if we have a redirection response code (300 range)
        tIt = tokenizer.iterator("location");
        if (tIt.hasNext() && m_request.responseCode > 299 && m_request.responseCode < 400) {
            locationStr = QString::fromUtf8(tIt.next().trimmed());
        }
        // We need to do a redirect
        if (!locationStr.isEmpty())
        {
            KUrl u(m_request.url, locationStr);
            if(!u.isValid())
            {
                error(ERR_MALFORMED_URL, u.url());
                return false;
            }
            if ((u.protocol() != QLatin1String("http")) && (u.protocol() != QLatin1String("https")) &&
                (u.protocol() != QLatin1String("webdav")) && (u.protocol() != QLatin1String("webdavs")))
            {
                redirection(u);
                error(ERR_ACCESS_DENIED, u.url());
                return false;
            }

            // preserve #ref: (bug 124654)
            // if we were at http://host/resource1#ref, we sent a GET for "/resource1"
            // if we got redirected to http://host/resource2, then we have to re-add
            // the fragment:
            if (m_request.url.hasRef() && !u.hasRef() &&
                (m_request.url.host() == u.host()) &&
                (m_request.url.protocol() == u.protocol()))
                u.setRef(m_request.url.ref());

            m_isRedirection = true;

            if (!m_request.id.isEmpty())
            {
                sendMetaData();
            }

            // If we're redirected to a http:// url, remember that we're doing webdav...
            if (m_protocol == "webdav" || m_protocol == "webdavs"){
                if(u.protocol() == QLatin1String("http")){
                    u.setProtocol(QString::fromLatin1("webdav"));
                }else if(u.protocol() == QLatin1String("https")){
                    u.setProtocol(QString::fromLatin1("webdavs"));
                }

                m_request.redirectUrl = u;
            }

            kDebug(7113) << "Re-directing from" << m_request.url.url()
                         << "to" << u.url();

            redirection(u);

            // It would be hard to cache the redirection response correctly. The possible benefit
            // is small (if at all, assuming fast disk and slow network), so don't do it.
            cacheFileClose();
            setCacheabilityMetadata(false);
        }

        // Inform the job that we can indeed resume...
        if (bCanResume && m_request.offset) {
            //TODO turn off caching???
            canResume();
        } else {
            m_request.offset = 0;
        }

        // Correct a few common wrong content encodings
        fixupResponseContentEncoding();

        // Correct some common incorrect pseudo-mimetypes
        fixupResponseMimetype();

        // parse everything related to expire and other dates, and cache directives; also switch
        // between cache reading and writing depending on cache validation result.
        cacheParseResponseHeader(tokenizer);

    }

    if (m_request.cacheTag.ioMode == ReadFromCache) {
        if (m_request.cacheTag.policy == CC_Verify &&
            m_request.cacheTag.plan(m_maxCacheAge) != CacheTag::UseCached) {
            kDebug(7113) << "Reading resource from cache even though the cache plan is not "
                            "UseCached; the server is probably sending wrong expiry information.";
        }
        parseHeaderFromCache();
        return true;
    }

    // Let the app know about the mime-type iff this is not
    // a redirection and the mime-type string is not empty.
    if (!m_isRedirection &&
        (!m_mimeType.isEmpty() || m_request.method == HTTP_HEAD) &&
        (m_isLoadingErrorPage || !authRequiresAnotherRoundtrip)) {
        kDebug(7113) << "Emitting mimetype " << m_mimeType;
        mimeType( m_mimeType );
    }

    if (config()->readEntry("PropagateHttpHeader", false) ||
        m_request.cacheTag.ioMode == WriteToCache) {
        // store header lines if they will be used; note that the tokenizer removing
        // line continuation special cases is probably more good than bad.
        int nextLinePos = 0;
        int prevLinePos = 0;
        bool haveMore = true;
        while (haveMore) {
            haveMore = nextLine(buffer, &nextLinePos, bufPos);
            int prevLineEnd = nextLinePos;
            while (buffer[prevLineEnd - 1] == '\r' || buffer[prevLineEnd - 1] == '\n') {
                prevLineEnd--;
            }

            m_responseHeaders.append(QString::fromLatin1(&buffer[prevLinePos],
                                                         prevLineEnd - prevLinePos));
            prevLinePos = nextLinePos;
        }
    }

    // Do not move send response header before any redirection as it seems
    // to screw up some sites. See BR# 150904.
    forwardHttpResponseHeader();

    if (m_request.method == HTTP_HEAD) {
        return true;
    }

    return !authRequiresAnotherRoundtrip; // return true if no more credentials need to be sent
}

void HTTPProtocol::parseContentDisposition(const QString &disposition)
{
    const QMap<QString, QString> parameters = contentDispositionParser(disposition);

    QMap<QString, QString>::const_iterator i = parameters.constBegin();
    while (i != parameters.constEnd()) {
        setMetaData(QLatin1String("content-disposition-") + i.key(), i.value());
        kDebug(7113) << "Content-Disposition: " << i.key() << "=" << i.value();
        ++i;
    }
}

void HTTPProtocol::addEncoding(const QString &_encoding, QStringList &encs)
{
  QString encoding = _encoding.trimmed().toLower();
  // Identity is the same as no encoding
  if (encoding == QLatin1String("identity")) {
    return;
  } else if (encoding == QLatin1String("8bit")) {
    // Strange encoding returned by http://linac.ikp.physik.tu-darmstadt.de
    return;
  } else if (encoding == QLatin1String("chunked")) {
    m_isChunked = true;
    // Anyone know of a better way to handle unknown sizes possibly/ideally with unsigned ints?
    //if ( m_cmd != CMD_COPY )
      m_iSize = NO_SIZE;
  } else if ((encoding == QLatin1String("x-gzip")) || (encoding == QLatin1String("gzip"))) {
    encs.append(QString::fromLatin1("gzip"));
  } else if ((encoding == QLatin1String("x-bzip2")) || (encoding == QLatin1String("bzip2"))) {
    encs.append(QString::fromLatin1("bzip2")); // Not yet supported!
  } else if ((encoding == QLatin1String("x-deflate")) || (encoding == QLatin1String("deflate"))) {
    encs.append(QString::fromLatin1("deflate"));
  } else {
    kDebug(7113) << "Unknown encoding encountered.  "
                 << "Please write code. Encoding =" << encoding;
  }
}

void HTTPProtocol::cacheParseResponseHeader(const HeaderTokenizer &tokenizer)
{
    if (!m_request.cacheTag.useCache)
        return;

    // might have to add more response codes
    if (m_request.responseCode != 200 && m_request.responseCode != 304) {
        return;
    }

    // -1 is also the value returned by KDateTime::toTime_t() from an invalid instance.
    m_request.cacheTag.servedDate = -1;
    m_request.cacheTag.lastModifiedDate = -1;
    m_request.cacheTag.expireDate = -1;

    const qint64 currentDate = time(0);

    bool mayCache = m_request.cacheTag.ioMode != NoCache;

    TokenIterator tIt = tokenizer.iterator("last-modified");
    if (tIt.hasNext()) {
        m_request.cacheTag.lastModifiedDate =
              KDateTime::fromString(QString::fromLatin1(tIt.next()), KDateTime::RFCDate).toTime_t();

        //### might be good to canonicalize the date by using KDateTime::toString()
        if (m_request.cacheTag.lastModifiedDate != -1) {
            setMetaData(QLatin1String("modified"), QString::fromLatin1(tIt.current()));
        }
    }

    // determine from available information when the response was served by the origin server
    {
        qint64 dateHeader = -1;
        tIt = tokenizer.iterator("date");
        if (tIt.hasNext()) {
            dateHeader = KDateTime::fromString(QString::fromLatin1(tIt.next()), KDateTime::RFCDate).toTime_t();
            // -1 on error
        }

        qint64 ageHeader = 0;
        tIt = tokenizer.iterator("age");
        if (tIt.hasNext()) {
            ageHeader = tIt.next().toLongLong();
            // 0 on error
        }

        if (dateHeader != -1) {
            m_request.cacheTag.servedDate = dateHeader;
        } else if (ageHeader) {
            m_request.cacheTag.servedDate = currentDate - ageHeader;
        } else {
            m_request.cacheTag.servedDate = currentDate;
        }
    }

    bool hasCacheDirective = false;
    // determine when the response "expires", i.e. becomes stale and needs revalidation
    {
        // (we also parse other cache directives here)
        qint64 maxAgeHeader = 0;
        tIt = tokenizer.iterator("cache-control");
        while (tIt.hasNext()) {
            QByteArray cacheStr = tIt.next().toLower();
            if (cacheStr.startsWith("no-cache") || cacheStr.startsWith("no-store")) { // krazy:exclude=strings
                // Don't put in cache
                mayCache = false;
                hasCacheDirective = true;
            } else if (cacheStr.startsWith("max-age=")) { // krazy:exclude=strings
                QByteArray ba = cacheStr.mid(strlen("max-age=")).trimmed();
                bool ok = false;
                maxAgeHeader = ba.toLongLong(&ok);
                if (ok) {
                    hasCacheDirective = true;
                }
            }
        }

        qint64 expiresHeader = -1;
        tIt = tokenizer.iterator("expires");
        if (tIt.hasNext()) {
            expiresHeader = KDateTime::fromString(QString::fromLatin1(tIt.next()), KDateTime::RFCDate).toTime_t();
            kDebug(7113) << "parsed expire date from 'expires' header:" << tIt.current();
        }

        if (maxAgeHeader) {
            m_request.cacheTag.expireDate = m_request.cacheTag.servedDate + maxAgeHeader;
        } else if (expiresHeader != -1) {
            m_request.cacheTag.expireDate = expiresHeader;
        } else {
            // heuristic expiration date
            if (m_request.cacheTag.lastModifiedDate != -1) {
                // expAge is following the RFC 2616 suggestion for heuristic expiration
                qint64 expAge = (m_request.cacheTag.servedDate -
                                 m_request.cacheTag.lastModifiedDate) / 10;
                // not in the RFC: make sure not to have a huge heuristic cache lifetime
                expAge = qMin(expAge, qint64(3600 * 24));
                m_request.cacheTag.expireDate = m_request.cacheTag.servedDate + expAge;
            } else {
                m_request.cacheTag.expireDate = m_request.cacheTag.servedDate +
                                                DEFAULT_CACHE_EXPIRE;
            }
        }
        // make sure that no future clock monkey business causes the cache entry to un-expire
        if (m_request.cacheTag.expireDate < currentDate) {
            m_request.cacheTag.expireDate = 0;  // January 1, 1970 :)
        }
    }

    tIt = tokenizer.iterator("etag");
    if (tIt.hasNext()) {
        QString prevEtag = m_request.cacheTag.etag;
        m_request.cacheTag.etag = QString::fromLatin1(tIt.next());
        if (m_request.cacheTag.etag != prevEtag && m_request.responseCode == 304) {
            kDebug(7103) << "304 Not Modified but new entity tag - I don't think this is legal HTTP.";
        }
    }

    // whoops.. we received a warning
    tIt = tokenizer.iterator("warning");
    if (tIt.hasNext()) {
        //Don't use warning() here, no need to bother the user.
        //Those warnings are mostly about caches.
        infoMessage(QString::fromLatin1(tIt.next()));
    }

    // Cache management (HTTP 1.0)
    tIt = tokenizer.iterator("pragma");
    while (tIt.hasNext()) {
        if (tIt.next().toLower().startsWith("no-cache")) { // krazy:exclude=strings
            mayCache = false;
            hasCacheDirective = true;
        }
    }

    // The deprecated Refresh Response
    tIt = tokenizer.iterator("refresh");
    if (tIt.hasNext()) {
        mayCache = false;
        setMetaData(QLatin1String("http-refresh"), QString::fromLatin1(tIt.next().trimmed()));
    }

    // We don't cache certain text objects
    if (m_mimeType.startsWith(QLatin1String("text/")) && (m_mimeType != QLatin1String("text/css")) &&
        (m_mimeType != QLatin1String("text/x-javascript")) && !hasCacheDirective) {
        // Do not cache secure pages or pages
        // originating from password protected sites
        // unless the webserver explicitly allows it.
        if (isUsingSsl() || m_wwwAuth) {
            mayCache = false;
        }
    }

    // note that we've updated cacheTag, so the plan() is with current data
    if (m_request.cacheTag.plan(m_maxCacheAge) == CacheTag::ValidateCached) {
        kDebug(7113) << "Cache needs validation";
        if (m_request.responseCode == 304) {
            kDebug(7113) << "...was revalidated by response code but not by updated expire times. "
                            "We're going to set the expire date to 60 seconds in the future...";
            m_request.cacheTag.expireDate = currentDate + 60;
            if (m_request.cacheTag.policy == CC_Verify &&
                m_request.cacheTag.plan(m_maxCacheAge) != CacheTag::UseCached) {
                // "apparently" because we /could/ have made an error ourselves, but the errors I
                // witnessed were all the server's fault.
                kDebug(7113) << "this proxy or server apparently sends bogus expiry information.";
            }
        }
    }

    // validation handling
    if (mayCache && m_request.responseCode == 200 && !m_mimeType.isEmpty()) {
        kDebug(7113) << "Cache, adding" << m_request.url.url();
        // ioMode can still be ReadFromCache here if we're performing a conditional get
        // aka validation
        m_request.cacheTag.ioMode = WriteToCache;
        if (!cacheFileOpenWrite()) {
            kDebug(7113) << "Error creating cache entry for " << m_request.url.url()<<"!\n";
        }
        m_maxCacheSize = config()->readEntry("MaxCacheSize", DEFAULT_MAX_CACHE_SIZE) / 2;
    } else if (mayCache && m_request.responseCode == 304 && m_request.cacheTag.file) {
        // the cache file should still be open for reading, see satisfyRequestFromCache().
        Q_ASSERT(m_request.cacheTag.file->openMode() == QIODevice::ReadOnly);
        Q_ASSERT(m_request.cacheTag.ioMode == ReadFromCache);
    } else {
        cacheFileClose();
    }

    setCacheabilityMetadata(mayCache);
}

void HTTPProtocol::setCacheabilityMetadata(bool cachingAllowed)
{
    if (!cachingAllowed) {
        setMetaData(QLatin1String("no-cache"), QLatin1String("true"));
        setMetaData(QLatin1String("expire-date"), QLatin1String("1")); // Expired
    } else {
        QString tmp;
        tmp.setNum(m_request.cacheTag.expireDate);
        setMetaData(QLatin1String("expire-date"), tmp);
        // slightly changed semantics from old creationDate, probably more correct now
        tmp.setNum(m_request.cacheTag.servedDate);
        setMetaData(QLatin1String("cache-creation-date"), tmp);
    }
}

bool HTTPProtocol::sendBody()
{
  infoMessage( i18n( "Requesting data to send" ) );

  int readFromApp = -1;

  // m_POSTbuf will NOT be empty iff authentication was required before posting
  // the data OR a re-connect is requested from ::readResponseHeader because the
  // connection was lost for some reason.
  if (m_POSTbuf.isEmpty())
  {
    kDebug(7113) << "POST'ing live data...";

    QByteArray buffer;

    do {
      m_POSTbuf.append(buffer);
      buffer.clear();
      dataReq(); // Request for data
      readFromApp = readData(buffer);
    } while (readFromApp > 0);
  }
  else
  {
    kDebug(7113) << "POST'ing saved data...";
    readFromApp = 0;
  }

  if (readFromApp < 0)
  {
    error(ERR_ABORTED, m_request.url.host());
    return false;
  }

  infoMessage(i18n("Sending data to %1" ,  m_request.url.host()));

  const QString cLength = QString::fromLatin1("Content-Length: %1\r\n\r\n").arg(m_POSTbuf.size());
  kDebug( 7113 ) << cLength.trimmed();

  // Send the content length...
  bool sendOk = (write(cLength.toLatin1(), cLength.length()) == (ssize_t) cLength.length());
  if (!sendOk)
  {
    kDebug( 7113 ) << "Connection broken when sending "
                    << "content length: (" << m_request.url.host() << ")";
    error( ERR_CONNECTION_BROKEN, m_request.url.host() );
    return false;
  }

  // Send the data...
  // kDebug( 7113 ) << "POST DATA: " << QCString(m_POSTbuf);
  sendOk = (write(m_POSTbuf.data(), m_POSTbuf.size()) == (ssize_t) m_POSTbuf.size());
  if (!sendOk)
  {
    kDebug(7113) << "Connection broken when sending message body: ("
                  << m_request.url.host() << ")";
    error( ERR_CONNECTION_BROKEN, m_request.url.host() );
    return false;
  }

  return true;
}

void HTTPProtocol::httpClose( bool keepAlive )
{
  kDebug(7113) << "keepAlive =" << keepAlive;

  cacheFileClose();

  // Only allow persistent connections for GET requests.
  // NOTE: we might even want to narrow this down to non-form
  // based submit requests which will require a meta-data from
  // khtml.
  if (keepAlive) {
    if (!m_request.keepAliveTimeout)
       m_request.keepAliveTimeout = DEFAULT_KEEP_ALIVE_TIMEOUT;
    else if (m_request.keepAliveTimeout > 2*DEFAULT_KEEP_ALIVE_TIMEOUT)
       m_request.keepAliveTimeout = 2*DEFAULT_KEEP_ALIVE_TIMEOUT;

    kDebug(7113) << "keep alive (" << m_request.keepAliveTimeout << ")";
    QByteArray data;
    QDataStream stream( &data, QIODevice::WriteOnly );
    stream << int(99); // special: Close connection
    setTimeoutSpecialCommand(m_request.keepAliveTimeout, data);

    return;
  }

  httpCloseConnection();
}

void HTTPProtocol::closeConnection()
{
  kDebug(7113);
  httpCloseConnection();
}

void HTTPProtocol::httpCloseConnection()
{
  kDebug(7113);
  m_request.isKeepAlive = false;
  m_server.clear();
  disconnectFromHost();
  clearUnreadBuffer();
  setTimeoutSpecialCommand(-1); // Cancel any connection timeout
}

void HTTPProtocol::slave_status()
{
  kDebug(7113);

  if ( !isConnected() )
     httpCloseConnection();

  slaveStatus( m_server.url.host(), isConnected() );
}

void HTTPProtocol::mimetype( const KUrl& url )
{
  kDebug(7113) << url.url();

  if (!maybeSetRequestUrl(url))
    return;
  resetSessionSettings();

  m_request.method = HTTP_HEAD;
  m_request.cacheTag.policy= CC_Cache;

  proceedUntilResponseHeader();
  httpClose(m_request.isKeepAlive);
  finished();

  kDebug(7113) << "http: mimetype = " << m_mimeType;
}

void HTTPProtocol::special( const QByteArray &data )
{
  kDebug(7113);

  int tmp;
  QDataStream stream(data);

  stream >> tmp;
  switch (tmp) {
    case 1: // HTTP POST
    {
      KUrl url;
      stream >> url;
      post( url );
      break;
    }
    case 2: // cache_update
    {
        KUrl url;
        bool no_cache;
        qint64 expireDate;
        stream >> url >> no_cache >> expireDate;
        if (no_cache) {
            QString filename = cacheFilePathFromUrl(url);
            // there is a tiny risk of deleting the wrong file due to hash collisions here.
            // this is an unimportant performance issue.
            // FIXME on Windows we may be unable to delete the file if open
            QFile::remove(filename);
            finished();
            break;
        }
        // let's be paranoid and inefficient here...
        HTTPRequest savedRequest = m_request;

        m_request.url = url;
        if (cacheFileOpenRead()) {
            m_request.cacheTag.expireDate = expireDate;
            cacheFileClose(); // this sends an update command to the cache cleaner process
        }

        m_request = savedRequest;
        finished();
        break;
    }
    case 5: // WebDAV lock
    {
      KUrl url;
      QString scope, type, owner;
      stream >> url >> scope >> type >> owner;
      davLock( url, scope, type, owner );
      break;
    }
    case 6: // WebDAV unlock
    {
      KUrl url;
      stream >> url;
      davUnlock( url );
      break;
    }
    case 7: // Generic WebDAV
    {
      KUrl url;
      int method;
      stream >> url >> method;
      davGeneric( url, (KIO::HTTP_METHOD) method );
      break;
    }
    case 99: // Close Connection
    {
      httpCloseConnection();
      break;
    }
    default:
      // Some command we don't understand.
      // Just ignore it, it may come from some future version of KDE.
      break;
  }
}

/**
 * Read a chunk from the data stream.
 */
int HTTPProtocol::readChunked()
{
  if ((m_iBytesLeft == 0) || (m_iBytesLeft == NO_SIZE))
  {
     // discard CRLF from previous chunk, if any, and read size of next chunk

     int bufPos = 0;
     m_receiveBuf.resize(4096);

     bool foundCrLf = readDelimitedText(m_receiveBuf.data(), &bufPos, m_receiveBuf.size(), 1);

     if (foundCrLf && bufPos == 2) {
         // The previous read gave us the CRLF from the previous chunk. As bufPos includes
         // the trailing CRLF it has to be > 2 to possibly include the next chunksize.
         bufPos = 0;
         foundCrLf = readDelimitedText(m_receiveBuf.data(), &bufPos, m_receiveBuf.size(), 1);
     }
     if (!foundCrLf) {
         kDebug(7113) << "Failed to read chunk header.";
         return -1;
     }
     Q_ASSERT(bufPos > 2);

     long long nextChunkSize = STRTOLL(m_receiveBuf.data(), 0, 16);
     if (nextChunkSize < 0)
     {
        kDebug(7113) << "Negative chunk size";
        return -1;
     }
     m_iBytesLeft = nextChunkSize;

     kDebug(7113) << "Chunk size = " << m_iBytesLeft << " bytes";

     if (m_iBytesLeft == 0)
     {
       // Last chunk; read and discard chunk trailer.
       // The last trailer line ends with CRLF and is followed by another CRLF
       // so we have CRLFCRLF like at the end of a standard HTTP header.
       // Do not miss a CRLFCRLF spread over two of our 4K blocks: keep three previous bytes.
       //NOTE the CRLF after the chunksize also counts if there is no trailer. Copy it over.
       char trash[4096];
       trash[0] = m_receiveBuf.constData()[bufPos - 2];
       trash[1] = m_receiveBuf.constData()[bufPos - 1];
       int trashBufPos = 2;
       bool done = false;
       while (!done && !m_isEOF) {
           if (trashBufPos > 3) {
               // shift everything but the last three bytes out of the buffer
               for (int i = 0; i < 3; i++) {
                   trash[i] = trash[trashBufPos - 3 + i];
               }
               trashBufPos = 3;
           }
           done = readDelimitedText(trash, &trashBufPos, 4096, 2);
       }
       if (m_isEOF && !done) {
           kDebug(7113) << "Failed to read chunk trailer.";
           return -1;
       }

       return 0;
     }
  }

  int bytesReceived = readLimited();
  if (!m_iBytesLeft) {
     m_iBytesLeft = NO_SIZE; // Don't stop, continue with next chunk
  }
  return bytesReceived;
}

int HTTPProtocol::readLimited()
{
  if (!m_iBytesLeft)
    return 0;

  m_receiveBuf.resize(4096);

  int bytesToReceive;
  if (m_iBytesLeft > KIO::filesize_t(m_receiveBuf.size()))
     bytesToReceive = m_receiveBuf.size();
  else
     bytesToReceive = m_iBytesLeft;

  const int bytesReceived = readBuffered(m_receiveBuf.data(), bytesToReceive, false);

  if (bytesReceived <= 0)
     return -1; // Error: connection lost

  m_iBytesLeft -= bytesReceived;
  return bytesReceived;
}

int HTTPProtocol::readUnlimited()
{
  if (m_request.isKeepAlive)
  {
     kDebug(7113) << "Unbounded datastream on a Keep-alive connection!";
     m_request.isKeepAlive = false;
  }

  m_receiveBuf.resize(4096);

  int result = readBuffered(m_receiveBuf.data(), m_receiveBuf.size());
  if (result > 0)
     return result;

  m_isEOF = true;
  m_iBytesLeft = 0;
  return 0;
}

void HTTPProtocol::slotData(const QByteArray &_d)
{
   if (!_d.size())
   {
      m_isEOD = true;
      return;
   }

   if (m_iContentLeft != NO_SIZE)
   {
      if (m_iContentLeft >= KIO::filesize_t(_d.size()))
         m_iContentLeft -= _d.size();
      else
         m_iContentLeft = NO_SIZE;
   }

   QByteArray d = _d;
   if ( !m_dataInternal )
   {
      // If a broken server does not send the mime-type,
      // we try to id it from the content before dealing
      // with the content itself.
      if ( m_mimeType.isEmpty() && !m_isRedirection &&
           !( m_request.responseCode >= 300 && m_request.responseCode <=399) )
      {
        kDebug(7113) << "Determining mime-type from content...";
        int old_size = m_mimeTypeBuffer.size();
        m_mimeTypeBuffer.resize( old_size + d.size() );
        memcpy( m_mimeTypeBuffer.data() + old_size, d.data(), d.size() );
        if ( (m_iBytesLeft != NO_SIZE) && (m_iBytesLeft > 0)
             && (m_mimeTypeBuffer.size() < 1024) )
        {
          m_cpMimeBuffer = true;
          return;   // Do not send up the data since we do not yet know its mimetype!
        }

        kDebug(7113) << "Mimetype buffer size: " << m_mimeTypeBuffer.size();

        KMimeType::Ptr mime = KMimeType::findByNameAndContent(m_request.url.fileName(), m_mimeTypeBuffer);
        if( mime && !mime->isDefault() )
        {
          m_mimeType = mime->name();
          kDebug(7113) << "Mimetype from content: " << m_mimeType;
        }

        if ( m_mimeType.isEmpty() )
        {
          m_mimeType = QString::fromLatin1( DEFAULT_MIME_TYPE );
          kDebug(7113) << "Using default mimetype: " <<  m_mimeType;
        }

        //### we could also open the cache file here

        if ( m_cpMimeBuffer )
        {
          d.resize(0);
          d.resize(m_mimeTypeBuffer.size());
          memcpy(d.data(), m_mimeTypeBuffer.data(), d.size());
        }
        mimeType(m_mimeType);
        m_mimeTypeBuffer.resize(0);
      }

      kDebug(7013) << "Sending data of size" << d.size();
      data( d );
      if (m_request.cacheTag.ioMode == WriteToCache) {
        cacheFileWritePayload(d);
      }
   }
   else
   {
      uint old_size = m_webDavDataBuf.size();
      m_webDavDataBuf.resize (old_size + d.size());
      memcpy (m_webDavDataBuf.data() + old_size, d.data(), d.size());
   }
}

/**
 * This function is our "receive" function.  It is responsible for
 * downloading the message (not the header) from the HTTP server.  It
 * is called either as a response to a client's KIOJob::dataEnd()
 * (meaning that the client is done sending data) or by 'sendQuery()'
 * (if we are in the process of a PUT/POST request). It can also be
 * called by a webDAV function, to receive stat/list/property/etc.
 * data; in this case the data is stored in m_webDavDataBuf.
 */
bool HTTPProtocol::readBody( bool dataInternal /* = false */ )
{
  // special case for reading cached body since we also do it in this function. oh well.
  if (!canHaveResponseBody(m_request.responseCode, m_request.method) &&
      !(m_request.cacheTag.ioMode == ReadFromCache && m_request.responseCode == 304 &&
        m_request.method != HTTP_HEAD)) {
      return true;
  }

  m_isEOD = false;
  // Note that when dataInternal is true, we are going to:
  // 1) save the body data to a member variable, m_webDavDataBuf
  // 2) _not_ advertise the data, speed, size, etc., through the
  //    corresponding functions.
  // This is used for returning data to WebDAV.
  m_dataInternal = dataInternal;
  if (dataInternal) {
    m_webDavDataBuf.clear();
  }

  // Check if we need to decode the data.
  // If we are in copy mode, then use only transfer decoding.
  bool useMD5 = !m_contentMD5.isEmpty();

  // Deal with the size of the file.
  KIO::filesize_t sz = m_request.offset;
  if ( sz )
    m_iSize += sz;

    if (!m_isRedirection) {
        // Update the application with total size except when
        // it is compressed, or when the data is to be handled
        // internally (webDAV).  If compressed we have to wait
        // until we uncompress to find out the actual data size
        if ( !dataInternal ) {
            if ((m_iSize > 0) && (m_iSize != NO_SIZE)) {
                totalSize(m_iSize);
                infoMessage(i18n("Retrieving %1 from %2...", KIO::convertSize(m_iSize),
                            m_request.url.host()));
            } else {
                totalSize (0);
            }
        } else {
            infoMessage( i18n( "Retrieving from %1..." ,  m_request.url.host() ) );
        }

        if (m_request.cacheTag.ioMode == ReadFromCache) {
            kDebug(7113) << "read data from cache!";

            m_iContentLeft = NO_SIZE;

            QByteArray d;
            while (true) {
                d = cacheFileReadPayload(4096);
                if (d.isEmpty()) {
                    break;
                }
                slotData(d);
                sz += d.size();
                if (!dataInternal) {
                    processedSize(sz);
                }
            }

            m_receiveBuf.resize(0);

            if (!dataInternal) {
                data(QByteArray());
            }

            return true;
        }
    }

  if (m_iSize != NO_SIZE)
    m_iBytesLeft = m_iSize - sz;
  else
    m_iBytesLeft = NO_SIZE;

  m_iContentLeft = m_iBytesLeft;

  if (m_isChunked)
    m_iBytesLeft = NO_SIZE;

  kDebug(7113) << "retrieve data."<<KIO::number(m_iBytesLeft)<<"left.";

  // Main incoming loop...  Gather everything while we can...
  m_cpMimeBuffer = false;
  m_mimeTypeBuffer.resize(0);

  HTTPFilterChain chain;

  // redirection ignores the body
  if (!m_isRedirection) {
      QObject::connect(&chain, SIGNAL(output(const QByteArray &)),
                       this, SLOT(slotData(const QByteArray &)));
  }
  QObject::connect(&chain, SIGNAL(error(const QString &)),
          this, SLOT(slotFilterError(const QString &)));

   // decode all of the transfer encodings
  while (!m_transferEncodings.isEmpty())
  {
    QString enc = m_transferEncodings.takeLast();
    if ( enc == QLatin1String("gzip") )
      chain.addFilter(new HTTPFilterGZip);
    else if ( enc == QLatin1String("deflate") )
      chain.addFilter(new HTTPFilterDeflate);
  }

  // From HTTP 1.1 Draft 6:
  // The MD5 digest is computed based on the content of the entity-body,
  // including any content-coding that has been applied, but not including
  // any transfer-encoding applied to the message-body. If the message is
  // received with a transfer-encoding, that encoding MUST be removed
  // prior to checking the Content-MD5 value against the received entity.
  HTTPFilterMD5 *md5Filter = 0;
  if ( useMD5 )
  {
     md5Filter = new HTTPFilterMD5;
     chain.addFilter(md5Filter);
  }

  // now decode all of the content encodings
  // -- Why ?? We are not
  // -- a proxy server, be a client side implementation!!  The applications
  // -- are capable of determinig how to extract the encoded implementation.
  // WB: That's a misunderstanding. We are free to remove the encoding.
  // WB: Some braindead www-servers however, give .tgz files an encoding
  // WB: of "gzip" (or even "x-gzip") and a content-type of "applications/tar"
  // WB: They shouldn't do that. We can work around that though...
  while (!m_contentEncodings.isEmpty())
  {
    QString enc = m_contentEncodings.takeLast();
    if ( enc == QLatin1String("gzip") )
      chain.addFilter(new HTTPFilterGZip);
    else if ( enc == QLatin1String("deflate") )
      chain.addFilter(new HTTPFilterDeflate);
  }

  while (!m_isEOF)
  {
    int bytesReceived;

    if (m_isChunked)
       bytesReceived = readChunked();
    else if (m_iSize != NO_SIZE)
       bytesReceived = readLimited();
    else
       bytesReceived = readUnlimited();

    // make sure that this wasn't an error, first
    // kDebug(7113) << "bytesReceived:"
    //              << (int) bytesReceived << " m_iSize:" << (int) m_iSize << " Chunked:"
    //              << m_isChunked << " BytesLeft:"<< (int) m_iBytesLeft;
    if (bytesReceived == -1)
    {
      if (m_iContentLeft == 0)
      {
         // gzip'ed data sometimes reports a too long content-length.
         // (The length of the unzipped data)
         m_iBytesLeft = 0;
         break;
      }
      // Oh well... log an error and bug out
      kDebug(7113) << "bytesReceived==-1 sz=" << (int)sz
                    << " Connection broken !";
      error(ERR_CONNECTION_BROKEN, m_request.url.host());
      return false;
    }

    // I guess that nbytes == 0 isn't an error.. but we certainly
    // won't work with it!
    if (bytesReceived > 0)
    {
      // Important: truncate the buffer to the actual size received!
      // Otherwise garbage will be passed to the app
      m_receiveBuf.truncate( bytesReceived );

      chain.slotInput(m_receiveBuf);

      if (m_isError)
         return false;

      sz += bytesReceived;
      if (!dataInternal)
        processedSize( sz );
    }
    m_receiveBuf.resize(0); // res

    if (m_iBytesLeft && m_isEOD && !m_isChunked)
    {
      // gzip'ed data sometimes reports a too long content-length.
      // (The length of the unzipped data)
      m_iBytesLeft = 0;
    }

    if (m_iBytesLeft == 0)
    {
      kDebug(7113) << "EOD received! Left = "<< KIO::number(m_iBytesLeft);
      break;
    }
  }
  chain.slotInput(QByteArray()); // Flush chain.

  if ( useMD5 )
  {
    QString calculatedMD5 = md5Filter->md5();

    if ( m_contentMD5 != calculatedMD5 )
      kWarning(7113) << "MD5 checksum MISMATCH! Expected: "
                     << calculatedMD5 << ", Got: " << m_contentMD5;
  }

  // Close cache entry
  if (m_iBytesLeft == 0) {
      cacheFileClose(); // no-op if not necessary
  }

  if (sz <= 1)
  {
    if (m_request.responseCode >= 500 && m_request.responseCode <= 599) {
      error(ERR_INTERNAL_SERVER, m_request.url.host());
      return false;
    } else if (m_request.responseCode >= 400 && m_request.responseCode <= 499 &&
               m_request.responseCode != 401 && m_request.responseCode != 407 &&
               // If we're doing a propfind, a 404 is not an error
               m_request.method != DAV_PROPFIND) {
      error(ERR_DOES_NOT_EXIST, m_request.url.host());
      return false;
    }
  }

  if (!dataInternal && !m_isRedirection)
    data( QByteArray() );
  return true;
}

void HTTPProtocol::slotFilterError(const QString &text)
{
    error(KIO::ERR_SLAVE_DEFINED, text);
}

void HTTPProtocol::error( int _err, const QString &_text )
{
  httpClose(false);

  if (!m_request.id.isEmpty())
  {
    forwardHttpResponseHeader();
    sendMetaData();
  }

  // It's over, we don't need it anymore
  m_POSTbuf.clear();

  SlaveBase::error( _err, _text );
  m_isError = true;
}


void HTTPProtocol::addCookies( const QString &url, const QByteArray &cookieHeader )
{
   qlonglong windowId = m_request.windowId.toLongLong();
   QDBusInterface kcookiejar( QLatin1String("org.kde.kded"), QLatin1String("/modules/kcookiejar"), QLatin1String("org.kde.KCookieServer") );
   (void)kcookiejar.call( QDBus::NoBlock, QLatin1String("addCookies"), url,
                           cookieHeader, windowId );
}

QString HTTPProtocol::findCookies( const QString &url)
{
  qlonglong windowId = m_request.windowId.toLongLong();
  QDBusInterface kcookiejar( QLatin1String("org.kde.kded"), QLatin1String("/modules/kcookiejar"), QLatin1String("org.kde.KCookieServer") );
  QDBusReply<QString> reply = kcookiejar.call( QLatin1String("findCookies"), url, windowId );

  if ( !reply.isValid() )
  {
     kWarning(7113) << "Can't communicate with kded_kcookiejar!";
     return QString();
  }
  return reply;
}

/******************************* CACHING CODE ****************************/

HTTPProtocol::CacheTag::CachePlan HTTPProtocol::CacheTag::plan(time_t maxCacheAge) const
{
    //notable omission: we're not checking cache file presence or integrity
    if (policy == KIO::CC_CacheOnly || policy == KIO::CC_Cache) {
        return UseCached;
    } else if (policy == KIO::CC_Refresh) {
        return ValidateCached;
    } else if (policy == KIO::CC_Reload) {
        return IgnoreCached;
    }
    Q_ASSERT(policy == CC_Verify);
    time_t currentDate = time(0);
    if ((servedDate != -1 && currentDate > servedDate + maxCacheAge) ||
        (expireDate != -1 && currentDate > expireDate)) {
        return ValidateCached;
    }
    return UseCached;
}

// !START SYNC!
// The following code should be kept in sync
// with the code in http_cache_cleaner.cpp

// we use QDataStream; this is just an illustration
struct BinaryCacheFileHeader
{
    quint8 version[2];
    quint8 compression; // for now fixed to 0
    quint8 reserved;    // for now; also alignment
    qint32 useCount;
    qint64 servedDate;
    qint64 lastModifiedDate;
    qint64 expireDate;
    qint32 bytesCached;
    // packed size should be 36 bytes; we explicitly set it here to make sure that no compiler
    // padding ruins it. We write the fields to disk without any padding.
    static const int size = 36;
};

enum CacheCleanerCommandCode {
    InvalidCommand = 0,
    CreateFileNotificationCommand,
    UpdateFileCommand
};

// illustration for cache cleaner update "commands"
struct CacheCleanerCommand
{
    BinaryCacheFileHeader header;
    quint32 commandCode;
    // filename in ASCII, binary isn't worth the coding and decoding
    quint8 filename[s_hashedUrlNibbles];
};

QByteArray HTTPProtocol::CacheTag::serialize() const
{
    QByteArray ret;
    QDataStream stream(&ret, QIODevice::WriteOnly);
    stream << quint8('A');
    stream << quint8('\n');
    stream << quint8(0);
    stream << quint8(0);

    stream << fileUseCount;

    // time_t overflow will only be checked when reading; we have no way to tell here.
    stream << qint64(servedDate);
    stream << qint64(lastModifiedDate);
    stream << qint64(expireDate);

    stream << bytesCached;
    Q_ASSERT(ret.size() == BinaryCacheFileHeader::size);
    return ret;
}


static bool compareByte(QDataStream *stream, quint8 value)
{
    quint8 byte;
    *stream >> byte;
    return byte == value;
}

static bool readTime(QDataStream *stream, time_t *time)
{
    qint64 intTime = 0;
    *stream >> intTime;
    *time = static_cast<time_t>(intTime);

    qint64 check = static_cast<qint64>(*time);
    return check == intTime;
}

// If starting a new file cacheFileWriteVariableSizeHeader() must have been called *before*
// calling this! This is to fill in the headerEnd field.
// If the file is not new headerEnd has already been read from the file and in fact the variable
// size header *may* not be rewritten because a size change would mess up the file layout.
bool HTTPProtocol::CacheTag::deserialize(const QByteArray &d)
{
    if (d.size() != BinaryCacheFileHeader::size) {
        return false;
    }
    QDataStream stream(d);
    stream.setVersion(QDataStream::Qt_4_5);

    bool ok = true;
    ok = ok && compareByte(&stream, 'A');
    ok = ok && compareByte(&stream, '\n');
    ok = ok && compareByte(&stream, 0);
    ok = ok && compareByte(&stream, 0);
    if (!ok) {
        return false;
    }

    stream >> fileUseCount;

    // read and check for time_t overflow
    ok = ok && readTime(&stream, &servedDate);
    ok = ok && readTime(&stream, &lastModifiedDate);
    ok = ok && readTime(&stream, &expireDate);
    if (!ok) {
        return false;
    }

    stream >> bytesCached;

    return true;
}

/* Text part of the header, directly following the binary first part:
URL\n
etag\n
mimetype\n
header line\n
header line\n
...
\n
*/

static KUrl storableUrl(const KUrl &url)
{
    KUrl ret(url);
    ret.setPassword(QString());
    ret.setFragment(QString());
    return ret;
}

static void writeLine(QIODevice *dev, const QByteArray &line)
{
    static const char linefeed = '\n';
    dev->write(line);
    dev->write(&linefeed, 1);
}

void HTTPProtocol::cacheFileWriteTextHeader()
{
    QFile *&file = m_request.cacheTag.file;
    Q_ASSERT(file);
    Q_ASSERT(file->openMode() & QIODevice::WriteOnly);

    file->seek(BinaryCacheFileHeader::size);
    writeLine(file, storableUrl(m_request.url).toEncoded());
    writeLine(file, m_request.cacheTag.etag.toLatin1());
    writeLine(file, m_mimeType.toLatin1());
    writeLine(file, m_responseHeaders.join(QString(QLatin1Char('\n'))).toLatin1());
    // join("\n") adds no \n to the end, but writeLine() does.
    // Add another newline to mark the end of text.
    writeLine(file, QByteArray());
}

static bool readLineChecked(QIODevice *dev, QByteArray *line)
{
    *line = dev->readLine(8192);
    // if nothing read or the line didn't fit into 8192 bytes(!)
    if (line->isEmpty() || !line->endsWith('\n')) {
        return false;
    }
    // we don't actually want the newline!
    line->chop(1);
    return true;
}

bool HTTPProtocol::cacheFileReadTextHeader1(const KUrl &desiredUrl)
{
    QFile *&file = m_request.cacheTag.file;
    Q_ASSERT(file);
    Q_ASSERT(file->openMode() == QIODevice::ReadOnly);

    QByteArray readBuf;
    bool ok = readLineChecked(file, &readBuf);
    if (storableUrl(desiredUrl).toEncoded() != readBuf) {
        kDebug(7103) << "You have witnessed a very improbable hash collision!";
        return false;
    }

    ok = ok && readLineChecked(file, &readBuf);
    m_request.cacheTag.etag = QString::fromLatin1(readBuf);

    return ok;
}

bool HTTPProtocol::cacheFileReadTextHeader2()
{
    QFile *&file = m_request.cacheTag.file;
    Q_ASSERT(file);
    Q_ASSERT(file->openMode() == QIODevice::ReadOnly);

    bool ok = true;
    QByteArray readBuf;
#ifndef NDEBUG
    // we assume that the URL and etag have already been read
    qint64 oldPos = file->pos();
    file->seek(BinaryCacheFileHeader::size);
    ok = ok && readLineChecked(file, &readBuf);
    ok = ok && readLineChecked(file, &readBuf);
    Q_ASSERT(file->pos() == oldPos);
#endif
    ok = ok && readLineChecked(file, &readBuf);
    m_mimeType = QString::fromLatin1(readBuf);

    m_responseHeaders.clear();
    // read as long as no error and no empty line found
    while (true) {
        ok = ok && readLineChecked(file, &readBuf);
        if (ok && !readBuf.isEmpty()) {
            m_responseHeaders.append(QString::fromLatin1(readBuf));
        } else {
            break;
        }
    }
    return ok; // it may still be false ;)
}

static QString filenameFromUrl(const KUrl &url)
{
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(storableUrl(url).toEncoded());
    return QString::fromLatin1(hash.result().toHex());
}

QString HTTPProtocol::cacheFilePathFromUrl(const KUrl &url) const
{
    QString filePath = m_strCacheDir;
    if (!filePath.endsWith(QLatin1Char('/'))) {
        filePath.append(QLatin1Char('/'));
    }
    filePath.append(filenameFromUrl(url));
    return filePath;
}

bool HTTPProtocol::cacheFileOpenRead()
{
    kDebug(7113);
    QString filename = cacheFilePathFromUrl(m_request.url);

    QFile *&file = m_request.cacheTag.file;
    if (file) {
        kDebug(7113) << "File unexpectedly open; old file is" << file->fileName()
                     << "new name is" << filename;
        Q_ASSERT(file->fileName() == filename);
    }
    Q_ASSERT(!file);
    file = new QFile(filename);
    if (file->open(QIODevice::ReadOnly)) {
        QByteArray header = file->read(BinaryCacheFileHeader::size);
        if (!m_request.cacheTag.deserialize(header)) {
            kDebug(7103) << "Cache file header is invalid.";

            file->close();
        }
    }

    if (file->isOpen() && !cacheFileReadTextHeader1(m_request.url)) {
        file->close();
    }

    if (!file->isOpen()) {
        cacheFileClose();
        return false;
    }
    return true;
}


bool HTTPProtocol::cacheFileOpenWrite()
{
    kDebug(7113);
    QString filename = cacheFilePathFromUrl(m_request.url);

    // if we open a cache file for writing while we have a file open for reading we must have
    // found out that the old cached content is obsolete, so delete the file.
    QFile *&file = m_request.cacheTag.file;
    if (file) {
        // ensure that the file is in a known state - either open for reading or null
        Q_ASSERT(!qobject_cast<QTemporaryFile *>(file));
        Q_ASSERT((file->openMode() & QIODevice::WriteOnly) == 0);
        Q_ASSERT(file->fileName() == filename);
        kDebug(7113) << "deleting expired cache entry and recreating.";
        file->remove();
        delete file;
        file = 0;
    }

    // note that QTemporaryFile will automatically append random chars to filename
    file = new QTemporaryFile(filename);
    file->open(QIODevice::WriteOnly);

    // if we have started a new file we have not initialized some variables from disk data.
    m_request.cacheTag.fileUseCount = 0;  // the file has not been *read* yet
    m_request.cacheTag.bytesCached = 0;

    if ((file->openMode() & QIODevice::WriteOnly) == 0) {
        kDebug(7113) << "Could not open file for writing:" << file->fileName()
                     << "due to error" << file->error();
        cacheFileClose();
        return false;
    }
    return true;
}

static QByteArray makeCacheCleanerCommand(const HTTPProtocol::CacheTag &cacheTag,
                                          CacheCleanerCommandCode cmd)
{
    QByteArray ret = cacheTag.serialize();
    QDataStream stream(&ret, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_4_5);

    stream.skipRawData(BinaryCacheFileHeader::size);
    // append the command code
    stream << quint32(cmd);
    // append the filename
    QString fileName = cacheTag.file->fileName();
    int basenameStart = fileName.lastIndexOf(QLatin1Char('/')) + 1;
    QByteArray baseName = fileName.mid(basenameStart, s_hashedUrlNibbles).toLatin1();
    stream.writeRawData(baseName.constData(), baseName.size());

    Q_ASSERT(ret.size() == BinaryCacheFileHeader::size + sizeof(quint32) + s_hashedUrlNibbles);
    return ret;
}

//### not yet 100% sure when and when not to call this
void HTTPProtocol::cacheFileClose()
{
    kDebug(7113);

    QFile *&file = m_request.cacheTag.file;
    if (!file) {
        return;
    }

    m_request.cacheTag.ioMode = NoCache;

    QByteArray ccCommand;
    QTemporaryFile *tempFile = qobject_cast<QTemporaryFile *>(file);

    if (file->openMode() & QIODevice::WriteOnly) {
        Q_ASSERT(tempFile);

        if (m_request.cacheTag.bytesCached && !m_isError) {
            QByteArray header = m_request.cacheTag.serialize();
            tempFile->seek(0);
            tempFile->write(header);

            ccCommand = makeCacheCleanerCommand(m_request.cacheTag, CreateFileNotificationCommand);

            QString oldName = tempFile->fileName();
            QString newName = oldName;
            int basenameStart = newName.lastIndexOf(QLatin1Char('/')) + 1;
            // remove the randomized name part added by QTemporaryFile
            newName.chop(newName.length() - basenameStart - s_hashedUrlNibbles);
            kDebug(7113) << "Renaming temporary file" << oldName << "to" << newName;

            // on windows open files can't be renamed
            tempFile->setAutoRemove(false);
            delete tempFile;
            file = 0;

            if (!QFile::rename(oldName, newName)) {
                // ### currently this hides a minor bug when force-reloading a resource. We
                //     should not even open a new file for writing in that case.
                kDebug(7113) << "Renaming temporary file failed, deleting it instead.";
                QFile::remove(oldName);
                ccCommand.clear();  // we have nothing of value to tell the cache cleaner
            }
        } else {
            // oh, we've never written payload data to the cache file.
            // the temporary file is closed and removed and no proper cache entry is created.
        }
    } else if (file->openMode() == QIODevice::ReadOnly) {
        Q_ASSERT(!tempFile);
        ccCommand = makeCacheCleanerCommand(m_request.cacheTag, UpdateFileCommand);
    }
    delete file;
    file = 0;

    if (!ccCommand.isEmpty()) {
        sendCacheCleanerCommand(ccCommand);
    }
}

void HTTPProtocol::sendCacheCleanerCommand(const QByteArray &command)
{
    kDebug(7113);
    Q_ASSERT(command.size() == BinaryCacheFileHeader::size + s_hashedUrlNibbles + sizeof(quint32));
    int attempts = 0;
    while (m_cacheCleanerConnection.state() != QLocalSocket::ConnectedState && attempts < 6) {
        if (attempts == 2) {
            KToolInvocation::startServiceByDesktopPath(QLatin1String("http_cache_cleaner.desktop"));
        }
        QString socketFileName = KStandardDirs::locateLocal("socket", QLatin1String("kio_http_cache_cleaner"));
        m_cacheCleanerConnection.connectToServer(socketFileName, QIODevice::WriteOnly);
        m_cacheCleanerConnection.waitForConnected(1500);
        attempts++;
    }

    if (m_cacheCleanerConnection.state() == QLocalSocket::ConnectedState) {
        m_cacheCleanerConnection.write(command);
        m_cacheCleanerConnection.flush();
    } else {
        // updating the stats is not vital, so we just give up.
        kDebug(7113) << "Could not connect to cache cleaner, not updating stats of this cache file.";
    }
}

QByteArray HTTPProtocol::cacheFileReadPayload(int maxLength)
{
    Q_ASSERT(m_request.cacheTag.file);
    Q_ASSERT(m_request.cacheTag.ioMode == ReadFromCache);
    Q_ASSERT(m_request.cacheTag.file->openMode() == QIODevice::ReadOnly);
    QByteArray ret = m_request.cacheTag.file->read(maxLength);
    if (ret.isEmpty()) {
        cacheFileClose();
    }
    return ret;
}


void HTTPProtocol::cacheFileWritePayload(const QByteArray &d)
{
    if (!m_request.cacheTag.file) {
        return;
    }
    Q_ASSERT(m_request.cacheTag.ioMode == WriteToCache);
    Q_ASSERT(m_request.cacheTag.file->openMode() & QIODevice::WriteOnly);
    if (d.isEmpty()) {
        cacheFileClose();
    }

    //### abort if file grows too big! (early abort if we know the size, implement!)

    // write the variable length text header as soon as we start writing to the file
    if (!m_request.cacheTag.bytesCached) {
        cacheFileWriteTextHeader();
    }
    m_request.cacheTag.bytesCached += d.size();
    m_request.cacheTag.file->write(d);
}

// The above code should be kept in sync
// with the code in http_cache_cleaner.cpp
// !END SYNC!

//**************************  AUTHENTICATION CODE ********************/

QString HTTPProtocol::authenticationHeader()
{
    QByteArray ret;

    // If the internal meta-data "cached-www-auth" is set, then check for cached
    // authentication data and preemtively send the authentication header if a
    // matching one is found.
    if (!m_wwwAuth && config()->readEntry("cached-www-auth", false)) {
        KIO::AuthInfo authinfo;
        authinfo.url = m_request.url;
        authinfo.realmValue = config()->readEntry("www-auth-realm", QString());
        // If no relam metadata, then make sure path matching is turned on.
        authinfo.verifyPath = (authinfo.realmValue.isEmpty());

        if (checkCachedAuthentication(authinfo)) {
            const QByteArray cachedChallenge = authinfo.digestInfo.toLatin1();
            if (!cachedChallenge.isEmpty()) {
                m_wwwAuth = KAbstractHttpAuthentication::newAuth(cachedChallenge, config());
                if (m_wwwAuth) {
                    kDebug(7113) << "Creating WWW authentcation object from cached info";
                    m_wwwAuth->setChallenge(cachedChallenge, m_request.url, methodString(m_request.method));
                    m_wwwAuth->generateResponse(authinfo.username, authinfo.password);
                }
            }
        }
    }

    // If the internal meta-data "cached-proxy-auth" is set, then check for cached
    // authentication data and preemtively send the authentication header if a
    // matching one is found.
    if (!m_proxyAuth && config()->readEntry("cached-proxy-auth", false)) {
        KIO::AuthInfo authinfo;
        authinfo.url = m_request.proxyUrl;
        authinfo.realmValue = config()->readEntry("proxy-auth-realm", QString());
        // If no relam metadata, then make sure path matching is turned on.
        authinfo.verifyPath = (authinfo.realmValue.isEmpty());

        if (checkCachedAuthentication(authinfo)) {
            const QByteArray cachedChallenge = authinfo.digestInfo.toLatin1();
            if (!cachedChallenge.isEmpty()) {
                m_proxyAuth = KAbstractHttpAuthentication::newAuth(cachedChallenge, config());
                if (m_proxyAuth) {
                    kDebug(7113) << "Creating Proxy authentcation object from cached info";
                    m_proxyAuth->setChallenge(cachedChallenge, m_request.proxyUrl, methodString(m_request.method));
                    m_proxyAuth->generateResponse(authinfo.username, authinfo.password);
                }
            }
        }
    }

    // the authentication classes don't know if they are for proxy or webserver authentication...
    if (m_wwwAuth && !m_wwwAuth->isError()) {
        ret += "Authorization: ";
        ret += m_wwwAuth->headerFragment();
    }

    if (m_proxyAuth && !m_proxyAuth->isError()) {
        ret += "Proxy-Authorization: ";
        ret += m_proxyAuth->headerFragment();
    }

    return QString::fromLatin1(ret); // ## encoding ok?
}


void HTTPProtocol::proxyAuthenticationForSocket(const QNetworkProxy &proxy, QAuthenticator *authenticator)
{
    Q_UNUSED(proxy);
    kDebug(7113) << "Authenticator received -- realm: " << authenticator->realm() << "user:"
                 << authenticator->user();

    AuthInfo info;
    Q_ASSERT(proxy.hostName() == m_request.proxyUrl.host() && proxy.port() == m_request.proxyUrl.port());
    info.url = m_request.proxyUrl;
    info.realmValue = authenticator->realm();
    info.verifyPath = true;    //### whatever
    info.username = authenticator->user();

    const bool haveCachedCredentials = checkCachedAuthentication(info);

    // if m_socketProxyAuth is a valid pointer then authentication has been attempted before,
    // and it was not successful. see below and saveProxyAuthenticationForSocket().
    if (!haveCachedCredentials || m_socketProxyAuth) {
        // Save authentication info if the connection succeeds. We need to disconnect
        // this after saving the auth data (or an error) so we won't save garbage afterwards!
        connect(socket(), SIGNAL(connected()),
                this, SLOT(saveProxyAuthenticationForSocket()));
        //### fillPromptInfo(&info);
        info.prompt = i18n("You need to supply a username and a password for "
                           "the proxy server listed below before you are allowed "
                           "to access any sites.");
        info.keepPassword = true;
        info.commentLabel = i18n("Proxy:");
        info.comment = i18n("<b>%1</b> at <b>%2</b>", info.realmValue, m_request.proxyUrl.host());
        const bool dataEntered = openPasswordDialog(info, i18n("Proxy Authentication Failed."));
        if (!dataEntered) {
            kDebug(7103) << "looks like the user canceled proxy authentication.";
            error(ERR_USER_CANCELED, m_request.proxyUrl.host());
        }
    }
    authenticator->setUser(info.username);
    authenticator->setPassword(info.password);

    if (m_socketProxyAuth) {
        *m_socketProxyAuth = *authenticator;
    } else {
        m_socketProxyAuth = new QAuthenticator(*authenticator);
    }

    m_request.proxyUrl.setUser(info.username);
    m_request.proxyUrl.setPassword(info.password);
}

void HTTPProtocol::saveProxyAuthenticationForSocket()
{
    kDebug(7113) << "Saving authenticator";
    disconnect(socket(), SIGNAL(connected()),
               this, SLOT(saveProxyAuthenticationForSocket()));
    Q_ASSERT(m_socketProxyAuth);
    if (m_socketProxyAuth) {
        kDebug(7113) << "-- realm: " << m_socketProxyAuth->realm() << "user:"
                     << m_socketProxyAuth->user();
        KIO::AuthInfo a;
        a.verifyPath = true;
        a.url = m_request.proxyUrl;
        a.realmValue = m_socketProxyAuth->realm();
        a.username = m_socketProxyAuth->user();
        a.password = m_socketProxyAuth->password();
        cacheAuthentication(a);
    }
    delete m_socketProxyAuth;
    m_socketProxyAuth = 0;
}

#include "http.moc"
