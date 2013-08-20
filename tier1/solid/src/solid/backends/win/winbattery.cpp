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

#include <setupapi.h>
#include <batclass.h>
#include <devguid.h>

using namespace Solid::Backends::Win;

QMap<QString,WinBattery::Battery> WinBattery::m_udiToGDI = QMap<QString,WinBattery::Battery>();

WinBattery::WinBattery(WinDevice *device) :
    WinInterface(device),
    m_state(Solid::Battery::NoCharge)
{
    powerChanged();
    connect(WinDeviceManager::instance(),SIGNAL(powerChanged()),this,SLOT(powerChanged()));
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

int WinBattery::capacity() const
{
    return m_capacity;
}

bool WinBattery::isRechargeable() const
{
    return m_rechargabel;
}

bool WinBattery::isPowerSupply() const
{
    return m_isPowerSupply;
}

Solid::Battery::ChargeState WinBattery::chargeState() const
{
    return m_state;
}

QSet<QString> WinBattery::getUdis()
{
    QSet<QString> udis;
    HDEVINFO hdev =
            SetupDiGetClassDevs(&GUID_DEVCLASS_BATTERY,
                                0,
                                0,
                                DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (INVALID_HANDLE_VALUE != hdev)
    {
        // Limit search to 100 batteries max
        for (int idev = 0; idev < 100; idev++)
        {
            SP_DEVICE_INTERFACE_DATA did;
            ZeroMemory(&did,sizeof(did));
            did.cbSize = sizeof(did);

            if (SetupDiEnumDeviceInterfaces(hdev,
                                            0,
                                            &GUID_DEVCLASS_BATTERY,
                                            idev,
                                            &did))
            {
                DWORD cbRequired = 0;

                SetupDiGetDeviceInterfaceDetailW(hdev,
                                                 &did,
                                                 0,
                                                 0,
                                                 &cbRequired,
                                                 0);
                if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
                {
                    char *buffer = new char[cbRequired];
                    SP_DEVICE_INTERFACE_DETAIL_DATA *pdidd = (SP_DEVICE_INTERFACE_DETAIL_DATA*)buffer;
                    ZeroMemory(pdidd,cbRequired);
                    pdidd->cbSize = sizeof(*pdidd);
                    if (SetupDiGetDeviceInterfaceDetail(hdev,
                                                        &did,
                                                        pdidd,
                                                        cbRequired,
                                                        &cbRequired,
                                                        0))
                    {
                        QString path = QString::fromWCharArray(pdidd->DevicePath);
                        ulong tag = WinDeviceManager::getDeviceInfo<ulong>(path,IOCTL_BATTERY_QUERY_TAG);
                        QString udi = QLatin1String("/org/kde/solid/win/power.battery/battery#") + QString::number(tag);
                        udis << udi;
                        m_udiToGDI[udi] = Battery(path,tag);
                    }
                    delete [] buffer;

                }
            }
        }

        SetupDiDestroyDeviceInfoList(hdev);
    }
    return udis;
}

const WinBattery::Battery WinBattery::batteryInfoFromUdi(const QString &udi)
{
    return m_udiToGDI[udi];
}

void WinBattery::powerChanged()
{

    const int old_charge  = m_charge;
    const int old_capacity = m_capacity;
    const Solid::Battery::ChargeState old_state = m_state;
    const bool old_pluggedIn = m_pluggedIn;
    const bool old_isPowerSupply = m_isPowerSupply;
    const double old_energy = m_energy;
    const double old_energyRate = m_energyRate;

    BATTERY_WAIT_STATUS batteryStatusQuery;
    ZeroMemory(&batteryStatusQuery,sizeof(batteryStatusQuery));
    Battery b =  m_udiToGDI[m_device->udi()];
    batteryStatusQuery.BatteryTag = b.second;
    BATTERY_STATUS status = WinDeviceManager::getDeviceInfo<BATTERY_STATUS,BATTERY_WAIT_STATUS>(b.first,IOCTL_BATTERY_QUERY_STATUS,&batteryStatusQuery);

    BATTERY_QUERY_INFORMATION batteryInformationQuery;
    ZeroMemory(&batteryInformationQuery,sizeof(batteryInformationQuery));
    batteryInformationQuery.BatteryTag = b.second;
    batteryInformationQuery.InformationLevel = BatteryInformation;
    BATTERY_INFORMATION info = WinDeviceManager::getDeviceInfo<BATTERY_INFORMATION,BATTERY_QUERY_INFORMATION>(b.first,IOCTL_BATTERY_QUERY_INFORMATION,&batteryInformationQuery);


    m_pluggedIn = status.PowerState & BATTERY_POWER_ON_LINE;

    m_isPowerSupply = true;//TODO: is there a wy to implement this




     QString tech = QString::fromUtf8((const char*)info.Chemistry,4);

    if(tech == "LION" || tech == "LI-I")
    {
        m_technology = Solid::Battery::LithiumIon;
    }
    else if(tech == "PBAC")
    {
        m_technology = Solid::Battery::LeadAcid;
    }
    else if(tech == "NICD")
    {
        m_technology = Solid::Battery::NickelCadmium;
    }
    else if(tech == "NIMH")
    {
        m_technology = Solid::Battery::NickelMetalHydride;
    }
    else
    {
         qDebug() << tech << tr("Unknown", "battery technology");
        m_technology = Solid::Battery::UnknownTechnology;
    }



    m_energy = status.Capacity;
    m_energyRate = status.Rate;
    m_voltage = status.Voltage;

    if(info.FullChargedCapacity!=0)
    {
        m_charge = m_energy/info.FullChargedCapacity*100.0;
    }

    if(info.DesignedCapacity != 0)
    {
        m_capacity = (float)info.FullChargedCapacity/info.DesignedCapacity*100.0;
    }

    if(status.PowerState == 0)
    {
        m_state = Solid::Battery::NoCharge;
    }
    else if(status.PowerState & BATTERY_CHARGING)
    {
        m_state = Solid::Battery::Charging;
    }
    else if(status.PowerState & BATTERY_DISCHARGING)
    {
        m_state = Solid::Battery::Discharging;
    }
    //    else if(info.PowerState & 0x00000008)//critical
    //    {

    //    }


    if(info.Capabilities & BATTERY_SYSTEM_BATTERY)
    {
        m_type = Solid::Battery::PrimaryBattery;
    }
    else{
        m_type = Solid::Battery::UnknownBattery;
    }

    m_rechargabel = info.Technology == 1;

    if(m_charge != old_charge)
    {
        emit chargePercentChanged(m_charge,m_device->udi());
    }

    if(m_capacity != old_capacity)
    {
        emit capacityChanged(m_capacity,m_device->udi());
    }

    if(old_state != m_state)
    {
        emit chargeStateChanged(m_state,m_device->udi());
    }

    if(old_pluggedIn != m_pluggedIn)
    {
        emit plugStateChanged(m_pluggedIn,m_device->udi());
    }

    if(old_isPowerSupply != m_isPowerSupply)
    {
        emit powerSupplyStateChanged(m_isPowerSupply,m_device->udi());
    }

    if(old_energy != m_energy)
    {
        emit energyChanged(m_energy,m_device->udi());
    }

    if(old_energyRate != m_energyRate)
    {
        emit energyRateChanged(m_energyRate,m_device->udi());
    }
}

Solid::Battery::Technology WinBattery::technology() const
{
    return m_technology;
}

double WinBattery::energy() const
{
    return m_energy;
}

double WinBattery::energyRate() const
{
    return m_energyRate;
}

double WinBattery::voltage() const
{
    return m_voltage;
}
