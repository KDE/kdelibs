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
