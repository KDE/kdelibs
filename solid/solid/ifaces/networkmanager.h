/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>

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

#ifndef SOLID_IFACES_NETWORKMANAGER
#define SOLID_IFACES_NETWORKMANAGER

#include <QObject>
#include <kdelibs_export.h>

#include "network.h"
#include "networkdevice.h"
#include "authentication.h"

namespace Solid
{
namespace Ifaces
{

/**
 * Main class for listing and activating network devices and controlling the backend's network status
 */
class KDE_EXPORT NetworkManager : public QObject
{
Q_OBJECT
    public:
        NetworkManager( QObject * parent );
        virtual ~NetworkManager();

        /**
         * Get a list of all network devices in the system
         * Note: includes getDeviceList and getDialupList from knm
         */
        virtual NetworkDeviceList networkDevices() const = 0;
        /**
         * Get the active devices (all types)
         * TODO: NM only supports 1 active device at present
         */
        virtual NetworkDeviceList activeNetworkDevices() const =0;

    public slots:
        /**
         * Tell the backend to activate a network
         * TODO: Also dialup, VPN?
         */
        virtual void activate( Ifaces::Network * ) = 0;
        /**
         * Tell the backend to activate a network
         * TODO: Also dialup, VPN?
         */
        virtual void deactivate( Ifaces::Network * ) = 0;
        /**
         * disable wireless networking
         */
        virtual void enableWireless( bool enabled ) = 0;
        /**
         * disable all networking - go to passive mode
         */
        virtual void enableNetworking( bool enabled ) = 0;
        /**
         * Inform the backend of hidden wireless networks
         */
        virtual void notifyHiddenNetworks( const QString & essid ) = 0;
    signals:
        /**
         * Emitted when the system notices a new device was added
         */
        void added( Ifaces::NetworkDevice * );
        /**
         * Emitted when the system notices a device was removed
         */
        void removed( Ifaces::NetworkDevice * );
};

/**
 * Read only system connection status oracle.  Intended for light weight usage in applications which want to read connection state but are not interested in details or control
 */
class KDE_EXPORT NetworkStatus : public QObject
{
Q_OBJECT
    public:
        /**
         * Possible states for the network status object
         * TODO: fix doxygen
         * - Unknown = the system has no knowledge of the actual network status. Perhaps the backend daemon is not running.
         * - Disconnected = The system is not connected to any network, but will connect if the possibility arises
         * - Connecting = The only network connection is currently being connected
         * - Connected = At least one network connection is active
         * - Passive = The system is disconnected and not attempting to make a connection
         */
        enum ConnectionState { Unknown, Disconnected, Connecting, Connected, Passive };

        virtual ~NetworkStatus();

       /**
         * Access the current connection state of the system
         */
        virtual ConnectionState connectionState() const = 0;
        /**
         * Check connection state for a given host
         * TODO KUrl?
         */
        virtual ConnectionState connectionState( const QString & host ) const = 0;
   signals:
        /**
         * Emitted when the connection state changed
         */
        void connectionStateChanged( ConnectionState );
   private:
        NetworkStatus( QObject * );
};

} // Ifaces

} // Solid

#endif
