/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

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

#include "storagevolume.h"
#include "storagevolume_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/storagevolume.h>

Solid::StorageVolume::StorageVolume(QObject *backendObject)
    : DeviceInterface(*new StorageVolumePrivate(), backendObject)
{
    connect(backendObject, SIGNAL(mountStateChanged(bool)),
             this, SIGNAL(mountStateChanged(bool)));
}

Solid::StorageVolume::StorageVolume(StorageVolumePrivate &dd, QObject *backendObject)
    : DeviceInterface(dd, backendObject)
{
    connect(backendObject, SIGNAL(mountStateChanged(bool)),
             this, SIGNAL(mountStateChanged(bool)));
}

Solid::StorageVolume::~StorageVolume()
{

}

bool Solid::StorageVolume::isIgnored() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), true, isIgnored());
}

bool Solid::StorageVolume::isMounted() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), false, isMounted());
}

QString Solid::StorageVolume::mountPoint() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), QString(), mountPoint());
}

Solid::StorageVolume::UsageType Solid::StorageVolume::usage() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), Unused, usage());
}

QString Solid::StorageVolume::fsType() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), QString(), fsType());
}

QString Solid::StorageVolume::label() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), QString(), label());
}

QString Solid::StorageVolume::uuid() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), QString(), uuid());
}

qulonglong Solid::StorageVolume::size() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), 0, size());
}

void Solid::StorageVolume::mount(QObject *receiver, const char *member)
{
    Q_D(StorageVolume);
    SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), mount(receiver, member));
}

void Solid::StorageVolume::unmount(QObject *receiver, const char *member)
{
    Q_D(StorageVolume);
    SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), unmount(receiver, member));
}

void Solid::StorageVolume::eject(QObject *receiver, const char *member)
{
    Q_D(StorageVolume);
    SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), eject(receiver, member));
}

#include "storagevolume.moc"
