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
#ifndef PLUGIN_H
#define PLUGIN_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>
#include <qlist.h>

#include "kdb.h"
#include "object.h"
#include "connection.h"

namespace KDB {

class Capability;
class Connector;

class Plugin;
typedef QList<Plugin>         PluginList;
typedef QListIterator<Plugin> PluginIterator;

/**
 * A Plugin handles all connections related to a DBMS (Postgres, MySQL, and so on).
 *
 * This class allows traversal of the list of available connections, and can
 * be queried about general informations and implemented capabilities through @ref provides
 *
 * The Plugin is also responsible of creating "capability objects", that is
 * objects that can implement a specific capabilities.
 *
 * Actually we have found the following capabilities:
 * @li Transactions (does not have a "capability object")
 * @li Reorganization, that is the ability of 'compressing' database objects after
 * a big number of deletes (does not have a "capability object")
 * @li Stored procedures
 * @li Administration
 * @li Users and Groups (ACL)
 * @li Views
 * @li Sequences
 * @li User defined functions
 *
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @author Michael Koch  <m_kock@bigfoot.de>
 * @version kdbcore 0.0.2
 */
class Plugin: public Object {
    
    friend class Connection;
    
    Q_OBJECT

public:

    struct PluginInfo {
        QString name;
        QString description;
        QString version;
        QString author;
        QString e_mail;
        QString copyright;
    };

public:

    Plugin(QObject *parent = 0L, const char *name = 0L);
    virtual ~Plugin();

    /**
     * Returns the information about the plugin.
     */
    virtual PluginInfo info() = 0;

    /**
     * Create a connection, append it to the connection list,
     * open the connection and return it.
     */
    Connection * openConnection (const QString &host,
                                 int port = 0,
                                 const QString &user = QString::null,
                                 const QString &password = QString::null);

    /**
     * Same as connect, but does not perform the connection.
     */
    virtual Connection * addConnection (const QString &host,
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
     * If no connection is found 0L is returned.
     */
    Connection * findConnection(const QString &host, const QString & user);

    /**
     * Returns an iterator that points to the first Connection
     * object.
     */
    ConnectionIterator begin();

    /**
     * returns true if the plugin can handle a given capability.
     */
    virtual bool provides(capability cap) = 0;

    /**
     * Create an object that will handle the specific capability
     * if the plugin does not support a capability, an exception is thrown.
     */
    virtual Capability * createObject(capability cap) = 0;


 signals:

    void closing(Plugin *);

 protected slots:
        
    void slotConnectionClose(Connection *);
        
 protected:

    void registerConnection(Connection *);
                  
    // these member functions must be overridden by implementations
    virtual Connector *createConnector() = 0;

 private:

    ConnectionList m_connections;
};

};

#endif
