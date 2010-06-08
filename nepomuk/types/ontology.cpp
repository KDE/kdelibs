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
#include "class.h"
#include "property.h"
#include "entitymanager.h"
#include "resourcemanager.h"

#include <Soprano/QueryResultIterator>
#include <Soprano/Model>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>

#undef D
#define D static_cast<Nepomuk::Types::OntologyPrivate*>( d.data() )

Nepomuk::Types::OntologyPrivate::OntologyPrivate( const QUrl& uri )
    : EntityPrivate( uri ),
      entitiesAvailable( uri.isValid() ? -1 : 0 )
{
}


void Nepomuk::Types::OntologyPrivate::initEntities()
{
    if ( entitiesAvailable < 0 ) {
        entitiesAvailable = loadEntities() ? 1 : 0;
    }
}


bool Nepomuk::Types::OntologyPrivate::loadEntities()
{
    // load classes
    // We use a FILTER(STR(?ns)...) to support both Soprano 2.3 (with plain literals) and earlier (with only typed ones)
    Soprano::QueryResultIterator it
        = ResourceManager::instance()->mainModel()->executeQuery( QString("select ?c where { "
                                                                          "graph ?g { ?c a <%1> . } . "
                                                                          "?g <%2> ?ns . "
                                                                          "FILTER(STR(?ns) = \"%3\") . }" )
                                                                  .arg( Soprano::Vocabulary::RDFS::Class().toString() )
                                                                  .arg( Soprano::Vocabulary::NAO::hasDefaultNamespace().toString() )
                                                                  .arg( QString::fromAscii( uri.toEncoded() ) ),
                                                                  Soprano::Query::QueryLanguageSparql );
    while ( it.next() ) {
        classes.append( Class( it.binding( "c" ).uri() ) );
    }


    // load properties
    it = ResourceManager::instance()->mainModel()->executeQuery( QString("select ?p where { "
                                                                         "graph ?g { ?p a <%1> . } . "
                                                                         "?g <%2> ?ns . "
                                                                         "FILTER(STR(?ns) = \"%3\") . }" )
                                                                 .arg( Soprano::Vocabulary::RDF::Property().toString() )
                                                                 .arg( Soprano::Vocabulary::NAO::hasDefaultNamespace().toString() )
                                                                 .arg( QString::fromAscii( uri.toEncoded() ) ),
                                                                 Soprano::Query::QueryLanguageSparql );
    while ( it.next() ) {
        properties.append( Property( it.binding( "p" ).uri() ) );
    }

    return !it.lastError();
}


bool Nepomuk::Types::OntologyPrivate::addProperty( const QUrl&, const Soprano::Node& )
{
    return false;
}


bool Nepomuk::Types::OntologyPrivate::addAncestorProperty( const QUrl&, const QUrl& )
{
    return false;
}


void Nepomuk::Types::OntologyPrivate::reset( bool recursive )
{
    QMutexLocker lock( &mutex );

    if ( entitiesAvailable != -1 ) {
        if ( recursive ) {
            foreach( Class c, classes ) {
                c.reset( true );
            }
            foreach( Property p, properties ) {
                p.reset( true );
            }
        }
        classes.clear();
        properties.clear();

        entitiesAvailable = -1;
    }

    EntityPrivate::reset( recursive );
}



Nepomuk::Types::Ontology::Ontology()
{
    d = new OntologyPrivate();
}


Nepomuk::Types::Ontology::Ontology( const QUrl& uri )
{
    d = EntityManager::self()->getOntology( uri );
}


Nepomuk::Types::Ontology::Ontology( const Ontology& other )
    : Entity( other )
{
}


Nepomuk::Types::Ontology::~Ontology()
{
}


Nepomuk::Types::Ontology& Nepomuk::Types::Ontology::operator=( const Ontology& other )
{
    d = other.d;
    return *this;
}


QList<Nepomuk::Types::Class> Nepomuk::Types::Ontology::allClasses()
{
    D->initEntities();
    return D->classes;
}


Nepomuk::Types::Class Nepomuk::Types::Ontology::findClassByName( const QString& name )
{
    D->initEntities();
    for ( QList<Class>::const_iterator it = D->classes.constBegin();
          it != D->classes.constEnd(); ++it ) {
        const Class& c = *it;
        if ( c.name() == name ) {
            return c;
        }
    }

    return Class();
}


Nepomuk::Types::Class Nepomuk::Types::Ontology::findClassByLabel( const QString& label, const QString& language )
{
    D->initEntities();
    for ( QList<Class>::iterator it = D->classes.begin();
          it != D->classes.end(); ++it ) {
        Class& c = *it;
        if ( c.label( language ) == label ) {
            return c;
        }
    }

    return Class();
}


QList<Nepomuk::Types::Property> Nepomuk::Types::Ontology::allProperties()
{
    D->initEntities();
    return D->properties;
}


Nepomuk::Types::Property Nepomuk::Types::Ontology::findPropertyByName( const QString& name )
{
    D->initEntities();
    for ( QList<Property>::const_iterator it = D->properties.constBegin();
          it != D->properties.constEnd(); ++it ) {
        const Property& p = *it;
        if ( p.name() == name ) {
            return p;
        }
    }

    return Property();
}


Nepomuk::Types::Property Nepomuk::Types::Ontology::findPropertyByLabel( const QString& label, const QString& language )
{
    D->initEntities();
    for ( QList<Property>::iterator it = D->properties.begin();
          it != D->properties.end(); ++it ) {
        Property& p = *it;
        if ( p.label( language ) == label ) {
            return p;
        }
    }

    return Property();
}



// Start of code for deprecated Ontology
// -------------------------------------
#include "global.h"
#include "ontologymanager.h"


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
