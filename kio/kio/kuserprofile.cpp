/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Torben Weis <weis@kde.org>
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
 **/

#include "kuserprofile.h"
#include "kservice.h"
#include "kservicetype.h"
#include "kservicetypefactory.h"
#include "kstandarddirs.h"

#include <kconfig.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kstaticdeleter.h>

#include <qtl.h>

#include <qvaluelist.h>
template class QPtrList<KServiceTypeProfile>;
typedef QPtrList<KServiceTypeProfile> KServiceTypeProfileList;

/*********************************************
 *
 * KServiceTypeProfile
 *
 *********************************************/

KServiceTypeProfileList* KServiceTypeProfile::s_lstProfiles = 0L;
static KStaticDeleter< KServiceTypeProfileList > profileDeleter;
bool KServiceTypeProfile::s_configurationMode = false;

void KServiceTypeProfile::initStatic()
{
  if ( s_lstProfiles )
    return;
  kdDebug(7014) << "KServiceTypeProfile::initStatic" << endl;

  // Make sure that a KServiceTypeFactory gets created.
  (void) KServiceTypeFactory::self();

  profileDeleter.setObject(s_lstProfiles, new KServiceTypeProfileList);
  s_lstProfiles->setAutoDelete( true );

  KConfig config( "profilerc", true, false);

  static const QString & defaultGroup = KGlobal::staticQString("<default>");

  QStringList tmpList = config.groupList();
  for (QStringList::Iterator aIt = tmpList.begin();
       aIt != tmpList.end(); ++aIt) {
    if ( *aIt == defaultGroup )
      continue;

    config.setGroup( *aIt );

    QString appId = config.readEntry( "Application" );

    KService::Ptr pService = KService::serviceByStorageId(appId);

    if ( pService ) {
      QString application = pService->storageId();
      QString type = config.readEntry( "ServiceType" );
      QString type2 = config.readEntry( "GenericServiceType" );
      if (type2.isEmpty()) // compat code
          type2 = (pService->type() == "Application") ? "Application" : "KParts/ReadOnlyPart";
      int pref = config.readNumEntry( "Preference" );

      if ( !type.isEmpty() /* && pref >= 0*/ ) // Don't test for pref here. We want those in the list, to mark them as forbidden
      {
        KServiceTypeProfile* p =
          KServiceTypeProfile::serviceTypeProfile( type, type2 );

        if ( !p ) {
          p = new KServiceTypeProfile( type, type2 );
          s_lstProfiles->append( p );
        }

        bool allow = config.readBoolEntry( "AllowAsDefault" );
        kdDebug(7014) << "KServiceTypeProfile::initStatic adding service " << application << " to profile for " 
                      << type << "," << type2 << " with preference " << pref << endl;
        p->addService( application, pref, allow );
      }
    }
  }
  initKDE4compatibility();
}

  // KDE4 Compatibility
  // in KDE4 profilerc is no longer used instead preffered applications
  // are saved in the file mimeapps.list in xdgdata-apps.
  // The goal of this compatibility code is to parse the mimeapps.list files
  // and use the information from that file for KDE3 and the kuserprofile class.
  //
  // Code is in parts reused from kdelibs 4 /kded/kmimeassociations.cpp

void KServiceTypeProfile::initKDE4compatibility()
{
    // Using the "merged view" from KConfig is not enough since we -add- at every level, we don't replace.
    const QStringList mimeappsFiles = KGlobal::dirs()->findAllResources("xdgdata-apps",
            QString::fromLatin1("mimeapps.list"), true, false);
    if (mimeappsFiles.isEmpty()) {
        kdDebug(7014) << "mimeapps files is empty"<< endl;
        return;
    }

    int basePreference = 1000; // start high :)
    QStringList::const_iterator mimeappsIter = mimeappsFiles.begin();
    for ( ; mimeappsIter != mimeappsFiles.end(); mimeappsIter++ )
    {
        parseMimeAppsList(*(mimeappsIter), basePreference, QString::fromLatin1("Added Associations"));
        basePreference -= 50;
    }
}
void KServiceTypeProfile::parseMimeAppsList(const QString& file, int basePreference, const QString& group)
{
    KConfig applist(file, true, false);
    kdDebug(7014) << "KServiceTypeProfile::ParseMimeAppsList" << file << endl;
    QMap<QString, QString> mimemapping = applist.entryMap(group);

    for (QMap<QString, QString>::Iterator aIt = mimemapping.begin();
         aIt != mimemapping.end(); ++aIt) {
      // Key is the mimetype like message/rfc822 value is the list of associated
      // desktop files

      kdDebug(7014) << "Found mimetype " << aIt.key() << " Mapping: " << aIt.data() << endl;

      QStringList desktopNames = QStringList::split(QString::fromLatin1(";"), aIt.data());
      int pref = basePreference;
      for (QStringList::const_iterator serviceIt = desktopNames.begin();
              serviceIt != desktopNames.end(); serviceIt++) {
            KService::Ptr pService = KService::serviceByStorageId(*(serviceIt));
            if (!pService) {
                kdDebug(7014) << file << " specifies unknown service: " 
                              << *(serviceIt) << " for " << aIt.key() << endl;
            } else {
                KServiceTypeProfile* p =
                    KServiceTypeProfile::serviceTypeProfile( aIt.key(), "Application" );

                if ( !p ) {
                    kdDebug(7014) << "Created new ServiceTypeProfile" << endl;
                    p = new KServiceTypeProfile( aIt.key(), "Application" );
                    s_lstProfiles->append( p );
                }

                kdDebug(7014) << "KServiceTypeProfile::parseMimeAppsList adding service " << *(serviceIt)
                              << " to profile for " << aIt.key() << " with preference " << pref << endl;
                p->addService(*(serviceIt), pref, true);
                --pref;
            }
        }
    }
}


//static
void KServiceTypeProfile::clear()
{
    // HACK ksycoca may open the dummy db, in such case the first call to ksycoca
    // in initStatic() leads to closing the dummy db and clear() being called
    // in the middle of it, making s_lstProfiles be NULL
    if( s_lstProfiles == NULL || s_lstProfiles->count() == 0 )
        return;
    profileDeleter.destructObject();
}

//static
KServiceTypeProfile::OfferList KServiceTypeProfile::offers( const QString& _servicetype, const QString& _genericServiceType )
{
    OfferList offers;
    QStringList serviceList;
    //kdDebug(7014) << "KServiceTypeProfile::offers( " << _servicetype << "," << _genericServiceType << " )" << endl;

    // Note that KServiceTypeProfile::offers() calls KServiceType::offers(),
    // so we _do_ get the new services, that are available but not in the profile.
    if ( _genericServiceType.isEmpty() )
    {
        initStatic();
        // We want all profiles for servicetype, if we have profiles.
        // ## Slow loop, if profilerc is big. We should use a map instead?
        QPtrListIterator<KServiceTypeProfile> it( *s_lstProfiles );
        for( ; it.current(); ++it )
            if ( it.current()->m_strServiceType == _servicetype )
            {
                offers += it.current()->offers();
            }
        //kdDebug(7014) << "Found profile: " << offers.count() << " offers" << endl;
    }
    else
    {
        KServiceTypeProfile* profile = serviceTypeProfile( _servicetype, _genericServiceType );
        if ( profile )
        {
            //kdDebug(7014) << "Found profile: " << profile->offers().count() << " offers" << endl;
            offers += profile->offers();
        }
        else
        {
            // Try the other way round, order is not like size, it doesn't matter.
            profile = serviceTypeProfile( _genericServiceType, _servicetype );
            if ( profile )
            {
                //kdDebug(7014) << "Found profile after switching: " << profile->offers().count() << " offers" << endl;
                offers += profile->offers();
            }
        }
    }

    // Collect services, to make the next loop faster
    OfferList::Iterator itOffers = offers.begin();
    for( ; itOffers != offers.end(); ++itOffers )
        serviceList += (*itOffers).service()->desktopEntryPath(); // this should identify each service uniquely
    kdDebug(7014) << "serviceList: " << serviceList.join(",") << endl;

    // Now complete with any other offers that aren't in the profile
    // This can be because the services have been installed after the profile was written,
    // but it's also the case for any service that's neither App nor ReadOnlyPart, e.g. RenameDlg/Plugin
    KService::List list = KServiceType::offers( _servicetype );
    //kdDebug(7014) << "Using KServiceType::offers, result: " << list.count() << " offers" << endl;
    QValueListIterator<KService::Ptr> it = list.begin();
    for( ; it != list.end(); ++it )
    {
        if (_genericServiceType.isEmpty() /*no constraint*/ || (*it)->hasServiceType( _genericServiceType ))
        {
            // Check that we don't already have it ;)
            if ( serviceList.find( (*it)->desktopEntryPath() ) == serviceList.end() )
            {
                bool allow = (*it)->allowAsDefault();
                KServiceOffer o( (*it), (*it)->initialPreferenceForMimeType(_servicetype), allow );
                offers.append( o );
                //kdDebug(7014) << "Appending offer " << (*it)->name() << " initial preference=" << (*it)->initialPreference() << " allow-as-default=" << allow << endl;
            }
            //else
            //    kdDebug(7014) << "Already having offer " << (*it)->name() << endl;
        }
    }

    qBubbleSort( offers );

#if 1
    // debug code, comment if you wish but don't remove.
    kdDebug(7014) << "Sorted list:" << endl;
    OfferList::Iterator itOff = offers.begin();
    for( ; itOff != offers.end(); ++itOff )
        kdDebug(7014) << (*itOff).service()->name() << " allow-as-default=" << (*itOff).allowAsDefault() << endl;
#endif

    kdDebug(7014) << "Returning " << offers.count() << " offers" << endl;
    return offers;
}

KServiceTypeProfile::KServiceTypeProfile( const QString& _servicetype, const QString& _genericServiceType )
{
  initStatic();

  m_strServiceType = _servicetype;
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

int KServiceTypeProfile::preference( const QString& _service ) const
{
  KService::Ptr service = KService::serviceByName( _service );
  if (!service)
    return 0;
  QMap<QString,Service>::ConstIterator it = m_mapServices.find( service->storageId() );
  if ( it == m_mapServices.end() )
    return 0;

  return it.data().m_iPreference;
}

bool KServiceTypeProfile::allowAsDefault( const QString& _service ) const
{
  KService::Ptr service = KService::serviceByName( _service );
  if (!service)
    return false;

  // Does the service itself not allow that ?
  if ( !service->allowAsDefault() )
    return false;

  // Look what the user says ...
  QMap<QString,Service>::ConstIterator it = m_mapServices.find( service->storageId() );
  if ( it == m_mapServices.end() )
    return 0;

  return it.data().m_bAllowAsDefault;
}

KServiceTypeProfile* KServiceTypeProfile::serviceTypeProfile( const QString& _servicetype, const QString& _genericServiceType )
{
  initStatic();
  static const QString& app_str = KGlobal::staticQString("Application");

  const QString &_genservicetype  = ((!_genericServiceType.isEmpty()) ? _genericServiceType : app_str);

  QPtrListIterator<KServiceTypeProfile> it( *s_lstProfiles );
  for( ; it.current(); ++it )
    if (( it.current()->m_strServiceType == _servicetype ) &&
        ( it.current()->m_strGenericServiceType == _genservicetype))
      return it.current();

  return 0;
}


KServiceTypeProfile::OfferList KServiceTypeProfile::offers() const
{
  OfferList offers;

  //kdDebug(7014) << "KServiceTypeProfile::offers() serviceType=" << m_strServiceType << " genericServiceType=" << m_strGenericServiceType << endl;
  KService::List list = KServiceType::offers( m_strServiceType );

  // KDE4 compatibility:
  // Also look into the services registrerd for this mimetype in the profile
  // but not in the desktop file database.
  // This allows a more generic mapping which is used here to correctly find the
  // preffered application.
  //
  // HACK: Search the map to find those services
  QMapConstIterator<QString, Service> mapIt = m_mapServices.constBegin();
  for ( ; mapIt != m_mapServices.end(); mapIt++ ) {
      KService::Ptr pService = KService::serviceByStorageId(mapIt.key());
      if ( pService ) {
//          kdDebug(7014) << "Additionally adding " << mapIt.key() << " to available services for: "
//                        << m_strServiceType << " as configured. " << endl;
          list.append(pService);
      } else {
          kdDebug(7014) << mapIt.key() << " specifies unknown service in service map for "
                        << m_strServiceType << endl;
      }
  }

  QValueListIterator<KService::Ptr> it = list.begin();
  for( ; it != list.end(); ++it )
  {
    kdDebug(7014) << "KServiceTypeProfile::offers() considering " << (*it)->name() << endl;
    if ( m_strGenericServiceType.isEmpty() || (*it)->hasServiceType( m_strGenericServiceType ) )
    {
      // Now look into the profile, to find this service's preference.
      QMap<QString,Service>::ConstIterator it2 = m_mapServices.find( (*it)->storageId() );

      if( it2 != m_mapServices.end() )
      {
        //kdDebug(7014) << "found in mapServices pref=" << it2.data().m_iPreference << endl;
        if ( it2.data().m_iPreference > 0 ) {
          bool allow = (*it)->allowAsDefault();
          if ( allow )
            allow = it2.data().m_bAllowAsDefault;
          KServiceOffer o( (*it), it2.data().m_iPreference, allow );
          offers.append( o );
        }
      }
      else
      {
        //kdDebug(7014) << "not found in mapServices. Appending." << endl;
        // We use 0 as the preference to ensure new apps don't take over existing apps (which default to 1)
        KServiceOffer o( (*it), 0, (*it)->allowAsDefault() );
        offers.append( o );
      }
    }/* else
      kdDebug(7014) << "Doesn't have " << m_strGenericServiceType << endl;*/
  }

  qBubbleSort( offers );

  kdDebug(7014) << "KServiceTypeProfile::offers returning " << offers.count() << " offers" << endl;
  return offers;
}

KService::Ptr KServiceTypeProfile::preferredService( const QString & _serviceType, const QString & _genericServiceType )
{
  OfferList lst = offers( _serviceType, _genericServiceType );

  OfferList::Iterator itOff = lst.begin();
  // Look for the first one that is allowed as default.
  // Since the allowed-as-default are first anyway, we only have
  // to look at the first one to know.
  if( itOff != lst.end() && (*itOff).allowAsDefault() )
    return (*itOff).service();

  kdDebug(7014) << "No offers, or none allowed as default" << endl;
  return 0L;
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
