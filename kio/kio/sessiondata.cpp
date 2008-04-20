/* This file is part of the KDE project
   Copyright (C) 2000 Dawit Alemayehu <adawit@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any
   later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 51 Franklin Street,
   Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "sessiondata.h"
#include "sessiondata.moc"

#include <config.h>

#include <QtCore/QList>
#include <QtCore/QTextCodec>

#include <kdebug.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kprotocolmanager.h>
#include <ksharedconfig.h>
#include <kstandarddirs.h>

#include <kdesu/client.h>
#include <kio/slaveconfig.h>
#include <kio/http_slave_defaults.h>

namespace KIO {

/***************************** SessionData::AuthData ************************/
struct SessionData::AuthData
{

public:
  AuthData() {}

  AuthData(const QByteArray& k, const QByteArray& g, bool p) {
    key = k;
    group = g;
    persist = p;
  }

  bool isKeyMatch( const QByteArray& val ) const {
    return (val==key);
  }

  bool isGroupMatch( const QByteArray& val ) const {
    return (val==group);
  }

  QByteArray key;
  QByteArray group;
  bool persist;
};

#if 0
/************************* SessionData::AuthDataList ****************************/
class SessionData::AuthDataList : public QList<SessionData::AuthData*>
{
public:
  AuthDataList();
  ~AuthDataList();

  void addData( SessionData::AuthData* );
  void removeData( const QByteArray& );

  bool pingCacheDaemon();
  void registerAuthData( SessionData::AuthData* );
  void unregisterAuthData( SessionData::AuthData* );
  void purgeCachedData();

private:
#ifdef Q_OS_UNIX
  KDEsuClient * m_kdesuClient;
#endif
};

SessionData::AuthDataList::AuthDataList()
{
#ifdef Q_OS_UNIX
  m_kdesuClient = new KDEsuClient;
#endif
  qDeleteAll(*this);
}

SessionData::AuthDataList::~AuthDataList()
{
  purgeCachedData();
#ifdef Q_OS_UNIX
  delete m_kdesuClient;
  m_kdesuClient = 0;
#endif
}

void SessionData::AuthDataList::addData( SessionData::AuthData* d )
{
  QList<SessionData::AuthData*>::iterator it;
  for ( it = begin() ; it != end(); ++it )
  {
    if ( (*it)->isKeyMatch( d->key ) )
        return;
  }
  registerAuthData( d );
  append( d );
}

void SessionData::AuthDataList::removeData( const QByteArray& gkey )
{
  QList<SessionData::AuthData*>::iterator it;
  for ( it = begin() ; it != end(); ++it )
  {
    if ( (*it)->isGroupMatch(gkey) &&  pingCacheDaemon() )
    {
        unregisterAuthData( (*it) );
        erase( it );
    }
  }
}

bool SessionData::AuthDataList::pingCacheDaemon()
{
#ifdef Q_OS_UNIX
  Q_ASSERT(m_kdesuClient);

  int success = m_kdesuClient->ping();
  if( success == -1 )
  {
    success = m_kdesuClient->startServer();
    if( success == -1 )
        return false;
  }
  return true;
#else
  return false;
#endif
}

void SessionData::AuthDataList::registerAuthData( SessionData::AuthData* d )
{
  if( !pingCacheDaemon() )
    return;

#ifdef Q_OS_UNIX
  bool ok;
  QByteArray ref_key = d->key + "-refcount";
  int count = m_kdesuClient->getVar(ref_key).toInt( &ok );
  if( ok )
  {
    QByteArray val;
    val.setNum( count+1 );
    m_kdesuClient->setVar( ref_key, val, 0, d->group );
  }
  else
    m_kdesuClient->setVar( ref_key, "1", 0, d->group );
#endif
}

void SessionData::AuthDataList::unregisterAuthData( SessionData::AuthData* d )
{
  if ( !d  || d->persist )
    return;

#ifdef Q_OS_UNIX
  bool ok;
  QByteArray ref_key = d->key + "-refcount";
  int count = m_kdesuClient->getVar( ref_key ).toInt( &ok );
  if ( ok )
  {
    if ( count > 1 )
    {
        QByteArray val;
        val.setNum(count-1);
        m_kdesuClient->setVar( ref_key, val, 0, d->group );
    }
    else
    {
        m_kdesuClient->delVars(d->key);
    }
  }
#endif
}

void SessionData::AuthDataList::purgeCachedData()
{
  if ( !isEmpty() && pingCacheDaemon() )
  {
    QList<SessionData::AuthData*>::iterator it;
    for ( it = begin() ; it != end(); ++it )
        unregisterAuthData( (*it) );
  }
}
#endif

/********************************* SessionData ****************************/

class SessionData::SessionDataPrivate
{
public:
  SessionDataPrivate() {
    useCookie = true;
    initDone = false;
  }

  bool initDone;
  bool useCookie;
  QString charsets;
  QString language;
};

SessionData::SessionData()
	:d(new SessionDataPrivate)
{
//  authData = 0;
}

SessionData::~SessionData()
{
  delete d;
}

void SessionData::configDataFor( MetaData &configData, const QString &proto,
                             const QString & )
{
  if ( (proto.startsWith("http", Qt::CaseInsensitive) ) ||
       (proto.startsWith("webdav", Qt::CaseInsensitive) ) )
  {
    if (!d->initDone)
        reset();

    // These might have already been set so check first
    // to make sure that we do not trumpt settings sent
    // by apps or end-user.
    if ( configData["Cookies"].isEmpty() )
        configData["Cookies"] = d->useCookie ? "true" : "false";
    if ( configData["Languages"].isEmpty() )
        configData["Languages"] = d->language;
    if ( configData["Charsets"].isEmpty() )
        configData["Charsets"] = d->charsets;
    if ( configData["CacheDir"].isEmpty() )
        configData["CacheDir"] = KGlobal::dirs()->saveLocation("cache", "http");
    if ( configData["UserAgent"].isEmpty() )
    {
      configData["UserAgent"] = KProtocolManager::defaultUserAgent();
    }
  }
}

void SessionData::reset()
{
    d->initDone = true;
    // Get Cookie settings...
    d->useCookie = KSharedConfig::openConfig("kcookiejarrc", KConfig::NoGlobals)->
                   group("Cookie Policy" ).
                   readEntry("Cookies", true);

    d->language = KProtocolManager::acceptLanguagesHeader();

    d->charsets = QString::fromLatin1(QTextCodec::codecForLocale()->name()).toLower();
    KProtocolManager::reparseConfiguration();
}

}
