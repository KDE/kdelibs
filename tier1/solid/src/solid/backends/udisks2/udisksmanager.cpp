/*
    Copyright 2012 Lukáš Tinkl <ltinkl@redhat.com>

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

#include "udisksmanager.h"

#include <QtCore/QDebug>
#include <QtDBus>

#include "../shared/rootdevice.h"

using namespace Solid::Backends::UDisks2;
using namespace Solid::Backends::Shared;

Manager::Manager(QObject *parent)
    : Solid::Ifaces::DeviceManager(parent),
      m_manager(UD2_DBUS_SERVICE,
                UD2_DBUS_PATH,
                QDBusConnection::systemBus())
{
    m_supportedInterfaces
            << Solid::DeviceInterface::GenericInterface
            << Solid::DeviceInterface::Block
            << Solid::DeviceInterface::StorageAccess
            << Solid::DeviceInterface::StorageDrive
            << Solid::DeviceInterface::OpticalDrive
            << Solid::DeviceInterface::OpticalDisc
            << Solid::DeviceInterface::StorageVolume;

    qDBusRegisterMetaType<QList<QDBusObjectPath> >();
    qDBusRegisterMetaType<QVariantMap>();
    qDBusRegisterMetaType<QVariantMapMap>();
    qDBusRegisterMetaType<QByteArrayList>();
    qDBusRegisterMetaType<DBUSManagerStruct>();

    bool serviceFound = m_manager.isValid();
    if (!serviceFound) {
        // find out whether it will be activated automatically
        QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.DBus",
                                                              "/org/freedesktop/DBus",
                                                              "org.freedesktop.DBus",
                                                              "ListActivatableNames");

        QDBusReply<QStringList> reply = QDBusConnection::systemBus().call(message);
        if (reply.isValid() && reply.value().contains(UD2_DBUS_SERVICE)) {
            QDBusConnection::systemBus().interface()->startService(UD2_DBUS_SERVICE);
            serviceFound = true;
        }
    }

    if (serviceFound) {
        connect(&m_manager, SIGNAL(InterfacesAdded(QDBusObjectPath, QVariantMapMap)),
                this, SLOT(slotInterfacesAdded(QDBusObjectPath,QVariantMapMap)));
        connect(&m_manager, SIGNAL(InterfacesRemoved(QDBusObjectPath,QStringList)),
                this, SLOT(slotInterfacesRemoved(QDBusObjectPath,QStringList)));
    }
}

Manager::~Manager()
{
}

QObject* Manager::createDevice(const QString& udi)
{
    if (udi==udiPrefix()) {
        RootDevice *root = new RootDevice(udi);

        root->setProduct(tr("Storage"));
        root->setDescription(tr("Storage devices"));
        root->setIcon("server-database"); // Obviously wasn't meant for that, but maps nicely in oxygen icon set :-p

        return root;
    } else if (deviceCache().contains(udi)) {
        return new Device(udi);
    } else {
        return 0;
    }
}

QStringList Manager::devicesFromQuery(const QString& parentUdi, Solid::DeviceInterface::Type type)
{
    QStringList result;

    if (!parentUdi.isEmpty())
    {
        Q_FOREACH (const QString &udi, deviceCache())
        {
            Device device(udi);
            if (device.queryDeviceInterface(type) && device.parentUdi() == parentUdi)
                result << udi;
        }

        return result;
    }
    else if (type != Solid::DeviceInterface::Unknown)
    {
        Q_FOREACH (const QString &udi, deviceCache())
        {
            Device device(udi);
            if (device.queryDeviceInterface(type))
                result << udi;
        }

        return result;
    }

    return deviceCache();
}

QStringList Manager::allDevices()
{
    m_deviceCache.clear();

    QDBusPendingReply<DBUSManagerStruct> reply = m_manager.GetManagedObjects();
    reply.waitForFinished();
    if (!reply.isError()) {  // enum devices
        m_deviceCache << udiPrefix();

        Q_FOREACH(const QDBusObjectPath &path, reply.value().keys()) {
            const QString udi = path.path();
            qDebug() << "Adding device" << udi;

            if (udi == UD2_DBUS_PATH_MANAGER || udi == UD2_UDI_DISKS_PREFIX || udi.startsWith(UD2_DBUS_PATH_JOBS))
                continue;

            Device device(udi);
            if (device.mightBeOpticalDisc()) {
                QDBusConnection::systemBus().connect(UD2_DBUS_SERVICE, udi, DBUS_INTERFACE_PROPS, "PropertiesChanged", this,
                                                     SLOT(slotMediaChanged(QDBusMessage)));
                if (!device.isOpticalDisc())  // skip empty CD disc
                    continue;
            }

            m_deviceCache.append(udi);
        }
    }
    else  // show error
    {
        qWarning() << "Failed enumerating UDisks2 objects:" << reply.error().name() << "\n" << reply.error().message();
    }

    return m_deviceCache;
}


QSet< Solid::DeviceInterface::Type > Manager::supportedInterfaces() const
{
    return m_supportedInterfaces;
}

QString Manager::udiPrefix() const
{
    return UD2_UDI_DISKS_PREFIX;
}

#if 0
void Manager::slotDeviceAdded(const QDBusObjectPath &opath)
{
    const QString udi = opath.path();

    if (!m_deviceCache.contains(udi)) {
        m_deviceCache.append(udi);
    }

    Device device(udi);
    if (device.queryDeviceInterface(Solid::DeviceInterface::StorageDrive)
            && !device.prop("DeviceIsMediaAvailable").toBool()
            && !m_dirtyDevices.contains(udi))
        m_dirtyDevices.append(udi);

    Q_EMIT deviceAdded(udi);
    slotDeviceChanged(opath);  // case: hotswap event (optical drive with media inside)
}

void Manager::slotDeviceRemoved(const QDBusObjectPath &opath)
{
    const QString udi = opath.path();

    // case: hotswap event (optical drive with media inside)
    if (m_knownDrivesWithMedia.contains(udi)) {
        m_knownDrivesWithMedia.removeAll(udi);
        m_deviceCache.removeAll(udi + ":media");
        Q_EMIT deviceRemoved(udi + ":media");
    }

    if (m_dirtyDevices.contains(udi))
        m_dirtyDevices.removeAll(udi);

    Q_EMIT deviceRemoved(udi);
    m_deviceCache.removeAll(udi);
}

void Manager::slotDeviceChanged(const QDBusObjectPath &opath)
{
    const QString udi = opath.path();
    Device device(udi);

    if (device.queryDeviceInterface(Solid::DeviceInterface::OpticalDrive))
    {
        if (!m_knownDrivesWithMedia.contains(udi) && device.prop("DeviceIsOpticalDisc").toBool())
        {
            m_knownDrivesWithMedia.append(udi);
            if (!m_deviceCache.isEmpty()) {
                m_deviceCache.append(udi + ":media");
            }
            Q_EMIT deviceAdded(udi + ":media");
        }

        if (m_knownDrivesWithMedia.contains(udi) && !device.prop("DeviceIsOpticalDisc").toBool())
        {
            m_knownDrivesWithMedia.removeAll(udi);
            m_deviceCache.removeAll(udi + ":media");
            Q_EMIT deviceRemoved(udi + ":media");
        }
    }

    if (device.queryDeviceInterface(Solid::DeviceInterface::StorageDrive) && device.prop("MediaAvailable").toBool() && m_dirtyDevices.contains(udi))
    {
        qDebug() << "dirty device added:" << udi;
        Q_EMIT deviceAdded(udi);
        m_dirtyDevices.removeAll(udi);
    }
}
#endif


void Manager::slotInterfacesAdded(const QDBusObjectPath &object_path, const QVariantMapMap &interfaces_and_properties)
{
    const QString udi = object_path.path();

    qDebug() << udi << "has new interfaces:" << interfaces_and_properties.keys();

    // new device, we don't know it yet
    if (!m_deviceCache.contains(udi)) {
        m_deviceCache.append(udi);
        Q_EMIT deviceAdded(udi);
    }
}

void Manager::slotInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces)
{
    const QString udi = object_path.path();

    qDebug() << udi << "lost interfaces:" << interfaces;

    if (interfaces.isEmpty() && !udi.isEmpty()) {
        Q_EMIT deviceRemoved(udi);
        m_deviceCache.removeAll(udi);
    }
}

void Manager::slotMediaChanged(const QDBusMessage & msg)
{
    const QVariantMap properties = qdbus_cast<QVariantMap>(msg.arguments().at(1));
    const QString udi = msg.path();
    qulonglong size = properties.value("Size").toULongLong();
    qDebug() << "MEDIA CHANGED in" << udi << "; size is:" << size;

    if (!m_deviceCache.contains(udi) && size > 0) { // we don't know the optdisc, got inserted
        m_deviceCache.append(udi);
        Q_EMIT deviceAdded(udi);
    }

    if (m_deviceCache.contains(udi) && size == 0) {  // we know the optdisc, got removed
        Q_EMIT deviceRemoved(udi);
        m_deviceCache.removeAll(udi);
    }
}

const QStringList & Manager::deviceCache()
{
    if (m_deviceCache.isEmpty())
        allDevices();

    return m_deviceCache;
}
