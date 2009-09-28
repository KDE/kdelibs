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

using namespace Soprano;


Nepomuk::ResourceManagerPrivate::ResourceManagerPrivate( ResourceManager* manager )
    : mainModel( 0 ),
      overrideModel( 0 ),
      dataCnt( 0 ),
      m_manager( manager )
{
}


Nepomuk::ResourceData* Nepomuk::ResourceManagerPrivate::data( const QUrl& uri, const QUrl& type )
{
    QUrl url( uri );

    if ( url.isEmpty() ) {
        // return an invalid resource which may be activated by calling setProperty
        return new ResourceData( url, QString(), type, this );
    }

    // default to "file" scheme, i.e. we do not allow an empty scheme
    if ( url.scheme().isEmpty() ) {
        url.setScheme( "file" );
    }

    // if scheme is file, try to follow a symlink
    if ( url.scheme() == "file" ) {
        QFileInfo fileInfo( url.toLocalFile() );
        QString linkTarget = fileInfo.canonicalFilePath();
        // linkTarget is empty for dangling symlinks
        if ( !linkTarget.isEmpty() ) {
            url = QUrl::fromLocalFile( linkTarget );
        }
    }

    ResourceDataHash::iterator it = m_initializedData.find( url.toString() );

    //
    // The uriOrId has no local representation yet -> create one
    //
    if( it == m_initializedData.end() ) {
//        kDebug() << "No existing ResourceData instance found for uri " << url;
        //
        // The actual URI is already known here
        //
        ResourceData* d = new ResourceData( url, QString(), type, this );
        m_initializedData.insert( url.toString(), d );

        return d;
    }
    else {
        //
        // Reuse the already existing ResourceData object
        //
        return it.value();
    }
}


Nepomuk::ResourceData* Nepomuk::ResourceManagerPrivate::data( const QString& uriOrId, const QUrl& type )
{
    if ( uriOrId.isEmpty() ) {
        return new ResourceData( QUrl(), QString(), type, this );
    }

    // special case: local files
    if ( QFile::exists(uriOrId) ) {
        return data( QUrl::fromLocalFile (uriOrId), type );
    }

    ResourceDataHash::iterator it = m_initializedData.find( uriOrId );

    bool resFound = ( it != m_initializedData.end() );

    //
    // The uriOrId is not a known local URI. Might be a kickoff value though
    //
    if( it == m_initializedData.end() ) {
        it = m_kickoffData.find( uriOrId );

        // check if the type matches (see determineUri for details)
        if ( !type.isEmpty() && type != Soprano::Vocabulary::RDFS::Resource() ) {
            Types::Class wantedType = type;
            while ( it != m_kickoffData.end() &&
                    it.key() == uriOrId ) {
                if ( it.value()->hasType( type ) ) {
                    break;
                }
                ++it;
            }
        }

        resFound = ( it != m_kickoffData.end() && it.key() == uriOrId );
    }

    //
    // The uriOrId has no local representation yet -> create one
    //
    if( !resFound ) {
//        kDebug() << "No existing ResourceData instance found for uriOrId " << uriOrId;
        //
        // Every new ResourceData object ends up in the kickoffdata since its actual URI is not known yet
        //
        ResourceData* d = new ResourceData( QUrl(), uriOrId, type, this );
        m_kickoffData.insert( uriOrId, d );

        return d;
    }
    else {
        //
        // Reuse the already existing ResourceData object
        //
        return it.value();
    }
}


QList<Nepomuk::ResourceData*> Nepomuk::ResourceManagerPrivate::allResourceDataOfType( const QUrl& type )
{
    QList<ResourceData*> l;

    if( !type.isEmpty() ) {
        for( ResourceDataHash::iterator rdIt = m_kickoffData.begin();
             rdIt != m_kickoffData.end(); ++rdIt ) {
            if( rdIt.value()->type() == type ) {
                l.append( rdIt.value() );
            }
        }
    }

    return l;
}


QList<Nepomuk::ResourceData*> Nepomuk::ResourceManagerPrivate::allResourceDataWithProperty( const QUrl& uri, const Variant& v )
{
    QList<ResourceData*> l;

    for( ResourceDataHash::iterator rdIt = m_kickoffData.begin();
         rdIt != m_kickoffData.end(); ++rdIt ) {

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

    for( ResourceDataHash::iterator rdIt = m_kickoffData.begin();
         rdIt != m_kickoffData.end(); ++rdIt ) {
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


Nepomuk::ResourceManager::ResourceManager()
    : QObject(),
      d( new ResourceManagerPrivate( this ) )
{
    d->resourceFilterModel = new ResourceFilterModel();
    connect( d->resourceFilterModel, SIGNAL(statementsAdded()),
             this, SLOT(slotStoreChanged()) );
    connect( d->resourceFilterModel, SIGNAL(statementsRemoved()),
             this, SLOT(slotStoreChanged()) );
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
    QMutexLocker lock( &d->mutex );

    if( !d->mainModel ) {
        d->mainModel = new MainModel( this );
    }

    d->resourceFilterModel->setParentModel( d->mainModel );

    return d->mainModel->isValid() ? 0 : -1;
}


bool Nepomuk::ResourceManager::initialized() const
{
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
    Soprano::Model* model = mainModel();

    QUrl uri;
    QString normalizedName( name );
    normalizedName.remove( QRegExp( "[^\\w\\.\\-_:]" ) );
    if ( !normalizedName.isEmpty() ) {
        uri = "nepomuk:/" + normalizedName;
    }
    else {
        uri = "nepomuk:/" + KRandom::randomString( 20 );
    }

    while( 1 ) {
        if ( !model->executeQuery( QString("ask where { { <%1> ?p1 ?o1 . } UNION { ?r2 <%1> ?o2 . } UNION { ?r3 ?p3 <%1> . } }")
                                   .arg( QString::fromAscii( uri.toEncoded() ) ), Soprano::Query::QueryLanguageSparql ).boolValue() ) {
            return uri;
        }
        uri = "nepomuk:/" + normalizedName + '_' +  KRandom::randomString( 20 );
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
//    kDebug();
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
