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

#include "registrydbusinterface.h"
#include "servicedbusinterfacebackend.h"
#include "dbustools.h"

#include <QDBusReply>

#include <kdebug.h>

Nepomuk::Middleware::DBus::RegistryInterface::RegistryInterface( QObject* parent )
    : QDBusInterface( QLatin1String("org.semanticdesktop.nepomuk.ServiceRegistry"),
                      QLatin1String("/org/semanticdesktop/nepomuk/ServiceRegistry"),
                      QLatin1String("org.semanticdesktop.nepomuk.ServiceRegistry"),
                      QDBusConnection::sessionBus(),
                      parent )
{
}


Nepomuk::Middleware::DBus::RegistryInterface::~RegistryInterface()
{
}


int Nepomuk::Middleware::DBus::RegistryInterface::registerService( const Nepomuk::Middleware::ServiceDesc& desc )
{
    QVariant v;
    v.setValue( desc );
    QDBusReply<int> reply = call( QLatin1String("registerService"), v );
    if( reply.isValid() )
        return reply.value();
    else
        return -1; // FIXME: define proper error codes
}


int Nepomuk::Middleware::DBus::RegistryInterface::unregisterService( const Nepomuk::Middleware::ServiceDesc& desc )
{
    QVariant v;
    v.setValue( desc );
    QDBusReply<int> reply = call( QLatin1String("unregisterService"), v );
    if( reply.isValid() )
        return reply.value();
    else
        return -1; // FIXME: define proper error codes
}


int Nepomuk::Middleware::DBus::RegistryInterface::unregisterService( const QString& uri )
{
    QDBusReply<int> reply = call( QLatin1String("unregisterService"), uri );
    if( reply.isValid() )
        return reply.value();
    else
        return -1; // FIXME: define proper error codes
}


Nepomuk::Middleware::ServiceDesc
Nepomuk::Middleware::DBus::RegistryInterface::discoverServiceByUrl( const QString& type )
{
    QDBusReply<ServiceDesc> reply = call( QLatin1String("discoverServiceByUrl"), type );
    return reply.value();
}


Nepomuk::Middleware::ServiceDesc
Nepomuk::Middleware::DBus::RegistryInterface::discoverServiceByType( const QString& type )
{
    QDBusReply<ServiceDesc> reply = call( QLatin1String("discoverServiceByType"), type );
    return reply.value();
}


QList<Nepomuk::Middleware::ServiceDesc>
Nepomuk::Middleware::DBus::RegistryInterface::discoverServicesByType( const QString& type )
{
    QDBusReply<QList<ServiceDesc> > reply = call( QLatin1String("discoverServicesByType"), type );
    return reply.value();
}


QList<Nepomuk::Middleware::ServiceDesc>
Nepomuk::Middleware::DBus::RegistryInterface::discoverServicesByName( const QString& name )
{
    QDBusReply<QList<ServiceDesc> > reply = call( QLatin1String("discoverServicesByName"), name );
    return reply.value();
}


QList<Nepomuk::Middleware::ServiceDesc>
Nepomuk::Middleware::DBus::RegistryInterface::discoverAllServices()
{
    QDBusReply<QList<ServiceDesc> > reply = call( QLatin1String("discoverAllServices") );
    if( !reply.isValid() )
        kDebug(300001) << "(Nepomuk::Middleware::DBus::RegistryInterface::discoverAllServices)" << reply.error().message();
    return reply.value();
}

#include "registrydbusinterface.moc"
