/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include <kglobal.h>

#include "soliddefs_p.h"
#include "networking_p.h"
#include "networking.h"
#include "org_kde_solid_networking.h"

K_GLOBAL_STATIC(Solid::NetworkingPrivate, globalNetworkManager)

Solid::NetworkingPrivate::NetworkingPrivate() : iface( 
        new OrgKdeSolidNetworkingInterface( "org.kde.Solid.Networking",
            "/status",
            QDBusConnection::sessionBus(),
            this ) )
{
    connect( iface, SIGNAL( statusChanged( uint ) ), globalNetworkManager, SIGNAL( statusChanged( uint ) ) );
}

Solid::NetworkingPrivate::~NetworkingPrivate()
{
}

uint Solid::NetworkingPrivate::requestConnection()
{
    return iface->requestConnection();
}

void Solid::NetworkingPrivate::releaseConnection()
{
    iface->releaseConnection();
}

uint Solid::NetworkingPrivate::status() const
{
    return iface->status();
}

Solid::Networking::Result Solid::Networking::requestConnection()
{
    return static_cast<Solid::Networking::Result>( globalNetworkManager->requestConnection() );
}

void Solid::Networking::releaseConnection()
{
    globalNetworkManager->releaseConnection();
}

Solid::Networking::Status Solid::Networking::status()
{
    return static_cast<Solid::Networking::Status>( globalNetworkManager->status() );
}

Solid::Networking::Notifier *Solid::Networking::notifier()
{
    return globalNetworkManager;
}

#include "networking_p.moc"
#include "networking.moc"
