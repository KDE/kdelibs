/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

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

#include "battery.h"
#include "battery_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/battery.h>

Solid::Battery::Battery(QObject *backendObject)
    : DeviceInterface(*new BatteryPrivate(), backendObject)
{
    connect(backendObject, SIGNAL(chargePercentChanged(int, const QString &)),
             this, SIGNAL(chargePercentChanged(int, const QString &)));

    connect(backendObject, SIGNAL(chargeStateChanged(int, const QString &)),
             this, SIGNAL(chargeStateChanged(int, const QString &)));

    connect(backendObject, SIGNAL(plugStateChanged(bool, const QString &)),
             this, SIGNAL(plugStateChanged(bool, const QString &)));
}

Solid::Battery::~Battery()
{

}

bool Solid::Battery::isPlugged() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), false, isPlugged());
}

Solid::Battery::BatteryType Solid::Battery::type() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), UnknownBattery, type());
}

int Solid::Battery::chargePercent() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), 0, chargePercent());
}

bool Solid::Battery::isRechargeable() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), false, isRechargeable());
}

Solid::Battery::ChargeState Solid::Battery::chargeState() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), NoCharge, chargeState());
}

#include "battery.moc"
