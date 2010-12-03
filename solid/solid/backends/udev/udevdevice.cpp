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
#include "udevprocessor.h"
#include "udevcamera.h"
#include "udevvideo.h"
#include "udevportablemediaplayer.h"
#include "udevdvbinterface.h"
#include "udevblock.h"
#include "cpuinfo.h"

using namespace Solid::Backends::UDev;

UDevDevice::UDevDevice(const UdevQt::Device device)
    : Solid::Ifaces::Device()
    , m_device(device)
{
}

UDevDevice::~UDevDevice()
{
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
    QString vendor = m_device.sysfsProperty("manufacturer").toString();
    if (vendor.isEmpty()) {
         if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
             // sysfs doesn't have anything useful here
            vendor = extractCpuInfoLine(deviceNumber(), "vendor_id\\s+:\\s+(\\S.+)");
         } else if (queryDeviceInterface(Solid::DeviceInterface::Video)) {
             vendor = m_device.deviceProperty("ID_VENDOR").toString();
         }
    }
    return vendor;
}

QString UDevDevice::product() const
{
    QString product = m_device.sysfsProperty("product").toString();
    if (product.isEmpty()) {
        if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
            // sysfs doesn't have anything useful here
            product = extractCpuInfoLine(deviceNumber(), "model name\\s+:\\s+(\\S.+)");
        } else if(queryDeviceInterface(Solid::DeviceInterface::Video)) {
            product = m_device.deviceProperty("ID_V4L_PRODUCT").toString();
        }
    }
    return product;
}

QString UDevDevice::icon() const
{
    if (parentUdi().isEmpty()) {
        return "computer";
    }

    if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
        return "cpu";
    } else if (queryDeviceInterface(Solid::DeviceInterface::Camera)) {
        return "camera-photo";
    } else if (queryDeviceInterface(Solid::DeviceInterface::PortableMediaPlayer)) {
        // TODO: check out special cases like iPod
        return "multimedia-player";
    } else if (queryDeviceInterface(Solid::DeviceInterface::Video)) {
        return "camera-web";
    }

    return QString();
}

QStringList UDevDevice::emblems() const
{
    return QStringList();
}

QString UDevDevice::description() const
{
    if (parentUdi().isEmpty()) {
        return QObject::tr("Computer");
    }

    if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
        return QObject::tr("Processor");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Camera)) {
        return QObject::tr("Camera");
    } else if (queryDeviceInterface(Solid::DeviceInterface::PortableMediaPlayer)) {
        // TODO: check out special cases like iPod
        return QObject::tr("Portable Media Player"); // FIXME: not sure what HAL was returning since I can't check it
    } else if (queryDeviceInterface(Solid::DeviceInterface::Video)) {
        return product();
    }

    return QString();
}

bool UDevDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    switch (type) {
    case Solid::DeviceInterface::GenericInterface:
        return true;

    case Solid::DeviceInterface::Processor:
        return property("DRIVER").toString() == "processor";

    case Solid::DeviceInterface::Camera:
        return !property("ID_GPHOTO2").toString().isEmpty();

    case Solid::DeviceInterface::PortableMediaPlayer:
        return false; // TODO: check what determines a portable media player

    case Solid::DeviceInterface::DvbInterface:
        return m_device.subsystem() ==  QLatin1String("dvb");

    case Solid::DeviceInterface::Block:
        return !property("MAJOR").toString().isEmpty();

    case Solid::DeviceInterface::Video:
        return m_device.subsystem() == QLatin1String("video4linux");

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

    case Solid::DeviceInterface::Processor:
        return new Processor(this);

    case Solid::DeviceInterface::Camera:
        return new Camera(this);

    case Solid::DeviceInterface::PortableMediaPlayer:
        return new PortableMediaPlayer(this);

    case Solid::DeviceInterface::DvbInterface:
        return new DvbInterface(this);

    case Solid::DeviceInterface::Block:
        return new Block(this);

    case Solid::DeviceInterface::Video:
        return new Video(this);

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
    const QVariant res = m_device.deviceProperty(key);
    if (res.isValid()) {
        return res;
    }
    return m_device.sysfsProperty(key);
}

QMap<QString, QVariant> UDevDevice::allProperties() const
{
    QMap<QString, QVariant> res;
    foreach (const QString &prop, m_device.deviceProperties()) {
        res[prop] = property(prop);
    }
    return res;
}

bool UDevDevice::propertyExists(const QString &key) const
{
    return m_device.deviceProperties().contains(key);
}

QString UDevDevice::systemAttribute(const char *attribute) const
{
    return m_device.sysfsProperty(attribute).toString();
}

QString UDevDevice::deviceName() const
{
    return m_device.sysfsPath();
}

int UDevDevice::deviceNumber() const
{
    return m_device.sysfsNumber();
}

QString UDevDevice::devicePath() const
{
    return QString(UDEV_UDI_PREFIX) + deviceName();
}
