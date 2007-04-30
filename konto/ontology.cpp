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

#include "ontology.h"
#include "ontology_p.h"
#include "ontologymanager.h"
#include "class.h"
#include "property.h"
#include "qurlhash.h"


Konto::Ontology::Ontology()
{
    d = new Private;
}


Konto::Ontology::Ontology( const Ontology& other )
{
    d = other.d;
}


Konto::Ontology::~Ontology()
{
}


Konto::Ontology& Konto::Ontology::operator=( const Ontology& other)
{
    d = other.d;
    return *this;
}


QUrl Konto::Ontology::uri() const
{
    return d->uri;
}


QList<const Konto::Class*> Konto::Ontology::allClasses() const
{
    return d->classes.values();
}


const Konto::Class* Konto::Ontology::findClassByName( const QString& name ) const
{
    for ( QHash<QUrl, const Class*>::const_iterator it = d->classes.constBegin();
          it != d->classes.constEnd(); ++it ) {
        const Class* c = it.value();
        if ( c->name() == name ) {
            return c;
        }
    }

    return 0;
}


const Konto::Class* Konto::Ontology::findClassByLabel( const QString& label, const QString& language ) const
{
    for ( QHash<QUrl, const Class*>::const_iterator it = d->classes.constBegin();
          it != d->classes.constEnd(); ++it ) {
        const Class* c = it.value();
        if ( c->label( language ) == label ) {
            return c;
        }
    }

    return 0;
}


const Konto::Class* Konto::Ontology::findClassByUri( const QUrl& uri ) const
{
    QHash<QUrl, const Class*>::const_iterator it = d->classes.find( uri );
    if ( it != d->classes.constEnd() ) {
        return it.value();
    }
    else {
        return 0;
    }
}


QList<const Konto::Property*> Konto::Ontology::allProperties() const
{
    return d->properties.values();
}


const Konto::Property* Konto::Ontology::findPropertyByName( const QString& name ) const
{
    for ( QHash<QUrl, const Property*>::const_iterator it = d->properties.constBegin();
          it != d->properties.constEnd(); ++it ) {
        const Property* p = it.value();
        if ( p->name() == name ) {
            return p;
        }
    }

    return 0;
}


const Konto::Property* Konto::Ontology::findPropertyByLabel( const QString& label, const QString& language ) const
{
    for ( QHash<QUrl, const Property*>::const_iterator it = d->properties.constBegin();
          it != d->properties.constEnd(); ++it ) {
        const Property* p = it.value();
        if ( p->label( language ) == label ) {
            return p;
        }
    }

    return 0;
}


const Konto::Property* Konto::Ontology::findPropertyByUri( const QUrl& uri ) const
{
    QHash<QUrl, const Property*>::const_iterator it = d->properties.find( uri );
    if ( it != d->properties.constEnd() ) {
        return it.value();
    }
    else {
        return 0;
    }
}


const Konto::Ontology* Konto::Ontology::load( const QUrl& uri )
{
    // load the ontology in the cache
    return OntologyManager::instance()->getOntology( uri );
}
