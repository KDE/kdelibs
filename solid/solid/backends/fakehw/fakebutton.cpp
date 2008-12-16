/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davide.bettio@kdemail.net>

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

#include "fakebutton.h"

using namespace Solid::Backends::Fake;

FakeButton::FakeButton(FakeDevice *device)
    : FakeDeviceInterface(device)
{

}

FakeButton::~FakeButton()
{

}

Solid::Button::ButtonType FakeButton::type() const
{
    QString buttontype = fakeDevice()->property("type").toString();

    if (buttontype=="LidButton")
    {
        return Solid::Button::LidButton;
    }
    else if (buttontype=="PowerButton")
    {
        return Solid::Button::PowerButton;
    }
    else if (buttontype=="SleepButton")
    {
        return Solid::Button::SleepButton;
    }
    else
    {
        return Solid::Button::UnknownButtonType;
    }
}

bool FakeButton::hasState() const
{
    return fakeDevice()->property("hasState").toBool();
}

bool FakeButton::stateValue() const
{
    return fakeDevice()->property("stateValue").toBool();
}

void FakeButton::press()
{
    if (hasState()) fakeDevice()->setProperty("stateValue", !stateValue());
    emit pressed(type(), fakeDevice()->udi());
}

#include "backends/fakehw/fakebutton.moc"
