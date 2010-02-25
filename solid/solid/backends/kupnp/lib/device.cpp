/*
    This file is part of the KUPnP library, part of the KDE project.

    Copyright 2009 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#include "device.h"
#include "device_p.h"


namespace UPnP
{

Device::Device()
  : d( new DevicePrivate() )
{}

Device::Device( DevicePrivate* _d )
  : d( _d )
{
}

Device::Device( const Device& other )
  : d( other.d )
{
}


QString Device::udn() const { return d->udn(); }
QString Device::displayName() const { return d->displayName(); }
QString Device::type() const { return d->type(); }
QList<Service> Device::services() const { return d->services(); }
QString Device::ipAddress() const { return d->ipAddress(); }
int Device::ipPortNumber() const { return d->ipPortNumber(); }
QString Device::presentationUrl() const { return d->presentationUrl(); }
Device Device::parentDevice() const { return d->parentDevice(); }
QList<Device> Device::childDevices() const { return d->childDevices(); }
bool Device::hasParentDevice() const { return d->hasParentDevice(); }
bool Device::isValid() const { return d->isValid(); }

Device& Device::operator =( const Device& other )
{
    d = other.d;
    return *this;
}

bool Device::operator==( const Device& other ) const
{
    return ( d == other.d );
}

Device::~Device()
{
}

}
