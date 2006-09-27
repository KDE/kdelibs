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

#include "networkmanager.h"
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
            QObject *findRegisteredNetwork( const QString &uni );
            QMap<QString, QObject*> networkMap;
    };
}


Solid::NetworkDevice::NetworkDevice()
    : QObject( ), d( new Private )
{
}

Solid::NetworkDevice::NetworkDevice( const QString &uni )
    : QObject(), d( new Private )
{
    const NetworkDevice &device = NetworkManager::self().findNetworkDevice( uni );

    d->iface = device.d->iface;

    connect( d->iface, SIGNAL( activeChanged( bool ) ),
             this, SIGNAL( activeChanged( bool ) ) );
    connect( d->iface, SIGNAL( linkUpChanged( bool ) ),
             this, SIGNAL( linkUpChanged( bool ) ) );
    connect( d->iface, SIGNAL( signalStrengthChanged( int ) ),
             this, SIGNAL( signalStrengthChanged( int ) ) );
    connect( d->iface, SIGNAL( connectionStateChanged( int ) ),
             this, SIGNAL( connectionStateChanged( int ) ) );
    connect( d->iface, SIGNAL( destroyed( QObject * ) ),
             this, SLOT( slotDestroyed( QObject * ) ) );
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
    connect( d->iface, SIGNAL( destroyed( QObject * ) ),
             this, SLOT( slotDestroyed( QObject * ) ) );
}

Solid::NetworkDevice::NetworkDevice( const NetworkDevice &device )
    : QObject(), d( new Private )
{
    d->iface = device.d->iface;

    connect( d->iface, SIGNAL( activeChanged( bool ) ),
             this, SIGNAL( activeChanged( bool ) ) );
    connect( d->iface, SIGNAL( linkUpChanged( bool ) ),
             this, SIGNAL( linkUpChanged( bool ) ) );
    connect( d->iface, SIGNAL( signalStrengthChanged( int ) ),
             this, SIGNAL( signalStrengthChanged( int ) ) );
    connect( d->iface, SIGNAL( connectionStateChanged( int ) ),
             this, SIGNAL( connectionStateChanged( int ) ) );
    connect( d->iface, SIGNAL( destroyed( QObject * ) ),
             this, SLOT( slotDestroyed( QObject * ) ) );
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

bool Solid::NetworkDevice::isValid() const
{
    return d->iface!=0;
}

QString Solid::NetworkDevice::uni() const
{
    return d->iface->uni();
}

bool Solid::NetworkDevice::isActive() const
{
    return d->iface->isActive();
}

Solid::NetworkDevice::Type Solid::NetworkDevice::type() const
{
    return d->iface->type();

}
Solid::NetworkDevice::ConnectionState Solid::NetworkDevice::connectionState() const
{
    return d->iface->connectionState();
}

int Solid::NetworkDevice::signalStrength() const
{
    return d->iface->signalStrength();
}

int Solid::NetworkDevice::speed() const
{
    return d->iface->speed();
}

bool Solid::NetworkDevice::isLinkUp() const
{
    return d->iface->isLinkUp();
}

Solid::NetworkDevice::Capabilities Solid::NetworkDevice::capabilities() const
{
    return d->iface->capabilities();
}

Solid::Network * Solid::NetworkDevice::findNetwork( const QString & uni ) const
{
    if ( d->iface == 0 ) return 0;

    return new Network( d->findRegisteredNetwork( uni ) );
}

Solid::NetworkList Solid::NetworkDevice::networks() const
{
    Solid::NetworkList list;

    if ( d->iface == 0 ) return list;

    QStringList uniList = d->iface->networks();

    foreach( QString uni, uniList )
    {
        QObject *network = d->findRegisteredNetwork( uni );
        if ( qobject_cast<Ifaces::WirelessNetwork *>( network )!=0 )
            list.append( new Solid::WirelessNetwork( network ) );
        else
            list.append( new Network( network ) );
    }

    return list;
}

void Solid::NetworkDevice::slotDestroyed( QObject *object )
{
    if ( object == d->iface )
    {
        d->iface = 0;

        foreach( QObject *network, d->networkMap )
        {
            delete network;
        }
    }
}

/***************************************************************************/

Solid::NetworkDevice::Private::~Private()
{
    foreach( QObject *network, networkMap )
    {
        delete network;
    }
}

QObject *Solid::NetworkDevice::Private::findRegisteredNetwork( const QString &uni )
{
    QObject *network;

    if ( networkMap.contains( uni ) )
    {
        network = networkMap[uni];
    }
    else
    {
        network = iface->createNetwork( uni );

        if ( network != 0 )
        {
            networkMap[uni] = network;
        }
    }

    return network;
}

#include "networkdevice.moc"
