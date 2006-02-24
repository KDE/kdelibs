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

#include "cdrom.h"

#include <kdehw/ifaces/cdrom.h>

namespace KDEHW
{
    class Cdrom::Private
    {
    public:
        Private() : iface( 0 ) {}

        Ifaces::Cdrom *iface;
    };
}

KDEHW::Cdrom::Cdrom( Ifaces::Cdrom *iface, QObject *parent )
    : Storage( iface, parent ), d( new Private() )
{
    d->iface = iface;
}

KDEHW::Cdrom::~Cdrom()
{
    delete d->iface;
    delete d;
}

KDEHW::Cdrom::MediumTypes KDEHW::Cdrom::supportedMedia() const
{
    return d->iface->supportedMedia();
}

int KDEHW::Cdrom::readSpeed() const
{
    return d->iface->readSpeed();
}

int KDEHW::Cdrom::writeSpeed() const
{
    return d->iface->writeSpeed();
}

QList<int> KDEHW::Cdrom::writeSpeeds() const
{
    return d->iface->writeSpeeds();
}

#include "cdrom.moc"
