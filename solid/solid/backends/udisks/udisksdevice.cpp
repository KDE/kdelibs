/*  Copyright 2010  Michael Zanetti <mzanetti@kde.org>

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
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "udisksdevice.h"
#include "udisksdeviceinterface.h"
#include "udisksstoragevolume.h"
#include "udisksopticaldisc.h"
#include "udisksopticaldrive.h"
#include "udisksstorageaccess.h"

#include <QtCore/QStringList>
#include <QtDBus/QDBusReply>
#include <QtCore/QDebug>

using namespace Solid::Backends::UDisks;

UDisksDevice::UDisksDevice(const QString &udi)
    : m_device("org.freedesktop.UDisks",
               udi,
               "org.freedesktop.UDisks.Device",
               QDBusConnection::systemBus()),
      m_udi(udi)
{
}

UDisksDevice::~UDisksDevice()
{

}

QObject* UDisksDevice::createDeviceInterface(const Solid::DeviceInterface::Type& type)
{
    if (!queryDeviceInterface(type)) {
        return 0;
    }

    DeviceInterface *iface = 0;
    switch (type)
    {
    case Solid::DeviceInterface::Block:
        iface = new Block(this);
        break;
    case Solid::DeviceInterface::StorageAccess:
        iface = new UDisksStorageAccess(this);
        break;
    case Solid::DeviceInterface::StorageDrive:
        iface = new UDisksStorageDrive(this);
        break;
    case Solid::DeviceInterface::OpticalDrive:
        iface = new UDisksOpticalDrive(this);
        break;
    case Solid::DeviceInterface::StorageVolume:
        iface = new UDisksStorageVolume(this);
        break;
    case Solid::DeviceInterface::OpticalDisc:
        iface = new OpticalDisc(this);
        break;
    default:
        break;
    }
    return iface;
}

bool UDisksDevice::queryDeviceInterface(const Solid::DeviceInterface::Type& type) const
{
    QStringList cap_list = DeviceInterface::toStringList(type);

    // TODO: improve this!
    switch (type) {
        case Solid::DeviceInterface::Block:
            return m_device.property("DeviceBlockSize").toULongLong() > 0;
        case Solid::DeviceInterface::StorageVolume:
            return m_device.property("DeviceIsPartition").toBool();
        case Solid::DeviceInterface::StorageAccess:
            return false;
        case Solid::DeviceInterface::StorageDrive:
            return true;
        case Solid::DeviceInterface::OpticalDrive:
            return m_device.property("DeviceIsOpticalDisc").toBool();
        case Solid::DeviceInterface::OpticalDisc:
            return m_device.property("DeviceIsOpticalDisc").toBool();
        default:
            return false;

    }
}

QString UDisksDevice::description() const
{
    // TODO
    const UDisksStorageDrive storageDrive(const_cast<UDisksDevice*>(this));
    Solid::StorageDrive::DriveType drive_type = storageDrive.driveType();

    if (drive_type == Solid::StorageDrive::CdromDrive) {

    }

    if (drive_type == Solid::StorageDrive::Floppy) {

    }

    bool drive_is_removable = storageDrive.isRemovable();

    if (drive_type == Solid::StorageDrive::HardDisk && !drive_is_removable) {

    }

    return "";
}

QStringList UDisksDevice::emblems() const
{
    // TODO
    return QStringList();
}

QString UDisksDevice::icon() const
{
    // TODO
    return "";
}

QString UDisksDevice::product() const
{
    return property("DriveModel").toString();
}

QString UDisksDevice::vendor() const
{
    return property("DriveVendor").toString();
}

QString UDisksDevice::udi() const
{
    return m_udi;
}

QString UDisksDevice::parentUdi() const
{
    return property("PartitionSlave").value<QDBusObjectPath>().path();
}

void UDisksDevice::checkCache(const QString &key) const
{
    if (m_cache.contains(key) && !m_invalidKeys.contains(key)) {
        return;
    }
    
    QVariant reply = m_device.property(key.toLocal8Bit());

    if (reply.isValid()) {
        m_cache[key] = reply;
    } else {
        m_cache[key] = QVariant();
    }

    m_invalidKeys.remove(key);
}

QVariant UDisksDevice::property(const QString &key) const
{
    checkCache(key);
    return m_cache.value(key);
}
