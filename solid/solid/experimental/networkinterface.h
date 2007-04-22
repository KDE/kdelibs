/*  This file is part of the KDE project
    Copyright (C) 2006,2007 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_NETWORKINTERFACE_H
#define SOLID_NETWORKINTERFACE_H

#include <QtCore/QObject>
#include <solid/experimental/network.h>

namespace SolidExperimental
{
    class NetworkInterfacePrivate;

    /**
     * Represents a network interface as seen by the networking subsystem.
     *
     * For non-networking specific hardware details,
     * @see SolidExperimental::NetworkHw
     */
    class SOLID_EXPORT NetworkInterface : public QObject
    {
        Q_OBJECT
        Q_ENUMS( ConnectionState Capability Type )
        Q_FLAGS( Capabilities )

    public:
        // == NM ActivationStage
        /**
         * Device connection states describe the possible states of a
         * network connection from the user's point of view.  For
         * simplicity, states from several different layers are present -
         * this is a high level view
         */
        enum ConnectionState{ UnknownState, Prepare, Configure, NeedUserKey,
                              IPStart, IPGet, IPCommit, Activated, Failed,
                              Cancelled };
        /**
         * Possible Device capabilities
         * - IsManageable: denotes that the device can be controlled by this API
         * - SupportsCarrierDetect: the device informs us when it is plugged in to the medium
         * - SupportsWirelessScan: the device can scan for wireless networks
         */
        enum Capability { IsManageable = 0x1, SupportsCarrierDetect = 0x2,
                          SupportsWirelessScan = 0x4 };
        /**
         * Device medium types
         * - Ieee8023: wired ethernet
         * - Ieee80211: the popular family of wireless networks
         */
        enum Type { UnknownType, Ieee8023, Ieee80211 };

        Q_DECLARE_FLAGS( Capabilities, Capability )



        /**
         * Constructs an invalid network interface
         */
        NetworkInterface();

        /**
         * Constructs a network interface for a given Unique Network Identifier (UNI).
         *
         * @param uni the uni of the network interface to create
         */
        explicit NetworkInterface( const QString &uni );

        /**
         * Constructs a new network interface taking its data from a backend.
         *
         * @param backendObject the object given by the backend
         */
        explicit NetworkInterface( QObject *backendObject );

        /**
         * Constructs a copy of a network interface.
         *
         * @param device the network interface to copy
         */
        NetworkInterface( const NetworkInterface &device );

        /**
         * Destroys the device.
         */
        ~NetworkInterface();

        /**
         * Assigns a network interface to this network interface and returns a reference to it.
         *
         * @param device the network interface to assign
         * @return a reference to the network interface
         */
        NetworkInterface &operator=( const NetworkInterface &device );

        /**
         * Indicates if this network interface is valid.
         * A network interface is considered valid if it's available in the system.
         *
         * @return true if this network interface is available, false otherwise
         */
        bool isValid() const;

        /**
         * Retrieves the Unique Network Identifier (UNI) of the NetworkInterface.
         * This identifier is unique for each network and network interface in the system.
         *
         * @returns the Unique Network Identifier of the current network interface
         */
        QString uni() const;

        /**
         * Retrieves the activation status of this network interface.
         *
         * @return true if this network interface is active, false otherwise
         */
        bool isActive() const;

        /**
         * Retrieves the type of this network interface. For example it allows to check
         * if a device is wired or wireless.
         *
         * @return this network interface type
         * @see SolidExperimental::NetworkInterface::Type
         */
        Type type() const;

        /**
         * Retrieves the current state of the network connection held by this device.
         * It's a high level view of the connection. It is user oriented, so
         * actually it provides state coming from different layers.
         *
         * @return the current connection state
         * @see SolidExperimental::NetworkInterface::ConnectionState
         */
        ConnectionState connectionState() const;

        /**
         * Retrieves the current signal strength of this network interface.
         * It ranges from 0 to 100.
         *
         * @return the signal strength as a percentage, for ethernet devices it always reports 100.
         */
        int signalStrength() const;

        /**
         * Retrieves the maximum speed as reported by the device.
         * Note that this is only a design related piece of information, and that
         * the device might not reach this maximum.
         *
         * @return the device's maximum speed
         */
        int designSpeed() const;

        /**
         * Indicates if the network interface sees a carrier, e.g. that an
         * ethernet device is connected, or a wireless device is associated.
         *
         * @return true if there's a carrier, false otherwise
         */
        bool isLinkUp() const;

        /**
         * Retrieves the capabilities supported by this device.
         *
         * @return the capabilities of the device
         */
        Capabilities capabilities() const;

        /**
         * Finds Network object given its Unique Network Identifier.
         *
         * @param uni the identifier of the network to find from this network interface
         * @returns a valid Network object if a network having the given UNI for this device is known to the system, an invalid Network object otherwise
         */
        Network *findNetwork( const QString & uni ) const;

        /**
         * Retrieves the networks available via this network interfaces.
         *
         * For wired network interfaces, this will probably be a single network,
         * but with wireless, multiple networks may be accessible.
         *
         * @return a list of network UNIs.
         */
        NetworkList networks() const;

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
         * This signal is emitted when the device's link status changed. For example, if there
         * is no carrier anymore.
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


    private:
        Q_PRIVATE_SLOT(d, void _k_destroyed(QObject*))

        NetworkInterfacePrivate * const d;
    };

} //Solid

Q_DECLARE_OPERATORS_FOR_FLAGS( SolidExperimental::NetworkInterface::Capabilities )

#endif
