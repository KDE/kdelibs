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

#ifndef SOLID_POWERMANAGEMENT_H
#define SOLID_POWERMANAGEMENT_H

#include <QtCore/QObject>

#include <solid/solid_export.h>

class KJob;

namespace Solid
{
    /**
     * This namespace allows to query the underlying system to obtain information
     * about the hardware available.
     *
     * It's the unique entry point for power management. Applications should use
     * it to control or query the power management features of the system.
     *
     * Note that it's implemented as a singleton and encapsulates the backend logic.
     *
     * @author Kevin Ottens <ervin@kde.org>
     */
    namespace PowerManagement
    {
        /**
         * This enum type defines the different states of the AC adapter.
         *
         * - UnknownAcAdapterState: The AC adapter has an unknown state
         * - Plugged: The AC adapter is plugged
         * - Unplugged: The AC adapter is unplugged
         */
        enum AcAdapterState{ UnknownAcAdapterState, Plugged, Unplugged };

        /**
         * This enum type defines the different suspend methods.
         *
         * - UnknownSuspendMethod: The name says it all
         * - Standby: Processes are stopped, some hardware is deactivated (ACPI S1)
         * - ToRam: Most devices are deactivated, only RAM is powered (ACPI S3)
         * - ToDisk: State of the machine is saved to disk, and it's powered down (ACPI S4)
         */
        enum SuspendMethod{ UnknownSuspendMethod = 0, Standby = 1, ToRam = 2, ToDisk = 4};

        /**
         * This type stores an OR combination of SuspendMethod values.
         */
        Q_DECLARE_FLAGS( SuspendMethods, SuspendMethod )

        /**
         * Retrieves the current state of the system AC adapter.
         *
         * @return the current AC adapter state
         * @see Solid::PowerManager::AcAdapterState
         */
        SOLID_EXPORT AcAdapterState acAdapterState();


        /**
         * Retrieves the set of suspend methods supported by the system.
         *
         * @return the suspend methods supported by this system
         * @see Solid::PowerManager::SuspendMethod
         * @see Solid::PowerManager::SuspendMethods
         */
        SOLID_EXPORT SuspendMethods supportedSuspendMethods();

        /**
         * Requests a suspend of the system.
         *
         * @param method the suspend method to use
         * @return the job handling the operation
         */
        SOLID_EXPORT KJob *suspend(SuspendMethod method);


        class Notifier : public QObject
        {
            Q_OBJECT
        Q_SIGNALS:
            /**
             * This signal is emitted when the AC adapter is plugged or unplugged.
             *
             * @param newState the new state of the AC adapter, it's one of the
             * type @see Solid::PowerManager::AcAdapterState
             */
            void acAdapterStateChanged(int newState);
        };

        SOLID_EXPORT Notifier *notifier();
    }
}

Q_DECLARE_OPERATORS_FOR_FLAGS( Solid::PowerManagement::SuspendMethods )

#endif
