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
#include <qptrlist.h>
#include <qvaluelist.h>

#include <kservice.h>

/**
 * This class holds the user-specific preferences of a service
 * (whether it can be a default offer or not, how big is the preference
 * for this offer, ...). Basically it is a reference to a 
 * @ref KService, a number that represents the user's preference (bigger
 * is better) and a flag whether the KService can be used as default.
 *
 * @see KService
 * @short Holds the user's preference of a service.
 */
class KServiceOffer
{
public:
  /**
   * Create an invalid service offer.
   */
  KServiceOffer();
  
  /**
   * Copy constructor.
   * Shallow copy (the @ref KService will not be copied).
   */
  KServiceOffer( const KServiceOffer& );

  /**
   * Creates a new KServiceOffer.
   * @param _service a pointer to the @ref KService
   * @param _pref the user's preference value, must be positive, 
   *              bigger is better
   * @param _default true if the service should be used as
   *                 default
   */
  KServiceOffer( KService::Ptr _service,
		 int _pref, bool _default );

  /**
   * A service is bigger that the other when it can be default
   * (and the other is not) and its preference value it higher.
   */
  bool operator< ( const KServiceOffer& ) const;
  /**
   * Is it allowed to use this service for default actions
   * (e.g. Left Click in a file manager, or KRun in general).
   * @return true if the service is a allowed as default
   */
  bool allowAsDefault() const { return m_bAllowAsDefault; }
  /**
   * The bigger this number is, the better is this service.
   * @return the preference number (negative numbers will be
   *         returned by invalid service offers)
   */
  int preference() const { return m_iPreference; }
  /**
   * The service which this offer is about.
   * @return the service this offer is about, can be 0
   *         in valid offers or when not set
   */
  KService::Ptr service() const { return m_pService; }
  /**
   * Check whether the entry is valid. A service is valid if
   * its preference value is positive.
   * @return true if the service offer is valid
   */
  bool isValid() const { return m_iPreference >= 0; }

private:
  int m_iPreference;
  bool m_bAllowAsDefault;
  KService::Ptr m_pService;
private:
  class KServiceOfferPrivate;
};

/**
 * KServiceTypeProfile represents the user's preferences for services
 * of a service type.
 * It consists of a list of services (service offers) for the service type 
 * that is sorted by the user's preference. 
 * @ref KTrader uses KServiceTypeProfile to sort its results, so usually
 * you can just use @ref KTrader to find the user's preferred service.
 *
 * @see KService
 * @see KServiceType
 * @see KServiceOffer
 * @see KTrader
 * @short Represents the user's preferences for services of a service type
 */
class KServiceTypeProfile
{
public:
  typedef QValueList<KServiceOffer> OfferList;

  ~KServiceTypeProfile();

  /**
   * Returns the users preference of the given service.
   * @param _service the name of the service to check
   * @return the user's preference number of the given 
   *         @p _service, or 0 the service is unknown.
   */
  int preference( const QString& _service ) const;

  /**
   * Checks whether the given @p _service can be used as default.
   * @param _service the name of the service to check
   * @return true if allowed as default
   */
  bool allowAsDefault( const QString& _service ) const;

  /**
   * Returns the list of all service offers for the service types
   * that are represented by this profile.
   * @return the list of @ref KServiceOffer instances
   */
  OfferList offers() const;

  /**
   * Returns the preferred service for @p _serviceType and @p _genericServiceType 
   * ("Application", type of component, or null).
   *
   * @param serviceType the service type (e.g. a MIME type)
   * @param genericServiceType the generic service type (e.g. "Application" or
   *                           "KParts/ReadOnlyPart")
   * @return the preferred service, or 0 if no service is available
   */
  static KService::Ptr preferredService( const QString & serviceType, const QString & genericServiceType );

  /**
   * Returns the profile for the requested service type.
   * @param serviceType the service type (e.g. a MIME type)
   * @param genericServiceType the generic service type (e.g. "Application"
   *                           or "KParts/ReadOnlyPart"). Can be QString::null,
   *                           then the "Application" generic type will be used
   * @return the KServiceTypeProfile with the given arguments, or 0 if not found
   */
  static KServiceTypeProfile* serviceTypeProfile( const QString& servicetype, const QString & genericServiceType = QString::null );

  /**
   * Returns the offers associated with a given servicetype, sorted by preference.
   * This is what @ref KTrader uses to get the list of offers, before applying the
   * constraints and preferences.
   *
   * If @p genericServiceType is specified, a list is returned with 
   * the offers associated with the combination of the two service types.
   * This is almost like an "foo in ServiceTypes" constraint in the @ref KTrader,
   * but the difference is that to order the offers, we will look at entries
   * specifically for those two service types. Typically, this is used for
   * getting the list of embeddable components that can handle a given mimetype.
   * In that case, @p servicetype is the mimetype and @p genericServiceType is "KParts/ReadOnlyPart".
   *
   * @param serviceType the service type (e.g. a MIME type)
   * @param genericServiceType the generic service type (e.g. "Application"
   *                           or "KParts/ReadOnlyPart"). Can be QString::null,
   *                           then all generic types will be included
   * @return the list of offers witht he given parameters
   */
  static OfferList offers( const QString& servicetype, const QString& genericServiceType = QString::null );

  /**
   * Returns a list of all KServiceTypeProfiles.
   * @return a list of all KServiceTypeProfiles
   */
  static const QPtrList<KServiceTypeProfile>& serviceTypeProfiles() { return *s_lstProfiles; }

  /**
   * Clear all cached information
   */
  static void clear() { delete s_lstProfiles; s_lstProfiles = 0L; }

  /**
   * This method activates a special mode of KServiceTypeProfile, in which all/all
   * and all/allfiles are excluded from the results of the queries.
   * It is meant for the configuration module _only_. 
   * @internal
   */
  static void setConfigurationMode() { s_configurationMode = true; }
  /**
   * @internal
   */
  static bool configurationMode() { return s_configurationMode; }

protected:
  /**
   * Constructor is called when the user profile is read for the
   * first time.
   * @param serviceType the service type (e.g. a MIME type)
   * @param genericServiceType the generic service type (e.g. "Application"
   *                           or "KParts/ReadOnlyPart"). Can be QString::null,
   *                           then the "Application" generic type will be used
   */
  KServiceTypeProfile( const QString& _servicetype, const QString& _genericServiceType = QString::null );

  /**
   * Add a service to this profile.
   * @param _service the name of the service
   * @param _pref the user's preference value, must be positive, 
   *              bigger is better
   * @param _default true if the service should be used as
   *                 default
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

  /**
   * Secondary ServiceType of this profile.
   */
  QString m_strGenericServiceType;

  static void initStatic();
  static QPtrList<KServiceTypeProfile>* s_lstProfiles;
  static bool s_configurationMode;
private:
  class KServiceTypeProfilePrivate* d;
};

#endif
