/*
    Copyright 2009 Harald Fernengel <harry@kdevelop.org>

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

#include "iokitbattery.h"
#include "iokitdevice.h"

#include <QtCore/qdebug.h>

// TODO - emit the signals

using namespace Solid::Backends::IOKit;

Battery::Battery(IOKitDevice *device)
    : DeviceInterface(device)
{
}

Battery::~Battery()
{

}

bool Battery::isPlugged() const
{
    return m_device->property(QLatin1String("ExternalConnected")).toBool();
}

Solid::Battery::BatteryType Battery::type() const
{
    // TODO - how to figure that one out?
    return Solid::Battery::UnknownBattery;
}

int Battery::chargePercent() const
{
    if (m_device->property(QLatin1String("FullyCharged")).toBool())
        return 100;

    int maxCapacity = m_device->property(QLatin1String("MaxCapacity")).toInt();
    if (maxCapacity == 0)
        return 0; // prevent divide by 0
    return m_device->property(QLatin1String("CurrentCapacity")).toInt() / maxCapacity;
}

bool Battery::isRechargeable() const
{
    return m_device->property(QLatin1String("DesignCycleCount")).toInt() > 1;
}

Solid::Battery::ChargeState Battery::chargeState() const
{
    if (m_device->property(QLatin1String("IsCharging")).toBool())
        return Solid::Battery::Charging;
    if (m_device->property(QLatin1String("FullyCharged")).toBool())
        return Solid::Battery::NoCharge;
    return Solid::Battery::Discharging;
}

#include "backends/iokit/iokitbattery.moc"
