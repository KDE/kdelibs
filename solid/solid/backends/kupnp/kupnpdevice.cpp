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
    {"InternetGatewayDevice1", "network-server", QI18N("UPnP Internet Gateway Device") },
    {"PrinterBasic1", "printer", QI18N("UPnP Printer (Basic)") },
    {"PrinterEnhanced1", "printer", QI18N("UPnP Printer (Enhanced)") },
    {"Scanner1", "scanner", QI18N("UPnP Scanner") },
    {"MediaServer1", "folder-remote", QI18N("UPnP Media Server") },
    {"MediaRenderer1", "terminal", QI18N("UPnP Media Renderer") },
    {"MediaServer2", "folder-remote", QI18N("UPnP Media Server") },
    {"MediaRenderer2", "terminal", QI18N("UPnP Media Renderer") },
    {"MediaServer3", "folder-remote", QI18N("UPnP Media Server") },
    {"SolarProtectionBlind1", "device", QI18N("UPnP Solar Protection Blind") },
    {"DigitalSecurityCamera1", "camera", QI18N("UPnP Digital Security Camera") },
    {"HVAC1", "device", QI18N("UPnP HVAC") },
    {"LightingControls1", "light", QI18N("UPnP Lighting Controls") },
    {"RemoteUIClient1", "printer", QI18N("UPnP Remote UI Client") },
    {"RemoteUIServer1", "printer", QI18N("UPnP Remote UI Server") },
    {"Unknown", "device", QI18N("Unknown UPnP Device") },
    {"LANDevice1", "network-wired", QI18N("UPnP LAN Device") },
    {"WANDevice1", "network-wireless", QI18N("UPnP WAN Device") },
    {"WANConnectionDevice1", "network-wireless", QI18N("UPnP WAN Connection Device") },
    {"WFADevice1", "network-wireless", QI18N("UPnP Wi-Fi Alliance Device") }
};

static const int deviceDataCount = sizeof(deviceData) / sizeof(deviceData[0]);


KUPnPDevice::KUPnPDevice(const UPnP::Device& device)
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
    QString result = QString::fromLatin1("/org/kde/KUPnP");
    if( mDevice.isValid() )
        result += '/' + mDevice.udn();
    return result;
}

QString KUPnPDevice::parentUdi() const
{
    QString result =
        ! mDevice.isValid() ?
            QString() :
        mDevice.hasParentDevice() ?
            QString::fromLatin1("/org/kde/KUPnP") + '/' + mDevice.parentUdn() :
        /* else */
            QString::fromLatin1("/org/kde/KUPnP");
qDebug() << mDevice.displayName()<< result;
    return result;
}

QString KUPnPDevice::vendor() const
{
    return mDevice.isValid() ? QString("UPnP vendor") : QString(); // TODO: expose vendor in UPnP::Device
}

QString KUPnPDevice::product() const
{
    return mDevice.isValid() ? mDevice.displayName()/*QString("UPnP product")*/ : QString("UPnP devices"); // TODO: expose product in UPnP::Device
}

QString KUPnPDevice::icon() const
{
    const char* result;

    const bool isRootDevice = (!mDevice.isValid());
    if (isRootDevice) {
        result = "network-server";
    } else {
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
    return mDevice.displayName();
}


bool KUPnPDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    bool result = false;

    if (type==Solid::DeviceInterface::StorageAccess) {
        result = (mDevice.type() == QLatin1String("MediaServer1"));
    }

    return result;
}

QObject* KUPnPDevice::createDeviceInterface(const Solid::DeviceInterface::Type& type)
{
    if (!queryDeviceInterface(type)) {
        return 0;
    }

    DeviceInterface* interface = 0;

    switch (type)
    {
    case Solid::DeviceInterface::StorageAccess:
        if (mDevice.type() == QLatin1String("MediaServer1")) {
            interface = new StorageAccess(this);
        }
        break;
    case Solid::DeviceInterface::Unknown:
    case Solid::DeviceInterface::Last:
    default:
        break;
    }

    return interface;
}

QString KUPnPDevice::storageDescription() const
{
    QString description; //= QObject::tr("blah", "context");
    return description;
}

QString KUPnPDevice::volumeDescription() const
{
    QString description;
    return description;
}

}
}
}
