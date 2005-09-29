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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
 * do complex queries that KServiceTypeProfile can't handle.
 *
 * \par Examples
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
 * then will use KRun::run() to invoke the application.  In "trader-speak",
 * this looks like so:
 * \code
 * KTrader::OfferList offers = KTrader::self()->query("text/html", "Type == 'Application'");
 * KService::Ptr ptr = offers.first();
 * KURL::List lst;
 * lst.append("http://www.kde.org/index.html");
 * KRun::run(*ptr, lst);
 * \endcode
 *
 * It should be noted that in the above example, using
 * KServiceTypeProfile would be the better choice since you would
 * probably want the preferred service and the trader doesn't take
 * this into account.  The trader does allow you to do more complex
 * things, though.  Say, for instance, you want to only choose
 * Netscape.  You can do it with the constraint: "(Type ==
 * 'Application') and (Name == 'Netscape')"
 *
 * More the likely, though, you will only use the trader for such
 * things as finding components.  In our continuing example, we say
 * that we want to load any KParts component that can handle HTML.  We
 * will need to use the KLibFactory and KLibLoader to
 * actually do something with our query, then.  Our code would look
 * like so:
 * \code
 * KTrader::OfferList offers = KTrader::self()->query("text/html", "'KParts/ReadOnlyPart' in ServiceTypes");
 * KService::Ptr ptr = offers.first();
 * KLibFactory *factory = KLibLoader::self()->factory( ptr->library() );
 * if (factory)
 *   part = static_cast<KParts::ReadOnlyPart *>(factory->create(this, ptr->name(), "KParts::ReadOnlyPart"));
 * \endcode
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
 * See also the formal syntax defined in @ref tradersyntax .
 *
 * @short Provides a way to query the KDE infrastructure for specific
 *        applications or components.
 * @author Torben Weis <weis@kde.org>
 */
class KIO_EXPORT KTrader : public QObject
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
     * @param servicetype A service type like 'text/plain', 'text/html', or 'KOfficePlugin'.
     * @param constraint  A constraint to limit the choices returned, QString::null to
     *                    get all services of the given @p servicetype
     * @param preferences Indicates a particular preference to return, QString::null to ignore.
     *                    Uses an expression in the constraint language that must return
     *                    a number
     *
     * @return A list of services that satisfy the query
     * @see http://developer.kde.org/documentation/library/kdeqt/tradersyntax.html
     */
    virtual OfferList query( const QString& servicetype,
			     const QString& constraint = QString::null,
			     const QString& preferences = QString::null) const;

    /**
     * A variant of query(), that takes two service types as an input.
     * It is not exactly the same as adding the second service type
     * in the constraints of the other query call, because this one
     * takes into account user preferences for this combination of service types.
     *
     * Example usage:
     * To get list of applications that can handle a given mimetype,
     * set @p servicetype to the mimetype and @p genericServiceType is "Application".
     * To get list of embeddable components that can handle a given mimetype,
     * set @p servicetype to the mimetype and @p genericServiceType is "KParts/ReadOnlyPart".
     *
     * @param servicetype A service type like 'text/plain', 'text/html', or 'KOfficePlugin'.
     * @param genericServiceType a basic service type, like 'KParts/ReadOnlyPart' or 'Application'
     * @param constraint  A constraint to limit the choices returned, QString::null to
     *                    get all services of the given @p servicetype
     * @param preferences Indicates a particular preference to return, QString::null to ignore.
     *                    Uses an expression in the constraint language that must return
     *                    a number
     *
     * @return A list of services that satisfy the query
     * @see http://developer.kde.org/documentation/library/kdeqt/tradersyntax.html
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

/** @page tradersyntax Trader Syntax
 *
 *
 * @section Literals
 *
 * As elementary atoms of the constraint language, KTrader supports
 * booleans, integers, floats and strings. Boolean literals are
 * @a TRUE and @a FALSE . Integers can be positive or negative,
 * i.e. @a 42 and @a -10 are legal values. Floating point
 * numbers are @a 3.141592535 or @a -999.999 . Scientific notation
 * like @a 1.5e-2 is not supported. Character literals are delimited
 * by single quotation marks, e.g. @a 'Bernd' .
 *
 *
 * @section Symbols
 *
 * Identifiers in query string are interpreted as property names, which
 * are listed in the service's <tt>.desktop</tt> file. For example,
 * <tt>Name</tt> is the name of the service, <tt>ServiceTypes</tt> is a
 * list of the service types it supports. Note that only properties can
 * be written as-is which start with an alphabetical character and contain
 * only alphanumerical characters. Other properties have to be enclosed in
 * brackets, e.g. <tt>[X-KDE-Init]</tt>. Properties must not contain any
 * special characters other than <tt>-</tt>.
 *
 * Special property names:
 *   - <b>DesktopEntryName</b> stands for the filename of the service
 *     desktop entry without any extension. This can be useful to
 *     exclude some specific services.
 *   - <b>DesktopEntryPath</b> stands for the relative or full path
 *     to the .desktop file, see KService::desktopEntryPath. Mentionned
 *     here for completeness, better not use it (things can be moved
 *     around).
 *   - <b>Library</b> is the property whose value is set by
 *     <tt>X-KDE-Library</tt> in the .desktop file. This renaming
 *     happened to conform to the desktop file standard, but the
 *     property name didn't change.
 *
 *
 * @section Comparison
 *
 * Supported comparison operators are:
 *
 *   - <tt>==</tt>
 *   - <tt>!=</tt>
 *   - <tt>&lt;</tt>
 *   - <tt>&lt;=</tt>
 *   - <tt>&gt;</tt>
 *   - <tt>&gt;=</tt>
 *
 *
 * @section Arithmetic Arithmetic and boolean expressions
 *
 *   - <tt>+</tt>
 *   - <tt>-</tt>
 *   - <tt>*</tt>
 *   - <tt>/</tt>
 *   - <tt>and</tt>
 *   - <tt>or</tt>
 *   - <tt>not</tt>
 *
 * Note that the arithmetic operators are possible for integers and
 * floating point numbers. <tt>-</tt> is both a unary and binary operator,
 * <tt>not</tt> is a unary operator.
 *
 *
 * @section Other Other operators
 *
 *   - <tt>~</tt>
 *   - <tt>in</tt>
 *   - <tt>exist</tt>
 *   - <tt>()</tt>
 *
 * The tilde operator stands for a substring match. For example,
 * <tt>KParts ~ 'KParts/ReadOnlyPart'</tt> is TRUE. The membership
 * operator <tt>in</tt> tests whether a value is in a list. A list is a
 * string with semi-colon- or comma-separated entries, depending on the
 * type. An example for the membership operator is
 * <tt>'text/plain' in ServiceTypes</tt>.
 * The <tt>exist</tt> tests whether a certain property is defined in the
 * <tt>.desktop</tt> file. Subexpressions are written in parentheses.
 *
 * Warning, testing the contents of a property only works if the property
 * is specified. There is not support for default values. If the property
 * might be missing, and you still want such services to be included, you
 * have to check for existence before testing it. For instance, to say
 * that MyProp is a boolean that defaults to true, and that you want the
 * services that have it set to true, use:
 * <tt>not exist MyProp or MyProp</tt>
 * Simply testing for <tt>MyProp</tt> would
 * exclude the services without the property at all.
 *
 *
 * @section Examples
 *
 * The following examples show filters for .desktop files.
 * <tt>Type</tt>, <tt>ServiceTypes</tt> and <tt>MimeType</tt> are
 * properties in .desktop files. Be aware that within KTrader MimeType
 * properties are understood as ServiceTypes ones.
 *
 *
 *   - <tt>Type == 'Application'</tt>@n
 *     All services that are applications.
 *   - <tt>'KParts/ReadOnlyPart' in ServiceTypes</tt>@n
 *     All read-only KParts.
 *   - <tt>('KParts/ReadOnlyPart' in ServiceTypes) and ('text/plain' in ServiceTypes)</tt>@n
 *     All read-only KParts that handle the mime type 'text/plain'.
 *
 * @author Bernd Gehrmann <a href="mailto:bernd@kdevelop.org">bernd@kdevelop.org</a>
*/


#endif
