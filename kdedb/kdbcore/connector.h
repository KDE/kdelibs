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
#ifndef CONNECTOR_H
#define CONNECTOR_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>
#include <qstringlist.h>

#include "exception.h"
#include "object.h"
#include "kdb.h"

namespace KDB {

class Exception;
class Handler;
class Table;

/**
 * @short base interaction class for plugins.
 *
 * This is the base class for the interaction between kdbcore and
 * the various plugins. It should be subclassed by plugins, but should
 * not be used by the client programs
 *
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @author Michael Koch  <m_kock@bigfoot.de>
 * @version kdbcore 0.0.2
 */
 
class Connector :public Object {

 public:
    Connector();
    Connector(const Connector& c);
    virtual ~Connector();

    // virtual methods: must be overridden in the subclass

    /**
     * closes the connection to the database backend
     */
    virtual void close();

    /**
     * opens the connection to the database backend
     */
    virtual bool connect() = 0;

    /**
     * returns the list of the available databases
     * for the current open connection
     */
    virtual QStringList databases() = 0;

    /**
     * returns the list of the available tables
     * for the current open database
     */
    virtual QStringList tables() = 0;

    /**
     * returns the list of the fields for the given table
     * for the current open database. The rows must contain:
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
     * the only mandatory fields are field name, datatype and size. All
     * other values can be present if the DBMS is able to give them
     */
    virtual RowList fields(const QString & tableName) = 0;

    /**
     * executes an SQL statement. The return value is the number of
     * rows affected by the statement
     */
    virtual KDB_ULONG execute(const QString &sql) = 0;

    /**
     * creates an empty database
     */
    virtual bool createDatabase(const QString & name) = 0;

    /**
     * drops a database with all its content
     */
    virtual bool dropDatabase(const QString & name) = 0;

    /**
     * creates a table with the characteristic of the given one
     */
    virtual bool createTable(const Table & tab) = 0;

    /**
     * remove a table in the currend database deleting all its content
     */
    virtual bool dropTable(const QString & name) = 0;

    /**
     * clones the current object. If the object holds an open connection,
     * the resulting connector must be open too.
     */
    virtual Connector * clone() = 0;

    /**
     * set the operational database for the current connector. From this
     * point all the methods will affect the selected database
     */
    virtual bool setCurrentDatabase(const QString &name) = 0;

    /**
     * returns the corresponding KDB datatype for the given native one
     */
    virtual DataType nativeToKDB(const QString &type) = 0;

    /**
     *  returns the corresponding native datatype for the given KDB one
     */
    virtual QString KDBToNative(DataType type) = 0;

    /**
     * executes a select to the backend and returns an handler to the
     * result
     */
    virtual Handler *query(const QString &SQL) = 0;

    /**
     * start a transaction for the current connection
     * an error is raised if the underlying DBMS does not support
     * transactions
     */
    virtual void beginTransaction() = 0;

    /**
     * end the current transaction and save all the changes to the
     * database.
     * an error is raised if the underlying DBMS does not support
     * transactions or if there isn't any transaction started
     */
    virtual void commit() = 0;

    /**
     * end the current transaction and drop all changes made up to now.
     * an error is raised if the underlying DBMS does not support
     * transactions or if there isn't any transaction started
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
     * must be called by subclasses when a connection to the backend
     * is established
     */
    void setConnected(bool conn);

 private:

    QString _host;
    int _port;
    QString _user;
    QString _pwd;

    bool _connected;

};

};
#endif



