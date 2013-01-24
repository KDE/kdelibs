/*
    Copyright 2013 Patrick von Reth <vonreth@kde.org>

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
#include "winstoragedrive.h"

#include <windows.h>
#include <winioctl.h>

using namespace Solid::Backends::Win;

WinStorageDrive::WinStorageDrive(WinDevice *device):
    WinBlock(device)
{
    updateCache();
}

WinStorageDrive::~WinStorageDrive()
{
}

Solid::StorageDrive::Bus WinStorageDrive::bus() const
{
    return m_bus;
}

Solid::StorageDrive::DriveType WinStorageDrive::driveType() const
{
    if(m_device->type() == Solid::DeviceInterface::OpticalDrive)
        return Solid::StorageDrive::CdromDrive;
    if(bus() == Solid::StorageDrive::Usb)
        return Solid::StorageDrive::MemoryStick;
    return Solid::StorageDrive::HardDisk;
}

bool WinStorageDrive::isRemovable() const
{
    return m_isRemovable;
}

bool WinStorageDrive::isHotpluggable() const
{
    return m_isHotplugges;
}

qulonglong WinStorageDrive::size() const
{
    return m_size;
}

void WinStorageDrive::updateCache()
{
    STORAGE_PROPERTY_QUERY storageProperty;
    storageProperty.PropertyId = StorageAdapterProperty;
    storageProperty.QueryType = PropertyStandardQuery;

    QString dev = QString("PhysicalDrive%1").arg(deviceMajor());

    STORAGE_ADAPTER_DESCRIPTOR  busInfo = WinDeviceManager::getDeviceInfo<STORAGE_ADAPTER_DESCRIPTOR>(dev,IOCTL_STORAGE_QUERY_PROPERTY,&storageProperty);

    switch(busInfo.BusType)
    {
    case BusTypeUsb:
        m_bus = Solid::StorageDrive::Usb;
        break;
    case BusType1394:
        m_bus = Solid::StorageDrive::Ieee1394;
        break;
    case BusTypeScsi:
        m_bus = Solid::StorageDrive::Scsi;
        break;
    case BusTypeAta:
    default:
        m_bus = Solid::StorageDrive::Ide;
    }

    GET_LENGTH_INFORMATION sizeInfo = WinDeviceManager::getDeviceInfo<GET_LENGTH_INFORMATION,void*>(dev,IOCTL_DISK_GET_LENGTH_INFO);
    m_size = sizeInfo.Length.QuadPart;//TODO: why do I only get a size of 0


    STORAGE_HOTPLUG_INFO plugInfo = WinDeviceManager::getDeviceInfo<STORAGE_HOTPLUG_INFO,void*>(dev,IOCTL_STORAGE_GET_HOTPLUG_INFO);
    m_isHotplugges = plugInfo.DeviceHotplug != 0;
    m_isRemovable = plugInfo.MediaRemovable != 0;

}

#include "winstoragedrive.moc"
