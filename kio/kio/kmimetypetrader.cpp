/* This file is part of the KDE libraries
   Copyright (C) 2000 Torben Weis <weis@kde.org>
   Copyright (C) 2006 David Faure <faure@kde.org>

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

#include "kmimetypetrader.h"

#include "ktraderparsetree.h"
#include <kservicetypeprofile.h>
#include <kstaticdeleter.h>
#include <kdebug.h>
#include "kservicetype.h"
#include "kservicetypetrader.h"

KMimeTypeTrader* KMimeTypeTrader::s_self = 0;
static KStaticDeleter<KMimeTypeTrader> kmimetypetradersd;

KMimeTypeTrader* KMimeTypeTrader::self()
{
    if ( !s_self )
	kmimetypetradersd.setObject( s_self, new KMimeTypeTrader );
    return s_self;
}

KMimeTypeTrader::~KMimeTypeTrader()
{
}

KMimeTypeTrader::KMimeTypeTrader()
{
}

KServiceOfferList KMimeTypeTrader::weightedOffers( const QString& mimeType,
                                                    const QString& genericServiceType ) const
{
    kDebug(7014) << "KMimeTypeTrader::weightedOffers( " << mimeType << "," << genericServiceType << " )" << endl;
    Q_ASSERT( !genericServiceType.isEmpty() );

    // First look into the user preference (profile)
    KServiceOfferList offers = KServiceTypeProfile::mimeTypeProfileOffers( mimeType, genericServiceType );

    // Collect services, to make the next loop faster
    QStringList serviceList;
    KServiceOfferList::const_iterator itOffers = offers.begin();
    for( ; itOffers != offers.end(); ++itOffers )
        serviceList += (*itOffers).service()->desktopEntryPath(); // this should identify each service uniquely
    //kDebug(7014) << "serviceList: " << serviceList.join(",") << endl;

    // Now complete with any other offers that aren't in the profile
    // This can be because the services have been installed after the profile was written,
    // but it's also the case for any service that's neither App nor ReadOnlyPart, e.g. RenameDlg/Plugin
    const KService::List list = KServiceType::offers( mimeType );
    //kDebug(7014) << "Using KServiceType::offers, result: " << list.count() << " offers" << endl;
    KService::List::const_iterator it = list.begin();
    for( ; it != list.end(); ++it )
    {
        if ( (*it)->hasServiceType( genericServiceType ) )
        {
            // Check that we don't already have it ;)
            if ( !serviceList.contains( (*it)->desktopEntryPath() ) )
            {
                bool allow = (*it)->allowAsDefault();
                KServiceOffer o( (*it), (*it)->initialPreferenceForMimeType(mimeType), allow );
                offers.append( o );
                //kDebug(7014) << "Appending offer " << (*it)->name() << " initial preference=" << (*it)->initialPreference() << " allow-as-default=" << allow << endl;
            }
            //else
            //    kDebug(7014) << "Already having offer " << (*it)->name() << endl;
        } //else
        //    kDebug(7014) << (*it)->name() << " doesn't have " << genericServiceType << endl;
    }

    if (!offers.isEmpty())
        qStableSort( offers );

    return offers;
}

KService::List KMimeTypeTrader::query( const QString& mimeType,
                                       const QString& genericServiceType,
                                       const QString& constraint ) const
{
    // Get all services of this mime type.
    const KServiceOfferList offers = weightedOffers( mimeType, genericServiceType );

    // Now extract only the services; the weighting was only used for sorting.
    KService::List lst;
    KServiceOfferList::const_iterator itOff = offers.begin();
    for( ; itOff != offers.end(); ++itOff )
        lst.append( (*itOff).service() );

    KServiceTypeTrader::applyConstraints( lst, constraint );

    kDebug(7014) << "query for mimeType " << mimeType << " , " << genericServiceType
                 << " : returning " << lst.count() << " offers" << endl;
    return lst;
}

KService::Ptr KMimeTypeTrader::preferredService( const QString & mimeType, const QString & genericServiceType )
{
    const KServiceOfferList offers = weightedOffers( mimeType, genericServiceType );

    KServiceOfferList::const_iterator itOff = offers.begin();
    // Look for the first one that is allowed as default.
    // Since the allowed-as-default are first anyway, we only have
    // to look at the first one to know.
    if( itOff != offers.end() && (*itOff).allowAsDefault() )
        return (*itOff).service();

    //kDebug(7014) << "No offers, or none allowed as default" << endl;
    return KService::Ptr();
}
