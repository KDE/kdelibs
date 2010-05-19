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
#include "cagibidbuscodec.h"
// Qt
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtCore/QDebug>


namespace Solid
{
namespace Backends
{
namespace KUPnP
{

const char KUPnPUdiPrefix[] = "/org/kde/KUPnP";
static const int KUPnPUdiPrefixLength = sizeof( KUPnPUdiPrefix ); // count final \0 for / separator


KUPnPManager::KUPnPManager(QObject* parent)
  : Solid::Ifaces::DeviceManager(parent),
    mUdiPrefix( QString::fromLatin1(KUPnPUdiPrefix) )
{
    qDBusRegisterMetaType<DeviceTypeMap>();
    qDBusRegisterMetaType<Cagibi::Device>();

    QDBusConnection dbusConnection = QDBusConnection::sessionBus();

    mDBusCagibiProxy =
        new QDBusInterface("org.kde.Cagibi",
                           "/org/kde/Cagibi",
                           "org.kde.Cagibi",
                           dbusConnection, this);
    dbusConnection.connect("org.kde.Cagibi",
                           "/org/kde/Cagibi",
                           "org.kde.Cagibi",
                           "devicesAdded",
                           this, SLOT(onDevicesAdded( const DeviceTypeMap& )) );
    dbusConnection.connect("org.kde.Cagibi",
                           "/org/kde/Cagibi",
                           "org.kde.Cagibi",
                           "devicesRemoved",
                           this, SLOT(onDevicesRemoved( const DeviceTypeMap& )) );

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

    QDBusReply<DeviceTypeMap> reply =
        mDBusCagibiProxy->asyncCall("allDevices");

    if( reply.isValid() )
    {
        const DeviceTypeMap deviceTypeMap = reply;
        DeviceTypeMap::ConstIterator it = deviceTypeMap.constBegin();
        DeviceTypeMap::ConstIterator end = deviceTypeMap.constEnd();
        for( ; it != end; ++it )
            result << udiFromUdn( it.key() );
    }
    else
        qWarning() << Q_FUNC_INFO << " error: " << reply.error().name() << endl;

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

QObject* KUPnPManager::createDevice(const QString& udi)
{
    QObject* result = 0;

    const QString udn = udnFromUdi( udi );
    if( udn.isEmpty() ) {
        result = new KUPnPRootDevice();
    } else {
        QDBusReply<Cagibi::Device> reply =
            mDBusCagibiProxy->asyncCall("deviceDetails",udn);

        if( reply.isValid() )
        {
            Cagibi::Device device = reply;
qDebug() << "device of type: "<<device.type();
            foreach( AbstractDeviceFactory* factory, mDeviceFactories ) {
                result = factory->tryCreateDevice( device );
                if( result != 0 )
                    break;
            }
        }
        else
            qWarning() << Q_FUNC_INFO << " error: " << reply.error().name() << endl;
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

        QDBusReply<DeviceTypeMap> reply =
            mDBusCagibiProxy->asyncCall("devicesByParent",parentUdn); // TODO: optional recursive?

        if( reply.isValid() )
        {
            DeviceTypeMap deviceTypeMap = reply;

            foreach( AbstractDeviceFactory* factory, mDeviceFactories ) {
                const QStringList typeNames = factory->typeNames( type );
                foreach( const QString& typeName, typeNames ) {
                    DeviceTypeMap::Iterator it = deviceTypeMap.begin();
                    while( it != deviceTypeMap.end() ) {
                        if( it.value() == typeName ) {
                            result << udiFromUdn( it.key() );
                            // to prevent double inclusion remove the device
                            it = deviceTypeMap.erase( it );
                        } else
                            ++it;
                    }
                }
            }
        }
        else
            qWarning() << Q_FUNC_INFO << " error: " << reply.error().name() << endl;
    }

    return result;
}

QStringList KUPnPManager::findDeviceByDeviceInterface(Solid::DeviceInterface::Type type)
{
    QStringList result;

    foreach( AbstractDeviceFactory* factory, mDeviceFactories ) {
        const QStringList typeNames = factory->typeNames( type );
        foreach( const QString& typeName, typeNames ) {
            QDBusReply<DeviceTypeMap> reply =
                mDBusCagibiProxy->asyncCall("devicesByType",typeName);

            if( reply.isValid() )
            {
                const DeviceTypeMap deviceTypeMap = reply;

                DeviceTypeMap::ConstIterator it = deviceTypeMap.constBegin();
                DeviceTypeMap::ConstIterator end = deviceTypeMap.constEnd();
                for( ; it != end; ++it )
                    result << udiFromUdn( it.key() );
            }
            else
                qWarning() << Q_FUNC_INFO << " error: " << reply.error().name() << endl;
        }
    }

    return result;
}

QString KUPnPManager::udiFromUdn( const QString& udn ) const { return mUdiPrefix + '/' + udn; }
QString KUPnPManager::udnFromUdi( const QString& udi ) const { return udi.mid( KUPnPUdiPrefixLength ); }

void KUPnPManager::onDevicesAdded( const DeviceTypeMap& deviceTypeMap )
{
    DeviceTypeMap::ConstIterator it = deviceTypeMap.constBegin();
    DeviceTypeMap::ConstIterator end = deviceTypeMap.constEnd();
    for( ; it != end; ++it )
        emit deviceAdded( udiFromUdn(it.key()) );
}

void KUPnPManager::onDevicesRemoved( const DeviceTypeMap& deviceTypeMap )
{
    DeviceTypeMap::ConstIterator it = deviceTypeMap.constBegin();
    DeviceTypeMap::ConstIterator end = deviceTypeMap.constEnd();
    for( ; it != end; ++it )
        emit deviceRemoved( udiFromUdn(it.key()) );
}


KUPnPManager::~KUPnPManager()
{
    qDeleteAll(mDeviceFactories);
}

}
}
}
