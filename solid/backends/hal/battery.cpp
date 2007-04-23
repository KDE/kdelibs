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

#include "battery.h"

Battery::Battery(HalDevice *device)
    : DeviceInterface(device)
{
    connect(device, SIGNAL(propertyChanged(const QMap<QString,int> &)),
             this, SLOT(slotPropertyChanged(const QMap<QString,int> &)));
}

Battery::~Battery()
{

}


bool Battery::isPlugged() const
{
    return m_device->property("battery.present").toBool();
}

Solid::Battery::BatteryType Battery::type() const
{
    QString name = m_device->property("battery.type").toString();

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

QString Battery::chargeLevelUnit() const
{
    return m_device->property("battery.charge_level.unit").toString();
}

int Battery::charge(Solid::Battery::LevelType type) const
{
    switch(type)
    {
    case Solid::Battery::MaxLevel:
        return m_device->property("battery.charge_level.design").toInt();
    case Solid::Battery::LastFullLevel:
        return m_device->property("battery.charge_level.last_full").toInt();
    case Solid::Battery::CurrentLevel:
        return m_device->property("battery.charge_level.current").toInt();
    case Solid::Battery::WarningLevel:
        return m_device->property("battery.charge_level.warning").toInt();
    case Solid::Battery::LowLevel:
        return m_device->property("battery.charge_level.low").toInt();
    }

    // Shouldn't happen...
    return -1;
}

int Battery::chargePercent() const
{
    return m_device->property("battery.charge_level.percentage").toInt();
}

QString Battery::voltageUnit() const
{
    return m_device->property("battery.voltage.unit").toString();
}

int Battery::voltage() const
{
    return m_device->property("battery.voltage.current").toInt();
}

bool Battery::isRechargeable() const
{
    return m_device->property("battery.is_rechargeable").toBool();
}

Solid::Battery::ChargeState Battery::chargeState() const
{
    bool charging = m_device->property("battery.rechargeable.is_charging").toBool();
    bool discharging = m_device->property("battery.rechargeable.is_discharging").toBool();

    if (!charging && !discharging)
    {
        return Solid::Battery::NoCharge;
    }
    else if (charging)
    {
        return Solid::Battery::Charging;
    }
    else
    {
        return Solid::Battery::Discharging;
    }
}

void Battery::slotPropertyChanged(const QMap<QString,int> &changes)
{
    if (changes.contains("battery.charge_level.percentage"))
    {
        emit chargePercentChanged(chargePercent());
    }
    else if (changes.contains("battery.rechargeable.is_charging")
           || changes.contains("battery.rechargeable.is_discharging"))
    {
        emit chargeStateChanged(chargeState());
    }
}

#include "battery.moc"
