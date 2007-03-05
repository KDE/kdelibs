/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davbet@aliceposta.it>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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
#include "button_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/button.h>

Solid::Button::Button( QObject *backendObject )
    : Capability(*new ButtonPrivate, backendObject)
{
    connect( backendObject, SIGNAL( pressed( int ) ),
             this, SIGNAL( pressed( int ) ) );
}

Solid::Button::Button(ButtonPrivate &dd, QObject *backendObject)
    : Capability(dd, backendObject)
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
    return_SOLID_CALL( Ifaces::Button*, backendObject(), false, hasState() );
}

bool Solid::Button::stateValue() const
{
    return_SOLID_CALL( Ifaces::Button*, backendObject(), false, stateValue() );
}

#include "button.moc"
