/*
    Copyright 2013 Ivan Cukic <ivan.cukic@kde.org>

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

#ifndef SOLID_IFACES_POINTING_DEVICE_H
#define SOLID_IFACES_POINTING_DEVICE_H

#include <solid/ifaces/deviceinterface.h>
#include <solid/pointingdevice.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This device interface is available on pointing devices
     *
     * PointingDevice are input devices such as mice, touchpads,
     * touch screens and tablets (graphics tablets).
     *
     * @author Ivan Cukic <ivan.cukic@kde.org>
     */
    class PointingDevice : virtual public DeviceInterface
    {
    public:
        /**
         * Destroys a PointingDevice object.
         */
        virtual ~PointingDevice();

        /**
         * Retrieves the type of the pointing device.
         *
         * @return the type of the pointing device.
         * @see Solid::PointingDevice::PointingDeviceType
         */
        virtual Solid::PointingDevice::PointingDeviceType type() const = 0;
    };
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::PointingDevice, "org.kde.Solid.Ifaces.PointingDevice/0.1")

#endif
