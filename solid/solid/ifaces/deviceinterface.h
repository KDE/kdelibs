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

#ifndef SOLID_IFACES_DEVICEINTERFACE_H
#define SOLID_IFACES_DEVICEINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QBool>

namespace Solid
{
namespace Ifaces
{
    /**
     * Base interface of all the device interfaces.
     *
     * A device interface describes what a device can do. A device generally has
     * a set of device interfaces.
     *
     * @see Solid::Ifaces::AbstractDeviceInterface
     */
    class DeviceInterface
    {
    public:
        /**
         * Destroys a DeviceInterface object.
         */
        virtual ~DeviceInterface();
    };
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::DeviceInterface, "org.kde.Solid.Ifaces.DeviceInterface/0.1")

#endif
