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

#include <QtCore/QStringList>

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
         * Retrieves known protocols this device can speak.  This list may be dependent
         * on installed device driver libraries.
         *
         * @return a list of known protocols this device can speak
         */
        virtual QStringList supportedProtocols() const = 0;

        /**
         * Retrieves known installed device drivers that claim to handle this device
         * using the requested protocol.
         *
         * @param protocol The protocol to get drivers for.
         * @return a list of known device drivers that can handle this device
         */
        virtual QStringList supportedDrivers(QString protocol = QString()) const = 0;

        /**
         * Retrieves a driver specific string allowing to access the device.
         *
         * For example for the "gphoto" driver it will return a list of the
         * form '("usb", vendor_id, product_id)'.
         *
         * @return the driver specific data
         */
        virtual QVariant driverHandle(const QString &driver) const = 0;
    };
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::Camera, "org.kde.Solid.Ifaces.Camera/0.1")

#endif
