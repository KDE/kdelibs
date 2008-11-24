/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2008 Sebastian Trueg <trueg@kde.org>
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
#include "resourcedata.h"
#include "tools.h"
#include "nepomukmainmodel.h"
#include "resource.h"

#include <kglobal.h>
#include <kdebug.h>
#include <krandom.h>

#include <Soprano/Node>
#include <Soprano/Statement>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/StatementIterator>
#include <Soprano/QueryResultIterator>

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

using namespace Soprano;



class Nepomuk::ResourceManager::Private
{
public:
    Private( ResourceManager* manager )
        : mainModel( 0 ),
          overrideModel( 0 ),
          m_parent(manager) {
    }

    Nepomuk::MainModel* mainModel;
    Soprano::Model* overrideModel;

    QMutex mutex;

private:
    ResourceManager* m_parent;
};


Nepomuk::ResourceManager::ResourceManager()
    : QObject(),
      d( new Private( this ) )
{
}


Nepomuk::ResourceManager::~ResourceManager()
{
    delete d;
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
        connect( d->mainModel, SIGNAL(statementsAdded()),
                 this, SLOT(slotStoreChanged()) );
        connect( d->mainModel, SIGNAL(statementsRemoved()),
                 this, SLOT(slotStoreChanged()) );
    }
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
    kDebug(300004) << "(Nepomuk::ResourceManager) error: " << uri << " " << errorCode;
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
        QList<ResourceData*> localData = ResourceData::allResourceDataOfType( type );
        for( QList<ResourceData*>::iterator rdIt = localData.begin();
             rdIt != localData.end(); ++rdIt ) {
            l.append( Resource( *rdIt ) );
        }

//        kDebug(300004) << " added local resources: " << l.count();

        Soprano::Model* model = mainModel();
        Soprano::StatementIterator it = model->listStatements( Soprano::Statement( Soprano::Node(), Soprano::Vocabulary::RDF::type(), type ) );

        while( it.next() ) {
            Statement s = *it;
            Resource res( s.subject().uri() );
            if( !l.contains( res ) )
                l.append( res );
        }

//        kDebug(300004) << " added remote resources: " << l.count();
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
        kDebug(300004) << "(ResourceManager::allResourcesWithProperty) list values not supported.";
    }
    else {
        // check local data
        QList<ResourceData*> localData = ResourceData::allResourceDataWithProperty( uri, v );
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
    normalizedName.replace( QRegExp( "[^\\w\\.\\-_:]" ), "" );
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
    if ( d->overrideModel ) {
        return d->overrideModel;
    }

    // make sure we are initialized
    if ( !initialized() ) {
        init();
    }

    return d->mainModel;
}


void Nepomuk::ResourceManager::slotStoreChanged()
{
//    kDebug();
    Q_FOREACH( ResourceData* data, ResourceData::allResourceData()) {
        data->invalidateCache();
    }
}


void Nepomuk::ResourceManager::setOverrideMainModel( Soprano::Model* model )
{
    QMutexLocker lock( &d->mutex );

    if ( d->overrideModel ) {
        d->overrideModel->disconnect( this );
    }

    d->overrideModel = model;

    if ( model ) {
        connect( model, SIGNAL(statementsAdded()),
                 this, SLOT(slotStoreChanged()) );
        connect( model, SIGNAL(statementsRemoved()),
                 this, SLOT(slotStoreChanged()) );
    }

    if ( d->mainModel ) {
        d->mainModel->blockSignals( model != 0 );
    }

    // clear cache to make sure we do not mix data
    Q_FOREACH( ResourceData* data, ResourceData::allResourceData()) {
        data->invalidateCache();
    }
}

#include "resourcemanager.moc"
