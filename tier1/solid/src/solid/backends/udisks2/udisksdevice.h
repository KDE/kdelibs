/*
    Copyright 2010 Michael Zanetti <mzanetti@kde.org>
    Copyright 2010-2012 Lukáš Tinkl <ltinkl@redhat.com>

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

#ifndef UDISKS2DEVICE_H
#define UDISKS2DEVICE_H

#include <ifaces/device.h>
#include <solid/deviceinterface.h>
#include <solid/solidnamespace.h>

#include <QtDBus/QDBusInterface>
#include <QtCore/QStringList>

namespace Solid
{
namespace Backends
{
namespace UDisks2
{

class Device: public Solid::Ifaces::Device
{
    Q_OBJECT
public:
    Device(const QString &udi);
    virtual ~Device();

    virtual QObject* createDeviceInterface(const Solid::DeviceInterface::Type& type);
    virtual bool queryDeviceInterface(const Solid::DeviceInterface::Type& type) const;
    virtual QString description() const;
    virtual QStringList emblems() const;
    virtual QString icon() const;
    virtual QString product() const;
    virtual QString vendor() const;
    virtual QString udi() const;
    virtual QString parentUdi() const;

    QVariant prop(const QString &key) const;
    bool propertyExists(const QString &key) const;
    QVariantMap allProperties() const;

    bool hasInterface(const QString & name) const;
    QStringList interfaces() const;

    QString errorToString(const QString & error) const;
    Solid::ErrorType errorToSolidError(const QString & error) const;

    bool isBlock() const;
    bool isPartition() const;
    bool isPartitionTable() const;
    bool isStorageVolume() const;
    bool isStorageAccess() const;
    bool isDrive() const;
    bool isOpticalDrive() const;
    bool isOpticalDisc() const;
    bool mightBeOpticalDisc() const;
    bool isMounted() const;
    bool isEncryptedContainer() const;
    bool isEncryptedCleartext() const;
    bool isSwap() const;

Q_SIGNALS:
    void changed();
    void propertyChanged(const QMap<QString,int> &changes);

private Q_SLOTS:
    void slotPropertiesChanged(const QString & ifaceName, const QVariantMap & changedProps, const QStringList & invalidatedProps);

private:
    QString storageDescription() const;
    QString volumeDescription() const;
    mutable QDBusInterface *m_device;
    QString m_udi;
    mutable QVariantMap m_cache;

    void initInterfaces();
    QStringList m_interfaces;

    void checkCache(const QString &key) const;
    QString introspect() const;
};

}
}
}

#endif // UDISKS2DEVICE_H
