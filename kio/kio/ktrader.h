/* This file is part of the KDE libraries
   Copyright (C) 2000 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef __ktrader_h__
#define __ktrader_h__

#include <qstring.h>
#include <qobject.h>
#include <kservice.h>

/**
 * A Trader interface, similar to the CORBA Trader.
 *
 * Basically, it provides a way for an application to query
 * all KDE services (that is, applications and components) that match
 * a specific set of requirements.  This allows you to find an
 * application in real-time without you having to hard-code the name
 * and/or path of the application.  It is mostly used when you want to
 * do complex queries that @ref KServiceTypeProfile can't handle.
 *
 * @sect Examples
 *
 * A few examples will make this a lot more clear.
 *
 * Say you have an application that will display HTML.  In this
 * example, you don't want to link to khtml... and furthermore, you
 * really don't care if the HTML browser is ours or not, as long as
 * it works.  The way that you formulate your query as well as the way
 * that you execute the browser depends on whether or not you want the
 * browser to run stand-alone or embedded.
 *
 * If you want the browser to run standalone, then you will limit the
 * query to search for all services that handle 'text/html' @em and,
 * furthermore, they must be applications (Type=Application).  You
 * then will use @ref KRun::run() to invoke the application.  In "trader-speak",
 * this looks like so:
 * <PRE>
 * KTrader::OfferList offers = KTrader::self()->query("text/html", "Type == 'Application'");
 * KService::Ptr ptr = offers.first();
 * KURL::List lst;
 * lst.append("http://www.kde.org/index.html");
 * KRun::run(*ptr, lst);
 * </PRE>
 *
 * It should be noted that in the above example, using
 * @ref KServiceTypeProfile would be the better choice since you would
 * probably want the preferred service and the trader doesn't take
 * this into account.  The trader does allow you to do more complex
 * things, though.  Say, for instance, you want to only choose
 * Netscape.  You can do it with the constraint: "(Type ==
 * 'Application') and (Name == 'Netscape')"
 *
 * More the likely, though, you will only use the trader for such
 * things as finding components.  In our continuing example, we say
 * that we want to load any KParts component that can handle HTML.  We
 * will need to use the @ref KLibFactory and @ref KLibLoader to
 * actually do something with our query, then.  Our code would look
 * like so:
 * <PRE>
 * KTrader::OfferList offers = KTrader::self()->query("text/html", "'KParts/ReadOnlyPart' in ServiceTypes");
 * KService::Ptr ptr = offers.first();
 * KLibFactory *factory = KLibLoader::self()->factory( ptr->library() );
 * if (factory)
 *   part = static_cast<KParts::ReadOnlyPart *>(factory->create(this, ptr->name(), "KParts::ReadOnlyPart"));
 * </PRE>
 *
 * Please note that when including property names containing arithmetic operators like - or +, then you have
 * to put brackets around the property name, in order to correctly separate arithmetic operations from
 * the name. So for example a constraint expression like
 *  X-KDE-Blah < 4
 * needs to be written as
 * [X-KDE-Blah] < 4
 * otherwise it could also be interpreted as
 * Substract the numeric value of the property "KDE" and "Blah" from the property "X" and make sure it
 * is less than 4.
 * Instead of the other meaning, make sure that the numeric value of "X-KDE-Blah" is less than 4.
 *
 * Please read http://developer.kde.org/documentation/library/tradersyntax.html for
 * a more complete description of the trader language syntax.
 *
 * @short Provides a way to query the KDE infrastructure for specific
 *        applications or components.
 * @author Torben Weis <weis@kde.org>
 */
class KTrader : public QObject
{
    Q_OBJECT
public:
    /**
     * A list of services.
     */
    typedef QValueList<KService::Ptr> OfferList;
    typedef QValueListIterator<KService::Ptr> OfferListIterator;

    /**
     * Standard destructor
     */
    virtual ~KTrader();

    /**
     * The main function in the KTrader class.
     *
     * It will return a list of services that match your
     * specifications.  The only required parameter is the service
     * type.  This is something like 'text/plain' or 'text/html'.  The
     * constraint parameter is used to limit the possible choices
     * returned based on the constraints you give it.
     *
     * The constraint language is rather full.  The most common
     * keywords are AND, OR, NOT, IN, and EXIST.. all used in an
     * almost spoken-word form.  An example is:
     * <PRE>
     * (Type == 'Service') and (('KParts/ReadOnlyPart' in ServiceTypes) or (exist Exec))
     * </PRE>
     *
     * The keys used in the query (Type, ServiceType, Exec) are all
     * fields found in the .desktop files.
     *
     * @param servicetype A service type like 'text/plain', 'text/html', or 'KOfficePlugin'.
     * @param constraint  A constraint to limit the choices returned.
     * @param preferences Indicates a particular preference to return.
     *
     * @return A list of services that satisfy the query.
     */
    virtual OfferList query( const QString& servicetype,
			     const QString& constraint = QString::null,
			     const QString& preferences = QString::null) const;

    /**
     * A variant of query, that takes two service types as an input.
     * It is not exactly the same as adding the second service type
     * in the constraints of the other query call, because this one
     * takes into account user preferences for this combination of service types.
     *
     * Typically, this is used for getting the list of embeddable components
     * that can handle a given mimetype.
     * In that case, @p servicetype is the mimetype and @p genericServiceType is "KParts/ReadOnlyPart".
     */
    OfferList query( const QString& servicetype, const QString& genericServiceType,
                     const QString& constraint /*= QString::null*/,
                     const QString& preferences /*= QString::null*/) const;

    /**
     * This is a static pointer to a KTrader instance.
     *
     *  You will need
     * to use this to access the KTrader functionality since the
     * constuctors are protected.
     *
     * @return Static KTrader instance
     */
    static KTrader* self();

protected:
    /**
     * @internal
     */
    KTrader();

private:
    static KTrader* s_self;
protected:
    virtual void virtual_hook( int id, void* data );
};

#endif
