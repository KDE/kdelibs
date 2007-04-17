/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

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

#include "managerbase_p.h"

#include <kservicetypetrader.h>
#include <kservice.h>
#include <klibloader.h>

#include <klocale.h>
#include <kdebug.h>

static QMap<QString, QObject*> _k_preloadedBackends;

Solid::ManagerBasePrivate::ManagerBasePrivate()
    : backend(0)
{
}

Solid::ManagerBasePrivate::~ManagerBasePrivate()
{
}

void Solid::ManagerBasePrivate::loadBackend(const QString &description, const char *serviceName,
                                            const char *backendClassName)
{
    if (_k_preloadedBackends.contains(backendClassName)) {
        backend = _k_preloadedBackends[backendClassName];
        return;
    }

    QStringList error_msg;

    KService::List offers = KServiceTypeTrader::self()->query(serviceName, "(Type == 'Service')");

    foreach (KService::Ptr ptr, offers)
    {
        int error = 0;
        backend = KService::createInstance<QObject>(ptr, 0, QStringList(), &error);

        if( backend != 0 )
        {
            if ( backend->inherits( backendClassName ) )
            {
                kDebug() << "Backend loaded: " << ptr->name() << endl;
                break;
            }
            else
            {
                QString error_string = i18n( "Backend loaded but wrong type obtained, expected %1",
                                             backendClassName );

                kDebug() << "Error loading '" << ptr->name() << "': " << error_string << endl;
                error_msg.append( error_string );

                delete backend;
                backend = 0;
            }
        }
        else
        {
            QString error_string = KLibLoader::errorString( error );
            kDebug() << "Error loading '" << ptr->name() << "', KLibLoader said: " << error_string << endl;
            error_msg.append( error_string );
        }
    }

    if ( backend == 0 )
    {
        if ( offers.size() == 0 )
        {
            errorText = i18n("No %1 Backend found", description);
        }
        else
        {
            errorText = "<qt>";
            errorText+= i18n("Unable to use any of the %1 Backends", description);
            errorText+= "<table>";

            QString line = "<tr><td><b>%1</b></td><td>%2</td></tr>";

            for ( int i = 0; i< offers.size(); i++ )
            {
                errorText+= line.arg(offers[i]->name()).arg(error_msg[i]);
            }

            errorText+= "</table></qt>";
        }
    }
}

void Solid::ManagerBasePrivate::_k_forcePreloadedBackend(const char *backendClassName, QObject *backend)
{
    _k_preloadedBackends[backendClassName] = backend;
}


