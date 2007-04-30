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
#include <QtCore/QSet>

#include <solid/solid_export.h>

class KJob;

namespace Solid
{
    /**
     * This namespace allows to query the underlying system to obtain information
     * about the hardware available.
     *
     * It is the single entry point for power management. Applications should use
     * it to control or query the power management features of the system.
     *
     * Note that it's implemented as a singleton and encapsulates the backend logic.
     *
     * @author Kevin Ottens <ervin@kde.org>
     */
    namespace PowerManagement
    {
        /**
         * This enum type defines the different suspend methods.
         *
         * - StandbyState: Processes are stopped, some hardware is deactivated (ACPI S1)
         * - SuspendState: Most devices are deactivated, only RAM is powered (ACPI S3)
         * - HibernateState: State of the machine is saved to disk, and the machine is powered down (ACPI S4)
         */
        enum SleepState { StandbyState = 1, SuspendState = 2, HibernateState = 4 };

        /**
         * Retrieves a high level indication of how applications should behave according to the 
         * power management subsystem.  For example, when on battery power, this method will return
         * true.
         *
         * @return whether apps should conserve power
         */
        SOLID_EXPORT bool appShouldConserveResources();


        /**
         * Retrieves the set of suspend methods supported by the system.
         *
         * @return the suspend methods supported by this system
         * @see Solid::PowerManager::SuspendMethod
         * @see Solid::PowerManager::SuspendMethods
         */
        SOLID_EXPORT QSet<SleepState> supportedSleepStates();

        SOLID_EXPORT QString stringForSleepState(SleepState state);

        /**
         * Requests that the system goes to sleep
         *
         * @param method the sleep state use
         * @receiver the object to call a slot on once the operation completes
         * @member the slot to call
         */
        SOLID_EXPORT void requestSleep(SleepState state, QObject *receiver, const char *member);

        /**
         * This enum describes scenarios when system sleep should not be suppressed.
         * The motivation for these is for a user application to be able to suppress system sleep,
         * for example so that the system does not sleep during a DVD-R write even if the laptop lid
         * is closed, but that it should sleep if the battery is almost empty.
         * This enum is not related to programming exceptions.
         *
         * - SuppressWithoutException: Always suppress system sleep
         * - ExceptUserTriggered: Suppress unless the sleep was caused by the user
         * - ExceptOnLowBattery: Suppress sleep unless due to low battery levels
         * - DefaultSuppressExceptions: Convenience value encompassing both user triggered and low
         *   battery exceptions
         */
        enum SuppressException { SuppressWithoutException = 0, ExceptUserTriggered = 1, ExceptOnLowBattery = 2,
                                 DefaultSuppressExceptions = ExceptUserTriggered|ExceptOnLowBattery };

        Q_DECLARE_FLAGS(SuppressExceptions, SuppressException)

        /**
         * Tell the power management subsystem to suppress automatic system sleep until further
         * notice.
         *
         * @param reason Give a reason for not allowing sleep, to be used in giving user feedback
         * about why a sleep event was prevented
         * @param exceptions Set of SuppressException flags stating which suppress reasons should be
         * * allowed despite the suppress. @see Solid::PowerManager::SuppressException
         * @return a 'cookie' value representing the suppression request.  Used by the power manager to
         * track the application's outstanding suppression requests.  Returns -1 if the request was
         * denied.
         */
        SOLID_EXPORT int beginSuppressingSleep(const QString &reason = QString(),
                                               SuppressExceptions exceptions = DefaultSuppressExceptions);
        /**
         * Tell the power management that a particular sleep suppression is no longer needed.  When
         * no more suppressions are active, the system will be free to sleep automatically
         * @param cookie The cookie acquired when requesting sleep suppression
         * @return true if the suppression was stopped, false if an invalid cookie was given
         */
        SOLID_EXPORT bool stopSuppressingSleep(int cookie);

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
            void appShouldConserveResourcesChanged(bool newState);
        };

        SOLID_EXPORT Notifier *notifier();
    }
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Solid::PowerManagement::SuppressExceptions)

#endif
