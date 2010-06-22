/*
   This file is part of the KDE project

   Copyright 2010 Paulo Romulo Alves Barros <paulo.romulo@kdemail.net>

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