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
#ifndef KDB_QUERY_H
#define KDB_QUERY_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qdict.h>

#include <ksharedptr.h>

#include "recordset.h"
#include "dataobject.h"

namespace KDB {

class Connector;
class Recordset;

class Query;
class QueryPrivate;

typedef KSharedPtr<Query>            QueryPtr;
typedef QValueList<QueryPtr>         QueryList;
typedef QValueListIterator<QueryPtr> QueryIterator;
typedef QDict<char>                  ParameterList; 

struct qryField {
    QString table;
    QString name;
    QString val;
};
 
typedef QList<qryField>         FldList;
typedef QListIterator<qryField> FldIterator;

struct qryCond {
    QString condition;
    int type;
    int level;
};

typedef QList<qryCond>         CondList;
typedef QListIterator<qryCond> CondIterator;
 
/**
 * representation of a stored query.
 *
 * Through this object we access the fields and
 * parameters of the query, if any.
 *
 * Each query is stored in the database as an XML definition. This definition
 * will contain the list of fields, calculated fields, tables, filter and join
 * expressions and parameters. One exception to that is done when the query is built
 * using an SQL expression in @ref Database::newQuery . In this case, the SQL is stored.
 *
 * Queries are of two types: select queries and command queries. Select queries can
 * be used to obtain a recordset using @ref openRecordset , while command queries are
 * executed using @ref execute . Command queries have different types: Update, Insert,
 * Delete. By default, a newly built query is a Select query. You can change the query
 * type using @ref setType
 *
 * Queries can be parametrized. While building the query, 
 * the user can use the special construct %name, and provide a
 * corresponding value using the @ref setParameter call. Before the execution (through
 * @ref openRecordset or @ref execute ), a parameter substitution is done. When there is no
 * corresponding parameter value, the %xxx keyword is removed. this can lead to incorrect
 * SQL. Parameter substitution is done on the complete SQL statement, allowing creation
 * of queries that access data with similar definition in different tables with only one
 * query definition.
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @version kdbcore 0.0.2
 */

class Query :public DataObject{

    friend class Database;
    Q_OBJECT

 public:

    /**
     * the type of query. Select queries can be executed using @ref openRecordset, while
     * command queries through @ref execute. the opposite will generate an error.
     */
    enum QueryType { Select, Insert, Update, Delete };

    /**
     * type of conditional expression.
     */
    enum ConditionType { And, Or };
 public:

    ~Query(); 

    /**
     * Sets the query type. 
     */
    void setType(QueryType type);

    /**
     * Returns the query type
     */
    QueryType type() const;

    /**
     * Return the list of known properties. This is a QDict<char> where the keys are
     * the parameter names and the items are the parameter values
     */
    ParameterList parameters() const;

    /**
     * Set the value of a parameter
     */
    void setParameter(const QString &prop, const char * value);

    /**
     * Return the actual value of a parameter
     */
    const char * parameter(const QString &prop) const;
    
    /**
     * Append a new field to the field list of the query.
     * @param table The table to which this field belongs. Ignored for command queries.
     * @param name The name of the field
     * @param value The value this field should get (useful only for update and insert queries,
     * ignored otherwhise).
     */
    void addField(const QString &table, const QString &name, const QString &value = QString::null);

    /**
     * Remove a field from the list of fields
     */
    void removeField( const QString & table, const QString &name );

    /**
     * returns the list of fields
     */
    FldList fields() const;

    /**
     * Add a table to the existing list of tables. For insert, update and delete queries
     * only the first table is taken into consideration. all others will be discarded
     * silently
     */
    void addTable(const QString &name) ;

    /**
     * Remove a table from the list of tables. This will also remove all fields for the
     * removed table
     */
    void removeTable(const QString &name);

    /**
     * Return the list of tables
     */
    QStringList tables();
    
    /**
     * Add a condition to the query. Ignored for insert queries.
     *
     * Conditions can be nested to an arbitrary level. Proper use of the level parameter can
     * lead to complex conditional expressions.
     *
     * for example:
     * <pre>
     * qry->addCondition("A = B", And, 0);
     * qry->addCondition("B = C", And, 1);
     * qry->addCondition("C = D", Or, 2);
     * qry->addCondition("E = C", And, 2);
     * qry->addCondition("F = A", And, 0);
     * </pre>
     * will lead to the following SQL condition:
     * <pre>
     * WHERE A = B
     *   AND (B = C
     *        OR (C = D
     *            AND E = C
     *           )
     *       )
     *   AND F = A
     * </pre>
     * @param condition The condition without 'where', 'and', or parenthesis. Something
     * like "table1.field1 is null" or "table1.field1 = table2.field1"
     * @param type Wether the condition should be ANDed or ORed with other same level conditions
     * @param level The nesting level of the condition
     */
    void addCondition(const QString &condition, ConditionType type = And, int level = 0);

    /**
     * Remove a condition from the query. It will be removed the first condition that matches
     */
    void removeCondition(const QString &condition, int level = 0);

    /**
     * Return a list of conditions
     */
    CondList conditions();
    
    /**
     * Return the SQL code associated to the query. The statement is computed
     * on the fly using the stored definition, and parameter parsing is performed.
     */
    QString SQL();
    
    /**
     * Executes a command query, and return the number of rows affected by this
     * query. If called on a select query, it will fail and generate an error.
     */
    KDB_ULONG execute();

    /**
     * Creates a recordset based on this query.
     * If called on a command query, it will fail and generate an error.
     */
    RecordsetPtr openRecordset();    


    /**
     * Saves the query into the database as XML definition or SQL, depending on how it is
     * created
     */
    void save();

    /**
     * Return true if the query has been modified somewhat
     */
    bool isDirty();
        
    
 signals:

    /**
     * This signal is emitted whenever the definition of the query changes.
     * That means a field or table or condition is added or removed
     */
    void definitionChanged();

    /**
     * this signal is emitted once when the query is saved the first time into
     * the database. It is used by Database to add the query name to the list of
     * database queries
     */
    void created(Query *);
    
 protected:

    /**
     * This function will create the SQL string to pass to
     * the DBMS engine. It can be overridden by special types of
     * queries ?? dunno if it will help
     */
    virtual QString buildSQL();

 private:

    Query( Connector * conn, QObject *parent = 0L, const char *name = 0L, const QString &sql = QString::null );

    QueryPrivate *d;

    Connector * connector;


};

}

#endif
