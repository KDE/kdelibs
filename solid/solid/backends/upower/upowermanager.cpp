/*
    Copyright 2010 Michael Zanetti <mzanetti@kde.org>
    Copyright 2010 Lukas Tinkl <ltinkl@redhat.com>

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

#include "upowermanager.h"
#include "upowerdevice.h"
#include "upower.h"

#include <QtDBus/QDBusReply>
#include <QtCore/QDebug>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusConnectionInterface>

#include "../shared/rootdevice.h"

using namespace Solid::Backends::UPower;
using namespace Solid::Backends::Shared;

UPowerManager::UPowerManager(QObject *parent)
    : Solid::Ifaces::DeviceManager(parent),
      m_manager(UP_DBUS_SERVICE,
                UP_DBUS_PATH,
                UP_DBUS_INTERFACE,
                QDBusConnection::systemBus())
{
    m_supportedInterfaces
            << Solid::DeviceInterface::GenericInterface
            << Solid::DeviceInterface::AcAdapter
            << Solid::DeviceInterface::Battery;

    qDBusRegisterMetaType<QList<QDBusObjectPath> >();
    qDBusRegisterMetaType<QVariantMap>();

    bool serviceFound = m_manager.isValid();
    if (!serviceFound) {
        // find out whether it will be activated automatically
        QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.DBus",
                                                              "/org/freedesktop/DBus",
                                                              "org.freedesktop.DBus",
                                                              "ListActivatableNames");

        QDBusReply<QStringList> reply = QDBusConnection::systemBus().call(message);
        if (reply.isValid() && reply.value().contains(UP_DBUS_SERVICE)) {
            QDBusConnection::systemBus().interface()->startService(UP_DBUS_SERVICE);
            serviceFound = true;
        }
    }

    if (serviceFound) {
        connect(&m_manager, SIGNAL(DeviceAdded(QString)),
                this, SLOT(slotDeviceAdded(QString)));
        connect(&m_manager, SIGNAL(DeviceRemoved(QString)),
                this, SLOT(slotDeviceRemoved(QString)));
    }
}

UPowerManager::~UPowerManager()
{
}

QObject* UPowerManager::createDevice(const QString& udi)
{
    if (udi==udiPrefix()) {
        RootDevice *root = new RootDevice(udiPrefix());

        root->setProduct(tr("Power Management"));
        root->setDescription(tr("Batteries and other sources of power"));
        root->setIcon("preferences-system-power-management");

        return root;

    } else if (allDevices().contains(udi)) {
        return new UPowerDevice(udi);

    } else {
        return 0;
    }
}

QStringList UPowerManager::devicesFromQuery(const QString& parentUdi, Solid::DeviceInterface::Type type)
{
    QStringList allDev = allDevices();
    QStringList result;

    if (!parentUdi.isEmpty())
    {
        foreach (const QString & udi, allDev)
        {
            UPowerDevice device(udi);
            if (device.queryDeviceInterface(type) && device.parentUdi() == parentUdi)
                result << udi;
        }

        return result;
    }
    else if (type != Solid::DeviceInterface::Unknown)
    {
        foreach (const QString & udi, allDev)
        {
            UPowerDevice device(udi);
            if (device.queryDeviceInterface(type))
                result << udi;
        }

        return result;
    }
    else
        return allDev;
}

QStringList UPowerManager::allDevices()
{
    QDBusReply<QList<QDBusObjectPath> > reply = m_manager.call("EnumerateDevices");

    if (!reply.isValid()) {
        qWarning() << Q_FUNC_INFO << " error: " << reply.error().name();
        return QStringList();
    }

    QStringList retList;
    retList << udiPrefix();

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

void UPowerManager::slotDeviceAdded(const QString &opath)
{
    emit deviceAdded(opath);
}

void UPowerManager::slotDeviceRemoved(const QString &opath)
{
    emit deviceRemoved(opath);
}

#include "backends/upower/upowermanager.moc"
