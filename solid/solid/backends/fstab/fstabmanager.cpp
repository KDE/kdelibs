/*
    This file is part of the KDE project

    Copyright 2010 Mario Bensi <mbensi@ipsquad.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "fstabmanager.h"
#include "fstabdevice.h"
#include "fstabhandling.h"
#include "rootdevice.h"
#include "fstabservice.h"


using namespace Solid::Backends::Fstab;


FstabManager::FstabManager(QObject *parent)
  : Solid::Ifaces::DeviceManager(parent)
{
    m_supportedInterfaces << Solid::DeviceInterface::StorageAccess;
}


QString FstabManager::udiPrefix() const
{
    return QString::fromLatin1(FSTAB_UDI_PREFIX);
}

QSet<Solid::DeviceInterface::Type> FstabManager::supportedInterfaces() const
{
    return m_supportedInterfaces;
}

QStringList FstabManager::allDevices()
{
    QStringList result;

    result << udiPrefix();
    foreach (QString device, FstabHandling::deviceList()) {
        result << udiPrefix() + "/" + device;
    }

    return result;
}


QStringList FstabManager::devicesFromQuery( const QString &/*parentUdi*/,
                                             Solid::DeviceInterface::Type type)
{
    if (type == Solid::DeviceInterface::StorageAccess) {
        return allDevices();
    }
    return QStringList();
}

QObject *FstabManager::createDevice(const QString &udi)
{
    QObject* result = 0;
    if (udi == udiPrefix()) {
        result = new RootDevice(FSTAB_UDI_PREFIX, "fstab devices", "list of nfs and smb device present in fstab");
    } else {
        result = new FstabDevice(udi);
    }
    return result;
}

FstabManager::~FstabManager()
{
}
