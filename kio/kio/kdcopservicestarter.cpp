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
#include "ktrader.h"
#include <kapplication.h>
#include "kservice.h"
#include <kstaticdeleter.h>
#include <kdebug.h>
#include <klocale.h>
#include <dcopclient.h>
#include <ktoolinvocation.h>

#include <q3valuelist.h>

static KStaticDeleter<KDCOPServiceStarter> dss_sd;
KDCOPServiceStarter* KDCOPServiceStarter::s_self;

KDCOPServiceStarter* KDCOPServiceStarter::self()
{
    if ( !s_self )
        dss_sd.setObject( s_self, new KDCOPServiceStarter );
    return s_self;
}

KDCOPServiceStarter::KDCOPServiceStarter()
{
    // Set the singleton instance - useful when a derived KDCOPServiceStarter
    // was created (before self() was called)
    s_self = this;
}

KDCOPServiceStarter::~KDCOPServiceStarter()
{
}

int KDCOPServiceStarter::findServiceFor( const QString& serviceType,
                                         const QString& _constraint,
                                         const QString& preferences,
                                         QString *error, DCOPCString* pDcopService,
                                         int flags )
{
    // Ask the trader which service is preferred for this servicetype
    // We want one that provides a DCOP interface
    QString constraint = _constraint;
    if ( !constraint.isEmpty() )
        constraint += " and ";
    constraint += "exist [X-DCOP-ServiceName]";
    KTrader::OfferList offers = KTrader::self()->query(serviceType, "Application", constraint, preferences);
    if ( offers.isEmpty() ) {
        if ( error )
            *error = i18n("No service implementing %1").arg( serviceType );
        kdWarning() << "KDCOPServiceStarter: No service implementing " << serviceType << endl;
        return -1;
    }
    KService::Ptr ptr = offers.first();
    DCOPCString dcopService = ptr->property("X-DCOP-ServiceName").toString().latin1();

    if ( !KApplication::dcopClient()->isApplicationRegistered( dcopService ) )
    {
        QString error;
        if ( startServiceFor( serviceType, constraint, preferences, &error, &dcopService, flags ) != 0 )
        {
            kdDebug() << "KDCOPServiceStarter: Couldn't start service: " << error << endl;
            return -2;
        }
    }
    kdDebug() << "KDCOPServiceStarter: DCOP service is available now, as " << dcopService << endl;
    if ( pDcopService )
        *pDcopService = dcopService;
    return 0;
}

int KDCOPServiceStarter::startServiceFor( const QString& serviceType,
                                          const QString& constraint,
                                          const QString& preferences,
                                          QString *error, DCOPCString* dcopService, int /*flags*/ )
{
    KTrader::OfferList offers = KTrader::self()->query(serviceType, "Application", constraint, preferences);
    if ( offers.isEmpty() )
        return -1;
    KService::Ptr ptr = offers.first();
    kdDebug() << "KDCOPServiceStarter: starting " << ptr->desktopEntryPath() << endl;
    return KToolInvocation::startServiceByDesktopPath( ptr->desktopEntryPath(), QStringList(), error, dcopService );
}
