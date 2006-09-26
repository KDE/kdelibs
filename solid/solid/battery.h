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

#ifndef SOLID_BATTERY_H
#define SOLID_BATTERY_H

#include <kdelibs_export.h>

#include <solid/capability.h>
#include <solid/ifaces/enums.h>

namespace Solid
{
    /**
     * This capability is available on batteries.
     */
    class KDE_EXPORT Battery : public Capability, public Ifaces::Enums::Battery
    {
        Q_OBJECT
    public:
        /**
         * Creates a new Battery object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the capability object provided by the backend
         * @see Solid::Device::as()
         */
        Battery( QObject *backendObject );

        /**
         * Destroys a Battery object.
         */
        virtual ~Battery();


        /**
         * Get the Solid::Capability::Type of the Battery capability.
         *
         * @return the Battery capability type
         * @see Solid::Ifaces::Enums::Capability::Type
         */
        static Type capabilityType() { return Capability::Battery; }

        /**
         * Indicates if this battery is plugged.
         *
         * @return true if the battery is plugged, false otherwise
         */
        bool isPlugged() const;

        /**
         * Retrieves the type of device holding this battery.
         *
         * @return the type of device holding this battery
         * @see Solid::Ifaces::Enums::Battery::BatteryType
         */
        BatteryType type() const;



        /**
         * Retrieves the physical unit used by the charge level values
         * (for example mWh).
         *
         * @return the charge level unit as a string
         * @see charge()
         */
        QString chargeLevelUnit() const;

        /**
         * Retrieves one of the charge level of the battery as requested.
         * By default the current charge level is returned.
         *
         * The unit of the returned value is determined by chargeLevelUnit()
         *
         * @return the requested charge level
         * @see Solid::Ifaces::Enums::Battery::LevelType
         */
        int charge( LevelType type = CurrentLevel ) const;

        /**
         * Retrieves the current charge level of the battery normalised
         * to percent.
         *
         * @return the current charge level normalised to percent
         */
        int chargePercent() const;



        /**
         * Retrieves the physical unit used by the voltage values
         * (for example mV).
         *
         * @return the voltage level unit as a string
         * @see voltage()
         */
        QString voltageUnit() const;

        /**
         * Retrieves the current voltage level of the battery.
         *
         * The unit of the returned value is determined by voltageUnit()
         */
        int voltage() const;



        /**
         * Indicates if the battery is rechargeable.
         *
         * @return true if the battery is rechargeable, false otherwise (one time usage)
         */
        bool isRechargeable() const;

        /**
         * Retrieves the current charge state of the battery. It can be in a stable
         * state (no charge), charging or discharging.
         *
         * @return the current battery charge state
         * @see Solid::Ifaces::Enums::Battery::ChargeState
         */
        ChargeState chargeState() const;

    Q_SIGNALS:
        /**
         * This signal is emitted when the charge percent value of this
         * battery has changed.
         *
         * @param value the new charge percent value of the battery
         */
        void chargePercentChanged( int value );

        /**
         * This signal is emitted when the charge state of this battery
         * has changed.
         *
         * @param newState the new charge state of the battery, it's one of
         * the type Solid::Ifaces::Enums::ChargeState
         * @see Solid::Ifaces::Enums::ChargeState
         */
        void chargeStateChanged( int newState );
    };
}

#endif
