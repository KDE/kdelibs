/*  This file is part of the KDE project
    Copyright (C) 2006,2007 Will Stephenson <wstephenson@kde.org>

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

#ifndef SOLID_IFACES_WIRELESSNETWORK_H
#define SOLID_IFACES_WIRELESSNETWORK_H

#include <solid/solid_export.h>

#include <QStringList>

#include <solid/experimental/wirelessnetwork.h>
#include <solid/experimental/ifaces/network.h>


typedef QString MacAddress;
typedef QStringList MacAddressList;

namespace SolidExperimental
{
namespace Ifaces
{
    /**
     * This type of networks is used by wifi network interfaces.
     */
    class SOLIDIFACES_EXPORT WirelessNetwork : virtual public Network
    {
    public:
        /**
         * Destroys a WirelessNetwork object
         */
        virtual ~WirelessNetwork();

        /**
         * Retrieves the current signal strength of this wifi network.
         *
         * @return the signal strength as a percentage
         */
        virtual int signalStrength() const = 0;

        /**
         * Retrieves the announced bitrate of this wifi network.
         *
         * @return the bitrate in bit/s
         */
        virtual int bitrate() const = 0;

        /**
         * Retrieves the radio channel frequency that this wifi network is visible on.
         *
         * @return the frequency
         */
        virtual double frequency() const = 0;

        /**
         * Retrieves the capabilities of this wifi network.
         *
         * @return the flag set describing the capabilities
         * @see SolidExperimental::WirelessNetwork::DeviceInterface
         */
        virtual SolidExperimental::WirelessNetwork::Capabilities capabilities() const = 0;




        /**
         * Retrieves the Extended Service Set Identifier (ESSID) of this wifi network.
         *
         * @return the network essid
         */
        virtual QString essid() const = 0;

        /**
         * Retrieves the operation mode of this network.
         *
         * @return the current mode
         * @see SolidExperimental::WirelessNetwork::OperationMode
         */
        virtual SolidExperimental::WirelessNetwork::OperationMode mode() const = 0;

        /**
         * Indicates if the network interface is associated to this network.
         *
         * @return true if the device is associated, false otherwise
         */
        virtual bool isAssociated() const = 0;

        /**
         * Indicates if this network uses an encryption scheme.
         *
         * @return true if the network is encrypted, false otherwise
         */
        virtual bool isEncrypted() const = 0;

        /**
         * Indicates if the network is hidden and doesn't announce its ESSID.
         *
         * @return true if the network is hidden, false otherwise
         */
        virtual bool isHidden() const = 0;

        /**
         * Retrieves the list of access points or ad hoc network nodes making up the
         * network that are currently visible to the device.
         *
         * @return the list of MAC address of the currently visible nodes
         */
        virtual MacAddressList bssList() const = 0;

        /**
         * Retrieves the current authentication scheme used by this network.
         *
         * @return the current authentication object, or 0 if none is used for now
         */
        virtual SolidExperimental::Authentication *authentication() const = 0;

        /**
         * Sets the authentication object to use with this network.
         *
         * @param authentication the new authentication scheme to use
         */
        virtual void setAuthentication( Authentication *authentication ) = 0;

    protected:
    //Q_SIGNALS:

        /**
         * This signal is emitted when the signal strength of this network has changed.
         *
         * @param strength the new signal strength value for this network
         */
        virtual void signalStrengthChanged( int strength ) = 0;

        /**
         * This signal is emitted when the bitrate of this network has changed.
         *
         * @param bitrate the new bitrate value for this network
         */
        virtual void bitrateChanged( int bitrate ) = 0;

        /**
         * This signal is emitted when the association state of this device
         * has changed.
         *
         * @param associated true if the network is associated, false otherwise
         */
        virtual void associationChanged( bool associated ) = 0;

        /**
         * Emitted when the network requires authentication data in order to be able to connect.
         * Respond to this by calling setAuthentication.
         *
         * This signal is emitted when the network requires authentication data in order
         * to be able to connect. Respond to this by calling setAuthentication().
         */
        virtual void authenticationNeeded() = 0;
    };
} //Ifaces
} //Solid

Q_DECLARE_INTERFACE( SolidExperimental::Ifaces::WirelessNetwork, "org.kde.Solid.Ifaces.WirelessNetwork/0.1" )

#endif
