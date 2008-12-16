/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davide.bettio@kdemail.net>

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
