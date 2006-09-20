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

    namespace Ifaces
    {
        class WirelessNetwork;
    }

    class WirelessNetwork : public Network, public Ifaces::Enums::WirelessNetwork
    {
        Q_OBJECT
    public:
        WirelessNetwork( Ifaces::WirelessNetwork * iface);
        virtual ~WirelessNetwork();

        //TODO compare method would look for identical ESSID and at least one AP in common
        bool isSameAs( const WirelessNetwork & ) const;

        // PHY stuff
        int signalStrength() const;

        int bitRate() const;

        int frequency() const;

        Capabilities capabilities() const;

        // Service Set stuff
        QString essid() const;

        OperationMode mode() const;

        bool isAssociated() const;

        bool isEncrypted() const;

        bool isHidden() const;

        bool isActive() const;

        /**
         * List of access points making up the network,
         * or ad hoc network nodes
         */
        MacAddressList bssList() const;

        /**
         * TODO decide how to handle these objects - pass by value with implicit sharing?
         */
        Authentication *authentication() const;

    signals:
        void signalStrengthChanged( int );
        void bitrateChanged( int );
        void associationChanged( bool );
        void activeChanged( bool );

    private:
        class Private;
        Private * d;
};

} //Solid

#endif
