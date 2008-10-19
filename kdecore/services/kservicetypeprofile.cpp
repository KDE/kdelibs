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
#include <kstandarddirs.h>
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
        qDeleteAll( *this );
        QHash<QString, KServiceTypeProfileEntry *>::clear();
    }
};


K_GLOBAL_STATIC(KServiceTypeProfiles, s_serviceTypeProfiles)

static bool s_configurationMode = false;
static bool s_profilesParsed = false;

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
        KConfig configFile( "servicetype_profilerc", KConfig::NoGlobals );
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
}

//static
void KServiceTypeProfile::clearCache()
{
    s_serviceTypeProfiles->clear();
    s_profilesParsed = false;
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
            if( it2 != profile->m_mapServices.constEnd() )
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
