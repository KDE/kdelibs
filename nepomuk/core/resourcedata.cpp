/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "resourcedata.h"
#include "resourcemanager.h"
#include "resource.h"
#include "tools.h"
#include "resourcefiltermodel.h"
#include "nie.h"

#include <Soprano/Statement>
#include <Soprano/StatementIterator>
#include <Soprano/Model>
#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/NAO>

#include "ontology/class.h"
#include "ontology/entity_p.h" // for qHash(QUrl)

#include <QtCore/QFile>
#include <QtCore/QDateTime>

#include <kdebug.h>
#include <kurl.h>

using namespace Soprano;

typedef QHash<QString, Nepomuk::ResourceData*> ResourceDataHash;

// FIXME: make these things thread-safe. Even better: put them into the ResourceManager and make that thread-safe!
Q_GLOBAL_STATIC( ResourceDataHash, initializedData )
Q_GLOBAL_STATIC( ResourceDataHash, kickoffData )


static int s_dataCnt = 0;


static Nepomuk::Variant nodeToVariant( const Soprano::Node& node )
{
    if ( node.isResource() ) {
        return Nepomuk::Variant( Nepomuk::Resource( node.uri() ) );
    }
    else if ( node.isLiteral() ) {
        return Nepomuk::Variant( node.literal().variant() );
    }
    else {
        return Nepomuk::Variant();
    }
}


Nepomuk::ResourceData::ResourceData( const QUrl& uri, const QString& uriOrId, const QUrl& type )
    : m_kickoffUriOrId( uriOrId ),
      m_uri( uri ),
      m_mainType( type ),
      m_ref(0),
      m_proxyData(0),
      m_cacheDirty(true)
{
    if( m_mainType.isEmpty() )
        m_mainType = Soprano::Vocabulary::RDFS::Resource();

    m_types << m_mainType;

    // TODO: handle the caching in a decent Cache class and not this ugly.
    if ( s_dataCnt >= 1000 ) {
        for( ResourceDataHash::iterator rdIt = initializedData()->begin();
             rdIt != initializedData()->end(); ++rdIt ) {
            ResourceData* data = rdIt.value();
            if ( !data->cnt() ) {
                data->deleteData();
                break;
            }
        }
    }

    ++s_dataCnt;
}


Nepomuk::ResourceData::~ResourceData()
{
    --s_dataCnt;
}


QString Nepomuk::ResourceData::kickoffUriOrId() const
{
    if( m_proxyData )
        return m_proxyData->kickoffUriOrId();
    return m_kickoffUriOrId;
}


QUrl Nepomuk::ResourceData::uri() const
{
    if( m_proxyData )
        return m_proxyData->uri();
    return m_uri;
}


QUrl Nepomuk::ResourceData::type()
{
    if( m_proxyData )
        return m_proxyData->type();
    load();
    return m_mainType;
}


QList<QUrl> Nepomuk::ResourceData::allTypes()
{
    if( m_proxyData )
        return m_proxyData->allTypes();
    load();
    return m_types;
}


void Nepomuk::ResourceData::deleteData()
{
    if( m_proxyData ) {
        m_proxyData->deref();
        m_proxyData = 0;
    }
    else {
        if( !m_uri.isEmpty() )
            initializedData()->remove( m_uri.toString() );
        if( !m_kickoffUriOrId.isEmpty() )
            kickoffData()->remove( m_kickoffUriOrId );
    }

    deleteLater();
}


QHash<QUrl, Nepomuk::Variant> Nepomuk::ResourceData::allProperties()
{
    if( m_proxyData )
        return m_proxyData->allProperties();

    load();

    return m_cache;
}


bool Nepomuk::ResourceData::hasProperty( const QUrl& uri )
{
    if( m_proxyData )
        return m_proxyData->hasProperty( uri );

    if ( determineUri() ) {
        return ResourceManager::instance()->mainModel()->containsAnyStatement( Soprano::Statement( m_uri, uri, Soprano::Node() ) );
    }
    else {
        return false;
    }
}


bool Nepomuk::ResourceData::hasType( const QUrl& uri )
{
    if( m_proxyData )
        return m_proxyData->hasType( uri );

    if ( load() ) {
        return constHasType( uri );
    }

    return false;
}


bool Nepomuk::ResourceData::constHasType( const QUrl& uri ) const
{
    Types::Class requestedType( uri );
    for ( QList<QUrl>::const_iterator it = m_types.constBegin();
          it != m_types.constEnd(); ++it ) {
        Types::Class availType( *it );
        if ( availType == requestedType ||
             availType.isSubClassOf( requestedType ) ) {
            return true;
        }
    }
    return false;
}


Nepomuk::Variant Nepomuk::ResourceData::property( const QUrl& uri )
{
    if( m_proxyData )
        return m_proxyData->property( uri );

    if ( load() ) {
        QHash<QUrl, Variant>::const_iterator it = m_cache.find( uri );
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
//             v.append( nodeToVariant( statement.object() ) );
//         }
//         it.close();
//     }
//     return v;
}


bool Nepomuk::ResourceData::store()
{
    if ( !determineUri() ) {
        // create a random URI and add us to the initialized data, i.e. make us "valid"
        m_modificationMutex.lock();
        m_uri = ResourceManager::instance()->generateUniqueUri();
        initializedData()->insert( m_uri.toString(), this );
        m_modificationMutex.unlock();
    }

    if ( !exists() ) {
        QList<Statement> statements;

        // save type (There should be no need to save all the types since there is only one way
        // that m_types contains more than one element: if we loaded them)
        statements.append( Statement( m_uri, Soprano::Vocabulary::RDF::type(), m_types.first() ) );

        // save the creation date
        statements.append( Statement( m_uri, Soprano::Vocabulary::NAO::created(), Soprano::LiteralValue( QDateTime::currentDateTime() ) ) );

        // save the kickoff identifier (other identifiers are stored via setProperty)
        if ( !m_kickoffIdentifier.isEmpty() ) {
            statements.append( Statement( m_uri, QUrl(Resource::identifierUri()), LiteralValue(m_kickoffIdentifier) ) );
        }

        // HACK: make sure that files have proper fileUrl properties so long as we do not have a File class for
        // Dolphin and co.
        if ( constHasType( Soprano::Vocabulary::Xesam::File() ) &&
             QFile::exists( m_uri.toLocalFile()) ) {
            statements.append( Statement( m_uri,
                                          Soprano::Vocabulary::Xesam::url(),
                                          LiteralValue( m_uri.toLocalFile() ) ) );
        }

        ResourceFilterModel fm( ResourceManager::instance()->mainModel() );
        return fm.addStatements( statements ) == Soprano::Error::ErrorNone;
    }
    else {
        return true;
    }
}


bool Nepomuk::ResourceData::load()
{
    if ( m_cacheDirty ) {
        m_cache.clear();

        if ( determineUri() ) {
            Soprano::Model* model = ResourceManager::instance()->mainModel();
            Soprano::StatementIterator it = model->listStatements( Soprano::Statement( m_uri, Soprano::Node(), Soprano::Node() ) );

            while ( it.next() ) {
                Statement statement = *it;
                if ( statement.predicate().uri() == Soprano::Vocabulary::RDF::type() ) {
                    if ( statement.object().isResource() ) {
                        QUrl storedType = statement.object().uri();
                        if ( !m_types.contains( storedType ) ) {
                            m_types << storedType;
                        }
                        if ( m_mainType == Soprano::Vocabulary::RDFS::Resource() ) {
                            Q_ASSERT( !storedType.isEmpty() );
                            m_mainType = storedType;
                        }
                        else {
                            Types::Class currentTypeClass = m_mainType;
                            Types::Class storedTypeClass = storedType;

                            // Keep the type that is further down the hierarchy
                            if ( storedTypeClass.isSubClassOf( currentTypeClass ) ) {
                                m_mainType = storedTypeClass.uri();
                            }
                            else {
                                // This is a little convenience hack since the user is most likely
                                // more interested in the file content than the actual file
                                Types::Class xesamContentClass( Soprano::Vocabulary::Xesam::Content() );
                                if ( m_mainType == Soprano::Vocabulary::Xesam::File() &&
                                     ( storedTypeClass == xesamContentClass ||
                                       storedTypeClass.isSubClassOf( xesamContentClass ) ) ) {
                                    m_mainType = storedTypeClass.uri();
                                }
                                else {
                                    // the same is true for nie:DataObject vs. nie:InformationElement
                                    Types::Class nieInformationElementClass( Vocabulary::NIE::InformationElement() );
                                    Types::Class nieDataObjectClass( Vocabulary::NIE::DataObject() );
                                    if( ( currentTypeClass == nieDataObjectClass ||
                                          currentTypeClass.isSubClassOf( nieDataObjectClass ) ) &&
                                        ( storedTypeClass == nieInformationElementClass ||
                                          storedTypeClass.isSubClassOf( nieInformationElementClass ) ) ) {
                                        m_mainType = storedTypeClass.uri();
                                    }
                                }
                            }
                        }
                    }
                }
                else {
                    m_cache[statement.predicate().uri()].append( nodeToVariant( statement.object() ) );
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


void Nepomuk::ResourceData::setProperty( const QUrl& uri, const Nepomuk::Variant& value )
{
    if( m_proxyData )
        return m_proxyData->setProperty( uri, value );

    // step 0: make sure this resource is in the store
    if ( store() ) {
        QList<Node> valueNodes;

        // make sure resource values are in the store
        if ( value.simpleType() == qMetaTypeId<Resource>() ) {
            QList<Resource> l = value.toResourceList();
            for( QList<Resource>::iterator resIt = l.begin(); resIt != l.end(); ++resIt ) {
                if ( resIt->isValid() )
                    resIt->m_data->store();
            }
        }

        // add the actual property statements

        // one-to-one Resource
        if( value.isResource() ) {
            valueNodes.append( value.toResource().resourceUri() );
        }

        // one-to-many Resource
        else if( value.isResourceList() ) {
            const QList<Resource>& l = value.toResourceList();
            for( QList<Resource>::const_iterator resIt = l.constBegin(); resIt != l.constEnd(); ++resIt ) {
                valueNodes.append( (*resIt).resourceUri() );
            }
        }

        // one-to-many literals
        else if( value.isList() ) {
            valueNodes += Nepomuk::valuesToRDFNodes( value );
        }

        // one-to-one literal
        else {
            valueNodes.append( Nepomuk::valueToRDFNode( value ) );
        }

        // update the cache for now
        m_cache[uri] = value;

        // update the store
        ResourceFilterModel fm( ResourceManager::instance()->mainModel() );
        fm.updateProperty( m_uri, uri, valueNodes );
    }
}


void Nepomuk::ResourceData::removeProperty( const QUrl& uri )
{
    if( m_proxyData )
        return m_proxyData->removeProperty( uri );

    if ( determineUri() ) {
        ResourceFilterModel fm( ResourceManager::instance()->mainModel() );
        fm.removeProperty( m_uri, uri );
    }
}


void Nepomuk::ResourceData::remove( bool recursive )
{
    if( m_proxyData )
        return m_proxyData->remove();

    if ( determineUri() ) {
        ResourceFilterModel fm( ResourceManager::instance()->mainModel() );
        fm.removeAllStatements( Statement( m_uri, Node(), Node() ) );
        if ( recursive ) {
            fm.removeAllStatements( Statement( Node(), Node(), m_uri ) );
        }

        // the url is invalid now
        initializedData()->remove( m_uri.toString() );
        m_uri = QUrl();
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
    return( !m_mainType.isEmpty() && ( !m_uri.isEmpty() || !m_kickoffIdentifier.isEmpty() ) );
}


bool Nepomuk::ResourceData::determineUri()
{
    if( m_proxyData )
        return m_proxyData->determineUri();

    else if( m_uri.isEmpty() && !m_kickoffUriOrId.isEmpty() ) {
        Q_ASSERT( !m_kickoffUriOrId.isEmpty() );

        m_modificationMutex.lock();

        Soprano::Model* model = ResourceManager::instance()->mainModel();

        if( model->containsAnyStatement( Statement( QUrl( kickoffUriOrId() ), Node(), Node() ) ) ) {
            //
            // The kickoffUriOrId is actually a URI
            //
            m_uri = kickoffUriOrId();
//            kDebug(300004) << " kickoff identifier " << kickoffUriOrId() << " exists as a URI " << uri();
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
            Q_ASSERT( !m_mainType.isEmpty() );
            if ( it.next() ) {
                // At this point we only have one type since without a uri we could not have loaded other types
                // than the one used on construction
                Types::Class wantedType( m_mainType );

                if ( m_mainType != Soprano::Vocabulary::RDFS::Resource() && wantedType.isAvailable() ) {
                    do {
                        // get the type of the stored resource
                        StatementIterator resourceSl = model->listStatements( Statement( it.current().subject(),
                                                                                         Soprano::Vocabulary::RDF::type(),
                                                                                         Node() ) );
                        while ( resourceSl.next() ) {
                            if ( resourceSl.current().object().isResource() ) {
                                Types::Class storedType = resourceSl.current().object().uri();
                                if ( storedType == wantedType ||
                                     wantedType.isSubClassOf( storedType ) ||
                                     storedType.isSubClassOf( wantedType ) ) {
                                    // in load() The type will be updated properly
                                    m_uri = it.current().subject().uri();
                                    break;
                                }
                            }
                        }
                    } while ( it.next() && m_uri.isEmpty() );
                }
                else {
                    m_uri = it.current().subject().uri();
//                    kDebug(300004) << k_funcinfo << " kickoff identifier " << kickoffUriOrId() << " already exists with URI " << uri();
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

//                kDebug(300004) << " kickoff identifier " << kickoffUriOrId() << " seems fresh. Generated new URI " << m_uri;
            }
        }

        //
        // Move us to the final data hash now that the URI is known
        //
        if( !uri().isEmpty() && uri() != kickoffUriOrId() ) {
            QString s = uri().toString();
            if( !initializedData()->contains( s ) ) {
                initializedData()->insert( s, this );
            }
            else {
                m_proxyData = initializedData()->value( s );
                m_proxyData->ref();
            }
        }

        m_modificationMutex.unlock();
    }

    return !m_uri.isEmpty();
}


// void Nepomuk::ResourceData::updateType()
// {
//     Soprano::Model* model = ResourceManager::instance()->mainModel();

//     // get the type of the stored resource
//     StatementIterator typeStatements = model->listStatements( Statement( m_uri,
//                                                                          Soprano::Vocabulary::RDF::type(),
//                                                                          Node() ) );
//     if ( typeStatements.next() && typeStatements.current().object().isResource() ) {
//         // FIXME: handle multple types, maybe select the one type that fits best
//         QUrl storedType = typeStatements.current().object().uri();
//         if ( m_type == Soprano::Vocabulary::RDFS::Resource() ) {
//             Q_ASSERT( !storedType.isEmpty() );
//             m_type = storedType;
//         }
//         else {
//             Types::Class wantedTypeClass = m_type;
//             Types::Class storedTypeClass = storedType;

//             // Keep the type that is further down the hierarchy
//             if ( wantedTypeClass.isSubClassOf( storedTypeClass ) ) {
//                 m_type = wantedTypeClass.uri();
//             }
//         }
//     }
// }


bool Nepomuk::ResourceData::operator==( const ResourceData& other ) const
{
    const ResourceData* that = this;
    if( m_proxyData )
        that = m_proxyData;

    if( that == &other )
        return true;

    if( that->m_uri != other.m_uri ||
        that->m_mainType != other.m_mainType ) {
        return false;
    }

    return true;
}


Nepomuk::ResourceData* Nepomuk::ResourceData::data( const QUrl& uri, const QUrl& type )
{
    if ( uri.isEmpty() ) {
        // return an invalid resource which may be activated by calling setProperty
        return new ResourceData( uri, QString(), type );
    }

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
//        kDebug(300004) << "No existing ResourceData instance found for uri " << uri;
        //
        // The actual URI is already known here
        //
        ResourceData* d = new ResourceData( uri, QString(), type );
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
    if ( uriOrId.isEmpty() ) {
        return new ResourceData( QUrl(), QString(), type );
    }

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
            Types::Class wantedType = type;
            while ( it != kickoffData()->end() &&
                    it.key() == uriOrId ) {
                if ( it.value()->hasType( type ) ) {
                    break;
                }
                ++it;
            }
        }

        resFound = ( it != kickoffData()->end() && it.key() == uriOrId );
    }

    //
    // The uriOrId has no local representation yet -> create one
    //
    if( !resFound ) {
//        kDebug(300004) << "No existing ResourceData instance found for uriOrId " << uriOrId;
        //
        // Every new ResourceData object ends up in the kickoffdata since its actual URI is not known yet
        //
        ResourceData* d = new ResourceData( QUrl(), uriOrId, type );
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


QList<Nepomuk::ResourceData*> Nepomuk::ResourceData::allResourceDataOfType( const QUrl& type )
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


QList<Nepomuk::ResourceData*> Nepomuk::ResourceData::allResourceDataWithProperty( const QUrl& uri, const Variant& v )
{
    QList<ResourceData*> l;

    for( ResourceDataHash::iterator rdIt = kickoffData()->begin();
         rdIt != kickoffData()->end(); ++rdIt ) {

        if( rdIt.value()->hasProperty( uri ) &&
            rdIt.value()->property( uri ) == v ) {
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
