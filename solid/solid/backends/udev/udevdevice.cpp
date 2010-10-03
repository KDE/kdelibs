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

#include "udevdevice.h"

#include "udevgenericinterface.h"
#include "udevcamera.h"

using namespace Solid::Backends::UDev;

UDevDevice::UDevDevice(udev_device_ *const device)
    : Solid::Ifaces::Device(),
      m_device(device)
{
}

UDevDevice::~UDevDevice()
{
    udev_device_unref(m_device);
}

QString UDevDevice::udi() const
{
    return devicePath();
}

QString UDevDevice::parentUdi() const
{
    return UDEV_UDI_PREFIX;
}

QString UDevDevice::vendor() const
{
    return systemAttribute("manufacturer");
}

QString UDevDevice::product() const
{
    return systemAttribute("product");
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
    switch (type) {
    case Solid::DeviceInterface::GenericInterface:
        return true;

    case Solid::DeviceInterface::Camera:
        return !property("ID_GPHOTO2").toString().isEmpty();

    default:
        return false;
    }
}

QObject *UDevDevice::createDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    if (!queryDeviceInterface(type)) {
        return 0;
    }

    switch (type) {
    case Solid::DeviceInterface::GenericInterface:
        return new GenericInterface(this);

    case Solid::DeviceInterface::Camera:
        return new Camera(this);

    default:
        qFatal("Shouldn't happen");
        return 0;
    }
}

QString UDevDevice::device() const
{
    return devicePath();
}

QVariant UDevDevice::property(const QString &key) const
{
    return QString::fromUtf8(udev_device_get_property_value(m_device, key.toAscii()));
}

QMap<QString, QVariant> UDevDevice::allProperties() const
{
    QMap<QString, QVariant> properties;

    udev_list_entry_ *list_entry;
    udev_list_entry_foreach(list_entry, udev_device_get_properties_list_entry(m_device)) {
        properties[udev_list_entry_get_name(list_entry)] = udev_list_entry_get_value(list_entry);
    }

    return properties;
}

bool UDevDevice::propertyExists(const QString &key) const
{
    return allProperties().contains(key);
}

QString UDevDevice::systemAttribute(const char *attribute) const
{
    return QString::fromUtf8(udev_device_get_sysattr_value(m_device, attribute));
}

QString UDevDevice::deviceName() const
{
    return QString::fromUtf8(udev_device_get_syspath(m_device));
}

QString UDevDevice::devicePath() const
{
    return QString(UDEV_UDI_PREFIX) + deviceName();
}
