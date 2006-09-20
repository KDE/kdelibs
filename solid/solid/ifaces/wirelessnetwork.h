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

#ifndef SOLID_IFACES_WIRELESSNETWORK_H
#define SOLID_IFACES_WIRELESSNETWORK_H

#include <kdelibs_export.h>

#include <QStringList>

#include <solid/ifaces/enums.h>

#include <solid/ifaces/authentication.h>
#include <solid/ifaces/network.h>


typedef QString MacAddress;
typedef QStringList MacAddressList;

namespace Solid
{
namespace Ifaces
{
    /**
     * A Wifi wireless network
     */
    class KDE_EXPORT WirelessNetwork : public Network, public Enums::WirelessNetwork
    {
        Q_OBJECT
    public:
        WirelessNetwork( const QString & uni, QObject *parent = 0 );
        virtual ~WirelessNetwork();

        //TODO compare method would look for identical ESSID and at least one AP in common
        virtual bool isSameAs( const WirelessNetwork & ) const = 0;

        // PHY stuff
        virtual int signalStrength() = 0;

        virtual int bitRate() = 0;

        virtual int frequency() = 0;

        virtual Capabilities capabilities() = 0;

        // Service Set stuff
        virtual QString essid() = 0;

        virtual OperationMode mode() = 0;

        virtual bool isAssociated() = 0;

        virtual bool isEncrypted() = 0;

        virtual bool isHidden() = 0;

        virtual bool isActive() = 0;

        /**
         * List of access points making up the network,
         * or ad hoc network nodes
         */
        virtual MacAddressList bssList() = 0;

        /**
         * TODO decide how to handle these objects - pass by value?
         */
        virtual Authentication * authentication() = 0;
        /**
         * set the authentication currently in use on this network
         */
        virtual void setAuthentication( Authentication * ) = 0;
    signals:
        void signalStrengthChanged( int );
        void bitrateChanged( int );
        void associationChanged( bool );
        void activeChanged( bool );
        /**
         * Emitted when the network requires authentication data in order to be able to connect.
         * Respond to this by calling setAuthentication.
         */
        void authenticationNeeded();
    };
} //Ifaces
} //Solid

#endif
