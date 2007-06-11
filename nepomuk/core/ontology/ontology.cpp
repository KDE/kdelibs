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


Nepomuk::Ontology::Ontology()
{
    d = new Private;
}


Nepomuk::Ontology::Ontology( const Ontology& other )
{
    d = other.d;
}


Nepomuk::Ontology::~Ontology()
{
}


Nepomuk::Ontology& Nepomuk::Ontology::operator=( const Ontology& other)
{
    d = other.d;
    return *this;
}


QUrl Nepomuk::Ontology::uri() const
{
    return d->uri;
}


QList<const Nepomuk::Class*> Nepomuk::Ontology::allClasses() const
{
    return d->classes.values();
}


const Nepomuk::Class* Nepomuk::Ontology::findClassByName( const QString& name ) const
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


const Nepomuk::Class* Nepomuk::Ontology::findClassByLabel( const QString& label, const QString& language ) const
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


const Nepomuk::Class* Nepomuk::Ontology::findClassByUri( const QUrl& uri ) const
{
    QHash<QUrl, const Class*>::const_iterator it = d->classes.find( uri );
    if ( it != d->classes.constEnd() ) {
        return it.value();
    }
    else {
        return 0;
    }
}


QList<const Nepomuk::Property*> Nepomuk::Ontology::allProperties() const
{
    return d->properties.values();
}


const Nepomuk::Property* Nepomuk::Ontology::findPropertyByName( const QString& name ) const
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


const Nepomuk::Property* Nepomuk::Ontology::findPropertyByLabel( const QString& label, const QString& language ) const
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


const Nepomuk::Property* Nepomuk::Ontology::findPropertyByUri( const QUrl& uri ) const
{
    QHash<QUrl, const Property*>::const_iterator it = d->properties.find( uri );
    if ( it != d->properties.constEnd() ) {
        return it.value();
    }
    else {
        return 0;
    }
}


const Nepomuk::Ontology* Nepomuk::Ontology::load( const QUrl& uri )
{
    // load the ontology in the cache
    return OntologyManager::instance()->getOntology( uri );
}
