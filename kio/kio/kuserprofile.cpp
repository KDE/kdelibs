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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "kuserprofile.h"
#include "kservice.h"
#include "kservicetype.h"
#include "kservicetypefactory.h"

#include <kconfig.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kdebug.h>

#include <qtl.h>

template class QPtrList<KServiceTypeProfile>;

/*********************************************
 *
 * KServiceTypeProfile
 *
 *********************************************/

QPtrList<KServiceTypeProfile>* KServiceTypeProfile::s_lstProfiles = 0L;
bool KServiceTypeProfile::s_configurationMode = false;

void KServiceTypeProfile::initStatic()
{
  if ( s_lstProfiles )
    return;

  // Make sure that a KServiceTypeFactory gets created.
  (void) KServiceTypeFactory::self();

  s_lstProfiles = new QPtrList<KServiceTypeProfile>;

  KSimpleConfig config( "profilerc");

  static const QString & defaultGroup = KGlobal::staticQString("<default>");

  QStringList tmpList = config.groupList();
  for (QStringList::Iterator aIt = tmpList.begin();
       aIt != tmpList.end(); ++aIt) {
    if ( *aIt == defaultGroup )
      continue;

    config.setGroup( *aIt );

    QString appDesktopPath = config.readEntry( "Application" );

    KService::Ptr pService = KService::serviceByDesktopPath( appDesktopPath );

    if ( pService ) {
      QString application = pService->name();
      QString type = config.readEntry( "ServiceType" );
      QString type2 = config.readEntry( "GenericServiceType" );
      if (type2.isEmpty()) // compat code
          type2 = (pService->type() == "Application") ? "Application" : "KParts/ReadOnlyPart";
      int pref = config.readNumEntry( "Preference" );

      if ( !type.isEmpty() /* && pref >= 0*/ ) // Don't test for pref here. We want those in the list, to mark them as forbidden
      {
        KServiceTypeProfile* p =
          KServiceTypeProfile::serviceTypeProfile( type, type2 );

        if ( !p )
          p = new KServiceTypeProfile( type, type2 );

        bool allow = config.readBoolEntry( "AllowAsDefault" );
        //kdDebug(7014) << "KServiceTypeProfile::initStatic adding service " << application << " to profile for " << type << "," << type2 << " with preference " << pref << endl;
        p->addService( application, pref, allow );
      }
    }
  }
}

//static
KServiceTypeProfile::OfferList KServiceTypeProfile::offers( const QString& _servicetype, const QString& _genericServiceType )
{
    OfferList offers;
    QStringList serviceList;
    kdDebug(7014) << "KServiceTypeProfile::offers( " << _servicetype << "," << _genericServiceType << " )" << endl;

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
    //kdDebug(7014) << "serviceList: " << serviceList.join(",") << endl;

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
                KServiceOffer o( (*it), (*it)->initialPreference(), allow );
                offers.append( o );
                //kdDebug(7014) << "Appending offer " << (*it)->name() << " initial preference=" << (*it)->initialPreference() << " allow-as-default=" << allow << endl;
            }
            //else
            //    kdDebug(7014) << "Already having offer " << (*it)->name() << endl;
        }
    }

    qBubbleSort( offers );

#if 0
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

  s_lstProfiles->append( this );
}

KServiceTypeProfile::~KServiceTypeProfile()
{
  Q_ASSERT( s_lstProfiles );

  s_lstProfiles->removeRef( this );
}

void KServiceTypeProfile::addService( const QString& _service,
				      int _preference, bool _allow_as_default )
{
  m_mapServices[ _service ].m_iPreference = _preference;
  m_mapServices[ _service ].m_bAllowAsDefault = _allow_as_default;
}

int KServiceTypeProfile::preference( const QString& _service ) const
{
  QMap<QString,Service>::ConstIterator it = m_mapServices.find( _service );
  if ( it == m_mapServices.end() )
    return 0;

  return it.data().m_iPreference;
}

bool KServiceTypeProfile::allowAsDefault( const QString& _service ) const
{
  // Does the service itself not allow that ?
  KService::Ptr s = KService::serviceByName( _service );
  if ( s && !s->allowAsDefault() )
    return false;

  // Look what the user says ...
  QMap<QString,Service>::ConstIterator it = m_mapServices.find( _service );
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

  kdDebug(7014) << "KServiceTypeProfile::offers serviceType=" << m_strServiceType << " genericServiceType=" << m_strGenericServiceType << endl;
  KService::List list = KServiceType::offers( m_strServiceType );
  QValueListIterator<KService::Ptr> it = list.begin();
  for( ; it != list.end(); ++it )
  {
    //kdDebug(7014) << "KServiceTypeProfile::offers considering " << (*it)->name() << endl;
    if ( m_strGenericServiceType.isEmpty() || (*it)->hasServiceType( m_strGenericServiceType ) )
    {
      // Now look into the profile, to find this service's preference.
      QMap<QString,Service>::ConstIterator it2 = m_mapServices.find( (*it)->name() );

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
        KServiceOffer o( (*it), 1, (*it)->allowAsDefault() );
        offers.append( o );
      }
    }/* else
      kdDebug(7014) << "Doesn't have " << m_strGenericServiceType << endl;*/
  }

  qBubbleSort( offers );

  //kdDebug(7014) << "KServiceTypeProfile::offers returning " << offers.count() << " offers" << endl;
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
