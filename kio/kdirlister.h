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
 */
class KDirLister : public QObject
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
   * Update the currently displayed directory
   * The current implementation calls it automatically for
   * local files, using KDirWatch, but it might be useful to force an
   * update manually.
   */
  virtual void updateDirectory( const KURL& dir );

  /**
   * Changes the "is viewing dot files" setting.
   * Calls updateDirectory() if setting changed
   */
  virtual void setShowingDotFiles( bool _showDotFiles );

  /**
   * Find an item
   * @param _url the item URL
   * @return the pointer to the KFileItem
   **/
  KFileItem* find( const QString& _url ); // why not a KURL ?

  /**
   * @return the list of file items currently displayed
   */
  QList<KFileItem> & items() { return m_lstFileItems; }
  /**
   * @return the file item for url() itself (".")
   */
  KFileItem * rootItem() { return m_rootFileItem; }

  KIO::ListJob * job() { return m_job; }

  /**
   * Call this with @p dirsOnly = true to list only directories
   */
  void setDirOnlyMode( bool dirsOnly ) { m_bDirOnlyMode = dirsOnly; }

  /**
   * @return true if setDirOnlyMode(true) was called
   */
  bool dirOnlyMode() { return m_bDirOnlyMode; }

  /**
   * Set a name filter to only list items matching this name, e.g. "*.cpp".
   *
   * You can set more than one filter by separating them with whitespace, e.g
   * "*.cpp *.h".
   * Call setNameFilter( QString::null ) to disable filtering.
   *
   * @see #matchesFilter
   */
  void setNameFilter(const QString&);

  /**
   * @returns true if @param name matches a filter in the list,
   * otherwise fale.
   * @see #setNameFilter
   */
  bool matchesFilter( const QString& name ) const;

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
  /** Signal new items */
  void newItems( const KFileItemList & items );
  /** Signal a item to remove */
  void deleteItem( KFileItem * _fileItem );
  /**
   * Signal an item to refresh (its mimetype/icon/name has changed)
   * Note: KFileItem::refresh has already been called on those items.
   */
  void refreshItems( const KFileItemList & items );

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
   */
  virtual KFileItem * createFileItem( const KIO::UDSEntry&, const KURL&,
				      bool determineMimeTypeOnDemand,
				      bool urlIsDirectory = false );

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
  virtual bool matchesFilter( const KFileItem * );

  /**
   * Unregister dirs from kdirwatch and clear list of dirs
   */
  void forgetDirs();

  /**
   * Delete unmarked items, as it says on the tin
   */
  void deleteUnmarkedItems();

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

