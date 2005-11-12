/*  This file is part of the KDE project
    Copyright (C) 2005 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include <QtDBUS/qdbus.h>

#include <kdebug.h>

#include "halmanager.h"
#include "haldevice.h"

class HalManagerPrivate
{
public:
    QDBusConnection connection;
};

HalManager::HalManager()
    : DeviceManager(),  d( new HalManagerPrivate() )
{
    d->connection = QDBusConnection::addConnection(QDBusConnection::SystemBus);

    d->connection.connect( "/org/freedesktop/Hal/Manager",
                           "org.freedesktop.Hal.Manager",
                           "DeviceAdded",
                           this, SLOT( slotDeviceAdded( const QString& ) ) );

    d->connection.connect( "/org/freedesktop/Hal/Manager",
                           "org.freedesktop.Hal.Manager",
                           "DeviceRemoved",
                           this, SLOT( slotDeviceRemoved( const QString& ) ) );

    d->connection.connect( "/org/freedesktop/Hal/Manager",
                           "org.freedesktop.Hal.Manager",
                           "NewCapability",
                           this,
                           SLOT( slotNewCapability(const QString&, const QString& ) ) );
}

HalManager::~HalManager()
{
    delete d;
}

QStringList HalManager::allDevices()
{
    QList<QVariant> reply = callHalMethod( "GetAllDevices" );

    if ( reply.size() != 1 || !reply[0].canConvert( QVariant::StringList ) )
    {
        kdDebug() << "Looks like the HAL protocol changed" << endl;
        return QStringList();
    }

    return reply[0].toStringList();
}

bool HalManager::deviceExists( const QString &udi )
{
    QList<QVariant> params;
    params << udi;
    QList<QVariant> reply = callHalMethod( "DeviceExists", params );

    if ( reply.size() != 1 || !reply[0].canConvert( QVariant::Bool ) )
    {
        kdDebug() << "Looks like the HAL protocol changed" << endl;
        return false;
    }

    return reply[0].toBool();
}

KDEHW::Ifaces::Device * HalManager::createDevice( const QString &udi )
{
    return new HalDevice( udi );
}

QStringList HalManager::findDeviceStringMatch( const QString &key, const QString &value )
{
    QList<QVariant> params;
    params << key << value;
    QList<QVariant> reply = callHalMethod( "FindDeviceStringMatch" );

    if ( reply.size() != 1 || !reply[0].canConvert( QVariant::StringList ) )
    {
        kdDebug() << "Looks like the HAL protocol changed" << endl;
        return QStringList();
    }

    return reply[0].toStringList();
}

QStringList HalManager::findDeviceByCapability( const QString &capability )
{
    QList<QVariant> params;
    params << capability;
    QList<QVariant> reply = callHalMethod( "FindDeviceByCapability" );

    if ( reply.size() != 1 || !reply[0].canConvert( QVariant::StringList ) )
    {
        kdDebug() << "Looks like the HAL protocol changed" << endl;
        return QStringList();
    }

    return reply[0].toStringList();
}

QList<QVariant> HalManager::callHalMethod( const QString &methodName,
                                           const QList<QVariant> &parameters )
{
    QDBusMessage msg = QDBusMessage::methodCall( "org.freedesktop.Hal",
                                                 "/org/freedesktop/Hal/Manager",
                                                 "org.freedesktop.Hal.Manager",
                                                 methodName );

    msg << parameters;

    QDBusMessage reply = d->connection.sendWithReply(msg);

    if ( reply.isEmpty() )
    {
        kdDebug() << "Didn't get a valid reply!" << endl;
        return QList<QVariant>();
    }

    return reply;
}

void HalManager::slotDeviceAdded( const QString &udi )
{
    emit deviceAdded( udi );
}

void HalManager::slotDeviceRemoved( const QString &udi )
{
    emit deviceRemoved( udi );
}

void HalManager::slotNewCapability( const QString &udi, const QString &capability )
{
    emit newCapability( udi,  capability );
}

#include "halmanager.moc"
