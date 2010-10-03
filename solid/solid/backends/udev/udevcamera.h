/*
    Copyright 2010 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_BACKENDS_UDEV_CAMERA_H
#define SOLID_BACKENDS_UDEV_CAMERA_H

#include <solid/ifaces/camera.h>
#include "udevdeviceinterface.h"

namespace Solid
{
namespace Backends
{
namespace UDev
{
class Camera : public DeviceInterface, virtual public Solid::Ifaces::Camera
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Camera)

public:
    Camera(UDevDevice *device);
    virtual ~Camera();

    virtual QStringList supportedProtocols() const;
    virtual QStringList supportedDrivers(QString protocol = QString()) const;
    virtual QVariant driverHandle(const QString &driver) const;
};
}
}
}

#endif
