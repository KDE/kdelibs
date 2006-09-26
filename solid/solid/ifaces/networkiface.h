/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_IFACES_NETWORKIFACE_H
#define SOLID_IFACES_NETWORKIFACE_H

#include <kdelibs_export.h>

#include <solid/ifaces/capability.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This capability is available on network interfaces.
     */
    class KDE_EXPORT NetworkIface : virtual public Capability
    {
//         Q_PROPERTY( QString ifaceName READ ifaceName )
//         Q_PROPERTY( bool wireless READ isWireless )
//         Q_PROPERTY( QString hwAddress READ hwAddress )
//         Q_PROPERTY( qulonglong macAddress READ macAddress )

    public:
        /**
         * Destroys a NetworkIface object.
         */
        virtual ~NetworkIface();


        /**
         * Retrieves the name of the interface in the system.
         * This name is system dependent, it allows to identify the interface
         * in the system. For example it can be of the form "eth0" under Linux.
         *
         * @return the interface name
         */
        virtual QString ifaceName() const = 0;

        /**
         * Indicates if this interface is wireless.
         *
         * @return true if the interface is wireless, false otherwise
         */
        virtual bool isWireless() const = 0;


        /**
         * Retrieves the hardware address of the interface.
         *
         * @return the hardware address as a string
         */
        virtual QString hwAddress() const = 0;

        /**
         * Retrieves the MAC address of the interface.
         *
         * @return the MAC address
         */
        virtual qulonglong macAddress() const = 0;
    };
}
}

Q_DECLARE_INTERFACE( Solid::Ifaces::NetworkIface, "org.kde.Solid.Ifaces.NetworkIface/0.1" )

#endif
