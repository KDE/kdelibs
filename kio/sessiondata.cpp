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
   write to the Free Software Foundation, Inc., 59 Temple Place -
   Suite 330, Boston, MA 02111-1307, USA.
*/

#include <qlist.h>

#include <kdebug.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcharsets.h>
#include <dcopclient.h>
#include <kprotocolmanager.h>

#include <kdesu/client.h>
#include <kio/slaveconfig.h>
#include <kio/http_slave_defaults.h>

#include "sessiondata.h"



using namespace KIO;

/***************************** KIO::SessionData::AuthData ************************/
struct KIO::SessionData::AuthData
{

public:
    AuthData() {}

    AuthData(const QCString& k, const QCString& g, bool p) {
        key = k;
        group = g;
        persist = p;
    }

    bool isKeyMatch( const QCString& val ) const {
        return (val==key);
    }

    bool isGroupMatch( const QCString& val ) const {
        return (val==group);
    }

    QCString key;
    QCString group;
    bool persist;
};

/************************* KIO::SessionData::AuthDataList ****************************/
class KIO::SessionData::AuthDataList : public QList<KIO::SessionData::AuthData>
{
public:
    AuthDataList() { setAutoDelete(true); }
    ~AuthDataList() { purgeCachedData(); }

    void addData( KIO::SessionData::AuthData* );
    void removeData( const QCString& );

private:
    bool pingCacheDaemon();
    void registerAuthData( KIO::SessionData::AuthData* );
    void unregisterAuthData( KIO::SessionData::AuthData* );
    void purgeCachedData();
};

void KIO::SessionData::AuthDataList::addData( KIO::SessionData::AuthData* d )
{
    QListIterator<KIO::SessionData::AuthData> it ( *this );
    for ( ; it.current(); ++it )
    {
        if ( it.current()->isKeyMatch( d->key ) )
            return;
    }
    registerAuthData( d );
    append( d );
}

void KIO::SessionData::AuthDataList::removeData( const QCString& gkey )
{
    QListIterator<KIO::SessionData::AuthData> it( *this );
    for( ; it.current(); ++it )
    {
        if ( it.current()->isGroupMatch(gkey) &&  pingCacheDaemon() )
        {
            unregisterAuthData( it.current() );
            remove( it.current() );
        }
    }
}

bool KIO::SessionData::AuthDataList::pingCacheDaemon()
{
    KDEsuClient client;
    int sucess = client.ping();
    if( sucess == -1 )
    {
        sucess = client.startServer();
        if( sucess == -1 )
            return false;
    }
    return true;
}

void KIO::SessionData::AuthDataList::registerAuthData( KIO::SessionData::AuthData* d )
{
    if( !pingCacheDaemon() )
        return;

    bool ok;
    KDEsuClient client;
    QCString ref_key = d->key + "-refcount";
    int count = client.getVar(ref_key).toInt( &ok );
    if( ok )
    {
        QCString val;
        val.setNum( count+1 );
        client.setVar( ref_key, val, 0, d->group );
    }
    else
        client.setVar( ref_key, "1", 0, d->group );
}

void KIO::SessionData::AuthDataList::unregisterAuthData( KIO::SessionData::AuthData* d )
{
    if ( !d  || d->persist )
        return;

    bool ok;
    int count;
    KDEsuClient client;
    QCString ref_key = d->key + "-refcount";

    count = client.getVar( ref_key ).toInt( &ok );
    if ( ok )
    {
        if ( count > 1 )
        {
            QCString val;
            val.setNum(count-1);
            client.setVar( ref_key, val, 0, d->group );
        }
        else
        {
            client.delVars(d->key);
        }
    }
}

void KIO::SessionData::AuthDataList::purgeCachedData()
{
    if ( !isEmpty() && pingCacheDaemon() )
    {
        QListIterator<KIO::SessionData::AuthData> it( *this );
        for ( ; it.current(); ++it )
            unregisterAuthData( it.current() );
    }
}

/********************************* SessionData ****************************/

class KIO::SessionData::SessionDataPrivate
{
public:
    SessionDataPrivate() {
        useCookie = true;
        maxCacheAge = DEFAULT_MAX_CACHE_AGE;
    }

    int maxCacheAge;
    bool useCookie;
    bool useCache;
    QString charsets;
    QString language;
    QString cacheDir;
    QString userAgent;
};

SessionData::SessionData()
{
    authData = new AuthDataList;
    d = new SessionDataPrivate;
    reset();
}

SessionData::~SessionData()
{
    delete d;
    delete authData;
    d = 0L;
    authData = 0L;
}

void KIO::SessionData::configDataFor( SlaveConfig* cfg, const QString& proto,
                                      const QString& host )
{
    if ( cfg && proto.find("http", 0, false) == 0 )
    {
        cfg->setConfigData( proto, host, "Cookies",
                            d->useCookie ? "true":"false" );

        // These might have already been set so check first
        // to make sure that we do not trumpt settings sent
        // by apps or end-user.
        if ( cfg->configData(proto,host)["Languages"].isEmpty() )
            cfg->setConfigData( proto, host, "Languages", d->language );
        if ( cfg->configData(proto,host)["Charsets"].isEmpty() )
            cfg->setConfigData( proto, host, "Charsets", d->charsets );
        if ( cfg->configData(proto,host)["UseCache"].isEmpty() )
            cfg->setConfigData( proto, host, "UseCache",
                                d->useCache ? "true":"false" );
        if ( cfg->configData(proto,host)["CacheDir"].isEmpty() )
            cfg->setConfigData( proto, host, "CacheDir", d->cacheDir );
        if ( cfg->configData(proto,host)["MaxCacheAge"].isEmpty() )
            cfg->setConfigData( proto, host, "MaxCacheAge",
                                QString::number(d->maxCacheAge) );
        if ( cfg->configData(proto,host)["UserAgent"].isEmpty() )
        {
            if ( d->userAgent.isEmpty() )
            {
                QString keys = cfg->configData(proto,host)["UserAgentKeys"];
                if ( keys.isEmpty() )
                    keys = DEFAULT_USER_AGENT_KEYS;
                d->userAgent = KProtocolManager::defaultUserAgent( keys );
            }
            cfg->setConfigData( proto, host, "UserAgent", d->userAgent );
        }
    }
}

void KIO::SessionData::reset()
{
    // Get Cookie settings...
    KConfig* cfg = new KConfig("kcookiejarrc", false, false);
    cfg->setGroup( "Cookie Policy" );
    d->useCookie = cfg->readBoolEntry( "Cookies", true );
    delete cfg;

    // Get language settings...
    QStringList languageList = KGlobal::locale()->languageList();
    QStringList::Iterator it = languageList.find( QString::fromLatin1("C") );
    if ( it != languageList.end() )
    {
        if ( languageList.contains( QString::fromLatin1("en") ) > 0 )
          languageList.remove( it );
        else
          (*it) = QString::fromLatin1("en");
    }
    d->language = languageList.join( ", " );

    // Get charset settings...
    // FIXME: Ugly hack to get appropriate charset value.  Needs
    // to be simplified when it gets fixed in QT/kdecore.
    d->charsets = KGlobal::charsets()->name(KGlobal::charsets()->xNameToID(KGlobal::locale()->charset()));

    // Get cache settings...
    KProtocolManager::reparseConfiguration();
    d->useCache = KProtocolManager::useCache();
    d->cacheDir = KProtocolManager::cacheDir();
    d->maxCacheAge = KProtocolManager::maxCacheAge();
    d->userAgent = QString::null;
}

void KIO::SessionData::slotAuthData( const QCString& key, const QCString& gkey,
                                     bool keep )
{
    authData->addData( new KIO::SessionData::AuthData(key, gkey, keep) );
}

void SessionData::slotDelAuthData( const QCString& gkey )
{
    authData->removeData( gkey );
}

#include "sessiondata.moc"
