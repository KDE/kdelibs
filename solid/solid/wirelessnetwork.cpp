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

#include "wirelessnetwork.h"

#include <solid/ifaces/wirelessnetwork.h>

namespace Solid
{
    class WirelessNetwork::Private
    {
        public:
            Ifaces::WirelessNetwork * iface;
    };
}

Solid::WirelessNetwork::WirelessNetwork( Ifaces::WirelessNetwork *iface )
    : Network( iface ), d( new Private )
{
    d->iface = iface;

    connect( d->iface, SIGNAL( signalStrengthChanged( int ) ),
             this, SIGNAL( signalStrengthChanged( int ) ) );
    connect( d->iface, SIGNAL( bitrateChanged( int ) ),
             this, SIGNAL( lbitrateChanged( int ) ) );
    connect( d->iface, SIGNAL( associationChanged( bool ) ),
             this, SIGNAL( associationChanged( bool ) ) );
    connect( d->iface, SIGNAL( activeChanged( bool ) ),
             this, SIGNAL( activeChanged( bool ) ) );
}

Solid::WirelessNetwork::~WirelessNetwork()
{
    delete d;
}

bool Solid::WirelessNetwork::isSameAs( const WirelessNetwork & other) const
{
    return ( other.essid() == essid() &&
             other.bssList() == /*intersects*/ bssList() );
}

int Solid::WirelessNetwork::signalStrength() const
{
    return d->iface->signalStrength();
}

int Solid::WirelessNetwork::bitRate() const
{
    return d->iface->bitRate();
}

int Solid::WirelessNetwork::frequency() const
{
    return d->iface->frequency();
}

Solid::MacAddressList Solid::WirelessNetwork::bssList() const
{
    return d->iface->bssList();
}

Solid::Authentication *Solid::WirelessNetwork::authentication() const
{
    return d->iface->authentication();
}

Solid::WirelessNetwork::Capabilities Solid::WirelessNetwork::capabilities() const
{
    return d->iface->capabilities();
}

QString Solid::WirelessNetwork::essid() const
{
    return d->iface->essid();
}

Solid::WirelessNetwork::OperationMode Solid::WirelessNetwork::mode() const
{
    return d->iface->mode();
}

bool Solid::WirelessNetwork::isAssociated() const
{
    return d->iface->isAssociated();
}

bool Solid::WirelessNetwork::isEncrypted() const
{
    return d->iface->isEncrypted();
}

bool Solid::WirelessNetwork::isHidden() const
{
    return d->iface->isHidden();
}

bool Solid::WirelessNetwork::isActive() const
{
    return d->iface->isActive();
}

#include "wirelessnetwork.moc"
