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

#include "backends/hal/halvolume.h"

Volume::Volume(HalDevice *device)
    : Block(device)
{
}

Volume::~Volume()
{

}


bool Volume::isIgnored() const
{
    return m_device->property("volume.ignore").toBool();
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

#include "backends/hal/halvolume.moc"
