/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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
#include "network_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/network.h>

Solid::Network::Network( QObject *backendObject )
    : FrontendObject(*new NetworkPrivate)
{
    registerBackendObject( backendObject );
}

Solid::Network::Network( const Network &network )
    : FrontendObject(*new NetworkPrivate)
{
    registerBackendObject( network.backendObject() );
}

Solid::Network::Network(NetworkPrivate &dd, QObject *backendObject)
    : FrontendObject(dd)
{
    registerBackendObject( backendObject );
}

Solid::Network::Network(NetworkPrivate &dd, const Network &network)
    : FrontendObject(dd)
{
    registerBackendObject( network.backendObject() );
}

Solid::Network::~Network()
{

}

QList<QNetworkAddressEntry> Solid::Network::addressEntries() const
{
    return_SOLID_CALL( Ifaces::Network*, backendObject(), QList<QNetworkAddressEntry>(), addressEntries() );
}

QString Solid::Network::route() const
{
    return_SOLID_CALL( Ifaces::Network*, backendObject(), QString(), route() );
}

QList<QHostAddress> Solid::Network::dnsServers() const
{
    return_SOLID_CALL( Ifaces::Network*, backendObject(), QList<QHostAddress>(), dnsServers() );
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

void Solid::Network::registerBackendObject( QObject *backendObject )
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

#include "network.moc"
