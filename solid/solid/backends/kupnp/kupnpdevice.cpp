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


namespace Solid
{
namespace Backends
{
namespace KUPnP
{

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
    return mDevice.isValid() ? QString::fromLatin1("/org/kde/KUPnP") : QString();
}

QString KUPnPDevice::vendor() const
{
    return QString("UPnP vendor"); // TODO: expose vendor in UPnP::Device
}

QString KUPnPDevice::product() const
{
    return mDevice.isValid() ? mDevice.displayName()/*QString("UPnP product")*/ : QString("UPnP devices"); // TODO: expose product in UPnP::Device
}

QString KUPnPDevice::icon() const
{
    const char* iconName =
        !mDevice.isValid() ? "network-server" :
        (mDevice.type() == QLatin1String("MediaServer1")) ? "folder-remote" :
        (mDevice.type() == QLatin1String("InternetGatewayDevice1")) ? "network-server" :
        (mDevice.type() == QLatin1String("WANConnectionDevice1")) ? "network-wireless" :
        (mDevice.type() == QLatin1String("WANDevice1")) ? "network-wireless" :
        (mDevice.type() == QLatin1String("LANDevice1")) ? "network-server" :
                                                           "network-server";

    return QString::fromLatin1(iconName);
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
