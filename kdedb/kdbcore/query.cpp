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

#include <qdom.h>
#include <qregexp.h>

#include <kdebug.h>

#include "query.moc"

namespace KDB {

    
    class QueryPrivate {

    public:
        QueryPrivate()
            : type(Query::Select),
              dirty(true)
        {
            paramList.setAutoDelete(true);
            cond.setAutoDelete(true);
            fields.setAutoDelete(true);
        }

        QString sql;
        Query::QueryType type;
        ParameterList paramList;

        FldList fields;
        QStringList tables;
        CondList cond;
        bool dirty;
    };
}

using namespace KDB;



Query::Query( Connector * conn, QObject *parent, const char *name, const QString &sql )
    :DataObject( parent, name ), connector(conn->clone())
{
    //kdDebug(20000) << "Query::Query" << endl;
    d = new QueryPrivate;
    d->sql = sql;

    
    Database * base = static_cast<Database *>(parent);
    TablePtr tab = base->getTable(QUERYTABLE);
    if (tab) {
        RecordsetPtr set = tab->openRecordset();
        RecordsetIterator iter = set->begin();
        RecordPtr rec = iter.findFirst("Name",QString(name));
        if (rec) {
            // load the query def here
            QDomDocument doc;
            if (doc.setContent(rec->field("Definition")->toString())) {
                QDomElement e = doc.documentElement();
                // load type and sql
                d->type = static_cast<QueryType>(e.attribute("type").toInt());
                d->sql = e.namedItem("SQL").nodeValue();

                // parameters
                QDomNode param = e.namedItem("Parameters");
                QDomNode node = param.firstChild();
                while( !node.isNull() ) {
                    QDomElement el = node.toElement(); // try to convert the node to an element.
                    if( !el.isNull() ) { // the node was really an element.
                        setParameter(el.attribute("name"),"");
                    }
                    node = node.nextSibling();
                }

                // fields
                QDomNode flds = e.namedItem("Fields");
                node = flds.firstChild();
                while( !node.isNull() ) {
                    QDomElement el = node.toElement(); // try to convert the node to an element.
                    if( !el.isNull() ) { // the node was really an element.
                        addField(el.attribute("table"),el.attribute("name"), el.attribute("value"));
                    }
                    node = node.nextSibling();
                }

                // Tables
                QDomNode tabs = e.namedItem("Tables");
                node = tabs.firstChild();
                while( !node.isNull() ) {
                    QDomElement el = node.toElement(); // try to convert the node to an element.
                    if( !el.isNull() ) { // the node was really an element.
                        addTable(el.attribute("name"));
                    }
                    node = node.nextSibling();
                }
                
                // Conditions
                QDomNode conds = e.namedItem("Conditions");
                node = conds.firstChild();
                while( !node.isNull() ) {
                    QDomElement el = node.toElement(); // try to convert the node to an element.
                    if( !el.isNull() ) { // the node was really an element.
                        addCondition(el.attribute("condition").replace(QRegExp("\\'"),"'"),
                                     static_cast<ConditionType>(el.attribute("type").toInt()),
                                     el.attribute("level").toInt());
                    }
                    node = node.nextSibling();
                }
                
                d->dirty = false;
            }
        }
    }
}

Query::~Query()
{
    //kdDebug(20000) << "Query::~Query" << endl;
    delete d;
}


void
Query::setType (QueryType t)
{
    d->type = t;
    d->dirty = true;
}

Query::QueryType
Query::type() const
{
    return d->type;
}

ParameterList
Query::parameters() const
{
    return d->paramList;
}

void
Query::setParameter(const QString &param, const char *value)
{
    d->paramList.insert(param, value);
    d->dirty = true;
}

const char *
Query::parameter(const QString &param) const
{
    return d->paramList[param];
}

void 
Query::addField(const QString &table, const QString &name, const QString &val)
{
    kdDebug(20000) << "Query::addField" << " table=" << table << " name=" << name
                   << " value=" << val << endl;
    // add the table name to the table list, if not already threre
    addTable(table);

    // look for a field with the same name and table in the field list
    FldIterator it(d->fields);

    while (it.current()) {
        if (it.current()->table == table && it.current()->name == name) {
            it.current()->val = val;
            break;
        }
        ++it;
    }
    if (!it.current()) {
        qryField * f = new qryField;
        f->table = table;
        f->name = name;
        f->val = val;
        d->fields.append(f);
    }   
    d->dirty = true;
        
}

void
Query::removeField(const QString &table, const QString &name)
{
    FldIterator it(d->fields);

    while (it.current()) {
        if (it.current()->table == table && it.current()->name == name) {
            d->fields.removeRef(it.current());
            break;
        }
        ++it;
    }
    d->dirty = true;
    
}

FldList
Query::fields() const
{
    return d->fields;
}

void 
Query::addTable(const QString &name)
{
    kdDebug(20000) << "Query::addTable" << " name=" << name << endl;
    if ( !d->tables.contains(name) ) {
        d->tables.append(name);
    }
    d->dirty = true;
}

void
Query::removeTable( const QString &name )
{
    if (!d->tables.contains(name))
        return;

    d->tables.remove(name);

    FldIterator it(d->fields);

    while (it.current()) {
        if (it.current()->table == name)
            d->fields.removeRef(it.current());
        else //???
            ++it;
    }
    d->dirty = true;
}


void 
Query::addCondition(const QString &condition, ConditionType t, int level)
{
    kdDebug(20000) << k_funcinfo << condition << endl;

    // do not check for duplicate entries!
    qryCond *cond = new qryCond;
    cond->condition = condition;
    cond->type = static_cast<int>(t);
    cond->level = level;
    d->cond.append(cond);
    d->dirty = true;
}

void
Query::removeCondition(const QString &condition, int level)
{
    // remove the first that matches

    CondIterator it(d->cond);

    while (it.current()) {
        if (it.current()->condition == condition && it.current()->level == level) {
            d->cond.removeRef(it.current());
            break;
        }
        ++it;
    }
    d->dirty = true;
    
}

CondList
Query::conditions()
{
    return d->cond;
}


QString 
Query::SQL()
{
    kdDebug(20000) << "Query::SQL" << endl;
    if (d->sql.isNull())
        return buildSQL();
    return d->sql;
    
}

KDB_ULONG
Query::execute()
{
    if (type() == Select) {
        pushError(new InvalidRequest(this,"not a command query"));
        return 0;
    }
    return connector->execute(SQL());
}

RecordsetPtr
Query::openRecordset()
{
    if (type() != Select) {
        pushError(new InvalidRequest(this,"not a select query"));
        return 0L;
    }
    //kdDebug(20000) << "Query::openRecordset" << endl;
    RecordsetPtr r = new Recordset(connector, SQL(), this);
    if (error())
        return 0L;
    else
        return r;
    
}


void
Query::save()
{
    if (! isDirty())
        return;
    
    //warning! the query table must exist here, otherwise we don't save anything
    Database * base = static_cast<Database *>(parent());
    TablePtr tab = base->getTable(QUERYTABLE);
    if (tab) {
        
        RecordsetPtr set = tab->openRecordset();
        RecordsetIterator iter = set->begin();
        RecordPtr rec = iter.findFirst("Name",QString(name()));
        if (!rec) {
            rec = set->addRecord();
            rec->field("Name") = Value(name());
        }
        QDomDocument doc( "QueryDef" );
        doc.appendChild( doc.createProcessingInstruction( "xml",
                                                          "version=\"1.0\" encoding=\"UTF-8\"" ) );
        // create the main node and save its type
        QDomElement def = doc.createElement( "QueryDef" );
        def.setAttribute("type",static_cast<int>(d->type));
        doc.appendChild(def);

        // save the sql definition (if any)
        QDomElement sql = doc.createElement( "SQL" );
        QDomText sqltxt = doc.createTextNode(d->sql);
        sql.appendChild(sqltxt);
        def.appendChild(sql);

        // parameters
        QDomElement param = doc.createElement( "Parameters" );
        QDictIterator<char> itp( d->paramList ); // iterator for dict
        
        while ( itp.current() ) {
            QDomElement par = doc.createElement( "Parameter" );
            par.setAttribute("name",itp.current());
            param.appendChild(par);
            ++itp;
        }
        def.appendChild(param);

        // fields
        QDomElement flds = doc.createElement( "Fields" );
        FldIterator itf(d->fields);

        while (itf.current()) {
            QDomElement fld = doc.createElement( "Field" );
            fld.setAttribute("name",itf.current()->name);
            fld.setAttribute("table",itf.current()->table);
            fld.setAttribute("value",itf.current()->val);
            flds.appendChild(fld);
            ++itf;
        }
        def.appendChild(flds);
        
        // tables
        QDomElement tabs = doc.createElement( "Tables" );
        for (QStringList::Iterator itt = d->tables.begin(); itt != d->tables.end(); itt++) {
            QDomElement tab = doc.createElement( "Table" );
            tab.setAttribute("name",*itt);
            tabs.appendChild(tab);
        }
        def.appendChild(tabs);

        // conditions
        QDomElement conds = doc.createElement( "Conditions" );
        CondIterator itc(d->cond);
        
        while (itc.current()) {
            QDomElement cnd = doc.createElement( "Condition" );
            cnd.setAttribute("condition",itc.current()->condition.replace(QRegExp("'"),"\\'"));
            cnd.setAttribute("type",itc.current()->type);
            cnd.setAttribute("level",itc.current()->level);
            conds.appendChild(cnd);
            ++itc;
        }
        def.appendChild(conds);

        rec->field("Definition") = Value(doc.toString());
        rec->update();
    }

    d->dirty = false;
}

bool
Query::isDirty()
{
    return d->dirty;
}

QString
Query::buildSQL()
{
    /*
     * the SQL string is built in 3 steps:
     * first the keyword and the field list(omitted for deletes),
     * that is dependent on the query type.
     * Then the where clause (omitted for inserts)
     * Then parameter substitution is performed
     */
    
    kdDebug(20000) << "Query::buildSQL" << endl;
    bool first = true;
    QString tmpSql;

    // step one: build the field list
    switch (d->type) {
    case Select:
        {
            kdDebug(20000) << "QueryType: SELECT" << endl;
            tmpSql = "SELECT ";
            FldIterator itf(d->fields);
            
            // build field list
            while (itf.current()) {
                if (!first)
                    tmpSql += ",";
                first = false;
                tmpSql += itf.current()->table + "." + itf.current()->name + "\n";
                ++itf;
            }
            
            first = true;
            tmpSql += "FROM ";
            // add table list
            for (QStringList::Iterator itt = d->tables.begin(); itt != d->tables.end(); itt++) {
                if (!first)
                    tmpSql += ",";
                first = false;
                tmpSql += *itt + "\n";
            }
            break;
        }
    case Insert:
        {
            kdDebug(20000) << "QueryType: INSERT" << endl;
            QString tabName = *(d->tables.begin());
            tmpSql = QString("INSERT INTO %1 (").arg(tabName);

            // build field list
            FldIterator itf(d->fields);
            while (itf.current()) {
                if (itf.current()->table == tabName) {
                    if (!first)
                        tmpSql += ",";
                    first = false;
                    tmpSql += itf.current()->name + "\n";
                }
                ++itf;
            }

            first = true;
            tmpSql += ") VALUES (";

            // we need to know the datatipe for each field, to properly quote strings
            Database * base = static_cast<Database *>(parent());
            TablePtr tab = base->getTable(tabName);
            if (tab) {
                FldIterator it(d->fields);
                while (it.current()) {
                    kdDebug(20000) << "examinig field " << it.current()->name << endl;
                    KDB::Field * f = tab->getField(it.current()->name);
                    if (!f) //if the field does not exists, drop it
                        break;
                    bool quotes = false;
                    switch (f->type()) {
                    case CHAR:
                    case VARCHAR:
                    case BLOB:
                        kdDebug(20000) << "needs quotes" << endl;
                        quotes = true;
                        break;
                    default:
                        break;
                    }
                    
                    if (!first)
                        tmpSql += ",";

                    first = false;

                    if (quotes)
                        tmpSql += "'";
                    
                    tmpSql += it.current()->val;
                    
                    if (quotes)
                        tmpSql += "'";
                    
                    tmpSql += "\n";
                    ++it;
                }
            }
            tmpSql += ")";
            break;
        }
    case Update:
        {
            kdDebug(20000) << "QueryType: UPDATE" << endl;
            QString tabName = *(d->tables.begin());
            tmpSql = QString("UPDATE  %1 SET ").arg(tabName);
            
            // we need to know the datatipe for each field, to properly quote strings
            Database * base = static_cast<Database *>(parent());
            TablePtr tab = base->getTable(tabName);
            if (tab) {
                FldIterator it(d->fields);
                while (it.current()) {
                    KDB::Field * f = tab->getField(it.current()->name);
                    if (!f) //if the field does not exists, drop it
                        break;                    
                    bool quotes = false;
                    switch (f->type()) {
                    case CHAR:
                    case VARCHAR:
                    case BLOB:
                        quotes = true;
                        break;
                    default:
                        break;
                    }
                    QString strQuotes = quotes ? "'": QString::null;

                    if (!first)
                        tmpSql += ",";
                    first = false;
                    
                    tmpSql += it.current()->name +
                        " = " + strQuotes
                        + it.current()->val
                        + strQuotes + "\n";

                    ++it;
                }
            }
            break;
        }
    case Delete:
        kdDebug(20000) << "QueryType: DELETE" << endl;
        tmpSql = QString("DELETE FROM %1 ").arg(*(d->tables.begin()));
        break;
    default:
        break;
    }
    
    // step two: build the where clause (not for insert)
    if (d->cond.count() != 0 && d->type != Insert) {
        tmpSql += " WHERE ";
        
        first = true;
        int prevLevel = 0;
        // grab the conditions in order
        CondIterator itc(d->cond);
        
        while (itc.current()) {
            qryCond *c = itc.current();

            //first, determine AND/OR
            if (!first) {
                if (c->type == And) 
                    tmpSql += " AND ";
                else
                    tmpSql += " OR ";
            }
            first = false;

            // then, determine the parentesis
            if (c->level > prevLevel) { // must open some (
                while (prevLevel < c->level) {
                    tmpSql += " ( ";
                    prevLevel++;
                }
            } else if (c->level < prevLevel) { // must close )
                while (prevLevel > c->level) {
                    tmpSql += " ) ";
                    prevLevel--;
                }
            } // do nothing if the level is the same

            // then, add the condition
            tmpSql += c->condition;
            ++itc;
        }

        // we must close all parentesis
        while (prevLevel--) {
            tmpSql += " ) ";
        }
    }

    // now, replace the % fields with the parameters
                
    kdDebug(20000) << "resulted SQL: " << tmpSql << endl;
    return tmpSql;
}



