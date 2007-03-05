/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
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

#ifndef SOLID_NETWORK_H
#define SOLID_NETWORK_H

#include <solid/frontendobject.h>

#include <QStringList>
#include <ksocketaddress.h>

namespace Solid
{
    class NetworkInterface;
    class NetworkPrivate;

    /**
     * This interface represents a generic Internet Protocol (IP) network which we may be connected to.
     */
    class SOLID_EXPORT Network : public FrontendObject
    {
        Q_OBJECT
        Q_DECLARE_PRIVATE(Network)

    public:
        /**
         * Creates a new Network object.
         *
         * @param backendObject the network object provided by the backend
         */
        Network( QObject *backendObject = 0 );

        /**
         * Constructs a copy of a network.
         *
         * @param network the network to copy
         */
        Network( const Network &network );

        /**
         * Destroys a Network object.
         */
        virtual ~Network();

        /**
         * Retrieves the Universal Network Identifier (UNI) of the Network.
         * This identifier is unique for each network and network interface in the system.
         *
         * @returns the Universal Network Identifier of the current network
         */
        QString uni() const;



        /**
         * Retrieves the IP version 4 addresses the device has on this network.
         *
         * @return the list of IP version 4 addresses
         */
        QList<KNetwork::KIpAddress> ipV4Addresses() const;

        /**
         * Retrieves the IP version 6 addresses the device has on this network.
         *
         * @return the list of IP version 6 addresses
         */
        QList<KNetwork::KIpAddress> ipV6Addresses() const;
        // TODO check with ipv6 guy about this
        /**
         * Retrieves the IP version 4 subnetwork mask of this network.
         *
         * @return the subnetwork mask
         */
        QString subnetMask() const;
        /**
         * Retrieves the IP version 4 broadcast address of this network.
         *
         * @return the broadcast address
         */
        QString broadcastAddress() const;

        /**
         * Retrieves the route we must follow when using this network. It's
         * in particular used for VPN.
         *
         * @return the route address is available, QString() otherwise
         */
        QString route() const;

        /**
         * Retrieves the list of DNS servers to use on this network.
         *
         * @return the dns servers
         */
        QList<KNetwork::KIpAddress> dnsServers() const;

        /**
         * Retrieves the activation status of this network. For ethernets, this will always be true.
         *
         * @return true if this network is active, false otherwise
         */
        bool isActive() const;

        /**
         * Activates or deactivates this network. For ethernets, this has no effect.
         *
         * @param activated true to activate this network, false otherwise
         */
        void setActivated( bool activated );

    Q_SIGNALS:
        /**
         * This signal is emitted when the settings of this network have changed.
         */
        void ipDetailsChanged();

        /**
         * This signal is emitted when the activation state of this network
         * has changed.
         *
         * @param activated true if the network is activated, false otherwise
         */
        void activationStateChanged( bool activated );

    protected:
        void registerBackendObject( QObject * );

        /**
         * @internal
         */
        Network(NetworkPrivate &dd, QObject *backendObject);

        /**
         * @internal
         */
        Network(NetworkPrivate &dd, const Network &network);

    private:
        friend class NetworkInterface;
    //HACK: to make NetworkList polymorphic (containing both wired and wireless networks, I used Network * here - Will.
    };
    typedef QList<Network*> NetworkList;


} //Solid

#endif

