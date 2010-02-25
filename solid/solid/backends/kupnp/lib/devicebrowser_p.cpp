/*
    This file is part of the KUPnP library, part of the KDE project.

    Copyright 2009-2010 Friedrich W. H. Kossebau <kossebau@kde.org>
    Copyright 2009 Adriaan de Groot <groot@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as argument proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received argument copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "devicebrowser_p.h"

// lib
#include "device_p.h"
#include "service_p.h"
// Qt
#include <QtDBus/QDBusArgument>

#include <QtCore/QDebug>

namespace UPnP
{

static const char requiredCoherenceVersionString[] = "0.6.5";
static const int requiredCoherenceVersion[] = {0,6,5};
static const int requiredCoherenceVersionLength =
    sizeof(requiredCoherenceVersion) / sizeof(requiredCoherenceVersion[0]);


static bool checkVersion( const QString& coherenceVersionString )
{
    const QStringList coherenceVersionParts = coherenceVersionString.split( '.' );

    for( int i=0; i<requiredCoherenceVersionLength; ++i )
    {
        const int versionPart = coherenceVersionParts[i].toInt();
        if( requiredCoherenceVersion[i] < versionPart )
            break;

        if( requiredCoherenceVersion[i] > versionPart )
        {
qDebug() << "Coherence version" << coherenceVersionString << "is not ok,"
         << requiredCoherenceVersionString << "required." << endl;
            return false;
        }
    }
qDebug() << "Coherence version" << coherenceVersionString << "is ok.";

    return true;
}

QList<UPnP::Service> DeviceBrowserPrivate::demarshallServices( const Device& device, const QVariant& variant )
{
    QList<UPnP::Service> result;

    const QStringList serviceDBusPathList = variant.value<QStringList>();
    foreach( const QString& serviceDBusPath, serviceDBusPathList )
    {
        const QString typeName = serviceDBusPath.mid( serviceDBusPath.lastIndexOf('/')+1 );

        ServicePrivate* servicePrivate = new ServicePrivate();
        servicePrivate->setDevice( device );
        servicePrivate->setDisplayName( typeName );
        servicePrivate->setType( typeName );
        servicePrivate->setDBusPath( serviceDBusPath );

        result.append( Service(servicePrivate) );
    }

    return result;
}

Device DeviceBrowserPrivate::addDevice( const QDBusArgument& dBusArgument )
{
    DevicePrivate* devicePrivate = new DevicePrivate();
    Device device( devicePrivate );

    QString parentUdn;

    dBusArgument.beginMap();
    while( ! dBusArgument.atEnd() )
    {
        dBusArgument.beginMapEntry();
        QString key;
        QDBusVariant dBusVariant;
        dBusArgument >> key >> dBusVariant;
        const QVariant variant = dBusVariant.variant();

        if( key == QLatin1String("udn") )
        {
            const QString uuid = variant.value<QString>().mid( 5 );
            devicePrivate->setUdn( uuid );
        }
        else if( key == QLatin1String("friendly_name") )
            devicePrivate->setDisplayName( variant.value<QString>() );
        else if( key == QLatin1String("parent_udn") )
            parentUdn = variant.value<QString>().mid( 5 );
        else if( key == QLatin1String("device_type") )
        {
            const QStringList typeParts = variant.value<QString>().split( ':' );
            devicePrivate->setType( typeParts[3]+typeParts[4] );
        }
        else if( key == QLatin1String("path") )
            devicePrivate->setDBusPath( variant.value<QString>() );
        else if( key == QLatin1String("presentation_url") )
            devicePrivate->setPresentationUrl( variant.value<QString>() );
        else if( key == QLatin1String("uri") )
        {
            const QString hostAddressAndPort = variant.value<QStringList>()[1];
            const int indexOfSeparator = hostAddressAndPort.indexOf( ':' );
            const QString hostAddress = hostAddressAndPort.left( indexOfSeparator );
            const int portNumber = hostAddressAndPort.mid( indexOfSeparator+1 ).toInt();
            devicePrivate->setIpAddress( hostAddress );
            devicePrivate->setIpPortNumber( portNumber );
        }
        else if( key == QLatin1String("services") )
        {
            QList<Service> services = demarshallServices( device, variant );
            devicePrivate->setServices( services );
        }
//         else
        {
            if( variant.canConvert<QString>())
            {
                const QString value = variant.value<QString>();
qDebug() << "  " << key << value;
            }
            else
            {
qDebug() << "  " << key << "-not argument string-";
            }
        }
        dBusArgument.endMapEntry();
    }
    dBusArgument.endMap();

    const QString& udn = devicePrivate->udn();
    if( udn.isEmpty() )
    {
qDebug()<<"No udn found!";
        devicePrivate->setInvalid();
    }
    else if( mDevices.contains(udn) )
    {
qDebug()<<"Already inserted:"<<udn<<"!";
        devicePrivate->setInvalid();
    }
    else if( ! mBrowsedDeviceTypes.isEmpty() && ! mBrowsedDeviceTypes.contains(devicePrivate->type()) )
    {
qDebug()<<"Not interested in:"<<devicePrivate->type();
        devicePrivate->setInvalid();
    }
    else
    {
qDebug()<<"Adding: "<<device.displayName()<<udn;
        mDevices[udn] = device;

        if( ! parentUdn.isEmpty() )
        {
            if( mDevices.contains(parentUdn) )
            {
                devicePrivate->setParentDevice( mDevices[parentUdn].dPtr() );
                // TODO: else set to pending devices waiting for their parent device streamed
                // but how to find about orphaned devices? might increase by time if the backend is broken
                // what to do if devices browsed are filtered? set a flag like wouldHaveParent?
            }
        }
    }

    return device;
}



void DeviceBrowserPrivate::init()
{
qDebug() << "Connecting to Coherence...";

    // TODO: or use system bus?
    mCoherence = new org::Coherence( "org.Coherence", "/org/Coherence", QDBusConnection::sessionBus()/*systemBus*/, q );

    QDBusPendingReply<QString> versionReply = mCoherence->version();
    versionReply.waitForFinished();
    if( versionReply.isError() )
    {
qDebug()<< versionReply.error();
        return;
    }

    const QString version = versionReply.value();

    const bool versionOk = checkVersion( version );
    if( ! versionOk )
        return;

    connect( mCoherence, SIGNAL(device_detected( const QDBusVariant&, const QString& )),
             SLOT(onDeviceAdded( const QDBusVariant&, const QString& )) );

    connect( mCoherence, SIGNAL(device_removed(const QString& )),
             SLOT(onDeviceRemoved( const QString& )) );

    QDBusPendingReply<QVariantList> devicesReply = mCoherence->get_devices();
    devicesReply.waitForFinished();
    if( devicesReply.isError() )
    {
qDebug()<< devicesReply.error();
        return;
    }

qDebug()<< "Current devices...";
    QVariantList devicesReplyValue = devicesReply.value();
    foreach( const QVariant& devicesReplyValueItem, devicesReplyValue )
    {
        const QVariant variant = qvariant_cast<QDBusVariant>( devicesReplyValueItem ).variant();
        const QDBusArgument dBusArgument = variant.value<QDBusArgument>();
        addDevice( dBusArgument );
    }
qDebug()<<"That's all.";
}

QList<Device> DeviceBrowserPrivate::devices() const
{
    QList<Device> result;

    QHashIterator<QString,Device> it( mDevices);
    while( it.hasNext() )
    {
        it.next();
        result.append( it.value() );
    }

    return result;
}

void DeviceBrowserPrivate::onDeviceAdded( const QDBusVariant& dBusVariant, const QString& udn )
{
    Q_UNUSED( udn );

    const QVariant variant = dBusVariant.variant();
    const QDBusVariant sv = variant.value<QDBusVariant>();
    const QVariant v = sv.variant();
    const QDBusArgument dBusArgument = v.value<QDBusArgument>();

    Device device = addDevice( dBusArgument );
    if( device.isValid() )
        emit q->deviceAdded( device );
}

void DeviceBrowserPrivate::onDeviceRemoved( const QString& _udn )
{
    const QString udn = _udn.mid( 5 );
    QHash<QString,Device>::Iterator it = mDevices.find( udn );
    if( it != mDevices.end() )
    {
        Device device = it.value();
        mDevices.erase( it );
qDebug() << "Removing"<<device.displayName();
        emit q->deviceRemoved( device );
    }
    else
qDebug() << "Not found in device list:"<<udn;
}


DeviceBrowserPrivate::~DeviceBrowserPrivate()
{
    delete mCoherence;
}


}
