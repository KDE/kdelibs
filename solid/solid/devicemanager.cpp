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

#include "devicenotifier.h"
#include "devicemanager_p.h" //krazy:exclude=includes (devicenotifier.h is the header file for this class)

#include "device.h"
#include "device_p.h"
#include "predicate.h"

#include "ifaces/devicemanager.h"
#include "ifaces/device.h"

#include "soliddefs_p.h"

SOLID_GLOBAL_STATIC(Solid::DeviceManagerPrivate, globalDeviceManager)

Solid::DeviceManagerPrivate::DeviceManagerPrivate()
    : m_nullDevice(new DevicePrivate(QString()))
{
    loadBackend();

    if (managerBackend()!=0) {
        connect(managerBackend(), SIGNAL(deviceAdded(QString)),
                this, SLOT(_k_deviceAdded(QString)));
        connect(managerBackend(), SIGNAL(deviceRemoved(QString)),
                this, SLOT(_k_deviceRemoved(QString)));
    }
}

Solid::DeviceManagerPrivate::~DeviceManagerPrivate()
{
    foreach (DevicePrivate *dev, m_devicesMap) {
        delete dev;
    }

    m_devicesMap.clear();
}

QList<Solid::Device> Solid::Device::allDevices()
{
    QList<Device> list;
    Ifaces::DeviceManager *backend
        = qobject_cast<Ifaces::DeviceManager *>(globalDeviceManager->managerBackend());

    if (backend == 0) return list;

    QStringList udis = backend->allDevices();

    foreach (const QString &udi, udis)
    {
        list.append(Device(udi));
    }

    return list;
}

QList<Solid::Device> Solid::Device::listFromQuery(const QString &predicate,
                                                  const QString &parentUdi)
{
    Predicate p = Predicate::fromString(predicate);

    if (p.isValid())
    {
        return listFromQuery(p, parentUdi);
    }
    else
    {
        return QList<Device>();
    }
}

QList<Solid::Device> Solid::Device::listFromType(const DeviceInterface::Type &type,
                                                 const QString &parentUdi)
{
    QList<Device> list;

    Ifaces::DeviceManager *backend
        = qobject_cast<Ifaces::DeviceManager *>(globalDeviceManager->managerBackend());

    if (backend == 0) return list;

    QStringList udis = backend->devicesFromQuery(parentUdi, type);

    foreach (const QString &udi, udis)
    {
        list.append(Device(udi));
    }

    return list;
}

QList<Solid::Device> Solid::Device::listFromQuery(const Predicate &predicate,
                                                  const QString &parentUdi)
{
    QList<Device> list;

    Ifaces::DeviceManager *backend
        = qobject_cast<Ifaces::DeviceManager *>(globalDeviceManager->managerBackend());

    if (backend == 0) return list;

    QSet<QString> udis;
    if (predicate.isValid()) {
        QSet<DeviceInterface::Type> types = predicate.usedTypes();

        foreach (DeviceInterface::Type type, types) {
            udis+= QSet<QString>::fromList(backend->devicesFromQuery(parentUdi, type));
        }
    } else {
        udis+= QSet<QString>::fromList(backend->allDevices());
    }

    foreach (const QString &udi, udis)
    {
        Device dev(udi);

        bool matches = false;

        if(!predicate.isValid()) {
            matches = true;
        } else {
            matches = predicate.matches(dev);
        }

        if (matches)
        {
            list.append(dev);
        }
    }

    return list;
}

Solid::DeviceNotifier *Solid::DeviceNotifier::instance()
{
    return globalDeviceManager;
}

void Solid::DeviceManagerPrivate::_k_deviceAdded(const QString &udi)
{
    if (m_devicesMap.contains(udi)) {
        DevicePrivate *dev = m_devicesMap[udi];

        // Ok, this one was requested somewhere was invalid
        // and now becomes magically valid!

        Q_ASSERT(dev->backendObject()==0);
        dev->setBackendObject(createBackendObject(udi));
        Q_ASSERT(dev->backendObject()!=0);
    }

    emit deviceAdded(udi);
}

void Solid::DeviceManagerPrivate::_k_deviceRemoved(const QString &udi)
{
    if (m_devicesMap.contains(udi)) {
        DevicePrivate *dev = m_devicesMap[udi];

        // Ok, this one was requested somewhere was valid
        // and now becomes magically invalid!

        Q_ASSERT(dev->backendObject()!=0);
        delete dev->backendObject();
        Q_ASSERT(dev->backendObject()==0);
    }

    emit deviceRemoved(udi);
}

void Solid::DeviceManagerPrivate::_k_destroyed(QObject *object)
{
    QString udi = m_reverseMap.take(object);

    if (!udi.isEmpty()) {
        m_devicesMap.remove(udi);
    }
}

Solid::DevicePrivate *Solid::DeviceManagerPrivate::findRegisteredDevice(const QString &udi)
{
    if (udi.isEmpty()) {
        return m_nullDevice.data();
    } else if (m_devicesMap.contains(udi)) {
        return m_devicesMap[udi];
    } else {
        Ifaces::Device *iface = createBackendObject(udi);

        DevicePrivate *devData = new DevicePrivate(udi);
        devData->setBackendObject(iface);

        m_devicesMap[udi] = devData;
        m_reverseMap[devData] = udi;

        connect(devData, SIGNAL(destroyed(QObject *)),
                this, SLOT(_k_destroyed(QObject *)));

        return devData;
    }
}

Solid::Ifaces::Device *Solid::DeviceManagerPrivate::createBackendObject(const QString &udi)
{
    Ifaces::DeviceManager *backend = qobject_cast<Ifaces::DeviceManager *>(managerBackend());
    Ifaces::Device *iface = 0;

    if (backend!= 0) {
        QObject *object = backend->createDevice(udi);
        iface = qobject_cast<Ifaces::Device *>(object);

        if (iface==0) {
            delete object;
        }
    }

    return iface;
}

#include "devicenotifier.moc"
#include "devicemanager_p.moc"

