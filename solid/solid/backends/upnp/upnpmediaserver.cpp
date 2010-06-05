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

#include "upnpmediaserver.h"
#include "upnpstorageaccess.h"

#include <HDeviceInfo>

namespace Solid
{
namespace Backends
{
namespace UPnP
{

UPnPMediaServer::UPnPMediaServer(Herqq::Upnp::HDeviceProxy* device) :
    UPnPDevice(device)
{
    m_version = device->deviceInfo().deviceType().toString(Herqq::Upnp::HResourceType::Version);
}

UPnPMediaServer::~UPnPMediaServer()
{

}

QString UPnPMediaServer::icon() const
{
    return QString::fromLatin1("folder-remote");
}

bool UPnPMediaServer::queryDeviceInterface(const Solid::DeviceInterface::Type& type) const
{
    return type == Solid::DeviceInterface::StorageAccess;
}

QObject* UPnPMediaServer::createDeviceInterface(const Solid::DeviceInterface::Type& type)
{
    if (type == Solid::DeviceInterface::StorageAccess)
    {
      return new Solid::Backends::UPnP::UPnPStorageAccess(this);
    }

    return 0;
}

QString UPnPMediaServer::version() const
{
    return m_version;
}

}
}
}