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

#ifndef UPNP_DEVICE_H
#define UPNP_DEVICE_H

// lib
#include "upnp_export.h"
// Qt
#include <QtCore/QExplicitlySharedDataPointer>

template<class T> class QList;


namespace UPnP
{
class DevicePrivate;
class Service;


class KUPNP_EXPORT Device
{
  friend class ServicePrivate;
  friend class DevicePrivate;
  friend class DeviceBrowserPrivate;

  protected:
    explicit Device( DevicePrivate* d );

  public:
    Device();
    Device( const Device& other );

    virtual ~Device();

  public:
    /// without the prefix "uuid:"
    QString udn() const;
    QString displayName() const;
    QString type() const;
    QString ipAddress() const;
    int ipPortNumber() const;
    QString presentationUrl() const;
    QList<Service> services() const;
    Device parentDevice() const;
    QList<Device> childDevices() const;

    bool hasParentDevice() const;
    bool isValid() const;

  public:
    Device& operator =( const Device& other );
    bool operator==( const Device& other ) const;

  private:
    DevicePrivate* dPtr() const;

  protected:
    QExplicitlySharedDataPointer<DevicePrivate> d;
};


inline  DevicePrivate* Device::dPtr() const { return const_cast<DevicePrivate*>( d.data() ); }

}

#endif
