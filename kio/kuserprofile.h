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
 *
 * @short Holds the characteristics of a service offer.
 */
class KServiceOffer
{
public:
  KServiceOffer();
  KServiceOffer( const KServiceOffer& );
  KServiceOffer( KService::Ptr _service,
		 int _pref, bool _default );

  bool operator< ( const KServiceOffer& ) const;
  /**
   * Is it allowed to use this service for default actions
   * (e.g. Left Click in a file manager, or KRun in general)
   */
  bool allowAsDefault() const { return m_bAllowAsDefault; }
  /**
   * The bigger this number is, the better is this service.
   */
  int preference() const { return m_iPreference; }
  /**
   * The service which this offer is about.
   */
  KService::Ptr service() const { return m_pService; }
  /**
   * Whether the entry is valid
   */
  bool isValid() const { return m_iPreference >= 0; }

private:
  int m_iPreference;
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
   * @return the service type_s_ for which this profile is responsible.
   * @internal - you're not supposed to call this !
   */
  QString serviceType() const { return m_strServiceType; }

  /**
   * @return the preferred service (convenience method)
   * @param needApp if we need an service of type Application
   * (as opposed to any service, including non-app services)
   * @deprecated, see the other @ref preferredService.
   */
  static KService::Ptr preferredService( const QString & serviceType, bool needApp );  // ### remove in KDE 3.0

  /**
   * @return the preferred service (convenience method)
   * for _serviceType and _genericServiceType (Application, type of component, or empty).
   *
   * preferredService(m,true) is equivalent to preferredService(m,"Application")
   * preferredService(m,false) is equivalent to preferredService(m,QString::null)
   * This call allows e.g. preferredService(m,"KParts/ReadOnlyPart").
   */
  static KService::Ptr preferredService( const QString & serviceType, const QString & genericServiceType );

  /**
   * @return the profile for the requested service type.
   */
  static KServiceTypeProfile* serviceTypeProfile( const QString& servicetype );
  /**
   * @return the profile for the requested service type.
   */
  static KServiceTypeProfile* serviceTypeProfile( const QString& servicetype, const QString & genericServiceType );
  // BCI : merge into one, with genericServiceType = QString::null

  /**
   * @return the offers associated with a given servicetype, sorted by preference
   * This is what KTrader uses to get the list of offers, before applying the
   * constraints and preferences.
   */
  static OfferList offers( const QString& servicetype );

  /**
   * @return the offers associated with the combination of two service types
   * This is almost like an "blah in ServiceTypes" constraint in the Trader,
   * but the difference is that to order the offers, we will look at entries
   * specifically for those two service types. Typically, this is used for
   * getting the list of embeddable components that can handle a given mimetype.
   * In that case, @p servicetype is the mimetype and @p genericServiceType is "KParts/ReadOnlyPart".
   */
  static OfferList offers( const QString& servicetype, const QString& genericServiceType );

  //BCI : merge both offers methods, and genericServiceType = QString::null

  static const QList<KServiceTypeProfile>& serviceTypeProfiles() { return *s_lstProfiles; }

  /**
   * Clear all cached information
   */
  static void clear() { delete s_lstProfiles; s_lstProfiles = 0L; }

protected:
  /**
   * Constructor is called when the user profile is read for the
   * first time.
   */
  KServiceTypeProfile( const QString& _servicetype, const QString& _genericServiceType = QString::null );

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
   * For BCI reasons, this is actually m_strServiceType + "%!%" + m_strGenericServiceType
   */
  QString m_strServiceType;

  ///BCI m_strGenericServiceType;
  ///BCI: d pointer

  static void initStatic();
  static QList<KServiceTypeProfile>* s_lstProfiles;
};

#endif
