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
#include <QPair>

#include <kservicetypetrader.h>
#include <kservice.h>
#include <klibloader.h>

#include <klocale.h>
#include <kdebug.h>

#include "ifaces/networkmanager.h"
#include "ifaces/networkdevice.h"

#include "networkdevice.h"
#include "networkmanager.h"

namespace Solid
{
    class NetworkManager::Private
    {
    public:
        Private( NetworkManager *manager ) : q( manager ), backend( 0 ) {}

        QPair<NetworkDevice*, Ifaces::NetworkDevice*> findRegisteredNetworkDevice( const QString &uni );
        void registerBackend( Ifaces::NetworkManager * newBackend );
        void unregisterBackend();

        NetworkManager *q;
        Ifaces::NetworkManager *backend;
        QMap<QString, QPair<NetworkDevice*, Ifaces::NetworkDevice*> > networkDeviceMap;
        NetworkDevice invalidDevice;

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

Solid::NetworkDeviceList Solid::NetworkManager::buildDeviceList( const QStringList & uniList )
{
    NetworkDeviceList list;

    if ( d->backend == 0 ) return list;

    foreach( QString uni, uniList )
    {
        QPair<NetworkDevice*, Ifaces::NetworkDevice*> pair = d->findRegisteredNetworkDevice( uni );

        if ( pair.first!= 0 )
        {
            list.append( *pair.first );
        }
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

Solid::Ifaces::NetworkManager *Solid::NetworkManager::backend() const
{
    return d->backend;
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

const Solid::NetworkDevice &Solid::NetworkManager::findNetworkDevice( const QString &uni )
{
    if ( d->backend == 0 ) return d->invalidDevice;

    QPair<NetworkDevice*, Ifaces::NetworkDevice*> pair = d->findRegisteredNetworkDevice( uni );

    if ( pair.first != 0 )
    {
        return *pair.first;
    }
    else
    {
        return d->invalidDevice;
    }
}

void Solid::NetworkManager::slotAdded( const QString &uni )
{
    QPair<NetworkDevice*, Ifaces::NetworkDevice*> pair = d->networkDeviceMap.take( uni );

    if ( pair.first!= 0 )
    {
        // Oops, I'm not sure it should happen...
        // But well in this case we'd better kill the old device we got, it's probably outdated

        delete pair.first;
        delete pair.second;
    }

    emit added( uni );
}

void Solid::NetworkManager::slotRemoved( const QString &uni )
{
    QPair<NetworkDevice*, Ifaces::NetworkDevice*> pair = d->networkDeviceMap.take( uni );

    if ( pair.first!= 0 )
    {
        delete pair.first;
        delete pair.second;
    }

    emit removed( uni );
}

void Solid::NetworkManager::slotDestroyed( QObject *object )
{
    Ifaces::NetworkDevice *device = qobject_cast<Ifaces::NetworkDevice*>( object );

    if ( device!=0 )
    {
        QString uni = device->uni();
        QPair<NetworkDevice*, Ifaces::NetworkDevice*> pair = d->networkDeviceMap.take( uni );
        delete pair.first;
    }
}

/***************************************************************************/

void Solid::NetworkManager::Private::registerBackend( Ifaces::NetworkManager *newBackend )
{
    unregisterBackend();
    backend = newBackend;

    QObject::connect( backend, SIGNAL( added( const QString & ) ),
                      q, SLOT( slotAdded( const QString & ) ) );
    QObject::connect( backend, SIGNAL( removed( const QString & ) ),
                      q, SLOT( slotRemoved( const QString & ) ) );
}

void Solid::NetworkManager::Private::unregisterBackend()
{
    if ( backend != 0 )
    {
        // Delete all the devices, they are now outdated
        typedef QPair<NetworkDevice*, Ifaces::NetworkDevice*> NetworkDeviceIfacePair;

        // Delete all the devices, they are now outdated
        foreach( NetworkDeviceIfacePair pair, networkDeviceMap.values() )
        {
            delete pair.first;
            delete pair.second;
        }

        networkDeviceMap.clear();

        delete backend;
        backend = 0;
    }
}

QPair<Solid::NetworkDevice*, Solid::Ifaces::NetworkDevice*> Solid::NetworkManager::Private::findRegisteredNetworkDevice( const QString &uni )
{
    if ( networkDeviceMap.contains( uni ) )
    {
        return networkDeviceMap[uni];
    }
    else
    {
        Ifaces::NetworkDevice *iface = qobject_cast<Ifaces::NetworkDevice*>( backend->createNetworkDevice( uni ) );

        if ( iface!=0 )
        {
            NetworkDevice *device = new NetworkDevice( iface );
            QPair<NetworkDevice*, Ifaces::NetworkDevice*> pair( device, iface );
            networkDeviceMap[uni] = pair;
            return pair;
        }
        else
        {
            return QPair<NetworkDevice*, Ifaces::NetworkDevice*>( 0, 0 );
        }
    }
}

#include "networkmanager.moc"
