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

#ifndef SOLID_IFACES_CAMERA_H
#define SOLID_IFACES_CAMERA_H

#include <solid/ifaces/deviceinterface.h>
#include <solid/camera.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This device interface is available on digital camera devices.
     *
     * A digital camera is a device used to transform images into
     * data. Nowaday most digital cameras are multifunctional and
     * able to take photographs, video or sound. On the system side
     * they are a particular type of device holding data, the access
     * method can be different from the typical storage device, hence
     * why it's a separate device interface.
     */
    class Camera : virtual public DeviceInterface
    {
    public:
        /**
         * Destroys a Camera object.
         */
        virtual ~Camera();


        /**
         * Retrieves the type of method that should be used to access this
         * device.
         *
         * @return the access method type
         * @see Solid::Camera::AccessType
         */
        virtual Solid::Camera::AccessType accessMethod() const = 0;

        /**
         * Indicates if the camera is supported by a driver from the libgphoto2 project.
         *
         * @return true if the camera is supported by libgphoto2, false otherwise.
         */
        virtual bool isGphotoSupported() const = 0;
    };
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::Camera, "org.kde.Solid.Ifaces.Camera/0.1")

#endif
