#include "kuserprofile.h"

QList<KServiceTypeProfile>* KServiceTypeProfile::s_lstProfiles = 0L;

void KServiceTypeProfile::initStatic()
{
  if ( !s_lstProfiles )
    s_lstProfiles = new QList<KServiceTypeProfile>;
}

KServiceTypeProfile::KServiceTypeProfile( const char *_servicetype )
{
  initStatic();

  m_strServiceType = _servicetype;

  s_lstProfiles->append( this );
}
  
KServiceTypeProfile::~KServiceTypeProfile()  
{
  assert( s_lstProfiles );

  s_lstProfiles->removeRef( this );
}

void KServiceTypeProfile::addService( const char *_service, int _preference, bool _allow_as_default )
{
  m_mapServices[ _service ].m_iPreference = _preference;
  m_mapServices[ _service ].m_bAllowAsDefault = _allow_as_default;
}

int KServiceTypeProfile::preference( const char *_service )
{
  map<string,Service>::iterator it = m_mapServices.find( _service );
  if ( it == m_mapServices.end() )
    return 0;
  
  return it->second.m_iPreference;
}

bool KServiceTypeProfile::allowAsDefault( const char *_service )
{
  map<string,Service>::iterator it = m_mapServices.find( _service );
  if ( it == m_mapServices.end() )
    return 0;
  
  return it->second.m_bAllowAsDefault;
}

KServiceTypeProfile* KServiceTypeProfile::find( const char *_servicetype )
{
  initStatic();

  KServiceTypeProfile* p;
  for( p = s_lstProfiles->first(); p != 0L; p = s_lstProfiles->next() )
    if ( strcmp( p->serviceType(), _servicetype ) == 0 )
      return p;
  
  return 0L;
}
