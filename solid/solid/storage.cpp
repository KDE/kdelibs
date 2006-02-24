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

#include <kdehw/ifaces/storage.h>

namespace KDEHW
{
    class Storage::Private
    {
    public:
        Private() : iface( 0 ) {}

        Ifaces::Storage *iface;
    };
}

KDEHW::Storage::Storage( Ifaces::Storage *iface, QObject *parent )
    : Block( iface, parent ), d( new Private() )
{
    d->iface = iface;
}

KDEHW::Storage::~Storage()
{
    delete d->iface;
    delete d;
}

KDEHW::Storage::Bus KDEHW::Storage::bus() const
{
    return d->iface->bus();
}

KDEHW::Storage::DriveType KDEHW::Storage::driveType() const
{
    return d->iface->driveType();
}

bool KDEHW::Storage::isRemovable() const
{
    return d->iface->isRemovable();
}

bool KDEHW::Storage::isEjectRequired() const
{
    return d->iface->isEjectRequired();
}

bool KDEHW::Storage::isHotpluggable() const
{
    return d->iface->isHotpluggable();
}

bool KDEHW::Storage::isMediaCheckEnabled() const
{
    return d->iface->isMediaCheckEnabled();
}

QString KDEHW::Storage::vendor() const
{
    return d->iface->vendor();
}

QString KDEHW::Storage::product() const
{
    return d->iface->product();
}

#include "storage.moc"

