/* This file is part of the KDE project
   Copyright (C) 2002 Michael Brade <brade@kde.org>

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

#ifndef kdirlister_p_h
#define kdirlister_p_h

#include "kfileitem.h"

#include <qmap.h>
#include <qdict.h>
#include <qcache.h>
#include <qwidget.h>

#include <kurl.h>
#include <kio/global.h>
#include <kdirwatch.h>

class KDirLister;
namespace KIO { class Job; class ListJob; }


class KDirLister::KDirListerPrivate
{
public:
  KDirListerPrivate()
  {
    numJobs = 0;
    complete = false;

    autoUpdate = false;
    isShowingDotFiles = false;
    dirOnlyMode = false;

    autoErrorHandling = false;
    errorParent = 0;

    delayedMimeTypes = false;

    rootFileItem = 0;
    lstNewItems = 0;
    lstRefreshItems = 0;
    lstMimeFilteredItems = 0;
    
    changes = NONE;
    
    window = 0;
    
    lstFilters.setAutoDelete( true );
    oldFilters.setAutoDelete( true );
  }

  /**
   * List of dirs handled by this dirlister. The first entry is the base URL.
   * For a tree view, it contains all the dirs shown.
   */
  KURL::List lstDirs;

  // toplevel URL
  KURL url;

  unsigned short numJobs;

  bool complete;

  bool autoUpdate;
  bool isShowingDotFiles;
  bool dirOnlyMode;

  bool autoErrorHandling;
  QWidget *errorParent;

  bool delayedMimeTypes;

  struct JobData {
    long unsigned int percent, speed;
    KIO::filesize_t processedSize, totalSize;
  };

  QMap< KIO::ListJob*, JobData > jobData;

  // file item for the root itself (".")
  KFileItem *rootFileItem;

  KFileItemList *lstNewItems, *lstRefreshItems, *lstMimeFilteredItems;

  int changes;
 
  QWidget *window; // Main window ths lister is associated with

  QString nameFilter;
  QPtrList<QRegExp> lstFilters, oldFilters;
  QStringList mimeFilter, oldMimeFilter;
  QStringList mimeExcludeFilter, oldMimeExcludeFilter;
};

/**
 * Design of the cache:
 * There is a single KDirListerCache for the whole process.
 * It holds all the items used by the dir listers (itemsInUse)
 * as well as a cache of the recently used items (itemsCached).
 * Those items are grouped by directory (a DirItem represents a whole directory).
 *
 * KDirListerCache also runs all the jobs for listing directories, whether they are for
 * normal listing or for updates.
 * For faster lookups, it also stores two dicts:
 * a URL -> dirlister holding that URL (urlsCurrentlyHeld)
 * a URL -> dirlister currently listing that URL (urlsCurrentlyListed)
 */
class KDirListerCache : public QObject, KDirNotify
{
  Q_OBJECT
public:
  KDirListerCache( int maxCount = 10 );
  ~KDirListerCache();

  void listDir( KDirLister* lister, const KURL &_url, bool _keep, bool _reload );

  // stop all running jobs for lister
  void stop( KDirLister *lister );
  // stop just the job listing url for lister
  void stop( KDirLister *lister, const KURL &_url );

  void setAutoUpdate( KDirLister *lister, bool enable );

  void forgetDirs( KDirLister *lister );
  void forgetDirs( KDirLister *lister, const KURL &_url, bool notify );

  void updateDirectory( const KURL &_dir );

  KFileItemList* itemsForDir( const KURL &_dir ) const;

  KFileItem* findByName( const KDirLister *lister, const QString &_name ) const;
  // if lister is set, it is checked that the url is held by the lister
  KFileItem* findByURL( const KDirLister *lister, const KURL &_url ) const;

  /**
   * Notify that files have been added in @p directory
   * The receiver will list that directory again to find
   * the new items (since it needs more than just the names anyway).
   * Reimplemented from KDirNotify.
   */
  virtual void FilesAdded( const KURL &directory );

  /**
   * Notify that files have been deleted.
   * This call passes the exact urls of the deleted files
   * so that any view showing them can simply remove them
   * or be closed (if its current dir was deleted)
   * Reimplemented from KDirNotify.
   */
  virtual void FilesRemoved( const KURL::List &fileList );

  /**
   * Notify that files have been changed.
   * At the moment, this is only used for new icon, but it could be
   * used for size etc. as well.
   * Note: this is ASYNC so that it can be used with a broadcast
   */
  virtual void FilesChanged( const KURL::List &fileList );
  virtual void FileRenamed( const KURL &src, const KURL &dst );

  static KDirListerCache* self();

private slots:
  void slotFileDirty( const QString &_file );
  void slotFileCreated( const QString &_file );
  void slotFileDeleted( const QString &_file );
  void slotURLDirty( const KURL &_dir );

  void slotEntries( KIO::Job *job, const KIO::UDSEntryList &entries );
  void slotResult( KIO::Job *j );
  void slotRedirection( KIO::Job *job, const KURL &url );

  void slotUpdateEntries( KIO::Job *job, const KIO::UDSEntryList &entries );
  void slotUpdateResult( KIO::Job *job );

private:
  bool killJob( const QString &_url );

  // when there were items deleted from the filesystem all the listers holding
  // the parent directory need to be notified, the unmarked items have to be deleted
  // and removed from the cache including all the childs.
  void deleteUnmarkedItems( QPtrList<KDirLister> *, KFileItemList * );
  void processPendingUpdates();
  // common for slotRedirection and FileRenamed
  void renameDir( const KURL &oldUrl, const KURL &url );
  // common for deleteUnmarkedItems and FilesRemoved
  void deleteDir( const KURL& dirUrl );
  // remove directory from cache (itemsCached), including all child dirs
  void removeDirFromCache( const KURL& dir );
  // helper for renameDir
  void emitRedirections( const KURL &oldUrl, const KURL &url );
  void emitRefreshItem( KFileItem* fileitem );
#ifndef NDEBUG
  void printDebug();
#endif

  struct DirItem
  {
    DirItem( const KURL &dir )
      : url(dir), rootItem(0), lstItems(new KFileItemList)
    {
      autoUpdates = 0;
      complete = false;
      lstItems->setAutoDelete( true );
    }

    ~DirItem()
    {
      if ( autoUpdates && KDirWatch::exists() )
        kdirwatch->removeDir( url.path() );

      delete rootItem;
      delete lstItems;
    }

    void incAutoUpdate()
    {
      if ( url.isLocalFile() && autoUpdates++ == 0 )
        kdirwatch->addDir( url.path() );
    }

    void decAutoUpdate()
    {
      if ( url.isLocalFile() )
      {
        if ( --autoUpdates == 0 )
          kdirwatch->removeDir( url.path() );
        else if ( autoUpdates < 0 )
          autoUpdates = 0;
      }
    }

    // number of KDirListers using autoUpdate for this dir
    short autoUpdates;

    // this directory is up-to-date
    bool complete;

    // the complete url of this directory
    KURL url;

    // KFileItem representing the root of this directory.
    // Remember that this is optional. FTP sites don't return '.' in
    // the list, so they give no root item
    KFileItem* rootItem;
    KFileItemList* lstItems;
  };

  static const unsigned short MAX_JOBS_PER_LISTER;
  QMap< KIO::ListJob *, KIO::UDSEntryList > jobs;

  // an item is a complete directory
  QDict<DirItem> itemsInUse;
  QCache<DirItem> itemsCached;

  // A lister can be EITHER in urlsCurrentlyListed OR urlsCurrentlyHeld but NOT
  // in both at the same time.
  //     On the other hand there can be some listers in urlsCurrentlyHeld
  // and some in urlsCurrentlyListed for the same url!
  // Or differently said, there can be an entry for url in urlsCurrentlyListed
  // and urlsCurrentlyHeld. This happens if more listers are requesting url at
  // the same time and one lister was stopped during the listing of files.

  // saves all urls that are currently being listed and maps them
  // to their KDirListers
  QDict< QPtrList<KDirLister> > urlsCurrentlyListed;

  // saves all KDirListers that are just holding url
  QDict< QPtrList<KDirLister> > urlsCurrentlyHeld;

  static KDirListerCache* s_pSelf;
};

const unsigned short KDirListerCache::MAX_JOBS_PER_LISTER = 5;

#define s_pCache KDirListerCache::self()

#endif
