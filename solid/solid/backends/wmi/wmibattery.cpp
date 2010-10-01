/*
    Copyright 2006 Kevin Ottens <ervin@kde.org>

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

#include "wmibattery.h"

using namespace Solid::Backends::Wmi;

Battery::Battery(WmiDevice *device)
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

int Battery::chargePercent() const
{
    return m_device->property("battery.charge_level.percentage").toInt();
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
        emit chargePercentChanged(chargePercent(), m_device->udi());
    }
    else if (changes.contains("battery.rechargeable.is_charging")
           || changes.contains("battery.rechargeable.is_discharging"))
    {
        emit chargeStateChanged(chargeState(), m_device->udi());
    }
    else if ( changes.contains( "battery.present" ) )
    {
        emit plugStateChanged(isPlugged(), m_device->udi());
    }

}

#include "backends/wmi/wmibattery.moc"
