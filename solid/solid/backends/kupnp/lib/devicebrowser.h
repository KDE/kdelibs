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

#ifndef UPNP_DEVICEBROWSER_H
#define UPNP_DEVICEBROWSER_H

// lib
#include "upnp_export.h"
// Qt
#include <QtCore/QStringList>
#include <QtCore/QObject>

template<class T> class QList;


namespace UPnP
{
class DeviceBrowserPrivate;
class Device;


class KUPNP_EXPORT DeviceBrowser : public QObject
{
  Q_OBJECT

  friend class DeviceBrowserPrivate;

  public:
    explicit DeviceBrowser( const QStringList& deviceTypes = QStringList(), QObject* parent = 0 );
    explicit DeviceBrowser( const QString& deviceType, QObject* parent = 0 );

    virtual ~DeviceBrowser();

  public:
    QList<Device> devices() const;

    QStringList browsedDeviceTypes() const;

  Q_SIGNALS:
    void deviceAdded( const UPnP::Device& device );
    void deviceRemoved( const UPnP::Device& device );

  protected:
    DeviceBrowserPrivate* const d;
};

}

#endif
