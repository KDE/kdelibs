/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000- Waldo Bastain <bastain@kde.org>
   Copyright (C) 2000- Dawit Alemayehu <adawit@kde.org>
   Copyright (C) 2008 Jarosław Staniek <staniek@kde.org>

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

#include "kprotocolmanager.h"

#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <QtCore/QCoreApplication>
#include <QtDBus/QtDBus>

#include <kdeversion.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <kurl.h>
#include <kio/slaveconfig.h>
#include <kio/ioslave_defaults.h>
#include <kio/http_slave_defaults.h>

#include <kprotocolinfofactory.h>

class
KProtocolManagerPrivate
{
public:
   KProtocolManagerPrivate();

   ~KProtocolManagerPrivate();

   KSharedConfig::Ptr config;
   KSharedConfig::Ptr http_config;
   KUrl url;
   QString protocol;
   QString proxy;
   QString modifiers;
   QString useragent;

    QMap<QString /*mimetype*/, QString /*protocol*/> protocolForArchiveMimetypes;
};

K_GLOBAL_STATIC(KProtocolManagerPrivate, kProtocolManagerPrivate)

KProtocolManagerPrivate::KProtocolManagerPrivate()
{
    // post routine since KConfig::sync() breaks if called too late
    qAddPostRoutine(kProtocolManagerPrivate.destroy);
}

KProtocolManagerPrivate::~KProtocolManagerPrivate()
{
    qRemovePostRoutine(kProtocolManagerPrivate.destroy);
}


// DEFAULT USERAGENT STRING
#define CFG_DEFAULT_UAGENT(X) \
QString("Mozilla/5.0 (compatible; Konqueror/%1.%2%3) KHTML/%4.%5.%6 (like Gecko)") \
        .arg(KDE_VERSION_MAJOR).arg(KDE_VERSION_MINOR).arg(X).arg(KDE_VERSION_MAJOR).arg(KDE_VERSION_MINOR).arg(KDE_VERSION_RELEASE)

#define PRIVATE_DATA \
KProtocolManagerPrivate *d = kProtocolManagerPrivate

void KProtocolManager::reparseConfiguration()
{
    PRIVATE_DATA;
    if (d->http_config) {
        d->http_config->reparseConfiguration();
    }
    if (d->config) {
        d->config->reparseConfiguration();
    }
    d->protocol.clear();
    d->proxy.clear();
    d->modifiers.clear();
    d->useragent.clear();
    d->url.clear();

  // Force the slave config to re-read its config...
  KIO::SlaveConfig::self()->reset ();
}

KSharedConfig::Ptr KProtocolManager::config()
{
    PRIVATE_DATA;
  if (!d->config)
  {
     d->config = KSharedConfig::openConfig("kioslaverc", KConfig::NoGlobals);
  }
  return d->config;
}

static KConfigGroup http_config()
{
    PRIVATE_DATA;
  if (!d->http_config) {
     d->http_config = KSharedConfig::openConfig("kio_httprc", KConfig::NoGlobals);
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
    noProxy = QString::fromLocal8Bit(qgetenv(noProxy.toLocal8Bit()));

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

            if ( p.startsWith(QLatin1String("http")) || p == "ftp" || p == "gopher" )
            {
              QDBusReply<QString> reply =
                  QDBusInterface( "org.kde.kded", "/modules/proxyscout", "org.kde.KPAC.ProxyScout" )
                  .call( "proxyForUrl", u.url() );
              proxy = reply;
            }
          }
          break;
      case EnvVarProxy:
          proxy = QString::fromLocal8Bit(qgetenv(proxyFor(url.protocol()).toLocal8Bit())).trimmed();
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
      if ( nptr[-1]=='/' && hptr == host ) // "bugs.kde.org" vs "http://bugs.kde.org", the config UI says URLs are ok
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

    PRIVATE_DATA;
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
           if (d->url.isValid() && !d->url.protocol().isEmpty())
           {
              // The idea behind slave protocols is not applicable to http
              // and webdav protocols.
              QString protocol = url.protocol().toLower();
              if (protocol.startsWith(QLatin1String("http")) || protocol.startsWith(QLatin1String("webdav")))
                d->protocol = protocol;
              else
              {
                d->protocol = d->url.protocol();
                kDebug () << "slaveProtocol: " << d->protocol;
              }

              d->url = url;
              d->proxy = proxy;
              return d->protocol;
           }
        }
     }
  }

  d->url = url;
  d->proxy.clear(); proxy.clear();
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
    PRIVATE_DATA;
  QString modifiers = _modifiers.toLower();
  if (modifiers.isEmpty())
     modifiers = DEFAULT_USER_AGENT_KEYS;

  if (d->modifiers == modifiers)
     return d->useragent;

  QString systemName, systemVersion, machine, supp;
  if (getSystemNameVersionAndMachine( systemName, systemVersion, machine ))
  {
    if( modifiers.contains('o') )
    {
      supp += QString("; %1").arg(systemName);
      if ( modifiers.contains('v') )
        supp += QString(" %1").arg(systemVersion);
    }
#ifdef Q_WS_X11
    if( modifiers.contains('p') )
    {
      supp += QLatin1String("; X11");
    }
#endif
    if( modifiers.contains('m') )
    {
      supp += QString("; %1").arg(machine);
    }
    if( modifiers.contains('l') )
    {
      supp += QString("; %1").arg(KGlobal::locale()->language());
    }
  }
  d->modifiers = modifiers;
  d->useragent = CFG_DEFAULT_UAGENT(supp);
  return d->useragent;
}

QString KProtocolManager::userAgentForApplication( const QString &appName, const QString& appVersion,
  const QStringList& extraInfo )
{
  QString systemName, systemVersion, machine;
  QStringList info;
  if (getSystemNameVersionAndMachine( systemName, systemVersion, machine ))
    info += QString::fromLatin1("%1/%2").arg(systemName).arg(systemVersion);
  info += QString::fromLatin1("KDE/%1.%2.%3").arg(KDE_VERSION_MAJOR)
    .arg(KDE_VERSION_MINOR).arg(KDE_VERSION_RELEASE);
  if (!machine.isEmpty())
    info += machine;
  info += extraInfo;
  return QString::fromLatin1("%1/%2 (%3)").arg(appName).arg(appVersion).arg(info.join("; "));
}

bool KProtocolManager::getSystemNameVersionAndMachine(
  QString& systemName, QString& systemVersion, QString& machine )
{
  struct utsname unameBuf;
  if ( 0 != uname( &unameBuf ) )
    return false;
#if defined(Q_WS_WIN) && !defined(_WIN32_WCE)
  // we do not use unameBuf.sysname information constructed in kdewin32
  // because we want to get separate name and version
  systemName = QLatin1String( "Windows" );
  OSVERSIONINFOEX versioninfo;
  ZeroMemory(&versioninfo, sizeof(OSVERSIONINFOEX));
  // try calling GetVersionEx using the OSVERSIONINFOEX, if that fails, try using the OSVERSIONINFO
  versioninfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
  bool ok = GetVersionEx( (OSVERSIONINFO *) &versioninfo );
  if ( !ok ) {
    versioninfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    ok = GetVersionEx( (OSVERSIONINFO *) &versioninfo );
  }
  if ( ok )
    systemVersion = QString::fromLatin1("%1.%2")
      .arg(versioninfo.dwMajorVersion).arg(versioninfo.dwMinorVersion);
#else
  systemName = unameBuf.sysname;
  systemVersion = unameBuf.release;
#endif
  machine = unameBuf.machine;
  return true;
}

QString KProtocolManager::acceptLanguagesHeader()
{
  static const QString &english = KGlobal::staticQString("en");

  // User's desktop language preference.
  QStringList languageList = KGlobal::locale()->languageList();

  // Replace possible "C" in the language list with "en", unless "en" is
  // already pressent. This is to keep user's priorities in order.
  // If afterwards "en" is still not present, append it.
  int idx = languageList.indexOf(QString::fromLatin1("C"));
  if (idx != -1)
  {
    if (languageList.contains(english))
      languageList.removeAt(idx);
    else
      languageList[idx] = english;
  }
  if (!languageList.contains(english))
    languageList += english;

  // Some languages may have web codes different from locale codes,
  // read them from the config and insert in proper order.
  KConfig acclangConf("accept-languages.codes", KConfig::NoGlobals);
  KConfigGroup replacementCodes(&acclangConf, "ReplacementCodes");
  QStringList languageListFinal;
  foreach (const QString &lang, languageList)
  {
    const QStringList langs = replacementCodes.readEntry(lang, QStringList());
    if (langs.isEmpty())
      languageListFinal += lang;
    else
      languageListFinal += langs;
  }

  // The header is composed of comma separated languages, with an optional
  // associated priority estimate (q=1..0) defaulting to 1.
  // As our language tags are already sorted by priority, we'll just decrease
  // the value evenly
  int prio = 10;
  QString header;
  foreach (const QString &lang,languageListFinal) {
      header += lang;
      if (prio < 10)
          header += QString(";q=0.%1").arg(prio);
      // do not add cosmetic whitespace in here : it is less compatible (#220677)
      header += ",";
      if (prio > 1)
          --prio;
  }
  header.chop(1);

  // Some of the languages may have country specifier delimited by
  // underscore, or modifier delimited by at-sign.
  // The header should use dashes instead.
  header.replace('_', '-');
  header.replace('@', '-');

  return header;
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

QString KProtocolManager::protocolForArchiveMimetype( const QString& mimeType )
{
    PRIVATE_DATA;
    if (d->protocolForArchiveMimetypes.isEmpty()) {
        const KProtocolInfo::List allProtocols = KProtocolInfoFactory::self()->allProtocols();
        for (KProtocolInfo::List::const_iterator it = allProtocols.begin();
             it != allProtocols.end(); ++it) {
            const QStringList archiveMimetypes = (*it)->archiveMimeTypes();
            Q_FOREACH(const QString& mime, archiveMimetypes) {
                d->protocolForArchiveMimetypes.insert(mime, (*it)->name());
            }
        }
    }
    return d->protocolForArchiveMimetypes.value(mimeType);
}

#undef PRIVATE_DATA
