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

#ifndef __kurifilter_h__
#define __kurifilter_h__

#include <qptrlist.h>
#include <qobject.h>
#include <qstringlist.h>

#include <kurl.h>

#ifdef Q_OS_WIN
#undef ERROR
#endif

class KURIFilterPrivate;
class KURIFilterDataPrivate;

class KCModule;

/**
* A basic message object used for exchanging filtering
* information between the filter plugins and the application
* requesting the filtering service.
*
* Use this object if you require a more detailed information
* about the URI you want to filter. Any application can create
* an instance of this class and send it to KURIFilter to
* have the plugins fill out all possible information about the
* URI.
*
* \b Example
*
* \code
*   QString text = "kde.org";
*   KURIFilterData d = text;
*   bool filtered = KURIFilter::self()->filter( d );
*   cout << "URL: " << text.latin1() << endl
*        << "Filtered URL: " << d.uri().url().latin1() << endl
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

class KIO_EXPORT KURIFilterData
{
friend class KURIFilterPlugin;

public:
    /**
     * Describes the type of the URI that was filtered.
     * Here is a brief description of the types:
     *
     * @li NET_PROTOCOL - Any network protocol: http, ftp, nttp, pop3, etc...
     * @li LOCAL_FILE   - A local file whose executable flag is not set
     * @li LOCAL_DIR    - A local directory
     * @li EXECUTABLE   - A local file whose executable flag is set
     * @li HELP         - A man or info page
     * @li SHELL        - A shell executable (ex: echo "Test..." >> ~/testfile)
     * @li BLOCKED      - A URI that should be blocked/filtered (ex: ad filtering)
     * @li ERROR        - An incorrect URI (ex: "~johndoe" when user johndoe
     *                    does not exist in that system )
     * @li UNKNOWN      - A URI that is not identified. Default value when
     *                    a KURIFilterData is first created.
     */
    enum URITypes { NET_PROTOCOL=0, LOCAL_FILE, LOCAL_DIR, EXECUTABLE, HELP, SHELL, BLOCKED, ERROR, UNKNOWN };

    /**
     * Default constructor.
     *
     * Creates a URIFilterData object.
     */
    KURIFilterData() { init(); }

    /**
     * Creates a URIFilterData object from the given URL.
     *
     * @param url is the URL to be filtered.
     */
    KURIFilterData( const KURL& url ) { init( url); }

    /**
     * Creates a URIFilterData object from the given string.
     *
     * @param url is the string to be filtered.
     */
    KURIFilterData( const QString& url ) { init( url ); }

    /**
     * Copy constructor.
     *
     * Creates a URIFilterData object from another
     * URI filter data object.
     *
     * @param data the uri filter data to be copied.
     */
    KURIFilterData( const KURIFilterData& data);

    /**
     * Destructor.
     */
    ~KURIFilterData();

    /**
     * This method has been deprecated and will always return
     * true.  You should instead use the result from the
     * KURIFilter::filterURI() calls.
     *
     * @deprecated
     */
    KDE_DEPRECATED bool hasBeenFiltered() const { return true; }

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
    KURL uri() const { return m_pURI; }

    /**
     * Returns an error message.
     *
     * This functions returns the error message set
     * by the plugin whenever the uri type is set to
     * KURIFilterData::ERROR.  Otherwise, it returns
     * a QString::null.
     *
     * @return the error message or a NULL when there is none.
     */
    QString errorMsg() const { return m_strErrMsg; }

    /**
     * Returns the URI type.
     *
     * This method always returns KURIFilterData::UNKNOWN
     * if the given URL was not filtered.
     * @return the type of the URI
     */
    URITypes uriType() const { return m_iType; }

    /**
     * Sets the URL to be filtered.
     *
     * Use this function to set the string to be
     * filtered when you construct an empty filter
     * object.
     *
     * @param url the string to be filtered.
     */
    void setData( const QString& url ) { init( url ); }

    /**
     * Same as above except the argument is a URL.
     *
     * Use this function to set the string to be
     * filtered when you construct an empty filter
     * object.
     *
     * @param url the URL to be filtered.
     */
    void setData( const KURL& url ) { init( url ); }

    /**
     * Sets the absolute path to be used whenever the supplied
     * data is a relative local URL.
     *
     * NOTE: This function should only be used for local resources,
     * i.e. the "file:/" protocol. It is useful for specifying the
     * absolute path in cases where the actual URL might be relative.
     * meta object.  If deriving the path from a KURL, make sure you
     * set the argument for this function to the result of calling
     * path () instead of url ().
     *
     * @param abs_path  the abolute path to the local resource.
     * @return true if absolute path is successfully set. Otherwise, false.
     */
    bool setAbsolutePath( const QString& abs_path );

    /**
     * Returns the absolute path if one has already been set.
     * @return the absolute path, or QString::null
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
     * @return options and arguments when present, otherwise QString::null
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
     *         or QString::null if not found
     */
    QString iconName();

    /**
     * Check whether the provided uri is executable or not.
     *
     * Setting this to false ensures that typing the name of
     * an executable does not start that application. This is
     * useful in the location bar of a browser. The default
     * value is true.
     *
     * @since 3.2
     */
    void setCheckForExecutables (bool check);

    /**
     * @return true if the filters should attempt to check whether the
     * supplied uri is an executable. False otherwise.
     *
     * @since 3.2
     */
    bool checkForExecutables() const { return m_bCheckForExecutables; }

    /**
     * @return the string as typed by the user, before any URL processing is done
     * @since 3.2
     */
    QString typedString() const;

    /**
     * Overloaded assigenment operator.
     *
     * This function allows you to easily assign a KURL
     * to a KURIFilterData object.
     *
     * @return an instance of a KURIFilterData object.
     */
    KURIFilterData& operator=( const KURL& url ) { init( url ); return *this; }

    /**
     * Overloaded assigenment operator.
     *
     * This function allows you to easily assign a QString
     * to a KURIFilterData object.
     *
     * @return an instance of a KURIFilterData object.
     */
    KURIFilterData& operator=( const QString& url ) { init( url ); return *this; }

protected:

    /**
     * Initializes the KURIFilterData on construction.
     * @param url the URL to initialize the object with
     */
    void init( const KURL& url);

    /**
     * Initializes the KURIFilterData on construction.
     * @param url the URL to initialize the object with
     */
    void init( const QString& url = QString::null );

private:
    bool m_bCheckForExecutables;
    bool m_bChanged;

    QString m_strErrMsg;
    QString m_strIconName;

    KURL m_pURI;
    URITypes m_iType;
    KURIFilterDataPrivate *d;
};


/**
 * Base class for URI filter plugins.
 *
 * This class applies a single filter to a URI.  All plugins designed
 * to provide URI filtering service should inherit from this abstract
 * class and provide a concrete implementation.
 *
 * All inheriting classes need to implement the pure virtual function
 * filterURI.
 *
 * @short Abstract class for URI filter plugins.
 */
class KIO_EXPORT KURIFilterPlugin : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructs a filter plugin with a given name and
     * priority.
     *
     * @param parent the parent object, or 0 for no parent
     * @param name the name of the plugin, or 0 for no name
     * @param pri the priority of the plugin.
     */
    KURIFilterPlugin( QObject *parent = 0, const char *name = 0, double pri = 1.0 );

    /**
     * Returns the filter's name.
     *
     * @return A string naming the filter.
     */
    virtual QString name() const { return m_strName; }

    /**
     * Returns the filter's priority.
     *
     * Each filter has an assigned priority, a float from 0 to 1. Filters
     * with the lowest priority are first given a chance to filter a URI.
     *
     * @return The priority of the filter.
     */
    virtual double priority() const { return m_dblPriority; }

    /**
     * Filters a URI.
     *
     * @param data the URI data to be filtered.
     * @return A boolean indicating whether the URI has been changed.
     */
    virtual bool filterURI( KURIFilterData& data ) const = 0;

    /**
     * Creates a configuration module for the filter.
     *
     * It is the responsibility of the caller to delete the module
     * once it is not needed anymore.
     *
     * @return A configuration module, 0 if the filter isn't configurable.
     */
    virtual KCModule *configModule( QWidget*, const char* ) const { return 0; }

    /**
     * Returns the name of the configuration module for the filter.
     *
     * @return the name of a configuration module or QString::null if none.
     */
    virtual QString configName() const { return name(); }

protected:

    /**
     * Sets the the URL in @p data to @p uri.
     */
    void setFilteredURI ( KURIFilterData& data, const KURL& uri ) const;

    /**
     * Sets the error message in @p data to @p errormsg.
     */
    void setErrorMsg ( KURIFilterData& data, const QString& errmsg ) const {
        data.m_strErrMsg = errmsg;
    }

    /**
     * Sets the URI type in @p data to @p type.
     */
    void setURIType ( KURIFilterData& data, KURIFilterData::URITypes type) const {
        data.m_iType = type;
        data.m_bChanged = true;
    }

    /**
     * Sets the arguments and options string in @p data
     * to @p args if any were found during filterting.
     */
    void setArguments( KURIFilterData& data, const QString& args ) const;

    QString m_strName;
    double m_dblPriority;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KURIFilterPluginPrivate *d;
};


/**
 * A list of filter plugins.
 */
class KIO_EXPORT KURIFilterPluginList : public QPtrList<KURIFilterPlugin>
{
public:
    virtual int compareItems(Item a, Item b)
    {
      double diff = ((KURIFilterPlugin *) a)->priority() - ((KURIFilterPlugin *) b)->priority();
      return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
    }

private:
    KURIFilterPrivate *d;

};

/**
 * Manages the filtering of URIs.
 *
 * The intention of this plugin class is to allow people to extend the
 * functionality of KURL without modifying it directly. This way KURL will
 * remain a generic parser capable of parsing any generic URL that adheres
 * to specifications.
 *
 * The KURIFilter class applies a number of filters to a URI and returns the
 * filtered version whenever possible. The filters are implemented using
 * plugins to provide easy extensibility of the filtering mechanism. New
 * filters can be added in the future by simply inheriting from
 * KURIFilterPlugin and implementing the KURIFilterPlugin::filterURI
 * method.
 *
 * Use of this plugin-manager class is straight forward.  Since it is a
 * singleton object, all you have to do is obtain an instance by doing
 * @p KURIFilter::self() and use any of the public member functions to
 * preform the filtering.
 *
 * \b Example
 *
 * To simply filter a given string:
 *
 * \code
 * bool filtered = KURIFilter::self()->filterURI( "kde.org" );
 * \endcode
 *
 * You can alternatively use a KURL:
 *
 * \code
 * KURL url = "kde.org";
 * bool filtered = KURIFilter::self()->filterURI( url );
 * \endcode
 *
 * If you have a constant string or a constant URL, simply invoke the
 * corresponding function to obtain the filtered string or URL instead
 * of a boolean flag:
 *
 * \code
 * QString u = KURIFilter::self()->filteredURI( "kde.org" );
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
 * bool filtered = KURIFilter::self()->filterURI( text, "kshorturifilter" );
 * \endcode
 *
 * \code
 * QStringList list;
 * list << "kshorturifilter" << "localdomainfilter";
 * bool filtered = KURIFilter::self()->filterURI( text, list );
 * \endcode
 *
 * KURIFilter also allows richer data exchange through a simple
 * meta-object called @p KURIFilterData. Using this meta-object
 * you can find out more information about the URL you want to
 * filter. See KURIFilterData for examples and details.
 *
 * @short Filters a given URL into its proper format whenever possible.
 */

class KIO_EXPORT KURIFilter
{
public:
    /**
     *  Destructor
     */
    ~KURIFilter ();

    /**
     * Returns an instance of KURIFilter.
     */
    static KURIFilter* self();

    /**
     * Filters the URI given by the object URIFilterData.
     *
     * The given URL is filtered based on the specified list of filters.
     * If the list is empty all available filters would be used.
     *
     * @param data object that contains the URI to be filtered.
     * @param filters specify the list of filters to be used.
     *
     * @return a boolean indicating whether the URI has been changed
     */
    bool filterURI( KURIFilterData& data, const QStringList& filters = QStringList() );

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
    bool filterURI( KURL &uri, const QStringList& filters = QStringList() );

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
    bool filterURI( QString &uri, const QStringList& filters = QStringList() );

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
    KURL filteredURI( const KURL &uri, const QStringList& filters = QStringList() );

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
    QString filteredURI( const QString &uri, const QStringList& filters = QStringList() );

    /**
     * Return an iterator to iterate over all loaded
     * plugins.
     *
     * @return a plugin iterator.
     */
    QPtrListIterator<KURIFilterPlugin> pluginsIterator() const;

    /**
     * Return a list of the names of all loaded plugins.
     *
     * @return a QStringList of plugin names
     * @since 3.1
     */
    QStringList pluginNames() const;

protected:

    /**
     * A protected constructor.
     *
     * This constructor creates a KURIFilter and
     * initializes all plugins it can find by invoking
     * loadPlugins.
     */
    KURIFilter();

    /**
     * Loads all allowed plugins.
     *
     * This function loads all filters that have not
     * been disbled.
     */
    void loadPlugins();

private:
    static KURIFilter *s_self;
    KURIFilterPluginList m_lstPlugins;
    KURIFilterPrivate *d;
};

#endif
