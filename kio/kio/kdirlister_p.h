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

#include <kurl.h>
#include <kio/global.h>

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

  QString nameFilter;
  QPtrList<QRegExp> lstFilters, oldFilters;
  QStringList mimeFilter, oldMimeFilter;
};


class KDirListerCache : public QObject, KDirNotify
{
  Q_OBJECT
public:
  KDirListerCache( int maxCount = 10 );
  ~KDirListerCache();
  
  void listDir( KDirLister* lister, const KURL &_url, bool _keep, bool _reload );
  
  /**
   * Stop all running jobs for lister 
   */
  void stop( KDirLister *lister );
  void stop( KDirLister *lister, const KURL &_url );

  void setAutoUpdate( KDirLister *lister, bool enable );

  void forgetDirs( KDirLister *lister );
  void forgetDirs( KDirLister *lister, const KURL &_url );

  void updateDirectory( const KURL &_dir );

  KFileItemList* items( const KURL &_dir ) const;

  KFileItem* findByName( const KDirLister *lister, const QString &_name ) const;
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
  void slotDirectoryDirty( const QString &_dir );
  void slotURLDirty( const KURL &_dir );

  void slotEntries( KIO::Job *job, const KIO::UDSEntryList &entries );
  void slotResult( KIO::Job *j );
  void slotRedirection( KIO::Job *job, const KURL &url );

  void slotUpdateEntries( KIO::Job *job, const KIO::UDSEntryList &entries );
  void slotUpdateResult( KIO::Job *job );
  
private:
  bool killJob( const QString &_url );
  void deleteUnmarkedItems( QPtrList<KDirLister> *, KFileItemList *, bool really );
  void processPendingUpdates();
  void forgetDirInternal( KDirLister *lister, const KURL &_url );

  struct DirItem
  {
    DirItem() : rootItem(0), lstItems(new KFileItemList)
    {
      count = 1;
      autoUpdates = 0;
      complete = false;
      lstItems->setAutoDelete( true );
    }

    ~DirItem()
    {
      delete rootItem;
      delete lstItems;
    }

    // number of KDirListers currently holding this dir
    unsigned short count;

    // number of KDirListers using autoUpdate for this dir
    unsigned short autoUpdates;

    bool complete;

    KFileItem* rootItem;
    KFileItemList* lstItems;
  };

  static const unsigned short MAX_JOBS_PER_LISTER;
  QMap< KIO::ListJob *, KIO::UDSEntryList > jobs;

  // an item is a complete directory
  QDict<DirItem> itemsInUse;
  QCache<DirItem> itemsCached;

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
