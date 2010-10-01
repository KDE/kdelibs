/*
    Copyright 2006 Davide Bettio <davbet@aliceposta.it>
    Copyright 2007 Jeff Mitchell <kde-dev@emailgoeshere.com>

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

#ifndef SOLID_IFACES_PORTABLEMEDIAPLAYER_H
#define SOLID_IFACES_PORTABLEMEDIAPLAYER_H

#include <solid/ifaces/deviceinterface.h>
#include <solid/portablemediaplayer.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This class implements Portable Media Player device interface and represents
     * a portable media player attached to the system.
     * A portable media player is a portable device able to play multimedia files.
     * Some of them have even recording capabilities.
     * @author Davide Bettio <davbet@aliceposta.it>
     */
    class PortableMediaPlayer : virtual public DeviceInterface
    {
    public:
        /**
         * Destroys a portable media player object.
         */
        virtual ~PortableMediaPlayer();

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
         * For example for the "mtp" driver it will return the serial number
         * of the device.
         *
         * @return the driver specific data
         */
        virtual QVariant driverHandle(const QString &driver) const = 0;
    };
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::PortableMediaPlayer, "org.kde.Solid.Ifaces.PortableMediaPlayer/0.1")

#endif
