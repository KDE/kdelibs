#ifndef __kuserprofile_h__
#define __kuserprofile_h__

#include <map>
#include <list>
#include <string>

class KServiceTypeProfile
{
public:
  KServiceTypeProfile( const char *_servicetype );
  ~KServiceTypeProfile();
  
  void addService( const char *_service, int _preference, bool _allow_as_default );
  
  int preference( const char *_service );
  bool allowAsDefault( const char *_service );
  
  const char* serviceType() { return m_strServiceType.c_str(); }
  
  static void initStatic();
  static KServiceTypeProfile* find( const char *_servicetype );
  
protected:
  struct Service
  {
    int m_iPreference;
    bool m_bAllowAsDefault;
  };
  
  map<string,Service> m_mapServices;
  
  string m_strServiceType;

  static list<KServiceTypeProfile*>* s_lstProfiles;
};

#endif
