/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <string.h>
#include <sys/utsname.h>

#include <kstaticdeleter.h>
#include <kstringhandler.h>
#include <klibloader.h>
#include <kstddirs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kio/kpac.h>

#include "kprotocolmanager.h"

// CACHE SETTINGS
#define DEFAULT_MAX_CACHE_SIZE          5120          //  5 MB
#define DEFAULT_MAX_CACHE_AGE           60*60*24*14   // 14 DAYS
#define DEFAULT_EXPIRE_TIME             1*60          //  1 MIN

// DEFAULT TIMEOUT VALUE FOR REMOTE AND PROXY CONNECTION
// AND RESPONSE WAIT PERIOD.  NOTE: CHANGING THESE VALUES
// ALSO CHANGES THE DEFAULT ESTABLISHED INITIALLY.
#define DEFAULT_RESPONSE_TIMEOUT         60           //  1 MIN
#define DEFAULT_CONNECT_TIMEOUT          20           // 20 SEC
#define DEFAULT_READ_TIMEOUT             15           // 15 SEC
#define DEFAULT_PROXY_CONNECT_TIMEOUT    10           // 10 SEC

// MINIMUM TIMEOUT VALUE ALLOWED
#define MIN_TIMEOUT_VALUE                 2           //  2 SEC

// DEFUALT USERAGENT STRING
#define CFG_DEFAULT_UAGENT(X) \
QString("Mozilla/5.0 (compatible; Konqueror/%1%2)").arg(KDE_VERSION_STRING).arg(X)


KConfig *KProtocolManager::_config = 0;
KPAC *KProtocolManager::_pac = 0;
KStaticDeleter<KPAC> _pacDeleter;


void KProtocolManager::reparseConfiguration()
{
  delete _config;
  _config = 0;
}

KConfig *KProtocolManager::config()
{
  if (!_config)
  {
     qAddPostRoutine(KProtocolManager::reparseConfiguration);
     _config = new KConfig("kioslaverc", false, false);
  }
  return _config;
}

KPAC *KProtocolManager::pac()
{
  if (!_pac)
  {
    KConfig *cfg = config();
    cfg->setGroup( "Proxy Settings" );
    if (!cfg->readEntry( "Proxy Config Script" ).isEmpty())
    {
      KLibrary *lib = KLibLoader::self()->library("libkpac");
      if (lib)
      {
        KPAC *(*create_pac)() = (KPAC *(*)())(lib->symbol("create_pac"));
        if (create_pac)
        {
          _pacDeleter.setObject(_pac = create_pac());
          _pac->init();
        }
      }
    }
  }
  return _pac;
}

bool KProtocolManager::markPartial()
{
  KConfig *cfg = config();
  cfg->setGroup( QString::null );
  return cfg->readBoolEntry( "MarkPartial", true );
}

int KProtocolManager::minimumKeepSize()
{
  KConfig *cfg = config();
  cfg->setGroup( QString::null );
  return cfg->readNumEntry( "MinimumKeepSize", 5000 ); // 5000 byte
}

bool KProtocolManager::autoResume()
{
  KConfig *cfg = config();
  cfg->setGroup( QString::null );
  return cfg->readBoolEntry( "AutoResume", false );
}

bool KProtocolManager::persistentConnections()
{
  KConfig *cfg = config();
  cfg->setGroup( QString::null );
  return cfg->readBoolEntry( "PersistentConnections", true );
}

int KProtocolManager::readTimeout()
{
  KConfig *cfg = config();
  cfg->setGroup( QString::null );
  int val = cfg->readNumEntry( "ReadTimeout", DEFAULT_READ_TIMEOUT );
  return QMAX(MIN_TIMEOUT_VALUE, val);
}

int KProtocolManager::connectTimeout()
{
  KConfig *cfg = config();
  cfg->setGroup( QString::null );
  int val = cfg->readNumEntry( "ConnectTimeout", DEFAULT_CONNECT_TIMEOUT );
  return QMAX(MIN_TIMEOUT_VALUE, val);
}

int KProtocolManager::proxyConnectTimeout()
{
  KConfig *cfg = config();
  cfg->setGroup( QString::null );
  int val = cfg->readNumEntry( "ProxyConnectTimeout", DEFAULT_PROXY_CONNECT_TIMEOUT );
  return QMAX(MIN_TIMEOUT_VALUE, val);
}

int KProtocolManager::responseTimeout()
{
  KConfig *cfg = config();
  cfg->setGroup( QString::null );
  int val = cfg->readNumEntry( "ResponseTimeout", DEFAULT_RESPONSE_TIMEOUT );
  return QMAX(MIN_TIMEOUT_VALUE, val);
}

int KProtocolManager::defaultConnectTimeout()
{
  return DEFAULT_CONNECT_TIMEOUT;
}

int KProtocolManager::defaultProxyConnectTimeout()
{
  return DEFAULT_PROXY_CONNECT_TIMEOUT;
}

int KProtocolManager::defaultResponseTimeout()
{
  return DEFAULT_RESPONSE_TIMEOUT;
}

int KProtocolManager::defaultReadTimeout()
{
  return DEFAULT_READ_TIMEOUT;
}

int KProtocolManager::minimumTimeoutThreshold()
{
  return MIN_TIMEOUT_VALUE;
}

bool KProtocolManager::useProxy()
{
  KConfig *cfg = config();
  cfg->setGroup( "Proxy Settings" );
  return cfg->readBoolEntry( "UseProxy", false );
}

bool KProtocolManager::useCache()
{
  KConfig *cfg = config();
  cfg->setGroup( "Cache Settings" );
  return cfg->readBoolEntry( "UseCache", true );
}

KIO::CacheControl KProtocolManager::defaultCacheControl()
{
    KConfig *cfg = config();
    cfg->setGroup("Cache Settings");
    return KIO::parseCacheControl(cfg->readEntry("Default Cache Policy", "verify"));
}

void KProtocolManager::setDefaultCacheControl(KIO::CacheControl policy)
{
    KConfig *cfg = config();
    cfg->setGroup("Cache Settings");
    QString tmp = KIO::getCacheControlString(policy);
    cfg->writeEntry("Default Cache Policy", tmp);
    cfg->sync();
}

int KProtocolManager::maxCacheAge()
{
  KConfig *cfg = config();
  cfg->setGroup( "Cache Settings" );
  return cfg->readNumEntry( "MaxCacheAge", DEFAULT_MAX_CACHE_AGE ); // 14 days
}

int KProtocolManager::maxCacheSize()
{
  KConfig *cfg = config();
  cfg->setGroup( "Cache Settings" );
  return cfg->readNumEntry( "MaxCacheSize", DEFAULT_MAX_CACHE_SIZE ); // 5 MB
}

QString KProtocolManager::ftpProxy()
{
  return proxyFor( "ftp" );
}

QString KProtocolManager::httpProxy()
{
  return proxyFor( "http" );
}

QString KProtocolManager::noProxyFor()
{
  KConfig *cfg = config();
  cfg->setGroup( "Proxy Settings" );
  return cfg->readEntry( "NoProxyFor" );
}

QString KProtocolManager::proxyFor( const QString& protocol )
{
  KConfig *cfg = config();
  cfg->setGroup( "Proxy Settings" );
  return cfg->readEntry( protocol.lower() + "Proxy" );
}

QString KProtocolManager::proxyForURL( const KURL &url )
{
  if (pac())
    return pac()->proxyForURL( url );
  else
  {
    QString proxy = proxyFor( url.protocol() );
    return proxy.isEmpty() ? QString::fromLatin1("DIRECT") : proxy;
  }
}

void KProtocolManager::badProxy( const QString &proxy )
{
  if ( _pac ) // don't load KPAC here if it isn't already
    _pac->badProxy( proxy );
}

bool KProtocolManager::hasProxyConfigScript()
{
    return pac() != 0;
}

QString KProtocolManager::slaveProtocol( const QString & protocol )
{
  return ( useProxy() && !proxyFor(protocol).isEmpty() )
           ? QString::fromLatin1("http") : protocol;
}

/*
    Domain suffix match. E.g. return true if host is "cuzco.inka.de" and
    nplist is "inka.de,hadiko.de" or if host is "localhost" and nplist is
    "localhost".
*/
static bool revmatch(const char *host, const char *nplist)
{
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
      hptr--; nptr--;
    }
  }

  return false;
}


QString KProtocolManager::slaveProtocol(const KURL &url, QString &proxy)
{
  if (useProxy())
  {
     proxy = proxyForURL(url);
     if ((proxy != "DIRECT") && (!proxy.isEmpty()))
     {
        QString noProxy = noProxyFor();
        if (!revmatch( url.host().lower().latin1(),
                       noProxy.lower().latin1() ))
           return QString::fromLatin1("http");
     }
  }
  proxy = QString::null;
  return url.protocol();
}

void KProtocolManager::setReadTimeout( int _timeout )
{
  KConfig *cfg = config();
  cfg->setGroup( QString::null );
  cfg->writeEntry("ReadTimeout", QMAX(MIN_TIMEOUT_VALUE,_timeout));
  cfg->sync();
}

void KProtocolManager::setConnectTimeout( int _timeout )
{
  KConfig *cfg = config();
  cfg->setGroup( QString::null );
  cfg->writeEntry("ConnectTimeout", QMAX(MIN_TIMEOUT_VALUE,_timeout));
  cfg->sync();
}

void KProtocolManager::setProxyConnectTimeout( int _timeout )
{
  KConfig *cfg = config();
  cfg->setGroup( QString::null );
  cfg->writeEntry("ProxyConnectTimeout", QMAX(MIN_TIMEOUT_VALUE,_timeout));
  cfg->sync();
}

void KProtocolManager::setResponseTimeout( int _timeout )
{
  KConfig *cfg = config();
  cfg->setGroup( QString::null );
  cfg->writeEntry("ResponseTimeout", QMAX(MIN_TIMEOUT_VALUE,_timeout));
  cfg->sync();
}

void KProtocolManager::setMarkPartial( bool _mode )
{
  KConfig *cfg = config();
  cfg->setGroup( QString::null );
  cfg->writeEntry( "MarkPartial", _mode );
  cfg->sync();
}

void KProtocolManager::setMinimumKeepSize( int _size )
{
  KConfig *cfg = config();
  cfg->setGroup( QString::null );
  cfg->writeEntry( "MinimumKeepSize", _size );
  cfg->sync();
}

void KProtocolManager::setAutoResume( bool _mode )
{
  KConfig *cfg = config();
  cfg->setGroup( QString::null );
  cfg->writeEntry( "AutoResume", _mode );
  cfg->sync();
}

void KProtocolManager::setPersistentConnections( bool _mode )
{
  KConfig *cfg = config();
  cfg->setGroup( QString::null );
  cfg->writeEntry( "PersistentConnections", _mode );
  cfg->sync();
}

void KProtocolManager::setUseCache( bool _mode )
{
  KConfig *cfg = config();
  cfg->setGroup( "Cache Settings" );
  cfg->writeEntry( "UseCache", _mode );
  cfg->sync();
}

void KProtocolManager::setMaxCacheSize( int cache_size )
{
  KConfig *cfg = config();
  cfg->setGroup( "Cache Settings" );
  cfg->writeEntry( "MaxCacheSize", cache_size );
  cfg->sync();
}

void KProtocolManager::setMaxCacheAge( int cache_age )
{
  KConfig *cfg = config();
  cfg->setGroup( "Cache Settings" );
  cfg->writeEntry( "MaxCacheAge", cache_age );
  cfg->sync();
}

void KProtocolManager::setUseProxy( bool _mode )
{
  KConfig *cfg = config();
  cfg->setGroup( "Proxy Settings" );
  cfg->writeEntry( "UseProxy", _mode );
  cfg->sync();
}

void KProtocolManager::setFtpProxy( const QString& _proxy )
{
  setProxyFor( "ftp", _proxy );
}

void KProtocolManager::setHttpProxy( const QString& _proxy )
{
  setProxyFor( "http", _proxy );
}

void KProtocolManager::setNoProxyFor( const QString& _noproxy )
{
  KConfig *cfg = config();
  cfg->setGroup( "Proxy Settings" );
  cfg->writeEntry( "NoProxyFor", _noproxy );
  cfg->sync();
}

void KProtocolManager::setProxyConfigScript( const QString& _url )
{
  KConfig *cfg = config();
  cfg->setGroup( "Proxy Settings" );
  cfg->writeEntry( "Proxy Config Script", _url );
  cfg->sync();
  // TODO: download it
  if (_pac)
    _pac->init();
}

void KProtocolManager::setProxyFor( const QString& protocol, const QString& _proxy )
{
  KConfig *cfg = config();
  cfg->setGroup( "Proxy Settings" );
  cfg->writeEntry( protocol.lower() + "Proxy", _proxy );
  cfg->sync();
}

QString KProtocolManager::userAgentForHost( const QString& hostname )
{
  QString user_agent = defaultUserAgent();
  QStringList list = KProtocolManager::userAgentList();
  if ( list.count() > 0 )
  {
    QStringList::ConstIterator it = list.begin();
    for( ; it != list.end(); ++it)
    {
      QStringList split;
      int pos = (*it).find("::");
      if ( pos == -1 )
      {
        pos = (*it).find(':');
        if ( pos != -1 )
        {
          split.append((*it).left(pos));
          split.append((*it).mid(pos+1));
        }
      }
      else
        split = QStringList::split("::", (*it));

      QString match = split[0];
      int match_len = match.length();
      int host_len = hostname.length();
      if ( match.isEmpty() || split[1].isEmpty() ||
           match_len > host_len ||
           (match.contains( '.' ) == 1 && match[0] == '.') )
        continue;

      // We look for a reverse domain name match...
      int rev_match = hostname.findRev(match, -1, false);
      if ( rev_match != -1 && match_len == (host_len - rev_match) )
      {
        user_agent = split[1];
        break;
      }
    }
  }
  return user_agent;
}

QStringList KProtocolManager::userAgentList()
{
  KConfig *cfg = config();
  // NOTE: Please, please DO NOT remove this check before
  // this software is cycled into the next beta release!!!
  // Otherwise, you will be breaking backwards compatability!!! (DA)
  if( cfg->hasGroup("UserAgent") )
    cfg->setGroup( "UserAgent" );
  else
    cfg->setGroup("Browser Settings/UserAgent");

  QString entry;
  QStringList settingsList;
  int entries = cfg->readNumEntry( "EntriesCount", 0 );
  for( int i = 0; i < entries; i++ )
  {
    entry = cfg->readEntry( QString("Entry%1").arg(i), "" );
    // Ignore wildcard matches...
    if( !entry.isEmpty() && !entry.startsWith("*") )
      settingsList.append(entry);
  }
  return settingsList;
}

void KProtocolManager::setUserAgentList( const QStringList& agentList )
{
  KConfig *cfg = config();
  cfg->setGroup("UserAgent");

  int i = 0, count= agentList.count();
  cfg->writeEntry( "EntriesCount", count );
  QStringList::ConstIterator it = agentList.begin();
  if ( count )
  {
    for( ; it != agentList.end(); ++it )
      cfg->writeEntry( QString("Entry%1").arg(i++), *it );
  }
  cfg->sync();
}

QString KProtocolManager::cacheDir()
{
    KConfig *cfg = config();
    cfg->setGroup("Cache Settings");
    return cfg->readEntry("Cache Dir", KGlobal::dirs()->saveLocation("data","kio_http/cache"));
}

void KProtocolManager::setDefaultUserAgentModifiers( const UAMODIFIERS& mods )
{
  KConfig *cfg = config();
  cfg->setGroup("UserAgent");

  cfg->writeEntry("ShowOS", mods.showOS);
  cfg->writeEntry("ShowOSVersion", mods.showOSVersion);
  cfg->writeEntry("ShowPlatform", mods.showPlatform);
  cfg->writeEntry("ShowMachine", mods.showMachine);
  cfg->writeEntry("ShowLanguage", mods.showLanguage);
  cfg->sync();
}

void KProtocolManager::defaultUserAgentModifiers( UAMODIFIERS& mods )
{
  KConfig *cfg = config();
  cfg->setGroup("UserAgent");

  mods.showOS = cfg->readBoolEntry("ShowOS");
  mods.showOSVersion = cfg->readBoolEntry("ShowOSVersion");
  mods.showPlatform = cfg->readBoolEntry("ShowPlatform");
  mods.showMachine = cfg->readBoolEntry("ShowMachine");
  mods.showLanguage = cfg->readBoolEntry("ShowLanguage");
}

QString KProtocolManager::defaultUserAgent()
{
  KProtocolManager::UAMODIFIERS mods;
  defaultUserAgentModifiers( mods );
  return customDefaultUserAgent( mods );
}

QString KProtocolManager::customDefaultUserAgent( const UAMODIFIERS& mods )
{
  KConfig *cfg = config();
  cfg->setGroup("UserAgent");

  QString supp;
  struct utsname nam;
  if( uname(&nam) == 0 )
  {
    if( mods.showOS )
    {
      supp += QString("; %1").arg(nam.sysname);
      if ( mods.showOSVersion )
        supp += QString(" %1").arg(nam.release);
    }
    if( mods.showPlatform )
    {
      supp += QString::fromLatin1("; X11");  // TODO: determine this valye instead of hardcoding...
    }
    if( mods.showMachine )
    {
      supp += QString("; %1").arg(nam.machine);
    }
    if( mods.showLanguage )
    {
      QStringList languageList = KGlobal::locale()->languageList();
      QStringList::Iterator it = languageList.find( QString::fromLatin1("C") );
      if( it != languageList.end() )
      {
        if( languageList.contains( QString::fromLatin1("en") ) > 0 )
          languageList.remove( it );
        else
          (*it) = QString::fromLatin1("en");
      }
      if( languageList.count() )
        supp += QString("; %1").arg(languageList.join(", "));
    }
  }
  return CFG_DEFAULT_UAGENT(supp);
}

bool KProtocolManager::sendUserAgent()
{
  KConfig *cfg = config();
  cfg->setGroup("UserAgent");
  return cfg->readBoolEntry("SendUserAgent", true);
}

void KProtocolManager::setEnableSendUserAgent( bool show )
{
  KConfig *cfg = config();
  cfg->setGroup("UserAgent");
  cfg->writeEntry( "SendUserAgent", show );
  cfg->sync();
}
