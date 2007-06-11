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
#include "global.h"

#include <QtCore/QHash>

#include <kdebug.h>


Nepomuk::Class::Class()
    : Entity()
{
    d = new Private();
}


Nepomuk::Class::Class( const Class& other )
    : Entity( other )
{
    d = other.d;
}


Nepomuk::Class::~Class()
{
}


Nepomuk::Class& Nepomuk::Class::operator=( const Class& other )
{
    d = other.d;
    return *this;
}


QList<const Nepomuk::Property*> Nepomuk::Class::allProperties() const
{
    return d->domainOf.values();
}


const Nepomuk::Property* Nepomuk::Class::findPropertyByName( const QString& name ) const
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


const Nepomuk::Property* Nepomuk::Class::findPropertyByLabel( const QString& label, const QString& language ) const
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


const Nepomuk::Property* Nepomuk::Class::findPropertyByUri( const QUrl& uri ) const
{
    QHash<QUrl, const Property*>::const_iterator it = d->domainOf.find( uri );
    if ( it != d->domainOf.constEnd() ) {
        return it.value();
    }
    else {
        return 0;
    }
}


QList<const Nepomuk::Class*> Nepomuk::Class::parentClasses() const
{
    return d->parents;
}


QList<const Nepomuk::Class*> Nepomuk::Class::subClasses() const
{
    return d->children;
}


bool Nepomuk::Class::isParentOf( const Class* other ) const
{
    if ( d->children.contains( other ) ) {
        return true;
    }
    else {
        for ( QList<const Nepomuk::Class*>::const_iterator it = d->children.constBegin();
              it != d->children.constEnd(); ++it ) {
            if ( ( *it )->isParentOf( other ) ) {
                return true;
            }
        }
        return false;
    }
}


bool Nepomuk::Class::isSubClassOf( const Class* other ) const
{
    if ( d->parents.contains( other ) ) {
        return true;
    }
    else {
        for ( QList<const Nepomuk::Class*>::const_iterator it = d->parents.constBegin();
              it != d->parents.constEnd(); ++it ) {
            if ( ( *it )->isSubClassOf( other ) ) {
                return true;
            }
        }
        return false;
    }
}


const Nepomuk::Class* Nepomuk::Class::load( const QUrl& uri )
{
    Q_ASSERT( !uri.isEmpty() );

    QUrl ns = extractNamespace( uri );

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

