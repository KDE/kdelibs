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

#include <kdehw/ifaces/block.h>

namespace KDEHW
{
    class Block::Private
    {
    public:
        Private() : iface( 0 ) {}

        Ifaces::Block *iface;
    };
}


KDEHW::Block::Block( Ifaces::Block *iface, QObject *parent )
    : Capability( parent ), d( new Private() )
{
    d->iface = iface;
}

KDEHW::Block::~Block()
{
    delete d->iface;
    delete d;
}

int KDEHW::Block::major() const
{
    return d->iface->major();
}

int KDEHW::Block::minor() const
{
    return d->iface->minor();
}

QString KDEHW::Block::device() const
{
    return d->iface->device();
}

#include "block.moc"
