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

#include <solid/ifaces/network.h>

namespace Solid
{
    class Network::Private
    {
        public:
            Ifaces::Network * iface;
    };
}

Solid::Network::Network( Ifaces::Network *iface )
    : QObject(), d( new Private )
{
    d->iface = iface;

    connect( d->iface, SIGNAL( ipDetailsChanged() ),
             this, SIGNAL( ipDetailsChanged() ) );
}

Solid::Network::~Network()
{
    delete d;
}

QStringList Solid::Network::ipV4Addresses()
{
    return d->iface->ipV4Addresses();
}

QStringList Solid::Network::ipV6Addresses()
{
    return d->iface->ipV6Addresses();
}

QString Solid::Network::subnetMask()
{
    return d->iface->subnetMask();

}

QString Solid::Network::broadcastAddress()
{
    return d->iface->broadcastAddress();
}

QString Solid::Network::route()
{
    return d->iface->route();
}

QString Solid::Network::primaryDNS()
{
    return d->iface->primaryDNS();
}

QString Solid::Network::secondaryDNS()
{
    return d->iface->secondaryDNS();
}

void Solid::Network::setActivated( bool active )
{
    d->iface->setActivated( active );
}

QString Solid::Network::uni()
{
    return d->iface->uni();
}

#include "network.moc"
