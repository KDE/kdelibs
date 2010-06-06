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

#ifndef SOLID_BACKENDS_UPNP_UPNPINTERNETGATEWAYDEVICE_H
#define SOLID_BACKENDS_UPNP_UPNPINTERNETGATEWAYDEVICE_H

#include <backends/upnp/upnpdevice.h>

namespace Solid
{
namespace Backends
{
namespace UPnP
{

    class UPnPInternetGatewayDevice : public Solid::Backends::UPnP::UPnPDevice
    {
        public:
            explicit UPnPInternetGatewayDevice(Herqq::Upnp::HDeviceProxy* device);

            virtual ~UPnPInternetGatewayDevice();

            virtual QString icon() const;

            virtual bool queryDeviceInterface(const Solid::DeviceInterface::Type& type) const;

            virtual QObject* createDeviceInterface(const Solid::DeviceInterface::Type& type);
    };

}
}
}

#endif // SOLID_BACKENDS_UPNP_UPNPINTERNETGATEWAYDEVICE_H
