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

#include <klocale.h>
#include <kdebug.h>

#include "device.h"
#include "ifaces/devicemanager.h"
#include "ifaces/device.h"


namespace KDEHW
{
    class DeviceManager::Private
    {
    public:
        Private( DeviceManager *manager ) : q( manager ), backend( 0 ) {}

        Ifaces::Device *findRegisteredDevice( const QString &udi );
        void registerBackend( Ifaces::DeviceManager *newBackend );
        void unregisterBackend();

        DeviceManager *q;
        Ifaces::DeviceManager *backend;
        QMap<QString, Ifaces::Device*> devicesMap;

        QString errorText;
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

KDEHW::DeviceManager &KDEHW::DeviceManager::selfForceBackend( KDEHW::Ifaces::DeviceManager *backend )
{
    if( !s_self )
    {
        s_self = new KDEHW::DeviceManager( backend );
        sd.setObject( s_self, s_self );
    }

    return *s_self;
}

KDEHW::DeviceManager::DeviceManager()
    : QObject(), d( new Private( this ) )
{
    QStringList error_msg;

    Ifaces::DeviceManager *backend = 0;

    KTrader::OfferList offers = KTrader::self()->query( "KdeHwDeviceManager", "(Type == 'Service')" );
    KService::Ptr ptr = offers.first();

    foreach ( KService::Ptr ptr, offers )
    {
        KLibFactory * factory = KLibLoader::self()->factory( QFile::encodeName( ptr->library() ) );

        if ( factory )
        {
            backend = (Ifaces::DeviceManager*)factory->create( 0, "Device Manager", "KDEHW::Ifaces::DeviceManager" );

            if( backend != 0 )
            {
                d->registerBackend( backend );
                kdDebug() << "Using backend: " << ptr->name() << endl;
            }
            else
            {
                kdDebug() << "Error loading '" << ptr->name() << "', factory's create method returned 0" << endl;
                error_msg.append( i18n("Factory's create method failed") );
            }
        }
        else
        {
            kdDebug() << "Error loading '" << ptr->name() << "', factory creation failed" << endl;
            error_msg.append( i18n("Factory creation failed") );
        }
    }

    if ( backend == 0 )
    {
        if ( offers.size() == 0 )
        {
            d->errorText = i18n( "No Hardware Discovery Backend found" );
        }
        else
        {
            d->errorText = "<qt>";
            d->errorText+= i18n( "Unable to use any of the Hardware Discovery Backends" );
            d->errorText+= "<table>";

            QString line = "<tr><td><b>%1</b></td><td>%2</td></tr>";

            for ( int i = 0; i< offers.size(); i++ )
            {
                d->errorText+= line.arg( offers[i]->name() ).arg( error_msg[i] );
            }

            d->errorText+= "</table></qt>";
        }
    }
}

KDEHW::DeviceManager::DeviceManager( KDEHW::Ifaces::DeviceManager *backend )
    : QObject(), d( new Private( this ) )
{
    if ( backend != 0 )
    {
        d->registerBackend( backend );
    }
}

KDEHW::DeviceManager::~DeviceManager()
{
    d->unregisterBackend();
}

const QString &KDEHW::DeviceManager::errorText() const
{
    return d->errorText;
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

const KDEHW::Ifaces::DeviceManager *KDEHW::DeviceManager::backend() const
{
    return d->backend;
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

void KDEHW::DeviceManager::slotNewCapability( const QString &udi, int capability )
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

    QObject::connect( backend, SIGNAL( deviceAdded( QString ) ),
                      q, SLOT( slotDeviceAdded( QString ) ) );
    QObject::connect( backend, SIGNAL( deviceRemoved( QString ) ),
                      q, SLOT( slotDeviceRemoved( QString ) ) );
    QObject::connect( backend, SIGNAL( newCapability( QString, int ) ),
                      q, SLOT( slotNewCapability( QString, int ) ) );
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

