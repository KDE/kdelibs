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

#ifndef SOLID_POINTER_H
#define SOLID_POINTER_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
    class PointerPrivate;
    class Device;

    /**
     * This device interface is available on pointer devices.
     *
     * A pointer is a device like a mouse, touchpad, touch screen or a tablet.
     *
     * @author Ivan Cukic <ivan.cukic@kde.org>
     */
    class SOLID_EXPORT Pointer : public DeviceInterface
    {
        Q_OBJECT
        Q_ENUMS(PointerType)
        Q_PROPERTY(PointerType type READ type)
        Q_DECLARE_PRIVATE(Pointer)
        friend class Device;

    public:
        /**
         * This enum type defines the type of a pointer.
         *
         * -
         * - UnknownPointerType : The type of the pointer is unknown.
         */
         enum PointerType{ MousePointer, TouchpadPointer, TouchscreenPointer, TabletPointer, UnknownPointerType };


    private:
        /**
         * Creates a new Pointer object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         */
        explicit Pointer(QObject *backendObject);

    public:
        /**
         * Destroys a Pointer object.
         */
        virtual ~Pointer();

        /**
         * Get the Solid::DeviceInterface::Type of the Pointer device interface.
         *
         * @return the Pointer device interface type
         * @see Solid::DeviceInterface::Type
         */
        static Type deviceInterfaceType() { return DeviceInterface::Pointer; }

        /**
         * Retrieves the type of the pointer device.
         *
         * @return the type of the pointer device.
         * @see Solid::Pointer::PointerType
         */
        PointerType type() const;

    };
}

#endif
