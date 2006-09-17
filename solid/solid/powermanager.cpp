/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#include "powermanager.h"

#include <QFile>

#include <kservicetypetrader.h>
#include <kservice.h>
#include <klibloader.h>

#include <klocale.h>
#include <kdebug.h>

#include "ifaces/powermanager.h"


namespace Solid
{
    class PowerManager::Private
    {
    public:
        Private( PowerManager *manager ) : q( manager ), backend( 0 ) {}

        void registerBackend( Ifaces::PowerManager *newBackend );
        void unregisterBackend();

        PowerManager *q;
        Ifaces::PowerManager *backend;

        QString errorText;
    };
}

static ::KStaticDeleter<Solid::PowerManager> sd;

Solid::PowerManager *Solid::PowerManager::s_self = 0;


Solid::PowerManager &Solid::PowerManager::self()
{
    if( !s_self )
    {
        s_self = new Solid::PowerManager();
        sd.setObject( s_self, s_self );
    }

    return *s_self;
}

Solid::PowerManager &Solid::PowerManager::selfForceBackend( Ifaces::PowerManager *backend )
{
    if( !s_self )
    {
        s_self = new Solid::PowerManager( backend );
        sd.setObject( s_self, s_self );
    }

    return *s_self;
}

Solid::PowerManager::PowerManager()
    : QObject(), d( new Private( this ) )
{
    QStringList error_msg;

    Ifaces::PowerManager *backend = 0;

    KService::List offers = KServiceTypeTrader::self()->query( "SolidPowerManager", "(Type == 'Service')" );

    foreach ( KService::Ptr ptr, offers )
    {
        KLibFactory * factory = KLibLoader::self()->factory( QFile::encodeName( ptr->library() ) );

        if ( factory )
        {
            backend = (Ifaces::PowerManager*)factory->create( 0, "Solid::Ifaces::PowerManager" );

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
            d->errorText = i18n( "No Power Management Backend found" );
        }
        else
        {
            d->errorText = "<qt>";
            d->errorText+= i18n( "Unable to use any of the Power Management Backends" );
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

Solid::PowerManager::PowerManager( Ifaces::PowerManager *backend )
    : QObject(), d( new Private( this ) )
{
    if ( backend != 0 )
    {
        d->registerBackend( backend );
    }
}

Solid::PowerManager::~PowerManager()
{
    d->unregisterBackend();
}

const QString &Solid::PowerManager::errorText() const
{
    return d->errorText;
}

QStringList Solid::PowerManager::supportedSchemes() const
{
    if ( d->backend == 0 ) return QStringList();

    return d->backend->supportedSchemes();
}

QString Solid::PowerManager::schemeDescription( const QString &schemeName ) const
{
    if ( d->backend == 0 ) return QString();

    return d->backend->schemeDescription( schemeName );
}

QString Solid::PowerManager::scheme() const
{
    if ( d->backend == 0 ) return QString();

    return d->backend->scheme();
}

bool Solid::PowerManager::setScheme( const QString &name )
{
    if ( d->backend == 0 ) return false;

    return d->backend->setScheme( name );
}

Solid::PowerManager::BatteryState Solid::PowerManager::batteryState() const
{
    if ( d->backend == 0 ) return NoBatteryState;

    return d->backend->batteryState();
}

Solid::PowerManager::AcAdapterState Solid::PowerManager::acAdapterState() const
{
    if ( d->backend == 0 ) return Plugged;

    return d->backend->acAdapterState();
}

Solid::PowerManager::SuspendMethods Solid::PowerManager::supportedSuspendMethods() const
{
    if ( d->backend == 0 ) return UnknownSuspendMethod;

    return d->backend->supportedSuspendMethods();
}

KJob * Solid::PowerManager::suspend( SuspendMethod method ) const
{
    if ( d->backend == 0 ) return 0;

    return d->backend->suspend( method );
}

Solid::PowerManager::CpuFreqPolicies Solid::PowerManager::supportedCpuFreqPolicies() const
{
    if ( d->backend == 0 ) return UnknownCpuFreqPolicy;

    return d->backend->supportedCpuFreqPolicies();
}

Solid::PowerManager::CpuFreqPolicy Solid::PowerManager::cpuFreqPolicy() const
{
    if ( d->backend == 0 ) return UnknownCpuFreqPolicy;

    return d->backend->cpuFreqPolicy();
}

bool Solid::PowerManager::setCpuFreqPolicy( CpuFreqPolicy newPolicy )
{
    if ( d->backend == 0 ) return false;

    return d->backend->setCpuFreqPolicy( newPolicy );
}

bool Solid::PowerManager::canDisableCpu( int cpuNum ) const
{
    if ( d->backend == 0 ) return false;

    return d->backend->canDisableCpu( cpuNum );
}

bool Solid::PowerManager::setCpuEnabled( int cpuNum, bool enabled )
{
    if ( d->backend == 0 ) return false;

    return d->backend->setCpuEnabled( cpuNum, enabled );
}

const Solid::Ifaces::PowerManager *Solid::PowerManager::backend() const
{
    return d->backend;
}

void Solid::PowerManager::Private::registerBackend( Ifaces::PowerManager *newBackend )
{
    unregisterBackend();
    backend = newBackend;

    QObject::connect( backend, SIGNAL( schemeChanged( QString ) ),
                      q, SIGNAL( schemeChanged( QString ) ) );
    QObject::connect( backend, SIGNAL( acAdapterStateChanged( int ) ),
                      q, SIGNAL( acAdapterStateChanged( int ) ) );
    QObject::connect( backend, SIGNAL( batteryStateChanged( int ) ),
                      q, SIGNAL( batteryStateChanged( int ) ) );
    QObject::connect( backend, SIGNAL( buttonPressed( int ) ),
                      q, SIGNAL( buttonPressed( int ) ) );
}

void Solid::PowerManager::Private::unregisterBackend()
{
    if ( backend!=0 )
    {
        delete backend;
        backend = 0;
    }
}

#include "powermanager.moc"

