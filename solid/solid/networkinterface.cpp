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
#include <solid/ifaces/networkinterface.h>
#include <solid/ifaces/wirelessnetwork.h>

#include "soliddefs_p.h"
#include "networkmanager.h"
#include "network.h"
#include "networkinterface.h"
#include "wirelessnetwork.h"

namespace Solid
{
    class NetworkInterface::Private
    {
    public:
        QMap<QString, Network*> networkMap;
        Network invalidNetwork;
    };
}


Solid::NetworkInterface::NetworkInterface()
    : FrontendObject(), d( new Private )
{
}

Solid::NetworkInterface::NetworkInterface( const QString &uni )
    : FrontendObject(), d( new Private )
{
    const NetworkInterface &device = NetworkManager::self().findNetworkInterface( uni );
    registerBackendObject( device.backendObject() );
}

Solid::NetworkInterface::NetworkInterface( QObject *backendObject )
    : FrontendObject(), d( new Private )
{
    registerBackendObject( backendObject );
}

Solid::NetworkInterface::NetworkInterface( const NetworkInterface &device )
    : FrontendObject(), d( new Private )
{
    registerBackendObject( device.backendObject() );
}

Solid::NetworkInterface::~NetworkInterface()
{
    foreach( QObject *network, d->networkMap )
    {
        delete network;
    }

    delete d;
}

Solid::NetworkInterface &Solid::NetworkInterface::operator=( const Solid::NetworkInterface & dev )
{
    unregisterBackendObject();
    registerBackendObject( dev.backendObject() );

    return *this;
}

QString Solid::NetworkInterface::uni() const
{
    return_SOLID_CALL( Ifaces::NetworkInterface*, backendObject(), QString(), uni() );
}

bool Solid::NetworkInterface::isActive() const
{
    return_SOLID_CALL( Ifaces::NetworkInterface*, backendObject(), false, isActive() );
}

Solid::NetworkInterface::Type Solid::NetworkInterface::type() const
{
    return_SOLID_CALL( Ifaces::NetworkInterface*, backendObject(), UnknownType, type() );
}
Solid::NetworkInterface::ConnectionState Solid::NetworkInterface::connectionState() const
{
    return_SOLID_CALL( Ifaces::NetworkInterface*, backendObject(), UnknownState, connectionState() );
}

int Solid::NetworkInterface::signalStrength() const
{
    return_SOLID_CALL( Ifaces::NetworkInterface*, backendObject(), 0, signalStrength() );
}

int Solid::NetworkInterface::designSpeed() const
{
    return_SOLID_CALL( Ifaces::NetworkInterface*, backendObject(), 0, designSpeed() );
}

bool Solid::NetworkInterface::isLinkUp() const
{
    return_SOLID_CALL( Ifaces::NetworkInterface*, backendObject(), false, isLinkUp() );
}

Solid::NetworkInterface::Capabilities Solid::NetworkInterface::capabilities() const
{
    return_SOLID_CALL( Ifaces::NetworkInterface*, backendObject(), Capabilities(), capabilities() );
}

Solid::Network * Solid::NetworkInterface::findNetwork( const QString & uni ) const
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

Solid::NetworkList Solid::NetworkInterface::networks() const
{
    NetworkList list;
    Ifaces::NetworkInterface *device = qobject_cast<Ifaces::NetworkInterface*>( backendObject() );

    if ( device==0 ) return list;

    QStringList uniList = device->networks();

    foreach( const QString &uni, uniList )
    {
        Network *network = findRegisteredNetwork( uni );
        if ( network!=0 )
        {
            list.append( network );
        }
    }

    return list;
}

void Solid::NetworkInterface::slotDestroyed( QObject *object )
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

void Solid::NetworkInterface::registerBackendObject( QObject *backendObject )
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

void Solid::NetworkInterface::unregisterBackendObject()
{
    setBackendObject( 0 );

    foreach( QObject *network, d->networkMap )
    {
        delete network;
    }

    d->networkMap.clear();
}

Solid::Network *Solid::NetworkInterface::findRegisteredNetwork( const QString &uni ) const
{
    Network *network = 0;

    if ( d->networkMap.contains( uni ) )
    {
        network = d->networkMap[uni];
    }
    else
    {
        Ifaces::NetworkInterface *device = qobject_cast<Ifaces::NetworkInterface*>( backendObject() );

        if ( device!=0 )
        {
            QObject *iface = device->createNetwork( uni );

            if ( qobject_cast<Ifaces::WirelessNetwork *>( iface )!=0 )
            {
                network = new WirelessNetwork( iface );
            }
            else if ( qobject_cast<Ifaces::Network *>( iface )!=0 )
            {
                network = new Network( iface );
            }

            if ( network != 0 )
            {
                d->networkMap[uni] = network;
            }
        }
    }

    return network;
}

#include "networkinterface.moc"
