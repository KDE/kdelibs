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

#include "registry.h"
#include "service.h"
#include "servicepublisher.h"
#include "registrydbusbackend.h"

#include <QtCore/QList>
#include <QtCore/QMap>

#include <kdebug.h>

#include <QtCore/QRegExp>
#include <QtCore/QStringList>


class Nepomuk::Middleware::Registry::Private
{
public:
    DBus::RegistryBackend* backend;

    QList<Service*> services;
    QMap<QString, Service*> serviceUrlMap;

    QMap<QString, ServicePublisher*> servicePublisherMap;
};


Nepomuk::Middleware::Registry::Registry( QObject* parent )
    : QObject( parent ),
      d( new Private() )
{
    d->backend = new DBus::RegistryBackend( this );

    connect( d->backend, SIGNAL(serviceRegistered(const ServiceDesc&)),
             this, SLOT(slotServiceRegistered(const ServiceDesc&)) );
    connect( d->backend, SIGNAL(serviceUnregistered(const ServiceDesc&)),
             this, SLOT(slotServiceUnregistered(const ServiceDesc&)) );

    updateServiceCache();
}


Nepomuk::Middleware::Registry::~Registry()
{
    delete d->backend;
    delete d;
}


int Nepomuk::Middleware::Registry::registerService( Nepomuk::Middleware::ServicePublisher* service )
{
    int ret = d->backend->registerService( service );
    if( ret == 0 ) {
        //
        // Store the successfully exported service publisher
        //
        d->servicePublisherMap[service->url()] = service;
    }
    return ret;
}


int Nepomuk::Middleware::Registry::unregisterService( Nepomuk::Middleware::ServicePublisher* service )
{
    if( d->servicePublisherMap.contains( service->url() ) ) {
        d->backend->unregisterService( service );
        return 0;
    }
    else
        return -1; // FIXME: proper return value
}


int Nepomuk::Middleware::Registry::unregisterService( const QString& url )
{
    if( d->servicePublisherMap.contains( url ) )
        return unregisterService( d->servicePublisherMap[url] );
    else
        return -1; // FIXME: proper return value
}


Nepomuk::Middleware::Service* Nepomuk::Middleware::Registry::discoverServiceByUrl( const QString& url )
{
    updateServiceCache();
    return d->serviceUrlMap[url];
}


QList<Nepomuk::Middleware::Service*>  Nepomuk::Middleware::Registry::discoverServicesByName( const QString& name )
{
    updateServiceCache();

    QRegExp rex( name );
    QList<Nepomuk::Middleware::Service*> l;
    for( QList<Nepomuk::Middleware::Service*>::const_iterator it = d->services.constBegin();
         it != d->services.constEnd(); ++it ) {
        Service* s = *it;
        if( rex.exactMatch( s->name() ) )
            l.append( s );
    }
    return l;
}


Nepomuk::Middleware::Service*  Nepomuk::Middleware::Registry::discoverServiceByType( const QString& type )
{
    // for testing
    d->backend->discoverServiceByType( type );

    updateServiceCache();

    for( QList<Nepomuk::Middleware::Service*>::const_iterator it = d->services.constBegin();
         it != d->services.constEnd(); ++it ) {
        Service* s = *it;
        if( s->type() == type )
            return s;
    }
    return 0;
}


QList<Nepomuk::Middleware::Service*>  Nepomuk::Middleware::Registry::discoverServicesByType( const QString& type )
{
    updateServiceCache();

    QList<Nepomuk::Middleware::Service*> l;
    for( QList<Nepomuk::Middleware::Service*>::const_iterator it = d->services.constBegin();
         it != d->services.constEnd(); ++it ) {
        Service* s = *it;
        if( s->type() == type )
            l.append( s );
    }
    return l;
}


QList<Nepomuk::Middleware::Service*> Nepomuk::Middleware::Registry::discoverAllServices()
{
    updateServiceCache();
    return d->services;
}


void Nepomuk::Middleware::Registry::slotServiceRegistered( const Nepomuk::Middleware::ServiceDesc& desc )
{
    // check if we already have that service and allow only one service per url
    if( d->serviceUrlMap.contains( desc.url ) )
        slotServiceUnregistered( desc );

    // create a new Service object
    Service* newService = new Service( desc, this );
    d->serviceUrlMap[desc.url] = newService;
    d->services.append( newService );

    // inform the world
    emit serviceRegistered( newService );
    emit serviceRegistered( desc.url );
}


void Nepomuk::Middleware::Registry::slotServiceUnregistered( const Nepomuk::Middleware::ServiceDesc& desc )
{
    QMap<QString, Service*>::iterator it = d->serviceUrlMap.find( desc.url );
    if( it != d->serviceUrlMap.end() ) {
        Service* s = it.value();

        d->serviceUrlMap.erase( it );
        d->services.removeAll( s );

        // inform the world
        emit serviceUnregistered( s );
        emit serviceUnregistered( desc.url );

        // and remove the service forever
        delete s;
    }
}


void Nepomuk::Middleware::Registry::updateServiceCache()
{
    //
    // Here we update the service cache. In case only our own DBus registry is running this method will
    // change nothing as we already hanhdle the signals. However, other registry implementations may not
    // provide these signals. To keep in sync we update the cache before each discover method call.
    //
    // The main reason for the cache is the ease of handling the Service objects.
    //

    QList<ServiceDesc> allServiceUrls = d->backend->discoverAllServices();
    QStringList urls;
    for( QList<ServiceDesc>::const_iterator it = allServiceUrls.constBegin();
         it != allServiceUrls.constEnd(); ++it ) {
        const ServiceDesc& desc = *it;
        urls.append( desc.url );
        if( !d->serviceUrlMap.contains( desc.url ) )
            slotServiceRegistered( desc );
    }

    // remove non-existing services
    for( QList<Nepomuk::Middleware::Service*>::const_iterator it = d->services.constBegin();
         it != d->services.constEnd(); ++it ) {
        Service* s = *it;
        if( !urls.contains( s->url() ) )
            slotServiceUnregistered( s->desc() );
    }
}


Nepomuk::Middleware::Service* Nepomuk::Middleware::Registry::discoverRDFRepository()
{
    return discoverServiceByType( "http://nepomuk.semanticdesktop.org/services/RDFRepository" );
}

#include "registry.moc"
