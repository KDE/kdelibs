/**
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000 Yves Arrouye <yves@realnames.com>
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef __kurifilter_h__
#define __kurifilter_h__ "$Id$"

#include <qlist.h>
#include <qobject.h>

#include <kurl.h>

class KURIFilterPrivate;
class QStringList;
class KCModule;

/**
* This is a basic message object used for exchanging filtering
* info between the filter plugins and the application whenever
* the application requires more information about the URI than
* just a filtered version of it.  Any application can create an
* instance of this class and send it to @ref KURIFilter to have
* the filter plugins fill the necessary information.
*
* @sect Example
*
* <pre>
*   QString text = "kde.org";
*   KURIFilterData d = text;
*   bool filtered = KURIFilter::self()->filter( d );
*   if( filtered )
*     print ( "URI : %s\n"Filtered URI : %s\n URI Type : %i\n"Was Filtered :%i"
*             text.latin1(), d.uri().url().latin1(), d.uriType(), d.hasBeenFiltered() );
* </pre>
*
* The above code should give the following result:
*
* <pre>
*   kde.org
*   http://kde.org
*   0 <== which would be NET_PROTOCOL
*   1 <== which means the url has been filtered
* </pre>
*
* @short A message object for exchanging filtering URI info.
*/

class KURIFilterData
{
friend class KURIFilterPlugin;

public:
    /**
     * Describes the type of URI to be filtered.
     *
     * This enumerator prvoides the return value for
     * @ref uriType.  A breif description for each value:
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
    KURIFilterData( const KURIFilterData& /*data*/);

    /**
     * Destructor.
     */
    ~KURIFilterData() {};

    /**
     * Returns true if the URI has been filtered.
     *
     * This function returns true whenever one of
     * the URI filters plug-ins sucessfully filters
     * the given URL.  Use this function to check whether
     * a URL has been filtered or not.
     *
     * @return true if the URL has been filtered.
     */
    bool hasBeenFiltered() const { return m_bFiltered; }

    /**
     * Returns the filtered or the original URL.
     *
     * This function returns the filtered url if one
     * of the plugins sucessfully filtered the original
     * URL.  Otherwise, it returns the original URL.
     * See @ref #hasBeenFiltered() and
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
     * Returns the name of the icon that matches
     * the current filtered URL.
     *
     * NOTE that this function will return a NULL
     * string by default and when no associated icon
     * is found.
     *
     * @return the name of the icon associated with the resource
     */    
    QString iconName();
    
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
     */
    void init( const KURL& url = QString::null );

private:
    bool m_bFiltered;
    bool m_bChanged;
    
    QString m_strErrMsg;
    QString m_strIconName;
    
    KURL m_pURI; 
    URITypes m_iType;
    KURIFilterPrivate *d; // BCI    
};


/**
 * Base class for URI filter plugins.
 *
 * This class applies a single filter to a URI.  All
 * plugins designed to provide URI filtering functionalities
 * should inherit from this abstract class and provide a
 * specific filtering implementation.
 *
 * All inheriting classes need to implement the pure
 * virtual function @ref filterURI.  Otherwise, they
 * would also become abstract.
 *
 * @short Abstract class for URI filter plugins.
 */
class KURIFilterPlugin : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor to create a filter plugin with a given name and
     * priority.
     *
     * @param parent the parent object.
     * @param name the name of the plugin.
     * @param pri the priority of the plugin.
     */
    KURIFilterPlugin( QObject *parent = 0, const char *name = 0, double pri = 1.0 );

    /**
     * Return the filter's name.
     *
     * @return A string naming the filter.
     */
    virtual QString name() const { return m_strName; }

    /**
     * Return the filter's .
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
     * Returns a configuration module for the filter.
     *
     * It is the responsability of the caller to delete the module
     * once it is not needed anymore.
     *
     * @return A configuration module, @p null if the filter isn't configurable.
     */
    virtual KCModule *configModule( QWidget*, const char* ) const { return 0; }

    /**
     * Returns the name of the configuration module for the filter.
     *
     * It is the responsability of the caller to delete the module
     * once it is not needed anymore.
     *
     * @return the name of a configuration module or @p null if none.
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

    QString m_strName;
    double m_dblPriority;

private:
    KURIFilterPrivate *d;
};


class KURIFilterPluginList : public QList<KURIFilterPlugin>
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
 * Manages the filtering of a URI.
 *
 * The intention of this plugin class is to allow people extend
 * the functionality of KURL w/o modifying it directly.  This
 * way KURL will remain a generic parser capable of parsing any
 * generic URL that adheres to specifications.
 *
 * The KURIFilter class applies a number of filters to a URI,
 * and returns the filtered version if successful. The filters
 * are implemented using plugins to provide easy extensibility
 * of the filtering mechanism.  Any new filters can be added in
 * the future without modifying this manager.  New plugins can
 * be simply added at anytime by inheriting from @ref KURIFilterPlugin.
 *
 * Use of this plugin is simple and straight forward.  Since
 * KURIFilter is a Singleton object, you obtain an instance of
 * it by doing @p KURIFilter::self().  Then you can use any of
 * the public member functions to preform the filtering.  
 * 
 * @sect Example
 *
 * To simply filter a given string
 *
 * <pre>
 * // Should return "http://kde.org"
 * bool filtered = KURIFilter::self()->filterURI( "kde.org" );
 * </pre>
 * 
 * You can alternatively use a KURL
 * 
 * <pre>
 * KURL url = "kde.org";
 * // Should also return "http://kde.org"
 * bool filtered = KURIFilter::self()->filterURI( url );
 * </pre>
 *
 * If you have a constant string or url simply invoking
 * the corresponding function that return the filtered
 * string or URL instead of a boolean flag:
 *
 * <pre>
 * QString u = KURIFilter::self()->filteredURI( "kde.org" );
 * </pre>
 *
 * With any of the filter functions you can specify which
 * specific filter(s) are supposed to be applied by supplying
 * the a single or a list of filter name(s) as a second
 * argument.  By defualt all filters that are found are loaded
 * when the KURIFilter object is created.  These names are taken
 * from the enteries in the desktop files.  This information can
 * also be easily obtained from the control panel. Here are a
 * couple of examples:
 *
 * <pre>
 * QString text = "kde.org";
 * bool filtered = KURIFilter::self()->filterURI( text, "KShortURIFilter" );
 *
 * QStringList list;
 * list << "KShortURIFilter" << "MyFilter";
 * bool filtered = KURIFilter::self()->filterURI( text, list );
 * </pre>
 *
 * KURIFilter also allows richer data exchange through a simple
 * object called @p KURIFilterData.  Using this message object
 * you can find out more information about the URL you want to
 * filter.  See @ref KURIFilterData for examples and details.
 *
 * @short Filters, whenever possible, a given URL into its proper format.
 */

class KURIFilter
{
public:

    /**
     * Return a static instance of KURIFilter.
     */
    static KURIFilter* self();

    /**
     * Filters the URI given by the object URIFilterData.
     *
     * This filters the given data based on the specified
     * filter list.  If the list is empty all avaliable
     * filter plugins would be used.  If not, only those
     * given in the list are used.
     *
     * @param data object that contains the URI to be filtered.
     * @param filters specify the list filters to be used
     *
     * @return a boolean indicating whether the URI has been changed
     */
    bool filterURI( KURIFilterData& data, const QStringList& filters = QStringList() );

    /**
     * Filters the URI given by the URL.
     *
     * This filters the given URL based on the specified
     * filter list.  If the list is empty all avaliable
     * filter plugins would be used.  If not, only those
     * given in the list are used.
     *
     * @param uri the URI to filter.
     * @param filters specify the list of filters to be used
     *
     * @return a boolean indicating whether the URI has been changed
     */
    bool filterURI( KURL &uri, const QStringList& filters = QStringList() );

    /**
     * Filters a string representing a URI.
     *
     * This filters the given string based on the specified
     * filter list.  If the list is empty all avaliable
     * filter plugins would be used.  If not, only those
     * given in the list are used.
     *
     * @param uri The URI to filter.
     * @param filters specify the list filters to be used
     *
     * @return a boolean indicating whether the URI has been changed
     */
    bool filterURI( QString &uri, const QStringList& filters = QStringList() );

    /**
     * Returns the filtered URI.
     *
     * This filters the given URL based on the specified
     * filter list.  If the list is empty all avaliable
     * filter plugins would be used.  If not, only those
     * given in the list are used.
     *
     * @param uri The URI to filter.
     * @param filters specify the list filters to be used
     *
     * @return the filtered URI or null if it cannot be filtered
     */
    KURL filteredURI( const KURL &uri, const QStringList& filters = QStringList() );

    /**
     * Return a filtered string representation of a URI.
     *
     * This filters the given URL based on the specified
     * filter list.  If the list is empty all avaliable
     * filter plugins would be used.  If not, only those
     * given in the list are used.
     *
     * @param uri the URI to filter.
     * @param filters specify the list filters to be used
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
    QListIterator<KURIFilterPlugin> pluginsIterator() const;

protected:

    /**
     * A protected constructor.
     *
     * This constructor creates a KURIFilter and
     * initializes all plugins it can find by invoking
     * @ref loadPlugins.
     */
    KURIFilter();

    /**
     * Loads all allowed plugins.
     *
     * This function loads all filters that have not
     * been dis
     */
    void loadPlugins();

private:

    static KURIFilter *ms_pFilter;
    KURIFilterPluginList m_lstPlugins;
    KURIFilterPrivate *d;

};

#endif

