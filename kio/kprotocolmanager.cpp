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
#include "kprotocolmanager.h"

#include <string.h>

#include <kstddirs.h>
#include <kglobal.h>

#include <kdebug.h>
#include <ksimpleconfig.h>
#include <qdir.h>
#include <qstrlist.h>
#include <kconfig.h>
#include <kstringhandler.h>

// CACHE SETTINGS
#define DEFAULT_MAX_CACHE_SIZE          5120          //  5 MB
#define DEFAULT_MAX_CACHE_AGE           60*60*24*14   // 14 DAYS

// MAXIMUM VALUE ALLOWED WHEN CONFIGURING
// REMOTE AND PROXY SERVERS CONNECTION AND
// RESPONSE TIMEOUTS.
#define MAX_RESPONSE_TIMEOUT            360           //  6 MIN
#define MAX_CONNECT_TIMEOUT             360           //  6 MIN
#define MAX_PROXY_CONNECT_TIMEOUT       120           //  2 MIN

// DEFAULT TIMEOUT VALUE FOR REMOTE AND PROXY CONNECTION
// AND RESPONSE WAIT PERIOD.  NOTE: CHANGING THESE VALUES
// ALSO CHANGES THE DEFAULT ESTABLISHED INITIALLY.
#define DEFAULT_RESPONSE_TIMEOUT         60           //  1 MIN
#define DEFAULT_CONNECT_TIMEOUT          20           // 20 SEC
#define DEFAULT_PROXY_CONNECT_TIMEOUT    10           // 10 SEC

// MINIMUM TIMEOUT VALUE ALLOWED
#define MIN_TIMEOUT_VALUE                 5           //  5 SEC

KConfig *KProtocolManager::_config = 0;

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

int KProtocolManager::readTimeout()
{
  KConfig *cfg = config();
  cfg->setGroup( QString::null );
  return cfg->readNumEntry( "ReadTimeout", 15 ); // 15 seconds
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

int KProtocolManager::connectTimeout()
{
  KConfig *cfg = config();
  cfg->setGroup( QString::null );
  int mrct = cfg->readNumEntry( "RemoteConnectTimeout", DEFAULT_CONNECT_TIMEOUT );
  if( mrct < MIN_TIMEOUT_VALUE || mrct > MAX_CONNECT_TIMEOUT )
    mrct = DEFAULT_CONNECT_TIMEOUT;
  return mrct;
}

int KProtocolManager::proxyConnectTimeout()
{
  KConfig *cfg = config();
  cfg->setGroup( QString::null );
  int mpct = cfg->readNumEntry( "ProxyConnectTimeout", DEFAULT_CONNECT_TIMEOUT );
  if( mpct < MIN_TIMEOUT_VALUE || mpct > MAX_PROXY_CONNECT_TIMEOUT )
    mpct = DEFAULT_PROXY_CONNECT_TIMEOUT;
  return mpct;
}

int KProtocolManager::responseTimeout()
{
  KConfig *cfg = config();
  cfg->setGroup( QString::null );
  int mrrt = cfg->readNumEntry( "RemoteResponseTimeout", DEFAULT_PROXY_CONNECT_TIMEOUT );
  if( mrrt < MIN_TIMEOUT_VALUE || mrrt > MAX_PROXY_CONNECT_TIMEOUT )
    mrrt = DEFAULT_RESPONSE_TIMEOUT;
  return mrrt;
}

int KProtocolManager::maximumConnectTimeout()
{
  return MAX_CONNECT_TIMEOUT;
}

int KProtocolManager::defaultConnectTimeout()
{
  return DEFAULT_CONNECT_TIMEOUT;
}

int KProtocolManager::maximumProxyConnectTimeout()
{
  return MAX_PROXY_CONNECT_TIMEOUT;
}

int KProtocolManager::defaultProxyConnectTimeout()
{
  return DEFAULT_PROXY_CONNECT_TIMEOUT;
}

int KProtocolManager::maximumResponseTimeout()
{
  return MAX_RESPONSE_TIMEOUT;
}

int KProtocolManager::defaultResponseTimeout()
{
  return DEFAULT_RESPONSE_TIMEOUT;
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
  return cfg->readNumEntry( "MaxCacheSize", DEFAULT_MAX_CACHE_SIZE );
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

QString KProtocolManager::slaveProtocol( const QString & protocol )
{
  return ( protocol == "ftp" && useProxy() && !proxyFor("ftp").isEmpty() )
           ? QString::fromLatin1("ftp-proxy") : protocol;
}

void KProtocolManager::setReadTimeout( int _timeout )
{
  KConfig *cfg = config();
  cfg->setGroup( QString::null );
  cfg->writeEntry( "ReadTimeout", _timeout );
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

void KProtocolManager::setProxyFor( const QString& protocol, const QString& _proxy )
{
  KConfig *cfg = config();
  cfg->setGroup( "Proxy Settings" );
  cfg->writeEntry( protocol.lower() + "Proxy", _proxy );
  cfg->sync();
}

QString KProtocolManager::userAgentForHost( const QString& hostname )
{
  KConfig *cfg = config();
  // NOTE: Please, please DO NOT remove this check before
  // this software is cycled into the next beta release!!!
  // Otherwise, you will be breaking backwards compatability!!! (DA)
  if( cfg->hasGroup("UserAgent") )
    cfg->setGroup( "UserAgent" );
  else
    cfg->setGroup( "Browser Settings/UserAgent" );

  int entries = cfg->readNumEntry( "EntriesCount", 0 );
  QStringList list;
  for( int i = 0; i < entries; i++ )
  {
      QString key = QString( "Entry%1" ).arg( i );
      list.append( cfg->readEntry( key, "" ) );
  }

  QString user_agent = DEFAULT_USERAGENT_STRING;

  if ( list.count() == 0 )
    return user_agent;

  // Now, we need to do our pattern matching on the host name.
  QStringList::ConstIterator it(list.begin());
  for( ; it != list.end(); ++it)
  {
    QStringList split(QStringList::split( ':', (*it) ));

    // if our user agent is null, we go to the next one
    if ( split[1].isNull() )
      continue;

    QRegExp regexp(split[0], true, true);

    // we also make sure our regexp is valid
    if ( !regexp.isValid() )
      continue;

    // we look for a match
    if ( regexp.match( hostname ) > -1 )
    {
      user_agent = split[1];

      // if the match was for '*', we keep trying.. otherwise, we are
      // done
      if ( split[0] != "*" )
        break;
    }
  }

  return user_agent;
}

QStringList KProtocolManager::userAgentList()
{
  KConfig *cfg = config();

  if( cfg->hasGroup("UserAgent") )
    cfg->setGroup( "UserAgent" );
  else
    cfg->setGroup("Browser Settings/UserAgent");

  QStringList settingsList;
  int entries = cfg->readNumEntry( "EntriesCount", 0 );
  for( int i = 0; i < entries; i++ )
  {
    QString entry = cfg->readEntry( QString("Entry%1").arg(i), "" );
    if (entry.left(37) == "*:Mozilla/5.0 (compatible; Konqueror/") // update version number
      settingsList.append( "*:" + DEFAULT_USERAGENT_STRING );
    else
      settingsList.append( entry );
  }
  return settingsList;
}

void KProtocolManager::setUserAgentList( const QStringList& agentList )
{
  KConfig *cfg = config();

  cfg->setGroup("UserAgent");

  int count = agentList.count();
  int i = 0;
  cfg->writeEntry( "EntriesCount", count );
  for( QStringList::ConstIterator it = agentList.begin(); it != agentList.end() ; ++it )
  {
      cfg->writeEntry( QString("Entry%1").arg(i++), *it );
  }
  cfg->sync();
}
