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

#include "storage.h"

#include "soliddefs_p.h"
#include <solid/ifaces/storage.h>

Solid::Storage::Storage( QObject *backendObject )
    : Block( backendObject )
{
}

Solid::Storage::~Storage()
{
}

Solid::Storage::Bus Solid::Storage::bus() const
{
    return_SOLID_CALL( Ifaces::Storage*, backendObject(), Platform, bus() );
}

Solid::Storage::DriveType Solid::Storage::driveType() const
{
    return_SOLID_CALL( Ifaces::Storage*, backendObject(), HardDisk, driveType() );
}

bool Solid::Storage::isRemovable() const
{
    return_SOLID_CALL( Ifaces::Storage*, backendObject(), false, isRemovable() );
}

bool Solid::Storage::isEjectRequired() const
{
    return_SOLID_CALL( Ifaces::Storage*, backendObject(), false, isEjectRequired() );
}

bool Solid::Storage::isHotpluggable() const
{
    return_SOLID_CALL( Ifaces::Storage*, backendObject(), false, isHotpluggable() );
}

bool Solid::Storage::isMediaCheckEnabled() const
{
    return_SOLID_CALL( Ifaces::Storage*, backendObject(), false, isMediaCheckEnabled() );
}

QString Solid::Storage::vendor() const
{
    return_SOLID_CALL( Ifaces::Storage*, backendObject(), QString(), vendor() );
}

QString Solid::Storage::product() const
{
    return_SOLID_CALL( Ifaces::Storage*, backendObject(), QString(), product() );
}

#include "storage.moc"

