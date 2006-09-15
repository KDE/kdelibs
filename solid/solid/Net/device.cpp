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

#include "device.h"

namespace Solid
{
namespace Net
{
    class Device::Private
    {
        public:
            Ifaces::Device * iface;
    };
}
}

Solid::Net::Device::Device( Solid::Net::Ifaces::Device * iface, QObject * parent ) : QObject( parent ), d( new Private )
{
    d->iface = iface;
    connect( d->iface, SIGNAL( activeChanged( bool ) ), this, SIGNAL( activeChanged( bool ) ) );
    connect( d->iface, SIGNAL( linkUpChanged( bool ) ), this, SIGNAL( linkUpChanged( bool ) ) );
    connect( d->iface, SIGNAL( signalStrengthChanged( int ) ), this, SIGNAL( signalStrengthChanged( int ) ) );
    connect( d->iface, SIGNAL( connectionStateChanged( ConnectionState ) ), this, SIGNAL( connectionStateChanged( ConnectionState ) ) );
}

Solid::Net::Device::~Device()
{
    delete d;
}

bool Solid::Net::Device::isActive()
{
    return d->iface->isActive();
}

Solid::Net::Ifaces::Enums::Device::Type Solid::Net::Device::type()
{
    return d->iface->type();

}
Solid::Net::Ifaces::Enums::Device::ConnectionState Solid::Net::Device::connectionState()
{
    return d->iface->connectionState();
}

int Solid::Net::Device::signalStrength()
{
    return d->iface->signalStrength();
}

int Solid::Net::Device::speed()
{
    return d->iface->speed();
}

bool Solid::Net::Device::isLinkUp()
{
    return d->iface->isLinkUp();
}

Solid::Net::Ifaces::Enums::Device::Capabilities Solid::Net::Device::capabilities()
{
    return d->iface->capabilities();
}

#include "device.moc"

