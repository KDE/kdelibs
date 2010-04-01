/*  This file is part of the KDE project
    Copyright (C) 2005,2006 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef SOLID_BACKENDS_WMI_WMIMANAGER_H
#define SOLID_BACKENDS_WMI_WMIMANAGER_H

#include <solid/ifaces/devicemanager.h>
#include <solid/deviceinterface.h>

#include <QtCore/QVariant>
#include <QtCore/QStringList>

namespace Solid
{
namespace Backends
{
namespace Wmi
{
class WmiManagerPrivate;

class WmiManager : public Solid::Ifaces::DeviceManager
{
    Q_OBJECT

public:
    WmiManager(QObject *parent);
    virtual ~WmiManager();

    virtual QString udiPrefix() const ;
    virtual QSet<Solid::DeviceInterface::Type> supportedInterfaces() const;

    virtual QStringList allDevices();
    virtual bool deviceExists(const QString &udi);

    virtual QStringList devicesFromQuery(const QString &parentUdi,
                                         Solid::DeviceInterface::Type type);

    virtual QObject *createDevice(const QString &udi);


private Q_SLOTS:
    void slotDeviceAdded(const QString &udi);
    void slotDeviceRemoved(const QString &udi);

private:
    QStringList findDeviceStringMatch(const QString &key, const QString &value);
    QStringList findDeviceByDeviceInterface(Solid::DeviceInterface::Type type);

    WmiManagerPrivate *d;
};
}
}
}

#endif // SOLID_BACKENDS_WMI_WMIMANAGER_H
