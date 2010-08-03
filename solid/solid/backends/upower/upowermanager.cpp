/*  Copyright 2010  Michael Zanetti <mzanetti@kde.org>
              2010  Lukas Tinkl <ltinkl@redhat.com>

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

#include "upowermanager.h"
#include "upowerdevice.h"
#include "upower.h"

#include <QtDBus/QDBusReply>
#include <QtCore/QDebug>
#include <QtDBus/QDBusMetaType>

using namespace Solid::Backends::UPower;

UPowerManager::UPowerManager(QObject *parent)
    : m_manager(UP_DBUS_SERVICE,
                UP_DBUS_PATH,
                UP_DBUS_INTERFACE,
                QDBusConnection::systemBus())
{
    m_supportedInterfaces
            << Solid::DeviceInterface::GenericInterface
            << Solid::DeviceInterface::AcAdapter
            << Solid::DeviceInterface::Battery;

    qDBusRegisterMetaType<QList<QDBusObjectPath> >();

    connect(&m_manager, SIGNAL(DeviceAdded(QDBusObjectPath)),
            this, SLOT(slotDeviceAdded(QDBusObjectPath)));
    connect(&m_manager, SIGNAL(DeviceRemoved(QDBusObjectPath)),
            this, SLOT(slotDeviceRemoved(QDBusObjectPath)));
}

UPowerManager::~UPowerManager()
{
}

QObject* UPowerManager::createDevice(const QString& udi)
{
    // TODO should check for device existence given the UDI
    return new UPowerDevice(udi);
}

QStringList UPowerManager::devicesFromQuery(const QString& parentUdi, Solid::DeviceInterface::Type type)
{
    // TODO: do this!
    return QStringList();
}

QStringList UPowerManager::allDevices()
{
    QDBusReply<QList<QDBusObjectPath> > reply = m_manager.call("EnumerateDevices");

    if (!reply.isValid()) {
        qWarning() << Q_FUNC_INFO << " error: " << reply.error().name();
        return QStringList();
    }

    QStringList retList;
    foreach (const QDBusObjectPath &path, reply.value()) {
        retList << path.path();
    }

    return retList;
}

QSet< Solid::DeviceInterface::Type > UPowerManager::supportedInterfaces() const
{
    return m_supportedInterfaces;
}

QString UPowerManager::udiPrefix() const
{
    return UP_UDI_PREFIX;
}

void UPowerManager::slotDeviceAdded(const QDBusObjectPath &opath)
{
    emit deviceAdded(opath.path());
}

void UPowerManager::slotDeviceRemoved(const QDBusObjectPath &opath)
{
    emit deviceRemoved(opath.path());
}

#include "backends/upower/upowermanager.moc"
