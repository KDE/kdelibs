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

namespace Solid
{
namespace Net
{
    class WirelessNetwork::Private
    {
        public:
            Ifaces::WirelessNetwork * iface;
    };
}
}

Solid::Net::WirelessNetwork::WirelessNetwork( Ifaces::Network * networkIface, Ifaces::WirelessNetwork * wirelessNetIface, QObject * parent ) : Network( networkIface, parent ), d( new Private )
{
    d->iface = wirelessNetIface;
    connect( d->iface, SIGNAL( signalStrengthChanged( int ) ), this, SIGNAL( signalStrengthChanged( int ) ) );
    connect( d->iface, SIGNAL( bitrateChanged( int ) ), this, SIGNAL( lbitrateChanged( int ) ) );
    connect( d->iface, SIGNAL( associationChanged( bool ) ), this, SIGNAL( associationChanged( bool ) ) );
    connect( d->iface, SIGNAL( activeChanged( bool ) ), this, SIGNAL( activeChanged( bool ) ) );
}

Solid::Net::WirelessNetwork::~WirelessNetwork()
{
    delete d;
}

bool Solid::Net::WirelessNetwork::isSameAs( const WirelessNetwork & other) const
{
    return ( other.essid() == essid() &&
        other.bssList() == /*intersects*/ bssList() );
}

int Solid::Net::WirelessNetwork::signalStrength() const
{
    return d->iface->signalStrength();
}

int Solid::Net::WirelessNetwork::bitRate() const
{
    return d->iface->bitRate();
}

int Solid::Net::WirelessNetwork::frequency() const
{
    return d->iface->frequency();
}

MacAddressList Solid::Net::WirelessNetwork::bssList() const
{
    return d->iface->bssList();
}

Solid::Net::Ifaces::Authentication * Solid::Net::WirelessNetwork::authentication() const
{
    return d->iface->authentication();
}

Solid::Net::Ifaces::WirelessNetwork::Capabilities Solid::Net::WirelessNetwork::capabilities() const
{
    return d->iface->capabilities();
}

QString Solid::Net::WirelessNetwork::essid() const
{
    return d->iface->essid();
}

Solid::Net::Ifaces::WirelessNetwork::OperationMode Solid::Net::WirelessNetwork::mode() const
{
    return d->iface->mode();
}

bool Solid::Net::WirelessNetwork::isAssociated() const
{
    return d->iface->isAssociated();
}

bool Solid::Net::WirelessNetwork::isEncrypted() const
{
    return d->iface->isEncrypted();
}

bool Solid::Net::WirelessNetwork::isHidden() const
{
    return d->iface->isHidden();
}

bool Solid::Net::WirelessNetwork::isActive() const
{
    return d->iface->isActive();
}

#include "wirelessnetwork.moc"
