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

#ifndef SOLID_NET_NETWORK_H
#define SOLID_NET_NETWORK_H

#include <solid/Net/ifaces/network.h>

#include <QObject>

namespace Solid
{
namespace Net
{

class Network : public QObject
{
Q_OBJECT
    public:
        Network( Ifaces::Network *, QObject * parent );
        virtual ~Network();
        // TODO ask Thiago whether to use QHostAddress or KIPAddress for these 
        virtual QString ipV4Address();
        // virtual QString ipV6Address();
        
        virtual QString subnetMask();
        virtual QString broadcastAddress();
        // wtf does NM use this for?
        virtual QString route();
        
        virtual QString primaryDNS();
        virtual QString secondaryDNS();
        
    signals:
        void ipDetailsChanged();
    private:
        class Private;
        Private * d;
};

} //Net
} //Solid

#endif

