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

Solid::Battery::BatteryType FakeBattery::type() const
{
    QString name = fakeDevice()->property( "batteryType" ).toString();

    if ( name == "pda" )
    {
        return Solid::Battery::PdaBattery;
    }
    else if ( name == "ups" )
    {
        return Solid::Battery::UpsBattery;
    }
    else if ( name == "primary" )
    {
        return Solid::Battery::PrimaryBattery;
    }
    else if ( name == "mouse" )
    {
        return Solid::Battery::MouseBattery;
    }
    else if ( name == "keyboard" )
    {
        return Solid::Battery::KeyboardBattery;
    }
    else if ( name == "keyboard_mouse" )
    {
        return Solid::Battery::KeyboardMouseBattery;
    }
    else if ( name == "camera" )
    {
        return Solid::Battery::CameraBattery;
    }
    else
    {
        return Solid::Battery::UnknownBattery;
    }
}

QString FakeBattery::chargeLevelUnit() const
{
    return fakeDevice()->property( "chargeLevelUnit" ).toString();
}

int FakeBattery::charge( Solid::Battery::LevelType type ) const
{
    switch( type )
    {
    case Solid::Battery::MaxLevel:
        return fakeDevice()->property( "maxLevel" ).toInt();
    case Solid::Battery::LastFullLevel:
        return fakeDevice()->property( "lastFullLevel" ).toInt();
    case Solid::Battery::CurrentLevel:
        return fakeDevice()->property( "currentLevel" ).toInt();
    case Solid::Battery::WarningLevel:
        return fakeDevice()->property( "warningLevel" ).toInt();
    case Solid::Battery::LowLevel:
        return fakeDevice()->property( "lowLevel" ).toInt();
    }

    // Shouldn't happen...
    return -1;
}

int FakeBattery::chargePercent() const
{
    int last_full = charge( Solid::Battery::LastFullLevel );
    int current = charge( Solid::Battery::CurrentLevel );

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

Solid::Battery::ChargeState FakeBattery::chargeState() const
{
    QString state = fakeDevice()->property( "chargeState" ).toString();

    if ( state == "charging" )
    {
        return Solid::Battery::Charging;
    }
    else if ( state == "discharging" )
    {
        return Solid::Battery::Discharging;
    }
    else
    {
        return Solid::Battery::NoCharge;
    }
}

void FakeBattery::setChargeState( Solid::Battery::ChargeState newState )
{
    QString name;

    switch( newState )
    {
    case Solid::Battery::Charging:
        name = "charging";
        break;
    case Solid::Battery::Discharging:
        name = "discharging";
    case Solid::Battery::NoCharge:
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
