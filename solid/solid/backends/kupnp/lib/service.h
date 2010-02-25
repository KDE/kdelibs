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

#ifndef UPNP_SERVICE_H
#define UPNP_SERVICE_H

// lib
#include "upnp_export.h"
// #include "device.h"
// Qt
#include <QtCore/QExplicitlySharedDataPointer>


namespace UPnP
{
class ServicePrivate;
class Device;


class KUPNP_EXPORT Service
{
  friend class ServicePrivate;
  friend class DeviceBrowserPrivate;

  protected:
    explicit Service( ServicePrivate* d );

  public:
    Service();
    Service( const Service& other );

    virtual ~Service();

  public:
//     QString udn() const;
    QString displayName() const;
    QString type() const;
    Device device() const;

  public:
    Service& operator =( const Service& other );

  private:
    ServicePrivate* dPtr() const;

  protected:
    QExplicitlySharedDataPointer<ServicePrivate> d;
};


inline  ServicePrivate* Service::dPtr() const { return const_cast<ServicePrivate*>( d.data() ); }

}

#endif
