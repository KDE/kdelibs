/* This file is part of the Nepomuk-KDE libraries
    Copyright (c) 2007 Sebastian Trueg <trueg@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "class.h"
#include "class_p.h"
#include "ontology.h"
#include "ontologymanager.h"
#include "property.h"
#include "qurlhash.h"

#include <QHash>


Konto::Class::Class()
    : Entity()
{
    d = new Private();
}


Konto::Class::Class( const Class& other )
    : Entity( other )
{
    d = other.d;
}


Konto::Class::~Class()
{
}


Konto::Class& Konto::Class::operator=( const Class& other )
{
    d = other.d;
    return *this;
}


QList<const Konto::Property*> Konto::Class::allProperties() const
{
    return d->domainOf.values();
}


const Konto::Property* Konto::Class::findPropertyByName( const QString& name ) const
{
    for ( QHash<QUrl, const Property*>::const_iterator it = d->domainOf.constBegin();
          it != d->domainOf.constEnd(); ++it ) {
        const Property* p = it.value();
        if ( p->name() == name ) {
            return p;
        }
    }

    return 0;
}


const Konto::Property* Konto::Class::findPropertyByLabel( const QString& label, const QString& language ) const
{
    for ( QHash<QUrl, const Property*>::const_iterator it = d->domainOf.constBegin();
          it != d->domainOf.constEnd(); ++it ) {
        const Property* p = it.value();
        if ( p->label( language ) == label ) {
            return p;
        }
    }

    return 0;
}


const Konto::Property* Konto::Class::findPropertyByUri( const QUrl& uri ) const
{
    QHash<QUrl, const Property*>::const_iterator it = d->domainOf.find( uri );
    if ( it != d->domainOf.constEnd() ) {
        return it.value();
    }
    else {
        return 0;
    }
}


QList<const Konto::Class*> Konto::Class::parentClasses() const
{
    return d->parents;
}


QList<const Konto::Class*> Konto::Class::subClasses() const
{
    return d->children;
}


bool Konto::Class::isParentOf( const Class* other ) const
{
    if ( d->children.contains( other ) ) {
        return true;
    }
    else {
        for ( QList<const Konto::Class*>::const_iterator it = d->children.constBegin();
              it != d->children.constEnd(); ++it ) {
            if ( ( *it )->isParentOf( other ) ) {
                return true;
            }
        }
        return false;
    }
}


bool Konto::Class::isSubClassOf( const Class* other ) const
{
    if ( d->parents.contains( other ) ) {
        return true;
    }
    else {
        for ( QList<const Konto::Class*>::const_iterator it = d->parents.constBegin();
              it != d->parents.constEnd(); ++it ) {
            if ( ( *it )->isSubClassOf( other ) ) {
                return true;
            }
        }
        return false;
    }
}


const Konto::Class* Konto::Class::load( const QUrl& uri )
{
    // extract the namespace of the class, i.e. the ontology uri
    QUrl ns( uri );
    ns.setFragment( QString() );

    // load the ontology in the cache
    const Ontology* ont = OntologyManager::instance()->getOntology( ns );

    // return the requested class or an invalid one if the ontology
    // was not found
    if ( ont ) {
        return ont->findClassByUri( uri );
    }
    else {
        return 0;
    }
}

