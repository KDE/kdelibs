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

#ifndef BATTERY_H
#define BATTERY_H

#include <solid/ifaces/battery.h>
#include "deviceinterface.h"

class Battery : public DeviceInterface, virtual public Solid::Ifaces::Battery
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Battery)

public:
    Battery(HalDevice *device);
    virtual ~Battery();

    virtual bool isPlugged() const;
    virtual Solid::Battery::BatteryType type() const;

    virtual QString chargeLevelUnit() const;
    virtual int charge(Solid::Battery::LevelType type = Solid::Battery::CurrentLevel) const;
    virtual int chargePercent() const;

    virtual QString voltageUnit() const;
    virtual int voltage() const;

    virtual bool isRechargeable() const;
    virtual Solid::Battery::ChargeState chargeState() const;

Q_SIGNALS:
    void chargePercentChanged(int value);
    void chargeStateChanged(int newState);

private Q_SLOTS:
    void slotPropertyChanged(const QMap<QString,int> &changes);
};

#endif
