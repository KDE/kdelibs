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
#include "ontology.h"
#include "literal.h"
#include "entitymanager.h"

#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/NRL>
#include <Soprano/Vocabulary/XMLSchema>

#undef D
#define D static_cast<Nepomuk::Types::PropertyPrivate*>( d.data() )

Nepomuk::Types::PropertyPrivate::PropertyPrivate( const QUrl& uri )
    : EntityPrivate( uri ),
      minCardinality( -1 ),
      maxCardinality( -1 ),
      cardinality( -1 )
{
}


bool Nepomuk::Types::PropertyPrivate::addProperty( const QUrl& property, const Soprano::Node& value )
{
    if( property == Soprano::Vocabulary::RDFS::subPropertyOf() ) {
        parents.append( value.uri() );
        return true;
    }

    else if( property == Soprano::Vocabulary::RDFS::domain() ) {
        domain = value.uri();
        return true;
    }

    else if( property == Soprano::Vocabulary::RDFS::range() ) {
        if ( value.toString().startsWith( Soprano::Vocabulary::XMLSchema::xsdNamespace().toString() ) ) {
            literalRange = Literal( value.uri() );
        }
        else if ( value.uri() == Soprano::Vocabulary::RDFS::Literal()) {
            literalRange = Literal( value.uri() );
        }
        else {
            range = value.uri();
        }
        return true;
    }

    else if( property == Soprano::Vocabulary::NRL::minCardinality() ) {
        minCardinality = value.literal().toInt();
        return true;
    }

    else if( property == Soprano::Vocabulary::NRL::maxCardinality() ) {
        maxCardinality = value.literal().toInt();
        return true;
    }

    else if ( property == Soprano::Vocabulary::NRL::cardinality() ) {
        cardinality = value.literal().toInt();
        return true;
    }

    else if ( property == Soprano::Vocabulary::NRL::inverseProperty() ) {
        inverse = value.uri();
        return true;
    }

    return false;
}


bool Nepomuk::Types::PropertyPrivate::addAncestorProperty( const QUrl& ancestorResource, const QUrl& property )
{
    if( property == Soprano::Vocabulary::RDFS::subPropertyOf() ) {
        children.append( ancestorResource );
        return true;
    }
    else if ( property == Soprano::Vocabulary::NRL::inverseProperty() ) {
        inverse = ancestorResource;
        return true;
    }

    return false;
}


void Nepomuk::Types::PropertyPrivate::reset( bool recursive )
{
    EntityPrivate::reset( recursive );

    if ( available != -1 ) {
        if ( recursive ) {
            range.reset( true );
            domain.reset( true );
            inverse.reset( true );
            foreach( Property p, parents ) {
                p.reset( true );
            }
        }

        parents.clear();
        available = -1;
    }

    if ( ancestorsAvailable != -1 ) {
        if ( recursive ) {
            foreach( Property p, children ) {
                p.reset( true );
            }
        }

        children.clear();
        ancestorsAvailable = -1;
    }
}



Nepomuk::Types::Property::Property()
    : Entity()
{
    d = 0;
}


Nepomuk::Types::Property::Property( const QUrl& uri )
    : Entity()
{
    d = EntityManager::self()->getProperty( uri );
}


Nepomuk::Types::Property::Property( const Property& other )
    : Entity( other )
{
}


Nepomuk::Types::Property::~Property()
{
}


Nepomuk::Types::Property& Nepomuk::Types::Property::operator=( const Property& other )
{
    d = other.d;
    return *this;
}


QList<Nepomuk::Types::Property> Nepomuk::Types::Property::parentProperties()
{
    if ( d ) {
        D->init();
        return D->parents;
    }
    else {
        return QList<Nepomuk::Types::Property>();
    }
}


QList<Nepomuk::Types::Property> Nepomuk::Types::Property::subProperties()
{
    if ( d ) {
        D->initAncestors();
        return D->children;
    }
    else {
        return QList<Nepomuk::Types::Property>();
    }
}


Nepomuk::Types::Property Nepomuk::Types::Property::inverseProperty()
{
    if ( d ) {
        D->init();
        D->initAncestors();
        return D->inverse;
    }
    else {
        return Property();
    }
}


Nepomuk::Types::Class Nepomuk::Types::Property::range()
{
    if ( d ) {
        D->init();
        return D->range;
    }
    else {
        return Class();
    }
}


Nepomuk::Types::Literal Nepomuk::Types::Property::literalRangeType()
{
    if ( d ) {
        D->init();
        return D->literalRange;
    }
    else {
        return Literal();
    }
}


Nepomuk::Types::Class Nepomuk::Types::Property::domain()
{
    if ( d ) {
        D->init();
        return D->domain;
    }
    else {
        return Class();
    }
}


int Nepomuk::Types::Property::cardinality()
{
    if ( d ) {
        D->init();
        return D->cardinality;
    }
    else {
        return -1;
    }
}


int Nepomuk::Types::Property::minCardinality()
{
    if ( d ) {
        D->init();
        if ( D->minCardinality > 0 ) {
            return D->minCardinality;
        }
        else {
            return D->cardinality;
        }
    }
    else {
        return -1;
    }
}


int Nepomuk::Types::Property::maxCardinality()
{
    if ( d ) {
        D->init();
        if ( D->maxCardinality > 0 ) {
            return D->maxCardinality;
        }
        else {
            return D->cardinality;
        }
    }
    else {
        return -1;
    }
}


bool Nepomuk::Types::Property::isParentOf( const Property& other )
{
    if ( d ) {
        D->initAncestors();

        if ( D->children.contains( other ) ) {
            return true;
        }
        else {
            for ( QList<Nepomuk::Types::Property>::iterator it = D->children.begin();
                  it != D->children.end(); ++it ) {
                if ( ( *it ).isParentOf( other ) ) {
                    return true;
                }
            }
        }
    }

    return false;
}


bool Nepomuk::Types::Property::isSubPropertyOf( const Property& other )
{
    if ( d ) {
        D->init();

        if ( D->parents.contains( other ) ) {
            return true;
        }
        else {
            for ( QList<Nepomuk::Types::Property>::iterator it = D->parents.begin();
                  it != D->parents.end(); ++it ) {
                if ( ( *it ).isSubPropertyOf( other ) ) {
                    return true;
                }
            }
        }
    }

    return false;
}





// Start of code for deprecated Property
// -------------------------------------
#include "global.h"
#include "ontologymanager.h"

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
