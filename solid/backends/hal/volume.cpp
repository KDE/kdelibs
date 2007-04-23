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

#include "volume.h"

#include "halcalljob.h"

Volume::Volume(HalDevice *device)
    : Block(device)
{
    connect(device, SIGNAL(propertyChanged(const QMap<QString,int> &)),
             this, SLOT(slotPropertyChanged(const QMap<QString,int> &)));
}

Volume::~Volume()
{

}


bool Volume::isIgnored() const
{
    return m_device->property("volume.ignore").toBool();
}

bool Volume::isMounted() const
{
    return m_device->property("volume.is_mounted").toBool();
}

QString Volume::mountPoint() const
{
    return m_device->property("volume.mount_point").toString();
}

Solid::Volume::UsageType Volume::usage() const
{
    QString usage = m_device->property("volume.fsusage").toString();

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

KJob * Volume::mount()
{
    QDBusConnection c = QDBusConnection::systemBus();
    QString udi = m_device->udi();
    QList<QVariant> params;

    params << "" << "" << QStringList();

    return new HalCallJob(c, udi, "org.freedesktop.Hal.Device.Volume",
                           "Mount", params);
}

KJob * Volume::unmount()
{
    QDBusConnection c = QDBusConnection::systemBus();
    QString udi = m_device->udi();
    QList<QVariant> params;

    params << QStringList();

    return new HalCallJob(c, udi, "org.freedesktop.Hal.Device.Volume",
                           "Unmount", params);
}

KJob * Volume::eject()
{
    QDBusConnection c = QDBusConnection::systemBus();
    QString udi = m_device->udi();
    QList<QVariant> params;

    params << QStringList();

    return new HalCallJob(c, udi, "org.freedesktop.Hal.Device.Volume",
                           "Eject", params);
}

void Volume::slotPropertyChanged(const QMap<QString,int> &changes)
{
    if (changes.contains("volume.is_mounted"))
    {
        emit mountStateChanged(isMounted());
    }
}

#include "volume.moc"
