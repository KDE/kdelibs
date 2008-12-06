/*
   Copyright (C) 2000-2003 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2000-2002 George Staikos <staikos@kde.org>
   Copyright (C) 2000-2002 Dawit Alemayehu <adawit@kde.org>
   Copyright (C) 2001,2002 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2007      Nick Shaforostoff <shafff@ukr.net>
   Copyright (C) 2007      Daniel Nicoletti <mirttex@users.sourceforge.net>
   Copyright (C) 2008      Andreas Hartmetz <ahartmetz@gmail.com>

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

#include "http.h"

#include <config.h>
#include <config-gssapi.h>

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
#include <kcodecs.h>
#include <kcomponentdata.h>
#include <krandom.h>
#include <kmimetype.h>
#include <ktoolinvocation.h>
#include <kstandarddirs.h>
#include <kremoteencoding.h>

#include <kio/ioslave_defaults.h>
#include <kio/http_slave_defaults.h>

#include <httpfilter.h>

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
  if (originURL == "true") // Backwards compatibility
     return true;

  KUrl url ( originURL );

  // Document Origin domain
  QString a = url.host();
  // Current request domain
  QString b = fqdn;

  if (a == b)
    return false;

  QStringList la = a.split('.', QString::SkipEmptyParts);
  QStringList lb = b.split('.', QString::SkipEmptyParts);

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
  const QStringList headers = _header.split(QRegExp("[\r\n]"));

  for(QStringList::ConstIterator it = headers.begin(); it != headers.end(); ++it)
  {
    QString header = (*it).toLower();
    // Do not allow Request line to be specified and ignore
    // the other HTTP headers.
    if (!header.contains(':') || header.startsWith("host") ||
        header.startsWith("proxy-authorization") ||
        header.startsWith("via"))
      continue;

    sanitizedHeaders += (*it);
    sanitizedHeaders += "\r\n";
  }
  sanitizedHeaders.chop(2);

  return sanitizedHeaders;
}

static bool isEncryptedHttpVariety(const QString &p)
{
    return p == "https" || p == "webdavs"; 
}

static QString methodString(HTTP_METHOD m)
{
    switch(m) {
    case HTTP_GET:
        return"GET ";
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
    default:
        Q_ASSERT(false);
        return QString();
    }
}






#define NO_SIZE		((KIO::filesize_t) -1)

#ifdef HAVE_STRTOLL
#define STRTOLL	strtoll
#else
#define STRTOLL	strtol
#endif


/************************************** HTTPProtocol **********************************************/


HTTPProtocol::HTTPProtocol( const QByteArray &protocol, const QByteArray &pool,
                            const QByteArray &app )
    : TCPSlaveBase(protocol, pool, app, isEncryptedHttpVariety(protocol))
    , m_defaultPort(0)
    , m_iSize(NO_SIZE)
    , m_isBusy(false)
    , m_isFirstRequest(false)
    , m_maxCacheAge(DEFAULT_MAX_CACHE_AGE)
    , m_maxCacheSize(DEFAULT_MAX_CACHE_SIZE/2)
    , m_protocol(protocol)
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

    m_proxyAuth.realm.clear();
    m_proxyAuth.authorization.clear();
    m_proxyAuth.scheme = AUTH_None;
    m_request.proxyUrl.clear(); //TODO revisit

    if (isEncryptedHttpVariety(m_protocol))
        m_defaultPort = DEFAULT_HTTPS_PORT;
    else if (m_protocol == "ftp")
        m_defaultPort = DEFAULT_FTP_PORT;
    else
        m_defaultPort = DEFAULT_HTTP_PORT;
}

void HTTPProtocol::resetConnectionSettings()
{
  m_isEOF = false;
  m_isError = false;
  m_auth.authCount = 0;
  m_proxyAuth.authCount = 0;
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

  setMetaData("request-id", m_request.id);
}

void HTTPProtocol::resetSessionSettings()
{
  // Do not reset the URL on redirection if the proxy
  // URL, username or password has not changed!
  KUrl proxy ( config()->readEntry("UseProxy") );
  QNetworkProxy::ProxyType proxyType = QNetworkProxy::NoProxy;

  if ( m_proxyAuth.realm.isEmpty() || !proxy.isValid() ||
       m_request.proxyUrl.host() != proxy.host() ||
       m_request.proxyUrl.port() != proxy.port() ||
       (!proxy.user().isEmpty() && proxy.user() != m_request.proxyUrl.user()) ||
       (!proxy.pass().isEmpty() && proxy.pass() != m_request.proxyUrl.pass()) )
  {
    m_request.proxyUrl = proxy;

    kDebug(7113) << "Using proxy:" << m_request.proxyUrl.isValid()
                 << "URL: " << m_request.proxyUrl.url()
                 << "Realm: " << m_proxyAuth.realm;
  }

  if (m_request.proxyUrl.isValid() && m_request.proxyUrl.hasHost()) {
      if (m_request.proxyUrl.protocol() == "socks") {
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


  m_request.isPersistentProxyConnection = config()->readEntry("PersistentProxyConnection", false);
  kDebug(7113) << "Enable Persistent Proxy Connection: "
                << m_request.isPersistentProxyConnection;

  m_request.useCookieJar = config()->readEntry("Cookies", false);
  m_request.cacheTag.useCache = config()->readEntry("UseCache", true);
  m_request.preferErrorPage = config()->readEntry("errorPage", true);
  m_request.doNotAuthenticate = config()->readEntry("no-auth", false);
  m_strCacheDir = config()->readPathEntry("CacheDir", QString());
  m_maxCacheAge = config()->readEntry("MaxCacheAge", DEFAULT_MAX_CACHE_AGE);
  m_request.windowId = config()->readEntry("window-id");

  kDebug(7113) << "Window Id =" << m_request.windowId;
  kDebug(7113) << "ssl_was_in_use ="
               << metaData ("ssl_was_in_use");

  m_request.referrer.clear();
  // RFC 2616: do not send the referrer if the referrer page was served using SSL and
  //           the current page does not use SSL.
  if ( config()->readEntry("SendReferrer", true) &&
       (isEncryptedHttpVariety(m_protocol) || metaData ("ssl_was_in_use") != "TRUE" ) )
  {
     KUrl refUrl(metaData("referrer"));
     if (refUrl.isValid()) {
        // Sanitize
        QString protocol = refUrl.protocol();
        if (protocol.startsWith("webdav")) {
           protocol.replace(0, 6, "http");
           refUrl.setProtocol(protocol);
        }

        if (protocol.startsWith("http")) {
           m_request.referrer = refUrl.toEncoded(QUrl::RemoveUserInfo | QUrl::RemoveFragment);
        }
     }
  }

  if (config()->readEntry("SendLanguageSettings", true)) {
      m_request.charsets = config()->readEntry( "Charsets", "iso-8859-1" );
      if (!m_request.charsets.isEmpty()) {
          m_request.charsets += DEFAULT_PARTIAL_CHARSET_HEADER;
      }
      m_request.languages = config()->readEntry( "Languages", DEFAULT_LANGUAGE_HEADER );
  } else {
      m_request.charsets.clear();
      m_request.languages.clear();
  }

  // Adjust the offset value based on the "resume" meta-data.
  QString resumeOffset = metaData("resume");
  if (!resumeOffset.isEmpty()) {
     m_request.offset = resumeOffset.toULongLong();
  } else {
     m_request.offset = 0;
  }
  // Same procedure for endoffset.
  QString resumeEndOffset = metaData("resume_until");
  if (!resumeEndOffset.isEmpty()) {
     m_request.endoffset = resumeEndOffset.toULongLong();
  } else {
     m_request.endoffset = 0;
  }

  m_request.disablePassDialog = config()->readEntry("DisablePassDlg", false);
  m_request.allowTransferCompression = config()->readEntry("AllowCompressedPage", true);
  m_request.id = metaData("request-id");

  // Store user agent for this host.
  if (config()->readEntry("SendUserAgent", true)) {
     m_request.userAgent = metaData("UserAgent");
  } else {
     m_request.userAgent.clear();
  }

  // Deal with cache cleaning.
  // TODO: Find a smarter way to deal with cleaning the
  // cache ?
  if (m_request.cacheTag.useCache) {
     cleanCache();
  }

  m_request.responseCode = 0;
  m_request.prevResponseCode = 0;

  m_auth.realm.clear();
  m_auth.authorization.clear();
  m_auth.scheme = AUTH_None;

  // Obtain timeout values
  m_remoteRespTimeout = responseTimeout();

  // Bounce back the actual referrer sent
  setMetaData("referrer", m_request.referrer);

  // Follow HTTP/1.1 spec and enable keep-alive by default
  // unless the remote side tells us otherwise or we determine
  // the persistent link has been terminated by the remote end.
  m_request.isKeepAlive = true;
  m_request.keepAliveTimeout = 0;
  m_isUnauthorized = false;

  // A single request can require multiple exchanges with the remote
  // server due to authentication challenges or SSL tunneling.
  // m_isFirstRequest is a flag that indicates whether we are
  // still processing the first request. This is important because we
  // should not force a close of a keep-alive connection in the middle
  // of the first request.
  // m_isFirstRequest is set to "true" whenever a new connection is
  // made in httpOpenConnection()
  m_isFirstRequest = false;
}

void HTTPProtocol::setHost( const QString& host, quint16 port,
                            const QString& user, const QString& pass )
{
  // Reset the webdav-capable flags for this host
  if ( m_request.url.host() != host )
    m_davHostOk = m_davHostUnsupported = false;

  m_request.url.setHost(host);

  // is it an IPv6 address?
  if (host.indexOf(':') == -1) {
      m_request.encoded_hostname = QUrl::toAce(host);
  } else  {
      int pos = host.indexOf('%');
      if (pos == -1)
        m_request.encoded_hostname = '[' + host + ']';
      else
        // don't send the scope-id in IPv6 addresses to the server
        m_request.encoded_hostname = '[' + host.left(pos) + ']';
  }
  m_request.url.setPort((port <= 0) ? m_defaultPort : port);
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
  m_request.url.setPort((u.port() <= 0) ? m_defaultPort : u.port());

  if (u.host().isEmpty()) {
     error( KIO::ERR_UNKNOWN_HOST, i18n("No host specified."));
     return false;
  }

  if (u.path().isEmpty()) {
     KUrl newUrl(u);
     newUrl.setPath("/");
     redirection(newUrl);
     finished();
     return false;
  }

  if (m_protocol != u.protocol().toLatin1()) {
     short unsigned int oldDefaultPort = m_defaultPort;
     m_protocol = u.protocol().toLatin1();
     reparseConfiguration();
     if (m_defaultPort != oldDefaultPort && m_request.url.port() == oldDefaultPort) {
        m_request.url.setPort(m_defaultPort);
     }
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
          error(ERR_NO_CONTENT, "");
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
      } else if (m_isError) {
          // Hard error, abort everything.
          return false;
      }
  }

  // Do not save authorization if the current response code is
  // 4xx (client error) or 5xx (server error).
  kDebug(7113) << "Previous Response:" << m_request.prevResponseCode;
  kDebug(7113) << "Current Response:" << m_request.responseCode;

  setMetaData("responsecode", QString::number(m_request.responseCode));
  setMetaData("content-type", m_mimeType);

  if (m_request.responseCode < 400 &&
      (m_request.prevResponseCode == 401 || m_request.prevResponseCode == 407)) {
      saveAuthorization(m_request.prevResponseCode == 407);
  }

  // At this point sendBody() should have delivered any POST data.
  m_POSTbuf.clear();

  // Save the most interesting current state for keep-alive decisions
  m_prevRequest.initFrom(m_request);

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
    if ( statSide != "source" )
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
  QString query = metaData("davSearchQuery");
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
    if ( hasMetaData( "davRequestResponse" ) )
      request += metaData( "davRequestResponse" ).toUtf8();
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

    QDomElement href = thisResponse.namedItem( "href" ).toElement();
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
        // don't list the base dir of a listDir()
        if ( !stat && thisURL.path(KUrl::AddTrailingSlash).length() == url.path(KUrl::AddTrailingSlash).length() )
          continue;

        entry.insert( KIO::UDSEntry::UDS_NAME, thisURL.fileName() );
      } else {
        // This is a relative URL.
        entry.insert( KIO::UDSEntry::UDS_NAME, href.text() );
      }

      QDomNodeList propstats = thisResponse.elementsByTagName( "propstat" );

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
      kDebug(7113) << "Error: no URL contained in response to PROPFIND on"
                    << url.prettyUrl();
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
  int firstSpace = response.indexOf( ' ' );
  int secondSpace = response.indexOf( ' ', firstSpace + 1 );
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

    QDomElement status = propstat.namedItem( "status" ).toElement();
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

    QDomElement prop = propstat.namedItem( "prop" ).toElement();
    if ( prop.isNull() )
    {
      kDebug(7113) << "Error: no prop segment in this propstat.";
      return;
    }

    if ( hasMetaData( "davRequestResponse" ) )
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

      if ( property.namespaceURI() != "DAV:" )
      {
        // break out - we're only interested in properties from the DAV namespace
        continue;
      }

      if ( property.tagName() == "creationdate" )
      {
        // Resource creation date. Should be is ISO 8601 format.
        entry.insert( KIO::UDSEntry::UDS_CREATION_TIME, parseDateTime( property.text(), property.attribute("dt") ) );
      }
      else if ( property.tagName() == "getcontentlength" )
      {
        // Content length (file size)
        entry.insert( KIO::UDSEntry::UDS_SIZE, property.text().toULong() );
      }
      else if ( property.tagName() == "displayname" )
      {
        // Name suitable for presentation to the user
        setMetaData( "davDisplayName", property.text() );
      }
      else if ( property.tagName() == "source" )
      {
        // Source template location
        QDomElement source = property.namedItem( "link" ).toElement()
                                      .namedItem( "dst" ).toElement();
        if ( !source.isNull() )
          setMetaData( "davSource", source.text() );
      }
      else if ( property.tagName() == "getcontentlanguage" )
      {
        // equiv. to Content-Language header on a GET
        setMetaData( "davContentLanguage", property.text() );
      }
      else if ( property.tagName() == "getcontenttype" )
      {
        // Content type (mime type)
        // This may require adjustments for other server-side webdav implementations
        // (tested with Apache + mod_dav 1.0.3)
        if ( property.text() == "httpd/unix-directory" )
        {
          isDirectory = true;
        }
        else
        {
	  mimeType = property.text();
        }
      }
      else if ( property.tagName() == "executable" )
      {
        // File executable status
        if ( property.text() == "T" )
          foundExecutable = true;

      }
      else if ( property.tagName() == "getlastmodified" )
      {
        // Last modification date
        entry.insert( KIO::UDSEntry::UDS_MODIFICATION_TIME, parseDateTime( property.text(), property.attribute("dt") ) );
      }
      else if ( property.tagName() == "getetag" )
      {
        // Entity tag
        setMetaData( "davEntityTag", property.text() );
      }
      else if ( property.tagName() == "supportedlock" )
      {
        // Supported locking specifications
        for ( QDomNode n2 = property.firstChild(); !n2.isNull(); n2 = n2.nextSibling() )
        {
          QDomElement lockEntry = n2.toElement();
          if ( lockEntry.tagName() == "lockentry" )
          {
            QDomElement lockScope = lockEntry.namedItem( "lockscope" ).toElement();
            QDomElement lockType = lockEntry.namedItem( "locktype" ).toElement();
            if ( !lockScope.isNull() && !lockType.isNull() )
            {
              // Lock type was properly specified
              supportedLockCount++;
              QString scope = lockScope.firstChild().toElement().tagName();
              QString type = lockType.firstChild().toElement().tagName();

              setMetaData( QString("davSupportedLockScope%1").arg(supportedLockCount), scope );
              setMetaData( QString("davSupportedLockType%1").arg(supportedLockCount), type );
            }
          }
        }
      }
      else if ( property.tagName() == "lockdiscovery" )
      {
        // Lists the available locks
        davParseActiveLocks( property.elementsByTagName( "activelock" ), lockCount );
      }
      else if ( property.tagName() == "resourcetype" )
      {
        // Resource type. "Specifies the nature of the resource."
        if ( !property.namedItem( "collection" ).toElement().isNull() )
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

  setMetaData( "davLockCount", QString("%1").arg(lockCount) );
  setMetaData( "davSupportedLockCount", QString("%1").arg(supportedLockCount) );

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
    QDomElement lockScope = activeLock.namedItem( "lockscope" ).toElement();
    QDomElement lockType = activeLock.namedItem( "locktype" ).toElement();
    QDomElement lockDepth = activeLock.namedItem( "depth" ).toElement();
    // optional
    QDomElement lockOwner = activeLock.namedItem( "owner" ).toElement();
    QDomElement lockTimeout = activeLock.namedItem( "timeout" ).toElement();
    QDomElement lockToken = activeLock.namedItem( "locktoken" ).toElement();

    if ( !lockScope.isNull() && !lockType.isNull() && !lockDepth.isNull() )
    {
      // lock was properly specified
      lockCount++;
      QString scope = lockScope.firstChild().toElement().tagName();
      QString type = lockType.firstChild().toElement().tagName();
      QString depth = lockDepth.text();

      setMetaData( QString("davLockScope%1").arg( lockCount ), scope );
      setMetaData( QString("davLockType%1").arg( lockCount ), type );
      setMetaData( QString("davLockDepth%1").arg( lockCount ), depth );

      if ( !lockOwner.isNull() )
        setMetaData( QString("davLockOwner%1").arg( lockCount ), lockOwner.text() );

      if ( !lockTimeout.isNull() )
        setMetaData( QString("davLockTimeout%1").arg( lockCount ), lockTimeout.text() );

      if ( !lockToken.isNull() )
      {
        QDomElement tokenVal = lockScope.namedItem( "href" ).toElement();
        if ( !tokenVal.isNull() )
          setMetaData( QString("davLockToken%1").arg( lockCount ), tokenVal.text() );
      }
    }
  }
}

long HTTPProtocol::parseDateTime( const QString& input, const QString& type )
{
  if ( type == "dateTime.tz" )
  {
    return KDateTime::fromString( input, KDateTime::ISODate ).toTime_t();
  }
  else if ( type == "dateTime.rfc1123" )
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
  if ( hasMetaData( "davLockCount" ) )
  {
    QString response("If:");
    int numLocks;
    numLocks = metaData( "davLockCount" ).toInt();
    bool bracketsOpen = false;
    for ( int i = 0; i < numLocks; i++ )
    {
      if ( hasMetaData( QString("davLockToken%1").arg(i) ) )
      {
        if ( hasMetaData( QString("davLockURL%1").arg(i) ) )
        {
          if ( bracketsOpen )
          {
            response += ')';
            bracketsOpen = false;
          }
          response += " <" + metaData( QString("davLockURL%1").arg(i) ) + '>';
        }

        if ( !bracketsOpen )
        {
          response += " (";
          bracketsOpen = true;
        }
        else
        {
          response += ' ';
        }

        if ( hasMetaData( QString("davLockNot%1").arg(i) ) )
          response += "Not ";

        response += '<' + metaData( QString("davLockToken%1").arg(i) ) + '>';
      }
    }

    if ( bracketsOpen )
      response += ')';

    response += "\r\n";
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
  m_request.url.setPath("*");
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

  QString tmp(metaData("cache"));
  if (!tmp.isEmpty())
    m_request.cacheTag.policy = parseCacheControl(tmp);
  else
    m_request.cacheTag.policy = DEFAULT_CACHE_CONTROL;

  proceedUntilResponseContent();
}

void HTTPProtocol::put( const KUrl &url, int, KIO::JobFlags flags )
{
  kDebug(7113) << url.url();

  if (!maybeSetRequestUrl(url))
    return;
  resetSessionSettings();

  // Webdav hosts are capable of observing overwrite == false
  if (!(flags & KIO::Overwrite) && m_protocol.startsWith("webdav")) {
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
    httpError();
}

void HTTPProtocol::copy( const KUrl& src, const KUrl& dest, int, KIO::JobFlags flags )
{
  kDebug(7113) << src.url() << "->" << dest.url();

  if (!maybeSetRequestUrl(dest) || !maybeSetRequestUrl(src))
    return;
  resetSessionSettings();

  // destination has to be "http(s)://..."
  KUrl newDest = dest;
  if (newDest.protocol() == "webdavs")
    newDest.setProtocol("https");
  else
    newDest.setProtocol("http");

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
  if (newDest.protocol() == "webdavs")
    newDest.setProtocol("https");
  else
    newDest.setProtocol("http");

  m_request.method = DAV_MOVE;
  m_request.davData.desturl = newDest.url();
  m_request.davData.overwrite = (flags & KIO::Overwrite);
  m_request.url.setQuery(QString());
  m_request.cacheTag.policy = CC_Reload;

  proceedUntilResponseHeader();

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

  // The server returns a HTTP/1.1 200 Ok or HTTP/1.1 204 No Content
  // on successful completion
  if ( m_protocol.startsWith( "webdav" ) ) {
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

  QDomElement lockInfo = lockReq.createElementNS( "DAV:", "lockinfo" );
  lockReq.appendChild( lockInfo );

  QDomElement lockScope = lockReq.createElement( "lockscope" );
  lockInfo.appendChild( lockScope );

  lockScope.appendChild( lockReq.createElement( scope ) );

  QDomElement lockType = lockReq.createElement( "locktype" );
  lockInfo.appendChild( lockType );

  lockType.appendChild( lockReq.createElement( type ) );

  if ( !owner.isNull() ) {
    QDomElement ownerElement = lockReq.createElement( "owner" );
    lockReq.appendChild( ownerElement );

    QDomElement ownerHref = lockReq.createElement( "href" );
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

    QDomElement prop = multiResponse.documentElement().namedItem( "prop" ).toElement();

    QDomElement lockdiscovery = prop.namedItem( "lockdiscovery" ).toElement();

    uint lockCount = 0;
    davParseActiveLocks( lockdiscovery.elementsByTagName( "activelock" ), lockCount );

    setMetaData( "davLockCount", QString("%1").arg( lockCount ) );

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

      QDomElement multistatus = multiResponse.documentElement().namedItem( "multistatus" ).toElement();

      QDomNodeList responses = multistatus.elementsByTagName( "response" );

      for (int i = 0; i < responses.count(); i++)
      {
        int errCode;
        QString errUrl;

        QDomElement response = responses.item(i).toElement();
        QDomElement code = response.namedItem( "status" ).toElement();

        if ( !code.isNull() )
        {
          errCode = codeFromResponse( code.text() );
          QDomElement href = response.namedItem( "href" ).toElement();
          if ( !href.isNull() )
            errUrl = href.text();
          errors << davError( errCode, errUrl );
        }
      }

      //kError = ERR_SLAVE_DEFINED;
      errorString = i18nc( "%1: request type, %2: url",
                           "An error occurred while attempting to %1, %2. A "
                           "summary of the reasons is below.", action, url );

      errorString += "<ul>";

      for ( QStringList::const_iterator it = errors.constBegin(); it != errors.constEnd(); ++it )
        errorString += "<li>" + *it + "</li>";

      errorString += "</ul>";
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

void HTTPProtocol::httpError()
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

bool HTTPProtocol::isOffline(const KUrl &url)
{
  const int NetWorkStatusUnknown = 1;
  const int NetWorkStatusOnline = 8;

  QDBusReply<int> reply =
    QDBusInterface( "org.kde.kded", "/modules/networkstatus", "org.kde.NetworkStatusModule" ).
    call( "status", url.url() );

  if ( reply.isValid() )
  {
     int result = reply;
     kDebug(7113) << "networkstatus status = " << result;
     return (result != NetWorkStatusUnknown) && (result != NetWorkStatusOnline);
  }
  kDebug(7113) << "networkstatus <unreachable>";
  return false; // On error, assume we are online
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
        
        QString tmp = metaData("cache");
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
            kDebug(7113) << "check one: isKeepAlive =" << m_request.isKeepAlive;
            it.setValue(m_request);
            m_prevRequest.initFrom(m_request);
        }
        // collect the responses
        //### for the moment we use a hack: instead of saving and restoring request-id
        //    we just count up like ParallelGetJobs does.
        int requestId = 0;
        foreach (const HTTPRequest &r, m_requestQueue) {
            m_request = r;
            kDebug(7113) << "check two: isKeepAlive =" << m_request.isKeepAlive;
            setMetaData("request-id", QString::number(requestId++));
            sendAndKeepMetaData();
            if (!(readResponseHeader() && readBody())) {
                return;
            }
            // the "next job" signal for ParallelGetJob is data of size zero which
            // readBody() sends without our intervention.
            kDebug(7113) << "check three: isKeepAlive =" << m_request.isKeepAlive;
            httpClose(m_request.isKeepAlive);  //actually keep-alive is mandatory for pipelining
            m_prevRequest.initFrom(m_request);
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

void HTTPProtocol::unread(char *buf, size_t size)
{
    // implement LIFO (stack) semantics
    const int newSize = m_unreadBuf.size() + size;
    m_unreadBuf.resize(newSize);
    for (int i = 0; i < size; i++) {
        m_unreadBuf.data()[newSize - i - 1] = buf[i];
    }
    if (size) {
        //hey, we still have data, closed connection or not!
        m_isEOF = false;
    }
}

size_t HTTPProtocol::readBuffered(char *buf, size_t size)
{
    size_t bytesRead = 0;
    if (!m_unreadBuf.isEmpty()) {
        const int bufSize = m_unreadBuf.size();
        bytesRead = qMin((int)size, bufSize);

        for (int i = 0; i < bytesRead; i++) {
            buf[i] = m_unreadBuf.constData()[bufSize - i - 1];
        }
        m_unreadBuf.truncate(bufSize - bytesRead);
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

        for (int i = 0; i < bufferFill ; i++, pos++) {
            char c = mybuf[i];
            buf[pos] = c;
            
            // did we just copy one or two times the \r\n delimiter?
            if (c == '\n' && pos > (2 * numNewlines - 2) && buf[pos - 1] == '\r' &&
                ((numNewlines == 1) || (buf[pos - 3] == '\r' && buf [pos - 2] == '\n'))) {
                i++;    // unget bytes *after* CRLF
                unread(&mybuf[i], bufferFill - i);
                *idx = pos + 1;
                return true;
            }
        }
    }
    *idx = pos;
    return false;
}


bool HTTPProtocol::httpShouldCloseConnection()
{
  kDebug(7113) << "Keep Alive:" << m_request.isKeepAlive << "First:" << m_isFirstRequest;

  if (m_isFirstRequest || !isConnected()) {
      return false;
  }

  if (m_request.method != HTTP_GET && m_request.method != HTTP_POST) {
      return true;
  }

  if (m_request.proxyUrl != m_prevRequest.proxyUrl) {
      return true;
  }

  // TODO compare current proxy state against proxy needs of next request,
  // *when* we actually have variable proxy settings!

  if (m_request.proxyUrl.isValid())  {
      if (m_request.proxyUrl != m_prevRequest.proxyUrl ||
          m_request.proxyUrl.user() != m_prevRequest.proxyUrl.user() ||
          m_request.proxyUrl.pass() != m_prevRequest.proxyUrl.pass()) {
          return true;
      }
  } else {
      if (m_request.url.host() != m_prevRequest.url.host() ||
          m_request.url.port() != m_prevRequest.url.port() ||
          m_request.url.user() != m_prevRequest.url.user() ||
          m_request.url.pass() != m_prevRequest.url.pass()) {
          return true;
      }
  }
  return false;
}

bool HTTPProtocol::httpOpenConnection()
{
  kDebug(7113);

  // Only save proxy auth information after proxy authentication has
  // actually taken place, which will set up exactly this connection.
  disconnect(socket(), SIGNAL(connected()),
             this, SLOT(saveProxyAuthenticationForSocket()));

  clearUnreadBuffer();

  bool connectOk = false;
  if (m_request.proxyUrl.isValid() && m_request.proxyUrl.protocol() == "http" && !isAutoSsl()) {
      connectOk = connectToHost(m_request.proxyUrl.protocol(), m_request.proxyUrl.host(), m_request.proxyUrl.port());
  } else {
      connectOk = connectToHost(m_protocol, m_state.hostname, m_state.port);
  }

  if (!connectOk) {
      return false;
  }

#if 0                           // QTcpSocket doesn't support this
  // Set our special socket option!!
  socket().setNoDelay(true);
#endif

  m_isFirstRequest = true;
  connected();
  return true;
}

bool HTTPProtocol::satisfyRequestFromCache(bool *success)
{
    m_request.cacheTag.gzs = 0;
    m_request.cacheTag.readFromCache = false;
    m_request.cacheTag.writeToCache = false;
    m_request.cacheTag.isExpired = false;
    m_request.cacheTag.expireDate = 0;
    m_request.cacheTag.creationDate = 0;

    if (m_request.cacheTag.useCache) {

        m_request.cacheTag.gzs = checkCacheEntry();
        bool bCacheOnly = (m_request.cacheTag.policy == KIO::CC_CacheOnly);
        bool bOffline = isOffline(m_request.proxyUrl.isValid() ? m_request.proxyUrl : m_request.url);

        if (bOffline && m_request.cacheTag.policy != KIO::CC_Reload) {
            m_request.cacheTag.policy= KIO::CC_CacheOnly;
        }

        if (m_request.cacheTag.policy == CC_Reload && m_request.cacheTag.gzs) {
            gzclose(m_request.cacheTag.gzs);
            m_request.cacheTag.gzs = 0;
        }
        if (m_request.cacheTag.policy == KIO::CC_CacheOnly ||
            m_request.cacheTag.policy == KIO::CC_Cache) {
            m_request.cacheTag.isExpired = false;
        }

        m_request.cacheTag.writeToCache = true;

        if (m_request.cacheTag.gzs && !m_request.cacheTag.isExpired) {
            // Cache entry is OK. Cache hit.
            m_request.cacheTag.readFromCache = true;
            *success = true;
            return true;
        } else if (!m_request.cacheTag.gzs) {
            // Cache miss.
            m_request.cacheTag.isExpired = false;
        } else {
            // Conditional cache hit. (Validate)
        }

        if (bCacheOnly) {
            error(ERR_DOES_NOT_EXIST, m_request.url.url());
            *success = false;
            return true;
        }
        if (bOffline) {
            error(ERR_COULD_NOT_CONNECT, m_request.url.url());
            *success = false;
            return true;
        }
    }
    *success = true;   //whatever
    return false;
}

QString HTTPProtocol::formatRequestUri() const
{
    // Only specify protocol, host and port when they are not already clear, i.e. when
    // we handle HTTP proxying ourself and the proxy server needs to know them.
    // Sending protocol/host/port in other cases confuses some servers, and it's not their fault.
    if (m_request.proxyUrl.isValid() && m_request.proxyUrl.protocol() == "http" && !isAutoSsl()) {
        KUrl u;

        QString protocol = m_protocol;
        if (protocol.startsWith("webdav")) {
            protocol.replace(0, strlen("webdav"), "http");
        }
        u.setProtocol(protocol);

        u.setHost( m_state.hostname );
        if (m_state.port != m_defaultPort) {
            u.setPort( m_state.port );
        }
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
    error(ERR_UNSUPPORTED_PROTOCOL, m_protocol);
    return false;
  }

  bool cacheHasPage = false;
  if (satisfyRequestFromCache(&cacheHasPage)) {
    return cacheHasPage;  
  }

  QString header;

  bool hasBodyData = false;
  bool hasDavData = false;

  // Let's update our current state
  m_state.hostname = m_request.url.host();
  m_state.encoded_hostname = m_request.encoded_hostname;
  m_state.port = m_request.url.port();
  m_state.user = m_request.url.user();
  m_state.passwd = m_request.url.pass();

  {
    header = methodString(m_request.method);
    QString davHeader;
  
    // Fill in some values depending on the HTTP method to guide further processing
    switch (m_request.method)
    {
    case HTTP_GET:
    case HTTP_HEAD:
        break;
    case HTTP_PUT:
    case HTTP_POST:
        hasBodyData = true;
        m_request.cacheTag.writeToCache = false; // Do not put any result in the cache
        break;
    case HTTP_DELETE:
    case HTTP_OPTIONS:
        m_request.cacheTag.writeToCache = false; // Do not put any result in the cache
        break;
    case DAV_PROPFIND:
        hasDavData = true;
        davHeader = "Depth: ";
        if ( hasMetaData( "davDepth" ) )
        {
          kDebug(7113) << "Reading DAV depth from metadata: " << metaData( "davDepth" );
          davHeader += metaData( "davDepth" );
        }
        else
        {
          if ( m_request.davData.depth == 2 )
            davHeader += "infinity";
          else
            davHeader += QString("%1").arg( m_request.davData.depth );
        }
        davHeader += "\r\n";
        m_request.cacheTag.writeToCache = false; // Do not put any result in the cache
        break;
    case DAV_PROPPATCH:
        hasDavData = true;
        m_request.cacheTag.writeToCache = false; // Do not put any result in the cache
        break;
    case DAV_MKCOL:
        m_request.cacheTag.writeToCache = false; // Do not put any result in the cache
        break;
    case DAV_COPY:
    case DAV_MOVE:
        davHeader = "Destination: " + m_request.davData.desturl;
        // infinity depth means copy recursively
        // (optional for copy -> but is the desired action)
        davHeader += "\r\nDepth: infinity\r\nOverwrite: ";
        davHeader += m_request.davData.overwrite ? "T" : "F";
        davHeader += "\r\n";
        m_request.cacheTag.writeToCache = false; // Do not put any result in the cache
        break;
    case DAV_LOCK:
        davHeader = "Timeout: ";
        {
          uint timeout = 0;
          if ( hasMetaData( "davTimeout" ) )
            timeout = metaData( "davTimeout" ).toUInt();
          if ( timeout == 0 )
            davHeader += "Infinite";
          else
            davHeader += QString("Seconds-%1").arg(timeout);
        }
        davHeader += "\r\n";
        m_request.cacheTag.writeToCache = false; // Do not put any result in the cache
        hasDavData = true;
        break;
    case DAV_UNLOCK:
        davHeader = "Lock-token: " + metaData("davLockToken") + "\r\n";
        m_request.cacheTag.writeToCache = false; // Do not put any result in the cache
        break;
    case DAV_SEARCH:
        hasDavData = true;
        /* fall through */
    case DAV_SUBSCRIBE:
    case DAV_UNSUBSCRIBE:
    case DAV_POLL:
        m_request.cacheTag.writeToCache = false;
        break;
    default:
        error (ERR_UNSUPPORTED_ACTION, QString());
        return false;
    }
    // DAV_POLL; DAV_NOTIFY

    header += formatRequestUri() + " HTTP/1.1\r\n"; /* start header */
    
    /* support for virtual hosts and required by HTTP 1.1 */
    header += "Host: " + m_state.encoded_hostname;
    if (m_state.port != m_defaultPort) {
      header += QString(":%1").arg(m_state.port);
    }
    header += "\r\n";

    // Support old HTTP/1.0 style keep-alive header for compatibility
    // purposes as well as performance improvements while giving end
    // users the ability to disable this feature proxy servers that
    // don't not support such feature, e.g. junkbuster proxy server.
    if (!m_request.proxyUrl.isValid() || m_request.isPersistentProxyConnection) {
        header += "Connection: Keep-Alive\r\n";
    } else {
        header += "Connection: close\r\n";
    }

    if (!m_request.userAgent.isEmpty())
    {
        header += "User-Agent: ";
        header += m_request.userAgent;
        header += "\r\n";
    }

    if (!m_request.referrer.isEmpty())
    {
        header += "Referer: "; //Don't try to correct spelling!
        header += m_request.referrer;
        header += "\r\n";
    }

    if ( m_request.endoffset > m_request.offset )
    {
        header += QString("Range: bytes=%1-%2\r\n").arg(KIO::number(m_request.offset))
                         .arg(KIO::number(m_request.endoffset));
        kDebug(7103) << "kio_http : Range = " << KIO::number(m_request.offset) <<
                        " - "  << KIO::number(m_request.endoffset);
    }
    else if ( m_request.offset > 0 && m_request.endoffset == 0 )
    {
        header += QString("Range: bytes=%1-\r\n").arg(KIO::number(m_request.offset));
        kDebug(7103) << "kio_http : Range = " << KIO::number(m_request.offset);
    }

    if ( m_request.cacheTag.policy== CC_Reload )
    {
      /* No caching for reload */
      header += "Pragma: no-cache\r\n"; /* for HTTP/1.0 caches */
      header += "Cache-control: no-cache\r\n"; /* for HTTP >=1.1 caches */
    }

    if (m_request.cacheTag.isExpired)
    {
      /* conditional get */
      if (!m_request.cacheTag.etag.isEmpty())
        header += "If-None-Match: "+m_request.cacheTag.etag+"\r\n";
      if (!m_request.cacheTag.lastModified.isEmpty())
        header += "If-Modified-Since: "+m_request.cacheTag.lastModified+"\r\n";
    }

    header += "Accept: ";
    QString acceptHeader = metaData("accept");
    if (!acceptHeader.isEmpty())
      header += acceptHeader;
    else
      header += DEFAULT_ACCEPT_HEADER;
    header += "\r\n";

#ifdef DO_GZIP
    if (m_request.allowTransferCompression)
      header += "Accept-Encoding: x-gzip, x-deflate, gzip, deflate\r\n";
#endif

    if (!m_request.charsets.isEmpty())
      header += "Accept-Charset: " + m_request.charsets + "\r\n";

    if (!m_request.languages.isEmpty())
      header += "Accept-Language: " + m_request.languages + "\r\n";

    QString cookieStr;
    QString cookieMode = metaData("cookies").toLower();
    if (cookieMode == "none")
    {
      m_request.cookieMode = HTTPRequest::CookiesNone;
    }
    else if (cookieMode == "manual")
    {
      m_request.cookieMode = HTTPRequest::CookiesManual;
      cookieStr = metaData("setcookies");
    }
    else
    {
      m_request.cookieMode = HTTPRequest::CookiesAuto;
      if (m_request.useCookieJar)
        cookieStr = findCookies(m_request.url.url());
    }

    if (!cookieStr.isEmpty())
      header += cookieStr + "\r\n";

    QString customHeader = metaData( "customHTTPHeader" );
    if (!customHeader.isEmpty())
    {
      header += sanitizeCustomHTTPHeader(customHeader);
      header += "\r\n";
    }

    QString contentType = metaData("content-type");
    if ((m_request.method == HTTP_POST || m_request.method == HTTP_PUT)
    && !contentType.isEmpty())
    {
      header += contentType;
      header += "\r\n";
    }

    // Remember that at least one failed (with 401 or 407) request/response
    // roundtrip is necessary for the server to tell us that it requires
    // authentication.
    // If we already have cached credentials, or we have obtained authentication
    // data from the user since the previous attempt, we add that information here.
    header += wwwAuthenticationHeader();

    // Do we need to authorize to the proxy server ?
    if (m_request.proxyUrl.isValid()) {
      if ( m_request.isPersistentProxyConnection )
        header += "Proxy-Connection: Keep-Alive\r\n";
      header += proxyAuthenticationHeader();
    }

    if ( m_protocol == "webdav" || m_protocol == "webdavs" )
    {
      header += davProcessLocks();

      // add extra webdav headers, if supplied
      davHeader += metaData("davHeader");

      // Set content type of webdav data
      if (hasDavData)
        davHeader += "Content-Type: text/xml; charset=utf-8\r\n";

      // add extra header elements for WebDAV
      header += davHeader;
    }
  }

  kDebug(7103) << "============ Sending Header:";
  foreach (const QString &s, header.split("\r\n", QString::SkipEmptyParts)) {
    kDebug(7103) << s;
  }

  // End the header iff there is no payload data. If we do have payload data
  // sendBody() will add another field to the header, Content-Length.
  if (!hasBodyData && !hasDavData)
    header += "\r\n";

  // Check the reusability of the current connection.
  if (httpShouldCloseConnection()) {
    httpCloseConnection();
  }

  // Now that we have our formatted header, let's send it!
  // Create a new connection to the remote machine if we do
  // not already have one...
  if ( !isConnected() )
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
    kDebug(7113) << "Connection broken! (" << m_state.hostname << ")"
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
    error( ERR_CONNECTION_BROKEN, m_state.hostname );
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
    setMetaData("HTTP-Headers", m_responseHeaders.join("\n"));
    sendMetaData();
  }
}

bool HTTPProtocol::readHeaderFromCache() {
    m_responseHeaders.clear();

    // Read header from cache...
    char buffer[4097];
    if (!gzgets(m_request.cacheTag.gzs, buffer, 4096) )
    {
        // Error, delete cache entry
        kDebug(7113) << "Could not access cache to obtain mimetype!";
        error( ERR_CONNECTION_BROKEN, m_state.hostname );
        return false;
    }

    m_mimeType = QString::fromLatin1(buffer).trimmed();

    kDebug(7113) << "cached data mimetype: " << m_mimeType;

    // read http-headers, first the response code
    if (!gzgets(m_request.cacheTag.gzs, buffer, 4096) )
    {
        // Error, delete cache entry
        kDebug(7113) << "Could not access cached data! ";
        error( ERR_CONNECTION_BROKEN, m_state.hostname );
        return false;
    }
    m_responseHeaders << buffer;
    // then the headers
    while(true) {
        if (!gzgets(m_request.cacheTag.gzs, buffer, 8192) )
        {
            // Error, delete cache entry
            kDebug(7113) << "Could not access cached data!";
            error( ERR_CONNECTION_BROKEN, m_state.hostname );
            return false;
        }
        m_responseHeaders << buffer;
        QString header = QString::fromLatin1(buffer).trimmed().toLower();
        if (header.isEmpty()) {
            break;
        }
        if (header.startsWith("content-type: ")) {
            int pos = header.indexOf("charset=");
            if (pos != -1) {
                QString charset = header.mid(pos+8);
                m_request.cacheTag.charset = charset;
                setMetaData("charset", charset);
            }
        } else
        if (header.startsWith("content-language: ")) {
            QString language = header.mid(18);
            setMetaData("content-language", language);
        } else
        if (header.startsWith("content-disposition:")) {
            parseContentDisposition(header.mid(20));
        }
    }
    forwardHttpResponseHeader();

    if (!m_request.cacheTag.lastModified.isEmpty())
        setMetaData("modified", m_request.cacheTag.lastModified);

    setMetaData("expire-date", QString::number(m_request.cacheTag.expireDate));
    setMetaData("cache-creation-date", QString::number(m_request.cacheTag.creationDate));
    
    mimeType(m_mimeType);
    return true;
}

void HTTPProtocol::fixupResponseMimetype()
{
    // Convert some common mimetypes to standard mimetypes
    if (m_mimeType == "application/x-targz")
        m_mimeType = QString::fromLatin1("application/x-compressed-tar");
    else if (m_mimeType == "image/x-png")
        m_mimeType = QString::fromLatin1("image/png");
    else if (m_mimeType == "audio/x-mp3" || m_mimeType == "audio/x-mpeg" || m_mimeType == "audio/mp3")
        m_mimeType = QString::fromLatin1("audio/mpeg");
    else if (m_mimeType == "audio/microsoft-wave")
        m_mimeType = QString::fromLatin1("audio/x-wav");

    // Crypto ones....
    else if (m_mimeType == "application/pkix-cert" ||
             m_mimeType == "application/binary-certificate") {
        m_mimeType = QString::fromLatin1("application/x-x509-ca-cert");
    }

    // Prefer application/x-compressed-tar or x-gzpostscript over application/x-gzip.
    else if (m_mimeType == "application/x-gzip") {
        if ((m_request.url.path().endsWith(".tar.gz")) ||
            (m_request.url.path().endsWith(".tar")))
            m_mimeType = QString::fromLatin1("application/x-compressed-tar");
        if ((m_request.url.path().endsWith(".ps.gz")))
            m_mimeType = QString::fromLatin1("application/x-gzpostscript");
    }

    // Some webservers say "text/plain" when they mean "application/x-bzip"
    else if ((m_mimeType == "text/plain") || (m_mimeType == "application/octet-stream")) {
        QString ext = m_request.url.path().right(4).toUpper();
        if (ext == ".BZ2")
            m_mimeType = QString::fromLatin1("application/x-bzip");
        else if (ext == ".PEM")
            m_mimeType = QString::fromLatin1("application/x-x509-ca-cert");
        else if (ext == ".SWF")
            m_mimeType = QString::fromLatin1("application/x-shockwave-flash");
        else if (ext == ".PLS")
            m_mimeType = QString::fromLatin1("audio/x-scpls");
        else if (ext == ".WMV")
            m_mimeType = QString::fromLatin1("video/x-ms-wmv");
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
try_again:
    kDebug(7113);

    if (m_request.cacheTag.readFromCache) {
        return readHeaderFromCache();
    }

    // QStrings to force deep copy from "volatile" QByteArray that TokenIterator supplies.
    // One generally has to be very careful with those!
    QString locationStr; // In case we get a redirect.
    QByteArray cookieStr; // In case we get a cookie.

    QString mediaValue;
    QString mediaAttribute;

    QStringList upgradeOffers;

    bool upgradeRequired = false;   // Server demands that we upgrade to something
                                    // This is also true if we ask to upgrade and
                                    // the server accepts, since we are now
                                    // committed to doing so
    bool canUpgrade = false;        // The server offered an upgrade


    m_request.cacheTag.etag.clear();
    m_request.cacheTag.lastModified.clear();
    m_request.cacheTag.charset.clear();
    m_responseHeaders.clear();

    time_t dateHeader = 0;
    time_t expireDate = 0; // 0 = no info, 1 = already expired, > 1 = actual date
    int currentAge = 0;
    int maxAge = -1; // -1 = no max age, 0 already expired, > 0 = actual time
    static const int maxHeaderSize = 128 * 1024;

    int len = 0;
    char buffer[maxHeaderSize];
    bool cont = false;
    bool cacheValidated = false; // Revalidation was successful
    bool mayCache = true;
    bool hasCacheDirective = false;
    bool bCanResume = false;

    if (!isConnected()) {
        kDebug(7113) << "No connection.";
        return false; // Reestablish connection and try again
    }

    if (!waitForResponse(m_remoteRespTimeout)) {
        // No response error
        error(ERR_SERVER_TIMEOUT , m_state.hostname);
        return false;
    }

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
        error( ERR_CONNECTION_BROKEN, m_state.hostname );
        return false;
    }
    if (!foundDelimiter) {
        //### buffer too small for first line of header(!)
        Q_ASSERT(0);
    }

    kDebug(7103) << "============ Received Response:";
    kDebug(7113) << buffer;

    bool noHeader = true;
    HTTP_REV httpRev = HTTP_None;
    int headerSize = 0;

    int idx = 0;

    if (idx != bufPos && buffer[idx] == '<') {
        kDebug(7103) << "No valid HTTP header found! Document starts with XML/HTML tag";
        // document starts with a tag, assume HTML instead of text/plain
        m_mimeType = "text/html";
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
        goto endParsing; //### ### correct?
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

    if (m_request.responseCode >= 500 && m_request.responseCode <= 599) {
        // Server side errors
        
        if (m_request.method == HTTP_HEAD) {
            ; // Ignore error
        } else {
            if (m_request.preferErrorPage) {
                errorPage();
            } else {
                error(ERR_INTERNAL_SERVER, m_request.url.url());
                return false;
            }
        }
        m_request.cacheTag.writeToCache = false; // Don't put in cache
        mayCache = false;
    } else if (m_request.responseCode == 401 || m_request.responseCode == 407) {
        // Unauthorized access
    
        // Double authorization requests, i.e. a proxy auth
        // request followed immediately by a regular auth request.
        if (m_request.prevResponseCode != m_request.responseCode &&
            (m_request.prevResponseCode == 401 || m_request.prevResponseCode == 407)) {
            saveAuthorization(m_request.prevResponseCode == 407);
        }
        m_isUnauthorized = true;
        m_request.cacheTag.writeToCache = false; // Don't put in cache
        mayCache = false;
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
        if (m_request.preferErrorPage) {
            errorPage();
        } else {
            error(ERR_DOES_NOT_EXIST, m_request.url.url());
            return false;
        }
        m_request.cacheTag.writeToCache = false; // Don't put in cache
        mayCache = false;
    } else if (m_request.responseCode == 307) {
        // 307 Temporary Redirect
        m_request.cacheTag.writeToCache = false; // Don't put in cache
        mayCache = false;
    } else if (m_request.responseCode == 304) {
        // 304 Not Modified
        // The value in our cache is still valid.
        cacheValidated = true;
    
    } else if (m_request.responseCode >= 301 && m_request.responseCode<= 303) {
        // 301 Moved permanently
        if (m_request.responseCode == 301) {
            setMetaData("permanent-redirect", "true");
        }
        // 302 Found (temporary location)
        // 303 See Other
        if (m_request.method != HTTP_HEAD && m_request.method != HTTP_GET) {
#if 0
            // Reset the POST buffer to avoid a double submit
            // on redirection
            if (m_request.method == HTTP_POST) {
                m_POSTbuf.resize(0);
            }
#endif

            // NOTE: This is wrong according to RFC 2616.  However,
            // because most other existing user agent implementations
            // treat a 301/302 response as a 303 response and preform
            // a GET action regardless of what the previous method was,
            // many servers have simply adapted to this way of doing
            // things!!  Thus, we are forced to do the same thing or we
            // won't be able to retrieve these pages correctly!! See RFC
            // 2616 sections 10.3.[2/3/4/8]
            m_request.method = HTTP_GET; // Force a GET
        }
        m_request.cacheTag.writeToCache = false; // Don't put in cache
        mayCache = false;
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

    // done with the first line; now tokenize the other lines

  endParsing: //### if we goto here nothing good comes out of it. rethink.

    // TODO review use of STRTOLL vs. QByteArray::toInt()

    foundDelimiter = readDelimitedText(buffer, &bufPos, maxHeaderSize, 2);
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
    if (tIt.hasNext() && tIt.next().toLower().startsWith("none")) {
        bCanResume = false;
    }

    tIt = tokenizer.iterator("keep-alive");
    while (tIt.hasNext()) {
        if (tIt.next().startsWith("timeout=")) {
            m_request.keepAliveTimeout = tIt.current().mid(strlen("timeout=")).trimmed().toInt();
        }
    }

    tIt = tokenizer.iterator("cache-control");
    while (tIt.hasNext()) {
        QByteArray cacheStr = tIt.next().toLower();
        if (cacheStr.startsWith("no-cache") || cacheStr.startsWith("no-store")) {
            // Don't put in cache
            m_request.cacheTag.writeToCache = false;
            mayCache = false;
            hasCacheDirective = true;
        } else if (cacheStr.startsWith("max-age=")) {
            QByteArray age = cacheStr.mid(strlen("max-age=")).trimmed();
            if (!age.isEmpty()) {
                maxAge = STRTOLL(age.constData(), 0, 10);
                hasCacheDirective = true;
            }
        }
    }

    // get the size of our data
    tIt = tokenizer.iterator("content-length");
    if (tIt.hasNext()) {
        m_iSize = STRTOLL(tIt.next().constData(), 0, 10);
    }

    tIt = tokenizer.iterator("content-location");
    if (tIt.hasNext()) {
        setMetaData("content-location", QString::fromLatin1(tIt.next().trimmed()));
    }

    // which type of data do we have?
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
        foreach (const QByteArray &statement, l) {
            QList<QByteArray> parts = statement.split('=');
            if (parts.count() != 2) {
                continue;
            }
            mediaAttribute = parts[0].trimmed().toLower();
            mediaValue = parts[1].trimmed();
            if (mediaValue.length() && (mediaValue[0] == '"') &&
                (mediaValue[mediaValue.length() - 1] == '"')) {
                mediaValue = mediaValue.mid(1, mediaValue.length() - 2);
            }
            kDebug (7113) << "Encoding-type: " << mediaAttribute
                          << "=" << mediaValue;

            if (mediaAttribute == "charset") {
                mediaValue = mediaValue.toLower();
                m_request.cacheTag.charset = mediaValue;
                setMetaData("charset", mediaValue);
            } else {
                setMetaData("media-" + mediaAttribute, mediaValue);
            }
        }
    }  

    // Date
    tIt = tokenizer.iterator("date");
    if (tIt.hasNext()) {
        dateHeader = KDateTime::fromString(tIt.next(), KDateTime::RFCDate).toTime_t();
    }

    // Cache management
    tIt = tokenizer.iterator("date");
    if (tIt.hasNext()) {
        //note QByteArray -> QString conversion will make a deep copy; we want one.
        m_request.cacheTag.etag = QString(tIt.next());
    }

    tIt = tokenizer.iterator("date");
    if (tIt.hasNext()) {
        //note QByteArray -> QString conversion will make a deep copy; we want one.
        m_request.cacheTag.etag = QString(tIt.next());
    }

    tIt = tokenizer.iterator("expires");
    if (tIt.hasNext()) {
        expireDate = KDateTime::fromString(tIt.next(), KDateTime::RFCDate).toTime_t();
        if (!expireDate) {
            expireDate = 1; // Already expired
        }
    }

    tIt = tokenizer.iterator("last-modified");
    if (tIt.hasNext()) {
        m_request.cacheTag.lastModified = QString(tIt.next());
    }

    // whoops.. we received a warning
    tIt = tokenizer.iterator("warning");
    if (tIt.hasNext()) {
        //Don't use warning() here, no need to bother the user.
        //Those warnings are mostly about caches.
        infoMessage(tIt.next());
    }

    // Cache management (HTTP 1.0)
    tIt = tokenizer.iterator("pragma");
    while (tIt.hasNext()) {
        if (tIt.next().toLower().startsWith("no-cache")) {
            m_request.cacheTag.writeToCache = false; // Don't put in cache
            mayCache = false;
            hasCacheDirective = true;
        }
    }
    
    // The deprecated Refresh Response
    tIt = tokenizer.iterator("refresh");
    if (tIt.hasNext()) {
        mayCache = false;  // Do not cache page as it defeats purpose of Refresh tag!
        setMetaData("http-refresh", QString::fromLatin1(tIt.next().trimmed()));
    }

    // In fact we should do redirection only if we have a redirection response code (300 range)
    tIt = tokenizer.iterator("location");
    if (tIt.hasNext() && m_request.responseCode > 299 && m_request.responseCode < 400) {
        locationStr = tIt.next().trimmed();
    }

    // Harvest cookies (mmm, cookie fields!)
    tIt = tokenizer.iterator("set-cookie");
    while (tIt.hasNext()) {
        cookieStr += "Set-Cookie: ";
        cookieStr += tIt.next();
        cookieStr += '\n';
    }

    // check for direct authentication
    tIt = tokenizer.iterator("www-authenticate");
    if (tIt.hasNext()) {
        configAuth(tIt.all(), false);
    }

    // check for proxy-based authentication
    tIt = tokenizer.iterator("proxy-authenticate");
    if (tIt.hasNext()) {
        configAuth(tIt.all(), true);
    }

    tIt = tokenizer.iterator("upgrade");
    if (tIt.hasNext()) {
        // Now we have to check to see what is offered for the upgrade
        QString offered = QString::fromLatin1(tIt.next());
        upgradeOffers = offered.split(QRegExp("[ \n,\r\t]"), QString::SkipEmptyParts);
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
        addEncoding(tIt.next(), m_contentEncodings);
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
            setMetaData("content-language", language);
        }
    }
    
    tIt = tokenizer.iterator("proxy-connection");
    if (tIt.hasNext()) {
        QByteArray pc = tIt.next().toLower();
        if (pc.startsWith("close")) {
            m_request.isKeepAlive = false;
        } else if (pc.startsWith("keep-alive")) {
            m_request.isKeepAlive = true;
        }
    }
    
    tIt = tokenizer.iterator("link");
    if (tIt.hasNext()) {
        // We only support Link: <url>; rel="type"   so far
        QStringList link = QString::fromLatin1(tIt.next()).split(';', QString::SkipEmptyParts);
        if (link.count() == 2) {
            QString rel = link[1].trimmed();
            if (rel.startsWith("rel=\"")) {
                rel = rel.mid(5, rel.length() - 6);
                if (rel.toLower() == "pageservices") {
                    //### the remove() part looks fishy!
                    QString url = link[0].remove(QRegExp("[<>]")).trimmed();
                    setMetaData("PageServices", url);
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
                                 .split('=', QString::SkipEmptyParts);
            if (policy.count() == 2) {
                if (policy[0].toLower() == "policyref") {
                    policyrefs << policy[1].remove(QRegExp("[\"\']")).trimmed();
                } else if (policy[0].toLower() == "cp") {
                    // We convert to cp\ncp\ncp\n[...]\ncp to be consistent with
                    // other metadata sent in strings.  This could be a bit more
                    // efficient but I'm going for correctness right now.
                    const QString s = policy[1].remove(QRegExp("[\"\']"));
                    const QStringList cps = s.split(' ', QString::SkipEmptyParts);
                    compact << cps;
                }
            }
        }
        if (!policyrefs.isEmpty()) {
            setMetaData("PrivacyPolicy", policyrefs.join("\n"));
        }
        if (!compact.isEmpty()) {
            setMetaData("PrivacyCompactPolicy", compact.join("\n"));
        }
    }

    // continue only if we know that we're at least HTTP/1.0
    if (httpRev == HTTP_11 || httpRev == HTTP_10) {
        // let them tell us if we should stay alive or not
        tIt = tokenizer.iterator("connection");
        while (tIt.hasNext()) {
            QByteArray connection = tIt.next().toLower();
            if (connection.startsWith("close")) {
                m_request.isKeepAlive = false;
            } else if (connection.startsWith("keep-alive")) {
                m_request.isKeepAlive = true;
            } else if (connection.startsWith("upgrade")) {
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
            addEncoding(tIt.next().trimmed(), m_transferEncodings);
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
    foreach (const QString &opt, upgradeOffers) {
        if (opt == "TLS/1.0") {
            if (!startSsl() && upgradeRequired) {
                error(ERR_UPGRADE_REQUIRED, opt);
                return false;
            }
        } else if (opt == "HTTP/1.1") {
            httpRev = HTTP_11;
        } else if (upgradeRequired) {
            // we are told to do an upgrade we don't understand
            error(ERR_UPGRADE_REQUIRED, opt);
            return false;
        }
    }

    // If we do not support the requested authentication method...
    if ((m_request.responseCode == 401 && m_auth.scheme == AUTH_None) ||
        (m_request.responseCode == 407 && m_proxyAuth.scheme == AUTH_None)) {
        m_isUnauthorized = false;
        if (m_request.preferErrorPage) {
            errorPage();
        } else {
            error( ERR_UNSUPPORTED_ACTION, "Unknown Authorization method!" );
            return false;
        }
    }

  // Fixup expire date for clock drift.
  if (expireDate && (expireDate <= dateHeader))
    expireDate = 1; // Already expired.

  // Convert max-age into expireDate (overriding previous set expireDate)
  if (maxAge == 0)
    expireDate = 1; // Already expired.
  else if (maxAge > 0)
  {
    if (currentAge)
      maxAge -= currentAge;
    if (maxAge <=0)
      maxAge = 0;
    expireDate = time(0) + maxAge;
  }

  if (!expireDate)
  {
    time_t lastModifiedDate = 0;
    if (!m_request.cacheTag.lastModified.isEmpty())
       lastModifiedDate = KDateTime::fromString(m_request.cacheTag.lastModified, KDateTime::RFCDate).toTime_t();

    if (lastModifiedDate)
    {
       long diff = static_cast<long>(difftime(dateHeader, lastModifiedDate));
       if (diff < 0)
          expireDate = time(0) + 1;
       else
          expireDate = time(0) + (diff / 10);
    }
    else
    {
       expireDate = time(0) + DEFAULT_CACHE_EXPIRE;
    }
  }

  // DONE receiving the header!
  if (!cookieStr.isEmpty())
  {
    if ((m_request.cookieMode == HTTPRequest::CookiesAuto) && m_request.useCookieJar)
    {
      // Give cookies to the cookiejar.
      QString domain = config()->readEntry("cross-domain");
      if (!domain.isEmpty() && isCrossDomainRequest(m_request.url.host(), domain))
         cookieStr = "Cross-Domain\n" + cookieStr;
      addCookies( m_request.url.url(), cookieStr );
    }
    else if (m_request.cookieMode == HTTPRequest::CookiesManual)
    {
      // Pass cookie to application
      setMetaData("setcookies", cookieStr);
    }
  }

  if (m_request.cacheTag.isExpired)
  {
    m_request.cacheTag.isExpired = false; // Reset just in case.
    if (cacheValidated)
    {
      // Yippie, we can use the cached version.
      // Update the cache with new "Expire" headers.
      gzclose(m_request.cacheTag.gzs);
      m_request.cacheTag.gzs = 0;
      updateExpireDate( expireDate, true );
      m_request.cacheTag.gzs = checkCacheEntry( ); // Re-read cache entry

      if (m_request.cacheTag.gzs)
      {
          m_request.cacheTag.readFromCache = true;
          goto try_again; // Read header again, but now from cache.
       }
       else
       {
          // Where did our cache entry go???
       }
     }
     else
     {
       // Validation failed. Close cache.
       gzclose(m_request.cacheTag.gzs);
       m_request.cacheTag.gzs = 0;
     }
  }

  // We need to reread the header if we got a '100 Continue' or '102 Processing'
  if ( cont )
  {
    kDebug(7113) << "cont; returning to mark try_again";
    goto try_again;
  }

  // Do not do a keep-alive connection if the size of the
  // response is not known and the response is not Chunked.
  if (!m_isChunked && (m_iSize == NO_SIZE)) {
    m_request.isKeepAlive = false;
  }

  if ( m_request.responseCode == 204 )
  {
    return true;
  }

  // We need to try to login again if we failed earlier
  if (m_isUnauthorized) {

      kDebug(7113) << "Trace A" << m_request.responseCode << m_request.proxyUrl.prettyUrl();

      if ((m_request.responseCode == 401) ||
          (m_request.responseCode == 407 && m_request.proxyUrl.isValid())) {

          kDebug(7113) << "Trace B";

          if (getAuthorization()) {
              // for NTLM authentication we have to keep the connection open!
              if (m_auth.scheme == AUTH_NTLM && m_auth.authorization.length() > 4) {
                  m_request.isKeepAlive = true;
                  readBody(true);
              } else if (m_proxyAuth.scheme == AUTH_NTLM && m_proxyAuth.authorization.length() > 4) {
                  readBody(true);
              } else {
                  // discard anything waiting in the read buffers - servers may send
                  // an explanatory HTML page together with the 40x status code.
                  // an alternative, more tricky but with less reconnection overhead, is
                  // readBody(true).
                  httpCloseConnection();
              }
              
              kDebug(7113) << "Trace C";
              
              return false; // Try again.
          }

          if (m_isError) {
              kDebug(7113) << "Trace D";
              return false; // Error out
          }
      }
      // readBody(true);   // discard the body to keep processing in sync
      kDebug(7113) << "Trace E";
      m_isUnauthorized = false;
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
    if ((u.protocol() != "http") && (u.protocol() != "https") &&
       (u.protocol() != "ftp") && (u.protocol() != "webdav") &&
       (u.protocol() != "webdavs"))
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
    if (m_protocol == "webdav" || m_protocol == "webdavs")
      u.setProtocol(m_protocol);

    kDebug(7113) << "Re-directing from" << m_request.url.url()
                 << "to" << u.url();

    redirection(u);
    m_request.cacheTag.writeToCache = false; // Turn off caching on re-direction (DA)
    mayCache = false;
  }

  // Inform the job that we can indeed resume...
  if ( bCanResume && m_request.offset )
    canResume();
  else
    m_request.offset = 0;

  // We don't cache certain text objects
  if (m_mimeType.startsWith("text/") &&
      (m_mimeType != "text/css") &&
      (m_mimeType != "text/x-javascript") &&
      !hasCacheDirective)
  {
     // Do not cache secure pages or pages
     // originating from password protected sites
     // unless the webserver explicitly allows it.
     if (isUsingSsl() || (m_auth.scheme != AUTH_None) )
     {
        m_request.cacheTag.writeToCache = false;
        mayCache = false;
     }
  }

  // WABA: Correct for tgz files with a gzip-encoding.
  // They really shouldn't put gzip in the Content-Encoding field!
  // Web-servers really shouldn't do this: They let Content-Size refer
  // to the size of the tgz file, not to the size of the tar file,
  // while the Content-Type refers to "tar" instead of "tgz".
  if (!m_contentEncodings.isEmpty() && m_contentEncodings.last() == "gzip")
  {
     if (m_mimeType == "application/x-tar")
     {
        m_contentEncodings.removeLast();
        m_mimeType = QString::fromLatin1("application/x-compressed-tar");
     }
     else if (m_mimeType == "application/postscript")
     {
        // LEONB: Adding another exception for psgz files.
        // Could we use the mimelnk files instead of hardcoding all this?
        m_contentEncodings.removeLast();
        m_mimeType = QString::fromLatin1("application/x-gzpostscript");
     }
     else if ( (m_request.allowTransferCompression &&
                m_mimeType == "text/html")
                ||
               (m_request.allowTransferCompression &&
                m_mimeType != "application/x-compressed-tar" &&
                m_mimeType != "application/x-tgz" && // deprecated name
                m_mimeType != "application/x-targz" && // deprecated name
                m_mimeType != "application/x-gzip" &&
                !m_request.url.path().endsWith(QLatin1String(".gz")))
                )
     {
        // Unzip!
     }
     else
     {
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
  if (!m_contentEncodings.isEmpty() && m_contentEncodings.last() == "bzip2")
  {
     m_contentEncodings.removeLast();
     m_mimeType = QString::fromLatin1("application/x-bzip");
  }

  // Correct some common incorrect pseudo-mimetypes
  fixupResponseMimetype();

  if (!m_request.cacheTag.lastModified.isEmpty())
    setMetaData("modified", m_request.cacheTag.lastModified);

  if (!mayCache)
  {
    setMetaData("no-cache", "true");
    setMetaData("expire-date", "1"); // Expired
  }
  else
  {
    QString tmp;
    tmp.setNum(expireDate);
    setMetaData("expire-date", tmp);
    tmp.setNum(time(0)); // Cache entry will be created shortly.
    setMetaData("cache-creation-date", tmp);
  }

  // Let the app know about the mime-type iff this is not
  // a redirection and the mime-type string is not empty.
  if (locationStr.isEmpty() && (!m_mimeType.isEmpty() ||
      m_request.method == HTTP_HEAD))
  {
    kDebug(7113) << "Emitting mimetype " << m_mimeType;
    mimeType( m_mimeType );
  }

  if (config()->readEntry("PropagateHttpHeader", false) ||
      (m_request.cacheTag.useCache) && m_request.cacheTag.writeToCache) {
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

  if (m_request.method == HTTP_HEAD)
     return true;

  // Do we want to cache this request?
  if (m_request.cacheTag.useCache)
  {
    ::unlink(QFile::encodeName(m_request.cacheTag.file));
    if ( m_request.cacheTag.writeToCache && !m_mimeType.isEmpty() )
    {
      kDebug(7113) << "Cache, adding" << m_request.url.url();
      createCacheEntry(m_mimeType, expireDate); // Create a cache entry
      if (!m_request.cacheTag.gzs)
      {
        m_request.cacheTag.writeToCache = false; // Error creating cache entry.
        kDebug(7113) << "Error creating cache entry for " << m_request.url.url()<<"!\n";
      }
      m_request.cacheTag.expireDate = expireDate;
      m_maxCacheSize = config()->readEntry("MaxCacheSize", DEFAULT_MAX_CACHE_SIZE) / 2;
    }
  }

  return true;
}

static void skipLWS(const QString &str, int &pos)
{
    while (pos < str.length() && (str[pos] == ' ' || str[pos] == '\t'))
        ++pos;
}

// Extracts token-like input until terminator char or EOL.. Also skips over the terminator.
// We don't try to be strict or anything..
static QString extractUntil(const QString &str, unsigned char term, int &pos)
{
    QString out;
    skipLWS(str, pos);
    while (pos < str.length() && (str[pos] != term)) {
        out += str[pos];
        ++pos;
    }

    if (pos < str.length()) // Stopped due to finding term
        ++pos;

    // Remove trailing linear whitespace...
    while (out.endsWith(' ') || out.endsWith('\t'))
        out.chop(1);

    return out;
}

// As above, but also handles quotes..
static QString extractMaybeQuotedUntil(const QString &str, unsigned char term, int &pos)
{
    skipLWS(str, pos);

    // Are we quoted?
    if (pos < str.length() && str[pos] == '"') {
        QString out;

        // Skip the quote...
        ++pos;

        // Parse until trailing quote...
        while (pos < str.length()) {
            if (str[pos] == '\\' && pos + 1 < str.length()) {
                // quoted-pair = "\" CHAR
                out += str[pos + 1];
                pos += 2; // Skip both...
            } else if (str[pos] == '"') {
                ++pos;
                break;
            }  else {
                out += str[pos];
                ++pos;
            }
        }

        // Skip until term..
        while (pos < str.length() && (str[pos] != term))
            ++pos;

        if (pos < str.length()) // Stopped due to finding term
            ++pos;

        return out;
    } else {
        return extractUntil(str, term, pos);
    }
}

void HTTPProtocol::parseContentDisposition(const QString &disposition)
{
    kDebug(7113) << "disposition: " << disposition;
    QString strDisposition;
    QString strFilename;

    int pos = 0;

    strDisposition = extractUntil(disposition, ';', pos);

    while (pos < disposition.length()) {
        QString key = extractUntil(disposition, '=', pos);
        QString val = extractMaybeQuotedUntil(disposition, ';', pos);
        if (key == "filename")
            strFilename = val;
    }

    // Content-Dispostion is not allowed to dictate directory
    // path, thus we extract the filename only.
    if ( !strFilename.isEmpty() )
    {
        int pos = strFilename.lastIndexOf( '/' );

        if( pos > -1 )
            strFilename = strFilename.mid(pos+1);

        kDebug(7113) << "Content-Disposition: filename=" << strFilename;
    }
    setMetaData("content-disposition-type", strDisposition);
    if (!strFilename.isEmpty())
        setMetaData("content-disposition-filename", KCodecs::decodeRFC2047String(strFilename));
}

void HTTPProtocol::addEncoding(const QString &_encoding, QStringList &encs)
{
  QString encoding = _encoding.trimmed().toLower();
  // Identity is the same as no encoding
  if (encoding == "identity") {
    return;
  } else if (encoding == "8bit") {
    // Strange encoding returned by http://linac.ikp.physik.tu-darmstadt.de
    return;
  } else if (encoding == "chunked") {
    m_isChunked = true;
    // Anyone know of a better way to handle unknown sizes possibly/ideally with unsigned ints?
    //if ( m_cmd != CMD_COPY )
      m_iSize = NO_SIZE;
  } else if ((encoding == "x-gzip") || (encoding == "gzip")) {
    encs.append(QString::fromLatin1("gzip"));
  } else if ((encoding == "x-bzip2") || (encoding == "bzip2")) {
    encs.append(QString::fromLatin1("bzip2")); // Not yet supported!
  } else if ((encoding == "x-deflate") || (encoding == "deflate")) {
    encs.append(QString::fromLatin1("deflate"));
  } else {
    kDebug(7113) << "Unknown encoding encountered.  "
                 << "Please write code. Encoding =" << encoding;
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

  QString cLength = QString("Content-Length: %1\r\n\r\n").arg(m_POSTbuf.size());
  kDebug( 7113 ) << cLength;

  // Send the content length...
  bool sendOk = (write(cLength.toLatin1(), cLength.length()) == (ssize_t) cLength.length());
  if (!sendOk)
  {
    kDebug( 7113 ) << "Connection broken when sending "
                    << "content length: (" << m_state.hostname << ")";
    error( ERR_CONNECTION_BROKEN, m_state.hostname );
    return false;
  }

  // Send the data...
  // kDebug( 7113 ) << "POST DATA: " << QCString(m_POSTbuf);
  sendOk = (write(m_POSTbuf.data(), m_POSTbuf.size()) == (ssize_t) m_POSTbuf.size());
  if (!sendOk)
  {
    kDebug(7113) << "Connection broken when sending message body: ("
                  << m_state.hostname << ")";
    error( ERR_CONNECTION_BROKEN, m_state.hostname );
    return false;
  }

  return true;
}

void HTTPProtocol::httpClose( bool keepAlive )
{
  kDebug(7113) << "keepAlive =" << keepAlive;

  if (m_request.cacheTag.gzs)
  {
     gzclose(m_request.cacheTag.gzs);
     m_request.cacheTag.gzs = 0;
     if (m_request.cacheTag.writeToCache)
     {
        QString filename = m_request.cacheTag.file + ".new";
        ::unlink( QFile::encodeName(filename) );
     }
  }

  // Only allow persistent connections for GET requests.
  // NOTE: we might even want to narrow this down to non-form
  // based submit requests which will require a meta-data from
  // khtml.
  if (keepAlive && 
      (!m_request.proxyUrl.isValid() || m_request.isPersistentProxyConnection))
  {
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
  disconnectFromHost();
  clearUnreadBuffer();
  setTimeoutSpecialCommand(-1); // Cancel any connection timeout
}

void HTTPProtocol::slave_status()
{
  kDebug(7113);

  if ( !isConnected() )
     httpCloseConnection();

  slaveStatus( m_state.hostname, isConnected() );
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
      qlonglong expireDate;
      stream >> url >> no_cache >> expireDate;
      cacheUpdate( url, no_cache, time_t(expireDate) );
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

  int bytesReceived = readBuffered(m_receiveBuf.data(), bytesToReceive);

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

        if ( m_request.cacheTag.writeToCache )
        {
          createCacheEntry( m_mimeType, m_request.cacheTag.expireDate );
          if (!m_request.cacheTag.gzs)
            m_request.cacheTag.writeToCache = false;
        }

        if ( m_cpMimeBuffer )
        {
          d.resize(0);
          d.resize(m_mimeTypeBuffer.size());
          memcpy( d.data(), m_mimeTypeBuffer.data(),
                  d.size() );
        }
        mimeType(m_mimeType);
        m_mimeTypeBuffer.resize(0);
      }

      data( d );
      if (m_request.cacheTag.writeToCache && m_request.cacheTag.gzs)
         writeCacheEntry(d.data(), d.size());
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
  if (m_request.responseCode == 204)
     return true;

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

  // Update the application with total size except when
  // it is compressed, or when the data is to be handled
  // internally (webDAV).  If compressed we have to wait
  // until we uncompress to find out the actual data size
  if ( !dataInternal ) {
    if ( (m_iSize > 0) && (m_iSize != NO_SIZE)) {
       totalSize(m_iSize);
       infoMessage(i18n("Retrieving %1 from %2...", KIO::convertSize(m_iSize),
                   m_request.url.host()));
    } else {
       totalSize (0);
    }
  } else {
    infoMessage( i18n( "Retrieving from %1..." ,  m_request.url.host() ) );
  }

  if (m_request.cacheTag.readFromCache)
  {
    kDebug(7113) << "read data from cache!";
    m_request.cacheTag.writeToCache = false;

    char buffer[ MAX_IPC_SIZE ];

    m_iContentLeft = NO_SIZE;

    // Jippie! It's already in the cache :-)
    //int zliberrnum;
    while (!gzeof(m_request.cacheTag.gzs)/* && !gzerror(m_request.cacheTag.gzs,&zliberrnum)*/)
    {
      int nbytes = gzread( m_request.cacheTag.gzs, buffer, MAX_IPC_SIZE);

      if (nbytes > 0)
      {
        slotData( QByteArray::fromRawData( buffer, nbytes ) );
        sz += nbytes;
      }
    }

    m_receiveBuf.resize( 0 );

    if ( !dataInternal )
    {
      processedSize( sz );
      data( QByteArray() );
    }

    return true;
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
  struct timeval last_tv;
  gettimeofday( &last_tv, 0L );

  HTTPFilterChain chain;

  QObject::connect(&chain, SIGNAL(output(const QByteArray &)),
          this, SLOT(slotData(const QByteArray &)));
  QObject::connect(&chain, SIGNAL(error(int, const QString &)),
          this, SLOT(error(int, const QString &)));

   // decode all of the transfer encodings
  while (!m_transferEncodings.isEmpty())
  {
    QString enc = m_transferEncodings.takeLast();
    if ( enc == "gzip" )
      chain.addFilter(new HTTPFilterGZip);
    else if ( enc == "deflate" )
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
    if ( enc == "gzip" )
      chain.addFilter(new HTTPFilterGZip);
    else if ( enc == "deflate" )
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
      error(ERR_CONNECTION_BROKEN, m_state.hostname);
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
  if (m_iBytesLeft == 0)
  {
     if (m_request.cacheTag.writeToCache && m_request.cacheTag.gzs)
        closeCacheEntry();
  }

  if (sz <= 1)
  {
    if (m_request.responseCode >= 500 && m_request.responseCode <= 599) {
      error(ERR_INTERNAL_SERVER, m_state.hostname);
      return false;
    } else if (m_request.responseCode >= 400 && m_request.responseCode <= 499) {
      error(ERR_DOES_NOT_EXIST, m_state.hostname);
      return false;
    }
  }

  if (!dataInternal)
    data( QByteArray() );
  return true;
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
   QDBusInterface kcookiejar( "org.kde.kded", "/modules/kcookiejar", "org.kde.KCookieServer" );
   (void)kcookiejar.call( QDBus::NoBlock, "addCookies", url,
                           cookieHeader, windowId );
}

QString HTTPProtocol::findCookies( const QString &url)
{
  qlonglong windowId = m_request.windowId.toLongLong();
  QDBusInterface kcookiejar( "org.kde.kded", "/modules/kcookiejar", "org.kde.KCookieServer" );
  QDBusReply<QString> reply = kcookiejar.call( "findCookies", url, windowId );

  if ( !reply.isValid() )
  {
     kWarning(7113) << "Can't communicate with kded_kcookiejar!";
     return QString();
  }
  return reply;
}

/******************************* CACHING CODE ****************************/


void HTTPProtocol::cacheUpdate( const KUrl& url, bool no_cache, time_t expireDate)
{
  if (!maybeSetRequestUrl(url))
      return;

  // Make sure we read in the cache info.
  resetSessionSettings();

  m_request.cacheTag.policy= CC_Reload;

  if (no_cache)
  {
     m_request.cacheTag.gzs = checkCacheEntry( );
     if (m_request.cacheTag.gzs)
     {
       gzclose(m_request.cacheTag.gzs);
       m_request.cacheTag.gzs = 0;
       ::unlink( QFile::encodeName(m_request.cacheTag.file) );
     }
  }
  else
  {
     updateExpireDate( expireDate );
  }
  finished();
}

// !START SYNC!
// The following code should be kept in sync
// with the code in http_cache_cleaner.cpp

gzFile HTTPProtocol::checkCacheEntry( bool readWrite)
{
   const QChar separator = '_';

   QString CEF = m_request.url.path();

   int p = CEF.indexOf('/');

   while(p != -1)
   {
      CEF[p] = separator;
      p = CEF.indexOf('/', p);
   }

   QString host = m_request.url.host().toLower();
   CEF = host + CEF + '_';

   QString dir = m_strCacheDir;
   if (dir[dir.length()-1] != '/')
      dir += '/';

   int l = host.length();
   for(int i = 0; i < l; i++)
   {
      if (host[i].isLetter() && (host[i] != 'w'))
      {
         dir += host[i];
         break;
      }
   }
   if (dir[dir.length()-1] == '/')
      dir += '0';

   unsigned long hash = 0x00000000;
   QByteArray u = m_request.url.url().toLatin1();
   for(int i = u.length(); i--;)
   {
      hash = (hash * 12211 + u.at(i)) % 2147483563;
   }

   QString hashString;
   hashString.sprintf("%08lx", hash);

   CEF = CEF + hashString;

   CEF = dir + '/' + CEF;

   m_request.cacheTag.file = CEF;

   const char *mode = (readWrite ? "r+b" : "rb");

   gzFile fs = gzopen( QFile::encodeName(CEF), mode); // Open for reading and writing
   if (!fs)
      return 0;

   char buffer[401];
   bool ok = true;

  // CacheRevision
  if (ok && (!gzgets(fs, buffer, 400)))
      ok = false;
   if (ok && (strcmp(buffer, CACHE_REVISION) != 0))
      ok = false;

   time_t date;
   time_t currentDate = time(0);

   // URL
   if (ok && (!gzgets(fs, buffer, 400)))
      ok = false;
   if (ok)
   {
      int l = strlen(buffer);
      if (l>0)
         buffer[l-1] = 0; // Strip newline
      if (m_request.url.url() != buffer)
      {
         ok = false; // Hash collision
      }
   }

   // Creation Date
   if (ok && (!gzgets(fs, buffer, 400)))
      ok = false;
   if (ok)
   {
      date = (time_t) strtoul(buffer, 0, 10);
      m_request.cacheTag.creationDate = date;
      if (m_maxCacheAge && (difftime(currentDate, date) > m_maxCacheAge))
      {
         m_request.cacheTag.isExpired = true;
         m_request.cacheTag.expireDate = currentDate;
      }
   }

   // Expiration Date
   m_request.cacheTag.expireDateOffset = gztell(fs);
   if (ok && (!gzgets(fs, buffer, 400)))
      ok = false;
   if (ok)
   {
      if (m_request.cacheTag.policy== CC_Verify)
      {
         date = (time_t) strtoul(buffer, 0, 10);
         // After the expire date we need to revalidate.
         if (!date || difftime(currentDate, date) >= 0)
            m_request.cacheTag.isExpired = true;
         m_request.cacheTag.expireDate = date;
      }
      else if (m_request.cacheTag.policy== CC_Refresh)
      {
         m_request.cacheTag.isExpired = true;
         m_request.cacheTag.expireDate = currentDate;
      }
   }

   // ETag
   if (ok && (!gzgets(fs, buffer, 400)))
      ok = false;
   if (ok)
   {
      m_request.cacheTag.etag = QString(buffer).trimmed();
   }

   // Last-Modified
   if (ok && (!gzgets(fs, buffer, 400)))
      ok = false;
   if (ok)
   {
      m_request.cacheTag.bytesCached=0;
      m_request.cacheTag.lastModified = QString(buffer).trimmed();
//    }

//    if (ok)
//    {

      //write hit frequency data
      int freq=0;
      FILE* hitdata = fopen( QFile::encodeName(CEF+"_freq"), "r+");
         if (hitdata)
         {
             freq=fgetc(hitdata);
             if (freq!=EOF)
                freq+=fgetc(hitdata)<<8;
             else
                freq=0;
            KDE_fseek(hitdata,0,SEEK_SET);
         }
         if (hitdata||(hitdata=fopen(QFile::encodeName(CEF+"_freq"), "w")))
         {
             fputc(++freq,hitdata);
             fputc(freq>>8,hitdata);
             fclose(hitdata);
         }

      return fs;
   }

   gzclose(fs);
   unlink( QFile::encodeName(CEF));
   return 0;
}

void HTTPProtocol::updateExpireDate(time_t expireDate, bool updateCreationDate)
{
    bool ok = true;

    gzFile fs = checkCacheEntry(true);
    if (fs)
    {
        QString date;
        char buffer[401];
        time_t creationDate;

        gzseek(fs, 0, SEEK_SET);
        if (ok && !gzgets(fs, buffer, 400))
            ok = false;
        if (ok && !gzgets(fs, buffer, 400))
            ok = false;
        long cacheCreationDateOffset = gztell(fs);
        if (ok && !gzgets(fs, buffer, 400))
            ok = false;
        creationDate = strtoul(buffer, 0, 10);
        if (!creationDate)
            ok = false;

        if (updateCreationDate)
        {
           if (!ok || gzseek(fs, cacheCreationDateOffset, SEEK_SET))
              return;
           QString date;
           date.setNum( time(0) );
           date = date.leftJustified(16);
           gzputs(fs, date.toLatin1());      // Creation date
           gzputc(fs, '\n');
        }

        if (expireDate > (30 * 365 * 24 * 60 * 60))
        {
            // expire date is a really a big number, it can't be
            // a relative date.
            date.setNum( expireDate );
        }
        else
        {
            // expireDate before 2000. those values must be
            // interpreted as relative expiration dates from
            // <META http-equiv="Expires"> tags.
            // so we have to scan the creation time and add
            // it to the expiryDate
            date.setNum( creationDate + expireDate );
        }
        date = date.leftJustified(16);
        if (!ok || gzseek(fs, m_request.cacheTag.expireDateOffset, SEEK_SET))
            return;
        gzputs(fs, date.toLatin1());      // Expire date
        gzseek(fs, 0, SEEK_END);
        gzclose(fs);
    }
}

void HTTPProtocol::createCacheEntry( const QString &mimetype, time_t expireDate)
{
   QString dir = m_request.cacheTag.file;
   int p = dir.lastIndexOf('/');
   if (p == -1) return; // Error.
   dir.truncate(p);

   // Create file
   KDE_mkdir( QFile::encodeName(dir), 0700 );

   QString filename = m_request.cacheTag.file + ".new";  // Create a new cache entryexpireDate

//   kDebug( 7103 ) <<  "creating new cache entry: " << filename;

   m_request.cacheTag.gzs = gzopen( QFile::encodeName(filename), "wb");
   if (!m_request.cacheTag.gzs)
   {
      kWarning(7113) << "opening" << filename << "failed.";
      return; // Error.
   }

   gzputs(m_request.cacheTag.gzs, CACHE_REVISION);    // Revision

   gzputs(m_request.cacheTag.gzs, m_request.url.url().toLatin1());  // Url
   gzputc(m_request.cacheTag.gzs, '\n');

   QString date;
   m_request.cacheTag.creationDate = time(0);
   date.setNum( m_request.cacheTag.creationDate );
   date = date.leftJustified(16);
   gzputs(m_request.cacheTag.gzs, date.toLatin1());      // Creation date
   gzputc(m_request.cacheTag.gzs, '\n');

   date.setNum( expireDate );
   date = date.leftJustified(16);
   gzputs(m_request.cacheTag.gzs, date.toLatin1());      // Expire date
   gzputc(m_request.cacheTag.gzs, '\n');

   if (!m_request.cacheTag.etag.isEmpty())
      gzputs(m_request.cacheTag.gzs, m_request.cacheTag.etag.toLatin1());    //ETag
   gzputc(m_request.cacheTag.gzs, '\n');

   if (!m_request.cacheTag.lastModified.isEmpty())
      gzputs(m_request.cacheTag.gzs, m_request.cacheTag.lastModified.toLatin1());    // Last modified
   gzputc(m_request.cacheTag.gzs, '\n');

   gzputs(m_request.cacheTag.gzs, mimetype.toLatin1());  // Mimetype
   gzputc(m_request.cacheTag.gzs, '\n');

   gzputs(m_request.cacheTag.gzs, m_responseHeaders.join("\n").toLatin1());
   gzputc(m_request.cacheTag.gzs, '\n');

   gzputc(m_request.cacheTag.gzs, '\n');

   return;
}
// The above code should be kept in sync
// with the code in http_cache_cleaner.cpp
// !END SYNC!

void HTTPProtocol::writeCacheEntry( const char *buffer, int nbytes)
{
   // gzwrite's second argument has type void *const in 1.1.4 and
   // const void * in 1.2.3, so we futz buffer to a plain void * and
   // let the compiler figure it out from there.
   if (gzwrite(m_request.cacheTag.gzs, const_cast<void *>(static_cast<const void *>(buffer)), nbytes) == 0)
   {
      kWarning(7113) << "writeCacheEntry: writing " << nbytes << " bytes failed.";
      gzclose(m_request.cacheTag.gzs);
      m_request.cacheTag.gzs = 0;
      QString filename = m_request.cacheTag.file + ".new";
      ::unlink( QFile::encodeName(filename) );
      return;
   }
   m_request.cacheTag.bytesCached+=nbytes;
   if ( m_request.cacheTag.bytesCached>>10 > m_maxCacheSize )
   {
      kDebug(7113) << "writeCacheEntry: File size reaches " << (m_request.cacheTag.bytesCached>>10)
                    << "Kb, exceeds cache limits. (" << m_maxCacheSize << "Kb)";
      gzclose(m_request.cacheTag.gzs);
      m_request.cacheTag.gzs = 0;
      QString filename = m_request.cacheTag.file + ".new";
      ::unlink( QFile::encodeName(filename) );
      return;
   }
}

void HTTPProtocol::closeCacheEntry()
{
   QString filename = m_request.cacheTag.file + ".new";
   int result = gzclose( m_request.cacheTag.gzs);
   m_request.cacheTag.gzs = 0;
   if (result == 0)
   {
#ifdef Q_OS_WIN
      if ( MoveFileExW( (LPCWSTR)filename.utf16(),
                        (LPCWSTR)m_request.cacheTag.file.utf16(),
                        MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED ) != 0 )
        return;
#else
      if (KDE_rename( QFile::encodeName(filename), QFile::encodeName(m_request.cacheTag.file)) == 0)
         return; // Success
#endif
      kWarning(7113) << "closeCacheEntry: error renaming "
                      << "cache entry. (" << filename << " -> " << m_request.cacheTag.file
                      << ")";
   }

   kWarning(7113) << "closeCacheEntry: error closing cache "
                   << "entry. (" << filename<< ")";
}

void HTTPProtocol::cleanCache()
{
   const time_t maxAge = DEFAULT_CLEAN_CACHE_INTERVAL; // 30 Minutes.
   bool doClean = false;
   QString cleanFile = m_strCacheDir;
   if (cleanFile[cleanFile.length()-1] != '/')
      cleanFile += '/';
   cleanFile += "cleaned";

   KDE_struct_stat stat_buf;

   int result = KDE_stat(QFile::encodeName(cleanFile), &stat_buf);
   if (result == -1)
   {
      int fd = creat( QFile::encodeName(cleanFile), 0600);
      if (fd != -1)
      {
         doClean = true;
         ::close(fd);
      }
   }
   else
   {
      time_t age = (time_t) difftime( time(0), stat_buf.st_mtime );
      if (age > maxAge) //
        doClean = true;
   }
   if (doClean)
   {
      // Touch file.
      utime(QFile::encodeName(cleanFile), 0);
      KToolInvocation::startServiceByDesktopPath("http_cache_cleaner.desktop");
   }
}



//**************************  AUTHENTICATION CODE ********************/


void HTTPProtocol::configAuth(const QList<QByteArray> &l, bool isProxyAuth)
{
    AUTH_SCHEME f = AUTH_None;
    QString strAuth;
    QByteArray realm;

    QByteArray scheme = l.first().toLower();

    if (scheme.startsWith("basic") || scheme.startsWith("MBS_PWD_COOKIE")) {
        // MBS_PWD_COOKIE found on http://www.webscription.net/baen/default.asp
        f = AUTH_Basic;
        strAuth = "Basic";
    } else if (scheme.startsWith("digest")) {
        f = AUTH_Digest;
        strAuth = "Digest";  // Correct for upper-case variations in input

#ifdef HAVE_LIBGSSAPI
    } else if (scheme.startsWith("negotiate")) {
        // if we get two 401 in a row let's assume for now that
        // Negotiate isn't working and ignore it
        if (!isProxyAuth && !(m_request.responseCode == 401 && m_request.prevResponseCode == 401))  {
            f = AUTH_Negotiate;
            strAuth = "Negotiate";
        }
#endif

    } else if (scheme.startsWith("ntlm") && (!isProxyAuth || m_request.isPersistentProxyConnection)) {
        f = AUTH_NTLM;
        strAuth = "NTLM";
        realm = "NTLM"; // set a dummy realm
    } else  {
        kWarning(7113) << "Unsupported or invalid authorization type requested";

        if (isProxyAuth) {
            kWarning(7113) << "Proxy URL: " << m_request.proxyUrl;
        } else {
            kWarning(7113) << "URL: " << m_request.url;
        }
        kWarning(7113) << "Request Authorization: " << l.first();
    }

    /*
     This check ensures the following:
     1.) Rejection of any unknown/unsupported authentication schemes
     2.) Usage of the strongest possible authentication schemes if
         and when multiple Proxy-Authenticate or WWW-Authenticate
         header field is sent.
    */
    if (isProxyAuth) {
        if (f == AUTH_None || (m_proxyAuth.authCount > 0 && f < m_proxyAuth.scheme)) {
            // I purposefully made the Proxy-Authentication settings
            // persistent to reduce the number of round-trips to kdesud we
            // have to take special care when an unknown/unsupported auth-
            // scheme is received. This check accomplishes just that...
            if (m_proxyAuth.authCount == 0) {
                m_proxyAuth.scheme = f;
            }
            kDebug(7113) << "Rejected proxy auth method: " << f;
            return;
        }
        m_proxyAuth.authCount++;
        kDebug(7113) << "Accepted proxy auth method: " << f;
    } else {
        if (f == AUTH_None || (m_auth.authCount > 0 && f < m_auth.scheme)) {
            kDebug(7113) << "Rejected auth method: " << f;
            return;
        }
        m_auth.authCount++;
        kDebug(7113) << "Accepted auth method: " << f;
    }

    if (realm.isEmpty()) {
        // take the first "realm=", be it before or after a comma
        int realmIdx = scheme.indexOf("realm=");
        if (realmIdx != -1) {
            // we want the original letter cases here
            realm = l[0].mid(realmIdx + strlen("realm="));
        } else if ((realmIdx = l[1].toLower().indexOf("realm=")) != -1) {
            realm = l[1].mid(realmIdx + strlen("realm="));
        }
    }
    //strip quotes
    if (realm.length() && (realm[0] == '"') && (realm[realm.length() - 1] == '"')) {
        realm = realm.mid(1, realm.length() - 2);
    }
    QString realmStr = QString::fromLatin1(realm);
    if (KGlobal::locale()->language().contains("ru")) {
        //for sites like lib.homelinux.org
        realmStr = QTextCodec::codecForName("CP1251")->toUnicode(realm);
    }

    //recreate the original header line...
    QString authorizationStr;
    foreach (const QByteArray &ba, l) {
        authorizationStr += QString::fromLatin1(ba);
        authorizationStr += ", ";
    }
    authorizationStr.chop(2);

    if (isProxyAuth) {
        m_proxyAuth.scheme = f;
        m_proxyAuth.authorization = authorizationStr;
        m_proxyAuth.realm = realmStr;
    } else {
        m_auth.scheme = f;
        m_auth.authorization = authorizationStr;
        m_auth.realm = realmStr;
    }
}


bool HTTPProtocol::retryPrompt()
{
    QString prompt;
    if (m_request.responseCode == 401) {
        prompt = i18n("Authentication Failed.");
    } else {
        prompt = i18n("Proxy Authentication Failed.");
    }
    prompt += i18n(" Do you want to retry?");
    return messageBox(QuestionYesNo, prompt, i18n("Authentication")) == 3;
}

void HTTPProtocol::fillPromptInfo(AuthInfo *inf)
{
  AuthInfo &info = *inf;    //no use rewriting everything below
  if ( m_request.responseCode == 401 )
  {
    info.url = m_request.url;
    if ( !m_state.user.isEmpty() )
      info.username = m_state.user;
    info.readOnly = !m_request.url.user().isEmpty();
    info.prompt = i18n( "You need to supply a username and a "
                        "password to access this site." );
    info.keepPassword = true; // Prompt the user for persistence as well.
    if ( !m_auth.realm.isEmpty() )
    {
      info.realmValue = m_auth.realm;
      info.verifyPath = false;
      info.digestInfo = m_auth.authorization;
      info.commentLabel = i18n("Site:");
      info.comment = i18n("<b>%1</b> at <b>%2</b>", m_auth.realm , m_request.url.host());
    }
  }
  else if ( m_request.responseCode == 407 )
  {
    info.url = m_request.proxyUrl;
    info.username = m_request.proxyUrl.user();
    info.prompt = i18n( "You need to supply a username and a password for "
                        "the proxy server listed below before you are allowed "
                        "to access any sites." );
    info.keepPassword = true;
    if (!m_proxyAuth.realm.isEmpty())
    {
      info.realmValue = m_proxyAuth.realm;
      info.verifyPath = false;
      info.digestInfo = m_proxyAuth.authorization;
      info.commentLabel = i18n("Proxy:");
      info.comment = i18n("<b>%1</b> at <b>%2</b>", m_proxyAuth.realm, m_request.proxyUrl.host());
    }
  }
}

//Helper for (ATM only) HTTProtocol::getAuthorization().
static int valueForKey(const QString &haystack, const QString &key, int from = 0)
{
    int pos = haystack.indexOf(key, from, Qt::CaseInsensitive);
    if (pos == -1) {
        return -1;
    }
    pos += key.length();
    const int len = haystack.length();
    while (pos < len && (haystack[pos] == ' ' || haystack[pos] == '=')) {
        pos++;
    }
    if (pos >= len - 1) {
        return -1;
    }
    return pos;
}

bool HTTPProtocol::getAuthorization()
{
    AuthInfo info;
    bool result = false;

    kDebug(7113)  << "Current Response: " << m_request.responseCode << ", "
                  << "Previous Response: " << m_request.prevResponseCode << ", "
                  << "Authentication: " << m_auth.scheme << ", "
                  << "ProxyAuthentication: " << m_proxyAuth.scheme;

    if (m_request.doNotAuthenticate) {
        if (m_request.preferErrorPage) {
            errorPage();
        } else {
            error(ERR_COULD_NOT_LOGIN, i18n("Authentication needed for %1 but "
                                            "authentication is disabled.",
                                            m_request.url.host()));
        }
        return false;
    }

    if (m_request.responseCode != 401 && m_request.responseCode != 407) {
        return false;
    }

    KUrl &authUrl = (m_request.responseCode == 401) ? m_request.url : m_request.proxyUrl;
    AuthState &authState = (m_request.responseCode == 401) ? m_auth : m_proxyAuth;

    const bool repeatFailure = (m_request.prevResponseCode == m_request.responseCode);

    if (repeatFailure) {
        bool prompt = true;
        if (m_auth.scheme == AUTH_Digest || m_proxyAuth.scheme == AUTH_Digest) {
            bool isStaleNonce = false;
            const QString auth = authState.authorization;

            int pos = valueForKey(auth, "stale");
            if (pos != -1 && auth.indexOf("true", pos, Qt::CaseInsensitive) != -1) {
                isStaleNonce = true;
                kDebug(7113) << "Stale nonce value. Will retry using same info...";
            }

            if (isStaleNonce) {
                prompt = false;
                result = true;

                info.username = authUrl.user();
                info.password = authUrl.pass();
                info.realmValue = authState.realm;
                info.digestInfo = authState.authorization;
            }
        }

        if (m_auth.scheme == AUTH_NTLM || m_proxyAuth.scheme == AUTH_NTLM) {
            const QString auth = authState.authorization;

            kDebug(7113) << "auth: " << auth;
            if (auth.length() > 4) {
                prompt = false;
                result = true;
                kDebug(7113) << "NTLM auth second phase, "
                             << "sending response...";

                info.username = authUrl.user();
                info.password = authUrl.pass();
                info.realmValue = authState.realm;
                info.digestInfo = authState.authorization;
            }
        }
    } else {
        // !repeatFailure case
    
        // At this point we know more details, so use it to find
        // out if we have a cached version and avoid a re-prompt!
        // We also do not use verify path unlike the pre-emptive
        // requests because we already know the realm value...
        //### that comment sounds like it's exactly wrong(?) --ahartmetz

        if (m_proxyAuth.scheme != AUTH_None) {  //### we actually need some "stale flag".
            // Reset cached proxy auth
            m_proxyAuth.scheme = AUTH_None;
            KUrl proxy(config()->readEntry("UseProxy"));
            m_request.proxyUrl.setUser(proxy.user());
            m_request.proxyUrl.setPass(proxy.pass());
        }

        info.verifyPath = false;
        info.url = authUrl;
        info.username = authUrl.user();
        info.password = authUrl.pass();
        info.realmValue = authState.realm;
        info.digestInfo = authState.authorization;

        // If either username or password is not supplied
        // with the request, check the password cache.
        if (info.username.isEmpty() || info.password.isEmpty()) {
            result = checkCachedAuthentication(info);
        }

        if (m_auth.scheme == AUTH_Digest) {
            const QString auth = authState.authorization;

            int pos = valueForKey(auth, "stale");
            if (pos != -1 && auth.indexOf("true", pos, Qt::CaseInsensitive) != -1) {
                info.digestInfo = auth;
                kDebug(7113) << "Just a stale nonce value! Retrying "
                                "using the new nonce sent...";
            }
        }
    }

    if (!result) {
        // Do not prompt if the username & password are already supplied and
        // the login attempt did not fail before.
        if (!repeatFailure &&
            !info.username.isEmpty() && !info.password.isEmpty()) {
            result = true;
        } else {
            if (m_auth.scheme == AUTH_Negotiate) {
                if (!repeatFailure) {
                    result = true;
                }
            } else if (!m_request.disablePassDialog)  {
                kDebug(7113) << "Prompting the user for authorization...";
                fillPromptInfo(&info);
                QString msg = (m_request.responseCode == 401) ? 
                                  i18n("Authentication Failed.") :
                                  i18n("Proxy Authentication Failed.");
                result = openPasswordDialog(info, msg);
            }
        }
    }

    if (result) {
        authUrl.setUser(info.username);
        authUrl.setPass(info.password);
        authState.realm = info.realmValue;
        authState.authorization = info.digestInfo;
    } else {
        if (m_request.preferErrorPage) {
            errorPage();
        } else {
            error(ERR_USER_CANCELED, QString());
        }
    }

    return result;
}

void HTTPProtocol::saveAuthorization(bool isForProxy)
{
    if (m_request.prevResponseCode == 407 && !m_request.proxyUrl.isValid()) {
        return;
    }
    AuthInfo info;
    KUrl &authUrl = isForProxy ? m_request.proxyUrl : m_request.url;
    AuthState &authState = isForProxy ? m_proxyAuth : m_auth;

    info.url = authUrl;   //### strip username / password?
    info.username = authUrl.user();
    info.password = authUrl.pass();
    info.realmValue = authState.realm;
    info.digestInfo = authState.authorization;

    cacheAuthentication(info);
}

#ifdef HAVE_LIBGSSAPI
QByteArray HTTPProtocol::gssError( int major_status, int minor_status )
{
  OM_uint32 new_status;
  OM_uint32 msg_ctx = 0;
  gss_buffer_desc major_string;
  gss_buffer_desc minor_string;
  OM_uint32 ret;
  QByteArray errorstr;

  errorstr = "";

  do {
    ret = gss_display_status(&new_status, major_status, GSS_C_GSS_CODE, GSS_C_NULL_OID, &msg_ctx, &major_string);
    errorstr += (const char *)major_string.value;
    errorstr += ' ';
    ret = gss_display_status(&new_status, minor_status, GSS_C_MECH_CODE, GSS_C_NULL_OID, &msg_ctx, &minor_string);
    errorstr += (const char *)minor_string.value;
    errorstr += ' ';
  } while (!GSS_ERROR(ret) && msg_ctx != 0);

  return errorstr;
}

QString HTTPProtocol::createNegotiateAuth()
{
  QString auth;
  QByteArray servicename;
  OM_uint32 major_status, minor_status;
  OM_uint32 req_flags = 0;
  gss_buffer_desc input_token = GSS_C_EMPTY_BUFFER;
  gss_buffer_desc output_token = GSS_C_EMPTY_BUFFER;
  gss_name_t server;
  gss_ctx_id_t ctx;
  gss_OID mech_oid;
  static gss_OID_desc krb5_oid_desc = {9, (void *) "\x2a\x86\x48\x86\xf7\x12\x01\x02\x02"};
  static gss_OID_desc spnego_oid_desc = {6, (void *) "\x2b\x06\x01\x05\x05\x02"};
  int found = 0;
  unsigned int i;
  gss_OID_set mech_set;
  gss_OID tmp_oid;

  ctx = GSS_C_NO_CONTEXT;
  mech_oid = &krb5_oid_desc;

  // see whether we can use the SPNEGO mechanism
  major_status = gss_indicate_mechs(&minor_status, &mech_set);
  if (GSS_ERROR(major_status)) {
    kDebug(7113) << "gss_indicate_mechs failed: " << gssError(major_status, minor_status);
  } else {
    for (i=0; i<mech_set->count && !found; i++) {
      tmp_oid = &mech_set->elements[i];
      if (tmp_oid->length == spnego_oid_desc.length &&
        !memcmp(tmp_oid->elements, spnego_oid_desc.elements, tmp_oid->length)) {
        kDebug(7113) << "found SPNEGO mech";
        found = 1;
        mech_oid = &spnego_oid_desc;
        break;
      }
    }
    gss_release_oid_set(&minor_status, &mech_set);
  }

  // the service name is "HTTP/f.q.d.n"
  servicename = "HTTP@";
  servicename += m_state.hostname.toAscii();

  input_token.value = (void *)servicename.data();
  input_token.length = servicename.length() + 1;

  major_status = gss_import_name(&minor_status, &input_token,
                                 GSS_C_NT_HOSTBASED_SERVICE, &server);

  input_token.value = NULL;
  input_token.length = 0;

  if (GSS_ERROR(major_status)) {
    kDebug(7113) << "gss_import_name failed: " << gssError(major_status, minor_status);
    // reset the auth string so that subsequent methods aren't confused
    m_auth.authorization.clear();
    return QString();
  }

  major_status = gss_init_sec_context(&minor_status, GSS_C_NO_CREDENTIAL,
                                      &ctx, server, mech_oid,
                                      req_flags, GSS_C_INDEFINITE,
                                      GSS_C_NO_CHANNEL_BINDINGS,
                                      GSS_C_NO_BUFFER, NULL, &output_token,
                                      NULL, NULL);


  if (GSS_ERROR(major_status) || (output_token.length == 0)) {
    kDebug(7113) << "gss_init_sec_context failed: " << gssError(major_status, minor_status);
    gss_release_name(&minor_status, &server);
    if (ctx != GSS_C_NO_CONTEXT) {
      gss_delete_sec_context(&minor_status, &ctx, GSS_C_NO_BUFFER);
      ctx = GSS_C_NO_CONTEXT;
    }
    // reset the auth string so that subsequent methods aren't confused
    m_auth.authorization.clear();
    return QString();
  }

  auth = "Authorization: Negotiate ";
  auth += QByteArray::fromRawData((const char *)output_token.value, output_token.length).toBase64();
  auth += "\r\n";

  // free everything
  gss_release_name(&minor_status, &server);
  if (ctx != GSS_C_NO_CONTEXT) {
    gss_delete_sec_context(&minor_status, &ctx, GSS_C_NO_BUFFER);
    ctx = GSS_C_NO_CONTEXT;
  }
  gss_release_buffer(&minor_status, &output_token);

  return auth;
}
#else

// Dummy
QByteArray HTTPProtocol::gssError( int, int )
{
  return QByteArray();
}

// Dummy
QString HTTPProtocol::createNegotiateAuth()
{
  return QString();
}
#endif

QString HTTPProtocol::createNTLMAuth( bool isForProxy )
{
  uint len;
  QString auth, user, domain, passwd;
  QByteArray strauth;
  QByteArray buf;

  if ( isForProxy )
  {
    auth = "Proxy-Authorization: NTLM ";
    user = m_request.proxyUrl.user();
    passwd = m_request.proxyUrl.pass();
    strauth = m_proxyAuth.authorization.toLatin1();
    len = m_proxyAuth.authorization.length();
  }
  else
  {
    auth = "Authorization: NTLM ";
    user = m_state.user;
    passwd = m_state.passwd;
    strauth = m_auth.authorization.toLatin1();
    len = m_auth.authorization.length();
  }
  if ( user.contains('\\') ) {
    domain = user.section( '\\', 0, 0);
    user = user.section( '\\', 1 );
  }

  kDebug(7113) << "NTLM length: " << len;
  if ( user.isEmpty() || passwd.isEmpty() || len < 4 )
    return QString();

  if ( len > 4 )
  {
    // create a response
    QByteArray challenge;
    KCodecs::base64Decode( strauth.right( len - 5 ), challenge );
    KNTLM::getAuth( buf, challenge, user, passwd, domain,
		    QHostInfo::localHostName() );
  }
  else
  {
    KNTLM::getNegotiate( buf );
  }

  // remove the challenge to prevent reuse
  if ( isForProxy )
    m_proxyAuth.authorization = "NTLM";
  else
    m_auth.authorization = "NTLM";

  auth += KCodecs::base64Encode( buf );
  auth += "\r\n";

  return auth;
}

QString HTTPProtocol::createBasicAuth( bool isForProxy )
{
  QString auth;
  QByteArray user, passwd;
  if ( isForProxy )
  {
    auth = "Proxy-Authorization: Basic ";
    user = m_request.proxyUrl.user().toLatin1();
    passwd = m_request.proxyUrl.pass().toLatin1();
  }
  else
  {
    auth = "Authorization: Basic ";
    user = m_state.user.toLatin1();
    passwd = m_state.passwd.toLatin1();
  }

  if (user.isEmpty() || passwd.isEmpty()) {
    return QString();    
  }

  return auth + KCodecs::base64Encode(user + ':' + passwd) + "\r\n";
}

void HTTPProtocol::calculateResponse( DigestAuthInfo& info, QByteArray& Response )
{
  kDebug(7113);
  KMD5 md;
  QByteArray HA1;
  QByteArray HA2;

  // Calculate H(A1)
  QByteArray authStr = info.username;
  authStr += ':';
  authStr += info.realm;
  authStr += ':';
  authStr += info.password;
  md.update( authStr );

  if ( info.algorithm.toLower() == "md5-sess" )
  {
    authStr = md.hexDigest();
    authStr += ':';
    authStr += info.nonce;
    authStr += ':';
    authStr += info.cnonce;
    md.reset();
    md.update( authStr );
  }
  HA1 = md.hexDigest();

  kDebug(7113) << "A1 => " << HA1;

  // Calcualte H(A2)
  authStr = info.method;
  authStr += ':';
  authStr += m_request.url.encodedPathAndQuery(KUrl::LeaveTrailingSlash,KUrl::AvoidEmptyPath).toLatin1();
  if ( info.qop == "auth-int" )
  {
    authStr += ':';
    authStr += info.entityBody;
  }
  md.reset();
  md.update( authStr );
  HA2 = md.hexDigest();

  kDebug(7113) << "A2 => " << HA2;

  // Calcualte the response.
  authStr = HA1;
  authStr += ':';
  authStr += info.nonce;
  authStr += ':';
  if ( !info.qop.isEmpty() )
  {
    authStr += info.nc;
    authStr += ':';
    authStr += info.cnonce;
    authStr += ':';
    authStr += info.qop;
    authStr += ':';
  }
  authStr += HA2;
  md.reset();
  md.update( authStr );
  Response = md.hexDigest();

  kDebug(7113) << "Response => " << Response;
}

QString HTTPProtocol::createDigestAuth( bool isForProxy )
{
  const char *p;

  QString auth;
  QByteArray opaque;
  QByteArray Response;

  DigestAuthInfo info;

  opaque = "";
  if ( isForProxy )
  {
    auth = "Proxy-Authorization: Digest ";
    info.username = m_request.proxyUrl.user().toLatin1();
    info.password = m_request.proxyUrl.pass().toLatin1();
    p = m_proxyAuth.authorization.toLatin1();
  }
  else
  {
    auth = "Authorization: Digest ";
    info.username = m_state.user.toLatin1();
    info.password = m_state.passwd.toLatin1();
    p = m_auth.authorization.toLatin1();
  }
  if (!p || !*p) {
    return QString();
  }
  p += 6; // Skip "Digest"

  if ( info.username.isEmpty() || info.password.isEmpty() || !p ) {
    return QString();
  }

  // info.entityBody = p;  // FIXME: send digest of data for POST action ??
  info.realm = "";
  info.algorithm = "MD5";
  info.nonce = "";
  info.qop = "";

  // cnonce is recommended to contain about 64 bits of entropy
  info.cnonce = KRandom::randomString(16).toLatin1();

  // HACK: Should be fixed according to RFC 2617 section 3.2.2
  info.nc = "00000001";

  // Set the method used...
  info.method = methodString(m_request.method).toLatin1();
  info.method.chop(1);  // trim space
  if (info.method.isEmpty()) {
      error(ERR_UNSUPPORTED_ACTION, i18n("Unsupported method: authentication will fail. "
                                         "Please submit a bug report."));
  }

  // Parse the Digest response....
  while (*p)
  {
    int i = 0;
    while ( (*p == ' ') || (*p == ',') || (*p == '\t')) { p++; }
    if (strncasecmp(p, "realm=", 6 )==0)
    {
      p+=6;
      while ( *p == '"' ) p++;  // Go past any number of " mark(s) first
      while ( p[i] != '"' ) i++;  // Read everything until the last " mark
      info.realm = QByteArray( p, i );
    }
    else if (strncasecmp(p, "algorith=", 9)==0)
    {
      p+=9;
      while ( *p == '"' ) p++;  // Go past any number of " mark(s) first
      while ( ( p[i] != '"' ) && ( p[i] != ',' ) && ( p[i] != '\0' ) ) i++;
      info.algorithm = QByteArray(p, i);
    }
    else if (strncasecmp(p, "algorithm=", 10)==0)
    {
      p+=10;
      while ( *p == '"' ) p++;  // Go past any " mark(s) first
      while ( ( p[i] != '"' ) && ( p[i] != ',' ) && ( p[i] != '\0' ) ) i++;
      info.algorithm = QByteArray(p,i);
    }
    else if (strncasecmp(p, "domain=", 7)==0)
    {
      p+=7;
      while ( *p == '"' ) p++;  // Go past any " mark(s) first
      while ( p[i] != '"' ) i++;  // Read everything until the last " mark
      int pos;
      int idx = 0;
      QByteArray uri(p, i);
      do
      {
        pos = uri.indexOf( ' ', idx );
        if ( pos != -1 )
        {
          KUrl u (m_request.url, uri.mid(idx, pos-idx));
          if (u.isValid ())
            info.digestURIs.append( u );
        }
        else
        {
          KUrl u (m_request.url, uri.mid(idx, uri.length()-idx));
          if (u.isValid ())
            info.digestURIs.append( u );
        }
        idx = pos+1;
      } while ( pos != -1 );
    }
    else if (strncasecmp(p, "nonce=", 6)==0)
    {
      p+=6;
      while ( *p == '"' ) p++;  // Go past any " mark(s) first
      while ( p[i] != '"' ) i++;  // Read everything until the last " mark
      info.nonce = QByteArray(p,i);
    }
    else if (strncasecmp(p, "opaque=", 7)==0)
    {
      p+=7;
      while ( *p == '"' ) p++;  // Go past any " mark(s) first
      while ( p[i] != '"' ) i++;  // Read everything until the last " mark
      opaque = QByteArray(p,i);
    }
    else if (strncasecmp(p, "qop=", 4)==0)
    {
      p+=4;
      while ( *p == '"' ) p++;  // Go past any " mark(s) first
      while ( p[i] != '"' ) i++;  // Read everything until the last " mark
      info.qop = QByteArray(p,i);
    }
    p+=(i+1);
  }

  if (info.realm.isEmpty() || info.nonce.isEmpty()) {
    return QString();
  }

  // If the "domain" attribute was not specified and the current response code
  // is authentication needed, add the current request url to the list over which
  // this credential can be automatically applied.
  if (info.digestURIs.isEmpty() && (m_request.responseCode == 401 || m_request.responseCode == 407))
    info.digestURIs.append (m_request.url);
  else
  {
    // Verify whether or not we should send a cached credential to the
    // server based on the stored "domain" attribute...
    bool send = true;

    // Determine the path of the request url...
    QString requestPath = m_request.url.directory(KUrl::AppendTrailingSlash|KUrl::ObeyTrailingSlash);
    if (requestPath.isEmpty())
      requestPath = "/";

    foreach (const KUrl &u, info.digestURIs)
    {
      send &= (m_request.url.protocol().toLower() == u.protocol().toLower());
      send &= (m_request.url.host().toLower() == u.host().toLower());

      if (m_request.url.port() > 0 && u.port() > 0)
        send &= (m_request.url.port() == u.port());

      QString digestPath = u.directory (KUrl::AppendTrailingSlash|KUrl::ObeyTrailingSlash);
      if (digestPath.isEmpty())
        digestPath = "/";

      send &= (requestPath.startsWith(digestPath));

      if (send)
        break;
    }

    kDebug(7113) << "passed digest authentication credential test: " << send;

    if (!send) {
      return QString();
    }
  }

  kDebug(7113) << "RESULT OF PARSING:";
  kDebug(7113) << "  algorithm: " << info.algorithm;
  kDebug(7113) << "  realm:     " << info.realm;
  kDebug(7113) << "  nonce:     " << info.nonce;
  kDebug(7113) << "  opaque:    " << opaque;
  kDebug(7113) << "  qop:       " << info.qop;

  // Calculate the response...
  calculateResponse( info, Response );

  auth += "username=\"";
  auth += info.username;

  auth += "\", realm=\"";
  auth += info.realm;
  auth += "\"";

  auth += ", nonce=\"";
  auth += info.nonce;

  auth += "\", uri=\"";
  auth += m_request.url.encodedPathAndQuery(KUrl::LeaveTrailingSlash,KUrl::AvoidEmptyPath);

  auth += "\", algorithm=\"";
  auth += info.algorithm;
  auth +="\"";

  if ( !info.qop.isEmpty() )
  {
    auth += ", qop=\"";
    auth += info.qop;
    auth += "\", cnonce=\"";
    auth += info.cnonce;
    auth += "\", nc=";
    auth += info.nc;
  }

  auth += ", response=\"";
  auth += Response;
  if ( !opaque.isEmpty() )
  {
    auth += "\", opaque=\"";
    auth += opaque;
  }
  auth += "\"\r\n";

  return auth;
}

QString HTTPProtocol::proxyAuthenticationHeader()
{
    kDebug(7113);
    // We keep proxy authentication locally until they are changed.
    // Thus, no need to check with the password manager for every
    // connection.
    if (m_proxyAuth.realm.isEmpty()) {
        AuthInfo info;
        info.url = m_request.proxyUrl;
        info.username = m_request.proxyUrl.user();
        info.password = m_request.proxyUrl.pass();
        info.verifyPath = true;

        // If the proxy URL already contains username
        // and password simply attempt to retrieve it
        // without prompting the user...
        if (!info.username.isEmpty() && !info.password.isEmpty()) {
            if( m_proxyAuth.authorization.isEmpty() )
                m_proxyAuth.scheme = AUTH_None;
            else if( m_proxyAuth.authorization.startsWith("Basic") )
                m_proxyAuth.scheme = AUTH_Basic;
            else if( m_proxyAuth.authorization.startsWith("NTLM") )
                m_proxyAuth.scheme = AUTH_NTLM;
            else
                m_proxyAuth.scheme = AUTH_Digest;
        } else {
            if (checkCachedAuthentication(info) && !info.digestInfo.isEmpty()) {
                m_request.proxyUrl.setUser(info.username);
                m_request.proxyUrl.setPass(info.password);
                m_proxyAuth.realm = info.realmValue;
                m_proxyAuth.authorization = info.digestInfo;
                if (m_proxyAuth.authorization.startsWith("Basic")) {
                    m_proxyAuth.scheme = AUTH_Basic;
                } else if (m_proxyAuth.authorization.startsWith("NTLM")) {
                    m_proxyAuth.scheme = AUTH_NTLM;
                } else {
                    m_proxyAuth.scheme = AUTH_Digest;
                }
            } else {
                m_proxyAuth.scheme = AUTH_None;
            }
        }
    }

    if (m_proxyAuth.scheme != AUTH_None) {
        kDebug(7113) << "Using Proxy Authentication: ";
        kDebug(7113) << " HOST =" << m_request.proxyUrl.host();
        kDebug(7113) << " PORT =" << m_request.proxyUrl.port();
        kDebug(7113) << " USER =" << m_request.proxyUrl.user();
        kDebug(7113) << " PASSWORD = [protected]";
        kDebug(7113) << " REALM =" << m_proxyAuth.realm;
        kDebug(7113) << " EXTRA =" << m_proxyAuth.authorization;
    } else {
        kDebug(7113) << "m_proxyAuth.scheme = AUTH_None.";
    }

    switch (m_proxyAuth.scheme) {
    case AUTH_Basic:
        return createBasicAuth(true);
        break;
    case AUTH_Digest:
        return createDigestAuth(true);
        break;
    case AUTH_NTLM:
        if (m_isFirstRequest) {
            return createNTLMAuth(true);
        }
        break;
    case AUTH_None:
    default:
        break;
    }
    return QString();
}

QString HTTPProtocol::wwwAuthenticationHeader()
{
    kDebug(7113);
    // Only check for cached authentication if the previous response was NOT a 401 or 407.
    // In that case we have already tried the cached authentication in a previous attempt
    // (because it's the first thing we try) and it did not work.
    // Also no caching for schemes where challenges become stale.
    
    // m_auth *might* already contain useful values from getAuthorization(), continue
    // and try them in any case.
    
    if (!m_request.doNotAuthenticate && m_request.responseCode != 401
        && m_request.responseCode != 407
        && m_auth.scheme != AUTH_Negotiate) {

        AuthInfo info;
        info.url = m_request.url;
        info.verifyPath = true;
        if (!m_request.url.user().isEmpty()) {
            info.username = m_request.url.user();
        }

        kDebug(7113) << "Calling checkCachedAuthentication";
        
        if (checkCachedAuthentication(info) && !info.digestInfo.isEmpty()) {
            m_auth.scheme = AUTH_Digest;
            if (info.digestInfo.startsWith("Basic")) {
                m_auth.scheme = AUTH_Basic;
            } else if (info.digestInfo.startsWith("NTLM")) {
                m_auth.scheme = AUTH_NTLM;
            } else if (info.digestInfo.startsWith("Negotiate")) {
                m_auth.scheme = AUTH_Negotiate;
            }

            m_state.user = info.username;
            m_state.passwd = info.password;
            m_auth.realm = info.realmValue;
            if (m_auth.scheme != AUTH_NTLM && m_auth.scheme != AUTH_Negotiate) { // don't use the cached challenge
                m_auth.authorization = info.digestInfo;
            }
        }
    } else {
        kDebug(7113) << "Not calling checkCachedAuthentication";
    }

    if (m_auth.scheme != AUTH_None) {
        kDebug(7113) << "Using Authentication: ";
        kDebug(7113) << " HOST =" << m_state.hostname;
        kDebug(7113) << " PORT =" << m_state.port;
        kDebug(7113) << " USER =" << m_state.user;
        kDebug(7113) << " PASSWORD = [protected]";
        kDebug(7113) << " REALM =" << m_auth.realm;
        kDebug(7113) << " EXTRA =" << m_auth.authorization;
    }

    switch (m_auth.scheme) {
    case AUTH_Basic:
        return createBasicAuth();
    case AUTH_Digest:
        return createDigestAuth();
#ifdef HAVE_LIBGSSAPI
    case AUTH_Negotiate:
        return createNegotiateAuth();
#endif
    case AUTH_NTLM:
        return createNTLMAuth();
    case AUTH_None:
    default:
        break;
    }
    return QString();
}

void HTTPProtocol::proxyAuthenticationForSocket(const QNetworkProxy &proxy, QAuthenticator *authenticator)
{
    kDebug(7113) << "Authenticator received -- realm: " << authenticator->realm() << "user:"
                 << authenticator->user();

    AuthInfo info;
    info.url = m_request.proxyUrl;
    info.realmValue = authenticator->realm();
    info.verifyPath = true;    //### whatever
    info.username = authenticator->user();
    info.password = authenticator->password();  // well...

    if (!checkCachedAuthentication(info)) {
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
        openPasswordDialog(info, i18n("Proxy Authentication Failed."));
    }
    authenticator->setUser(info.username);
    authenticator->setPassword(info.password);

    m_request.proxyUrl.setUser(info.username);
    m_request.proxyUrl.setPassword(info.password);
    m_proxyAuth.scheme = AUTH_Basic;    // anything but AUTH_None should be fine
    m_proxyAuth.realm = info.realmValue;
    m_proxyAuth.authorization.clear();  // internal to QAbstractSocket and not needed
}

void HTTPProtocol::saveProxyAuthenticationForSocket()
{
    disconnect(socket(), SIGNAL(connected()),
               this, SLOT(saveProxyAuthenticationForSocket()));
    saveAuthorization(true /*isForProxy*/);
}

#include "http.moc"
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
