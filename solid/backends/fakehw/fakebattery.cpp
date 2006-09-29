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

#include "fakebattery.h"

FakeBattery::FakeBattery( FakeDevice *device )
    : FakeCapability( device )
{
}

FakeBattery::~FakeBattery()
{
}


bool FakeBattery::isPlugged() const
{
    return fakeDevice()->property( "isPlugged" ).toBool();
}

FakeBattery::BatteryType FakeBattery::type() const
{
    QString name = fakeDevice()->property( "batteryType" ).toString();

    if ( name == "pda" )
    {
        return PdaBattery;
    }
    else if ( name == "ups" )
    {
        return UpsBattery;
    }
    else if ( name == "primary" )
    {
        return PrimaryBattery;
    }
    else if ( name == "mouse" )
    {
        return MouseBattery;
    }
    else if ( name == "keyboard" )
    {
        return KeyboardBattery;
    }
    else if ( name == "keyboard_mouse" )
    {
        return KeyboardMouseBattery;
    }
    else if ( name == "camera" )
    {
        return CameraBattery;
    }
    else
    {
        return UnknownBattery;
    }
}

QString FakeBattery::chargeLevelUnit() const
{
    return fakeDevice()->property( "chargeLevelUnit" ).toString();
}

int FakeBattery::charge( LevelType type ) const
{
    switch( type )
    {
    case MaxLevel:
        return fakeDevice()->property( "maxLevel" ).toInt();
    case LastFullLevel:
        return fakeDevice()->property( "lastFullLevel" ).toInt();
    case CurrentLevel:
        return fakeDevice()->property( "currentLevel" ).toInt();
    case WarningLevel:
        return fakeDevice()->property( "warningLevel" ).toInt();
    case LowLevel:
        return fakeDevice()->property( "lowLevel" ).toInt();
    }

    // Shouldn't happen...
    return -1;
}

int FakeBattery::chargePercent() const
{
    int last_full = charge( LastFullLevel );
    int current = charge( CurrentLevel );

    int percent = ( 100 * current ) / last_full;

    return percent;
}

QString FakeBattery::voltageUnit() const
{
    return fakeDevice()->property( "voltageUnit" ).toString();
}

int FakeBattery::voltage() const
{
    return fakeDevice()->property( "voltage" ).toInt();
}

bool FakeBattery::isRechargeable() const
{
    return fakeDevice()->property( "isRechargeable" ).toBool();
}

FakeBattery::ChargeState FakeBattery::chargeState() const
{
    QString state = fakeDevice()->property( "chargeState" ).toString();

    if ( state == "charging" )
    {
        return Charging;
    }
    else if ( state == "discharging" )
    {
        return Discharging;
    }
    else
    {
        return NoCharge;
    }
}

void FakeBattery::setChargeState( ChargeState newState )
{
    QString name;

    switch( newState )
    {
    case Charging:
        name = "charging";
        break;
    case Discharging:
        name = "discharging";
    case NoCharge:
        name = "noCharge";
    }

    fakeDevice()->setProperty( "chargeState", name );
    emit chargeStateChanged( newState );
}

void FakeBattery::setChargeLevel( int newLevel )
{
    fakeDevice()->setProperty( "currentLevel", newLevel );
    emit chargePercentChanged( chargePercent() );
}

#include "fakebattery.moc"
