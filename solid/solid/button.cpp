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

#include "soliddefs_p.h"
#include <solid/ifaces/button.h>


Solid::Button::Button( QObject *backendObject )
    : Capability( backendObject )
{
    connect( backendObject, SIGNAL( pressed( int ) ),
             this, SIGNAL( pressed( int ) ) );
}

Solid::Button::~Button()
{
}

Solid::Button::ButtonType Solid::Button::type() const
{
    return_SOLID_CALL( Ifaces::Button*, backendObject(), UnknownButtonType, type() );
}

bool Solid::Button::hasState() const
{
    return_SOLID_CALL( Ifaces::Button*, backendObject(), UnknownButtonType, type() );
}

bool Solid::Button::stateValue() const
{
    return_SOLID_CALL( Ifaces::Button*, backendObject(), UnknownButtonType, type() );
}


#include "button.moc"
