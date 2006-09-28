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
#include "ifaces/networkdevice.h"

#include "soliddefs_p.h"
#include "networkdevice.h"

#include "networkmanager.h"

namespace Solid
{
    class NetworkManager::Private
    {
    public:
        Private( NetworkManager *manager ) : q( manager ) {}

        QPair<NetworkDevice*, Ifaces::NetworkDevice*> findRegisteredNetworkDevice( const QString &uni );
        void registerBackend( QObject *newBackend );

        NetworkManager *q;
        QMap<QString, QPair<NetworkDevice*, Ifaces::NetworkDevice*> > networkDeviceMap;
        NetworkDevice invalidDevice;

        QString errorText;
    };
}

SOLID_SINGLETON_IMPLEMENTATION( Solid::NetworkManager )


Solid::NetworkManager::NetworkManager()
    : ManagerBase("Network Management", "SolidNetworkManager", "Solid::Ifaces::NetworkManager"),
      d( new Private( this ) )
{
    if ( managerBackend() != 0 )
    {
        d->registerBackend( managerBackend() );
    }
}

Solid::NetworkManager::NetworkManager( QObject *backend )
    : ManagerBase( backend ), d( new Private( this ) )
{
    if ( managerBackend() != 0 )
    {
        d->registerBackend( managerBackend() );
    }
}

Solid::NetworkManager::~NetworkManager()
{
    // Delete all the devices, they are now outdated
    typedef QPair<NetworkDevice*, Ifaces::NetworkDevice*> NetworkDeviceIfacePair;

    // Delete all the devices, they are now outdated
    foreach( NetworkDeviceIfacePair pair, d->networkDeviceMap.values() )
    {
        delete pair.first;
        delete pair.second;
    }

    d->networkDeviceMap.clear();

    delete d;
}

Solid::NetworkDeviceList Solid::NetworkManager::buildDeviceList( const QStringList & uniList ) const
{
    NetworkDeviceList list;
    Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager*>( managerBackend() );

    if ( backend == 0 ) return list;

    foreach( QString uni, uniList )
    {
        QPair<NetworkDevice*, Ifaces::NetworkDevice*> pair = d->findRegisteredNetworkDevice( uni );

        if ( pair.first!= 0 )
        {
            list.append( *pair.first );
        }
    }

    return list;
}

Solid::NetworkDeviceList Solid::NetworkManager::networkDevices() const
{
    Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager*>( managerBackend() );

    if ( backend!= 0 )
    {
        return buildDeviceList( backend->networkDevices() );
    }
    else
    {
        return NetworkDeviceList();
    }
}

Solid::NetworkDeviceList Solid::NetworkManager::activeNetworkDevices() const
{
    Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager*>( managerBackend() );

    if ( backend!= 0 )
    {
        return buildDeviceList( backend->activeNetworkDevices() );
    }
    else
    {
        return NetworkDeviceList();
    }
}

void Solid::NetworkManager::activate( const QString & net )
{
    SOLID_CALL( Ifaces::NetworkManager*, managerBackend(), activate( net ) );
}

void Solid::NetworkManager::deactivate( const QString & net )
{
    SOLID_CALL( Ifaces::NetworkManager*, managerBackend(), deactivate( net ) );
}

void Solid::NetworkManager::enableWireless( bool enabled )
{
    SOLID_CALL( Ifaces::NetworkManager*, managerBackend(), enableWireless( enabled ) );
}

void Solid::NetworkManager::enableNetworking( bool enabled )
{
    SOLID_CALL( Ifaces::NetworkManager*, managerBackend(), enableNetworking( enabled ) );
}

void Solid::NetworkManager::notifyHiddenNetwork( const QString & essid )
{
    SOLID_CALL( Ifaces::NetworkManager*, managerBackend(), notifyHiddenNetwork( essid ) );
}

const Solid::NetworkDevice &Solid::NetworkManager::findNetworkDevice( const QString &uni ) const
{
    Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager*>( managerBackend() );

    if ( backend == 0 ) return d->invalidDevice;

    QPair<NetworkDevice*, Ifaces::NetworkDevice*> pair = d->findRegisteredNetworkDevice( uni );

    if ( pair.first != 0 )
    {
        return *pair.first;
    }
    else
    {
        return d->invalidDevice;
    }
}

void Solid::NetworkManager::slotAdded( const QString &uni )
{
    QPair<NetworkDevice*, Ifaces::NetworkDevice*> pair = d->networkDeviceMap.take( uni );

    if ( pair.first!= 0 )
    {
        // Oops, I'm not sure it should happen...
        // But well in this case we'd better kill the old device we got, it's probably outdated

        delete pair.first;
        delete pair.second;
    }

    emit added( uni );
}

void Solid::NetworkManager::slotRemoved( const QString &uni )
{
    QPair<NetworkDevice*, Ifaces::NetworkDevice*> pair = d->networkDeviceMap.take( uni );

    if ( pair.first!= 0 )
    {
        delete pair.first;
        delete pair.second;
    }

    emit removed( uni );
}

void Solid::NetworkManager::slotDestroyed( QObject *object )
{
    Ifaces::NetworkDevice *device = qobject_cast<Ifaces::NetworkDevice*>( object );

    if ( device!=0 )
    {
        QString uni = device->uni();
        QPair<NetworkDevice*, Ifaces::NetworkDevice*> pair = d->networkDeviceMap.take( uni );
        delete pair.first;
    }
}

/***************************************************************************/

void Solid::NetworkManager::Private::registerBackend( QObject *newBackend )
{
    q->setManagerBackend( newBackend );

    QObject::connect( newBackend, SIGNAL( added( const QString & ) ),
                      q, SLOT( slotAdded( const QString & ) ) );
    QObject::connect( newBackend, SIGNAL( removed( const QString & ) ),
                      q, SLOT( slotRemoved( const QString & ) ) );
}

QPair<Solid::NetworkDevice*, Solid::Ifaces::NetworkDevice*> Solid::NetworkManager::Private::findRegisteredNetworkDevice( const QString &uni )
{
    if ( networkDeviceMap.contains( uni ) )
    {
        return networkDeviceMap[uni];
    }
    else
    {
        Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager*>( q->managerBackend() );
        Ifaces::NetworkDevice *iface = 0;

        if ( backend!=0 )
        {
            iface = qobject_cast<Ifaces::NetworkDevice*>( backend->createNetworkDevice( uni ) );
        }

        if ( iface!=0 )
        {
            NetworkDevice *device = new NetworkDevice( iface );
            QPair<NetworkDevice*, Ifaces::NetworkDevice*> pair( device, iface );
            connect( iface, SIGNAL( destroyed( QObject* ) ),
                     q, SLOT( slotDestroyed( QObject* ) ) );
            networkDeviceMap[uni] = pair;
            return pair;
        }
        else
        {
            return QPair<NetworkDevice*, Ifaces::NetworkDevice*>( 0, 0 );
        }
    }
}

#include "networkmanager.moc"
