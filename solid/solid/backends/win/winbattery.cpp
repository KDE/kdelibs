/*
    Copyright 2013 Patrick von Reth <vonreth@kde.org>

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
#include "winbattery.h"

using namespace Solid::Backends::Win;


WinBattery::WinBattery(WinDevice *device) :
    WinInterface(device)
{
    SYSTEM_POWER_STATUS data;
    GetSystemPowerStatus(&data);
    m_pluggedIn = data.ACLineStatus == 1;
    m_charge = data.BatteryLifePercent;

    if(data.BatteryFlag & 8)
    {
        m_state = Solid::Battery::Charging;
    }
    else
    {
         m_state = Solid::Battery::Discharging;
    }


    m_type = Solid::Battery::UnknownBattery;

}

bool WinBattery::isPlugged() const
{
    return m_pluggedIn;
}

Solid::Battery::BatteryType WinBattery::type() const
{
    return m_type;
}

int WinBattery::chargePercent() const
{
    return m_charge;
}

bool WinBattery::isRechargeable() const
{
    return true;
}

Solid::Battery::ChargeState WinBattery::chargeState() const
{
    return m_state;
}


#include "winbattery.moc"
