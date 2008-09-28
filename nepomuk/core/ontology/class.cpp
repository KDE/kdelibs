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
#include "resourcemanager.h"
#include "property.h"
#include "entitymanager.h"

#include <QtCore/QList>

#include <kdebug.h>

#include <Soprano/QueryResultIterator>
#include <Soprano/Model>
#include <Soprano/Vocabulary/NRL>
#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/RDF>


#define D static_cast<Nepomuk::Types::ClassPrivate*>( d.data() )

Nepomuk::Types::ClassPrivate::ClassPrivate( const QUrl& uri )
    : EntityPrivate( uri ),
      propertiesAvailable( uri.isValid() ? -1 : 0 )
{
}


bool Nepomuk::Types::ClassPrivate::load()
{
    //
    // Nearly all here can be done in a very clean way. There is only
    // one special case: rdfs:Resource, the base class of them all
    //
    if ( EntityPrivate::load() ) {
        // undefined super class means that we are derived from rdfs:Resource directly
        if ( parents.isEmpty() ) {
            if ( uri != Soprano::Vocabulary::RDFS::Resource() ) {
                parents += Soprano::Vocabulary::RDFS::Resource();
            }
        }
        return true;
    }
    else {
        return false;
    }
}


bool Nepomuk::Types::ClassPrivate::loadAncestors()
{
    //
    // Nearly all here can be done in a very clean way. There is only
    // one special case: rdfs:Resource, the base class of them all
    //
    if ( uri == Soprano::Vocabulary::RDFS::Resource() ) {
        //
        // All classes that do not explicetely state a superclass are
        // derived from rdfs:Resource. This query selects those classes
        // (might fail on redland though)
        //
        Soprano::QueryResultIterator it
            = ResourceManager::instance()->mainModel()->executeQuery( QString("select distinct ?s where { "
                                                                              "graph ?g1 { ?s a <%1> . } . "
                                                                              "OPTIONAL { graph ?g2 { ?s <%2> ?ss . } } . "
                                                                              "?g1 a <%3> . "
                                                                              "?g2 a <%3> . "
                                                                              "FILTER(!BOUND(?ss)) . }")
                                                                      .arg( Soprano::Vocabulary::RDFS::Class().toString() )
                                                                      .arg( Soprano::Vocabulary::RDFS::subClassOf().toString() )
                                                                      .arg( Soprano::Vocabulary::NRL::Ontology().toString() ),
                                                                      Soprano::Query::QueryLanguageSparql );
        bool success = false;
        while ( it.next() ) {
            success = true;
            QUrl resUri = it.binding( "s" ).uri();
            if ( resUri != Soprano::Vocabulary::RDFS::Resource() ) {
                children.append( resUri );
            }
        }
    }

    return EntityPrivate::loadAncestors();
}


bool Nepomuk::Types::ClassPrivate::addProperty( const QUrl& property, const Soprano::Node& value )
{
    if( property == Soprano::Vocabulary::RDFS::subClassOf() ) {
        parents.append( Class( value.uri() ) );
        return true;
    }

    return false;
}


bool Nepomuk::Types::ClassPrivate::addAncestorProperty( const QUrl& ancestorResource, const QUrl& property )
{
    if ( property == Soprano::Vocabulary::RDFS::subClassOf() ) {
        children.append( Class( ancestorResource ) );
        return true;
    }

    return false;
}


void Nepomuk::Types::ClassPrivate::initProperties()
{
    if ( propertiesAvailable < 0 ) {
        propertiesAvailable = loadProperties() ? 1 : 0;
    }
}


bool Nepomuk::Types::ClassPrivate::loadProperties()
{
    // load domains
    Soprano::QueryResultIterator it
        = ResourceManager::instance()->mainModel()->executeQuery( QString("select ?p where { "
                                                                          "graph ?g { ?p <%1> <%2> . } . "
                                                                          "?g a <%3> . }")
                                                                  .arg( Soprano::Vocabulary::RDFS::domain().toString() )
                                                                  .arg( QString::fromAscii( uri.toEncoded() ) )
                                                                  .arg( Soprano::Vocabulary::NRL::Ontology().toString() ),
                                                                  Soprano::Query::QueryLanguageSparql );
    bool success = false;
    while ( it.next() ) {
        success = true;
        domainOf.append( Property( it.binding( "p" ).uri() ) );
    }


    // load ranges
    it = ResourceManager::instance()->mainModel()->executeQuery( QString("select ?p where { "
                                                                          "graph ?g { ?p <%1> <%2> . } . "
                                                                          "?g a <%3> . }")
                                                                  .arg( Soprano::Vocabulary::RDFS::range().toString() )
                                                                 .arg( QString::fromAscii( uri.toEncoded() ) )
                                                                  .arg( Soprano::Vocabulary::NRL::Ontology().toString() ),
                                                                  Soprano::Query::QueryLanguageSparql );
    while ( it.next() ) {
        success = true;
        rangeOf.append( Property( it.binding( "p" ).uri() ) );
    }

    return success;
}


void Nepomuk::Types::ClassPrivate::reset( bool recursive )
{
    EntityPrivate::reset( recursive );

    if ( propertiesAvailable != -1 ) {
        if ( recursive ) {
            foreach( Property p, domainOf ) {
                p.reset( true );
            }
            foreach( Property p, rangeOf ) {
                p.reset( true );
            }
        }

        domainOf.clear();
        rangeOf.clear();
        propertiesAvailable = -1;
    }

    if ( available != -1 ) {
        if ( recursive ) {
            foreach( Class c, parents ) {
                c.reset( true );
            }
        }
        parents.clear();
        available = -1;
    }

    if ( ancestorsAvailable != -1 ) {
        if ( recursive ) {
            foreach( Class c, children ) {
                c.reset( true );
            }
        }
        children.clear();
        ancestorsAvailable = -1;
    }
}


QSet<Nepomuk::Types::Class> Nepomuk::Types::ClassPrivate::findParentClasses( ClassPrivate* requestingClass )
{
    QSet<Class> allParents;

    for ( QList<Class>::iterator it = parents.begin(); it != parents.end(); ++it ) {
        ClassPrivate* p = static_cast<Nepomuk::Types::ClassPrivate*>( it->d.data() );
        if ( p != requestingClass ) {
            p->init();
            allParents += p->findParentClasses( requestingClass );
            allParents += *it;
        }
    }

    return allParents;
}


QSet<Nepomuk::Types::Class> Nepomuk::Types::ClassPrivate::findSubClasses( ClassPrivate* requestingClass )
{
    QSet<Class> allChildren;

    for ( QList<Class>::iterator it = children.begin(); it != children.end(); ++it ) {
        ClassPrivate* p = static_cast<Nepomuk::Types::ClassPrivate*>( it->d.data() );
        if ( p != requestingClass ) {
            p->initAncestors();
            allChildren += p->findSubClasses( requestingClass );
            allChildren += *it;
        }
    }

    return allChildren;
}



Nepomuk::Types::Class::Class()
    : Entity()
{
    d = 0;
}


Nepomuk::Types::Class::Class( const QUrl& uri )
    : Entity()
{
    d = EntityManager::self()->getClass( uri );
}


Nepomuk::Types::Class::Class( const Class& other )
    : Entity( other )
{
}


Nepomuk::Types::Class::~Class()
{
}


Nepomuk::Types::Class& Nepomuk::Types::Class::operator=( const Class& other )
{
    d = other.d;
    return *this;
}


QList<Nepomuk::Types::Property> Nepomuk::Types::Class::rangeOf()
{
    if ( d ) {
        D->initProperties();
        return D->rangeOf;
    }
    else {
        return QList<Nepomuk::Types::Property>();
    }
}


QList<Nepomuk::Types::Property> Nepomuk::Types::Class::domainOf()
{
    if ( d ) {
        D->initProperties();
        return D->domainOf;
    }
    else {
        return QList<Nepomuk::Types::Property>();
    }
}


Nepomuk::Types::Property Nepomuk::Types::Class::findPropertyByName( const QString& name )
{
    if ( d ) {
        D->initProperties();
        for ( QList<Property>::const_iterator it = D->domainOf.constBegin();
              it != D->domainOf.constEnd(); ++it ) {
            const Property& p = *it;
            if ( p.name() == name ) {
                return p;
            }
        }
    }

    return Property();
}


Nepomuk::Types::Property Nepomuk::Types::Class::findPropertyByLabel( const QString& label, const QString& language )
{
    if ( d ) {
        D->initProperties();
        for ( QList<Property>::iterator it = D->domainOf.begin();
              it != D->domainOf.end(); ++it ) {
            Property& p = *it;
            if ( p.label( language ) == label ) {
                return p;
            }
        }
    }

    return Property();
}


QList<Nepomuk::Types::Class> Nepomuk::Types::Class::parentClasses()
{
    if ( d ) {
        D->init();
        return D->parents;
    }
    else {
        return QList<Nepomuk::Types::Class>();
    }
}


QList<Nepomuk::Types::Class> Nepomuk::Types::Class::subClasses()
{
    if ( d ) {
        D->initAncestors();
        return D->children;
    }
    else {
        return QList<Nepomuk::Types::Class>();
    }
}


QList<Nepomuk::Types::Class> Nepomuk::Types::Class::allParentClasses()
{
    if ( d ) {
        D->init();
        return D->findParentClasses( D ).toList();
    }
    else {
        return QList<Nepomuk::Types::Class>();
    }
}


QList<Nepomuk::Types::Class> Nepomuk::Types::Class::allSubClasses()
{
    if ( d ) {
        D->initAncestors();
        return D->findSubClasses( D ).toList();
    }
    else {
        return QList<Nepomuk::Types::Class>();
    }
}


bool Nepomuk::Types::Class::isParentOf( const Class& other )
{
    if ( d ) {
        D->initAncestors();

        if ( D->children.contains( other ) ) {
            return true;
        }
        else {
            for ( QList<Nepomuk::Types::Class>::iterator it = D->children.begin();
                  it != D->children.end(); ++it ) {
                if ( ( *it ).isParentOf( other ) ) {
                    return true;
                }
            }
        }
    }

    return false;
}


bool Nepomuk::Types::Class::isSubClassOf( const Class& other )
{
    if ( d ) {
        D->init();

        if ( D->parents.contains( other ) ) {
            return true;
        }
        else {
            for ( QList<Nepomuk::Types::Class>::iterator it = D->parents.begin();
                  it != D->parents.end(); ++it ) {
                if ( ( *it ).isSubClassOf( other ) ) {
                    return true;
                }
            }
        }
    }

    return false;
}




// Start of code for deprecated Class
// ----------------------------------

#include "global.h"
#include "ontologymanager.h"

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
