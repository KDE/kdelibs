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

  KConfig config( kapp->kde_configdir() + "/profilerc",
		  kapp->localconfigdir() + "/profilerc" );

  QSmartPtr<KGroupIterator> g( config.groupIterator() );
  for( ; g->current(); ++(*g) )
  {
    config.setGroup( g->currentKey() );
    
    QString type = config.readEntry( "ServiceType" );
    int pref = config.readNumEntry( "Preference" );
    bool allow = config.readBoolEntry( "AllowAsDefault" );
    
    if ( !type.isEmpty() && pref >= 0 )
    {
      KServiceTypeProfile* p = KServiceTypeProfile::find( type );
      if ( !p )
	p = new KServiceTypeProfile( type );
      
      p->addService( config.group(), pref, allow );
    }
  }
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
  KService* s = KService::find( _service );
  if ( s && !s->allowAsDefault() )
    return false;
  
  // Look what the user says ...
  QMap<QString,Service>::ConstIterator it = m_mapServices.find( _service );
  if ( it == m_mapServices.end() )
    return 0;
  
  return it.data().m_bAllowAsDefault;
}

KServiceTypeProfile* KServiceTypeProfile::find( const QString& _servicetype )
{
  initStatic();

  KServiceTypeProfile* p;
  for( p = s_lstProfiles->first(); p != 0L; p = s_lstProfiles->next() )
    if ( strcmp( p->serviceType(), _servicetype ) == 0 )
      return p;
  
  return 0L;
}

KServiceTypeProfile::OfferList KServiceTypeProfile::offers() const
{
  OfferList offers;

  QMap<QString,Service>::ConstIterator it = m_mapServices.begin();
  
  for( ; it != m_mapServices.end(); ++it )
  {
    KService*s = KService::find( it.key() );
    if( s )
    {
      bool allow = s->allowAsDefault();
      if ( allow )
	allow = it.data().m_bAllowAsDefault;
      KServiceOffer o( this, s, it.data().m_iPreference, allow );
      offers.append( o );
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
  m_pProfile = _o.m_pProfile;
  m_pService = _o.m_pService;
  m_iPreference = _o.m_iPreference;
  m_bAllowAsDefault = _o.m_bAllowAsDefault;
}

KServiceOffer::KServiceOffer( const KServiceTypeProfile* _prof, const KService* _service,
			       int _pref, bool _default )
{
  m_pProfile = _prof;
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
