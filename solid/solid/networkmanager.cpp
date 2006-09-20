/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2006 Kévin Ottens <ervin@kde.org>

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

#include <QFile>

#include <kservicetypetrader.h>
#include <kservice.h>
#include <klibloader.h>

#include <klocale.h>
#include <kdebug.h>

#include "device.h"
#include "ifaces/networkmanager.h"
#include "ifaces/device.h"

#include "networkmanager.h"

namespace Solid
{
    class NetworkManager::Private
    {
    public:
        Private( NetworkManager *manager ) : q( manager ), backend( 0 ) {}

        Ifaces::NetworkDevice *findRegisteredNetworkDevice( const QString &udi );
        void registerBackend( Ifaces::NetworkManager * newBackend );
        void unregisterBackend();

        NetworkManager *q;
        Ifaces::NetworkManager *backend;
        QMap<QString, Ifaces::NetworkDevice*> networkDeviceMap;

        QString errorText;
    };

    class NetworkStatus::Private
    {
    public:
        Private( NetworkStatus *status ) : q( status ), backend( 0 ) {}

        void registerBackend( Ifaces::NetworkStatus * newBackend );
        void unregisterBackend();

        NetworkStatus *q;
        Ifaces::NetworkStatus *backend;
        QString errorText;
    };
}

static ::KStaticDeleter<Solid::NetworkManager> sd;

Solid::NetworkManager *Solid::NetworkManager::s_self = 0;


Solid::NetworkManager &Solid::NetworkManager::self()
{
    if( !s_self )
    {
        s_self = new Solid::NetworkManager();
        sd.setObject( s_self, s_self );
    }

    return *s_self;
}

Solid::NetworkManager &Solid::NetworkManager::selfForceBackend( Ifaces::NetworkManager *backend )
{
    if( !s_self )
    {
        s_self = new Solid::NetworkManager( backend );
        sd.setObject( s_self, s_self );
    }

    return *s_self;
}

Solid::NetworkManager::NetworkManager()
    : QObject(), d( new Private( this ) )
{
    QStringList error_msg;

    Ifaces::NetworkManager *backend = 0;

    KService::List offers = KServiceTypeTrader::self()->query( "SolidNetworkManager", "(Type == 'Service')" );

    foreach ( KService::Ptr ptr, offers )
    {
        KLibFactory * factory = KLibLoader::self()->factory( QFile::encodeName( ptr->library() ) );

        if ( factory )
        {
            backend = (Ifaces::NetworkManager*)factory->create( 0, "Solid::Ifaces::NetworkManager" );

            if( backend != 0 )
            {
                d->registerBackend( backend );
                kDebug() << "Using network management backend: " << ptr->name() << endl;
            }
            else
            {
                kDebug() << "Error loading network management backend'" << ptr->name() << "', factory's create method returned 0" << endl;
                error_msg.append( i18n("Factory's create method failed") );
            }
        }
        else
        {
            kDebug() << "Error loading network management backend'" << ptr->name() << "', factory creation failed" << endl;
            error_msg.append( i18n("Factory creation failed") );
        }
    }

    if ( backend == 0 )
    {
        if ( offers.size() == 0 )
        {
            d->errorText = i18n( "No Network Management Backend found" );
        }
        else
        {
            d->errorText = "<qt>";
            d->errorText+= i18n( "Unable to use any of the Network Management Backends" );
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

Solid::NetworkManager::NetworkManager( Ifaces::NetworkManager *backend )
    : QObject(), d( new Private( this ) )
{
    if ( backend != 0 )
    {
        d->registerBackend( backend );
    }
}

Solid::NetworkManager::~NetworkManager()
{
    d->unregisterBackend();
}

Solid::NetworkDeviceList Solid::NetworkManager::buildDeviceList( const QStringList & udiList )
{
    NetworkDeviceList list;

    if ( d->backend == 0 ) return list;

    foreach( QString udi, udiList )
    {
        Ifaces::NetworkDevice *device = d->findRegisteredNetworkDevice( udi );
        list.append( NetworkDevice( device ) );
    }

    return list;
}

Solid::NetworkDeviceList Solid::NetworkManager::networkDevices()
{
    return buildDeviceList( d->backend->networkDevices() );
}

Solid::NetworkDeviceList Solid::NetworkManager::activeNetworkDevices()
{
    return buildDeviceList( d->backend->activeNetworkDevices() );
}

void Solid::NetworkManager::activate( const QString & net )
{
    d->backend->activate( net );
}

void Solid::NetworkManager::deactivate( const QString & net )
{
    d->backend->deactivate( net );
}

void Solid::NetworkManager::enableWireless( bool enabled )
{
    d->backend->enableWireless( enabled );
}

void Solid::NetworkManager::enableNetworking( bool enabled )
{
    d->backend->enableNetworking( enabled );
}

void Solid::NetworkManager::notifyHiddenNetwork( const QString & essid )
{
    d->backend->notifyHiddenNetwork( essid );
}

Solid::NetworkDevice Solid::NetworkManager::findNetworkDevice( const QString &udi )
{
    if ( d->backend == 0 ) return NetworkDevice();

    Ifaces::NetworkDevice *device = d->findRegisteredNetworkDevice( udi );

    if ( device != 0 )
    {
        return NetworkDevice( device );
    }
    else
    {
        return NetworkDevice();
    }
}

/***************************************************************************/

void Solid::NetworkManager::Private::registerBackend( Ifaces::NetworkManager *newBackend )
{
    unregisterBackend();
    backend = newBackend;

    QObject::connect( backend, SIGNAL( added( const QString & ) ),
                      q, SIGNAL( added( const QString & ) ) );
    QObject::connect( backend, SIGNAL( removed( const QString & ) ),
                      q, SLOT( removed( const QString & ) ) );
}

void Solid::NetworkManager::Private::unregisterBackend()
{
    if ( backend != 0 )
    {
        // Delete all the devices, they are now outdated
        foreach( Ifaces::NetworkDevice *device, networkDeviceMap.values() )
        {
            delete device;
        }

        networkDeviceMap.clear();

        delete backend;
        backend = 0;
    }
}

Solid::Ifaces::NetworkDevice *Solid::NetworkManager::Private::findRegisteredNetworkDevice( const QString &udi )
{
    Ifaces::NetworkDevice *device;

    if ( networkDeviceMap.contains( udi ) )
    {
        device = networkDeviceMap[udi];
    }
    else
    {
        device = backend->createNetworkDevice( udi );

        if ( device != 0 )
        {
            networkDeviceMap[udi] = device;
        }
    }

    return device;
}

/***************************************************************************/

static ::KStaticDeleter<Solid::NetworkStatus> statusSd;

Solid::NetworkStatus *Solid::NetworkStatus::s_self = 0;

Solid::NetworkStatus &Solid::NetworkStatus::self()
{
    if( !s_self )
    {
        s_self = new Solid::NetworkStatus();
        statusSd.setObject( s_self, s_self );
    }

    return *s_self;
}

Solid::NetworkStatus &Solid::NetworkStatus::selfForceBackend( Ifaces::NetworkStatus *backend )
{
    if( !s_self )
    {
        s_self = new Solid::NetworkStatus( backend );
        statusSd.setObject( s_self, s_self );
    }

    return *s_self;
}

Solid::NetworkStatus::NetworkStatus()
    : QObject(), d( new Private( this ) )
{
    QStringList error_msg;

    Ifaces::NetworkStatus *backend = 0;

    KService::List offers = KServiceTypeTrader::self()->query( "SolidNetworkManager", "(Type == 'Service')" );

    foreach ( KService::Ptr ptr, offers )
    {
        KLibFactory * factory = KLibLoader::self()->factory( QFile::encodeName( ptr->library() ) );

        if ( factory )
        {
            backend = (Ifaces::NetworkStatus*)factory->create( 0, "Solid::Ifaces::NetworkStatus" );

            if( backend != 0 )
            {
                d->registerBackend( backend );
                kDebug() << "Using network management backend: " << ptr->name() << endl;
            }
            else
            {
                kDebug() << "Error loading network management backend'" << ptr->name() << "', factory's create method returned 0" << endl;
                error_msg.append( i18n("Factory's create method failed") );
            }
        }
        else
        {
            kDebug() << "Error loading network management backend'" << ptr->name() << "', factory creation failed" << endl;
            error_msg.append( i18n("Factory creation failed") );
        }
    }

    if ( backend == 0 )
    {
        if ( offers.size() == 0 )
        {
            d->errorText = i18n( "No Network Management Backend found" );
        }
        else
        {
            d->errorText = "<qt>";
            d->errorText+= i18n( "Unable to use any of the Network Management Backends" );
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

Solid::NetworkStatus::NetworkStatus( Ifaces::NetworkStatus *backend )
    : QObject(), d( new Private( this ) )
{
    if ( backend != 0 )
    {
        d->registerBackend( backend );
    }
}

Solid::NetworkStatus::~NetworkStatus()
{
    d->unregisterBackend();
}

Solid::Ifaces::NetworkStatus::ConnectionState Solid::NetworkStatus::connectionState() const
{
    return d->backend->connectionState();
}

Solid::Ifaces::NetworkStatus::ConnectionState Solid::NetworkStatus::connectionState( const QString & host) const
{
    return d->backend->connectionState( host );
}

void Solid::NetworkStatus::Private::registerBackend( Ifaces::NetworkStatus *newBackend )
{
    // clear any existing backend
    unregisterBackend();
    // store the new one
    backend = newBackend;
    // make the connections
    QObject::connect( backend, SIGNAL( connectionStateChanged( Solid::Ifaces::NetworkStatus::ConnectionState ) ),
                      q, SIGNAL( added( const QString & ) ) );
}

void Solid::NetworkStatus::Private::unregisterBackend()
{
    delete backend;
}


#include "networkmanager.moc"
