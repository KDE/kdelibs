/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>

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

#include "network.h"

#include "soliddefs_p.h"
#include <solid/ifaces/network.h>


Solid::Network::Network( QObject *backendObject )
    : FrontendObject()
{
    setBackendObject( backendObject );

    if ( backendObject )
    {
        connect( backendObject, SIGNAL( ipDetailsChanged() ),
                 this, SIGNAL( ipDetailsChanged() ) );
        connect( backendObject, SIGNAL( activationStateChanged( bool ) ),
                 this, SIGNAL( activationStateChanged( bool ) ) );
    }
}

Solid::Network::~Network()
{
}

QList<KNetwork::KIpAddress> Solid::Network::ipV4Addresses() const
{
    return_SOLID_CALL( Ifaces::Network*, backendObject(), QList<KNetwork::KIpAddress>(), ipV4Addresses() );
}

QList<KNetwork::KIpAddress> Solid::Network::ipV6Addresses() const
{
    return_SOLID_CALL( Ifaces::Network*, backendObject(), QList<KNetwork::KIpAddress>(), ipV6Addresses() );
}

QString Solid::Network::subnetMask() const
{
    return_SOLID_CALL( Ifaces::Network*, backendObject(), QString(), subnetMask() );
}

QString Solid::Network::broadcastAddress() const
{
    return_SOLID_CALL( Ifaces::Network*, backendObject(), QString(), broadcastAddress() );
}

QString Solid::Network::route() const
{
    return_SOLID_CALL( Ifaces::Network*, backendObject(), QString(), route() );
}

QList<KNetwork::KIpAddress> Solid::Network::dnsServers() const
{
    return_SOLID_CALL( Ifaces::Network*, backendObject(), QList<KNetwork::KIpAddress>(), dnsServers() );
}

void Solid::Network::setActivated( bool active )
{
    SOLID_CALL( Ifaces::Network*, backendObject(), setActivated(active) );
}

bool Solid::Network::isActive() const
{
    return_SOLID_CALL( Ifaces::Network*, backendObject(), false, isActive() );
}

QString Solid::Network::uni() const
{
    return_SOLID_CALL( Ifaces::Network*, backendObject(), QString(), uni() );
}

#include "network.moc"
