/*
    Copyright 2006 Michaël Larouche <michael.larouche@kdemail.net>

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
#ifndef SOLID_BACKENDS_FAKEHW_FAKEDEVICE_H
#define SOLID_BACKENDS_FAKEHW_FAKEDEVICE_H

#include <solid/ifaces/device.h>

#include <QtCore/QMap>
#include <QtCore/QSharedPointer>

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeDevice : public Solid::Ifaces::Device
{
    Q_OBJECT
public:
    FakeDevice(const QString &udi, const QMap<QString, QVariant> &propertyMap);
    FakeDevice(const FakeDevice& dev);
    ~FakeDevice();

public Q_SLOTS:
    virtual QString udi() const;
    virtual QString parentUdi() const;
    virtual QString vendor() const;
    virtual QString product() const;
    virtual QString icon() const;
    virtual QStringList emblems() const;
    virtual QString description() const;

    virtual QVariant property(const QString &key) const;
    virtual QMap<QString, QVariant> allProperties() const;
    virtual bool propertyExists(const QString &key) const;
    virtual bool setProperty(const QString &key, const QVariant &value);
    virtual bool removeProperty(const QString &key);

    virtual bool lock(const QString &reason);
    virtual bool unlock();
    virtual bool isLocked() const;
    virtual QString lockReason() const;

    void setBroken(bool broken);
    bool isBroken();
    void raiseCondition(const QString &condition, const QString &reason);

public:
    virtual bool queryDeviceInterface(const Solid::DeviceInterface::Type &type) const;
    virtual QObject *createDeviceInterface(const Solid::DeviceInterface::Type &type);

Q_SIGNALS:
    void propertyChanged(const QMap<QString,int> &changes);
    void conditionRaised(const QString &condition, const QString &reason);


private:
    class Private;
    QSharedPointer<Private> d;
};
}
}
}

#endif // SOLID_BACKENDS_FAKEHW_FAKEDEVICE_H
