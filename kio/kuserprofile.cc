#include "kuserprofile.h"
#include "kservices.h"

#include <kconfig.h>
#include <kapp.h>

#include <qtl.h>
#include <qsmartptr.h>

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

  KConfig config( "profilerc");

  QStringList tmpList = config.groupList();
  for (QStringList::Iterator aIt = tmpList.begin(); 
       aIt != tmpList.end(); ++aIt) {
    config.setGroup( *aIt );
    
    QString type = config.readEntry( "ServiceType" );
    int pref = config.readNumEntry( "Preference" );
    bool allow = config.readBoolEntry( "AllowAsDefault" );

    if ( !type.isEmpty() && pref >= 0 )
    {
      KServiceTypeProfile* p = KServiceTypeProfile::serviceTypeProfile( type );
      if ( !p )
	p = new KServiceTypeProfile( type );

      p->addService( config.group(), pref, allow );
    }
  }
}

KServiceTypeProfile::OfferList KServiceTypeProfile::offers( const QString& _servicetype )
{
  OfferList offers;

  KServiceTypeProfile* profile = serviceTypeProfile( _servicetype );
  if ( profile )
    return profile->offers( _servicetype );

  QListIterator<KService> it( KService::services() );
  for( ; it.current(); ++it )
  {
    if ( it.current()->hasServiceType( _servicetype ) )
    {
      bool allow = it.current()->allowAsDefault();
      KServiceOffer o( it.current(), 1, allow );
      offers.append( o );
    }
  }

  qBubbleSort( offers );

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
  KService* s = KService::service( _service );
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

  QListIterator<KService> it( KService::services() );
  for( ; it.current(); ++it )
  {
    if ( it.current()->hasServiceType( m_strServiceType ) )
    {
      QMap<QString,Service>::ConstIterator it2 = m_mapServices.find( it.current()->name() );

      if( it2 != m_mapServices.end() )
      {
	bool allow = it.current()->allowAsDefault();
	if ( allow )
	  allow = it2.data().m_bAllowAsDefault;
	KServiceOffer o( it.current(), it2.data().m_iPreference, allow );
	offers.append( o );
      }
      else
      {
	KServiceOffer o( it.current(), 1, it.current()->allowAsDefault() );
	offers.append( o );
      }
    }
  }

  qBubbleSort( offers );

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

KServiceOffer::KServiceOffer( const KService* _service, int _pref, bool _default )
{
  m_pService = _service;
  m_iPreference = _pref;
  m_bAllowAsDefault = _default;
}

bool KServiceOffer::operator< ( const KServiceOffer& _o ) const
{
  if ( _o.m_bAllowAsDefault && !m_bAllowAsDefault )
    return true;
  if ( _o.m_iPreference > m_iPreference )
    return true;
  return false;
}
