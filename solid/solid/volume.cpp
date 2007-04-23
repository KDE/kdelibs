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

#include "volume.h"
#include "volume_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/volume.h>

Solid::Volume::Volume(QObject *backendObject)
    : DeviceInterface(*new VolumePrivate(), backendObject)
{
    connect(backendObject, SIGNAL(mountStateChanged(bool)),
             this, SIGNAL(mountStateChanged(bool)));
}

Solid::Volume::Volume(VolumePrivate &dd, QObject *backendObject)
    : DeviceInterface(dd, backendObject)
{
    connect(backendObject, SIGNAL(mountStateChanged(bool)),
             this, SIGNAL(mountStateChanged(bool)));
}

Solid::Volume::~Volume()
{

}

bool Solid::Volume::isIgnored() const
{
    Q_D(const Volume);
    return_SOLID_CALL(Ifaces::Volume *, d->backendObject(), true, isIgnored());
}

bool Solid::Volume::isMounted() const
{
    Q_D(const Volume);
    return_SOLID_CALL(Ifaces::Volume *, d->backendObject(), false, isMounted());
}

QString Solid::Volume::mountPoint() const
{
    Q_D(const Volume);
    return_SOLID_CALL(Ifaces::Volume *, d->backendObject(), QString(), mountPoint());
}

Solid::Volume::UsageType Solid::Volume::usage() const
{
    Q_D(const Volume);
    return_SOLID_CALL(Ifaces::Volume *, d->backendObject(), Unused, usage());
}

QString Solid::Volume::fsType() const
{
    Q_D(const Volume);
    return_SOLID_CALL(Ifaces::Volume *, d->backendObject(), QString(), fsType());
}

QString Solid::Volume::label() const
{
    Q_D(const Volume);
    return_SOLID_CALL(Ifaces::Volume *, d->backendObject(), QString(), label());
}

QString Solid::Volume::uuid() const
{
    Q_D(const Volume);
    return_SOLID_CALL(Ifaces::Volume *, d->backendObject(), QString(), uuid());
}

qulonglong Solid::Volume::size() const
{
    Q_D(const Volume);
    return_SOLID_CALL(Ifaces::Volume *, d->backendObject(), 0, size());
}

KJob *Solid::Volume::mount()
{
    Q_D(Volume);
    return_SOLID_CALL(Ifaces::Volume *, d->backendObject(), 0, mount());
}

KJob *Solid::Volume::unmount()
{
    Q_D(Volume);
    return_SOLID_CALL(Ifaces::Volume *, d->backendObject(), 0, unmount());
}

KJob *Solid::Volume::eject()
{
    Q_D(Volume);
    return_SOLID_CALL(Ifaces::Volume *, d->backendObject(), 0, eject());
}

#include "volume.moc"
