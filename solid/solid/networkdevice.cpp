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

#include <QMap>
#include <QStringList>

#include <solid/ifaces/network.h>
#include <solid/ifaces/networkdevice.h>
#include <solid/ifaces/wirelessnetwork.h>

#include "network.h"
#include "networkdevice.h"
#include "wirelessnetwork.h"

namespace Solid
{
    class NetworkDevice::Private
    {
        public:
            Private() : iface( 0 ) {}
            ~Private();
            Ifaces::NetworkDevice * iface;
            Ifaces::Network *findRegisteredNetwork( const QString &udi );
            QMap<QString, Ifaces::Network*> networkMap;
    };
}


Solid::NetworkDevice::NetworkDevice()
    : QObject( ), d( new Private )
{
}

Solid::NetworkDevice::NetworkDevice( Ifaces::NetworkDevice *iface )
    : QObject( ), d( new Private )
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

Solid::NetworkDevice::NetworkDevice( const NetworkDevice &device )
    : QObject(), d( new Private )
{
    d->iface = device.d->iface;
}

Solid::NetworkDevice::~NetworkDevice()
{
    delete d;
}

Solid::NetworkDevice &Solid::NetworkDevice::operator=( const Solid::NetworkDevice & dev )
{
    disconnect( d->iface );
    delete d->iface;
    d->iface = dev.d->iface;

    connect( d->iface, SIGNAL( activeChanged( bool ) ),
             this, SIGNAL( activeChanged( bool ) ) );
    connect( d->iface, SIGNAL( linkUpChanged( bool ) ),
             this, SIGNAL( linkUpChanged( bool ) ) );
    connect( d->iface, SIGNAL( signalStrengthChanged( int ) ),
             this, SIGNAL( signalStrengthChanged( int ) ) );
    connect( d->iface, SIGNAL( connectionStateChanged( int ) ),
             this, SIGNAL( connectionStateChanged( int ) ) );

    return *this;
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

Solid::Network * Solid::NetworkDevice::findNetwork( const QString & udi )
{
    if ( d->iface == 0 ) return 0;

    return new Network( d->findRegisteredNetwork( udi ) );
}

Solid::NetworkList Solid::NetworkDevice::networks()
{
    Solid::NetworkList list;

    if ( d->iface == 0 ) return list;

    QStringList udiList = d->iface->networks();

    foreach( QString udi, udiList )
    {
        Ifaces::Network *network = d->findRegisteredNetwork( udi );
        if ( Ifaces::WirelessNetwork * wlan = dynamic_cast<Ifaces::WirelessNetwork *>( network ) )
            list.append( new Solid::WirelessNetwork( wlan ) );
        else
            list.append( new Network( network ) );
    }

    return list;
}

/***************************************************************************/

Solid::NetworkDevice::Private::~Private()
{
    foreach( Ifaces::Network * network, networkMap )
    {
        delete network;
    }
}

Solid::Ifaces::Network *Solid::NetworkDevice::Private::findRegisteredNetwork( const QString &udi )
{
    Ifaces::Network *network;

    if ( networkMap.contains( udi ) )
    {
        network = networkMap[udi];
    }
    else
    {
        network = iface->findNetwork( udi );

        if ( network != 0 )
        {
            networkMap[udi] = network;
        }
    }

    return network;
}

#include "networkdevice.moc"
