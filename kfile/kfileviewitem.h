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
#include <qlist.h>
#include <qmap.h>
#include <qstring.h>

#include <kiconloader.h>
#include <kio_interface.h>
#include <kmimetype.h>

class QFileInfo;
class QPixmap;
class KFileViewItem;
class KFileView;

typedef QList<KFileViewItem> KFileViewBaseList;

class KFileViewItemList : public KFileViewBaseList
{
public:
    void append( const KFileViewItem * );
    void clear();
    const KFileViewItem * findByName( const QString& ) const;

private:
    QDict<KFileViewItem> myDict;
    bool dictdirty;
};


/**
  * Provides information about a file that has been examined
  * with KDir.
  *
  * @author rich@kde.org
  * @version $Id$
  */
class KFileViewItem {

public:
    /**
      * Construct a KFileViewItem object from a KDirEntry.
      */
    KFileViewItem(const KUDSEntry &);

    /**
      * Copy a KFileViewItem.
      */
    KFileViewItem (const KFileViewItem &);

    /**
      * Constructs a "little" KFileViewItem (just for local files)
      **/
    KFileViewItem(const QString& dir, const QString& name, bool delaystat = false);

    /**
      * Destroy the KFileViewItem object.
      */
    ~KFileViewItem();

    /**
      * Copy a KFileViewItem.
      */
    KFileViewItem &operator=(const KFileViewItem &);

    /**
      * Returns true if this file is a directory.
      */
    bool isDir() const { return myIsDir; }
    bool isFile() const { return myIsFile; }
    bool isSymLink() const { return myIsSymLink; }

    /**
      * Returns the name of the file
      *
      * Note: If this object does not refer to a real file
      * (broken symlink), it will return QString::null
      **/
    QString name() const { return myName; }

    QString url() const;

    /**
     * Returns an icon for this file.
     * It will return the icon according to the mimetype of this file, when
     * you have called @see mimeType() once. Otherwise, it will return a
     * default icon.
     * @param size specifies the size of the icon.
     * The default icon is only available in small size, so far.
     */
    QPixmap pixmap( KIconLoader::Size size ) const;

    /**
     * Returns a pixmap for this file. The only semantic difference to the 
     * other pixmap() method is that the last used Pixmap-size will be used.
     * E.g. if you call pixmap( KIconLoader::Large ) once, you can refer to
     * this pixmap thru this method without specifying the size.
     * The default size is KIconLoader::Small
     */
    QPixmap pixmap() const { return pixmap( myPixmapSize ); }
    
    /**
      * Returns a string of the date of the file.
      */
    QString date() const;

    /**
     * Returns the modification time of the file.
     */
    time_t mTime() const { return myDate_t; }

    /**
      * Returns the access permissions for the file as a string.
      */
    QString access() const;

    /**
      * Returns the owner of the file.
      */
    QString owner() const;

    /**
      * Returns the group of the file.
      */
    QString group() const;

    /**
      * Returns the size of the file.
      */
    uint size() const { return mySize; }

    /**
      * Returns true if the specified permission flag is set.
      */
    bool permission(uint permissionSpec);

    /**
     * Return true if the file is readable
     */
    bool isReadable() const  { return myIsReadable; }

    /**
     * Returns the mimetype, e.g. "image/png"
     * When this method is called the first time, it will also search for
     * the correct icon according to this mimetype. The next call to
     * @see pixmap() will return this correct pixmap.
     */
    QString mimeType();

    void stat(bool alreadyindir);

    /**
     * frees the static allocated ressources (user information from
     * /etc/passwd and /etc/group)
     */
    static void cleanup();

    void setViewItem( const KFileView *view, const void *item );
    const void *viewItem( const KFileView *view ) const;

    static QString dateTime(time_t secsSince1Jan1970UTC);

    void setHidden(bool h) { myHidden = h; }
    bool isHidden() const { return myHidden; }

    void setDeleted();

    KFileViewItem *next() const { return myNext; }
    void setNext(KFileViewItem *n) { myNext = n; }

protected:
    void parsePermissions(const char *perms);
    QString parsePermissions(uint perm) const;
    void readUserInfo();
    bool testReadable(const QCString& file, struct stat& buf);

private:
    bool myHidden;
    QString myName;
    QString myBaseURL;

    // will be filled on first access
    mutable QString myDate;
    mutable QString myFilePath;
    mutable QString myOwner;
    mutable QString myGroup;
    mutable QString myAccess;

    time_t myDate_t;
    uid_t myOwner_t;
    gid_t myGroup_t;

    bool myIsDir;
    bool myIsFile;
    bool myIsSymLink;
    uint myPermissions;
    int mySize;
    bool myIsReadable;

    QMap< const KFileView *, const void * > viewItems;

    KFileViewItem *myNext;

    mutable const QPixmap *myPixmap;
    mutable bool myPixmapDirty;
    mutable KIconLoader::Size myPixmapSize;
    KMimeType::Ptr myMimeType;

    // cache for passwd and group entries
    typedef QIntDict<char> IntCache;
    static IntCache *passwdCache;
    static IntCache *groupCache;

    typedef QValueList<gid_t> GroupCache;
    static GroupCache *myGroupMemberships;

    void init();
    QString defaultIcon() const;

};

// typedef QList<KFileViewItem> KFileViewItemList;
typedef QListIterator<KFileViewItem> KFileViewItemListIterator;

inline bool KFileViewItem::permission(uint permissionSpec)
{
  return ((myPermissions & permissionSpec) != 0);
}


#endif // KFILEINFO_H
