/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2010 Sebastian Trueg <trueg@kde.org>
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

#include "resourcemanager.h"
#include "resourcemanager_p.h"
#include "resourcedata.h"
#include "tools.h"
#include "nepomukmainmodel.h"
#include "resource.h"
#include "resourcefiltermodel.h"
#include "class.h"
#include "nie.h"

#include <kglobal.h>
#include <kdebug.h>
#include <krandom.h>

#include <Soprano/Node>
#include <Soprano/Statement>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/StatementIterator>
#include <Soprano/QueryResultIterator>

#include <QtCore/QFileInfo>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QUuid>
#include <QtCore/QMutableHashIterator>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusServiceWatcher>

using namespace Soprano;


Nepomuk::ResourceManagerPrivate::ResourceManagerPrivate( ResourceManager* manager )
    : mainModel( 0 ),
      overrideModel( 0 ),
      mutex(QMutex::Recursive),
      dataCnt( 0 ),
      m_manager( manager )
{
}


Nepomuk::ResourceData* Nepomuk::ResourceManagerPrivate::data( const QUrl& uri, const QUrl& type )
{
    if ( uri.isEmpty() ) {
        // return an invalid resource which may be activated by calling setProperty
        return new ResourceData( QUrl(), QUrl(), type, this );
    }

    if( ResourceData* data = findData( uri ) ) {
        return data;
    }
    else {
        QMutexLocker lock( &mutex );
        return new ResourceData( QUrl(), uri, type, this );
    }
}


Nepomuk::ResourceData* Nepomuk::ResourceManagerPrivate::data( const QString& uriOrId, const QUrl& type )
{
    if ( !uriOrId.isEmpty() ) {
        KUrl url(uriOrId);
        return data( url, type );
    }

    return new ResourceData( QUrl(), QUrl(), type, this );
}


Nepomuk::ResourceData* Nepomuk::ResourceManagerPrivate::dataForResourceUri( const QUrl& uri, const QUrl& type )
{
    if ( uri.isEmpty() ) {
        // return an invalid resource which may be activated by calling setProperty
        return new ResourceData( QUrl(), QUrl(), type, this );
    }

    if( ResourceData* data = findData( uri ) ) {
        return data;
    }
    else {
        QMutexLocker lock( &mutex );
        return new ResourceData( uri, QUrl(), type, this );
    }
}


QList<Nepomuk::ResourceData*> Nepomuk::ResourceManagerPrivate::allResourceDataOfType( const QUrl& type )
{
    QMutexLocker lock( &mutex );

    QList<ResourceData*> l;

    if( !type.isEmpty() ) {
        QSet<ResourceData*> rdl = m_uriKickoffData.values().toSet();
        for( QSet<ResourceData*>::iterator rdIt = rdl.begin();
             rdIt != rdl.end(); ++rdIt ) {
            ResourceData* rd = *rdIt;
            //
            // make sure we do not trigger a load here since
            // 1. that could result in the deletion of values from the iterated list (m_cache.clear() in ResourceData::load)
            // 2. We only need to check non-existing resources anyway, since the rest is queried from the db below
            //
            if( rd->constHasType( type ) ) {
                l.append( rd );
            }
        }
    }

    return l;
}


QList<Nepomuk::ResourceData*> Nepomuk::ResourceManagerPrivate::allResourceDataWithProperty( const QUrl& uri, const Variant& v )
{
    QMutexLocker lock( &mutex );

    QList<ResourceData*> l;

    //
    // We need to cache m_uriKickoffData since it might be changed
    // in the loop by ResourceData::load()
    //
    QSet<ResourceData*> rdl = m_uriKickoffData.values().toSet();

    //
    // make sure none of the ResourceData objects are deleted by ResourceData::load below
    // which would result in a crash since we have them cached.
    //
    QList<Resource> tmp;
    foreach( ResourceData* rd, rdl ) {
        tmp << Resource( rd );
    }

    for( QSet<ResourceData*>::iterator rdIt = rdl.begin();
         rdIt != rdl.end(); ++rdIt ) {
        ResourceData* rd = *rdIt;
        if( rd->hasProperty( uri ) &&
            rd->property( uri ) == v ) {
            l.append( rd );
        }
    }

    return l;
}


QList<Nepomuk::ResourceData*> Nepomuk::ResourceManagerPrivate::allResourceData()
{
    return m_uriKickoffData.values().toSet().toList();
}


bool Nepomuk::ResourceManagerPrivate::dataCacheFull() const
{
    return dataCnt >= 1000;
}


void Nepomuk::ResourceManagerPrivate::cleanupCache( int num )
{
    QMutexLocker lock( &mutex );

    QSet<ResourceData*> rdl = m_uriKickoffData.values().toSet();
    for( QSet<ResourceData*>::iterator rdIt = rdl.begin();
         rdIt != rdl.end(); ++rdIt ) {
        ResourceData* data = *rdIt;
        if ( !data->cnt() ) {
            delete data;
            if( num > 0 && --num == 0 )
                break;
        }
    }
}


bool Nepomuk::ResourceManagerPrivate::shouldBeDeleted( ResourceData * rd ) const
{
    //
    // We delete data objects in one of two cases:
    // 1. They are not valid and as such not in one of the ResourceManagerPrivate kickoff lists
    // 2. The cache is already full and we need to clean up
    //
    return( !rd->cnt() && ( !rd->isValid() || dataCacheFull() ));
}


void Nepomuk::ResourceManagerPrivate::addToKickOffList( ResourceData* rd, const QSet<KUrl> & uris )
{
    Q_FOREACH( const KUrl& uri, uris )
        m_uriKickoffData.insert( uri, rd );
}


void Nepomuk::ResourceManagerPrivate::_k_storageServiceInitialized( bool success )
{
    if( success ) {
        kDebug() << "Nepomuk Storage service up and initialized.";
        cleanupCache(-1);
        m_manager->init();
        emit m_manager->nepomukSystemStarted();
    }
}


void Nepomuk::ResourceManagerPrivate::_k_dbusServiceUnregistered( const QString& serviceName )
{
    if( serviceName == QLatin1String("org.kde.NepomukStorage") ) {
        kDebug() << "Nepomuk Storage service went down.";
        cleanupCache(-1);
        emit m_manager->nepomukSystemStopped();
    }
}



Nepomuk::ResourceData* Nepomuk::ResourceManagerPrivate::findData( const QUrl& uri )
{
    if ( !uri.isEmpty() ) {
        QMutexLocker lock( &mutex );

        // look for the URI in the initialized and in the URI kickoff data
        ResourceDataHash::iterator end = m_initializedData.end();
        ResourceDataHash::iterator it = m_initializedData.find( uri );
        if( it == end ) {
            end = m_uriKickoffData.end();
            it = m_uriKickoffData.find( uri );
        }

        if( it != end ) {
            return it.value();
        }
    }

    return 0;
}


Nepomuk::ResourceManager::ResourceManager()
    : QObject(),
      d( new ResourceManagerPrivate( this ) )
{
    d->resourceFilterModel = new ResourceFilterModel( this );
    connect( d->resourceFilterModel, SIGNAL(statementsAdded()),
             this, SLOT(slotStoreChanged()) );
    connect( d->resourceFilterModel, SIGNAL(statementsRemoved()),
             this, SLOT(slotStoreChanged()) );

    // connect to the storage service's initialized signal to be able to emit
    // the nepomukSystemStarted signal
    QDBusConnection::sessionBus().connect( QLatin1String("org.kde.NepomukStorage"),
                                           QLatin1String("/servicecontrol"),
                                           QLatin1String("org.kde.nepomuk.ServiceControl"),
                                           QLatin1String("serviceInitialized"),
                                           this,
                                           SLOT(_k_storageServiceInitialized(bool)) );

    // connect to the serviceUnregistered signal to be able to connect the nepomukSystemStopped
    // signal once the storage service goes away
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher( QLatin1String("org.kde.NepomukStorage"),
                                                            QDBusConnection::sessionBus(),
                                                            QDBusServiceWatcher::WatchForUnregistration,
                                                            this );
    connect( watcher, SIGNAL(serviceUnregistered(QString)),
             this, SLOT(_k_dbusServiceUnregistered(QString)) );

    init();
}


Nepomuk::ResourceManager::~ResourceManager()
{
    clearCache();
    delete d->resourceFilterModel;
    delete d->mainModel;
    delete d;
}


void Nepomuk::ResourceManager::deleteInstance()
{
    delete this;
}


class Nepomuk::ResourceManagerHelper
{
    public:
        Nepomuk::ResourceManager q;
};
K_GLOBAL_STATIC(Nepomuk::ResourceManagerHelper, instanceHelper)

Nepomuk::ResourceManager* Nepomuk::ResourceManager::instance()
{
    return &instanceHelper->q;
}


int Nepomuk::ResourceManager::init()
{
    QMutexLocker lock( &d->initMutex );

    if( !d->mainModel ) {
        d->mainModel = new MainModel( this );
    }

    d->resourceFilterModel->setParentModel( d->mainModel );

    d->mainModel->init();

    return d->mainModel->isValid() ? 0 : -1;
}


bool Nepomuk::ResourceManager::initialized() const
{
    QMutexLocker lock( &d->initMutex );
    return d->mainModel && d->mainModel->isValid();
}


Nepomuk::Resource Nepomuk::ResourceManager::createResourceFromUri( const QString& uri )
{
    return Resource( uri, QUrl() );
}

void Nepomuk::ResourceManager::removeResource( const QString& uri )
{
    Resource res( uri );
    res.remove();
}

void Nepomuk::ResourceManager::notifyError( const QString& uri, int errorCode )
{
    kDebug() << "(Nepomuk::ResourceManager) error: " << uri << " " << errorCode;
    emit error( uri, errorCode );
}


QList<Nepomuk::Resource> Nepomuk::ResourceManager::allResourcesOfType( const QString& type )
{
    return allResourcesOfType( QUrl(type) );
}


QList<Nepomuk::Resource> Nepomuk::ResourceManager::allResourcesOfType( const QUrl& type )
{
    QSet<Resource> set;

    if( !type.isEmpty() ) {
        // check local data
        QList<ResourceData*> localData = d->allResourceDataOfType( type );
        for( QList<ResourceData*>::iterator rdIt = localData.begin();
             rdIt != localData.end(); ++rdIt ) {
            Resource res( *rdIt );
            set.insert(res);
        }

//        kDebug() << " added local resources: " << l.count();

        Soprano::Model* model = mainModel();
        Soprano::StatementIterator it = model->listStatements( Soprano::Statement( Soprano::Node(), Soprano::Vocabulary::RDF::type(), type ) );

        while( it.next() ) {
            Statement s = *it;
            Resource res( s.subject().uri() );
            set.insert(res);
        }

//        kDebug() << " added remote resources: " << l.count();
    }

    return set.toList();
}


QList<Nepomuk::Resource> Nepomuk::ResourceManager::allResources()
{
    QList<Nepomuk::Resource> l;
    Q_FOREACH( ResourceData* data, d->allResourceData()) {
        l << Resource( data );
    }

    Soprano::QueryResultIterator it = mainModel()->executeQuery( QLatin1String("select distinct ?r where { ?r a ?t . FILTER(?t != rdf:Property && ?t != rdfs:Class) . }"),
                                                                 Soprano::Query::QueryLanguageSparql );
    while( it.next() ) {
        Resource r( it[0].uri() );
        l << r;
    }

    return l;
}


QList<Nepomuk::Resource> Nepomuk::ResourceManager::allResourcesWithProperty( const QString& uri, const Variant& v )
{
    return allResourcesWithProperty( QUrl(uri), v );
}


QList<Nepomuk::Resource> Nepomuk::ResourceManager::allResourcesWithProperty( const QUrl& uri, const Variant& v )
{
    QSet<Resource> set;

    if( v.isList() ) {
        kDebug() << "(ResourceManager::allResourcesWithProperty) list values not supported.";
    }
    else {
        // check local data
        QList<ResourceData*> localData = d->allResourceDataWithProperty( uri, v );
        for( QList<ResourceData*>::iterator rdIt = localData.begin();
             rdIt != localData.end(); ++rdIt ) {
            set.insert( Resource( *rdIt ) );
        }

        // check remote data
        Soprano::Node n;
        if( v.isResource() ) {
            n = v.toResource().resourceUri();
        }
        else {
            n = valueToRDFNode(v);
        }

        Soprano::Model* model = mainModel();
        Soprano::StatementIterator it = model->listStatements( Soprano::Statement( Soprano::Node(), uri, n ) );

        while( it.next() ) {
            Statement s = *it;
            Resource res( s.subject().uri() );
            set.insert( res );
        }
    }

    return set.toList();
}


void Nepomuk::ResourceManager::clearCache()
{
    d->cleanupCache( -1 );
}


QString Nepomuk::ResourceManager::generateUniqueUri()
{
    return generateUniqueUri( QString() ).toString();
}


QUrl Nepomuk::ResourceManager::generateUniqueUri( const QString& name )
{
    // default to res URIs
    QString type = QLatin1String("res");

    // ctx is the only used value for name
    if(name == QLatin1String("ctx")) {
        type = name;
    }

    Soprano::Model* model = mainModel();

    while( 1 ) {
        QString uuid = QUuid::createUuid().toString();
        uuid = uuid.mid(1, uuid.length()-2);
        QUrl uri = QUrl( QLatin1String("nepomuk:/") + type + QLatin1String("/") + uuid );
        if ( !model->executeQuery( QString::fromLatin1("ask where { "
                                                       "{ <%1> ?p1 ?o1 . } "
                                                       "UNION "
                                                       "{ ?s2 <%1> ?o2 . } "
                                                       "UNION "
                                                       "{ ?s3 ?p3 <%1> . } "
                                                       "UNION "
                                                       "{ graph <%1> { ?s4 ?4 ?o4 . } . } "
                                                       "}")
                                   .arg( QString::fromAscii( uri.toEncoded() ) ), Soprano::Query::QueryLanguageSparql ).boolValue() ) {
            return uri;
        }
    }
}


Soprano::Model* Nepomuk::ResourceManager::mainModel()
{
    // make sure we are initialized
    if ( !d->overrideModel && !initialized() ) {
        init();
    }

    return d->resourceFilterModel;
}


void Nepomuk::ResourceManager::slotStoreChanged()
{
    QMutexLocker lock( &d->mutex );

    Q_FOREACH( ResourceData* data, d->allResourceData()) {
        data->invalidateCache();
    }
}


void Nepomuk::ResourceManager::setOverrideMainModel( Soprano::Model* model )
{
    QMutexLocker lock( &d->mutex );

    if( model != d->resourceFilterModel ) {
        d->overrideModel = model;
        d->resourceFilterModel->setParentModel( model ? model : d->mainModel );

        // clear cache to make sure we do not mix data
        Q_FOREACH( ResourceData* data, d->allResourceData()) {
            data->invalidateCache();
        }
    }
}


Nepomuk::ResourceManager* Nepomuk::ResourceManager::createManagerForModel( Soprano::Model* model )
{
    ResourceManager* manager = new ResourceManager();
    manager->setOverrideMainModel( model );
    return manager;
}

#include "resourcemanager.moc"
