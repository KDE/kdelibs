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

#ifndef CAGIBIDEVICE_H
#define CAGIBIDEVICE_H

// Qt
#include <QtCore/QExplicitlySharedDataPointer>

namespace Cagibi { class Device; }
class QString;
class QDBusArgument;
extern QDBusArgument& operator<<( QDBusArgument& argument,
                                  const Cagibi::Device& device );
extern const QDBusArgument& operator>>( const QDBusArgument& argument,
                                        Cagibi::Device& device );


namespace Cagibi
{
class DevicePrivate;

class Device
{
  friend QDBusArgument& ::operator<<( QDBusArgument& argument,
                                      const Cagibi::Device& device );
  friend const QDBusArgument& ::operator>>( const QDBusArgument& argument,
                                            Cagibi::Device& device );

  protected:
    explicit Device( DevicePrivate* _d );

  public:
    Device();
    Device( const Device& other );

    ~Device();

  public:
    Device& operator=( const Device& other );

  public:
    const QString& type() const;
    const QString& friendlyName() const;
    const QString& manufacturerName() const;
//     QString manufacturerUrl() const;
    const QString& modelDescription() const;
    const QString& modelName() const;
    const QString& modelNumber() const;
    const QString& serialNumber() const;
    const QString& udn() const;
//     QString upc() const;
    const QString& presentationUrl() const;
    const QString& ipAddress() const;
    int ipPortNumber() const;

    bool hasParentDevice() const;
    const QString& parentDeviceUdn() const;

  protected:
    QExplicitlySharedDataPointer<DevicePrivate> d;
};

}

#endif
