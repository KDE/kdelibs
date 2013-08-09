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

#ifndef SOLID_POINTING_DEVICE_H
#define SOLID_POINTING_DEVICE_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
    class PointingDevicePrivate;
    class Device;

    /**
     * This device interface is available on pointing devices.
     *
     * A pointing device is a device like a mouse, touchpad, touch
     * screen or a tablet (graphics tablet).
     *
     * @author Ivan Cukic <ivan.cukic@kde.org>
     */
    class SOLID_EXPORT PointingDevice : public DeviceInterface
    {
        Q_OBJECT
        Q_ENUMS(PointingDeviceType)
        Q_PROPERTY(PointingDeviceType type READ type)
        Q_DECLARE_PRIVATE(PointingDevice)
        friend class Device;

    public:
        /**
         * This enum type defines the type of a PointingDevice.
         *
         * - Mouse : A mouse
         * - Touchpad : A touchpad
         * - Touchscreen : The touchscreen input device
         * - Tablet : A graphics tablet with a stylus
         * - UnknownPointingDeviceType : The type of the pointing device is unknown.
         */
         enum PointingDeviceType{ Mouse, Touchpad, Touchscreen, Tablet, UnknownPointingDeviceType };


    private:
        /**
         * Creates a new PointingDevice object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         */
        explicit PointingDevice(QObject *backendObject);

    public:
        /**
         * Destroys a PointingDevice object.
         */
        virtual ~PointingDevice();

        /**
         * Get the Solid::DeviceInterface::Type of the pointing device interface.
         *
         * @return the pointing device interface type
         * @see Solid::DeviceInterface::Type
         */
        static Type deviceInterfaceType() { return DeviceInterface::PointingDevice; }

        /**
         * Retrieves the type of the pointing device.
         *
         * @return the type of the pointing device.
         * @see Solid::PointingDevice::PointingDeviceType
         */
        PointingDeviceType type() const;

    };
}

#endif
