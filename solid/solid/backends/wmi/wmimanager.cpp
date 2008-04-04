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

#include "wmimanager.h"
#include "wmidevice.h"
#include "wmideviceinterface.h"

#include <QtCore/QDebug>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

using namespace Solid::Backends::Wmi;

class Solid::Backends::Wmi::WmiManagerPrivate
{
public:
    WmiManagerPrivate() : manager("org.freedesktop.Wmi",
                                   "/org/freedesktop/Wmi/Manager",
                                   "org.freedesktop.Wmi.Manager",
                                   QDBusConnection::systemBus()),
                          cacheSynced(false) { }

    QDBusInterface manager;
    QList<QString> devicesCache;
    bool cacheSynced;
};


WmiManager::WmiManager(QObject *parent)
    : DeviceManager(parent),  d(new WmiManagerPrivate())
{
    d->manager.connection().connect("org.freedesktop.Wmi",
                                     "/org/freedesktop/Wmi/Manager",
                                     "org.freedesktop.Wmi.Manager",
                                     "DeviceAdded",
                                     this, SLOT(slotDeviceAdded(const QString &)));

    d->manager.connection().connect("org.freedesktop.Wmi",
                                     "/org/freedesktop/Wmi/Manager",
                                     "org.freedesktop.Wmi.Manager",
                                     "DeviceRemoved",
                                     this, SLOT(slotDeviceRemoved(const QString &)));
}

WmiManager::~WmiManager()
{
    delete d;
}

QStringList WmiManager::allDevices()
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

bool WmiManager::deviceExists(const QString &udi)
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

QStringList WmiManager::devicesFromQuery(const QString &parentUdi,
                                         Solid::DeviceInterface::Type type)
{
    if (!parentUdi.isEmpty())
    {
        QStringList result = findDeviceStringMatch("info.parent", parentUdi);

        if (type!=Solid::DeviceInterface::Unknown) {
            QStringList::Iterator it = result.begin();
            QStringList::ConstIterator end = result.end();

            for (; it!=end; ++it)
            {
                WmiDevice device(*it);

                if (!device.queryDeviceInterface(type)) {
                    result.erase(it);
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

QObject *WmiManager::createDevice(const QString &udi)
{
    if (deviceExists(udi)) {
        return new WmiDevice(udi);
    } else {
        return 0;
    }
}

QStringList WmiManager::findDeviceStringMatch(const QString &key, const QString &value)
{
    QDBusReply<QStringList> reply = d->manager.call("FindDeviceStringMatch", key, value);

    if (!reply.isValid())
    {
        qWarning() << Q_FUNC_INFO << " error: " << reply.error().name() << endl;
        return QStringList();
    }

    return reply;
}

QStringList WmiManager::findDeviceByDeviceInterface(const Solid::DeviceInterface::Type &type)
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
                QDBusInterface device( "org.freedesktop.Wmi", udi, "org.freedesktop.Wmi.Device", QDBusConnection::systemBus() );
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

void WmiManager::slotDeviceAdded(const QString &udi)
{
    d->devicesCache.append(udi);
    emit deviceAdded(udi);
}

void WmiManager::slotDeviceRemoved(const QString &udi)
{
    d->devicesCache.removeAll(udi);
    emit deviceRemoved(udi);
}

#include "backends/wmi/wmimanager.moc"
