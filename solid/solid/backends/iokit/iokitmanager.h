/*  This file is part of the KDE project
    Copyright (C) 2009 Harald Fernengel <harry@kdevelop.org>

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

#ifndef SOLID_BACKENDS_IOKIT_IOKITMANAGER_H
#define SOLID_BACKENDS_IOKIT_IOKITMANAGER_H

#include <solid/ifaces/devicemanager.h>
#include <solid/deviceinterface.h>

#include <QtCore/QVariant>
#include <QtCore/QStringList>

namespace Solid
{
namespace Backends
{
namespace IOKit
{
class IOKitManagerPrivate;

class IOKitManager : public Solid::Ifaces::DeviceManager
{
    Q_OBJECT

public:
    IOKitManager(QObject *parent);
    virtual ~IOKitManager();

    virtual QString udiPrefix() const ;
    virtual QSet<Solid::DeviceInterface::Type> supportedInterfaces() const;

    virtual QStringList allDevices();
    virtual QStringList devicesFromQuery(const QString &parentUdi,
                                         Solid::DeviceInterface::Type type);
    virtual QObject *createDevice(const QString &udi);

private:
    IOKitManagerPrivate *d;
};
}
}
}

#endif // SOLID_BACKENDS_IOKIT_IOKITMANAGER_H

