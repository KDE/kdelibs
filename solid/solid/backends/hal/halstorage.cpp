/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#include "halstorage.h"

using namespace Solid::Backends::Hal;

Storage::Storage(HalDevice *device)
    : Block(device)
{

}

Storage::~Storage()
{

}

Solid::StorageDrive::Bus Storage::bus() const
{
    QString bus = m_device->property("storage.bus").toString();

    if (bus=="ide")
    {
        return Solid::StorageDrive::Ide;
    }
    else if (bus=="usb")
    {
        return Solid::StorageDrive::Usb;
    }
    else if (bus=="ieee1394")
    {
        return Solid::StorageDrive::Ieee1394;
    }
    else if (bus=="scsi")
    {
        return Solid::StorageDrive::Scsi;
    }
    else if (bus=="sata")
    {
        return Solid::StorageDrive::Sata;
    }
    else
    {
        return Solid::StorageDrive::Platform;
    }
}

Solid::StorageDrive::DriveType Storage::driveType() const
{
    QString type = m_device->property("storage.drive_type").toString();

    if (type=="disk")
    {
        return Solid::StorageDrive::HardDisk;
    }
    else if (type=="cdrom")
    {
        return Solid::StorageDrive::CdromDrive;
    }
    else if (type=="floppy")
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
    }
    else if (type=="memory_stick")
    {
        return Solid::StorageDrive::MemoryStick;
    }
    else if (type=="smart_media")
    {
        return Solid::StorageDrive::SmartMedia;
    }
    else if (type=="sd_mmc")
    {
        return Solid::StorageDrive::SdMmc;
    }
    else
    {
        return Solid::StorageDrive::HardDisk;
    }
}

bool Storage::isRemovable() const
{
    return m_device->property("storage.removable").toBool();
}

bool Storage::isHotpluggable() const
{
    return m_device->property("storage.hotpluggable").toBool();
}

qulonglong Storage::size() const
{
  return m_device->property("storage.size").toULongLong();
}

#include "backends/hal/halstorage.moc"
