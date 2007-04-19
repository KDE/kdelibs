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
#include "devicemanager_p.h"

#include "device.h"
#include "device_p.h"
#include "predicate.h"
#include "ifaces/devicemanager.h"
#include <kglobal.h>

K_GLOBAL_STATIC(Solid::DeviceManagerPrivate, globalDeviceManager)

Solid::DeviceManagerPrivate::DeviceManagerPrivate()
{
    loadBackend("Hardware Discovery",
                "SolidDeviceManager",
                "Solid::Ifaces::DeviceManager");

    if (managerBackend()!=0) {
        connect(managerBackend(), SIGNAL(deviceAdded(QString)),
                this, SLOT(_k_deviceAdded(QString)));
        connect(managerBackend(), SIGNAL(deviceRemoved(QString)),
                this, SLOT(_k_deviceRemoved(QString)));
        connect(managerBackend(), SIGNAL(newDeviceInterface(QString, int)),
                this, SLOT(_k_newDeviceInterface(QString, int)));
    }
}

Solid::DeviceManagerPrivate::~DeviceManagerPrivate()
{
    typedef QPair<Device*, Ifaces::Device*> DeviceIfacePair;

    foreach (const DeviceIfacePair &pair, m_devicesMap.values()) {
        delete pair.first;
        delete pair.second;
    }

    m_devicesMap.clear();
}

QList<Solid::Device> Solid::DeviceManagerPrivate::allDevices()
{
    QList<Device> list;
    Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager*>(managerBackend());

    if ( backend == 0 ) return list;

    QStringList udis = backend->allDevices();

    foreach( const QString &udi, udis )
    {
        QPair<Device*, Ifaces::Device*> pair = findRegisteredDevice(udi);

        if ( pair.first!=0 )
        {
            list.append( *pair.first );
        }
    }

    return list;
}

bool Solid::DeviceManagerPrivate::deviceExists(const QString &udi)
{
    Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager*>(managerBackend());

    if ( backend == 0 ) return false;

    if (m_devicesMap.contains(udi)) {
        return true;
    } else {
        return backend->deviceExists( udi );
    }
}

Solid::Device Solid::DeviceManagerPrivate::findDevice(const QString &udi)
{
    Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager*>(managerBackend());

    if ( backend == 0 ) return m_invalidDevice;

    QPair<Device*, Ifaces::Device*> pair = findRegisteredDevice( udi );

    if ( pair.first!=0 )
    {
        return *pair.first;
    }
    else
    {
        return m_invalidDevice;
    }
}

QList<Solid::Device> Solid::DeviceManagerPrivate::findDevicesFromQuery(const QString &predicate,
                                                                       const QString &parentUdi)
{
    Predicate p = Predicate::fromString( predicate );

    if ( p.isValid() )
    {
        return findDevicesFromQuery( p, parentUdi );
    }
    else
    {
        return QList<Device>();
    }
}

QList<Solid::Device> Solid::DeviceManagerPrivate::findDevicesFromQuery(const DeviceInterface::Type &type,
                                                                       const QString &parentUdi)
{
    QList<Device> list;

    Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager*>(managerBackend());

    if ( backend == 0 ) return list;

    QStringList udis = backend->devicesFromQuery(parentUdi, type);

    foreach( const QString &udi, udis )
    {
        QPair<Device*, Ifaces::Device*> pair = findRegisteredDevice( udi );

        list.append( *pair.first );
    }

    return list;
}

QList<Solid::Device> Solid::DeviceManagerPrivate::findDevicesFromQuery(const Predicate &predicate,
                                                                       const QString &parentUdi)
{
    QList<Device> list;

    Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager*>(managerBackend());

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
        QPair<Device*, Ifaces::Device*> pair = findRegisteredDevice(udi);

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

Solid::DeviceManager::Notifier *Solid::DeviceManager::notifier()
{
    return globalDeviceManager;
}

void Solid::DeviceManagerPrivate::_k_deviceAdded(const QString &udi)
{
    QPair<Device*, Ifaces::Device*> pair = m_devicesMap.take(udi);

    if ( pair.first!= 0 )
    {
        // Oops, I'm not sure it should happen...
        // But well in this case we'd better kill the old device we got, it's probably outdated

        delete pair.first;
        delete pair.second;
    }

    emit deviceAdded(udi);
}

void Solid::DeviceManagerPrivate::_k_deviceRemoved(const QString &udi)
{
    QPair<Device*, Ifaces::Device*> pair = m_devicesMap.take(udi);

    if ( pair.first!= 0 )
    {
        delete pair.first;
        delete pair.second;
    }

    emit deviceRemoved(udi);
}

void Solid::DeviceManagerPrivate::_k_newDeviceInterface(const QString &udi, int type)
{
    emit newDeviceInterface(udi, type);
}

void Solid::DeviceManagerPrivate::_k_destroyed(QObject *object)
{
    Ifaces::Device *device = qobject_cast<Ifaces::Device*>( object );

    if ( device!=0 )
    {
        QString udi = device->udi();
        QPair<Device*, Ifaces::Device*> pair = m_devicesMap.take(udi);
        delete pair.first;
    }
}

QPair<Solid::Device*, Solid::Ifaces::Device*> Solid::DeviceManagerPrivate::findRegisteredDevice(const QString &udi)
{
    if ( m_devicesMap.contains( udi ) )
    {
        return m_devicesMap[udi];
    }
    else
    {
        Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager*>(managerBackend());
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
            m_devicesMap[udi] = pair;
            QObject::connect(iface, SIGNAL(destroyed(QObject*)),
                             this, SLOT(_k_destroyed(QObject*)));
            return pair;
        }
        else
        {
            return QPair<Device*, Ifaces::Device*>( 0, 0 );
        }
    }
}

QString Solid::DeviceManager::errorText()
{
    return globalDeviceManager->errorText();
}

QList<Solid::Device> Solid::DeviceManager::allDevices()
{
    return globalDeviceManager->allDevices();
}

bool Solid::DeviceManager::deviceExists(const QString &udi)
{
    return globalDeviceManager->deviceExists(udi);
}

Solid::Device Solid::DeviceManager::findDevice(const QString &udi)
{
    return globalDeviceManager->findDevice(udi);
}

QList<Solid::Device> Solid::DeviceManager::findDevicesFromQuery(const DeviceInterface::Type &type,
                                                                const QString &parentUdi)
{
    return globalDeviceManager->findDevicesFromQuery(type, parentUdi);
}

QList<Solid::Device> Solid::DeviceManager::findDevicesFromQuery(const Predicate &predicate,
                                                                const QString &parentUdi)
{
    return globalDeviceManager->findDevicesFromQuery(predicate, parentUdi);
}

QList<Solid::Device> Solid::DeviceManager::findDevicesFromQuery(const QString &predicate,
                                                                const QString &parentUdi)
{
    return globalDeviceManager->findDevicesFromQuery(predicate, parentUdi);
}

#include "devicemanager.moc"
#include "devicemanager_p.moc"

