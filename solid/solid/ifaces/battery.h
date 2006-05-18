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

#ifndef SOLID_IFACES_BATTERY_H
#define SOLID_IFACES_BATTERY_H

#include <kdelibs_export.h>

#include <solid/ifaces/capability.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This capability is available on batteries.
     */
    class KDE_EXPORT Battery : virtual public Capability, public Enums::Battery
    {
//         Q_ENUMS( BatteryType ChargeState )
//         Q_PROPERTY( bool plugged READ isPlugged )
//         Q_PROPERTY( BatteryType type READ type )
//         Q_PROPERTY( QString chargeLevelUnit READ chargeLevelUnit )
//         Q_PROPERTY( int charge READ charge )
//         Q_PROPERTY( int chargePercent READ chargePercent )
//         Q_PROPERTY( QString voltageUnit READ voltageUnit )
//         Q_PROPERTY( int voltage READ voltage )
//         Q_PROPERTY( bool rechargeable READ isRechargeable )
//         Q_PROPERTY( ChargeState chargeState READ chargeState )

    public:
        /**
         * Destroys a Battery object.
         */
        virtual ~Battery();



        /**
         * Indicates if this battery is plugged.
         *
         * @return true if the battery is plugged, false otherwise
         */
        virtual bool isPlugged() const = 0;

        /**
         * Retrieves the type of device holding this battery.
         *
         * @return the type of device holding this battery
         * @see Solid::Ifaces::Enums::Battery::BatteryType
         */
        virtual BatteryType type() const = 0;



        /**
         * Retrieves the physical unit used by the charge level values
         * (for example mWh).
         *
         * @return the charge level unit as a string
         * @see charge()
         */
        virtual QString chargeLevelUnit() const = 0;

        /**
         * Retrieves one of the charge level of the battery as requested.
         * By default the current charge level is returned.
         *
         * The unit of the returned value is determined by chargeLevelUnit()
         *
         * @return the requested charge level
         * @see Solid::Ifaces::Enums::Battery::LevelType
         */
        virtual int charge( LevelType type = CurrentLevel ) const = 0;

        /**
         * Retrieves the current charge level of the battery normalised
         * to percent.
         *
         * @return the current charge level normalised to percent
         */
        virtual int chargePercent() const = 0;



        /**
         * Retrieves the physical unit used by the voltage values
         * (for example mV).
         *
         * @return the voltage level unit as a string
         * @see voltage()
         */
        virtual QString voltageUnit() const = 0;

        /**
         * Retrieves the current voltage level of the battery.
         *
         * The unit of the returned value is determined by voltageUnit()
         */
        virtual int voltage() const = 0;



        /**
         * Indicates if the battery is rechargeable.
         *
         * @return true if the battery is rechargeable, false otherwise (one time usage)
         */
        virtual bool isRechargeable() const = 0;

        /**
         * Retrieves the current charge state of the battery. It can be in a stable
         * state (no charge), charging or discharging.
         *
         * @return the current battery charge state
         * @see Solid::Ifaces::Enums::Battery::ChargeState
         */
        virtual ChargeState chargeState() const = 0;

    protected:
    //signals:
        /**
         * This signal is emitted when the charge percent value of this
         * battery has changed.
         *
         * @param value the new charge percent value of the battery
         */
        virtual void chargePercentChanged( int value ) = 0;

        /**
         * This signal is emitted when the charge state of this battery
         * has changed.
         *
         * @param newState the new charge state of the battery, it's one of
         * the type Solid::Ifaces::Enums::ChargeState
         * @see Solid::Ifaces::Enums::ChargeState
         */
        virtual void chargeStateChanged( int newState ) = 0;
    };
}
}

#endif
