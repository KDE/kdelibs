/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davbet@aliceposta.it>

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

#include "display.h"

#include <kdehw/ifaces/display.h>

namespace KDEHW
{
    class Display::Private
    {
    public:
        Private() : iface( 0 ) {}

        Ifaces::Display *iface;
    };
}

KDEHW::Display::Display( Ifaces::Display *iface, QObject *parent )
    : Capability( parent ), d( new Private() )
{
    d->iface = iface;
}

KDEHW::Display::~Display()
{
    delete d;
}

KDEHW::Display::DisplayType KDEHW::Display::displayType() const
{
    return d->iface->displayType();
}

int KDEHW::Display::lcdBrightness() const
{
    return d->iface->lcdBrightness();
}

#include "display.moc"
