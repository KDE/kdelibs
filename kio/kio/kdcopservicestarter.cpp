/* This file is part of the KDE libraries
   Copyright (C) 2003 David Faure <faure@kde.org>

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

#include "kdcopservicestarter.h"
#include "kservicetypetrader.h"
#include <kapplication.h>
#include "kservice.h"
#include <kstaticdeleter.h>
#include <kdebug.h>
#include <klocale.h>
#include <ktoolinvocation.h>
#include "dbus/qdbus.h"

#ifdef __GCC__
# warning Rename this file after merge
#endif

static KStaticDeleter<KDBusServiceStarter> dss_sd;
KDBusServiceStarter* KDBusServiceStarter::s_self;

KDBusServiceStarter* KDBusServiceStarter::self()
{
    if ( !s_self )
        dss_sd.setObject( s_self, new KDBusServiceStarter );
    return s_self;
}

KDBusServiceStarter::KDBusServiceStarter()
{
    // Set the singleton instance - useful when a derived KDBusServiceStarter
    // was created (before self() was called)
    s_self = this;
}

KDBusServiceStarter::~KDBusServiceStarter()
{
}

int KDBusServiceStarter::findServiceFor( const QString& serviceType,
                                         const QString& _constraint,
                                         QString *error, QString* pDBusService,
                                         int flags )
{
    // Ask the trader which service is preferred for this servicetype
    // We want one that provides a DBus interface
    QString constraint = _constraint;
    if ( !constraint.isEmpty() )
        constraint += " and ";
    constraint += "exist [X-DBus-ServiceName]";
    const KService::List offers = KServiceTypeTrader::self()->query(serviceType, constraint);
    if ( offers.isEmpty() ) {
        if ( error )
            *error = i18n("No service implementing %1",  serviceType );
        kWarning() << "KDBusServiceStarter: No service implementing " << serviceType << endl;
        return -1;
    }
    KService::Ptr ptr = offers.first();
    QString dbusService = ptr->property("X-DBUS-ServiceName").toString();

    if ( !QDBus::sessionBus().busService()->nameHasOwner( dbusService ) )
    {
        QString error;
        if ( startServiceFor( serviceType, constraint, &error, &dbusService, flags ) != 0 )
        {
            kDebug() << "KDBusServiceStarter: Couldn't start service: " << error << endl;
            return -2;
        }
    }
    kDebug() << "KDBusServiceStarter: DBus service is available now, as " << dbusService << endl;
    if ( pDBusService )
        *pDBusService = dbusService;
    return 0;
}

int KDBusServiceStarter::startServiceFor( const QString& serviceType,
                                          const QString& constraint,
                                          QString *error, QString* dbusService, int /*flags*/ )
{
    const KService::List offers = KServiceTypeTrader::self()->query(serviceType, constraint);
    if ( offers.isEmpty() )
        return -1;
    KService::Ptr ptr = offers.first();
    kDebug() << "KDBusServiceStarter: starting " << ptr->desktopEntryPath() << endl;
    return KToolInvocation::startServiceByDesktopPath( ptr->desktopEntryPath(), QStringList(), error, dbusService );
}
