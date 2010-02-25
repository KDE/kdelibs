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

#include "kupnpmanager.h"

// backend
#include "lib/devicebrowser.h"
#include "lib/device.h"
#include "kupnpdevice.h"
// Qt
#include <QtCore/QDebug>


namespace Solid
{
namespace Backends
{
namespace KUPnP
{

const char KUPnPUdiPrefix[] = "/org/kde/KUPnP";
static const int KUPnPUdiPrefixLength = sizeof( KUPnPUdiPrefix ); // count final \0 for / separator


KUPnPManager::KUPnPManager(QObject *parent)
  : DeviceManager(parent),
    mDeviceBrowser( new UPnP::DeviceBrowser(QString::fromLatin1("MediaServer1"),this) ),
    mUdiPrefix( QString::fromLatin1(KUPnPUdiPrefix) )
{
    connect( mDeviceBrowser, SIGNAL(deviceAdded( const UPnP::Device& )),
             SLOT(onDeviceAdded( const UPnP::Device& )) );
    connect( mDeviceBrowser, SIGNAL(deviceRemoved( const UPnP::Device& )),
             SLOT(onDeviceRemoved( const UPnP::Device& )) );

    mSupportedInterfaces << Solid::DeviceInterface::StorageAccess;
}


QString KUPnPManager::udiPrefix() const
{
    return mUdiPrefix;
}

QSet<Solid::DeviceInterface::Type> KUPnPManager::supportedInterfaces() const
{
    return mSupportedInterfaces;
}

QStringList KUPnPManager::allDevices()
{
    QStringList result;
    result << mUdiPrefix; // group parent
    const QList<UPnP::Device> devices = mDeviceBrowser->devices();
    foreach( const UPnP::Device& device, devices )
        result << udiFromUdn( device.udn() );

    return result;
}


QStringList KUPnPManager::devicesFromQuery( const QString& parentUdi,
                                            Solid::DeviceInterface::Type type)
{
    QStringList result;
    if (!parentUdi.isEmpty())
    {
        result = findDeviceByParent(parentUdi,type);
    } else if (type!=Solid::DeviceInterface::Unknown) {
        return findDeviceByDeviceInterface(type);
    } else {
        return allDevices();
    }

    return result;
}

QObject* KUPnPManager::createDevice(const QString &udi)
{
    QObject* result = 0;

    const QString udn = udnFromUdi( udi );
    if( udn.isEmpty() ) {
        result = new KUPnPDevice( UPnP::Device() );
    } else {
        QList<UPnP::Device> devices = mDeviceBrowser->devices();
        foreach( const UPnP::Device& device, devices ) {
            if( device.udn() == udn ) {
                result = new KUPnPDevice( device );
                break;
            }
        }
    }

    return result;
}

QStringList KUPnPManager::findDeviceByParent(const QString& parentUdi,
                                             Solid::DeviceInterface::Type type)
{
    QStringList result;

    if( parentUdi.isEmpty() ) {
        if (type!=Solid::DeviceInterface::Unknown) {
            result << mUdiPrefix;
        }
    } else if (parentUdi == mUdiPrefix) {
        QList<UPnP::Device> devices = mDeviceBrowser->devices();
        foreach( const UPnP::Device& device, devices ) {
            if (type==Solid::DeviceInterface::StorageAccess) {
                result << udiFromUdn( device.udn() );
            }
        }
    }

    return result;
}

QStringList KUPnPManager::findDeviceByDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    QStringList result;

    if (type == Solid::DeviceInterface::StorageAccess) {
        QList<UPnP::Device> devices = mDeviceBrowser->devices();
        foreach( const UPnP::Device& device, devices ) {
            result << udiFromUdn( device.udn() );
        }
    }

    return result;
}

QString KUPnPManager::udiFromUdn( const QString& udn ) const { return mUdiPrefix + '/' + udn; }
QString KUPnPManager::udnFromUdi( const QString& udi ) const { return udi.mid( KUPnPUdiPrefixLength ); }

void KUPnPManager::onDeviceAdded( const UPnP::Device& device )
{
    emit deviceAdded( udiFromUdn(device.udn()) );
}

void KUPnPManager::onDeviceRemoved( const UPnP::Device& device )
{
    emit deviceRemoved( udiFromUdn(device.udn()) );
}


KUPnPManager::~KUPnPManager()
{
}

}
}
}
