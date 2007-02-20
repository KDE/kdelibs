/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000- Waldo Bastain <bastain@kde.org>
   Copyright (C) 2000- Dawit Alemayehu <adawit@kde.org>

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

#include <string.h>
#include <sys/utsname.h>
#include <QtDBus/QtDBus>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kstandarddirs.h>
#include <klibloader.h>
#include <kstringhandler.h>
#include <kstaticdeleter.h>
#include <kurl.h>
#include <kio/slaveconfig.h>
#include <kio/ioslave_defaults.h>
#include <kio/http_slave_defaults.h>

#include "kprotocolmanager.h"
#include <kprotocolinfofactory.h>

class
KProtocolManagerPrivate
{
public:
   KProtocolManagerPrivate();

   ~KProtocolManagerPrivate();

   KConfig *config;
   KConfig *http_config;
   bool init_busy;
   KUrl url;
   QString protocol;
   QString proxy;
   QString modifiers;
   QString useragent;
};

static KProtocolManagerPrivate* d = 0;
static KStaticDeleter<KProtocolManagerPrivate> kpmpksd;

KProtocolManagerPrivate::KProtocolManagerPrivate()
                        :config(0), http_config(0), init_busy(false)
{
   kpmpksd.setObject(d, this);
}

KProtocolManagerPrivate::~KProtocolManagerPrivate()
{
   delete config;
   delete http_config;
}


// DEFAULT USERAGENT STRING
#define CFG_DEFAULT_UAGENT(X) \
QString("Mozilla/5.0 (compatible; Konqueror/%1.%2%3) KHTML/%4.%5.%6 (like Gecko)") \
        .arg(KDE_VERSION_MAJOR).arg(KDE_VERSION_MINOR).arg(X).arg(KDE_VERSION_MAJOR).arg(KDE_VERSION_MINOR).arg(KDE_VERSION_RELEASE)

void KProtocolManager::reparseConfiguration()
{
  kpmpksd.destructObject();

  // Force the slave config to re-read its config...
  KIO::SlaveConfig::self()->reset ();
}

KConfig *KProtocolManager::config()
{
  if (!d)
     d = new KProtocolManagerPrivate;

  if (!d->config)
  {
     d->config = new KConfig("kioslaverc", KConfig::NoGlobals);
  }
  return d->config;
}

KConfigGroup KProtocolManager::http_config()
{
  if (!d)
     d = new KProtocolManagerPrivate;

  if (!d->http_config)
  {
     d->http_config = new KConfig("kio_httprc", KConfig::NoGlobals);
  }
  return KConfigGroup(d->http_config, QString());
}

/*=============================== TIMEOUT SETTINGS ==========================*/

int KProtocolManager::readTimeout()
{
  KConfigGroup cg( config(), QString() );
  int val = cg.readEntry( "ReadTimeout", DEFAULT_READ_TIMEOUT );
  return qMax(MIN_TIMEOUT_VALUE, val);
}

int KProtocolManager::connectTimeout()
{
  KConfigGroup cg( config(), QString() );
  int val = cg.readEntry( "ConnectTimeout", DEFAULT_CONNECT_TIMEOUT );
  return qMax(MIN_TIMEOUT_VALUE, val);
}

int KProtocolManager::proxyConnectTimeout()
{
  KConfigGroup cg( config(), QString() );
  int val = cg.readEntry( "ProxyConnectTimeout", DEFAULT_PROXY_CONNECT_TIMEOUT );
  return qMax(MIN_TIMEOUT_VALUE, val);
}

int KProtocolManager::responseTimeout()
{
  KConfigGroup cg( config(), QString() );
  int val = cg.readEntry( "ResponseTimeout", DEFAULT_RESPONSE_TIMEOUT );
  return qMax(MIN_TIMEOUT_VALUE, val);
}

/*========================== PROXY SETTINGS =================================*/

bool KProtocolManager::useProxy()
{
  return proxyType() != NoProxy;
}

bool KProtocolManager::useReverseProxy()
{
  KConfigGroup cg(config(), "Proxy Settings" );
  return cg.readEntry("ReversedException", false);
}

KProtocolManager::ProxyType KProtocolManager::proxyType()
{
  KConfigGroup cg(config(), "Proxy Settings" );
  return static_cast<ProxyType>(cg.readEntry( "ProxyType" , 0));
}

KProtocolManager::ProxyAuthMode KProtocolManager::proxyAuthMode()
{
  KConfigGroup cg(config(), "Proxy Settings" );
  return static_cast<ProxyAuthMode>(cg.readEntry( "AuthMode" , 0));
}

/*========================== CACHING =====================================*/

bool KProtocolManager::useCache()
{
  return http_config().readEntry( "UseCache", true );
}

KIO::CacheControl KProtocolManager::cacheControl()
{
  QString tmp = http_config().readEntry("cache");
  if (tmp.isEmpty())
    return DEFAULT_CACHE_CONTROL;
  return KIO::parseCacheControl(tmp);
}

QString KProtocolManager::cacheDir()
{
  return http_config().readPathEntry("CacheDir", KGlobal::dirs()->saveLocation("cache","http"));
}

int KProtocolManager::maxCacheAge()
{
  return http_config().readEntry( "MaxCacheAge", DEFAULT_MAX_CACHE_AGE ); // 14 days
}

int KProtocolManager::maxCacheSize()
{
  return http_config().readEntry( "MaxCacheSize", DEFAULT_MAX_CACHE_SIZE ); // 5 MB
}

QString KProtocolManager::noProxyFor()
{
  KProtocolManager::ProxyType type = proxyType();

  QString noProxy = config()->group("Proxy Settings").readEntry( "NoProxyFor" );
  if (type == EnvVarProxy)
    noProxy = QString::fromLocal8Bit(getenv(noProxy.toLocal8Bit()));

  return noProxy;
}

QString KProtocolManager::proxyFor( const QString& protocol )
{
  QString scheme = protocol.toLower();

  if (scheme == "webdav")
    scheme = "http";
  else if (scheme == "webdavs")
    scheme = "https";

  return config()->group("Proxy Settings" ).readEntry( scheme + "Proxy", QString() );
}

QString KProtocolManager::proxyForUrl( const KUrl &url )
{
  QString proxy;
  ProxyType pt = proxyType();

  switch (pt)
  {
      case PACProxy:
      case WPADProxy:
          if (!url.host().isEmpty())
          {
            KUrl u (url);
            QString p = u.protocol().toLower();

            // webdav is a KDE specific protocol. Look up proxy
            // information using HTTP instead...
            if ( p == "webdav" )
            {
              p = "http";
              u.setProtocol( p );
            }
            else if ( p == "webdavs" )
            {
              p = "https";
              u.setProtocol( p );
            }

            if ( p.startsWith("http") || p == "ftp" || p == "gopher" )
            {
              QDBusReply<QString> reply =
                  QDBusInterface( "org.kde.kded", "/modules/proxyscout", "org.kde.KPAC.ProxyScout" )
                  .call( "proxyForURL", u.url() );
              proxy = reply;
            }
          }
          break;
      case EnvVarProxy:
          proxy = QString::fromLocal8Bit(getenv(proxyFor(url.protocol()).toLocal8Bit())).trimmed();
          break;
      case ManualProxy:
          proxy = proxyFor( url.protocol() );
          break;
      case NoProxy:
      default:
          break;
  }

  return (proxy.isEmpty() ? QLatin1String("DIRECT") : proxy);
}

void KProtocolManager::badProxy( const QString &proxy )
{
  QDBusInterface( "org.kde.kded", "/modules/proxyscout" )
      .call( "blackListProxy", proxy );
}

/*
    Domain suffix match. E.g. return true if host is "cuzco.inka.de" and
    nplist is "inka.de,hadiko.de" or if host is "localhost" and nplist is
    "localhost".
*/
static bool revmatch(const char *host, const char *nplist)
{
  if (host == 0)
    return false;

  const char *hptr = host + strlen( host ) - 1;
  const char *nptr = nplist + strlen( nplist ) - 1;
  const char *shptr = hptr;

  while ( nptr >= nplist )
  {
    if ( *hptr != *nptr )
    {
      hptr = shptr;

      // Try to find another domain or host in the list
      while(--nptr>=nplist && *nptr!=',' && *nptr!=' ') ;

      // Strip out multiple spaces and commas
      while(--nptr>=nplist && (*nptr==',' || *nptr==' ')) ;
    }
    else
    {
      if ( nptr==nplist || nptr[-1]==',' || nptr[-1]==' ')
        return true;
      if ( hptr == host ) // e.g. revmatch("bugs.kde.org","mybugs.kde.org")
        return false;

      hptr--;
      nptr--;
    }
  }

  return false;
}

QString KProtocolManager::slaveProtocol(const KUrl &url, QString &proxy)
{
  if (url.hasSubUrl()) // We don't want the suburl's protocol
  {
     KUrl::List list = KUrl::split(url);
     KUrl l = list.last();
     return slaveProtocol(l, proxy);
  }

  if (!d)
    d = new KProtocolManagerPrivate;

  if (d->url == url)
  {
     proxy = d->proxy;
     return d->protocol;
  }

  if (useProxy())
  {
     proxy = proxyForUrl(url);
     if ((proxy != "DIRECT") && (!proxy.isEmpty()))
     {
        bool isRevMatch = false;
        KProtocolManager::ProxyType type = proxyType();
        bool useRevProxy = ((type == ManualProxy) && useReverseProxy());

        QString noProxy;
        // Check no proxy information iff the proxy type is either
        // manual or environment variable based...
        if ( (type == ManualProxy) || (type == EnvVarProxy) )
          noProxy = noProxyFor();

        if (!noProxy.isEmpty())
        {
           QString qhost = url.host().toLower();
           QByteArray host = qhost.toLatin1();
           QString qno_proxy = noProxy.trimmed().toLower();
           const QByteArray no_proxy = qno_proxy.toLatin1();
           isRevMatch = revmatch(host, no_proxy);

           // If no match is found and the request url has a port
           // number, try the combination of "host:port". This allows
           // users to enter host:port in the No-proxy-For list.
           if (!isRevMatch && url.port() > 0)
           {
              qhost += ':' + QString::number (url.port());
              host = qhost.toLatin1();
              isRevMatch = revmatch (host, no_proxy);
           }

           // If the hostname does not contain a dot, check if
           // <local> is part of noProxy.
           if (!isRevMatch && !host.isEmpty() && (strchr(host, '.') == NULL))
              isRevMatch = revmatch("<local>", no_proxy);
        }

        if ( (!useRevProxy && !isRevMatch) || (useRevProxy && isRevMatch) )
        {
           d->url = proxy;
           if ( d->url.isValid() )
           {
              // The idea behind slave protocols is not applicable to http
              // and webdav protocols.
              QString protocol = url.protocol().toLower();
              if (protocol.startsWith("http") || protocol.startsWith("webdav"))
                d->protocol = protocol;
              else
              {
                d->protocol = d->url.protocol();
                kDebug () << "slaveProtocol: " << d->protocol << endl;
              }

              d->url = url;
              d->proxy = proxy;
              return d->protocol;
           }
        }
     }
  }

  d->url = url;
  d->proxy = proxy = QString();
  d->protocol = url.protocol();
  return d->protocol;
}

/*================================= USER-AGENT SETTINGS =====================*/

QString KProtocolManager::userAgentForHost( const QString& hostname )
{
  QString sendUserAgent = KIO::SlaveConfig::self()->configData("http", hostname.toLower(), "SendUserAgent").toLower();
  if (sendUserAgent == "false")
     return QString();

  QString useragent = KIO::SlaveConfig::self()->configData("http", hostname.toLower(), "UserAgent");

  // Return the default user-agent if none is specified
  // for the requested host.
  if (useragent.isEmpty())
    return defaultUserAgent();

  return useragent;
}

QString KProtocolManager::defaultUserAgent( )
{
  QString modifiers = KIO::SlaveConfig::self()->configData("http", QString(), "UserAgentKeys");
  return defaultUserAgent(modifiers);
}

QString KProtocolManager::defaultUserAgent( const QString &_modifiers )
{
  if (!d)
     d = new KProtocolManagerPrivate;

  QString modifiers = _modifiers.toLower();
  if (modifiers.isEmpty())
     modifiers = DEFAULT_USER_AGENT_KEYS;

  if (d->modifiers == modifiers)
     return d->useragent;

  QString supp;
  struct utsname nam;
  if( uname(&nam) >= 0 )
  {
    if( modifiers.contains('o') )
    {
      supp += QString("; %1").arg(nam.sysname);
      if ( modifiers.contains('v') )
        supp += QString(" %1").arg(nam.release);
    }
    if( modifiers.contains('p') )
    {
      supp += QLatin1String("; X11");  // TODO: determine this valye instead of hardcoding...
    }
    if( modifiers.contains('m') )
    {
      supp += QString("; %1").arg(nam.machine);
    }
    if( modifiers.contains('l') )
    {
      static const QString & english = KGlobal::staticQString( "en" );

      QStringList languageList = KGlobal::locale()->languageList();
      int idx = languageList.indexOf( QLatin1String("C") );
      if( idx != -1 )
      {
        if( languageList.contains( english ) )
          languageList.removeAt( idx );
        else
          languageList[idx] = english;
      }
      if( languageList.count() )
        supp += QString("; %1").arg(languageList.join(", "));
    }
  }
  d->modifiers = modifiers;
  d->useragent = CFG_DEFAULT_UAGENT(supp);
  return d->useragent;
}

/*==================================== OTHERS ===============================*/

bool KProtocolManager::markPartial()
{
  return config()->group(QByteArray()).readEntry( "MarkPartial", true );
}

int KProtocolManager::minimumKeepSize()
{
    return config()->group(QByteArray()).readEntry( "MinimumKeepSize",
                                                DEFAULT_MINIMUM_KEEP_SIZE ); // 5000 byte
}

bool KProtocolManager::autoResume()
{
  return config()->group(QByteArray()).readEntry( "AutoResume", false );
}

bool KProtocolManager::persistentConnections()
{
  return config()->group(QByteArray()).readEntry( "PersistentConnections", true );
}

bool KProtocolManager::persistentProxyConnection()
{
  return config()->group(QByteArray()).readEntry( "PersistentProxyConnection", false );
}

QString KProtocolManager::proxyConfigScript()
{
  return config()->group("Proxy Settings").readEntry( "Proxy Config Script" );
}

/* =========================== PROTOCOL CAPABILITIES ============== */

static KProtocolInfo::Ptr findProtocol(const KUrl &url)
{
   QString protocol = url.protocol();

   if ( !KProtocolInfo::proxiedBy( protocol ).isEmpty() )
   {
      QString dummy;
      protocol = KProtocolManager::slaveProtocol(url, dummy);
   }

   return KProtocolInfoFactory::self()->findProtocol(protocol);
}


KProtocolInfo::Type KProtocolManager::inputType( const KUrl &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return KProtocolInfo::T_NONE;

  return prot->m_inputType;
}

KProtocolInfo::Type KProtocolManager::outputType( const KUrl &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return KProtocolInfo::T_NONE;

  return prot->m_outputType;
}


bool KProtocolManager::isSourceProtocol( const KUrl &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_isSourceProtocol;
}

bool KProtocolManager::supportsListing( const KUrl &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsListing;
}

QStringList KProtocolManager::listing( const KUrl &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return QStringList();

  return prot->m_listing;
}

bool KProtocolManager::supportsReading( const KUrl &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsReading;
}

bool KProtocolManager::supportsWriting( const KUrl &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsWriting;
}

bool KProtocolManager::supportsMakeDir( const KUrl &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsMakeDir;
}

bool KProtocolManager::supportsDeleting( const KUrl &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsDeleting;
}

bool KProtocolManager::supportsLinking( const KUrl &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsLinking;
}

bool KProtocolManager::supportsMoving( const KUrl &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsMoving;
}

bool KProtocolManager::supportsOpening( const KUrl &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_supportsOpening;
}

bool KProtocolManager::canCopyFromFile( const KUrl &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_canCopyFromFile;
}


bool KProtocolManager::canCopyToFile( const KUrl &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->m_canCopyToFile;
}

bool KProtocolManager::canRenameFromFile( const KUrl &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->canRenameFromFile();
}


bool KProtocolManager::canRenameToFile( const KUrl &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->canRenameToFile();
}

bool KProtocolManager::canDeleteRecursive( const KUrl &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return false;

  return prot->canDeleteRecursive();
}

KProtocolInfo::FileNameUsedForCopying KProtocolManager::fileNameUsedForCopying( const KUrl &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return KProtocolInfo::FromUrl;

  return prot->fileNameUsedForCopying();
}

QString KProtocolManager::defaultMimetype( const KUrl &url )
{
  KProtocolInfo::Ptr prot = findProtocol(url);
  if ( !prot )
    return QString();

  return prot->m_defaultMimetype;
}

