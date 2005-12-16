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
#include <QtDBUS/qdbusvariant.h>

#include <kdebug.h>

#include "haldevice.h"


class HalDevicePrivate
{
public:
    QDBusConnection connection;
    QString udi;

    QDBusMessage callHalMethod( const QString &methodName,
                                const QList<QVariant> &parameters = QList<QVariant>() ) const;
};

HalDevice::HalDevice(const QString &udi)
    : Device(), d( new HalDevicePrivate() )
{
    d->connection = QDBusConnection::addConnection(QDBusConnection::SystemBus);
    d->udi = udi;

    d->connection.connect( udi, "org.freedesktop.Hal.Device",
                           "PropertyModified",
                           this, SLOT( slotPropertyModified( const QString&, bool, bool ) ) );
}

HalDevice::~HalDevice()
{
    delete d;
}

QString HalDevice::udi() const
{
    return property( "info.udi" ).toString();
}

bool HalDevice::setProperty( const QString &key, const QVariant &value )
{
    QList<QVariant> params;
    params << key << value;
    QDBusMessage reply = d->callHalMethod( "SetProperty", params );

    if ( reply.type() == QDBusMessage::ErrorMessage )
    {
        kdDebug() << "Looks like the HAL protocol changed" << endl;
        return false;
    }

    return true;
}

QVariant HalDevice::property( const QString &key ) const
{
    QList<QVariant> params;
    params << key;
    QDBusMessage reply = d->callHalMethod( "GetProperty", params );

    if ( reply.size() != 1 )
    {
        kdDebug() << "Looks like the HAL protocol changed" << endl;
        return false;
    }

    return reply[0];
}

QMap<QString, QVariant> HalDevice::allProperties() const
{
    QDBusMessage reply = d->callHalMethod( "GetAllProperties" );

    if ( reply.size() != 1 || !reply[0].canConvert( QVariant::Map ))
    {
        kdDebug() << "Looks like the HAL protocol changed" << endl;
        return QMap<QString, QVariant>();
    }

    // Post process to avoid QDBusVariant
    QMap<QString, QVariant> properties = reply[0].toMap();

    QMap<QString, QVariant>::ConstIterator it = properties.begin();
    QMap<QString, QVariant>::ConstIterator end = properties.end();

    for ( ; it!=end; ++it )
    {
        QDBusVariant casted = qvariant_cast<QDBusVariant>( it.value() );

        if ( casted.value.type()!=QVariant::UserType )
        {
            properties[it.key()] = casted.value;
        }
    }

    return properties;
}

bool HalDevice::removeProperty( const QString &key )
{
    QList<QVariant> params;
    params << key;
    QDBusMessage reply = d->callHalMethod( "RemoveProperty", params );

    if ( reply.type() == QDBusMessage::ErrorMessage )
    {
        kdDebug() << "Looks like the HAL protocol changed" << endl;
        return false;
    }

    return true;
}

bool HalDevice::propertyExists( const QString &key ) const
{
    QList<QVariant> params;
    params << key;
    QDBusMessage reply = d->callHalMethod( "PropertyExists", params );

    if ( reply.size() != 1 || !reply[0].canConvert( QVariant::Bool ))
    {
        kdDebug() << "Looks like the HAL protocol changed" << endl;
        return false;
    }

    return reply[0].toBool();
}

bool HalDevice::addCapability( const QString &capability )
{
    QList<QVariant> params;
    params << capability;
    QDBusMessage reply = d->callHalMethod( "AddCapability", params );

    if ( reply.type() == QDBusMessage::ErrorMessage )
    {
        kdDebug() << "Looks like the HAL protocol changed" << endl;
        return false;
    }

    return true;
}

bool HalDevice::queryCapability( const QString &capability ) const
{
    QList<QVariant> params;
    params << capability;
    QDBusMessage reply = d->callHalMethod( "QueryCapability", params );

    if ( reply.size() != 1 || !reply[0].canConvert( QVariant::Bool ))
    {
        kdDebug() << "Looks like the HAL protocol changed" << endl;
        return false;
    }

    return reply[0].toBool();
}

bool HalDevice::lock(const QString &reason)
{
    QList<QVariant> parameters;
    parameters << reason;
    QDBusMessage reply = d->callHalMethod( "Lock", parameters );

    if ( reply.type() == QDBusMessage::ErrorMessage )
    {
        kdDebug() << "Can't lock" << endl;
        return false;
    }

    return true;
}

bool HalDevice::unlock()
{
    QDBusMessage reply = d->callHalMethod( "Unlock" );

    if ( reply.type() == QDBusMessage::ErrorMessage )
    {
        kdDebug() << "Can't unlock" << endl;
        return false;
    }

    return true;
}

void HalDevice::slotPropertyModified( const QString &key, bool added, bool removed )
{
    if ( added )
    {
        emit propertyChanged( key, KDEHW::PropertyAdded );
    }
    else if ( removed )
    {
        emit propertyChanged( key, KDEHW::PropertyRemoved );
    }
    else
    {
        emit propertyChanged( key, KDEHW::PropertyModified );
    }
}

void HalDevice::slotCondition( const QString &condition, const QString &reason )
{
    emit conditionRaised( condition, reason );
}

QDBusMessage HalDevicePrivate::callHalMethod( const QString &methodName,
                                              const QList<QVariant> &parameters ) const
{
    QDBusMessage msg = QDBusMessage::methodCall( "org.freedesktop.Hal", udi,
                                                 "org.freedesktop.Hal.Device",
                                                 methodName );

    msg << parameters;

    QDBusMessage reply = connection.sendWithReply(msg);

    if ( reply.type() == QDBusMessage::InvalidMessage )
    {
        kdDebug() << "Didn't get a valid reply" << reply.type() << endl;
    }

    return reply;
}

#include "haldevice.moc"
