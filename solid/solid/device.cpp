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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "device.h"
#include "devicemanager.h"

#include <kdehw/ifaces/device.h>

#include <kdehw/processor.h>
#include <kdehw/ifaces/processor.h>
#include <kdehw/block.h>
#include <kdehw/ifaces/block.h>
#include <kdehw/storage.h>
#include <kdehw/ifaces/storage.h>
#include <kdehw/cdrom.h>
#include <kdehw/ifaces/cdrom.h>
#include <kdehw/volume.h>
#include <kdehw/ifaces/volume.h>
#include <kdehw/opticaldisc.h>
#include <kdehw/ifaces/opticaldisc.h>
#include <kdehw/camera.h>
#include <kdehw/ifaces/camera.h>
#include <kdehw/portablemediaplayer.h>
#include <kdehw/ifaces/portablemediaplayer.h>
#include <kdehw/networkiface.h>
#include <kdehw/ifaces/networkiface.h>
#include <kdehw/display.h>
#include <kdehw/ifaces/display.h>

namespace KDEHW
{
    class Device::Private
    {
    public:
        Private( Device *device ) : q( device ), data( 0 ) { }

        void unregisterData();
        void registerData( Ifaces::Device *newData );

        Device *q;
        Ifaces::Device *data;
        QMap<Capability::Type,Capability*> ifaces;
    };
}

KDEHW::Device::Device()
    : QObject(), d( new Private( this ) )
{
}

KDEHW::Device::Device( const Device &device )
    : QObject(), d( new Private( this ) )
{
    d->registerData( device.d->data );
}

KDEHW::Device::Device( Ifaces::Device *data )
    : QObject(), d( new Private( this ) )
{
    d->registerData( data );
}

KDEHW::Device::~Device()
{
    foreach( Capability *iface, d->ifaces.values() )
    {
        delete iface;
    }

    delete d;
}

KDEHW::Device &KDEHW::Device::operator=( const KDEHW::Device &device )
{
    d->unregisterData();
    d->registerData( device.d->data );

    return *this;
}

bool KDEHW::Device::isValid() const
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

QString KDEHW::Device::parentUdi() const
{
    if ( d->data!=0 )
    {
        return d->data->parentUdi();
    }
    else
    {
        return QString();
    }
}

KDEHW::Device KDEHW::Device::parent() const
{
    QString udi = parentUdi();

    if ( udi.isEmpty() )
    {
        return Device();
    }
    else
    {
        return DeviceManager::self().findDevice( udi );
    }
}

QString KDEHW::Device::vendor() const
{
    if ( d->data!=0 )
    {
        return d->data->vendor();
    }
    else
    {
        return QString();
    }
}

QString KDEHW::Device::product() const
{
    if ( d->data!=0 )
    {
        return d->data->product();
    }
    else
    {
        return QString();
    }
}

bool KDEHW::Device::setProperty( const QString &key, const QVariant &value )
{
    if ( d->data!=0 )
    {
        return d->data->setProperty( key, value );
    }
    else
    {
        return false;
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

bool KDEHW::Device::queryCapability( const Capability::Type &capability ) const
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

template<typename IfaceType, typename CapType>
inline CapType* capability_cast( KDEHW::Ifaces::Capability *cap_iface )
{
    IfaceType *iface = dynamic_cast<IfaceType*>( cap_iface );

    if ( iface )
    {
        return new CapType( iface );
    }
    else
    {
        return 0;
    }
}

KDEHW::Capability *KDEHW::Device::asCapability( const Capability::Type &capability )
{
    if ( d->data!=0 )
    {
        if ( d->ifaces.contains( capability ) )
        {
            return d->ifaces.value( capability );
        }

        Ifaces::Capability *cap_iface = d->data->asCapability( capability );

        Capability *iface = 0;

        if ( cap_iface!=0 )
        {
            switch ( capability )
            {
            case Capability::Processor:
                iface = capability_cast<Ifaces::Processor, Processor>( cap_iface );
                break;
            case Capability::Block:
                iface = capability_cast<Ifaces::Block, Block>( cap_iface );
                break;
            case Capability::Storage:
                iface = capability_cast<Ifaces::Storage, Storage>( cap_iface );
                break;
            case Capability::Cdrom:
                iface = capability_cast<Ifaces::Cdrom, Cdrom>( cap_iface );
                break;
            case Capability::Volume:
                iface = capability_cast<Ifaces::Volume, Volume>( cap_iface );
                break;
            case Capability::OpticalDisc:
                iface = capability_cast<Ifaces::OpticalDisc, OpticalDisc>( cap_iface );
                break;
            case Capability::Camera:
                iface = capability_cast<Ifaces::Camera, Camera>( cap_iface );
                break;
            case Capability::PortableMediaPlayer:
                iface = capability_cast<Ifaces::PortableMediaPlayer, PortableMediaPlayer>( cap_iface );
                break;
            case Capability::NetworkIface:
                iface = capability_cast<Ifaces::NetworkIface, NetworkIface>( cap_iface );
                break;
            case Capability::Display:
                iface = capability_cast<Ifaces::Display, Display>( cap_iface );
                break;
            case Capability::Unknown:
                break;
            }
        }

        if ( iface!=0 )
        {
            d->ifaces[capability] = iface;
        }

        return iface;
    }
    else
    {
        return 0;
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

bool KDEHW::Device::isLocked() const
{
    if ( d->data!=0 )
    {
        return d->data->isLocked();
    }
    else
    {
        return false;
    }
}

QString KDEHW::Device::lockReason() const
{
    if ( d->data!=0 )
    {
        return d->data->lockReason();
    }
    else
    {
        return QString();
    }
}

void KDEHW::Device::slotPropertyChanged( const QMap<QString,int> &changes )
{
    emit propertyChanged( changes );
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
        d->unregisterData();
    }
}

void KDEHW::Device::Private::registerData( KDEHW::Ifaces::Device *newData )
{
    data = newData;

    if ( data )
    {
        connect( data, SIGNAL( propertyChanged( const QMap<QString,int>& ) ),
                 q, SLOT( slotPropertyChanged( const QMap<QString,int>& ) ) );
        connect( data, SIGNAL( conditionRaised( const QString &, const QString & ) ),
                 q, SLOT( slotConditionRaised( const QString &, const QString & ) ) );
        connect( data, SIGNAL( destroyed( QObject * ) ),
                 q, SLOT( slotDestroyed( QObject * ) ) );
    }
}

void KDEHW::Device::Private::unregisterData()
{
    if ( data )
    {
        disconnect( data, SIGNAL( propertyChanged( const QMap<QString,int>& ) ),
                    q, SLOT( slotPropertyChanged( const QMap<QString,int>& ) ) );
        disconnect( data, SIGNAL( conditionRaised( const QString &, const QString & ) ),
                    q, SLOT( slotConditionRaised( const QString &, const QString & ) ) );
        disconnect( data, SIGNAL( destroyed( QObject * ) ),
                    q, SLOT( slotDestroyed( QObject * ) ) );
    }

    data = 0;

    foreach( Capability *iface, ifaces.values() )
    {
        delete iface;
    }

    ifaces.clear();
}

#include "device.moc"
