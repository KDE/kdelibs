/*
    Copyright 2006-2007 Kevin Ottens <ervin@kde.org>
    Copyright 2007      Will Stephenson <wstephenson@kde.org>

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

#ifndef SOLID_VIDEO_H
#define SOLID_VIDEO_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

#include <QtCore/QStringList>
#include <QtCore/QVariant>

namespace Solid
{
    class VideoPrivate;
    class Device;

    /**
     * This device interface is available on video devices.
     *
     * A video device provides access to a live stream of video, in contrast
     * to a camera device which records individual frames or movies which
     * are accessed discretely.
     */
    class SOLID_EXPORT Video : public DeviceInterface
    {
        Q_OBJECT
        Q_DECLARE_PRIVATE(Video)
        Q_PROPERTY(QStringList supportedDrivers READ supportedDrivers)
        friend class Device;

    private:
        /**
         * Creates a new Video object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         */
        explicit Video(QObject *backendObject);

    public:
        /**
         * Destroys a Video object.
         */
        virtual ~Video();


        /**
         * Get the Solid::DeviceInterface::Type of the Video device interface.
         *
         * @return the Video device interface type
         * @see Solid::DeviceInterface::Type
         */
        static Type deviceInterfaceType() { return DeviceInterface::Video; }

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
         * For example for the "mtp" driver it will return the serial number
         * of the device.
         *
         * @return the driver specific data
         */
        QVariant driverHandle(const QString &driver) const;
    };
}

#endif
