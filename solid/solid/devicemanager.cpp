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

#include "devicemanager.h"

#include <QFile>

#include <ktrader.h>
#include <kservice.h>
#include <klibloader.h>

#include <kdebug.h>

#include "device.h"
#include "ifaces/devicemanager.h"
#include "ifaces/device.h"


namespace KDEHW
{
    class DeviceManager::Private
    {
    public:
        Private() : backend( 0 ) {}

        Ifaces::Device *findRegisteredDevice( const QString &udi );
        void registerBackend( Ifaces::DeviceManager *newBackend );
        void unregisterBackend();

        Ifaces::DeviceManager *backend;
        QMap<QString, Ifaces::Device*> devicesMap;
    };
}

static ::KStaticDeleter<KDEHW::DeviceManager> sd;

KDEHW::DeviceManager *KDEHW::DeviceManager::s_self = 0;


KDEHW::DeviceManager &KDEHW::DeviceManager::self()
{
    if( !s_self )
    {
        s_self = new KDEHW::DeviceManager();
        sd.setObject( s_self, s_self );
    }

    return *s_self;
}

KDEHW::DeviceManager::DeviceManager()
    : QObject(), d( new Private() )
{
    // TODO: Of course we don't want HAL hardcoded here...
    //d->registerBackend( new HalManager() );

    KTrader::OfferList offers = KTrader::self()->query( "KdeHwDeviceManager", "(Type == 'Service') and (Name == 'HAL')" );
    KService::Ptr ptr = offers.first();

    KLibFactory * factory = KLibLoader::self()->factory( QFile::encodeName( ptr->library() ) );
    if ( factory )
    {
        Ifaces::DeviceManager *backend = (Ifaces::DeviceManager*)factory->create( 0, "Device Manager", "KDEHW::Ifaces::DeviceManager" );
        if( backend )
        {
            d->registerBackend( backend );
        }
        else
        {
            kdDebug() << "no HAL backend found" << endl;
        }
    }

}

KDEHW::DeviceManager::~DeviceManager()
{
    d->unregisterBackend();
}

KDEHW::DeviceList KDEHW::DeviceManager::allDevices()
{
    DeviceList list;

    if ( d->backend == 0 ) return list;

    QStringList udis = d->backend->allDevices();

    foreach( QString udi, udis )
    {
        Ifaces::Device *device = d->findRegisteredDevice( udi );

        if ( device!=0 )
        {
            list.append( Device( device ) );
        }
    }

    return list;
}

bool KDEHW::DeviceManager::deviceExists( const QString &udi )
{
    if ( d->backend == 0 ) return false;

    if ( d->devicesMap.contains( udi ) )
    {
        return true;
    }
    else
    {
        return d->backend->deviceExists( udi );
    }
}

KDEHW::Device KDEHW::DeviceManager::findDevice( const QString &udi )
{
    if ( d->backend == 0 ) return Device();

    Ifaces::Device *device = d->findRegisteredDevice( udi );

    if ( device!=0 )
    {
        return Device( device );
    }
    else
    {
        return Device();
    }
}

void KDEHW::DeviceManager::slotDeviceAdded( const QString &udi )
{
    Ifaces::Device *device = d->devicesMap.take( udi );

    if ( device!= 0 )
    {
        // Oops, I'm not sure it should happen...
        // But well in this case we'd better kill the old device we got, it's probably outdated

        delete device;
    }

    emit deviceAdded( udi );
}

void KDEHW::DeviceManager::slotDeviceRemoved( const QString &udi )
{
    Ifaces::Device *device = d->devicesMap.take( udi );

    if ( device!= 0 )
    {
        delete device;
    }

    emit deviceRemoved( udi );
}

void KDEHW::DeviceManager::slotNewCapability( const QString &udi, const QString &capability )
{
    emit newCapability( udi, capability );
}

void KDEHW::DeviceManager::slotDestroyed( QObject *object )
{
    Ifaces::Device *device = dynamic_cast<Ifaces::Device*>( object );

    if ( device!=0 )
    {
        QString udi = device->udi();
        d->devicesMap.remove( udi );
    }
}

KDEHW::Ifaces::Device *KDEHW::DeviceManager::Private::findRegisteredDevice( const QString &udi )
{
    Ifaces::Device *device;

    if ( devicesMap.contains( udi ) )
    {
        device = devicesMap[udi];
    }
    else
    {
        device = backend->createDevice( udi );

        if ( device!=0 )
        {
            devicesMap[udi] = device;
        }
    }

    return device;
}

void KDEHW::DeviceManager::Private::registerBackend( Ifaces::DeviceManager *newBackend )
{
    unregisterBackend();
    backend = newBackend;
}

void KDEHW::DeviceManager::Private::unregisterBackend()
{
    if ( backend!=0 )
    {
        // Delete all the devices, they are now outdated
        foreach( Ifaces::Device *device, devicesMap.values() )
        {
            delete device;
        }

        devicesMap.clear();

        delete backend;
        backend = 0;
    }
}

#include "devicemanager.moc"

