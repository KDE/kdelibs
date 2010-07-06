/*
   This file is part of the KDE project

   Copyright 2010 Paulo Romulo Alves Barros <paulo.romulo@kdemail.net>

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

#ifndef SOLID_BACKENDS_UPNP_UPNP_DEVICE_H
#define SOLID_BACKENDS_UPNP_UPNP_DEVICE_H

#include <solid/ifaces/device.h>

#include <HDeviceProxy>

namespace Solid
{
namespace Backends
{
namespace UPnP
{

    class UPnPDevice : public Solid::Ifaces::Device
    {
        Q_OBJECT

        public:
            explicit UPnPDevice(const Herqq::Upnp::HDeviceProxy* device);

            virtual ~UPnPDevice();

            virtual QString udi() const;

            virtual QString parentUdi() const;

            virtual QString vendor() const;

            virtual QString product() const;

            virtual QString icon() const;

            virtual QStringList emblems() const;

            virtual QString description() const;

            virtual bool queryDeviceInterface(const Solid::DeviceInterface::Type& type) const;

            virtual QObject* createDeviceInterface(const Solid::DeviceInterface::Type& type);

            bool isValid() const;

            const Herqq::Upnp::HDeviceProxy* device() const;

            const QString specVersion() const;

            const QString deviceType() const;

            bool isMediaServer() const;

            bool isInternetGatewayDevice() const;

        protected:
            const Herqq::Upnp::HDeviceProxy* m_device;

            const QString m_specVersion;

            const QString m_deviceType;
    };

}
}
}
#endif // SOLID_BACKENDS_UPNP_UPNP_DEVICE_H