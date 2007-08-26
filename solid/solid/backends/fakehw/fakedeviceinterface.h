/*  This file is part of the KDE project
    Copyright (C) 2006 Michaël Larouche <michael.larouche@kdemail.net>

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
#ifndef SOLID_BACKENDS_FAKEHW_FAKEDEVICEINTERFACE_H
#define SOLID_BACKENDS_FAKEHW_FAKEDEVICEINTERFACE_H

#include <QtCore/QObject>
#include <solid/ifaces/deviceinterface.h>

#include "fakedevice.h"

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeDeviceInterface : public QObject, virtual public Solid::Ifaces::DeviceInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::DeviceInterface)
public:
    explicit FakeDeviceInterface(FakeDevice *device);
    ~FakeDeviceInterface();

protected:
    FakeDevice *fakeDevice() const
    {
        return m_device;
    }
    FakeDevice *fakeDevice()
    {
        return m_device;
    }

private:
    FakeDevice *m_device;
};
}
}
}

#endif // SOLID_BACKENDS_FAKEHW_FAKEDEVICEINTERFACE_H
