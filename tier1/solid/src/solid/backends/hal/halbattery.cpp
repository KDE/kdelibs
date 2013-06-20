/*
    Copyright 2006 Kevin Ottens <ervin@kde.org>
    Copyright 2012 Lukas Tinkl <ltinkl@redhat.com>

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

#include "halbattery.h"

using namespace Solid::Backends::Hal;

Battery::Battery(HalDevice *device)
    : DeviceInterface(device)
{
    connect(device, SIGNAL(propertyChanged(QMap<QString,int>)),
             this, SLOT(slotPropertyChanged(QMap<QString,int>)));
}

Battery::~Battery()
{

}


bool Battery::isPlugged() const
{
    return m_device->prop("battery.present").toBool();
}

Solid::Battery::BatteryType Battery::type() const
{
    QString name = m_device->prop("battery.type").toString();

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
    return m_device->prop("battery.charge_level.percentage").toInt();
}

int Battery::capacity() const
{
    const qreal lastFull = m_device->prop("battery.charge_level.last_full").toDouble();
    const qreal designFull = m_device->prop("battery.charge_level.design").toDouble();

    return lastFull / designFull * 100;
}

bool Battery::isRechargeable() const
{
    return m_device->prop("battery.is_rechargeable").toBool();
}

bool Battery::isPowerSupply() const
{
    // NOTE Hal doesn't support the is power supply property, so we're assuming that primary
    // and UPS batteries are power supply and all the others are not
    if (type() == Solid::Battery::PrimaryBattery || type() == Solid::Battery::UpsBattery) {
      return true;
    }

    return false;
}

Solid::Battery::ChargeState Battery::chargeState() const
{
    bool charging = m_device->prop("battery.rechargeable.is_charging").toBool();
    bool discharging = m_device->prop("battery.rechargeable.is_discharging").toBool();

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

Solid::Battery::Technology Battery::technology() const
{
    const QString tech = m_device->prop("battery.technology").toString();

    if (tech == "lithium-ion")
        return Solid::Battery::LithiumIon;
    else if (tech == "lead-acid")
        return Solid::Battery::LeadAcid;
    else if (tech == "lithium-polymer")
        return Solid::Battery::LithiumPolymer;
    else if (tech == "nickel-metal-hydride")
        return Solid::Battery::NickelMetalHydride;
    else if (tech == "lithium-iron-phosphate")
        return Solid::Battery::LithiumIronPhosphate;

    return Solid::Battery::UnknownTechnology;
}

double Battery::energy() const
{
    return m_device->prop("battery.charge_level.current").toInt() / 1000;
}

double Battery::energyRate() const
{
    return m_device->prop("battery.charge_level.rate").toInt() / 1000;
}

double Battery::voltage() const
{
    return m_device->prop("battery.voltage.current").toInt() / 1000;
}

void Battery::slotPropertyChanged(const QMap<QString,int> &changes)
{
    if (changes.contains("battery.charge_level.percentage"))
    {
        Q_EMIT chargePercentChanged(chargePercent(), m_device->udi());
    }

    if (changes.contains("battery.charge_level.last_full")
           || changes.contains("battery.charge_level.design"))
    {
        emit capacityChanged(capacity(), m_device->udi());
    }

    if (changes.contains("battery.rechargeable.is_charging")
           || changes.contains("battery.rechargeable.is_discharging"))
    {
        Q_EMIT chargeStateChanged(chargeState(), m_device->udi());
    }

    if ( changes.contains( "battery.present" ) )
    {
        Q_EMIT plugStateChanged(isPlugged(), m_device->udi());
    }

    if ( changes.contains( "battery.charge_level.current" ) ) {
        Q_EMIT energyChanged(energy(), m_device->udi());
    }

    if ( changes.contains( "battery.charge_level.rate" ) ) {
        Q_EMIT energyRateChanged(energyRate(), m_device->udi());
    }

}
