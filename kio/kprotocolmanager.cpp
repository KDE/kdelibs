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
#include <kio/ioslave_defaults.h>
#include <kio/http_slave_defaults.h>
#include <kio/slaveconfig.h>

#include "kprotocolmanager.h"

KConfig *KProtocolManager::_config = 0;
KConfig *KProtocolManager::_http_config = 0;
KPAC *KProtocolManager::_pac = 0;
KStaticDeleter<KPAC> _pacDeleter;

// DEFUALT USERAGENT STRING
#define CFG_DEFAULT_UAGENT(X) \
QString("Mozilla/5.0 (compatible; Konqueror/%1%2)").arg(KDE_VERSION_STRING).arg(X)

void KProtocolManager::reparseConfiguration()
{
  delete _config;
  _config = 0;
  delete _http_config;
  _http_config = 0;
}

KConfig *KProtocolManager::config()
{
  if (!_config)
  {
     if (!_http_config)
        qAddPostRoutine(KProtocolManager::reparseConfiguration);
     _config = new KConfig("kioslaverc", false, false);
  }
  return _config;
}

KConfig *KProtocolManager::http_config()
{
  if (!_http_config)
  {
     if (!_config)
        qAddPostRoutine(KProtocolManager::reparseConfiguration);
     _http_config = new KConfig("kio_httprc", false, false);
  }
  return _http_config;
}

KPAC *KProtocolManager::pac()
{
  if (!_pac)
  {
    KConfig *cfg = config();
    cfg->setGroup( "Proxy Settings" );
    QString proxyScript = proxyConfigScript();
    if (!proxyScript.isEmpty())
    {
      KLibrary *lib = KLibLoader::self()->library("libkpac");
      if (lib)
      {
        KPAC *(*create_pac)() = (KPAC *(*)())(lib->symbol("create_pac"));
        if (create_pac)
        {
          _pacDeleter.setObject(_pac = create_pac());
          _pac->init( proxyScript );
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
  KConfig *cfg = http_config();
  return cfg->readBoolEntry( "UseCache", true );
}

KIO::CacheControl KProtocolManager::cacheControl()
{
    KConfig *cfg = http_config();
    QString tmp = cfg->readEntry("cache");
    if (tmp.isEmpty())
       return DEFAULT_CACHE_CONTROL;   
    return KIO::parseCacheControl(tmp);
}

void KProtocolManager::setCacheControl(KIO::CacheControl policy)
{
    KConfig *cfg = http_config();
    QString tmp = KIO::getCacheControlString(policy);
    cfg->writeEntry("cache", tmp);
    cfg->sync();
}

int KProtocolManager::maxCacheAge()
{
  KConfig *cfg = http_config();
  return cfg->readNumEntry( "MaxCacheAge", DEFAULT_MAX_CACHE_AGE ); // 14 days
}

int KProtocolManager::maxCacheSize()
{
  KConfig *cfg = http_config();
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
  if (!url.host().isEmpty() && pac())
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
        if (noProxy.isEmpty() || url.host().isEmpty() ||
            !revmatch( url.host().lower().latin1(),
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
  KConfig *cfg = http_config();
  cfg->writeEntry( "UseCache", _mode );
  cfg->sync();
}

void KProtocolManager::setMaxCacheSize( int cache_size )
{
  KConfig *cfg = http_config();
  cfg->writeEntry( "MaxCacheSize", cache_size );
  cfg->sync();
}

void KProtocolManager::setMaxCacheAge( int cache_age )
{
  KConfig *cfg = http_config();
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

QString KProtocolManager::proxyConfigScript()
{
  KConfig *cfg = config();
  cfg->setGroup( "Proxy Settings" );
  return cfg->readEntry( "Proxy Config Script" );
}

void KProtocolManager::setProxyConfigScript( const QString& _url )
{
  KConfig *cfg = config();
  cfg->setGroup( "Proxy Settings" );
  cfg->writeEntry( "Proxy Config Script", _url );
  cfg->sync();
  if (_pac)
    if (_url.isEmpty())
    {
      _pacDeleter.destructObject();
      _pac = 0;
    }
    else
      _pac->init( _url );
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
  QString user_agent = KIO::SlaveConfig::self()->configData("http", hostname, "UserAgent");

  if (user_agent.isEmpty())
    user_agent = defaultUserAgent();

  return user_agent;
}

QString KProtocolManager::cacheDir()
{
    KConfig *cfg = http_config();
    return cfg->readEntry("CacheDir", KGlobal::dirs()->saveLocation("data","kio_http/cache"));
}

QString KProtocolManager::defaultUserAgent( )
{
  QString modifiers = KIO::SlaveConfig::self()->configData("http", QString::null, "UserAgentKeys");
  return defaultUserAgent(modifiers);
}

QString KProtocolManager::defaultUserAgent( const QString &_modifiers )
{
  QString modifiers = _modifiers.lower();
  if (modifiers.isEmpty())
     modifiers = DEFAULT_USER_AGENT_KEYS;
     
  QString supp;
  struct utsname nam;
  if( uname(&nam) == 0 )
  {
    if( modifiers.contains('o') )
    {
      supp += QString("; %1").arg(nam.sysname);
      if ( modifiers.contains('v') )
        supp += QString(" %1").arg(nam.release);
    }
    if( modifiers.contains('p') )
    {
      supp += QString::fromLatin1("; X11");  // TODO: determine this valye instead of hardcoding...
    }
    if( modifiers.contains('m') )
    {
      supp += QString("; %1").arg(nam.machine);
    }
    if( modifiers.contains('l') )
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

// obsolete, remove me in KDE 3.0
void 
KProtocolManager::setUserAgentList( const QStringList& /*agentlist*/ )
{ }

// obsolete, remove me in KDE 3.0
QStringList 
KProtocolManager::userAgentList()
{ return QStringList(); }
