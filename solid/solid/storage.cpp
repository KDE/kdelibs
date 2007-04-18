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

#include "storage.h"
#include "storage_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/storage.h>

Solid::Storage::Storage(QObject *backendObject)
    : DeviceInterface(*new StoragePrivate(this), backendObject)
{
}

Solid::Storage::Storage(StoragePrivate &dd, QObject *backendObject)
    : DeviceInterface(dd, backendObject)
{

}

Solid::Storage::~Storage()
{

}

Solid::Storage::Bus Solid::Storage::bus() const
{
    Q_D(const Storage);
    return_SOLID_CALL(Ifaces::Storage*, d->backendObject(), Platform, bus());
}

Solid::Storage::DriveType Solid::Storage::driveType() const
{
    Q_D(const Storage);
    return_SOLID_CALL(Ifaces::Storage*, d->backendObject(), HardDisk, driveType());
}

bool Solid::Storage::isRemovable() const
{
    Q_D(const Storage);
    return_SOLID_CALL(Ifaces::Storage*, d->backendObject(), false, isRemovable());
}

bool Solid::Storage::isEjectRequired() const
{
    Q_D(const Storage);
    return_SOLID_CALL(Ifaces::Storage*, d->backendObject(), false, isEjectRequired());
}

bool Solid::Storage::isHotpluggable() const
{
    Q_D(const Storage);
    return_SOLID_CALL(Ifaces::Storage*, d->backendObject(), false, isHotpluggable());
}

bool Solid::Storage::isMediaCheckEnabled() const
{
    Q_D(const Storage);
    return_SOLID_CALL(Ifaces::Storage*, d->backendObject(), false, isMediaCheckEnabled());
}

QString Solid::Storage::vendor() const
{
    Q_D(const Storage);
    return_SOLID_CALL(Ifaces::Storage*, d->backendObject(), QString(), vendor());
}

QString Solid::Storage::product() const
{
    Q_D(const Storage);
    return_SOLID_CALL(Ifaces::Storage*, d->backendObject(), QString(), product());
}

#include "storage.moc"

