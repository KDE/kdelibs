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

/*
 * A KFileItem is a generic class to handle a file, local or remote.
 * In particular, it makes it easier to handle the result of KIO::listDir.
 * (UDSEntry isn't very friendly to use)
 * It includes many file attributes such as mimetype, icon, text, mode, link...
 */
class KFileItem
{
public:
  /**
   * Create an item representing a file, from an UDSEntry (see kio/global.h)
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
   * Create an item representing a file, from all the necessary info for it
   * @param _mode the file mode (according to stat())
   * Set to -1 if unknown. For local files, KFileItem will use stat().
   * @param _mode the mode (S_IFDIR...)
   * @param _permissions the access permissions
   * If you set both the mode and the permissions, you save a ::stat() for
   * local files
   * Set to -1 if you don't know the mode or the permission.
   * @param _url the file url
   *
   * @param _determineMimeTypeOnDemand specify if the mimetype of the given URL
   *       should be determined immediately or on demand
   */
  KFileItem( mode_t _mode, mode_t _permissions, const KURL& _url,
             bool _determineMimeTypeOnDemand = false );

  /**
   * Create an item representing a file, for which the mimetype is already known
   * @param url the file url
   * @param mimeType the name of the file's mimetype
   * @param mode the mode (S_IFDIR...)
   */
  KFileItem( const KURL &url, const QString &mimeType, mode_t mode );

  /**
   * Copy constructor
   */
  KFileItem( const KFileItem &item );

  /**
   * Destructor
   */
  virtual ~KFileItem();

  /**
   * Re-read information (currently only permissions and mimetype)
   * This is called when the _file_ changes
   */
  void refresh();

  /**
   * Re-read mimetype information
   * This is called when the mimetype database changes
   */
  void refreshMimeType();

  /**
   * @return the url of the file
   */
  const KURL & url() const { return m_url; }

  /**
   * Set the item's URL. Do not call unless you know what you are doing!
   * (used for example when an item got renamed)
   */
  void setURL( const KURL &url );

  /**
   * @return the permissions of the file (stat.st_mode containing only permissions)
   */
  mode_t permissions() const { return m_permissions; }

  /**
   * @return the file type (stat.st_mode containing only S_IFDIR, S_IFLNK, ...)
   */
  mode_t mode() const { return m_fileMode; }

  /**
   * @return the owner of the file.
   */
  QString user() const;

  /**
   * @return the group of the file.
   */
  QString group() const;

  /**
   * @returns true if this item represents a link in the UNIX sense of
   * a link.
   */
  bool isLink() const { return m_bLink; }

  /**
   * @returns true if this item represents a directory
   */
  bool isDir() const;

  /**
   * @returns true if the file can be read - more precisely,
   * returns false if we know for sure it can't. In some cases
   * (remote files), we may return true even though it can't be read.
   */
  bool isReadable() const;

  /**
   * @return the link destination if isLink() == true
   */
  QString linkDest() const;

  /**
   * @return the size of the file, if known
   */
  KIO::filesize_t size() const;

  /**
   * @param which UDS_MODIFICATION_TIME, UDS_ACCESS_TIME or even UDS_CREATION_TIME
   * @return the time asked for, (time_t)0 if not available
   */
  time_t time( unsigned int which ) const;

  /**
   * @return true if the file is a local file
   */
  bool isLocalFile() const { return m_bIsLocalURL; }

  /**
   * @return the text of the file item
   * It's not exactly the filename since some decoding happens ('%2F'->'/')
   */
  const QString& text() const { return m_strText; }

  /**
   * @returns the name of the file item (without a path)
   * Similar to @ref text(), but unencoded, i.e. the original name
   * If @p lowerCase is true, the name will be returned in lower case,
   * which is useful to speed up sorting by name, case insensitively.
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
   * @return the mimetype of the file item
   */
  QString mimetype() const;

  /**
   * @return the mimetype of the file item
   * If determineMimeTypeOnDemand was used, this will determine the mimetype first.
   */
  KMimeType::Ptr determineMimeType();
  /**
   * @return the currently-known mimetype of the file item
   * This will not try to determine the mimetype if unknown.
   */
  KMimeType::Ptr mimeTypePtr() { return m_pMimeType; }

  bool isMimeTypeKnown() const;

  /**
   * @return the descriptive comment for this mime type, or
   *         the mime type itself if none is present.
   */
  QString mimeComment();

  /**
   * @return the full path name to the icon that represents
   *         this mime type.
   */
  QString iconName();

  /**
   * Returns a pixmap representing the file
   * @param _size Size for the pixmap in pixels. Zero will return the
   * globally configured default size.
   * @param _state The state of the icon: KIcon::DefaultState,
   * KIcon::ActiveState or KIcon::DisabledState.
   * @return the pixmap
   */
  QPixmap pixmap( int _size, int _state=0 ) const;

  /**
   * @return the string to be displayed in the statusbar e.g. when the mouse
   *         is over this item
   */
  QString getStatusBarInfo();

  /**
   * @return true if files can be dropped over this item
   * Contrary to popular belief, not only dirs will return true :)
   * Executables, .desktop files, will do so as well.
   */
  bool acceptsDrops( );

  /**
   * Let's "KRun" this file !
   * (e.g. when file is clicked or double-clicked or return is pressed)
   */
  void run();

  /**
   * Give the file a "hidden" flag, so that a view doesn't show it.
   * E.g. a filefilter sets this flag if this item matches *.cpp or not.
   * @see #isHidden
   */
    //  void setHidden( bool b ) { m_bHidden = b; }

  /**
   * @returns this item's hidden flag. True if a view shall not show it, false
   * otherwise.
   * @see #setHidden
   */
    //  bool isHidden() const { return m_bHidden; }

  /**
   * @return the UDS entry. Used by the tree view to access all details
   * by position.
   */
  const KIO::UDSEntry & entry() const { return m_entry; }

  // Used when updating a directory - marked == seen when refreshing
  bool isMarked() const { return m_bMarked; }
  void mark() { m_bMarked = true; }
  void unmark() { m_bMarked = false; }

  /**
   * Somewhat like a comparison operator, but more explicit
   */
  bool cmp( const KFileItem & item );

  /**
   * Somewhat like an assignment operator, but more explicit
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
   * We keep a copy of the UDSEntry since we need it for @ref #getStatusBarInfo
   */
  KIO::UDSEntry m_entry;
  /**
   * The url of the file
   */
  KURL m_url;
  /**
   * True if local file
   */
  bool m_bIsLocalURL;

  /**
   * The text for this item, i.e. the file name without path,
   */
  QString m_strName;

    /**
   * The text for this item, i.e. the file name without path, encoded
   */
  QString m_strText;

  /**
   * The file mode
   */
  mode_t m_fileMode;
  /**
   * The permissions
   */
  mode_t m_permissions;

  /**
   * the user and group assigned to the file.
   */
  mutable QString m_user, m_group;

  /**
   * Whether the file is a link
   */
  bool m_bLink;
  /**
   * The mimetype of the file
   */
  KMimeType::Ptr m_pMimeType;

  /**
   * The filename in lower case (to speed up sorting)
   */
  mutable QString m_strLowerCaseName;

private:
  /**
   * Marked : see @ref #mark()
   */
  bool m_bMarked;

  /**
   * Hidden : see @ref #setHidden(), @ref #isHidden()
   */
  // bool m_bHidden;

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
