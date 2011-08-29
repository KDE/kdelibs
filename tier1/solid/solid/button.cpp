/*
    Copyright 2006 Davide Bettio <davbet@aliceposta.it>
    Copyright 2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
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
