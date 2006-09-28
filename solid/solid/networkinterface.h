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

#ifndef SOLID_NETWORKINTERFACE_H
#define SOLID_NETWORKINTERFACE_H

#include <solid/frontendobject.h>
#include <solid/ifaces/enums.h>
#include <solid/network.h>

namespace Solid
{
    /**
     * Represents a network device as seen by the networking subsystem.
     *
     * For non network specific hardware details,
     * @see Solid::NetworkIface
     */
    class KDE_EXPORT NetworkInterface : public FrontendObject, public Ifaces::Enums::NetworkInterface
    {
        Q_OBJECT
    public:
        /**
         * Constructs an invalid network device
         */
        NetworkInterface();

        /**
         * Constructs a network device for a given Universal Network Identifier (UNI).
         *
         * @param uni the uni of the network device to create
         */
        NetworkInterface( const QString &uni );

        /**
         * Constructs a new network device taking its data from a backend.
         *
         * @param backendObject the object given by the backend
         */
        NetworkInterface( QObject *backendObject );

        /**
         * Constructs a copy of a network device.
         *
         * @param device the network device to copy
         */
        NetworkInterface( const NetworkInterface &device );

        /**
         * Destroys the device.
         */
        ~NetworkInterface();



        /**
         * Assigns a network device to this network device and returns a reference to it.
         *
         * @param device the network device to assign
         * @return a reference to the network device
         */
        NetworkInterface &operator=( const NetworkInterface &device );



        /**
         * Retrieves the Universal Network Identifier (UNI) of the NetworkInterface.
         * This identifier is unique for each network and network device in the system.
         *
         * @returns the Universal Network Identifier of the current network device
         */
        QString uni() const;

        /**
         * Retrieves the activation status of this network device.
         *
         * @return true if this network device is active, false otherwise
         */
        bool isActive() const;

        /**
         * Retrieves the type of this network device. For example it allows to check
         * if a device is wired or wireless.
         *
         * @return this network device type
         * @see Solid::Ifaces::Enums::NetworkInterface::Type
         */
        Type type() const;

        /**
         * Retrieves the current state of the network connection held by this device.
         * It's a high level view of the connection. It's user oriented so technically
         * it provides states coming from different layers.
         *
         * @return the current connection state
         * @see Solid::Ifaces::Enums::NetworkInterface::ConnectionState
         */
        ConnectionState connectionState() const;

        /**
         * Retrieves the current signal strength of this network device. It ranges from 0 to 100.
         *
         * @return the signal strength as a percentage, for ethernet device it reports 100
         */
        int signalStrength() const;

        /**
         * Retrieves the maximum speed as reported by the device. Note that it's a design
         * related information and that the device might not reach this maximum.
         *
         * @return the device maximum speed
         */
        int designSpeed() const;

        /**
         * Indicates if the network devices sees a carrier.
         *
         * @return true if there's a carrier, false otherwise
         */
        bool isLinkUp() const;

        /**
         * Retrieves the capabilities supported by this device.
         *
         * @return the capabilities of the device
         * @param Solid::Ifaces::Enums::NetworkInterface
         */
        Capabilities capabilities() const;

        /**
         * Finds Network object given its UNI.
         *
         * @param uni the identifier of the network to find from this network device
         * @returns a Network object if there's a network having the given UNI for this device, 0 otherwise
         */
        Network *findNetwork( const QString & uni ) const;

        /**
         * Retrieves the networks available via this network devices.
         *
         * For wired network devices, this will probably be a single network,
         * but with wireless, multiple networks may be accessible.
         *
         * @return a list of network UNIs.
         */
        NetworkList networks() const;

    Q_SIGNALS:
        /**
         * This signal is emitted when the device's activation status changed.
         * This may be emitted if the user turns off the network device via a physical switch.
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
         * @see Solid::Ifaces::NetworkInterface::ConnectionState
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


    protected Q_SLOTS:
        /**
         * @internal
         * Notifies when the backend object disappears.
         *
         * @param object the backend object destroyed
         */
        void slotDestroyed( QObject *object );

    private:
        void registerBackendObject( QObject *backendObject );
        void unregisterBackendObject();

        Network *findRegisteredNetwork( const QString &uni ) const;

        class Private;
        Private * d;
};

} //Solid

#endif
