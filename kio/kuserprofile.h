#ifndef __kuserprofile_h__
#define __kuserprofile_h__

#include <qmap.h>
#include <qstring.h>
#include <qlist.h>
#include <qvaluelist.h>

class KService;
class KServiceTypeProfile;

class KServiceOffer
{
public:
  KServiceOffer();
  KServiceOffer( const KServiceOffer& );
  KServiceOffer( const KService* _service,
		 int _pref, bool _default );
  
  bool operator< ( const KServiceOffer& ) const;
  bool allowAsDefault() const { return m_bAllowAsDefault; }
  int preference() const { return m_iPreference; }
  const KService& service() const { return *m_pService; }
  bool isValid() const { return m_iPreference >= 0; }
  
private:
  /**
   * The bigger this number is, the better is this service.
   */
  int m_iPreference;
  /**
   * Is it allowed to use this service for default actions.
   */
  bool m_bAllowAsDefault;
  const KService* m_pService;
};

class KServiceTypeProfile
{
public:  
  typedef QValueList<KServiceOffer> OfferList;
  
  ~KServiceTypeProfile();
    
  /**
   * @return the users preference of this special service or 0 if
   *         the service is unknown.
   */
  int preference( const QString& _service ) const;
  bool allowAsDefault( const QString& _service ) const;
  
  OfferList offers() const;
  
  /**
   * @return the service type for which this profile is responsible.
   */
  QString serviceType() const { return m_strServiceType; }
  
  /**
   * @return the profile for the requested service type.
   */
  static KServiceTypeProfile* serviceTypeProfile( const QString& _servicetype );

  static OfferList offers( const QString& _servicetype );

  static const QList<KServiceTypeProfile>& serviceTypeProfiles() { return *s_lstProfiles; }
  
protected:
  /**
   * Constructor is called when the user profile is read for the
   * first time.
   */
  KServiceTypeProfile( const QString& _servicetype );

  /**
   * Add a service to this profile.
   */
  void addService( const QString& _service, int _preference = 1, bool _allow_as_default = TRUE );

private:
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
  QMap<QString,Service> m_mapServices;
  
  /**
   * ServiceType of this profile.
   */
  QString m_strServiceType;

  static void initStatic();
  static QList<KServiceTypeProfile>* s_lstProfiles;
};

#endif
