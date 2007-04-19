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

#include "devicemanager.h"
#include "device.h"
#include "ifaces/device.h"

#include <QPair>


namespace Solid
{
    class DeviceManagerPrivate : public DeviceManager::Notifier, public ManagerBasePrivate
    {
        Q_OBJECT
    public:
        DeviceManagerPrivate();
        ~DeviceManagerPrivate();

        QList<Device> allDevices();
        bool deviceExists(const QString &udi);
        Device findDevice(const QString &udi);
        QList<Device> findDevicesFromQuery(const DeviceInterface::Type &type,
                                           const QString &parentUdi = QString());
        QList<Device> findDevicesFromQuery(const Predicate &predicate,
                                           const QString &parentUdi = QString());
        QList<Device> findDevicesFromQuery(const QString &predicate,
                                           const QString &parentUdi = QString());

    private Q_SLOTS:
        void _k_deviceAdded(const QString &udi);
        void _k_deviceRemoved(const QString &udi);
        void _k_newDeviceInterface(const QString &udi, int type);
        void _k_destroyed(QObject *object);

    private:
        QPair<Device*, Ifaces::Device*> findRegisteredDevice(const QString &udi);

        QMap<QString, QPair<Device*, Ifaces::Device*> > m_devicesMap;
        Device m_invalidDevice;
    };
}


#endif
