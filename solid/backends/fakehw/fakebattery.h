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

#ifndef FAKEBATTERY_H
#define FAKEBATTERY_H

#include <solid/ifaces/battery.h>
#include "fakecapability.h"

class FakeBattery : public FakeCapability, virtual public Solid::Ifaces::Battery
{
    Q_OBJECT
    Q_INTERFACES( Solid::Ifaces::Battery )
    Q_ENUMS( BatteryType ChargeState )
    Q_PROPERTY( bool plugged READ isPlugged )
    Q_PROPERTY( BatteryType type READ type )
    Q_PROPERTY( QString chargeLevelUnit READ chargeLevelUnit )
    Q_PROPERTY( int charge READ charge )
    Q_PROPERTY( int chargePercent READ chargePercent )
    Q_PROPERTY( QString voltageUnit READ voltageUnit )
    Q_PROPERTY( int voltage READ voltage )
    Q_PROPERTY( bool rechargeable READ isRechargeable )
    Q_PROPERTY( ChargeState chargeState READ chargeState )

public:
    FakeBattery( FakeDevice *device );
    virtual ~FakeBattery();

public Q_SLOTS:
    virtual bool isPlugged() const;
    virtual BatteryType type() const;

    virtual QString chargeLevelUnit() const;
    virtual int charge( LevelType type = CurrentLevel ) const;
    virtual int chargePercent() const;

    virtual QString voltageUnit() const;
    virtual int voltage() const;

    virtual bool isRechargeable() const;
    virtual ChargeState chargeState() const;

    void setChargeState( ChargeState newState );
    void setChargeLevel( int newLevel );

Q_SIGNALS:
    void chargePercentChanged( int value );
    void chargeStateChanged( int newState );
};

#endif
