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

#include "kservicetypetrader.h"

#include "ktraderparsetree.h"
#include <kservicetypeprofile.h>
#include <kstaticdeleter.h>
#include <kdebug.h>
#include "kservicetype.h"

template class KStaticDeleter<KServiceTypeTrader>;

using namespace KTraderParse;

// --------------------------------------------------

KServiceTypeTrader* KServiceTypeTrader::s_self = 0;
static KStaticDeleter<KServiceTypeTrader> ktradersd;

KServiceTypeTrader* KServiceTypeTrader::self()
{
    if ( !s_self )
	ktradersd.setObject( s_self, new KServiceTypeTrader );

    return s_self;
}

KServiceTypeTrader::KServiceTypeTrader()
{
}

KServiceTypeTrader::~KServiceTypeTrader()
{
}

// shared with KMimeTypeTrader
void KServiceTypeTrader::applyConstraints( KService::List& lst,
                                const QString& constraint )
{
    if ( lst.isEmpty() || constraint.isEmpty() )
        return;

    const ParseTreeBase::Ptr constr = parseConstraints( constraint ); // for ownership
    const ParseTreeBase* pConstraintTree = constr.data(); // for speed

    if ( !!constr )
    {
        // Find all services matching the constraint
        // and remove the other ones
        KService::List::iterator it = lst.begin();
        while( it != lst.end() )
        {
            if ( matchConstraint( pConstraintTree, (*it), lst ) != 1 )
                it = lst.erase( it );
            else
                ++it;
        }
    }
    // TODO: catch parse errors here (to delete the partial tree)
}

#if 0
static void dumpOfferList( const KServiceOfferList& offers )
{
    kDebug(7014) << "Sorted list:" << endl;
    OfferList::Iterator itOff = offers.begin();
    for( ; itOff != offers.end(); ++itOff )
        kDebug(7014) << (*itOff).service()->name() << " allow-as-default=" << (*itOff).allowAsDefault() << " preference=" << (*itOff).preference() << endl;
}
#endif


KServiceOfferList KServiceTypeTrader::weightedOffers( const QString& serviceType ) const
{
    KServiceOfferList offers = KServiceTypeProfile::serviceTypeProfileOffers( serviceType );
    //kDebug(7014) << "KServiceTypeTrader::weightedOffers( " << serviceType << " )" << endl;

    // Note that KServiceTypeTrader::offers() calls KServiceType::offers(),
    // so we _do_ get the new services, that are available but not in the profile.
    //kDebug(7014) << "Found profile: " << offers.count() << " offers" << endl;

    // Collect services, to make the next loop faster
    QStringList serviceList;
    KServiceOfferList::const_iterator itOffers = offers.begin();
    for( ; itOffers != offers.end(); ++itOffers )
        serviceList += (*itOffers).service()->desktopEntryPath(); // this should identify each service uniquely
    //kDebug(7014) << "serviceList: " << serviceList.join(",") << endl;

    // Now complete with any other offers that aren't in the profile
    // This can be because the services have been installed after the profile was written,
    // but it's also the case for any service that's neither App nor ReadOnlyPart, e.g. RenameDlg/Plugin
    const KService::List list = KServiceType::offers( serviceType );
    //kDebug(7014) << "Using KServiceType::offers, result: " << list.count() << " offers" << endl;
    KService::List::const_iterator it = list.begin();
    for( ; it != list.end(); ++it )
    {
        // Check that we don't already have it ;)
        if ( !serviceList.contains( (*it)->desktopEntryPath() ) )
        {
            bool allow = (*it)->allowAsDefault();
            offers.append( KServiceOffer( (*it), 1, allow ) );
            //kDebug(7014) << "Appending offer " << (*it)->name() << " initial preference=" << (*it)->initialPreference() << " allow-as-default=" << allow << endl;
        }
        //else
        //    kDebug(7014) << "Already having offer " << (*it)->name() << endl;
    }

    if (!offers.isEmpty())
        qStableSort( offers );

#if 0
    dumpOfferList( offers );
#endif

    return offers;
}

KService::List KServiceTypeTrader::query( const QString& serviceType,
                               const QString& constraint ) const
{
    // Get all services of this service type.
    const KServiceOfferList offers = weightedOffers( serviceType );

    // Now extract only the services; the weighting was only used for sorting.
    KService::List lst;
    KServiceOfferList::const_iterator itOff = offers.begin();
    for( ; itOff != offers.end(); ++itOff )
        lst.append( (*itOff).service() );

    applyConstraints( lst, constraint );

    kDebug(7014) << "query for serviceType " << serviceType
                 << " : returning " << lst.count() << " offers" << endl;
    return lst;
}

KService::Ptr KServiceTypeTrader::preferredService( const QString & serviceType ) const
{
    const KServiceOfferList offers = weightedOffers( serviceType );

    KServiceOfferList::const_iterator itOff = offers.begin();
    // Look for the first one that is allowed as default.
    // Since the allowed-as-default are first anyway, we only have
    // to look at the first one to know.
    if( itOff != offers.end() && (*itOff).allowAsDefault() )
        return (*itOff).service();

    //kDebug(7014) << "No offers, or none allowed as default" << endl;
    return KService::Ptr();
}
