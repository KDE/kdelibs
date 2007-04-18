/*  This file is part of the KDE project
    Copyright (C) 2005-2007 Kevin Ottens <ervin@kde.org>

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
#include "device_p.h"
#include "devicemanager.h"

#include "capability_p.h"
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
#include <solid/dvbhw.h>
#include <solid/ifaces/dvbhw.h>


Solid::Device::Device()
    : QObject(), d(new DevicePrivate())
{
}

Solid::Device::Device( const QString &udi )
    : QObject(), d(DeviceManager::self().findDevice(udi).d)
{
}

Solid::Device::Device( const Device &device )
    : QObject(), d(device.d)
{
}

Solid::Device::~Device()
{
    //qDeleteAll( d->ifaces.values() );
}

Solid::Device &Solid::Device::operator=( const Solid::Device &device )
{
    d = device.d;
    return *this;
}

bool Solid::Device::isValid() const
{
    return d->backendObject()!=0;
}

QString Solid::Device::udi() const
{
    return_SOLID_CALL( Ifaces::Device*, d->backendObject(), QString(), udi() );
}

QString Solid::Device::parentUdi() const
{
    return_SOLID_CALL( Ifaces::Device*, d->backendObject(), QString(), parentUdi() );
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
    return_SOLID_CALL( Ifaces::Device*, d->backendObject(), QString(), vendor() );
}

QString Solid::Device::product() const
{
    return_SOLID_CALL( Ifaces::Device*, d->backendObject(), QString(), product() );
}

bool Solid::Device::setProperty( const QString &key, const QVariant &value )
{
    return_SOLID_CALL( Ifaces::Device*, d->backendObject(), false, setProperty( key, value ) );
}

QVariant Solid::Device::property( const QString &key ) const
{
    return_SOLID_CALL( Ifaces::Device*, d->backendObject(), QVariant(), property( key ) );
}

QMap<QString, QVariant> Solid::Device::allProperties() const
{
    return_SOLID_CALL( Ifaces::Device*, d->backendObject(), QVariantMap(), allProperties() );
}

bool Solid::Device::propertyExists( const QString &key ) const
{
    return_SOLID_CALL( Ifaces::Device*, d->backendObject(), false, propertyExists( key ) );
}

bool Solid::Device::queryCapability( const Capability::Type &capability ) const
{
    return_SOLID_CALL( Ifaces::Device*, d->backendObject(), false, queryCapability( capability ) );
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
    const Solid::Capability *cap = const_cast<const Device *>(this)->asCapability(capability);
    return const_cast<Solid::Capability*>( cap );
}

const Solid::Capability *Solid::Device::asCapability( const Capability::Type &capability ) const
{
    Ifaces::Device *device = qobject_cast<Ifaces::Device*>( d->backendObject() );

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
            case Capability::DvbHw:
                iface = capability_cast<Ifaces::DvbHw, DvbHw>( cap_iface );
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
    return_SOLID_CALL( Ifaces::Device*, d->backendObject(), false, lock( reason ) );
}

bool Solid::Device::unlock()
{
    return_SOLID_CALL( Ifaces::Device*, d->backendObject(), false, unlock() );
}

bool Solid::Device::isLocked() const
{
    return_SOLID_CALL( Ifaces::Device*, d->backendObject(), false, isLocked() );
}

QString Solid::Device::lockReason() const
{
    return_SOLID_CALL( Ifaces::Device*, d->backendObject(), QString(), lockReason() );
}

Solid::DevicePrivate::DevicePrivate(const QString &udi)
    : QObject(), FrontendObjectPrivate(this)
{
    this->udi = udi;
}

Solid::DevicePrivate::~DevicePrivate()
{
    qDeleteAll( ifaces.values() );
}

void Solid::DevicePrivate::_k_destroyed(QObject *object)
{
    if (object == backendObject()) {
        FrontendObjectPrivate::_k_destroyed(object);

        foreach( Capability *iface, ifaces.values() )
        {
            delete iface->d_ptr->backendObject();
            delete iface;
        }

        ifaces.clear();
    }
}

void Solid::DevicePrivate::setBackendObject(QObject *object)
{
    foreach (Capability *iface, ifaces.values()) {
        delete iface->d_ptr->backendObject();
        delete iface;
    }

    ifaces.clear();

    FrontendObjectPrivate::setBackendObject(object);
}

#include "device.moc"
#include "device_p.moc"
