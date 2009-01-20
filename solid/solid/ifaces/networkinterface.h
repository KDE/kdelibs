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

#ifndef SOLID_IFACE_NETWORKINTERFACE_H
#define SOLID_IFACE_NETWORKINTERFACE_H

#include <solid/ifaces/deviceinterface.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This device interface is available on network interfaces.
     */
    class NetworkInterface : virtual public DeviceInterface
    {
    public:
        /**
         * Destroys a NetworkInterface object.
         */
        virtual ~NetworkInterface();


        /* TODO for KDE 5 - rename ifaceName() to interfaceName() */
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

Q_DECLARE_INTERFACE(Solid::Ifaces::NetworkInterface, "org.kde.Solid.Ifaces.NetworkInterface/0.1")

#endif // SOLID_IFACE_NETWORKINTERFACE_H
