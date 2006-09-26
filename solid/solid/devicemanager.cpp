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

#include "devicemanager.h"

#include <QFile>
#include <QPair>

#include <kservicetypetrader.h>
#include <kservice.h>
#include <klibloader.h>

#include <klocale.h>
#include <kdebug.h>

#include "device.h"
#include "ifaces/devicemanager.h"
#include "ifaces/device.h"


namespace Solid
{
    class DeviceManager::Private
    {
    public:
        Private( DeviceManager *manager ) : q( manager ), backend( 0 ) {}

        QPair<Device*, Ifaces::Device*> findRegisteredDevice( const QString &udi );
        void registerBackend( Ifaces::DeviceManager *newBackend );
        void unregisterBackend();

        DeviceManager *q;
        Ifaces::DeviceManager *backend;
        QMap<QString, QPair<Device*, Ifaces::Device*> > devicesMap;
        Device invalidDevice;

        QString errorText;
    };
}

static ::KStaticDeleter<Solid::DeviceManager> sd;

Solid::DeviceManager *Solid::DeviceManager::s_self = 0;


Solid::DeviceManager &Solid::DeviceManager::self()
{
    if( !s_self )
    {
        s_self = new Solid::DeviceManager();
        sd.setObject( s_self, s_self );
    }

    return *s_self;
}

Solid::DeviceManager &Solid::DeviceManager::selfForceBackend( Ifaces::DeviceManager *backend )
{
    if( !s_self )
    {
        s_self = new Solid::DeviceManager( backend );
        sd.setObject( s_self, s_self );
    }

    return *s_self;
}

Solid::DeviceManager::DeviceManager()
    : QObject(), d( new Private( this ) )
{
    QStringList error_msg;

    Ifaces::DeviceManager *backend = 0;

    KService::List offers = KServiceTypeTrader::self()->query( "SolidDeviceManager", "(Type == 'Service')" );

    foreach ( KService::Ptr ptr, offers )
    {
        KLibFactory * factory = KLibLoader::self()->factory( QFile::encodeName( ptr->library() ) );

        if ( factory )
        {
            backend = (Ifaces::DeviceManager*)factory->create( 0, "Solid::Ifaces::DeviceManager" );

            if( backend != 0 )
            {
                d->registerBackend( backend );
                kDebug() << "Using backend: " << ptr->name() << endl;
            }
            else
            {
                kDebug() << "Error loading '" << ptr->name() << "', factory's create method returned 0" << endl;
                error_msg.append( i18n("Factory's create method failed") );
            }
        }
        else
        {
            kDebug() << "Error loading '" << ptr->name() << "', factory creation failed" << endl;
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

Solid::DeviceManager::DeviceManager( Ifaces::DeviceManager *backend )
    : QObject(), d( new Private( this ) )
{
    if ( backend != 0 )
    {
        d->registerBackend( backend );
    }
}

Solid::DeviceManager::~DeviceManager()
{
    d->unregisterBackend();
}

const QString &Solid::DeviceManager::errorText() const
{
    return d->errorText;
}

Solid::DeviceList Solid::DeviceManager::allDevices()
{
    DeviceList list;

    if ( d->backend == 0 ) return list;

    QStringList udis = d->backend->allDevices();

    foreach( QString udi, udis )
    {
        QPair<Device*, Ifaces::Device*> pair = d->findRegisteredDevice( udi );

        if ( pair.first!=0 )
        {
            list.append( *pair.first );
        }
    }

    return list;
}

bool Solid::DeviceManager::deviceExists( const QString &udi )
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

const Solid::Device &Solid::DeviceManager::findDevice( const QString &udi )
{
    if ( d->backend == 0 ) return d->invalidDevice;

    QPair<Device*, Ifaces::Device*> pair = d->findRegisteredDevice( udi );

    if ( pair.first!=0 )
    {
        return *pair.first;
    }
    else
    {
        return d->invalidDevice;
    }
}

Solid::DeviceList Solid::DeviceManager::findDevicesFromQuery( const QString &parentUdi,
                                                              const Capability::Type &capability,
                                                              const QString &predicate )
{
    Predicate p = Predicate::fromString( predicate );

    if ( p.isValid() )
    {
        return findDevicesFromQuery( parentUdi, capability, p );
    }
    else
    {
        return DeviceList();
    }
}

Solid::DeviceList Solid::DeviceManager::findDevicesFromQuery( const QString &parentUdi,
                                                              const Capability::Type &capability,
                                                              const Predicate &predicate )
{
    DeviceList list;

    if ( d->backend == 0 ) return list;

    QStringList udis = d->backend->devicesFromQuery( parentUdi, capability );

    foreach( QString udi, udis )
    {
        QPair<Device*, Ifaces::Device*> pair = d->findRegisteredDevice( udi );

        bool matches = false;
        if ( pair.first )
        {
            if( !predicate.isValid() )
               matches = true;
            else
               matches = predicate.matches( pair.second );
        }

        if ( matches )
        {
            list.append( *pair.first );
        }
    }

    return list;
}

Solid::Ifaces::DeviceManager *Solid::DeviceManager::backend() const
{
    return d->backend;
}

void Solid::DeviceManager::slotDeviceAdded( const QString &udi )
{
    QPair<Device*, Ifaces::Device*> pair = d->devicesMap.take( udi );

    if ( pair.first!= 0 )
    {
        // Oops, I'm not sure it should happen...
        // But well in this case we'd better kill the old device we got, it's probably outdated

        delete pair.first;
        delete pair.second;
    }

    emit deviceAdded( udi );
}

void Solid::DeviceManager::slotDeviceRemoved( const QString &udi )
{
    QPair<Device*, Ifaces::Device*> pair = d->devicesMap.take( udi );

    if ( pair.first!= 0 )
    {
        delete pair.first;
        delete pair.second;
    }

    emit deviceRemoved( udi );
}

void Solid::DeviceManager::slotNewCapability( const QString &udi, int capability )
{
    emit newCapability( udi, capability );
}

void Solid::DeviceManager::slotDestroyed( QObject *object )
{
    Ifaces::Device *device = dynamic_cast<Ifaces::Device*>( object );

    if ( device!=0 )
    {
        QString udi = device->udi();
        QPair<Device*, Ifaces::Device*> pair = d->devicesMap.take( udi );
        delete pair.first;
    }
}

QPair<Solid::Device*, Solid::Ifaces::Device*> Solid::DeviceManager::Private::findRegisteredDevice( const QString &udi )
{
    if ( devicesMap.contains( udi ) )
    {
        return devicesMap[udi];
    }
    else
    {
        Ifaces::Device *iface = qobject_cast<Ifaces::Device*>( backend->createDevice( udi ) );

        if ( iface!=0 )
        {
            Device *device = new Device( iface );
            QPair<Device*, Ifaces::Device*> pair( device, iface );
            devicesMap[udi] = pair;
            return pair;
        }
        else
        {
            return QPair<Device*, Ifaces::Device*>( 0, 0 );
        }
    }
}

void Solid::DeviceManager::Private::registerBackend( Ifaces::DeviceManager *newBackend )
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

void Solid::DeviceManager::Private::unregisterBackend()
{
    if ( backend!=0 )
    {
        typedef QPair<Device*, Ifaces::Device*> DeviceIfacePair;

        // Delete all the devices, they are now outdated
        foreach( DeviceIfacePair pair, devicesMap.values() )
        {
            delete pair.first;
            delete pair.second;
        }

        devicesMap.clear();

        delete backend;
        backend = 0;
    }
}

#include "devicemanager.moc"

