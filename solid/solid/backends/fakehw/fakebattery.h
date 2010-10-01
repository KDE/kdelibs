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

#ifndef SOLID_BACKENDS_FAKEHW_FAKEBATTERY_H
#define SOLID_BACKENDS_FAKEHW_FAKEBATTERY_H

#include <solid/ifaces/battery.h>
#include "fakedeviceinterface.h"

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeBattery : public FakeDeviceInterface, virtual public Solid::Ifaces::Battery
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Battery)

public:
    explicit FakeBattery(FakeDevice *device);
    virtual ~FakeBattery();

public Q_SLOTS:
    virtual bool isPlugged() const;
    virtual Solid::Battery::BatteryType type() const;

    virtual int chargePercent() const;

    virtual bool isRechargeable() const;
    virtual Solid::Battery::ChargeState chargeState() const;

    void setChargeState(Solid::Battery::ChargeState newState);
    void setChargeLevel(int newLevel);

Q_SIGNALS:
    void chargePercentChanged(int value, const QString &udi);
    void chargeStateChanged(int newState, const QString &udi);
    void plugStateChanged(bool newState, const QString &udi);
};
}
}
}

#endif // SOLID_BACKENDS_FAKEHW_FAKEBATTERY_H
