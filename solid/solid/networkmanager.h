/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2006 Kévin Ottens <ervin@kde.org>

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

#include <QObject>
#include <kdelibs_export.h>
#include <kstaticdeleter.h>

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
class KDE_EXPORT NetworkManager : public QObject
{
Q_OBJECT
    public:
         /**
         * Retrieves the unique instance of this class.
         *
         * @return unique instance of the class
         */
        static NetworkManager &self();
        /**
         * Retrieves the unique instance of this class and forces the registration of
         * the given backend.
         * The NetworkManager will use this backend. The parameter will be ignored if an
         * instance of NetworkManager already exists.
         *
         * Use this method at your own risks. It's primarily available to easier tests
         * writing. If you need to test the NetworkManager, use a call to this method, and
         * then use self() as usual.
         *
         * @param backend the in the application
         * @return unique instance of the class
         * @see self()
         */
        static NetworkManager &selfForceBackend( Ifaces::NetworkManager *backend );
        /**
         * Get a list of all network devices in the system
         * Note: includes getDeviceList and getDialupList from knm
         */
        NetworkDeviceList networkDevices();
        /**
         * Get the active devices (all types)
         * TODO: NM only supports 1 active device at present
         */
        NetworkDeviceList activeNetworkDevices();

        /**
         * Access a given device instance
         */
        NetworkDevice findNetworkDevice( const QString & udi );

        /**
         * Retrieves a reference to the loaded backend.
         *
         * @return a pointer to the backend, or 0 if no backend is loaded
         */
        Ifaces::NetworkManager *backend() const;

    public slots:
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
    signals:
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
        NetworkManager( Ifaces::NetworkManager *backend );
        virtual ~NetworkManager();
        NetworkDeviceList buildDeviceList( const QStringList & udiList );

        static NetworkManager * s_self;
        class Private;
        Private * d;
        friend void ::KStaticDeleter<NetworkManager>::destructObject();
};

} // Solid

#endif
