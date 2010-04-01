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
#include "kupnprootdevice.h"
#include "mediaserver1.h"
#include "mediaserver2.h"
#include "mediaserver3.h"
#include "internetgatewaydevice1.h"
#include "kupnpdevice.h"
#include "lib/devicebrowser.h"
#include "lib/device.h"
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
  : Solid::Ifaces::DeviceManager(parent),
    mDeviceBrowser( new UPnP::DeviceBrowser(QStringList(),this) ),
    mUdiPrefix( QString::fromLatin1(KUPnPUdiPrefix) )
{
    connect( mDeviceBrowser, SIGNAL(deviceAdded( const UPnP::Device& )),
             SLOT(onDeviceAdded( const UPnP::Device& )) );
    connect( mDeviceBrowser, SIGNAL(deviceRemoved( const UPnP::Device& )),
             SLOT(onDeviceRemoved( const UPnP::Device& )) );

    mDeviceFactories
        << new MediaServer1Factory()
        << new MediaServer2Factory()
        << new MediaServer3Factory()
        << new InternetGatewayDevice1Factory()
        // keep last:
        << new DeviceFactory();

    foreach( AbstractDeviceFactory* factory, mDeviceFactories )
        factory->addSupportedInterfaces( mSupportedInterfaces );
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
    return
        (!parentUdi.isEmpty()) ?
            findDeviceByParent(parentUdi,type) :
        (type!=Solid::DeviceInterface::Unknown) ?
            findDeviceByDeviceInterface(type) :
        /* else */
            allDevices();
}

QObject* KUPnPManager::createDevice(const QString &udi)
{
    QObject* result = 0;

    const QString udn = udnFromUdi( udi );
    if( udn.isEmpty() ) {
        result = new KUPnPRootDevice();
    } else {
        QList<UPnP::Device> devices = mDeviceBrowser->devices();
        foreach( const UPnP::Device& device, devices ) {
            if( device.udn() == udn ) {
                foreach( AbstractDeviceFactory* factory, mDeviceFactories ) {
                    result = factory->tryCreateDevice( device );
                    if( result != 0 )
                        break;
                }
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
    } else {
        const QString parentUdn = udnFromUdi( parentUdi );
        const QList<UPnP::Device> devices = mDeviceBrowser->devices();

        foreach( const UPnP::Device& device, devices ) {
            if ((parentUdn.isEmpty() && device.hasParentDevice())
                ||(! parentUdn.isEmpty() && device.parentDevice().udn() == parentUdn )) {
                continue;
            }
            foreach( AbstractDeviceFactory* factory, mDeviceFactories ) {
                if( factory->hasDeviceInterface(device,type) ) {
                    result << udiFromUdn( device.udn() );
                    break;
                }
            }
        }
    }

    return result;
}

QStringList KUPnPManager::findDeviceByDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    QStringList result;

    const QList<UPnP::Device> devices = mDeviceBrowser->devices();

    foreach( const UPnP::Device& device, devices ) {
        foreach( AbstractDeviceFactory* factory, mDeviceFactories ) {
            if( factory->hasDeviceInterface(device,type) ) {
                result << udiFromUdn( device.udn() );
                break;
            }
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
    qDeleteAll(mDeviceFactories);
}

}
}
}
