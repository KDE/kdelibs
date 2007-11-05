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

#include "resourcemanager.h"
#include "resourcedata.h"
#include "tools.h"

#include "resource.h"

#include <kglobal.h>
#include <kdebug.h>
#include <krandom.h>

#include <Soprano/Client/DBusClient>
#include <Soprano/Client/DBusModel>
#include <Soprano/Node>
#include <Soprano/Statement>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/StatementIterator>

// just to be sure nobody hits me due to the API change in Soprano
#include <soprano/dummymodel.h>
namespace Soprano {
   namespace Util {
      class Dummy {};
   }
}

using namespace Soprano;
using namespace Soprano::Util;


static const char* NEPOMUK_NAMESPACE = "http://nepomuk.kde.org/resources#";


class Nepomuk::ResourceManager::Private
{
public:
    Private( ResourceManager* manager )
        : client( "org.kde.NepomukServer" ),
          mainModel( 0 ),
          dummyModel( 0 ),
          m_parent(manager) {
    }

    Soprano::Client::DBusClient client;
    Soprano::Model* mainModel;

    DummyModel* dummyModel;

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
    delete d->dummyModel;
    delete d;
}

class Nepomuk::ResourceManagerHelper
{
    public:
        Nepomuk::ResourceManager q;
};
K_GLOBAL_STATIC(Nepomuk::ResourceManagerHelper, instanceHelper)

// FIXME: make the singleton deletion thread-safe so autosyncing will be forced when shutting
//        down the application
//        Maybe connect to QCoreApplication::aboutToQuit?
Nepomuk::ResourceManager* Nepomuk::ResourceManager::instance()
{
    return &instanceHelper->q;
}


int Nepomuk::ResourceManager::init()
{
    return ( initialized() ? 0 : -1 );
}


bool Nepomuk::ResourceManager::initialized() const
{
    return d->client.isValid();
}


Nepomuk::Resource Nepomuk::ResourceManager::createResourceFromUri( const QString& uri )
{
    return Resource( uri, QString() );
}

void Nepomuk::ResourceManager::notifyError( const QString& uri, int errorCode )
{
    kDebug(300004) << "(Nepomuk::ResourceManager) error: " << uri << " " << errorCode;
    emit error( uri, errorCode );
}


QList<Nepomuk::Resource> Nepomuk::ResourceManager::allResourcesOfType( const QString& type )
{
    QList<Resource> l;

    if( !type.isEmpty() ) {
        // check local data
        QList<ResourceData*> localData = ResourceData::allResourceDataOfType( type );
        for( QList<ResourceData*>::iterator rdIt = localData.begin();
             rdIt != localData.end(); ++rdIt ) {
            l.append( Resource( *rdIt ) );
        }

        kDebug(300004) << " added local resources: " << l.count();

        Soprano::Model* model = mainModel();
        Soprano::StatementIterator it = model->listStatements( Soprano::Statement( Soprano::Node(), Soprano::Vocabulary::RDF::type(), QUrl(type) ) );

        while( it.next() ) {
            Statement s = *it;
            Resource res( s.subject().toString() );
            if( !l.contains( res ) )
                l.append( res );
        }

        kDebug(300004) << " added remote resources: " << l.count();
    }

    return l;
}


QList<Nepomuk::Resource> Nepomuk::ResourceManager::allResourcesWithProperty( const QString& uri, const Variant& v )
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
            n = QUrl( v.toResource().uri() );
        }
        else {
            n = valueToRDFNode(v);
        }

        Soprano::Model* model = mainModel();
        Soprano::StatementIterator it = model->listStatements( Soprano::Statement( Soprano::Node(), QUrl(uri), n ) );

        while( it.next() ) {
            Statement s = *it;
            Resource res( s.subject().toString() );
            if( !l.contains( res ) )
                l.append( res );
        }
    }

    return l;
}


QString Nepomuk::ResourceManager::generateUniqueUri()
{
    Soprano::Model* model = mainModel();
    QUrl s;
    while( 1 ) {
        // Should we use the Nepomuk localhost whatever namespace here?
        s = NEPOMUK_NAMESPACE + KRandom::randomString( 20 );
        if( !model->containsContext( s ) &&
            !model->containsAnyStatement( Soprano::Statement( s, Soprano::Node(), Soprano::Node() ) ) &&
            !model->containsAnyStatement( Soprano::Statement( Soprano::Node(), s, Soprano::Node() ) ) &&
            !model->containsAnyStatement( Soprano::Statement( Soprano::Node(), Soprano::Node(), s ) ) )
            return s.toString();
    }
}


Soprano::Model* Nepomuk::ResourceManager::mainModel()
{
    // make sure we are initialized
    if ( !initialized() ) {
        delete d->mainModel;
        d->mainModel = 0;
        init();
    }

    if ( !d->mainModel ) {
        d->mainModel = d->client.createModel( "main" );
    }

    if ( !d->mainModel ) {
        if ( !d->dummyModel ) {
            d->dummyModel = new DummyModel();
        }
        return d->dummyModel;
    }

    return d->mainModel;
}

#include "resourcemanager.moc"
