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
#include "table.h"
#include "field.h"
#include "index.h"
#include "connector.h"
#include "recordset.h"

#include <kdebug.h>

#include "table.moc"

using namespace KDB;

Table::Table(Connector *conn, QObject *parent, const char *name, bool design )
    : DataObject( parent, name ), connector(conn->clone()), m_design(design)
{
    if (connector->tables().contains(name)) {
        RowList l = connector->fields(name);
        for ( RowList::Iterator it = l.begin(); it != l.end(); ++it ) {
            Row r = *it;
            unsigned int nField = r.count();
            // name
            Field * f = new Field( r[0].asString(), this );

            // datatype
            f->setType(connector->nativeToKDB(r[1].asString()));
            f->setNativeType(r[1].asString());

            // size
            if (nField > 2) {
                unsigned int size = r[2].asInt(); 
                if (size) f->setSize(size);
            }

            // precision
            if (nField > 3) {
                unsigned int prec = r[3].asInt();
                if (prec) f->setPrecision(prec);
            }

            // accepts null
            if (nField > 4) {
                f->setAcceptNull("Y" == r[4].asString());
            }

            // Default value
            if (nField > 5) {
                f->setDefaultValue(r[5]);
            }
            
            // referential constraint
            if (nField > 6) {
                f->setConstraint(r[6].asString());
            }
            
            // comment
            if (nField > 7) {
                f->setComment(r[7].asString());
            }
            
            m_fields.append(f);
        }
    }
    m_fields.setAutoDelete(true);
}

Table::~Table()
{
    kdDebug(20000) << "Table::~Table" << endl;
    delete connector;
}

Field *
Table::getField(const QString &name) const
{
    FieldIterator it(m_fields);

    while ( it.current() ) {
        if ( it.current()->name() == name )
            break;
        ++it;
    }

    return it.current();
}

FieldList
Table::fields() const
{
    return m_fields;
}

QStringList
Table::fieldNames() const
{
    QStringList lst;

    // TODO: build list of field names

    return lst;
}

Index *
Table::getIndex(const QString &name) const
{
    IndexIterator it(m_indexes);

    while ( it.current() ) {
        if ( it.current()->name() == name )
            break;
        ++it;
    }

    return it.current();
}

IndexList
Table::indexes() const
{
    return m_indexes;
}

Field *
Table::newField(const QString &name)
{
    return new Field(name, this);
}

bool
Table::appendField(Field *f)
{
    // must change it to an alter when in design mode.
    if (!designMode()) {
        pushError(new InvalidRequest(QString("Table %1 is not in design mode").arg(name())));
        return false;
    }
    m_fields.append(f);
    emit fieldAdded(f);
    emit changed();
    return true;
}

bool
Table::removeField(const QString &name)
{
    // must change it to an alter when in design mode.
    if (!designMode()) {
        pushError(new InvalidRequest(QString("Table %1 is not in design mode").arg(this->name())));
        return false;
    }
    Field * f = getField(name);
    if (error()) {
        return false;
    }
    
    m_fields.remove(f);
    emit fieldRemoved(f);
    emit changed();
    return true;
}

Index *
Table::newIndex(const QString & /* name */)
{
    return 0L;
}

RecordsetPtr
Table::openRecordset() 
{
    if (m_design) {
        pushError(new InvalidRequest(QString("Table %1 is in design mode").arg(name())));
        return 0L;
    }
    return new Recordset(connector, QString("Select * from %1").arg(name()), const_cast<Table *>(this));
}


bool
Table::create() 
{
    connector->createTable(*this);
    if (error())
        return false;
    
    m_design = false;
    emit created( this );
    return true;
}

bool
Table::designMode() const
{
    return m_design;
}


