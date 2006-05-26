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
#ifndef __kservicetypetrader_h__
#define __kservicetypetrader_h__

#include "kserviceoffer.h"

/**
 * A Trader interface, similar to the CORBA Trader.
 *
 * Basically, it provides a way for an application to query
 * all KDE services (that is, applications, components, plugins) that match
 * a specific set of requirements. This allows you to find such services
 * in real-time without you having to hard-code their names and/or paths.
 *
 * For anything relating to mimetypes (type of files), ignore KServiceTypeTrader
 * and use KMimeTypeTrader instead.
 *
 * \par Examples
 *
 * As an example: if you want to find all plugins for your application,
 * you would define a KMyApp/Plugin servicetype, and then you can query
 * the trader for it:
 * \code
 * KServiceOfferList offers = KServiceTypeTrader::self()->query("KMyApp/Plugin");
 * \endcode
 *
 * You can add a constraint in the "trader query language". For instance:
 * \code
 * KServiceTypeTrader::self()->query("KMyApp/Plugin", "[X-KMyApp-InterfaceVersion] > 15");
 * \endcode
 *
 * Please note that when including property names containing arithmetic operators like - or +, then you have
 * to put brackets around the property name, in order to correctly separate arithmetic operations from
 * the name. So for example a constraint expression like
 *  X-KMyApp-InterfaceVersion > 4
 * needs to be written as
 * [X-KMyApp-InterfaceVersion] > 4
 * otherwise it could also be interpreted as
 * Substract the numeric value of the property "KMyApp" and "InterfaceVersion" from the
 * property "X" and make sure it is greater than 4.
 * Instead of the other meaning, make sure that the numeric value of "X-KMyApp-InterfaceVersion" is
 * greater than 4.
 *
 * Please read http://developer.kde.org/documentation/library/kdeqt/tradersyntax.html for
 * a more complete description of the trader language syntax.
 *
 * @short Provides a way to query the KDE infrastructure for specific
 *        applications or components.
 * @see KMimeTypeTrader, KService
 */
class KDECORE_EXPORT KServiceTypeTrader
{
public:
    /**
     * Standard destructor
     */
    ~KServiceTypeTrader();

    /**
     * The main function in the KServiceTypeTrader class.
     *
     * It will return a list of services that match your
     * specifications.  The only required parameter is the service
     * type.  This is something like 'text/plain' or 'text/html'.  The
     * constraint parameter is used to limit the possible choices
     * returned based on the constraints you give it.
     *
     * The @p constraint language is rather full.  The most common
     * keywords are AND, OR, NOT, IN, and EXIST, all used in an
     * almost spoken-word form.  An example is:
     * \code
     * (Type == 'Service') and (('KParts/ReadOnlyPart' in ServiceTypes) or (exist Exec))
     * \endcode
     *
     * The keys used in the query (Type, ServiceType, Exec) are all
     * fields found in the .desktop files.
     *
     * @param servicetype A service type like 'KMyApp/Plugin' or 'KFilePlugin'.
     * @param constraint  A constraint to limit the choices returned, QString() to
     *                    get all services of the given @p servicetype
     *
     * @return A list of services that satisfy the query
     * @see http://developer.kde.org/documentation/library/kdeqt/tradersyntax.html
     */
    KService::List query( const QString& servicetype,
                          const QString& constraint = QString() ) const;

    /**
     * Returns the offers associated with a given servicetype, sorted by preference.
     * This is what query() uses to get the list of offers, before applying the
     * constraints and preferences. In general you want to simply use query().
     *
     * @param servicetype A service type like 'KMyApp/Plugin' or 'KFilePlugin'.
     */
    KServiceOfferList weightedOffers( const QString& serviceType ) const;

    /**
     * Returns the preferred service for @p serviceType.
     *
     * @param serviceType the service type (e.g. "KMyApp/Plugin")
     * @return the preferred service, or 0 if no service is available
     */
    KService::Ptr preferredService( const QString & serviceType ) const;

    /**
     * This is a static pointer to the KServiceTypeTrader singleton.
     *
     * You will need to use this to access the KServiceTypeTrader functionality since the
     * constructors are protected.
     *
     * @return Static KServiceTypeTrader instance
     */
    static KServiceTypeTrader* self();

    /**
     * @internal  (public for KMimeTypeTrader)
     */
    static void applyConstraints( KService::List& lst,
                                  const QString& constraint );

private:
    /**
     * @internal
     */
    KServiceTypeTrader();

private:
    static KServiceTypeTrader* s_self;
};

#endif
