/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000-2001,2003 Dawit Alemayehu <adawit at kde.org>
 *
 *  Original author
 *  Copyright (C) 2000 Yves Arrouye <yves@realnames.com>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef KURIFILTER_H
#define KURIFILTER_H

#include <kio/kio_export.h>
#include <QtCore/QObject>
#include <QtCore/QStringList>

#include <kurl.h>

#ifdef Q_OS_WIN
#undef ERROR
#endif

class KUriFilterDataPrivate;

class KCModule;

/**
* A basic message object used for exchanging filtering
* information between the filter plugins and the application
* requesting the filtering service.
*
* Use this object if you require a more detailed information
* about the URI you want to filter. Any application can create
* an instance of this class and send it to KUriFilter to
* have the plugins fill out all possible information about the
* URI.
*
* \b Example
*
* \code
*   QString text = "kde.org";
*   KUriFilterData d = text;
*   bool filtered = KUriFilter::self()->filter( d );
*   cout << "URL: " << text.toLatin1() << endl
*        << "Filtered URL: " << d.uri().url().toLatin1() << endl
*        << "URI Type: " << d.uriType() << endl
*        << "Was Filtered: " << filtered << endl;
* \endcode
*
* The above code should yield the following output:
* \code
*   URI: kde.org
*   Filtered URI: http://kde.org
*   URI Type: 0            <== means NET_PROTOCOL
*   Was Filtered: 1        <== means the URL was successfully filtered
* \endcode
*
* @short A message object for exchanging filtering URI info.
* @author Dawit Alemayehu <adawit at kde.org>
*/

class KIO_EXPORT KUriFilterData
{
friend class KUriFilterPlugin;

public:
    /**
     * Describes the type of the URI that was filtered.
     * Here is a brief description of the types:
     *
     * @li NetProtocol  - Any network protocol: http, ftp, nttp, pop3, etc...
     * @li LocalFile    - A local file whose executable flag is not set
     * @li LocalDir     - A local directory
     * @li Executable   - A local file whose executable flag is set
     * @li Help         - A man or info page
     * @li Shell        - A shell executable (ex: echo "Test..." >> ~/testfile)
     * @li Blocked      - A URI that should be blocked/filtered (ex: ad filtering)
     * @li Error        - An incorrect URI (ex: "~johndoe" when user johndoe
     *                    does not exist in that system )
     * @li Unknown      - A URI that is not identified. Default value when
     *                    a KUriFilterData is first created.
     */
    enum UriTypes { NetProtocol=0, LocalFile, LocalDir, Executable, Help, Shell, Blocked, Error, Unknown };

    /**
     * Default constructor.
     *
     * Creates a UriFilterData object.
     */
    KUriFilterData();

    /**
     * Creates a UriFilterData object from the given URL.
     *
     * @param url is the URL to be filtered.
     */
    explicit KUriFilterData( const KUrl& url );

    /**
     * Creates a UriFilterData object from the given string.
     *
     * @param url is the string to be filtered.
     */
    explicit KUriFilterData( const QString& url );

    /**
     * Copy constructor.
     *
     * Creates a UriFilterData object from another
     * URI filter data object.
     *
     * @param data the uri filter data to be copied.
     */
    KUriFilterData( const KUriFilterData& data );

    /**
     * Destructor.
     */
    ~KUriFilterData();


    /**
     * Returns the filtered or the original URL.
     *
     * This function returns the filtered url if one
     * of the plugins successfully filtered the original
     * URL.  Otherwise, it returns the original URL.
     * See hasBeenFiltered() and
     *
     * @return the filtered or original url.
     */
    KUrl uri() const;

    /**
     * Returns an error message.
     *
     * This functions returns the error message set
     * by the plugin whenever the uri type is set to
     * KUriFilterData::ERROR.  Otherwise, it returns
     * a QString().
     *
     * @return the error message or a NULL when there is none.
     */
    QString errorMsg() const;

    /**
     * Returns the URI type.
     *
     * This method always returns KUriFilterData::UNKNOWN
     * if the given URL was not filtered.
     * @return the type of the URI
     */
    UriTypes uriType() const;

    /**
     * Same as above except the argument is a URL.
     *
     * Use this function to set the string to be
     * filtered when you construct an empty filter
     * object.
     *
     * @param url the URL to be filtered.
     */
    void setData( const KUrl& url );

    /**
     * Sets the URL to be filtered.
     *
     * Use this function to set the string to be
     * filtered when you construct an empty filter
     * object.
     *
     * @param url the string to be filtered.
     */
    void setData( const QString& url );

    /**
     * Sets the absolute path to be used whenever the supplied
     * data is a relative local URL.
     *
     * NOTE: This function should only be used for local resources,
     * i.e. the "file:/" protocol. It is useful for specifying the
     * absolute path in cases where the actual URL might be relative.
     * meta object.  If deriving the path from a KUrl, make sure you
     * set the argument for this function to the result of calling
     * path () instead of url ().
     *
     * @param abs_path  the abolute path to the local resource.
     * @return true if absolute path is successfully set. Otherwise, false.
     */
    bool setAbsolutePath( const QString& abs_path );

    /**
     * Returns the absolute path if one has already been set.
     * @return the absolute path, or QString()
     * @see hasAbsolutePath()
     */
    QString absolutePath() const;

    /**
     * Checks whether the supplied data had an absolute path.
     * @return true if the supplied data has an absolute path
     * @see absolutePath()
     */
    bool hasAbsolutePath() const;

    /**
     * Returns the command line options and arguments for a
     * local resource when present.
     *
     * @return options and arguments when present, otherwise QString()
     */
    QString argsAndOptions() const;

    /**
     * Checks whether the current data is a local resource with
     * command line options and arguments.
     * @return true if the current data has command line options and arguments
     */
    bool hasArgsAndOptions() const;

    /**
     * Returns the name of the icon that matches
     * the current filtered URL.
     *
     * NOTE that this function will return a NULL
     * string by default and when no associated icon
     * is found.
     *
     * @return the name of the icon associated with the resource,
     *         or QString() if not found
     */
    QString iconName();

    /**
     * Check whether the provided uri is executable or not.
     *
     * Setting this to false ensures that typing the name of
     * an executable does not start that application. This is
     * useful in the location bar of a browser. The default
     * value is true.
     */
    void setCheckForExecutables (bool check);

    /**
     * @return true if the filters should attempt to check whether the
     * supplied uri is an executable. False otherwise.
     */
    bool checkForExecutables() const;

    /**
     * @return the string as typed by the user, before any URL processing is done
     */
    QString typedString() const;

    /**
     * Overloaded assigenment operator.
     *
     * This function allows you to easily assign a KUrl
     * to a KUriFilterData object.
     *
     * @return an instance of a KUriFilterData object.
     */
    KUriFilterData& operator=( const KUrl& url );

    /**
     * Overloaded assigenment operator.
     *
     * This function allows you to easily assign a QString
     * to a KUriFilterData object.
     *
     * @return an instance of a KUriFilterData object.
     */
    KUriFilterData& operator=( const QString& url );

private:
    KUriFilterDataPrivate * const d;
};


/**
 * Base class for URI filter plugins.
 *
 * This class applies a single filter to a URI.  All plugins designed
 * to provide URI filtering service should inherit from this abstract
 * class and provide a concrete implementation.
 *
 * All inheriting classes need to implement the pure virtual function
 * filterUri.
 *
 * @short Abstract class for URI filter plugins.
 */
class KIO_EXPORT KUriFilterPlugin : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructs a filter plugin with a given name
     *
     * @param parent the parent object, or 0 for no parent
     * @param name the name of the plugin, mandatory
     */
    explicit KUriFilterPlugin( const QString &name, QObject *parent = 0 );

    /**
     * Filters a URI.
     *
     * @param data the URI data to be filtered.
     * @return A boolean indicating whether the URI has been changed.
     */
    virtual bool filterUri( KUriFilterData& data ) const = 0;

    /**
     * Creates a configuration module for the filter.
     *
     * It is the responsibility of the caller to delete the module
     * once it is not needed anymore.
     *
     * @return A configuration module, 0 if the filter isn't configurable.
     */
    virtual KCModule *configModule( QWidget*, const char* ) const;

    /**
     * Returns the name of the configuration module for the filter.
     *
     * @return the name of a configuration module or QString() if none.
     */
    virtual QString configName() const;

protected:

    /**
     * Sets the URL in @p data to @p uri.
     */
    void setFilteredUri ( KUriFilterData& data, const KUrl& uri ) const;

    /**
     * Sets the error message in @p data to @p errormsg.
     */
    void setErrorMsg ( KUriFilterData& data, const QString& errmsg ) const;

    /**
     * Sets the URI type in @p data to @p type.
     */
    void setUriType ( KUriFilterData& data, KUriFilterData::UriTypes type) const;

    /**
     * Sets the arguments and options string in @p data
     * to @p args if any were found during filterting.
     */
    void setArguments( KUriFilterData& data, const QString& args ) const;

private:
    class KUriFilterPluginPrivate * const d;
};


class KUriFilterPrivate;
/**
 * Manages the filtering of URIs.
 *
 * The intention of this plugin class is to allow people to extend the
 * functionality of KUrl without modifying it directly. This way KUrl will
 * remain a generic parser capable of parsing any generic URL that adheres
 * to specifications.
 *
 * The KUriFilter class applies a number of filters to a URI and returns the
 * filtered version whenever possible. The filters are implemented using
 * plugins to provide easy extensibility of the filtering mechanism. New
 * filters can be added in the future by simply inheriting from
 * KUriFilterPlugin and implementing the KUriFilterPlugin::filterUri
 * method.
 *
 * Use of this plugin-manager class is straight forward.  Since it is a
 * singleton object, all you have to do is obtain an instance by doing
 * @p KUriFilter::self() and use any of the public member functions to
 * preform the filtering.
 *
 * \b Example
 *
 * To simply filter a given string:
 *
 * \code
 * bool filtered = KUriFilter::self()->filterUri( "kde.org" );
 * \endcode
 *
 * You can alternatively use a KUrl:
 *
 * \code
 * KUrl url = "kde.org";
 * bool filtered = KUriFilter::self()->filterUri( url );
 * \endcode
 *
 * If you have a constant string or a constant URL, simply invoke the
 * corresponding function to obtain the filtered string or URL instead
 * of a boolean flag:
 *
 * \code
 * QString u = KUriFilter::self()->filteredUri( "kde.org" );
 * \endcode
 *
 * You can also restrict the filter(s) to be used by supplying
 * the name of the filter(s) to use.  By defualt all available
 * filters will be used. To use specific filters, add the names
 * of the filters you want to use to a QStringList and invoke
 * the appropriate filtering function. The examples below show
 * the use of specific filters. The first one uses a single
 * filter called kshorturifilter while the second example uses
 * multiple filters:
 *
 * \code
 * QString text = "kde.org";
 * bool filtered = KUriFilter::self()->filterUri( text, "kshorturifilter" );
 * \endcode
 *
 * \code
 * QStringList list;
 * list << "kshorturifilter" << "localdomainfilter";
 * bool filtered = KUriFilter::self()->filterUri( text, list );
 * \endcode
 *
 * KUriFilter also allows richer data exchange through a simple
 * meta-object called @p KUriFilterData. Using this meta-object
 * you can find out more information about the URL you want to
 * filter. See KUriFilterData for examples and details.
 *
 * @short Filters a given URL into its proper format whenever possible.
 */

class KIO_EXPORT KUriFilter
{
public:
    /**
     *  Destructor
     */
    ~KUriFilter ();

    /**
     * Returns an instance of KUriFilter.
     */
    static KUriFilter* self();

    /**
     * Filters the URI given by the object UriFilterData.
     *
     * The given URL is filtered based on the specified list of filters.
     * If the list is empty all available filters would be used.
     *
     * @param data object that contains the URI to be filtered.
     * @param filters specify the list of filters to be used.
     *
     * @return a boolean indicating whether the URI has been changed
     */
    bool filterUri( KUriFilterData& data, const QStringList& filters = QStringList() );

    /**
     * Filters the URI given by the URL.
     *
     * The given URL is filtered based on the specified list of filters.
     * If the list is empty all available filters would be used.
     *
     * @param uri the URI to filter.
     * @param filters specify the list of filters to be used.
     *
     * @return a boolean indicating whether the URI has been changed
     */
    bool filterUri( KUrl &uri, const QStringList& filters = QStringList() );

    /**
     * Filters a string representing a URI.
     *
     * The given URL is filtered based on the specified list of filters.
     * If the list is empty all available filters would be used.
     *
     * @param uri The URI to filter.
     * @param filters specify the list of filters to be used.
     *
     * @return a boolean indicating whether the URI has been changed
     */
    bool filterUri( QString &uri, const QStringList& filters = QStringList() );

    /**
     * Returns the filtered URI.
     *
     * The given URL is filtered based on the specified list of filters.
     * If the list is empty all available filters would be used.
     *
     * @param uri The URI to filter.
     * @param filters specify the list of filters to be used.
     *
     * @return the filtered URI or null if it cannot be filtered
     */
    KUrl filteredUri( const KUrl &uri, const QStringList& filters = QStringList() );

    /**
     * Return a filtered string representation of a URI.
     *
     * The given URL is filtered based on the specified list of filters.
     * If the list is empty all available filters would be used.
     *
     * @param uri the URI to filter.
     * @param filters specify the list of filters to be used.
     *
     * @return the filtered URI or null if it cannot be filtered
     */
    QString filteredUri( const QString &uri, const QStringList& filters = QStringList() );

    /**
     * Return a list of the names of all loaded plugins.
     *
     * @return a QStringList of plugin names
     */
    QStringList pluginNames() const;

protected:

    /**
     * A protected constructor.
     *
     * This constructor creates a KUriFilter and
     * initializes all plugins it can find by invoking
     * loadPlugins.
     */
    KUriFilter();

    /**
     * Loads all allowed plugins.
     *
     * This function loads all filters that have not
     * been disbled.
     */
    void loadPlugins();

private:
    KUriFilterPrivate * const d;
};

#endif
