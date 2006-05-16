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

#include "audioiface.h"

#include <kdehw/ifaces/audioiface.h>

namespace KDEHW
{
    class AudioIface::Private
    {
    public:
        Private() : iface( 0 ) {}

        Ifaces::AudioIface *iface;
    };
}

KDEHW::AudioIface::AudioIface( Ifaces::AudioIface *iface, QObject *parent )
    : Capability( parent ), d( new Private() )
{
    d->iface = iface;
}

KDEHW::AudioIface::~AudioIface()
{
    delete d;
}


KDEHW::AudioIface::AudioDriver KDEHW::AudioIface::driver()
{
    return d->iface->driver();
}

QString KDEHW::AudioIface::driverHandler()
{
    return d->iface->driverHandler();
}

QString KDEHW::AudioIface::name()
{
    return d->iface->name();
}

KDEHW::AudioIface::AudioIfaceTypes KDEHW::AudioIface::type()
{
    return d->iface->type();
}

#include "audioiface.moc"
