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

#include "registrydbusinterface.h"
#include "servicedbusinterfacebackend.h"
#include "dbustools.h"

#include <kdebug.h>


Nepomuk::Backbone::DBus::RegistryInterface::RegistryInterface( QObject* parent )
    : QDBusInterface( QLatin1String("org.semanticdesktop.nepomuk.ServiceRegistry"),
                      QLatin1String("/org/semanticdesktop/nepomuk/ServiceRegistry"),
                      QLatin1String("org.semanticdesktop.nepomuk.ServiceRegistry"),
                      QDBusConnection::sessionBus(),
                      parent )
{
}


Nepomuk::Backbone::DBus::RegistryInterface::~RegistryInterface()
{
}


int Nepomuk::Backbone::DBus::RegistryInterface::registerService( const Nepomuk::Backbone::ServiceDesc& desc )
{
    QVariant v;
    v.setValue( desc );
    QDBusReply<int> reply = call( QLatin1String("registerService"), v );
    if( reply.isValid() )
        return reply.value();
    else
        return -1; // FIXME: define proper error codes
}


int Nepomuk::Backbone::DBus::RegistryInterface::unregisterService( const Nepomuk::Backbone::ServiceDesc& desc )
{
    QVariant v;
    v.setValue( desc );
    QDBusReply<int> reply = call( QLatin1String("unregisterService"), v );
    if( reply.isValid() )
        return reply.value();
    else
        return -1; // FIXME: define proper error codes
}


int Nepomuk::Backbone::DBus::RegistryInterface::unregisterService( const QString& uri )
{
    QDBusReply<int> reply = call( QLatin1String("unregisterService"), uri );
    if( reply.isValid() )
        return reply.value();
    else
        return -1; // FIXME: define proper error codes
}


Nepomuk::Backbone::ServiceDesc
Nepomuk::Backbone::DBus::RegistryInterface::discoverServiceByUrl( const QString& type )
{
    QDBusReply<ServiceDesc> reply = call( QLatin1String("discoverServiceByUrl"), type );
    return reply.value();
}


Nepomuk::Backbone::ServiceDesc
Nepomuk::Backbone::DBus::RegistryInterface::discoverServiceByType( const QString& type )
{
    QDBusReply<ServiceDesc> reply = call( QLatin1String("discoverServiceByType"), type );
    return reply.value();
}


QList<Nepomuk::Backbone::ServiceDesc>
Nepomuk::Backbone::DBus::RegistryInterface::discoverServicesByType( const QString& type )
{
    QDBusReply<QList<ServiceDesc> > reply = call( QLatin1String("discoverServicesByType"), type );
    return reply.value();
}


QList<Nepomuk::Backbone::ServiceDesc>
Nepomuk::Backbone::DBus::RegistryInterface::discoverServicesByName( const QString& name )
{
    QDBusReply<QList<ServiceDesc> > reply = call( QLatin1String("discoverServicesByName"), name );
    return reply.value();
}


QList<Nepomuk::Backbone::ServiceDesc>
Nepomuk::Backbone::DBus::RegistryInterface::discoverAllServices()
{
    QDBusReply<QList<ServiceDesc> > reply = call( QLatin1String("discoverAllServices") );
    if( !reply.isValid() )
        kDebug(300001) << "(Nepomuk::Backbone::DBus::RegistryInterface::discoverAllServices)" << reply.error().message() << endl;
    return reply.value();
}

#include "registrydbusinterface.moc"
