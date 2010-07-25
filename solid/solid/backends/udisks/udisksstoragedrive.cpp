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

#include "udisksstoragedrive.h"

#include <QtCore/QDebug>

using namespace Solid::Backends::UDisks;

UDisksStorageDrive::UDisksStorageDrive(UDisksDevice* device)
    : Block(device)
{

}

Solid::Backends::UDisks::UDisksStorageDrive::~UDisksStorageDrive()
{

}

qulonglong UDisksStorageDrive::size() const
{
    return m_device->property("DeviceSize").toULongLong();
}

bool UDisksStorageDrive::isHotpluggable() const
{
    return m_device->property("DriveCanDetach").toULongLong();
}

bool UDisksStorageDrive::isRemovable() const
{
    return m_device->property("DeviceIsRemovable").toULongLong();
}

Solid::StorageDrive::DriveType UDisksStorageDrive::driveType() const
{

    if (m_device->property("DeviceIsPartition").toBool())
    {
        return Solid::StorageDrive::HardDisk;
    }
    else if (m_device->property("DeviceIsOpticalDisc").toBool())
    {
        return Solid::StorageDrive::CdromDrive;
    }
    
    //TODO: http://hal.freedesktop.org/docs/udisks/Device.html#Device:DriveMediaCompatibility
    
    // Seems there is no floppy support in UDisks?
/*    else if (type=="floppy")
    {
        return Solid::StorageDrive::Floppy;
    }
    else if (type=="tape")
    {
        return Solid::StorageDrive::Tape;
    }
    else if (type=="compact_flash")
    {
        return Solid::StorageDrive::CompactFlash;
    }*/
    else if (m_device->property("DeviceIsDrive").toBool())
    {
        return Solid::StorageDrive::MemoryStick;
    }
/*    else if (type=="smart_media")
    {
        return Solid::StorageDrive::SmartMedia;
    }
    else if (type=="sd_mmc")
    {
        return Solid::StorageDrive::SdMmc;
    }*/
    else
    {
        return Solid::StorageDrive::HardDisk;
    }

}

Solid::StorageDrive::Bus UDisksStorageDrive::bus() const
{
    QString bus = m_device->property("DriveConnectionInterface").toString();
    
    if (bus=="ide") //TODO: Test/Verify
    {
        return Solid::StorageDrive::Ide;
    }
    else if (bus=="usb")
    {
        return Solid::StorageDrive::Usb;
    }
    else if (bus=="ieee1394") // TODO: Test/Verify
    {
        return Solid::StorageDrive::Ieee1394;
    }
    else if (bus=="scsi")
    {
        return Solid::StorageDrive::Scsi;
    }
    else if (bus=="ata")
    {
        return Solid::StorageDrive::Sata;
    }
    else
    {
        return Solid::StorageDrive::Platform;
    }
}

