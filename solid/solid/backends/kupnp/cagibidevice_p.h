/*
    This file is part of the KUPnP library, part of the KDE project.

    Copyright 2009-2010 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#ifndef CAGIBIDEVICE_P_H
#define CAGIBIDEVICE_P_H

// lib
#include "cagibidevice.h"
// Qt
#include <QtCore/QString>
#include <QtCore/QSharedData>


namespace Cagibi
{

class DevicePrivate : public QSharedData
{
  friend const QDBusArgument& ::operator>>( const QDBusArgument& argument,
                                            Cagibi::Device& device );

  public:
    DevicePrivate();

  public:
    const QString& type() const;
    const QString& friendlyName() const;
    const QString& manufacturerName() const;
//     const QString& manufacturerUrl() const;
    const QString& modelDescription() const;
    const QString& modelName() const;
    const QString& modelNumber() const;
    const QString& serialNumber() const;
    const QString& udn() const;
//     const QString upc() const;
    const QString& presentationUrl() const;
    const QString& ipAddress() const;
    int ipPortNumber() const;

    bool hasParentDevice() const;
    const QString& parentDeviceUdn() const;

  protected:
    QString mType;
    /// short user-friendly title
    QString mFriendlyName;
    QString mManufacturerName;
//     QString mManufacturerUrl;
    /// long user-friendly title
    QString mModelDescription;
    QString mModelName;
    QString mModelNumber;
    QString mSerialNumber;
    QString mUdn;
//     QString mUpc; Universal Product Code;
    QString mPresentationUrl;

    QString mIpAddress;
    int mIpPortNumber;

    QString mParentDeviceUdn;
};


inline DevicePrivate::DevicePrivate() {}
inline const QString& DevicePrivate::type() const { return mType; }
inline const QString& DevicePrivate::friendlyName() const { return mFriendlyName; }
inline const QString& DevicePrivate::manufacturerName() const { return mManufacturerName; }
inline const QString& DevicePrivate::modelDescription() const { return mModelDescription; }
inline const QString& DevicePrivate::modelName() const { return mModelName; }
inline const QString& DevicePrivate::modelNumber() const { return mModelNumber; }
inline const QString& DevicePrivate::serialNumber() const { return mSerialNumber; }
inline const QString& DevicePrivate::udn() const { return mUdn; }
inline const QString& DevicePrivate::presentationUrl() const { return mPresentationUrl; }
inline const QString& DevicePrivate::ipAddress() const { return mIpAddress; }
inline int DevicePrivate::ipPortNumber() const { return mIpPortNumber; }
inline bool DevicePrivate::hasParentDevice() const { return (! mParentDeviceUdn.isEmpty() ); }
inline const QString& DevicePrivate::parentDeviceUdn() const { return mParentDeviceUdn; }

}

#endif
