#ifndef __kuserprofile_h__
#define __kuserprofile_h__

#include <map>
#include <qstring.h>
#include <qlist.h>

#include <string>

class KServiceTypeProfile
{
public:
  KServiceTypeProfile( const char *_servicetype );
  ~KServiceTypeProfile();
  
  /**
   * Add a service to this profile.
   */
  void addService( const char *_service, int _preference, bool _allow_as_default );
  
  /**
   * @return the users preference of this special service or 0 if
   *         the service is unknown.
   */
  int preference( const char *_service );
  bool allowAsDefault( const char *_service );
  
  /**
   * @return the service type for which this profile is responsible.
   */
  const char* serviceType() { return m_strServiceType; }
  
  /**
   * @return the profile for the requested service type.
   */
  static KServiceTypeProfile* find( const char *_servicetype );
  
protected:
  /**
   * Represents the users assessment of a special service
   */
  struct Service
  {
    /**
     * The bigger this number is, the better is this service.
     */
    int m_iPreference;
    /**
     * Is it allowed to use this service for default actions.
     */
    bool m_bAllowAsDefault;
  };
  
  /**
   * Map of all services for which we have assessments.
   */
  map<string,Service> m_mapServices;
  
  /**
   * ServiceType of this profile.
   */
  QString m_strServiceType;

  static void initStatic();
  static QList<KServiceTypeProfile>* s_lstProfiles;
};

#endif
