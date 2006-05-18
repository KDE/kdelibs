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

#include "processor.h"

#include <solid/ifaces/processor.h>

namespace Solid
{
    class Processor::Private
    {
    public:
        Private() : iface( 0 ) {}

        Ifaces::Processor *iface;
    };
}

Solid::Processor::Processor( Ifaces::Processor *iface, QObject *parent )
    : Capability( parent ), d( new Private() )
{
    d->iface = iface;
}

Solid::Processor::~Processor()
{
    delete d;
}

int Solid::Processor::number() const
{
    return d->iface->number();
}

qulonglong Solid::Processor::maxSpeed() const
{
    return d->iface->maxSpeed();
}

bool Solid::Processor::canThrottle() const
{
    return d->iface->canThrottle();
}

#include "processor.moc"
