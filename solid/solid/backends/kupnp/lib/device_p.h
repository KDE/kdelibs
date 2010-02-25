/*
    This file is part of the KUPnP library, part of the KDE project.

    Copyright 2009 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#ifndef UPNP_DEVICE_P_H
#define UPNP_DEVICE_P_H

// lib
#include "device.h"
#include "service.h"
// Qt
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QSharedData>


namespace UPnP
{

class DevicePrivate : public QSharedData
{
  public:
    DevicePrivate();

    ~DevicePrivate();

  public:
    const QString& udn() const;
    const QString& displayName() const;
    const QString& type() const;
    const QString& ipAddress() const;
    int ipPortNumber() const;
    const QString& presentationUrl() const;
    const QString& dBusPath() const;
    const QList<Service>& services() const;
    Device parentDevice() const;
    const QList<Device>& childDevices() const;

    bool hasParentDevice() const;
    bool isValid() const;

  public:
    void setUdn( const QString& udn );
    void setType( const QString& deviceType );
    void setDisplayName( const QString& displayName );
    void setIpAddress( const QString& ipAddress );
    void setIpPortNumber( int portNumber );
    void setPresentationUrl( const QString& presentationUrl );
    void setDBusPath( const QString& dBusPath );
    void setServices( const QList<Service>& services );
    void setParentDevice( DevicePrivate* parentDevicePrivate );

    void setInvalid();

    void addChildDevice( DevicePrivate* childDevice );
    void removeChildDevice( DevicePrivate* childDevice );

  protected:
    // TODO: could just be a QByteArray
    QString mUdn;
    QString mDeviceType;
    QString mDisplayName;
    QString mIpAddress;
    int mPortNumber;
    QString mPresentationUrl;
    QString mDBusPath;

    QList<Service> mServices;

    //just pointer to private to avoid circular refcounting
    DevicePrivate* mParentDevicePrivate;
    QList<Device> mChildDevices;
};


inline DevicePrivate::DevicePrivate()
 : mParentDevicePrivate( 0 )
{}

inline const QString& DevicePrivate::udn() const { return mUdn; }
inline const QString& DevicePrivate::type() const { return mDeviceType; }
inline const QString& DevicePrivate::displayName() const { return mDisplayName; }
inline const QString& DevicePrivate::ipAddress() const { return mIpAddress; }
inline int DevicePrivate::ipPortNumber() const { return mPortNumber; }
inline const QString& DevicePrivate::presentationUrl() const { return mPresentationUrl; }
inline const QString& DevicePrivate::dBusPath() const { return mDBusPath; }
inline const QList<Service>& DevicePrivate::services() const { return mServices; }
inline Device DevicePrivate::parentDevice() const { return Device(mParentDevicePrivate); }
inline const QList<Device>& DevicePrivate::childDevices() const { return mChildDevices; }
inline bool DevicePrivate::hasParentDevice() const { return (mParentDevicePrivate != 0); }
inline bool DevicePrivate::isValid() const { return ! mUdn.isEmpty(); }

inline void DevicePrivate::setUdn( const QString& udn ) { mUdn = udn; }
inline void DevicePrivate::setType( const QString& deviceType ) { mDeviceType = deviceType; }
inline void DevicePrivate::setDisplayName( const QString& displayName ) { mDisplayName = displayName; }
inline void DevicePrivate::setIpAddress( const QString& ipAddress ) { mIpAddress = ipAddress; }
inline void DevicePrivate::setIpPortNumber( int portNumber ) { mPortNumber = portNumber; }
inline void DevicePrivate::setPresentationUrl( const QString& presentationUrl ) { mPresentationUrl = presentationUrl; }
inline void DevicePrivate::setDBusPath( const QString& dBusPath ) { mDBusPath = dBusPath; }
inline void DevicePrivate::setServices( const QList<Service>& services ) { mServices = services; }
inline void DevicePrivate::setParentDevice( DevicePrivate* parentDevicePrivate )
{
    mParentDevicePrivate = parentDevicePrivate;
    if( mParentDevicePrivate )
        mParentDevicePrivate->addChildDevice( this );
}

inline void DevicePrivate::setInvalid() { mUdn.clear(); }

inline void DevicePrivate::addChildDevice( DevicePrivate* childDevice )
{
    mChildDevices.append( Device(childDevice) );
}
inline void DevicePrivate::removeChildDevice( DevicePrivate* childDevice )
{
    mChildDevices.removeOne( Device(childDevice) );
}

inline DevicePrivate::~DevicePrivate()
{
    if( mParentDevicePrivate )
        mParentDevicePrivate->removeChildDevice( this );
    foreach( const Device& device, mChildDevices )
        device.dPtr()->setParentDevice( 0 );
}

}

#endif
