/*
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

#include "wmivolume.h"

using namespace Solid::Backends::Wmi;

Volume::Volume(WmiDevice *device)
    : Block(device)
{
}

Volume::~Volume()
{

}


bool Volume::isIgnored() const
{
    if (m_device->property("volume.ignore").toBool()) {
        return true;
    }

    /* Now be a bit more aggressive on what we want to ignore,
     * the user generally need to check only what's removable or in /media
     * the volumes mounted to make the system (/, /boot, /var, etc.)
     * are useless to him.
     */
    WmiDevice drive(m_device->property("block.storage_device").toString());
    QString mount_point = m_device->property("volume.mount_point").toString();
    bool mounted = m_device->property("volume.is_mounted").toBool();
    bool removable = drive.property("storage.removable").toBool();
    bool hotpluggable = drive.property("storage.hotpluggable").toBool();


    return !removable && !hotpluggable
        && mounted && !mount_point.startsWith(QLatin1String("/media/"))
        && !mount_point.startsWith(QLatin1String("/mnt/"));
}

Solid::StorageVolume::UsageType Volume::usage() const
{
    QString usage = m_device->property("volume.fsusage").toString();

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

QString Volume::fsType() const
{
    return m_device->property("volume.fstype").toString();
}

QString Volume::label() const
{
    return m_device->property("volume.label").toString();
}

QString Volume::uuid() const
{
    return m_device->property("volume.uuid").toString();
}

qulonglong Volume::size() const
{
    return m_device->property("volume.size").toULongLong();
}

QString Solid::Backends::Wmi::Volume::encryptedContainerUdi() const
{
    return m_device->property("volume.crypto_luks.clear.backing_volume").toString();
}

#include "backends/wmi/wmivolume.moc"
