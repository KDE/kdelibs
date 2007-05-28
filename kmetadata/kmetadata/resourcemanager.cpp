/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
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

#include <knepomuk/knepomuk.h>
#include <knepomuk/services/rdfrepository.h>
#include <knepomuk/rdf/statementlistiterator.h>

#include <kglobal.h>
#include <kdebug.h>
#include <krandom.h>



using namespace Nepomuk::Services;
using namespace Nepomuk::RDF;
using namespace Soprano;


static const char* KMETADATA_NAMESPACE = "http://kmetadata.kde.org/resources#";


class Nepomuk::KMetaData::ResourceManager::Private
{
public:
    Private( ResourceManager* manager )
        : initialized(false),
          registry(0),
          m_parent(manager) {
    }

    bool initialized;

    Nepomuk::Backbone::Registry* registry;

private:
    ResourceManager* m_parent;
};


Nepomuk::KMetaData::ResourceManager::ResourceManager()
    : QObject(),
      d( new Private( this ) )
{
}


Nepomuk::KMetaData::ResourceManager::~ResourceManager()
{
    delete d;
}

class Nepomuk::KMetaData::ResourceManagerHelper
{
    public:
        Nepomuk::KMetaData::ResourceManager q;
};
K_GLOBAL_STATIC(Nepomuk::KMetaData::ResourceManagerHelper, instanceHelper)

// FIXME: make the singleton deletion thread-safe so autosyncing will be forced when shutting
//        down the application
//        Maybe connect to QCoreApplication::aboutToQuit?
Nepomuk::KMetaData::ResourceManager* Nepomuk::KMetaData::ResourceManager::instance()
{
    return &instanceHelper->q;
}


int Nepomuk::KMetaData::ResourceManager::init()
{
    if( !d->initialized ) {
        if( !d->registry )
            d->registry = new Backbone::Registry( this );

        //   if( serviceRegistry()->status() != VALID ) {
        //     kDebug(300004) << "(ResourceManager) failed to initialize registry." << endl;
        //     return -1;
        //  }

        if( !serviceRegistry()->discoverRDFRepository() ) {
            kDebug(300004) << "(ResourceManager) No NEPOMUK RDFRepository service found." << endl;
            return -1;
        }

        //   if( !serviceRegistry()->discoverResourceIdService() ) {
        //     kDebug(300004) << "(ResourceManager) No NEPOMUK ResourceId service found." << endl;
        //     return -1;
        //   }

        d->initialized = true;
    }

    return 0;
}


bool Nepomuk::KMetaData::ResourceManager::initialized() const
{
    return d->initialized;
}


Nepomuk::Backbone::Registry* Nepomuk::KMetaData::ResourceManager::serviceRegistry() const
{
    return d->registry;
}


Nepomuk::KMetaData::Resource Nepomuk::KMetaData::ResourceManager::createResourceFromUri( const QString& uri )
{
    return Resource( uri, QString() );
}

void Nepomuk::KMetaData::ResourceManager::notifyError( const QString& uri, int errorCode )
{
    kDebug(300004) << "(Nepomuk::KMetaData::ResourceManager) error: " << uri << " " << errorCode << endl;
    emit error( uri, errorCode );
}


QList<Nepomuk::KMetaData::Resource> Nepomuk::KMetaData::ResourceManager::allResourcesOfType( const QString& type ) const
{
    QList<Resource> l;

    if( !type.isEmpty() ) {
        // check local data
        QList<ResourceData*> localData = ResourceData::allResourceDataOfType( type );
        for( QList<ResourceData*>::iterator rdIt = localData.begin();
             rdIt != localData.end(); ++rdIt ) {
            l.append( Resource( *rdIt ) );
        }

        kDebug(300004) << k_funcinfo << " added local resources: " << l.count() << endl;

        // check remote data
        RDFRepository rdfr( serviceRegistry()->discoverRDFRepository() );
        StatementListIterator it( rdfr.queryListStatements( KMetaData::defaultGraph(),
                                                            Statement( Node(), QUrl( KMetaData::typePredicate() ), QUrl(type) ),
                                                            100 ),
                                  &rdfr );
        while( it.hasNext() ) {
            const Statement& s = it.next();
            Resource res( s.subject().toString() );
            if( !l.contains( res ) )
                l.append( res );
        }

        kDebug(300004) << k_funcinfo << " added remote resources: " << l.count() << endl;
    }

    return l;
}


QList<Nepomuk::KMetaData::Resource> Nepomuk::KMetaData::ResourceManager::allResourcesWithProperty( const QString& uri, const Variant& v ) const
{
    QList<Resource> l;

    if( v.isList() ) {
        kDebug(300004) << "(ResourceManager::allResourcesWithProperty) list values not supported." << endl;
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
            n = KMetaData::valueToRDFNode( v );
        }

        StatementListIterator it( rdfr.queryListStatements( KMetaData::defaultGraph(),
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


QString Nepomuk::KMetaData::ResourceManager::generateUniqueUri() const
{
    RDFRepository rdfr( serviceRegistry()->discoverRDFRepository() );

    QUrl s;
    while( 1 ) {
        // Should we use the Nepomuk localhost whatever namespace here?
        s = KMETADATA_NAMESPACE + KRandom::randomString( 20 );
        if( !rdfr.listRepositoryIds().contains( KMetaData::defaultGraph() ) ||
            ( !rdfr.contains( KMetaData::defaultGraph(), Statement( s, Node(), Node() ) ) &&
              !rdfr.contains( KMetaData::defaultGraph(), Statement( Node(), s, Node() ) ) &&
              !rdfr.contains( KMetaData::defaultGraph(), Statement( Node(), Node(), s ) ) ) )
            return s.toString();
    }
}

#include "resourcemanager.moc"
