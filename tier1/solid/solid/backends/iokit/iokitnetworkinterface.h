/*
    Copyright 2009 Harald Fernengel <harry@kdevelop.org>

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

#ifndef SOLID_BACKENDS_IOKIT_IOKITNETWORKINTERFACE_H
#define SOLID_BACKENDS_IOKIT_IOKITNETWORKINTERFACE_H

#include <solid/ifaces/networkinterface.h>
#include "iokitdeviceinterface.h"

namespace Solid
{
namespace Backends
{
namespace IOKit
{
class IOKitDevice;

class NetworkInterface : public DeviceInterface, virtual public Solid::Ifaces::NetworkInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::NetworkInterface)

public:
    NetworkInterface(IOKitDevice *device);
    virtual ~NetworkInterface();

    virtual QString ifaceName() const;
    virtual bool isWireless() const;
    virtual QString hwAddress() const;
    virtual qulonglong macAddress() const;
};
}
}
}

#endif // SOLID_BACKENDS_IOKIT_IOKITNETWORKINTERFACE_H
