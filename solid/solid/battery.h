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

#ifndef SOLID_BATTERY_H
#define SOLID_BATTERY_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
    class BatteryPrivate;

    /**
     * This device interface is available on batteries.
     */
    class SOLID_EXPORT Battery : public DeviceInterface
    {
        Q_OBJECT
        Q_ENUMS(BatteryType LevelType ChargeState)
        Q_PROPERTY(bool plugged READ isPlugged)
        Q_PROPERTY(BatteryType type READ type)
        Q_PROPERTY(QString chargeLevelUnit READ chargeLevelUnit)
        Q_PROPERTY(int charge READ charge)
        Q_PROPERTY(int chargePercent READ chargePercent)
        Q_PROPERTY(QString voltageUnit READ voltageUnit)
        Q_PROPERTY(int voltage READ voltage)
        Q_PROPERTY(bool rechargeable READ isRechargeable)
        Q_PROPERTY(ChargeState chargeState READ chargeState)
        Q_DECLARE_PRIVATE(Battery)

    public:
        /**
         * This enum type defines the type of the device holding the battery
         *
         * - PdaBattery : A battery in a Personal Digital Assistant
         * - UpsBattery : A battery in an Uninterruptible Power Supply
         * - PrimaryBattery : A primary battery for the system (for example laptop battery)
         * - MouseBattery : A battery in a mouse
         * - KeyboardBattery : A battery in a keyboard
         * - KeyboardMouseBattery : A battery in a combined keyboard and mouse
         * - CameraBattery : A battery in a camera
         * - UnknownBattery : A battery in an unknown device
         */
        enum BatteryType { UnknownBattery, PdaBattery, UpsBattery,
                           PrimaryBattery, MouseBattery, KeyboardBattery,
                           KeyboardMouseBattery, CameraBattery };

        /**
         * This enum type defines the kind of charge level a battery can expose
         *
         * - MaxLevel : The maximum charge level the battery got designed for
         * - LastFullLevel : The last charge level the battery got when full
         * - CurrentLevel : The current charge level
         * - WarningLevel : The battery is in 'warning' state below this level
         * - LowLevel : The battery is in 'low' state below this level
         */
        enum LevelType { MaxLevel, LastFullLevel, CurrentLevel,
                         WarningLevel, LowLevel };

        /**
         * This enum type defines charge state of a battery
         *
         * - NoCharge : Battery charge is stable, not charging or discharging
         * - Charging : Battery is charging
         * - Discharging : Battery is discharging
         */
        enum ChargeState { NoCharge, Charging, Discharging };



        /**
         * Creates a new Battery object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         */
        explicit Battery(QObject *backendObject);

        /**
         * Destroys a Battery object.
         */
        virtual ~Battery();


        /**
         * Get the Solid::DeviceInterface::Type of the Battery device interface.
         *
         * @return the Battery device interface type
         * @see Solid::DeviceInterface::Type
         */
        static Type deviceInterfaceType() { return DeviceInterface::Battery; }

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
         * @see Solid::Battery::BatteryType
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
         * @see Solid::Battery::LevelType
         */
        int charge(LevelType type = CurrentLevel) const;

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
         * @see Solid::Battery::ChargeState
         */
        ChargeState chargeState() const;

    Q_SIGNALS:
        /**
         * This signal is emitted when the charge percent value of this
         * battery has changed.
         *
         * @param value the new charge percent value of the battery
         */
        void chargePercentChanged(int value);

        /**
         * This signal is emitted when the charge state of this battery
         * has changed.
         *
         * @param newState the new charge state of the battery, it's one of
         * the type Solid::Battery::ChargeState
         * @see Solid::Battery::ChargeState
         */
        void chargeStateChanged(int newState);
    };
}

#endif
