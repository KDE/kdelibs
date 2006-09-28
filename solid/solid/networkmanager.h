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
    class NetworkDevice;
    typedef QList<NetworkDevice> NetworkDeviceList;

    /**
     * Main class for listing and activating network devices and controlling the backend's network status
     */
    class KDE_EXPORT NetworkManager : public ManagerBase
    {
        Q_OBJECT
        SOLID_SINGLETON( NetworkManager )

    public:
        /**
         * Get a list of all network devices in the system
         * Note: includes getDeviceList and getDialupList from knm
         */
        NetworkDeviceList networkDevices() const;

        /**
         * Access a given device instance
         */
        const NetworkDevice &findNetworkDevice( const QString & udi ) const;

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
         * @param true to activate networking, false otherwise
         */
        void setNetworkingEnabled( bool enabled );

        /**
         * Activates or deactivates wireless networking.
         *
         * @param true to activate wireless networking, false otherwise
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
         * This signal is emitted when a new network device is available.
         *
         * @param uni the network device identifier
         */
        void networkDeviceAdded( const QString & uni );

        /**
         * This signal is emitted when a network device is not available anymore.
         *
         * @param uni the network device identifier
         */
        void networkDeviceRemoved( const QString & uni );

    private:
        NetworkManager();
        NetworkManager( QObject *backend );
        virtual ~NetworkManager();
        NetworkDeviceList buildDeviceList( const QStringList & udiList ) const;

    private Q_SLOTS:
        void slotNetworkDeviceAdded( const QString &uni );
        void slotNetworkDeviceRemoved( const QString &uni );
        void slotDestroyed( QObject *object );

    private:
        class Private;
        Private * d;
};

} // Solid

#endif
