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
#include <solid/device.h>

Solid::StorageVolume::StorageVolume(QObject *backendObject)
    : DeviceInterface(*new StorageVolumePrivate(), backendObject)
{
}

Solid::StorageVolume::StorageVolume(StorageVolumePrivate &dd, QObject *backendObject)
    : DeviceInterface(dd, backendObject)
{
}

Solid::StorageVolume::~StorageVolume()
{

}

bool Solid::StorageVolume::isIgnored() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), true, isIgnored());
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
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), QString(), uuid().toLower());
}

qulonglong Solid::StorageVolume::size() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), 0, size());
}

Solid::Device Solid::StorageVolume::encryptedContainer() const
{
    Q_D(const StorageVolume);

    Ifaces::StorageVolume *iface
        = qobject_cast<Ifaces::StorageVolume*>(d->backendObject());

    if (iface!=0) {
        return Device(iface->encryptedContainerUdi());
    } else {
        return Device();
    }
}

#include "storagevolume.moc"
