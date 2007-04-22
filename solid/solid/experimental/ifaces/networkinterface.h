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

#ifndef SOLID_IFACES_NETWORKINTERFACE_H
#define SOLID_IFACES_NETWORKINTERFACE_H

#include <QList>
#include <solid/solid_export.h>

#include <solid/experimental/networkinterface.h>

#include <QObject>

namespace SolidExperimental
{
namespace Ifaces
{
    /**
     * Represents a network interface as seen by the networking subsystem.
     *
     * For non network specific hardware details,
     * @see SolidExperimental::Ifaces::NetworkHw
     */
    class SOLIDIFACES_EXPORT NetworkInterface : public QObject
    {
        Q_OBJECT
    public:
        /**
         * Create a NetworkInterface.
         *
         * @param parent the parent object
         */
        NetworkInterface( QObject *parent = 0 );

        /**
         * Destructs a NetworkInterface object.
         */
        virtual ~NetworkInterface();

        /**
         * Retrieves the Unique Network Identifier (UNI) of the NetworkInterface.
         * This identifier is unique for each network and network interface in the system.
         *
         * @returns the Unique Network Identifier of the current network interface
         */
        virtual QString uni() const = 0;

        /**
         * Retrieves the activation status of this network interface.
         *
         * @return true if this network interface is active, false otherwise
         */
        virtual bool isActive() const = 0;

        /**
         * Retrieves the type of this network interface. For example it allows to check
         * if a device is wired or wireless.
         *
         * @return this network interface type
         * @see SolidExperimental::NetworkInterface::Type
         */
        virtual SolidExperimental::NetworkInterface::Type type() const = 0;

        /**
         * Retrieves the current state of the network connection held by this device.
         * It's a high level view of the connection. It's user oriented so technically
         * it provides states coming from different layers.
         *
         * @return the current connection state
         * @see SolidExperimental::NetworkInterface::ConnectionState
         */
        virtual SolidExperimental::NetworkInterface::ConnectionState connectionState() const = 0;

        /**
         * Retrieves the current signal strength of this network interface. It ranges from 0 to 100.
         *
         * @return the signal strength as a percentage, for ethernet device it reports 100
         */
        virtual int signalStrength() const = 0;

        /**
         * Retrieves the maximum speed as reported by the device. Note that it's a design
         * related information and that the device might not reach this maximum.
         *
         * @return the device maximum speed
         */
        virtual int designSpeed() const = 0;

        /**
         * Indicates if the network interfaces sees a carrier.
         *
         * @return true if there's a carrier, false otherwise
         */
        virtual bool isLinkUp() const = 0;

        /**
         * Retrieves the capabilities supported by this device.
         *
         * @return the capabilities of the device
         * @see SolidExperimental::NetworkInterface::Capabilities
         */
        virtual SolidExperimental::NetworkInterface::Capabilities capabilities() const = 0;

        /**
         * Instantiates a new Network object from the current backend given its UNI.
         *
         * @param uni the identifier of the network instantiated
         * @returns a new Network object if there's a network having the given UNI for this device, 0 otherwise
         */
        virtual QObject *createNetwork( const QString & uni ) = 0;

        /**
         * Retrieves the networks available via this network interfaces.
         *
         * For wired network interfaces, this will probably be a single network,
         * but with wireless, multiple networks may be accessible.
         *
         * @return a list of network UNIs.
         */
        virtual QStringList networks() const = 0;

    Q_SIGNALS:

        /**
         * This signal is emitted when the device's activation status changed.
         * This may be emitted if the user turns off the network interface via a physical switch.
         *
         * @param activated true if the device is active, false otherwise
         */
        void activeChanged( bool activated );

        /**
         * This signal is emitted when the device's link status changed. For example, if there
         * is no carrier anymore.
         *
         * @param linkActivated true if the carrier got detected, false otherwise
         */
        void linkUpChanged( bool linkActivated );


        /**
         * This signal is emitted when the device's signal strength changed.
         *
         * @param strength the new device signal strength
         */
        void signalStrengthChanged( int strength );

        /**
         * This signal is emitted when the device's connection state changed.
         * For example, if the device was disconnected and started to activate
         *
         * @param state the new state of the connection
         * @see SolidExperimental::NetworkInterface::ConnectionState
         */
        void connectionStateChanged( int state );

        /**
         * This signal is emitted when the device detects a new reachable network.
         *
         * @param uni the new network identifier
         */
        void networkAppeared( const QString & uni );

        /**
         * This signal is emitted when the device decides that a network is not reachable anymore.
         *
         * @param uni the identifier of the network that disappeared
         */
        void networkDisappeared( const QString & uni );
    };
} //Ifaces
} //Solid

Q_DECLARE_INTERFACE( SolidExperimental::Ifaces::NetworkInterface, "org.kde.Solid.Ifaces.NetworkInterface/0.1" )

#endif
