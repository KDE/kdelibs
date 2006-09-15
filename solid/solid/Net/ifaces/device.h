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

#ifndef SOLID_NET_IFACES_DEVICE_H
#define SOLID_NET_IFACES_DEVICE_H

#include <kdelibs_export.h>

#include <solid/Net/ifaces/enums.h>

#include <QObject>

namespace Solid
{
namespace Net
{
namespace Ifaces
{
    /**
     * Represents a network device as seen by the networking subsystem.
     * For non network specific hardware details, 
     * @see Solid::Ifaces::NetworkIface
     */
     // TODO talk to Ervin about how to cleanly combine this with NetworkIface, perhaps a union class elsewhere
    class KDE_EXPORT Device : public QObject, public Enums::Device
    {
        Q_OBJECT
        public:
            virtual ~Device();
    
            virtual bool isActive() = 0;
    
            virtual Type type() = 0;
            
            virtual ConnectionState connectionState() = 0;
            
            virtual int signalStrength() = 0;
            
            virtual int speed() = 0;
            
            virtual bool isLinkUp() = 0;
            
            virtual Capabilities capabilities() = 0;
            
            // signals:
        signals:
            virtual void activeChanged( bool ) = 0;
            virtual void linkUpChanged( bool ) = 0;
            virtual void signalStrengthChanged( int ) = 0;
            virtual void connectionStateChanged( ConnectionState ) = 0;
    };
} //Ifaces
} // Net
} //Solid

#endif
