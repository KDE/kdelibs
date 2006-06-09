/* This file is part of the KDE libraries
   Copyright (C) 2000 Torben Weis <weis@kde.org>
   Copyright (C) 2006 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KSERVICEOFFER_H
#define KSERVICEOFFER_H

#include <qstring.h>
#include <kservice.h>

/**
 * This class holds the user-specific preferences of a service
 * (whether it can be a default offer or not, how big is the preference
 * for this offer, ...). Basically it is a reference to a
 * KService, a number that represents the user's preference (bigger
 * is better) and a flag whether the KService can be used as default.
 *
 * @see KService
 * @short Holds the user's preference of a service.
 */
class KDECORE_EXPORT KServiceOffer
{
public:
  /**
   * Create an invalid service offer.
   */
  KServiceOffer();

  /**
   * Copy constructor.
   * Shallow copy (the KService will not be copied).
   */
  KServiceOffer( const KServiceOffer& );

  /**
   * Creates a new KServiceOffer.
   * @param _service a pointer to the KService
   * @param _pref the user's preference value, must be positive,
   *              bigger is better
   * @param _default true if the service should be used as
   *                 default
   */
  KServiceOffer( const KService::Ptr& _service,
		 int _pref, bool _default );

  /**
   * Creates a new KServiceOffer.
   * @param _service a pointer to the KService
   * @param _pref the user's preference value, must be positive, bigger is better
   * @param _default true if the service should be used as
   *                 default
   */
  KServiceOffer( const KService::Ptr& _service, int _pref );

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
   * The bigger this number is, the better is this service.
   * Set the preference number
   * @internal - only for KMimeTypeTrader
   */
  void setPreference( int p ) { m_iPreference = p; }
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
};

/**
 * A list of weighted offers.
 */
typedef QList<KServiceOffer> KServiceOfferList;

#endif /* KSERVICEOFFER_H */

