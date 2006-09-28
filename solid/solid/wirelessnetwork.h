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

#ifndef SOLID_WIRELESSNETWORK_H
#define SOLID_WIRELESSNETWORK_H

#include <solid/network.h>
#include <solid/authentication.h>
#include <solid/ifaces/enums.h>


namespace Solid
{
    typedef QString MacAddress;
    typedef QStringList MacAddressList;

    /**
     * This type of networks is used by wifi network interfaces.
     */
    class KDE_EXPORT WirelessNetwork : public Network, public Ifaces::Enums::WirelessNetwork
    {
        Q_OBJECT
    public:
        /**
         * Creates a new WirelessNetwork object.
         *
         * @param backendObject the network object provided by the backend
         */
        WirelessNetwork( QObject *backendObject );

        /**
         * Destroys a WirelessNetwork object
         */
        virtual ~WirelessNetwork();

        /**
         * Indicates if two WirelessNetwork objects describe in fact the same network.
         *
         * TODO: compare method would look for identical ESSID and at least one AP in common
         *
         * @param other the network to compare
         * @return true if other and this wireless network are the same network, false otherwise
         */
        bool isSameAs( const WirelessNetwork &other ) const;

        /**
         * Retrieves the current signal strength of this wifi network.
         *
         * @return the signal strength as a percentage
         */
        int signalStrength() const;

        /**
         * Retrieves the announced bitrate of this wifi network.
         *
         * @return the bitrate in bit/s
         */
        int bitRate() const;

        /**
         * Retrieves the frequency of this wifi network.
         *
         * @return the frequency
         */
        int frequency() const;

        /**
         * Retrieves the capabilities of this wifi network.
         *
         * @return the flag set describing the capabilities
         * @see Solid::Ifaces::Enums::WirelessNetwork::Capability
         */
        Capabilities capabilities() const;



        /**
         * Retrieves the Extended Service Set Identifier (ESSID) of this wifi network.
         *
         * @return the network essid
         */
        QString essid() const;

        /**
         * Retrieves the operation mode of this network.
         *
         * @return the current mode
         * @see Solid::Ifaces::Enums::WirelessNetwork::OperationMode
         */
        OperationMode mode() const;

        /**
         * Indicates if the network interface is associated to this network.
         *
         * @return true if the device is associated, false otherwise
         */
        bool isAssociated() const;

        /**
         * Indicates if this network uses an encryption scheme.
         *
         * @return true if the network is encrypted, false otherwise
         */
        bool isEncrypted() const;

        /**
         * Indicates if the network is hidden and doesn't announce its ESSID.
         *
         * @return true if the network is hidden, false otherwise
         */
        bool isHidden() const;

        /**
         * Retrieves the list of access points or ad hoc network nodes making up the
         * network that are currently visible to the device.
         *
         * @return the list of MAC address of the currently visible nodes
         */
        MacAddressList bssList() const;

        /**
         * Retrieves the current authentication scheme used by this network.
         *
         * TODO decide how to handle these objects - pass by value?
         *
         * @return the current authentication object, or 0 if none is used for now
         */
        Authentication *authentication() const;

        /**
         * Sets the authentication object to with this network.
         *
         * @param authentication the new authentication scheme to use
         */
        void setAuthentication( Authentication *authentication );

    Q_SIGNALS:
        /**
         * This signal is emitted when the signal strength of this network has changed.
         *
         * @param strength the new signal strength value for this network
         */
        void signalStrengthChanged( int strength );

        /**
         * This signal is emitted when the bitrate of this network has changed.
         *
         * @param bitrate the new bitrate value for this network
         */
        void bitrateChanged( int bitrate );

        /**
         * This signal is emitted when the association state of this device
         * has changed.
         *
         * @param associated true if the network is associated, false otherwise
         */
        void associationChanged( bool associated );

        /**
         * Emitted when the network requires authentication data in order to be able to connect.
         * Respond to this by calling setAuthentication.
         *
         * This signal is emitted when the network requires authentication data in order
         * to be able to connect. Respond to this by calling setAuthentication().
         */
        void authenticationNeeded();
};

} //Solid

#endif
