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

#ifndef SOLID_BACKENDS_KUPNP_INTERNETGATEWAYDEVICE1_H
#define SOLID_BACKENDS_KUPNP_INTERNETGATEWAYDEVICE1_H

// KUPnP
#include "kupnpdevice.h"


namespace Solid
{
namespace Backends
{
namespace KUPnP
{

class InternetGatewayDevice1Factory : public AbstractDeviceFactory
{
public:
    InternetGatewayDevice1Factory();

public: // AbstractDeviceFactory API
    virtual QObject* tryCreateDevice( const Cagibi::Device& device ) const;
};


class InternetGatewayDevice1 : public KUPnPDevice
{
public:
    explicit InternetGatewayDevice1(const Cagibi::Device& device);
    virtual ~InternetGatewayDevice1();

public: // Solid::Ifaces::Device API
    virtual QString icon() const;
    virtual QString description() const;

    virtual bool queryDeviceInterface(const Solid::DeviceInterface::Type& type) const;
    virtual QObject* createDeviceInterface(const Solid::DeviceInterface::Type& type);
};

}
}
}

#endif
