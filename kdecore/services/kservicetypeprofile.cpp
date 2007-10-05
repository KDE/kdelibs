/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Torben Weis <weis@kde.org>
 *  Copyright (C) 2006 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kservicetypeprofile.h"
#include "kservicetypeprofile_p.h"
#include "kservice.h"
#include "kservicetype.h"
#include "kservicetypefactory.h"
#include "kservicefactory.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kconfiggroup.h>

#include <QtCore/QHash>
#include <QtAlgorithms>

// servicetype -> profile
class KServiceTypeProfiles : public QHash<QString, KServiceTypeProfileEntry *>
{
public:
    ~KServiceTypeProfiles() { clear(); }
    void clear() {
        const_iterator it = begin();
        for ( ; it != end() ; ++it )
            delete *it;
        QHash<QString, KServiceTypeProfileEntry *>::clear();
    }
};

// mimetype -> profile, with multiple entries (one per generic servicetype)
class KMimeTypeProfiles : public QMultiHash<QString, KMimeTypeProfileEntry *>
{
public:
    ~KMimeTypeProfiles() { clear(); }
    void clear() {
        const_iterator it = begin();
        for ( ; it != end() ; ++it )
            delete *it;
        QMultiHash<QString, KMimeTypeProfileEntry *>::clear();
    }
};

K_GLOBAL_STATIC(KServiceTypeProfiles, s_serviceTypeProfiles)
K_GLOBAL_STATIC(KMimeTypeProfiles, s_mimeTypeProfiles)

static bool s_configurationMode = false;
static bool s_profilesParsed = false;

static KMimeTypeProfileEntry* findMimeTypeProfile( const QString& mimeType, const QString& genservicetype )
{
    Q_ASSERT( !genservicetype.isEmpty() );
    // This uses a standard multihash-lookup to find the entry where
    // both mimeType and generic servicetype match.
    KMimeTypeProfiles::const_iterator it = s_mimeTypeProfiles->find( mimeType );
    while (it != s_mimeTypeProfiles->end() && it.key() == mimeType) {
        if ( it.value()->m_strGenericServiceType == genservicetype )
            return it.value();
        ++it;
    }
    return 0;
}

static void initStatic()
{
    if ( s_profilesParsed )
        return;
    s_profilesParsed = true;

    // Make sure that a KServiceTypeFactory gets created.
    (void) KServiceTypeFactory::self();

    {
        // Read the service type profiles from servicetype_profilerc (new in kde4)
        // See writeServiceTypeProfile for a description of the file format.
        // ### Since this new format names groups after servicetypes maybe we can even
        // avoid doing any init upfront, and just look up the group when asked...
        KConfig configFile( "servicetype_profilerc", KConfig::CascadeConfig );
        const QStringList tmpList = configFile.groupList();
        for (QStringList::const_iterator aIt = tmpList.begin();
             aIt != tmpList.end(); ++aIt) {
            const QString type = *aIt;
            KConfigGroup config(&configFile, type);
            const int count = config.readEntry( "NumberOfEntries", 0 );
            KServiceTypeProfileEntry* p = s_serviceTypeProfiles->value( type, 0 );
            if ( !p ) {
                p = new KServiceTypeProfileEntry();
                s_serviceTypeProfiles->insert( type, p );
            }

            for ( int i = 0; i < count; ++i ) {
                const QString num = QString::number(i);
                const QString serviceId = config.readEntry( "Entry" + num + "_Service", QString() );
                Q_ASSERT(!serviceId.isEmpty());
                const int pref = config.readEntry( "Entry" + num + "_Preference", 0 );
                //kDebug(7014) << "KServiceTypeProfile::initStatic adding service " << serviceId << " to profile for " << type << " with preference " << pref;
                p->addService( serviceId, pref );
            }
        }
    }

    KConfig profilerc( "profilerc", KConfig::CascadeConfig );

    const QStringList tmpList = profilerc.groupList();
    for (QStringList::const_iterator aIt = tmpList.begin();
         aIt != tmpList.end(); ++aIt) {
        if ( *aIt == "<default>" )
            continue;

        KConfigGroup config(&profilerc, *aIt );

        QString appId = config.readEntry( "Application" );

        // DF: we used to look up every service... but we don't really need to, at this point.
        // KService::Ptr pService = KService::serviceByStorageId(appId);
        //if ( pService ) {
        //  appId = pService->storageId();

        const QString type = config.readEntry( "ServiceType" );
        const QString type2 = config.readEntry( "GenericServiceType" );
        int pref = config.readEntry( "Preference", 0 );

        if ( !type.isEmpty() /* && pref >= 0*/ ) // Don't test for pref here. We want those in the list, to mark them as forbidden
        {
            KMimeTypeProfileEntry* p = findMimeTypeProfile( type, type2 );

            if ( !p ) {
                p = new KMimeTypeProfileEntry( type2 ); // type is the key, it's not stored in KMimeTypeProfileEntry
                s_mimeTypeProfiles->insert( type, p );
            }

            const bool allow = config.readEntry( "AllowAsDefault", false );
            //kDebug(7014) << "KServiceTypeProfile::initStatic adding service " << appId << " to profile for " << type << "," << type2 << " with preference " << pref;
            p->addService( appId, pref, allow );
        }
    }
}

//static
void KServiceTypeProfile::clearCache()
{
    s_serviceTypeProfiles->clear();
    s_mimeTypeProfiles->clear();
    s_profilesParsed = false;
}

void KMimeTypeProfileEntry::addService( const QString& _service,
                                        int _preference, bool _allow_as_default )
{
    ServiceFlags& flags = m_mapServices[ _service ];
    flags.m_iPreference = _preference;
    flags.m_bAllowAsDefault = _allow_as_default;
}

/**
 * Returns the offers in the profile for the requested service type.
 * @param list list of offers (including initialPreference)
 * @param servicetype the service type
 * @return the weighted and sorted offer list
 * @internal used by KServiceTypeTrader
 */
namespace KServiceTypeProfile {
    KServiceOfferList sortServiceTypeOffers( const KServiceOfferList& list, const QString& servicetype );
}

KServiceOfferList KServiceTypeProfile::sortServiceTypeOffers( const KServiceOfferList& list, const QString& serviceType )
{
    initStatic();

    KServiceTypeProfileEntry* profile = s_serviceTypeProfiles->value(serviceType, 0);

    KServiceOfferList offers;

    KServiceOfferList::const_iterator it = list.begin();
    const KServiceOfferList::const_iterator end = list.end();
    for( ; it != end; ++it )
    {
        const KService::Ptr servPtr = (*it).service();
        //kDebug(7014) << "KServiceTypeProfile::offers considering " << servPtr->storageId();
        // Look into the profile (if there's one), to find this service's preference.
        bool foundInProfile = false;
        if ( profile )
        {
            QMap<QString,int>::ConstIterator it2 = profile->m_mapServices.find( servPtr->storageId() );
            if( it2 != profile->m_mapServices.end() )
            {
                const int pref = it2.value();
                //kDebug(7014) << "found in mapServices pref=" << pref;
                if ( pref > 0 ) { // 0 disables the service
                    offers.append( KServiceOffer( servPtr, pref, 0, servPtr->allowAsDefault() ) );
                }
                foundInProfile = true;
            }
        }
        if ( !foundInProfile )
        {
            // This offer isn't in the profile
            // This can be because we have no profile at all, or because the
            // services have been installed after the profile was written,
            // but it's also the case for any service that's neither App nor ReadOnlyPart, e.g. RenameDlg/Plugin
            //kDebug(7014) << "not found in mapServices. Appending.";

            // If there's a profile, we use 0 as the preference to ensure new apps don't take over existing apps (which default to 1)
            offers.append( KServiceOffer( servPtr,
                                          profile ? 0 : (*it).preference(),
                                          0,
                                          servPtr->allowAsDefault() ) );
        }
    }

    qStableSort( offers );

    //kDebug(7014) << "KServiceTypeProfile::offers returning " << offers.count() << " offers";
    return offers;
}

/**
 * Sort the offers for the requested mime type according to the profile (if any),
 * and filter for genericServiceType. This method is really internal to KMimeTypeTrader
 * and might go away at any time.
 *
 * @param list list of offers (key=service, value=initialPreference)
 * @param mimeType the mime type
 * @param genericServiceType the generic service type (e.g. "Application"
 *                           or "KParts/ReadOnlyPart"). Can be QString(),
 *                           then the "Application" generic type will be used
 * @return the weighted and sorted offer list
 * @internal used by KMimeTypeTrader
 */
namespace KServiceTypeProfile {
    KServiceOfferList sortMimeTypeOffers( const KServiceOfferList& list, const QString& mimeType, const QString & genericServiceType );
}

KServiceOfferList KServiceTypeProfile::sortMimeTypeOffers( const KServiceOfferList& list, const QString& mimeType, const QString& genericServiceType )
{
    initStatic();

    KServiceOfferList offers;

    KMimeTypeProfileEntry* profile = findMimeTypeProfile( mimeType, genericServiceType );

    KServiceType::Ptr genericServiceTypePtr( 0 );
    if ( !genericServiceType.isEmpty() ) {
        genericServiceTypePtr = KServiceType::serviceType( genericServiceType );
        Q_ASSERT( genericServiceTypePtr );
    }

    // Assign preferences from profilerc to those offers.
    KServiceOfferList::const_iterator it = list.begin();
    const KServiceOfferList::const_iterator end = list.end();
    for( ; it != end; ++it )
    {
        const KService::Ptr servPtr = (*it).service();
        //kDebug(7014) << "KServiceTypeProfile::offers considering " << servPtr->name();
        if ( !genericServiceTypePtr ||
             // Expand servPtr->hasServiceType( genericServiceTypePtr ) to avoid lookup each time:
             KServiceFactory::self()->hasOffer( genericServiceTypePtr->offset(),
                                                genericServiceTypePtr->serviceOffersOffset(),
                                                servPtr->offset() )
            )
        {
            // Now look into the profile, to find this service's preference.
            bool foundInProfile = false;
            if ( profile )
            {
                QMap<QString,KMimeTypeProfileEntry::ServiceFlags>::ConstIterator it2 = profile->m_mapServices.find( servPtr->storageId() );
                if( it2 != profile->m_mapServices.end() )
                {
                    const KMimeTypeProfileEntry::ServiceFlags& userService = it2.value();
                    //kDebug(7014) << "found in mapServices pref=" << it2.value().m_iPreference;
                    if ( userService.m_iPreference > 0 ) {
                        const bool allow = servPtr->allowAsDefault() && userService.m_bAllowAsDefault;
                        offers.append( KServiceOffer( servPtr, userService.m_iPreference, 0, allow ) );
                    }
                    foundInProfile = true;
                }
            }
            if ( !foundInProfile )
            {
                // This offer isn't in the profile
                // This can be because we have no profile at all, or because
                // the services have been installed after the profile was written.
                //kDebug(7014) << "not found in mapServices. Appending.";

                // If there's a profile, we use 0 as the preference to ensure new apps don't take over existing apps (which default to 1)
                offers.append( KServiceOffer( servPtr,
                                              profile ? 0 : (*it).preference(),
                                              profile ? 0 : (*it).mimeTypeInheritanceLevel(),
                                              servPtr->allowAsDefault() ) );
            }
        }/* else
            kDebug(7014) << "Doesn't have " << genericServiceType;*/
    }

    qStableSort( offers );

    return offers;
}

bool KServiceTypeProfile::hasProfile( const QString& serviceType )
{
    initStatic();
    return s_serviceTypeProfiles->find( serviceType ) != s_serviceTypeProfiles->end();
}

void KServiceTypeProfile::writeServiceTypeProfile( const QString& serviceType,
                                                   const KService::List& services,
                                                   const KService::List& disabledServices )
{
    /*
     * [ServiceType]
     * NumEntries=3
     * Entry0_Service=serv.desktop
     * Entry0_Preference=10
     * Entry1_Service=otherserv.desktop
     * Entry1_Preference=5
     * Entry2_Service=broken_service.desktop
     * Entry2_Preference=0
     */

    KConfig configFile( "servicetype_profilerc", KConfig::SimpleConfig);
    configFile.deleteGroup( serviceType );

    KConfigGroup config(&configFile, serviceType );
    const int count = services.count();
    config.writeEntry( "NumberOfEntries", count + disabledServices.count() );
    KService::List::ConstIterator servit = services.begin();
    int i = 0;
    for( ; servit != services.end(); ++servit, ++i ) {
        const QString num = QString::number(i);
        config.writeEntry( "Entry" + num + "_Service", (*servit)->storageId() );
        config.writeEntry( "Entry" + num + "_Preference", count - i );
    }
    servit = disabledServices.begin();
    for( ; servit != disabledServices.end(); ++servit, ++i ) {
        const QString num = QString::number(i);
        config.writeEntry( "Entry" + num + "_Service", (*servit)->storageId() );
        config.writeEntry( "Entry" + num + "_Preference", 0 );
    }
    configFile.sync();

    // Drop the whole cache...
    clearCache();
}

void KServiceTypeProfile::deleteServiceTypeProfile( const QString& serviceType)
{
    KConfig config( "servicetype_profilerc", KConfig::SimpleConfig );
    config.deleteGroup( serviceType );
    config.sync();

    if (s_serviceTypeProfiles)
        s_serviceTypeProfiles->remove( serviceType );
}

void KServiceTypeProfile::setConfigurationMode()
{
     s_configurationMode = true;
}

bool KServiceTypeProfile::configurationMode()
{
    return s_configurationMode;
}
