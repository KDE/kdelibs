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
#ifndef QUERY_H
#define QUERY_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>
#include <qstringlist.h>

#include <ksharedptr.h>

#include "recordset.h"
#include "dataobject.h"

namespace KDB {

class Connector;
class Recordset;

class Query;

typedef KSharedPtr<Query>            QueryPtr;
typedef QValueList<QueryPtr>         QueryList;
typedef QValueListIterator<QueryPtr> QueryIterator;

/**
 * representation of a stored query.
 *
 * Through this object we access the fields and
 * parameters of the query, if any.
 *
 * WARNING: this is unfinished: will gain query type (select, insert, update, delete)
 * and parameter parsing and a lot of other stuff !!!
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @author Michael Koch  <m_kock@bigfoot.de>
 * @version kdbcore 0.0.2
 */

class Query :public DataObject{

    friend class Database;
    Q_OBJECT

 public:

     ~Query();

    /**
     * append a new field to the output list of the query
     * it works only in design mode
     */
     void addField(const QString &table, const QString &name);

    /**
     * add a table to the existing output list of tables
     */
     void addTable(const QString &name) ;

    /**
     * add a condition (where clause, but without 'where') to the query
     */
     void addCondition(const QString &condition);

    /**
     * return the SQL code associated to the query.
     * if the query is in design mode, it will build with the actual
     * tables, fields and conditions
     */
     QString SQL();

    /**
     * creates a recordset based on this query.
     */
     RecordsetPtr openRecordset();

    /**
     * returns true if the query is in design mode
     */
     bool isDesign() ;
     
 protected:

    Query( Connector * conn, QObject *parent = 0L, const char *name = 0L, const QString &sql = QString::null );

    /**
     * this function will create the SQL string to pass to
     * the DBMS engine. It can be overridden by special types of
     * queries ?? dunno if it will help
     */
    virtual void buildSQL();

 private:

    QString m_SQL;

    QStringList m_fields;
    QStringList m_tables;
    QStringList m_conditions;

    Connector * connector;
};

}

#endif
