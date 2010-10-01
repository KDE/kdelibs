/*
    Copyright 2009 Harald Fernengel <harry@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SOLID_IFACE_SERIALINTERFACE_H
#define SOLID_IFACE_SERIALINTERFACE_H

#include <solid/ifaces/deviceinterface.h>
#include <solid/serialinterface.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This device interface is available on serial interfaces,
     * like modems.
     * @since 4.3
     */
    class SerialInterface : virtual public DeviceInterface
    {
    public:
        /**
         * Destroys a SerialInterface object.
         * @since 4.3
         */
        virtual ~SerialInterface();


        /**
         * Retrieves the name of the interface in the system.
         * This name is system dependent, it allows to identify the interface
         * in the system. For example it can be of the form "/dev/ttyS0" under Linux.
         *
         * @return the interface name
         * @since 4.3
         */
        virtual QVariant driverHandle() const = 0;

        /**
         * Retrieves the type of the serial device.
         * Examples for Linux are "usb" for USB based serial devices,
         * or "platform" for built-in serial ports.
         *
         * @return the type of the serial device
         * @since 4.3
         */
        virtual Solid::SerialInterface::SerialType serialType() const = 0;

        /**
         * Retrieves the port number, e.g. 0 for the first COM port.
         *
         * @return The port number of the serial device, or -1 if unknown.
         * @since 4.3
         */
        virtual int port() const = 0;
    };
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::SerialInterface, "org.kde.Solid.Ifaces.SerialInterface/0.1")

#endif // SOLID_IFACE_SERIALINTERFACE_H
