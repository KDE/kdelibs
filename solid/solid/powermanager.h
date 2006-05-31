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

#ifndef SOLID_POWERMANAGER_H
#define SOLID_POWERMANAGER_H

#include <QObject>

#include <kdelibs_export.h>

#include <kstaticdeleter.h>

#include <solid/ifaces/enums.h>

class KJob;

namespace Solid
{
    namespace Ifaces
    {
        class PowerManager;
    }

    /**
     * This class allow to query the underlying system to obtain information
     * about the hardware available.
     *
     * It's the unique entry point for power management. Applications should use
     * it to control or query the power management features of the system.
     *
     * Note that it's implemented as a singleton and encapsulates the backend logic.
     *
     * @author Kevin Ottens <ervin@kde.org>
     */
    class KDE_EXPORT PowerManager : public QObject, public Ifaces::Enums::PowerManager
    {
        Q_OBJECT

    public:
        /**
         * Retrieves the unique instance of this class.
         *
         * @return unique instance of the class
         */
        static PowerManager &self();

        /**
         * Retrieves the unique instance of this class and forces the registration of
         * the given backend.
         * The PowerManager will use this backend. The parameter will be ignored if an
         * instance of PowerManager already exists.
         *
         * Use this method at your own risks. It's primarily available to easier tests
         * writing. If you need to test the PowerManager, use a call to this method, and
         * then use self() as usual.
         *
         * @param backend the in the application
         * @return unique instance of the class
         * @see self()
         */
        static PowerManager &selfForceBackend( Ifaces::PowerManager *backend );

        /**
         * Returns a text describing the error that occured while loading
         * the backend.
         *
         * @return the error description, or QString() if the backend loaded successfully
         */
        const QString &errorText() const;


        /**
         * Retrieves the list of power management schemes available on this system.
         *
         * @return the available power management schemes
         */
        QStringList supportedSchemes() const;

        /**
         * Retrieves a localized description corresponding to the given scheme.
         *
         * @param schemeName the name of the scheme we request the description for
         * @return the associated description
         */
        QString schemeDescription( const QString &schemeName ) const;

        /**
         * Retrieves the name of the current power management scheme used
         * by the system.
         *
         * @return the current scheme
         */
        QString scheme() const;

        /**
         * Changes the current power management scheme.
         *
         * @param name the name of the new scheme
         * @return true if the scheme change succeeded, false otherwise
         */
        bool setScheme( const QString &name );


        /**
         * Retrieves the current state of the system battery.
         *
         * @return the current battery state
         * @see Solid::Ifaces::Enums::PowerManager::BatteryState
         */
        BatteryState batteryState() const;

        /**
         * Retrieves the current state of the system AC adapter.
         *
         * @return the current AC adapter state
         * @see Solid::Ifaces::Enums::PowerManager::AcAdapterState
         */
        AcAdapterState acAdapterState() const;


        /**
         * Retrieves the set of suspend methods supported by the system.
         *
         * @return the suspend methods supported by this system
         * @see Solid::Ifaces::Enums::PowerManager::SuspendMethod
         * @see Solid::Ifaces::Enums::PowerManager::SuspendMethods
         */
        SuspendMethods supportedSuspendMethods() const;

        /**
         * Requests a suspend of the system.
         *
         * @param method the suspend method to use
         * @return the job handling the operation
         */
        KJob *suspend( SuspendMethod method ) const;


        /**
         * Retrieves the set of CPU frequency policies supported by the system.
         *
         * @return the CPU frequency policies supported by this system
         * @see Solid::Ifaces::Enums::PowerManager::CpuFreqPolicy
         * @see Solid::Ifaces::Enums::PowerManager::CpuFreqPolicies
         */
        CpuFreqPolicies supportedCpuFreqPolicies() const;

        /**
         * Retrieves the current CPU frequency policy of the system.
         *
         * @return the current CPU frequency policy used by the system
         * @see Solid::Ifaces::Enums::PowerManager::CpuFreqPolicy
         */
        CpuFreqPolicy cpuFreqPolicy() const;

        /**
         * Changes the current CPU frequency policy of the system.
         *
         * @param newPolicy the new policy
         * @return true if the policy change succeeded, false otherwise
         * @see Solid::Ifaces::Enums::PowerManager::CpuFreqPolicy
         */
        bool setCpuFreqPolicy( CpuFreqPolicy newPolicy );

        /**
         * Checks if a CPU can be disabled.
         *
         * @param cpuNum the number of the CPU we want to check
         * @return true if the given CPU can be disabled, false otherwise
         */
        bool canDisableCpu( int cpuNum ) const;

        /**
         * Enables or disables a CPU.
         *
         * @param cpuNum the number of the CPU we want to enable or disable
         * @param enabled the new state of the CPU
         * @return true if the state change succeded, false otherwise
         */
        bool setCpuEnabled( int cpuNum, bool enabled );


        /**
         * Retrieves a reference to the loaded backend.
         *
         * @return a pointer to the backend, or 0 if no backend is loaded
         */
        const Ifaces::PowerManager *backend() const;

    signals:
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

    private:
        PowerManager();
        PowerManager( Ifaces::PowerManager *backend );
        ~PowerManager();

    private slots:
        void slotSchemeChanged( QString newScheme );
        void slotAcAdapterStateChanged( int newState );
        void slotBatteryStateChanged( int newState );
        void slotButtonPressed( int buttonType );

    private:
        static PowerManager *s_self;
        class Private;
        Private *d;

        friend void ::KStaticDeleter<PowerManager>::destructObject();
    };
}

#endif
