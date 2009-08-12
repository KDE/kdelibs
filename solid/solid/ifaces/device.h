/*  This file is part of the KDE project
    Copyright (C) 2005 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_IFACES_DEVICE_H
#define SOLID_IFACES_DEVICE_H

#include <QtCore/QObject>
#include <QtCore/QVariant>

#include <QtCore/QMap>

#include <solid/deviceinterface.h>
#include <solid/device.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This class specifies the interface a device will have to comply to in order to be used in the system.
     *
     * Backends will have to implement it to gather and modify data in the underlying system.
     * Each device has a set of key/values pair describing its properties. It has also a list of interfaces
     * describing what the device actually is (a cdrom drive, a portable media player, etc.)
     *
     * @author Kevin Ottens <ervin@kde.org>
     */
    class Device : public QObject
    {
        Q_OBJECT

    public:
        /**
         * Constructs a Device
         */
        Device(QObject *parent = 0);
        /**
         * Destruct the Device object
         */
        virtual ~Device();


        /**
         * Retrieves the Universal Device Identifier (UDI) of the Device.
         * This identifier is unique for each device in the system.
         *
         * @returns the Universal Device Identifier of the current device
         */
        virtual QString udi() const = 0;

        /**
         * Retrieves the Universal Device Identifier (UDI) of the Device's
         * parent.
         *
         * @returns the Universal Device Identifier of the parent device
         */
        virtual QString parentUdi() const;


        /**
         * Retrieves the name of the device vendor.
         *
         * @return the vendor name
         */
        virtual QString vendor() const = 0;

        /**
         * Retrieves the name of the product corresponding to this device.
         *
         * @return the product name
         */
        virtual QString product() const = 0;

        /**
         * Retrieves the name of the icon representing this device.
         * The naming follows the freedesktop.org specification.
         *
         * @return the icon name
         */
        virtual QString icon() const = 0;

        /**
         * Retrieves the name of the emblems representing the state of this device.
         * The naming follows the freedesktop.org specification.
         *
         * @return the emblem names
         */
        virtual QStringList emblems() const = 0;

       /**
         * Retrieves the description of device.
         *
         * @return the description
         */
        virtual QString description() const = 0;

        /**
         * Tests if a property exist.
         *
         * @param type the device interface type
         * @returns true if the device interface is provided by this device, false otherwise
         */
        virtual bool queryDeviceInterface(const Solid::DeviceInterface::Type &type) const = 0;

        /**
         * Create a specialized interface to interact with the device corresponding to
         * a particular device interface.
         *
         * @param type the device interface type
         * @returns a pointer to the device interface if supported by the device, 0 otherwise
         */
        virtual QObject *createDeviceInterface(const Solid::DeviceInterface::Type &type) = 0;
    };
}
}

#endif
