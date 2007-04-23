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

#include <solid/solid_export.h>

#include <solid/battery.h>
#include <solid/ifaces/deviceinterface.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This device interface is available on batteries.
     */
    class SOLIDIFACES_EXPORT Battery : virtual public DeviceInterface
    {
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
         * @see Solid::Battery::BatteryType
         */
        virtual Solid::Battery::BatteryType type() const = 0;



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
         * @see Solid::Battery::LevelType
         */
        virtual int charge(Solid::Battery::LevelType type = Solid::Battery::CurrentLevel) const = 0;

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
         * @see Solid::Battery::ChargeState
         */
        virtual Solid::Battery::ChargeState chargeState() const = 0;

    protected:
    //Q_SIGNALS:
        /**
         * This signal is emitted when the charge percent value of this
         * battery has changed.
         *
         * @param value the new charge percent value of the battery
         */
        virtual void chargePercentChanged(int value) = 0;

        /**
         * This signal is emitted when the charge state of this battery
         * has changed.
         *
         * @param newState the new charge state of the battery, it's one of
         * the type Solid::Battery::ChargeState
         * @see Solid::Battery::ChargeState
         */
        virtual void chargeStateChanged(int newState) = 0;
    };
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::Battery, "org.kde.Solid.Ifaces.Battery/0.1")

#endif
