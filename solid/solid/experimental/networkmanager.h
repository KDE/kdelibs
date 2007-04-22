/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
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

#ifndef SOLID_NETWORKMANAGER
#define SOLID_NETWORKMANAGER

#include <QtCore/QObject>

#include <solid/solid_export.h>

namespace SolidExperimental
{
    namespace Ifaces
    {
        class NetworkManager;
    }
    class Network;
    class NetworkInterface;
    typedef QList<NetworkInterface> NetworkInterfaceList;
    class NetworkManagerPrivate;
    class AuthenticationValidator;

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
    namespace NetworkManager
    {
        /**
         * Retrieves the list of all the network interfaces in the system.
         * It includes both hardware and virtual devices.
         *
         * @return the list of network interfaces available in this system
         */
        SOLID_EXPORT NetworkInterfaceList networkInterfaces();

        /**
         * Find a new NetworkInterface object given its UNI.
         *
         * @param uni the identifier of the network interface to find
         * @returns a valid NetworkInterface object if there's a device having the given UNI, an invalid one otherwise
         */
        SOLID_EXPORT const NetworkInterface &findNetworkInterface(const QString& uni);

        /**
         * Retrieves the status of networking (as a whole) in the system.
         * This is distinct from whether the system's networking is online or offline.
         * To check that, see @ref NetworkStatus.
         *
         * @return true if this networking is enabled, false otherwise
         */
        SOLID_EXPORT bool isNetworkingEnabled();

        /**
         * Retrieves the activation status of wireless networking in the system.
         *
         * @return true if this wireless networking is enabled, false otherwise
         */
        SOLID_EXPORT bool isWirelessEnabled();


        /**
         * Activates or deactivates networking (as a whole).
         *
         * @param enabled true to activate networking, false otherwise
         */
        SOLID_EXPORT void setNetworkingEnabled(bool enabled);

        /**
         * Activates or deactivates wireless networking.
         *
         * @param enabled true to activate wireless networking, false otherwise
         */
        SOLID_EXPORT void setWirelessEnabled(bool enabled);

        /**
         * Informs the system of hidden networks.
         *
         * @param networkName the name of the hidden network that could be discovered
         */
        SOLID_EXPORT void notifyHiddenNetwork(const QString &networkName);

        class Notifier : public QObject
        {
            Q_OBJECT
        Q_SIGNALS:
            /**
             * This signal is emitted when a new network interface is available.
             *
             * @param uni the network interface identifier
             */
            void networkInterfaceAdded(const QString &uni);

            /**
             * This signal is emitted when a network interface is not available anymore.
             *
             * @param uni the network interface identifier
             */
            void networkInterfaceRemoved(const QString &uni);
        };

        SOLID_EXPORT Notifier *notifier();
    }

} // Solid

#endif
