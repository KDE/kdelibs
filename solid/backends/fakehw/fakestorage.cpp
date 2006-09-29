/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davbet@aliceposta.it>

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

#include "fakestorage.h"

FakeStorage::FakeStorage( FakeDevice *device )
    : FakeBlock( device )
{

}

FakeStorage::~FakeStorage()
{

}

Storage::Bus FakeStorage::bus() const
{
    QString bus = fakeDevice()->property("bus").toString();

    if ( bus=="ide" )
    {
        return Ide;
    }
    else if ( bus=="usb" )
    {
        return Usb;
    }
    else if ( bus=="ieee1394" )
    {
        return Ieee1394;
    }
    else if ( bus=="scsi" )
    {
        return Scsi;
    }
    else if ( bus=="sata" )
    {
        return Sata;
    }
    else
    {
        return Platform;
    }
}

Storage::DriveType FakeStorage::driveType() const
{
    QString type = fakeDevice()->property("major").toString();

    if ( type=="disk" )
    {
        return HardDisk;
    }
    else if ( type=="cdrom" )
    {
        return CdromDrive;
    }
    else if ( type=="floppy" )
    {
        return Floppy;
    }
    else if ( type=="tape" )
    {
        return Tape;
    }
    else if ( type=="compact_flash" )
    {
        return CompactFlash;
    }
    else if ( type=="memory_stick" )
    {
        return MemoryStick;
    }
    else if ( type=="smart_media" )
    {
        return SmartMedia;
    }
    else if ( type=="sd_mmc" )
    {
        return SdMmc;
    }
    else
    {
        return HardDisk;
    }
}

bool FakeStorage::isRemovable() const
{
    return fakeDevice()->property("isRemovable").toBool();
}

bool FakeStorage::isEjectRequired() const
{
    return fakeDevice()->property("isEjectRequired").toBool();
}

bool FakeStorage::isHotpluggable() const
{
    return fakeDevice()->property("isHotpluggable").toBool();
}

bool FakeStorage::isMediaCheckEnabled() const
{
    return fakeDevice()->property("isMediaCheckEnabled").toBool();
}

QString FakeStorage::vendor() const
{
    return fakeDevice()->property("vendor").toString();
}

QString FakeStorage::product() const
{
    return fakeDevice()->property("product").toString();
}

#include "fakestorage.moc"
