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

#ifndef SOLID_CAMERA_H
#define SOLID_CAMERA_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

#include <QtCore/QStringList>
#include <QtCore/QVariant>

namespace Solid
{
    class CameraPrivate;
    class Device;

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
    class SOLID_EXPORT Camera : public DeviceInterface
    {
        Q_OBJECT
        Q_PROPERTY(QStringList supportedProtocols READ supportedProtocols)
        Q_PROPERTY(QStringList supportedDrivers READ supportedDrivers)
        Q_DECLARE_PRIVATE(Camera)
        friend class Device;

    private:
        /**
         * Creates a new Camera object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         */
        explicit Camera(QObject *backendObject);

    public:
        /**
         * Destroys a Camera object.
         */
        virtual ~Camera();


        /**
         * Get the Solid::DeviceInterface::Type of the Camera device interface.
         *
         * @return the Camera device interface type
         * @see Solid::DeviceInterface::Type
         */
        static Type deviceInterfaceType() { return DeviceInterface::Camera; }



        /**
         * Retrieves known protocols this device can speak.  This list may be dependent
         * on installed device driver libraries.
         *
         * @return a list of known protocols this device can speak
         */
        QStringList supportedProtocols() const;

        /**
         * Retrieves known installed device drivers that claim to handle this device
         * using the requested protocol.  If protocol is blank, returns a list of
         * all drivers supporting the device.
         *
         * @param protocol The protocol to get drivers for.
         * @return a list of installed drivers meeting the criteria
         */
        QStringList supportedDrivers(QString protocol = QString()) const;

        /**
         * Retrieves a driver specific string allowing to access the device.
         *
         * For example for the "gphoto" driver it will return a list of the
         * form '("usb", vendor_id, product_id)'.
         *
         * @return the driver specific data
         */
        QVariant driverHandle(const QString &driver) const;
    };
}

#endif
