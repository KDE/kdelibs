/*  This file is part of the KDE project
    Copyright (C) 2005-2007 Kevin Ottens <ervin@kde.org>

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

#include "devicemanager.h"

#include <QPair>

#include "soliddefs_p.h"
#include "managerbase_p.h"
#include "device.h"
#include "ifaces/devicemanager.h"
#include "ifaces/device.h"


namespace Solid
{
    class DeviceManagerPrivate : public ManagerBasePrivate
    {
        Q_DECLARE_PUBLIC(DeviceManager)

    public:
        //DeviceManagerPrivate( DeviceManager *manager ) : q( manager ) {}

        QPair<Device*, Ifaces::Device*> findRegisteredDevice( const QString &udi ) const;
        void connectBackend( QObject *newBackend );

        //DeviceManager *q;
        mutable QMap<QString, QPair<Device*, Ifaces::Device*> > devicesMap;
        Device invalidDevice;

        QString errorText;
    };
}

SOLID_SINGLETON_IMPLEMENTATION( Solid::DeviceManager, DeviceManager )

Solid::DeviceManager::DeviceManager()
    : ManagerBase(*new DeviceManagerPrivate, "Hardware Discovery",
                  "SolidDeviceManager", "Solid::Ifaces::DeviceManager")
{
    Q_D(DeviceManager);

    if ( managerBackend() != 0 )
    {
        d->connectBackend( managerBackend() );
    }
}

Solid::DeviceManager::~DeviceManager()
{
    Q_D(DeviceManager);

    typedef QPair<Device*, Ifaces::Device*> DeviceIfacePair;

    foreach( const DeviceIfacePair &pair, d->devicesMap.values() )
    {
        delete pair.first;
        delete pair.second;
    }

    d->devicesMap.clear();
}

Solid::DeviceList Solid::DeviceManager::allDevices() const
{
    Q_D(const DeviceManager);

    DeviceList list;
    Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager*>( managerBackend() );

    if ( backend == 0 ) return list;

    QStringList udis = backend->allDevices();

    foreach( const QString &udi, udis )
    {
        QPair<Device*, Ifaces::Device*> pair = d->findRegisteredDevice( udi );

        if ( pair.first!=0 )
        {
            list.append( *pair.first );
        }
    }

    return list;
}

bool Solid::DeviceManager::deviceExists( const QString &udi ) const
{
    Q_D(const DeviceManager);

    Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager*>( managerBackend() );

    if ( backend == 0 ) return false;

    if ( d->devicesMap.contains( udi ) )
    {
        return true;
    }
    else
    {
        return backend->deviceExists( udi );
    }
}

const Solid::Device &Solid::DeviceManager::findDevice( const QString &udi ) const
{
    Q_D(const DeviceManager);

    Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager*>( managerBackend() );

    if ( backend == 0 ) return d->invalidDevice;

    QPair<Device*, Ifaces::Device*> pair = d->findRegisteredDevice( udi );

    if ( pair.first!=0 )
    {
        return *pair.first;
    }
    else
    {
        return d->invalidDevice;
    }
}

Solid::DeviceList Solid::DeviceManager::findDevicesFromQuery( const QString &predicate,
                                                              const QString &parentUdi ) const
{
    Predicate p = Predicate::fromString( predicate );

    if ( p.isValid() )
    {
        return findDevicesFromQuery( p, parentUdi );
    }
    else
    {
        return DeviceList();
    }
}

Solid::DeviceList Solid::DeviceManager::findDevicesFromQuery( const Capability::Type &capability,
                                                              const QString &parentUdi ) const
{
    Q_D(const DeviceManager);

    DeviceList list;

    Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager*>( managerBackend() );

    if ( backend == 0 ) return list;

    QStringList udis = backend->devicesFromQuery( parentUdi, capability );

    foreach( const QString &udi, udis )
    {
        QPair<Device*, Ifaces::Device*> pair = d->findRegisteredDevice( udi );

        list.append( *pair.first );
    }

    return list;
}

Solid::DeviceList Solid::DeviceManager::findDevicesFromQuery( const Predicate &predicate,
                                                              const QString &parentUdi ) const
{
    Q_D(const DeviceManager);

    DeviceList list;

    Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager*>( managerBackend() );

    if ( backend == 0 ) return list;

    QSet<QString> udis;
    if ( predicate.isValid() ) {
        QSet<Capability::Type> capabilities = predicate.usedCapabilities();

        foreach (Capability::Type capability, capabilities) {
            udis+= QSet<QString>::fromList(backend->devicesFromQuery(parentUdi, capability));
        }
    } else {
        udis+= QSet<QString>::fromList(backend->allDevices());
    }

    foreach( const QString &udi, udis )
    {
        QPair<Device*, Ifaces::Device*> pair = d->findRegisteredDevice( udi );

        bool matches = false;
        if ( pair.first )
        {
            if( !predicate.isValid() )
               matches = true;
            else
               matches = predicate.matches( *pair.first );
        }

        if ( matches )
        {
            list.append( *pair.first );
        }
    }

    return list;
}

void Solid::DeviceManager::setManagerBackend( QObject *backend )
{
    Q_D(DeviceManager);

    ManagerBase::setManagerBackend(backend);
    if (backend) {
        d->connectBackend(backend);
    }
}

void Solid::DeviceManager::slotDeviceAdded( const QString &udi )
{
    Q_D(DeviceManager);

    QPair<Device*, Ifaces::Device*> pair = d->devicesMap.take( udi );

    if ( pair.first!= 0 )
    {
        // Oops, I'm not sure it should happen...
        // But well in this case we'd better kill the old device we got, it's probably outdated

        delete pair.first;
        delete pair.second;
    }

    emit deviceAdded( udi );
}

void Solid::DeviceManager::slotDeviceRemoved( const QString &udi )
{
    Q_D(DeviceManager);

    QPair<Device*, Ifaces::Device*> pair = d->devicesMap.take( udi );

    if ( pair.first!= 0 )
    {
        delete pair.first;
        delete pair.second;
    }

    emit deviceRemoved( udi );
}

void Solid::DeviceManager::slotNewCapability( const QString &udi, int capability )
{
    emit newCapability( udi, capability );
}

void Solid::DeviceManager::slotDestroyed( QObject *object )
{
    Q_D(DeviceManager);

    Ifaces::Device *device = qobject_cast<Ifaces::Device*>( object );

    if ( device!=0 )
    {
        QString udi = device->udi();
        QPair<Device*, Ifaces::Device*> pair = d->devicesMap.take( udi );
        delete pair.first;
    }
}

QPair<Solid::Device*, Solid::Ifaces::Device*> Solid::DeviceManagerPrivate::findRegisteredDevice( const QString &udi ) const
{
    Q_Q(const DeviceManager);

    if ( devicesMap.contains( udi ) )
    {
        return devicesMap[udi];
    }
    else
    {
        Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager*>( q->managerBackend() );
        Ifaces::Device *iface = 0;

        if ( backend!= 0 )
        {
            iface = qobject_cast<Ifaces::Device*>( backend->createDevice( udi ) );
        }

        if ( iface!=0 )
        {
            Device *device = new Device( iface );
            QPair<Device*, Ifaces::Device*> pair( device, iface );
            devicesMap[udi] = pair;
            QObject::connect( iface, SIGNAL( destroyed( QObject* ) ),
                              q, SLOT( slotDestroyed( QObject* ) ) );
            return pair;
        }
        else
        {
            return QPair<Device*, Ifaces::Device*>( 0, 0 );
        }
    }
}

void Solid::DeviceManagerPrivate::connectBackend( QObject *newBackend )
{
    Q_Q(DeviceManager);

    QObject::connect( newBackend, SIGNAL( deviceAdded( QString ) ),
                      q, SLOT( slotDeviceAdded( QString ) ) );
    QObject::connect( newBackend, SIGNAL( deviceRemoved( QString ) ),
                      q, SLOT( slotDeviceRemoved( QString ) ) );
    QObject::connect( newBackend, SIGNAL( newCapability( QString, int ) ),
                      q, SLOT( slotNewCapability( QString, int ) ) );
}

#include "devicemanager.moc"

