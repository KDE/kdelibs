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

#include "button.h"

#include <kdehw/ifaces/button.h>

namespace KDEHW
{
    class Button::Private
    {
    public:
        Private() : iface( 0 ) {}

        Ifaces::Button *iface;
    };
}

KDEHW::Button::Button( Ifaces::Button *iface, QObject *parent )
    : Capability( parent ), d( new Private() )
{
    d->iface = iface;

    connect( d->iface->qobject(), SIGNAL( pressed( ButtonType ) ),
             this, SLOT( slotPressed( ButtonType ) ) );
}

KDEHW::Button::~Button()
{
    delete d;
}

KDEHW::Button::ButtonType KDEHW::Button::type() const
{
    return d->iface->type();
}

bool KDEHW::Button::hasState() const
{
    return d->iface->hasState();
}

bool KDEHW::Button::stateValue() const
{
    return d->iface->stateValue();
}

void KDEHW::Button::slotPressed( int type )
{
    emit pressed( type );
}

#include "button.moc"
