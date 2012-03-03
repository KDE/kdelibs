/*
    Copyright 2006-2007 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_NETWORKINTERFACE_H
#define SOLID_NETWORKINTERFACE_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
    class NetworkInterfacePrivate;
    class Device;

    /**
     * This device interface is available on network interfaces.
     */
    class SOLID_EXPORT NetworkInterface : public DeviceInterface
    {
        Q_OBJECT
        Q_PROPERTY(QString ifaceName READ ifaceName)
        Q_PROPERTY(bool wireless READ isWireless)
        Q_PROPERTY(QString hwAddress READ hwAddress)
        Q_PROPERTY(qulonglong macAddress READ macAddress)
        Q_DECLARE_PRIVATE(NetworkInterface)
        friend class Device;

    private:
        /**
         * Creates a new NetworkInterface object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         */
        explicit NetworkInterface(QObject *backendObject);

    public:
        /**
         * Destroys a NetworkInterface object.
         */
        virtual ~NetworkInterface();


        /**
         * Get the Solid::DeviceInterface::Type of the NetworkInterface device interface.
         *
         * @return the NetworkInterface device interface type
         * @see Solid::Ifaces::Enums::DeviceInterface::Type
         */
        static Type deviceInterfaceType() { return DeviceInterface::NetworkInterface; }


        /**
         * Retrieves the name of the interface in the system.
         * This name is system dependent, it allows to identify the interface
         * in the system. For example it can be of the form "eth0" under Linux.
         *
         * @return the interface name
         */
        QString ifaceName() const;

        /**
         * Indicates if this interface is wireless.
         *
         * @return true if the interface is wireless, false otherwise
         */
        bool isWireless() const;


        /**
         * Retrieves the hardware address of the interface.
         *
         * @return the hardware address as a string
         */
        QString hwAddress() const;

        /**
         * Retrieves the MAC address of the interface.
         *
         * @return the MAC address
         */
        qulonglong macAddress() const;
    };
}

#endif // SOLID_NETWORKINTERFACE_H
