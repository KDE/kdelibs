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

#ifndef SOLID_IFACES_BLOCK_H
#define SOLID_IFACES_BLOCK_H

#include <solid/ifaces/deviceinterface.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This device interface is available on block devices.
     *
     * A block device is an adressable device such as drive or partition.
     * It is possible to interact with such a device using a special file
     * in the system.
     */
    class Block : virtual public DeviceInterface
    {
    public:
        /**
         * Destroys a Block object.
         */
        virtual ~Block();

        /**
         * Retrieves the major number of the node file to interact with
         * the device.
         *
         * @return the device major number
         */
        virtual int deviceMajor() const = 0;

        /**
         * Retrieves the minor number of the node file to interact with
         * the device.
         *
         * @return the device minor number
         */
        virtual int deviceMinor() const = 0;

        /**
         * Retrieves the absolute path of the special file to interact
         * with the device.
         *
         * @return the absolute path of the special file to interact with
         * the device
         */
        virtual QString device() const = 0;
    };
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::Block, "org.kde.Solid.Ifaces.Block/0.1")

#endif
