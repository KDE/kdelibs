/*  This file is part of the KDE project
    Copyright (C) 2009 Pino Toscano <pino@kde.org>

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

#ifndef SOLID_BACKENDS_UPOWER_GENERICINTERFACE_H
#define SOLID_BACKENDS_UPOWER_GENERICINTERFACE_H

#include <solid/ifaces/genericinterface.h>
#include <solid/genericinterface.h>
#include "upowerdeviceinterface.h"

namespace Solid
{
namespace Backends
{
namespace UPower
{
class UPowerDevice;

class GenericInterface : public DeviceInterface, virtual public Solid::Ifaces::GenericInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::GenericInterface)

public:
    GenericInterface(UPowerDevice *device);
    virtual ~GenericInterface();

    virtual QVariant property(const QString &key) const;
    virtual QMap<QString, QVariant> allProperties() const;
    virtual bool propertyExists(const QString &key) const;

Q_SIGNALS:
    void propertyChanged(const QMap<QString, int> &changes);
    void conditionRaised(const QString &condition, const QString &reason);
};
}
}
}

#endif // SOLID_BACKENDS_UPOWER_GENERICINTERFACE_H
