/* This file is part of the KDE libraries
   Copyright (C) 2000 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "ktrader.h"
#include "ktraderparsetree.h"

#include <qtl.h>
#include <qbuffer.h>

#include <kuserprofile.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>

template class KStaticDeleter<KTrader>;

using namespace KIO;

class KTraderSorter
{
public:
  KTraderSorter() { m_pService = 0; };
  KTraderSorter( const KTraderSorter& s ) : m_userPreference( s.m_userPreference ),
    m_bAllowAsDefault( s.m_bAllowAsDefault ),
    m_traderPreference( s.m_traderPreference ), m_pService( s.m_pService ) { }
  KTraderSorter( const KService::Ptr &_service, double _pref1, int _pref2, bool _default )
  { m_pService = _service;
    m_userPreference = _pref2;
    m_traderPreference = _pref1;
    m_bAllowAsDefault = _default;
  }

  KService::Ptr service() const { return m_pService; }

  bool operator< ( const KTraderSorter& ) const;

private:
  /**
   * The bigger this number is, the better is this service in
   * the users opinion.
   */
  int m_userPreference;
  /**
   * Is it allowed to use this service for default actions.
   */
  bool m_bAllowAsDefault;

  /**
   * The bigger this number is, the better is this service with
   * respect to the queries preferences expression.
   */
  double m_traderPreference;

  KService::Ptr m_pService;
};

bool KTraderSorter::operator< ( const KTraderSorter& _o ) const
{
  if ( _o.m_bAllowAsDefault && !m_bAllowAsDefault )
    return true;
  if ( _o.m_userPreference > m_userPreference )
    return true;
  if ( _o.m_userPreference < m_userPreference )
    return false;
  if ( _o.m_traderPreference > m_traderPreference )
    return true;
  return false;
}

// --------------------------------------------------

KTrader* KTrader::s_self = 0;
KStaticDeleter<KTrader> ktradersd;

KTrader* KTrader::self()
{
    if ( !s_self )
	s_self = ktradersd.setObject( new KTrader );

    return s_self;
}

KTrader::KTrader()
{
}

KTrader::~KTrader()
{
}

KTrader::OfferList KTrader::query( const QString& _servicetype, const QString& _constraint,
                                   const QString& _preferences ) const
{
    return query( _servicetype, QString::null, _constraint, _preferences );
}

KTrader::OfferList KTrader::query( const QString& _servicetype, const QString& _genericServiceType,
                                   const QString& _constraint,
                                   const QString& _preferences ) const
{
  // TODO: catch errors here
  ParseTreeBase::Ptr constr;
  ParseTreeBase::Ptr prefs;

  if ( !_constraint.isEmpty() )
    constr = KIO::parseConstraints( _constraint );

  if ( !_preferences.isEmpty() )
    prefs = KIO::parsePreferences( _preferences );

  KServiceTypeProfile::OfferList lst;
  KTrader::OfferList ret;

  // Get all services of this service type.
  lst = KServiceTypeProfile::offers( _servicetype, _genericServiceType );
  if ( lst.count() == 0 )
    return ret;

  if ( !!constr )
  {
    // Find all services matching the constraint
    // and remove the other ones
    KServiceTypeProfile::OfferList::Iterator it = lst.begin();
    while( it != lst.end() )
    {
      if ( matchConstraint( constr, (*it).service(), lst ) != 1 )
	it = lst.remove( it );
      else
	++it;
    }
  }

  if ( !!prefs )
  {
    QValueList<KTraderSorter> sorter;
    KServiceTypeProfile::OfferList::Iterator it = lst.begin();
    for( ; it != lst.end(); ++it )
    {
      PreferencesReturn p = matchPreferences( prefs, (*it).service(), lst );
      if ( p.type == PreferencesReturn::PRT_DOUBLE )
	sorter.append( KTraderSorter( (*it).service(), p.f, (*it).preference(), (*it).allowAsDefault() ) );
    }
    qBubbleSort( sorter );

    QValueList<KTraderSorter>::Iterator it2 = sorter.begin();
    for( ; it2 != sorter.end(); ++it2 )
      ret.prepend( (*it2).service() );
  }
  else
  {
    KServiceTypeProfile::OfferList::Iterator it = lst.begin();
    for( ; it != lst.end(); ++it )
      ret.append( (*it).service() );
  }

  return ret;
}

void KTrader::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "ktrader.moc"
