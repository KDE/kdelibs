/*  This file is part of the KDE project
    Copyright (C) 2009 Harald Fernengel <harry@kdevelop.org>

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

#ifndef SOLID_SERIALINTERFACE_H
#define SOLID_SERIALINTERFACE_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
    class SerialInterfacePrivate;
    class Device;

    /**
     * This device interface is available on serial interfaces.
     * @since 4.3
     */
    class SOLID_EXPORT SerialInterface : public DeviceInterface
    {
        Q_OBJECT
        Q_PROPERTY(QVariant driverHandle READ driverHandle)
        Q_PROPERTY(QString serialType READ serialType)
        Q_PROPERTY(int port READ port)
        Q_ENUMS(SerialType)
        Q_DECLARE_PRIVATE(SerialInterface)
        friend class Device;

    private:
        /**
         * Creates a new SerialInterface object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         * @since 4.3
         */
        explicit SerialInterface(QObject *backendObject);

    public:
        /**
         * Destroys a SerialInterface object.
         * @since 4.3
         */
        virtual ~SerialInterface();


        /**
         * Get the Solid::DeviceInterface::Type of the SerialInterface device interface.
         *
         * @return the SerialInterface device interface type
         * @see Solid::Ifaces::Enums::DeviceInterface::Type
         * @since 4.3
         */
        static Type deviceInterfaceType() { return DeviceInterface::SerialInterface; }


        /**
         * Retrieves the name of the interface in the system.
         * This name is system dependent, it allows to identify the interface
         * in the system. For example it can be of the form "/dev/ttyS0" under Linux.
         *
         * @return the interface name
         * @since 4.3
         */
        virtual QVariant driverHandle() const;

        /**
         * This enum type defines the type of a serial interface.
         *
         * - Unknown : The type could not be determined
         * - Platform : A built-in serial port
         * - USB : A USB serial port
         *
         * @since 4.3
         */
        enum SerialType { Unknown = 0, Platform, Usb };

        /**
         * Retrieves the type of the serial device.
         * Examples for Linux are "usb" for USB based serial devices,
         * or "platform" for built-in serial ports.
         *
         * @return the type of the serial device
         * @since 4.3
         */
        virtual SerialType serialType() const;

        /**
         * Retrieves the port number, e.g. 0 for the first COM port.
         *
         * @return The port number of the serial device, or -1 if unknown.
         * @since 4.3
         */
        virtual int port() const;
    };
}

#endif // SOLID_SERIALINTERFACE_H
