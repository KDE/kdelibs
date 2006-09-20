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
#include <solid/ifaces/networkdevice.h>
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
        NetworkManager( QObject * parent = 0 );
        virtual ~NetworkManager();

        /**
         * Get a list of all network device UDIs in the system
         * Note: includes getDeviceList and getDialupList from knm
         */
        virtual QStringList networkDevices() const = 0;
        /**
         * Get the active device UDIs (all types)
         * TODO: NM only supports 1 active device at present
         */
        virtual QStringList activeNetworkDevices() const = 0;
        /**
         * Create a backend specific device instance
         */
        virtual NetworkDevice * createNetworkDevice( const QString & ) = 0;
        
    public slots:
        /**
         * Tell the backend to activate a network
         * TODO: Also dialup, VPN?
         */
        virtual void activate( const QString & ) = 0;
        /**
         * Tell the backend to activate a network
         * TODO: Also dialup, VPN?
         */
        virtual void deactivate( const QString & ) = 0;
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
        virtual void notifyHiddenNetwork( const QString & essid ) = 0;
    signals:
        /**
         * Emitted when the system notices a new device was added
         */
        void added( const QString & udi );
        /**
         * Emitted when the system notices a device was removed
         */
        void removed( const QString & udi );
};

} // Ifaces

} // Solid

#endif
