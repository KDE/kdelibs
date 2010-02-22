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

#include "storagedrive.h"
#include "storagedrive_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/storagedrive.h>

Solid::StorageDrive::StorageDrive(QObject *backendObject)
    : DeviceInterface(*new StorageDrivePrivate(), backendObject)
{
}

Solid::StorageDrive::StorageDrive(StorageDrivePrivate &dd, QObject *backendObject)
    : DeviceInterface(dd, backendObject)
{

}

Solid::StorageDrive::~StorageDrive()
{

}

Solid::StorageDrive::Bus Solid::StorageDrive::bus() const
{
    Q_D(const StorageDrive);
    return_SOLID_CALL(Ifaces::StorageDrive *, d->backendObject(), Platform, bus());
}

Solid::StorageDrive::DriveType Solid::StorageDrive::driveType() const
{
    Q_D(const StorageDrive);
    return_SOLID_CALL(Ifaces::StorageDrive *, d->backendObject(), HardDisk, driveType());
}

bool Solid::StorageDrive::isRemovable() const
{
    Q_D(const StorageDrive);
    return_SOLID_CALL(Ifaces::StorageDrive *, d->backendObject(), false, isRemovable());
}

bool Solid::StorageDrive::isHotpluggable() const
{
    Q_D(const StorageDrive);
    return_SOLID_CALL(Ifaces::StorageDrive *, d->backendObject(), false, isHotpluggable());
}

qulonglong Solid::StorageDrive::size() const
{
    Q_D(const StorageDrive);
    return_SOLID_CALL(Ifaces::StorageDrive *, d->backendObject(), false, size());
}

#include "storagedrive.moc"

