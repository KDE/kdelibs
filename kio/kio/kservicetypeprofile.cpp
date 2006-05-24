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
#include "kservice.h"
#include "kservicetype.h"
#include "kservicetypefactory.h"

#include <kconfig.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kstaticdeleter.h>

#include <qhash.h>
#include <QtAlgorithms>
#include "kservicefactory.h"

// servicetype -> profile
class KServiceTypeProfileList : public QMultiMap<QString, KServiceTypeProfile *>
{
public:
    ~KServiceTypeProfileList() {
        KServiceTypeProfileList::const_iterator it = begin();
        for ( ; it != end() ; ++it )
            delete *it;
    }
};

KServiceTypeProfileList* KServiceTypeProfile::s_lstProfiles = 0;
static KStaticDeleter< KServiceTypeProfileList > profileDeleter;
bool KServiceTypeProfile::s_configurationMode = false;

KServiceTypeProfile* KServiceTypeProfile::findProfile( const QString& mimeType, const QString& genservicetype )
{
    initStatic();
    // This uses a standard multimap-lookup to find the entry where
    // both mimeType and generic servicetype match.
    KServiceTypeProfileList::const_iterator it = s_lstProfiles->find( mimeType );
    while (it != s_lstProfiles->end() && it.key() == mimeType) {
        if ( it.value()->m_strGenericServiceType == genservicetype )
            return it.value();
        ++it;
    }
    return 0;
}

void KServiceTypeProfile::initStatic()
{
  if ( s_lstProfiles )
    return;

  // Make sure that a KServiceTypeFactory gets created.
  (void) KServiceTypeFactory::self();

  profileDeleter.setObject(s_lstProfiles, new KServiceTypeProfileList);

  KConfig config( "profilerc", true, false);

  static const QString & defaultGroup = KGlobal::staticQString("<default>");

  QStringList tmpList = config.groupList();
  for (QStringList::Iterator aIt = tmpList.begin();
       aIt != tmpList.end(); ++aIt) {
    if ( *aIt == defaultGroup )
      continue;

    config.setGroup( *aIt );

    QString appId = config.readEntry( "Application" );

    // DF: we used to look up every service... but we don't really need to, at this point.
    // KService::Ptr pService = KService::serviceByStorageId(appId);
    //if ( pService ) {
    //  appId = pService->storageId();

    QString type = config.readEntry( "ServiceType" );
    QString type2 = config.readEntry( "GenericServiceType" );
    int pref = config.readEntry( "Preference", 0 );

    if ( !type.isEmpty() /* && pref >= 0*/ ) // Don't test for pref here. We want those in the list, to mark them as forbidden
    {
        KServiceTypeProfile* p = findProfile( type, type2 );

        if ( !p ) {
          p = new KServiceTypeProfile( type, type2 );
          s_lstProfiles->insert( type, p );
        }

        const bool allow = config.readEntry( "AllowAsDefault", false );
        //kDebug(7014) << "KServiceTypeProfile::initStatic adding service " << appId << " to profile for " << type << "," << type2 << " with preference " << pref << endl;
        p->addService( appId, pref, allow );
    }
  }
}

//static
void KServiceTypeProfile::clear()
{
    profileDeleter.destructObject();
}

KServiceTypeProfile::KServiceTypeProfile( const QString& _mimeType, const QString& _genericServiceType )
{
    initStatic();

    m_strServiceType = _mimeType;
    m_strGenericServiceType = _genericServiceType;
}

KServiceTypeProfile::~KServiceTypeProfile()
{
}

void KServiceTypeProfile::addService( const QString& _service,
				      int _preference, bool _allow_as_default )
{
    m_mapServices[ _service ].m_iPreference = _preference;
    m_mapServices[ _service ].m_bAllowAsDefault = _allow_as_default;
}

KServiceOfferList KServiceTypeProfile::sortServiceTypeOffers( const KService::List& list, const QString& serviceType )
{
    initStatic();

    // ######## TODO simplify s_lstProfiles to be a single-map for servicetypes.
    KServiceTypeProfile* profile = 0;
    KServiceTypeProfileList::const_iterator itprof = s_lstProfiles->find( serviceType ); // ## -> value()
    if (itprof != s_lstProfiles->end()) {
        profile = itprof.value();
    }

    KServiceOfferList offers;

    KService::List::const_iterator it = list.begin();
    const KService::List::const_iterator end = list.end();
    for( ; it != end; ++it )
    {
        //kDebug(7014) << "KServiceTypeProfile::offers considering " << (*it)->name() << endl;
        // Look into the profile (if there's one), to find this service's preference.
        bool foundInProfile = false;
        if ( profile )
        {
            QMap<QString,Service>::ConstIterator it2 = profile->m_mapServices.find( (*it)->storageId() );
            if( it2 != profile->m_mapServices.end() )
            {
                const Service& userService = it2.value();
                //kDebug(7014) << "found in mapServices pref=" << it2.value().m_iPreference << endl;
                if ( userService.m_iPreference > 0 ) {
                    const bool allow = (*it)->allowAsDefault() && userService.m_bAllowAsDefault;
                    offers.append( KServiceOffer( (*it), userService.m_iPreference, allow ) );
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
            //kDebug(7014) << "not found in mapServices. Appending." << endl;

            // We use 0 as the preference to ensure new apps don't take over existing apps (which default to 1)
            offers.append( KServiceOffer( (*it), 0, (*it)->allowAsDefault() ) );
        }
    }

    qStableSort( offers );

    //kDebug(7014) << "KServiceTypeProfile::offers returning " << offers.count() << " offers" << endl;
    return offers;
}

KServiceOfferList KServiceTypeProfile::sortMimeTypeOffers( const KService::List& list, const QString& mimeType, const QString& genericServiceType )
{
    initStatic();

    KServiceOfferList offers;

    KServiceTypeProfile* profile = findProfile( mimeType, genericServiceType );

    // Assign preferences from profilerc to those offers.
    KService::List::const_iterator it = list.begin();
    const KService::List::const_iterator end = list.end();
    for( ; it != end; ++it )
    {
        //kDebug(7014) << "KServiceTypeProfile::offers considering " << (*it)->name() << endl;
        if ( genericServiceType.isEmpty() || (*it)->hasServiceType( genericServiceType ) )
        {
            // Now look into the profile, to find this service's preference.
            bool foundInProfile = false;
            if ( profile )
            {
                QMap<QString,Service>::ConstIterator it2 = profile->m_mapServices.find( (*it)->storageId() );
                if( it2 != profile->m_mapServices.end() )
                {
                    const Service& userService = it2.value();
                    //kDebug(7014) << "found in mapServices pref=" << it2.data().m_iPreference << endl;
                    if ( userService.m_iPreference > 0 ) {
                        const bool allow = (*it)->allowAsDefault() && userService.m_bAllowAsDefault;
                        offers.append( KServiceOffer( (*it), userService.m_iPreference, allow ) );
                    }
                    foundInProfile = true;
                }
            }
            if ( !foundInProfile )
            {
                // This offer isn't in the profile
                // This can be because we have no profile at all, or because
                // the services have been installed after the profile was written.
                //kDebug(7014) << "not found in mapServices. Appending." << endl;

                offers.append( KServiceOffer( (*it), (*it)->initialPreferenceForMimeType(mimeType),
                                              (*it)->allowAsDefault() ) );
            }
        }/* else
            kDebug(7014) << "Doesn't have " << m_strGenericServiceType << endl;*/
    }

    qStableSort( offers );

    return offers;
}

/*********************************************
 *
 * KServiceOffer
 *
 *********************************************/

KServiceOffer::KServiceOffer()
{
  m_iPreference = -1;
}

KServiceOffer::KServiceOffer( const KServiceOffer& _o )
{
  m_pService = _o.m_pService;
  m_iPreference = _o.m_iPreference;
  m_bAllowAsDefault = _o.m_bAllowAsDefault;
}

KServiceOffer::KServiceOffer( KService::Ptr _service, int _pref, bool _default )
{
  m_pService = _service;
  m_iPreference = _pref;
  m_bAllowAsDefault = _default;
}


bool KServiceOffer::operator< ( const KServiceOffer& _o ) const
{
  // Put offers allowed as default FIRST.
  if ( _o.m_bAllowAsDefault && !m_bAllowAsDefault )
    return false; // _o is default and not 'this'.
  if ( !_o.m_bAllowAsDefault && m_bAllowAsDefault )
    return true; // 'this' is default but not _o.
 // Both offers are allowed or not allowed as default
 // -> use preferences to sort them
 // The bigger the better, but we want the better FIRST
  return _o.m_iPreference < m_iPreference;
}
