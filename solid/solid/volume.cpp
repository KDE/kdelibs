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

#include "soliddefs_p.h"
#include <solid/ifaces/volume.h>

Solid::Volume::Volume( QObject *backendObject )
    : Block( backendObject )
{
    connect( backendObject, SIGNAL( mountStateChanged( bool ) ),
             this, SIGNAL( mountStateChanged( bool ) ) );
}

Solid::Volume::~Volume()
{
}

bool Solid::Volume::isIgnored() const
{
    return_SOLID_CALL( Ifaces::Volume*, backendObject(), true, isIgnored() );
}

bool Solid::Volume::isMounted() const
{
    return_SOLID_CALL( Ifaces::Volume*, backendObject(), false, isMounted() );
}

QString Solid::Volume::mountPoint() const
{
    return_SOLID_CALL( Ifaces::Volume*, backendObject(), QString(), mountPoint() );
}

Solid::Volume::UsageType Solid::Volume::usage() const
{
    return_SOLID_CALL( Ifaces::Volume*, backendObject(), Unused, usage() );
}

QString Solid::Volume::fsType() const
{
    return_SOLID_CALL( Ifaces::Volume*, backendObject(), QString(), fsType() );
}

QString Solid::Volume::label() const
{
    return_SOLID_CALL( Ifaces::Volume*, backendObject(), QString(), label() );
}

QString Solid::Volume::uuid() const
{
    return_SOLID_CALL( Ifaces::Volume*, backendObject(), QString(), uuid() );
}

qulonglong Solid::Volume::size() const
{
    return_SOLID_CALL( Ifaces::Volume*, backendObject(), 0, size() );
}

KJob *Solid::Volume::mount()
{
    return_SOLID_CALL( Ifaces::Volume*, backendObject(), 0, mount() );
}

KJob *Solid::Volume::unmount()
{
    return_SOLID_CALL( Ifaces::Volume*, backendObject(), 0, unmount() );
}

KJob *Solid::Volume::eject()
{
    return_SOLID_CALL( Ifaces::Volume*, backendObject(), 0, eject() );
}

#include "volume.moc"
