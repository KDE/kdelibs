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

#ifndef SOLID_IFACES_POWERMANAGER_H
#define SOLID_IFACES_POWERMANAGER_H

#include <QObject>
#include <QStringList>

#include <kdelibs_export.h>

#include <solid/ifaces/enums.h>

class KJob;

namespace Solid
{
namespace Ifaces
{
    /**
     * This class specifies the interface a backend will have to implement in
     * order to be used in the system.
     *
     * A power manager allows to control or query the power management features
     * or the underlying platform.
     */
    class KDE_EXPORT PowerManager : public QObject, public Enums::PowerManager
    {
        Q_OBJECT

    public:
        /**
         * Constructs a PowerManager
         */
        PowerManager( QObject *parent = 0 );

        /**
         * Destructs a PowerManager object
         */
        virtual ~PowerManager();


        /**
         * Retrieves the list of power management schemes available on this system.
         *
         * @return the available power management schemes
         */
        virtual QStringList supportedSchemes() const = 0;

        /**
         * Retrieves a localized description corresponding to the given scheme.
         *
         * @param schemeName the name of the scheme we request the description for
         * @return the associated description
         */
        virtual QString schemeDescription( const QString &schemeName ) const = 0;

        /**
         * Retrieves the name of the current power management scheme used
         * by the system.
         *
         * @return the current scheme
         */
        virtual QString scheme() const = 0;

        /**
         * Changes the current power management scheme.
         *
         * @param name the name of the new scheme
         * @return true if the scheme change succeeded, false otherwise
         */
        virtual bool setScheme( const QString &name ) = 0;


        /**
         * Retrieves the current state of the system battery.
         *
         * @return the current battery state
         * @see Solid::Ifaces::Enums::PowerManager::BatteryState
         */
        virtual BatteryState batteryState() const = 0;

        /**
         * Retrieves the current state of the system AC adapter.
         *
         * @return the current AC adapter state
         * @see Solid::Ifaces::Enums::PowerManager::AcAdapterState
         */
        virtual AcAdapterState acAdapterState() const = 0;


        /**
         * Retrieves the set of suspend methods supported by the system.
         *
         * @return the suspend methods supported by this system
         * @see Solid::Ifaces::Enums::PowerManager::SuspendMethod
         * @see Solid::Ifaces::Enums::PowerManager::SuspendMethods
         */
        virtual SuspendMethods supportedSuspendMethods() const = 0;

        /**
         * Requests a suspend of the system.
         *
         * @param method the suspend method to use
         * @return the job handling the operation
         */
        virtual KJob *suspend( SuspendMethod method ) const = 0;


        /**
         * Retrieves the set of CPU frequency policies supported by the system.
         *
         * @return the CPU frequency policies supported by this system
         * @see Solid::Ifaces::Enums::PowerManager::CpuFreqPolicy
         * @see Solid::Ifaces::Enums::PowerManager::CpuFreqPolicies
         */
        virtual CpuFreqPolicies supportedCpuFreqPolicies() const = 0;

        /**
         * Retrieves the current CPU frequency policy of the system.
         *
         * @return the current CPU frequency policy used by the system
         * @see Solid::Ifaces::Enums::PowerManager::CpuFreqPolicy
         */
        virtual CpuFreqPolicy cpuFreqPolicy() const = 0;

        /**
         * Changes the current CPU frequency policy of the system.
         *
         * @param newPolicy the new policy
         * @return true if the policy change succeeded, false otherwise
         * @see Solid::Ifaces::Enums::PowerManager::CpuFreqPolicy
         */
        virtual bool setCpuFreqPolicy( CpuFreqPolicy newPolicy ) = 0;

        /**
         * Checks if a CPU can be disabled.
         *
         * @param cpuNum the number of the CPU we want to check
         * @return true if the given CPU can be disabled, false otherwise
         */
        virtual bool canDisableCpu( int cpuNum ) const = 0;

        /**
         * Enables or disables a CPU.
         *
         * @param cpuNum the number of the CPU we want to enable or disable
         * @param enabled the new state of the CPU
         * @return true if the state change succeded, false otherwise
         */
        virtual bool setCpuEnabled( int cpuNum, bool enabled ) = 0;

    Q_SIGNALS:
        /**
         * This signal is emitted when the power management scheme has changed.
         *
         * @param newScheme the new scheme name
         */
        void schemeChanged( QString newScheme );

        /**
         * This signal is emitted when the AC adapter is plugged or unplugged.
         *
         * @param newState the new state of the AC adapter, it's one of the
         * type @see Solid::Ifaces::Enums::PowerManager::AcAdapterState
         */
        void acAdapterStateChanged( int newState );

        /**
         * This signal is emitted when the system battery state changed.
         *
         * @param newState the new state of the system battery, it's one of the
         * type @see Solid::Ifaces::Enums::PowerManager::BatteryState
         */
        void batteryStateChanged( int newState );

        /**
         * This signal is emitted when a button has been pressed.
         *
         * @param buttonType the pressed button type, it's one of the
         * type @see Solid::Ifaces::Enums::PowerManager::ButtonType
         */
        void buttonPressed( int buttonType );
    };
}
}

#endif
