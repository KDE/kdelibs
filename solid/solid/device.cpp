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

#include "deviceinterface_p.h"
#include "soliddefs_p.h"

#include <solid/ifaces/device.h>

#include <solid/genericinterface.h>
#include <solid/ifaces/genericinterface.h>
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
    : d(new DevicePrivate())
{
}

Solid::Device::Device( const QString &udi )
    : d(DeviceManager::self().findDevice(udi).d)
{
}

Solid::Device::Device( const Device &device )
    : d(device.d)
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

bool Solid::Device::queryDeviceInterface(const DeviceInterface::Type &type) const
{
    return_SOLID_CALL( Ifaces::Device*, d->backendObject(), false, queryDeviceInterface(type) );
}

template<typename IfaceType, typename DevType>
inline DevType* deviceinterface_cast( QObject *backendObject )
{
    IfaceType *iface = qobject_cast<IfaceType*>( backendObject );

    if ( iface )
    {
        return new DevType( backendObject );
    }
    else
    {
        return 0;
    }
}

Solid::DeviceInterface *Solid::Device::asDeviceInterface(const DeviceInterface::Type &type)
{
    const Solid::DeviceInterface *interface = const_cast<const Device *>(this)->asDeviceInterface(type);
    return const_cast<Solid::DeviceInterface*>(interface);
}

const Solid::DeviceInterface *Solid::Device::asDeviceInterface(const DeviceInterface::Type &type) const
{
    Ifaces::Device *device = qobject_cast<Ifaces::Device*>( d->backendObject() );

    if ( device!=0 )
    {
        if (d->ifaces.contains(type)) {
            return d->ifaces.value(type);
        }

        QObject *dev_iface = device->createDeviceInterface(type);

        DeviceInterface *iface = 0;

        if ( dev_iface!=0 )
        {
            switch (type)
            {
            case DeviceInterface::GenericInterface:
                iface = deviceinterface_cast<Ifaces::GenericInterface, GenericInterface>(dev_iface);
                break;
            case DeviceInterface::Processor:
                iface = deviceinterface_cast<Ifaces::Processor, Processor>(dev_iface);
                break;
            case DeviceInterface::Block:
                iface = deviceinterface_cast<Ifaces::Block, Block>(dev_iface);
                break;
            case DeviceInterface::Storage:
                iface = deviceinterface_cast<Ifaces::Storage, Storage>(dev_iface);
                break;
            case DeviceInterface::Cdrom:
                iface = deviceinterface_cast<Ifaces::Cdrom, Cdrom>(dev_iface);
                break;
            case DeviceInterface::Volume:
                iface = deviceinterface_cast<Ifaces::Volume, Volume>(dev_iface);
                break;
            case DeviceInterface::OpticalDisc:
                iface = deviceinterface_cast<Ifaces::OpticalDisc, OpticalDisc>(dev_iface);
                break;
            case DeviceInterface::Camera:
                iface = deviceinterface_cast<Ifaces::Camera, Camera>(dev_iface);
                break;
            case DeviceInterface::PortableMediaPlayer:
                iface = deviceinterface_cast<Ifaces::PortableMediaPlayer, PortableMediaPlayer>(dev_iface);
                break;
            case DeviceInterface::NetworkHw:
                iface = deviceinterface_cast<Ifaces::NetworkHw, NetworkHw>(dev_iface);
                break;
            case DeviceInterface::AcAdapter:
                iface = deviceinterface_cast<Ifaces::AcAdapter, AcAdapter>(dev_iface);
                break;
            case DeviceInterface::Battery:
                iface = deviceinterface_cast<Ifaces::Battery, Battery>(dev_iface);
                break;
            case DeviceInterface::Button:
                iface = deviceinterface_cast<Ifaces::Button, Button>(dev_iface);
                break;
            case DeviceInterface::Display:
                iface = deviceinterface_cast<Ifaces::Display, Display>(dev_iface);
                break;
            case DeviceInterface::AudioHw:
                iface = deviceinterface_cast<Ifaces::AudioHw, AudioHw>(dev_iface);
                break;
            case DeviceInterface::DvbHw:
                iface = deviceinterface_cast<Ifaces::DvbHw, DvbHw>(dev_iface);
                break;
            case DeviceInterface::Unknown:
                break;
            }
        }

        if ( iface!=0 )
        {
            d->ifaces[type] = iface;
        }

        return iface;
    }
    else
    {
        return 0;
    }
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

        foreach( DeviceInterface *iface, ifaces.values() )
        {
            delete iface->d_ptr->backendObject();
            delete iface;
        }

        ifaces.clear();
    }
}

void Solid::DevicePrivate::setBackendObject(QObject *object)
{
    foreach (DeviceInterface *iface, ifaces.values()) {
        delete iface->d_ptr->backendObject();
        delete iface;
    }

    ifaces.clear();

    FrontendObjectPrivate::setBackendObject(object);
}

#include "device_p.moc"
