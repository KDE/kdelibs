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

#include "soliddefs_p.h"
#include "networkmanager.h"
#include "network.h"
#include "networkdevice.h"
#include "wirelessnetwork.h"

namespace Solid
{
    class NetworkDevice::Private
    {
    public:
        QMap<QString, Network*> networkMap;
        Network invalidNetwork;
    };
}


Solid::NetworkDevice::NetworkDevice()
    : FrontendObject(), d( new Private )
{
}

Solid::NetworkDevice::NetworkDevice( const QString &uni )
    : FrontendObject(), d( new Private )
{
    const NetworkDevice &device = NetworkManager::self().findNetworkDevice( uni );
    registerBackendObject( device.backendObject() );
}

Solid::NetworkDevice::NetworkDevice( QObject *backendObject )
    : FrontendObject(), d( new Private )
{
    registerBackendObject( backendObject );
}

Solid::NetworkDevice::NetworkDevice( const NetworkDevice &device )
    : FrontendObject(), d( new Private )
{
    registerBackendObject( device.backendObject() );
}

Solid::NetworkDevice::~NetworkDevice()
{
    foreach( QObject *network, d->networkMap )
    {
        delete network;
    }

    delete d;
}

Solid::NetworkDevice &Solid::NetworkDevice::operator=( const Solid::NetworkDevice & dev )
{
    unregisterBackendObject();
    registerBackendObject( dev.backendObject() );

    return *this;
}

QString Solid::NetworkDevice::uni() const
{
    return_SOLID_CALL( Ifaces::NetworkDevice*, backendObject(), QString(), uni() );
}

bool Solid::NetworkDevice::isActive() const
{
    return_SOLID_CALL( Ifaces::NetworkDevice*, backendObject(), false, isActive() );
}

Solid::NetworkDevice::Type Solid::NetworkDevice::type() const
{
    return_SOLID_CALL( Ifaces::NetworkDevice*, backendObject(), UnknownType, type() );
}
Solid::NetworkDevice::ConnectionState Solid::NetworkDevice::connectionState() const
{
    return_SOLID_CALL( Ifaces::NetworkDevice*, backendObject(), UnknownState, connectionState() );
}

int Solid::NetworkDevice::signalStrength() const
{
    return_SOLID_CALL( Ifaces::NetworkDevice*, backendObject(), 0, signalStrength() );
}

int Solid::NetworkDevice::speed() const
{
    return_SOLID_CALL( Ifaces::NetworkDevice*, backendObject(), 0, speed() );
}

bool Solid::NetworkDevice::isLinkUp() const
{
    return_SOLID_CALL( Ifaces::NetworkDevice*, backendObject(), false, isLinkUp() );
}

Solid::NetworkDevice::Capabilities Solid::NetworkDevice::capabilities() const
{
    return_SOLID_CALL( Ifaces::NetworkDevice*, backendObject(), Capabilities(), capabilities() );
}

Solid::Network * Solid::NetworkDevice::findNetwork( const QString & uni ) const
{
    if ( !isValid() ) return 0;

    Network *network = findRegisteredNetwork( uni );

    if ( network!=0 )
    {
        return network;
    }
    else
    {
        return &( d->invalidNetwork );
    }
}

Solid::NetworkList Solid::NetworkDevice::networks() const
{
    NetworkList list;
    Ifaces::NetworkDevice *device = qobject_cast<Ifaces::NetworkDevice*>( backendObject() );

    if ( device==0 ) return list;

    QStringList uniList = device->networks();

    foreach( QString uni, uniList )
    {
        Network *network = findRegisteredNetwork( uni );
        if ( network!=0 )
        {
            list.append( network );
        }
    }

    return list;
}

void Solid::NetworkDevice::slotDestroyed( QObject *object )
{
    if ( object == backendObject() )
    {
        FrontendObject::slotDestroyed(object);

        foreach( Network *network, d->networkMap )
        {
            delete network->backendObject();
            delete network;
        }

        d->networkMap.clear();
    }
}

void Solid::NetworkDevice::registerBackendObject( QObject *backendObject )
{
    setBackendObject( backendObject );

    if ( backendObject )
    {
        connect( backendObject, SIGNAL( activeChanged( bool ) ),
                 this, SIGNAL( activeChanged( bool ) ) );
        connect( backendObject, SIGNAL( linkUpChanged( bool ) ),
                 this, SIGNAL( linkUpChanged( bool ) ) );
        connect( backendObject, SIGNAL( signalStrengthChanged( int ) ),
                 this, SIGNAL( signalStrengthChanged( int ) ) );
        connect( backendObject, SIGNAL( connectionStateChanged( int ) ),
                 this, SIGNAL( connectionStateChanged( int ) ) );
        connect( backendObject, SIGNAL( networkAppeared( const QString & ) ),
                 this, SIGNAL( networkAppeared( const QString & ) ) );
        connect( backendObject, SIGNAL( networkDisappeared( const QString & ) ),
                 this, SIGNAL( networkDisappeared( const QString & ) ) );
    }
}

void Solid::NetworkDevice::unregisterBackendObject()
{
    setBackendObject( 0 );

    foreach( QObject *network, d->networkMap )
    {
        delete network;
    }

    d->networkMap.clear();
}

Solid::Network *Solid::NetworkDevice::findRegisteredNetwork( const QString &uni ) const
{
    Network *network = 0;

    if ( d->networkMap.contains( uni ) )
    {
        network = d->networkMap[uni];
    }
    else
    {
        Ifaces::NetworkDevice *device = qobject_cast<Ifaces::NetworkDevice*>( backendObject() );

        if ( device!=0 )
        {
            QObject *iface = device->createNetwork( uni );

            if ( qobject_cast<Ifaces::WirelessNetwork *>( iface )!=0 )
            {
                network = new WirelessNetwork( network );
            }
            else if ( qobject_cast<Ifaces::Network *>( iface )!=0 )
            {
                network = new Network( network );
            }

            if ( network != 0 )
            {
                d->networkMap[uni] = network;
            }
        }
    }

    return network;
}

#include "networkdevice.moc"
