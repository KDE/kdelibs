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

#include "halmanager.h"
#include "haldevice.h"
#include "haldeviceinterface.h"

#include <QtCore/QDebug>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

using namespace Solid::Backends::Hal;

class Solid::Backends::Hal::HalManagerPrivate
{
public:
    HalManagerPrivate() : manager("org.freedesktop.Hal",
                                   "/org/freedesktop/Hal/Manager",
                                   "org.freedesktop.Hal.Manager",
                                   QDBusConnection::systemBus()),
                          cacheSynced(false) { }

    QDBusInterface manager;
    QList<QString> devicesCache;
    bool cacheSynced;
    QSet<Solid::DeviceInterface::Type> supportedInterfaces;
};


HalManager::HalManager(QObject *parent)
    : DeviceManager(parent),  d(new HalManagerPrivate())
{
    d->manager.connection().connect("org.freedesktop.Hal",
                                     "/org/freedesktop/Hal/Manager",
                                     "org.freedesktop.Hal.Manager",
                                     "DeviceAdded",
                                     this, SLOT(slotDeviceAdded(const QString &)));

    d->manager.connection().connect("org.freedesktop.Hal",
                                     "/org/freedesktop/Hal/Manager",
                                     "org.freedesktop.Hal.Manager",
                                     "DeviceRemoved",
                                     this, SLOT(slotDeviceRemoved(const QString &)));

    d->supportedInterfaces << Solid::DeviceInterface::GenericInterface
                           << Solid::DeviceInterface::Processor
                           << Solid::DeviceInterface::Block
                           << Solid::DeviceInterface::StorageAccess
                           << Solid::DeviceInterface::StorageDrive
                           << Solid::DeviceInterface::OpticalDrive
                           << Solid::DeviceInterface::StorageVolume
                           << Solid::DeviceInterface::OpticalDisc
                           << Solid::DeviceInterface::Camera
                           << Solid::DeviceInterface::PortableMediaPlayer
                           << Solid::DeviceInterface::NetworkInterface
                           << Solid::DeviceInterface::AcAdapter
                           << Solid::DeviceInterface::Battery
                           << Solid::DeviceInterface::Button
                           << Solid::DeviceInterface::AudioInterface
                           << Solid::DeviceInterface::DvbInterface
                           << Solid::DeviceInterface::Video
                           << Solid::DeviceInterface::SerialInterface
                           << Solid::DeviceInterface::SmartCardReader;
}

HalManager::~HalManager()
{
    delete d;
}

QString HalManager::udiPrefix() const
{
    return "/org/freedesktop/Hal";
}

QSet<Solid::DeviceInterface::Type> HalManager::supportedInterfaces() const
{
    return d->supportedInterfaces;
}

QStringList HalManager::allDevices()
{
    if (d->cacheSynced)
    {
        return d->devicesCache;
    }

    QDBusReply<QStringList> reply = d->manager.call("GetAllDevices");

    if (!reply.isValid())
    {
        qWarning() << Q_FUNC_INFO << " error: " << reply.error().name() << endl;
        return QStringList();
    }

    d->devicesCache = reply;
    d->cacheSynced = true;

    return reply;
}

bool HalManager::deviceExists(const QString &udi)
{
    if (d->devicesCache.contains(udi))
    {
        return true;
    }
    else if (d->cacheSynced)
    {
        return false;
    }

    QDBusReply<bool> reply = d->manager.call("DeviceExists", udi);

    if (!reply.isValid())
    {
        qWarning() << Q_FUNC_INFO << " error: " << reply.error().name() << endl;
        return false;
    }

    if (reply)
    {
        d->devicesCache.append(udi);
    }

    return reply;
}

QStringList HalManager::devicesFromQuery(const QString &parentUdi,
                                         Solid::DeviceInterface::Type type)
{
    if (!parentUdi.isEmpty())
    {
        QStringList result = findDeviceStringMatch("info.parent", parentUdi);

        if (type!=Solid::DeviceInterface::Unknown) {
            const QStringList matches = result;
            result.clear();

            foreach (const QString &match, matches) {
                HalDevice device(match);

                if (device.queryDeviceInterface(type)) {
                    result << match;
                }
            }
        }

        return result;

    } else if (type!=Solid::DeviceInterface::Unknown) {
        return findDeviceByDeviceInterface(type);
    } else {
        return allDevices();
    }
}

QObject *HalManager::createDevice(const QString &udi)
{
    if (deviceExists(udi)) {
        return new HalDevice(udi);
    } else {
        return 0;
    }
}

QStringList HalManager::findDeviceStringMatch(const QString &key, const QString &value)
{
    QDBusReply<QStringList> reply = d->manager.call("FindDeviceStringMatch", key, value);

    if (!reply.isValid())
    {
        qWarning() << Q_FUNC_INFO << " error: " << reply.error().name() << endl;
        return QStringList();
    }

    return reply;
}

QStringList HalManager::findDeviceByDeviceInterface(Solid::DeviceInterface::Type type)
{
    QStringList cap_list = DeviceInterface::toStringList(type);
    QStringList result;

    foreach (const QString &cap, cap_list)
    {
        QDBusReply<QStringList> reply = d->manager.call("FindDeviceByCapability", cap);

        if (!reply.isValid())
        {
            qWarning() << Q_FUNC_INFO << " error: " << reply.error().name() << endl;
            return QStringList();
        }
        if ( cap == QLatin1String( "video4linux" ) )
        {
            QStringList foundDevices ( reply );
            QStringList filtered;
            foreach ( const QString &udi, foundDevices )
            {
                QDBusInterface device( "org.freedesktop.Hal", udi, "org.freedesktop.Hal.Device", QDBusConnection::systemBus() );
                QDBusReply<QString> reply = device.call( "GetProperty", "video4linux.device" );
                if (!reply.isValid())
                {
                    qWarning() << Q_FUNC_INFO << " error getting video4linux.device: " << reply.error().name() << endl;
                    continue;
                }
                if ( !reply.value().contains( "video" ) )
                {
                    continue;
                }
                filtered.append( udi );
            }
            result += filtered;
        }
        else
        {
            result << reply;
        }
    }

    return result;
}

void HalManager::slotDeviceAdded(const QString &udi)
{
    d->devicesCache.append(udi);
    emit deviceAdded(udi);
}

void HalManager::slotDeviceRemoved(const QString &udi)
{
    d->devicesCache.removeAll(udi);
    emit deviceRemoved(udi);
}

#include "backends/hal/halmanager.moc"
