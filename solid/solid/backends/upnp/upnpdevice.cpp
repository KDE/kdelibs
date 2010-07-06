/*
   This file is part of the KDE project

   Copyright 2010 Paulo Romulo Alves Barros <paulo.romulo@kdemail.net>

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

#include "upnpdevice.h"
#include "upnpmediaserver.h"

#include <HResourceType>
#include <HDeviceInfo>
#include <HUdn>

namespace Solid
{
namespace Backends
{
namespace UPnP
{

static QMap<QString, QString> makeTypeIconMap()
{
    QMap<QString, QString> ret;
    ret.insert("BasicDevice:1", "network-server");
    ret.insert("WLANAccessPointDevice:1", "network-wireless");
    ret.insert("PrinterBasic:1", "printer");
    ret.insert("PrinterEnhanced:1", "printer");
    ret.insert("Scanner:1", "scanner");
    ret.insert("MediaServer:1", "folder-remote");
    ret.insert("MediaServer:2", "folder-remote");
    ret.insert("MediaServer:3", "folder-remote");
    ret.insert("MediaRenderer:1", "video-television");
    ret.insert("MediaRenderer:2", "video-television");
    ret.insert("SolarProtectionBlind:1", "device");
    ret.insert("DigitalSecurityCamera:1", "camera");
    ret.insert("HVAC:1", "device");
    ret.insert("LightingControls:1", "light");
    ret.insert("RemoteUIClientDevice:1", "device");
    ret.insert("RemoteUIServerDevice:1", "device");
    ret.insert("RAClient:1", "device");
    ret.insert("RAServer:1", "device");
    ret.insert("RADiscoveryAgent:1", "device");
    ret.insert("Unknown", "device");
    ret.insert("InternetGatewayDevice:1", "network-server");
    ret.insert("LANDevice:1", "network-wired");
    ret.insert("WANDevice:1", "network-wired");
    ret.insert("WANConnectionDevice:1", "network-wired");
    ret.insert("WFADevice:1", "network-wireless");

    return ret;
}

static const QMap<QString, QString> typeIconMap = makeTypeIconMap();

UPnPDevice::UPnPDevice(const Herqq::Upnp::HDeviceProxy* device) :
    Solid::Ifaces::Device(),
    m_device(device),
    m_specVersion(device->deviceInfo().deviceType().toString(Herqq::Upnp::HResourceType::Version)),
    m_deviceType(device->deviceInfo().deviceType().toString(Herqq::Upnp::HResourceType::TypeSuffix | Herqq::Upnp::HResourceType::Version))
{
}

UPnPDevice::~UPnPDevice()
{
    delete m_device;
}

const Herqq::Upnp::HDeviceProxy* UPnPDevice::device() const
{
    return m_device;
}

QString UPnPDevice::udi() const
{
    const Herqq::Upnp::HDeviceInfo deviceInfo = device()->deviceInfo();

    if (!deviceInfo.udn().isValid())
    {
        qWarning("This device UDN is not a valid one!");
    }

    QString udn = deviceInfo.udn().toString();

    return QString::fromLatin1("/org/kde/upnp/%1").arg(udn);
}

QString UPnPDevice::parentUdi() const
{
    const Herqq::Upnp::HDeviceProxy* parent = device()->parentProxyDevice();
    if (parent)
    {
        Herqq::Upnp::HDeviceInfo parentInfo = parent->deviceInfo();

        if (!parentInfo.udn().isValid())
        {
            qWarning("This device UDN is not a valid one!");
        }

        return QString::fromLatin1("/org/kde/upnp/%1").arg(parentInfo.udn().toString());
    }

    return QString::fromLatin1("/org/kde/upnp");
}

QString UPnPDevice::vendor() const
{
    const Herqq::Upnp::HDeviceInfo deviceInfo = device()->deviceInfo();
    QString vendor = deviceInfo.manufacturer();

    return vendor;
}

QString UPnPDevice::product() const
{
    const Herqq::Upnp::HDeviceInfo deviceInfo = device()->deviceInfo();
    QString model = deviceInfo.modelName();

    return model;
}

QString UPnPDevice::icon() const
{
    const Herqq::Upnp::HDeviceInfo deviceInfo = device()->deviceInfo();

    if (typeIconMap.contains(deviceType()))
    {
        return typeIconMap[deviceType()];
    }

    return QString::fromLatin1("network-server");
}

QStringList UPnPDevice::emblems() const
{
    return QStringList();
}

QString UPnPDevice::description() const
{
    return device()->deviceInfo().modelDescription();
}

bool UPnPDevice::isMediaServer() const
{
    return deviceType().startsWith(QString::fromLatin1("MediaServer"));
}

bool UPnPDevice::isInternetGatewayDevice() const
{
    return deviceType().startsWith(QString::fromLatin1("InternetGatewayDevice"));
}

bool UPnPDevice::queryDeviceInterface(const Solid::DeviceInterface::Type& type) const
{
    if (type == Solid::DeviceInterface::StorageAccess)
    {
        if (isMediaServer())
        {
            return true;
        }
        else
        {
            return false;
        }  
    }
    
    return false;
}

QObject* UPnPDevice::createDeviceInterface(const Solid::DeviceInterface::Type& type)
{
    if (type == Solid::DeviceInterface::StorageAccess)
    {
        if (isMediaServer())
        {
            return new Solid::Backends::UPnP::UPnPMediaServer(this);
        }
        else
        {
            return 0;
        }
    }

    return 0;
}

bool UPnPDevice::isValid() const
{
    return m_device->deviceInfo().isValid();
}

const QString UPnPDevice::specVersion() const
{
    return m_specVersion;
}

const QString UPnPDevice::deviceType() const
{
    return m_deviceType;
}

}
}
}