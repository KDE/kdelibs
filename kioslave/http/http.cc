/*
   Copyright (C) 2000,2001 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2000,2001 George Staikos <staikos@kde.org>
   Copyright (C) 2000,2001 Dawit Alemayehu <adawit@kde.org>
   Copyright (C) 2001,2002 Hamish Rodda <meddie@yoyo.cc.monash.edu.au>


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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>         // Needed on some systems.
#endif

#ifdef HAVE_LIBZ
#define DO_GZIP
#endif

#ifdef DO_GZIP
#include <zlib.h>
#endif

#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <utime.h>
#include <errno.h>
#include <netdb.h>

#include <qregexp.h>
#include <qfile.h>
#include <qdom.h>
#include <qdatetime.h>
#include <qstringlist.h>

#include <kapplication.h>
#include <kurl.h>
#include <ksocks.h>
#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
#include <kextsock.h>
#include <kstandarddirs.h>
#include <kservice.h>
#include <krfcdate.h>
#include <kinstance.h>
#include <kmimemagic.h>
#include <dcopclient.h>
#include <kdatastream.h>
#include <kmdcodec.h>

#include "kio/ioslave_defaults.h"
#include "kio/http_slave_defaults.h"

#include "http.h"

using namespace KIO;

extern "C" {
  int kdemain(int argc, char **argv);
};

int kdemain( int argc, char **argv )
{
  KLocale::setMainCatalogue("kdelibs");
  KInstance instance( "kio_http" );
  ( void ) KGlobal::locale();

  kdDebug(7113) << "Starting : " << getpid() << endl;

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

/************************************** HTTPProtocol **********************************************/

HTTPProtocol::HTTPProtocol( const QCString &protocol, const QCString &pool,
                            const QCString &app )
             :TCPSlaveBase( 0, protocol , pool, app,
                            (protocol == "https" || protocol == "webdavs") )
{
  m_requestQueue.setAutoDelete(true);
  m_lineBufUnget = 0;
  m_protocol = protocol;
  m_bKeepAlive = false;
  m_bUseCache = true;
  m_bBusy = false;
  m_maxCacheAge = 0;
  m_fcache = 0;
  m_iSize = -1;
  m_HTTPrev = HTTP_Unknown;

  m_proxyConnTimeout = DEFAULT_PROXY_CONNECT_TIMEOUT;
  m_remoteConnTimeout = DEFAULT_CONNECT_TIMEOUT;
  m_remoteRespTimeout = DEFAULT_RESPONSE_TIMEOUT;
  m_maxCacheAge = DEFAULT_MAX_CACHE_AGE;
  m_pid = getpid();

  m_dcopClient = new DCOPClient();

  if (!m_dcopClient->attach())
     kdDebug(7113) << "(" << m_pid << ") Can't connect with DCOP server." << endl;

  setMultipleAuthCaching( true );
  reparseConfiguration();
}

HTTPProtocol::~HTTPProtocol()
{
  delete m_dcopClient;
  m_bKeepAlive = false;
  httpClose();
}

void HTTPProtocol::reparseConfiguration()
{
  kdDebug(7113) << "(" << m_pid << ") Reparse Configuration!" << endl;

  m_strProxyRealm = QString::null;
  m_strProxyAuthorization = QString::null;
  ProxyAuthentication = AUTH_None;
  m_bUseProxy = false;

  if (m_protocol == "https" || m_protocol == "webdavs")
    m_iDefaultPort = DEFAULT_HTTPS_PORT;
  else if (m_protocol == "ftp")
    m_iDefaultPort = DEFAULT_FTP_PORT;
  else
    m_iDefaultPort = DEFAULT_HTTP_PORT;
}

void HTTPProtocol::resetSessionSettings()
{
  // Do not reset the URL on redirection if the proxy
  // URL, username or password has not changed!
  KURL proxy = config()->readEntry("UseProxy");

  if ( m_strProxyRealm.isEmpty() || !proxy.isValid() ||
       m_proxyURL.host() != proxy.host() ||
       (!proxy.user().isNull() && proxy.user() != m_proxyURL.user()) ||
       (!proxy.pass().isNull() && proxy.pass() != m_proxyURL.pass()) )
  {
    m_proxyURL = proxy;
    m_bUseProxy = m_proxyURL.isValid();

    kdDebug(7113) << "(" << m_pid << ") Using proxy: " << m_bUseProxy << endl;
    kdDebug(7113) << "(" << m_pid << ")   URL: " << m_proxyURL.url() << endl;
    kdDebug(7113) << "(" << m_pid << ")   Realm: " << m_strRealm << endl;
  }

  m_bUseCookiejar = config()->readBoolEntry("Cookies");
  m_bUseCache = config()->readBoolEntry("UseCache", true);
  m_strCacheDir = config()->readEntry("CacheDir");
  m_maxCacheAge = config()->readNumEntry("MaxCacheAge", DEFAULT_MAX_CACHE_AGE);
  m_request.window = config()->readEntry("window-id");

  bool sendReferrer = config()->readBoolEntry("SendReferrer", true);
  if ( sendReferrer )
     m_request.referrer = metaData("referrer");
  else
     m_request.referrer = QString::null;
     
  if (!m_request.referrer.startsWith("http"))
  {
     if (m_request.referrer.startsWith("webdav"))
        m_request.referrer.replace(0, 6, "http");
     else
        m_request.referrer = QString::null;
  }

  if ( config()->readBoolEntry("SendLanguageSettings", true) )
  {
      m_request.charsets = config()->readEntry( "Charsets",
                                           DEFAULT_FULL_CHARSET_HEADER );
      if ( m_request.charsets.isEmpty() )
        m_request.charsets += DEFAULT_PARIAL_CHARSET_HEADER;

      m_request.languages = config()->readEntry( "Languages",
                                            DEFAULT_LANGUAGE_HEADER );
  }
  else
  {
      m_request.charsets = QString::null;
      m_request.languages = QString::null;
  }

  // Adjust the offset value based on the "resume" meta-data.
  QString resumeOffset = metaData("resume");
  if ( !resumeOffset.isEmpty() )
     m_request.offset = resumeOffset.toInt(); // TODO: Convert to 64 bit
  else
     m_request.offset = 0;

  m_request.disablePassDlg = config()->readBoolEntry("DisablePassDlg", false);
  m_request.allowCompressedPage = config()->readBoolEntry("AllowCompressedPage", true);
  m_request.id = metaData("request-id");

  // Store user agent for this host.
  if ( config()->readBoolEntry("SendUserAgent", true) )
     m_request.userAgent = metaData("UserAgent");
  else
     m_request.userAgent = QString::null;

  // Deal with cache cleaning.
  // TODO: Find a smarter way to deal with cleaning the
  // cache ?
  if ( m_bUseCache )
    cleanCache();

  // Deal with HTTP tunneling
  if ( m_bIsSSL && m_bUseProxy && m_proxyURL.protocol() != "https" &&
       m_proxyURL.protocol() != "webdavs" && !m_bKeepAlive )
  {
    setEnableSSLTunnel( true );

    // HACK:  This is a temporary solution to fix the condition
    // where the realhost name was set for the previous request.
    // SSL tunneling definitely needs to be re-designed!
    setRealHost( m_request.hostname );

    kdDebug(7113) << "(" << m_pid << ") Setting real host name to: " << m_request.hostname << endl;
  }
  else
  {
    // HACK:  This is a temporary solution to fix the condition
    // where the realhost name was set for the previous request.
    // SSL tunneling definitely needs to be re-designed!
    setRealHost( QString::null);
  }

  m_responseCode = 0;
  m_prevResponseCode = 0;

  m_strRealm = QString::null;
  m_strAuthorization = QString::null;
  Authentication = AUTH_None;

  // Obtain the proxy and remote server timeout values
  m_proxyConnTimeout = proxyConnectTimeout();
  m_remoteConnTimeout = connectTimeout();
  m_remoteRespTimeout = responseTimeout();

  setMetaData("request-id", m_request.id);

  m_bCanResume = false;
  m_bUnauthorized = false;
}

void HTTPProtocol::setHost( const QString& host, int port,
                            const QString& user, const QString& pass )
{
  kdDebug(7113) << "(" << m_pid << ") Hostname is now: " << host << endl;

  // Reset the webdav-capable flags for this host
  if ( m_request.hostname != host )
    m_davHostOk = m_davHostUnsupported = false;

  m_request.hostname = host;
  m_request.port = (port == 0) ? m_iDefaultPort : port;
  m_request.user = user;
  m_request.passwd = pass;

  m_bKeepAlive = false;
  m_bIsTunneled = false;
}

bool HTTPProtocol::checkRequestURL( const KURL& u )
{
  m_request.url = u;
  if (m_request.hostname.isEmpty())
  {
     error( KIO::ERR_UNKNOWN_HOST, i18n("No host specified!"));
     return false;
  }

  if ( m_protocol != u.protocol().latin1() )
  {
    short unsigned int oldDefaultPort = m_iDefaultPort;
    m_protocol = u.protocol().latin1();
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

  httpClose();
  // if data is required internally, don't finish,
  // it is processed before we finish()
  if ( !dataInternal )
    finished();
}

bool HTTPProtocol::retrieveHeader( bool close_connection )
{
  while ( 1 )
  {
    if (!httpOpen())
      return false;

    if (!readHeader())
    {
      if ( m_bError )
        return false;
    }
    else
    {
      // Do not save authorization if the current response code is
      // 4xx (client error) or 5xx (server error).
      kdDebug(7113) << "(" << m_pid << ") Previous Response: "
                    << m_prevResponseCode << endl;
      kdDebug(7113) << "(" << m_pid << ") Current Response: "
                    << m_responseCode << endl;

      if (m_responseCode < 400 && (m_prevResponseCode == 401 ||
          m_prevResponseCode == 407))
        saveAuthorization();

      if (isSSLTunnelEnabled() &&  m_bIsSSL && !m_bUnauthorized && !m_bError)
      {
        // Only disable tunneling if the error
        if ( m_responseCode < 400 )
        {
          kdDebug(7113) << "(" << m_pid << ") Unset tunneling flag!" << endl;
          setEnableSSLTunnel( false );
          m_bIsTunneled = true;
          continue;
        }
        else
        {
          if ( !m_bErrorPage )
          {
            kdDebug(7113) << "(" << m_pid << ") Sending an error message!" << endl;
            error( ERR_UNKNOWN_PROXY_HOST, m_proxyURL.host() );
            return false;
          }

          kdDebug(7113) << "(" << m_pid << ") Sending an error page!" << endl;
        }
      }
      break;
    }
  }

  if ( close_connection )
  {
    httpClose();
    finished();
  }

  return true;
}

void HTTPProtocol::stat(const KURL& url)
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::stat " << url.prettyURL()
                << endl;

  if ( !checkRequestURL( url ) )
    return;

  if ( m_protocol != "webdav" && m_protocol != "webdavs" )
  {
    UDSEntry entry;
    UDSAtom atom;
    atom.m_uds = KIO::UDS_NAME;
    atom.m_str = url.fileName();
    entry.append( atom );

    atom.m_uds = KIO::UDS_FILE_TYPE;
    atom.m_long = S_IFREG; // a file
    entry.append( atom );

    atom.m_uds = KIO::UDS_ACCESS;
    atom.m_long = S_IRUSR | S_IRGRP | S_IROTH; // readable by everybody
    entry.append( atom );

    statEntry( entry );
    finished();
    return;
  }

  davStatList( url );
}

void HTTPProtocol::listDir( const KURL& url )
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::listDir " << url.url()
                << endl;

  if ( !checkRequestURL( url ) )
    return;

  davStatList( url, false );
}

void HTTPProtocol::davStatList( const KURL& url, bool stat )
{
  UDSEntry entry;
  UDSAtom atom;

  // check to make sure this host supports WebDAV
  if ( !davHostOk() )
    return;

  // WebDAV Stat or List...
  m_request.method = DAV_PROPFIND;
  m_request.query = QString::null;
  m_request.cache = CC_Reload;
  m_request.doProxy = m_bUseProxy;
  m_request.davData.depth = stat ? 0 : 1;
  if (!stat)
     m_request.url.adjustPath(+1);

  retrieveContent( true );

  QDomDocument multiResponse;
  multiResponse.setContent( m_intData, true );

  for ( QDomElement thisResponse = multiResponse.documentElement().firstChild().toElement();
        !thisResponse.isNull();
        thisResponse = thisResponse.nextSibling().toElement() )
  {
    QDomElement href = thisResponse.namedItem( "href" ).toElement();
    if ( !href.isNull() )
    {
      entry.clear();

      KURL thisURL = KURL::decode_string( href.text() );

      // don't list the base dir of a listDir()
      if ( !stat && thisURL.path(+1).length() == url.path(+1).length() )
        continue;

      atom.m_uds = KIO::UDS_NAME;
      atom.m_str = thisURL.filename();
      entry.append( atom );

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

  if ( stat )
  {
    error( ERR_DOES_NOT_EXIST, url.prettyURL() );
  }
  else
  {
    listEntry( entry, true );
    finished();
  }
}

int HTTPProtocol::codeFromResponse( const QString& response )
{
  int firstSpace = response.find( ' ' );
  int secondSpace = response.find( ' ', firstSpace + 1 );
  return response.mid( firstSpace + 1, secondSpace - firstSpace - 1 ).toInt();
}

void HTTPProtocol::davParsePropstats( const QDomNodeList& propstats, UDSEntry& entry )
{
  UDSAtom atom;
  bool foundExecutable = false;
  bool foundContentType = false;
  bool isDirectory = false;
  uint lockCount = 0;
  uint supportedLockCount = 0;

  for ( uint i = 0; i < propstats.count(); i++)
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
      kdDebug(7113) << "Error: status code " << code << endl;
      continue;
    }

    QDomElement prop = propstat.namedItem( "prop" ).toElement();
    if ( prop.isNull() )
    {
      kdDebug(7113) << "Error: no prop segment in this propstat." << endl;
      return;
    }

    for ( QDomElement property = prop.firstChild().toElement();
          !property.isNull();
          property = property.nextSibling().toElement() )
    {

      if ( property.namespaceURI() != "DAV:" )
      {
        // break out - we're only interested in properties from the DAV namespace
        continue;
      }

      if ( property.tagName() == "creationdate" )
      {
        // Resource creation date. Should be is ISO 8601 format.
        atom.m_uds = KIO::UDS_CREATION_TIME;
        atom.m_long = parseDateTime( property.text(), property.attribute("dt") );
        entry.append( atom );
      }
      else if ( property.tagName() == "getcontentlength" )
      {
        // Content length (file size)
        atom.m_uds = KIO::UDS_SIZE;
        atom.m_long = property.text().toULong();
        entry.append( atom );
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
          if ( !isDirectory )
          {
            atom.m_uds = KIO::UDS_FILE_TYPE;
            atom.m_long = S_IFDIR;
            entry.append( atom );

            isDirectory = true;
          }
        } 
        else if ( property.text() != "" )
        {
          atom.m_uds = KIO::UDS_FILE_TYPE;
          atom.m_long = S_IFREG;
          entry.append( atom );

          atom.m_uds = KIO::UDS_MIME_TYPE;
          atom.m_str = property.text();
          entry.append( atom );

          foundContentType = true;
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
        atom.m_uds = KIO::UDS_MODIFICATION_TIME;
        atom.m_long = parseDateTime( property.text(), property.attribute("dt") );
        entry.append( atom );

      }
      else if ( property.tagName() == "getetag" )
      {
        // Entity tag
        setMetaData( "davEntityTag", property.text() );
      }
      else if ( property.tagName() == "supportedlock" )
      {
        // Supported locking specifications
        for ( QDomElement lockEntry = property.firstChild().toElement();
              !lockEntry.isNull();
              lockEntry = lockEntry.nextSibling().toElement() )
        {
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
          if ( !isDirectory )
          {
            atom.m_uds = KIO::UDS_FILE_TYPE;
            atom.m_long = S_IFDIR;
            entry.append( atom );
            isDirectory = true;
          }
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

  if ( foundExecutable || isDirectory )
  {
    // File was executable, or is a directory.
    atom.m_uds = KIO::UDS_ACCESS;
    atom.m_long = 0700;
    entry.append(atom);
  }
  else
  {
    atom.m_uds = KIO::UDS_ACCESS;
    atom.m_long = 0600;
    entry.append(atom);
  }

  if ( !foundContentType && !isDirectory )
  {
    // No type specified for this resource. Assume file.
    atom.m_uds = KIO::UDS_FILE_TYPE;
    atom.m_long = S_IFREG;
    entry.append( atom );
  }
}

void HTTPProtocol::davParseActiveLocks( const QDomNodeList& activeLocks,
                                        uint& lockCount )
{
  for ( uint i = 0; i < activeLocks.count(); i++ )
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

  return QString::null;
}

bool HTTPProtocol::davHostOk()
{
  // FIXME needs to be reworked. Switched off for now.
  return true;

  // cached?
  if ( m_davHostOk )
  {
    // Call a HTTP OPTIONS to find out if we're good...
    kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::davHostOk: true" << endl;
    return true;
  }
  else if ( m_davHostUnsupported )
  {
    // Call a HTTP OPTIONS to find out if we're good...
    kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::davHostOk: false" << endl;
    davError( -2 );
    return false;
  }

  m_request.method = HTTP_OPTIONS;

  // query the server's capabilities generally, not for a specific URL
  m_request.path = "*";
  m_request.query = QString::null;
  m_request.cache = CC_Reload;
  m_request.doProxy = m_bUseProxy;

  // clear davVersions variable, which holds the response to the DAV: header
  m_davVersions = QString::null;

  retrieveHeader( false );

  if ( m_davVersions != QString::null )
  {
    QStringList vers = QStringList::split( ',', m_davVersions );
    for (QStringList::iterator it = vers.begin(); it != vers.end(); it++)
    {
      uint verNo = (*it).toUInt();
      if ( verNo > 0 && verNo < 3 )
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
  httpClose();
  finished();
}

void HTTPProtocol::mkdir( const KURL& url, int )
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::mkdir " << url.url()
                << endl;

  if ( !checkRequestURL( url ) )
    return;

  m_request.method = DAV_MKCOL;
  m_request.path = url.path();
  m_request.query = QString::null;
  m_request.cache = CC_Reload;
  m_request.doProxy = m_bUseProxy;

  // Never use a keep-alive connection for MKDIR
  httpCloseConnection();

  retrieveHeader( false );

  if ( m_responseCode == 201 )
    davFinished();
  else
    davError();
}

void HTTPProtocol::get( const KURL& url )
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

void HTTPProtocol::put( const KURL &url, int, bool, bool)
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::put " << url.prettyURL()
                << endl;

  if ( !checkRequestURL( url ) )
    return;

  m_request.method = HTTP_PUT;
  m_request.path = url.path();
  m_request.query = QString::null;
  m_request.cache = CC_Reload;
  m_request.doProxy = m_bUseProxy;

  // Never use a keep-alive connection for PUT
  httpCloseConnection();

  retrieveHeader( true );
}

void HTTPProtocol::copy( const KURL& src, const KURL& dest, int, bool overwrite )
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::copy " << src.prettyURL()
                << " -> " << dest.prettyURL() << endl;

  if ( !checkRequestURL( dest ) || !checkRequestURL( src ) )
    return;

  // destination has to be "http://..."
  KURL newDest = dest;
  newDest.setProtocol( "http" );

  m_request.method = DAV_COPY;
  m_request.path = src.path();
  m_request.davData.desturl = newDest.url();
  m_request.davData.overwrite = overwrite;
  m_request.query = QString::null;
  m_request.cache = CC_Reload;
  m_request.doProxy = m_bUseProxy;

  // Never use a keep-alive connection for COPY
  httpCloseConnection();

  retrieveHeader( false );

  // The server returns a HTTP/1.1 201 Created or 204 No Content on successful completion
  if ( m_responseCode == 201 || m_responseCode == 204 )
    davFinished();
  else
    davError();
}

void HTTPProtocol::rename( const KURL& src, const KURL& dest, bool overwrite )
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::rename " << src.prettyURL()
                << " -> " << dest.prettyURL() << endl;

  if ( !checkRequestURL( dest ) || !checkRequestURL( src ) )
    return;

  // destination has to be "http://..."
  KURL newDest = dest;
  newDest.setProtocol( "http" );

  m_request.method = DAV_MOVE;
  m_request.path = src.path();
  m_request.davData.desturl = newDest.url();
  m_request.davData.overwrite = overwrite;
  m_request.query = QString::null;
  m_request.cache = CC_Reload;
  m_request.doProxy = m_bUseProxy;

  // Never use a keep-alive connection for MOVE
  httpCloseConnection();

  retrieveHeader( false );

  if ( m_responseCode == 201 )
    davFinished();
  else
    davError();
}

void HTTPProtocol::del( const KURL& url, bool )
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::del " << url.prettyURL()
                << endl;

  if ( !checkRequestURL( url ) )
    return;

  m_request.method = HTTP_DELETE;
  m_request.path = url.path();
  m_request.query = QString::null;
  m_request.cache = CC_Reload;
  m_request.doProxy = m_bUseProxy;

  // Never use a keep-alive connection for DELETE
  httpCloseConnection();

  retrieveHeader( false );

  // The server returns a HTTP/1.1 200 Ok or HTTP/1.1 204 No Content
  // on successful completion
  if ( m_responseCode == 200 || m_responseCode == 204 )
    davFinished();
  else
    davError();
}

void HTTPProtocol::post( const KURL& url )
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

  // Never use a keep-alive connection for POST
  httpCloseConnection();

  retrieveContent();
}

void HTTPProtocol::davLock( const KURL& url, const QString& scope,
                            const QString& type, const QString& owner )
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::davLock "
                << url.prettyURL() << endl;

  if ( !checkRequestURL( url ) )
    return;

  m_request.method = DAV_LOCK;
  m_request.path = url.path();
  m_request.query = QString::null;
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

  if ( owner != QString::null) {
    QDomElement ownerElement = lockReq.createElement( "owner" );
    lockReq.appendChild( ownerElement );

    QDomElement ownerHref = lockReq.createElement( "href" );
    ownerElement.appendChild( ownerHref );

    ownerHref.appendChild( lockReq.createTextNode( owner ) );
  }

  // insert the document into the POST buffer
  m_bufPOST = lockReq.toCString();

  // Never use a keep-alive connection for LOCK
  httpCloseConnection();

  retrieveContent( true );

  if ( m_responseCode == 200 ) {
    // success
    QDomDocument multiResponse;
    multiResponse.setContent( m_intData, true );

    QDomElement prop = multiResponse.documentElement().namedItem( "prop" ).toElement();

    QDomElement lockdiscovery = prop.namedItem( "lockdiscovery" ).toElement();

    uint lockCount = 0;
    davParseActiveLocks( lockdiscovery.elementsByTagName( "activelock" ), lockCount );

    setMetaData( "davLockCount", QString("%1").arg( lockCount ) );

    finished();

  } else
    davError();
}

void HTTPProtocol::davUnlock( const KURL& url )
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::davUnlock "
                << url.prettyURL() << endl;

  if ( !checkRequestURL( url ) )
    return;

  m_request.method = DAV_UNLOCK;
  m_request.path = url.path();
  m_request.query = QString::null;
  m_request.cache = CC_Reload;
  m_request.doProxy = m_bUseProxy;

  // Never use a keep-alive connection for UNLOCK
  httpCloseConnection();

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

  if ( url != QString::null )
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
      action = i18n( "create the requested directory" );
      break;
    case DAV_COPY:
      action = i18n( "copy the specified file or directory" );
      break;
    case DAV_MOVE:
      action = i18n( "move the specified file or directory" );
      break;
    case DAV_LOCK:
      action = i18n( "lock the specified file or directory" );
      break;
    case DAV_UNLOCK:
      action = i18n( "unlock the specified file or directory" );
      break;
    case HTTP_DELETE:
      action = i18n( "delete the specified file or directory" );
      break;
    case HTTP_OPTIONS:
      action = i18n( "query the server's capabilities" );
      break;
    case HTTP_GET:
      action = i18n( "retrieve the contents of the specified file or directory" );
      break;
    case HTTP_PUT:
    case HTTP_POST:
    case HTTP_HEAD:
    default:
      // this should not happen, this function is for webdav errors only
      assert(0);
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
        return QString::null;

      QStringList errors;
      QDomDocument multiResponse;

      multiResponse.setContent( m_intData, true );

      QDomElement multistatus = multiResponse.documentElement().namedItem( "multistatus" ).toElement();

      QDomNodeList responses = multistatus.elementsByTagName( "response" );

      for (uint i = 0; i < responses.count(); i++)
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
        errorString = i18n("The specified directory already exists.");
      }
      break;
    case 409:
      // 409 Conflict
      kError = ERR_ACCESS_DENIED;
      errorString = i18n("A resource cannot be created at the destination "
                  "until one or more intermediate collections (directories) "
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
                           "to accept the file or directory.").arg( action );
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
  {
    httpClose();
    error( ERR_SLAVE_DEFINED, errorString );
  }

  return errorString;
}

void HTTPProtocol::multiGet(const QByteArray &data)
{
  QDataStream stream(data, IO_ReadOnly);
  Q_UINT32 n;
  stream >> n;

  kdDebug(7113) << "(" << m_pid << ") HTTPProtcool::multiGet n = " << n << endl;

//  m_requestQueue.clear();
  for(unsigned i = 0; i < n; i++)
  {
     KURL url;
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

  if (!m_bBusy)
  {
     m_bBusy = true;
     while(!m_requestQueue.isEmpty())
     {
        HTTPRequest *request = m_requestQueue.take(0);
        m_request = *request;
        delete request;
        retrieveContent();
     }
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
      // a valid exception(s) occured, let's retry...
      if (n < 0 && ((errno == EINTR) || (errno == EAGAIN)))
        continue;
      // some other error occured ?
      return -1;
    }

    nbytes -= n;
    buf += n;
    bytes_sent += n;
  }

  return bytes_sent;
}

char *HTTPProtocol::gets (char *s, int size)
{
  int len=0;
  char *buf=s;
  char mybuf[2]={0,0};

  while (len < size)
  {
    read(mybuf, 1);

    if (*mybuf == '\r') // Ignore!
      continue;

    if (*mybuf == '\n')
      break;

    *buf++ = *mybuf;
    len++;
  }

  *buf=0;
  return s;
}

void HTTPProtocol::ungets(char *str, int size)
{
  char *newbuf = (char *) malloc(size+1+m_lineCountUnget);
  memcpy(newbuf, str, size );
  newbuf[size] = '\n';
  if (m_lineCountUnget)
     memcpy(newbuf+size+1, m_linePtrUnget, m_lineCountUnget);

  if (m_lineBufUnget)
     free(m_lineBufUnget);

  m_lineBufUnget = newbuf;
  m_linePtrUnget = newbuf;
  m_lineCountUnget = size+1+m_lineCountUnget;
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

    if (m_lineCountUnget == 0)
    {
      free(m_lineBufUnget);
      m_lineBufUnget = 0;
    }

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

    ret = TCPSlaveBase::read( b, nbytes);
    if (ret == 0)
      m_bEOF = true;

  } while (( ret == -1) && ((errno == EAGAIN) || (errno == EINTR)));

  return ret;
}


void HTTPProtocol::httpCheckConnection()
{
  if ( m_iSock != -1 )
  {
     bool closeDown = false;
     if ( m_request.doProxy && m_state.doProxy )
     {
        // Keep the connection to the proxy.
     }
     else if ( !m_state.doProxy && !m_request.doProxy )
     {
        if (m_state.hostname != m_request.hostname)
        {
           closeDown = true;
        }
        else if (m_state.port != m_request.port)
        {
           closeDown = true;
        }
        else if (m_state.user != m_request.user)
        {
           closeDown = true;
        }
        else if (m_state.passwd != m_request.passwd)
        {
           closeDown = true;
        }
     }
     else
     {
        closeDown = true;
     }

     if (!closeDown && !isConnectionValid())
        closeDown = true;

     if (closeDown)
        httpCloseConnection();
  }

  // Let's update our current state
  m_state.hostname = m_request.hostname;
  m_state.port = m_request.port;
  m_state.user = m_request.user;
  m_state.passwd = m_request.passwd;
  m_state.doProxy = m_request.doProxy;
}

bool HTTPProtocol::httpOpenConnection()
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::httpOpenConnection"
                << endl;

  m_bKeepAlive = false;
  setBlockConnection( true );

  int errCode;
  QString errMsg;

  if ( m_state.doProxy )
  {
    QString proxy_host = m_proxyURL.host();
    int proxy_port = m_proxyURL.port();

    kdDebug(7113) << "(" << getpid() << ") Connecting to proxy server: "
                  << proxy_host << ", port: " << proxy_port << endl;

    infoMessage( i18n("Connecting to <b>%1</b>...").arg(m_state.hostname) );

    setConnectTimeout( m_proxyConnTimeout );

    if ( !connectToHost(proxy_host, proxy_port, false) )
    {
      if (userAborted()) {
        error(ERR_NO_CONTENT, "");
        return false;
      }

      switch ( connectResult() )
      {
        case IO_LookupError:
          errMsg = proxy_host;
          errCode = ERR_UNKNOWN_PROXY_HOST;
          break;
        case IO_TimeOutError:
          errMsg = i18n("Timed out while waiting to connect to %1").arg(proxy_host);
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
    if ( !connectToHost(m_state.hostname, m_state.port, false ) )
    {
      if (userAborted()) {
        error(ERR_NO_CONTENT, "");
        return false;
      }

      switch ( connectResult() )
      {
        case IO_LookupError:
          errMsg = m_state.hostname;
          errCode = ERR_UNKNOWN_HOST;
          break;
        case IO_TimeOutError:
          errMsg = i18n("Timed out while waiting to connect to %1").arg(m_state.hostname);
          errCode = ERR_SERVER_TIMEOUT;
          break;
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
  int on = 1;
  (void) setsockopt( m_iSock, IPPROTO_TCP, TCP_NODELAY, (char*)&on, sizeof(on) );
  kdDebug(7113) << "(" << getpid() << ") Sending connected @ " << time(0L) << endl;

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
  // only happen if InitializeSSL() call failed which in turn means that SSL
  // is not supported by current installation !!
  if ( (m_protocol == "https" || m_protocol == "webdavs") && !m_bIsSSL )
  {
    error( ERR_UNSUPPORTED_PROTOCOL, m_protocol );
    return false;
  }

  m_fcache = 0;
  m_bCachedRead = false;
  m_bCachedWrite = false;
  m_bMustRevalidate = false;

  if (m_bUseCache)
  {
     m_fcache = checkCacheEntry( );

     if (m_request.cache == CC_Reload && m_fcache)
     {
        if (m_fcache)
          fclose(m_fcache);
        m_fcache = 0;
     }

     m_bCachedWrite = true;

     if (m_fcache && !m_bMustRevalidate)
     {
        // Cache entry is OK.
        m_bCachedRead = true; // Cache hit.
        return true;
     }
     else if (!m_fcache)
     {
        m_bMustRevalidate = false; // Cache miss
     }
     else
     {
        // Conditional cache hit. (Validate)
     }

     if (m_request.cache == CC_CacheOnly)
     {
        error( ERR_DOES_NOT_EXIST, m_request.url.url() );
        return false;
     }
  }

  // Clear out some things so that bogus values aren't used.
  m_bEOF = false;
  m_bError = false;
  m_bChunked = false;

  m_iSize = -1;
  m_lineCount = 0;
  m_iWWWAuthCount = 0;
  m_lineCountUnget = 0;
  m_iProxyAuthCount = 0;

  m_qContentEncodings.clear();
  m_qTransferEncodings.clear();
  m_sContentMD5 = QString::null;
  m_strMimeType = QString::null;
  m_bErrorPage = (metaData("errorPage") != "false");


  httpCheckConnection();

  // Let's try to open up our socket if we don't have one already.
  if ( m_iSock == -1)
  {
    if (!httpOpenConnection())
       return false;
  }
  else
  {
    setSSLMetaData();
  }

  // Clean up previous POST
  bool moreData = false;
  bool davData = false;

  // Variable to hold the entire header...
  QString header;
  QString davHeader;

  // Determine if this is a POST or GET method
  switch ( m_request.method)
  {
  case HTTP_GET:
      header = "GET ";
      break;
  case HTTP_PUT:
      header = "PUT ";
      moreData = !isSSLTunnelEnabled();
      m_bCachedWrite = false; // Do not put any result in the cache
      break;
  case HTTP_POST:
      header = "POST ";
      moreData = !isSSLTunnelEnabled();
      m_bCachedWrite = false; // Do not put any result in the cache
      break;
  case HTTP_HEAD:
      header = "HEAD ";
      break;
  case HTTP_DELETE:
      header = "DELETE ";
      m_bCachedWrite = false; // Do not put any result in the cache
      break;
  case HTTP_OPTIONS:
      header = "OPTIONS ";
      m_bCachedWrite = false; // Do not put any result in the cache
      break;
  case DAV_PROPFIND:
      header = "PROPFIND ";
      davHeader = "Depth: ";
      if ( m_request.davData.depth == 2 )
        davHeader += "infinity";
      else
        davHeader += QString("%1").arg( m_request.davData.depth );
      davHeader += "\r\n";
      m_bCachedWrite = false; // Do not put any result in the cache
      break;
  case DAV_PROPPATCH:
      header = "PROPPATCH ";
      davData = true;
      m_bCachedWrite = false; // Do not put any result in the cache
      break;
  case DAV_MKCOL:
      header = "MKCOL ";
      m_bCachedWrite = false; // Do not put any result in the cache
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
      m_bCachedWrite = false; // Do not put any result in the cache
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
          davHeader += "Seconds-" + timeout;
      }
      davHeader += "\r\n";
      m_bCachedWrite = false; // Do not put any result in the cache
      davData = true;
      break;
  case DAV_UNLOCK:
      header = "UNLOCK ";
      davHeader = "Lock-token: " + metaData("davLockToken") + "\r\n";
      m_bCachedWrite = false; // Do not put any result in the cache
      break;
  }

  if ( isSSLTunnelEnabled() )
  {
    header = QString("CONNECT %1:%2 HTTP/1.1"
                     "\r\n").arg( m_request.hostname).arg(m_request.port);

    // Identify who you are to the proxy server!
    if (!m_request.userAgent.isEmpty())
        header += "User-Agent: " + m_request.userAgent + "\r\n";

    /* Add hostname information */
    header += "Host: ";
    if (m_state.hostname.find(':') != -1)
    {
      // This is an IPv6 (not hostname)
      header += '[';
      header += m_state.hostname;
      header += ']';
    }
    else
    {
      header += m_state.hostname;
    }

    if (m_state.port != m_iDefaultPort)
      header += QString(":%1").arg(m_state.port);
    header += "\r\n";

    header += proxyAuthenticationHeader();
  }
  else
  {
    // format the URI
    if (m_state.doProxy && !m_bIsTunneled)
    {
      KURL u;
      // u.setUser( m_state.user );
      if (m_protocol == "http")
         u.setProtocol( "http" );
      else if (m_protocol == "https" )
         u.setProtocol( "https" );
      if (m_protocol == "webdav")
         u.setProtocol( "http" );
      else if (m_protocol == "webdavs" )
         u.setProtocol( "https" );
      else
         u.setProtocol( m_protocol );

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
#if 0
    // Don't send Keep-Alive it may confuse some proxies and
    // it shouldn't be necassery for HTTP 1.1 servers.
    // We also don't support keep-alive connections with 1.0 servers
    // at the moment.
    header += "Connection: Keep-Alive\r\n";
#endif

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

    if (m_bMustRevalidate)
    {
      /* conditional get */
      if (!m_etag.isEmpty())
        header += "If-None-Match: "+m_etag+"\r\n";
      if (!m_lastModified.isEmpty())
        header += "If-Modified-Since: "+m_lastModified+"\r\n";
    }

    header += "Accept: ";
    QString acceptHeader = metaData("accept");
    if (!acceptHeader.isEmpty())
      header += acceptHeader;
    else
      header += DEFAULT_ACCEPT_HEADER;
    header += "\r\n";

    if (m_request.allowCompressedPage)
    {
#ifdef DO_GZIP
      // Content negotiation
      // header += "Accept-Encoding: x-gzip, x-deflate, gzip, deflate, identity\r\n";
      header += "Accept-Encoding: x-gzip, gzip, identity\r\n";
#endif
    }

    if (!m_request.charsets.isEmpty())
      header += "Accept-Charset: " + m_request.charsets + "\r\n";

    if (!m_request.languages.isEmpty())
      header += "Accept-Language: " + m_request.languages + "\r\n";


    /* support for virtual hosts and required by HTTP 1.1 */
    header += "Host: ";
    if (m_state.hostname.find(':') != -1)
    {
      // This is an IPv6 (not hostname)
      header += '[';
      header += m_state.hostname;
      header += ']';
    }
    else
    {
      header += m_state.hostname;
    }

    if (m_state.port != m_iDefaultPort)
      header += QString(":%1").arg(m_state.port);
    header += "\r\n";

    QString cookieStr;
    QString cookieMode = metaData("cookies").lower();
    if (cookieMode == "none")
    {
      m_cookieMode = CookiesNone;
    }
    else if (cookieMode == "manual")
    {
      m_cookieMode = CookiesManual;
      cookieStr = metaData("setcookies");
    }
    else
    {
      m_cookieMode = CookiesAuto;
      if (m_bUseCookiejar)
        cookieStr = findCookies( m_request.url.url());
    }

    if (!cookieStr.isEmpty())
      header += cookieStr + "\r\n";

    if (m_request.method == HTTP_POST)
    {
      header += metaData("content-type");
      header += "\r\n";
    }

    // Only check for a cached copy if the previous
    // response was NOT a 401 or 407.
    if ( m_responseCode != 401 && m_responseCode != 407 )
    {
      AuthInfo info;
      info.url = m_request.url;
      info.verifyPath = true;
      if ( !m_request.user.isEmpty() )
        info.username = m_request.user;
      if ( checkCachedAuthentication( info ) )
      {
        Authentication = info.digestInfo.isEmpty() ? AUTH_Basic : AUTH_Digest ;
        m_state.user   = info.username;
        m_state.passwd = info.password;
        m_strRealm = info.realmValue;
        if ( Authentication == AUTH_Digest )
          m_strAuthorization = info.digestInfo;
      }
    }

    switch ( Authentication )
    {
      case AUTH_Basic:
          header += createBasicAuth();
          header += "\r\n";
          break;
      case AUTH_Digest:
          header += createDigestAuth();
          header += "\r\n";
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
      header += proxyAuthenticationHeader();
  }

  // add extra header elements for WebDAV
  if ( !davHeader.isNull() )
    header += davHeader;

  if ( m_protocol == "webdav" || m_protocol == "webdavs" )
    header += davProcessLocks();

  if ( !moreData )
    header += "\r\n";  /* end header */

  kdDebug(7103) << "(" << m_pid << ") ============ Sending Header:" << endl;

  QStringList headerOutput = QStringList::split("\r\n", header);
  QStringList::Iterator it = headerOutput.begin();

  for (; it != headerOutput.end(); it++)
    kdDebug(7103) << "(" << m_pid << ") " << (*it) << endl;

  // now that we have our formatted header, let's send it!
  bool sendOk;
  sendOk = (write(header.latin1(), header.length()) == (ssize_t) header.length());
  if (!sendOk)
  {
    kdDebug(7113) << "(" << m_pid << ") httpOpen: Connection broken! ("
                  << m_state.hostname << ")" << endl;
    if (m_bKeepAlive)
    {
       // With a Keep-Alive connection this can happen.
       // Just reestablish the connection.
       httpCloseConnection();
       if (!httpOpenConnection())
          return false;
       sendOk = (write(header.latin1(), header.length()) == (ssize_t) header.length());
    }
    if (!sendOk)
    {
       kdDebug(7113) << "(" << m_pid << ") httpOpen: sendOk==false. Connnection"
                        " broken ! " << endl;
       error( ERR_CONNECTION_BROKEN, m_state.hostname );
       return false;
    }
  }

  bool res = true;

  if ( moreData )
    res = sendBody();
  else if ( davData )
    res = sendBody( true );

  infoMessage( i18n( "<b>%1</b> contacted. "
                     "Waiting for reply..." ).arg( m_request.hostname ) );
  return res;
}


/**
 * This function will read in the return header from the server.  It will
 * not read in the body of the return message.  It will also not transmit
 * the header to our client as the client doesn't need to know the gory
 * details of HTTP headers.
 */
bool HTTPProtocol::readHeader()
{
  // Check
  if (m_bCachedRead)
  {
     // Read header from cache...
     char buffer[4097];
     if (!fgets(buffer, 4096, m_fcache) )
     {
        // Error, delete cache entry
        kdDebug(7103) << "(" << m_pid << ") HTTPProtocol::readHeader: "
                      << "Could not access cache to obtain mimetype!" << endl;
        error( ERR_CONNECTION_BROKEN, m_state.hostname );
        return false;
     }

     kdDebug(7103) << "(" << m_pid << ") HTTPProtocol::readHeader: cached "
                   << "data mimetype: " << buffer << endl;

     m_strMimeType = QString::fromUtf8( buffer).stripWhiteSpace();
     mimeType(m_strMimeType);

     if (!fgets(buffer, 4096, m_fcache) )
     {
        // Error, delete cache entry
        kdDebug(7103) << "(" << m_pid << ") HTTPProtocol::readHeader: "
                      << "Could not access cached data! " << endl;
        error( ERR_CONNECTION_BROKEN, m_state.hostname );
        return false;
     }

     m_strCharset = QString::fromUtf8( buffer).stripWhiteSpace().lower();
     setMetaData("charset", m_strCharset);
     if (!m_lastModified.isEmpty())
         setMetaData("modified", m_lastModified);
     QString tmp;
     tmp.setNum(m_expireDate);
     setMetaData("expire-date", tmp);
     return true;
  }

  QCString locationStr; // In case we get a redirect.
  QCString cookieStr; // In case we get a cookie.
  QString disposition; // Incase we get a Content-Disposition
  QString mediaValue;
  QString mediaAttribute;
  QStringList responseHeader;


  m_etag = QString::null;
  m_lastModified = QString::null;
  m_strCharset = QString::null;

  time_t dateHeader = 0;
  time_t expireDate = 0; // 0 = no info, 1 = already expired, > 1 = actual date
  int currentAge = 0;
  int maxAge = -1; // -1 = no max age, 0 already expired, > 0 = actual time

  // read in 4096 bytes at a time (HTTP cookies can be quite large.)
  int len = 0;
  char buffer[4097];
  bool cont = false;
  bool cacheValidated = false; // Revalidation was successfull
  bool mayCache = true;
  bool hasCacheDirective = false;

  if (!waitForResponse(m_remoteRespTimeout))
  {
     // No response error
     error( ERR_SERVER_TIMEOUT , m_state.hostname );
     return false;
  }

  gets(buffer, sizeof(buffer)-1);
  if (m_bEOF)
  {
    kdDebug(7103) << "(" << m_pid << ")" << "HTTPProtocol::readHeader: "
                  << "EOF while waiting for header start." << endl;
    if (m_bKeepAlive) // Try to reestablish connection.
    {
      httpCloseConnection();
      if ( !httpOpen() )
        return false;

      if (!waitForResponse(m_remoteRespTimeout))
      {
        // No response error
        error( ERR_SERVER_TIMEOUT, m_state.hostname );
        return false;
      }
      gets(buffer, sizeof(buffer)-1);
    }

    if (m_bEOF)
    {
      if (m_request.method == HTTP_HEAD)
      {
        // HACK
        // Some web-servers fail to respond properly to a HEAD request.
        // We compensate for their failure to properly implement the HTTP standard
        // by assuming that they will be sending html.
        kdDebug(7103) << "(" << m_pid << ") HTTPPreadHeader: HEAD -> returne mimetype: " << DEFAULT_MIME_TYPE << endl;
        mimeType(QString::fromLatin1(DEFAULT_MIME_TYPE));
           return true;
      }
      
      kdDebug(7103) << "readHeader(3): Connnection broken ! " << endl;
      error( ERR_CONNECTION_BROKEN, m_state.hostname );
      return false;
    }
  }

  kdDebug(7103) << "(" << m_pid << ") ============ Received Response:"<< endl;

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

    kdDebug(7103) << "(" << m_pid << ") \"" << buffer << "\"" << endl;

    // Save broken servers from damnation!!
    char* buf = buffer;
    while( *buf == ' ' )
        buf++;

    if (strncasecmp(buf, "HTTP/", 5) == 0)
    {
      // Store the the headers so they can be passed to the
      // calling application later
      responseHeader << QString::fromLatin1(buf);

      if (strncmp((buf + 5), "1.0",3) == 0)
      {
        m_HTTPrev = HTTP_10;
        m_bKeepAlive = false;
      }
      else // Assume everything else to be 1.1 or higher....
      {
        m_HTTPrev = HTTP_11;

        // Connections with proxies are closed by default because
        // some proxies like junkbuster can't handle persistent
        // connections but don't tell us.
        // We will still use persistent connections if the proxy
        // sends us a "Connection: Keep-Alive" header.
        if (m_state.doProxy)
          m_bKeepAlive = false;
        else
          m_bKeepAlive = true; // HTTP 1.1 has persistant connections.
      }

      if ( m_responseCode )
        m_prevResponseCode = m_responseCode;

      m_responseCode = atoi(buf+9);

      // server side errors
      if (m_responseCode >= 500 && m_responseCode <= 599)
      {
        if (m_request.method == HTTP_HEAD); // Ignore error
        else
        {
           if (m_bErrorPage)
              errorPage();
           else
           {
              error(ERR_INTERNAL_SERVER, m_request.url.url());
              return false;
           }
        }
        m_bCachedWrite = false; // Don't put in cache
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
        m_bCachedWrite = false; // Don't put in cache
        mayCache = false;
      }
      // Any other client errors
      else if (m_responseCode >= 400 && m_responseCode <= 499)
      {
        // Tell that we will only get an error page here.
        if (m_bErrorPage)
          errorPage();
        else
        {
          error(ERR_DOES_NOT_EXIST, m_request.url.url());
          return false;
        }
        m_bCachedWrite = false; // Don't put in cache
        mayCache = false;
      }
      else if (m_responseCode == 307)
      {
        // 307 Temporary Redirect
        m_bCachedWrite = false; // Don't put in cache
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
        // 302 Found (temporary location)
        // 303 See Other
        if (m_request.method != HTTP_HEAD && m_request.method != HTTP_GET)
        {
           // NOTE: This is wrong according to RFC 2616.  However,
           // because most other existing user agent implementations
           // treat a 301/302 response as a 303 response and preform
           // a GET action regardless of what the previous method was,
           // many servers have simply adapted to this way of doing
           // things!!  Thus, we are forced to do the same thing or we
           // won't be able to retrieve these pages correctly!!  This
           // implementation is therefore only correct for a 303 response
           // according to RFC 2616 section 10.3.2/3/4/8
           m_request.method = HTTP_GET; // Force a GET
        }
        m_bCachedWrite = false; // Don't put in cache
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
          m_bCanResume = true;
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
            m_bCanResume = false;
    }

    // Cache control
    else if (strncasecmp(buf, "Cache-Control:", 14) == 0) {
      QStringList cacheControls = QStringList::split(',',
                                     QString::fromLatin1(trimLead(buf+14)));
      for(QStringList::ConstIterator it = cacheControls.begin();
          it != cacheControls.end();
          it++)
      {
         QString cacheControl = (*it).stripWhiteSpace();
         if (strncasecmp(cacheControl.latin1(), "no-cache", 8) == 0)
         {
            m_bCachedWrite = false; // Don't put in cache
            mayCache = false;
         }
         else if (strncasecmp(cacheControl.latin1(), "no-store", 8) == 0)
         {
            m_bCachedWrite = false; // Don't put in cache
            mayCache = false;
         }
         else if (strncasecmp(cacheControl.latin1(), "max-age=", 8) == 0)
         {
            maxAge = atol(cacheControl.mid(8).stripWhiteSpace().latin1());
         }
      }
      hasCacheDirective = true;
    }

    // get the size of our data
    else if (strncasecmp(buf, "Content-length:", 15) == 0) {
      m_iSize = atol(trimLead(buf + 15));
    }

    // what type of data do we have?
    else if (strncasecmp(buf, "Content-type:", 13) == 0) {
      char *start = trimLead(buf + 13);
      char *pos = start;

      // Increment until we encounter ";" or the end of the buffer
      while ( *pos && *pos != ';' )  pos++;

      // Assign the mime-type.
      m_strMimeType = QString::fromLatin1(start, pos-start).stripWhiteSpace().lower();
      kdDebug(7113) << "(" << m_pid << ") Content-type: " << m_strMimeType << endl;

      // If we still have text, then it means we have a mime-type with a
      // paramter (eg: charset=iso-8851) ; so let's get that...
      if (*pos)
      {
        start = ++pos;
        while ( *pos && *pos != '=' )  pos++;

        if (*pos)
        {
          mediaAttribute = QString::fromLatin1(start, pos-start).stripWhiteSpace().lower();
          mediaValue = QString::fromLatin1(++pos).stripWhiteSpace().lower();

          kdDebug (7113) << "(" << m_pid << ") Media-Parameter Attribute: "
                         << mediaAttribute << endl;
          kdDebug (7113) << "(" << m_pid << ") Media-Parameter Value: "
                         << mediaValue << endl;

          if ( mediaAttribute.find ("charset", 0, false) &&
               mediaAttribute.length () == 7)
            m_strCharset = mediaValue;

        }
      }
    }

    // Date
    else if (strncasecmp(buf, "Date:", 5) == 0) {
      dateHeader = KRFCDate::parseDate(trimLead(buf+5));
    }

    // Cache management
    else if (strncasecmp(buf, "ETag:", 5) == 0) {
      m_etag = trimLead(buf+5);
    }

    // Cache management
    else if (strncasecmp(buf, "Expires:", 8) == 0) {
      expireDate = KRFCDate::parseDate(trimLead(buf+8));
      if (!expireDate)
        expireDate = 1; // Already expired
    }

    // Cache management
    else if (strncasecmp(buf, "Last-Modified:", 14) == 0) {
      m_lastModified = (QString::fromLatin1(trimLead(buf+14))).stripWhiteSpace();
    }

    // whoops.. we received a warning
    else if (strncasecmp(buf, "Warning:", 8) == 0) {
      //Don't use warning() here, no need to bother the user.
      //Those warnings are mostly about caches.
      infoMessage(trimLead(buf + 8));
    }

    // Cache management (HTTP 1.0)
    else if (strncasecmp(buf, "Pragma:", 7) == 0) {
      QCString pragma = QCString(trimLead(buf+7)).stripWhiteSpace().lower();
      if (pragma == "no-cache")
      {
         m_bCachedWrite = false; // Don't put in cache
         mayCache = false;
      }
      hasCacheDirective = true;
    }

    // The deprecated Refresh Response
    else if (strncasecmp(buf,"Refresh:", 8) == 0) {
      mayCache = false;  // Do not cache page as it defeats purpose of Refresh tag!
      setMetaData( "http-refresh", QString::fromLatin1(trimLead(buf+8)).stripWhiteSpace() );
    }

    // In fact we should do redirection only if we got redirection code
    else if (strncasecmp(buf, "Location:", 9) == 0) {
      // Redirect only for 3xx status code, will ya! Thanks, pal!
      if ( m_responseCode > 299 && m_responseCode < 400 )
        locationStr = QCString(trimLead(buf+9)).stripWhiteSpace();
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

          while ( *dispositionBuf != ';' )
            dispositionBuf++;

          if ( dispositionBuf > bufStart )
          {
            // Skip any leading quotes...
            while ( *bufStart == '"' )
              bufStart++;

            // Skip any trailing quotes as well as white spaces...
            while ( *dispositionBuf == ' ' || *dispositionBuf == '"')
              dispositionBuf--;

            if ( dispositionBuf > bufStart )
              disposition = QString::fromLatin1( bufStart, dispositionBuf-bufStart );

            break;
          }
        }
        else
        {
          char *bufStart = dispositionBuf;

          while ( *dispositionBuf != ';' )
            dispositionBuf++;

          if ( dispositionBuf > bufStart )
            disposition = QString::fromLatin1( bufStart, dispositionBuf-bufStart ).stripWhiteSpace();

          while ( *dispositionBuf == ';' || *dispositionBuf == ' ' )
            dispositionBuf++;
        }
      }

      // Content-Dispostion is not allowed to dictate directory
      // path, thus we extract the filename only.
      if ( !disposition.isEmpty() )
      {
        int pos = disposition.findRev( '/' );
        if( pos > -1 )
          disposition = disposition.mid(pos+1);
        kdDebug(7113) << "(" << m_pid << ") (" << m_pid << ") Content-Disposition: "
                      << disposition << endl;
      }
    }
    else if (strncasecmp(buf, "Proxy-Connection:", 17) == 0) {
      if (strncasecmp(trimLead(buf + 17), "Close", 5) == 0) {
        m_bKeepAlive = false;
      } else if (strncasecmp(trimLead(buf + 17), "Keep-Alive", 10)==0) {
        m_bKeepAlive = true;
      }
    }
    // continue only if we know that we're HTTP/1.1
    else if (m_HTTPrev == HTTP_11) {
      // let them tell us if we should stay alive or not
      if (strncasecmp(buf, "Connection:", 11) == 0) {
        if (strncasecmp(trimLead(buf + 11), "Close", 5) == 0) {
          m_bKeepAlive = false;
        } else if (strncasecmp(trimLead(buf + 11), "Keep-Alive", 10)==0) {
          m_bKeepAlive = true;
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
        if (m_davVersions.isEmpty()) {
          m_davVersions = QString::fromLatin1(trimLead(buf + 4));
        }
        else {
          m_davVersions += ',';
          m_davVersions += QString::fromLatin1(trimLead(buf + 4));
        }
      }
      // *** Responses to the HTTP OPTIONS method finished
    }
    else if (buf[0] == '<')
    {
      // We get XML / HTTP without a proper header
      // put string back
      ungets(buf, strlen(buf));
      break;
    }

    // Clear out our buffer for further use.
    memset(buffer, 0, sizeof(buffer));
  } while (len && (gets(buffer, sizeof(buffer)-1)));

  // Send HTTP version
  if (m_HTTPrev == HTTP_11)
    setMetaData("HTTP-Version", "1.1");
  else if (m_HTTPrev == HTTP_10)
    setMetaData("HTTP-Version", "1.0");
  else
    setMetaData("HTTP-Version", "Unknown");

  // Send the reponse
  if (!responseHeader.isEmpty())
    setMetaData("HTTP-Headers", responseHeader.join("\n"));

  // If we do not support the requested authentication method...
  if ( (m_responseCode == 401 && Authentication == AUTH_None) ||
       (m_responseCode == 407 && ProxyAuthentication == AUTH_None) )
  {
    error( ERR_UNSUPPORTED_ACTION, "Unknown Authorization method!" );
    return false;
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
    expireDate = time(0) + DEFAULT_CACHE_EXPIRE;

  // DONE receiving the header!
  if (!cookieStr.isEmpty())
  {
    if ((m_cookieMode == CookiesAuto) && m_bUseCookiejar)
    {
      // Give cookies to the cookiejar.
      addCookies( m_request.url.url(), cookieStr );
    }
    else if (m_cookieMode == CookiesManual)
    {
      // Pass cookie to application
      setMetaData("setcookies", cookieStr);
    }
  }

  if (m_bMustRevalidate)
  {
    m_bMustRevalidate = false; // Reset just in case.
    if (cacheValidated)
    {
      // Yippie, we can use the cached version.
      // Update the cache with new "Expire" headers.
      fclose(m_fcache);
      m_fcache = 0;
      updateExpireDate( expireDate, true );
      m_fcache = checkCacheEntry( ); // Re-read cache entry
      
      if (m_fcache)
      {
          m_bCachedRead = true;
          return readHeader(); // Read header again, but now from cache.
       }
       else
       {
          // Where did our cache entry go???
       }
     }
     else
     {
       // Validation failed. Close cache.
       fclose(m_fcache);
       m_fcache = 0;
     }
  }

  // We need to reread the header if we got a '100 Continue' or '102 Processing'
  if ( cont )
  {
    return readHeader();
  }

  // We need to try to login again if we failed earlier
  if ( m_bUnauthorized )
  {
    if ( m_responseCode == 401 || m_responseCode == 407 )
    {
        if ( getAuthorization() )
        {
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
    KURL u(m_request.url, locationStr);
    if(u.isMalformed())
    {
      error(ERR_MALFORMED_URL, u.url());
      return false;
    }
    if ((u.protocol() != "http") && (u.protocol() != "https") &&
       (u.protocol() != "ftp") && (u.protocol() != "webdav") &&
       (u.protocol() != "webdavs"))
    {
      error(ERR_ACCESS_DENIED, u.url());
      return false;
    }

    if (!m_request.id.isEmpty())
    {
       sendMetaData();
    }

    kdDebug(7113) << "(" << m_pid << ") request.url: " << m_request.url.url()
                  << endl << "LocationStr: " << locationStr.data() << endl;

    kdDebug(7113) << "(" << m_pid << ") Requesting redirection to: " << u.url()
                  << endl;

    redirection(u.url());
    m_bCachedWrite = false; // Turn off caching on re-direction (DA)
    mayCache = false;
  }

  // Inform the job that we can indeed resume...
  if ( m_bCanResume && m_request.offset )
    canResume();

  // Reset the POST buffer if we do not get an authorization
  // request and the previous action was POST.
  if ( m_request.method==HTTP_POST && !m_bUnauthorized )
    m_bufPOST.resize(0);

  // Do not cache pages originating from password
  // protected sites.
  if ( !hasCacheDirective && Authentication != AUTH_None )
  {
    m_bCachedWrite = false;
    mayCache = false;
  }

  // Do not cache any text from SSL sites.
  if (m_bIsSSL && m_strMimeType.startsWith("text/"))
  {
    m_bCachedWrite = false;
    mayCache = false;
  }

  // WABA: Correct for tgz files with a gzip-encoding.
  // They really shouldn't put gzip in the Content-Encoding field!
  // Web-servers really shouldn't do this: They let Content-Size refer
  // to the size of the tgz file, not to the size of the tar file,
  // while the Content-Type refers to "tar" instead of "tgz".
  if (m_qContentEncodings.last() == "gzip")
  {
     if (m_strMimeType == "application/x-tar")
     {
        m_qContentEncodings.remove(m_qContentEncodings.fromLast());
        m_strMimeType = QString::fromLatin1("application/x-tgz");
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
        m_qContentEncodings.remove(m_qContentEncodings.fromLast());
        m_strMimeType = QString::fromLatin1("application/x-gzip");
     }
  }

  // We can't handle "bzip2" encoding (yet). So if we get something with
  // bzip2 encoding, we change the mimetype to "application/x-bzip2".
  // Note for future changes: some web-servers send both "bzip2" as
  //   encoding and "application/x-bzip2" as mimetype. That is wrong.
  //   currently that doesn't bother us, because we remove the encoding
  //   and set the mimetype to x-bzip2 anyway.
  if (m_qContentEncodings.last() == "bzip2")
  {
     m_qContentEncodings.remove(m_qContentEncodings.fromLast());
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

  // Prefer application/x-tgz over application/x-gzip
  else if (m_strMimeType == "application/x-gzip")
  {
     if ((m_request.url.path().right(7) == ".tar.gz") ||
         (m_request.url.path().right(4) == ".tar"))
        m_strMimeType = QString::fromLatin1("application/x-tgz");
  }

  // Some webservers say "text/plain" when they mean "application/x-bzip2"
  else if (m_strMimeType == "text/plain")
  {
     if (m_request.url.path().right(4) == ".bz2")
        m_strMimeType = QString::fromLatin1("application/x-bzip2");
     else if (m_request.url.path().right(4).upper() == ".PEM")
        m_strMimeType = QString::fromLatin1("application/x-x509-ca-cert");
  }
#if 0
  // Even if we can't rely on content-length, it seems that we should
  // never get more data than content-length. Maybe less, if the
  // content-length refers to the unzipped data.
  if (!m_qContentEncodings.isEmpty())
  {
     // If we still have content encoding we can't rely on the Content-Length.
     m_iSize = -1;
  }
#endif

  // Set charset. Maybe charSet should be a class member, since
  // this method is somewhat recursive....
  if ( !mediaAttribute.isEmpty() )
    setMetaData(mediaAttribute, mediaValue);

  if( !disposition.isEmpty() )
  {
    kdDebug(7113) << "(" << m_pid << ") Setting Content-Disposition metadata to: "
                  << disposition << endl;
    setMetaData("content-disposition", disposition);
  }

  if (!m_lastModified.isEmpty())
    setMetaData("modified", m_lastModified);

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
  }

  // Let the app know about the mime-type iff this is not
  // a redirection and the mime-type string is not empty.
  if (locationStr.isEmpty() && (!m_strMimeType.isEmpty() ||
      m_request.method == HTTP_HEAD))
  {
    kdDebug(7113) << "(" << m_pid << ") Emitting mimetype " << m_strMimeType << endl;
    mimeType( m_strMimeType );
  }

  if (m_request.method == HTTP_HEAD)
     return true;


  // Do we want to cache this request?
  if (m_bUseCache)
  {
     if ( m_bCachedWrite && !m_strMimeType.isEmpty() )
     {
        // Check...
        createCacheEntry(m_strMimeType, expireDate); // Create a cache entry
        if (!m_fcache)
           m_bCachedWrite = false; // Error creating cache entry.
        m_expireDate = expireDate;
     }
     else
     {
        ::unlink( QFile::encodeName(m_state.cef));
     }
  }

  if (m_bCachedWrite && !m_strMimeType.isEmpty())
    kdDebug(7113) << "(" << m_pid << ") Cache, adding \"" << m_request.url.url() << "\"" << endl;
  else if (m_bCachedWrite && m_strMimeType.isEmpty())
    kdDebug(7113) << "(" << m_pid << ") Cache, pending \"" << m_request.url.url() << "\"" << endl;
  else
    kdDebug(7113) << "(" << m_pid << ") Cache, not adding \"" << m_request.url.url() << "\"" << endl;
  return true;
}


void HTTPProtocol::addEncoding(QString encoding, QStringList &encs)
{
  encoding = encoding.stripWhiteSpace().lower();
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
      m_iSize = -1;
  } else if ((encoding == "x-gzip") || (encoding == "gzip")) {
    encs.append(QString::fromLatin1("gzip"));
  } else if ((encoding == "x-bzip2") || (encoding == "bzip2")) {
    encs.append(QString::fromLatin1("bzip2")); // Not yet supported!
  } else if ((encoding == "x-deflate") || (encoding == "deflate")) {
    encs.append(QString::fromLatin1("deflate"));
  } else {
    kdWarning(7103) << "Unknown encoding encountered.  Please write code. "
                       "pid = (" << m_pid << ") Encoding = \"" << encoding
                    << "\"" << endl;
  }
}

bool HTTPProtocol::sendBody( bool dataInternal /* = false */ )
{
  int result=-1;
  int length=0;

  infoMessage( i18n( "Requesting data to send" ) );

  // Loop until we got 'dataEnd'
  kdDebug(7113) << "(" << m_pid << ") Response code: " << m_responseCode << endl;
  if ( m_responseCode == 401 || m_responseCode == 407 || dataInternal )
  {
    // For RE-POST on authentication failure the
    // buffer should not be empty...
    if ( m_bufPOST.isNull() )
    {
      error( ERR_ABORTED, m_request.hostname );
      return false;
    }
    kdDebug(7113) << "(" << m_pid << ") POST'ing saved data..." << endl;
    length = m_bufPOST.size();
    result = 0;
  }
  else
  {
    kdDebug(7113) << "(" << m_pid << ") POST'ing live data..." << endl;
    m_bufPOST.resize(0);
    QByteArray buffer;
    int old_size;
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

  if ( result != 0 )
  {
    error( ERR_ABORTED, m_request.hostname );
    return false;
  }

  char c_buffer[64];
  sprintf(c_buffer, "Content-Length: %d\r\n\r\n", length);
  kdDebug( 7113 ) << "(" << m_pid << ")" << c_buffer << endl;

  infoMessage( i18n( "Sending data to <b>%1</b>" ).arg( m_request.hostname ) );

  // Send the content length...
  bool sendOk = (write(c_buffer, strlen(c_buffer)) == (ssize_t) strlen(c_buffer));
  if (!sendOk)
  {
    kdDebug(7113) << "(" << m_pid << ") Connection broken when sending content length: ("
                  << m_state.hostname << ")" << endl;
    error( ERR_CONNECTION_BROKEN, m_state.hostname );
    return false;
  }

  // Send the data...
  //kdDebug() << "POST DATA: " << QCString(m_bufPOST) << endl;
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

void HTTPProtocol::httpClose()
{
  if (m_fcache)
  {
     fclose(m_fcache);
     m_fcache = 0;
     if (m_bCachedWrite)
     {
        QString filename = m_state.cef + ".new";
        ::unlink( QFile::encodeName(filename) );
     }
  }
  if (!m_bKeepAlive)
     httpCloseConnection();
  else
     kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::httpClose: keep alive" << endl;
}

void HTTPProtocol::closeConnection()
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::closeConnection" << endl;
  httpCloseConnection();
}

void HTTPProtocol::httpCloseConnection()
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::httpCloseConnection" << endl;
  m_bKeepAlive = false;
  m_bIsTunneled = false;
  closeDescriptor();
}

void HTTPProtocol::slave_status()
{
  kdDebug(7113) << "(" << m_pid << ") HTTPProtocol::slave_status" << endl;

  bool connected = isConnectionValid();
  if ( !connected && m_iSock != -1 )
     httpCloseConnection();
  slaveStatus( m_state.hostname, connected );
}

void HTTPProtocol::mimetype( const KURL& url )
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
  QDataStream stream(data, IO_ReadOnly);

  stream >> tmp;
  switch (tmp) {
    case 1: // HTTP POST
    {
      KURL url;
      stream >> url;
      post( url );
      break;
    }
    case 2: // cache_update
    {
      KURL url;
      bool no_cache;
      time_t expireDate;
      stream >> url >> no_cache >> expireDate;
      cacheUpdate( url, no_cache, expireDate );
      break;
    }
    case 5: // WebDAV lock
    {
      KURL url;
      QString scope, type, owner;
      stream >> url >> scope >> type >> owner;
      davLock( url, scope, type, owner );
      break;
    }
    case 6: // WebDAV unlock
    {
      KURL url;
      stream >> url;
      davUnlock( url );
      break;
    }
    default:
      // Some command we don't understand.
      // Just ignore it, it may come from some future version of KDE.
      break;
  }
}

void HTTPProtocol::decodeDeflate()
{
#ifdef DO_GZIP
  // Okay the code below can probably be replaced with
  // a single call to decompress(...) instead of a read/write
  // to and from a temporary file, but I was not sure of how to
  // estimate the size of the decompressed data which needs to be
  // passed as a parameter to decompress function call.  Anyone
  // want to try this approach or have a better experience with
  // the cool zlib library ??? (DA)
  //
  // TODO: Neither deflate nor gzip completely check for errors!!!
  z_stream z;
  QByteArray tmp_buf;
  const unsigned int max_len = 1024;
  unsigned char in_buf[max_len];  // next_in
  unsigned char out_buf[max_len]; // next_out
  int status = Z_OK; //status of the deflation
  char* filename=strdup("/tmp/kio_http.XXXXXX");

  z.avail_in = 0;
  z.avail_out = max_len;
  z.next_out = out_buf;

  // Create the file
  int fd = mkstemp(filename);

  // TODO: Need sanity check here. Doing no error checking when writing
  // to HD cannot be good :))  What if the target is bloody full or it
  // for some reason could not be written to ?? (DA)
  ::write(fd, m_bufEncodedData.data(), m_bufEncodedData.size()); // Write data into file
  lseek(fd, 0, SEEK_SET);
  FILE* fin = fdopen( fd, "rb" );

  // Read back and decompress data.
  for( ; ; )
  {
    if( z.avail_in == 0 )
    {
        z.next_in = in_buf;
        z.avail_in = ::fread(in_buf, 1, max_len, fin );
    }
    if( z.avail_in == 0 )
        break;
    status = inflate( &z, Z_NO_FLUSH );
    if( status !=  Z_OK )
        break;
    unsigned int count = max_len - z.avail_out;
    if( count )
    {
        unsigned int old_len = tmp_buf.size();
        memcpy( tmp_buf.data() + old_len, out_buf, count );
        z.next_out = out_buf;
        z.avail_out = max_len;
    }
  }

  for( ; ; )
  {
    status = inflate( &z, Z_FINISH );
    unsigned int count = max_len - z.avail_out;
    if ( count )
    {
        unsigned int old_len = tmp_buf.size();
        // Copy the data into a temporary buffer
        memcpy( tmp_buf.data() + old_len, out_buf, count );
        z.next_out = out_buf;
        z.avail_out = max_len;
    }
    if( status !=  Z_OK )
        break;
  }
  if( fin )
    ::fclose( fin );
  ::unlink(filename); // Bye bye to beloved temp file.  We will miss you!!

  // Replace m_bufEncodedData with the
  // "decoded" data.
  m_bufEncodedData.resize(0);
  m_bufEncodedData = tmp_buf;
  m_bufEncodedData.detach();
#endif
}

void HTTPProtocol::decodeGzip()
{
#ifdef DO_GZIP
  // Note I haven't found an implementation
  // that correctly handles this stuff.  Apache
  // sends both Transfer-Encoding and Content-Length
  // headers, which is a no-no because the content
  // could really be bigger than the content-length
  // header implies.. like with gzip.
  // eek. This is no fun for progress indicators.
  QByteArray ar;

  char tmp_buf[1024], *filename=strdup("/tmp/kio_http.XXXXXX");
  signed long len;
  int fd;

  kdDebug(7103) << "decode gzip" << endl;

  // Siince I can't figure out how to do the mem to mem
  // gunzipping, this should suffice.  It just writes out
  // the gzip'd data to a file.
  fd=mkstemp(filename);
  ::write(fd, m_bufEncodedData.data(), m_bufEncodedData.size());
  lseek(fd, 0, SEEK_SET);
  gzFile gzf = gzdopen(fd, "rb");
  unlink(filename); // If you want to inspect the raw data, comment this line out
  free(filename);
  filename = 0;

  // And then reads it back in with gzread so it'll
  // decompress on the fly.
  while ( (len=gzread(gzf, tmp_buf, 1024))>0){
    if (len < 0) break; // -1 is error
    int old_len=ar.size();
    ar.resize(ar.size()+len);
    memcpy(ar.data()+old_len, tmp_buf, len);
  }
  gzclose(gzf);

  // And then we replace m_bufEncodedData with
  // the "decoded" data.
  m_bufEncodedData.resize(0);
  m_bufEncodedData=ar;
  m_bufEncodedData.detach();
#endif
}

/**
 * Read a chunk from the data stream.
 */
int HTTPProtocol::readChunked()
{
  if (m_iBytesLeft <= 0)
  {
     m_bufReceive.resize(4096);

     if (!gets(m_bufReceive.data(), m_bufReceive.size()-1))
     {
       kdDebug(7113) << "(" << m_pid << ") gets() failure on Chunk header" << endl;
       return -1;
     }
     // We could have got the CRLF of the previous chunk.
     // If so, try again.
     if (m_bufReceive[0] == '\0')
     {
        if (!gets(m_bufReceive.data(), m_bufReceive.size()-1))
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

     m_iBytesLeft = strtol(m_bufReceive.data(), 0, 16);
     if (m_iBytesLeft < 0)
     {
        kdDebug(7113) << "(" << m_pid << ") Negative chunk size" << endl;
        return -1;
     }

     // kdDebug(7113) << "(" << m_pid << ") Chunk size = " << m_iBytesLeft << " bytes" << endl;

     if (m_iBytesLeft == 0)
     {
       // Last chunk.
       // Skip trailers.
       do {
         // Skip trailer of last chunk.
         if (!gets(m_bufReceive.data(), m_bufReceive.size()-1))
         {
           kdDebug(7113) << "(" << m_pid << ") gets() failure on Chunk trailer" << endl;
           return -1;
         }
         kdDebug(7113) << "(" << m_pid << ") Chunk trailer = \"" << m_bufReceive.data() << "\"" << endl;
       }
       while (strlen(m_bufReceive.data()) != 0);

       return 0;
     }
  }

  int bytesReceived = readLimited();
  if (!m_iBytesLeft)
     m_iBytesLeft = -1; // Don't stop, continue with next chunk
  return bytesReceived;
}

int HTTPProtocol::readLimited()
{
  if (!m_iBytesLeft) return 0;
  m_bufReceive.resize(4096);

  int bytesReceived;
  int bytesToReceive;

  if (m_iBytesLeft > (int) m_bufReceive.size())
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
     kdWarning(7113) << "(" << m_pid << ") Unbounded datastream on a Keep "
                                        "alive connection!" << endl;
     m_bKeepAlive = false;
  }
  m_bufReceive.resize(4096);

  return read(m_bufReceive.data(), m_bufReceive.size());
}

/**
 * This function is our "receive" function.  It is responsible for
 * downloading the message (not the header) from the HTTP server.  It
 * is called either as a response to a client's KIOJob::dataEnd()
 * (meaning that the client is done sending data) or by 'httpOpen()'
 * (if we are in the process of a PUT/POST request). It can also be
 * called by a webDAV function, to recieve stat/list/property/etc.
 * data; in this case the data is stored in m_intData.
 */
bool HTTPProtocol::readBody( bool dataInternal /* = false */ )
{
  // Note that when dataInternal is true, we are going to:
  // 1) save the body data to a member variable, m_intData
  // 2) _not_ advertise the data, speed, size, etc., through the
  //    corresponding functions.
  // This is used for returning data to WebDAV.
  if ( dataInternal )
    m_intData = QString::null;

  // Check if we need to decode the data.
  // If we are in copy mode, then use only transfer decoding.
  bool decode = ( !m_qTransferEncodings.isEmpty() ||
                  !m_qContentEncodings.isEmpty() );
  bool useMD5 = !m_sContentMD5.isEmpty();

  // Deal with the size of the file.
  KIO::filesize_t sz = m_request.offset;
  if ( sz )
    m_iSize += sz;

  // Update the application with total size except when
  // it is compressed, or when the data is to be handled
  // internally (webDAV).  If compressed we have to wait
  // until we uncompress to find out the actual data size
  if ( !decode && !dataInternal )
    totalSize( (m_iSize > -1) ? m_iSize : 0 );

  infoMessage( i18n( "Retrieving data from "
                     "<b>%1</b>" ).arg( m_request.hostname ) );

  if (m_bCachedRead)
  {
    kdDebug(7113) << "HTTPProtocol::readBody: read data from cache!" << endl;

    char buffer[ MAX_IPC_SIZE ];

    // Jippie! It's already in the cache :-)
    while (!feof(m_fcache) && !ferror(m_fcache))
    {
      int nbytes = fread( buffer, 1, MAX_IPC_SIZE, m_fcache);

      if (nbytes > 0)
      {
        m_bufReceive.setRawData( buffer, nbytes);
        if ( !dataInternal )
          data( m_bufReceive );
        else
          m_intData += m_bufReceive;

        m_bufReceive.resetRawData( buffer, nbytes );
        sz += nbytes;
      }
    }

    if ( !dataInternal )
      processedSize( sz );

    m_bufReceive.resize( 0 );

    if ( !dataInternal )
      data( QByteArray() );

    return true;
  }

  if (m_iSize > -1)
    m_iBytesLeft = m_iSize - sz;
  else
    m_iBytesLeft = -1;

  if (m_bChunked)
    m_iBytesLeft = -1;

  // Main incoming loop...  Gather everything while we can...
  KMD5 context;
  bool cpMimeBuffer = false;
  QByteArray mimeTypeBuffer;
  m_bufEncodedData.resize(0);
  struct timeval last_tv;
  gettimeofday( &last_tv, 0L );

  while (!m_bEOF)
  {
    int bytesReceived;
    if (m_bChunked)
       bytesReceived = readChunked();
    else if (m_iSize > -1)
       bytesReceived = readLimited();
    else
       bytesReceived = readUnlimited();

    // make sure that this wasn't an error, first
    if (bytesReceived == -1)
    {
      // erg.  oh well, log an error and bug out
      kdDebug(7113) << "(" << m_pid << ") readBody: bytesReceived==-1. Connnection broken ! " << endl;
      error(ERR_CONNECTION_BROKEN, m_state.hostname);
      return false;
    }

    // i guess that nbytes == 0 isn't an error.. but we certainly
    // won't work with it!
    if (bytesReceived > 0)
    {
      // internal functions don't worry abount mimetypes
      if ( !dataInternal )
      {
        // If a broken server does not send the mime-type,
        // we try to id it from the content before dealing
        // with the content itself.
        if ( m_strMimeType.isEmpty() && !( m_responseCode >= 300 &&
                                          m_responseCode <=399) )
        {
          kdDebug(7113) << "(" << m_pid << ") Determining mime-type from content..." << endl;
          int old_size = mimeTypeBuffer.size();
          mimeTypeBuffer.resize( old_size + bytesReceived );
          memcpy( mimeTypeBuffer.data() + old_size, m_bufReceive.data(),
                  bytesReceived );
          if ( m_iBytesLeft > 0 && mimeTypeBuffer.size() < 1024 )
          {
            cpMimeBuffer = true;
            continue;   // Do not send up the data since we do not yet know its mimetype!
          }

          kdDebug(7113) << "(" << m_pid << ") Mimetype buffer size: " << mimeTypeBuffer.size()
                        << endl;

          KMimeMagicResult *result;

          result = KMimeMagic::self()->findBufferFileType( mimeTypeBuffer,
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

          if ( m_bCachedWrite )
          {
            createCacheEntry( m_strMimeType, m_expireDate );
            if (!m_fcache)
              m_bCachedWrite = false;
          }

          if ( cpMimeBuffer )
          {
            bytesReceived = mimeTypeBuffer.size();
            m_bufReceive.resize(0);
            m_bufReceive.resize(bytesReceived);
            memcpy( m_bufReceive.data(), mimeTypeBuffer.data(),
                    bytesReceived );
          }
          mimeType(m_strMimeType);
          mimeTypeBuffer.resize(0);
        }
      }

      // check on the encoding.  can we get away with it as is?
      if ( !decode )
      {
        // Important: truncate the buffer to the actual size received!
        // Otherwise garbage will be passed to the app
        m_bufReceive.truncate( bytesReceived );

        // Update the message digest engine...
        if (useMD5)
          context.update( m_bufReceive );
        // Let the world know that we have some data, unless it is
        // requested by webDAV
        if ( !dataInternal )
          data( m_bufReceive );
        else
          m_intData += m_bufReceive;

        if (m_bCachedWrite && m_fcache)
           writeCacheEntry(m_bufReceive.data(), bytesReceived);

        sz += bytesReceived;
        if (!dataInternal)
          processedSize( sz );
      }
      else
      {
        // nope.  slap this all onto the end of a big buffer for later use
        unsigned int old_len = 0;
        old_len = m_bufEncodedData.size();
        m_bufEncodedData.resize(old_len + bytesReceived);
        memcpy( m_bufEncodedData.data() + old_len, m_bufReceive.data(),
                bytesReceived );
      }
    }
    m_bufReceive.resize(0); // res
    if (m_iBytesLeft == 0)
      break;
  }

  // if we have something in m_bufEncodedData, then we know that we have
  // encoded data.  of course, we need to do something about this
  if (!m_bufEncodedData.isNull())
  {
    // decode all of the transfer encodings
    while (!m_qTransferEncodings.isEmpty())
    {
      QString enc = m_qTransferEncodings.last();
      m_qTransferEncodings.remove(m_qTransferEncodings.fromLast());
      if ( enc == "gzip" )
        decodeGzip();
      else if ( enc == "deflate" )
        decodeDeflate();
    }

    // From HTTP 1.1 Draft 6:
    // The MD5 digest is computed based on the content of the entity-body,
    // including any content-coding that has been applied, but not including
    // any transfer-encoding applied to the message-body. If the message is
    // received with a transfer-encoding, that encoding MUST be removed
    // prior to checking the Content-MD5 value against the received entity.
    if ( useMD5 )
      context.update( m_bufEncodedData );

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
      QString enc = m_qContentEncodings.last();
      m_qContentEncodings.remove(m_qContentEncodings.fromLast());
      if ( enc == "gzip" )
        decodeGzip();
      else if ( enc == "deflate" )
        decodeDeflate();
    }

    uint bytesReceived = m_bufEncodedData.size();
    uint bytesToSend = MAX_IPC_SIZE;
    if ( !dataInternal )
      totalSize( bytesReceived );

    if ( bytesReceived > bytesToSend )
    {
      sz = 0;
      QByteArray array;

      while ( 1 )
      {
        array.setRawData( m_bufEncodedData.data()+sz, bytesToSend);

        if ( !dataInternal )
          data( array );
        else
          m_intData += array;

        array.resetRawData( m_bufEncodedData.data()+sz, bytesToSend);
        sz += bytesToSend;
        
        if ( !dataInternal )
          processedSize( sz );
        
        if ( sz >= bytesReceived )
          break;
        
        if ( bytesReceived-sz < bytesToSend )
          bytesToSend = bytesReceived-sz;
      }
    }
    else
    {
      sz = bytesReceived;
      if ( !dataInternal ) 
      {
        data( m_bufEncodedData );
        processedSize( bytesReceived );
      } 
      else
      {
        m_intData += m_bufEncodedData;
      }
    }

    if ( m_bCachedWrite &&  m_fcache )
    {
      writeCacheEntry(m_bufEncodedData.data(), m_bufEncodedData.size());
      closeCacheEntry();
    }
  }

  if ( useMD5 )
  {
    QString calculatedMD5 = QString::fromLatin1(context.base64Digest());

    if ( m_sContentMD5 == calculatedMD5 )
      kdDebug(7113) << "(" << m_pid << ") MD5 checksum MATCHED!!" << endl;
    else
      kdDebug(7113) << "(" << m_pid << ") MD5 checksum MISMATCH! Expected: "
                    << calculatedMD5 << ", Got: " << m_sContentMD5 << endl;
  }

  // Close cache entry
  if (m_iBytesLeft == 0)
  {
     if (m_bCachedWrite && m_fcache)
        closeCacheEntry();
  }

  if (!dataInternal)
    data( QByteArray() );
  return true;
}


void HTTPProtocol::error( int _err, const QString &_text )
{
  m_bKeepAlive = false;
  httpClose();
  
  if (!m_request.id.isEmpty())
    sendMetaData();

  SlaveBase::error( _err, _text );
  m_bError = true;
}


void HTTPProtocol::addCookies( const QString &url, const QCString &cookieHeader )
{
   kdDebug(7113) << "(" << m_pid << ") (" << m_pid << ") " << cookieHeader << endl;

   long windowId = m_request.window.toLong();
   QByteArray params;
   QDataStream stream(params, IO_WriteOnly);
   stream << url << cookieHeader << windowId;

  bool attemptedRestart = false;
   while ( 1 )
   {
      if ( !m_dcopClient->send( "kcookiejar", "kcookiejar", "addCookies(QString,QCString,long int)", params ) )
      {
        if( !initCookieJar() || attemptedRestart )
        {
          kdWarning(7113) << "(" << m_pid << ") Can't communicate with cookiejar!" << endl;
          break;
        }
        else
        {
          attemptedRestart = true;
        }
      }
      else
      {
        break;
      }
   }
}

QString HTTPProtocol::findCookies( const QString &url)
{
  QCString replyType;
  QByteArray params;
  QByteArray reply;
  QString result;

  result = QString::null;
  QDataStream stream(params, IO_WriteOnly);
  stream << url;

  bool attemptedRestart = false;
  while ( 1 )
  {
    if ( !m_dcopClient->call( "kcookiejar", "kcookiejar", "findCookies(QString)",
                                             params, replyType, reply ) )
    {
      if ( !initCookieJar() || attemptedRestart )
      {
        kdWarning(7113) << "(" << m_pid << ") Can't communicate with cookiejar!" << endl;
        break;
      }
      else
      {
        attemptedRestart = true;
      }
    }
    else
    {
      if ( replyType == "QString" )
      {
        QDataStream stream2( reply, IO_ReadOnly );
        stream2 >> result;
      }
      else
      {
        kdError(7113) << "(" << m_pid << ") DCOP function findCookies(...) returns "
                                      << replyType << ", expected QString" << endl;
      }
      break;
    }
  }
  return result;
}

bool HTTPProtocol::initCookieJar() const
{
  if ( m_bUseCookiejar && !m_dcopClient->isApplicationRegistered( "kcookiejar" ) )
  {
     QString error;
     if ( KApplication::startServiceByDesktopName( "kcookiejar", QStringList(),
                                                  &error ) )
     {
        kdDebug(1202) << "Error starting KCookiejar: " << error << endl;
        return false;
     }
  }
  return true;
}


/******************************* CACHING CODE ****************************/


void HTTPProtocol::cacheUpdate( const KURL& url, bool no_cache, time_t expireDate)
{
  if ( !checkRequestURL( url ) )
    return;

  m_request.path = url.path();
  m_request.query = url.query();
  m_request.cache = CC_Reload;
  m_request.doProxy = m_bUseProxy;

  if (no_cache)
  {
     m_fcache = checkCacheEntry( );
     if (m_fcache)
     {
       fclose(m_fcache);
       m_fcache = 0;
       ::unlink( QFile::encodeName(m_state.cef) );
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
   const QChar seperator = '_';

   QString CEF = m_request.path;

   int p = CEF.find('/');

   while(p != -1)
   {
      CEF[p] = seperator;
      p = CEF.find('/', p);
   }

   QString host = m_request.hostname.lower();
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
   QCString u = m_request.url.url().latin1();
   for(int i = u.length(); i--;)
   {
      hash = (hash * 12211 + u[i]) % 2147483563;
   }

   QString hashString;
   hashString.sprintf("%08lx", hash);

   CEF = CEF + hashString;

   CEF = dir + "/" + CEF;

   m_state.cef = CEF;

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
      if (m_maxCacheAge && (difftime(currentDate, date) > m_maxCacheAge))
      {
         m_bMustRevalidate = true;
         m_expireDate = currentDate;
      }
   }

   // Expiration Date
   m_cacheExpireDateOffset = ftell(fs);
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      if (m_request.cache == CC_Verify)
      {
         date = (time_t) strtoul(buffer, 0, 10);
         // After the expire date we need to revalidate.
         if (!date || difftime(currentDate, date) >= 0)
            m_bMustRevalidate = true;
         m_expireDate = date;
      }
      else if (m_request.cache == CC_Refresh)
      {
         m_bMustRevalidate = true;
         m_expireDate = currentDate;
      }
   }

   // ETag
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      m_etag = QString(buffer).stripWhiteSpace();
   }

   // Last-Modified
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      m_lastModified = QString(buffer).stripWhiteSpace();
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
           date = date.leftJustify(16);
           fputs(date.latin1(), fs);      // Creation date
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
        date = date.leftJustify(16);
        if (!ok || fseek(fs, m_cacheExpireDateOffset, SEEK_SET))
            return;
        fputs(date.latin1(), fs);      // Expire date
        fseek(fs, 0, SEEK_END);
        fclose(fs);
    }
}

void HTTPProtocol::createCacheEntry( const QString &mimetype, time_t expireDate)
{
   QString dir = m_state.cef;
   int p = dir.findRev('/');
   if (p == -1) return; // Error.
   dir.truncate(p);

   // Create file
   (void) ::mkdir( QFile::encodeName(dir), 0700 );

   QString filename = m_state.cef + ".new";  // Create a new cache entryexpireDate

//   kdDebug( 7103 ) <<  "creating new cache entry: " << filename << endl;

   m_fcache = fopen( QFile::encodeName(filename), "w");
   if (!m_fcache)
   {
      kdWarning(7103) << "createCacheEntry: opening " << filename << " failed." << endl;
      return; // Error.
   }

   fputs(CACHE_REVISION, m_fcache);    // Revision

   fputs(m_request.url.url().latin1(), m_fcache);  // Url
   fputc('\n', m_fcache);

   QString date;
   date.setNum( time(0) );
   date = date.leftJustify(16);
   fputs(date.latin1(), m_fcache);      // Creation date
   fputc('\n', m_fcache);

   date.setNum( expireDate );
   date = date.leftJustify(16);
   fputs(date.latin1(), m_fcache);      // Expire date
   fputc('\n', m_fcache);

   if (!m_etag.isEmpty())
      fputs(m_etag.latin1(), m_fcache);    //ETag
   fputc('\n', m_fcache);

   if (!m_lastModified.isEmpty())
      fputs(m_lastModified.latin1(), m_fcache);    // Last modified
   fputc('\n', m_fcache);

   fputs(mimetype.latin1(), m_fcache);  // Mimetype
   fputc('\n', m_fcache);

   if (!m_strCharset.isEmpty())
      fputs(m_strCharset.latin1(), m_fcache);    // Charset
   fputc('\n', m_fcache);

   return;
}
// The above code should be kept in sync
// with the code in http_cache_cleaner.cpp
// !END SYNC!

void HTTPProtocol::writeCacheEntry( const char *buffer, int nbytes)
{
   if (fwrite( buffer, nbytes, 1, m_fcache) != 1)
   {
      kdWarning(7103) << "writeCacheEntry: writing " << nbytes << " bytes failed." << endl;
      fclose(m_fcache);
      m_fcache = 0;
      QString filename = m_state.cef + ".new";
      ::unlink( QFile::encodeName(filename) );
      return;
   }
}

void HTTPProtocol::closeCacheEntry()
{
   QString filename = m_state.cef + ".new";
   int result = fclose( m_fcache);
   m_fcache = 0;
   if (result == 0)
   {
      if (::rename( QFile::encodeName(filename), QFile::encodeName(m_state.cef)) == 0)
         return; // Success

      kdWarning(7103) << "closeCacheEntry: error renaming cache entry. ("
                   << filename << " -> " << m_state.cef << ")" << endl;
   }
   kdWarning(7103) << "closeCacheEntry: error closing cache entry. ("
                   << filename<< ")" << endl;
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
      KApplication::startServiceByDesktopPath("http_cache_cleaner.desktop");
   }
}



//**************************  AUTHENTICATION CODE ********************/


void HTTPProtocol::configAuth( const char *p, bool b )
{
  HTTP_AUTH f = AUTH_None;
  const char *strAuth = p;

  while( *p == ' ' ) p++;

  if ( strncasecmp( p, "Basic", 5 ) == 0 )
  {
    f = AUTH_Basic;
    p += 5;
  }
  else if (strncasecmp (p, "Digest", 6) ==0 )
  {
    f = AUTH_Digest;
    p += 6;
    strAuth = p;
  }
  else if (strncasecmp( p, "MBS_PWD_COOKIE", 14 ) == 0)
  {
    // Found on http://www.webscription.net/baen/default.asp
    f = AUTH_Basic;
    p += 14;
  }
  else
  {
    kdWarning(7103) << "Unsupported or invalid authorization type requested" << endl;
    kdWarning(7103) << "Request Authorization: " << p << endl;
  }

  /*
     This check ensures the following:
     1.) Rejection of any unknown/unsupported authentication schemes
     2.) Usage of the strongest possible authentication schemes if
         and when multiple Proxy-Authenticate or WWW-Authenticate
         header field is sent.
  */
  if ( f == AUTH_None ||
       (b && m_iProxyAuthCount > 0 && f < ProxyAuthentication) ||
       (!b && m_iWWWAuthCount > 0 && f < Authentication) )
  {
    // Since I purposefully made the Proxy-Authentication settings
    // persistent to reduce the number of round-trips to kdesud we
    // have to take special care when an unknown/unsupported auth-
    // scheme is received. This check accomplishes just that...
    if ( b )
    {
      if ( !m_iProxyAuthCount )
        ProxyAuthentication = f;
      m_iProxyAuthCount++;
    }
    else
      m_iWWWAuthCount++;
    return;
  }

  while (*p)
  {
    int i = 0;
    while( (*p == ' ') || (*p == ',') || (*p == '\t') ) { p++; }
    if ( strncasecmp( p, "realm=\"", 7 ) == 0 )
    {
      p += 7;
      while( p[i] != '"' ) i++;
      if( b )
        m_strProxyRealm = QString::fromLatin1( p, i );
      else
        m_strRealm = QString::fromLatin1( p, i );
    }
    p+=(i+1);
  }

  if( b )
  {
    ProxyAuthentication = f;
    m_strProxyAuthorization = QString::fromLatin1( strAuth, strlen(strAuth) );
  }
  else
  {
    Authentication = f;
    m_strAuthorization = QString::fromLatin1( strAuth, strlen(strAuth) );
  }
}


bool HTTPProtocol::retryPrompt()
{
  QString prompt;
  switch ( m_responseCode )
  {
    case 401:
      prompt = i18n("Authentication Failed!");
      break;
    case 407:
      prompt = i18n("Proxy Authentication Failed!");
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
    //info.keepPassword = true; // Prompt the user for persistence as well.
    if ( !m_strRealm.isEmpty() )
    {
      info.realmValue = m_strRealm;
      info.verifyPath = false;
      if ( Authentication == AUTH_Digest )
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
      if ( ProxyAuthentication == AUTH_Digest )
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
  bool repeatFailure = (m_prevResponseCode == m_responseCode);

  if ( repeatFailure )
  {
    bool prompt = true;
    if ( Authentication == AUTH_Digest )
    {
      bool isStaleNonce = false;
      QString auth = ( m_responseCode == 401 ) ? m_strAuthorization : m_strProxyAuthorization;
      int pos = auth.find("stale", 0, false);
      if ( pos != -1 )
      {
        pos += 5;
        int len = auth.length();
        while( pos < len && (auth[pos] == ' ' || auth[pos] == '=') ) pos++;
        if ( pos < len && auth.find("true", pos, false) != -1 )
        {
          isStaleNonce = true;
          kdDebug(7113) << "(" << m_pid << ") Stale nonce value. Will retry using same info..." << endl;
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
    if ( prompt && !retryPrompt() )
    {
      if (m_bErrorPage)
         errorPage();
      else
         error(ERR_USER_CANCELED, QString::null);
      return false;
    }
  }
  else
  {
    // At this point we know more detials, so use it to find
    // out if we have a cached version and avoid a re-prompt!
    // We also do not use verify path unlike the pre-emptive
    // requests because we already know the realm value...
    info.verifyPath = false;
    if ( m_responseCode == 407 )
    {
      info.url = m_proxyURL;
      info.username = m_proxyURL.user();
      info.password = m_proxyURL.pass();
      info.realmValue = m_strProxyRealm;
    }
    else
    {
      info.url = m_request.url;
      info.username = m_request.user;
      info.password = m_request.passwd;
      info.realmValue = m_strRealm;
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

      int pos = auth.find("stale", 0, false);
      if ( pos != -1 )
      {
        pos += 5;
        int len = auth.length();
        while( pos < len && (auth[pos] == ' ' || auth[pos] == '=') ) pos++;
        if ( pos < len && auth.find("true", pos, false) != -1 )
        {
          info.digestInfo = (m_responseCode == 401) ? m_strAuthorization : m_strProxyAuthorization;
          kdDebug(7113) << "(" << m_pid << ") Just a stale nonce value! Retrying with the new nonce sent!" << endl;
        }
      }
    }
  }

  if ( !result  )
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
      if ( m_request.disablePassDlg == false )
      {
        kdDebug( 7113 ) << "About to prompt user for authorization..." << endl;
        promptInfo( info );
        result = openPassDlg( info );
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
        if ( Authentication == AUTH_Digest )
          m_strAuthorization = info.digestInfo;
        break;
      case 407: // Proxy-Authentication
        m_proxyURL.setUser( info.username );
        m_proxyURL.setPass( info.password );
        m_strProxyRealm = info.realmValue;
        if ( Authentication == AUTH_Digest )
          m_strProxyAuthorization = info.digestInfo;
        break;
      default:
        break;
    }
    return true;
  }

  if (m_bErrorPage)
     errorPage();
  else
     error( ERR_USER_CANCELED, QString::null );
  return false;
}

void HTTPProtocol::saveAuthorization()
{
  AuthInfo info;
  if ( m_prevResponseCode == 407 )
  {
    info.url = m_proxyURL;
    info.username = m_proxyURL.user();
    info.password = m_proxyURL.pass();
    info.realmValue = m_strProxyRealm;
    if( Authentication == AUTH_Digest )
      info.digestInfo = m_strProxyAuthorization;
    cacheAuthentication( info );
  }
  else
  {
    info.url = m_request.url;
    info.username = m_request.user;
    info.password = m_request.passwd;
    info.realmValue = m_strRealm;
    if( Authentication == AUTH_Digest )
      info.digestInfo = m_strAuthorization;
    cacheAuthentication( info );
  }
}

QString HTTPProtocol::createBasicAuth( bool isForProxy )
{
  QString auth;
  QCString user, passwd;
  if ( isForProxy )
  {
    auth = "Proxy-Authorization: Basic ";
    user = m_proxyURL.user().latin1();
    passwd = m_proxyURL.pass().latin1();
  }
  else
  {
    auth = "Authorization: Basic ";
    user = m_state.user.latin1();
    passwd = m_state.passwd.latin1();
  }

  if ( user.isEmpty() )
    user = "";
  if ( passwd.isEmpty() )
    passwd = "";

  user += ':';
  user += passwd;
  auth += KCodecs::base64Encode( user );
  return auth;
}

void HTTPProtocol::calculateResponse( DigestAuthInfo& info, QCString& Response )
{
  KMD5 md;
  QCString HA1;
  QCString HA2;

  // Calculate H(A1)
  QCString authStr = info.username;
  authStr += ':';
  authStr += info.realm;
  authStr += ':';
  authStr += info.password;
  md.update( authStr );

  if ( info.algorithm == "md5-sess" )
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

  kdDebug(7113) << "(" << m_pid << ") A1 => " << HA1 << endl;

  // Calcualte H(A2)
  authStr = info.method;
  authStr += ':';
  authStr += info.digestURI.at( 0 );
  if ( info.qop == "auth-int" )
  {
    authStr += ':';
    authStr += info.entityBody;
  }
  md.reset();
  md.update( authStr );
  HA2 = md.hexDigest();

  kdDebug(7113) << "(" << m_pid << ") A2 => " << HA2 << endl;

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

  kdDebug(7113) << "(" << m_pid << ") Response => " << Response << endl;
}

QString HTTPProtocol::createDigestAuth ( bool isForProxy )
{
  const char *p;

  QString auth;
  QCString opaque;
  QCString Response;

  DigestAuthInfo info;

  opaque = "";
  if ( isForProxy )
  {
    auth = "Proxy-Authorization: Digest ";
    info.username = m_proxyURL.user().latin1();
    info.password = m_proxyURL.pass().latin1();
    p = m_strProxyAuthorization.latin1();
  }
  else
  {
    auth = "Authorization: Digest ";
    info.username = m_state.user.latin1();
    info.password = m_state.passwd.latin1();
    p = m_strAuthorization.latin1();
  }

  if ( info.username.isEmpty() || info.password.isEmpty() || !p )
    return QString::null;

  // info.entityBody = p;  // FIXME: send digest of data for POST action ??
  info.realm = "";
  info.algorithm = "MD5";
  info.nonce = "";
  info.qop = "";

  // Use some random # b/n 1 and 100,000 for generating the nonce value...
  info.cnonce.setNum((1 + static_cast<int>(100000.0*rand()/(RAND_MAX+1.0))));
  info.cnonce = KCodecs::base64Encode( info.cnonce );

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
    default:
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
      info.realm = QCString( p, i+1 );
    }
    else if (strncasecmp(p, "algorith=", 9)==0)
    {
      p+=9;
      while ( *p == '"' ) p++;  // Go past any number of " mark(s) first
      while ( ( p[i] != '"' ) && ( p[i] != ',' ) && ( p[i] != '\0' ) ) i++;
      info.algorithm = QCString(p, i+1).lower();
    }
    else if (strncasecmp(p, "algorithm=", 10)==0)
    {
      p+=10;
      while ( *p == '"' ) p++;  // Go past any " mark(s) first
      while ( ( p[i] != '"' ) && ( p[i] != ',' ) && ( p[i] != '\0' ) ) i++;
      info.algorithm = QCString(p,i+1).lower();
    }
    else if (strncasecmp(p, "domain=", 7)==0)
    {
      p+=7;
      while ( *p == '"' ) p++;  // Go past any " mark(s) first
      while ( p[i] != '"' ) i++;  // Read everything until the last " mark
      int pos = 0, idx = 0;
      QCString uri = QCString(p,i+1);
      do
      {
        pos = uri.find( ',', pos );
        if ( pos != -1 )
          info.digestURI.append( uri.mid(idx, pos-idx) );
        else
          info.digestURI.append( uri.mid(idx, uri.length()-idx) );
        idx = pos+1;
      } while ( pos != -1 );
    }
    else if (strncasecmp(p, "nonce=", 6)==0)
    {
      p+=6;
      while ( *p == '"' ) p++;  // Go past any " mark(s) first
      while ( p[i] != '"' ) i++;  // Read everything until the last " mark
      info.nonce = QCString(p,i+1);
    }
    else if (strncasecmp(p, "opaque=", 7)==0)
    {
      p+=7;
      while ( *p == '"' ) p++;  // Go past any " mark(s) first
      while ( p[i] != '"' ) i++;  // Read everything until the last " mark
      opaque = QCString(p,i+1);
    }
    else if (strncasecmp(p, "qop=", 4)==0)
    {
      p+=4;
      while ( *p == '"' ) p++;  // Go past any " mark(s) first
      while ( p[i] != '"' ) i++;  // Read everything until the last " mark
      info.qop = QCString(p,i+1);
    }
    p+=(i+1);
  }

  if ( info.digestURI.isEmpty() )
    info.digestURI.append( m_request.path.latin1() );

  kdDebug(7113) << "(" << m_pid << ") RESULT OF PARSING:" << endl;
  kdDebug(7113) << "(" << m_pid << ")   algorithm: " << info.algorithm << endl;
  kdDebug(7113) << "(" << m_pid << ")   realm:     " << info.realm << endl;
  kdDebug(7113) << "(" << m_pid << ")   nonce:     " << info.nonce << endl;
  kdDebug(7113) << "(" << m_pid << ")   opaque:    " << opaque << endl;
  kdDebug(7113) << "(" << m_pid << ")   qop:       " << info.qop << endl;

  int count = info.digestURI.count();
  for( int i = 0; i < count; i++ )
    kdDebug(7113) << "(" << m_pid << ")   domain[" << i << "]:    "
                  << info.digestURI.at(i) << endl;

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
  auth += info.digestURI.at(0);

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
  auth += "\"";

  kdDebug(7113) << "(" << m_pid << ") Digest header: " << auth << endl;
  return auth;
}

QString HTTPProtocol::proxyAuthenticationHeader()
{
  QString header;

  // We keep proxy authentication locally until they are changed.
  // Thus, no need to check with kdesud on every connection!!
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
        ProxyAuthentication = AUTH_Basic;
      else
        ProxyAuthentication = AUTH_Digest;
    }
    else
    {
      if ( checkCachedAuthentication(info) )
      {
        m_proxyURL.setUser( info.username );
        m_proxyURL.setPass( info.password );
        m_strProxyRealm = info.realmValue;
        if ( info.digestInfo.isEmpty() )
          ProxyAuthentication = AUTH_Basic;
        else
        {
          ProxyAuthentication = AUTH_Digest;
          m_strProxyAuthorization = info.digestInfo;
        }
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
      header += "\r\n";
      break;
    case AUTH_Digest:
      header += createDigestAuth( true );
      header += "\r\n";
      break;
    case AUTH_None:
    default:
      break;
  }

  return header;
}

/*

The webDAV DTD cannot be used with Qt's DOM yet...

static const char* webdavDTD =
"<!DOCTYPE webdav-1.0 ["
"  <!--============ XML Elements from Section 12 ==================-->"
"  <!ELEMENT activelock (lockscope, locktype, depth, owner?, timeout?, locktoken?) >"
"  <!ELEMENT lockentry (lockscope, locktype) >"
"  <!ELEMENT lockinfo (lockscope, locktype, owner?) >"
"  <!ELEMENT locktype (write) >"
"  <!ELEMENT write EMPTY >"
"  <!ELEMENT lockscope (exclusive | shared) >"
"  <!ELEMENT exclusive EMPTY >"
"  <!ELEMENT shared EMPTY >"
"  <!ELEMENT depth (#PCDATA) >"
"  <!ELEMENT owner ANY >"
"  <!ELEMENT timeout (#PCDATA) >"
"  <!ELEMENT locktoken (href+) >"
"  <!ELEMENT href (#PCDATA) >"
"  <!ELEMENT link (src+, dst+) >"
"  <!ELEMENT dst (#PCDATA) >"
"  <!ELEMENT src (#PCDATA) >"
"  <!ELEMENT multistatus (response+, responsedescription?) >"
"  <!ELEMENT response (href, ((href*, status)|(propstat+)), responsedescription?) >"
"  <!ELEMENT status (#PCDATA) >"
"  <!ELEMENT propstat (prop, status, responsedescription?) >"
"  <!ELEMENT responsedescription (#PCDATA) >"
"  <!ELEMENT prop ANY >"
"  <!ELEMENT propertybehavior (omit | keepalive) >"
"  <!ELEMENT omit EMPTY >"
"  <!ELEMENT keepalive (#PCDATA | href+) >"
"  <!ELEMENT propertyupdate (remove | set)+ >"
"  <!ELEMENT remove (prop) >"
"  <!ELEMENT set (prop) >"
"  <!ELEMENT propfind (allprop | propname | prop) >"
"  <!ELEMENT allprop EMPTY >"
"  <!ELEMENT propname EMPTY >"
"  <!ELEMENT collection EMPTY >"
"  <!--=========== Property Elements from Section 13 ===============-->"
"  <!ELEMENT creationdate (#PCDATA) >"
"  <!ELEMENT displayname (#PCDATA) >"
"  <!ELEMENT getcontentlanguage (#PCDATA) >"
"  <!ELEMENT getcontentlength (#PCDATA) >"
"  <!ELEMENT getcontenttype (#PCDATA) >"
"  <!ELEMENT getetag (#PCDATA) >"
"  <!ELEMENT getlastmodified (#PCDATA) >"
"  <!ELEMENT lockdiscovery (activelock)* >"
"  <!ELEMENT resourcetype ANY >"
"  <!ELEMENT source (link)* >"
"  <!ELEMENT supportedlock (lockentry)* >"
"]>";
*/
