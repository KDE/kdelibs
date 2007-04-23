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

#include "storage.h"

Storage::Storage(HalDevice *device)
    : Block(device)
{

}

Storage::~Storage()
{

}

Solid::Storage::Bus Storage::bus() const
{
    QString bus = m_device->property("storage.bus").toString();

    if (bus=="ide")
    {
        return Solid::Storage::Ide;
    }
    else if (bus=="usb")
    {
        return Solid::Storage::Usb;
    }
    else if (bus=="ieee1394")
    {
        return Solid::Storage::Ieee1394;
    }
    else if (bus=="scsi")
    {
        return Solid::Storage::Scsi;
    }
    else if (bus=="sata")
    {
        return Solid::Storage::Sata;
    }
    else
    {
        return Solid::Storage::Platform;
    }
}

Solid::Storage::DriveType Storage::driveType() const
{
    QString type = m_device->property("storage.drive_type").toString();

    if (type=="disk")
    {
        return Solid::Storage::HardDisk;
    }
    else if (type=="cdrom")
    {
        return Solid::Storage::CdromDrive;
    }
    else if (type=="floppy")
    {
        return Solid::Storage::Floppy;
    }
    else if (type=="tape")
    {
        return Solid::Storage::Tape;
    }
    else if (type=="compact_flash")
    {
        return Solid::Storage::CompactFlash;
    }
    else if (type=="memory_stick")
    {
        return Solid::Storage::MemoryStick;
    }
    else if (type=="smart_media")
    {
        return Solid::Storage::SmartMedia;
    }
    else if (type=="sd_mmc")
    {
        return Solid::Storage::SdMmc;
    }
    else
    {
        return Solid::Storage::HardDisk;
    }
}

bool Storage::isRemovable() const
{
    return m_device->property("storage.removable").toBool();
}

bool Storage::isEjectRequired() const
{
    return m_device->property("storage.requires_eject").toBool();
}

bool Storage::isHotpluggable() const
{
    return m_device->property("storage.hotpluggable").toBool();
}

bool Storage::isMediaCheckEnabled() const
{
    return m_device->property("storage.media_check_enabled").toBool();
}

QString Storage::vendor() const
{
    return m_device->property("storage.vendor").toString();
}

QString Storage::product() const
{
    return m_device->property("storage.model").toString();
}

#include "storage.moc"
