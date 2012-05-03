/*
    Copyright 2012 Patrick von Reth <vonreth@kde.org>
    Copyright 2006 Kevin Ottens <ervin@kde.org>

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

#include "wmistorage.h"
#include "wmiquery.h"

using namespace Solid::Backends::Wmi;

Storage::Storage(WmiDevice *device)
    : Block(device)
{

}

Storage::~Storage()
{

}

Solid::StorageDrive::Bus Storage::bus() const
{
    QString bus = m_device->property("InterfaceType").toString().toLower();

    if (bus=="ide")
    {
        return Solid::StorageDrive::Ide;
    }
    else if (bus=="usb")
    {
        return Solid::StorageDrive::Usb;
    }
    else if (bus=="1394")
    {
        return Solid::StorageDrive::Ieee1394;
    }
    else if (bus=="scsi")
    {
        return Solid::StorageDrive::Scsi;
    }
//    else if (bus=="sata")//not availible http://msdn.microsoft.com/en-us/library/windows/desktop/aa394132(v=vs.85).aspx
//    {
//        return Solid::StorageDrive::Sata;
//    }
    else
    {
        return Solid::StorageDrive::Platform;
    }
}

Solid::StorageDrive::DriveType Storage::driveType() const
{
    QString type = m_device->property("MediaType").toString();//was availibele in Win32_LogicalDiskDrive
     if (type=="Removable Media")
     {
         return Solid::StorageDrive::MemoryStick;
     }
     else{//if(type == "External hard disk media" || type == "Fixed hard disk media" || type == "Fixed hard disk"){
         return Solid::StorageDrive::HardDisk;
     }
//      else
//     else if (type=="tape")
//     {
//         return Solid::StorageDrive::Tape;
//     }
//     else if (type=="compact_flash")
//     {
//         return Solid::StorageDrive::CompactFlash;
//     }
//     else if (type=="smart_media")
//     {
//         return Solid::StorageDrive::SmartMedia;
//     }
//     else if (type=="sd_mmc")
//     {
//         return Solid::StorageDrive::SdMmc;
//     }
//     else
//     {
        
//    }
}

bool Storage::isRemovable() const
{
    //TODO:
    return false;
//    return m_device->property("storage.removable").toBool();
}

bool Storage::isHotpluggable() const
{
    //TODO:
    return false;
//    return m_device->property("storage.hotpluggable").toBool();
}

qulonglong Storage::size() const
{
    return m_device->property("Size").toULongLong();
}

#include "backends/wmi/wmistorage.moc"
