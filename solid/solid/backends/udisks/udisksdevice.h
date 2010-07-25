/*  Copyright 2010  Michael Zanetti <mzanetti@kde.org>

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
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef UDISKSDEVICE_H
#define UDISKSDEVICE_H

#include <ifaces/device.h>

#include <QtDBus/QDBusInterface>
#include <QtCore/QSet>

namespace Solid
{
namespace Backends
{
namespace UDisks
{

class UDisksDevice : public Solid::Ifaces::Device
{

public:
    UDisksDevice(const QString &udi);
    virtual ~UDisksDevice();

    
    virtual QObject* createDeviceInterface(const Solid::DeviceInterface::Type& type);
    virtual bool queryDeviceInterface(const Solid::DeviceInterface::Type& type) const;
    virtual QString description() const;
    virtual QStringList emblems() const;
    virtual QString icon() const;
    virtual QString product() const;
    virtual QString vendor() const;
    virtual QString udi() const;
    virtual QString parentUdi() const;

    QVariant property(const QString &key) const;
    
private:
    mutable QDBusInterface m_device;
    QString m_udi;
    mutable QMap<QString,QVariant> m_cache;
    mutable QSet<QString> m_invalidKeys;
    
    
    void checkCache(const QString &key) const;

};

}
}
}

#endif // UDISKSDEVICE_H
