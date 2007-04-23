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
    connect( backendObject, SIGNAL( chargePercentChanged( int ) ),
             this, SIGNAL( chargePercentChanged( int ) ) );

    connect( backendObject, SIGNAL( chargeStateChanged( int ) ),
             this, SIGNAL( chargeStateChanged( int ) ) );
}

Solid::Battery::~Battery()
{

}

bool Solid::Battery::isPlugged() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery*, d->backendObject(), false, isPlugged());
}

Solid::Battery::BatteryType Solid::Battery::type() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery*, d->backendObject(), UnknownBattery, type());
}

QString Solid::Battery::chargeLevelUnit() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery*, d->backendObject(), QString(), chargeLevelUnit());
}

int Solid::Battery::charge( LevelType type ) const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery*, d->backendObject(), 0, charge(type));
}

int Solid::Battery::chargePercent() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery*, d->backendObject(), 0, chargePercent());
}

QString Solid::Battery::voltageUnit() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery*, d->backendObject(), QString(), voltageUnit());
}

int Solid::Battery::voltage() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery*, d->backendObject(), 0, voltage());
}

bool Solid::Battery::isRechargeable() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery*, d->backendObject(), false, isRechargeable());
}

Solid::Battery::ChargeState Solid::Battery::chargeState() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery*, d->backendObject(), NoCharge, chargeState());
}

#include "battery.moc"
