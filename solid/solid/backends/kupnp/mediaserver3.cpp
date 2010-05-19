/*
    This file is part of the KDE project

    Copyright 2010 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#include "mediaserver3.h"

// Solid
#include "kupnpstorageaccess.h"
// Qt
#include <QtCore/QStringList>
#include <QtCore/QSet>

namespace Solid
{
namespace Backends
{
namespace KUPnP
{

static const char MediaServer3Udn[] = "urn:schemas-upnp-org:device:MediaServer:3";

MediaServer3Factory::MediaServer3Factory() {}

void MediaServer3Factory::addSupportedInterfaces( QSet<Solid::DeviceInterface::Type>& interfaces ) const
{
    interfaces << Solid::DeviceInterface::StorageAccess;
}

QStringList MediaServer3Factory::typeNames( Solid::DeviceInterface::Type type ) const
{
    QStringList result;

    if (type==Solid::DeviceInterface::StorageAccess)
        result << QLatin1String(MediaServer3Udn);

    return result;
}

QObject* MediaServer3Factory::tryCreateDevice( const Cagibi::Device& device ) const
{
    return ( device.type() == QLatin1String(MediaServer3Udn) ) ?
        new MediaServer3( device ) : 0;
}


MediaServer3::MediaServer3(const Cagibi::Device& device)
  : KUPnPDevice(device)
{
}

MediaServer3::~MediaServer3()
{
}

QString MediaServer3::icon() const
{
    return QString::fromLatin1("folder-remote");
}


QString MediaServer3::description() const
{
    return QObject::tr("UPnP Media Server v3");
}


bool MediaServer3::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    bool result = false;

    if (type==Solid::DeviceInterface::StorageAccess) {
        result = true;
    }

    return result;
}

QObject* MediaServer3::createDeviceInterface(const Solid::DeviceInterface::Type& type)
{
    DeviceInterface* interface = 0;

    if (type==Solid::DeviceInterface::StorageAccess) {
        interface = new StorageAccess(this);
    }

    return interface;
}

}
}
}
