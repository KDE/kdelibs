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

#include "property.h"
#include "property_p.h"
#include "class.h"
#include "ontologymanager.h"
#include "ontology.h"
#include "literal.h"
#include "global.h"


Nepomuk::Property::Property()
    : Entity()
{
    d = new Private();
}


Nepomuk::Property::Property( const Property& other )
    : Entity( other )
{
    d = other.d;
}


Nepomuk::Property::~Property()
{
}


Nepomuk::Property& Nepomuk::Property::operator=( const Property& other )
{
    d = other.d;
    return *this;
}


QList<const Nepomuk::Property*> Nepomuk::Property::parentProperties() const
{
    return d->parents;
}


QList<const Nepomuk::Property*> Nepomuk::Property::parentOf() const
{
    return d->children;
}


const Nepomuk::Property* Nepomuk::Property::inverseProperty() const
{
    return d->inverse;
}


const Nepomuk::Class* Nepomuk::Property::range() const
{
    return d->range;
}


Nepomuk::Literal Nepomuk::Property::literalRangeType() const
{
    return d->literalRange;
}


const Nepomuk::Class* Nepomuk::Property::domain() const
{
    return d->domain;
}


int Nepomuk::Property::cardinality() const
{
    return d->cardinality;
}


int Nepomuk::Property::minCardinality() const
{
    return d->minCardinality;
}


int Nepomuk::Property::maxCardinality() const
{
    return d->maxCardinality;
}


const Nepomuk::Property* Nepomuk::Property::load( const QUrl& uri )
{
    QUrl ns = extractNamespace( uri );

    // load the ontology in the cache
    const Ontology* ont = OntologyManager::instance()->getOntology( ns );

    // return the requested class or an invalid one if the ontology
    // was not found
    if ( ont ) {
        return ont->findPropertyByUri( uri );
    }
    else {
        return 0;
    }
}
