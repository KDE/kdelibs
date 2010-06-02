/*
   This file is part of the KDE project

   Copyright 2010 Paulo Romulo Alves Barros <paulo.romulo@kdemail.net>

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

#include "upnpdevice.h"

#include <HResourceType>
#include <HDeviceInfo>
#include <HUdn>

namespace Solid
{
namespace Backends
{
namespace UPnP
{

// based on the KUPnP::KUPnPDevice::DeviceData by Friedrich Kossebau :)
struct TypeIconMapping
{
    const char* type;
    const char* iconName;
};

static const TypeIconMapping typeIconMap[] =
{
    {"BasicDevice:1", "network-server"},
    {"WLANAccessPointDevice:1", "network-wireless"},
    {"PrinterBasic:1", "printer"},
    {"PrinterEnhanced:1", "printer"},
    {"Scanner:1", "scanner"},
    {"MediaServer:1", "folder-remote"},
    {"MediaServer:2", "folder-remote"},
    {"MediaServer:3", "folder-remote"},
    {"MediaRenderer:1", "video-television"},
    {"MediaRenderer:2", "video-television"},
    {"SolarProtectionBlind:1", "device"},
    {"DigitalSecurityCamera:1", "camera"},
    {"HVAC:1", "device"},
    {"LightingControls:1", "light"},
    {"RemoteUIClientDevice:1", "device"},
    {"RemoteUIServerDevice:1", "device"},
    {"RAClient:1", "device"},
    {"RAServer:1", "device"},
    {"RADiscoveryAgent:1", "device"},
    {"Unknown", "device"},
    {"InternetGatewayDevice:1", "network-server"},
    {"LANDevice:1", "network-wired"},
    {"WANDevice:1", "network-wired"},
    {"WANConnectionDevice:1", "network-wired"},
    {"WFADevice:1", "network-wireless"}
};

static const int typeIconMapCount = sizeof(typeIconMap) / sizeof(typeIconMap[0]);
  
UPnPDevice::UPnPDevice(const Herqq::Upnp::HDeviceProxy* device)
  : Solid::Ifaces::Device()   
{
  mDevice = device;
  if (device->parentDevice())
  {
    parentDevice = new UPnPDevice(device->parentProxyDevice());
  }
  else
  {
    parentDevice = 0;
  }
}

UPnPDevice::~UPnPDevice()
{
  delete parentDevice; //should I do this?
}

const Herqq::Upnp::HDeviceProxy* UPnPDevice::device() const
{
  return mDevice;
}

QString UPnPDevice::udi() const
{
  const Herqq::Upnp::HDeviceInfo deviceInfo = device()->deviceInfo();
  QString udi = deviceInfo.udn().toString();
  
  return QString::fromLatin1("/org/kde/upnp/%1").arg(udi);
}

QString UPnPDevice::parentUdi() const
{
  if (parentDevice)
  {
    Herqq::Upnp::HDeviceInfo parentInfo = parentDevice->device()->deviceInfo();
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
  const QString deviceType = deviceInfo.deviceType().toString(Herqq::Upnp::HResourceType::TypeSuffix | Herqq::Upnp::HResourceType::Version);
  
  for (int i = 0; i < typeIconMapCount; ++i)
  {
    const TypeIconMapping& mapping = typeIconMap[i];
    if (deviceType == QLatin1String(mapping.type))
    {
      return QString::fromLatin1(mapping.iconName);
    }
  }

  return QString::fromLatin1("network-server");
}

QStringList UPnPDevice::emblems() const
{
  return QStringList(); //does this apply here?
}

QString UPnPDevice::description() const
{
  return this->device()->deviceDescription();
}

bool UPnPDevice::queryDeviceInterface(const Solid::DeviceInterface::Type& type) const
{
  Q_UNUSED(type)
  
  return false;
}

QObject* UPnPDevice::createDeviceInterface(const Solid::DeviceInterface::Type& type)
{
  Q_UNUSED(type)
  
  return 0;
}
  
}
}
}