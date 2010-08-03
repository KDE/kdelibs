/*  This file is part of the KDE project
    Copyright (C) 2009 Pino Toscano <pino@kde.org>
                  2010 Lukas Tinkl <ltinkl@redhat.com>

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

#ifndef SOLID_BACKENDS_UPOWER_BATTERY_H
#define SOLID_BACKENDS_UPOWER_BATTERY_H

#include <solid/ifaces/battery.h>
#include "upowerdeviceinterface.h"

namespace Solid
{
namespace Backends
{
namespace UPower
{
class Battery : public DeviceInterface, virtual public Solid::Ifaces::Battery
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Battery)

public:
    Battery(UPowerDevice *device);
    virtual ~Battery();

    virtual bool isPlugged() const;
    virtual Solid::Battery::BatteryType type() const;

    virtual int chargePercent() const;

    virtual bool isRechargeable() const;
    virtual Solid::Battery::ChargeState chargeState() const;

    // TODO report stuff like capacity, technology, time-to-full, time-to-empty, energy rates, vendor, etc.

Q_SIGNALS:
    void chargePercentChanged(int value, const QString &udi);
    void chargeStateChanged(int newState, const QString &udi);
    void plugStateChanged(bool newState, const QString &udi);

private Q_SLOTS:
    void slotChanged();

private:
    void updateCache();

    bool m_isPlugged;
    int m_chargePercent;
    Solid::Battery::ChargeState m_chargeState;
};
}
}
}

#endif // SOLID_BACKENDS_UPOWER_BATTERY_H
