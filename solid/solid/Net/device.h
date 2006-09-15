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

#ifndef SOLID_NET_DEVICE_H
#define SOLID_NET_DEVICE_H

#include <solid/Net/ifaces/device.h>

#include <QObject>

namespace Solid
{
namespace Net
{

class Device : public QObject
{
Q_OBJECT
    public:
        Device( Ifaces::Device *, QObject * parent );
        virtual ~Device();

        virtual bool isActive();

        virtual Ifaces::Enums::Device::Type type();
        
        virtual Ifaces::Enums::Device::ConnectionState connectionState();
        
        virtual int signalStrength();
        
        virtual int speed();
        
        virtual bool isLinkUp();
        
        virtual Ifaces::Device::Capabilities capabilities();
    signals:
        virtual void activeChanged( bool );
        virtual void linkUpChanged( bool );
        virtual void signalStrengthChanged( int );
        virtual void connectionStateChanged( Ifaces::Enums::Device::ConnectionState );
    private:
        class Private;
        Private * d;
};

} //Net
} //Solid

#endif
