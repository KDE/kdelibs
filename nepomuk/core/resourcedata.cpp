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

#include <Soprano/Statement>
#include <Soprano/StatementIterator>
#include <Soprano/Model>
#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/RDF>

#include "ontology/class.h"

#include <QtCore/QFile>

#include <kdebug.h>
#include <kurl.h>

using namespace Soprano;


typedef QMap<QString, Nepomuk::ResourceData*> ResourceDataHash;

Q_GLOBAL_STATIC( ResourceDataHash, initializedData )
Q_GLOBAL_STATIC( ResourceDataHash, kickoffData )


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


Nepomuk::ResourceData::ResourceData( const QString& uriOrId, const QUrl& type )
    : m_kickoffUriOrId( uriOrId ),
      m_type( type ),
      m_ref(0),
      m_proxyData(0),
      m_cacheDirty(true)
{
    kDebug() << "Creating new ResourceData from uriOrId" << uriOrId;
    if( m_type.isEmpty() && !uriOrId.isEmpty() )
        m_type = Soprano::Vocabulary::RDFS::Resource();
}


Nepomuk::ResourceData::ResourceData( const QUrl& uri, const QUrl& type )
    : m_uri( uri ),
      m_type( type ),
      m_ref(0),
      m_proxyData(0),
      m_cacheDirty(true)
{
    kDebug() << "Creating new ResourceData from uri" << uri;
    if( m_type.isEmpty() && !uri.isEmpty() )
        m_type = Soprano::Vocabulary::RDFS::Resource();
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
    return m_uri.toString();
}


QUrl Nepomuk::ResourceData::type() const
{
    if( m_proxyData )
        return m_proxyData->type();
    return m_type;
}


void Nepomuk::ResourceData::deleteData()
{
    kDebug() << m_uri;
    if( m_proxyData )
        if( m_proxyData->deref() )
            m_proxyData->deleteData();

    m_proxyData = 0;

    if( !m_uri.isEmpty() )
        initializedData()->remove( m_uri.toString() );
    if( !m_kickoffUriOrId.isEmpty() )
        kickoffData()->remove( m_kickoffUriOrId );

    deleteLater();
}


QHash<QString, Nepomuk::Variant> Nepomuk::ResourceData::allProperties()
{
    if( m_proxyData )
        return m_proxyData->allProperties();

    Soprano::Model* model = ResourceManager::instance()->mainModel();
    QHash<QString, Variant> props;

    if ( determineUri() ) {
        Soprano::StatementIterator it = model->listStatements( Soprano::Statement( m_uri, Soprano::Node(), Soprano::Node() ) );
        while ( it.next() ) {
            Statement statement = *it;
            if ( props.contains( statement.predicate().toString() ) ) {
                props[statement.predicate().toString()].append( nodeToVariant( statement.object() ) );
            }
            else {
                props.insert( statement.predicate().toString(), nodeToVariant( statement.object() ) );
            }
        }
        it.close();
    }

    return props;
}


bool Nepomuk::ResourceData::hasProperty( const QString& uri )
{
    if( m_proxyData )
        return m_proxyData->hasProperty( uri );

    if ( determineUri() ) {
        return ResourceManager::instance()->mainModel()->containsAnyStatement( Soprano::Statement( m_uri, QUrl( uri ), Soprano::Node() ) );
    }
    else {
        return false;
    }
}


bool Nepomuk::ResourceData::hasType( const QUrl& uri )
{
    if( m_proxyData )
        return m_proxyData->hasType( uri );

    if ( determineUri() ) {
        return ResourceManager::instance()->mainModel()->containsAnyStatement( Soprano::Statement( m_uri, Soprano::Vocabulary::RDF::type(), QUrl( uri ) ) );
    }
    else {
        return false;
    }
}


Nepomuk::Variant Nepomuk::ResourceData::property( const QString& uri )
{
    if( m_proxyData )
        return m_proxyData->property( uri );

    if ( load() ) {
        QHash<QUrl, Variant>::iterator it = m_cache.find( uri );
        if ( it == m_cache.end() ) {
            return Variant();
        }
        else {
            return *it;
        }
    }
    else {
        return Variant();
    }

//     Variant v;

//     if ( determineUri() ) {
//         Soprano::Model* model = ResourceManager::instance()->mainModel();
//         Soprano::StatementIterator it = model->listStatements( Soprano::Statement( m_uri, QUrl(uri), Soprano::Node() ) );

//         while ( it.next() ) {
//             Statement statement = *it;
//             if ( !v.isValid() ) {
//                 v = nodeToVariant( statement.object() );
//             }
//             else {
//                 v.append( nodeToVariant( statement.object() ) );
//             }
//         }
//         it.close();
//     }
//     return v;
}


bool Nepomuk::ResourceData::store()
{
    if ( determineUri() ) {
        if ( !exists() ) {
            QList<Statement> statements;

            // save type
            // FIXME: handle multiple types
            statements.append( Statement( m_uri, Soprano::Vocabulary::RDF::type(), m_type ) );

            // save the kickoff identifier (other identifiers are stored via setProperty)
            if ( !m_kickoffIdentifier.isEmpty() ) {
                statements.append( Statement( m_uri, QUrl(Resource::identifierUri()), LiteralValue(m_kickoffIdentifier) ) );
            }

            // HACK: make sure that files have proper fileUrl properties so long as we do not have a File class for
            // Dolphin and co.
            if ( QFile::exists( m_uri.toLocalFile())) {
                statements.append( Statement( m_uri,
                                              QUrl("http://freedesktop.org/standards/xesam/1.0/core#url"),
                                              LiteralValue( m_uri.toLocalFile() ) ) );
            }

            return ResourceManager::instance()->mainModel()->addStatements( statements ) == Soprano::Error::ErrorNone;
        }
        else {
            return true;
        }
    }
    else {
        return false;
    }
}


bool Nepomuk::ResourceData::load()
{
    if ( m_cacheDirty ) {
        m_cache.clear();

        if ( determineUri() ) {
            kDebug();
            Soprano::Model* model = ResourceManager::instance()->mainModel();
            Soprano::StatementIterator it = model->listStatements( Soprano::Statement( m_uri, Soprano::Node(), Soprano::Node() ) );

            while ( it.next() ) {
                Statement statement = *it;
                if ( statement.predicate().uri() != Soprano::Vocabulary::RDF::type() ) {
                    m_cache.insert( statement.predicate().uri(), nodeToVariant( statement.object() ) );
                }
            }

            m_cacheDirty = false;
            return true;
        }
        else {
            return false;
        }
    }
    else {
        return true;
    }
}


void Nepomuk::ResourceData::setProperty( const QString& uri, const Nepomuk::Variant& value )
{
    if( m_proxyData )
        return m_proxyData->setProperty( uri, value );

    // step 0: make sure this resource is in the store
    if ( store() ) {
        Soprano::Model* model = ResourceManager::instance()->mainModel();

        // step 1: remove all the existing stuff
        model->removeAllStatements( Statement( m_uri, QUrl(uri), Node() ) );

        // step 2: make sure resource values are in the store
        if ( value.simpleType() == qMetaTypeId<Resource>() ) {
            QList<Resource> l = value.toResourceList();
            for( QList<Resource>::iterator resIt = l.begin(); resIt != l.end(); ++resIt ) {
                (*resIt).m_data->store();
            }
        }

        // step 3: add the actual property statements

        // one-to-one Resource
        if( value.isResource() ) {
            model->addStatement( Statement( m_uri, QUrl(uri), QUrl( value.toResource().uri() ) ) );
        }

        // one-to-many Resource
        else if( value.isResourceList() ) {
            const QList<Resource>& l = value.toResourceList();
            for( QList<Resource>::const_iterator resIt = l.constBegin(); resIt != l.constEnd(); ++resIt ) {
                model->addStatement( Statement( m_uri, QUrl(uri), QUrl( (*resIt).uri() ) ) );
            }
        }

        // one-to-many literals
        else if( value.isList() ) {
            QList<Node> nl = Nepomuk::valuesToRDFNodes( value );
            for( QList<Node>::const_iterator nIt = nl.constBegin(); nIt != nl.constEnd(); ++nIt ) {
                model->addStatement( Statement( m_uri, QUrl(uri), *nIt ) );
            }
        }

        // one-to-one literal
        else {
            model->addStatement( Statement( m_uri, QUrl(uri),
                                            Nepomuk::valueToRDFNode( value ) ) );
        }

        m_cacheDirty = true;
    }
}


void Nepomuk::ResourceData::removeProperty( const QString& uri )
{
    if( m_proxyData )
        return m_proxyData->removeProperty( uri );

    if ( determineUri() ) {
        ResourceManager::instance()->mainModel()->removeAllStatements( Statement( m_uri, QUrl(uri), Node() ) );
    }
}


void Nepomuk::ResourceData::remove( bool recursive )
{
    if( m_proxyData )
        return m_proxyData->remove();

    if ( determineUri() ) {
        Soprano::Model* model = ResourceManager::instance()->mainModel();
        model->removeAllStatements( Statement( m_uri, Node(), Node() ) );
        if ( recursive ) {
            model->removeAllStatements( Statement( Node(), Node(), m_uri ) );
        }
    }
}


bool Nepomuk::ResourceData::exists()
{
    if( m_proxyData )
        return m_proxyData->exists();

    if( determineUri() ) {
        return ResourceManager::instance()->mainModel()->containsAnyStatement( Statement( m_uri, Node(), Node() ) );
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

        Soprano::Model* model = ResourceManager::instance()->mainModel();

        if( model->containsAnyStatement( Statement( QUrl( kickoffUriOrId() ), Node(), Node() ) ) ) {
            //
            // The kickoffUriOrId is actually a URI
            //
            m_uri = kickoffUriOrId();
            kDebug(300004) << " kickoff identifier " << kickoffUriOrId() << " exists as a URI " << uri();
            updateType();
        }
        else {
            //
            // Check if the kickoffUriOrId is a resource identifier
            //
            StatementIterator it = model->listStatements( Statement( Node(),
                                                                     Node(QUrl( Resource::identifierUri() )),
                                                                     LiteralValue( kickoffUriOrId() ) ) );

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
            if ( it.next() ) {
                const Nepomuk::Class* wantedType = Nepomuk::Class::load( m_type );
                if ( wantedType && m_type != Soprano::Vocabulary::RDFS::Resource() ) {
                    do {
                        // get the type of the stored resource
                        StatementIterator resourceSl = model->listStatements( Statement( it.current().subject(),
                                                                                         Soprano::Vocabulary::RDF::type(),
                                                                                         Node() ) );
                        if ( resourceSl.next() && resourceSl.current().object().isResource() ) {
                            const Nepomuk::Class* storedType = Nepomuk::Class::load( resourceSl.current().object().uri() );
                            if ( storedType ) {
                                if ( storedType == wantedType ) {
                                    // great. :)
                                    m_uri = it.current().subject().uri();
                                }
                                else if ( wantedType->isSubClassOf( storedType ) ) {
                                    // Keep the type that is further down the hierarchy
                                    m_type = wantedType->uri();
                                    m_uri = it.current().subject().uri();
                                    break;
                                }
                                else if ( storedType->isSubClassOf( wantedType ) ) {
                                    // just use the existing data with the finer grained type
                                    m_uri = it.current().subject().uri();
                                    break;
                                }
                            }
                        }
                    } while ( it.next() );
                }
                else {
                    m_uri = it.current().subject().uri();
                    kDebug(300004) << k_funcinfo << " kickoff identifier " << kickoffUriOrId() << " already exists with URI " << uri();
                    updateType();
                }
            }

            it.close();

            if ( m_uri.isEmpty() ) {
                //
                // The resource does not exist, create a new one:
                // If the kickoffUriOrId is a valid URI we use it as such, otherwise we create a new URI
                // Special case: files: paths are always converted to URIs
                //
                QUrl uri( kickoffUriOrId() );
                if ( uri.isValid() && !uri.scheme().isEmpty() ) {
                    m_uri = uri;
                }
                else if ( QFile::exists( kickoffUriOrId() ) ) {
                    // KURL defaults to schema "file:"
                    m_uri = KUrl::fromPath( kickoffUriOrId() );
                }
                else {
                    m_kickoffIdentifier = kickoffUriOrId();
                    m_uri = ResourceManager::instance()->generateUniqueUri();
                }

                kDebug(300004) << " kickoff identifier " << kickoffUriOrId() << " seems fresh. Generated new URI " << m_uri;
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
    Soprano::Model* model = ResourceManager::instance()->mainModel();

    // get the type of the stored resource
    StatementIterator typeStatements = model->listStatements( Statement( m_uri,
                                                                         Soprano::Vocabulary::RDF::type(),
                                                                         Node() ) );
    if ( typeStatements.next() && typeStatements.current().object().isResource() ) {
        // FIXME: handle multple types, maybe select the one type that fits best
        QUrl storedType = typeStatements.current().object().uri();
        if ( m_type == Soprano::Vocabulary::RDFS::Resource() ) {
            m_type = storedType;
        }
        else {
            const Nepomuk::Class* wantedTypeClass = Nepomuk::Class::load( m_type );
            const Nepomuk::Class* storedTypeClass = Nepomuk::Class::load( storedType );

            if ( wantedTypeClass && storedTypeClass ) {
                // Keep the type that is further down the hierarchy
                if ( wantedTypeClass->isSubClassOf( storedTypeClass ) ) {
                    m_type = wantedTypeClass->uri();
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
        kDebug(300004) << "different uri or type";
        return false;
    }

    return true;
}


Nepomuk::ResourceData* Nepomuk::ResourceData::data( const QUrl& uri, const QUrl& type )
{
    Q_ASSERT( !uri.isEmpty() );

    // default to "file" scheme, i.e. we do not allow an empty scheme
    if ( uri.scheme().isEmpty() ) {
        QUrl fileUri( uri );
        fileUri.setScheme( "file" );
        return data( fileUri, type );
    }

    ResourceDataHash::iterator it = initializedData()->find( uri.toString() );

    //
    // The uriOrId has no local representation yet -> create one
    //
    if( it == initializedData()->end() ) {
        kDebug(300004) << "No existing ResourceData instance found for uri " << uri;
        //
        // The actual URI is already known here
        //
        ResourceData* d = new ResourceData( uri, type );
        initializedData()->insert( uri.toString(), d );

        return d;
    }
    else {
        //
        // Reuse the already existing ResourceData object
        //
        return it.value();
    }
}


Nepomuk::ResourceData* Nepomuk::ResourceData::data( const QString& uriOrId, const QUrl& type )
{
    Q_ASSERT( !uriOrId.isEmpty() );

    // special case: files (only absolute paths for now)
    if ( uriOrId[0] == '/' ) {
        ResourceDataHash::iterator it = initializedData()->find( "file://" + uriOrId );
        if ( it != initializedData()->end() ) {
            return *it;
        }
    }

    ResourceDataHash::iterator it = initializedData()->find( uriOrId );

    bool resFound = ( it != initializedData()->end() );

    //
    // The uriOrId is not a known local URI. Might be a kickoff value though
    //
    if( it == initializedData()->end() ) {
        it = kickoffData()->find( uriOrId );

        // check if the type matches (see determineUri for details)
        if ( !type.isEmpty() && type != Soprano::Vocabulary::RDFS::Resource() ) {
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
        kDebug(300004) << "No existing ResourceData instance found for uriOrId " << uriOrId;
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
    for( ResourceDataHash::iterator rdIt = initializedData()->begin();
         rdIt != initializedData()->end(); ++rdIt ) {
        l.append( rdIt.value() );
    }

    return l;
}

#include "resourcedata.moc"
