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

#include <kdehw/ifaces/opticaldisc.h>

namespace KDEHW
{
    class OpticalDisc::Private
    {
    public:
        Private() : iface( 0 ) {}

        Ifaces::OpticalDisc *iface;
    };
}

KDEHW::OpticalDisc::OpticalDisc( Ifaces::OpticalDisc *iface, QObject *parent )
    : Volume( iface, parent ), d( new Private() )
{
    d->iface = iface;
}

KDEHW::OpticalDisc::~OpticalDisc()
{
    delete d->iface;
    delete d;
}

KDEHW::OpticalDisc::ContentTypes KDEHW::OpticalDisc::availableContent() const
{
    return d->iface->availableContent();
}

KDEHW::OpticalDisc::DiscType KDEHW::OpticalDisc::discType() const
{
    return d->iface->discType();
}

bool KDEHW::OpticalDisc::isAppendable() const
{
    return d->iface->isAppendable();
}

bool KDEHW::OpticalDisc::isBlank() const
{
    return d->iface->isBlank();
}

bool KDEHW::OpticalDisc::isRewritable() const
{
    return d->iface->isRewritable();
}

qulonglong KDEHW::OpticalDisc::capacity() const
{
    return d->iface->capacity();
}

#include "opticaldisc.moc"
