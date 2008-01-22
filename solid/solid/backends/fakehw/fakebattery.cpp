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

#include "fakebattery.h"

using namespace Solid::Backends::Fake;

FakeBattery::FakeBattery(FakeDevice *device)
    : FakeDeviceInterface(device)
{
}

FakeBattery::~FakeBattery()
{
}


bool FakeBattery::isPlugged() const
{
    return fakeDevice()->property("isPlugged").toBool();
}

Solid::Battery::BatteryType FakeBattery::type() const
{
    QString name = fakeDevice()->property("batteryType").toString();

    if (name == "pda")
    {
        return Solid::Battery::PdaBattery;
    }
    else if (name == "ups")
    {
        return Solid::Battery::UpsBattery;
    }
    else if (name == "primary")
    {
        return Solid::Battery::PrimaryBattery;
    }
    else if (name == "mouse")
    {
        return Solid::Battery::MouseBattery;
    }
    else if (name == "keyboard")
    {
        return Solid::Battery::KeyboardBattery;
    }
    else if (name == "keyboard_mouse")
    {
        return Solid::Battery::KeyboardMouseBattery;
    }
    else if (name == "camera")
    {
        return Solid::Battery::CameraBattery;
    }
    else
    {
        return Solid::Battery::UnknownBattery;
    }
}

int FakeBattery::chargePercent() const
{
    int last_full = fakeDevice()->property("lastFullLevel").toInt();
    int current = fakeDevice()->property("currentLevel").toInt();

    int percent = (100 * current) / last_full;

    return percent;
}

bool FakeBattery::isRechargeable() const
{
    return fakeDevice()->property("isRechargeable").toBool();
}

Solid::Battery::ChargeState FakeBattery::chargeState() const
{
    QString state = fakeDevice()->property("chargeState").toString();

    if (state == "charging")
    {
        return Solid::Battery::Charging;
    }
    else if (state == "discharging")
    {
        return Solid::Battery::Discharging;
    }
    else
    {
        return Solid::Battery::NoCharge;
    }
}

void FakeBattery::setChargeState(Solid::Battery::ChargeState newState)
{
    QString name;

    switch(newState)
    {
    case Solid::Battery::Charging:
        name = "charging";
        break;
    case Solid::Battery::Discharging:
        name = "discharging";
        break;
    case Solid::Battery::NoCharge:
        name = "noCharge";
        break;
    }

    fakeDevice()->setProperty("chargeState", name);
    emit chargeStateChanged(newState, fakeDevice()->udi());
}

void FakeBattery::setChargeLevel(int newLevel)
{
    fakeDevice()->setProperty("currentLevel", newLevel);
    emit chargePercentChanged(chargePercent(), fakeDevice()->udi());
}

#include "backends/fakehw/fakebattery.moc"
