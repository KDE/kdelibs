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
#ifndef DBENGINE_H
#define DBENGINE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qlist.h>
#include <qstringlist.h>

#include <ktrader.h>

#include "object.h"
#include "plugin.h"
#include "connection.h"

class KConfigBase;

namespace KDB {

class Database;

/**
 * Entry point for the library.
 *
 * This object manages all available plugins, and can create connections to
 * servers or databases.
 *
 * In more detail:
 * @li On first reference, loads all plugins registered in $KDEDIR/share/services.
 * @li Allows iterations through all available plugins.
 * @li Provide functions to directly access connections and databases through @ref openDatabase
 * and @ref openConnection
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @author Michael Koch  <m_kock@bigfoot.de>
 * @version kdbcore 0.0.2
 */

class DBEngine : public Object{

    Q_OBJECT

 public:
    virtual ~DBEngine();

    /**
     * creates, if needed, the engine object, and returns it
     */
    static DBEngine *self();

    /**
     * Returns an iterator that points to the first plugin.
     */
    PluginIterator beginPlugins();

    /**
     * Find a plugin by name. If no plugin is found, OL is returned.
     */
    Plugin * findPlugin(const QString &name);

    /**
     * Returns the names of all detected plugins
     */
    QStringList pluginNames();

    /**
     * Open a connection to the given host, with the given plugin, username
     * and password. If the plugin does not exists, OL is returned. This is
     * a convenience function, to avoid passing through a plugin to get a connection
     */
    Connection * openConnection(const QString &pluginName, const QString &host, int port,
                                const QString &user, const QString &password = QString::null);

    /**
     * open a connection by name. If there is no such connection, 0L is returned. This differs
     * from the other openConnection call, that builds one on the fly 
     */
    Connection * openConnection(const QString &name);

    /**
     * Creates a connection object, without acually opening it
     */
    virtual Connection * addConnection (const QString &pluginName,
                                        const QString &host,
                                        int port = 0,
                                        const QString &user = QString::null,
                                        const QString &password = QString::null);

    /**
     * Removes a connection from the list of connections.
     * This will delete the connection, so make sure to not have
     * dangling reference to the connection.
     */
    void remove(Connection *);

    /**
     * Return a connection to the specified host for the given user.
     * If the returned value is 0L no connection with the specified
     * arguments is found.
     */
    Connection * findConnection(const QString &plugin,
                                const QString &host,
                                const int port,
                                const QString &user);
    /**
     * return a connection with the specified name
     */
    Connection * findConnection(const QString &name);

    /**
     * Returns an iterator that points to the first Connection
     * object.
     */
    ConnectionIterator beginConnections();

    /**
     * Load all connections saved in the KConfig-object config.
     * If config is 0L the connections will be load from the
     * standard file kdbrc.
     */
    void loadConnections(KConfigBase *config = 0L);

    /**
     * Save all connections to the KConfig-object config.
     * If config is 0L the connections will be saved into the
     * standard file kdbrc.
     */
    void saveConnections(KConfigBase *config = 0L);

    /**
     * Open a database. This is a convenience function.
     */
    Database * openDatabase(const QString &pluginName, const QString &host, int port,
                            const QString &user, const QString &password,
                            const QString &databaseName);

    /**
     * retrieves the engine config object. this is either the general kde-db config
     * file kdbrc or a user supplied one, set using setConfig()
     */
    KConfigBase * config();

    /**
     * sets the config file for the current session.
     * any unsaved change to the previous config file will be lost
     */
    void setConfig(KConfigBase* config);

 signals:

    void connectionAdded(KDB::Connection *);
    void connectionRemoved(KDB::Connection *);
        
 protected:

    /**
     * Loads a plugin by its name if it is not already loaded.
     */
    bool loadPlugin(const QString&);
    
 protected slots:

    void slotConnectionClosed(KDB::Connection *);
 
 private:

    explicit DBEngine();
    static DBEngine * m_engine;

    KTrader::OfferList m_pluginData;
    PluginList  m_plugins;

    ConnectionList m_connections;
    KConfigBase *m_config;
};

}
#define DBENGINE KDB::DBEngine::self()

#endif
