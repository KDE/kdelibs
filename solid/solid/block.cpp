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

#include "block.h"

#include "soliddefs_p.h"
#include <solid/ifaces/block.h>

namespace Solid
{
    class Block::Private
    {
    public:
    };
}

Solid::Block::Block( QObject *backendObject )
    : Capability(backendObject), d(new Private)
{
}

Solid::Block::~Block()
{
    delete d;
}

int Solid::Block::deviceMajor() const
{
    return_SOLID_CALL( Ifaces::Block*, backendObject(), 0, deviceMajor() );
}

int Solid::Block::deviceMinor() const
{
    return_SOLID_CALL( Ifaces::Block*, backendObject(), 0, deviceMinor() );
}

QString Solid::Block::device() const
{
    return_SOLID_CALL( Ifaces::Block*, backendObject(), QString(), device() );
}

#include "block.moc"
