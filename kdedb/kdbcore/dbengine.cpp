/*
   This file is part of the KDB libraries
   Copyright (c) 2000 Praduroux Alessandro <pradu@thekompany.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <iostream.h>

#include <qtimer.h>

#include <kdebug.h>
#include <ktrader.h>
#include <kconfig.h>
#include <kstddirs.h>
#include <klibloader.h>
#include <kstaticdeleter.h>

#include <kdb/dbengine.h>
#include <kdb/plugin.h>
#include <kdb/database.h>

#include "dbengine.moc"

using namespace KDB;

static KStaticDeleter<KDB::DBEngine> ksd_dbengine;

DBEngine *DBEngine::m_engine = 0L;


DBEngine::DBEngine()
    :Object(0L,"DBEngine"),
     m_config(0)
{
    //kdDebug(20010) << "DBEngine::DBEngine" << endl;

    // get offered plugins
    m_pluginData = KTrader::self()->query( "KDB/DBPlugin" );
}

DBEngine::~DBEngine()
{
#ifndef NDEBUG
    cerr << "KDB (DBEngine): DBEngine::~DBEngine" << endl;
#endif
#if 0
    printPendingObjects();
#endif
    if (m_config)
        delete m_config;
}

DBEngine *
DBEngine::self()
{
    if (!m_engine) {
        m_engine = ksd_dbengine.setObject( new DBEngine );
    }

    return m_engine;
}


PluginIterator
DBEngine::beginPlugins()
{
    // if the user request to traverse all pluging, let him
    // do it even if this means loading all plugins
    KTrader::OfferList::Iterator it;
    for( it = m_pluginData.begin(); it != m_pluginData.end(); ++it )
        loadPlugin( (*it)->name() );    
    
    PluginIterator iter(m_plugins);
    return iter;
}


Plugin *
DBEngine::findPlugin(const QString& name)
{
    //kdDebug(20010) << "DBEngine::findPlugin" << " name=" << name << endl;
    loadPlugin(name);
	
    PluginIterator it = beginPlugins();

    while ( it.current() ) {
        if ( it.current()->name() == name )
            break;
        ++it;
    }

    return it.current();
}

QStringList
DBEngine::pluginNames()
{
    QStringList pluginNames;

    KTrader::OfferList::Iterator it;
    for( it = m_pluginData.begin(); it != m_pluginData.end(); ++it )
        pluginNames.append( (*it)->name() );
    
    return pluginNames;
}

Connection *
DBEngine::openConnection(const QString &name)
{
    Connection * conn = 0L;

    // empty the error queue
    clearErrors();
    conn = findConnection(name);

    if ( conn )
        conn->open();

    if ( error() )
        conn = 0L;

    return conn;
    
}

Connection *
DBEngine::openConnection(const QString &pluginName,
                         const QString &host,
                         int port,
                         const QString &user,
                         const QString &password)
{
    //kdDebug(20010) << "DBEngine::openConnection" << " pluginName=" << pluginName << " host=" << host << " port=" << port << " user=" << user << " password=" << password << endl;
    Connection * conn = findConnection(pluginName, host, port, user);
    
    if ( !conn ) {
        conn = addConnection(pluginName, host, port, user, password);
    }

    if ( conn && !conn->isConnected()) {
        conn->open();

        if (error()) {
            remove(conn);
            conn = 0L;
        }
    }

    return conn;
}

Connection *
DBEngine::addConnection(const QString &plugin,
                        const QString &host,
                        int port,
                        const QString &user ,
                        const QString &password )
{
    //kdDebug(20010) << "DBEngine::addConnection" << " plugin=" << plugin << " host=" << host << " port=" << port << " user=" << user << " password=" << password << endl;
    Connection *conn = findConnection(plugin, host, port, user);

    // we do not need that until the connection is opened
    // loadPlugin( plugin );
    
    if ( conn ) {
        conn->setPassword(password);
        return conn;
    } else {    
        conn = new Connection(plugin, host, port, this);
    }
    
    if (conn) {
        m_connections.append(conn);
        connect(conn,
                SIGNAL(closing(KDB::Connection *)),
                SLOT(slotConnectionClosed(KDB::Connection *)));
        conn->setUser(user);
        conn->setPassword(password);
    }

    emit connectionAdded(conn);
    return conn;
}

void
DBEngine::remove(Connection * conn)
{
    // close() the connection, that in turn emit closing(). in slotConnectionClose it will
    // be deleted
    conn->close();
    emit connectionRemoved(conn);
}

Connection *
DBEngine::findConnection(const QString & name)
{
    //kdDebug(20010) << "DBEngine::findConnection" << " name=" << name << endl;
    ConnectionIterator it = beginConnections();

    while ( it.current() ) {
        if ( it.current()->name() == name ) {
            //kdDebug(20010) << "found connection " << it.current()->name() << endl;
            break;
        }
        ++it;
    }

    // we've said no error in findXXX.
    //if (!it.current())
    //    pushError( new ObjectNotFound(QString("no connection named %1 found").arg(name)));

    return it.current();
    
}

Connection *
DBEngine::findConnection(const QString & plugin,
                         const QString& host,
			 const int port,
                         const QString &user)
{
    //kdDebug(20010) << "DBEngine::findConnection" << " plugin=" << plugin << " host=" << host << " port=" << port << " user=" << user << endl;
    ConnectionIterator it = beginConnections();

    while ( it.current() ) {
        if ( it.current()->plugin() == plugin &&
             it.current()->host() == host &&
	     it.current()->port() == port &&
             it.current()->user() == user ) {
            //kdDebug(20010) << "found host " << it.current()->host() << endl;
            break;
        }
        ++it;
    }

    return it.current();
}

ConnectionIterator
DBEngine::beginConnections()
{
    ConnectionIterator it(m_connections);
    return it;
}

void
DBEngine::loadConnections(KConfigBase *conf)
{
    //kdDebug(20010) <<  locateLocal( "config", "kdbrc" ) << endl;

    if( !conf ) {
        conf = config();
    }

    KConfigGroupSaver saver( conf, "KDB connections" );

    //kdDebug(20010) << config->group() << endl;

    unsigned int maxConn = conf->readUnsignedNumEntry( "NumberOfConnections", 0 );

    kdDebug(20010) << "Number of connections: " << maxConn << endl;
    
    for( unsigned int count = 0; count < maxConn; count++ ) {
        kdDebug(20010) << "Loading connection number  " << count << endl;

        QString prefix, plugin, host, user, password, name;
        int port;

        prefix = QString( "%1_" ).arg( count );
    
        plugin = conf->readEntry( prefix + "Plugin", "MySQL" );
        host = conf->readEntry( prefix + "Host", "localhost" );
        port = conf->readNumEntry( prefix + "Port", 0 );
        user = conf->readEntry( prefix + "User", QString::null );
        password = conf->readEntry( prefix + "Password", QString::null );
        name = conf->readEntry( prefix + "Name", QString::null );
        
        Connection * conn = addConnection(plugin, host, port, user, password);
        conn->setName(name.utf8());
        
        kdDebug(20010) << "Loaded connection: "<< conn->prettyPrint() << endl;
    }

}

void
DBEngine::saveConnections(KConfigBase *conf)
{
    unsigned int count = 0;
    
    //kdDebug(20010) <<  locateLocal( "config", "kdbrc" ) << endl;

    if( !conf ) {
        conf = config();
    }

    KConfigGroupSaver( conf, "KDB connections" );

    // ??? seems that the above call does not set it correctly ???
    conf->setGroup( "KDB connections" );

    //kdDebug(20010) << config->group() << endl;
    
    for( ConnectionIterator it = beginConnections(); it != 0L ; ++it )
        {
            (*it)->saveToConfig(conf, count);
            count++;
        }

    conf->writeEntry( "NumberOfConnections", count );
    conf->sync();
}

Database *
DBEngine::openDatabase(const QString &pluginName,
                       const QString &host,
                       int port,
                       const QString &user,
                       const QString &password,
                       const QString &databaseName)
{
    //kdDebug(20010) << "DBEngine::openDatabase" << " pluginName=" << pluginName << " host=" << host << " port=" << port << " user=" << user << " password=" << password << " databaseName=" << databaseName << endl;
    Connection * conn = openConnection(pluginName, host, port, user, password);
    Database * db = 0L;

    if (conn) {
        db = conn->openDatabase(databaseName);
    }

    return db;
}

bool
DBEngine::loadPlugin(const QString& name)
{
    //kdDebug(20010) << "DBEngine::loadPlugin" << " name=" << name << endl;
    Plugin * plugin;
     
    // test if plugin is already loaded
    plugin = m_plugins.first();

    while( plugin ) {
        if( plugin->name() == name )
            break;
        plugin = m_plugins.next();
    }

    // when plugin is not loaded yet test if it is known
    if( !plugin ) {
        KTrader::OfferList::Iterator it;
	
	// search plugin in offerlist
	it = m_pluginData.begin();

        while( it != m_pluginData.end() ) {
            if( (*it)->name() == name )
                break;
            ++it;
        }

	// when plugin found in offerlist load it
        if( it != m_pluginData.end() ) {
            plugin = static_cast<Plugin *>( KLibLoader::self()->factory( (*it)->library().utf8() )->create() );
            m_plugins.append(plugin);
        }

        // TODO: do correct error handling
    }

    return plugin != 0L;
}

void
DBEngine::slotConnectionClosed(Connection * conn)
{
    kdDebug(20000) << k_funcinfo << conn->name() << " " << strlen(conn->name()) << endl;
    if (strlen(conn->name()) != 0) {
        // do nothing, named connections must be preserved
        return;
    }
    emit connectionRemoved(conn);
    m_connections.remove(conn);
    QTimer::singleShot(0,conn,SLOT(slotDeleteYourself()));

}

KConfigBase *
DBEngine::config()
{
    if (!m_config)
        m_config = new KConfig( locateLocal( "config", "kdbrc" ) );

    return m_config;
}

void
DBEngine::setConfig(KConfigBase *conf)
{
    if (m_config) {
        m_config->rollback();
        delete m_config;
    }
    
    m_config = conf;
}
