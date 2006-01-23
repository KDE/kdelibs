/*
   Copyright (C) 2000-2003 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2000-2002 George Staikos <staikos@kde.org>
   Copyright (C) 2000-2002 Dawit Alemayehu <adawit@kde.org>
   Copyright (C) 2001,2002 Hamish Rodda <rodda@kde.org>

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

#include <config.h>

#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <utime.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>  // Required for AIX
#include <netinet/tcp.h>
#include <unistd.h> // must be explicitly included for MacOSX

#include <qdom.h>
#include <qfile.h>
#include <qregexp.h>
#include <qdatetime.h>

#include <kurl.h>
#include <kidna.h>
#include <ksocks.h>
#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
#include <kservice.h>
#include <krfcdate.h>
#include <kcodecs.h>
#include <kinstance.h>
#include <kresolver.h>
#include <kmimemagic.h>
#include <dcopclient.h>
#include <kdatastream.h>
#include <kapplication.h>
#include <krandom.h>
#include <ktoolinvocation.h>
#include <kstreamsocket.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <kremoteencoding.h>

#include "kio/ioslave_defaults.h"
#include "kio/http_slave_defaults.h"

#include "httpfilter.h"
#include "http.h"

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

using namespace KIO;
using namespace KNetwork;

extern "C" int KDE_EXPORT kdemain( int argc, char **argv )
{
  KLocale::setMainCatalog("kdelibs");
  KInstance instance( "kio_http" );
  ( void ) KGlobal::locale();

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

static char * trimLead (char *orig_string)
{
  while (*orig_string == ' ')
    orig_string++;
  return orig_string;
}

static bool isCrossDomainRequest( const QString& fqdn, const QString& originURL )
{
  if (originURL == "true") // Backwards compatibility
     return true;

  KUrl url ( originURL );

  // Document Origin domain
  QString a = url.host();

  // Current request domain
  QString b = fqdn;

  if (a == b)
    return false;

  QStringList l1 = a.split(',',QString::SkipEmptyParts);
  QStringList l2 = b.split('.',QString::SkipEmptyParts);

  while(l1.count() > l2.count())
      l1.pop_front();

  while(l2.count() > l1.count())
      l2.pop_front();

  while(l2.count() >= 2)
  {
      if (l1 == l2)
          return false;

      l1.pop_front();
      l2.pop_front();
  }

  return true;
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
             :TCPSlaveBase( 0, protocol , pool, app,
                            (protocol == "https" || protocol == "webdavs") )
{

  m_bBusy = false;
  m_bFirstRequest = false;
  m_bProxyAuthValid = false;

  m_iSize = NO_SIZE;
  m_lineBufUnget = 0;

  m_protocol = protocol;

  m_maxCacheAge = DEFAULT_MAX_CACHE_AGE;
  m_maxCacheSize = DEFAULT_MAX_CACHE_SIZE / 2;
  m_remoteConnTimeout = DEFAULT_CONNECT_TIMEOUT;
  m_remoteRespTimeout = DEFAULT_RESPONSE_TIMEOUT;
  m_proxyConnTimeout = DEFAULT_PROXY_CONNECT_TIMEOUT;

  m_pid = getpid();

  reparseConfiguration();
}

HTTPProtocol::~HTTPProtocol()
{
  httpClose(false);
  qDeleteAll(m_requestQueue);
  m_requestQueue.clear();
}

void HTTPProtocol::reparseConfiguration()
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::reparseConfiguration" << endl;

  m_strProxyRealm.clear();
  m_strProxyAuthorization.clear();
  ProxyAuthentication = AUTH_None;
  m_bUseProxy = false;

  if (m_protocol == "https" || m_protocol == "webdavs")
    m_iDefaultPort = DEFAULT_HTTPS_PORT;
  else if (m_protocol == "ftp")
    m_iDefaultPort = DEFAULT_FTP_PORT;
  else
    m_iDefaultPort = DEFAULT_HTTP_PORT;
}

void HTTPProtocol::resetConnectionSettings()
{
  m_bEOF = false;
  m_bError = false;
  m_lineCount = 0;
  m_iWWWAuthCount = 0;
  m_lineCountUnget = 0;
  m_iProxyAuthCount = 0;

}

void HTTPProtocol::resetResponseSettings()
{
  m_bRedirect = false;
  m_bChunked = false;
  m_iSize = NO_SIZE;

  m_responseHeader.clear();
  m_qContentEncodings.clear();
  m_qTransferEncodings.clear();
  m_sContentMD5.clear();
  m_strMimeType.clear();

  setMetaData("request-id", m_request.id);
}

void HTTPProtocol::resetSessionSettings()
{
  // Do not reset the URL on redirection if the proxy
  // URL, username or password has not changed!
  KUrl proxy ( config()->readEntry("UseProxy") );

  if ( m_strProxyRealm.isEmpty() || !proxy.isValid() ||
       m_proxyURL.host() != proxy.host() ||
       (!proxy.user().isNull() && proxy.user() != m_proxyURL.user()) ||
       (!proxy.pass().isNull() && proxy.pass() != m_proxyURL.pass()) )
  {
    m_bProxyAuthValid = false;
    m_proxyURL = proxy;
    m_bUseProxy = m_proxyURL.isValid();

    kdDebug(7113) << "(" << m_pid << ") Using proxy: " << m_bUseProxy <<
                                              " URL: " << m_proxyURL.url() <<
                                            " Realm: " << m_strProxyRealm << endl;
  }

  m_bPersistentProxyConnection = config()->readEntry("PersistentProxyConnection", false);
  kdDebug(7113) << "(" << m_pid << ") Enable Persistent Proxy Connection: "
                << m_bPersistentProxyConnection << endl;

  m_request.bUseCookiejar = config()->readEntry("Cookies", false);
  m_request.bUseCache = config()->readEntry("UseCache", true);
  m_request.bErrorPage = config()->readEntry("errorPage", true);
  m_request.bNoAuth = config()->readEntry("no-auth", false);
  m_strCacheDir = config()->readPathEntry("CacheDir");
  m_maxCacheAge = config()->readEntry("MaxCacheAge", DEFAULT_MAX_CACHE_AGE);
  m_request.window = config()->readEntry("window-id");

  kdDebug(7113) << "(" << m_pid << ") Window Id = " << m_request.window << endl;
  kdDebug(7113) << "(" << m_pid << ") ssl_was_in_use = "
                << metaData ("ssl_was_in_use") << endl;

  m_request.referrer.clear();
  if ( config()->readEntry("SendReferrer", true) &&
       (m_protocol == "https" || m_protocol == "webdavs" ||
        metaData ("ssl_was_in_use") != "TRUE" ) )
  {
     KUrl referrerURL ( metaData("referrer") );
     if (referrerURL.isValid())
     {
        // Sanitize
        QString protocol = referrerURL.protocol();
        if (protocol.startsWith("webdav"))
        {
           protocol.replace(0, 6, "http");
           referrerURL.setProtocol(protocol);
        }

        if (protocol.startsWith("http"))
        {
           referrerURL.setRef(QString());
           referrerURL.setUser(QString());
           referrerURL.setPass(QString());
           m_request.referrer = referrerURL.url();
        }
     }
  }

  if ( config()->readEntry("SendLanguageSettings", true) )
  {
      m_request.charsets = config()->readEntry( "Charsets", "iso-8859-1" );

      if ( !m_request.charsets.isEmpty() )
          m_request.charsets += DEFAULT_PARTIAL_CHARSET_HEADER;

      m_request.languages = config()->readEntry( "Languages", DEFAULT_LANGUAGE_HEADER );
  }
  else
  {
      m_request.charsets.clear();
      m_request.languages.clear();
  }

  // Adjust the offset value based on the "resume" meta-data.
  QString resumeOffset = metaData("resume");
  if ( !resumeOffset.isEmpty() )
     m_request.offset = resumeOffset.toInt(); // TODO: Convert to 64 bit
  else
     m_request.offset = 0;

  m_request.disablePassDlg = config()->readEntry("DisablePassDlg", false);
  m_request.allowCompressedPage = config()->readEntry("AllowCompressedPage", true);
  m_request.id = metaData("request-id");

  // Store user agent for this host.
  if ( config()->readEntry("SendUserAgent", true) )
     m_request.userAgent = metaData("UserAgent");
  else
     m_request.userAgent.clear();

  // Deal with cache cleaning.
  // TODO: Find a smarter way to deal with cleaning the
  // cache ?
  if ( m_request.bUseCache )
    cleanCache();

  // Deal with HTTP tunneling
  if ( m_bIsSSL && m_bUseProxy && m_proxyURL.protocol() != "https" &&
       m_proxyURL.protocol() != "webdavs")
  {
    m_bNeedTunnel = true;
    setRealHost( m_request.hostname );
    kdDebug(7113) << "(" << m_pid << ") SSL tunnel: Setting real hostname to: "
                  << m_request.hostname << endl;
  }
  else
  {
    m_bNeedTunnel = false;
    setRealHost( QString());
  }

  m_responseCode = 0;
  m_prevResponseCode = 0;

  m_strRealm.clear();
  m_strAuthorization.clear();
  Authentication = AUTH_None;

  // Obtain the proxy and remote server timeout values
  m_proxyConnTimeout = proxyConnectTimeout();
  m_remoteConnTimeout = connectTimeout();
  m_remoteRespTimeout = responseTimeout();

  // Bounce back the actual referrer sent
  setMetaData("referrer", m_request.referrer);

  // Set the SSL meta-data here...
  setSSLMetaData();

  // Follow HTTP/1.1 spec and enable keep-alive by default
  // unless the remote side tells us otherwise or we determine
  // the persistent link has been terminated by the remote end.
  m_bKeepAlive = true;
  m_keepAliveTimeout = 0;
  m_bUnauthorized = false;

  // A single request can require multiple exchanges with the remote
  // server due to authentication challenges or SSL tunneling.
  // m_bFirstRequest is a flag that indicates whether we are
  // still processing the first request. This is important because we
  // should not force a close of a keep-alive connection in the middle
  // of the first request.
  // m_bFirstRequest is set to "true" whenever a new connection is
  // made in httpOpenConnection()
  m_bFirstRequest = false;
}

void HTTPProtocol::setHost( const QString& host, int port,
                            const QString& user, const QString& pass )
{
  // Reset the webdav-capable flags for this host
  if ( m_request.hostname != host )
    m_davHostOk = m_davHostUnsupported = false;

  // is it an IPv6 address?
  if (host.indexOf(':') == -1)
    {
      m_request.hostname = host;
      m_request.encoded_hostname = KIDNA::toAscii(host);
    }
  else
    {
      m_request.hostname = host;
      int pos = host.indexOf('%');
      if (pos == -1)
	m_request.encoded_hostname = '[' + host + ']';
      else
	// don't send the scope-id in IPv6 addresses to the server
	m_request.encoded_hostname = '[' + host.left(pos) + ']';
    }
  m_request.port = (port == 0) ? m_iDefaultPort : port;
  m_request.user = user;
  m_request.passwd = pass;

  m_bIsTunneled = false;

  kdDebug(7113) << "(" << m_pid << ") Hostname is now: " << m_request.hostname <<
    " (" << m_request.encoded_hostname << ")" <<endl;
}

bool HTTPProtocol::checkRequestURL( const KUrl& u )
{
  kdDebug (7113) << "(" << m_pid << ") HTTPProtocol::checkRequestURL:  " << u.url() << endl;

  m_request.url = u;

  if (m_request.hostname.isEmpty())
  {
     error( KIO::ERR_UNKNOWN_HOST, i18n("No host specified."));
     return false;
  }

  if (u.path().isEmpty())
  {
     KUrl newUrl(u);
     newUrl.setPath("/");
     redirection(newUrl);
     finished();
     return false;
  }

  if ( m_protocol != u.protocol().toLatin1() )
  {
    short unsigned int oldDefaultPort = m_iDefaultPort;
    m_protocol = u.protocol().toLatin1();
    reparseConfiguration();
    if ( m_iDefaultPort != oldDefaultPort &&
         m_request.port == oldDefaultPort )
        m_request.port = m_iDefaultPort;
  }

  resetSessionSettings();
  return true;
}

void HTTPProtocol::retrieveContent( bool dataInternal /* = false */ )
{
  kdDebug (7113) << "(" << m_pid << ") HTTPProtocol::retrieveContent " << endl;
  if ( !retrieveHeader( false ) )
  {
    if ( m_bError )
      return;
  }
  else
  {
    if ( !readBody( dataInternal ) && m_bError )
      return;
  }

  httpClose(m_bKeepAlive);

  // if data is required internally, don't finish,
  // it is processed before we finish()
  if ( !dataInternal )
  {
    if ((m_responseCode == 204) &&
        ((m_request.method == HTTP_GET) || (m_request.method == HTTP_POST)))
       error(ERR_NO_CONTENT, "");
    else
       finished();
  }
}

bool HTTPProtocol::retrieveHeader( bool close_connection )
{
  kdDebug (7113) << "(" << m_pid << ") HTTPProtocol::retrieveHeader " << endl;
  while ( 1 )
  {
    if (!httpOpen())
      return false;

    resetResponseSettings();
    if (!readHeader())
    {
      if ( m_bError )
        return false;

      if (m_bIsTunneled)
      {
        kdDebug(7113) << "(" << m_pid << ") Re-establishing SSL tunnel..." << endl;
        httpCloseConnection();
      }
    }
    else
    {
      // Do not save authorization if the current response code is
      // 4xx (client error) or 5xx (server error).
      kdDebug(7113) << "(" << m_pid << ") Previous Response: "
                    << m_prevResponseCode << endl;
      kdDebug(7113) << "(" << m_pid << ") Current Response: "
                    << m_responseCode << endl;

      if (isSSLTunnelEnabled() &&  m_bIsSSL && !m_bUnauthorized && !m_bError)
      {
        // If there is no error, disable tunneling
        if ( m_responseCode < 400 )
        {
          kdDebug(7113) << "(" << m_pid << ") Unset tunneling flag!" << endl;
          setEnableSSLTunnel( false );
          m_bIsTunneled = true;
          // Reset the CONNECT response code...
          m_responseCode = m_prevResponseCode;
          continue;
        }
        else
        {
          if ( !m_request.bErrorPage )
          {
            kdDebug(7113) << "(" << m_pid << ") Sending an error message!" << endl;
            error( ERR_UNKNOWN_PROXY_HOST, m_proxyURL.host() );
            return false;
          }

          kdDebug(7113) << "(" << m_pid << ") Sending an error page!" << endl;
        }
      }

      if (m_responseCode < 400 && (m_prevResponseCode == 401 ||
          m_prevResponseCode == 407))
        saveAuthorization();
      break;
    }
  }

  // Clear of the temporary POST buffer if it is not empty...
  if (!m_bufPOST.isEmpty())
  {
    m_bufPOST.resize(0);
    kdDebug(7113) << "(" << m_pid << ") HTTP::retreiveHeader: Cleared POST "
                     "buffer..." << endl;
  }

  if ( close_connection )
  {
    httpClose(m_bKeepAlive);
    finished();
  }

  return true;
}

void HTTPProtocol::stat(const KUrl& url)
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::stat " << url.prettyURL()
                << endl;

  if ( !checkRequestURL( url ) )
      return;

  if ( m_protocol != "webdav" && m_protocol != "webdavs" )
  {
    QString statSide = metaData(QString::fromLatin1("statSide"));
    if ( statSide != "source" )
    {
      // When uploading we assume the file doesn't exit
      error( ERR_DOES_NOT_EXIST, url.prettyURL() );
      return;
    }

    // When downloading we assume it exists
    UDSEntry entry;
    entry.insert( KIO::UDS_NAME, url.fileName() );
    entry.insert( KIO::UDS_FILE_TYPE, S_IFREG ); // a file
    entry.insert( KIO::UDS_ACCESS, S_IRUSR | S_IRGRP | S_IROTH ); // readable by everybody

    statEntry( entry );
    finished();
    return;
  }

  davStatList( url );
}

void HTTPProtocol::listDir( const KUrl& url )
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::listDir " << url.url()
                << endl;

  if ( !checkRequestURL( url ) )
    return;

  davStatList( url, false );
}

void HTTPProtocol::davSetRequest( const QByteArray& requestXML )
{
  // insert the document into the POST buffer, kill trailing zero byte
  m_bufPOST = requestXML;
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
  m_request.query.clear();
  m_request.cache = CC_Reload;
  m_request.doProxy = m_bUseProxy;
  m_request.davData.depth = stat ? 0 : 1;
  if (!stat)
     m_request.url.adjustPath(+1);

  retrieveContent( true );

  // Has a redirection already been called? If so, we're done.
  if (m_bRedirect) {
    finished();
    return;
  }

  QDomDocument multiResponse;
  multiResponse.setContent( m_bufWebDavData, true );

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

      QString urlStr = href.text();
      int encoding = remoteEncoding()->encodingMib();
      if ((encoding == 106) && (!KStringHandler::isUtf8(KUrl::decode_string(urlStr, 4).toLatin1())))
        encoding = 4; // Use latin1 if the file is not actually utf-8

      KUrl thisURL ( urlStr, encoding );

      if ( thisURL.isValid() ) {
        // don't list the base dir of a listDir()
        if ( !stat && thisURL.path(+1).length() == url.path(+1).length() )
          continue;

        entry.insert( KIO::UDS_NAME, thisURL.fileName() );
      } else {
        // This is a relative URL.
        entry.insert( KIO::UDS_NAME, href.text() );
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
      kdDebug(7113) << "Error: no URL contained in response to PROPFIND on "
                    << url.prettyURL() << endl;
    }
  }

  if ( stat || !hasResponse )
  {
    error( ERR_DOES_NOT_EXIST, url.prettyURL() );
  }
  else
  {
    listEntry( entry, true );
    finished();
  }
}

void HTTPProtocol::davGeneric( const KUrl& url, KIO::HTTP_METHOD method )
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::davGeneric " << url.url()
                << endl;

  if ( !checkRequestURL( url ) )
    return;

  // check to make sure this host supports WebDAV
  if ( !davHostOk() )
    return;

  // WebDAV method
  m_request.method = method;
  m_request.query.clear();
  m_request.cache = CC_Reload;
  m_request.doProxy = m_bUseProxy;

  retrieveContent( false );
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
      kdDebug(7113) << "Error, no status code in this propstat" << endl;
      return;
    }

    int code = codeFromResponse( status.text() );

    if ( code != 200 )
    {
      kdDebug(7113) << "Warning: status code " << code << " (this may mean that some properties are unavailable" << endl;
      continue;
    }

    QDomElement prop = propstat.namedItem( "prop" ).toElement();
    if ( prop.isNull() )
    {
      kdDebug(7113) << "Error: no prop segment in this propstat." << endl;
      return;
    }

    if ( hasMetaData( "davRequestResponse" ) )
    {
      QDomDocument doc;
      doc.appendChild(prop);
      entry.insert( KIO::UDS_XML_PROPERTIES, doc.toString() );
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
        entry.insert( KIO::UDS_CREATION_TIME, parseDateTime( property.text(), property.attribute("dt") ) );
      }
      else if ( property.tagName() == "getcontentlength" )
      {
        // Content length (file size)
        entry.insert( KIO::UDS_SIZE, property.text().toULong() );
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
        entry.insert( KIO::UDS_MODIFICATION_TIME, parseDateTime( property.text(), property.attribute("dt") ) );
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
        kdDebug(7113) << "Found unknown webdav property: " << property.tagName() << endl;
      }
    }
  }

  setMetaData( "davLockCount", QString("%1").arg(lockCount) );
  setMetaData( "davSupportedLockCount", QString("%1").arg(supportedLockCount) );

  entry.insert( KIO::UDS_FILE_TYPE, isDirectory ? S_IFDIR : S_IFREG );

  if ( foundExecutable || isDirectory )
  {
    // File was executable, or is a directory.
    entry.insert( KIO::UDS_ACCESS, 0700 );
  }
  else
  {
    entry.insert( KIO::UDS_ACCESS, 0600 );
  }

  if ( !isDirectory && !mimeType.isEmpty() )
  {
    entry.insert( KIO::UDS_MIME_TYPE, mimeType );
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
    return KRFCDate::parseDateISO8601( input );
  }
  else if ( type == "dateTime.rfc1123" )
  {
    return KRFCDate::parseDate( input );
  }

  // format not advertised... try to parse anyway
  time_t time = KRFCDate::parseDate( input );
  if ( time != 0 )
    return time;

  return KRFCDate::parseDateISO8601( input );
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
            response += ")";
            bracketsOpen = false;
          }
          response += " <" + metaData( QString("davLockURL%1").arg(i) ) + ">";
        }

        if ( !bracketsOpen )
        {
          response += " (";
          bracketsOpen = true;
        }
        else
        {
          response += " ";
        }

        if ( hasMetaData( QString("davLockNot%1").arg(i) ) )
          response += "Not ";

        response += "<" + metaData( QString("davLockToken%1").arg(i) ) + ">";
      }
    }

    if ( bracketsOpen )
      response += ")";

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
    kdDebug(7113) << "(" << m_pid << ") " << k_funcinfo << " true" << endl;
    return true;
  }
  else if ( m_davHostUnsupported )
  {
    kdDebug(7113) << "(" << m_pid << ") " << k_funcinfo << " false" << endl;
    davError( -2 );
    return false;
  }

  m_request.method = HTTP_OPTIONS;

  // query the server's capabilities generally, not for a specific URL
  m_request.path = "*";
  m_request.query.clear();
  m_request.cache = CC_Reload;
  m_request.doProxy = m_bUseProxy;

  // clear davVersions variable, which holds the response to the DAV: header
  m_davCapabilities.clear();

  retrieveHeader(false);

  if (m_davCapabilities.count())
  {
    for (int i = 0; i < m_davCapabilities.count(); i++)
    {
      bool ok;
      uint verNo = m_davCapabilities[i].toUInt(&ok);
      if (ok && verNo > 0 && verNo < 3)
      {
        m_davHostOk = true;
        kdDebug(7113) << "Server supports DAV version " << verNo << "." << endl;
      }
    }

    if ( m_davHostOk )
      return true;
  }

  m_davHostUnsupported = true;
  davError( -2 );
  return false;
}

// This function is for closing retrieveHeader( false ); requests
// Required because there may or may not be further info expected
void HTTPProtocol::davFinished()
{
  // TODO: Check with the DAV extension developers
  httpClose(m_bKeepAlive);
  finished();
}

void HTTPProtocol::mkdir( const KUrl& url, int )
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::mkdir " << url.url()
                << endl;

  if ( !checkRequestURL( url ) )
    return;

  m_request.method = DAV_MKCOL;
  m_request.path = url.path();
  m_request.query.clear();
  m_request.cache = CC_Reload;
  m_request.doProxy = m_bUseProxy;

  retrieveHeader( false );

  if ( m_responseCode == 201 )
    davFinished();
  else
    davError();
}

void HTTPProtocol::get( const KUrl& url )
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::get " << url.url()
                << endl;

  if ( !checkRequestURL( url ) )
    return;

  m_request.method = HTTP_GET;
  m_request.path = url.path();
  m_request.query = url.query();

  QString tmp = metaData("cache");
  if (!tmp.isEmpty())
    m_request.cache = parseCacheControl(tmp);
  else
    m_request.cache = DEFAULT_CACHE_CONTROL;

  m_request.passwd = url.pass();
  m_request.user = url.user();
  m_request.doProxy = m_bUseProxy;

  retrieveContent();
}

void HTTPProtocol::put( const KUrl &url, int, bool overwrite, bool)
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::put " << url.prettyURL()
                << endl;

  if ( !checkRequestURL( url ) )
    return;

  // Webdav hosts are capable of observing overwrite == false
  if (!overwrite && m_protocol.left(6) == "webdav") {
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
    m_request.query.clear();
    m_request.cache = CC_Reload;
    m_request.doProxy = m_bUseProxy;
    m_request.davData.depth = 0;

    retrieveContent(true);

    if (m_responseCode == 207) {
      error(ERR_FILE_ALREADY_EXIST, QString());
      return;
    }

    m_bError = false;
  }

  m_request.method = HTTP_PUT;
  m_request.path = url.path();
  m_request.query.clear();
  m_request.cache = CC_Reload;
  m_request.doProxy = m_bUseProxy;

  retrieveHeader( false );

  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::put error = " << m_bError << endl;
  if (m_bError)
    return;

  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::put responseCode = " << m_responseCode << endl;

  httpClose(false); // Always close connection.

  if ( (m_responseCode >= 200) && (m_responseCode < 300) )
    finished();
  else
    httpError();
}

void HTTPProtocol::copy( const KUrl& src, const KUrl& dest, int, bool overwrite )
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::copy " << src.prettyURL()
                << " -> " << dest.prettyURL() << endl;

  if ( !checkRequestURL( dest ) || !checkRequestURL( src ) )
    return;

  // destination has to be "http(s)://..."
  KUrl newDest = dest;
  if (newDest.protocol() == "webdavs")
    newDest.setProtocol("https");
  else
    newDest.setProtocol("http");

  m_request.method = DAV_COPY;
  m_request.path = src.path();
  m_request.davData.desturl = newDest.url();
  m_request.davData.overwrite = overwrite;
  m_request.query.clear();
  m_request.cache = CC_Reload;
  m_request.doProxy = m_bUseProxy;

  retrieveHeader( false );

  // The server returns a HTTP/1.1 201 Created or 204 No Content on successful completion
  if ( m_responseCode == 201 || m_responseCode == 204 )
    davFinished();
  else
    davError();
}

void HTTPProtocol::rename( const KUrl& src, const KUrl& dest, bool overwrite )
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::rename " << src.prettyURL()
                << " -> " << dest.prettyURL() << endl;

  if ( !checkRequestURL( dest ) || !checkRequestURL( src ) )
    return;

  // destination has to be "http://..."
  KUrl newDest = dest;
  if (newDest.protocol() == "webdavs")
    newDest.setProtocol("https");
  else
    newDest.setProtocol("http");

  m_request.method = DAV_MOVE;
  m_request.path = src.path();
  m_request.davData.desturl = newDest.url();
  m_request.davData.overwrite = overwrite;
  m_request.query.clear();
  m_request.cache = CC_Reload;
  m_request.doProxy = m_bUseProxy;

  retrieveHeader( false );

  if ( m_responseCode == 201 )
    davFinished();
  else
    davError();
}

void HTTPProtocol::del( const KUrl& url, bool )
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::del " << url.prettyURL()
                << endl;

  if ( !checkRequestURL( url ) )
    return;

  m_request.method = HTTP_DELETE;
  m_request.path = url.path();
  m_request.query.clear();
  m_request.cache = CC_Reload;
  m_request.doProxy = m_bUseProxy;

  retrieveHeader( false );

  // The server returns a HTTP/1.1 200 Ok or HTTP/1.1 204 No Content
  // on successful completion
  if ( m_responseCode == 200 || m_responseCode == 204 )
    davFinished();
  else
    davError();
}

void HTTPProtocol::post( const KUrl& url )
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::post "
                << url.prettyURL() << endl;

  if ( !checkRequestURL( url ) )
    return;

  m_request.method = HTTP_POST;
  m_request.path = url.path();
  m_request.query = url.query();
  m_request.cache = CC_Reload;
  m_request.doProxy = m_bUseProxy;

  retrieveContent();
}

void HTTPProtocol::davLock( const KUrl& url, const QString& scope,
                            const QString& type, const QString& owner )
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::davLock "
                << url.prettyURL() << endl;

  if ( !checkRequestURL( url ) )
    return;

  m_request.method = DAV_LOCK;
  m_request.path = url.path();
  m_request.query.clear();
  m_request.cache = CC_Reload;
  m_request.doProxy = m_bUseProxy;

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
  m_bufPOST = lockReq.toByteArray();

  retrieveContent( true );

  if ( m_responseCode == 200 ) {
    // success
    QDomDocument multiResponse;
    multiResponse.setContent( m_bufWebDavData, true );

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
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::davUnlock "
                << url.prettyURL() << endl;

  if ( !checkRequestURL( url ) )
    return;

  m_request.method = DAV_UNLOCK;
  m_request.path = url.path();
  m_request.query.clear();
  m_request.cache = CC_Reload;
  m_request.doProxy = m_bUseProxy;

  retrieveContent( true );

  if ( m_responseCode == 200 )
    finished();
  else
    davError();
}

QString HTTPProtocol::davError( int code /* = -1 */, QString url )
{
  bool callError = false;
  if ( code == -1 ) {
    code = m_responseCode;
    callError = true;
  }
  if ( code == -2 ) {
    callError = true;
  }

  if ( !url.isNull() )
    url = m_request.url.url();

  QString action, errorString;
  KIO::Error kError;

  // for 412 Precondition Failed
  QString ow = i18n( "Otherwise, the request would have succeeded." );

  switch ( m_request.method ) {
    case DAV_PROPFIND:
      action = i18n( "retrieve property values" );
      break;
    case DAV_PROPPATCH:
      action = i18n( "set property values" );
      break;
    case DAV_MKCOL:
      action = i18n( "create the requested folder" );
      break;
    case DAV_COPY:
      action = i18n( "copy the specified file or folder" );
      break;
    case DAV_MOVE:
      action = i18n( "move the specified file or folder" );
      break;
    case DAV_SEARCH:
      action = i18n( "search in the specified folder" );
      break;
    case DAV_LOCK:
      action = i18n( "lock the specified file or folder" );
      break;
    case DAV_UNLOCK:
      action = i18n( "unlock the specified file or folder" );
      break;
    case HTTP_DELETE:
      action = i18n( "delete the specified file or folder" );
      break;
    case HTTP_OPTIONS:
      action = i18n( "query the server's capabilities" );
      break;
    case HTTP_GET:
      action = i18n( "retrieve the contents of the specified file or folder" );
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
  errorString = i18n("An unexpected error (%1) occurred while attempting to %2.")
                      .arg( code ).arg( action );

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
      if ( !readBody( true ) && m_bError )
        return QString();

      QStringList errors;
      QDomDocument multiResponse;

      multiResponse.setContent( m_bufWebDavData, true );

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
      errorString = i18n("An error occurred while attempting to %1, %2. A "
                         "summary of the reasons is below.<ul>").arg( action ).arg( url );

      for ( QStringList::Iterator it = errors.begin(); it != errors.end(); ++it )
        errorString += "<li>" + *it + "</li>";

      errorString += "</ul>";
    }
    case 403:
    case 500: // hack: Apache mod_dav returns this instead of 403 (!)
      // 403 Forbidden
      kError = ERR_ACCESS_DENIED;
      errorString = i18n("Access was denied while attempting to %1.").arg( action );
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
                           "requesting that files are not overwritten. %1")
                           .arg( ow );

      }
      else if ( m_request.method == DAV_LOCK )
      {
        kError = ERR_ACCESS_DENIED;
        errorString = i18n("The requested lock could not be granted. %1").arg( ow );
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
      errorString = i18n("Unable to %1 because the resource is locked.").arg( action );
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
        errorString = i18n("Unable to %1 because the destination server refuses "
                           "to accept the file or folder.").arg( action );
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
  //errorString += " (" + url + ")";

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
      action = i18n( "upload %1" ).arg(m_request.url.prettyURL());
      break;
    default:
      // this should not happen, this function is for http errors only
      Q_ASSERT(0);
  }

  // default error message if the following code fails
  kError = ERR_INTERNAL;
  errorString = i18n("An unexpected error (%1) occurred while attempting to %2.")
                      .arg( m_responseCode ).arg( action );

  switch ( m_responseCode )
  {
    case 403:
    case 405:
    case 500: // hack: Apache mod_dav returns this instead of 403 (!)
      // 403 Forbidden
      // 405 Method Not Allowed
      kError = ERR_ACCESS_DENIED;
      errorString = i18n("Access was denied while attempting to %1.").arg( action );
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
      errorString = i18n("Unable to %1 because the resource is locked.").arg( action );
      break;
    case 502:
      // 502 Bad Gateway
      kError = ERR_WRITE_ACCESS_DENIED;
      errorString = i18n("Unable to %1 because the destination server refuses "
                         "to accept the file or folder.").arg( action );
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
  //errorString += " (" + url + ")";

  error( ERR_SLAVE_DEFINED, errorString );
}

bool HTTPProtocol::isOffline(const KUrl &url)
{
  const int NetWorkStatusUnknown = 1;
  const int NetWorkStatusOnline = 8;
  DCOPCString replyType;
  QByteArray params;
  QByteArray reply;

  QDataStream stream(&params, QIODevice::WriteOnly);
  stream << url.url();

  if ( dcopClient()->call( "kded", "networkstatus", "status(QString)",
                           params, replyType, reply ) && (replyType == "int") )
  {
     int result;
     QDataStream stream2( reply );
     stream2 >> result;
     kdDebug(7113) << "(" << m_pid << ") networkstatus status = " << result << endl;
     return (result != NetWorkStatusUnknown) && (result != NetWorkStatusOnline);
  }
  kdDebug(7113) << "(" << m_pid << ") networkstatus <unreachable>" << endl;
  return false; // On error, assume we are online
}

void HTTPProtocol::multiGet(const QByteArray &data)
{
  QDataStream stream(data);
  quint32 n;
  stream >> n;

  kdDebug(7113) << "(" << m_pid << ") HTTPProtcool::multiGet n = " << n << endl;

  HTTPRequest saveRequest;
  if (m_bBusy)
     saveRequest = m_request;

//  m_requestQueue.clear();
  for(unsigned i = 0; i < n; i++)
  {
     KUrl url;
     stream >> url >> mIncomingMetaData;

     if ( !checkRequestURL( url ) )
        continue;

     kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::multi_get " << url.url() << endl;

     m_request.method = HTTP_GET;
     m_request.path = url.path();
     m_request.query = url.query();
     QString tmp = metaData("cache");
     if (!tmp.isEmpty())
        m_request.cache = parseCacheControl(tmp);
     else
        m_request.cache = DEFAULT_CACHE_CONTROL;

     m_request.passwd = url.pass();
     m_request.user = url.user();
     m_request.doProxy = m_bUseProxy;

     HTTPRequest *newRequest = new HTTPRequest(m_request);
     m_requestQueue.append(newRequest);
  }

  if (m_bBusy)
     m_request = saveRequest;

  if (!m_bBusy)
  {
     m_bBusy = true;
	 QMutableListIterator<HTTPRequest*> i(m_requestQueue);
	 while (i.hasNext()) {
	 	HTTPRequest *request = i.next();
		m_request = *request;
		i.remove();
		retrieveContent();
	 }
#if 0
     while(!m_requestQueue.isEmpty())
     {
        HTTPRequest *request = m_requestQueue.take(0);
        m_request = *request;
        delete request;
        retrieveContent();
     }
#endif
     m_bBusy = false;
  }
}

ssize_t HTTPProtocol::write (const void *_buf, size_t nbytes)
{
  int bytes_sent = 0;
  const char* buf = static_cast<const char*>(_buf);
  while ( nbytes > 0 )
  {
    int n = TCPSlaveBase::write(buf, nbytes);

    if ( n <= 0 )
    {
      // remote side closed connection ?
      if ( n == 0 )
        break;
      // a valid exception(s) occurred, let's retry...
      if (n < 0 && ((errno == EINTR) || (errno == EAGAIN)))
        continue;
      // some other error occurred ?
      return -1;
    }

    nbytes -= n;
    buf += n;
    bytes_sent += n;
  }

  return bytes_sent;
}

void HTTPProtocol::setRewindMarker()
{
  m_rewindCount = 0;
}

void HTTPProtocol::rewind()
{
  m_linePtrUnget = m_rewindBuf,
  m_lineCountUnget = m_rewindCount;
  m_rewindCount = 0;
}


char *HTTPProtocol::gets (char *s, int size)
{
  int len=0;
  char *buf=s;
  char mybuf[2]={0,0};

  while (len < size)
  {
    read(mybuf, 1);
    if (m_bEOF)
      break;

    if (m_rewindCount < sizeof(m_rewindBuf))
       m_rewindBuf[m_rewindCount++] = *mybuf;

    if (*mybuf == '\r') // Ignore!
      continue;

    if ((*mybuf == '\n') || !*mybuf)
      break;

    *buf++ = *mybuf;
    len++;
  }

  *buf=0;
  return s;
}

ssize_t HTTPProtocol::read (void *b, size_t nbytes)
{
  ssize_t ret = 0;

  if (m_lineCountUnget > 0)
  {
    ret = ( nbytes < m_lineCountUnget ? nbytes : m_lineCountUnget );
    m_lineCountUnget -= ret;
    memcpy(b, m_linePtrUnget, ret);
    m_linePtrUnget += ret;

    return ret;
  }

  if (m_lineCount > 0)
  {
    ret = ( nbytes < m_lineCount ? nbytes : m_lineCount );
    m_lineCount -= ret;
    memcpy(b, m_linePtr, ret);
    m_linePtr += ret;
    return ret;
  }

  if (nbytes == 1)
  {
    ret = read(m_lineBuf, 1024); // Read into buffer
    m_linePtr = m_lineBuf;
    if (ret <= 0)
    {
      m_lineCount = 0;
      return ret;
    }
    m_lineCount = ret;
    return read(b, 1); // Read from buffer
  }

  do
  {
    ret = TCPSlaveBase::read( ( char* )b, nbytes);
    if (ret == 0)
      m_bEOF = true;

  } while ((ret == -1) && (errno == EAGAIN || errno == EINTR));

  return ret;
}

void HTTPProtocol::httpCheckConnection()
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::httpCheckConnection: " <<
//                                   " Socket status: " << m_iSock <<
                                      " Keep Alive: " << m_bKeepAlive <<
                                           " First: " << m_bFirstRequest << endl;

  if ( !m_bFirstRequest && isConnectionValid() )
  {
     bool closeDown = false;
     if ( !isConnectionValid())
     {
        kdDebug(7113) << "(" << m_pid << ") Connection lost!" << endl;
        closeDown = true;
     }
     else if ( m_request.method != HTTP_GET )
     {
        closeDown = true;
     }
     else if ( !m_state.doProxy && !m_request.doProxy )
     {
        if (m_state.hostname != m_request.hostname ||
            m_state.port != m_request.port ||
            m_state.user != m_request.user ||
            m_state.passwd != m_request.passwd)
          closeDown = true;
     }
     else
     {
        // Keep the connection to the proxy.
        if ( !(m_request.doProxy && m_state.doProxy) )
          closeDown = true;
     }

     if (closeDown)
        httpCloseConnection();
  }

  // Let's update our current state
  m_state.hostname = m_request.hostname;
  m_state.encoded_hostname = m_request.encoded_hostname;
  m_state.port = m_request.port;
  m_state.user = m_request.user;
  m_state.passwd = m_request.passwd;
  m_state.doProxy = m_request.doProxy;
}

bool HTTPProtocol::httpOpenConnection()
{
  int errCode;
  QString errMsg;

  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::httpOpenConnection" << endl;

  setBlockConnection( true );
  // kio_http uses its own proxying:
  KSocks::self()->disableSocks();

  if ( m_state.doProxy )
  {
    QString proxy_host = m_proxyURL.host();
    int proxy_port = m_proxyURL.port();

    kdDebug(7113) << "(" << m_pid << ") Connecting to proxy server: "
                  << proxy_host << ", port: " << proxy_port << endl;

    infoMessage( i18n("Connecting to %1...").arg(m_state.hostname) );

    setConnectTimeout( m_proxyConnTimeout );

    if ( !connectToHost(proxy_host, QString::number( proxy_port ), false) )
    {
      if (userAborted()) {
        error(ERR_NO_CONTENT, "");
        return false;
      }

      switch ( connectResult() )
      {
        case KSocketBase::LookupFailure:
          errMsg = proxy_host;
          errCode = ERR_UNKNOWN_PROXY_HOST;
          break;
        case KSocketBase::Timeout:
          errMsg = i18n("Proxy %1 at port %2").arg(proxy_host).arg(proxy_port);
          errCode = ERR_SERVER_TIMEOUT;
          break;
        default:
          errMsg = i18n("Proxy %1 at port %2").arg(proxy_host).arg(proxy_port);
          errCode = ERR_COULD_NOT_CONNECT;
      }
      error( errCode, errMsg );
      return false;
    }
  }
  else
  {
    // Apparently we don't want a proxy.  let's just connect directly
    setConnectTimeout(m_remoteConnTimeout);

    if ( !connectToHost(m_state.hostname, QString::number( m_state.port ), false ) )
    {
      if (userAborted()) {
        error(ERR_NO_CONTENT, "");
        return false;
      }

      switch ( connectResult() )
      {
/*/        case IO_LookupError:
          errMsg = m_state.hostname;
          errCode = ERR_UNKNOWN_HOST;
          break;
        case IO_TimeOutError:
          errMsg = i18n("Connection was to %1 at port %2").arg(m_state.hostname).arg(m_state.port);
          errCode = ERR_SERVER_TIMEOUT;
          break;*/
        default:
          errCode = ERR_COULD_NOT_CONNECT;
          if (m_state.port != m_iDefaultPort)
            errMsg = i18n("%1 (port %2)").arg(m_state.hostname).arg(m_state.port);
          else
            errMsg = m_state.hostname;
      }
      error( errCode, errMsg );
      return false;
    }
  }

  // Set our special socket option!!
  socket().setNoDelay(true);

  m_bFirstRequest = true;

  connected();
  return true;
}


/**
 * This function is responsible for opening up the connection to the remote
 * HTTP server and sending the header.  If this requires special
 * authentication or other such fun stuff, then it will handle it.  This
 * function will NOT receive anything from the server, however.  This is in
 * contrast to previous incarnations of 'httpOpen'.
 *
 * The reason for the change is due to one small fact: some requests require
 * data to be sent in addition to the header (POST requests) and there is no
 * way for this function to get that data.  This function is called in the
 * slotPut() or slotGet() functions which, in turn, are called (indirectly) as
 * a result of a KIOJob::put() or KIOJob::get().  It is those latter functions
 * which are responsible for starting up this ioslave in the first place.
 * This means that 'httpOpen' is called (essentially) as soon as the ioslave
 * is created -- BEFORE any data gets to this slave.
 *
 * The basic process now is this:
 *
 * 1) Open up the socket and port
 * 2) Format our request/header
 * 3) Send the header to the remote server
 */
bool HTTPProtocol::httpOpen()
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::httpOpen" << endl;

  // Cannot have an https request without the m_bIsSSL being set!  This can
  // only happen if TCPSlaveBase::InitializeSSL() function failed in which it
  // means the current installation does not support SSL...
  if ( (m_protocol == "https" || m_protocol == "webdavs") && !m_bIsSSL )
  {
    error( ERR_UNSUPPORTED_PROTOCOL, m_protocol );
    return false;
  }

  m_request.fcache = 0;
  m_request.bCachedRead = false;
  m_request.bCachedWrite = false;
  m_request.bMustRevalidate = false;
  m_request.expireDate = 0;
  m_request.creationDate = 0;

  if (m_request.bUseCache)
  {
     m_request.fcache = checkCacheEntry( );

     bool bCacheOnly = (m_request.cache == KIO::CC_CacheOnly);
     bool bOffline = isOffline(m_request.doProxy ? m_proxyURL : m_request.url);
     if (bOffline && (m_request.cache != KIO::CC_Reload))
        m_request.cache = KIO::CC_CacheOnly;

     if (m_request.cache == CC_Reload && m_request.fcache)
     {
        if (m_request.fcache)
          fclose(m_request.fcache);
        m_request.fcache = 0;
     }
     if ((m_request.cache == KIO::CC_CacheOnly) || (m_request.cache == KIO::CC_Cache))
        m_request.bMustRevalidate = false;

     m_request.bCachedWrite = true;

     if (m_request.fcache && !m_request.bMustRevalidate)
     {
        // Cache entry is OK.
        m_request.bCachedRead = true; // Cache hit.
        return true;
     }
     else if (!m_request.fcache)
     {
        m_request.bMustRevalidate = false; // Cache miss
     }
     else
     {
        // Conditional cache hit. (Validate)
     }

     if (bCacheOnly)
     {
        error( ERR_DOES_NOT_EXIST, m_request.url.url() );
        return false;
     }
     if (bOffline)
     {
        error( ERR_COULD_NOT_CONNECT, m_request.url.url() );
        return false;
     }
  }

  QString header;
  QString davHeader;

  bool moreData = false;
  bool davData = false;

  // Clear out per-connection settings...
  resetConnectionSettings ();

  // Check the validity of the current connection, if one exists.
  httpCheckConnection();

  if ( !m_bIsTunneled && m_bNeedTunnel )
  {
    setEnableSSLTunnel( true );
    // We send a HTTP 1.0 header since some proxies refuse HTTP 1.1 and we don't
    // need any HTTP 1.1 capabilities for CONNECT - Waba
    header = QString("CONNECT %1:%2 HTTP/1.0"
                     "\r\n").arg( m_request.encoded_hostname).arg(m_request.port);

    // Identify who you are to the proxy server!
    if (!m_request.userAgent.isEmpty())
        header += "User-Agent: " + m_request.userAgent + "\r\n";

    /* Add hostname information */
    header += "Host: " + m_state.encoded_hostname;

    if (m_state.port != m_iDefaultPort)
      header += QString(":%1").arg(m_state.port);
    header += "\r\n";

    header += proxyAuthenticationHeader();
  }
  else
  {
    // Determine if this is a POST or GET method
    switch (m_request.method)
    {
    case HTTP_GET:
        header = "GET ";
        break;
    case HTTP_PUT:
        header = "PUT ";
        moreData = true;
        m_request.bCachedWrite = false; // Do not put any result in the cache
        break;
    case HTTP_POST:
        header = "POST ";
        moreData = true;
        m_request.bCachedWrite = false; // Do not put any result in the cache
        break;
    case HTTP_HEAD:
        header = "HEAD ";
        break;
    case HTTP_DELETE:
        header = "DELETE ";
        m_request.bCachedWrite = false; // Do not put any result in the cache
        break;
    case HTTP_OPTIONS:
        header = "OPTIONS ";
        m_request.bCachedWrite = false; // Do not put any result in the cache
        break;
    case DAV_PROPFIND:
        header = "PROPFIND ";
        davData = true;
        davHeader = "Depth: ";
        if ( hasMetaData( "davDepth" ) )
        {
          kdDebug(7113) << "Reading DAV depth from metadata: " << metaData( "davDepth" ) << endl;
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
        m_request.bCachedWrite = false; // Do not put any result in the cache
        break;
    case DAV_PROPPATCH:
        header = "PROPPATCH ";
        davData = true;
        m_request.bCachedWrite = false; // Do not put any result in the cache
        break;
    case DAV_MKCOL:
        header = "MKCOL ";
        m_request.bCachedWrite = false; // Do not put any result in the cache
        break;
    case DAV_COPY:
    case DAV_MOVE:
        header = ( m_request.method == DAV_COPY ) ? "COPY " : "MOVE ";
        davHeader = "Destination: " + m_request.davData.desturl;
        // infinity depth means copy recursively
        // (optional for copy -> but is the desired action)
        davHeader += "\r\nDepth: infinity\r\nOverwrite: ";
        davHeader += m_request.davData.overwrite ? "T" : "F";
        davHeader += "\r\n";
        m_request.bCachedWrite = false; // Do not put any result in the cache
        break;
    case DAV_LOCK:
        header = "LOCK ";
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
        m_request.bCachedWrite = false; // Do not put any result in the cache
        davData = true;
        break;
    case DAV_UNLOCK:
        header = "UNLOCK ";
        davHeader = "Lock-token: " + metaData("davLockToken") + "\r\n";
        m_request.bCachedWrite = false; // Do not put any result in the cache
        break;
    case DAV_SEARCH:
        header = "SEARCH ";
        davData = true;
        m_request.bCachedWrite = false;
        break;
    case DAV_SUBSCRIBE:
        header = "SUBSCRIBE ";
        m_request.bCachedWrite = false;
        break;
    case DAV_UNSUBSCRIBE:
        header = "UNSUBSCRIBE ";
        m_request.bCachedWrite = false;
        break;
    case DAV_POLL:
        header = "POLL ";
        m_request.bCachedWrite = false;
        break;
    default:
        error (ERR_UNSUPPORTED_ACTION, QString());
        return false;
    }
    // DAV_POLL; DAV_NOTIFY

    // format the URI
    if (m_state.doProxy && !m_bIsTunneled)
    {
      KUrl u;

      if (m_protocol == "webdav")
         u.setProtocol( "http" );
      else if (m_protocol == "webdavs" )
         u.setProtocol( "https" );
      else
         u.setProtocol( m_protocol );

      // For all protocols other than the once handled by this io-slave
      // append the username.  This fixes a long standing bug of ftp io-slave
      // logging in anonymously in proxied connections even when the username
      // is explicitly specified.
      if (m_protocol != "http" && m_protocol != "https" &&
          !m_state.user.isEmpty())
        u.setUser (m_state.user);

      u.setHost( m_state.hostname );
      if (m_state.port != m_iDefaultPort)
         u.setPort( m_state.port );
      u.setEncodedPathAndQuery( m_request.url.encodedPathAndQuery(0,true) );
      header += u.url();
    }
    else
    {
      header += m_request.url.encodedPathAndQuery(0, true);
    }

    header += " HTTP/1.1\r\n"; /* start header */

    // Support old HTTP/1.0 style keep-alive header for compatability
    // purposes as well as performance improvements while giving end
    // users the ability to disable this feature proxy servers that
    // don't not support such feature, e.g. junkbuster proxy server.
    if (!m_bUseProxy || m_bPersistentProxyConnection || m_bIsTunneled)
      header += "Connection: Keep-Alive\r\n";
    else
      header += "Connection: close\r\n";

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

    if ( m_request.offset > 0 )
    {
      header += QString("Range: bytes=%1-\r\n").arg(KIO::number(m_request.offset));
      kdDebug(7103) << "kio_http : Range = " << KIO::number(m_request.offset) << endl;
    }

    if ( m_request.cache == CC_Reload )
    {
      /* No caching for reload */
      header += "Pragma: no-cache\r\n"; /* for HTTP/1.0 caches */
      header += "Cache-control: no-cache\r\n"; /* for HTTP >=1.1 caches */
    }

    if (m_request.bMustRevalidate)
    {
      /* conditional get */
      if (!m_request.etag.isEmpty())
        header += "If-None-Match: "+m_request.etag+"\r\n";
      if (!m_request.lastModified.isEmpty())
        header += "If-Modified-Since: "+m_request.lastModified+"\r\n";
    }

    header += "Accept: ";
    QString acceptHeader = metaData("accept");
    if (!acceptHeader.isEmpty())
      header += acceptHeader;
    else
      header += DEFAULT_ACCEPT_HEADER;
    header += "\r\n";

#ifdef DO_GZIP
    if (m_request.allowCompressedPage)
      header += "Accept-Encoding: x-gzip, x-deflate, gzip, deflate\r\n";
#endif

    if (!m_request.charsets.isEmpty())
      header += "Accept-Charset: " + m_request.charsets + "\r\n";

    if (!m_request.languages.isEmpty())
      header += "Accept-Language: " + m_request.languages + "\r\n";


    /* support for virtual hosts and required by HTTP 1.1 */
    header += "Host: " + m_state.encoded_hostname;

    if (m_state.port != m_iDefaultPort)
      header += QString(":%1").arg(m_state.port);
    header += "\r\n";

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
      if (m_request.bUseCookiejar)
        cookieStr = findCookies( m_request.url.url());
    }

    if (!cookieStr.isEmpty())
      header += cookieStr + "\r\n";

    QString customHeader = metaData( "customHTTPHeader" );
    if (!customHeader.isEmpty())
    {
      header += customHeader;
      header += "\r\n";
    }

    if (m_request.method == HTTP_POST)
    {
      header += metaData("content-type");
      header += "\r\n";
    }

    // Only check for a cached copy if the previous
    // response was NOT a 401 or 407.
    // no caching for Negotiate auth.
    if ( !m_request.bNoAuth && m_responseCode != 401 && m_responseCode != 407 && Authentication != AUTH_Negotiate )
    {
      kdDebug(7113) << "(" << m_pid << ") Calling checkCachedAuthentication " << endl;
      AuthInfo info;
      info.url = m_request.url;
      info.verifyPath = true;
      if ( !m_request.user.isEmpty() )
        info.username = m_request.user;
      if ( checkCachedAuthentication( info ) && !info.digestInfo.isEmpty() )
      {
        Authentication = info.digestInfo.startsWith("Basic") ? AUTH_Basic : info.digestInfo.startsWith("NTLM") ? AUTH_NTLM : info.digestInfo.startsWith("Negotiate") ? AUTH_Negotiate : AUTH_Digest ;
        m_state.user   = info.username;
        m_state.passwd = info.password;
        m_strRealm = info.realmValue;
        if ( Authentication != AUTH_NTLM && Authentication != AUTH_Negotiate ) // don't use the cached challenge
          m_strAuthorization = info.digestInfo;
      }
    }
    else
    {
      kdDebug(7113) << "(" << m_pid << ") Not calling checkCachedAuthentication " << endl;
    }

    switch ( Authentication )
    {
      case AUTH_Basic:
          header += createBasicAuth();
          break;
      case AUTH_Digest:
          header += createDigestAuth();
          break;
#ifdef HAVE_LIBGSSAPI
      case AUTH_Negotiate:
          header += createNegotiateAuth();
          break;
#endif
      case AUTH_NTLM:
          header += createNTLMAuth();
          break;
      case AUTH_None:
      default:
          break;
    }

    /********* Only for debugging purpose *********/
    if ( Authentication != AUTH_None )
    {
      kdDebug(7113) << "(" << m_pid << ") Using Authentication: " << endl;
      kdDebug(7113) << "(" << m_pid << ")   HOST= " << m_state.hostname << endl;
      kdDebug(7113) << "(" << m_pid << ")   PORT= " << m_state.port << endl;
      kdDebug(7113) << "(" << m_pid << ")   USER= " << m_state.user << endl;
      kdDebug(7113) << "(" << m_pid << ")   PASSWORD= [protected]" << endl;
      kdDebug(7113) << "(" << m_pid << ")   REALM= " << m_strRealm << endl;
      kdDebug(7113) << "(" << m_pid << ")   EXTRA= " << m_strAuthorization << endl;
    }

    // Do we need to authorize to the proxy server ?
    if ( m_state.doProxy && !m_bIsTunneled )
    {
      if ( m_bPersistentProxyConnection )
	header += "Proxy-Connection: Keep-Alive\r\n";

      header += proxyAuthenticationHeader();
    }

    if ( m_protocol == "webdav" || m_protocol == "webdavs" )
    {
      header += davProcessLocks();

      // add extra webdav headers, if supplied
      QString davExtraHeader = metaData("davHeader");
      if ( !davExtraHeader.isEmpty() )
        davHeader += davExtraHeader;

      // Set content type of webdav data
      if (davData)
        davHeader += "Content-Type: text/xml; charset=utf-8\r\n";

      // add extra header elements for WebDAV
      if ( !davHeader.isNull() )
        header += davHeader;
    }
  }

  kdDebug(7103) << "(" << m_pid << ") ============ Sending Header:" << endl;

  QStringList headerOutput = header.split("\r\n", QString::SkipEmptyParts);
  QStringList::Iterator it = headerOutput.begin();

  for (; it != headerOutput.end(); it++)
    kdDebug(7103) << "(" << m_pid << ") " << (*it) << endl;

  if ( !moreData && !davData)
    header += "\r\n";  /* end header */

  // Now that we have our formatted header, let's send it!
  // Create a new connection to the remote machine if we do
  // not already have one...
  if ( !isConnectionValid() )
  {
    if (!httpOpenConnection())
    {
       kdDebug(7113) << "Couldn't connect, oopsie!" << endl;
       return false;
    }
  }

  // Send the data to the remote machine...
  bool sendOk = (write(header.toLatin1(), header.length()) == (ssize_t) header.length());
  if (!sendOk)
  {
    kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::httpOpen: "
                     "Connection broken! (" << m_state.hostname << ")" << endl;

    // With a Keep-Alive connection this can happen.
    // Just reestablish the connection.
    if (m_bKeepAlive)
    {
       httpCloseConnection();
       return true; // Try again
    }

    if (!sendOk)
    {
       kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::httpOpen: sendOk==false."
                        " Connnection broken !" << endl;
       error( ERR_CONNECTION_BROKEN, m_state.hostname );
       return false;
    }
  }
  else
      kdDebug(7113) << "sent it!" << endl;

  bool res = true;

  if ( moreData || davData )
    res = sendBody();

  infoMessage(i18n("%1 contacted. Waiting for reply...").arg(m_request.hostname));

  return res;
}

void HTTPProtocol::forwardHttpResponseHeader()
{
  // Send the response header if it was requested
  if ( config()->readEntry("PropagateHttpHeader", false) )
  {
    setMetaData("HTTP-Headers", m_responseHeader.join("\n"));
    sendMetaData();
  }
  m_responseHeader.clear();
}

/**
 * This function will read in the return header from the server.  It will
 * not read in the body of the return message.  It will also not transmit
 * the header to our client as the client doesn't need to know the gory
 * details of HTTP headers.
 */
bool HTTPProtocol::readHeader()
{
try_again:
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::readHeader" << endl;

  // Check
  if (m_request.bCachedRead)
  {
     m_responseHeader << "HTTP-CACHE";
     // Read header from cache...
     char buffer[4097];
     if (!fgets(buffer, 4096, m_request.fcache) )
     {
        // Error, delete cache entry
        kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::readHeader: "
                      << "Could not access cache to obtain mimetype!" << endl;
        error( ERR_CONNECTION_BROKEN, m_state.hostname );
        return false;
     }

     m_strMimeType = QString::fromUtf8( buffer).trimmed();

     kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::readHeader: cached "
                   << "data mimetype: " << m_strMimeType << endl;

     if (!fgets(buffer, 4096, m_request.fcache) )
     {
        // Error, delete cache entry
        kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::readHeader: "
                      << "Could not access cached data! " << endl;
        error( ERR_CONNECTION_BROKEN, m_state.hostname );
        return false;
     }

     m_request.strCharset = QString::fromUtf8( buffer).trimmed().toLower();
     setMetaData("charset", m_request.strCharset);
     if (!m_request.lastModified.isEmpty())
         setMetaData("modified", m_request.lastModified);
     QString tmp;
     tmp.setNum(m_request.expireDate);
     setMetaData("expire-date", tmp);
     tmp.setNum(m_request.creationDate);
     setMetaData("cache-creation-date", tmp);
     mimeType(m_strMimeType);
     forwardHttpResponseHeader();
     return true;
  }

  QByteArray locationStr; // In case we get a redirect.
  Q3CString cookieStr; // In case we get a cookie.

  QString disposition; // Incase we get a Content-Disposition
  QString mediaValue;
  QString mediaAttribute;

  QStringList upgradeOffers;

  bool upgradeRequired = false;   // Server demands that we upgrade to something
                                  // This is also true if we ask to upgrade and
                                  // the server accepts, since we are now
                                  // committed to doing so
  bool canUpgrade = false;        // The server offered an upgrade


  m_request.etag.clear();
  m_request.lastModified.clear();
  m_request.strCharset.clear();

  time_t dateHeader = 0;
  time_t expireDate = 0; // 0 = no info, 1 = already expired, > 1 = actual date
  int currentAge = 0;
  int maxAge = -1; // -1 = no max age, 0 already expired, > 0 = actual time
  int maxHeaderSize = 64*1024; // 64Kb to catch DOS-attacks

  // read in 4096 bytes at a time (HTTP cookies can be quite large.)
  int len = 0;
  char buffer[4097];
  bool cont = false;
  bool cacheValidated = false; // Revalidation was successful
  bool mayCache = true;
  bool hasCacheDirective = false;
  bool bCanResume = false;

  if ( !isConnectionValid() )
  {
     kdDebug(7113) << "HTTPProtocol::readHeader: No connection." << endl;
     return false; // Restablish connection and try again
  }

  if (!waitForResponse(m_remoteRespTimeout))
  {
     // No response error
     error( ERR_SERVER_TIMEOUT , m_state.hostname );
     return false;
  }

  setRewindMarker();

  gets(buffer, sizeof(buffer)-1);

  if (m_bEOF || *buffer == '\0')
  {
    kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::readHeader: "
                  << "EOF while waiting for header start." << endl;
    if (m_bKeepAlive) // Try to reestablish connection.
    {
      httpCloseConnection();
      return false; // Reestablish connection and try again.
    }

    if (m_request.method == HTTP_HEAD)
    {
      // HACK
      // Some web-servers fail to respond properly to a HEAD request.
      // We compensate for their failure to properly implement the HTTP standard
      // by assuming that they will be sending html.
      kdDebug(7113) << "(" << m_pid << ") HTTPPreadHeader: HEAD -> returned "
                    << "mimetype: " << DEFAULT_MIME_TYPE << endl;
      mimeType(QString::fromLatin1(DEFAULT_MIME_TYPE));
      return true;
    }

    kdDebug(7113) << "HTTPProtocol::readHeader: Connection broken !" << endl;
    error( ERR_CONNECTION_BROKEN, m_state.hostname );
    return false;
  }

  kdDebug(7103) << "(" << m_pid << ") ============ Received Response:"<< endl;

  bool noHeader = true;
  HTTP_REV httpRev = HTTP_None;
  int headerSize = 0;

  do
  {
    // strip off \r and \n if we have them
    len = strlen(buffer);

    while(len && (buffer[len-1] == '\n' || buffer[len-1] == '\r'))
      buffer[--len] = 0;

    // if there was only a newline then continue
    if (!len)
    {
      kdDebug(7103) << "(" << m_pid << ") --empty--" << endl;
      continue;
    }

    headerSize += len;

    // We have a response header.  This flag is a work around for
    // servers that append a "\r\n" before the beginning of the HEADER
    // response!!!  It only catches x number of \r\n being placed at the
    // top of the reponse...
    noHeader = false;

    kdDebug(7103) << "(" << m_pid << ") \"" << buffer << "\"" << endl;

    // Save broken servers from damnation!!
    char* buf = buffer;
    while( *buf == ' ' )
        buf++;


    if (buf[0] == '<')
    {
      // We get XML / HTTP without a proper header
      // put string back
      kdDebug(7103) << "kio_http: No valid HTTP header found! Document starts with XML/HTML tag" << endl;

      // Document starts with a tag, assume html instead of text/plain
      m_strMimeType = "text/html";

      rewind();
      break;
    }

    // Store the the headers so they can be passed to the
    // calling application later
    m_responseHeader << QString::fromLatin1(buf);

    if ((strncasecmp(buf, "HTTP", 4) == 0) ||
        (strncasecmp(buf, "ICY ", 4) == 0)) // Shoutcast support
    {
      if (strncasecmp(buf, "ICY ", 4) == 0)
      {
        // Shoutcast support
        httpRev = SHOUTCAST;
        m_bKeepAlive = false;
      }
      else if (strncmp((buf + 5), "1.0",3) == 0)
      {
        httpRev = HTTP_10;
        // For 1.0 servers, the server itself has to explicitly
        // tell us whether it supports persistent connection or
        // not.  By default, we assume it does not, but we do
        // send the old style header "Connection: Keep-Alive" to
        // inform it that we support persistence.
        m_bKeepAlive = false;
      }
      else if (strncmp((buf + 5), "1.1",3) == 0)
      {
        httpRev = HTTP_11;
      }
      else
      {
        httpRev = HTTP_Unknown;
      }

      if (m_responseCode)
        m_prevResponseCode = m_responseCode;

      const char* rptr = buf;
      while ( *rptr && *rptr > ' ' )
          ++rptr;
      m_responseCode = atoi(rptr);

      // server side errors
      if (m_responseCode >= 500 && m_responseCode <= 599)
      {
        if (m_request.method == HTTP_HEAD)
        {
           ; // Ignore error
        }
        else
        {
           if (m_request.bErrorPage)
              errorPage();
           else
           {
              error(ERR_INTERNAL_SERVER, m_request.url.url());
              return false;
           }
        }
        m_request.bCachedWrite = false; // Don't put in cache
        mayCache = false;
      }
      // Unauthorized access
      else if (m_responseCode == 401 || m_responseCode == 407)
      {
        // Double authorization requests, i.e. a proxy auth
        // request followed immediately by a regular auth request.
        if ( m_prevResponseCode != m_responseCode &&
            (m_prevResponseCode == 401 || m_prevResponseCode == 407) )
          saveAuthorization();

        m_bUnauthorized = true;
        m_request.bCachedWrite = false; // Don't put in cache
        mayCache = false;
      }
      //
      else if (m_responseCode == 416) // Range not supported
      {
        m_request.offset = 0;
        httpCloseConnection();
        return false; // Try again.
      }
      // Upgrade Required
      else if (m_responseCode == 426)
      {
        upgradeRequired = true;
      }
      // Any other client errors
      else if (m_responseCode >= 400 && m_responseCode <= 499)
      {
        // Tell that we will only get an error page here.
        if (m_request.bErrorPage)
          errorPage();
        else
        {
          error(ERR_DOES_NOT_EXIST, m_request.url.url());
          return false;
        }
        m_request.bCachedWrite = false; // Don't put in cache
        mayCache = false;
      }
      else if (m_responseCode == 307)
      {
        // 307 Temporary Redirect
        m_request.bCachedWrite = false; // Don't put in cache
        mayCache = false;
      }
      else if (m_responseCode == 304)
      {
        // 304 Not Modified
        // The value in our cache is still valid.
        cacheValidated = true;
      }
      else if (m_responseCode >= 301 && m_responseCode<= 303)
      {
        // 301 Moved permanently
        if (m_responseCode == 301)
           setMetaData("permanent-redirect", "true");

        // 302 Found (temporary location)
        // 303 See Other
        if (m_request.method != HTTP_HEAD && m_request.method != HTTP_GET)
        {
#if 0
           // Reset the POST buffer to avoid a double submit
           // on redirection
           if (m_request.method == HTTP_POST)
              m_bufPOST.resize(0);
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
        m_request.bCachedWrite = false; // Don't put in cache
        mayCache = false;
      }
      else if ( m_responseCode == 207 ) // Multi-status (for WebDav)
      {

      }
      else if ( m_responseCode == 204 ) // No content
      {
        // error(ERR_NO_CONTENT, i18n("Data have been successfully sent."));
        // Short circuit and do nothing!

        // The original handling here was wrong, this is not an error: eg. in the
        // example of a 204 No Content response to a PUT completing.
        // m_bError = true;
        // return false;
      }
      else if ( m_responseCode == 206 )
      {
        if ( m_request.offset )
          bCanResume = true;
      }
      else if (m_responseCode == 102) // Processing (for WebDAV)
      {
        /***
         * This status code is given when the server expects the
         * command to take significant time to complete. So, inform
         * the user.
         */
        infoMessage( i18n( "Server processing request, please wait..." ) );
        cont = true;
      }
      else if (m_responseCode == 100)
      {
        // We got 'Continue' - ignore it
        cont = true;
      }
    }

    // are we allowd to resume?  this will tell us
    else if (strncasecmp(buf, "Accept-Ranges:", 14) == 0) {
      if (strncasecmp(trimLead(buf + 14), "none", 4) == 0)
            bCanResume = false;
    }
    // Keep Alive
    else if (strncasecmp(buf, "Keep-Alive:", 11) == 0) {
      QStringList options = QString::fromLatin1(trimLead(buf+11)).
          split(',',QString::SkipEmptyParts);
      for(QStringList::ConstIterator it = options.begin();
          it != options.end();
          it++)
      {
         QString option = (*it).trimmed().toLower();
         if (option.startsWith("timeout="))
         {
            m_keepAliveTimeout = option.mid(8).toInt();
         }
      }
    }

    // Cache control
    else if (strncasecmp(buf, "Cache-Control:", 14) == 0) {
      QStringList cacheControls = QString::fromLatin1(trimLead(buf+14)).
          split(',',QString::SkipEmptyParts);
      for(QStringList::ConstIterator it = cacheControls.begin();
          it != cacheControls.end();
          it++)
      {
         QString cacheControl = (*it).trimmed();
         if (strncasecmp(cacheControl.toLatin1(), "no-cache", 8) == 0)
         {
            m_request.bCachedWrite = false; // Don't put in cache
            mayCache = false;
         }
         else if (strncasecmp(cacheControl.toLatin1(), "no-store", 8) == 0)
         {
            m_request.bCachedWrite = false; // Don't put in cache
            mayCache = false;
         }
         else if (strncasecmp(cacheControl.toLatin1(), "max-age=", 8) == 0)
         {
            QString age = cacheControl.mid(8).trimmed();
            if (!age.isNull())
              maxAge = STRTOLL(age.toLatin1(), 0, 10);
         }
      }
      hasCacheDirective = true;
    }

    // get the size of our data
    else if (strncasecmp(buf, "Content-length:", 15) == 0) {
      char* len = trimLead(buf + 15);
      if (len)
        m_iSize = STRTOLL(len, 0, 10);
    }

    else if (strncasecmp(buf, "Content-location:", 17) == 0) {
      setMetaData ("content-location",
                   QString::fromLatin1(trimLead(buf+17)).trimmed());
    }

    // what type of data do we have?
    else if (strncasecmp(buf, "Content-type:", 13) == 0) {
      char *start = trimLead(buf + 13);
      char *pos = start;

      // Increment until we encounter ";" or the end of the buffer
      while ( *pos && *pos != ';' )  pos++;

      // Assign the mime-type.
      m_strMimeType = QString::fromLatin1(start, pos-start).trimmed().toLower();
      kdDebug(7113) << "(" << m_pid << ") Content-type: " << m_strMimeType << endl;

      // If we still have text, then it means we have a mime-type with a
      // parameter (eg: charset=iso-8851) ; so let's get that...
      while (*pos)
      {
        start = ++pos;
        while ( *pos && *pos != '=' )  pos++;

	char *end = pos;
	while ( *end && *end != ';' )  end++;

        if (*pos)
        {
          mediaAttribute = QString::fromLatin1(start, pos-start).trimmed().toLower();
          mediaValue = QString::fromLatin1(pos+1, end-pos-1).trimmed();
	  pos = end;
          if (mediaValue.length() &&
              (mediaValue[0] == '"') &&
              (mediaValue[mediaValue.length()-1] == '"'))
             mediaValue = mediaValue.mid(1, mediaValue.length()-2);

          kdDebug (7113) << "(" << m_pid << ") Media-Parameter Attribute: "
                         << mediaAttribute << endl;
          kdDebug (7113) << "(" << m_pid << ") Media-Parameter Value: "
                         << mediaValue << endl;

          if ( mediaAttribute == "charset")
          {
            mediaValue = mediaValue.toLower();
            m_request.strCharset = mediaValue;
          }
          else
          {
            setMetaData("media-"+mediaAttribute, mediaValue);
          }
        }
      }
    }

    // Date
    else if (strncasecmp(buf, "Date:", 5) == 0) {
      dateHeader = KRFCDate::parseDate(trimLead(buf+5));
    }

    // Cache management
    else if (strncasecmp(buf, "ETag:", 5) == 0) {
      m_request.etag = trimLead(buf+5);
    }

    // Cache management
    else if (strncasecmp(buf, "Expires:", 8) == 0) {
      expireDate = KRFCDate::parseDate(trimLead(buf+8));
      if (!expireDate)
        expireDate = 1; // Already expired
    }

    // Cache management
    else if (strncasecmp(buf, "Last-Modified:", 14) == 0) {
      m_request.lastModified = (QString::fromLatin1(trimLead(buf+14))).trimmed();
    }

    // whoops.. we received a warning
    else if (strncasecmp(buf, "Warning:", 8) == 0) {
      //Don't use warning() here, no need to bother the user.
      //Those warnings are mostly about caches.
      infoMessage(trimLead(buf + 8));
    }

    // Cache management (HTTP 1.0)
    else if (strncasecmp(buf, "Pragma:", 7) == 0) {
      QByteArray pragma = QByteArray(trimLead(buf+7)).trimmed().toLower();
      if (pragma == "no-cache")
      {
         m_request.bCachedWrite = false; // Don't put in cache
         mayCache = false;
         hasCacheDirective = true;
      }
    }

    // The deprecated Refresh Response
    else if (strncasecmp(buf,"Refresh:", 8) == 0) {
      mayCache = false;  // Do not cache page as it defeats purpose of Refresh tag!
      setMetaData( "http-refresh", QString::fromLatin1(trimLead(buf+8)).trimmed() );
    }

    // In fact we should do redirection only if we got redirection code
    else if (strncasecmp(buf, "Location:", 9) == 0) {
      // Redirect only for 3xx status code, will ya! Thanks, pal!
      if ( m_responseCode > 299 && m_responseCode < 400 )
        locationStr = QByteArray(trimLead(buf+9)).trimmed();
    }

    // Check for cookies
    else if (strncasecmp(buf, "Set-Cookie", 10) == 0) {
      cookieStr += buf;
      cookieStr += '\n';
    }

    // check for direct authentication
    else if (strncasecmp(buf, "WWW-Authenticate:", 17) == 0) {
      configAuth(trimLead(buf + 17), false);
    }

    // check for proxy-based authentication
    else if (strncasecmp(buf, "Proxy-Authenticate:", 19) == 0) {
      configAuth(trimLead(buf + 19), true);
    }

    else if (strncasecmp(buf, "Upgrade:", 8) == 0) {
       // Now we have to check to see what is offered for the upgrade
       QString offered = &(buf[8]);
       upgradeOffers = offered.split(QRegExp("[ \n,\r\t]"), QString::SkipEmptyParts);
    }

    // content?
    else if (strncasecmp(buf, "Content-Encoding:", 17) == 0) {
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
      addEncoding(trimLead(buf + 17), m_qContentEncodings);
    }
    // Refer to RFC 2616 sec 15.5/19.5.1 and RFC 2183
    else if(strncasecmp(buf, "Content-Disposition:", 20) == 0) {
      char* dispositionBuf = trimLead(buf + 20);
      while ( *dispositionBuf )
      {
        if ( strncasecmp( dispositionBuf, "filename", 8 ) == 0 )
        {
          dispositionBuf += 8;

          while ( *dispositionBuf == ' ' || *dispositionBuf == '=' )
            dispositionBuf++;

          char* bufStart = dispositionBuf;

          while ( *dispositionBuf && *dispositionBuf != ';' )
            dispositionBuf++;

          if ( dispositionBuf > bufStart )
          {
            // Skip any leading quotes...
            while ( *bufStart == '"' )
              bufStart++;

            // Skip any trailing quotes as well as white spaces...
            while ( *(dispositionBuf-1) == ' ' || *(dispositionBuf-1) == '"')
              dispositionBuf--;

            if ( dispositionBuf > bufStart )
              disposition = QString::fromLatin1( bufStart, dispositionBuf-bufStart );

            break;
          }
        }
        else
        {
          char *bufStart = dispositionBuf;

          while ( *dispositionBuf && *dispositionBuf != ';' )
            dispositionBuf++;

          if ( dispositionBuf > bufStart )
            disposition = QString::fromLatin1( bufStart, dispositionBuf-bufStart ).trimmed();

          while ( *dispositionBuf == ';' || *dispositionBuf == ' ' )
            dispositionBuf++;
        }
      }

      // Content-Dispostion is not allowed to dictate directory
      // path, thus we extract the filename only.
      if ( !disposition.isEmpty() )
      {
        int pos = disposition.lastIndexOf( '/' );

        if( pos > -1 )
          disposition = disposition.mid(pos+1);

        kdDebug(7113) << "(" << m_pid << ") Content-Disposition: "
                      << disposition<< endl;
      }
    }
    else if (strncasecmp(buf, "Proxy-Connection:", 17) == 0)
    {
      if (strncasecmp(trimLead(buf + 17), "Close", 5) == 0)
        m_bKeepAlive = false;
      else if (strncasecmp(trimLead(buf + 17), "Keep-Alive", 10)==0)
        m_bKeepAlive = true;
    }
    else if (strncasecmp(buf, "Link:", 5) == 0) {
      // We only support Link: <url>; rel="type"   so far
      QStringList link = QString(buf).replace(QRegExp("^Link:[ ]*"),"").
          split(';',QString::SkipEmptyParts);
      if (link.count() == 2) {
        QString rel = link[1].trimmed();
        if (rel.startsWith("rel=\"")) {
          rel = rel.mid(5, rel.length() - 6);
          if (rel.toLower() == "pageservices") {
            QString url = link[0].replace(QRegExp("[<>]"),"").trimmed();
            setMetaData("PageServices", url);
          }
        }
      }
    }
    else if (strncasecmp(buf, "P3P:", 4) == 0) {
      QString p3pstr = buf;
      p3pstr = p3pstr.mid(4).simplified();
      QStringList policyrefs, compact;
      QStringList policyfields = p3pstr.split(QRegExp(",[ ]*"), QString::SkipEmptyParts);
      for (QStringList::Iterator it = policyfields.begin();
                                  it != policyfields.end();
                                                      ++it) {
         QStringList policy = (*it).split('=',QString::SkipEmptyParts);

         if (policy.count() == 2) {
            if (policy[0].toLower() == "policyref") {
               policyrefs << policy[1].replace(QRegExp("[\"\']"), "")
                                      .trimmed();
            } else if (policy[0].toLower() == "cp") {
               // We convert to cp\ncp\ncp\n[...]\ncp to be consistent with
               // other metadata sent in strings.  This could be a bit more
               // efficient but I'm going for correctness right now.
               QStringList cps = policy[1].replace(QRegExp("[\"\']"), "")
                   .simplified().split(' ',QString::SkipEmptyParts);

               for (QStringList::Iterator j = cps.begin(); j != cps.end(); ++j)
                 compact << *j;
            }
         }
      }

      if (!policyrefs.isEmpty())
         setMetaData("PrivacyPolicy", policyrefs.join("\n"));

      if (!compact.isEmpty())
         setMetaData("PrivacyCompactPolicy", compact.join("\n"));
    }

    // continue only if we know that we're HTTP/1.1
    else if (httpRev == HTTP_11) {
      // let them tell us if we should stay alive or not
      if (strncasecmp(buf, "Connection:", 11) == 0)
      {
        if (strncasecmp(trimLead(buf + 11), "Close", 5) == 0)
          m_bKeepAlive = false;
        else if (strncasecmp(trimLead(buf + 11), "Keep-Alive", 10)==0)
          m_bKeepAlive = true;
        else if (strncasecmp(trimLead(buf + 11), "Upgrade", 7)==0)
        {
          if (m_responseCode == 101) {
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
      else if (strncasecmp(buf, "Transfer-Encoding:", 18) == 0) {
        // If multiple encodings have been applied to an entity, the
        // transfer-codings MUST be listed in the order in which they
        // were applied.
        addEncoding(trimLead(buf + 18), m_qTransferEncodings);
      }

      // md5 signature
      else if (strncasecmp(buf, "Content-MD5:", 12) == 0) {
        m_sContentMD5 = QString::fromLatin1(trimLead(buf + 12));
      }

      // *** Responses to the HTTP OPTIONS method follow
      // WebDAV capabilities
      else if (strncasecmp(buf, "DAV:", 4) == 0) {
        if (m_davCapabilities.isEmpty()) {
          m_davCapabilities << QString::fromLatin1(trimLead(buf + 4));
        }
        else {
          m_davCapabilities << QString::fromLatin1(trimLead(buf + 4));
        }
      }
      // *** Responses to the HTTP OPTIONS method finished
    }
    else if ((httpRev == HTTP_None) && (strlen(buf) != 0))
    {
      // Remote server does not seem to speak HTTP at all
      // Put the crap back into the buffer and hope for the best
      rewind();
      if (m_responseCode)
        m_prevResponseCode = m_responseCode;

      m_responseCode = 200; // Fake it
      httpRev = HTTP_Unknown;
      m_bKeepAlive = false;
      break;
    }
    setRewindMarker();

    // Clear out our buffer for further use.
    memset(buffer, 0, sizeof(buffer));

  } while (!m_bEOF && (len || noHeader) && (headerSize < maxHeaderSize) && (gets(buffer, sizeof(buffer)-1)));


  // Now process the HTTP/1.1 upgrade
  QStringList::Iterator opt = upgradeOffers.begin();
  for( ; opt != upgradeOffers.end(); ++opt) {
     if (*opt == "TLS/1.0") {
        if(upgradeRequired) {
           if (!startTLS() && !usingTLS()) {
              error(ERR_UPGRADE_REQUIRED, *opt);
              return false;
           }
        }
     } else if (*opt == "HTTP/1.1") {
        httpRev = HTTP_11;
     } else {
        // unknown
        if (upgradeRequired) {
           error(ERR_UPGRADE_REQUIRED, *opt);
           return false;
        }
     }
  }

  setMetaData("charset", m_request.strCharset);

  // If we do not support the requested authentication method...
  if ( (m_responseCode == 401 && Authentication == AUTH_None) ||
       (m_responseCode == 407 && ProxyAuthentication == AUTH_None) )
  {
    m_bUnauthorized = false;
    if (m_request.bErrorPage)
      errorPage();
    else
    {
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
    if (!m_request.lastModified.isEmpty())
       lastModifiedDate = KRFCDate::parseDate(m_request.lastModified);

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
    if ((m_request.cookieMode == HTTPRequest::CookiesAuto) && m_request.bUseCookiejar)
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

  if (m_request.bMustRevalidate)
  {
    m_request.bMustRevalidate = false; // Reset just in case.
    if (cacheValidated)
    {
      // Yippie, we can use the cached version.
      // Update the cache with new "Expire" headers.
      fclose(m_request.fcache);
      m_request.fcache = 0;
      updateExpireDate( expireDate, true );
      m_request.fcache = checkCacheEntry( ); // Re-read cache entry

      if (m_request.fcache)
      {
          m_request.bCachedRead = true;
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
       fclose(m_request.fcache);
       m_request.fcache = 0;
     }
  }

  // We need to reread the header if we got a '100 Continue' or '102 Processing'
  if ( cont )
  {
    goto try_again;
  }

  // Do not do a keep-alive connection if the size of the
  // response is not known and the response is not Chunked.
  if (!m_bChunked && (m_iSize == NO_SIZE))
    m_bKeepAlive = false;

  if ( m_responseCode == 204 )
  {
    return true;
  }

  // We need to try to login again if we failed earlier
  if ( m_bUnauthorized )
  {
    if ( (m_responseCode == 401) ||
         (m_bUseProxy && (m_responseCode == 407))
       )
    {
        if ( getAuthorization() )
        {
           // for NTLM Authentication we have to keep the connection open!
           if ( Authentication == AUTH_NTLM && m_strAuthorization.length() > 4 )
           {
             m_bKeepAlive = true;
             readBody( true );
           }
           else if (ProxyAuthentication == AUTH_NTLM && m_strProxyAuthorization.length() > 4)
           {
            readBody( true );
           }
           else
             httpCloseConnection();
           return false; // Try again.
        }

        if (m_bError)
           return false; // Error out

        // Show error page...
    }
    m_bUnauthorized = false;
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
    m_bRedirect = true;

    if (!m_request.id.isEmpty())
    {
       sendMetaData();
    }

    kdDebug(7113) << "(" << m_pid << ") request.url: " << m_request.url.url()
                  << endl << "LocationStr: " << locationStr.data() << endl;

    kdDebug(7113) << "(" << m_pid << ") Requesting redirection to: " << u.url()
                  << endl;

    // If we're redirected to a http:// url, remember that we're doing webdav...
    if (m_protocol == "webdav" || m_protocol == "webdavs")
      u.setProtocol(m_protocol);

    redirection(u);
    m_request.bCachedWrite = false; // Turn off caching on re-direction (DA)
    mayCache = false;
  }

  // Inform the job that we can indeed resume...
  if ( bCanResume && m_request.offset )
    canResume();
  else
    m_request.offset = 0;

  // We don't cache certain text objects
  if (m_strMimeType.startsWith("text/") &&
      (m_strMimeType != "text/css") &&
      (m_strMimeType != "text/x-javascript") &&
      !hasCacheDirective)
  {
     // Do not cache secure pages or pages
     // originating from password protected sites
     // unless the webserver explicitly allows it.
     if ( m_bIsSSL || (Authentication != AUTH_None) )
     {
        m_request.bCachedWrite = false;
        mayCache = false;
     }
  }

  // WABA: Correct for tgz files with a gzip-encoding.
  // They really shouldn't put gzip in the Content-Encoding field!
  // Web-servers really shouldn't do this: They let Content-Size refer
  // to the size of the tgz file, not to the size of the tar file,
  // while the Content-Type refers to "tar" instead of "tgz".
  if (!m_qContentEncodings.isEmpty() && m_qContentEncodings.last() == "gzip")
  {
     if (m_strMimeType == "application/x-tar")
     {
        m_qContentEncodings.removeLast();
        m_strMimeType = QString::fromLatin1("application/x-tgz");
     }
     else if (m_strMimeType == "application/postscript")
     {
        // LEONB: Adding another exception for psgz files.
        // Could we use the mimelnk files instead of hardcoding all this?
        m_qContentEncodings.removeLast();
        m_strMimeType = QString::fromLatin1("application/x-gzpostscript");
     }
     else if ( (m_request.allowCompressedPage &&
                m_strMimeType == "text/html")
                ||
               (m_request.allowCompressedPage &&
                m_strMimeType != "application/x-tgz" &&
                m_strMimeType != "application/x-targz" &&
                m_strMimeType != "application/x-gzip" &&
                m_request.url.path().right(3) != ".gz")
                )
     {
        // Unzip!
     }
     else
     {
        m_qContentEncodings.removeLast();
        m_strMimeType = QString::fromLatin1("application/x-gzip");
     }
  }

  // We can't handle "bzip2" encoding (yet). So if we get something with
  // bzip2 encoding, we change the mimetype to "application/x-bzip2".
  // Note for future changes: some web-servers send both "bzip2" as
  //   encoding and "application/x-bzip2" as mimetype. That is wrong.
  //   currently that doesn't bother us, because we remove the encoding
  //   and set the mimetype to x-bzip2 anyway.
  if (!m_qContentEncodings.isEmpty() && m_qContentEncodings.last() == "bzip2")
  {
     m_qContentEncodings.removeLast();
     m_strMimeType = QString::fromLatin1("application/x-bzip2");
  }

  // Convert some common mimetypes to standard KDE mimetypes
  if (m_strMimeType == "application/x-targz")
     m_strMimeType = QString::fromLatin1("application/x-tgz");
  else if (m_strMimeType == "application/zip")
     m_strMimeType = QString::fromLatin1("application/x-zip");
  else if (m_strMimeType == "image/x-png")
     m_strMimeType = QString::fromLatin1("image/png");
  else if (m_strMimeType == "image/bmp")
     m_strMimeType = QString::fromLatin1("image/x-bmp");
  else if (m_strMimeType == "audio/mpeg" || m_strMimeType == "audio/x-mpeg" || m_strMimeType == "audio/mp3")
     m_strMimeType = QString::fromLatin1("audio/x-mp3");
  else if (m_strMimeType == "audio/microsoft-wave")
     m_strMimeType = QString::fromLatin1("audio/x-wav");
  else if (m_strMimeType == "audio/midi")
     m_strMimeType = QString::fromLatin1("audio/x-midi");
  else if (m_strMimeType == "image/x-xpixmap")
     m_strMimeType = QString::fromLatin1("image/x-xpm");
  else if (m_strMimeType == "application/rtf")
     m_strMimeType = QString::fromLatin1("text/rtf");

  // Crypto ones....
  else if (m_strMimeType == "application/pkix-cert" ||
           m_strMimeType == "application/binary-certificate")
  {
     m_strMimeType = QString::fromLatin1("application/x-x509-ca-cert");
  }

  // Prefer application/x-tgz or x-gzpostscript over application/x-gzip.
  else if (m_strMimeType == "application/x-gzip")
  {
     if ((m_request.url.path().right(7) == ".tar.gz") ||
         (m_request.url.path().right(4) == ".tar"))
        m_strMimeType = QString::fromLatin1("application/x-tgz");
     if ((m_request.url.path().right(6) == ".ps.gz"))
        m_strMimeType = QString::fromLatin1("application/x-gzpostscript");
  }

  // Some webservers say "text/plain" when they mean "application/x-bzip2"
  else if ((m_strMimeType == "text/plain") || (m_strMimeType == "application/octet-stream"))
  {
     QString ext = m_request.url.path().right(4).toUpper();
     if (ext == ".BZ2")
        m_strMimeType = QString::fromLatin1("application/x-bzip2");
     else if (ext == ".PEM")
        m_strMimeType = QString::fromLatin1("application/x-x509-ca-cert");
     else if (ext == ".SWF")
        m_strMimeType = QString::fromLatin1("application/x-shockwave-flash");
     else if (ext == ".PLS")
        m_strMimeType = QString::fromLatin1("audio/x-scpls");
     else if (ext == ".WMV")
        m_strMimeType = QString::fromLatin1("video/x-ms-wmv");
  }

#if 0
  // Even if we can't rely on content-length, it seems that we should
  // never get more data than content-length. Maybe less, if the
  // content-length refers to the unzipped data.
  if (!m_qContentEncodings.isEmpty())
  {
     // If we still have content encoding we can't rely on the Content-Length.
     m_iSize = NO_SIZE;
  }
#endif

  if( !disposition.isEmpty() )
  {
    kdDebug(7113) << "(" << m_pid << ") Setting Content-Disposition metadata to: "
                  << disposition << endl;
    setMetaData("content-disposition", disposition);
  }

  if (!m_request.lastModified.isEmpty())
    setMetaData("modified", m_request.lastModified);

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
  if (locationStr.isEmpty() && (!m_strMimeType.isEmpty() ||
      m_request.method == HTTP_HEAD))
  {
    kdDebug(7113) << "(" << m_pid << ") Emitting mimetype " << m_strMimeType << endl;
    mimeType( m_strMimeType );
  }

  forwardHttpResponseHeader();

  if (m_request.method == HTTP_HEAD)
     return true;

  // Do we want to cache this request?
  if (m_request.bUseCache)
  {
     ::unlink( QFile::encodeName(m_request.cef));
     if ( m_request.bCachedWrite && !m_strMimeType.isEmpty() )
     {
        // Check...
        createCacheEntry(m_strMimeType, expireDate); // Create a cache entry
        if (!m_request.fcache)
	    {
		m_request.bCachedWrite = false; // Error creating cache entry.
		kdDebug(7113) << "(" << m_pid << ") Error creating cache entry for " << m_request.url.url()<<"!\n";
	    }
        m_request.expireDate = expireDate;
        m_maxCacheSize = config()->readEntry("MaxCacheSize", DEFAULT_MAX_CACHE_SIZE) / 2;
     }
  }

  if (m_request.bCachedWrite && !m_strMimeType.isEmpty())
    kdDebug(7113) << "(" << m_pid << ") Cache, adding \"" << m_request.url.url() << "\"" << endl;
  else if (m_request.bCachedWrite && m_strMimeType.isEmpty())
    kdDebug(7113) << "(" << m_pid << ") Cache, pending \"" << m_request.url.url() << "\"" << endl;
  else
    kdDebug(7113) << "(" << m_pid << ") Cache, not adding \"" << m_request.url.url() << "\"" << endl;
  return true;
}


void HTTPProtocol::addEncoding(QString encoding, QStringList &encs)
{
  encoding = encoding.trimmed().toLower();
  // Identity is the same as no encoding
  if (encoding == "identity") {
    return;
  } else if (encoding == "8bit") {
    // Strange encoding returned by http://linac.ikp.physik.tu-darmstadt.de
    return;
  } else if (encoding == "chunked") {
    m_bChunked = true;
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
    kdDebug(7113) << "(" << m_pid << ") Unknown encoding encountered.  "
                    << "Please write code. Encoding = \"" << encoding
                    << "\"" << endl;
  }
}

bool HTTPProtocol::sendBody()
{
  int result=-1;
  int length=0;

  infoMessage( i18n( "Requesting data to send" ) );

  // m_bufPOST will NOT be empty iff authentication was required before posting
  // the data OR a re-connect is requested from ::readHeader because the
  // connection was lost for some reason.
  if ( !m_bufPOST.isNull() )
  {
    kdDebug(7113) << "(" << m_pid << ") POST'ing saved data..." << endl;

    result = 0;
    length = m_bufPOST.size();
  }
  else
  {
    kdDebug(7113) << "(" << m_pid << ") POST'ing live data..." << endl;

    QByteArray buffer;
    int old_size;

    m_bufPOST.resize(0);
    do
    {
      dataReq(); // Request for data
      result = readData( buffer );
      if ( result > 0 )
      {
        length += result;
        old_size = m_bufPOST.size();
        m_bufPOST.resize( old_size+result );
        memcpy( m_bufPOST.data()+ old_size, buffer.data(), buffer.size() );
        buffer.resize(0);
      }
    } while ( result > 0 );
  }

  if ( result < 0 )
  {
    error( ERR_ABORTED, m_request.hostname );
    return false;
  }

  infoMessage( i18n( "Sending data to %1" ).arg( m_request.hostname ) );

  QString size = QString ("Content-Length: %1\r\n\r\n").arg(length);
  kdDebug( 7113 ) << "(" << m_pid << ")" << size << endl;

  // Send the content length...
  bool sendOk = (write(size.toLatin1(), size.length()) == (ssize_t) size.length());
  if (!sendOk)
  {
    kdDebug( 7113 ) << "(" << m_pid << ") Connection broken when sending "
                    << "content length: (" << m_state.hostname << ")" << endl;
    error( ERR_CONNECTION_BROKEN, m_state.hostname );
    return false;
  }

  // Send the data...
  // kdDebug( 7113 ) << "(" << m_pid << ") POST DATA: " << QCString(m_bufPOST) << endl;
  sendOk = (write(m_bufPOST.data(), m_bufPOST.size()) == (ssize_t) m_bufPOST.size());
  if (!sendOk)
  {
    kdDebug(7113) << "(" << m_pid << ") Connection broken when sending message body: ("
                  << m_state.hostname << ")" << endl;
    error( ERR_CONNECTION_BROKEN, m_state.hostname );
    return false;
  }

  return true;
}

void HTTPProtocol::httpClose( bool keepAlive )
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::httpClose" << endl;

  if (m_request.fcache)
  {
     fclose(m_request.fcache);
     m_request.fcache = 0;
     if (m_request.bCachedWrite)
     {
        QString filename = m_request.cef + ".new";
        ::unlink( QFile::encodeName(filename) );
     }
  }

  // Only allow persistent connections for GET requests.
  // NOTE: we might even want to narrow this down to non-form
  // based submit requests which will require a meta-data from
  // khtml.
  if (keepAlive && (!m_bUseProxy ||
      m_bPersistentProxyConnection || m_bIsTunneled))
  {
    if (!m_keepAliveTimeout)
       m_keepAliveTimeout = DEFAULT_KEEP_ALIVE_TIMEOUT;
    else if (m_keepAliveTimeout > 2*DEFAULT_KEEP_ALIVE_TIMEOUT)
       m_keepAliveTimeout = 2*DEFAULT_KEEP_ALIVE_TIMEOUT;

    kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::httpClose: keep alive (" << m_keepAliveTimeout << ")" << endl;
    QByteArray data;
    QDataStream stream( &data, QIODevice::WriteOnly );
    stream << int(99); // special: Close connection
    setTimeoutSpecialCommand(m_keepAliveTimeout, data);
    return;
  }

  httpCloseConnection();
}

void HTTPProtocol::closeConnection()
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::closeConnection" << endl;
  httpCloseConnection ();
}

void HTTPProtocol::httpCloseConnection ()
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::httpCloseConnection" << endl;
  m_bIsTunneled = false;
  m_bKeepAlive = false;
  closeDescriptor();
  setTimeoutSpecialCommand(-1); // Cancel any connection timeout
}

void HTTPProtocol::slave_status()
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::slave_status" << endl;

  if ( !isConnectionValid() )
     httpCloseConnection();

  slaveStatus( m_state.hostname, isConnectionValid() );
}

void HTTPProtocol::mimetype( const KUrl& url )
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::mimetype: "
                << url.prettyURL() << endl;

  if ( !checkRequestURL( url ) )
    return;

  m_request.method = HTTP_HEAD;
  m_request.path = url.path();
  m_request.query = url.query();
  m_request.cache = CC_Cache;
  m_request.doProxy = m_bUseProxy;

  retrieveHeader();

  kdDebug(7113) << "(" << m_pid << ") http: mimetype = " << m_strMimeType
                << endl;
}

void HTTPProtocol::special( const QByteArray &data )
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::special" << endl;

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
      time_t expireDate;
      stream >> url >> no_cache >> expireDate;
      cacheUpdate( url, no_cache, expireDate );
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
     setRewindMarker();

     m_bufReceive.resize(4096);

     if (!gets(m_bufReceive.data(), m_bufReceive.size()))
     {
       kdDebug(7113) << "(" << m_pid << ") gets() failure on Chunk header" << endl;
       return -1;
     }
     // We could have got the CRLF of the previous chunk.
     // If so, try again.
     if (m_bufReceive[0] == '\0')
     {
        if (!gets(m_bufReceive.data(), m_bufReceive.size()))
        {
           kdDebug(7113) << "(" << m_pid << ") gets() failure on Chunk header" << endl;
           return -1;
        }
     }
     if (m_bEOF)
     {
        kdDebug(7113) << "(" << m_pid << ") EOF on Chunk header" << endl;
        return -1;
     }

     long long trunkSize = STRTOLL(m_bufReceive.data(), 0, 16);
     if (trunkSize < 0)
     {
        kdDebug(7113) << "(" << m_pid << ") Negative chunk size" << endl;
        return -1;
     }
     m_iBytesLeft = trunkSize;

     // kdDebug(7113) << "(" << m_pid << ") Chunk size = " << m_iBytesLeft << " bytes" << endl;

     if (m_iBytesLeft == 0)
     {
       // Last chunk.
       // Skip trailers.
       do {
         // Skip trailer of last chunk.
         if (!gets(m_bufReceive.data(), m_bufReceive.size()))
         {
           kdDebug(7113) << "(" << m_pid << ") gets() failure on Chunk trailer" << endl;
           return -1;
         }
         // kdDebug(7113) << "(" << m_pid << ") Chunk trailer = \"" << m_bufReceive.data() << "\"" << endl;
       }
       while (strlen(m_bufReceive.data()) != 0);

       return 0;
     }
  }

  int bytesReceived = readLimited();
  if (!m_iBytesLeft)
     m_iBytesLeft = NO_SIZE; // Don't stop, continue with next chunk
  return bytesReceived;
}

int HTTPProtocol::readLimited()
{
  if (!m_iBytesLeft)
    return 0;

  m_bufReceive.resize(4096);

  int bytesReceived;
  int bytesToReceive;

  if (m_iBytesLeft > KIO::filesize_t(m_bufReceive.size()))
     bytesToReceive = m_bufReceive.size();
  else
     bytesToReceive = m_iBytesLeft;

  bytesReceived = read(m_bufReceive.data(), bytesToReceive);

  if (bytesReceived <= 0)
     return -1; // Error: connection lost

  m_iBytesLeft -= bytesReceived;
  return bytesReceived;
}

int HTTPProtocol::readUnlimited()
{
  if (m_bKeepAlive)
  {
     kdDebug(7113) << "(" << m_pid << ") Unbounded datastream on a Keep "
                     << "alive connection!" << endl;
     m_bKeepAlive = false;
  }

  m_bufReceive.resize(4096);

  int result = read(m_bufReceive.data(), m_bufReceive.size());
  if (result > 0)
     return result;

  m_bEOF = true;
  m_iBytesLeft = 0;
  return 0;
}

void HTTPProtocol::slotData(const QByteArray &_d)
{
   if (!_d.size())
   {
      m_bEOD = true;
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
      if ( m_strMimeType.isEmpty() && !m_bRedirect &&
           !( m_responseCode >= 300 && m_responseCode <=399) )
      {
        kdDebug(7113) << "(" << m_pid << ") Determining mime-type from content..." << endl;
        int old_size = m_mimeTypeBuffer.size();
        m_mimeTypeBuffer.resize( old_size + d.size() );
        memcpy( m_mimeTypeBuffer.data() + old_size, d.data(), d.size() );
        if ( (m_iBytesLeft != NO_SIZE) && (m_iBytesLeft > 0)
             && (m_mimeTypeBuffer.size() < 1024) )
        {
          m_cpMimeBuffer = true;
          return;   // Do not send up the data since we do not yet know its mimetype!
        }

        kdDebug(7113) << "(" << m_pid << ") Mimetype buffer size: " << m_mimeTypeBuffer.size()
                      << endl;

        KMimeMagicResult *result;
        result = KMimeMagic::self()->findBufferFileType( m_mimeTypeBuffer,
                                                         m_request.url.fileName() );
        if( result )
        {
          m_strMimeType = result->mimeType();
          kdDebug(7113) << "(" << m_pid << ") Mimetype from content: "
                        << m_strMimeType << endl;
        }

        if ( m_strMimeType.isEmpty() )
        {
          m_strMimeType = QString::fromLatin1( DEFAULT_MIME_TYPE );
          kdDebug(7113) << "(" << m_pid << ") Using default mimetype: "
                        <<  m_strMimeType << endl;
        }

        if ( m_request.bCachedWrite )
        {
          createCacheEntry( m_strMimeType, m_request.expireDate );
          if (!m_request.fcache)
            m_request.bCachedWrite = false;
        }

        if ( m_cpMimeBuffer )
        {
          d.resize(0);
          d.resize(m_mimeTypeBuffer.size());
          memcpy( d.data(), m_mimeTypeBuffer.data(),
                  d.size() );
        }
        mimeType(m_strMimeType);
        m_mimeTypeBuffer.resize(0);
      }

      data( d );
      if (m_request.bCachedWrite && m_request.fcache)
         writeCacheEntry(d.data(), d.size());
   }
   else
   {
      uint old_size = m_bufWebDavData.size();
      m_bufWebDavData.resize (old_size + d.size());
      memcpy (m_bufWebDavData.data() + old_size, d.data(), d.size());
   }
}

/**
 * This function is our "receive" function.  It is responsible for
 * downloading the message (not the header) from the HTTP server.  It
 * is called either as a response to a client's KIOJob::dataEnd()
 * (meaning that the client is done sending data) or by 'httpOpen()'
 * (if we are in the process of a PUT/POST request). It can also be
 * called by a webDAV function, to receive stat/list/property/etc.
 * data; in this case the data is stored in m_bufWebDavData.
 */
bool HTTPProtocol::readBody( bool dataInternal /* = false */ )
{
  if (m_responseCode == 204)
     return true;

  m_bEOD = false;
  // Note that when dataInternal is true, we are going to:
  // 1) save the body data to a member variable, m_bufWebDavData
  // 2) _not_ advertise the data, speed, size, etc., through the
  //    corresponding functions.
  // This is used for returning data to WebDAV.
  m_dataInternal = dataInternal;
  if ( dataInternal )
    m_bufWebDavData.resize (0);

  // Check if we need to decode the data.
  // If we are in copy mode, then use only transfer decoding.
  bool useMD5 = !m_sContentMD5.isEmpty();

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
       infoMessage( i18n( "Retrieving %1 from %2...").arg(KIO::convertSize(m_iSize))
         .arg( m_request.hostname ) );
    }
    else
    {
       totalSize ( 0 );
    }
  }
  else
    infoMessage( i18n( "Retrieving from %1..." ).arg( m_request.hostname ) );

  if (m_request.bCachedRead)
  {
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::readBody: read data from cache!" << endl;
    m_request.bCachedWrite = false;

    char buffer[ MAX_IPC_SIZE ];

    m_iContentLeft = NO_SIZE;

    // Jippie! It's already in the cache :-)
    while (!feof(m_request.fcache) && !ferror(m_request.fcache))
    {
      int nbytes = fread( buffer, 1, MAX_IPC_SIZE, m_request.fcache);

      if (nbytes > 0)
      {
        slotData( QByteArray::fromRawData( buffer, nbytes ) );
        sz += nbytes;
      }
    }

    m_bufReceive.resize( 0 );

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

  if (m_bChunked)
    m_iBytesLeft = NO_SIZE;

  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::readBody: retrieve data. "<<KIO::number(m_iBytesLeft)<<" left." << endl;

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
  while (!m_qTransferEncodings.isEmpty())
  {
    QString enc = m_qTransferEncodings.takeLast();
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
  while (!m_qContentEncodings.isEmpty())
  {
    QString enc = m_qContentEncodings.takeLast();
    if ( enc == "gzip" )
      chain.addFilter(new HTTPFilterGZip);
    else if ( enc == "deflate" )
      chain.addFilter(new HTTPFilterDeflate);
  }

  while (!m_bEOF)
  {
    int bytesReceived;

    if (m_bChunked)
       bytesReceived = readChunked();
    else if (m_iSize != NO_SIZE)
       bytesReceived = readLimited();
    else
       bytesReceived = readUnlimited();

    // make sure that this wasn't an error, first
    // kdDebug(7113) << "(" << (int) m_pid << ") readBody: bytesReceived: "
    //              << (int) bytesReceived << " m_iSize: " << (int) m_iSize << " Chunked: "
    //              << (int) m_bChunked << " BytesLeft: "<< (int) m_iBytesLeft << endl;
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
      kdDebug(7113) << "(" << m_pid << ") readBody: bytesReceived==-1 sz=" << (int)sz
                    << " Connnection broken !" << endl;
      error(ERR_CONNECTION_BROKEN, m_state.hostname);
      return false;
    }

    // I guess that nbytes == 0 isn't an error.. but we certainly
    // won't work with it!
    if (bytesReceived > 0)
    {
      // Important: truncate the buffer to the actual size received!
      // Otherwise garbage will be passed to the app
      m_bufReceive.truncate( bytesReceived );

      chain.slotInput(m_bufReceive);

      if (m_bError)
         return false;

      sz += bytesReceived;
      if (!dataInternal)
        processedSize( sz );
    }
    m_bufReceive.resize(0); // res

    if (m_iBytesLeft && m_bEOD && !m_bChunked)
    {
      // gzip'ed data sometimes reports a too long content-length.
      // (The length of the unzipped data)
      m_iBytesLeft = 0;
    }

    if (m_iBytesLeft == 0)
    {
      kdDebug(7113) << "("<<m_pid<<") EOD received! Left = "<< KIO::number(m_iBytesLeft) << endl;
      break;
    }
  }
  chain.slotInput(QByteArray()); // Flush chain.

  if ( useMD5 )
  {
    QString calculatedMD5 = md5Filter->md5();

    if ( m_sContentMD5 == calculatedMD5 )
      kdDebug(7113) << "(" << m_pid << ") MD5 checksum MATCHED!!" << endl;
    else
      kdDebug(7113) << "(" << m_pid << ") MD5 checksum MISMATCH! Expected: "
                    << calculatedMD5 << ", Got: " << m_sContentMD5 << endl;
  }

  // Close cache entry
  if (m_iBytesLeft == 0)
  {
     if (m_request.bCachedWrite && m_request.fcache)
        closeCacheEntry();
     else if (m_request.bCachedWrite) kdDebug(7113) << "(" << m_pid << ") no cache file!\n";
  }
  else kdDebug(7113) << "(" << m_pid << ") still "<< KIO::number(m_iBytesLeft) <<" bytes left! can't close cache entry!\n";

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

  // Clear of the temporary POST buffer if it is not empty...
  if (!m_bufPOST.isEmpty())
  {
    m_bufPOST.resize(0);
    kdDebug(7113) << "(" << m_pid << ") HTTP::retreiveHeader: Cleared POST "
                     "buffer..." << endl;
  }

  SlaveBase::error( _err, _text );
  m_bError = true;
}


void HTTPProtocol::addCookies( const QString &url, const Q3CString &cookieHeader )
{
   long windowId = m_request.window.toLong();
   QByteArray params;
   QDataStream stream(&params, QIODevice::WriteOnly);
   stream << url << cookieHeader << windowId;

   kdDebug(7113) << "(" << m_pid << ") " << cookieHeader << endl;
   kdDebug(7113) << "(" << m_pid << ") " << "Window ID: "
                 << windowId << ", for host = " << url << endl;

   if ( !dcopClient()->send( "kded", "kcookiejar", "addCookies(QString,QCString,long int)", params ) )
   {
      kdWarning(7113) << "(" << m_pid << ") Can't communicate with kded_kcookiejar!" << endl;
   }
}

QString HTTPProtocol::findCookies( const QString &url)
{
  DCOPCString replyType;
  QByteArray params;
  QByteArray reply;
  QString result;

  long windowId = m_request.window.toLong();
  result.clear();
  QDataStream stream(&params, QIODevice::WriteOnly);
  stream << url << windowId;

  if ( !dcopClient()->call( "kded", "kcookiejar", "findCookies(QString,long int)",
                            params, replyType, reply ) )
  {
     kdWarning(7113) << "(" << m_pid << ") Can't communicate with kded_kcookiejar!" << endl;
     return result;
  }
  if ( replyType == "QString" )
  {
     QDataStream stream2( reply );
     stream2 >> result;
  }
  else
  {
     kdError(7113) << "(" << m_pid << ") DCOP function findCookies(...) returns "
                          << replyType << ", expected QString" << endl;
  }
  return result;
}

/******************************* CACHING CODE ****************************/


void HTTPProtocol::cacheUpdate( const KUrl& url, bool no_cache, time_t expireDate)
{
  if ( !checkRequestURL( url ) )
      return;

  m_request.path = url.path();
  m_request.query = url.query();
  m_request.cache = CC_Reload;
  m_request.doProxy = m_bUseProxy;

  if (no_cache)
  {
     m_request.fcache = checkCacheEntry( );
     if (m_request.fcache)
     {
       fclose(m_request.fcache);
       m_request.fcache = 0;
       ::unlink( QFile::encodeName(m_request.cef) );
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

FILE* HTTPProtocol::checkCacheEntry( bool readWrite)
{
   const QChar separator = '_';

   QString CEF = m_request.path;

   int p = CEF.indexOf('/');

   while(p != -1)
   {
      CEF[p] = separator;
      p = CEF.indexOf('/', p);
   }

   QString host = m_request.hostname.toLower();
   CEF = host + CEF + '_';

   QString dir = m_strCacheDir;
   if (dir[dir.length()-1] != '/')
      dir += "/";

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
      dir += "0";

   unsigned long hash = 0x00000000;
   QByteArray u = m_request.url.url().toLatin1();
   for(int i = u.length(); i--;)
   {
      hash = (hash * 12211 + u.at(i)) % 2147483563;
   }

   QString hashString;
   hashString.sprintf("%08lx", hash);

   CEF = CEF + hashString;

   CEF = dir + "/" + CEF;

   m_request.cef = CEF;

   const char *mode = (readWrite ? "r+" : "r");

   FILE *fs = fopen( QFile::encodeName(CEF), mode); // Open for reading and writing
   if (!fs)
      return 0;

   char buffer[401];
   bool ok = true;

  // CacheRevision
  if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok && (strcmp(buffer, CACHE_REVISION) != 0))
      ok = false;

   time_t date;
   time_t currentDate = time(0);

   // URL
   if (ok && (!fgets(buffer, 400, fs)))
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
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      date = (time_t) strtoul(buffer, 0, 10);
      m_request.creationDate = date;
      if (m_maxCacheAge && (difftime(currentDate, date) > m_maxCacheAge))
      {
         m_request.bMustRevalidate = true;
         m_request.expireDate = currentDate;
      }
   }

   // Expiration Date
   m_request.cacheExpireDateOffset = ftell(fs);
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      if (m_request.cache == CC_Verify)
      {
         date = (time_t) strtoul(buffer, 0, 10);
         // After the expire date we need to revalidate.
         if (!date || difftime(currentDate, date) >= 0)
            m_request.bMustRevalidate = true;
         m_request.expireDate = date;
      }
      else if (m_request.cache == CC_Refresh)
      {
         m_request.bMustRevalidate = true;
         m_request.expireDate = currentDate;
      }
   }

   // ETag
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      m_request.etag = QString(buffer).trimmed();
   }

   // Last-Modified
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      m_request.lastModified = QString(buffer).trimmed();
   }

   if (ok)
      return fs;

   fclose(fs);
   unlink( QFile::encodeName(CEF));
   return 0;
}

void HTTPProtocol::updateExpireDate(time_t expireDate, bool updateCreationDate)
{
    bool ok = true;

    FILE *fs = checkCacheEntry(true);
    if (fs)
    {
        QString date;
        char buffer[401];
        time_t creationDate;

        fseek(fs, 0, SEEK_SET);
        if (ok && !fgets(buffer, 400, fs))
            ok = false;
        if (ok && !fgets(buffer, 400, fs))
            ok = false;
        long cacheCreationDateOffset = ftell(fs);
        if (ok && !fgets(buffer, 400, fs))
            ok = false;
        creationDate = strtoul(buffer, 0, 10);
        if (!creationDate)
            ok = false;

        if (updateCreationDate)
        {
           if (!ok || fseek(fs, cacheCreationDateOffset, SEEK_SET))
              return;
           QString date;
           date.setNum( time(0) );
           date = date.leftJustified(16);
           fputs(date.toLatin1(), fs);      // Creation date
           fputc('\n', fs);
        }

        if (expireDate>(30*365*24*60*60))
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
        if (!ok || fseek(fs, m_request.cacheExpireDateOffset, SEEK_SET))
            return;
        fputs(date.toLatin1(), fs);      // Expire date
        fseek(fs, 0, SEEK_END);
        fclose(fs);
    }
}

void HTTPProtocol::createCacheEntry( const QString &mimetype, time_t expireDate)
{
   QString dir = m_request.cef;
   int p = dir.lastIndexOf('/');
   if (p == -1) return; // Error.
   dir.truncate(p);

   // Create file
   (void) ::mkdir( QFile::encodeName(dir), 0700 );

   QString filename = m_request.cef + ".new";  // Create a new cache entryexpireDate

//   kdDebug( 7103 ) <<  "creating new cache entry: " << filename << endl;

   m_request.fcache = fopen( QFile::encodeName(filename), "w");
   if (!m_request.fcache)
   {
      kdWarning(7113) << "(" << m_pid << ")createCacheEntry: opening " << filename << " failed." << endl;
      return; // Error.
   }

   fputs(CACHE_REVISION, m_request.fcache);    // Revision

   fputs(m_request.url.url().toLatin1(), m_request.fcache);  // Url
   fputc('\n', m_request.fcache);

   QString date;
   m_request.creationDate = time(0);
   date.setNum( m_request.creationDate );
   date = date.leftJustified(16);
   fputs(date.toLatin1(), m_request.fcache);      // Creation date
   fputc('\n', m_request.fcache);

   date.setNum( expireDate );
   date = date.leftJustified(16);
   fputs(date.toLatin1(), m_request.fcache);      // Expire date
   fputc('\n', m_request.fcache);

   if (!m_request.etag.isEmpty())
      fputs(m_request.etag.toLatin1(), m_request.fcache);    //ETag
   fputc('\n', m_request.fcache);

   if (!m_request.lastModified.isEmpty())
      fputs(m_request.lastModified.toLatin1(), m_request.fcache);    // Last modified
   fputc('\n', m_request.fcache);

   fputs(mimetype.toLatin1(), m_request.fcache);  // Mimetype
   fputc('\n', m_request.fcache);

   if (!m_request.strCharset.isEmpty())
      fputs(m_request.strCharset.toLatin1(), m_request.fcache);    // Charset
   fputc('\n', m_request.fcache);

   return;
}
// The above code should be kept in sync
// with the code in http_cache_cleaner.cpp
// !END SYNC!

void HTTPProtocol::writeCacheEntry( const char *buffer, int nbytes)
{
   if (fwrite( buffer, nbytes, 1, m_request.fcache) != 1)
   {
      kdWarning(7113) << "(" << m_pid << ") writeCacheEntry: writing " << nbytes << " bytes failed." << endl;
      fclose(m_request.fcache);
      m_request.fcache = 0;
      QString filename = m_request.cef + ".new";
      ::unlink( QFile::encodeName(filename) );
      return;
   }
   long file_pos = ftell( m_request.fcache ) / 1024;
   if ( file_pos > m_maxCacheSize )
   {
      kdDebug(7113) << "writeCacheEntry: File size reaches " << file_pos
                    << "Kb, exceeds cache limits. (" << m_maxCacheSize << "Kb)" << endl;
      fclose(m_request.fcache);
      m_request.fcache = 0;
      QString filename = m_request.cef + ".new";
      ::unlink( QFile::encodeName(filename) );
      return;
   }
}

void HTTPProtocol::closeCacheEntry()
{
   QString filename = m_request.cef + ".new";
   int result = fclose( m_request.fcache);
   m_request.fcache = 0;
   if (result == 0)
   {
      if (::rename( QFile::encodeName(filename), QFile::encodeName(m_request.cef)) == 0)
         return; // Success

      kdWarning(7113) << "(" << m_pid << ") closeCacheEntry: error renaming "
                      << "cache entry. (" << filename << " -> " << m_request.cef
                      << ")" << endl;
   }

   kdWarning(7113) << "(" << m_pid << ") closeCacheEntry: error closing cache "
                   << "entry. (" << filename<< ")" << endl;
}

void HTTPProtocol::cleanCache()
{
   const time_t maxAge = DEFAULT_CLEAN_CACHE_INTERVAL; // 30 Minutes.
   bool doClean = false;
   QString cleanFile = m_strCacheDir;
   if (cleanFile[cleanFile.length()-1] != '/')
      cleanFile += "/";
   cleanFile += "cleaned";

   struct stat stat_buf;

   int result = ::stat(QFile::encodeName(cleanFile), &stat_buf);
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


void HTTPProtocol::configAuth( char *p, bool b )
{
  HTTP_AUTH f = AUTH_None;
  const char *strAuth = p;

  if ( strncasecmp( p, "Basic", 5 ) == 0 )
  {
    f = AUTH_Basic;
    p += 5;
    strAuth = "Basic"; // Correct for upper-case variations.
  }
  else if ( strncasecmp (p, "Digest", 6) == 0 )
  {
    f = AUTH_Digest;
    memcpy((void *)p, "Digest", 6); // Correct for upper-case variations.
    p += 6;
  }
  else if (strncasecmp( p, "MBS_PWD_COOKIE", 14 ) == 0)
  {
    // Found on http://www.webscription.net/baen/default.asp
    f = AUTH_Basic;
    p += 14;
    strAuth = "Basic";
  }
#ifdef HAVE_LIBGSSAPI
  else if ( strncasecmp( p, "Negotiate", 9 ) == 0 )
  {
    // if we get two 401 in a row let's assume for now that
    // Negotiate isn't working and ignore it
    if ( !b && !(m_responseCode == 401 && m_prevResponseCode == 401) )
    {
      f = AUTH_Negotiate;
      memcpy((void *)p, "Negotiate", 9); // Correct for upper-case variations.
      p += 9;
    };
  }
#endif
  else if ( strncasecmp( p, "NTLM", 4 ) == 0 &&
    (( b && m_bPersistentProxyConnection ) || !b ) )
  {
    f = AUTH_NTLM;
    memcpy((void *)p, "NTLM", 4); // Correct for upper-case variations.
    p += 4;
    m_strRealm = "NTLM"; // set a dummy realm
  }
  else
  {
    kdWarning(7113) << "(" << m_pid << ") Unsupported or invalid authorization "
                    << "type requested" << endl;
    if (b)
      kdWarning(7113) << "(" << m_pid << ") Proxy URL: " << m_proxyURL << endl;
    else
      kdWarning(7113) << "(" << m_pid << ") URL: " << m_request.url << endl;
    kdWarning(7113) << "(" << m_pid << ") Request Authorization: " << p << endl;
  }

  /*
     This check ensures the following:
     1.) Rejection of any unknown/unsupported authentication schemes
     2.) Usage of the strongest possible authentication schemes if
         and when multiple Proxy-Authenticate or WWW-Authenticate
         header field is sent.
  */
  if (b)
  {
    if ((f == AUTH_None) ||
        ((m_iProxyAuthCount > 0) && (f < ProxyAuthentication)))
    {
      // Since I purposefully made the Proxy-Authentication settings
      // persistent to reduce the number of round-trips to kdesud we
      // have to take special care when an unknown/unsupported auth-
      // scheme is received. This check accomplishes just that...
      if ( m_iProxyAuthCount == 0)
        ProxyAuthentication = f;
      kdDebug(7113) << "(" << m_pid << ") Rejected proxy auth method: " << f << endl;
      return;
    }
    m_iProxyAuthCount++;
    kdDebug(7113) << "(" << m_pid << ") Accepted proxy auth method: " << f << endl;
  }
  else
  {
    if ((f == AUTH_None) ||
        ((m_iWWWAuthCount > 0) && (f < Authentication)))
    {
      kdDebug(7113) << "(" << m_pid << ") Rejected auth method: " << f << endl;
      return;
    }
    m_iWWWAuthCount++;
    kdDebug(7113) << "(" << m_pid << ") Accepted auth method: " << f << endl;
  }


  while (*p)
  {
    int i = 0;
    while( (*p == ' ') || (*p == ',') || (*p == '\t') ) { p++; }
    if ( strncasecmp( p, "realm=", 6 ) == 0 )
    {
      p += 6;
      if (*p == '"') p++;
      while( p[i] && p[i] != '"' ) i++;
      if( b )
        m_strProxyRealm = QString::fromLatin1( p, i );
      else
        m_strRealm = QString::fromLatin1( p, i );
      if (!p[i]) break;
    }
    p+=(i+1);
  }

  if( b )
  {
    ProxyAuthentication = f;
    m_strProxyAuthorization = QString::fromLatin1( strAuth );
  }
  else
  {
    Authentication = f;
    m_strAuthorization = QString::fromLatin1( strAuth );
  }
}


bool HTTPProtocol::retryPrompt()
{
  QString prompt;
  switch ( m_responseCode )
  {
    case 401:
      prompt = i18n("Authentication Failed.");
      break;
    case 407:
      prompt = i18n("Proxy Authentication Failed.");
      break;
    default:
      break;
  }
  prompt += i18n("  Do you want to retry?");
  return (messageBox(QuestionYesNo, prompt, i18n("Authentication")) == 3);
}

void HTTPProtocol::promptInfo( AuthInfo& info )
{
  if ( m_responseCode == 401 )
  {
    info.url = m_request.url;
    if ( !m_state.user.isEmpty() )
      info.username = m_state.user;
    info.readOnly = !m_request.url.user().isEmpty();
    info.prompt = i18n( "You need to supply a username and a "
                        "password to access this site." );
    info.keepPassword = true; // Prompt the user for persistence as well.
    if ( !m_strRealm.isEmpty() )
    {
      info.realmValue = m_strRealm;
      info.verifyPath = false;
      info.digestInfo = m_strAuthorization;
      info.commentLabel = i18n( "Site:" );
      info.comment = i18n("<b>%1</b> at <b>%2</b>").arg( m_strRealm ).arg( m_request.hostname );
    }
  }
  else if ( m_responseCode == 407 )
  {
    info.url = m_proxyURL;
    info.username = m_proxyURL.user();
    info.prompt = i18n( "You need to supply a username and a password for "
                        "the proxy server listed below before you are allowed "
                        "to access any sites." );
    info.keepPassword = true;
    if ( !m_strProxyRealm.isEmpty() )
    {
      info.realmValue = m_strProxyRealm;
      info.verifyPath = false;
      info.digestInfo = m_strProxyAuthorization;
      info.commentLabel = i18n( "Proxy:" );
      info.comment = i18n("<b>%1</b> at <b>%2</b>").arg( m_strProxyRealm ).arg( m_proxyURL.host() );
    }
  }
}

bool HTTPProtocol::getAuthorization()
{
  AuthInfo info;
  bool result = false;

  kdDebug (7113) << "(" << m_pid << ") HTTPProtocol::getAuthorization: "
                 << "Current Response: " << m_responseCode << ", "
                 << "Previous Response: " << m_prevResponseCode << ", "
                 << "Authentication: " << Authentication << ", "
                 << "ProxyAuthentication: " << ProxyAuthentication << endl;

  if (m_request.bNoAuth)
  {
     if (m_request.bErrorPage)
        errorPage();
     else
        error( ERR_COULD_NOT_LOGIN, i18n("Authentication needed for %1 but authentication is disabled.").arg(m_request.hostname));
     return false;
  }

  bool repeatFailure = (m_prevResponseCode == m_responseCode);

  QString errorMsg;

  if (repeatFailure)
  {
    bool prompt = true;
    if ( Authentication == AUTH_Digest || ProxyAuthentication == AUTH_Digest )
    {
      bool isStaleNonce = false;
      QString auth = ( m_responseCode == 401 ) ? m_strAuthorization : m_strProxyAuthorization;
      int pos = auth.indexOf("stale", 0, Qt::CaseInsensitive);
      if ( pos != -1 )
      {
        pos += 5;
        int len = auth.length();
        while( pos < len && (auth[pos] == ' ' || auth[pos] == '=') ) pos++;
        if ( pos < len && auth.indexOf("true", pos, Qt::CaseInsensitive) != -1 )
        {
          isStaleNonce = true;
          kdDebug(7113) << "(" << m_pid << ") Stale nonce value. "
                        << "Will retry using same info..." << endl;
        }
      }
      if ( isStaleNonce )
      {
        prompt = false;
        result = true;
        if ( m_responseCode == 401 )
        {
          info.username = m_request.user;
          info.password = m_request.passwd;
          info.realmValue = m_strRealm;
          info.digestInfo = m_strAuthorization;
        }
        else if ( m_responseCode == 407 )
        {
          info.username = m_proxyURL.user();
          info.password = m_proxyURL.pass();
          info.realmValue = m_strProxyRealm;
          info.digestInfo = m_strProxyAuthorization;
        }
      }
    }

    if ( Authentication == AUTH_NTLM || ProxyAuthentication == AUTH_NTLM )
    {
      QString auth = ( m_responseCode == 401 ) ? m_strAuthorization : m_strProxyAuthorization;
      kdDebug(7113) << "auth: " << auth << endl;
      if ( auth.length() > 4 )
      {
        prompt = false;
        result = true;
        kdDebug(7113) << "(" << m_pid << ") NTLM auth second phase, "
                      << "sending response..." << endl;
        if ( m_responseCode == 401 )
        {
          info.username = m_request.user;
          info.password = m_request.passwd;
          info.realmValue = m_strRealm;
          info.digestInfo = m_strAuthorization;
        }
        else if ( m_responseCode == 407 )
        {
          info.username = m_proxyURL.user();
          info.password = m_proxyURL.pass();
          info.realmValue = m_strProxyRealm;
          info.digestInfo = m_strProxyAuthorization;
        }
      }
    }

    if ( prompt )
    {
      switch ( m_responseCode )
      {
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
  }
  else
  {
    // At this point we know more details, so use it to find
    // out if we have a cached version and avoid a re-prompt!
    // We also do not use verify path unlike the pre-emptive
    // requests because we already know the realm value...

    if (m_bProxyAuthValid)
    {
      // Reset cached proxy auth
      m_bProxyAuthValid = false;
      KUrl proxy ( config()->readEntry("UseProxy") );
      m_proxyURL.setUser(proxy.user());
      m_proxyURL.setPass(proxy.pass());
    }

    info.verifyPath = false;
    if ( m_responseCode == 407 )
    {
      info.url = m_proxyURL;
      info.username = m_proxyURL.user();
      info.password = m_proxyURL.pass();
      info.realmValue = m_strProxyRealm;
      info.digestInfo = m_strProxyAuthorization;
    }
    else
    {
      info.url = m_request.url;
      info.username = m_request.user;
      info.password = m_request.passwd;
      info.realmValue = m_strRealm;
      info.digestInfo = m_strAuthorization;
    }

    // If either username or password is not supplied
    // with the request, check the password cache.
    if ( info.username.isNull() ||
         info.password.isNull() )
      result = checkCachedAuthentication( info );

    if ( Authentication == AUTH_Digest )
    {
      QString auth;

      if (m_responseCode == 401)
        auth = m_strAuthorization;
      else
        auth = m_strProxyAuthorization;

      int pos = auth.indexOf("stale", 0, Qt::CaseInsensitive);
      if ( pos != -1 )
      {
        pos += 5;
        int len = auth.length();
        while( pos < len && (auth[pos] == ' ' || auth[pos] == '=') ) pos++;
        if ( pos < len && auth.indexOf("true", pos, Qt::CaseInsensitive) != -1 )
        {
          info.digestInfo = (m_responseCode == 401) ? m_strAuthorization : m_strProxyAuthorization;
          kdDebug(7113) << "(" << m_pid << ") Just a stale nonce value! "
                        << "Retrying using the new nonce sent..." << endl;
        }
      }
    }
  }

  if (!result )
  {
    // Do not prompt if the username & password
    // is already supplied and the login attempt
    // did not fail before.
    if ( !repeatFailure &&
         !info.username.isNull() &&
         !info.password.isNull() )
      result = true;
    else
    {
      if (Authentication == AUTH_Negotiate)
      {
        if (!repeatFailure)
          result = true;
      }
      else if ( m_request.disablePassDlg == false )
      {
        kdDebug( 7113 ) << "(" << m_pid << ") Prompting the user for authorization..." << endl;
        promptInfo( info );
        result = openPassDlg( info, errorMsg );
      }
    }
  }

  if ( result )
  {
    switch (m_responseCode)
    {
      case 401: // Request-Authentication
        m_request.user = info.username;
        m_request.passwd = info.password;
        m_strRealm = info.realmValue;
        m_strAuthorization = info.digestInfo;
        break;
      case 407: // Proxy-Authentication
        m_proxyURL.setUser( info.username );
        m_proxyURL.setPass( info.password );
        m_strProxyRealm = info.realmValue;
        m_strProxyAuthorization = info.digestInfo;
        break;
      default:
        break;
    }
    return true;
  }

  if (m_request.bErrorPage)
     errorPage();
  else
     error( ERR_USER_CANCELED, QString() );
  return false;
}

void HTTPProtocol::saveAuthorization()
{
  AuthInfo info;
  if ( m_prevResponseCode == 407 )
  {
    if (!m_bUseProxy)
       return;
    m_bProxyAuthValid = true;
    info.url = m_proxyURL;
    info.username = m_proxyURL.user();
    info.password = m_proxyURL.pass();
    info.realmValue = m_strProxyRealm;
    info.digestInfo = m_strProxyAuthorization;
    cacheAuthentication( info );
  }
  else
  {
    info.url = m_request.url;
    info.username = m_request.user;
    info.password = m_request.passwd;
    info.realmValue = m_strRealm;
    info.digestInfo = m_strAuthorization;
    cacheAuthentication( info );
  }
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
    errorstr += " ";
    ret = gss_display_status(&new_status, minor_status, GSS_C_MECH_CODE, GSS_C_NULL_OID, &msg_ctx, &minor_string);
    errorstr += (const char *)minor_string.value;
    errorstr += " ";
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
    kdDebug(7113) << "(" << m_pid << ") gss_indicate_mechs failed: " << gssError(major_status, minor_status) << endl;
  } else {
    for (i=0; i<mech_set->count && !found; i++) {
      tmp_oid = &mech_set->elements[i];
      if (tmp_oid->length == spnego_oid_desc.length &&
        !memcmp(tmp_oid->elements, spnego_oid_desc.elements, tmp_oid->length)) {
        kdDebug(7113) << "(" << m_pid << ") createNegotiateAuth: found SPNEGO mech" << endl;
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
    kdDebug(7113) << "(" << m_pid << ") gss_import_name failed: " << gssError(major_status, minor_status) << endl;
    // reset the auth string so that subsequent methods aren't confused
    m_strAuthorization.clear();
    return QString();
  }

  major_status = gss_init_sec_context(&minor_status, GSS_C_NO_CREDENTIAL,
                                      &ctx, server, mech_oid,
                                      req_flags, GSS_C_INDEFINITE,
                                      GSS_C_NO_CHANNEL_BINDINGS,
                                      GSS_C_NO_BUFFER, NULL, &output_token,
                                      NULL, NULL);


  if (GSS_ERROR(major_status) || (output_token.length == 0)) {
    kdDebug(7113) << "(" << m_pid << ") gss_init_sec_context failed: " << gssError(major_status, minor_status) << endl;
    gss_release_name(&minor_status, &server);
    if (ctx != GSS_C_NO_CONTEXT) {
      gss_delete_sec_context(&minor_status, &ctx, GSS_C_NO_BUFFER);
      ctx = GSS_C_NO_CONTEXT;
    }
    // reset the auth string so that subsequent methods aren't confused
    m_strAuthorization.clear();
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
  return "";
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
    user = m_proxyURL.user();
    passwd = m_proxyURL.pass();
    strauth = m_strProxyAuthorization.toLatin1();
    len = m_strProxyAuthorization.length();
  }
  else
  {
    auth = "Authorization: NTLM ";
    user = m_state.user;
    passwd = m_state.passwd;
    strauth = m_strAuthorization.toLatin1();
    len = m_strAuthorization.length();
  }
  if ( user.contains('\\') ) {
    domain = user.section( '\\', 0, 0);
    user = user.section( '\\', 1 );
  }

  kdDebug(7113) << "(" << m_pid << ") NTLM length: " << len << endl;
  if ( user.isEmpty() || passwd.isEmpty() || len < 4 )
    return QString();

  if ( len > 4 )
  {
    // create a response
    QByteArray challenge;
    KCodecs::base64Decode( strauth.right( len - 5 ), challenge );
    KNTLM::getAuth( buf, challenge, user, passwd, domain,
		    KNetwork::KResolver::localHostName(), false, false );
  }
  else
  {
    KNTLM::getNegotiate( buf );
  }

  // remove the challenge to prevent reuse
  if ( isForProxy )
    m_strProxyAuthorization = "NTLM";
  else
    m_strAuthorization = "NTLM";

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
    user = m_proxyURL.user().toLatin1();
    passwd = m_proxyURL.pass().toLatin1();
  }
  else
  {
    auth = "Authorization: Basic ";
    user = m_state.user.toLatin1();
    passwd = m_state.passwd.toLatin1();
  }

  if ( user.isEmpty() )
    user = "";
  if ( passwd.isEmpty() )
    passwd = "";

  user += ':';
  user += passwd;
  auth += KCodecs::base64Encode( user );
  auth += "\r\n";

  return auth;
}

void HTTPProtocol::calculateResponse( DigestAuthInfo& info, QByteArray& Response )
{
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

  kdDebug(7113) << "(" << m_pid << ") calculateResponse(): A1 => " << HA1 << endl;

  // Calcualte H(A2)
  authStr = info.method;
  authStr += ':';
  authStr += m_request.url.encodedPathAndQuery(0, true).toLatin1();
  if ( info.qop == "auth-int" )
  {
    authStr += ':';
    authStr += info.entityBody;
  }
  md.reset();
  md.update( authStr );
  HA2 = md.hexDigest();

  kdDebug(7113) << "(" << m_pid << ") calculateResponse(): A2 => "
                << HA2 << endl;

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

  kdDebug(7113) << "(" << m_pid << ") calculateResponse(): Response => "
                << Response << endl;
}

QString HTTPProtocol::createDigestAuth ( bool isForProxy )
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
    info.username = m_proxyURL.user().toLatin1();
    info.password = m_proxyURL.pass().toLatin1();
    p = m_strProxyAuthorization.toLatin1();
  }
  else
  {
    auth = "Authorization: Digest ";
    info.username = m_state.user.toLatin1();
    info.password = m_state.passwd.toLatin1();
    p = m_strAuthorization.toLatin1();
  }
  if (!p || !*p)
    return QString();

  p += 6; // Skip "Digest"

  if ( info.username.isEmpty() || info.password.isEmpty() || !p )
    return QString();

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
  switch ( m_request.method )
  {
    case HTTP_GET:
        info.method = "GET";
        break;
    case HTTP_PUT:
        info.method = "PUT";
        break;
    case HTTP_POST:
        info.method = "POST";
        break;
    case HTTP_HEAD:
        info.method = "HEAD";
        break;
    case HTTP_DELETE:
        info.method = "DELETE";
        break;
    case DAV_PROPFIND:
        info.method = "PROPFIND";
        break;
    case DAV_PROPPATCH:
        info.method = "PROPPATCH";
        break;
    case DAV_MKCOL:
        info.method = "MKCOL";
        break;
    case DAV_COPY:
        info.method = "COPY";
        break;
    case DAV_MOVE:
        info.method = "MOVE";
        break;
    case DAV_LOCK:
        info.method = "LOCK";
        break;
    case DAV_UNLOCK:
        info.method = "UNLOCK";
        break;
    case DAV_SEARCH:
        info.method = "SEARCH";
        break;
    case DAV_SUBSCRIBE:
        info.method = "SUBSCRIBE";
        break;
    case DAV_UNSUBSCRIBE:
        info.method = "UNSUBSCRIBE";
        break;
    case DAV_POLL:
        info.method = "POLL";
        break;
    default:
        error( ERR_UNSUPPORTED_ACTION, i18n("Unsupported method: authentication will fail. Please submit a bug report."));
        break;
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
            info.digestURI.append( u );
        }
        else
        {
          KUrl u (m_request.url, uri.mid(idx, uri.length()-idx));
          if (u.isValid ())
            info.digestURI.append( u );
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

  if (info.realm.isEmpty() || info.nonce.isEmpty())
    return QString();

  // If the "domain" attribute was not specified and the current response code
  // is authentication needed, add the current request url to the list over which
  // this credential can be automatically applied.
  if (info.digestURI.isEmpty() && (m_responseCode == 401 || m_responseCode == 407))
    info.digestURI.append (m_request.url);
  else
  {
    // Verify whether or not we should send a cached credential to the
    // server based on the stored "domain" attribute...
    bool send = true;

    // Determine the path of the request url...
    QString requestPath = m_request.url.directory(false, false);
    if (requestPath.isEmpty())
      requestPath = "/";

    int count = info.digestURI.count();

    for (int i = 0; i < count; i++ )
    {
      KUrl u ( info.digestURI.at(i) );

      send &= (m_request.url.protocol().toLower() == u.protocol().toLower());
      send &= (m_request.hostname.toLower() == u.host().toLower());

      if (m_request.port > 0 && u.port() > 0)
        send &= (m_request.port == u.port());

      QString digestPath = u.directory (false, false);
      if (digestPath.isEmpty())
        digestPath = "/";

      send &= (requestPath.startsWith(digestPath));

      if (send)
        break;
    }

    kdDebug(7113) << "(" << m_pid << ") createDigestAuth(): passed digest "
                     "authentication credential test: " << send << endl;

    if (!send)
      return QString();
  }

  kdDebug(7113) << "(" << m_pid << ") RESULT OF PARSING:" << endl;
  kdDebug(7113) << "(" << m_pid << ")   algorithm: " << info.algorithm << endl;
  kdDebug(7113) << "(" << m_pid << ")   realm:     " << info.realm << endl;
  kdDebug(7113) << "(" << m_pid << ")   nonce:     " << info.nonce << endl;
  kdDebug(7113) << "(" << m_pid << ")   opaque:    " << opaque << endl;
  kdDebug(7113) << "(" << m_pid << ")   qop:       " << info.qop << endl;

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
  auth += m_request.url.encodedPathAndQuery(0, true);

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
  QString header;

  // We keep proxy authentication locally until they are changed.
  // Thus, no need to check with the password manager for every
  // connection.
  if ( m_strProxyRealm.isEmpty() )
  {
    AuthInfo info;
    info.url = m_proxyURL;
    info.username = m_proxyURL.user();
    info.password = m_proxyURL.pass();
    info.verifyPath = true;

    // If the proxy URL already contains username
    // and password simply attempt to retrieve it
    // without prompting the user...
    if ( !info.username.isNull() && !info.password.isNull() )
    {
      if( m_strProxyAuthorization.isEmpty() )
        ProxyAuthentication = AUTH_None;
      else if( m_strProxyAuthorization.startsWith("Basic") )
        ProxyAuthentication = AUTH_Basic;
      else if( m_strProxyAuthorization.startsWith("NTLM") )
        ProxyAuthentication = AUTH_NTLM;
      else
        ProxyAuthentication = AUTH_Digest;
    }
    else
    {
      if ( checkCachedAuthentication(info) && !info.digestInfo.isEmpty() )
      {
        m_proxyURL.setUser( info.username );
        m_proxyURL.setPass( info.password );
        m_strProxyRealm = info.realmValue;
        m_strProxyAuthorization = info.digestInfo;
        if( m_strProxyAuthorization.startsWith("Basic") )
          ProxyAuthentication = AUTH_Basic;
        else if( m_strProxyAuthorization.startsWith("NTLM") )
          ProxyAuthentication = AUTH_NTLM;
        else
          ProxyAuthentication = AUTH_Digest;
      }
      else
      {
        ProxyAuthentication = AUTH_None;
      }
    }
  }

  /********* Only for debugging purpose... *********/
  if ( ProxyAuthentication != AUTH_None )
  {
    kdDebug(7113) << "(" << m_pid << ") Using Proxy Authentication: " << endl;
    kdDebug(7113) << "(" << m_pid << ")   HOST= " << m_proxyURL.host() << endl;
    kdDebug(7113) << "(" << m_pid << ")   PORT= " << m_proxyURL.port() << endl;
    kdDebug(7113) << "(" << m_pid << ")   USER= " << m_proxyURL.user() << endl;
    kdDebug(7113) << "(" << m_pid << ")   PASSWORD= [protected]" << endl;
    kdDebug(7113) << "(" << m_pid << ")   REALM= " << m_strProxyRealm << endl;
    kdDebug(7113) << "(" << m_pid << ")   EXTRA= " << m_strProxyAuthorization << endl;
  }

  switch ( ProxyAuthentication )
  {
    case AUTH_Basic:
      header += createBasicAuth( true );
      break;
    case AUTH_Digest:
      header += createDigestAuth( true );
      break;
    case AUTH_NTLM:
      if ( m_bFirstRequest ) header += createNTLMAuth( true );
      break;
    case AUTH_None:
    default:
      break;
  }

  return header;
}

#include "http.moc"
