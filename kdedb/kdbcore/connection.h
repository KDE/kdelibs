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
#ifndef CONNECTION_H
#define CONNECTION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>
#include <qlist.h>

#include "object.h"
#include "database.h"

class KConfigBase;

namespace KDB {

class Connector;
class Connection;
typedef QList<Connection>         ConnectionList;
typedef QListIterator<Connection> ConnectionIterator;

/**
 * Manages the connection state to a SQL engine.
 *
 * Basically stores all authentication tokens and
 * opens/closes connection as needed.
 *
 * It is also a collection of database objects, allowing
 * @li traversal of all databases (@ref DatabaseIterator and @ref databases )
 * @li retrieval by name (@ref findDatabase )
 * @li creation of new databases (@ref createDatabase )
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @author Michael Koch  <m_kock@bigfoot.de>
 * @version kdbcore 0.0.2
 */

class Connection : public Object {
    friend class DBEngine;

    Q_OBJECT

 public:

    virtual ~Connection();

    /**
     * Connect to the host. If username and password are missing,
     * pops up a dialog to ask them. If the connection is open, nothing
     * is actually done. Returns true if the connection has been succesfully
     * opened, false otherwise.
     */
    bool open() ;

    /**
     * Disconnects from the host. all databases are closed. unless destructive == false,
     * the connection is deleted right after the call
     */
    void close(bool destructive = true);

    /**
     * Returns true if a successful connection has been established.
     */
    bool isConnected();

    /**
     * Returns true if a the connection will be saved in config files
     */
    bool saveStatus();

    /**
     * sets the save status for this connection. If the save status is true,
     * the connection will be saved in kdbrc
     */
    void setSaveStatus(bool status);

    
    /**
     * Sets the user for this connection. The change will not take
     * place until a disconnect/connect is made.
     */
    void setUser(const QString &name);

    /**
     * Returns the user for this connection.
     */
    QString user();

    /**
     * Returns the plugin name of this connection.
     */
    QString plugin();
    
    /**
     * Sets the password for this connection. The change will not take
     * place until a disconnect/connect is made. Passing QString::null
     * will force the connection to ask for the password next time
     * @ref open is called.
     */
    void setPassword(const QString &pwd, bool save = false);

    /**
     * Returns the password for this connection.
     */
    QString password();
    
    /**
     * Returns the host name for this connection.
     */
    QString host();

    /**
     * Returns the port number for this connection.
     */
    int port();

    /**
     * Returns a complete description of the connection.
     */
    QString prettyPrint();

    /**
     * Open a database by name.
     */
    Database * openDatabase(const QString &name);

    /**
     * Return a database by name. If no database with this name exists
     * it returns 0L.
     */
    Database * findDatabase(const QString &name);

    /**
     * Creates a new database in the current connection.
     */
    Database * createDatabase(const QString &name) ;

    /**
     * Drops a database 
     */
    bool dropDatabase(const QString &name) ;

    /**
     * Return a list of all available databases.
     */
    DatabaseList databases();

    /**
     * Return an iterator that points to the first database.
     */
    DatabaseIterator begin();

    /**
     * saves this connection to the given config file, with an
     * optional connection number. The config file must exists
     * and must refer to the correct section.
     */
    void saveToConfig( KConfigBase *config, int number = -1);

 public slots:
    void slotDeleteYourself();
        
 signals:

    void closing(KDB::Connection *);
    void opened();
    void databaseAdded( QString );
    void databaseRemoved( QString );
    
 private:

    Connector *connector;
    DatabaseList m_databases;

    QString m_plugin;
    QString m_host;
    int m_port;
    QString m_user;
    QString m_pwd;
    bool m_savePwd;
    bool m_saveStatus;
    
    Connection(const QString &pluginName, const QString &host, int port, QObject *parent = 0);

};

}

#endif








