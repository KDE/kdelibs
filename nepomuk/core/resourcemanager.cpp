/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2009 Sebastian Trueg <trueg@kde.org>
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
#include "nie.h"

#include "ontology/class.h"

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
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>

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
        return new ResourceData( QUrl(), QString(), type, this );
    }

    QUrl url( uri );

    // default to "file" scheme, i.e. we do not allow an empty scheme
    if ( url.scheme().isEmpty() ) {
        url.setScheme( "file" );
    }

    QMutexLocker lock( &mutex );

    // look for the URI in the initialized and in the URI kickoff data
    ResourceDataHash::iterator end = m_initializedData.end();
    ResourceDataHash::iterator it = m_initializedData.find( url );
    if( it == end ) {
        end = m_uriKickoffData.end();
        it = m_uriKickoffData.find( url );
    }

    if( it == end ) {
        ResourceData* d = new ResourceData( url, QString(), type, this );
        m_uriKickoffData.insert( url, d );
        return d;
    }
    else {
        return it.value();
    }
}


Nepomuk::ResourceData* Nepomuk::ResourceManagerPrivate::data( const QString& uriOrId, const QUrl& type )
{
    if ( uriOrId.isEmpty() ) {
        return new ResourceData( QUrl(), QString(), type, this );
    }
    else if( QFile::exists( uriOrId ) ) {
        return data( KUrl(uriOrId), type );
    }

    QMutexLocker lock( &mutex );

    KickoffDataHash::iterator it = m_idKickoffData.find( uriOrId );
    if( it == m_idKickoffData.end() ) {
        ResourceData* d = new ResourceData( QUrl(), uriOrId, type, this );
        m_idKickoffData.insert( uriOrId, d );
        return d;
    }
    else {
        return it.value();
    }
}


QList<Nepomuk::ResourceData*> Nepomuk::ResourceManagerPrivate::allResourceDataOfType( const QUrl& type )
{
    QMutexLocker lock( &mutex );

    QList<ResourceData*> l;

    if( !type.isEmpty() ) {
        for( ResourceDataHash::iterator rdIt = m_uriKickoffData.begin();
             rdIt != m_uriKickoffData.end(); ++rdIt ) {
            if( rdIt.value()->type() == type ) {
                l.append( rdIt.value() );
            }
        }
        for( KickoffDataHash::iterator rdIt = m_idKickoffData.begin();
             rdIt != m_idKickoffData.end(); ++rdIt ) {
            if( rdIt.value()->type() == type ) {
                l.append( rdIt.value() );
            }
        }
    }

    return l;
}


QList<Nepomuk::ResourceData*> Nepomuk::ResourceManagerPrivate::allResourceDataWithProperty( const QUrl& uri, const Variant& v )
{
    QMutexLocker lock( &mutex );

    QList<ResourceData*> l;

    for( ResourceDataHash::iterator rdIt = m_uriKickoffData.begin();
         rdIt != m_uriKickoffData.end(); ++rdIt ) {
        if( rdIt.value()->hasProperty( uri ) &&
            rdIt.value()->property( uri ) == v ) {
            l.append( rdIt.value() );
        }
    }
    for( KickoffDataHash::iterator rdIt = m_idKickoffData.begin();
         rdIt != m_idKickoffData.end(); ++rdIt ) {
        if( rdIt.value()->hasProperty( uri ) &&
            rdIt.value()->property( uri ) == v ) {
            l.append( rdIt.value() );
        }
    }

    return l;
}


QList<Nepomuk::ResourceData*> Nepomuk::ResourceManagerPrivate::allResourceData()
{
    QList<ResourceData*> l;

    for( ResourceDataHash::iterator rdIt = m_uriKickoffData.begin();
         rdIt != m_uriKickoffData.end(); ++rdIt ) {
        l.append( rdIt.value() );
    }
    for( KickoffDataHash::iterator rdIt = m_idKickoffData.begin();
         rdIt != m_idKickoffData.end(); ++rdIt ) {
        l.append( rdIt.value() );
    }
    for( ResourceDataHash::iterator rdIt = m_initializedData.begin();
         rdIt != m_initializedData.end(); ++rdIt ) {
        l.append( rdIt.value() );
    }

    return l;
}


bool Nepomuk::ResourceManagerPrivate::dataCacheFull()
{
    return dataCnt >= 1000;
}


void Nepomuk::ResourceManagerPrivate::cleanupCache()
{
    if ( dataCnt >= 1000 ) {
        for( ResourceDataHash::iterator rdIt = m_initializedData.begin();
             rdIt != m_initializedData.end(); ++rdIt ) {
            ResourceData* data = rdIt.value();
            if ( !data->cnt() ) {
                data->deleteData();
                return;
            }
        }
    }
}


void Nepomuk::ResourceManagerPrivate::_k_storageServiceInitialized( bool success )
{
    if( success ) {
        kDebug() << "Nepomuk Storage service up and initialized.";
        emit m_manager->nepomukSystemStarted();
    }
}


void Nepomuk::ResourceManagerPrivate::_k_dbusServiceOwnerChanged( const QString& name, const QString&, const QString& newOwner )
{
    if( name == QLatin1String("org.kde.NepomukStorage") &&
        newOwner.isEmpty() ) {
        kDebug() << "Nepomuk Storage service went down.";
        emit m_manager->nepomukSystemStopped();
    }
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

    // connect to the ownerChanged signal to be able to connect the nepomukSystemStopped
    // signal once the storage service goes away
    connect( QDBusConnection::sessionBus().interface(), SIGNAL(serviceOwnerChanged(QString, QString, QString)),
             this, SLOT(_k_dbusServiceOwnerChanged(QString, QString, QString)) );

    init();
}


Nepomuk::ResourceManager::~ResourceManager()
{
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
    QList<Resource> l;

    if( !type.isEmpty() ) {
        // check local data
        QList<ResourceData*> localData = d->allResourceDataOfType( type );
        for( QList<ResourceData*>::iterator rdIt = localData.begin();
             rdIt != localData.end(); ++rdIt ) {
            l.append( Resource( *rdIt ) );
        }

//        kDebug() << " added local resources: " << l.count();

        Soprano::Model* model = mainModel();
        Soprano::StatementIterator it = model->listStatements( Soprano::Statement( Soprano::Node(), Soprano::Vocabulary::RDF::type(), type ) );

        while( it.next() ) {
            Statement s = *it;
            Resource res( s.subject().uri() );
            if( !l.contains( res ) )
                l.append( res );
        }

//        kDebug() << " added remote resources: " << l.count();
    }

    return l;
}


QList<Nepomuk::Resource> Nepomuk::ResourceManager::allResourcesWithProperty( const QString& uri, const Variant& v )
{
    return allResourcesWithProperty( QUrl(uri), v );
}


QList<Nepomuk::Resource> Nepomuk::ResourceManager::allResourcesWithProperty( const QUrl& uri, const Variant& v )
{
    QList<Resource> l;

    if( v.isList() ) {
        kDebug() << "(ResourceManager::allResourcesWithProperty) list values not supported.";
    }
    else {
        // check local data
        QList<ResourceData*> localData = d->allResourceDataWithProperty( uri, v );
        for( QList<ResourceData*>::iterator rdIt = localData.begin();
             rdIt != localData.end(); ++rdIt ) {
            l.append( Resource( *rdIt ) );
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
            if( !l.contains( res ) )
                l.append( res );
        }
    }

    return l;
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
        if ( !model->executeQuery( QString::fromLatin1("ask where { { <%1> ?p1 ?o1 . } UNION { ?r2 <%1> ?o2 . } UNION { ?r3 ?p3 <%1> . } }")
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

    d->overrideModel = model;
    d->resourceFilterModel->setParentModel( model ? model : d->mainModel );

    // clear cache to make sure we do not mix data
    Q_FOREACH( ResourceData* data, d->allResourceData()) {
        data->invalidateCache();
    }
}


Nepomuk::ResourceManager* Nepomuk::ResourceManager::createManagerForModel( Soprano::Model* model )
{
    ResourceManager* manager = new ResourceManager();
    manager->setOverrideMainModel( model );
    return manager;
}

#include "resourcemanager.moc"
