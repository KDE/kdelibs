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
#include <QtCore/QStringBuilder>
#include <QtNetwork/QSslSocket>
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
#include <kmimetypetrader.h>
#include <kio/slaveconfig.h>
#include <kio/ioslave_defaults.h>
#include <kio/http_slave_defaults.h>
#include <kprotocolinfofactory.h>

#define QL1S(x)   QLatin1String(x)
#define QL1C(x)   QLatin1Char(x)

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
QL1S("Mozilla/5.0 (compatible; Konqueror/") % \
QString::number(KDE::versionMajor()) % QL1C('.') % QString::number(KDE::versionMinor()) % \
X % QL1S(") KHTML/") % \
QString::number(KDE::versionMajor()) % QL1C('.') % QString::number(KDE::versionMinor()) % \
QL1C('.') % QString::number(KDE::versionRelease()) % QL1S(" (like Gecko)")

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
  QString key = protocol.toLower();

  if (key == QL1S("webdav"))
    key = QL1S("http");
  else if (key == QL1S("webdavs"))
    key = QL1S("https");

  key += QL1S("Proxy");

  return config()->group("Proxy Settings").readEntry(key, QString());
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
            if (p == QL1S("webdav"))
            {
              p = QL1S("http");
              u.setProtocol(p);
            }
            else if (p == QL1S("webdavs"))
            {
              p = QL1S("https");
              u.setProtocol(p);
            }

            if (p.startsWith(QL1S("http")) || p.startsWith(QL1S("ftp")))
            {
              QDBusReply<QString> reply = QDBusInterface(QL1S("org.kde.kded"),
                                                         QL1S("/modules/proxyscout"),
                                                         QL1S("org.kde.KPAC.ProxyScout"))
                                          .call(QL1S("proxyForUrl"), u.url() );
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

  return (proxy.isEmpty() ? QL1S("DIRECT") : proxy);
}

void KProtocolManager::badProxy( const QString &proxy )
{
  QDBusInterface( QL1S("org.kde.kded"), QL1S("/modules/proxyscout"))
      .call(QL1S("blackListProxy"), proxy);
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
              qhost += QL1C(':');
              qhost += QString::number(url.port());
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
              const QString protocol = url.protocol().toLower();
              if (protocol.startsWith(QL1S("http")) || protocol.startsWith(QL1S("webdav")))
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
  const QString sendUserAgent = KIO::SlaveConfig::self()->configData("http", hostname.toLower(), "SendUserAgent").toLower();
  if (sendUserAgent == QL1S("false"))
     return QString();

  const QString useragent = KIO::SlaveConfig::self()->configData("http", hostname.toLower(), "UserAgent");

  // Return the default user-agent if none is specified
  // for the requested host.
  if (useragent.isEmpty())
    return defaultUserAgent();

  return useragent;
}

QString KProtocolManager::defaultUserAgent( )
{
  const QString modifiers = KIO::SlaveConfig::self()->configData("http", QString(), "UserAgentKeys");
  return defaultUserAgent(modifiers);
}

static QString defaultUserAgentFromPreferredService()
{
  QString agentStr;

  // Check if the default COMPONENT contains a custom default UA string...
  KService::Ptr service = KMimeTypeTrader::self()->preferredService(QL1S("text/html"),
                                                      QL1S("KParts/ReadOnlyPart"));
  if (service && service->showInKDE())
    agentStr = service->property(QL1S("X-KDE-Default-UserAgent"),
                                 QVariant::String).toString();
  return agentStr;
}

QString KProtocolManager::defaultUserAgent( const QString &_modifiers )
{
    PRIVATE_DATA;
  QString modifiers = _modifiers.toLower();
  if (modifiers.isEmpty())
    modifiers = DEFAULT_USER_AGENT_KEYS;

  if (d->modifiers == modifiers && !d->useragent.isEmpty())
    return d->useragent;

  d->modifiers = modifiers;

  /*
     The following code attempts to determine the default user agent string
     from the 'X-KDE-UA-DEFAULT-STRING' property of the desktop file
     for the preferred service that was configured to handle the 'text/html'
     mime type. If the prefered service's desktop file does not specify this
     property, the long standing default user agent string will be used.
     The following keyword placeholders are automatically converted when the
     user agent string is read from the property:

     %SECURITY%      Expands to"U" when SSL is supported, "N" otherwise.
     %OSNAME%        Expands to operating system name, e.g. Linux.
     %OSVERSION%     Expands to operating system version, e.g. 2.6.32
     %SYSTYPE%       Expands to machine or system type, e.g. i386
     %PLATFORM%      Expands to windowing system, e.g. X11 on Unix/Linux.
     %LANGUAGE%      Expands to default language in use, e.g. en-US.
     %APPVERSION%    Expands to QCoreApplication applicationName()/applicationVerison(),
                     e.g. Konqueror/4.5.0. If application name and/or application version
                     number are not set, then "KDE" and the runtime KDE version numbers
                     are used respectively.

     All of the keywords are handled case-insensitively.
  */

  QString systemName, systemVersion, machine, supp;
  const bool sysInfoFound = getSystemNameVersionAndMachine( systemName, systemVersion, machine );
  QString agentStr = defaultUserAgentFromPreferredService();

  if (agentStr.isEmpty())
  {
    if (sysInfoFound)
    {
      if( modifiers.contains('o') )
      {
        supp += QL1S("; ");
        supp += systemName;
        if ( modifiers.contains('v') )
        {
          supp += QL1C(' ');
          supp += systemVersion;
        }
      }
#ifdef Q_WS_X11
      if( modifiers.contains('p') )
        supp += QL1S("; X11");
#endif
      if( modifiers.contains('m') )
      {
        supp += QL1S("; ");
        supp += machine;
      }
      if( modifiers.contains('l') )
      {
        supp += QL1S("; ");
        supp += KGlobal::locale()->language();
      }
    }

    d->useragent = CFG_DEFAULT_UAGENT(supp);
  }
  else
  {
    QString appName = QCoreApplication::applicationName();
    if (appName.isEmpty() || appName.startsWith(QL1S("kcmshell"), Qt::CaseInsensitive))
      appName = QL1S ("KDE");

    QString appVersion = QCoreApplication::applicationVersion();
    if (appVersion.isEmpty())
      appVersion += (QString::number(KDE::versionMajor()) % QL1C('.') %
                     QString::number(KDE::versionMinor()) % QL1C('.') %
                     QString::number(KDE::versionRelease()));

    appName += QL1C('/') % appVersion;

    agentStr.replace(QL1S("%appversion%"), appName, Qt::CaseInsensitive);

    if (QSslSocket::supportsSsl())
      agentStr.replace(QL1S("%security%"), QL1S("U"), Qt::CaseInsensitive);
    else
      agentStr.replace(QL1S("%security%"), QL1S("N"), Qt::CaseInsensitive);

    if (sysInfoFound)
    {
      if (modifiers.contains('o'))
      {
        agentStr.replace(QL1S("%osname%"), systemName, Qt::CaseInsensitive);
        if (modifiers.contains('v'))
            agentStr.replace(QL1S("%osversion%"), systemVersion, Qt::CaseInsensitive);
        else
            agentStr.remove(QL1S("%osversion%"), Qt::CaseInsensitive);
      }
      else
      {
         agentStr.remove(QL1S("%osname%"), Qt::CaseInsensitive);
         agentStr.remove(QL1S("%osversion%"), Qt::CaseInsensitive);
      }

      if (modifiers.contains('p'))
#if defined(Q_WS_X11)
        agentStr.replace(QL1S("%platform%"), QL1S("X11"), Qt::CaseInsensitive);
#elif defined(Q_WS_MAC)
        agentStr.replace(QL1S("%platform%"), QL1S("Macintosh"), Qt::CaseInsensitive);
#elif defined(Q_WS_WIN)
        agentStr.replace(QL1S("%platform%"), QL1S("Windows"), Qt::CaseInsensitive);
#elif defined (Q_WS_S60)
        agentStr.replace(QL1S("%platform%"), QL1S("Symbian"), Qt::CaseInsensitive);
#endif
      else
        agentStr.remove(QL1S("%platform%"), Qt::CaseInsensitive);

      if (modifiers.contains('m'))
        agentStr.replace(QL1S("%systype%"), machine, Qt::CaseInsensitive);
      else
        agentStr.remove(QL1S("%systype%"), Qt::CaseInsensitive);

      if (modifiers.contains('l'))
        agentStr.replace(QL1S("%language%"), KGlobal::locale()->language(), Qt::CaseInsensitive);
      else
        agentStr.remove(QL1S("%language%"), Qt::CaseInsensitive);

      // Clean up unnecessary separators that could be left over from the
      // possible keyword removal above...
      agentStr.replace(QRegExp("[(]\\s*[;]\\s*"), QL1S("("));
      agentStr.replace(QRegExp("[;]\\s*[;]\\s*"), QL1S(";"));
      agentStr.replace(QRegExp("\\s*[;]\\s*[)]"), QL1S(")"));
    }
    else
    {
      agentStr.remove(QL1S("%osname%"));
      agentStr.remove(QL1S("%osversion%"));
      agentStr.remove(QL1S("%platform%"));
      agentStr.remove(QL1S("%systype%"));
      agentStr.remove(QL1S("%language%"));
    }

    d->useragent = agentStr.simplified();
  }

  //kDebug() << "USERAGENT STRING:" << d->useragent;
  return d->useragent;
}

QString KProtocolManager::userAgentForApplication( const QString &appName, const QString& appVersion,
  const QStringList& extraInfo )
{
  QString systemName, systemVersion, machine, info;

  if (getSystemNameVersionAndMachine( systemName, systemVersion, machine ))
  {
    info +=  systemName;
    info += QL1C('/');
    info += systemVersion;
    info += QL1S("; ");
  }

  info += QL1S("KDE/");
  info += QString::number(KDE::versionMajor());
  info += QL1C('.');
  info += QString::number(KDE::versionMinor());
  info += QL1C('.');
  info += QString::number(KDE::versionRelease());

  if (!machine.isEmpty())
  {
    info += QL1S("; ");
    info += machine;
  }

  info += QL1S("; ");
  info += extraInfo.join(QL1S("; "));

  return (appName % QL1C('/') % appVersion % QL1S(" (") % info % QL1C(')'));
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
  systemName = QL1S( "Windows" );
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
    systemVersion = (QString::number(versioninfo.dwMajorVersion) %
                     QL1C('.') % QString::number(versioninfo.dwMinorVersion));
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
  Q_FOREACH (const QString &lang, languageList)
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
  Q_FOREACH (const QString &lang,languageListFinal) {
      header += lang;
      if (prio < 10) {
          header += QL1S(";q=0.");
          header += QString::number(prio);
      }
      // do not add cosmetic whitespace in here : it is less compatible (#220677)
      header += QL1S(",");
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
