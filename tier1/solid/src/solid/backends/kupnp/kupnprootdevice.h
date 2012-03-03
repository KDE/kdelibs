/*
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

#ifndef SOLID_BACKENDS_KUPnP_KUPNPROOTDEVICE_H
#define SOLID_BACKENDS_KUPnP_KUPNPROOTDEVICE_H

// Solid
#include <solid/ifaces/device.h>


namespace Solid
{
namespace Backends
{
namespace KUPnP
{

class KUPnPRootDevice : public Solid::Ifaces::Device
{
    Q_OBJECT

public:
    KUPnPRootDevice();
    virtual ~KUPnPRootDevice();

public: // Solid::Ifaces::Device API
    virtual QString udi() const;
    virtual QString parentUdi() const;

    virtual QString vendor() const;
    virtual QString product() const;
    virtual QString icon() const;
    virtual QStringList emblems() const;
    virtual QString description() const;

    virtual bool queryDeviceInterface(const Solid::DeviceInterface::Type& type) const;
    virtual QObject* createDeviceInterface(const Solid::DeviceInterface::Type& type);

private:
};

}
}
}

#endif
