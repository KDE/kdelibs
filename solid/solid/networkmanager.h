/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
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

#ifndef SOLID_NETWORKMANAGER
#define SOLID_NETWORKMANAGER

#include <solid/managerbase.h>
#include <kdelibs_export.h>

namespace Solid
{
    namespace Ifaces
    {
        class NetworkManager;
    }
    class Network;
    class NetworkInterface;
    typedef QList<NetworkInterface> NetworkInterfaceList;

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
    class SOLID_EXPORT NetworkManager : public ManagerBase
    {
        Q_OBJECT
        SOLID_SINGLETON( NetworkManager )

    public:
        /**
         * Retrieves the list of all the network interfaces in the system.
         * It includes both hardware and virtual devices.
         *
         * @return the list of network interfaces available in this system
         */
        NetworkInterfaceList networkInterfaces() const;

        /**
         * Find a new NetworkInterface object given its UNI.
         *
         * @param uni the identifier of the network interface to find
         * @returns a valid NetworkInterface object if there's a device having the given UNI, an invalid one otherwise
         */
        const NetworkInterface &findNetworkInterface( const QString & uni ) const;

        /**
         * Retrieves the activation status of networking (as a whole) in the system.
         *
         * @return true if this networking is enabled, false otherwise
         */
        bool isNetworkingEnabled() const;

        /**
         * Retrieves the activation status of wireless networking in the system.
         *
         * @return true if this wireless networking is enabled, false otherwise
         */
        bool isWirelessEnabled() const;


    public Q_SLOTS:
        /**
         * Activates or deactivates networking (as a whole).
         *
         * @param enabled true to activate networking, false otherwise
         */
        void setNetworkingEnabled( bool enabled );

        /**
         * Activates or deactivates wireless networking.
         *
         * @param enabled true to activate wireless networking, false otherwise
         */
        void setWirelessEnabled( bool enabled );

        /**
         * Informs the system of hidden networks.
         *
         * @param networkName the name of the hidden network that could be discovered
         */
        void notifyHiddenNetwork( const QString &networkName );

    Q_SIGNALS:
        /**
         * This signal is emitted when a new network interface is available.
         *
         * @param uni the network interface identifier
         */
        void networkInterfaceAdded( const QString & uni );

        /**
         * This signal is emitted when a network interface is not available anymore.
         *
         * @param uni the network interface identifier
         */
        void networkInterfaceRemoved( const QString & uni );

    private:
        NetworkManager();
        NetworkManager( QObject *backend );
        virtual ~NetworkManager();
        NetworkInterfaceList buildDeviceList( const QStringList & udiList ) const;

    private Q_SLOTS:
        void slotNetworkInterfaceAdded( const QString &uni );
        void slotNetworkInterfaceRemoved( const QString &uni );
        void slotDestroyed( QObject *object );

    private:
        class Private;
        Private * d;
};

} // Solid

#endif
