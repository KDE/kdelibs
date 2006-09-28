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
         * Get the active devices (all types)
         * TODO: NM only supports 1 active device at present
         */
        NetworkDeviceList activeNetworkDevices() const;

        /**
         * Access a given device instance
         */
        const NetworkDevice &findNetworkDevice( const QString & udi ) const;

    public Q_SLOTS:
        /**
         * Tell the backend to activate a network
         * TODO: Also dialup, VPN?
         */
        void activate( const QString & uid );
        /**
         * Tell the backend to activate a network
         * TODO: Also dialup, VPN?
         */
        void deactivate( const QString & uid );
        /**
         * disable wireless networking
         */
        void enableWireless( bool enabled );
        /**
         * disable all networking - go to passive mode
         */
        void enableNetworking( bool enabled );
        /**
         * Inform the backend of hidden wireless networks
         */
        void notifyHiddenNetwork( const QString & essid );
    Q_SIGNALS:
        /**
         * Emitted when the system notices a new device was added
         */
        void added( const QString & );
        /**
         * Emitted when the system notices a device was removed
         */
        void removed( const QString & );

    private:
        NetworkManager();
        NetworkManager( QObject *backend );
        virtual ~NetworkManager();
        NetworkDeviceList buildDeviceList( const QStringList & udiList ) const;

    private Q_SLOTS:
        void slotAdded( const QString &uni );
        void slotRemoved( const QString &uni );
        void slotDestroyed( QObject *object );

    private:
        class Private;
        Private * d;
};

} // Solid

#endif
