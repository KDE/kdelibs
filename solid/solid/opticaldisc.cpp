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

#include "opticaldisc.h"

#include <solid/ifaces/opticaldisc.h>

namespace Solid
{
    class OpticalDisc::Private
    {
    public:
        Private() : iface( 0 ) {}

        Ifaces::OpticalDisc *iface;
    };
}

Solid::OpticalDisc::OpticalDisc( Ifaces::OpticalDisc *iface, QObject *parent )
    : Volume( iface, parent ), d( new Private() )
{
    d->iface = iface;
}

Solid::OpticalDisc::~OpticalDisc()
{
    delete d;
}

Solid::OpticalDisc::ContentTypes Solid::OpticalDisc::availableContent() const
{
    return d->iface->availableContent();
}

Solid::OpticalDisc::DiscType Solid::OpticalDisc::discType() const
{
    return d->iface->discType();
}

bool Solid::OpticalDisc::isAppendable() const
{
    return d->iface->isAppendable();
}

bool Solid::OpticalDisc::isBlank() const
{
    return d->iface->isBlank();
}

bool Solid::OpticalDisc::isRewritable() const
{
    return d->iface->isRewritable();
}

qulonglong Solid::OpticalDisc::capacity() const
{
    return d->iface->capacity();
}

#include "opticaldisc.moc"
