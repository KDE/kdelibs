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

#include <qptrlist.h>
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

/************************* SessionData::AuthDataList ****************************/
class SessionData::AuthDataList : public QPtrList<SessionData::AuthData>
{
public:
    AuthDataList();
    ~AuthDataList();

    void addData( SessionData::AuthData* );
    void removeData( const QCString& );

    bool pingCacheDaemon();
    void registerAuthData( SessionData::AuthData* );
    void unregisterAuthData( SessionData::AuthData* );
    void purgeCachedData();
    
private:
    KDEsuClient * m_kdesuClient;    
};

SessionData::AuthDataList::AuthDataList()
{
    m_kdesuClient = new KDEsuClient;
    setAutoDelete(true);    
}

SessionData::AuthDataList::~AuthDataList()
{
    purgeCachedData();
    delete m_kdesuClient;
    m_kdesuClient = 0;
}

void SessionData::AuthDataList::addData( SessionData::AuthData* d )
{
    QPtrListIterator<SessionData::AuthData> it ( *this );
    for ( ; it.current(); ++it )
    {
        if ( it.current()->isKeyMatch( d->key ) )
            return;
    }
    registerAuthData( d );
    append( d );
}

void SessionData::AuthDataList::removeData( const QCString& gkey )
{
    QPtrListIterator<SessionData::AuthData> it( *this );
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
    Q_ASSERT(m_kdesuClient);
    
    int sucess = m_kdesuClient->ping();
    if( sucess == -1 )
    {
        sucess = m_kdesuClient->startServer();
        if( sucess == -1 )
            return false;
    }
    return true;
}

void SessionData::AuthDataList::registerAuthData( SessionData::AuthData* d )
{
    if( !pingCacheDaemon() )
        return;

    bool ok;
    QCString ref_key = d->key + "-refcount";
    int count = m_kdesuClient->getVar(ref_key).toInt( &ok );
    if( ok )
    {
        QCString val;
        val.setNum( count+1 );
        m_kdesuClient->setVar( ref_key, val, 0, d->group );
    }
    else
        m_kdesuClient->setVar( ref_key, "1", 0, d->group );
}

void SessionData::AuthDataList::unregisterAuthData( SessionData::AuthData* d )
{
    if ( !d  || d->persist )
        return;

    bool ok;
    int count;
    QCString ref_key = d->key + "-refcount";

    count = m_kdesuClient->getVar( ref_key ).toInt( &ok );
    if ( ok )
    {
        if ( count > 1 )
        {
            QCString val;
            val.setNum(count-1);
            m_kdesuClient->setVar( ref_key, val, 0, d->group );
        }
        else
        {
            m_kdesuClient->delVars(d->key);
        }
    }
}

void SessionData::AuthDataList::purgeCachedData()
{
    if ( !isEmpty() && pingCacheDaemon() )
    {
        QPtrListIterator<SessionData::AuthData> it( *this );
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
    authData = new AuthDataList;
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
            configData["UserAgent"] = KProtocolManager::defaultUserAgent();
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

    // Get language settings...
    QStringList languageList = KGlobal::locale()->languagesTwoAlpha();
    QStringList::Iterator it = languageList.find( QString::fromLatin1("C") );
    if ( it != languageList.end() )
    {
        if ( languageList.contains( QString::fromLatin1("en") ) > 0 )
          languageList.remove( it );
        else
          (*it) = QString::fromLatin1("en");
    }
    d->language = languageList.join( ", " );
    
    d->charsets = QTextCodec::codecForLocale()->mimeName();
    KProtocolManager::reparseConfiguration();
}

void SessionData::slotAuthData( const QCString& key, const QCString& gkey,
                                     bool keep )
{
    authData->addData( new SessionData::AuthData(key, gkey, keep) );
}

void SessionData::slotDelAuthData( const QCString& gkey )
{
    authData->removeData( gkey );
}

void SessionData::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

}
