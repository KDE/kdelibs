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
#include "database.h"
#include "table.h"
#include "query.h"
#include "field.h"
#include "recordset.h"
#include "connector.h"
#include "handler.h"

#include "kdebug.h"

#include "database.moc"


using namespace KDB;

Database::Database(const Database &base )
    :Object( base.parent(), base.name() ),
     connector(base.connector->clone()),
     m_tables(base.m_tables),
     m_queries(base.m_queries),
     m_open(base.m_open)
{
}


Database::Database(Connector *conn, QObject *parent, const char *name )
    :Object( parent, name ), connector(conn->clone()), m_open(false)
{
}

Database::~Database()
{
    kdDebug(20000) << "Database::~Database" << endl;
    delete connector;
}


TablePtr
Database::getTable(const QString &name)
{
    kdDebug(20000) << "Database::getTable" << " name=" << name << endl;
    QStringList::Iterator it = m_tables.find(name);

    if (it == m_tables.end() ) {
        pushError(new ObjectNotFound(QString("Table %1 not found").arg(name)));
        return 0L;
    }

    return new Table(connector, this, (*it).utf8());
}

TableList
Database::tables(bool system)
{
    kdDebug(20000) << "Database::tables" << " system=" << (system ? "true" : "false" ) << endl;
    // we don't keep table pointers, so build the Table list on the fly.
    // table reference are deleted automatically
    TableList lst;
    QStringList::Iterator it = m_tables.begin();

    while ( it != m_tables.end() ) {
        if ((*it).startsWith("__") ) {
            if (system)
                lst << TablePtr(new Table(connector, this, (*it).utf8()));
        } else {
            lst << TablePtr(new Table(connector, this, (*it).utf8()));
        }
        ++it;
    }

    return lst;
}

QStringList
Database::tableNames(bool system)
{
    QStringList result = m_tables;

    if( !system ) {
        QStringList::Iterator it;

        for( it = m_tables.begin() ; it != m_tables.end() ; ++it ) {
            kdDebug(20000) << *it << endl;
            if( (*it).startsWith("__") ) {
                result.remove( it );
	    }
	}
    }

    return result;
}

QueryPtr
Database::getQuery(const QString &name)
{
    QStringList::Iterator it = m_queries.find(name);

    if (it == m_queries.end() ) {
        pushError(new ObjectNotFound(QString("Query %1 not found").arg(name)));
        return 0L;
    }

    return new Query(connector, this, (*it).utf8());
}

QueryList
Database::queries()
{
    // TODO: think: are ther system queries beginning with "__" ?
    
    QueryList lst;
    QStringList::Iterator it = m_queries.begin();

    while ( it != m_queries.end() ) {
        if (! (*it).startsWith("__") ) {
            lst << QueryPtr(new Query(connector, this, (*it).utf8()));
        }
        ++it;
    }

    return lst;
}

QStringList
Database::queryNames()
{
    // TODO: think: are ther system queries beginning with "__" ?
    
    return m_queries;
}

bool
Database::isOpen()
{
    return m_open;
}

TablePtr
Database::newTable(const QString &name) 
{
    Table * tab = new Table(connector, this, name.utf8(), true);
    connect(tab, SIGNAL(created(Table *)),SLOT(tableCreated(Table *)));
    return tab;
}

bool
Database::removeTable(const QString &name)
{
    QStringList::Iterator it = m_tables.find(name);
    if (it == m_tables.end() ) {
        pushError(new ObjectNotFound(QString("Table %1 not found").arg(name)));
        return false;
    }

    // better ask the connetor to do it
    connector->execute(QString("Drop table %1").arg(name));
    m_tables.remove(*it);
    emit changed();
    emit tableRemoved(*it);
    return true;
}


QueryPtr
Database::newQuery(const QString &name, const QString &SQL) 
{
    //modify this to behave like table creation (signal on query saving)
    Query * q = new Query(connector, this, name.utf8(), SQL);
    m_queries.append(name);
    // build the __KDBQueries table here. If they build a query, they are going
    // to save it, and it's better if we have the table then!
    emit changed();
    emit queryAdded(name);
    createQueryTable();

    return q;
}


bool
Database::removeQuery(const QString &name) 
{
    QStringList::Iterator it = m_queries.find(name);
    if (it == m_queries.end() ) {
        pushError(new ObjectNotFound(QString("Query %1 not found").arg(name)));
        return false;
    }

    // should be changed to use Record.delete() which in turn will use the plugin to perform
    // the deletion
    connector->execute(QString("Delete from __KDBQueries where name = '%1'").arg(name));
    m_queries.remove(it);
    emit changed();
    emit queryRemoved(*it);
    return true;
}


RecordsetPtr
Database::openRecordset(const QString &SQL)
{
    return new Recordset(connector, SQL, this);
}


KDB_ULONG
Database::execute(const QString &SQL) 
{
    kdDebug(20000) << "Database::execute" << " SQL=" << SQL << endl;
    return connector->execute(SQL);
}


bool
Database::open() 
{
    if (!isOpen()) { // open only if not already opened
        if (!connector->isConnected())
            connector->connect();

        connector->setCurrentDatabase(name());

        if (error())
            return false;
        
        m_tables.clear();
        m_tables = connector->tables();
        if (error())
            return false;
        
        if (m_tables.contains(QUERYTABLE)) {
            TablePtr queryTable = getTable(QUERYTABLE);
            if (queryTable) {
                RecordsetPtr rec = queryTable->openRecordset();
                RecordsetIterator iter = rec->begin();
                while (iter++) {
                    m_queries.append(iter->field("Name")->asString());
                }
                
            }
        }
        
        /*
          m_queries.clear();
          m_queries = connector->queries();
        */

        emit opened();
        m_open = true;
    }
    return true;
}

void
Database::close()
{
    kdDebug(20000) << "Database::close" << endl;
    if (m_open) {
        kdDebug(20000) << "was open" << endl;
        connector->close();
        m_tables.clear();
        m_queries.clear();
        m_open = false;
        emit closed();
    }
}

void
Database::tableCreated( Table *tab )
{
    // once a table has been successfully created, it can be appended to the
    // table list
    m_tables.append( tab->name() );
    emit tableAdded( tab->name() );
}

void
Database::createQueryTable()
{
    if ( m_tables.contains(QUERYTABLE) )
        return;
    TablePtr tab = newTable(QUERYTABLE);

    Field * f = tab->newField("Name");
    f->setType(CHAR);
    f->setSize(20);

    // NO!: create a primary index instead (more portable) //f->setPrimary(true);

    tab->appendField(f);

    f = tab->newField("Definition");
    f->setType(BLOB);

    tab->appendField(f);

    tab->create();

}




