/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __kuserprofile_h__
#define __kuserprofile_h__

#include <qmap.h>
#include <qstring.h>
#include <qlist.h>
#include <qvaluelist.h>

#include <kservice.h>

/**
 * This class holds the characteristics of a service offer
 * (whether it can be a default offer or not, how big is the preference
 * for this offer, ...)
 */
class KServiceOffer
{
public:
  KServiceOffer();
  KServiceOffer( const KServiceOffer& );
  KServiceOffer( KService::Ptr _service,
		 int _pref, bool _default );
  
  bool operator< ( const KServiceOffer& ) const;
  bool allowAsDefault() const { return m_bAllowAsDefault; }
  int preference() const { return m_iPreference; }
  KService::Ptr service() const { return m_pService; }
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
  KService::Ptr m_pService;
};

/**
 * This class holds the offers for a service type
 * (e.g. in old terms "the bindings for a mimetype")
 */
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
   * @return the preferred service (convenience method)
   */
  static KService::Ptr preferredService( const QString & _serviceType);

  /**
   * @return the profile for the requested service type.
   */
  static KServiceTypeProfile* serviceTypeProfile( const QString& _servicetype );

  /**
   * @return the offers associated with a given servicetype
   */
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
