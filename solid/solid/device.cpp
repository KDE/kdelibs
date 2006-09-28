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

#include "soliddefs_p.h"

#include <solid/ifaces/device.h>

#include <solid/processor.h>
#include <solid/ifaces/processor.h>
#include <solid/block.h>
#include <solid/ifaces/block.h>
#include <solid/storage.h>
#include <solid/ifaces/storage.h>
#include <solid/cdrom.h>
#include <solid/ifaces/cdrom.h>
#include <solid/volume.h>
#include <solid/ifaces/volume.h>
#include <solid/opticaldisc.h>
#include <solid/ifaces/opticaldisc.h>
#include <solid/camera.h>
#include <solid/ifaces/camera.h>
#include <solid/portablemediaplayer.h>
#include <solid/ifaces/portablemediaplayer.h>
#include <solid/networkhw.h>
#include <solid/ifaces/networkhw.h>
#include <solid/acadapter.h>
#include <solid/ifaces/acadapter.h>
#include <solid/battery.h>
#include <solid/ifaces/battery.h>
#include <solid/button.h>
#include <solid/ifaces/button.h>
#include <solid/display.h>
#include <solid/ifaces/display.h>
#include <solid/audiohw.h>
#include <solid/ifaces/audiohw.h>

namespace Solid
{
    class Device::Private
    {
    public:
        QMap<Capability::Type,Capability*> ifaces;
    };
}

Solid::Device::Device()
    : FrontendObject(), d( new Private )
{
}

Solid::Device::Device( const QString &udi )
    : FrontendObject(), d( new Private )
{
    const Device &device = DeviceManager::self().findDevice( udi );
    registerBackendObject( device.backendObject() );
}

Solid::Device::Device( const Device &device )
    : FrontendObject(), d( new Private )
{
    registerBackendObject( device.backendObject() );
}

Solid::Device::Device( QObject *backendObject )
    : FrontendObject(), d( new Private )
{
    registerBackendObject( backendObject );
}

Solid::Device::~Device()
{
    foreach( Capability *iface, d->ifaces.values() )
    {
        delete iface;
    }

    delete d;
}

Solid::Device &Solid::Device::operator=( const Solid::Device &device )
{
    unregisterBackendObject();
    registerBackendObject( device.backendObject() );

    return *this;
}

QString Solid::Device::udi() const
{
    return_SOLID_CALL( Ifaces::Device*, backendObject(), QString(), udi() );
}

QString Solid::Device::parentUdi() const
{
    return_SOLID_CALL( Ifaces::Device*, backendObject(), QString(), parentUdi() );
}

Solid::Device Solid::Device::parent() const
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

QString Solid::Device::vendor() const
{
    return_SOLID_CALL( Ifaces::Device*, backendObject(), QString(), vendor() );
}

QString Solid::Device::product() const
{
    return_SOLID_CALL( Ifaces::Device*, backendObject(), QString(), product() );
}

bool Solid::Device::setProperty( const QString &key, const QVariant &value )
{
    return_SOLID_CALL( Ifaces::Device*, backendObject(), false, setProperty( key, value ) );
}

QVariant Solid::Device::property( const QString &key ) const
{
    return_SOLID_CALL( Ifaces::Device*, backendObject(), QVariant(), property( key ) );
}

QMap<QString, QVariant> Solid::Device::allProperties() const
{
    return_SOLID_CALL( Ifaces::Device*, backendObject(), QVariantMap(), allProperties() );
}

bool Solid::Device::propertyExists( const QString &key ) const
{
    return_SOLID_CALL( Ifaces::Device*, backendObject(), false, propertyExists( key ) );
}

bool Solid::Device::queryCapability( const Capability::Type &capability ) const
{
    return_SOLID_CALL( Ifaces::Device*, backendObject(), false, queryCapability( capability ) );
}

template<typename IfaceType, typename CapType>
inline CapType* capability_cast( QObject *backendObject )
{
    IfaceType *iface = qobject_cast<IfaceType*>( backendObject );

    if ( iface )
    {
        return new CapType( backendObject );
    }
    else
    {
        return 0;
    }
}

Solid::Capability *Solid::Device::asCapability( const Capability::Type &capability )
{
    Ifaces::Device *device = qobject_cast<Ifaces::Device*>( backendObject() );

    if ( device!=0 )
    {
        if ( d->ifaces.contains( capability ) )
        {
            return d->ifaces.value( capability );
        }

        QObject *cap_iface = device->createCapability( capability );

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
            case Capability::NetworkHw:
                iface = capability_cast<Ifaces::NetworkHw, NetworkHw>( cap_iface );
                break;
            case Capability::AcAdapter:
                iface = capability_cast<Ifaces::AcAdapter, AcAdapter>( cap_iface );
                break;
            case Capability::Battery:
                iface = capability_cast<Ifaces::Battery, Battery>( cap_iface );
                break;
            case Capability::Button:
                iface = capability_cast<Ifaces::Button, Button>( cap_iface );
                break;
            case Capability::Display:
                iface = capability_cast<Ifaces::Display, Display>( cap_iface );
                break;
            case Capability::AudioHw:
                iface = capability_cast<Ifaces::AudioHw, AudioHw>( cap_iface );
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

bool Solid::Device::lock(const QString &reason)
{
    return_SOLID_CALL( Ifaces::Device*, backendObject(), false, lock( reason ) );
}

bool Solid::Device::unlock()
{
    return_SOLID_CALL( Ifaces::Device*, backendObject(), false, unlock() );
}

bool Solid::Device::isLocked() const
{
    return_SOLID_CALL( Ifaces::Device*, backendObject(), false, isLocked() );
}

QString Solid::Device::lockReason() const
{
    return_SOLID_CALL( Ifaces::Device*, backendObject(), QString(), lockReason() );
}

void Solid::Device::slotDestroyed( QObject *object )
{
    if ( object == backendObject() )
    {
        FrontendObject::slotDestroyed(object);

        foreach( Capability *iface, d->ifaces.values() )
        {
            delete iface->backendObject();
            delete iface;
        }

        d->ifaces.clear();
    }
}

void Solid::Device::registerBackendObject( QObject *backendObject )
{
    setBackendObject( backendObject );

    if ( backendObject )
    {
        connect( backendObject, SIGNAL( propertyChanged( const QMap<QString,int>& ) ),
                 this, SIGNAL( propertyChanged( const QMap<QString,int>& ) ) );
        connect( backendObject, SIGNAL( conditionRaised( const QString &, const QString & ) ),
                 this, SIGNAL( conditionRaised( const QString &, const QString & ) ) );
    }
}

void Solid::Device::unregisterBackendObject()
{
    setBackendObject( 0 );

    foreach( Capability *iface, d->ifaces.values() )
    {
        delete iface->backendObject();
        delete iface;
    }

    d->ifaces.clear();
}

#include "device.moc"
