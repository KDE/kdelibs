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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __kservicetypeprofile_h__
#define __kservicetypeprofile_h__

#include <qmap.h>
#include <qstring.h>
#include <QList>

#include <ktrader.h>

class KServiceTypeProfileList;

/**
 * KServiceTypeProfile represents the user's preferences for services
 * of a service type.
 * It consists of a list of services (service offers) for the service type
 * that is sorted by the user's preference.
 * KTrader uses KServiceTypeProfile to sort its results.
 *
 * @see KService
 * @see KServiceType
 * @see KTrader
 * @short Represents the user's preferences for services of a service type
 *
 * @internal used by KTrader/KMimeTypeTrader
 */
class KIO_EXPORT KServiceTypeProfile
{
public:
  ~KServiceTypeProfile();

  /**
   * Returns the offers in the profile for the requested service type.
   * @param servicetype the service type
   * @return the KServiceTypeProfile with the given arguments, or 0 if not found
   * @internal used by KTrader
   */
  static KTrader::OfferList serviceTypeProfileOffers( const QString& servicetype );

  /**
   * Returns the offers in the profile for the requested mime type.
   * @param mimeType the mime type
   * @param genericServiceType the generic service type (e.g. "Application"
   *                           or "KParts/ReadOnlyPart"). Can be QString(),
   *                           then the "Application" generic type will be used
   * @return the KServiceTypeProfile with the given arguments, or 0 if not found
   * @internal used by KMimeTypeTrader
   */
  static KTrader::OfferList mimeTypeProfileOffers( const QString& mimeType, const QString & genericServiceType );

  /**
   * Clear all cached information
   */
  static void clear();

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

private:
  /**
   * Constructor is called when the user profile is read for the
   * first time.
   * @param serviceType the service type (e.g. a MIME type)
   * @param genericServiceType the generic service type (e.g. "Application"
   *                           or "KParts/ReadOnlyPart"). Can be QString(),
   *                           then the "Application" generic type will be used
   */
  KServiceTypeProfile( const QString& serviceType,
                       const QString& genericServiceType = QString() );

  /**
   * Add a service to this profile.
   * @param _service the name of the service
   * @param _preference the user's preference value, must be positive,
   *              bigger is better
   * @param _allow_as_default true if the service should be used as
   *                 default
   */
  void addService( const QString& _service, int _preference = 1, bool _allow_as_default = true );

private:
  /**
   * Returns the list of all service offers for the service types
   * that are represented by this profile.
   * @return the list of KServiceOffer instances
   * @internal used by KTrader/KMimeTypeTrader
   */
  KTrader::OfferList offers() const;

  static KServiceTypeProfile* findProfile( const QString& type, const QString& type2 );

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

  static KServiceTypeProfileList* s_lstProfiles;
  static bool s_configurationMode;
private:
  class KServiceTypeProfilePrivate* d;
};

#endif
