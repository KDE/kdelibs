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

#include "kservicetypeprofile.h"
#include "kservicetype.h"
#include "kservicetypetrader.h"
#include "kmimetype.h"
#include "kservicefactory.h"
#include "kmimetypefactory.h"

#include <kdebug.h>

class KMimeTypeTrader::Private
{
public:
    Private() {}
};

KMimeTypeTrader* KMimeTypeTrader::self()
{
    K_GLOBAL_STATIC(KMimeTypeTrader, s_self)
    return s_self;
}

KMimeTypeTrader::KMimeTypeTrader()
    : d(new Private())
{
}

KMimeTypeTrader::~KMimeTypeTrader()
{
    delete d;
}

static KServiceOfferList mimeTypeSycocaOffers(const QString& mimeType)
{
    KServiceOfferList lst;
    KMimeType::Ptr mime = KMimeTypeFactory::self()->findMimeTypeByName( mimeType, KMimeType::ResolveAliases );
    if ( !mime ) {
        kWarning(7014) << "KMimeTypeTrader: mimeType" << mimeType << "not found";
        return lst; // empty
    }
    if ( mime->serviceOffersOffset() > -1 ) {
        lst = KServiceFactory::self()->offers( mime->offset(), mime->serviceOffersOffset() );
    }
    return lst;
}

static KService::List mimeTypeSycocaServiceOffers(const QString& mimeType)
{
    KService::List lst;
    KMimeType::Ptr mime = KMimeTypeFactory::self()->findMimeTypeByName( mimeType, KMimeType::ResolveAliases );
    if ( !mime ) {
        kWarning(7014) << "KMimeTypeTrader: mimeType" << mimeType << "not found";
        return lst; // empty
    }
    if ( mime->serviceOffersOffset() > -1 ) {
        lst = KServiceFactory::self()->serviceOffers( mime->offset(), mime->serviceOffersOffset() );
    }
    return lst;
}

#define CHECK_SERVICETYPE(genericServiceTypePtr) \
    if (!genericServiceTypePtr) { \
        kError(7014) << "KMimeTypeTrader: couldn't find service type" << genericServiceType << \
            "\nPlease ensure that the .desktop file for it is installed; then run kbuildsycoca4."; \
        return; \
    }

/**
 * Filter the offers for the requested mime type for the genericServiceType.
 *
 * @param list list of offers (key=service, value=initialPreference)
 * @param genericServiceType the generic service type (e.g. "Application" or "KParts/ReadOnlyPart")
 */
static void filterMimeTypeOffers(KServiceOfferList& list, const QString& genericServiceType)
{
    KServiceType::Ptr genericServiceTypePtr = KServiceType::serviceType(genericServiceType);
    CHECK_SERVICETYPE(genericServiceTypePtr);

    QMutableListIterator<KServiceOffer> it(list);
    while(it.hasNext()) {
        const KService::Ptr servPtr = it.next().service();
        // Expand servPtr->hasServiceType( genericServiceTypePtr ) to avoid lookup each time:
        if (!KServiceFactory::self()->hasOffer(genericServiceTypePtr->offset(),
                                               genericServiceTypePtr->serviceOffersOffset(),
                                               servPtr->offset())
            || !servPtr->showInKDE()) {
            it.remove();
        }
    }
}

static void filterMimeTypeOffers(KService::List& list, const QString& genericServiceType)
{
    KServiceType::Ptr genericServiceTypePtr = KServiceType::serviceType(genericServiceType);
    CHECK_SERVICETYPE(genericServiceTypePtr);

    QMutableListIterator<KService::Ptr> it(list);
    while(it.hasNext()) {
        const KService::Ptr servPtr = it.next();
        // Expand servPtr->hasServiceType( genericServiceTypePtr ) to avoid lookup each time:
        if (!KServiceFactory::self()->hasOffer(genericServiceTypePtr->offset(),
                                               genericServiceTypePtr->serviceOffersOffset(),
                                               servPtr->offset())
            || !servPtr->showInKDE()) {
            it.remove();
        }
    }
}

#undef CHECK_SERVICETYPE

KService::List KMimeTypeTrader::query( const QString& mimeType,
                                       const QString& genericServiceType,
                                       const QString& constraint ) const
{
    // Get all services of this mime type.
    KService::List lst = mimeTypeSycocaServiceOffers(mimeType);
    filterMimeTypeOffers(lst, genericServiceType);

    KServiceTypeTrader::applyConstraints(lst, constraint);

    kDebug(7014) << "query for mimeType " << mimeType << ", " << genericServiceType
                 << " : returning " << lst.count() << " offers" << endl;
    return lst;
}

KService::Ptr KMimeTypeTrader::preferredService( const QString & mimeType, const QString & genericServiceType )
{
    // First, get all offers known to ksycoca.
    KServiceOfferList offers = mimeTypeSycocaOffers( mimeType );

    // Assign preferences from the profile to those offers - and filter for genericServiceType
    Q_ASSERT(!genericServiceType.isEmpty());
    filterMimeTypeOffers(offers, genericServiceType);

    KServiceOfferList::const_iterator itOff = offers.constBegin();
    // Look for the first one that is allowed as default.
    // Since the allowed-as-default are first anyway, we only have
    // to look at the first one to know.
    if( itOff != offers.constEnd() && (*itOff).allowAsDefault() )
        return (*itOff).service();

    //kDebug(7014) << "No offers, or none allowed as default";
    return KService::Ptr();
}
