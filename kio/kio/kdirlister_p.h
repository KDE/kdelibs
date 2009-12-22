/* This file is part of the KDE project
   Copyright (C) 2002-2006 Michael Brade <brade@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef kdirlister_p_h
#define kdirlister_p_h

#include "kfileitem.h"

#include <QtCore/QMap>
#include <QtCore/QHash>
#include <QtCore/QCache>
#include <QtCore/QSet>
#include <QtCore/QTimer>
#include <QtGui/QWidget>

#include <kurl.h>
#include <kio/global.h>
#include <kdirwatch.h>

class KDirLister;
namespace KIO { class Job; class ListJob; }
class OrgKdeKDirNotifyInterface;
struct KDirListerCacheDirectoryData;

class KDirLister::Private
{
public:
  Private(KDirLister *parent)
      : m_parent(parent)
  {
    complete = false;

    autoUpdate = false;

    autoErrorHandling = false;
    errorParent = 0;

    delayedMimeTypes = false;

    rootFileItem = KFileItem();

    lstNewItems = 0;
    lstRefreshItems = 0;
    lstMimeFilteredItems = 0;
    lstRemoveItems = 0;

    hasPendingChanges = false;

    window = 0;
    m_cachedItemsJob = 0;
  }

    void _k_emitCachedItems(const KUrl&, bool, bool);
  void _k_slotInfoMessage( KJob*, const QString& );
  void _k_slotPercent( KJob*, unsigned long );
  void _k_slotTotalSize( KJob*, qulonglong );
  void _k_slotProcessedSize( KJob*, qulonglong );
  void _k_slotSpeed( KJob*, unsigned long );

  bool doMimeExcludeFilter( const QString& mimeExclude, const QStringList& filters ) const;
  void jobStarted( KIO::ListJob * );
  void connectJob( KIO::ListJob * );
  void jobDone( KIO::ListJob * );
  uint numJobs();
    void addNewItem(const KUrl& directoryUrl, const KFileItem& item);
    void addNewItems(const KUrl& directoryUrl, const KFileItemList& items);
    void addRefreshItem(const KUrl& directoryUrl, const KFileItem& oldItem, const KFileItem& item);
  void emitItems();
  void emitItemsDeleted(const KFileItemList &items);

    /**
     * Redirect this dirlister from oldUrl to newUrl.
     * @param keepItems if true, keep the fileitems (e.g. when renaming an existing dir);
     * if false, clear out everything (e.g. when redirecting during listing).
     */
    void redirect(const KUrl& oldUrl, const KUrl& newUrl, bool keepItems);

    /**
     * Should this item be visible according to the current filter settings?
     */
    bool isItemVisible(const KFileItem& item) const;

    void prepareForSettingsChange() {
        if (!hasPendingChanges) {
            hasPendingChanges = true;
            oldSettings = settings;
        }
    }

    void emitChanges();


  KDirLister *m_parent;

  /**
   * List of dirs handled by this dirlister. The first entry is the base URL.
   * For a tree view, it contains all the dirs shown.
   */
  KUrl::List lstDirs;

  // toplevel URL
  KUrl url;

  bool complete:1;

  bool autoUpdate:1;

  bool delayedMimeTypes:1;

    bool hasPendingChanges:1; // i.e. settings != oldSettings

  bool autoErrorHandling:2;
  QWidget *errorParent;

  struct JobData {
    long unsigned int percent, speed;
    KIO::filesize_t processedSize, totalSize;
  };

  QMap<KIO::ListJob *, JobData> jobData;

  // file item for the root itself (".")
  KFileItem rootFileItem;

    typedef QHash<KUrl, KFileItemList> NewItemsHash;
    NewItemsHash *lstNewItems;
  QList<QPair<KFileItem,KFileItem> > *lstRefreshItems;
  KFileItemList *lstMimeFilteredItems, *lstRemoveItems;

    QWidget *window; // Main window this lister is associated with
    class CachedItemsJob;
    CachedItemsJob* m_cachedItemsJob;

    QString nameFilter; // parsed into lstFilters

    struct FilterSettings {
        FilterSettings() : isShowingDotFiles(false), dirOnlyMode(false) {}
        bool isShowingDotFiles;
        bool dirOnlyMode;
        QList<QRegExp> lstFilters;
        QStringList mimeFilter;
        QStringList mimeExcludeFilter;
    };
    FilterSettings settings;
    FilterSettings oldSettings;

    friend class KDirListerCache;
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
 * For faster lookups, it also stores a hash table, which gives for a directory URL:
 * - the dirlisters holding that URL (listersCurrentlyHolding)
 * - the dirlisters currently listing that URL (listersCurrentlyListing)
 */
class KDirListerCache : public QObject
{
    Q_OBJECT
public:
    KDirListerCache(); // only called by K_GLOBAL_STATIC
    ~KDirListerCache();

    void updateDirectory( const KUrl& dir );

    KFileItem itemForUrl( const KUrl& url ) const;
    KFileItemList *itemsForDir(const KUrl& dir) const;

    bool listDir( KDirLister *lister, const KUrl& _url, bool _keep, bool _reload );

    // stop all running jobs for lister
    void stop( KDirLister *lister, bool silent = false );
    // stop just the job listing url for lister
    void stop( KDirLister *lister, const KUrl &_url, bool silent = false );

  void setAutoUpdate( KDirLister *lister, bool enable );

  void forgetDirs( KDirLister *lister );
  void forgetDirs( KDirLister *lister, const KUrl &_url, bool notify );

    KFileItem findByName( const KDirLister *lister, const QString &_name ) const;
    // findByUrl returns a pointer so that it's possible to modify the item.
    // See itemForUrl for the version that returns a readonly kfileitem.
    // @param lister can be 0. If set, it is checked that the url is held by the lister
    KFileItem *findByUrl(const KDirLister *lister, const KUrl &url) const;

    // Called by CachedItemsJob:
    // Emits those items, for this lister and this url
    void emitItemsFromCache(KDirLister* lister, const KFileItemList& lst, const KFileItem& rootItem,
                            const KUrl& _url, bool _reload, bool _emitCompleted);

public Q_SLOTS:
  /**
   * Notify that files have been added in @p directory
   * The receiver will list that directory again to find
   * the new items (since it needs more than just the names anyway).
   * Connected to the DBus signal from the KDirNotify interface.
   */
  void slotFilesAdded( const QString& urlDirectory );

  /**
   * Notify that files have been deleted.
   * This call passes the exact urls of the deleted files
   * so that any view showing them can simply remove them
   * or be closed (if its current dir was deleted)
   * Connected to the DBus signal from the KDirNotify interface.
   */
  void slotFilesRemoved( const QStringList& fileList );

  /**
   * Notify that files have been changed.
   * At the moment, this is only used for new icon, but it could be
   * used for size etc. as well.
   * Connected to the DBus signal from the KDirNotify interface.
   */
  void slotFilesChanged( const QStringList& fileList );
  void slotFileRenamed( const QString& srcUrl, const QString& dstUrl );

private Q_SLOTS:
  void slotFileDirty( const QString &_file );
  void slotFileCreated( const QString &_file );
  void slotFileDeleted( const QString &_file );

  void slotEntries( KIO::Job *job, const KIO::UDSEntryList &entries );
  void slotResult( KJob *j );
  void slotRedirection( KIO::Job *job, const KUrl &url );

  void slotUpdateEntries( KIO::Job *job, const KIO::UDSEntryList &entries );
  void slotUpdateResult( KJob *job );
  void processPendingUpdates();

private:
    class DirItem;
    DirItem* dirItemForUrl(const KUrl& dir) const;

    bool validUrl( const KDirLister *lister, const KUrl& _url ) const;

    // helper for both stop methods
    void stopLister(KDirLister* lister, const QString& url, KDirListerCacheDirectoryData& dirData, bool silent);

    KIO::ListJob *jobForUrl( const QString& url, KIO::ListJob *not_job = 0 );
    const KUrl& joburl( KIO::ListJob *job );

    void killJob( KIO::ListJob *job );

    // Called when something tells us that the directory @p url has changed.
    // Returns true if @p url is held by some lister (meaning: do the update now)
    // otherwise mark the cached item as not-up-to-date for later and return false
    bool checkUpdate( const QString& url );

    // Helper method for slotFileDirty
    void handleFileDirty(const KUrl& url);
    void handleDirDirty(const KUrl& url);

  // when there were items deleted from the filesystem all the listers holding
  // the parent directory need to be notified, the unmarked items have to be deleted
  // and removed from the cache including all the children.
  void deleteUnmarkedItems( const QList<KDirLister *>&, KFileItemList & );
    // Helper method called when we know that a list of items was deleted
    void itemsDeleted(const QList<KDirLister *>& listers, const KFileItemList& deletedItems);
    void slotFilesRemoved(const KUrl::List& urls);
    // common for slotRedirection and slotFileRenamed
  void renameDir( const KUrl &oldUrl, const KUrl &url );
  // common for deleteUnmarkedItems and slotFilesRemoved
  void deleteDir( const KUrl& dirUrl );
  // remove directory from cache (itemsCached), including all child dirs
  void removeDirFromCache( const KUrl& dir );
  // helper for renameDir
  void emitRedirections( const KUrl &oldUrl, const KUrl &url );

    /**
     * Emits refreshItem() in the directories that cared for oldItem.
     * The caller has to remember to call emitItems in the set of dirlisters returned
     * (but this allows to buffer change notifications)
     */
    QSet<KDirLister *> emitRefreshItem(const KFileItem& oldItem, const KFileItem& fileitem);

    /**
     * When KDirWatch tells us that something changed in "dir", we need to
     * also notify the dirlisters that are listing a symlink to "dir" (#213799)
     */
    QStringList directoriesForCanonicalPath(const QString& dir) const;

#ifndef NDEBUG
  void printDebug();
#endif

  class DirItem
  {
  public:
    DirItem(const KUrl &dir, const QString& canonicalPath)
      : url(dir), m_canonicalPath(canonicalPath)
    {
      autoUpdates = 0;
      complete = false;
    }

    ~DirItem()
    {
      if ( autoUpdates )
      {
        if ( KDirWatch::exists() && url.isLocalFile() )
            KDirWatch::self()->removeDir(m_canonicalPath);
        sendSignal( false, url );
      }
      lstItems.clear();
    }

    void sendSignal( bool entering, const KUrl& url )
    {
        // Note that "entering" means "start watching", and "leaving" means "stop watching"
        // (i.e. it's not when the user leaves the directory, it's when the directory is removed from the cache)
        if (entering)
            org::kde::KDirNotify::emitEnteredDirectory( url.url() );
        else
            org::kde::KDirNotify::emitLeftDirectory( url.url() );
    }

    void redirect( const KUrl& newUrl )
    {
      if ( autoUpdates )
      {
        if ( url.isLocalFile() )
            KDirWatch::self()->removeDir(m_canonicalPath);
        sendSignal( false, url );

        if (newUrl.isLocalFile()) {
            m_canonicalPath = QFileInfo(newUrl.toLocalFile()).canonicalFilePath();
            KDirWatch::self()->addDir(m_canonicalPath);
        }
        sendSignal( true, newUrl );
      }

      url = newUrl;

      if ( !rootItem.isNull() )
        rootItem.setUrl( newUrl );
    }

    void incAutoUpdate()
    {
      if ( autoUpdates++ == 0 )
      {
        if ( url.isLocalFile() )
          KDirWatch::self()->addDir(m_canonicalPath);
        sendSignal( true, url );
      }
    }

    void decAutoUpdate()
    {
      if ( --autoUpdates == 0 )
      {
        if ( url.isLocalFile() )
          KDirWatch::self()->removeDir(m_canonicalPath);
        sendSignal( false, url );
      }

      else if ( autoUpdates < 0 )
        autoUpdates = 0;
    }

    // number of KDirListers using autoUpdate for this dir
    short autoUpdates;

    // this directory is up-to-date
    bool complete;

    // the complete url of this directory
    KUrl url;

    // the local path, with symlinks resolved, so that KDirWatch works
    QString m_canonicalPath;

    // KFileItem representing the root of this directory.
    // Remember that this is optional. FTP sites don't return '.' in
    // the list, so they give no root item
    KFileItem rootItem;
    KFileItemList lstItems;
  };

    //static const unsigned short MAX_JOBS_PER_LISTER;

    QMap<KIO::ListJob *, KIO::UDSEntryList> runningListJobs;

    // an item is a complete directory
    QHash<QString /*url*/, DirItem*> itemsInUse;
    QCache<QString /*url*/, DirItem> itemsCached;

    typedef QHash<QString /*url*/, KDirListerCacheDirectoryData> DirectoryDataHash;
    DirectoryDataHash directoryData;

    // Symlink-to-directories are registered here so that we can
    // find the url that changed, when kdirwatch tells us about
    // changes in the canonical url. (#213799)
    QHash<QString /*canonical path*/, QStringList /*dirlister urls*/> canonicalUrls;

    // Set of local files that we have changed recently (according to KDirWatch)
    // We temporize the notifications by keeping them 500ms in this list.
    QSet<QString /*path*/> pendingUpdates;
    // The timer for doing the delayed updates
    QTimer pendingUpdateTimer;

    // Set of remote files that have changed recently -- but we can't emit those
    // changes yet, we need to wait for the "update" directory listing.
    // The cmp() call can't differ mimetypes since they are determined on demand,
    // this is why we need to remember those files here.
    QSet<KFileItem*> pendingRemoteUpdates;

    // the KDirNotify signals
    OrgKdeKDirNotifyInterface *kdirnotify;

    struct ItemInUseChange;
};

// Data associated with a directory url
// This could be in DirItem but only in the itemsInUse dict...
struct KDirListerCacheDirectoryData
{
    // A lister can be EITHER in listersCurrentlyListing OR listersCurrentlyHolding
    // but NOT in both at the same time.
    // But both lists can have different listers at the same time; this
    // happens if more listers are requesting url at the same time and
    // one lister was stopped during the listing of files.

    // Listers that are currently listing this url
    QList<KDirLister *> listersCurrentlyListing;
    // Listers that are currently holding this url
    QList<KDirLister *> listersCurrentlyHolding;

    void moveListersWithoutCachedItemsJob();
};

//const unsigned short KDirListerCache::MAX_JOBS_PER_LISTER = 5;

// This job tells KDirListerCache to emit cached items asynchronously from listDir()
// to give the KDirLister user enough time for connecting to its signals, and so
// that KDirListerCache behaves just like when a real KIO::Job is used: nothing
// is emitted during the openUrl call itself.
class KDirLister::Private::CachedItemsJob : public KJob {
    Q_OBJECT
public:
    CachedItemsJob(KDirLister* lister, const KFileItemList& items, const KFileItem& rootItem,
                   const KUrl& url, bool reload)
        : KJob(lister),
          m_lister(lister), m_url(url),
          m_items(items), m_rootItem(rootItem),
          m_reload(reload), m_emitCompleted(true) {
        Q_ASSERT(lister->d->m_cachedItemsJob == 0);
        lister->d->m_cachedItemsJob = this;
        setAutoDelete(true);
        start();
    }

    /*reimp*/ void start() { QMetaObject::invokeMethod(this, "done", Qt::QueuedConnection); }

    // For updateDirectory() to cancel m_emitCompleted;
    void setEmitCompleted(bool b) { m_emitCompleted = b; }

    KUrl url() const { return m_url; }

public Q_SLOTS:
    void done();

private:
    KDirLister* m_lister;
    KUrl m_url;
    KFileItemList m_items;
    KFileItem m_rootItem;
    bool m_reload;
    bool m_emitCompleted;
};

#endif
