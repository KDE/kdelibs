/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2010 Sebastian Trueg <trueg@kde.org>
 * Copyright (C) 2010-2012 Vishesh Handa <handa.vish@gmail.com>
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
#include "resourcemanager_p.h"
#include "resource.h"
#include "tools.h"
#include "nie.h"
#include "nfo.h"
#include "pimo.h"
#include "nepomukmainmodel.h"
#include "dbusconnectionpool.h"
#include "class.h"
#include "dbustypes.h"
#include "resourcewatcher.h"

#include <Soprano/Statement>
#include <Soprano/StatementIterator>
#include <Soprano/QueryResultIterator>
#include <Soprano/Model>
#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/NAO>

#include <QtCore/QFile>
#include <QtCore/QDateTime>
#include <QtCore/QMutexLocker>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include <kdebug.h>
#include <kurl.h>
#include <kcomponentdata.h>

using namespace Soprano;

#define MAINMODEL (m_rm->m_manager->mainModel())


Nepomuk::ResourceData::ResourceData( const QUrl& uri, const QUrl& kickOffUri, const QUrl& type, ResourceManagerPrivate* rm )
    : m_uri(uri),
      m_mainType( type ),
      m_modificationMutex(QMutex::Recursive),
      m_cacheDirty(false),
      m_addedToWatcher(false),
      m_pimoThing(0),
      m_groundingOccurence(0),
      m_rm(rm)
{
    if( m_mainType.isEmpty() ) {
        m_mainType = Soprano::Vocabulary::RDFS::Resource();
    }

    m_types << m_mainType;

    m_rm->dataCnt.ref();

    if( !uri.isEmpty() ) {
        m_cacheDirty = true;
        m_rm->m_initializedData.insert( uri, this );
        m_kickoffUris.insert( uri );
    }
    if( !kickOffUri.isEmpty() ) {
        m_kickoffUris.insert( kickOffUri );

        if( kickOffUri.scheme().isEmpty() ) {
            // Label
            const QString label = kickOffUri.toString();
            m_cache.insert( Soprano::Vocabulary::NAO::identifier(), label );
        }
        else if( kickOffUri.scheme() != QLatin1String("nepomuk") ) {
            // It's probably the nie:url
            m_cache.insert( Nepomuk::Vocabulary::NIE::url(), kickOffUri );
        }
    }
    m_rm->addToKickOffList( this, m_kickoffUris );
}


Nepomuk::ResourceData::~ResourceData()
{
    resetAll(true);
    m_rm->dataCnt.deref();
}


bool Nepomuk::ResourceData::isFile()
{
    return( m_uri.scheme() == QLatin1String("file") ||
            m_nieUrl.scheme() == QLatin1String("file") ||
            (!m_kickoffUris.isEmpty() && (*m_kickoffUris.begin()).scheme() == QLatin1String("file")) ||
            constHasType( Soprano::Vocabulary::Xesam::File() ) ||
            constHasType( Nepomuk::Vocabulary::NFO::FileDataObject() ) );
}


QUrl Nepomuk::ResourceData::uri() const
{
    return m_uri;
}


QUrl Nepomuk::ResourceData::type()
{
    load();
    return m_mainType;
}


QList<QUrl> Nepomuk::ResourceData::allTypes()
{
    load();
    return m_types;
}


void Nepomuk::ResourceData::setTypes( const QList<QUrl>& types )
{
    store();

    QMutexLocker lock(&m_modificationMutex);

    // reset types
    m_types.clear();
    m_mainType = Soprano::Vocabulary::RDFS::Resource();

    QList<Node> nodes;
    // load types (and set maintype)
    foreach( const QUrl& url, types ) {
        loadType( url );
        nodes << Node( url );
    }

    // update the data store
    setProperty(Soprano::Vocabulary::RDF::type(), Nepomuk::Variant(types) );
}



void Nepomuk::ResourceData::resetAll( bool isDelete )
{
    // remove us from all caches (store() will re-insert us later if necessary)
    m_rm->mutex.lock();

    // IMPORTANT:
    // Remove from the kickOffList before removing from the resource watcher
    // This is required cause otherwise the Resource::fromResourceUri creates a new
    // resource which is correctly identified to the ResourceData (this), and it is
    // then deleted, which calls resetAll and this cycle continues.
    Q_FOREACH( const KUrl& uri, m_kickoffUris )
        m_rm->m_uriKickoffData.remove( uri );

    if( !m_uri.isEmpty() ) {
        m_rm->m_initializedData.remove( m_uri );
        if( m_rm->m_watcher && m_addedToWatcher ) {
            // See load() for an explanation of the QMetaObject call

            // stop the watcher since we do not want to watch all changes in case there is no ResourceData left
            if(m_rm->m_watcher->resourceCount() == 1) {
                QMetaObject::invokeMethod(m_rm->m_watcher, "stop", Qt::AutoConnection);
            }

            // remove this Resource from the list of watched resources
            QMetaObject::invokeMethod(m_rm->m_watcher, "removeResource", Qt::AutoConnection, Q_ARG(Nepomuk::Resource, Resource::fromResourceUri(m_uri)));
            m_addedToWatcher = false;
        }
    }
    m_rm->mutex.unlock();

    // reset all variables
    m_uri = QUrl();
    m_nieUrl = KUrl();
    m_kickoffUris.clear();
    m_cache.clear();
    m_cacheDirty = false;
    m_types.clear();
    delete m_pimoThing;
    m_pimoThing = 0;
    m_groundingOccurence = 0;

    // when we are being deleted the value of m_mainType is not important
    // anymore. Also since ResourceManager is a global static it might be
    // deleted after the global static behind Soprano::Vocabulary::RDFS
    // which results in a crash.
    if( !isDelete )
        m_mainType = Soprano::Vocabulary::RDFS::Resource();
}


QHash<QUrl, Nepomuk::Variant> Nepomuk::ResourceData::allProperties()
{
    load();
    return m_cache;
}


bool Nepomuk::ResourceData::hasProperty( const QUrl& uri )
{
    load();
    QHash<QUrl, Variant>::const_iterator it = m_cache.constFind( uri );
    if( it == m_cache.constEnd() )
        return false;

    return true;
}


bool Nepomuk::ResourceData::hasProperty( const QUrl& p, const Variant& v )
{
    QHash<QUrl, Variant>::const_iterator it = m_cache.constFind( p );
    if( it == m_cache.constEnd() )
        return false;

    QList<Variant> thisVals = it.value().toVariantList();
    QList<Variant> vals = v.toVariantList();
    Q_FOREACH( const Variant& val, vals ) {
        if( !thisVals.contains(val) )
            return false;
    }
    return true;
}


bool Nepomuk::ResourceData::hasType( const QUrl& uri )
{
    load();
    return constHasType( uri );
}


bool Nepomuk::ResourceData::constHasType( const QUrl& uri ) const
{
    // we need to protect the reading, too. setTypes may be triggered from another thread
    QMutexLocker lock(&m_modificationMutex);

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
    load();

    // we need to protect the reading, too. load my be triggered from another thread's
    // connection to a Soprano statement signal
    QMutexLocker lock(&m_modificationMutex);

    QHash<QUrl, Variant>::const_iterator it = m_cache.constFind( uri );
    if ( it == m_cache.constEnd() ) {
        return Variant();
    }
    else {
        return *it;
    }
}


bool Nepomuk::ResourceData::store()
{
    QMutexLocker lock(&m_modificationMutex);

    if ( m_uri.isEmpty() ) {
        QMutexLocker rmlock(&m_rm->mutex);

        if ( m_nieUrl.isValid() &&
             m_nieUrl.isLocalFile() &&
             m_mainType == Soprano::Vocabulary::RDFS::Resource() ) {
            m_mainType = Nepomuk::Vocabulary::NFO::FileDataObject();
            m_types << m_mainType;
        }

        QDBusConnection bus = QDBusConnection::sessionBus();
        QDBusMessage msg = QDBusMessage::createMethodCall( QLatin1String("org.kde.NepomukStorage"),
                                                           QLatin1String("/datamanagement"),
                                                           QLatin1String("org.kde.nepomuk.DataManagement"),
                                                           QLatin1String("createResource") );
        QString app = KGlobal::mainComponent().componentName();
        QVariantList arguments;

        //FIXME: Maybe we should be setting the 'label' over here.
        arguments << DBus::convertUriList(m_types) << QString() << QString() << app;
        msg.setArguments( arguments );

        QDBusMessage reply = bus.call( msg );
        if( reply.type() == QDBusMessage::ErrorMessage ) {
            //TODO: Set the error somehow
            kWarning() << reply.errorMessage();
            return false;
        }
        else if( reply.type() == QDBusMessage::ReplyMessage ) {
            m_uri = reply.arguments().at(0).toUrl();
        }

        // Add us to the initialized data, i.e. make us "valid"
        m_rm->m_initializedData.insert( m_uri, this );

        // each initialized resource has to be in a kickoff list
        // thus, we make sure that is the case.
        if( m_kickoffUris.isEmpty() ) {
            m_kickoffUris.insert( m_uri );
            m_rm->addToKickOffList( this, m_kickoffUris );
        }

        // store our grounding occurrence in case we are a thing created by the pimoThing() method
        if( m_groundingOccurence ) {
            if( m_groundingOccurence != this )
                m_groundingOccurence->store();
            setProperty(Vocabulary::PIMO::groundingOccurrence(), Variant(m_groundingOccurence->uri()) );
        }

        foreach( const KUrl& url, m_kickoffUris ) {
            if( url.scheme().isEmpty() )
                setProperty( Soprano::Vocabulary::NAO::identifier(), Variant(url.url()) );
            else
                setProperty( Nepomuk::Vocabulary::NIE::url(), Variant(url.url()) );
        }
    }

    return true;
}


void Nepomuk::ResourceData::loadType( const QUrl& storedType )
{
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


bool Nepomuk::ResourceData::load()
{
    QMutexLocker lock(&m_modificationMutex);

    if ( m_cacheDirty ) {
        m_cache.clear();

        if(!m_rm->m_watcher) {
            //
            // The ResourceWatcher is not thread-safe. Thus, we need to ensure the safety ourselves.
            // We do that by simply handling all RW related operations in the manager thread.
            // This also means to invoke methods on the watcher through QMetaObject to make sure they
            // get queued in case of calls between different threads.
            //
            m_rm->m_watcher = new ResourceWatcher(m_rm->m_manager);
            m_rm->m_watcher->moveToThread(m_rm->m_manager->thread());
            QObject::connect( m_rm->m_watcher, SIGNAL(propertyAdded(Nepomuk::Resource,Nepomuk::Types::Property,QVariant)),
                              m_rm->m_manager, SLOT(slotPropertyAdded(Nepomuk::Resource,Nepomuk::Types::Property,QVariant)) );
            QObject::connect( m_rm->m_watcher, SIGNAL(propertyRemoved(Nepomuk::Resource,Nepomuk::Types::Property,QVariant)),
                              m_rm->m_manager, SLOT(slotPropertyRemoved(Nepomuk::Resource,Nepomuk::Types::Property,QVariant)) );
            m_rm->m_watcher->addResource( Nepomuk::Resource::fromResourceUri(m_uri) );
        }
        else {
            QMetaObject::invokeMethod(m_rm->m_watcher, "addResource", Qt::AutoConnection, Q_ARG(Nepomuk::Resource, Nepomuk::Resource::fromResourceUri(m_uri)) );
        }
        // (re-)start the watcher in case this resource is the only one in the list of watched
        if(m_rm->m_watcher->resources().count() <= 1) {
            QMetaObject::invokeMethod(m_rm->m_watcher, "start", Qt::AutoConnection);
        }
        m_addedToWatcher = true;

        if ( m_uri.isValid() ) {
            //
            // We exclude properties that are part of the inference graph
            // It would only pollute the user interface
            //
            Soprano::QueryResultIterator it = MAINMODEL->executeQuery(QString("select distinct ?p ?o where { "
                                                                              "%1 ?p ?o . }").arg(Soprano::Node::resourceToN3(m_uri)),
                                                                      Soprano::Query::QueryLanguageSparqlNoInference);
            while ( it.next() ) {
                QUrl p = it["p"].uri();
                Soprano::Node o = it["o"];
                if ( p == Soprano::Vocabulary::RDF::type() ) {
                    if ( o.isResource() ) {
                        loadType( o.uri() );
                    }
                }
                else {
                    Nepomuk::Variant var = Variant::fromNode( o );
                    updateKickOffLists( p, var );
                    m_cache[p].append( var );
                }
            }

            m_cacheDirty = false;

            delete m_pimoThing;
            m_pimoThing = 0;
            if( hasType( Vocabulary::PIMO::Thing() ) ) {
                m_pimoThing = new Thing( m_uri );
            }
            else {
                // TODO: somehow handle pimo:referencingOccurrence and pimo:occurrence
                QueryResultIterator pimoIt = MAINMODEL->executeQuery( QString( "select ?r where { ?r <%1> <%2> . }")
                                                                      .arg( Vocabulary::PIMO::groundingOccurrence().toString() )
                                                                      .arg( QString::fromLatin1( m_uri.toEncoded() ) ),
                                                                      Soprano::Query::QueryLanguageSparqlNoInference );
                if( pimoIt.next() ) {
                    m_pimoThing = new Thing( pimoIt.binding("r").uri() );
                }
            }

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
    Q_ASSERT( uri.isValid() );

    if( store() ) {
        // step 0: make sure this resource is in the store
        QMutexLocker lock(&m_modificationMutex);

        // update the store
        QDBusConnection bus = QDBusConnection::sessionBus();
        QDBusMessage msg = QDBusMessage::createMethodCall( QLatin1String("org.kde.NepomukStorage"),
                                                           QLatin1String("/datamanagement"),
                                                           QLatin1String("org.kde.nepomuk.DataManagement"),
                                                           QLatin1String("setProperty") );
        QVariantList varList;
        foreach( const Variant& var, value.toVariantList() ) {
            // make sure resource values are identified and in the store
            if( var.simpleType() == qMetaTypeId<Resource>() ) {
                Resource res = var.toResource();
                res.determineFinalResourceData();
                res.m_data->store();

                varList << res.resourceUri();
            }
            else {
                varList << var.variant();
            }
        }

        msg.setArguments( QVariantList()
                          << DBus::convertUriList(QList<QUrl>() << m_uri)
                          << DBus::convertUri(uri)
                          << QVariant(DBus::normalizeVariantList(varList))
                          << KGlobal::mainComponent().componentName() );

        QDBusMessage reply = bus.call( msg );
        if( reply.type() == QDBusMessage::ErrorMessage ) {
            //TODO: Set the error somehow
            kWarning() << reply.errorMessage();
            return;
        }

        // update the cache for now
        if( value.isValid() )
            m_cache[uri] = value;
        else
            m_cache.remove(uri);

        // update the kickofflists
        updateKickOffLists( uri, value );
    }
}


void Nepomuk::ResourceData::addProperty( const QUrl& uri, const Nepomuk::Variant& value )
{
    Q_ASSERT( uri.isValid() );

    if( value.isValid() && store() ) {
        // step 0: make sure this resource is in the store
        QMutexLocker lock(&m_modificationMutex);

        // update the store
        QDBusConnection bus = QDBusConnection::sessionBus();
        QDBusMessage msg = QDBusMessage::createMethodCall( QLatin1String("org.kde.NepomukStorage"),
                                                           QLatin1String("/datamanagement"),
                                                           QLatin1String("org.kde.nepomuk.DataManagement"),
                                                           QLatin1String("addProperty") );
        QVariantList varList;
        foreach( const Nepomuk::Variant var, value.toVariantList() ) {
            // make sure resource values are in the store
            if( var.simpleType() == qMetaTypeId<Resource>() ) {
                Resource res = var.toResource();
                res.determineFinalResourceData();
                res.m_data->store();

                varList << res.resourceUri();
            }
            else {
                varList << var.variant();
            }
        }

        msg.setArguments( QVariantList()
                          << DBus::convertUriList(QList<QUrl>() << m_uri)
                          << DBus::convertUri(uri)
                          << QVariant(DBus::normalizeVariantList(varList))
                          << KGlobal::mainComponent().componentName() );

        QDBusMessage reply = bus.call( msg );
        if( reply.type() == QDBusMessage::ErrorMessage ) {
            //TODO: Set the error somehow
            kWarning() << reply.errorMessage();
            return;
        }

        // update the cache for now
        if( value.isValid() )
            m_cache[uri].append(value);

        // update the kickofflists
        updateKickOffLists( uri, value );
    }
}


void Nepomuk::ResourceData::removeProperty( const QUrl& uri )
{
    Q_ASSERT( uri.isValid() );
    if( !m_uri.isEmpty() ) {
        QMutexLocker lock(&m_modificationMutex);

        QDBusConnection bus = QDBusConnection::sessionBus();
        QDBusMessage msg = QDBusMessage::createMethodCall( QLatin1String("org.kde.NepomukStorage"),
                                                           QLatin1String("/datamanagement"),
                                                           QLatin1String("org.kde.nepomuk.DataManagement"),
                                                           QLatin1String("removeProperties") );
        msg.setArguments( QVariantList()
                          << DBus::convertUri(m_uri)
                          << DBus::convertUri(uri)
                          << KGlobal::mainComponent().componentName() );

        QDBusMessage reply = bus.call( msg );
        if( reply.type() == QDBusMessage::ErrorMessage ) {
            //TODO: Set the error somehow
            kWarning() << reply.errorMessage();
            return;
        }

        // Update the cache
        m_cache.remove( uri );

        // update the kickofflists
        updateKickOffLists( uri, Variant() );
    }
}


void Nepomuk::ResourceData::remove( bool recursive )
{
    Q_UNUSED(recursive)
    QMutexLocker lock(&m_modificationMutex);

    if( !m_uri.isEmpty() ) {
        QDBusConnection bus = QDBusConnection::sessionBus();
        QDBusMessage msg = QDBusMessage::createMethodCall( QLatin1String("org.kde.NepomukStorage"),
                                                           QLatin1String("/datamanagement"),
                                                           QLatin1String("org.kde.nepomuk.DataManagement"),
                                                           QLatin1String("removeResources") );
        // TODO: Set the flag over here
        msg.setArguments( QVariantList()
                          << DBus::convertUri(m_uri)
                          << 0 /* no flags */
                          << KGlobal::mainComponent().componentName());

        QDBusMessage reply = bus.call( msg );
        if( reply.type() == QDBusMessage::ErrorMessage ) {
            //TODO: Set the error somehow
            kWarning() << reply.errorMessage();
            return;
        }
    }

    resetAll();
}


bool Nepomuk::ResourceData::exists()
{
    if( m_uri.isValid() ) {
        const QString query = QString::fromLatin1("ask { %1 ?p ?o . }")
                              .arg( Soprano::Node::resourceToN3(m_uri) );
        return MAINMODEL->executeQuery( query, Soprano::Query::QueryLanguageSparql ).boolValue();
    }
    else {
        return false;
    }
}


bool Nepomuk::ResourceData::isValid() const
{
    return( !m_mainType.isEmpty() && ( !m_uri.isEmpty() || !m_kickoffUris.isEmpty() ) );
}


Nepomuk::ResourceData* Nepomuk::ResourceData::determineUri()
{
    // We have the following possible situations:
    // 1. m_uri is already valid
    //    -> simple, nothing to do
    //
    // 2. m_uri is not valid
    //    -> we need to determine the URI
    //
    // 2.1. m_kickoffUri is valid
    // 2.1.1. it is a file URL
    // 2.1.1.1. it is nie:url for r
    //          -> use r as m_uri
    // 2.1.1.2. it points to a file on a removable device for which we have a filex:/ URL
    //          -> use the r in r nie:url filex:/...
    // 2.1.1.3. it is a file which is not an object in some nie:url relation
    //          -> create new random m_uri and use kickoffUriOrId() as m_nieUrl
    // 2.1.2. it is a resource URI
    //          -> use it as m_uri
    //
    // 2.2. m_kickOffUri is not valid
    // 2.2.1. m_kickOffUri is a nao:identifier for r
    //        -> use r as m_uri
    //

    if( m_uri.isEmpty() ) {
        Soprano::Model* model = MAINMODEL;

        if( !m_kickoffUris.isEmpty() ) {
            KUrl kickOffUri = *m_kickoffUris.begin();
            if( kickOffUri.scheme().isEmpty() ) {
                //
                // Not valid. Checking for nao:identifier
                //
                QString query = QString::fromLatin1("select distinct ?r where { ?r %1 %2. } LIMIT 1")
                                .arg( Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::identifier()) )
                                .arg( Soprano::Node::literalToN3( kickOffUri.url() ) );

                Soprano::QueryResultIterator it = model->executeQuery( query, Soprano::Query::QueryLanguageSparql );
                if( it.next() ) {
                    m_uri = it["r"].uri();
                    it.close();
                }
            }
            else {
                //
                // In one query determine if the URI is already used as resource URI or as
                // nie:url
                //
                QString query = QString::fromLatin1("select distinct ?r ?o where { "
                                                    "{ ?r %1 %2 . FILTER(?r!=%2) . } "
                                                    "UNION "
                                                    "{ %2 ?p ?o . } "
                                                    "} LIMIT 1")
                                .arg( Soprano::Node::resourceToN3( Nepomuk::Vocabulary::NIE::url() ) )
                                .arg( Soprano::Node::resourceToN3( kickOffUri ) );
                Soprano::QueryResultIterator it = model->executeQuery( query, Soprano::Query::QueryLanguageSparql );
                if( it.next() ) {
                    QUrl uri = it["r"].uri();
                    if( uri.isEmpty() ) {
                        m_uri = kickOffUri;
                    }
                    else {
                        m_uri = uri;
                        m_nieUrl = kickOffUri;
                    }
                    it.close();
                }
                else if( kickOffUri.scheme() == QLatin1String("nepomuk") ) {
                    // for nepomuk URIs we simply use the kickoff URI as resource URI
                    m_uri = kickOffUri;
                }
                else {
                    // for everything else we use m_kickoffUri as nie:url with a new random m_uri
                    m_nieUrl = kickOffUri;
                }
            }
        }

        //
        // Move us to the final data hash now that the URI is known
        //
        if( !m_uri.isEmpty() ) {
            m_cacheDirty = true;
            ResourceDataHash::iterator it = m_rm->m_initializedData.find(m_uri);
            if( it == m_rm->m_initializedData.end() ) {
                m_rm->m_initializedData.insert( m_uri, this );
            }
            else {
                return it.value();
            }
        }
    }

    return this;
}


void Nepomuk::ResourceData::invalidateCache()
{
    m_cacheDirty = true;
}


Nepomuk::Thing Nepomuk::ResourceData::pimoThing()
{
    load();
    if( !m_pimoThing ) {
        //
        // We only create a new thing if we are a nie:InformationElement.
        // All other resources will simply be converted into a pimo:Thing
        //
        // Files, however, are a special case in every aspect. this includes pimo things.
        // Files are their own grounding occurrence. This makes a lot of things
        // much simpler.
        //
        if( hasType( Vocabulary::PIMO::Thing() ) ||
                isFile() ||
                !hasType( Vocabulary::NIE::InformationElement() ) ) {
            m_pimoThing = new Thing(this);
        }
        else {
            m_pimoThing = new Thing();
        }
        m_pimoThing->m_data->m_groundingOccurence = this;
    }
    return *m_pimoThing;
}


bool Nepomuk::ResourceData::operator==( const ResourceData& other ) const
{
    if( this == &other )
        return true;

    return( m_uri == other.m_uri &&
            m_mainType == other.m_mainType &&
            m_kickoffUris == other.m_kickoffUris );
}


QDebug Nepomuk::ResourceData::operator<<( QDebug dbg ) const
{
    KUrl::List list = m_kickoffUris.toList();
    dbg << QString::fromLatin1("[kickoffuri: %1; uri: %2; type: %3; ref: %4]")
        .arg( list.toStringList().join(QLatin1String(",")),
              m_uri.url(),
              m_mainType.toString() )
        .arg( m_ref );

    return dbg;
}


QDebug operator<<( QDebug dbg, const Nepomuk::ResourceData& data )
{
    return data.operator<<( dbg );
}


void Nepomuk::ResourceData::updateKickOffLists(const QUrl& prop, const Nepomuk::Variant& v)
{
    KUrl oldUrl;
    KUrl newUrl;
    if( prop == Nepomuk::Vocabulary::NIE::url() ) {
        oldUrl = m_nieUrl;
        newUrl = v.toUrl();
        m_nieUrl = newUrl;
    }
    else if( prop == Soprano::Vocabulary::NAO::identifier() ) {
        Q_FOREACH( const KUrl& url, m_kickoffUris ) {
            if( url.scheme().isEmpty() ) {
                oldUrl = url;
                break;
            }
        }
        newUrl = KUrl( v.toString() );
    }
    else {
        return;
    }

    if( oldUrl != newUrl ) {
        QMutexLocker rmlock(&m_rm->mutex);

        m_kickoffUris.remove( oldUrl );
        m_rm->m_uriKickoffData.remove( oldUrl );

        if( !newUrl.isEmpty() ) {
            m_kickoffUris.insert( newUrl );
            m_rm->m_uriKickoffData.insert( newUrl, this );
        }
    }
}
