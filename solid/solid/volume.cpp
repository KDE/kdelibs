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

#include <solid/ifaces/volume.h>

namespace Solid
{
    class Volume::Private
    {
    public:
        Private() : iface( 0 ) {}

        Ifaces::Volume *iface;
    };
}

Solid::Volume::Volume( Ifaces::Volume *iface, QObject *parent )
    : Block( iface, parent ), d( new Private() )
{
    d->iface = iface;

    connect( d->iface->qobject(), SIGNAL( mountStateChanged( bool ) ),
             this, SIGNAL( mountStateChanged( bool ) ) );
}

Solid::Volume::~Volume()
{
    delete d;
}

bool Solid::Volume::isIgnored() const
{
    return d->iface->isIgnored();
}

bool Solid::Volume::isMounted() const
{
    return d->iface->isMounted();
}

QString Solid::Volume::mountPoint() const
{
    return d->iface->mountPoint();
}

Solid::Volume::UsageType Solid::Volume::usage() const
{
    return d->iface->usage();
}

QString Solid::Volume::fsType() const
{
    return d->iface->fsType();
}

QString Solid::Volume::label() const
{
    return d->iface->label();
}

QString Solid::Volume::uuid() const
{
    return d->iface->uuid();
}

qulonglong Solid::Volume::size() const
{
    return d->iface->size();
}

KJob *Solid::Volume::mount()
{
    return d->iface->mount();
}

KJob *Solid::Volume::unmount()
{
    return d->iface->unmount();
}

KJob *Solid::Volume::eject()
{
    return d->iface->eject();
}

#include "volume.moc"
