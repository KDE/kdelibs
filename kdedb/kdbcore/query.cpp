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
#include "query.h"
#include "database.h"
#include "table.h"
#include "connector.h"
#include "recordset.h"

#include <kdebug.h>

#include "query.moc"

using namespace KDB;

Query::Query( Connector * conn, QObject *parent, const char *name, const QString &sql )
    :DataObject( parent, name ), m_SQL(sql), connector(conn->clone())
{
    kdDebug(20000) << "Query::Query" << endl;
    Database * base = static_cast<Database *>(parent);
    TablePtr tab = base->getTable(QUERYTABLE);
    if (tab) {
        RecordsetPtr set = tab->openRecordset();
        RecordsetIterator iter = set->begin();
        RecordPtr rec = iter.findFirst("Name",name);
        if (rec) {
            // load the query def here
            ;
        }
    }
}

Query::~Query()
{
    kdDebug(20000) << "Query::~Query" << endl;
}


void 
Query::addField(const QString &table, const QString &name)
{
    kdDebug(20000) << "Query::addField" << " table=" << table << " name=" << name << endl;
    // add the table name to the table list, if not already threre
    addTable(table);

    //build a fully qualified name for the field ("tablename.fieldname")
    QString full = QString("%1.%2").arg(table).arg(name);
    if ( !m_fields.contains(full) ) {
        m_fields.append(full);
    }
}

void 
Query::addTable(const QString &name)
{
    kdDebug(20000) << "Query::addTable" << " name=" << name << endl;
    if ( !m_tables.contains(name) ) {
        m_tables.append(name);
    }
}

void 
Query::addCondition(const QString &condition)
{
    kdDebug(20000) << "Query::addCondition" << " condition=" << condition << endl;
    // TODO: it is way too simplicistic. it assumes the user knows what
    // it is doing.

    m_conditions.append(condition);
}

QString 
Query::SQL()
{
    kdDebug(20000) << "Query::SQL" << endl;
    if ( m_SQL.isNull() ) {
        buildSQL();
    }
    return m_SQL;
}

bool 
Query::isDesign()
{
    kdDebug(20000) << "Query::isDesign" << endl;
    return m_SQL.isNull();
}

void
Query::buildSQL()
{
    kdDebug(20000) << "Query::buildSQL" << endl;
    bool first = true;
    QString tmpSQL;
    // build the SQL String here

    // create the field list
    for (QStringList::Iterator it = m_fields.begin(); it != m_fields.end(); ++it) {
        if (first) {
            tmpSQL += "Select ";
            first = false;
        } else {
            tmpSQL += ",";
        }
        tmpSQL += *it;
        tmpSQL += "\n";
    }

    first = true;

    // add the table list
    for (QStringList::Iterator it = m_tables.begin(); it != m_tables.end(); ++it) {
        if (first) {
            tmpSQL += "from ";
            first = false;
        } else {
            tmpSQL += ",";
        }
        tmpSQL += *it;
        tmpSQL += "\n";
    }

    // add the condition list
    for (QStringList::Iterator it = m_conditions.begin(); it != m_conditions.end(); ++it) {
        if (first) {
            tmpSQL += "where ";
            first = false;
        }
        tmpSQL += *it;
        tmpSQL += "\n";
    }

    m_SQL = tmpSQL;

    //TODO: save the SQL into the __KDBQueries table.
    // since we can get here only for newly created queries, let's insert into
    // the table.

    // What about a XML definition of the query? it will help successive
    // enhancements and will allow us to save a query definition outside
    // the database.

    kdDebug(20000) << "resulted SQL: " << m_SQL << endl;
}


RecordsetPtr
Query::openRecordset()
{
    kdDebug(20000) << "Query::openRecordset" << endl;
    RecordsetPtr r = new Recordset(connector, SQL(), this);
    if (error())
        return 0L;
    else
        return r;
    
}

