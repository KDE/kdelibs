/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>

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
// $Id$

#ifndef __kfileitem_h__
#define __kfileitem_h__

#include <qstringlist.h>
#include <sys/stat.h>

#include <qptrlist.h>
#include <kio/global.h>
#include <kurl.h>
#include <kmimetype.h>
#include <kfilemetainfo.h>

/**
 * A KFileItem is a generic class to handle a file, local or remote.
 * In particular, it makes it easier to handle the result of KIO::listDir
 * (UDSEntry isn't very friendly to use).
 * It includes many file attributes such as mimetype, icon, text, mode, link...
 */
class KFileItem
{
public:
  enum { Unknown = (mode_t) - 1 };

  /**
   * Creates an item representing a file, from a @ref UDSEntry.
   * This is the preferred constructor when using KIO::listDir().
   *
   * @param _entry the KIO entry used to get the file, contains info about it
   * @param _url the file url
   * @param _determineMimeTypeOnDemand specifies if the mimetype of the given
   *       URL should be determined immediately or on demand
   * @param _urlIsDirectory specifies if the url is just the directory of the
   *       fileitem and the filename from the UDSEntry should be used.
   */
  KFileItem( const KIO::UDSEntry& _entry, const KURL& _url,
             bool _determineMimeTypeOnDemand = false,
             bool _urlIsDirectory = false );

  /**
   * Creates an item representing a file, from all the necessary info for it.
   * @param _mode the file mode (according to stat() (e.g. S_IFDIR...)
   * Set to KFileItem::Unknown if unknown. For local files, KFileItem will use stat().
   * @param _permissions the access permissions
   * If you set both the mode and the permissions, you save a ::stat() for
   * local files.
   * Set to KFileItem::Unknown if you don't know the mode or the permission.
   * @param _url the file url
   *
   * @param _determineMimeTypeOnDemand specify if the mimetype of the given URL
   *       should be determined immediately or on demand
   */
  KFileItem( mode_t _mode, mode_t _permissions, const KURL& _url,
             bool _determineMimeTypeOnDemand = false );

  /**
   * Creates an item representing a file, for which the mimetype is already known.
   * @param url the file url
   * @param mimeType the name of the file's mimetype
   * @param mode the mode (S_IFDIR...)
   */
  KFileItem( const KURL &url, const QString &mimeType, mode_t mode );

  /**
   * Copy constructor. Note that extra-data set via @ref setExtraData() is not
   * deeply copied -- just the pointers are copied.
   */
  KFileItem( const KFileItem &item );

  /**
   * Destructs the KFileItem. Extra data set via setExtraData()
   * is not deleted.
   */
  virtual ~KFileItem();

  /**
   * Re-reads information (currently only permissions and mimetype).
   * This is called when the _file_ changes.
   */
  void refresh();

  /**
   * Re-reads mimetype information.
   * This is called when the mimetype database changes.
   */
  void refreshMimeType();

  /**
   * Returns the url of the file.
   * @return the url of the file
   */
  const KURL & url() const { return m_url; }

  /**
   * Sets the item's URL. Do not call unless you know what you are doing!
   * (used for example when an item got renamed).
   * @param url the item's URL
   */
  void setURL( const KURL &url );

  /**
   * Returns the permissions of the file (stat.st_mode containing only permissions).
   * @return the permissions of the file
   */
  mode_t permissions() const { return m_permissions; }

  /**
   * Returns the access permissions for the file as a string.
   * @return the access persmission as string
   */
  QString permissionsString() const;

  /**
   * Returns the file type (stat.st_mode containing only S_IFDIR, S_IFLNK, ...).
   * @return the file type
   */
  mode_t mode() const { return m_fileMode; }

  /**
   * Returns the owner of the file.
   * @return the file's owner
   */
  QString user() const;

  /**
   * Returns the group of the file.
   * @return the file's group
   */
  QString group() const;

  /**
   * Returns true if this item represents a link in the UNIX sense of
   * a link.
   * @return true if the file is a link
   */
  bool isLink() const { return m_bLink; }

  /**
   * Returns true if this item represents a directory.
   * @return true if the item is a directory
   */
  bool isDir() const;

  /**
   * Returns true if this item represents a file (and not a a directory)
   * @return true if the item is a file
   */
  bool isFile() const { return !isDir(); }

  /**
   * Checks whether the file is readable. In some cases
   * (remote files), we may return true even though it can't be read.
   * @return true if the file can be read - more precisely,
   *         false if we know for sure it can't
   */
  bool isReadable() const;

  /**
   * Returns the link destination if @ref isLink() == true.
   * @return the link destination. QString::null if the item is not a link
   */
  QString linkDest() const;

  /**
   * Returns the size of the file, if known.
   * @return the file size, or 0 if not known
   */
  KIO::filesize_t size() const;

  /**
   * Requests the modification, access or creation time, depending on @p which.
   * @param which UDS_MODIFICATION_TIME, UDS_ACCESS_TIME or even UDS_CREATION_TIME
   * @return the time asked for, (time_t)0 if not available
   * @see timeString()
   */
  time_t time( unsigned int which ) const;

  /**
   * Requests the modification, access or creation time as a string, depending
   * on @p which.
   * @param which UDS_MODIFICATION_TIME, UDS_ACCESS_TIME or even UDS_CREATION_TIME
   * @returns a formatted string of the requested time.
   * @see #time
   */
  QString timeString( unsigned int which = KIO::UDS_MODIFICATION_TIME ) const;

  /**
   * Returns true if the file is a local file.
   * @return true if the file is local, false otherwise
   */
  bool isLocalFile() const { return m_bIsLocalURL; }

  /**
   * Returns the text of the file item.
   * It's not exactly the filename since some decoding happens ('%2F'->'/').
   * @return the text of the file item
   */
  const QString& text() const { return m_strText; }

  /**
   * Return the name of the file item (without a path).
   * Similar to @ref text(), but unencoded, i.e. the original name.
   * @param lowerCase if true, the name will be returned in lower case,
   * which is useful to speed up sorting by name, case insensitively.
   * @return the file's name
   */
  const QString& name( bool lowerCase = false ) const {
      if ( !lowerCase )
          return m_strName;
      else
          if ( m_strLowerCaseName.isNull() )
              m_strLowerCaseName = m_strName.lower();
      return m_strLowerCaseName;
  }

  /**
   * Returns the mimetype of the file item.
   * If @p _determineMimeTypeOnDemand was used in the constructor, this will determine
   * the mimetype first. Equivalent to determineMimeType()->name()
   * @return the mime type of the file
   */
  QString mimetype() const;

  /**
   * Returns the mimetype of the file item.
   * If _determineMimeTypeOnDemand was used in the constructor, this will determine
   * the mimetype first.
   * @return the mime type
   */
  KMimeType::Ptr determineMimeType();

  /**
   * Returns the currently known mimetype of the file item.
   * This will not try to determine the mimetype if unknown.
   * @return the known mime type
   */
  KMimeType::Ptr mimeTypePtr() const { return m_pMimeType; }

  bool isMimeTypeKnown() const;
  /**
   * Returns the descriptive comment for this mime type, or
   * the mime type itself if none is present.
   * @return the mime type description, or the mime type itself
   */
  QString mimeComment();

  /**
   * Returns the full path name to the icon that represents
   * this mime type.
   * @return iconName the name of the file's icon
   */
  QString iconName();

  /**
   * Returns a pixmap representing the file.
   * @param _size Size for the pixmap in pixels. Zero will return the
   * globally configured default size.
   * @param _state The state of the icon: KIcon::DefaultState,
   * KIcon::ActiveState or KIcon::DisabledState.
   * @return the pixmap
   */
  QPixmap pixmap( int _size, int _state=0 ) const;

  /**
   * Returns the overlays (bitfield of KIcon::*Overlay flags) that are used
   * for this item's pixmap. Overlays are used to show for example, whether
   * a file can be modified.
   * @return the overlays of the pixmap
   */
  int overlays() const;

  /**
   * Returns the string to be displayed in the statusbar,
   * e.g. when the mouse is over this item
   * @return the status bar information
   */
  QString getStatusBarInfo();

  /**
   * Returns the string to be displayed in the tool tip when the mouse
   * is over this item. This may load a plugin to determine additional
   * information specific to the mimetype of the file.
   *
   * @param maxcount the maximum number of entries shown
   * @return the tool tip string
   */
  QString getToolTipText(int maxcount = 6);

  /**
   * Returns true if files can be dropped over this item.
   * Contrary to popular belief, not only dirs will return true :)
   * Executables, .desktop files, will do so as well.
   * @return true if you can drop files over the item
   */
  bool acceptsDrops( );

  /**
   * Let's "KRun" this file !
   * (e.g. when file is clicked or double-clicked or return is pressed)
   */
  void run();

  /**
   * Returns the UDS entry. Used by the tree view to access all details
   * by position.
   * @return the UDS entry
   */
  const KIO::UDSEntry & entry() const { return m_entry; }

  /**
   * Used when updating a directory. marked == seen when refreshing.
   * @return true if the file item is marked
   */
  bool isMarked() const { return m_bMarked; }
  /**
   * Marks the item.
   * @see isMarked()
   */
  void mark() { m_bMarked = true; }
  /**
   * Unmarks the item.
   * @see isMarked()
   */
  void unmark() { m_bMarked = false; }

  /**
   * Somewhat like a comparison operator, but more explicit.
   * @param item the item to compare
   * @return true if all values are equal
   */
  bool cmp( const KFileItem & item );

  /**
   * This allows to associate some "extra" data to a KFileItem. As one
   * KFileItem can be used by several objects (often views) which all need
   * to add some data, you have to use a key to reference your extra data
   * within the KFileItem.
   *
   * That way a KFileItem can hold and provide access to all those views
   * separately.
   *
   * I.e. a KFileIconView that associates a KFileIconViewItem (an item suitable
   * for use with QIconView) does
   *
   * <pre>
   * kfileItem->setExtraData( this, iconViewItem );
   * </pre>
   *
   * and can later access the iconViewItem by doing
   *
   * <pre>
   * KFileIconViewItem *iconViewItem = static_cast<KFileIconViewItem*>( kfileItem->extraData( this ));
   * </pre>
   *
   * This is usually more efficient then having every view associate data to
   * items by using a separate QDict or QMap.
   *
   * Note: you have to remove and destroy the data you associated yourself
   * when you don't need it anymore!
   *
   * @param key the key of the extra data
   * @param value the value of the extra data
   * @see #extraData
   * @see #removeExtraData
   */
  virtual void setExtraData( const void *key, void *value );

  /**
   * Retrieves the extra data with the given @p key.
   * @param key the key of the extra data
   * @return the extra data associated to an item with @p key via
   *         @ref setExtraData. 0L if nothing was associated with @p key.
   * @see #extraData
   */
  virtual const void * extraData( const void *key ) const;

  /**
   * Retrieves the extra data with the given @p key.
   * @param key the key of the extra data
   * @return the extra data associated to an item with @p key via
   *         @ref setExtraData. 0L if nothing was associated with @p key.
   * @see #extraData
   */
  virtual void * extraData( const void *key );

  /**
   * Removes the extra data associated with an item via @p key.
   * @param key the key of the extra data to remove
   */
  virtual void removeExtraData( const void *key );

  /**
   * Sets the metainfo of this item to @p info.
   * @param info the new meta info
   */
  void setMetaInfo( const KFileMetaInfo & info );

  /**
   * Returns the metainfo of this item.
   * @param autoget if true, the metainfo will automatically be created
   * @param what ignored
   */
  const KFileMetaInfo & metaInfo(bool autoget = true,
                                 int what = KFileMetaInfo::Fastest) const;

  /**
   * Somewhat like an assignment operator, but more explicit.
   * Note: extra-data set with @ref setExtraData() is not copied, so be careful
   * what you do!
   *
   * I.e. KDirLister uses it to update existing items from a fresh item.
   * @param item the item to copy
   */
  void assign( const KFileItem & item );


  /////////////

protected:
  /**
   * Computes the text, mode, and mimetype from the UDSEntry
   * Called by constructor, but can be called again later
   */
  void init( bool _determineMimeTypeOnDemand );

  /**
   * Parses the given permission set and provides it for @ref access()
   */
  QString parsePermissions( mode_t perm ) const;

private:
  /**
   * We keep a copy of the UDSEntry since we need it for @ref #getStatusBarInfo
   */
  KIO::UDSEntry m_entry;
  /**
   * The url of the file
   */
  KURL m_url;

  /**
   * The text for this item, i.e. the file name without path,
   */
  QString m_strName;

  /**
   * The text for this item, i.e. the file name without path, decoded
   * ('%%' becomes '%', '%2F' becomes '/')
   */
  QString m_strText;

  /**
   * the user and group assigned to the file.
   */
  mutable QString m_user, m_group;

  /**
   * The filename in lower case (to speed up sorting)
   */
  mutable QString m_strLowerCaseName;

  /**
   * The mimetype of the file
   */
  KMimeType::Ptr m_pMimeType;

  /**
   * The file mode
   */
  mode_t m_fileMode;
  /**
   * The permissions
   */
  mode_t m_permissions;

  /**
   * Marked : see @ref #mark()
   */
  bool m_bMarked:1;
  /**
   * Whether the file is a link
   */
  bool m_bLink:1;
  /**
   * True if local file
   */
  bool m_bIsLocalURL:1;

  bool m_bMimeTypeKnown:1;

   // For special case like link to dirs over FTP
  QString m_guessedMimeType;
  mutable QString m_access;
  QMap<const void*, void*> m_extra;
  mutable KFileMetaInfo m_metaInfo;

  enum { Modification = 0, Access = 1, Creation = 2, NumFlags = 3 };
  mutable time_t m_time[3];
  mutable KIO::filesize_t m_size;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KFileItemPrivate;
  KFileItemPrivate * d;
};

/**
 * List of KFileItems
 */
typedef QPtrList<KFileItem> KFileItemList;

/**
 * Iterator for KFileItemList
 */
typedef QPtrListIterator<KFileItem> KFileItemListIterator;

#endif
