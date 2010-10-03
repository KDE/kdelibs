/*
    Copyright 2010 Rafael Fernández López <ereslibre@kde.org>

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

#include "udev.h"
#include "udevdevice.h"

using namespace Solid::Backends::UDev;

class UDevDevice::Private
{
public:
    Private(udev_device_ *device);
    ~Private();

    QString deviceAttribute(const char *attribute) const;
    QString deviceName() const;
    QString devicePath() const;

    udev_device_ *m_device;
};

UDevDevice::Private::Private(udev_device_ *device)
    : m_device(device)
{
}

UDevDevice::Private::~Private()
{
    udev_device_unref(m_device);
}

QString UDevDevice::Private::deviceAttribute(const char *attribute) const
{
    return QString::fromUtf8(udev_device_get_sysattr_value(m_device, attribute));
}

QString UDevDevice::Private::deviceName() const
{
    return QString::fromUtf8(udev_device_get_syspath(m_device));
}

QString UDevDevice::Private::devicePath() const
{
    return QString(UDEV_UDI_PREFIX) + deviceName();
}

UDevDevice::UDevDevice(udev_device_ *const device)
    : Solid::Ifaces::Device()
    , d(new Private(device))
{
}

UDevDevice::~UDevDevice()
{
    delete d;
}

QString UDevDevice::udi() const
{
    return d->devicePath();
}

QString UDevDevice::parentUdi() const
{
    return UDEV_UDI_PREFIX;
}

QString UDevDevice::vendor() const
{
    return d->deviceAttribute("manufacturer");
}

QString UDevDevice::product() const
{
    return d->deviceAttribute("product");
}

QString UDevDevice::icon() const
{
    // TODO
    return QString();
}

QStringList UDevDevice::emblems() const
{
    // TODO
    return QStringList();
}

QString UDevDevice::description() const
{
    // TODO
    return QString();
}

bool UDevDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    // TODO
    return false;
}

QObject *UDevDevice::createDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    // TODO: IMPLEMENT ALL SUPPORTED
    return 0;
}

QString UDevDevice::device() const
{
    return d->devicePath();
}

