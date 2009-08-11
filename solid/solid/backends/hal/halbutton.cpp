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

#include "halbutton.h"

using namespace Solid::Backends::Hal;

Button::Button(HalDevice *device)
    : DeviceInterface(device)
{
    connect(device, SIGNAL(conditionRaised(const QString &, const QString &)),
             this, SLOT(slotConditionRaised(const QString &, const QString &)));
}

Button::~Button()
{

}

Solid::Button::ButtonType Button::type() const
{
    QString buttontype = m_device->property("button.type").toString();

    if (buttontype=="lid")
    {
        return Solid::Button::LidButton;
    }
    else if (buttontype=="power")
    {
        return Solid::Button::PowerButton;
    }
    else if (buttontype=="sleep")
    {
        return Solid::Button::SleepButton;
    }
    else if (buttontype=="tablet_mode")
    {
        return Solid::Button::TabletButton;
    }
    else
    {
        return Solid::Button::UnknownButtonType;
    }
}

bool Button::hasState() const
{
    return m_device->property("button.has_state").toBool();
}

bool Button::stateValue() const
{
    return m_device->property("button.state.value").toBool();
}

void Button::slotConditionRaised(const QString &name, const QString &/*reason */)
{
    if (name == "ButtonPressed")
    {
        emit pressed(type(), m_device->udi());
    }
}

#include "backends/hal/halbutton.moc"
