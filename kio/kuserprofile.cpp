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

#include <kconfig.h>
#include <kapp.h>
#include <kdebug.h>

#include <qtl.h>

template class QList<KServiceTypeProfile>;

/*********************************************
 *
 * KServiceTypeProfile
 *
 *********************************************/

QList<KServiceTypeProfile>* KServiceTypeProfile::s_lstProfiles = 0L;

void KServiceTypeProfile::initStatic()
{
  if ( s_lstProfiles )
    return;

  s_lstProfiles = new QList<KServiceTypeProfile>;

  KSimpleConfig config( "profilerc");

  QStringList tmpList = config.groupList();
  for (QStringList::Iterator aIt = tmpList.begin();
       aIt != tmpList.end(); ++aIt) {
    if ( *aIt == "<default>" )
      continue;

    config.setGroup( *aIt );
	
    QString appDesktopPath = config.readEntry( "Application" );
    QString type = config.readEntry( "ServiceType" );
    int pref = config.readNumEntry( "Preference" );
    bool allow = config.readBoolEntry( "AllowAsDefault" );

    KService::Ptr pService = KService::serviceByDesktopPath( appDesktopPath );
	
	if ( pService ) {
		QString application = pService->name();

    	if ( !type.isEmpty() && pref >= 0 )
    	{
    	  KServiceTypeProfile* p = 
		  	KServiceTypeProfile::serviceTypeProfile( type );

    	  if ( !p )
			p = new KServiceTypeProfile( type );

    	  p->addService( application, pref, allow );
		}
    }
  }
}

//static
KServiceTypeProfile::OfferList KServiceTypeProfile::offers( const QString& _servicetype )
{
  OfferList offers;

  //kdDebug(7010) << "KServiceTypeProfile::offers( " << _servicetype << ")" << endl;
  KServiceTypeProfile* profile = serviceTypeProfile( _servicetype );
  if ( profile )
  {
    //kdDebug(7010) << "Found profile, returning " << profile->offers().count() << " offers" << endl;
    return profile->offers();
  }

  KService::List list = KServiceType::offers( _servicetype );
  //kdDebug(7010) << "No profile, using KServiceType::offers, result: " << list.count() << " offers" << endl;
  QValueListIterator<KService::Ptr> it = list.begin();
  for( ; it != list.end(); ++it )
  {
    bool allow = (*it)->allowAsDefault();
    KServiceOffer o( (*it), (*it)->initialPreference(), allow );
    offers.append( o );
    //kdDebug(7010) << "Appending offer " << (*it)->name() << " allow-as-default=" << allow << endl;
  }

  qBubbleSort( offers );

  // debug code, please leave for debugging
  /*kdDebug(7010) << "Sorted list:" << endl;
  OfferList::Iterator itOff = offers.begin();
  for( ; itOff != offers.end(); ++itOff )
  {
    kdDebug(7010) << (*itOff).service()->name() << " allow-as-default=" << (*itOff).allowAsDefault() << endl;
  }
  */

  return offers;
}

KServiceTypeProfile::KServiceTypeProfile( const QString& _servicetype )
{
  initStatic();

  m_strServiceType = _servicetype;

  s_lstProfiles->append( this );
}

KServiceTypeProfile::~KServiceTypeProfile()
{
  ASSERT( s_lstProfiles );

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

KServiceTypeProfile* KServiceTypeProfile::serviceTypeProfile( const QString& _servicetype )
{
  initStatic();

  QListIterator<KServiceTypeProfile> it( *s_lstProfiles );
  for( ; it.current(); ++it )
    if ( it.current()->serviceType() == _servicetype )
      return it.current();

  return 0;
}


KServiceTypeProfile::OfferList KServiceTypeProfile::offers() const
{
  OfferList offers;

  KService::List list = KServiceType::offers( m_strServiceType );
  QValueListIterator<KService::Ptr> it = list.begin();
  for( ; it != list.end(); ++it )
  {
    if ( (*it)->hasServiceType( m_strServiceType ) )
    {
      QMap<QString,Service>::ConstIterator it2 = m_mapServices.find( (*it)->name() );

      if( it2 != m_mapServices.end() )
      {
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
	KServiceOffer o( (*it), 1, (*it)->allowAsDefault() );
	offers.append( o );
      }
    }
  }

  qBubbleSort( offers );

  return offers;
}

KService::Ptr KServiceTypeProfile::preferredService( const QString & _serviceType, bool needApp )
{
  OfferList lst = offers( _serviceType );

  OfferList::Iterator itOff = lst.begin();
  // Look for the first one that is allowed as default and
  for( ; itOff != lst.end(); ++itOff )
  {
      if ((*itOff).allowAsDefault())
      {
          if (!needApp || (*itOff).service()->type() == "Application")
              return (*itOff).service();
      }
      else break; // The allowed-as-default are first anyway
  }

  kdDebug(7010) << "No offers, or none allowed as default" << endl;
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
