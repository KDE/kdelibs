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

#ifndef KMIMETYPETRADER_H
#define KMIMETYPETRADER_H

#include "kserviceoffer.h"

/**
 * A trader for services associated to a given mimetype.
 * Service means Application or Component/Plugin, see KService.
 *
 * Example: say that you want to the list of all KParts components that can handle HTML.
 * Our code would look like:
 * \code
 * KServiceOfferList lst = KMimeTypeTrader::self()->query("text/html", "KParts/ReadOnlyPart");
 * \endcode
 *
 * If you want to get the preferred KParts component for text/html you could use
 * preferredService("text/html", "KParts/ReadOnlyPart"), although if this is about
 * loading that component you would rather use KParts::ComponentFactory directly.
 *
 * @short Provides a way to query the KDE infrastructure for
 *        applications or components that can handle a specific mimetype.
 * @see KServiceTypeTrader, KService
 */
class KIO_EXPORT KMimeTypeTrader
{
public:

    /**
     * Standard destructor
     */
    ~KMimeTypeTrader();

    /**
     * This method returns a list of services which are associated with a given mimetype.
     *
     * Example usage:
     * To get list of applications that can handle a given mimetype,
     * set @p genericServiceType to "Application" (which is the default).
     * To get list of embeddable components that can handle a given mimetype,
     * set @p genericServiceType to "KParts/ReadOnlyPart".
     *
     * The constraint parameter is used to limit the possible choices
     * returned based on the constraints you give it.
     *
     * The @p constraint language is rather full.  The most common
     * keywords are AND, OR, NOT, IN, and EXIST, all used in an
     * almost spoken-word form.  An example is:
     * \code
     * (Type == 'Service') and (('Browser/View' in ServiceTypes) and (exist Library))
     * \endcode
     *
     * The keys used in the query (Type, ServiceTypes, Library) are all
     * fields found in the .desktop files.
     *
     * @param mimeType A mime type like 'text/plain' or 'text/html'.
     * @param genericServiceType a basic service type, like 'KParts/ReadOnlyPart' or 'Application'
     * @param constraint  A constraint to limit the choices returned, QString() to
     *                    get all services that can handle the given @p mimetype
     *
     * @return A list of services that satisfy the query, sorted by preference
     * (preferred service first)
     * @see http://developer.kde.org/documentation/library/kdeqt/tradersyntax.html
     */
    KService::List query( const QString& mimeType,
                          const QString& genericServiceType = QString::fromLatin1("Application"),
                          const QString& constraint = QString() ) const;


    /**
     * Returns the offers associated with a given servicetype, sorted by preference.
     * This is what query() uses to get the list of offers, before applying the
     * constraints and preferences. In general you want to simply use query().
     *
     * @param mimeType A mime type like 'text/plain' or 'text/html'.
     * @param genericServiceType a basic service type, like 'KParts/ReadOnlyPart' or 'Application'
     */
    KServiceOfferList weightedOffers( const QString& mimeType, const QString& genericServiceType = QString::fromLatin1("Application") ) const;

    /**
     * Returns the preferred service for @p mimeType and @p genericServiceType
     *
     * This is almost like offers().first(), except that it also checks
     * if the service is allowed as a preferred service (see KService::allowAsDefault).
     *
     * @param mimeType the mime type (see offers())
     * @param genericServiceType the service type (see offers())
     * @return the preferred service, or 0 if no service is available
     */
    KService::Ptr preferredService( const QString & mimeType, const QString & genericServiceType = QString::fromLatin1("Application") );

    /**
     * This is a static pointer to the KMimeTypeTrader singleton.
     *
     * You will need to use this to access the KMimeTypeTrader functionality since the
     * constructors are protected.
     *
     * @return Static KMimeTypeTrader instance
     */
    static KMimeTypeTrader* self();

private:
    /**
     * @internal
     */
    KMimeTypeTrader();

private:
    static KMimeTypeTrader* s_self;
};

#endif /* KMIMETYPETRADER_H */

