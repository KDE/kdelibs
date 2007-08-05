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

#include "registrydbusbackend.h"
#include "registrydbusinterface.h"
#include "servicepublisherdbusinterface.h"
#include "dbustools.h"
#include "servicepublisher.h"
#include "servicedesc.h"

#include "services/dummyservice.h"
#include "services/rdfrepositorypublisherdbusinterface.h"
#include "rdf/rdfdbus.h"

#include <kdebug.h>



Nepomuk::Middleware::DBus::RegistryBackend::RegistryBackend( QObject* parent )
    : QObject( parent )
{
    // register our ServiceDesc structure for usage with QtDBus
    qDBusRegisterMetaType<Nepomuk::Middleware::ServiceDesc>();
    qDBusRegisterMetaType<QList<Nepomuk::Middleware::ServiceDesc> >();

    // register our RDF structures for usage with QtDBus
    // FIXME: it would be nicer to have these somewhere else
    qDBusRegisterMetaType<Soprano::Node>();
    qDBusRegisterMetaType<Soprano::Statement>();
    qDBusRegisterMetaType<QList<Soprano::Statement> >();
    qDBusRegisterMetaType<QList<Soprano::Node> >();
    qDBusRegisterMetaType<RDF::QueryResultTable>();

    // register the default service types
    // FIXME: I don't like these fixed type urls
    //        Why don't we just put a createDBusInterface method in the ServicePublisher
    registerServiceType( "http://nepomuk.semanticdesktop.org/services/DummyService",
                         new ServicePublisherTypeInterfaceFactory<DummyServicePublisherInterface>() );
    registerServiceType( "http://nepomuk.semanticdesktop.org/services/RDFRepository",
                         new ServicePublisherTypeInterfaceFactory<Nepomuk::Services::DBus::RDFRepositoryPublisherInterface>() );

    m_interface = new RegistryInterface( this );

    connect( m_interface, SIGNAL(serviceRegistered(const ServiceDesc&)),
             this, SIGNAL(serviceRegistered(const ServiceDesc&)) );
    connect( m_interface, SIGNAL(serviceUnregistered(const ServiceDesc&)),
             this, SIGNAL(serviceRegistered(const ServiceDesc&)) );
}


Nepomuk::Middleware::DBus::RegistryBackend::~RegistryBackend()
{
}


int Nepomuk::Middleware::DBus::RegistryBackend::registerService( Nepomuk::Middleware::ServicePublisher* service )
{
    // TODO: maybe reparent the service to this registry object

    //
    // Check if we have an interface for this type
    //
    DBus::ServicePublisherInterfaceFactory* factory = DBus::servicePublisherInterfaceFactory( service->type() );
    if( !factory ) {
        kDebug(300001) << "(Nepomuk::Middleware::Registry) No service type found for " << service->type();
        // FIXME: proper return values
        return -1;
    }

    //
    // Create the service dbus interface adaptor
    //
    factory->createInterface( service );

    //
    // register the Nepomuk Service as a DBus service under the name as defined by the standard
    //
    if( !QDBusConnection::sessionBus().registerService( DBus::dbusServiceFromUrl(service->url()) ) ) {
        kDebug(300001) << "(Nepomuk::Middleware::Registry) failed to register service name " << DBus::dbusServiceFromUrl(service->url());
        // FIXME: proper return values
        return -1;
    }

    //
    // Register the object as defined by the standard
    //
    if( !QDBusConnection::sessionBus().registerObject( DBus::dbusObjectFromType(service->type()), service ) ) {
        // cleanup
        QDBusConnection::sessionBus().unregisterService( DBus::dbusServiceFromUrl(service->url()) );

        kDebug(300001) << "(Nepomuk::Middleware::Registry) failed to register object " << DBus::dbusObjectFromType(service->type());
        // FIXME: proper return values
        return -1;
    }
//   if( !QDBusConnection::sessionBus().registerObject( "/org/semanticdesktop/nepomuk/Service", service ) ) {
//     // cleanup
//     QDBusConnection::sessionBus().unregisterObject( DBus::dbusObjectFromType(service->type()) );
//     QDBusConnection::sessionBus().unregisterService( DBus::dbusServiceFromUrl(service->url()) );

//     kDebug(300001) << "(Nepomuk::Middleware::Registry) failed to register object " << "/org/semanticdesktop/nepomuk/Service";
//     // FIXME: proper return values
//     return -1;
//   }

    //
    // Actually try to register the service with the registry
    //
    if( m_interface->registerService( service->desc() ) != 0 ) {
        // cleanup
        //    QDBusConnection::sessionBus().unregisterObject( "/org/semanticdesktop/nepomuk/Service" );
        QDBusConnection::sessionBus().unregisterObject( DBus::dbusObjectFromType(service->type()) );
        QDBusConnection::sessionBus().unregisterService( DBus::dbusServiceFromUrl(service->url()) );

        kDebug(300001) << "(Nepomuk::Middleware::Registry) failed to register service with registry ";
        // FIXME: proper return values
        return -1;
    }

    return 0;
}


int Nepomuk::Middleware::DBus::RegistryBackend::unregisterService( Nepomuk::Middleware::ServicePublisher* service )
{
    //
    // Unregister the service from the regisry
    //
    m_interface->unregisterService( service->url() );

    //
    // Unregister the DBus services
    //
    //    QDBusConnection::sessionBus().unregisterObject( "/org/semanticdesktop/nepomuk/Service" );
    QDBusConnection::sessionBus().unregisterObject( DBus::dbusObjectFromType(service->type()) );
    QDBusConnection::sessionBus().unregisterService( DBus::dbusServiceFromUrl(service->url()) );

    return 0;
}


Nepomuk::Middleware::ServiceDesc Nepomuk::Middleware::DBus::RegistryBackend::discoverServiceByUrl( const QString& url )
{
    return m_interface->discoverServiceByUrl( url );
}


Nepomuk::Middleware::ServiceDesc Nepomuk::Middleware::DBus::RegistryBackend::discoverServiceByType( const QString& type )
{
    return m_interface->discoverServiceByType( type );
}


QList<Nepomuk::Middleware::ServiceDesc> Nepomuk::Middleware::DBus::RegistryBackend::discoverServicesByName( const QString& name )
{
    return m_interface->discoverServicesByName( name );
}


QList<Nepomuk::Middleware::ServiceDesc> Nepomuk::Middleware::DBus::RegistryBackend::discoverServicesByType( const QString& type )
{
    return m_interface->discoverServicesByType( type );
}


QList<Nepomuk::Middleware::ServiceDesc> Nepomuk::Middleware::DBus::RegistryBackend::discoverAllServices()
{
    return m_interface->discoverAllServices();
}

#include "registrydbusbackend.moc"
