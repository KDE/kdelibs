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

#include "udisksstoragevolume.h"

using namespace Solid::Backends::UDisks;

UDisksStorageVolume::UDisksStorageVolume(UDisksDevice *device)
    : Block(device)
{
}

UDisksStorageVolume::~UDisksStorageVolume()
{

}

  
QString UDisksStorageVolume::encryptedContainerUdi() const
{
//TODO:...
}

qulonglong UDisksStorageVolume::size() const
{
    return m_device->property("PartitionSize").toULongLong();
}

QString UDisksStorageVolume::uuid() const
{
    return m_device->property("IdUuid").toString();
}

QString UDisksStorageVolume::label() const
{
    return m_device->property("IdLabel").toString();
}

QString UDisksStorageVolume::fsType() const
{
    return m_device->property("IdType").toString();
}

Solid::StorageVolume::UsageType UDisksStorageVolume::usage() const
{
    QString usage = m_device->property("IdUsage").toString();
    
    if (usage == "filesystem")
    {
        return Solid::StorageVolume::FileSystem;
    }
    else if (usage == "partitiontable")
    {
        return Solid::StorageVolume::PartitionTable;
    }
    else if (usage == "raid")
    {
        return Solid::StorageVolume::Raid;
    }
    else if (usage == "crypto")
    {
        return Solid::StorageVolume::Encrypted;
    }
    else if (usage == "unused")
    {
        return Solid::StorageVolume::Unused;
    }
    else
    {
        return Solid::StorageVolume::Other;
    }
}

bool UDisksStorageVolume::isIgnored() const
{
    return false;
    //TODO: ...
/*    const QString mount_point = StorageAccess(m_device).filePath();
    const bool mounted = m_device->property("volume.is_mounted").toBool();
    if (!mounted) {
        return false;
    } else if (mount_point.startsWith(QLatin1String("/media/")) || mount_point.startsWith(QLatin1String("/mnt/"))) {
        return false;
    }
*/
    /* Now be a bit more aggressive on what we want to ignore,
     * the user generally need to check only what's removable or in /media
     * the volumes mounted to make the system (/, /boot, /var, etc.)
     * are useless to him.
     */
/*    Solid::Device drive(m_device->property("block.storage_device").toString());

    const bool removable = drive.as<Solid::GenericInterface>()->property("storage.removable").toBool();
    const bool hotpluggable = drive.as<Solid::GenericInterface>()->property("storage.hotpluggable").toBool();

    return !removable && !hotpluggable;*/
}
