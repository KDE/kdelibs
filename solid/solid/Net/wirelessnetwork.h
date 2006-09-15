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

#ifndef SOLID_NET_WIRELESSNETWORK_H
#define SOLID_NET_WIRELESSNETWORK_H

#include <solid/Net/ifaces/network.h>
#include <solid/Net/ifaces/wirelessnetwork.h>

#include "network.h"

namespace Solid
{
namespace Net
{

class WirelessNetwork : public Network
{
Q_OBJECT
    public:
        WirelessNetwork( Ifaces::Network *, Ifaces::WirelessNetwork *, QObject * );
        virtual ~WirelessNetwork();
        
        //TODO compare method would look for identical ESSID and at least one AP in common
        virtual bool isSameAs( const WirelessNetwork & ) const;
        
        // PHY stuff
        virtual int signalStrength() const;
        
        virtual int bitRate() const;
        
        virtual int frequency() const;
        
        virtual Ifaces::Enums::WirelessNetwork::Capabilities capabilities() const;
        
        // Service Set stuff
        virtual QString essid() const;
        
        virtual Ifaces::Enums::WirelessNetwork::OperationMode mode() const;
        
        virtual bool isAssociated() const;
        
        virtual bool isEncrypted() const;
        
        virtual bool isHidden() const;
        
        virtual bool isActive() const;
        
        /**
         * List of access points making up the network,
         * or ad hoc network nodes
         */
        virtual MacAddressList bssList() const;
        
        /**
         * TODO decide how to handle these objects - pass by value with implicit sharing?
         */
        virtual Solid::Net::Ifaces::Authentication * authentication() const;
        
    signals:
        virtual void signalStrengthChanged( int );
        virtual void bitrateChanged( int );
        virtual void associationChanged( bool );
        virtual void activeChanged( bool );
    private:
        class Private;
        Private * d;
};

} //Net
} //Solid

#endif
