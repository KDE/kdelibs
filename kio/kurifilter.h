/* This file is part of the KDE libraries
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

class KCModule;

/**
 * Filters a URI.
 *
 * The KURIFilterPlugin class applies a single filter to a URI.
 *
 */

class KURIFilterPlugin : public QObject {
    Q_OBJECT

public:

    /**
     * Constructor to create a filter plugin with a given name and
     * priority.
     * @param pname The name of the plugin.
     * @param pri The priority of the plugin.
     *
     */
    KURIFilterPlugin(QObject *parent = 0, const char *name = 0,
                     const QString &pname = QString::null, double pri = 1.0)
        : QObject(parent, name), m_strName(pname), m_dblPriority(pri)
    {  }

    /**
     * Return the filter's name.
     * @return A string naming the filter.
     */
    virtual QString name() const {
	return m_strName;
    }

    /**
     * Return the filter's .
     * Each filter has an assigned priority, a float from 0 to 1. Filters
     * with the lowest priority are first given a chance to filter a URI.
     * @return The priority of the filter.
     */
    virtual double priority() const {
	return m_dblPriority;
    }

    /**
     * Filter a URI.
     * @param uri The URI to be filtered.
     * @return A boolean indicating whether the URI has been changed
     * or not.
     */
    virtual bool filterURI( KURL& ) {
	return false;
    }

    /**
     * Filter a string representing a URI.
     * @param uri The URI to be filtered.
     * @return A boolean indicating whether the URI has been changed
     * or not.
     */
    virtual bool filterURI( QString &uri );

    /**
     * Return a configuration module for the filter.
     * It is the responsability of the caller to delete the module
     * once it is not needed anymore.
     * @return A configuration module, or 0 if the filter isn't
     * configurable.
     */
    virtual KCModule *configModule(QWidget *parent = 0, const char *name = 0) const = 0;

    /**
     * Return a configuration module for the filter.
     * It is the responsability of the caller to delete the module
     * once it is not needed anymore.
     * @return A configuration module, or 0 if the filter isn't
     * configurable.
     */
    virtual QString configName() const {
	return name();
    }

protected:
    QString m_strName;
    double m_dblPriority;
};

class KURIFilterPluginList : public QList<KURIFilterPlugin> {
public:
    virtual int compareItems(Item a, Item b) {
	double diff = ((KURIFilterPlugin *) a)->priority() - ((KURIFilterPlugin *) b)->priority();
	return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
    }
};

/**
 * Manage the filtering of a URI.
 *
 * The KURIFilter class applies a number of filters to a URI, and returns
 * the filtered URI. The filters are implemented by plugins that provide
 * easy extensibility of the filtering mechanism.
 *
 */

class KURIFilter {
public:
    /**
     * Create a KURIFIlter.
     * The filter will be initialized with all the plugins it can find.
     */
    KURIFilter();

    /**
     * Return a static instance of KURIFilter.
     */
    static KURIFilter *filter();

    /**
     * Filter a URI.
     * @param uri The URI to filter.
     * @return A boolean indicating whether the URI has been changed
     * or not.
     */
    bool filterURI(KURL &uri);
    /**
     * Filter a string representing a URI.
     * @param uri The URI to filter.
     * @return A boolean indicating whether the URI has been changed
     * or not.
     */
    bool filterURI(QString &uri);

    /**
     * Return a filtered URI.
     * @param uri The URI to filter.
     * @return The filtered URI.
     */
    KURL filteredURI(const KURL &uri);
    /**
     * Return a filtered string representation of a URI.
     * @param uri The URI to filter.
     * @return The filtered URI.
     */
    QString filteredURI(const QString &uri);

    /**
     * Return an iterator to iterate over plugins.
     * @return The iterator.
     */
    QListIterator<KURIFilterPlugin> pluginsIterator() const;

protected:
    void loadPlugins();

private:
    static KURIFilter *ms_pFilter;
    KURIFilterPluginList m_lstPlugins;
};

#endif

