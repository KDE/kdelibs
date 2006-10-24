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
#include "managerbase.h"


namespace Solid
{
    class ManagerBase::Private
    {
    public:
        Private() : backend( 0 ) {}

        QObject *backend;
        QString errorText;
    };
}

Solid::ManagerBase::ManagerBase( const QString &description, const char *serviceName, const char *backendClassName )
    : QObject(), d( new Private )
{
    QObject *backend = loadBackend( description, serviceName, backendClassName );

    if ( backend != 0 )
    {
        setManagerBackend( backend );
    }
}

Solid::ManagerBase::ManagerBase( QObject *backend )
    : QObject(), d( new Private )
{
    if ( backend != 0 )
    {
        setManagerBackend( backend );
    }
}

Solid::ManagerBase::~ManagerBase()
{
    delete d->backend;
    delete d;
}

const QString &Solid::ManagerBase::errorText() const
{
    return d->errorText;
}

QObject *Solid::ManagerBase::managerBackend() const
{
    return d->backend;
}

void Solid::ManagerBase::setManagerBackend( QObject *backend )
{
    if ( d->backend )
    {
        disconnect( d->backend );
        d->backend->disconnect( this );
    }

    d->backend = backend;
}

QObject *Solid::ManagerBase::loadBackend( const QString &description, const char *serviceName, const char *backendClassName )
{
    QStringList error_msg;

    QObject *backend = 0;

    KService::List offers = KServiceTypeTrader::self()->query( serviceName, "(Type == 'Service')" );

    foreach ( KService::Ptr ptr, offers )
    {
        KLibFactory * factory = KLibLoader::self()->factory( QFile::encodeName( ptr->library() ) );

        if ( factory )
        {
            backend = factory->create( 0, backendClassName, QStringList() );

            if( backend != 0 )
            {
                kDebug() << "Backend loaded: " << ptr->name() << endl;
                break;
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
            d->errorText = i18n( "No %1 Backend found", description );
        }
        else
        {
            d->errorText = "<qt>";
            d->errorText+= i18n( "Unable to use any of the %1 Backends", description );
            d->errorText+= "<table>";

            QString line = "<tr><td><b>%1</b></td><td>%2</td></tr>";

            for ( int i = 0; i< offers.size(); i++ )
            {
                d->errorText+= line.arg( offers[i]->name() ).arg( error_msg[i] );
            }

            d->errorText+= "</table></qt>";
        }
    }

    return backend;
}

#include "managerbase.moc"

