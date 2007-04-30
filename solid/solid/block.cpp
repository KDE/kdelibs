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

#include "block.h"
#include "block_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/block.h>

Solid::Block::Block(QObject *backendObject)
    : DeviceInterface(*new BlockPrivate(), backendObject)
{
}

Solid::Block::~Block()
{

}

int Solid::Block::deviceMajor() const
{
    Q_D(const Block);
    return_SOLID_CALL(Ifaces::Block *, d->backendObject(), 0, deviceMajor());
}

int Solid::Block::deviceMinor() const
{
    Q_D(const Block);
    return_SOLID_CALL(Ifaces::Block *, d->backendObject(), 0, deviceMinor());
}

QString Solid::Block::device() const
{
    Q_D(const Block);
    return_SOLID_CALL(Ifaces::Block *, d->backendObject(), QString(), device());
}

#include "block.moc"
