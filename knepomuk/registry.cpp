/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include "registry.h"
#include "service.h"
#include "servicepublisher.h"
#include "registrydbusbackend.h"

#include <qlist.h>
#include <qmap.h>

#include <kdebug.h>

#include <QtCore/QRegExp>
#include <QtCore/QStringList>


class Nepomuk::Backbone::Registry::Private
{
public:
    DBus::RegistryBackend* backend;

    QList<Service*> services;
    QMap<QString, Service*> serviceUrlMap;

    QMap<QString, ServicePublisher*> servicePublisherMap;
};


Nepomuk::Backbone::Registry::Registry( QObject* parent )
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


Nepomuk::Backbone::Registry::~Registry()
{
    delete d->backend;
    delete d;
}


int Nepomuk::Backbone::Registry::registerService( Nepomuk::Backbone::ServicePublisher* service )
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


int Nepomuk::Backbone::Registry::unregisterService( Nepomuk::Backbone::ServicePublisher* service )
{
    if( d->servicePublisherMap.contains( service->url() ) ) {
        d->backend->unregisterService( service );
        return 0;
    }
    else
        return -1; // FIXME: proper return value
}


int Nepomuk::Backbone::Registry::unregisterService( const QString& url )
{
    if( d->servicePublisherMap.contains( url ) )
        return unregisterService( d->servicePublisherMap[url] );
    else
        return -1; // FIXME: proper return value
}


Nepomuk::Backbone::Service* Nepomuk::Backbone::Registry::discoverServiceByUrl( const QString& url )
{
    updateServiceCache();
    return d->serviceUrlMap[url];
}


QList<Nepomuk::Backbone::Service*>  Nepomuk::Backbone::Registry::discoverServicesByName( const QString& name )
{
    updateServiceCache();

    QRegExp rex( name );
    QList<Nepomuk::Backbone::Service*> l;
    for( QList<Nepomuk::Backbone::Service*>::const_iterator it = d->services.constBegin();
         it != d->services.constEnd(); ++it ) {
        Service* s = *it;
        if( rex.exactMatch( s->name() ) )
            l.append( s );
    }
    return l;
}


Nepomuk::Backbone::Service*  Nepomuk::Backbone::Registry::discoverServiceByType( const QString& type )
{
    // for testing
    d->backend->discoverServiceByType( type );

    updateServiceCache();

    for( QList<Nepomuk::Backbone::Service*>::const_iterator it = d->services.constBegin();
         it != d->services.constEnd(); ++it ) {
        Service* s = *it;
        if( s->type() == type )
            return s;
    }
    return 0;
}


QList<Nepomuk::Backbone::Service*>  Nepomuk::Backbone::Registry::discoverServicesByType( const QString& type )
{
    updateServiceCache();

    QList<Nepomuk::Backbone::Service*> l;
    for( QList<Nepomuk::Backbone::Service*>::const_iterator it = d->services.constBegin();
         it != d->services.constEnd(); ++it ) {
        Service* s = *it;
        if( s->type() == type )
            l.append( s );
    }
    return l;
}


QList<Nepomuk::Backbone::Service*> Nepomuk::Backbone::Registry::discoverAllServices()
{
    updateServiceCache();
    return d->services;
}


void Nepomuk::Backbone::Registry::slotServiceRegistered( const Nepomuk::Backbone::ServiceDesc& desc )
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


void Nepomuk::Backbone::Registry::slotServiceUnregistered( const Nepomuk::Backbone::ServiceDesc& desc )
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


void Nepomuk::Backbone::Registry::updateServiceCache()
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
    for( QList<Nepomuk::Backbone::Service*>::const_iterator it = d->services.constBegin();
         it != d->services.constEnd(); ++it ) {
        Service* s = *it;
        if( !urls.contains( s->url() ) )
            slotServiceUnregistered( s->desc() );
    }
}


Nepomuk::Backbone::Service* Nepomuk::Backbone::Registry::discoverRDFRepository()
{
    return discoverServiceByType( "http://nepomuk.semanticdesktop.org/services/RDFRepository" );
}

#include "registry.moc"
