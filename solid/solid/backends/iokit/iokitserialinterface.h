/*  This file is part of the KDE project
    Copyright (C) 2009 Harald Fernengel <harry@kdevelop.org>

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

#ifndef SOLID_BACKENDS_IOKIT_SERIALINTERFACE_H
#define SOLID_BACKENDS_IOKIT_SERIALINTERFACE_H

#include <solid/ifaces/serialinterface.h>
#include "iokitdeviceinterface.h"

namespace Solid
{
namespace Backends
{
namespace IOKit
{
class IOKitDevice;

class SerialInterface : public DeviceInterface, virtual public Solid::Ifaces::SerialInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::SerialInterface)

public:
    SerialInterface(IOKitDevice *device);
    virtual ~SerialInterface();

    QVariant driverHandle() const;
    Solid::SerialInterface::SerialType serialType() const;
    int port() const;
};
}
}
}

#endif // SOLID_BACKENDS_IOKIT_PROCESSOR_H
