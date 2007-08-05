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

#include "services/rdfrepository.h"
#include "rdf/statementlistiterator.h"
#include "registry.h"

#include <kglobal.h>
#include <kdebug.h>
#include <krandom.h>



using namespace Nepomuk::Services;
using namespace Nepomuk::RDF;
using namespace Soprano;


static const char* NEPOMUK_NAMESPACE = "http://nepomuk.kde.org/resources#";


class Nepomuk::ResourceManager::Private
{
public:
    Private( ResourceManager* manager )
        : initialized(false),
          registry(0),
          m_parent(manager) {
    }

    bool initialized;

    Nepomuk::Middleware::Registry* registry;

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

// FIXME: make the singleton deletion thread-safe so autosyncing will be forced when shutting
//        down the application
//        Maybe connect to QCoreApplication::aboutToQuit?
Nepomuk::ResourceManager* Nepomuk::ResourceManager::instance()
{
    return &instanceHelper->q;
}


int Nepomuk::ResourceManager::init()
{
    if( !d->initialized ) {
        if( !d->registry )
            d->registry = new Middleware::Registry( this );

        //   if( serviceRegistry()->status() != VALID ) {
        //     kDebug(300004) << "(ResourceManager) failed to initialize registry.";
        //     return -1;
        //  }

        if( !serviceRegistry()->discoverRDFRepository() ) {
            kDebug(300004) << "(ResourceManager) No NEPOMUK RDFRepository service found.";
            return -1;
        }

        //   if( !serviceRegistry()->discoverResourceIdService() ) {
        //     kDebug(300004) << "(ResourceManager) No NEPOMUK ResourceId service found.";
        //     return -1;
        //   }

        d->initialized = true;
    }

    return 0;
}


bool Nepomuk::ResourceManager::initialized() const
{
    return d->initialized;
}


Nepomuk::Middleware::Registry* Nepomuk::ResourceManager::serviceRegistry() const
{
    return d->registry;
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


QList<Nepomuk::Resource> Nepomuk::ResourceManager::allResourcesOfType( const QString& type ) const
{
    QList<Resource> l;

    if( !type.isEmpty() ) {
        // check local data
        QList<ResourceData*> localData = ResourceData::allResourceDataOfType( type );
        for( QList<ResourceData*>::iterator rdIt = localData.begin();
             rdIt != localData.end(); ++rdIt ) {
            l.append( Resource( *rdIt ) );
        }

        kDebug(300004) << k_funcinfo << " added local resources: " << l.count();

        // check remote data
        RDFRepository rdfr( serviceRegistry()->discoverRDFRepository() );
        StatementListIterator it( rdfr.queryListStatements( Nepomuk::defaultGraph(),
                                                            Statement( Node(), QUrl( Nepomuk::typePredicate() ), QUrl(type) ),
                                                            100 ),
                                  &rdfr );
        while( it.hasNext() ) {
            const Statement& s = it.next();
            Resource res( s.subject().toString() );
            if( !l.contains( res ) )
                l.append( res );
        }

        kDebug(300004) << k_funcinfo << " added remote resources: " << l.count();
    }

    return l;
}


QList<Nepomuk::Resource> Nepomuk::ResourceManager::allResourcesWithProperty( const QString& uri, const Variant& v ) const
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
        RDFRepository rdfr( serviceRegistry()->discoverRDFRepository() );
        Node n;
        if( v.isResource() ) {
            n = QUrl( v.toResource().uri() );
        }
        else {
            n = Nepomuk::valueToRDFNode( v );
        }

        StatementListIterator it( rdfr.queryListStatements( Nepomuk::defaultGraph(),
                                                            Statement( Node(), QUrl(uri), n ),
                                                            100 ), &rdfr );

        while( it.hasNext() ) {
            const Statement& s = it.next();
            Resource res( s.subject().toString() );
            if( !l.contains( res ) )
                l.append( res );
        }
    }

    return l;
}


QString Nepomuk::ResourceManager::generateUniqueUri() const
{
    RDFRepository rdfr( serviceRegistry()->discoverRDFRepository() );

    QUrl s;
    while( 1 ) {
        // Should we use the Nepomuk localhost whatever namespace here?
        s = NEPOMUK_NAMESPACE + KRandom::randomString( 20 );
        if( !rdfr.listRepositoryIds().contains( Nepomuk::defaultGraph() ) ||
            ( !rdfr.contains( Nepomuk::defaultGraph(), Statement( s, Node(), Node() ) ) &&
              !rdfr.contains( Nepomuk::defaultGraph(), Statement( Node(), s, Node() ) ) &&
              !rdfr.contains( Nepomuk::defaultGraph(), Statement( Node(), Node(), s ) ) ) )
            return s.toString();
    }
}

#include "resourcemanager.moc"
