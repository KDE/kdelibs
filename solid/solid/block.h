/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_BLOCK_H
#define SOLID_BLOCK_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
    class BlockPrivate;
    class Device;

    /**
     * This device interface is available on block devices.
     *
     * A block device is an adressable device such as drive or partition.
     * It is possible to interact with such a device using a special file
     * in the system.
     */
    class SOLID_EXPORT Block : public DeviceInterface
    {
        Q_OBJECT
        Q_PROPERTY(int major READ deviceMajor)
        Q_PROPERTY(int minor READ deviceMinor)
        Q_PROPERTY(QString device READ device)
        Q_DECLARE_PRIVATE(Block)
        friend class Device;

    private:
        /**
         * Creates a new Block object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         */
        explicit Block(QObject *backendObject);

    public:
        /**
         * Destroys a Block object.
         */
        virtual ~Block();


        /**
         * Get the Solid::DeviceInterface::Type of the Block device interface.
         *
         * @return the Block device interface type
         * @see Solid::Ifaces::Enums::DeviceInterface::Type
         */
        static Type deviceInterfaceType() { return DeviceInterface::Block; }


        /**
         * Retrieves the major number of the node file to interact with
         * the device.
         *
         * @return the device major number
         */
        int deviceMajor() const;

        /**
         * Retrieves the minor number of the node file to interact with
         * the device.
         *
         * @return the device minor number
         */
        int deviceMinor() const;

        /**
         * Retrieves the absolute path of the special file to interact
         * with the device.
         *
         * @return the absolute path of the special file to interact with
         * the device
         */
        QString device() const;
    };
}

#endif
