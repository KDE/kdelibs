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

#ifndef kdirlister_h
#define kdirlister_h

#include "kfileitem.h"
#include "kdirnotify.h"

#include <qtimer.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qregexp.h>

#include <kurl.h>
#include <kio/global.h>

namespace KIO { class Job; class ListJob; }

/**
 * The dir lister deals with the kiojob used to list and update a directory,
 * handles the timer, and has signals for the user of this class (e.g.
 * konqueror view or kdesktop) to create/destroy its items when asked.
 *
 * This class is independent from the graphical representation of the dir
 * (icon container, tree view, ...) and it stores the items (as KFileItems).
 *
 * Typical usage :
 * Create an instance,
 * Connect to at least update, clear, newItem, and deleteItem
 * Call openURL - the signals will be called
 * Reuse the instance when opening a new url (openURL)
 * Destroy the instance when not needed anymore (usually destructor)
 *
 * Advanced usage : call openURL with _keep = true to list directories
 * without forgetting the ones previously read (e.g. for a tree view)
 *
 *@short Helper class for the kiojob used to list and update a directory.
 */
class KDirLister : public QObject, public KDirNotify
{
  Q_OBJECT
public:
  /**
   * Create a directory lister
   */
  KDirLister( bool _delayedMimeTypes = false );
  /**
   * Destroy the directory lister
   */
  virtual ~KDirLister();

  /**
   * Run the directory lister on the given url
   * @param _url the directory URL
   * @param _showDotFiles whether to return the "hidden" files
   * @param _keep if true the previous directories aren't forgotten
   * (they are still watched by kdirwatch and their items are kept in
   * m_lstFileItems)
   *
   * The @ref newItems() signal may be emitted more than once to supply you
   * with KFileItems, up until the signal @ref completed() is emitted
   * (and isFinished() returns true).
   */
  virtual void openURL( const KURL& _url, bool _showDotFiles, bool _keep = false );

  /**
   * Stop listing the current directory
   */
  virtual void stop();

  /**
   * @return the url used by this instance to list the files
   * It might be different from the one we gave, if there was a redirection.
   */
  virtual const KURL & url() const { return m_url; }

  /**
   * Sets @p url as the current url, forgetting any previous ones and stopping
   * any pending job. If @p url is malformed, the previous url will be kept
   * and false will be returned.
   *
   * Does _not_ start loading that url,
   */
  virtual bool setURL( const KURL& url );

  /**
   * Update @p url.
   * The current implementation calls it automatically for
   * local files, using KDirWatch (if autoUpdate() is true), but it might be
   * useful to force an update manually.
   */
  virtual void updateDirectory( const KURL& dir );

  /**
   * Convenience method. Starts loading the current directory, e.g. set via
   * @ref setURL(), if the URL is "dirty" -- otherwise the cached entries are
   * reused. The url is dirty when a new URL was set via setURL or the
   * nameFilter was changed.
   *
   * @see #setURLDirty
   */
  void listDirectory();

  /**
   * Sets the current URL "dirty", so it will be reloaded upon the next
   * @ref listDirectory() call.
   */
  void setURLDirty( bool dirty );

  /**
   * Enable/disable automatic directory updating, when a directory changes
   * (using KDirWatch).
   */
  void setAutoUpdate( bool enable );

  /**
   * @returns whether KDirWatch is used to automatically update directories.
   * enabled by default.
   */
  bool autoUpdate() const;

  /**
   * Changes the "is viewing dot files" setting.
   * Calls updateDirectory() if setting changed
   */
  virtual void setShowingDotFiles( bool _showDotFiles );

  /**
   * @returns whether dotfiles are shown
   */
  virtual bool showingDotFiles() const;

  /**
   * Find an item by its URL
   * @param _url the item URL
   * @returns the pointer to the KFileItem
   **/
  KFileItem* find( const KURL& _url ) const;

  /**
   * Find an item by its name
   * @param name the item name
   * @returns the pointer to the KFileItem
   **/
  KFileItem* findByName( const QString& name ) const;

  /**
   * @returns the list of file items. The list may be incomplete if
   * @ref isFinished() is false, i.e. it is still loading items.
   */
  QList<KFileItem> & items() { return m_lstFileItems; }

  /**
   * @return the file item for url() itself (".")
   */
  KFileItem * rootItem() const { return m_rootFileItem; }

  KIO::ListJob * job() const { return m_job; }

  /**
   * Call this with @p dirsOnly = true to list only directories
   */
  void setDirOnlyMode( bool dirsOnly ) { m_bDirOnlyMode = dirsOnly; }

  /**
   * @return true if setDirOnlyMode(true) was called
   */
  bool dirOnlyMode() const { return m_bDirOnlyMode; }

  /**
   * Set a name filter to only list items matching this name, e.g. "*.cpp".
   *
   * You can set more than one filter by separating them with whitespace, e.g
   * "*.cpp *.h".
   * Call setNameFilter( QString::null ) to disable filtering.
   * Note: the direcory is not automatically reloaded.
   *
   * @see #matchesFilter
   */
  void setNameFilter(const QString&);

  /**
   * @returns the current name filter, as set via @ref setNameFilter()
   */
  const QString& nameFilter() const;

  /**
   * @returns true if @p name matches a filter in the list,
   * otherwise fale.
   * @see #setNameFilter
   */
  bool matchesFilter( const QString& name ) const;

  /**
   * Notify that files have been added in @p directory
   * The receiver will list that directory again to find
   * the new items (since it needs more than just the names anyway).
   * Reimplemented from KDirNotify.
   */
  virtual void FilesAdded( const KURL & directory );

  /**
   * Notify that files have been deleted.
   * This call passes the exact urls of the deleted files
   * so that any view showing them can simply remove them
   * or be closed (if its current dir was deleted)
   * Reimplemented from KDirNotify.
   */
  virtual void FilesRemoved( const KURL::List & fileList );

  /**
   * Notify that files have been changed.
   * At the moment, this is only used for new icon, but it could be
   * used for size etc. as well.
   * Note: this is ASYNC so that it can be used with a broadcast
   */
  virtual void FilesChanged( const KURL::List & fileList );

  virtual void FileRenamed( const KURL &src, const KURL &dst );

  /**
   * Returns true if no io operation is currently in progress.
   */
  bool isFinished() const { return (m_job == 0); }

signals:
  /**
   * Tell the view that we started to list _url.
   * The view knows that openURL should start it, so it might seem useless,
   * but the view also needs to know when an automatic update happens.
   */
  void started( const QString& _url );

  /** Tell the view that listing is finished */
  void completed();
  /** Tell the view that user canceled the listing */
  void canceled();

  /** Signal a redirection */
  void redirection( const KURL & url );

  /** Clear all items */
  void clear();
  /** Signal new items, @p complete is true when the directory loading has
   *  finished */
  void newItems( const KFileItemList & items );
  /** Signal an item to remove */
  void deleteItem( KFileItem * _fileItem );
  /**
   * Signal an item to refresh (its mimetype/icon/name has changed)
   * Note: KFileItem::refresh has already been called on those items.
   */
  void refreshItems( const KFileItemList & items );
  /**
   * Instruct the view to close itself, since the dir was just deleted.
   */
  void closeView();

protected slots:
  // internal slots used by the directory lister (connected to the job)
  void slotResult( KIO::Job * );
  void slotEntries( KIO::Job*, const KIO::UDSEntryList& );
  void slotUpdateResult( KIO::Job * );
  void slotUpdateEntries( KIO::Job*, const KIO::UDSEntryList& );
  void slotRedirection( KIO::Job *, const KURL & url );

  // internal slots connected to KDirWatch
  void slotDirectoryDirty( const QString& _dir );
  void slotFileDirty( const QString& _file );

  void slotURLDirty( const KURL& dir );

protected:

  /**
   * called to create a KFileItem - you may subclass and reimplement this
   * method if you use "special KFileItems", i.e. a subclass like KonqFileItem
   * Must return a valid KFileItem
   * @param url the URL of the DIRECTORY where this item is.
   */
  virtual KFileItem * createFileItem( const KIO::UDSEntry&, const KURL&url,
                                      bool determineMimeTypeOnDemand );

  /**
   * Called for every item after @ref #createFileItem().
   * @returns true if the item is "ok".
   * @returns false if the item shall not be shown in a view, e.g.
   * files not matching a pattern *.cpp (@ref KFileItem::isHidden())
   * You may reimplement this method in a subclass to implement your own
   * filtering.
   * The default implementation filters out ".." and everything not matching
   * the name filter(s)
   * @see #matchesFilter
   * @see #setNameFilter
   */
  virtual bool matchesFilter( const KFileItem * ) const;

  /**
   * Unregister dirs from kdirwatch and clear list of dirs
   */
  void forgetDirs();

  /**
   * Delete unmarked items, as it says on the tin
   */
  void deleteUnmarkedItems();

  /**
   * Checks if a url is malformed or not and displays an error message if it
   * is. Returns true if it is valid, otherwise false.
   */
  bool validURL( const KURL& ) const;

  /**
   * The url that we used to list (can be different in case of redirect)
   */
  KURL m_url;

  KIO::ListJob * m_job;

  /**
   * The internal storage of file items
   */
  QList<KFileItem> m_lstFileItems;
  /**
   * File Item for m_url itself (".")
   */
  KFileItem * m_rootFileItem;

  /**
   * List of dirs handled by this instance. Same as m_url if only one dir
   * But for a tree view, it contains all the dirs shown
   * (Used to unregister from kdirwatch)
   */
  KURL::List m_lstDirs;

  bool m_isShowingDotFiles;
  bool m_bComplete;

  /** Keep entries found - used by slotUpdate* */
  QValueList<KIO::UDSEntry> m_buffer;

  /** List only directories */
  bool m_bDirOnlyMode;

  bool m_bDelayedMimeTypes;

  /** a list of file-filters */
  QList<QRegExp> m_lstFilters;

  class KDirListerPrivate;
  KDirListerPrivate * d;
};

#endif

