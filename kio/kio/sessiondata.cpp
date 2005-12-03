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

#include <q3cstring.h>
#include <q3ptrlist.h>
#include <qtextcodec.h>

#include <kdebug.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcharsets.h>
#include <dcopclient.h>
#include <kprotocolmanager.h>
#include <kstandarddirs.h>

#include <kdesu/client.h>
#include <kio/slaveconfig.h>
#include <kio/http_slave_defaults.h>

#include "sessiondata.h"
#include "sessiondata.moc"

namespace KIO {

/***************************** SessionData::AuthData ************************/
struct SessionData::AuthData
{

public:
  AuthData() {}

  AuthData(const Q3CString& k, const Q3CString& g, bool p) {
    key = k;
    group = g;
    persist = p;
  }

  bool isKeyMatch( const Q3CString& val ) const {
    return (val==key);
  }

  bool isGroupMatch( const Q3CString& val ) const {
    return (val==group);
  }

  Q3CString key;
  Q3CString group;
  bool persist;
};

/************************* SessionData::AuthDataList ****************************/
class SessionData::AuthDataList : public Q3PtrList<SessionData::AuthData>
{
public:
  AuthDataList();
  ~AuthDataList();

  void addData( SessionData::AuthData* );
  void removeData( const Q3CString& );

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
  setAutoDelete(true);
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
  Q3PtrListIterator<SessionData::AuthData> it ( *this );
  for ( ; it.current(); ++it )
  {
    if ( it.current()->isKeyMatch( d->key ) )
        return;
  }
  registerAuthData( d );
  append( d );
}

void SessionData::AuthDataList::removeData( const Q3CString& gkey )
{
  Q3PtrListIterator<SessionData::AuthData> it( *this );
  for( ; it.current(); ++it )
  {
    if ( it.current()->isGroupMatch(gkey) &&  pingCacheDaemon() )
    {
        unregisterAuthData( it.current() );
        remove( it.current() );
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
  Q3CString ref_key = d->key + "-refcount";
  int count = m_kdesuClient->getVar(ref_key).toInt( &ok );
  if( ok )
  {
    Q3CString val;
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

  bool ok;
  int count;
  Q3CString ref_key = d->key + "-refcount";

#ifdef Q_OS_UNIX
  count = m_kdesuClient->getVar( ref_key ).toInt( &ok );
  if ( ok )
  {
    if ( count > 1 )
    {
        Q3CString val;
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
    Q3PtrListIterator<SessionData::AuthData> it( *this );
    for ( ; it.current(); ++it )
        unregisterAuthData( it.current() );
  }
}

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
{
  authData = 0;
  d = new SessionDataPrivate;
}

SessionData::~SessionData()
{
  delete d;
  delete authData;
  d = 0L;
  authData = 0L;
}

void SessionData::configDataFor( MetaData &configData, const QString &proto,
                             const QString & )
{
  if ( (proto.find("http", 0, false) == 0 ) ||
     (proto.find("webdav", 0, false) == 0) )
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
    KConfig* cfg = new KConfig("kcookiejarrc", true, false);
    cfg->setGroup( "Cookie Policy" );
    d->useCookie = cfg->readBoolEntry( "Cookies", true );
    delete cfg;

    static const QString & english = KGlobal::staticQString( "en" );

    // Get language settings...
    QStringList languageList = KGlobal::locale()->languagesTwoAlpha();
    QStringList::Iterator it = languageList.find( QString::fromLatin1("C") );
    if ( it != languageList.end() )
    {
        if ( languageList.contains( english ) > 0 )
          languageList.remove( it );
        else
          (*it) = english;
    }
    if ( !languageList.contains( english ) )
       languageList.append( english );

    d->language = languageList.join( ", " );

    d->charsets = QString::fromLatin1(QTextCodec::codecForLocale()->mimeName()).toLower();
    KProtocolManager::reparseConfiguration();
}

void SessionData::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

}
