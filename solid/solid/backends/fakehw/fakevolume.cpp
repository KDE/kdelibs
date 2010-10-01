/*
    Copyright 2006 Davide Bettio <davide.bettio@kdemail.net>

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

#include "fakevolume.h"

#include <QtDBus/QDBusConnection>

using namespace Solid::Backends::Fake;

FakeVolume::FakeVolume(FakeDevice *device)
    : FakeBlock(device)
{

}

FakeVolume::~FakeVolume()
{

}

bool FakeVolume::isIgnored() const
{
    return fakeDevice()->property("isIgnored").toBool();
}

Solid::StorageVolume::UsageType FakeVolume::usage() const
{
    QString usage = fakeDevice()->property("usage").toString();

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

QString FakeVolume::fsType() const
{
    return fakeDevice()->property("fsType").toString();
}

QString FakeVolume::label() const
{
    return fakeDevice()->property("label").toString();
}

QString FakeVolume::uuid() const
{
    return fakeDevice()->property("uuid").toString();
}

qulonglong FakeVolume::size() const
{
    return fakeDevice()->property("size").toULongLong();
}

QString Solid::Backends::Fake::FakeVolume::encryptedContainerUdi() const
{
    return QString();
}

#include "backends/fakehw/fakevolume.moc"
