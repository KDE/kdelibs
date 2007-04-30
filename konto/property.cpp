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


Konto::Property::Property()
    : Entity()
{
    d = new Private();
}


Konto::Property::Property( const Property& other )
    : Entity( other )
{
    d = other.d;
}


Konto::Property::~Property()
{
}


Konto::Property& Konto::Property::operator=( const Property& other )
{
    d = other.d;
    return *this;
}


QList<const Konto::Property*> Konto::Property::parentProperties() const
{
    return d->parents;
}


QList<const Konto::Property*> Konto::Property::parentOf() const
{
    return d->children;
}


const Konto::Property* Konto::Property::inverseProperty() const
{
    return d->inverse;
}


const Konto::Class* Konto::Property::range() const
{
    return d->range;
}


Konto::Literal Konto::Property::literalRangeType() const
{
    return d->literalRange;
}


const Konto::Class* Konto::Property::domain() const
{
    return d->domain;
}


int Konto::Property::cardinality() const
{
    return d->cardinality;
}


int Konto::Property::minCardinality() const
{
    return d->minCardinality;
}


int Konto::Property::maxCardinality() const
{
    return d->maxCardinality;
}


const Konto::Property* Konto::Property::load( const QUrl& uri )
{
    // extract the namespace of the class, i.e. the ontology uri
    QUrl ns( uri );
    ns.setFragment( QString() );

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
