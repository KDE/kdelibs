/*
    Copyright 2007 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_IFACE_DVBINTERFACE_H
#define SOLID_IFACE_DVBINTERFACE_H

#include <solid/ifaces/deviceinterface.h>

#include <solid/dvbinterface.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This device interface is available on Digital Video Broadcast (DVB) devices.
     *
     * A DVB device is a device implementing the open standards for digital
     * television maintained by the DVB Project
     * It is possible to interact with such a device using a special device
     * file in the system.
     */
    class DvbInterface : virtual public DeviceInterface
    {
    public:
        /**
         * Destroys a DvbInterface object.
         */
        virtual ~DvbInterface();

        /**
         * Retrieves the absolute path of the special file to interact
         * with the device.
         *
         * @return the absolute path of the special file to interact with
         * the device
         */
        virtual QString device() const = 0;

        /**
         * Retrieves the adapter number of this dvb device.
         * Note that -1 is returned in the case the adapter couldn't be
         * determined.
         *
         * @return the adapter number of this dvb device or -1
         */
        virtual int deviceAdapter() const = 0;

        /**
         * Retrieves the type of this dvb device.
         *
         * @return the device type of this dvb device
         * @see Solid::DvbInterface::DeviceType
         */
        virtual Solid::DvbInterface::DeviceType deviceType() const = 0;

        /**
         * Retrieves the index of this dvb device.
         * Note that -1 is returned in the case the device couldn't be
         * identified (deviceType() == DvbUnknown).
         *
         * @return the index of this dvb device or -1
         * @see Solid::Ifaces::DvbInterface::deviceType
         */
        virtual int deviceIndex() const = 0;
    };
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::DvbInterface, "org.kde.Solid.Ifaces.DvbInterface/0.1")

#endif // SOLID_IFACE_DVBINTERFACE_H
