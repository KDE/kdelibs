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
#include "device_p.h"
#include "ifaces/devicemanager.h"
#include "ifaces/device.h"


namespace Solid
{
    class DeviceManagerPrivate : public ManagerBasePrivate
    {
    public:
        DeviceManagerPrivate(DeviceManager *parent)
            : q(parent) { }

        DeviceManager * const q;

        QPair<Device*, Ifaces::Device*> findRegisteredDevice( const QString &udi ) const;
        void connectBackend( QObject *newBackend );

        void _k_deviceAdded(const QString &udi);
        void _k_deviceRemoved(const QString &udi);
        void _k_newDeviceInterface(const QString &udi, int type);
        void _k_destroyed(QObject *object);

        mutable QMap<QString, QPair<Device*, Ifaces::Device*> > devicesMap;
        Device invalidDevice;
    };
}

SOLID_SINGLETON_IMPLEMENTATION( Solid::DeviceManager, DeviceManager )

Solid::DeviceManager::DeviceManager()
    : QObject(), d(new DeviceManagerPrivate(this))
{
    d->loadBackend("Hardware Discovery",
                   "SolidDeviceManager",
                   "Solid::Ifaces::DeviceManager");

    if (d->backend!=0) {
        d->connectBackend(d->backend);
    }
}

Solid::DeviceManager::~DeviceManager()
{
    typedef QPair<Device*, Ifaces::Device*> DeviceIfacePair;

    foreach( const DeviceIfacePair &pair, d->devicesMap.values() )
    {
        delete pair.first;
        delete pair.second;
    }

    d->devicesMap.clear();
    delete d;
}

QString Solid::DeviceManager::errorText() const
{
    return d->errorText;
}

Solid::DeviceList Solid::DeviceManager::allDevices() const
{
    DeviceList list;
    Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager*>(d->backend);

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
    Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager*>(d->backend);

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
    Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager*>(d->backend);

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

Solid::DeviceList Solid::DeviceManager::findDevicesFromQuery(const DeviceInterface::Type &type,
                                                             const QString &parentUdi) const
{
    DeviceList list;

    Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager*>(d->backend);

    if ( backend == 0 ) return list;

    QStringList udis = backend->devicesFromQuery(parentUdi, type);

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
    DeviceList list;

    Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager*>(d->backend);

    if ( backend == 0 ) return list;

    QSet<QString> udis;
    if ( predicate.isValid() ) {
        QSet<DeviceInterface::Type> types = predicate.usedTypes();

        foreach (DeviceInterface::Type type, types) {
            udis+= QSet<QString>::fromList(backend->devicesFromQuery(parentUdi, type));
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

void Solid::DeviceManagerPrivate::_k_deviceAdded(const QString &udi)
{
    QPair<Device*, Ifaces::Device*> pair = devicesMap.take(udi);

    if ( pair.first!= 0 )
    {
        // Oops, I'm not sure it should happen...
        // But well in this case we'd better kill the old device we got, it's probably outdated

        delete pair.first;
        delete pair.second;
    }

    emit q->deviceAdded(udi);
}

void Solid::DeviceManagerPrivate::_k_deviceRemoved(const QString &udi)
{
    QPair<Device*, Ifaces::Device*> pair = devicesMap.take(udi);

    if ( pair.first!= 0 )
    {
        delete pair.first;
        delete pair.second;
    }

    emit q->deviceRemoved(udi);
}

void Solid::DeviceManagerPrivate::_k_newDeviceInterface(const QString &udi, int type)
{
    emit q->newDeviceInterface(udi, type);
}

void Solid::DeviceManagerPrivate::_k_destroyed(QObject *object)
{
    Ifaces::Device *device = qobject_cast<Ifaces::Device*>( object );

    if ( device!=0 )
    {
        QString udi = device->udi();
        QPair<Device*, Ifaces::Device*> pair = devicesMap.take(udi);
        delete pair.first;
    }
}

QPair<Solid::Device*, Solid::Ifaces::Device*> Solid::DeviceManagerPrivate::findRegisteredDevice( const QString &udi ) const
{
    if ( devicesMap.contains( udi ) )
    {
        return devicesMap[udi];
    }
    else
    {
        Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager*>( this->backend );
        Ifaces::Device *iface = 0;

        if ( backend!= 0 )
        {
            iface = qobject_cast<Ifaces::Device*>( backend->createDevice( udi ) );
        }

        if ( iface!=0 )
        {
            // TODO: clean up this...
            DevicePrivate *dev_p = new DevicePrivate(udi);
            dev_p->setBackendObject(iface);
            Device *device = new Device();
            device->d = dev_p;
            QPair<Device*, Ifaces::Device*> pair( device, iface );
            devicesMap[udi] = pair;
            QObject::connect( iface, SIGNAL( destroyed( QObject* ) ),
                              q, SLOT( _k_destroyed( QObject* ) ) );
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
    QObject::connect( newBackend, SIGNAL( deviceAdded( QString ) ),
                      q, SLOT( _k_deviceAdded( QString ) ) );
    QObject::connect( newBackend, SIGNAL( deviceRemoved( QString ) ),
                      q, SLOT( _k_deviceRemoved( QString ) ) );
    QObject::connect( newBackend, SIGNAL( newDeviceInterface( QString, int ) ),
                      q, SLOT( _k_newDeviceInterface( QString, int ) ) );
}

#include "devicemanager.moc"

