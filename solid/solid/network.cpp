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

namespace Solid
{
    class Network::Private
    {
    public:
        QObject *backendObject;
    };
}

Solid::Network::Network( QObject *backendObject )
    : QObject(), d( new Private )
{
    d->backendObject = backendObject;

    connect( backendObject, SIGNAL( ipDetailsChanged() ),
             this, SIGNAL( ipDetailsChanged() ) );
}

Solid::Network::~Network()
{
    delete d;
}

QStringList Solid::Network::ipV4Addresses()
{
    return_SOLID_CALL( Ifaces::Network*, backendObject(), QStringList(), ipV4Addresses() );
}

QStringList Solid::Network::ipV6Addresses()
{
    return_SOLID_CALL( Ifaces::Network*, backendObject(), QStringList(), ipV6Addresses() );
}

QString Solid::Network::subnetMask()
{
    return_SOLID_CALL( Ifaces::Network*, backendObject(), QString(), subnetMask() );
}

QString Solid::Network::broadcastAddress()
{
    return_SOLID_CALL( Ifaces::Network*, backendObject(), QString(), broadcastAddress() );
}

QString Solid::Network::route()
{
    return_SOLID_CALL( Ifaces::Network*, backendObject(), QString(), route() );
}

QString Solid::Network::primaryDNS()
{
    return_SOLID_CALL( Ifaces::Network*, backendObject(), QString(), primaryDNS() );
}

QString Solid::Network::secondaryDNS()
{
    return_SOLID_CALL( Ifaces::Network*, backendObject(), QString(), secondaryDNS() );
}

void Solid::Network::setActivated( bool active )
{
    SOLID_CALL( Ifaces::Network*, backendObject(), setActivated(active) );
}

QString Solid::Network::uni()
{
    return_SOLID_CALL( Ifaces::Network*, backendObject(), QString(), uni() );
}

QObject *Solid::Network::backendObject()
{
    return d->backendObject;
}

const QObject *Solid::Network::backendObject() const
{
    return d->backendObject;
}

#include "network.moc"
