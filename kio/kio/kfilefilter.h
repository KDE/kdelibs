/* This file is part of the KDE libraries

   Copyright (c) 2001,2002 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KFILEFILTER_H
#define KFILEFILTER_H

#include <qptrlist.h>
#include <qstringlist.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

class QRegExp;
class KFileItem;

/**
 * A KFileFilter is a simple base class for file filters. Just
 * reimplement @ref passesFilter().
 * @short Base class for file filters.
 */
class KFileFilter
{
public:
    /**
     * Checks the given @p item.
     * @param item the item to filter
     * @return true if the @p item passes the filter, false otherwise
     */
    virtual bool passesFilter( const KFileItem *item ) const = 0;
protected:
    virtual void virtual_hook( int id, void* data );
};

/**
 * A simple file filter that can filter hidden dot files, by name, 
 * by mime type and by mode.
 * @short A simple file filter.
 */
class KSimpleFileFilter : public KFileFilter
{
public:
    /**
     * Creates a new filter. By default, it filters only hidden dot files
     * and "." and "..".
     */
    KSimpleFileFilter();
    virtual ~KSimpleFileFilter();

    /**
     * Enable or disable filtering hidden dot files.
     * This option is enabled by default.
     * @param filter true to enable filtering dot files, false to 
     *        disable
     * @see filterDotFiles
     */
    virtual void setFilterDotFiles( bool filter );
    /**
     * Checks whether filtering dot files is enabled.
     * This option is enabled by default.
     * @return true if filtering is enabled, false otherwise
     * @see setFilterDotFiles
     */
    bool filterDotFiles() const { return m_filterDotFiles; }

    /**
     * Filters "." and "..", default is true.
     * @param filter true to enable, false otherwise
     */
    virtual void setFilterSpecials( bool filter );
    /**
     * Checks whether it filters "." and "..", default is true.
     * @return true if enabled, false otherwise
     */
    bool filterSpecials() const { return m_filterSpecials; }

    // ### KDE4 make virtual and bool caseSensitive = false
    /**
     * Sets a list of regular expressions to filter by name.
     * The file will only pass if its name matches one of the regular
     * expressions.
     * @param nameFilters a list of regular expressions, seperated by
     *                    the character @p separator
     * @param caseSensitive if true, matches case sensitive. False 
     *                      otherwise
     * @param separator the separator in the @p nameFilter
     * @since 3.1
     */
    void setNameFilters( const QString& nameFilters, bool caseSensitive,
                         const QChar& separator = ' ' );
    /**
     * Sets a list of regular expressions to filter by name.
     * The file will only pass if its name matches one of the regular
     * expressions.
     * @param nameFilters a list of regular expressions, seperated by
     *                    space (' ')
     */
    virtual void setNameFilters( const QString& nameFilters );

    /**
     * @internal 
     * not implemented?
     */
    QString nameFilters() const;

    /**
     * Sets a list of mime filters. A file can only pass if its
     * mime type is contained in this list.
     * @param mimeFilters the list of mime types
     * @see setMimeFilter
     */
    virtual void setMimeFilters( const QStringList& mimeFilters );
    /**
     * Returns the list of mime types.
     * @return the list of mime types
     * @see mimeFilter
     */
    QStringList mimeFilters() const { return m_mimeFilters; }

    /**
     * Sets the mode filter. If the @p mode is 0, the filter is
     * disabled. 
     * When enabled, a file will only pass if the files mode
     * ANDed with @p mode is not zero.
     * @param mode the new mode. 0 to disable
     * @see modeFilter
     */
    virtual void setModeFilter( mode_t mode );
    /**
     * Returns the mode filter, as set by @ref setModeFilter().
     * @return the mode filter, 0 if disabled
     * @see setModeFilter
     */
    mode_t modeFilter() const { return m_modeFilter; }

    /**
     * Checks the given @p item.
     * @param item the item to filter
     * @return true if the @p item passes the filter, false otherwise
     */
    virtual bool passesFilter( const KFileItem *item ) const;

protected:
    QPtrList<QRegExp>   m_nameFilters;

private:
    QStringList         m_mimeFilters;
    bool                m_filterDotFiles :1;
    bool                m_filterSpecials :1;
    mode_t              m_modeFilter;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KSimpleFileFilterPrivate* d;
};

#endif // KFILEFILTER_H
