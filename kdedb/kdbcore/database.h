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

#ifndef KDB_DATABASE_H
#define KDB_DATABASE_H "$Id$"

#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qvaluelist.h>

#include <ksharedptr.h>

#include <kdb/table.h>
#include <kdb/query.h>
#include <kdb/recordset.h>
#include <kdb/object.h>

namespace KDB {

class Connector;

class Database;
class DatabasePrivate;
 
typedef QList<Database>         DatabaseList;
typedef QListIterator<Database> DatabaseIterator;

/**
 * Central access point to all functionality of the single database.
 *
 * That means it provides a list of all queries and tables, and is able to
 * execute a command query and to return a recordset for a given query.
 *
 * Tables and queries can be accessed by name, and the class can provide
 * collection objects for both.
 *
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @version kdbcore 0.0.2
 */
class Database : public Object{

    friend class Connection;

    Q_OBJECT

 public:

    Database(const Database & base);
    
    virtual ~Database();

    /**
     * Creates a new table for the current database.  Please note that the
     * table is not yet part of the database until you call @ref
     * KDB::Table::create
     */
    TablePtr newTable(const QString &name);

    /**
     * Returns an existing table by name, or 0L if the table does not exists.
     */
    TablePtr getTable(const QString &name);

    /**
     * Return the list of available tables.  Tables whose name begins with
     * "__" ( so called system tables) will be returned only if system =
     * true.
     */
    TableList tables(bool system = false);

    /**
     * Return a list of names of available tables.  Tables whose name begins
     * with "__" ( so called system tables) will be returned only if system
     * = true.
     */
    QStringList tableNames(bool system = false);
    
    /**
     * Removes a table from the database. This means that the table is
     * dropped, and all the content is erased.
     */
     bool removeTable(const QString &name);

    /**
     * Creates a new query
     *
     * @param name this is the name of the query
     *
     * @param SQL this is the sql executed by the query. if it is not given,
     * the query can be built with addTable, addField etc.
     */
    QueryPtr newQuery(const QString &name, const QString &SQL = QString::null);

    /**
     * Returns an existing query by name, or 0L if the query does not exists.
     */
    QueryPtr getQuery(const QString &name);
    
    /**
     * Return the list of available queries.
     */
    QueryList queries();

    /**
     * Return the list of names of available queries.
     */
    QStringList queryNames();

    /**
     * Removes a query from the database. 
     */
    bool removeQuery(const QString &name);

    /**
     * Creates a recordset based on a given query.  If the query fails, 0L
     * is returned. You can then check @ref errorMessage() to get a specific
     * error message.
     */
    RecordsetPtr openRecordset(const QString &SQL);

    /**
     * Exec a command query. Returns the number of records affected.
     */
    KDB_ULONG execute(const QString &SQL);
    
    bool open();

    void close();

    bool isOpen();


 signals:

    void tableAdded( QString );
    void tableRemoved( QString );
    void queryAdded( QString );
    void queryRemoved( QString );
    void changed();
    void closed();
    void opened();

 protected slots:

    void tableCreated( KDB::Table * );

 private:

    Connector * connector;
    QStringList m_tables;
    QStringList m_queries;

    bool m_open;

    Database(Connector *conn, QObject *parent, const char *name );

    void createQueryTable();

    DatabasePrivate *d;
    
};

}

#endif
