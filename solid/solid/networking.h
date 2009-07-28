/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Will Stephenson <wstephenson@kde.org>
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

#ifndef SOLID_NETWORKING
#define SOLID_NETWORKING

#include <QtCore/QObject>

#include <solid/solid_export.h>

namespace Solid
{
    /**
     * This class allow to query the underlying system to discover the available
     * network interfaces and reachable network.It has also the
     * responsibility to notify when a network interface or a network appear or disappear.
     *
     * It's the unique entry point for network management. Applications should use
     * it to find network interfaces, or to be notified about network related changes.
     *
     * Note that it's implemented as a singleton and encapsulates the backend logic.
     */
    namespace Networking
    {
        /**
         * Describes the state of the networking system
         */
        enum Status {
            Unknown, /**< the networking system is not active or unable to report its status - proceed with caution */
            Unconnected,/**< the system is not connected to any network */
            Disconnecting, /**< the system is breaking the connection */
            Connecting, /**< the system is not connected to any network */
            Connected /**< the system is currently connected to a network */
        };

        /**
         * This defines application policy in response to networking connect/disconnect events
         */
        enum ManagementPolicy {
            Manual, /**< Manual - the app should only disconnect when the user does so manually */
            OnNextStatusChange, /**< the app should connect or disconnect the next time the network changes status, thereafter Manual */
            Managed /**< the app should connect or disconnect whenever the KConnectionManager reports a state change */
        };

        /**
         * Get the current networking status
         * If the result is Unknown, the backend may be unconfigured or otherwise in a state where
         * it cannot report useful networking status @ref Solid::Networking::Status.
         */
        SOLID_EXPORT Status status();

        /**
         * Set a policy to manage the application's connect behaviour
         * @param policy the new connection policy
         */
        void setConnectPolicy( ManagementPolicy policy );

        /**
         * Retrieve a policy managing the application's connect behaviour
         * @return the connection policy in use
         */
        ManagementPolicy connectPolicy();

        /**
         * Set a policy to manage the application's disconnect behaviour
         * @param policy the new disconnection policy
         */
        void setDisconnectPolicy( ManagementPolicy policy );

        /**
         * Retrieve a policy managing the application's disconnect behaviour
         * @return the disconnection policy in use
         */
        ManagementPolicy disconnectPolicy();

        /**
         * This object emits signals, for use if your application requires notification
         * of changes to networking.
         */
        class Notifier : public QObject
        {
        Q_OBJECT
        Q_SIGNALS:
        /**
         * Signals that the network status has changed
         * @param status the new status of the network status service
         */
        void statusChanged( Solid::Networking::Status status );
        /**
         * Signals that the system's network has become connected, so receivers
         * should connect their sockets, ioslaves etc.
         *
         * This signal is emitted according to the active connectPolicy.
         */
        void shouldConnect();
        /**
         * Signals that the system's network has become disconnected,
         * so receivers should adjust application state appropriately.
         *
         * This signal is emitted according to the active disconnectPolicy.
         */
        void shouldDisconnect();

        };

        SOLID_EXPORT Notifier *notifier();
    }

} // Solid

#endif
