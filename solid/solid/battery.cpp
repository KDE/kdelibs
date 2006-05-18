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

#include "battery.h"

#include <solid/ifaces/battery.h>

namespace Solid
{
    class Battery::Private
    {
    public:
        Private() : iface( 0 ) {}

        Ifaces::Battery *iface;
    };
}

Solid::Battery::Battery( Ifaces::Battery *iface, QObject *parent )
    : Capability( parent ), d( new Private() )
{
    d->iface = iface;

    connect( d->iface->qobject(), SIGNAL( chargePercentChanged( int ) ),
             this, SLOT( slotChargePercentChanged( int ) ) );

    connect( d->iface->qobject(), SIGNAL( chargeStateChanged( int ) ),
             this, SLOT( slotChargeStateChanged( int ) ) );

}

Solid::Battery::~Battery()
{
    delete d;
}

bool Solid::Battery::isPlugged() const
{
    return d->iface->isPlugged();
}

Solid::Battery::BatteryType Solid::Battery::type() const
{
    return d->iface->type();
}

QString Solid::Battery::chargeLevelUnit() const
{
    return d->iface->chargeLevelUnit();
}

int Solid::Battery::charge( LevelType type ) const
{
    return d->iface->charge( type );
}

int Solid::Battery::chargePercent() const
{
    return d->iface->chargePercent();
}

QString Solid::Battery::voltageUnit() const
{
    return d->iface->voltageUnit();
}

int Solid::Battery::voltage() const
{
    return d->iface->voltage();
}

bool Solid::Battery::isRechargeable() const
{
    return d->iface->isRechargeable();
}

Solid::Battery::ChargeState Solid::Battery::chargeState() const
{
    return d->iface->chargeState();
}

void Solid::Battery::slotChargePercentChanged( int value )
{
    emit chargePercentChanged( value );
}

void Solid::Battery::slotChargeStateChanged( int newState )
{
    emit chargeStateChanged( newState );
}

#include "battery.moc"
