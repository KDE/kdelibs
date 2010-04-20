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

#ifndef SOLID_BACKENDS_HAL_HALDEVICE_H
#define SOLID_BACKENDS_HAL_HALDEVICE_H

#include <solid/ifaces/device.h>

class QDBusVariant;

namespace Solid
{
namespace Backends
{
namespace Hal
{
class HalManager;
class HalDevicePrivate;

struct ChangeDescription
{
    QString key;
    bool added;
    bool removed;
};

class HalDevice : public Solid::Ifaces::Device
{
    Q_OBJECT

public:
    HalDevice(const QString &udi);
    virtual ~HalDevice();

    virtual QString udi() const;
    virtual QString parentUdi() const;

    virtual QString vendor() const;
    virtual QString product() const;
    virtual QString icon() const;
    virtual QStringList emblems() const;
    virtual QString description() const;

    virtual bool queryDeviceInterface(const Solid::DeviceInterface::Type &type) const;
    virtual QObject *createDeviceInterface(const Solid::DeviceInterface::Type &type);

public:
    QVariant property(const QString &key) const;
    QMap<QString, QVariant> allProperties() const;
    bool propertyExists(const QString &key) const;

    void broadcastActionDone(const QString& signalName, int error, QVariant errorData, const QString &udi) const;
    void broadcastActionRequested(const QString& signalName) const;
    void connectActionSignal(const QString& signalName, QObject* dest, const char * slot) const;
    static QVariant variantFromDBusVariant(const QDBusVariant variant);

Q_SIGNALS:
    void propertyChanged(const QMap<QString,int> &changes);
    void conditionRaised(const QString &condition, const QString &reason);

private Q_SLOTS:
    void slotPropertyModified(int count, const QList<ChangeDescription> &changes);
    void slotCondition(const QString &condition, const QString &reason);

private:
    QString storageDescription() const;
    QString volumeDescription() const;
    QString deviceDBusPath() const;

    HalDevicePrivate *d;
};
}
}
}

#endif // SOLID_BACKENDS_HAL_HALDEVICE_H
