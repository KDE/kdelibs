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

#ifndef UPNP_SERVICE_P_H
#define UPNP_SERVICE_P_H

// lib
#include "service.h"
#include "device.h"
// Qt
#include <QtCore/QString>
#include <QtCore/QSharedData>


namespace UPnP
{

class ServicePrivate : public QSharedData
{
  public:
    ServicePrivate();

    ~ServicePrivate();

  public:
    const Device& device() const;
    const QString& displayName() const;
    const QString& type() const;
    const QString& dBusPath() const;

  public:
    void setDevice( const Device& device );
    void setDisplayName( const QString& displayName );
    void setType( const QString& type );
    void setDBusPath( const QString& dBusPath );

  protected:
    Device mDevice;

    QString mDisplayName;
    QString mType;
    QString mDBusPath;
};


inline ServicePrivate::ServicePrivate() {}

inline const Device& ServicePrivate::device() const { return mDevice; }
inline const QString& ServicePrivate::displayName() const { return mDisplayName; }
inline const QString& ServicePrivate::type() const { return mType; }
inline const QString& ServicePrivate::dBusPath() const { return mDBusPath; }

inline void ServicePrivate::setDevice( const Device& device ) { mDevice = device; }
inline void ServicePrivate::setDisplayName( const QString& displayName ) { mDisplayName = displayName; }
inline void ServicePrivate::setType( const QString& type ) { mType = type; }
inline void ServicePrivate::setDBusPath( const QString& dBusPath ) { mDBusPath = dBusPath; }

inline ServicePrivate::~ServicePrivate() {}

}

#endif
