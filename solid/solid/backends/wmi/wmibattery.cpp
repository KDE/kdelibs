/*
    Copyright 2012 Patrick von Reth <vonreth@kde.org>
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
//     connect(device, SIGNAL(propertyChanged(QMap<QString,int>)),
//              this, SLOT(slotPropertyChanged(QMap<QString,int>)));
}

Battery::~Battery()
{

}


bool Battery::isPlugged() const
{
    return true;
}

Solid::Battery::BatteryType Battery::type() const
{
    //as far as i know peripheral bateries are not listed
        return Solid::Battery::PrimaryBattery;
}

int Battery::chargePercent() const
{
    return m_device->property("EstimatedChargeRemaining").toInt();
}

bool Battery::isRechargeable() const
{
    return true;
}

Solid::Battery::ChargeState Battery::chargeState() const
{
    ushort status =  m_device->property("BatteryStatus").toInt();
    bool charging = status == 2 || status >= 6 && status <=8;//2 = The system has access to AC so no battery is being discharged. However, the battery is not necessarily charging , but windows mostlikly wont tell anything else then 2 or 1
    bool discharging = status == 1 || status >=3 && status <=5 || status == 11;

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

QString Battery::batteryTechnology() const
{
    const ushort tech = m_device->property("Chemistry").toUInt();
    switch (tech)
    {
    case 3:
        return QObject::tr("Lead Acid", "battery technology");
    case 4:
        return QObject::tr("Nickel Cadmium", "battery technology");
    case 5:
        return QObject::tr("Nickel Metal Hydride", "battery technology");
    case 6:
        return QObject::tr("Lithium Ion", "battery technology");
    case 8:
        return QObject::tr("Lithium Polymer", "battery technology");
    default:
        return QObject::tr("Unknown", "battery technology");
    }
}

bool Battery::isPowerSupply() const
{
    return chargeState() == Solid::Battery::Charging;
}

int Battery::capacity() const
{
    const unsigned int fullChargeCapacity = m_device->property("FullChargeCapacity").toUInt();
    const unsigned int designCapacity = m_device->property("DesignCapacity").toUInt();
    
    if ( designCapacity == 0 )
    {
        return 0;
    }
    return ( (int) (100.0 * fullChargeCapacity / designCapacity) );
}

void Battery::slotPropertyChanged(const QMap<QString,int> &changes)
{
//     if (changes.contains("battery.charge_level.percentage"))
//     {
//         emit chargePercentChanged(chargePercent(), m_device->udi());
//     }
//     else if (changes.contains("battery.rechargeable.is_charging")
//            || changes.contains("battery.rechargeable.is_discharging"))
//     {
//         emit chargeStateChanged(chargeState(), m_device->udi());
//     }
//     else if ( changes.contains( "battery.present" ) )
//     {
//         emit plugStateChanged(isPlugged(), m_device->udi());
//     }

}

#include "backends/wmi/wmibattery.moc"
