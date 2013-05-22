/*
    Copyright 2006 Kevin Ottens <ervin@kde.org>
    Copyright 2012 Lukas Tinkl <ltinkl@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SOLID_IFACES_BATTERY_H
#define SOLID_IFACES_BATTERY_H

#include <solid/battery.h>
#include <solid/ifaces/deviceinterface.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This device interface is available on batteries.
     */
    class Battery : virtual public DeviceInterface
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
         * Retrieves the current charge level of the battery normalised
         * to percent.
         *
         * @return the current charge level normalised to percent
         */
        virtual int chargePercent() const = 0;

        /**
          * The capacity of the power source expressed as a percentage between 0 and 100.
          * The capacity of the battery will reduce with age.
          * A capacity value less than 75% is usually a sign that you should renew your battery.
          *
          * @return the current capacity normalized to percent
          */
        virtual int capacity() const = 0;


        /**
         * Indicates if the battery is rechargeable.
         *
         * @return true if the battery is rechargeable, false otherwise (one time usage)
         */
        virtual bool isRechargeable() const = 0;

        /**
         * Indicates if the battery is powering the machine.
         *
         * @return true if the battery is powersupply, false otherwise
         */
        virtual bool isPowerSupply() const = 0;

        /**
         * Retrieves the current charge state of the battery. It can be in a stable
         * state (no charge), charging or discharging.
         *
         * @return the current battery charge state
         * @see Solid::Battery::ChargeState
         */
        virtual Solid::Battery::ChargeState chargeState() const = 0;

        /**
          * Retrieves the technology used to manufacture the battery.
          *
          * @return the battery technology
          * @see Solid::Battery::Technology
          */
        virtual Solid::Battery::Technology technology() const = 0;

        /**
          * Amount of energy (measured in Wh) currently available in the power source.
          *
          * @return amount of battery energy in Wh
          */
        virtual double energy() const = 0;

        /**
          * Amount of energy being drained from the source, measured in W.
          * If positive, the source is being discharged, if negative it's being charged.
          *
          * @return battery rate in Watts
          *
          */
        virtual double energyRate() const = 0;

        /**
          * Voltage in the Cell or being recorded by the meter.
          *
          * @return voltage in Volts
          */
        virtual double voltage() const = 0;

    protected:
    //Q_SIGNALS:
        /**
         * This signal is emitted when the charge percent value of this
         * battery has changed.
         *
         * @param value the new charge percent value of the battery
         * @param udi the UDI of the battery with the new charge percent
         */
        virtual void chargePercentChanged(int value, const QString &udi) = 0;

        /**
         * This signal is emitted when the charge state of this battery
         * has changed.
         *
         * @param newState the new charge state of the battery, it's one of
         * the type Solid::Battery::ChargeState
         * @see Solid::Battery::ChargeState
         * @param udi the UDI of the battery with the new charge state
         */
        virtual void chargeStateChanged(int newState, const QString &udi) = 0;

        /**
         * This signal is emitted if the battery get plugged in/out of the
         * battery bay.
         *
         * @param newState the new plugging state of the battery, type is boolean
         * @param udi the UDI of the battery with thew new plugging state
         */
        virtual void plugStateChanged(bool newState, const QString &udi) = 0;

        /**
         * This signal is emitted when the energy value of this
         * battery has changed.
         *
         * @param energy the new energy value of the battery
         * @param udi the UDI of the battery with the new charge percent
         */
        virtual void energyChanged(double energy, const QString &udi) = 0;

        /**
         * This signal is emitted when the energy rate value of this
         * battery has changed.
         *
         * If positive, the source is being discharged, if negative it's being charged.
         *
         * @param energyRate the new energy rate value of the battery
         * @param udi the UDI of the battery with the new charge percent
         */
        virtual void energyRateChanged(double energyRate, const QString &udi) = 0;
    };
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::Battery, "org.kde.Solid.Ifaces.Battery/0.2")

#endif
