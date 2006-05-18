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

#include <solid/ifaces/storage.h>

namespace Solid
{
    class Storage::Private
    {
    public:
        Private() : iface( 0 ) {}

        Ifaces::Storage *iface;
    };
}

Solid::Storage::Storage( Ifaces::Storage *iface, QObject *parent )
    : Block( iface, parent ), d( new Private() )
{
    d->iface = iface;
}

Solid::Storage::~Storage()
{
    delete d;
}

Solid::Storage::Bus Solid::Storage::bus() const
{
    return d->iface->bus();
}

Solid::Storage::DriveType Solid::Storage::driveType() const
{
    return d->iface->driveType();
}

bool Solid::Storage::isRemovable() const
{
    return d->iface->isRemovable();
}

bool Solid::Storage::isEjectRequired() const
{
    return d->iface->isEjectRequired();
}

bool Solid::Storage::isHotpluggable() const
{
    return d->iface->isHotpluggable();
}

bool Solid::Storage::isMediaCheckEnabled() const
{
    return d->iface->isMediaCheckEnabled();
}

QString Solid::Storage::vendor() const
{
    return d->iface->vendor();
}

QString Solid::Storage::product() const
{
    return d->iface->product();
}

#include "storage.moc"

