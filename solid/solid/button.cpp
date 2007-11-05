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

Solid::Button::Button(QObject *backendObject)
    : DeviceInterface(*new ButtonPrivate(), backendObject)
{
    connect(backendObject, SIGNAL(pressed(Solid::Button::ButtonType, const QString &)),
             this, SIGNAL(pressed(Solid::Button::ButtonType, const QString &)));
}

Solid::Button::~Button()
{

}

Solid::Button::ButtonType Solid::Button::type() const
{
    Q_D(const Button);
    return_SOLID_CALL(Ifaces::Button *, d->backendObject(), UnknownButtonType, type());
}

bool Solid::Button::hasState() const
{
    Q_D(const Button);
    return_SOLID_CALL(Ifaces::Button *, d->backendObject(), false, hasState());
}

bool Solid::Button::stateValue() const
{
    Q_D(const Button);
    return_SOLID_CALL(Ifaces::Button *, d->backendObject(), false, stateValue());
}

#include "button.moc"
