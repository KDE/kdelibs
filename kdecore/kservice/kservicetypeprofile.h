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
#include <QtCore/QList>

#include <kservicetypetrader.h>

class KServiceTypeProfileList;

/**
 * KServiceTypeProfile represents the user's preferences for services
 * of a service type.
 * It consists of a list of services (service offers) for the service type
 * that is sorted by the user's preference.
 * KServiceTypeTrader uses KServiceTypeProfile to get results sorted according
 * to the user's profile.
 *
 * @see KService
 * @see KServiceType
 * @see KServiceTypeTrader
 * @short Represents the user's preferences for services of a service type
 *
 * @internal used by KServiceTypeTrader/KMimeTypeTrader
 */
class KDECORE_EXPORT KServiceTypeProfile
{
public:
    /**
     * Clear all cached information
     */
    static void clear(); // TODO: rename to clearCache()

    /**
     * Write the complete profile for a given servicetype.
     * Do not use this for mimetypes.
     * @param serviceType The name of the servicetype.
     * @param services Ordered list of services, from the preferred one to the least preferred one.
     * @param disabledServices List of services which are normally associated with this serviceType,
     * but which should be disabled, i.e. trader queries will not return them.
     */
    static void writeServiceTypeProfile( const QString& serviceType,
                                         const KService::List& services,
                                         const KService::List& disabledServices = KService::List() );


    /**
     * Delete the complete profile for a given servicetype, reverting to the default
     * preference order (the one specified by InitialPreference in the .desktop files).
     *
     * Do not use this for mimetypes.
     * @param serviceType The name of the servicetype.
     */
    static void deleteServiceTypeProfile( const QString& serviceType);

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

    /**
     * @internal, for KServiceTypeTrader
     */
    static bool hasProfile( const QString& serviceType );


private: // used by friend classes only
    friend class KServiceTypeTrader; // for sortServiceTypeOffers
    friend class KMimeTypeTrader; // for sortMimeTypeOffers

    /**
     * Returns the offers in the profile for the requested service type.
     * @param list list of offers (including initialPreference)
     * @param servicetype the service type
     * @return the weighted and sorted offer list
     * @internal used by KServiceTypeTrader
     */
    static KServiceOfferList sortServiceTypeOffers( const KServiceOfferList& list, const QString& servicetype );

    /**
     * Sort the offers for the requested mime type according to the profile (if any),
     * and filter for genericServiceType. This method is really internal to KMimeTypeTrader
     * and might go away at any time.
     *
     * @param list list of offers (key=service, value=initialPreference)
     * @param mimeType the mime type
     * @param genericServiceType the generic service type (e.g. "Application"
     *                           or "KParts/ReadOnlyPart"). Can be QString(),
     *                           then the "Application" generic type will be used
     * @return the weighted and sorted offer list
     * @internal used by KMimeTypeTrader
     */
    static KServiceOfferList sortMimeTypeOffers( const KServiceOfferList& list, const QString& mimeType, const QString & genericServiceType );

private: // really private
    static void initStatic();

    static bool s_configurationMode;
    KServiceTypeProfile();
    ~KServiceTypeProfile();

};

#endif
