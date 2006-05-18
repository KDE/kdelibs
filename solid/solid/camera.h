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

#ifndef SOLID_CAMERA_H
#define SOLID_CAMERA_H

#include <kdelibs_export.h>

#include <solid/capability.h>
#include <solid/ifaces/enums.h>

namespace Solid
{
    namespace Ifaces
    {
        class Camera;
    }

    /**
     * This capability is available on digital camera devices.
     *
     * A digital camera is a device used to transform images into
     * data. Nowaday most digital cameras are multifunctional and
     * able to take photographs, video or sound. On the system side
     * they are a particular type of device holding data, the access
     * method can be different from the typical storage device, hence
     * why it's a separate capability.
     */
    class KDE_EXPORT Camera : public Capability, public Ifaces::Enums::Camera
    {
        Q_OBJECT
    public:
        /**
         * Creates a new Camera object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param iface the capability interface provided by the backend
         * @param parent the parent QObject
         * @see Solid::Device::as()
         */
        Camera( Ifaces::Camera *iface, QObject *parent = 0 );

        /**
         * Destroys a Camera object.
         */
        virtual ~Camera();


        /**
         * Get the Solid::Capability::Type of the Camera capability.
         *
         * @return the Camera capability type
         * @see Solid::Ifaces::Enums::Capability::Type
         */
        static Type capabilityType() { return Capability::Camera; }


        /**
         * Retrieves the type of method that should be used to access this
         * device.
         *
         * @return the access method type
         * @see Solid::Ifaces::Enums::Camera::AccessType
         */
        AccessType accessMethod() const;

        /**
         * Indicates if the camera is supported by a driver from the libgphoto2 project.
         *
         * @return true if the camera is supported by libgphoto2, false otherwise.
         */
        bool isGphotoSupported() const;

    private:
        class Private;
        Private *d;
    };
}

#endif
