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

#include "device.h"

namespace KDEHW
{
    class Device::Private
    {
    public:
        Private() : data( 0 ) { }

        Ifaces::Device *data;
    };
}

KDEHW::Device::Device( QObject *parent )
    : Ifaces::Device( parent ), d( new Private() )
{
}

KDEHW::Device::Device( const Device &device )
    : Ifaces::Device( device.parent() ), d( new Private() )
{
    *d = *(device.d );

    if ( d->data )
    {
        connect( d->data, SIGNAL( propertyChanged( const QString &, PropertyChange ) ),
                 this, SLOT( slotPropertyChanged( const QString &, PropertyChange ) ) );
        connect( d->data, SIGNAL( conditionRaised( const QString &, const QString & ) ),
                 this, SLOT( slotConditionRaised( const QString &, const QString & ) ) );
        connect( d->data, SIGNAL( destroyed( QObject * ) ),
                 this, SLOT( slotDestroyed( QObject * ) ) );
    }
}

KDEHW::Device::Device( Ifaces::Device *data )
    : Ifaces::Device(), d( new Private() )
{
    d->data = data;

    if ( d->data )
    {
        connect( d->data, SIGNAL( propertyChanged( const QString &, PropertyChange ) ),
                 this, SLOT( slotPropertyChanged( const QString &, PropertyChange ) ) );
        connect( d->data, SIGNAL( conditionRaised( const QString &, const QString & ) ),
                 this, SLOT( slotConditionRaised( const QString &, const QString & ) ) );
        connect( d->data, SIGNAL( destroyed( QObject * ) ),
                 this, SLOT( slotDestroyed( QObject * ) ) );
    }
}

KDEHW::Device::~Device()
{
    delete d;
}

KDEHW::Device &KDEHW::Device::operator=( const KDEHW::Device &device )
{
    if ( d->data )
    {
        disconnect( d->data, SIGNAL( propertyChanged( const QString &, PropertyChange ) ),
                    this, SLOT( slotPropertyChanged( const QString &, PropertyChange ) ) );
        disconnect( d->data, SIGNAL( conditionRaised( const QString &, const QString & ) ),
                    this, SLOT( slotConditionRaised( const QString &, const QString & ) ) );
        disconnect( d->data, SIGNAL( destroyed( QObject * ) ),
                    this, SLOT( slotDestroyed( QObject * ) ) );
    }

    d->data = device.d->data;

    if ( d->data )
    {
        connect( d->data, SIGNAL( propertyChanged( const QString &, PropertyChange ) ),
                 this, SLOT( slotPropertyChanged( const QString &, PropertyChange ) ) );
        connect( d->data, SIGNAL( conditionRaised( const QString &, const QString & ) ),
                 this, SLOT( slotConditionRaised( const QString &, const QString & ) ) );
        connect( d->data, SIGNAL( destroyed( QObject * ) ),
                 this, SLOT( slotDestroyed( QObject * ) ) );
    }

    return *this;
}

bool KDEHW::Device::isValid()
{
    return d->data!=0;
}

QString KDEHW::Device::udi() const
{
    if ( d->data!=0 )
    {
        return d->data->udi();
    }
    else
    {
        return QString();
    }
}

QVariant KDEHW::Device::property( const QString &key ) const
{
    if ( d->data!=0 )
    {
        return d->data->property( key );
    }
    else
    {
        return QVariant();
    }
}

QMap<QString, QVariant> KDEHW::Device::allProperties() const
{
    if ( d->data!=0 )
    {
        return d->data->allProperties();
    }
    else
    {
        return QMap<QString, QVariant>();
    }
}

bool KDEHW::Device::propertyExists( const QString &key ) const
{
    if ( d->data!=0 )
    {
        return d->data->propertyExists( key );
    }
    else
    {
        return false;
    }
}

bool KDEHW::Device::queryCapability( const QString &capability ) const
{
    if ( d->data!=0 )
    {
        return d->data->queryCapability( capability );
    }
    else
    {
        return false;
    }
}

bool KDEHW::Device::lock(const QString &reason)
{
    if ( d->data!=0 )
    {
        return d->data->lock( reason );
    }
    else
    {
        return false;
    }
}

bool KDEHW::Device::unlock()
{
    if ( d->data!=0 )
    {
        return d->data->unlock();
    }
    else
    {
        return false;
    }
}

void KDEHW::Device::slotPropertyChanged( const QString &key, PropertyChange change )
{
    emit propertyChanged( key, change );
}

void KDEHW::Device::slotConditionRaised( const QString &condition, const QString &reason )
{
    emit conditionRaised( condition, reason );
}

void KDEHW::Device::slotDestroyed( QObject *object )
{
    if ( object == d->data )
    {
        d->data = 0;
    }
}

#include "device.moc"
