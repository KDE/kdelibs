#include "kuserprofile.h"

list<KServiceTypeProfile*>* KServiceTypeProfile::s_lstProfiles = 0L;

void KServiceTypeProfile::initStatic()
{
  if ( !s_lstProfiles )
    s_lstProfiles = new list<KServiceTypeProfile*>;
}

KServiceTypeProfile::KServiceTypeProfile( const char *_servicetype )
{
  assert( s_lstProfiles );

  m_strServiceType = _servicetype;

  s_lstProfiles->push_back( this );
}
  
KServiceTypeProfile::~KServiceTypeProfile()  
{
  assert( s_lstProfiles );

  s_lstProfiles->remove( this );
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
  assert( s_lstProfiles );

  list<KServiceTypeProfile*>::iterator it = s_lstProfiles->begin();
  for( ; it != s_lstProfiles->end(); it++ )
    if ( strcmp( (*it)->serviceType(), _servicetype ) == 0 )
      return *it;
  
  return 0L;
}
