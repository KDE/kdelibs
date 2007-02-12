/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2006 Kévin Ottens <ervin@kde.org>

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
#include <QPair>

#include "ifaces/networkmanager.h"
#include "ifaces/networkinterface.h"

#include "soliddefs_p.h"
#include "networkinterface.h"

#include "networkmanager.h"

namespace Solid
{
    class NetworkManager::Private
    {
    public:
        Private( NetworkManager *manager ) : q( manager ) {}

        QPair<NetworkInterface*, Ifaces::NetworkInterface*> findRegisteredNetworkInterface( const QString &uni );
        void connectBackend( QObject *newBackend );

        NetworkManager *q;
        QMap<QString, QPair<NetworkInterface*, Ifaces::NetworkInterface*> > networkInterfaceMap;
        NetworkInterface invalidDevice;

        QString errorText;
    };
}

SOLID_SINGLETON_IMPLEMENTATION( Solid::NetworkManager, NetworkManager )


Solid::NetworkManager::NetworkManager()
    : ManagerBase("Network Management", "SolidNetworkManager", "Solid::Ifaces::NetworkManager"),
      d( new Private( this ) )
{
    if ( managerBackend() != 0 )
    {
        d->connectBackend( managerBackend() );
    }
}

Solid::NetworkManager::~NetworkManager()
{
    // Delete all the devices, they are now outdated
    typedef QPair<NetworkInterface*, Ifaces::NetworkInterface*> NetworkInterfaceIfacePair;

    // Delete all the devices, they are now outdated
    foreach( const NetworkInterfaceIfacePair &pair, d->networkInterfaceMap.values() )
    {
        delete pair.first;
        delete pair.second;
    }

    d->networkInterfaceMap.clear();

    delete d;
}

Solid::NetworkInterfaceList Solid::NetworkManager::buildDeviceList( const QStringList & uniList ) const
{
    NetworkInterfaceList list;
    Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager*>( managerBackend() );

    if ( backend == 0 ) return list;

    foreach( const QString &uni, uniList )
    {
        QPair<NetworkInterface*, Ifaces::NetworkInterface*> pair = d->findRegisteredNetworkInterface( uni );

        if ( pair.first!= 0 )
        {
            list.append( *pair.first );
        }
    }

    return list;
}

Solid::NetworkInterfaceList Solid::NetworkManager::networkInterfaces() const
{
    Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager*>( managerBackend() );

    if ( backend!= 0 )
    {
        return buildDeviceList( backend->networkInterfaces() );
    }
    else
    {
        return NetworkInterfaceList();
    }
}

bool Solid::NetworkManager::isNetworkingEnabled() const
{
    return_SOLID_CALL( Ifaces::NetworkManager*, managerBackend(), false, isNetworkingEnabled() );
}

bool Solid::NetworkManager::isWirelessEnabled() const
{
    return_SOLID_CALL( Ifaces::NetworkManager*, managerBackend(), false, isWirelessEnabled() );
}

void Solid::NetworkManager::setNetworkingEnabled( bool enabled )
{
    SOLID_CALL( Ifaces::NetworkManager*, managerBackend(), setNetworkingEnabled( enabled ) );
}

void Solid::NetworkManager::setWirelessEnabled( bool enabled )
{
    SOLID_CALL( Ifaces::NetworkManager*, managerBackend(), setWirelessEnabled( enabled ) );
}

void Solid::NetworkManager::notifyHiddenNetwork( const QString &networkName )
{
    SOLID_CALL( Ifaces::NetworkManager*, managerBackend(), notifyHiddenNetwork( networkName ) );
}

const Solid::NetworkInterface &Solid::NetworkManager::findNetworkInterface( const QString &uni ) const
{
    Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager*>( managerBackend() );

    if ( backend == 0 ) return d->invalidDevice;

    QPair<NetworkInterface*, Ifaces::NetworkInterface*> pair = d->findRegisteredNetworkInterface( uni );

    if ( pair.first != 0 )
    {
        return *pair.first;
    }
    else
    {
        return d->invalidDevice;
    }
}

void Solid::NetworkManager::setManagerBackend( QObject *backend )
{
    ManagerBase::setManagerBackend(backend);
    if (backend) {
        d->connectBackend(backend);
    }
}

void Solid::NetworkManager::slotNetworkInterfaceAdded( const QString &uni )
{
    QPair<NetworkInterface*, Ifaces::NetworkInterface*> pair = d->networkInterfaceMap.take( uni );

    if ( pair.first!= 0 )
    {
        // Oops, I'm not sure it should happen...
        // But well in this case we'd better kill the old device we got, it's probably outdated

        delete pair.first;
        delete pair.second;
    }

    emit networkInterfaceAdded( uni );
}

void Solid::NetworkManager::slotNetworkInterfaceRemoved( const QString &uni )
{
    QPair<NetworkInterface*, Ifaces::NetworkInterface*> pair = d->networkInterfaceMap.take( uni );

    if ( pair.first!= 0 )
    {
        delete pair.first;
        delete pair.second;
    }

    emit networkInterfaceRemoved( uni );
}

void Solid::NetworkManager::slotDestroyed( QObject *object )
{
    Ifaces::NetworkInterface *device = qobject_cast<Ifaces::NetworkInterface*>( object );

    if ( device!=0 )
    {
        QString uni = device->uni();
        QPair<NetworkInterface*, Ifaces::NetworkInterface*> pair = d->networkInterfaceMap.take( uni );
        delete pair.first;
    }
}

/***************************************************************************/

void Solid::NetworkManager::Private::connectBackend( QObject *newBackend )
{
    QObject::connect( newBackend, SIGNAL( networkInterfaceAdded( const QString & ) ),
                      q, SLOT( slotNetworkInterfaceAdded( const QString & ) ) );
    QObject::connect( newBackend, SIGNAL( networkInterfaceRemoved( const QString & ) ),
                      q, SLOT( slotNetworkInterfaceRemoved( const QString & ) ) );
}

QPair<Solid::NetworkInterface*, Solid::Ifaces::NetworkInterface*> Solid::NetworkManager::Private::findRegisteredNetworkInterface( const QString &uni )
{
    if ( networkInterfaceMap.contains( uni ) )
    {
        return networkInterfaceMap[uni];
    }
    else
    {
        Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager*>( q->managerBackend() );
        Ifaces::NetworkInterface *iface = 0;

        if ( backend!=0 )
        {
            iface = qobject_cast<Ifaces::NetworkInterface*>( backend->createNetworkInterface( uni ) );
        }

        if ( iface!=0 )
        {
            NetworkInterface *device = new NetworkInterface( iface );
            QPair<NetworkInterface*, Ifaces::NetworkInterface*> pair( device, iface );
            connect( iface, SIGNAL( destroyed( QObject* ) ),
                     q, SLOT( slotDestroyed( QObject* ) ) );
            networkInterfaceMap[uni] = pair;
            return pair;
        }
        else
        {
            return QPair<NetworkInterface*, Ifaces::NetworkInterface*>( 0, 0 );
        }
    }
}

#include "networkmanager.moc"
