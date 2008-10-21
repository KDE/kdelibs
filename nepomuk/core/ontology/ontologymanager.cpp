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

#include "ontologymanager.h"
#include "nepomukontologyloader.h"
#include "desktopontologyloader.h"
#include "ontology.h"
#include "ontology_p.h"
#include "class_p.h"
#include "property_p.h"
#include "entity_p.h"
#include "global.h"

#include <Soprano/Statement>
#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/XMLSchema>

#include <QtCore/QDebug>


using namespace Soprano;

class Nepomuk::OntologyManager::Private
{
public:
    Private( OntologyManager* manager )
        : userOntologyLoader( 0 ),
          desktopOntologyLoader( 0 ),
          nepomukOntologyLoader( 0 ),
          m_manager( manager ) {
    }

    OntologyLoader* userOntologyLoader;
    DesktopOntologyLoader* desktopOntologyLoader;
    NepomukOntologyLoader* nepomukOntologyLoader;

    // cache
    QHash<QUrl, Ontology> ontoCache;
    QHash<QUrl, Class> classCache;
    QHash<QUrl, Property> propertyCache;

    /**
     * Find a class in the cache.
     *
     * \param recurse if true the method will try loading dependancies (i.e.
     * other ontologies to fulfill the requirement
     */
    Class* findClass( const QUrl& uri, bool recurse = false ) {
        QHash<QUrl, Class>::iterator it = classCache.find( uri );
        if ( it != classCache.end() ) {
            return &it.value();
        }
        else {
            if ( recurse ) {
                // try loading the ontology containing the class
                QUrl parentNs = extractNamespace( uri );
                if ( m_manager->getOntology( parentNs ) ) {
                    return findClass( uri );
                }
            }
            return 0;
        }
    }

    /**
     * Find a property in the cache.
     *
     * \param recurse if true the method will try loading dependancies (i.e.
     * other ontologies to fulfill the requirement
     */
    Property* findProperty( const QUrl& uri, bool recurse = false ) {
        QHash<QUrl, Property>::iterator it = propertyCache.find( uri );
        if ( it != propertyCache.end() ) {
            return &it.value();
        }
        else {
            if ( recurse ) {
                // try loading the ontology containing the property
                QUrl parentNs = extractNamespace( uri );
                if ( m_manager->getOntology( parentNs ) ) {
                    return findProperty( uri );
                }
            }
            return 0;
        }
    }

private:
    OntologyManager* m_manager;
};


Nepomuk::OntologyManager* Nepomuk::OntologyManager::instance()
{
    static OntologyManager* s_instance = 0;
    if ( !s_instance ) {
        s_instance = new OntologyManager();
    }

    return s_instance;
}


Nepomuk::OntologyManager::OntologyManager()
    : d( new Private( this ) )
{
}


Nepomuk::OntologyManager::~OntologyManager()
{
    delete d->userOntologyLoader;
    delete d->desktopOntologyLoader;
    delete d->nepomukOntologyLoader;
    delete d;
}


void Nepomuk::OntologyManager::setOntologyLoader( OntologyLoader* loader )
{
    if ( d->userOntologyLoader != loader ) {
        delete d->userOntologyLoader;
    }
    d->userOntologyLoader = loader;
}


const Nepomuk::Ontology* Nepomuk::OntologyManager::getOntology( const QUrl& uri )
{
    // if the ontology is already in the cache return it
    //
    QHash<QUrl, Ontology>::const_iterator it = d->ontoCache.constFind( uri );
    if ( it != d->ontoCache.constEnd() ) {
        return &it.value();
    }

    // if not in the cache, try loading it
    //
    else {
        QList<Statement> sl;
        if ( !d->userOntologyLoader ) {
            if ( !d->desktopOntologyLoader ) {
                d->desktopOntologyLoader = new DesktopOntologyLoader();
            }
            if ( !d->nepomukOntologyLoader ) {
                d->nepomukOntologyLoader = new NepomukOntologyLoader();
            }

            // we prefer the NepomukOntologyLoader since it does not parse
            // anything
            sl = d->nepomukOntologyLoader->loadOntology( uri );
            if ( sl.isEmpty() ) {
                sl = d->desktopOntologyLoader->loadOntology( uri );
            }
        }
        else {
            sl = d->userOntologyLoader->loadOntology( uri );
        }

        if ( !sl.isEmpty() ) {
            qDebug() << "(Nepomuk::OntologyManager) Found ontology " << uri << endl;
            return constructOntology( uri, sl );
        }
        else {
            // loading failed, i.e. ontology not stored.
            //
            qDebug() << "(Nepomuk::OntologyManager) Could not find ontology " << uri << endl;
            return 0;
        }
    }
}


Nepomuk::Ontology* Nepomuk::OntologyManager::constructOntology( const QUrl& ontoUri, const QList<Statement>& sl )
{
    Ontology& ont = d->ontoCache[ ontoUri ];
    ont.d->uri = ontoUri;

    // FIXME: Is it possible to define classes and properties with different namespaces in one file?
    //        Should we check the namespaces of the resources and if necessary create multiple Ontology
    //        instances?

    // step 1: collect all classes and properties
    for ( QList<Statement>::const_iterator it = sl.constBegin(); it != sl.constEnd(); ++it ) {
        const Statement& s = *it;
        QUrl subjectUri( s.subject().uri() );
        if( s.predicate().uri().toString().endsWith( "#type" ) ) {
            if ( s.object().uri().toString().endsWith( "#Class" ) ) {
                d->classCache.insert( subjectUri, Class() );
                Class* c = &d->classCache[ subjectUri ];
                Entity* entity = static_cast<Entity*>( c );
                entity->d->uri = subjectUri;
                entity->d->ontology = &ont;
                ont.d->classes.insert( subjectUri, c );
            }
            else if ( s.object().uri().toString().endsWith( "#Property" ) ) {
                d->propertyCache.insert( subjectUri, Property() );
                Property* p = &d->propertyCache[ subjectUri ];
                Entity* entity = static_cast<Entity*>( p );
                entity->d->uri = subjectUri;
                entity->d->ontology = &ont;
                ont.d->properties.insert( subjectUri, p );
            }
            else {
                qDebug() << "(OntologyManager) unknown type in ontology: " << s.object().uri();
            }
        }
    }

    // step 2: construct the classes and properties
    for ( QList<Statement>::const_iterator it = sl.constBegin(); it != sl.constEnd(); ++it ) {
        const Statement& s = *it;

        // build the objects in the cache. If the ontology references another ontology try loading that one
        // BIG PROBLEM: reverse dependancies of ontologies.
        // If one ontology depends on another one which in turn depends on the first one. It should work
        // since the first step already went through and thus, the class and property bodies have been created.

        const QUrl& subjectUri = s.subject().uri();
        Class* currentClass = d->findClass( subjectUri );
        Property* currentProperty = ( currentClass ? ( Property* )0 : d->findProperty( subjectUri ) );
        Entity* currentEntity = currentClass;
        if ( !currentEntity ) {
            currentEntity = currentProperty;
        }

        if ( !currentClass && !currentProperty ) {
            qDebug() << "(OntologyManager) invalid ontology statement: " << s;
            d->ontoCache.remove( ontoUri );
            return 0;
        }

        if( s.predicate().uri().toString().endsWith( "#subClassOf" ) ) {
            Class* parent = d->findClass( s.object().uri(), true );
            if ( !parent ) {
                qDebug() << "(OntologyManager) unable to load dependancy for: " << s;
            }
            else {
                currentClass->d->parents.append( parent );
                parent->d->children.append( currentClass );
            }
        }

        if( s.predicate().uri().toString().endsWith( "#subPropertyOf" ) ) {
            Property* parent = d->findProperty( s.object().uri(), true );
            if ( !parent ) {
                qDebug() << "(OntologyManager) unable to load dependancy for: " << s;
            }
            else {
                currentProperty->d->parents.append( parent );
                parent->d->children.append( currentProperty );
            }
        }

        else if( s.predicate().toString().endsWith( "#domain" ) ) {
            Class* domain = d->findClass( s.object().uri(), true );
            if ( !domain ) {
                qDebug() << "(OntologyManager) unable to load dependancy for: " << s;
            }
            else {
                currentProperty->d->domain = domain;
                domain->d->domainOf.insert( subjectUri, currentProperty );
            }
        }

        else if( s.predicate().toString().endsWith( "#range" ) ) {
            if ( s.object().toString().startsWith( Soprano::Vocabulary::XMLSchema::xsdNamespace().toString() ) ) {
                currentProperty->d->literalRange = Literal( s.object().literal().toString() );
            }
            else {
                Class* range = d->findClass( s.object().uri(), true );
                if ( range ) {
                    currentProperty->d->range = range;
                    range->d->rangeOf.insert( subjectUri, currentProperty );
                }
                else {
                    qDebug() << "(OntologyManager) unable to load dependancy for: " << s;
                }
            }
        }

        else if( s.predicate().toString().endsWith( "#minCardinality" ) ) {
            currentProperty->d->minCardinality = s.object().literal().toInt();
        }

        else if( s.predicate().toString().endsWith( "#maxCardinality" ) ) {
            currentProperty->d->maxCardinality = s.object().literal().toInt();
        }

        else if ( s.predicate().toString().endsWith( "#cardinality" ) ) {
            currentProperty->d->cardinality = s.object().literal().toInt();
        }

        else if ( s.predicate().toString().endsWith( "#inverseProperty" ) ) {
            Property* inverse = d->findProperty( s.object().toString(), true );
            if ( !inverse ) {
                qDebug() << "(OntologyManager) failed to load inverse property: " << s;
                d->ontoCache.remove( ontoUri );
                return 0;
            }
            currentProperty->d->inverse = inverse;
            inverse->d->inverse = currentProperty;
        }

        // load l10n'ed comments
        else if( s.predicate() == Soprano::Vocabulary::RDFS::comment() ) {
            if ( s.object().language().isEmpty() ) {
                currentEntity->d->comment = s.object().toString();
            }
            else {
                currentEntity->d->l10nComments[s.object().language()] = s.object().toString();
            }
        }

        // load l10n'ed labels
        else if( s.predicate() == Soprano::Vocabulary::RDFS::label() ) {
            if ( s.object().language().isEmpty() ) {
                currentEntity->d->label = s.object().toString();
            }
            else {
                currentEntity->d->l10nLabels[s.object().language()] = s.object().toString();
            }
        }
    }

    return &ont;
}
