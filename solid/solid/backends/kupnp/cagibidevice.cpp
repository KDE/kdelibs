/*
    This file is part of the KUPnP library, part of the KDE project.

    Copyright 2009-2010 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#include "cagibidevice.h"
#include "cagibidevice_p.h"


namespace Cagibi
{

Device::Device()
  : d( new DevicePrivate() )
{
}

Device::Device( DevicePrivate* _d )
  : d( _d )
{
}

Device::Device( const Device& other )
  : d( other.d )
{
}

const QString& Device::type() const { return d->type(); }
const QString& Device::friendlyName() const { return d->friendlyName(); }
const QString& Device::manufacturerName() const { return d->manufacturerName(); }
const QString& Device::modelDescription() const { return d->modelDescription(); }
const QString& Device::modelName() const { return d->modelName(); }
const QString& Device::modelNumber() const { return d->modelNumber(); }
const QString& Device::serialNumber() const { return d->serialNumber(); }
const QString& Device::udn() const { return d->udn(); }
const QString& Device::presentationUrl() const { return d->presentationUrl(); }
const QString& Device::ipAddress() const { return d->ipAddress(); }
int Device::ipPortNumber() const { return d->ipPortNumber(); }

bool Device::hasParentDevice() const { return d->hasParentDevice(); }
const QString& Device::parentDeviceUdn() const { return d->parentDeviceUdn(); }

Device& Device::operator=( const Device& other )
{
    d = other.d;
    return *this;
}

Device::~Device()
{
}

}
