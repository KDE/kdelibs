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

#include "networkdevice.h"

#include <solid/ifaces/networkdevice.h>

namespace Solid
{
    class NetworkDevice::Private
    {
        public:
            Ifaces::NetworkDevice * iface;
    };
}

Solid::NetworkDevice::NetworkDevice( Ifaces::NetworkDevice *iface, QObject *parent )
    : QObject( parent ), d( new Private )
{
    d->iface = iface;

    connect( d->iface, SIGNAL( activeChanged( bool ) ),
             this, SIGNAL( activeChanged( bool ) ) );
    connect( d->iface, SIGNAL( linkUpChanged( bool ) ),
             this, SIGNAL( linkUpChanged( bool ) ) );
    connect( d->iface, SIGNAL( signalStrengthChanged( int ) ),
             this, SIGNAL( signalStrengthChanged( int ) ) );
    connect( d->iface, SIGNAL( connectionStateChanged( int ) ),
             this, SIGNAL( connectionStateChanged( int ) ) );
}

Solid::NetworkDevice::~NetworkDevice()
{
    delete d;
}

bool Solid::NetworkDevice::isActive()
{
    return d->iface->isActive();
}

Solid::NetworkDevice::Type Solid::NetworkDevice::type()
{
    return d->iface->type();

}
Solid::NetworkDevice::ConnectionState Solid::NetworkDevice::connectionState()
{
    return d->iface->connectionState();
}

int Solid::NetworkDevice::signalStrength()
{
    return d->iface->signalStrength();
}

int Solid::NetworkDevice::speed()
{
    return d->iface->speed();
}

bool Solid::NetworkDevice::isLinkUp()
{
    return d->iface->isLinkUp();
}

Solid::NetworkDevice::Capabilities Solid::NetworkDevice::capabilities()
{
    return d->iface->capabilities();
}

#include "networkdevice.moc"

