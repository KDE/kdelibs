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

#ifndef UPNP_DEVICEBROWSER_P_H
#define UPNP_DEVICEBROWSER_P_H

// lib
#include "devicebrowser.h"
#include "device.h"
#include "coherence_interface.h"


namespace UPnP
{

class DeviceBrowserPrivate : public QObject
{
  Q_OBJECT

  protected:
    static QList<Service> demarshallServices( const Device& device, const QVariant& variant );

  public:
    explicit DeviceBrowserPrivate( DeviceBrowser* q, const QStringList& deviceTypes );
    explicit DeviceBrowserPrivate( DeviceBrowser* q, const QString& deviceType );

    ~DeviceBrowserPrivate();

  public:
    QList<Device> devices() const;
    const QStringList& browsedDeviceTypes() const;

  public:
    void init();

  protected:
    Device addDevice( const QDBusArgument& dbusArgument );

  protected Q_SLOTS:
    void onDeviceAdded( const QDBusVariant&, const QString& udn );
    void onDeviceRemoved( const QString& udn );

  protected:
    DeviceBrowser* const q;

    org::Coherence* mCoherence;

    QStringList mBrowsedDeviceTypes;
    QHash<QString,Device> mDevices;
};


inline DeviceBrowserPrivate::DeviceBrowserPrivate( DeviceBrowser* _q, const QStringList& deviceTypes )
  : q( _q ),
    mCoherence( 0 ),
    mBrowsedDeviceTypes( deviceTypes )
{}
inline DeviceBrowserPrivate::DeviceBrowserPrivate( DeviceBrowser* _q, const QString& deviceType )
  : q( _q ),
    mCoherence( 0 )
{
    mBrowsedDeviceTypes.append( deviceType );
}

inline const QStringList& DeviceBrowserPrivate::browsedDeviceTypes() const { return mBrowsedDeviceTypes; }

}

#endif
