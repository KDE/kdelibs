/*
    This file is part of the KDE project

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

#include "kupnpdevice.h"

// Solid
#include "kupnpstorageaccess.h"
// Qt
#include <QtCore/QStringList>
#include <QtCore/QDebug>

#define QI18N(x) x

namespace Solid
{
namespace Backends
{
namespace KUPnP
{

struct DeviceData
{
    const char* type;
    const char* iconName;
    const char* name;
};

static const
DeviceData deviceData[] =
{
    {"BasicDevice1", "network-server", QI18N("UPnP Basic Device") },
    {"WLANAccessPointDevice1", "network-wireless", QI18N("UPnP WLAN Access Point Device") },
    {"PrinterBasic1", "printer", QI18N("UPnP Printer (Basic)") },
    {"PrinterEnhanced1", "printer", QI18N("UPnP Printer (Enhanced)") },
    {"Scanner1", "scanner", QI18N("UPnP Scanner") },
    {"MediaRenderer1", "video-television", QI18N("UPnP Media Renderer") },
    {"MediaRenderer2", "video-television", QI18N("UPnP Media Renderer") },
    {"SolarProtectionBlind1", "device", QI18N("UPnP Solar Protection Blind") },
    {"DigitalSecurityCamera1", "camera", QI18N("UPnP Digital Security Camera") },
    {"HVAC1", "device", QI18N("UPnP HVAC") },
    {"LightingControls1", "light", QI18N("UPnP Lighting Controls") },
    {"RemoteUIClientDevice1", "device", QI18N("UPnP Remote UI Client") },
    {"RemoteUIServerDevice1", "device", QI18N("UPnP Remote UI Server") },
    {"RAClient1", "device", QI18N("UPnP Remote Access Client") },
    {"RAServer1", "device", QI18N("UPnP Remote Access Server") },
    {"RADiscoveryAgent1", "device", QI18N("UPnP Remote Access Discovery Agent") },
    {"Unknown", "device", QI18N("Unknown UPnP Device") },
    {"LANDevice1", "network-wired", QI18N("UPnP LAN Device") },
    {"WANDevice1", "network-wired", QI18N("UPnP WAN Device") },
    {"WANConnectionDevice1", "network-wired", QI18N("UPnP WAN Connection Device") },
    {"WFADevice1", "network-wireless", QI18N("UPnP Wi-Fi Alliance Device") }
};


static const int deviceDataCount = sizeof(deviceData) / sizeof(deviceData[0]);


AbstractDeviceFactory::~AbstractDeviceFactory() {}

DeviceFactory::DeviceFactory() {}

void AbstractDeviceFactory::addSupportedInterfaces( QSet<Solid::DeviceInterface::Type>& interfaces ) const
{
    Q_UNUSED( interfaces );
}
#if 0
bool AbstractDeviceFactory::hasDeviceInterface( const Cagibi::Device& device,
                                                Solid::DeviceInterface::Type type ) const
{
    Q_UNUSED( device );
    Q_UNUSED( type );

    return false;
}
#endif
QStringList AbstractDeviceFactory::typeNames( Solid::DeviceInterface::Type type ) const
{
    Q_UNUSED( type );

    return QStringList();
}

QObject* DeviceFactory::tryCreateDevice( const Cagibi::Device& device ) const
{
    return new KUPnPDevice( device );
}


KUPnPDevice::KUPnPDevice(const Cagibi::Device& device)
  : Device(),
    mDevice(device),
    mParentDevice(0)
{
}

KUPnPDevice::~KUPnPDevice()
{
    delete mParentDevice;
}

QString KUPnPDevice::udi() const
{
    return QString::fromLatin1("/org/kde/KUPnP/%1").arg( mDevice.udn() );
}

QString KUPnPDevice::parentUdi() const
{
    const QString result =
        mDevice.hasParentDevice() ?
            QString::fromLatin1("/org/kde/KUPnP/%1").arg( mDevice.parentDeviceUdn() ) :
            QString::fromLatin1("/org/kde/KUPnP");

    return result;
}

QString KUPnPDevice::vendor() const
{
    return mDevice.manufacturerName();
}

QString KUPnPDevice::product() const
{
    return mDevice.friendlyName();
}

QString KUPnPDevice::icon() const
{
    const char* result;

    const QString type = mDevice.type();
    int i = 0;
    for (;i<deviceDataCount; ++i ) {
        const DeviceData& data = deviceData[i];
        if (type == QLatin1String(data.type)) {
            result = data.iconName;
            break;
        }
    }
    if (i==deviceDataCount) {
        result = "network-server";
    }

    return QString::fromLatin1(result);
}

QStringList KUPnPDevice::emblems() const
{
    QStringList result;
#if 0
    if (queryDeviceInterface(Solid::DeviceInterface::StorageAccess)) {
        bool isEncrypted = property("volume.fsusage").toString()=="crypto";

        const Hal::StorageAccess accessIface(const_cast<KUPnPDevice *>(this));
        if (accessIface.isAccessible()) {
            if (isEncrypted) {
                result << "emblem-encrypted-unlocked";
            } else {
                result << "emblem-mounted";
            }
        } else {
            if (isEncrypted) {
                result << "emblem-encrypted-locked";
            } else {
                result << "emblem-unmounted";
            }
        }
    }
#endif
    return result;
}

QString KUPnPDevice::description() const
{
    return mDevice.modelDescription();
}


bool KUPnPDevice::queryDeviceInterface(const Solid::DeviceInterface::Type& type) const
{
    Q_UNUSED( type );

    return false;
}

QObject* KUPnPDevice::createDeviceInterface(const Solid::DeviceInterface::Type& type)
{
    Q_UNUSED( type );

    return 0;
}

}
}
}
