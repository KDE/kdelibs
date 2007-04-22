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

#ifndef SOLID_IFACES_NETWORK_H
#define SOLID_IFACES_NETWORK_H

#include <solid/solid_export.h>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtNetwork/QNetworkAddressEntry>

namespace SolidExperimental
{
namespace Ifaces
{
    /**
     * This interface represents a generic Internet Protocol (IP) network which we may be connected to.
     */
    class SOLIDIFACES_EXPORT Network
    {
    public:
        /**
         * Destroys a Network object.
         */
        virtual ~Network();

        /**
         * Retrieves the Unique Network Identifier (UNI) of the Network.
         * This identifier is unique for each network and network interface in the system.
         *
         * @returns the Unique Network Identifier of the current network
         */
        virtual QString uni() const = 0;

        /**
         * Retrieves the addresses the device has on this network.
         *
         * @return the list of addresses
         */
        virtual QList<QNetworkAddressEntry> addressEntries() const = 0;

        /**
         * Retrieves the route we must follow when using this network. It's
         * in particular used for VPN.
         *
         * @return the route address is available, QString() otherwise
         */
        virtual QString route() const = 0;

        /**
         * Retrieves the list of DNS servers to use on this network.
         *
         * @return the dns servers
         */
        virtual QList<QHostAddress> dnsServers() const = 0;

        /**
         * Retrieves the activation status of this network. For ethernets, this will always be true.
         *
         * @return true if this network is active, false otherwise
         */
        virtual bool isActive() const = 0;

        /**
         * Activates or deactivates this network. For ethernets, this has no effect.
         *
         * @param activated true to activate this network, false otherwise
         */
        virtual void setActivated( bool activated ) = 0;

    protected:
    //Q_SIGNALS:
        /**
         * This signal is emitted when the settings of this network have changed.
         */
        virtual void ipDetailsChanged() = 0;

        /**
         * This signal is emitted when the activation state of this network
         * has changed.
         *
         * @param activated true if the network is activated, false otherwise
         */
        virtual void activationStateChanged( bool activated ) = 0;
    };
} //Ifaces
} //Solid

Q_DECLARE_INTERFACE( SolidExperimental::Ifaces::Network, "org.kde.Solid.Ifaces.Network/0.1" )

#endif
