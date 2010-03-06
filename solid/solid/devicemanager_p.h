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

#ifndef SOLID_DEVICEMANAGER_P_H
#define SOLID_DEVICEMANAGER_P_H

#include "managerbase_p.h"

#include "devicenotifier.h"

#include <QtCore/QMap>
#include <QtCore/QWeakPointer>
#include <QtCore/QSharedData>
#include <QtCore/QThreadStorage>

namespace Solid
{
    namespace Ifaces
    {
        class Device;
    }
    class DevicePrivate;


    class DeviceManagerPrivate : public DeviceNotifier, public ManagerBasePrivate
    {
        Q_OBJECT
    public:
        DeviceManagerPrivate();
        ~DeviceManagerPrivate();

        DevicePrivate *findRegisteredDevice(const QString &udi);

    private Q_SLOTS:
        void _k_deviceAdded(const QString &udi);
        void _k_deviceRemoved(const QString &udi);
        void _k_destroyed(QObject *object);

    private:
        Ifaces::Device *createBackendObject(const QString &udi);

        QExplicitlySharedDataPointer<DevicePrivate> m_nullDevice;
        QMap<QString, QWeakPointer<DevicePrivate> > m_devicesMap;
        QMap<QObject *, QString> m_reverseMap;
    };

    class DeviceManagerStorage
    {
    public:
        DeviceManagerStorage();

        QList<QObject*> managerBackends();
        DeviceNotifier *notifier();

    private:
        void ensureManagerCreated();

        QThreadStorage<DeviceManagerPrivate*> m_storage;
    };
}


#endif
