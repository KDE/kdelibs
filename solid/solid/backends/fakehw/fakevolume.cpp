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

#include "backends/fakehw/fakevolume.h"

#include "backends/fakehw/fakejob.h"

#include <QDBusConnection>

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

bool FakeVolume::isMounted() const
{
    return fakeDevice()->property("isMounted").toBool();
}

QString FakeVolume::mountPoint() const
{
    return fakeDevice()->property("mountPoint").toString();
}

Solid::Volume::UsageType FakeVolume::usage() const
{
    QString usage = fakeDevice()->property("usage").toString();

    if (usage == "filesystem")
    {
        return Solid::Volume::FileSystem;
    }
    else if (usage == "partitiontable")
    {
        return Solid::Volume::PartitionTable;
    }
    else if (usage == "raid")
    {
        return Solid::Volume::Raid;
    }
    else if (usage == "unused")
    {
        return Solid::Volume::Unused;
    }
    else
    {
        return Solid::Volume::Other;
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

KJob *FakeVolume::mount()
{
    FakeJob *job = new FakeJob(this);
    job->setBroken(fakeDevice()->isBroken());
    return job;
}

KJob *FakeVolume::unmount()
{
    FakeJob *job = new FakeJob(this);
    job->setBroken(fakeDevice()->isBroken());
    return job;
}

KJob *FakeVolume::eject()
{
    FakeJob *job = new FakeJob(this);
    job->setBroken(fakeDevice()->isBroken());
    return job;
}

QString FakeVolume::createMountJob()
{
    static int count = 0;

    count++;

    KJob *job = mount();

    QString path = fakeDevice()->udi()+QString("/volume/mount_%1").arg(count);
    QDBusConnection::sessionBus().registerObject(path, job, QDBusConnection::ExportNonScriptableSlots);

    return path;
}

QString FakeVolume::createUnmountJob()
{
    static int count = 0;

    count++;

    KJob *job = unmount();

    QString path = fakeDevice()->udi()+QString("/volume/unmount_%1").arg(count);
    QDBusConnection::sessionBus().registerObject(path, job, QDBusConnection::ExportNonScriptableSlots);

    return path;
}

QString FakeVolume::createEjectJob()
{
    static int count = 0;

    count++;

    KJob *job = eject();

    QString path = fakeDevice()->udi()+QString("/volume/eject_%1").arg(count);
    QDBusConnection::sessionBus().registerObject(path, job, QDBusConnection::ExportNonScriptableSlots);

    return path;
}

#include "backends/fakehw/fakevolume.moc"
