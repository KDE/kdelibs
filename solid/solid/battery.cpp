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

#include <kdehw/ifaces/battery.h>

namespace KDEHW
{
    class Battery::Private
    {
    public:
        Private() : iface( 0 ) {}

        Ifaces::Battery *iface;
    };
}

KDEHW::Battery::Battery( Ifaces::Battery *iface, QObject *parent )
    : Capability( parent ), d( new Private() )
{
    d->iface = iface;

    connect( d->iface->qobject(), SIGNAL( chargePercentChanged( int ) ),
             this, SLOT( slotChargePercentChanged( int ) ) );

    connect( d->iface->qobject(), SIGNAL( chargeStateChanged( int ) ),
             this, SLOT( slotChargeStateChanged( int ) ) );

}

KDEHW::Battery::~Battery()
{
    delete d;
}

bool KDEHW::Battery::isPlugged() const
{
    return d->iface->isPlugged();
}

KDEHW::Battery::BatteryType KDEHW::Battery::type() const
{
    return d->iface->type();
}

QString KDEHW::Battery::chargeLevelUnit() const
{
    return d->iface->chargeLevelUnit();
}

int KDEHW::Battery::charge( LevelType type ) const
{
    return d->iface->charge( type );
}

int KDEHW::Battery::chargePercent() const
{
    return d->iface->chargePercent();
}

QString KDEHW::Battery::voltageUnit() const
{
    return d->iface->voltageUnit();
}

int KDEHW::Battery::voltage() const
{
    return d->iface->voltage();
}

bool KDEHW::Battery::isRechargeable() const
{
    return d->iface->isRechargeable();
}

KDEHW::Battery::ChargeState KDEHW::Battery::chargeState() const
{
    return d->iface->chargeState();
}

void KDEHW::Battery::slotChargePercentChanged( int value )
{
    emit chargePercentChanged( value );
}

void KDEHW::Battery::slotChargeStateChanged( int newState )
{
    emit chargeStateChanged( newState );
}

#include "battery.moc"
