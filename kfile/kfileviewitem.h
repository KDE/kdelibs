// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
		  1999 Carsten Pfeiffer <pfeiffer@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KFILEVIEWITEM_H
#define KFILEVIEWITEM_H

#include <time.h>

#include <qdict.h>
#include <qintdict.h>
#include <qptrlist.h>
#include <qmap.h>
#include <qstring.h>

#include <kfileitem.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <kio/global.h>


class QFileInfo;
class QPixmap;
class KFileViewItem;
class KFileView;

/**
  * Provides information about a file that has been created
  * by KDirLister/KIO.
  *
  * @author rich@kde.org
  * @version $Id$
  */
class KFileViewItem : public KFileItem
{
public:
    /**
      * Construct a KFileViewItem
      */
    KFileViewItem(const KURL& baseURL, const KIO::UDSEntry &);

    /**
      * Constructs a "little" KFileViewItem (just for local files)
      */
    KFileViewItem( mode_t _mode, mode_t _permissions, const KURL& _url,
		   bool _determineMimeTypeOnDemand );

    /**
     * Copy constructor. Note: you can't have an item twice in a view.
     * This constructor does <em>not</em> copy any set @ref viewItem() and
     * neither does it copy the reference to the @ref next() item.
     */
    KFileViewItem( const KFileViewItem& );

    /**
      * Destroy the KFileViewItem object.
      */
    ~KFileViewItem();

    /**
      * Returns true if this item represents a file (and not a a directory)
      */
    bool isFile() const { return !isDir(); }

    QString urlString() const;

    /**
     * Returns a pixmap representing the file
     * @param size KDE-size, for the pixmap, e.g. KIcon::Small
     * @returns the pixmap.
     * @see KFileItem::pixmap
     */
    QPixmap pixmap( int size, int state = 0 ) const {
	return KFileItem::pixmap( size, state );
    }

    /**
     * Returns a pixmap for this file. The only difference to the other
     * pixmap() method is that the last used Pixmap-size will be used.
     * E.g. if you call pixmap( KIcon::SizeSmall ) once, you can refer to
     * this pixmap thru this method without specifying the size.
     * The default size is KIcon::SizeSmall. Use 0 for the default size.
     */
    QPixmap pixmap() const;

    /**
      * Returns a string of the date of the file.
      */
    QString date() const;

    /**
      * Returns the access permissions for the file as a string.
      */
    QString access() const;

    void setViewItem( const KFileView *view, const void *item );
    const void *viewItem( const KFileView *view ) const;

    static QString dateTime(time_t secsSince1Jan1970UTC);

    KFileViewItem *next() const { return myNext; }
    void setNext(KFileViewItem *n) { myNext = n; }

    /**
     * Assigns an item to this item. Does not copy the @ref viewItem() and the 
     * @ref next() item.
     */
    KFileViewItem& operator=( const KFileViewItem& );

protected:
    QString parsePermissions(mode_t perm) const;
    //    bool testReadable(const QCString& file, struct stat& buf);

private:
    void init();

    // will be filled on first access
    mutable QString myURLString;

    QMap< const KFileView *, const void * > viewItems;
    KFileViewItem *myNext;

private:
    class KFileViewItemPrivate;
    KFileViewItemPrivate *d;

};

typedef QPtrList<KFileViewItem> KFileViewItemList;
typedef QPtrListIterator<KFileViewItem> KFileViewItemListIterator;

#endif // KFILEINFO_H
