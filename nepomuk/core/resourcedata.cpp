/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "resourcedata.h"
#include "resourcemanager.h"
#include "tools.h"
#include "generated/resource.h"

#include "rdfrepository.h"
#include "statementlistiterator.h"
#include "registry.h"

#include <soprano/statement.h>

#include "ontology/class.h"

#include <kdebug.h>


using namespace Nepomuk::RDF;
using namespace Nepomuk::Services;
using namespace Soprano;


typedef QMap<QString, Nepomuk::ResourceData*> ResourceDataHash;

Q_GLOBAL_STATIC( ResourceDataHash, initializedData )
Q_GLOBAL_STATIC( ResourceDataHash, kickoffData )

static const char* s_defaultType = "http://www.w3.org/2000/01/rdf-schema#Resource";

static Nepomuk::Variant nodeToVariant( const Soprano::Node& node )
{
    if ( node.isResource() ) {
        return Nepomuk::Variant( Nepomuk::Resource( node.toString() ) );
    }
    else if ( node.isLiteral() ) {
        return Nepomuk::Variant( node.literal().variant() );
    }
    else {
        return Nepomuk::Variant();
    }
}


Nepomuk::ResourceData::ResourceData( const QString& uriOrId, const QString& type_ )
    : m_kickoffUriOrId( uriOrId ),
      m_type( type_ ),
      m_ref(0),
      m_proxyData(0)
{
    if( m_type.isEmpty() && !uriOrId.isEmpty() )
        m_type = s_defaultType;
}


Nepomuk::ResourceData::~ResourceData()
{
    if( m_proxyData )
        m_proxyData->deref();
}


QString Nepomuk::ResourceData::kickoffUriOrId() const
{
    if( m_proxyData )
        return m_proxyData->kickoffUriOrId();
    return m_kickoffUriOrId;
}


QString Nepomuk::ResourceData::uri() const
{
    if( m_proxyData )
        return m_proxyData->uri();
    return m_uri;
}


QString Nepomuk::ResourceData::type() const
{
    if( m_proxyData )
        return m_proxyData->type();
    return m_type;
}


void Nepomuk::ResourceData::deleteData()
{
    if( m_proxyData )
        if( m_proxyData->deref() )
            m_proxyData->deleteData();

    m_proxyData = 0;

    if( !m_uri.isEmpty() )
        initializedData()->remove( m_uri );
    if( !m_kickoffUriOrId.isEmpty() )
        kickoffData()->remove( m_kickoffUriOrId );

    delete this;
}


QHash<QString, Nepomuk::Variant> Nepomuk::ResourceData::allProperties()
{
    if( m_proxyData )
        return m_proxyData->allProperties();

    RDFRepository rr( ResourceManager::instance()->serviceRegistry()->discoverRDFRepository() );
    QHash<QString, Variant> props;

    if ( determineUri() ) {
        QList<Statement> sl = rr.listStatements( Nepomuk::defaultGraph(), Statement( QUrl(m_uri), Node(), Node() ) );
        QList<Statement>::const_iterator endIt( sl.constEnd() );
        for( QList<Statement>::const_iterator it = sl.constBegin(); it != endIt; ++it ) {
            const Statement& statement = *it;
            if ( props.contains( statement.predicate().toString() ) ) {
                props[statement.predicate().toString()].append( nodeToVariant( statement.object() ) );
            }
            else {
                props.insert( statement.predicate().toString(), nodeToVariant( statement.object() ) );
            }
        }
    }

    return props;
}


bool Nepomuk::ResourceData::hasProperty( const QString& uri )
{
    if( m_proxyData )
        return m_proxyData->hasProperty( uri );

    if ( determineUri() ) {
        RDFRepository rr( ResourceManager::instance()->serviceRegistry()->discoverRDFRepository() );
        return rr.contains( Nepomuk::defaultGraph(), Statement( QUrl( m_uri ), QUrl( uri ), Node() ) );
    }
    else {
        return false;
    }
}


Nepomuk::Variant Nepomuk::ResourceData::property( const QString& uri )
{
    if( m_proxyData )
        return m_proxyData->property( uri );

    Variant v;

    if ( determineUri() ) {
        RDFRepository rr( ResourceManager::instance()->serviceRegistry()->discoverRDFRepository() );
        QList<Statement> sl = rr.listStatements( Nepomuk::defaultGraph(), Statement( QUrl(m_uri), QUrl(uri), Node() ) );

        QList<Statement>::const_iterator endIt( sl.constEnd() );
        for( QList<Statement>::const_iterator it = sl.constBegin(); it != endIt; ++it ) {
            const Statement& statement = *it;
            if ( !v.isValid() ) {
                v = nodeToVariant( statement.object() );
            }
            else {
                v.append( nodeToVariant( statement.object() ) );
            }
        }
    }
    return v;
}


bool Nepomuk::ResourceData::store()
{
    if ( determineUri() ) {
        if ( !exists() ) {
            RDFRepository rr( ResourceManager::instance()->serviceRegistry()->discoverRDFRepository() );

            if( !rr.listRepositoryIds().contains( Nepomuk::defaultGraph() ) )
                rr.createRepository( Nepomuk::defaultGraph() );

            QList<Statement> statements;

            // save type
            // FIXME: handle multiple types
            statements.append( Statement( QUrl(m_uri), QUrl(Nepomuk::typePredicate()), QUrl(m_type) ) );

            // save the kickoff identifier (other identifiers are stored via setProperty)
            if ( !m_kickoffIdentifier.isEmpty() ) {
                statements.append( Statement( QUrl(m_uri), QUrl(Resource::identifierUri()), LiteralValue(m_kickoffIdentifier) ) );
            }

            rr.addStatements( Nepomuk::defaultGraph(), statements );
            return rr.success();
        }
        else {
            return true;
        }
    }
    else {
        return false;
    }
}


void Nepomuk::ResourceData::setProperty( const QString& uri, const Nepomuk::Variant& value )
{
    if( m_proxyData )
        return m_proxyData->setProperty( uri, value );

    // step 0: make sure this resource is in the store
    if ( store() ) {
        RDFRepository rr( ResourceManager::instance()->serviceRegistry()->discoverRDFRepository() );

        if( !rr.listRepositoryIds().contains( Nepomuk::defaultGraph() ) )
            rr.createRepository( Nepomuk::defaultGraph() );

        // step 1: remove all the existing stuff
        rr.removeAllStatements( Nepomuk::defaultGraph(), Statement( QUrl(m_uri), QUrl(uri), Node() ) );

        // step 2: make sure resource values are in the store
        if ( value.simpleType() == qMetaTypeId<Resource>() ) {
            QList<Resource> l = value.toResourceList();
            for( QList<Resource>::iterator resIt = l.begin(); resIt != l.end(); ++resIt ) {
                (*resIt).m_data->store();
            }
        }

        // step 3: add the actual property statements
        QList<Statement> statements;

        // one-to-one Resource
        if( value.isResource() ) {
            statements.append( Statement( QUrl(m_uri), QUrl(uri), QUrl( value.toResource().uri() ) ) );
        }

        // one-to-many Resource
        else if( value.isResourceList() ) {
            const QList<Resource>& l = value.toResourceList();
            for( QList<Resource>::const_iterator resIt = l.constBegin(); resIt != l.constEnd(); ++resIt ) {
                statements.append( Statement( QUrl(m_uri), QUrl(uri), QUrl( (*resIt).uri() ) ) );
            }
        }

        // one-to-many literals
        else if( value.isList() ) {
            QList<Node> nl = Nepomuk::valuesToRDFNodes( value );
            for( QList<Node>::const_iterator nIt = nl.constBegin(); nIt != nl.constEnd(); ++nIt ) {
                statements.append( Statement( QUrl(m_uri), QUrl(uri), *nIt ) );
            }
        }

        // one-to-one literal
        else {
            statements.append( Statement( QUrl(m_uri), QUrl(uri),
                                          Nepomuk::valueToRDFNode( value ) ) );
        }

        rr.addStatements( Nepomuk::defaultGraph(), statements );
    }
}


void Nepomuk::ResourceData::removeProperty( const QString& uri )
{
    if( m_proxyData )
        return m_proxyData->removeProperty( uri );

    if ( determineUri() ) {
        RDFRepository rr( ResourceManager::instance()->serviceRegistry()->discoverRDFRepository() );
        rr.removeAllStatements( Nepomuk::defaultGraph(), Statement( QUrl(m_uri), QUrl(uri), Node() ) );
    }
}


void Nepomuk::ResourceData::remove( bool recursive )
{
    if( m_proxyData )
        return m_proxyData->remove();

    if ( determineUri() ) {
        RDFRepository rr( ResourceManager::instance()->serviceRegistry()->discoverRDFRepository() );
        rr.removeAllStatements( Nepomuk::defaultGraph(), Statement( QUrl(m_uri), Node(), Node() ) );
        if ( recursive ) {
            rr.removeAllStatements( Nepomuk::defaultGraph(), Statement( Node(), Node(), QUrl(m_uri) ) );
        }
    }
}


bool Nepomuk::ResourceData::exists()
{
    if( m_proxyData )
        return m_proxyData->exists();

    if( determineUri() ) {
        RDFRepository rr( ResourceManager::instance()->serviceRegistry()->discoverRDFRepository() );
        return rr.contains( Nepomuk::defaultGraph(), Statement( QUrl( m_uri ), Node(), Node() ) );
    }
    else
        return false;
}


bool Nepomuk::ResourceData::isValid() const
{
    if( m_proxyData )
        return m_proxyData->isValid();

    // FIXME: check namespaces and stuff
    return( !m_type.isEmpty() );
}


bool Nepomuk::ResourceData::determineUri()
{
    if( m_proxyData )
        return m_proxyData->determineUri();

    if( m_uri.isEmpty() ) {
        Q_ASSERT( !m_kickoffUriOrId.isEmpty() );

        m_modificationMutex.lock();

        RDFRepository rr( ResourceManager::instance()->serviceRegistry()->discoverRDFRepository() );

        if( rr.contains( Nepomuk::defaultGraph(), Statement( QUrl( kickoffUriOrId() ), Node(), Node() ) ) ) {
            //
            // The kickoffUriOrId is actually a URI
            //
            m_uri = kickoffUriOrId();
            kDebug(300004) << k_funcinfo << " kickoff identifier " << kickoffUriOrId() << " exists as a URI " << uri() << endl;
            updateType();
        }
        else {
            //
            // Check if the kickoffUriOrId is a resource identifier
            //
            QList<Statement> sl = rr.listStatements( Nepomuk::defaultGraph(),
                                                     Statement( Node(),
                                                                Node(QUrl( Resource::identifierUri() )),
                                                                LiteralValue( kickoffUriOrId() ) ) );

            if( !sl.isEmpty() ) {
                //
                // The kickoffUriOrId is an identifier
                //
                // Identifiers are not unique!
                // Thus, we do the following:
                // - If we have Ontology support, i.e. we know the classes:
                //   We reuse a resource if its type is derived from the
                //   current type of the other way around (example: an ImageFile
                //   is a File and if we open an ImageFile as File we do want it to
                //   keep its ImageFile type)
                // - If we do not have Ontology support:
                //   Fallback to the default behaviour of always reusing existing
                //   data.
                //
                // TODO: basically it is perfectly valid to store both types in the first case
                //
                Q_ASSERT( !m_type.isEmpty() );
                const Nepomuk::Class* wantedType = Nepomuk::Class::load( m_type );
                if ( wantedType && m_type != s_defaultType ) {
                    for ( QList<Statement>::const_iterator it = sl.constBegin(); it != sl.constEnd(); ++it ) {
                        // get the type of the stored resource
                        QList<Statement> resourceSl = rr.listStatements( Nepomuk::defaultGraph(),
                                                                         Statement( it->subject(),
                                                                                    QUrl( typePredicate() ),
                                                                                    Node() ) );
                        if ( !resourceSl.isEmpty() ) {
                            const Nepomuk::Class* storedType = Nepomuk::Class::load( resourceSl.first().object().uri() );
                            if ( storedType ) {
                                if ( storedType == wantedType ) {
                                    // great. :)
                                    m_uri = it->subject().toString();
                                }
                                else if ( wantedType->isSubClassOf( storedType ) ) {
                                    // Keep the type that is further down the hierarchy
                                    m_type = wantedType->uri().toString();
                                    m_uri = it->subject().toString();
                                    break;
                                }
                                else if ( storedType->isSubClassOf( wantedType ) ) {
                                    // just use the existing data with the finer grained type
                                    m_uri = it->subject().toString();
                                    break;
                                }
                            }
                        }
                    }

                    if ( m_uri.isEmpty() ) {
                        m_kickoffIdentifier = kickoffUriOrId();
                        m_uri = ResourceManager::instance()->generateUniqueUri();
                        kDebug(300004) << k_funcinfo << " kickoff identifier " << kickoffUriOrId() << " already used as identifier with incompatible type. Generated new URI " << uri() << endl;
                    }
                }
                else {
                    m_uri = sl.first().subject().toString();
                    kDebug(300004) << k_funcinfo << " kickoff identifier " << kickoffUriOrId() << " already exists with URI " << uri() << endl;
                    updateType();
                }
            }
            else {
                //
                // The resource does not exist, create a new one
                //
                m_kickoffIdentifier = kickoffUriOrId();
                m_uri = ResourceManager::instance()->generateUniqueUri();
                kDebug(300004) << k_funcinfo << " kickoff identifier " << kickoffUriOrId() << " seems fresh. Generated new URI " << uri() << endl;
            }
        }

        //
        // Move us to the final data hash now that the URI is known
        //
        if( !uri().isEmpty() && uri() != kickoffUriOrId() ) {
            if( !initializedData()->contains( uri() ) )
                initializedData()->insert( uri(), this );
            else {
                m_proxyData = initializedData()->value( uri() );
                m_proxyData->ref();
            }
        }

        m_modificationMutex.unlock();

        return !uri().isEmpty();
    }
    else
        return true;
}


void Nepomuk::ResourceData::updateType()
{
    RDFRepository rr( ResourceManager::instance()->serviceRegistry()->discoverRDFRepository() );

    // get the type of the stored resource
    QList<Statement> typeStatements = rr.listStatements( Nepomuk::defaultGraph(),
                                                         Statement( QUrl( m_uri ),
                                                                    QUrl( typePredicate() ),
                                                                    Node() ) );
    if ( !typeStatements.isEmpty() ) {
        // FIXME: handle multple types, maybe select the one type that fits best
        QString storedType = typeStatements.first().object().toString();
        if ( m_type == s_defaultType ) {
            m_type = storedType;
        }
        else {
            const Nepomuk::Class* wantedTypeClass = Nepomuk::Class::load( m_type );
            const Nepomuk::Class* storedTypeClass = Nepomuk::Class::load( storedType );

            if ( wantedTypeClass && storedTypeClass ) {
                // Keep the type that is further down the hierarchy
                if ( wantedTypeClass->isSubClassOf( storedTypeClass ) ) {
                    m_type = wantedTypeClass->uri().toString();
                }
            }
        }
    }
}


bool Nepomuk::ResourceData::operator==( const ResourceData& other ) const
{
    const ResourceData* that = this;
    if( m_proxyData )
        that = m_proxyData;

    if( that == &other )
        return true;

    if( that->m_uri != other.m_uri ||
        that->m_type != other.m_type ) {
        kDebug(300004) << k_funcinfo << "different uri or type" << endl;
        return false;
    }

    return true;
}


Nepomuk::ResourceData* Nepomuk::ResourceData::data( const QString& uriOrId, const QString& type )
{
    Q_ASSERT( !uriOrId.isEmpty() );

    ResourceDataHash::iterator it = initializedData()->find( uriOrId );

    bool resFound = ( it != initializedData()->end() );

    //
    // The uriOrId is not a known local URI. Might be a kickoff value though
    //
    if( it == initializedData()->end() ) {
        it = kickoffData()->find( uriOrId );

        // check if the type matches (see determineUri for details)
        if ( !type.isEmpty() && type != s_defaultType ) {
            const Nepomuk::Class* wantedType = Nepomuk::Class::load( type );
            if ( wantedType ) {
                while ( it != kickoffData()->end() &&
                        it.key() == uriOrId ) {
                    const Nepomuk::Class* storedType = Nepomuk::Class::load( it.value()->type() );
                    if ( storedType ) {
                        if ( storedType->isSubClassOf( wantedType ) ) {
                            break;
                        }
                        else if ( wantedType->isSubClassOf( storedType ) ) {
                            it.value()->m_type = type;
                            break;
                        }
                    }
                    ++it;
                }
            }
        }

        resFound = ( it != kickoffData()->end() && it.key() == uriOrId );
    }

    //
    // The uriOrId has no local representation yet -> create one
    //
    if( !resFound ) {
        kDebug(300004) << "No existing ResourceData instance found for uriOrId " << uriOrId << endl;
        //
        // Every new ResourceData object ends up in the kickoffdata since its actual URI is not known yet
        //
        ResourceData* d = new ResourceData( uriOrId, type );
        kickoffData()->insert( uriOrId, d );

        return d;
    }
    else {
        //
        // Reuse the already existing ResourceData object
        //
        return it.value();
    }
}


QList<Nepomuk::ResourceData*> Nepomuk::ResourceData::allResourceDataOfType( const QString& type )
{
    QList<ResourceData*> l;

    if( !type.isEmpty() ) {
        for( ResourceDataHash::iterator rdIt = kickoffData()->begin();
             rdIt != kickoffData()->end(); ++rdIt ) {
            if( rdIt.value()->type() == type ) {
                l.append( rdIt.value() );
            }
        }
    }

    return l;
}


QList<Nepomuk::ResourceData*> Nepomuk::ResourceData::allResourceDataWithProperty( const QString& _uri, const Variant& v )
{
    QList<ResourceData*> l;

    for( ResourceDataHash::iterator rdIt = kickoffData()->begin();
         rdIt != kickoffData()->end(); ++rdIt ) {

        if( rdIt.value()->hasProperty( _uri ) &&
            rdIt.value()->property( _uri ) == v ) {
            l.append( rdIt.value() );
        }
    }

    return l;
}


QList<Nepomuk::ResourceData*> Nepomuk::ResourceData::allResourceData()
{
    QList<ResourceData*> l;

    for( ResourceDataHash::iterator rdIt = kickoffData()->begin();
         rdIt != kickoffData()->end(); ++rdIt ) {
        l.append( rdIt.value() );
    }

    return l;
}
