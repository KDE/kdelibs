/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>
                 2001, 2002 Michael Brade <brade@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef kdirlister_h
#define kdirlister_h

#include "kfileitem.h"
#include "kdirnotify.h"

#include <qstring.h>
#include <qstringlist.h>

#include <kurl.h>

namespace KIO { class Job; class ListJob; }

/**
 * The dir lister deals with the kiojob used to list and update a directory
 * and has signals for the user of this class (e.g. konqueror view or
 * kdesktop) to create/destroy its items when asked.
 *
 * This class is independent from the graphical representation of the dir
 * (icon container, tree view, ...) and it stores the items (as KFileItems).
 *
 * Typical usage :
 * Create an instance.
 * Connect to at least update, clear, newItem, and deleteItem.
 * Call openURL - the signals will be called.
 * Reuse the instance when opening a new url (openURL).
 * Destroy the instance when not needed anymore (usually destructor).
 *
 * Advanced usage : call openURL with _keep = true to list directories
 * without forgetting the ones previously read (e.g. for a tree view)
 *
 * @short Helper class for the kiojob used to list and update a directory.
 */
class KDirLister : public QObject
{
  friend class KDirListerCache;

  Q_OBJECT
  Q_PROPERTY( bool autoUpdate READ autoUpdate WRITE setAutoUpdate )
  Q_PROPERTY( bool showingDotFiles READ showingDotFiles WRITE setShowingDotFiles )
  Q_PROPERTY( bool dirOnlyMode READ dirOnlyMode WRITE setDirOnlyMode )
  Q_PROPERTY( bool autoErrorHandlingEnabled READ autoErrorHandlingEnabled )
  Q_PROPERTY( QString nameFilter READ nameFilter WRITE setNameFilter )
  Q_PROPERTY( QStringList mimeFilter READ mimeFilters WRITE setMimeFilter RESET clearMimeFilter )
public:
  /**
   * Create a directory lister.
   */
  KDirLister( bool _delayedMimeTypes = false );

  /**
   * Destroy the directory lister.
   */
  virtual ~KDirLister();

  /**
   * Run the directory lister on the given url.
   *
   * This method causes KDirLister to emit _all_ the items of @p _url, in any case.
   * Depending on @p _keep either clear() or clear( const KURL & ) will be
   * emitted first.
   *
   * @param _url          the directory URL.
   * @param _keep         if true the previous directories aren't forgotten
   *                      (they are still watched by kdirwatch and their items
   *                      are kept for this KDirLister). This is useful for e.g.
   *                      a treeview.
   * @param _reload       indicates wether to use the cache (false) or to reread the
   *                      directory from the disk.
   *                      Use only when opening a dir not yet listed by this lister
   *                      without using the cache. Otherwise use updateDirectory.
   *
   * The @ref newItems() signal may be emitted more than once to supply you
   * with KFileItems, up until the signal @ref completed() is emitted
   * (and @ref isFinished() returns true).
   */
  virtual bool openURL( const KURL& _url, bool _keep = false, bool _reload = false );

  /**
   * Stop listing all directories currently being listed.
   *
   * Emits @ref canceled() if there was at least one job running.
   * Emits @ref canceled( const KURL& ) for each stopped job if
   * there are at least two dirctories being watched by KDirLister.
   */
  virtual void stop();

  /**
   * Stop listing the given directory.
   *
   * Emits @ref canceled() if the killed job was the last running one.
   * Emits @ref canceled( const KURL& ) for the killed job if
   * there are at least two directories being watched by KDirLister.
   * No signal is emitted if there was no job running for @p _url.
   * @param _url the directory URL
   */
  virtual void stop( const KURL& _url );

  /**
   * @return whether KDirWatch is used to automatically update directories.
   * This is enabled by default.
   */
  bool autoUpdate() const;

  /**
   * Enable/disable automatic directory updating, when a directory changes
   * (using KDirWatch).
   */
  virtual void setAutoUpdate( bool enable );

  /**
   * Error handling.
   */
  bool autoErrorHandlingEnabled() const;
  void setAutoErrorHandlingEnabled( bool enable, QWidget* parent );

  /**
   * @return whether dotfiles are shown
   */
  bool showingDotFiles() const;

  /**
   * Changes the "is viewing dot files" setting.
   * Calls @ref updateDirectory() if setting changed
   */
  virtual void setShowingDotFiles( bool _showDotFiles );

  /**
   * @return true if setDirOnlyMode(true) was called
   */
  bool dirOnlyMode() const;

  /**
   * Call this with @p dirsOnly == true to list only directories
   */
  virtual void setDirOnlyMode( bool dirsOnly );

  /**
   * @return the url used by this instance to list the files, with _keep == true,
   *         this is the first url opened (in e.g. a treeview this is the root).
   * It might be different from the one given with @ref openURL() or @ref setURL()
   * if there was a redirection.
   */
  const KURL& url() const;

  /**
   * actually emit the changes made with setShowingDotFiles, setDirOnlyMode,
   * setNameFilter and setMimeFilter.
   */
  virtual void emitChanges();

  /**
   * Update the directory @p _dir. This method causes KDirLister to _only_ emit
   * the items of @p _dir that actually changed compared to the current state in the
   * cache and updates the cache.
   *
   * The current implementation calls updateDirectory automatically for
   * local files, using KDirWatch (if autoUpdate() is true), but it might be
   * useful to force an update manually.
   *
   * @param _dir the directory URL
   */
  virtual void updateDirectory( const KURL& _dir );

  /**
   * Returns true if no io operation is currently in progress.
   */
  bool isFinished() const;
  
  /**
   * @return the file item for url() itself (".")
   */
  KFileItem* rootItem() const;

  /**
   * Find an item by its URL
   * @param _url the item URL
   * @return the pointer to the KFileItem
   */
  virtual KFileItem* findByURL( const KURL& _url ) const;
#ifndef KDE_NO_COMPAT
  KFileItem* find( const KURL& _url ) const;
#endif

  /**
   * Find an item by its name
   * @param name the item name
   * @return the pointer to the KFileItem
   */
  virtual KFileItem* findByName( const QString& name ) const;

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
  virtual void setNameFilter( const QString& );

  /**
   * @return the current name filter, as set via @ref setNameFilter()
   */
  const QString& nameFilter() const;

  /**
   * Set mime-based filter to only list items matching the given mimetypes
   *
   * NOTE: setting the filter does not automatically reload direcory.
   * Also calling this function will not affect any named filter already set.
   *
   * @see #clearMimeFilter
   * @see #matchesMimeFilter
   *
   * @param a list of mime-types.
   */
  virtual void setMimeFilter( const QStringList& );

  /**
   * Set mime-based exclude filter to only list items not matching the given mimetypes
   *
   * NOTE: setting the filter does not automatically reload direcory.
   * Also calling this function will not affect any named filter already set.
   *
   * @see #clearMimeFilter
   * @see #matchesMimeFilter
   *
   * @param a list of mime-types.
   */
  void setMimeExcludeFilter(const QStringList &);


  /**
   * Clears the mime based filter.
   *
   * @see #setMimeFilter
   */
  virtual void clearMimeFilter();

  /**
   * @return the list of mime based filters, as set via @ref setMimeFilter().
   * Empty, when no mime filter is set.
   */
  const QStringList& mimeFilters() const;

  /**
   * @return true if @p name matches a filter in the list,
   * otherwise false.
   * @see #setNameFilter
   */
  bool matchesFilter( const QString& name ) const;

  /**
   * @return true if @p name matches a filter in the list,
   * otherwise false.
   * @see #setNameFilter.
   *
   * @param mime the mimetype to find in the filter list.
   */
  bool matchesMimeFilter( const QString& mime ) const;

signals:
  /**
   * Tell the view that we started to list _url. NOTE: this does _not_ imply that there
   * is really a job running! I.e. KDirLister::jobs() may return an empty list. In this case
   * the items are taken from the cache.
   *
   * The view knows that openURL should start it, so it might seem useless,
   * but the view also needs to know when an automatic update happens.
   */
  void started( const KURL& _url );

  /**
   * Tell the view that listing is finished. There are no jobs running anymore.
   */
  void completed();

  /**
   * Tell the view that the listing of the directory @p _url is finished.
   * There might be other running jobs left.
   * This signal is only emitted if KDirLister is watching more than one directory.
   * @param _url the directory URL
   */
  void completed( const KURL& _url );

  /**
   * Tell the view that the user canceled the listing. No running jobs are left.
   */
  void canceled();

  /**
   * Tell the view that the listing of the directory @p _url was canceled.
   * There might be other running jobs left.
   * This signal is only emitted if KDirLister is watching more than one directory.
   * @param _url the directory URL
   */
  void canceled( const KURL& _url );

  /**
   * Signal a redirection.
   * Only emitted if there's just one directory to list, i.e. most
   * probably _keep == false
   */
  void redirection( const KURL& _url );

  /**
   * Signal a redirection.
   */
  void redirection( const KURL& oldUrl, const KURL& newUrl );

  /**
   * Signal to clear all items.
   * It must always be connected to this signal to avoid doubled items!
   */
  void clear();

  /**
   * Signal to empty the directory _url.
   * It is only emitted if the lister is holding more than one directory.
   */
  void clear( const KURL& _url );

  /**
   * Signal new items.
   */
  void newItems( const KFileItemList& items );

  /**
   * Send a list of items filtered-out by mime-type.
   */
  void itemsFilteredByMime( const KFileItemList& items );

  /**
   * Signal an item to remove.
   *
   * ATTENTION: if @p _fileItem == @p rootItem() the directory this lister
   *            is holding was deleted and you HAVE to release especially the
   *            rootItem of this lister otherwise your app will CRASH!!
   *            The clear() signals have been emitted already.
   */
  void deleteItem( KFileItem *_fileItem );

  /**
   * Signal an item to refresh (its mimetype/icon/name has changed)
   * Note: KFileItem::refresh has already been called on those items.
   */
  void refreshItems( const KFileItemList& items );

  /**
   * Emitted to display information about running jobs.
   * Examples of message are "Resolving host", "Connecting to host...", etc.
   */
  void infoMessage( const QString& msg );
          
  /**
   * Progress signal showing the overall progress of the KDirLister.
   * This allows using a progress bar very easily. (see @ref KProgress)
   */
  void percent( int percent );
  
  /**
   * Emitted when we know the size of the jobs.
   */
  void totalSize( KIO::filesize_t size );
                                           
  /**
   * Regularly emitted to show the progress of this KDirLister. 
   */
  void processedSize( KIO::filesize_t size );
  
  /**
   * Emitted to display information about the speed of the jobs.
   */
  void speed( int bytes_per_second );

protected:
  /**
   * Called for every new item before emitting @ref newItems().
   * @return true if the item is "ok".
   * @return false if the item shall not be shown in a view, e.g.
   * files not matching a pattern *.cpp (@ref KFileItem::isHidden())
   * You may reimplement this method in a subclass to implement your own
   * filtering.
   * The default implementation filters out ".." and everything not matching
   * the name filter(s)
   * @see #matchesFilter
   * @see #setNameFilter
   */
  virtual bool matchesFilter( const KFileItem * ) const;

  virtual bool matchesMimeFilter( const KFileItem * ) const;

  /**
   * Called by the public matchesFilter/matchesMimeFilter to do the
   * actual filtering. Those methods may be reimplemented to customize
   * filtering.
   */
  virtual bool doNameFilter( const QString& name, const QPtrList<QRegExp>& filters ) const;
  virtual bool doMimeFilter( const QString& mime, const QStringList& filters ) const;
  bool doMimeExcludeFilter( const QString& mimeExclude, const QStringList& filters ) const;

  /**
   * Checks if an url is malformed or not and displays an error message
   * if it is and autoErrorHandling is set to true.
   * @return true if url is valid, otherwise false.
   */
  virtual bool validURL( const KURL& ) const;

  /** Reimplement to customize error handling */
  virtual void handleError( KIO::Job* );

protected:
  virtual void virtual_hook( int id, void* data );

private slots:
  void slotInfoMessage( KIO::Job *, const QString& );
  void slotPercent( KIO::Job *, unsigned long );
  void slotTotalSize( KIO::Job *, KIO::filesize_t );
  void slotProcessedSize( KIO::Job *, KIO::filesize_t );
  void slotSpeed( KIO::Job *, unsigned long );

  void slotJobToBeKilled( const KURL& );
  void slotClearState();

private:
  virtual void addNewItem( const KFileItem *item );
  virtual void addNewItems( const KFileItemList& items );
  virtual void addRefreshItem( const KFileItem *item );
  virtual void emitItems();
  virtual void emitDeleteItem( KFileItem *item );

  enum Changes {
    NONE=0, NAME_FILTER=1, MIME_FILTER=2, DOT_FILES=4, DIR_ONLY_MODE=8
  };

  class KDirListerPrivate;
  KDirListerPrivate *d;
};

#endif

