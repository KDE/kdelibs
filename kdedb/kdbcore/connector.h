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

#ifndef KDB_CONNECTOR_H
#define KDB_CONNECTOR_H "$Id$"

#include <qstring.h>
#include <qstringlist.h>

#include <kdb/exception.h>
#include <kdb/object.h>
#include <kdb/kdb.h>

namespace KDB {

class Exception;
class Handler;
class Table;
class Field;
class Query; 
class ConnectorPrivate;
/**
 * @short base interaction class for plugins.
 *
 * This is the base class for the interaction between kdbcore and the
 * various plugins. It should be subclassed by plugins, but should not be
 * used by the client programs.
 *
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @version kdbcore 0.0.2
 */
 
class Connector : public Object {

 public:
    Connector();
    Connector(const Connector& c);
    virtual ~Connector();

    // virtual methods: must be overridden in the subclass

    /**
     * Closes the connection to the database backend.
     */
    virtual void close();

    /**
     * Opens the connection to the database backend.
     */
    virtual bool connect() = 0;

    /**
     * Returns the list of the available databases for the current open
     * connection.
     */
    virtual QStringList databases() = 0;

    /**
     * Returns the list of the available tables for the current open
     * database.
     */
    virtual QStringList tables() = 0;

    /**
     * Returns the list of the fields for the given table. The rows contain:
     *
     * @li field name
     * @li datatype, native form
     * @li size, where applicable
     * @li precision, where applicable
     * @li Null constraint (Y/N)
     * @li Default value
     * @li referential constraint
     * @li comment
     *
     * The only mandatory fields are field name, datatype and size. All
     * other values can be present if the DBMS is able to give them.
     */
    virtual RowList fields(const QString & tableName) = 0;

    /**
     * Executes an SQL statement. The return value is the number of rows
     * affected by the statement.
     */
    virtual KDB_ULONG execute(const QString &sql) = 0;

    /**
     * Creates an empty database.
     */
    virtual bool createDatabase(const QString & name) = 0;

    /**
     * Drops a database with all its content.
     */
    virtual bool dropDatabase(const QString & name) = 0;

    /**
     * Creates a table with the characteristic of the given one.
     */
    virtual bool createTable(const Table & tab) = 0;

    /**
     * Remove a table in the currend database deleting all its content.
     */
    virtual bool dropTable(const QString & name) = 0;

    /**
     * Append a file to a table. The underlying DBMS may issue an "ALTER
     * TABLE" statement here.
     */
    virtual bool appendField(const QString &table, Field *f) = 0;

    /**
     * Remove a field from a table. The underlying DBMS may issue an "ALTER
     * TABLE" statement here.
     */
    virtual bool removeField(const QString &table, const QString &field) = 0;

    /**
     * Change the definition of a field of the given table. The underlying
     * DBMS may issue an "ALTER TABLE" statement here.
     */
    virtual bool changeField(const QString &table, Field *f) = 0;
    
    /**
     * Clones the current object. If the object holds an open connection,
     * the resulting connector must be open, too.
     */
    virtual Connector * clone() = 0;

    /**
     * Set the operational database for the current connector. From this
     * point all the methods will affect the selected database.
     */
    virtual bool setCurrentDatabase(const QString &name) = 0;

    /**
     * Returns the corresponding KDB datatype for the given native one.
     */
    virtual DataType nativeToKDB(const QString &type) = 0;

    /**
     *  Returns the corresponding native datatype for the given KDB one.
     */
    virtual QString KDBToNative(DataType type) = 0;

    /**
     * Executes a "SELECT" statement on the backend and returns an handler
     * to the result.
     */
    virtual Handler *query(const QString &SQL) = 0;

    /**
     * Create a query object. Plugins can override some functions of the
     * class Query to take advantage of some specific optimizations. This is
     * entirely optional, default implementations with a meaningful
     * behaviour are provided in the kdedb core library.
     */
    virtual Query *createQueryObject(QObject *parent = 0L,
                                     const char *name = 0L,
                                     const QString &sql = QString::null);

    /**
     * Start a transaction for the current connection. An error is raised if
     * the underlying DBMS does not support transactions.
     */
    virtual void beginTransaction() = 0;

    /**
     * End the current transaction and save all the changes to the database. 
     * An error is raised if the underlying DBMS does not support
     * transactions or if there isn't any transaction started.
     */
    virtual void commit() = 0;

    /**
     * End the current transaction and drop all changes made up to now.  An
     * error is raised if the underlying DBMS does not support transactions
     * or if there isn't any transaction started.
     */
    virtual void rollback() = 0;


    // nonvirtual methods
    /// @internal
    void setHost(const QString & host);
    /// @internal
    void setPort(int port);
    /// @internal
    void setUser(const QString & user); 
    /// @internal
    void setPassword(const QString & pwd);
   
    /// @internal
    QString host() const;
    /// @internal
    int port() const;
    /// @internal
    QString user() const;
    /// @internal
    QString password() const;
    /// @internal
    bool isConnected() const;

 protected:

    /**
     * Must be called by subclasses when a connection to the backend
     * is established.
     */
    void setConnected(bool conn);

 private:

    QString _host;
    int _port;
    QString _user;
    QString _pwd;

    bool _connected;

    ConnectorPrivate *d;
};

};
#endif
