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

#ifndef SOLID_NET_IFACES_WIRELESSNETWORK_H
#define SOLID_NET_IFACES_WIRELESSNETWORK_H

#include <kdelibs_export.h>

#include <QStringList>

#include <solid/Net/ifaces/enums.h>

#include "authentication.h"
#include "network.h"


typedef QString MacAddress;
typedef QStringList MacAddressList;

namespace Solid
{
namespace Net
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
            virtual ~WirelessNetwork();
            
            //TODO compare method would look for identical ESSID and at least one AP in common
            virtual bool isSameAs( const WirelessNetwork & ) const = 0;
            
            // PHY stuff
            virtual int signalStrength() = 0;
            
            virtual int bitRate() = 0;
            
            virtual int frequency() = 0;
            
            virtual Enums::WirelessNetwork::Capabilities capabilities() = 0;
            
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
            * TODO decide how to handle these objects - pass by value
            */
            virtual Authentication * authentication() = 0;
            
            // TODO SIGNALS!!!
            //signals:
        protected:
            virtual void signalStrengthChanged( int ) = 0;
            virtual void bitrateChanged( int ) = 0;
            virtual void associationChanged( bool ) = 0;
            virtual void activeChanged( bool ) = 0;
    };
} //Ifaces
} //Net
} //Solid

#endif
