/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
                 2000 Carsten Pfeiffer <pfeiffer@kde.org>
                 2003-2005 David Faure <faure@kde.org>
                 2001-2006 Michael Brade <brade@kde.org>

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

#include "kdirlister.h"
#include "kdirlister_p.h"

#include <QtCore/QRegExp>
#include <QtCore/QTimer>

#include <kapplication.h>
#include <kdebug.h>
#include <kde_file.h>
#include <klocale.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include "kprotocolmanager.h"
#include "kmountpoint.h"
#include <sys/stat.h>

#include <assert.h>
#include <QFile>

// Enable this to get printDebug() called often, to see the contents of the cache
//#define DEBUG_CACHE

// Make really sure it doesn't get activated in the final build
#ifdef NDEBUG
#undef DEBUG_CACHE
#endif

K_GLOBAL_STATIC(KDirListerCache, kDirListerCache)

KDirListerCache::KDirListerCache()
    : itemsCached( 10 ) // keep the last 10 directories around
{
    //kDebug(7004);

  connect( &pendingUpdateTimer, SIGNAL(timeout()), this, SLOT(processPendingUpdates()) );
  pendingUpdateTimer.setSingleShot( true );

  connect( KDirWatch::self(), SIGNAL( dirty( const QString& ) ),
           this, SLOT( slotFileDirty( const QString& ) ) );
  connect( KDirWatch::self(), SIGNAL( created( const QString& ) ),
           this, SLOT( slotFileCreated( const QString& ) ) );
  connect( KDirWatch::self(), SIGNAL( deleted( const QString& ) ),
           this, SLOT( slotFileDeleted( const QString& ) ) );

  kdirnotify = new org::kde::KDirNotify(QString(), QString(), QDBusConnection::sessionBus(), this);
  connect(kdirnotify, SIGNAL(FileRenamed(QString,QString)), SLOT(slotFileRenamed(QString,QString)));
  connect(kdirnotify, SIGNAL(FilesAdded(QString)), SLOT(slotFilesAdded(QString)));
  connect(kdirnotify, SIGNAL(FilesChanged(QStringList)), SLOT(slotFilesChanged(QStringList)));
  connect(kdirnotify, SIGNAL(FilesRemoved(QStringList)), SLOT(slotFilesRemoved(QStringList)));

  // The use of KUrl::url() in ~DirItem (sendSignal) crashes if the static for QRegExpEngine got deleted already,
  // so we need to destroy the KDirListerCache before that.
  qAddPostRoutine(kDirListerCache.destroy);
}

KDirListerCache::~KDirListerCache()
{
    //kDebug(7004);

    qDeleteAll(itemsInUse);
    itemsInUse.clear();

    itemsCached.clear();
    directoryData.clear();

    if ( KDirWatch::exists() )
        KDirWatch::self()->disconnect( this );
}

// setting _reload to true will emit the old files and
// call updateDirectory
bool KDirListerCache::listDir( KDirLister *lister, const KUrl& _u,
                               bool _keep, bool _reload )
{
  // like this we don't have to worry about trailing slashes any further
  KUrl _url(_u);
  _url.cleanPath(); // kill consecutive slashes

  if (KProtocolInfo::protocolClass(_url.protocol()) == ":local") {
      // ":local" protocols ignore the hostname, so strip it out preventively - #160057
      _url.setHost(QString());
      if (_keep == false)
          emit lister->redirection(_url);
  }

  _url.adjustPath(KUrl::RemoveTrailingSlash);
  const QString urlStr = _url.url();

  if (!validUrl(lister, _url)) {
        kDebug(7004) << lister << "url=" << _url << "not a valid url";
        return false;
  }

#ifdef DEBUG_CACHE
  printDebug();
#endif
  //kDebug(7004) << lister << "url=" << _url << "keep=" << _keep << "reload=" << _reload;

  if ( !_keep )
  {
    // stop any running jobs for lister
    stop( lister );

    // clear our internal list for lister
    forgetDirs( lister );

    lister->d->rootFileItem = KFileItem();
  }
  else if ( lister->d->lstDirs.contains( _url ) )
  {
    // stop the job listing _url for this lister
    stop( lister, _url );

    // remove the _url as well, it will be added in a couple of lines again!
    // forgetDirs with three args does not do this
    // TODO: think about moving this into forgetDirs
    lister->d->lstDirs.removeAll( _url );

    // clear _url for lister
    forgetDirs( lister, _url, true );

    if ( lister->d->url == _url )
      lister->d->rootFileItem = KFileItem();
  }

    lister->d->complete = false;

    lister->d->lstDirs.append(_url);

    if (lister->d->url.isEmpty() || !_keep) // set toplevel URL only if not set yet
        lister->d->url = _url;

    DirItem *itemU = itemsInUse.value(urlStr);

    KDirListerCacheDirectoryData& dirData = directoryData[urlStr]; // find or insert

    if (dirData.listersCurrentlyListing.isEmpty()) {
        // if there is an update running for _url already we get into
        // the following case - it will just be restarted by updateDirectory().

        dirData.listersCurrentlyListing.append(lister);

        DirItem *itemFromCache;
        if (itemU || (!_reload && (itemFromCache = itemsCached.take(urlStr)) ) ) {
            if (itemU) {
                kDebug(7004) << "Entry already in use:" << _url;
                // if _reload is set, then we'll emit cached items and then updateDirectory.
            } else {
                kDebug(7004) << "Entry in cache:" << _url;
                itemFromCache->decAutoUpdate();
                itemsInUse.insert(urlStr, itemFromCache);
                itemU = itemFromCache;
            }

            emit lister->started(_url);

            // List items from the cache in a delayed manner, just like things would happen
            // if we were not using the cache.
            KDirLister::Private::CachedItemsJob* cachedItemsJob =
                new KDirLister::Private::CachedItemsJob(lister, itemU->lstItems, itemU->rootItem, _url, _reload);
            cachedItemsJob->start();
            lister->d->m_cachedItemsJob = cachedItemsJob;

        } else {
            // dir not in cache or _reload is true
            if (_reload) {
                kDebug(7004) << "Reloading directory:" << _url;
                itemsCached.remove(urlStr);
            } else {
                kDebug(7004) << "Listing directory:" << _url;
            }

            itemU = new DirItem(_url);
            itemsInUse.insert(urlStr, itemU);

//        // we have a limit of MAX_JOBS_PER_LISTER concurrently running jobs
//        if ( lister->d->numJobs() >= MAX_JOBS_PER_LISTER )
//        {
//          pendingUpdates.insert( _url );
//        }
//        else
            {
                KIO::ListJob* job = KIO::listDir(_url, KIO::HideProgressInfo);
                jobs.insert(job, KIO::UDSEntryList());

                lister->d->jobStarted(job);
                lister->d->connectJob(job);

                if (lister->d->window)
                    job->ui()->setWindow(lister->d->window);

                connect(job, SIGNAL(entries(KIO::Job *, KIO::UDSEntryList)),
                        this, SLOT(slotEntries(KIO::Job *, KIO::UDSEntryList)));
                connect(job, SIGNAL(result(KJob *)),
                        this, SLOT(slotResult(KJob *)));
                connect(job, SIGNAL(redirection(KIO::Job *,KUrl)),
                        this, SLOT(slotRedirection(KIO::Job *,KUrl)));

                emit lister->started(_url);
            }
        }
    } else {

        kDebug(7004) << "Entry currently being listed:" << _url << "by" << dirData.listersCurrentlyListing;
#ifdef DEBUG_CACHE
        printDebug();
#endif

        emit lister->started( _url );

        dirData.listersCurrentlyListing.append( lister );

        KIO::ListJob *job = jobForUrl( urlStr );
        // job will be 0 if we were listing from cache rather than listing from a kio job.
        if( job ) {
            lister->d->jobStarted( job );
            lister->d->connectJob( job );
        }
        Q_ASSERT( itemU );

        // List existing items in a delayed manner, just like things would happen
        // if we were not using the cache.
        //kDebug() << "Listing" << itemU->lstItems.count() << "cached items soon";
        KDirLister::Private::CachedItemsJob* cachedItemsJob =
            new KDirLister::Private::CachedItemsJob(lister, itemU->lstItems, itemU->rootItem, _url, _reload);
        cachedItemsJob->start();
        lister->d->m_cachedItemsJob = cachedItemsJob;

#ifdef DEBUG_CACHE
        printDebug();
#endif
    }

    // automatic updating of directories
    if (lister->d->autoUpdate)
        itemU->incAutoUpdate();

    return true;
}

void KDirLister::Private::CachedItemsJob::done()
{
    //kDebug() << "lister" << m_lister << "says" << m_lister->d->m_cachedItemsJob << "this=" << this;
    Q_ASSERT(m_lister->d->m_cachedItemsJob == this);
    kDirListerCache->emitItemsFromCache(m_lister, m_items, m_rootItem, m_url, m_reload, m_emitCompleted);
    emitResult();
}

void KDirListerCache::emitItemsFromCache(KDirLister* lister, const KFileItemList& items, const KFileItem& rootItem, const KUrl& _url, bool _reload, bool _emitCompleted)
{
    lister->d->m_cachedItemsJob = 0;

    const QString urlStr = _url.url();
    DirItem *itemU = kDirListerCache->itemsInUse.value(urlStr);
    Q_ASSERT(itemU); // hey we're listing that dir, so this can't be 0, right?

    KDirLister::Private* kdl = lister->d;

    kdl->complete = false;

    if ( kdl->rootFileItem.isNull() && kdl->url == _url )
        kdl->rootFileItem = rootItem;

    //kDebug(7004) << "emitting" << items.count() << "for lister" << lister;
    kdl->addNewItems( items );
    kdl->emitItems();

    KDirListerCacheDirectoryData& dirData = directoryData[urlStr];
    Q_ASSERT(dirData.listersCurrentlyListing.contains(lister));

    // Emit completed, unless we were told not to,
    // or if listDir() was called while another directory listing for this dir was happening,
    // so we "joined" it. We detect that using jobForUrl to ensure it's a real ListJob,
    // not just a lister-specific CachedItemsJob (which wouldn't emit completed for us).
    if (_emitCompleted && jobForUrl( urlStr ) == 0) {

        dirData.listersCurrentlyHolding.append( lister );
        dirData.listersCurrentlyListing.removeAll( lister );

        kdl->complete = true;
        emit lister->completed( _url );
        emit lister->completed();

        if ( _reload || !itemU->complete ) {
            updateDirectory( _url );
        }
    }
}

bool KDirListerCache::validUrl( const KDirLister *lister, const KUrl& url ) const
{
  if ( !url.isValid() )
  {
    if ( lister->d->autoErrorHandling )
    {
      QString tmp = i18n("Malformed URL\n%1", url.prettyUrl() );
      KMessageBox::error( lister->d->errorParent, tmp );
    }
    return false;
  }

  if ( !KProtocolManager::supportsListing( url ) )
  {
    if ( lister->d->autoErrorHandling )
    {
      QString tmp = i18n("URL cannot be listed\n%1", url.prettyUrl() );
      KMessageBox::error( lister->d->errorParent, tmp );
    }
    return false;
  }

  return true;
}

void KDirListerCache::stop( KDirLister *lister )
{
#ifdef DEBUG_CACHE
    printDebug();
#endif
    //kDebug(7004) << "lister: " << lister;
    bool stopped = false;

    QHash<QString,KDirListerCacheDirectoryData>::iterator dirit = directoryData.begin();
    const QHash<QString,KDirListerCacheDirectoryData>::iterator dirend = directoryData.end();
    for( ; dirit != dirend ; ++dirit ) {
        KDirListerCacheDirectoryData& dirData = dirit.value();
        if ( dirData.listersCurrentlyListing.removeAll(lister) ) { // contains + removeAll in one go
            // lister is listing url
            const QString url = dirit.key();

            //kDebug(7004) << " found lister in list - for " << url;
            stopLister(lister, url, dirData);
            stopped = true;
        }
    }

    if (lister->d->m_cachedItemsJob) {
        delete lister->d->m_cachedItemsJob;
        lister->d->m_cachedItemsJob = 0;
        stopped = true;
    }

    if ( stopped ) {
        emit lister->canceled();
        lister->d->complete = true;
    }

    // this is wrong if there is still an update running!
    //Q_ASSERT( lister->d->complete );
}

void KDirListerCache::stop( KDirLister *lister, const KUrl& _u )
{
    KUrl url(_u);
    url.adjustPath( KUrl::RemoveTrailingSlash );
    const QString urlStr = url.url();

    if (lister->d->m_cachedItemsJob && lister->d->m_cachedItemsJob->url() == url) {
        delete lister->d->m_cachedItemsJob;
        lister->d->m_cachedItemsJob = 0;
    }

    // TODO: consider to stop all the "child jobs" of url as well
    kDebug(7004) << lister << " url=" << url;

    QHash<QString,KDirListerCacheDirectoryData>::iterator dirit = directoryData.find(urlStr);
    if (dirit == directoryData.end())
        return;
    KDirListerCacheDirectoryData& dirData = dirit.value();
    if ( dirData.listersCurrentlyListing.removeAll(lister) ) { // contains + removeAll in one go

        stopLister(lister, urlStr, dirData);

        if ( lister->d->numJobs() == 0 ) {
            lister->d->complete = true;
            // we killed the last job for lister
            emit lister->canceled();
        }
    }
}

// Helper for both stop() methods
void KDirListerCache::stopLister(KDirLister* lister, const QString& url, KDirListerCacheDirectoryData& dirData)
{
    kDebug(7004) << "stopping lister" << lister << url;
    KIO::ListJob *job = jobForUrl( url );
    if ( job )
        lister->d->jobDone( job );

    // move lister to listersCurrentlyHolding
    dirData.listersCurrentlyHolding.append(lister);

    emit lister->canceled( KUrl( url ) );

    //kDebug(7004) << "remaining list: " << listers->count() << " listers";

    if ( dirData.listersCurrentlyListing.isEmpty() ) {
        // kill the job since it isn't used any more
        if ( job )
            killJob( job );
    }
}

void KDirListerCache::setAutoUpdate( KDirLister *lister, bool enable )
{
    // IMPORTANT: this method does not check for the current autoUpdate state!

    for ( KUrl::List::const_iterator it = lister->d->lstDirs.constBegin();
          it != lister->d->lstDirs.constEnd(); ++it ) {
        DirItem* dirItem = itemsInUse.value((*it).url());
        Q_ASSERT(dirItem);
        if ( enable )
            dirItem->incAutoUpdate();
        else
            dirItem->decAutoUpdate();
    }
}

void KDirListerCache::forgetDirs( KDirLister *lister )
{
    //kDebug(7004) << lister;

    emit lister->clear();
    // clear lister->d->lstDirs before calling forgetDirs(), so that
    // it doesn't contain things that itemsInUse doesn't. When emitting
    // the canceled signals, lstDirs must not contain anything that
    // itemsInUse does not contain. (otherwise it might crash in findByName()).
    const KUrl::List lstDirsCopy = lister->d->lstDirs;
    lister->d->lstDirs.clear();

    for ( KUrl::List::const_iterator it = lstDirsCopy.begin();
          it != lstDirsCopy.end(); ++it ) {
        forgetDirs( lister, *it, false );
    }
}

static bool manually_mounted(const QString& path, const KMountPoint::List& possibleMountPoints)
{
    KMountPoint::Ptr mp = possibleMountPoints.findByPath(path);
    if (!mp) // not listed in fstab -> yes, manually mounted
        return true;
    const bool supermount = mp->mountType() == "supermount";
    if (supermount) {
        return true;
    }
    // noauto -> manually mounted. Otherwise, mounted at boot time, won't be unmounted any time soon hopefully.
    return mp->mountOptions().contains("noauto");
}


void KDirListerCache::forgetDirs( KDirLister *lister, const KUrl& _url, bool notify )
{
    //kDebug(7004) << lister << " _url: " << _url;

    KUrl url( _url );
    url.adjustPath( KUrl::RemoveTrailingSlash );
    const QString urlStr = url.url();

    DirectoryDataHash::iterator dit = directoryData.find(urlStr);
    if (dit == directoryData.end())
        return;
    KDirListerCacheDirectoryData& dirData = *dit;
    dirData.listersCurrentlyHolding.removeAll(lister);

    DirItem *item = itemsInUse.value(urlStr);
    Q_ASSERT(item);

    if ( dirData.listersCurrentlyHolding.isEmpty() && dirData.listersCurrentlyListing.isEmpty() ) {
        // item not in use anymore -> move into cache if complete
        directoryData.erase(dit);
        itemsInUse.remove( urlStr );

        // this job is a running update
        KIO::ListJob *job = jobForUrl( urlStr );
        if ( job ) {
            lister->d->jobDone( job );
            killJob( job );
            kDebug(7004) << "Killing update job for " << urlStr;

            emit lister->canceled( url );
            if ( lister->d->numJobs() == 0 ) {
                lister->d->complete = true;
                emit lister->canceled();
            }
        }

        if ( notify ) {
            lister->d->lstDirs.removeAll( url );
            emit lister->clear( url );
        }

        if ( item->complete ) {
            kDebug(7004) << lister << " item moved into cache: " << url;
            itemsCached.insert( urlStr, item );

            const KMountPoint::List possibleMountPoints = KMountPoint::possibleMountPoints(KMountPoint::NeedMountOptions);

            // Should we forget the dir for good, or keep a watch on it?
            // Generally keep a watch, except when it would prevent
            // unmounting a removable device (#37780)
            const bool isLocal = item->url.isLocalFile();
            bool isManuallyMounted = false;
            bool containsManuallyMounted = false;
            if (isLocal) {
                isManuallyMounted = manually_mounted( item->url.path(), possibleMountPoints );
                if ( !isManuallyMounted ) {
                    // Look for a manually-mounted directory inside
                    // If there's one, we can't keep a watch either, FAM would prevent unmounting the CDROM
                    // I hope this isn't too slow
                    KFileItemList::const_iterator kit = item->lstItems.begin();
                    const KFileItemList::const_iterator kend = item->lstItems.end();
                    for ( ; kit != kend && !containsManuallyMounted; ++kit )
                        if ( (*kit).isDir() && manually_mounted((*kit).url().path(), possibleMountPoints) )
                            containsManuallyMounted = true;
                }
            }

            if ( isManuallyMounted || containsManuallyMounted )
            {
                kDebug(7004) << "Not adding a watch on " << item->url << " because it " <<
                    ( isManuallyMounted ? "is manually mounted" : "contains a manually mounted subdir" );
                item->complete = false; // set to "dirty"
            }
            else
                item->incAutoUpdate(); // keep watch
        }
        else
        {
            delete item;
            item = 0;
        }
    }

    if ( item && lister->d->autoUpdate )
        item->decAutoUpdate();
}

void KDirListerCache::updateDirectory( const KUrl& _dir )
{
    kDebug(7004) << _dir;

    QString urlStr = _dir.url(KUrl::RemoveTrailingSlash);
    if ( !checkUpdate( urlStr ) )
        return;

    // A job can be running to
    //   - only list a new directory: the listers are in listersCurrentlyListing
    //   - only update a directory: the listers are in listersCurrentlyHolding
    //   - update a currently running listing: the listers are in both

    KDirListerCacheDirectoryData& dirData = directoryData[urlStr];
    QList<KDirLister *> listers = dirData.listersCurrentlyListing;
    QList<KDirLister *> holders = dirData.listersCurrentlyHolding;

    // restart the job for _dir if it is running already
    bool killed = false;
    QWidget *window = 0;
    KIO::ListJob *job = jobForUrl( urlStr );
    if (job) {
        window = job->ui()->window();

        killJob( job );
        killed = true;

        foreach ( KDirLister *kdl, listers )
            kdl->d->jobDone( job );

        foreach ( KDirLister *kdl, holders )
            kdl->d->jobDone( job );
    } else {
        // Emit any cached items.
        // updateDirectory() is about the diff compared to the cached items...
        Q_FOREACH(KDirLister *kdl, listers) {
            if (kdl->d->m_cachedItemsJob) {
                KDirLister::Private::CachedItemsJob* job = kdl->d->m_cachedItemsJob;
                job->setEmitCompleted(false);
                job->done(); // sets kdl->d->m_cachedItemsJob to 0
                delete job;
                killed = true;
            }
        }
    }
    //if (killed) {
    //    kDebug(7004) << "Killed=" << killed;
    //}

    // we don't need to emit canceled signals since we only replaced the job,
    // the listing is continuing.

    Q_ASSERT( listers.isEmpty() || killed );

    job = KIO::listDir( _dir, KIO::HideProgressInfo );
    jobs.insert( job, KIO::UDSEntryList() );

    connect( job, SIGNAL(entries( KIO::Job *, const KIO::UDSEntryList & )),
             this, SLOT(slotUpdateEntries( KIO::Job *, const KIO::UDSEntryList & )) );
    connect( job, SIGNAL(result( KJob * )),
             this, SLOT(slotUpdateResult( KJob * )) );

    kDebug(7004) << "update started in" << _dir;

    foreach ( KDirLister *kdl, listers ) {
        kdl->d->jobStarted( job );
    }

    if ( !holders.isEmpty() ) {
        if ( !killed ) {
            bool first = true;
            foreach ( KDirLister *kdl, holders ) {
                kdl->d->jobStarted( job );
                if ( first && kdl->d->window ) {
                    first = false;
                    job->ui()->setWindow( kdl->d->window );
                }
                emit kdl->started( _dir );
            }
        } else {
            job->ui()->setWindow( window );

            foreach ( KDirLister *kdl, holders ) {
                kdl->d->jobStarted( job );
            }
        }
    }
}

bool KDirListerCache::checkUpdate( const QString& _dir )
{
  if ( !itemsInUse.contains(_dir) )
  {
    DirItem *item = itemsCached[_dir];
    if ( item && item->complete )
    {
      item->complete = false;
      item->decAutoUpdate();
      // Hmm, this debug output might include login/password from the _dir URL.
      //kDebug(7004) << "directory " << _dir << " not in use, marked dirty.";
    }
    //else
      //kDebug(7004) << "aborted, directory " << _dir << " not in cache.";

    return false;
  }
  else
    return true;
}

KFileItem KDirListerCache::itemForUrl( const KUrl& url ) const
{
    KFileItem *item = findByUrl( 0, url );
    if (item) {
        return *item;
    } else {
        return KFileItem();
    }
}

KDirListerCache::DirItem *KDirListerCache::dirItemForUrl(const KUrl& dir) const
{
    const QString urlStr = dir.url(KUrl::RemoveTrailingSlash);
    DirItem *item = itemsInUse.value(urlStr);
    if ( !item )
        item = itemsCached[urlStr];
    return item;
}

KFileItemList *KDirListerCache::itemsForDir(const KUrl& dir) const
{
    DirItem *item = dirItemForUrl(dir);
    return item ? &item->lstItems : 0;
}

KFileItem KDirListerCache::findByName( const KDirLister *lister, const QString& _name ) const
{
  Q_ASSERT( lister );

  for ( KUrl::List::Iterator it = lister->d->lstDirs.begin();
        it != lister->d->lstDirs.end(); ++it )
  {
    const KFileItem item = itemsInUse[(*it).url()]->lstItems.findByName( _name );
    if ( !item.isNull() )
      return item;
  }

  return KFileItem();
}

KFileItem *KDirListerCache::findByUrl( const KDirLister *lister, const KUrl& _u ) const
{
    KUrl url(_u);
    url.adjustPath(KUrl::RemoveTrailingSlash);

    // Maybe _u is a directory itself? (see KDirModelTest::testChmodDirectory)
    DirItem* dirItem = dirItemForUrl(url);
    if (dirItem && !dirItem->rootItem.isNull() && dirItem->rootItem.url() == url) {
        // If lister is set, check that it contains this dir
        if (!lister || lister->d->lstDirs.contains(url))
            return &dirItem->rootItem;
    }

    KUrl parentDir(url);
    parentDir.setPath( parentDir.directory() );

    // If lister is set, check that it contains this dir
    if (lister && !lister->d->lstDirs.contains(parentDir))
        return 0;

    dirItem = dirItemForUrl(parentDir);
    if (dirItem) {
        KFileItemList::iterator it = dirItem->lstItems.begin();
        const KFileItemList::iterator end = dirItem->lstItems.end();
        for (; it != end ; ++it) {
            if ((*it).url() == url) {
                return &*it;
            }
        }
    }

    return 0;
}

void KDirListerCache::slotFilesAdded( const QString &dir ) // from KDirNotify signals
{
  kDebug(7004) << dir;
  updateDirectory( KUrl(dir) );
}

void KDirListerCache::slotFilesRemoved( const QStringList &fileList ) // from KDirNotify signals
{
    kDebug(7004) << fileList.count();
    // Group notifications by parent dirs (usually there would be only one parent dir)
    QMap<QString, KFileItemList> removedItemsByDir;
    QStringList deletedSubdirs;

    for (QStringList::const_iterator it = fileList.begin(); it != fileList.end() ; ++it) {
    KUrl url( *it );
    KUrl parentDir( url );
    parentDir.setPath( parentDir.directory() );
    KFileItemList *lstItems = itemsForDir( parentDir );
    if ( lstItems )
    {
      for ( KFileItemList::iterator fit = lstItems->begin(), fend = lstItems->end() ; fit != fend ; ++fit ) {
        if ( (*fit ).url() == url ) {
          const KFileItem fileitem = *fit;
          removedItemsByDir[parentDir.url()].append(fileitem);
          // If we found a fileitem, we can test if it's a dir. If not, we'll go to deleteDir just in case.
          if (fileitem.isNull() || fileitem.isDir()) {
              deletedSubdirs.append(url.url());
          }
          lstItems->erase( fit ); // remove fileitem from list
          break;
        }
      }
    }
    }

    QMap<QString, KFileItemList>::const_iterator rit = removedItemsByDir.begin();
    for(; rit != removedItemsByDir.end(); ++rit) {
        // Tell the views about it before calling deleteDir.
        // They might need the subdirs' file items (see the dirtree).
        DirectoryDataHash::const_iterator dit = directoryData.find(rit.key());
        if (dit != directoryData.end()) {
            itemsDeleted((*dit).listersCurrentlyHolding, rit.value());
        }
    }

    Q_FOREACH(const QString& url, deletedSubdirs) {
        // in case of a dir, check if we have any known children, there's much to do in that case
        // (stopping jobs, removing dirs from cache etc.)
        deleteDir(url);
    }
}

void KDirListerCache::slotFilesChanged( const QStringList &fileList ) // from KDirNotify signals
{
    KUrl::List dirsToUpdate;
    QStringList::const_iterator it = fileList.begin();
    for (; it != fileList.end() ; ++it) {
        KUrl url( *it );
        KFileItem *fileitem = findByUrl(0, url);
        if (!fileitem) {
            kDebug(7004) << "item not found for" << url;
            continue;
        }
        if (url.isLocalFile()) {
            // we need to refresh the item, because e.g. the permissions can have changed.
            aboutToRefreshItem(*fileitem);
            KFileItem oldItem = *fileitem;
            fileitem->refresh();
            emitRefreshItem(oldItem, *fileitem);
        } else {
            // Forget any cached mimetype, it might have changed, and cmp() doesn't check that.
            fileitem->refreshMimeType();
            // For remote files, we won't be able to figure out the new information,
            // we have to do a update (directory listing)
            KUrl dir(url);
            dir.setPath(dir.directory());
            if (!dirsToUpdate.contains(dir))
                dirsToUpdate.prepend(dir);
        }
    }

    KUrl::List::const_iterator itdir = dirsToUpdate.begin();
    for (; itdir != dirsToUpdate.end() ; ++itdir)
        updateDirectory( *itdir );
    // ## TODO problems with current jobs listing/updating that dir
    // ( see kde-2.2.2's kdirlister )
}

void KDirListerCache::slotFileRenamed( const QString &_src, const QString &_dst ) // from KDirNotify signals
{
  KUrl src( _src );
  KUrl dst( _dst );
  kDebug(7004) << src << "->" << dst;
#ifdef DEBUG_CACHE
  printDebug();
#endif

  KUrl oldurl( src );
  oldurl.adjustPath( KUrl::RemoveTrailingSlash );
  KFileItem *fileitem = findByUrl( 0, oldurl );

  // If the item had a UDS_URL as well as UDS_NAME set, the user probably wants
  // to be updating the name only (since they can't see the URL).
  // Check to see if a URL exists, and if so, if only the file part has changed,
  // only update the name and not the underlying URL.
  bool nameOnly = fileitem && !fileitem->entry().stringValue( KIO::UDSEntry::UDS_URL ).isEmpty();
  nameOnly &= src.directory( KUrl::IgnoreTrailingSlash | KUrl::AppendTrailingSlash ) ==
                dst.directory( KUrl::IgnoreTrailingSlash | KUrl::AppendTrailingSlash );

  // Somehow this should only be called if src is a dir. But how could we know if it is?
  // (Note that looking into itemsInUse isn't good enough. One could rename a subdir in a view.)
  if( !nameOnly )
        renameDir( src, dst );

  // Now update the KFileItem representing that file or dir (not exclusive with the above!)
  if ( fileitem )
  {
    if ( !fileitem->isLocalFile() && !fileitem->localPath().isEmpty() ) // it uses UDS_LOCAL_PATH? ouch, needs an update then
        slotFilesChanged( QStringList() << src.url() );
    else
    {
        aboutToRefreshItem( *fileitem );
        KFileItem oldItem = *fileitem;
        if( nameOnly )
            fileitem->setName( dst.fileName() );
        else
            fileitem->setUrl( dst );
        fileitem->refreshMimeType();
        fileitem->determineMimeType();
        emitRefreshItem( oldItem, *fileitem );
    }
  }
#ifdef DEBUG_CACHE
  printDebug();
#endif
}

void KDirListerCache::aboutToRefreshItem( const KFileItem& fileitem )
{
    // Look whether this item was shown in any view, i.e. held by any dirlister
    KUrl parentDir( fileitem.url() );
    parentDir.setPath( parentDir.directory() );
    const QString parentDirURL = parentDir.url();

    DirectoryDataHash::iterator dit = directoryData.find(parentDirURL);
    if (dit == directoryData.end())
        return;

    foreach (KDirLister *kdl, (*dit).listersCurrentlyHolding)
        kdl->d->aboutToRefreshItem( fileitem );

    // Also look in listersCurrentlyListing, in case the user manages to rename during a listing
    foreach (KDirLister *kdl, (*dit).listersCurrentlyListing)
        kdl->d->aboutToRefreshItem( fileitem );
}

void KDirListerCache::emitRefreshItem( const KFileItem& oldItem, const KFileItem& fileitem )
{
    // Look whether this item was shown in any view, i.e. held by any dirlister
    KUrl parentDir( fileitem.url() );
    parentDir.setPath( parentDir.directory() );
    QString parentDirURL = parentDir.url();
    DirectoryDataHash::iterator dit = directoryData.find(parentDirURL);
    QList<KDirLister *> listers;
    // Also look in listersCurrentlyListing, in case the user manages to rename during a listing
    if (dit != directoryData.end())
        listers += (*dit).listersCurrentlyHolding + (*dit).listersCurrentlyListing;
    if (oldItem.isDir()) {
        // For a directory, look for dirlisters where it's the root item.
        dit = directoryData.find(fileitem.url().url());
        if (dit != directoryData.end())
            listers += (*dit).listersCurrentlyHolding + (*dit).listersCurrentlyListing;
    }
    Q_FOREACH(KDirLister *kdl, listers) {
        // For a directory, look for dirlisters where it's the root item.
        if (oldItem.isDir() && kdl->d->rootFileItem == oldItem) {
            kdl->d->rootFileItem = fileitem;
        }
        kdl->d->addRefreshItem( oldItem, fileitem );
        kdl->d->emitItems();
    }
}

// private slots

// Called by KDirWatch - usually when a dir we're watching has been modified,
// but it can also be called for a file.
void KDirListerCache::slotFileDirty( const QString& path )
{
    kDebug(7004) << path;
    // File or dir?
    KDE_struct_stat buff;
    if ( KDE_stat( QFile::encodeName(path), &buff ) != 0 )
        return; // error
    const bool isDir = S_ISDIR(buff.st_mode);
    KUrl url(path);

    if (isDir) {
        // A dir: launch an update job if anyone cares about it
        updateDirectory(url);
    } else {
        // A file: delay updating it, FAM is flooding us with events
        const QString urlStr = url.url(KUrl::RemoveTrailingSlash);
        if (!pendingUpdates.contains(urlStr)) {
            KUrl dir(url);
            dir.setPath(dir.directory());
            if (checkUpdate(dir.url())) {
                pendingUpdates.insert(urlStr);
                if (!pendingUpdateTimer.isActive())
                    pendingUpdateTimer.start( 500 );
            }
        }
    }
}

void KDirListerCache::slotFileCreated( const QString& path ) // from KDirWatch
{
  kDebug(7004) << path;
  // XXX: how to avoid a complete rescan here?
  KUrl u( path );
  u.setPath( u.directory() );
  updateDirectory( u );
}

void KDirListerCache::slotFileDeleted( const QString& path ) // from KDirWatch
{
  kDebug(7004) << path;
  KUrl u( path );
  slotFilesRemoved( QStringList() << u.url() );
}

void KDirListerCache::slotEntries( KIO::Job *job, const KIO::UDSEntryList &entries )
{
    KUrl url(joburl( static_cast<KIO::ListJob *>(job) ));
    url.adjustPath(KUrl::RemoveTrailingSlash);
    QString urlStr = url.url();

    //kDebug(7004) << "new entries for " << url;

    DirItem *dir = itemsInUse.value(urlStr);
    Q_ASSERT( dir );

    DirectoryDataHash::iterator dit = directoryData.find(urlStr);
    Q_ASSERT(dit != directoryData.end());
    KDirListerCacheDirectoryData& dirData = *dit;
    Q_ASSERT( !dirData.listersCurrentlyListing.isEmpty() );

    // check if anyone wants the mimetypes immediately
    bool delayedMimeTypes = true;
    foreach ( KDirLister *kdl, dirData.listersCurrentlyListing )
        delayedMimeTypes &= kdl->d->delayedMimeTypes;

    KIO::UDSEntryList::const_iterator it = entries.begin();
    const KIO::UDSEntryList::const_iterator end = entries.end();
    for ( ; it != end; ++it )
    {
        const QString name = (*it).stringValue( KIO::UDSEntry::UDS_NAME );

        Q_ASSERT( !name.isEmpty() );
        if ( name.isEmpty() )
            continue;

        if ( name == "." )
        {
            Q_ASSERT( dir->rootItem.isNull() );
            // Try to reuse an existing KFileItem (if we listed the parent dir)
            // rather than creating a new one. There are many reasons:
            // 1) renames and permission changes to the item would have to emit the signals
            // twice, otherwise, so that both views manage to recognize the item.
            // 2) with kio_ftp we can only know that something is a symlink when
            // listing the parent, so prefer that item, which has more info.
            dir->rootItem = itemForUrl(url);
            if (dir->rootItem.isNull())
                dir->rootItem = KFileItem( *it, url, delayedMimeTypes, true  );

            foreach ( KDirLister *kdl, dirData.listersCurrentlyListing )
                if ( kdl->d->rootFileItem.isNull() && kdl->d->url == url )
                    kdl->d->rootFileItem = dir->rootItem;
        }
        else if ( name != ".." )
        {
            KFileItem item( *it, url, delayedMimeTypes, true );

            //kDebug(7004)<< "Adding item: " << item.url();
            dir->lstItems.append( item );

            foreach ( KDirLister *kdl, dirData.listersCurrentlyListing )
                kdl->d->addNewItem( item );
        }
    }

    foreach ( KDirLister *kdl, dirData.listersCurrentlyListing )
        kdl->d->emitItems();
}

void KDirListerCache::slotResult( KJob *j )
{
  Q_ASSERT( j );
  KIO::ListJob *job = static_cast<KIO::ListJob *>( j );
  jobs.remove( job );

  KUrl jobUrl(joburl( job ));
  jobUrl.adjustPath(KUrl::RemoveTrailingSlash);  // need remove trailing slashes again, in case of redirections
  QString jobUrlStr = jobUrl.url();

  kDebug(7004) << "finished listing" << jobUrl;
#ifdef DEBUG_CACHE
  printDebug();
#endif

  DirectoryDataHash::iterator dit = directoryData.find(jobUrlStr);
  Q_ASSERT(dit != directoryData.end());
  KDirListerCacheDirectoryData& dirData = *dit;
  Q_ASSERT( !dirData.listersCurrentlyListing.isEmpty() );
  QList<KDirLister *> listers = dirData.listersCurrentlyListing;

  // move all listers to the holding list, do it before emitting
  // the signals to make sure it exists in KDirListerCache in case someone
  // calls listDir during the signal emission
  Q_ASSERT( dirData.listersCurrentlyHolding.isEmpty() );
  dirData.moveListersWithoutCachedItemsJob();

  if ( job->error() )
  {
    foreach ( KDirLister *kdl, listers )
    {
      kdl->d->jobDone( job );
      kdl->handleError( job );
      emit kdl->canceled( jobUrl );
      if ( kdl->d->numJobs() == 0 )
      {
        kdl->d->complete = true;
        emit kdl->canceled();
      }
    }
  }
  else
  {
    DirItem *dir = itemsInUse.value(jobUrlStr);
    Q_ASSERT( dir );
    dir->complete = true;

    foreach ( KDirLister* kdl, listers )
    {
      kdl->d->jobDone( job );
      emit kdl->completed( jobUrl );
      if ( kdl->d->numJobs() == 0 )
      {
        kdl->d->complete = true;
        emit kdl->completed();
      }
    }
  }

  // TODO: hmm, if there was an error and job is a parent of one or more
  // of the pending urls we should cancel it/them as well
  processPendingUpdates();

#ifdef DEBUG_CACHE
  printDebug();
#endif
}

void KDirListerCache::slotRedirection( KIO::Job *j, const KUrl& url )
{
    Q_ASSERT( j );
    KIO::ListJob *job = static_cast<KIO::ListJob *>( j );

    KUrl oldUrl(job->url());  // here we really need the old url!
    KUrl newUrl(url);

    // strip trailing slashes
    oldUrl.adjustPath(KUrl::RemoveTrailingSlash);
    newUrl.adjustPath(KUrl::RemoveTrailingSlash);

    if ( oldUrl == newUrl ) {
        kDebug(7004) << "New redirection url same as old, giving up.";
        return;
    }

    const QString oldUrlStr = oldUrl.url();
    const QString newUrlStr = newUrl.url();

    kDebug(7004) << oldUrl << "->" << newUrl;

#ifdef DEBUG_CACHE
    printDebug();
#endif

    // I don't think there can be dirItems that are children of oldUrl.
    // Am I wrong here? And even if so, we don't need to delete them, right?
    // DF: redirection happens before listDir emits any item. Makes little sense otherwise.

    // oldUrl cannot be in itemsCached because only completed items are moved there
    DirItem *dir = itemsInUse.take(oldUrlStr);
    Q_ASSERT( dir );

    DirectoryDataHash::iterator dit = directoryData.find(oldUrlStr);
    Q_ASSERT(dit != directoryData.end());
    KDirListerCacheDirectoryData oldDirData = *dit;
    directoryData.erase(dit);
    Q_ASSERT( !oldDirData.listersCurrentlyListing.isEmpty() );
    const QList<KDirLister *> listers = oldDirData.listersCurrentlyListing;
    Q_ASSERT( !listers.isEmpty() );

    foreach ( KDirLister *kdl, listers ) {
        kdl->d->redirect(oldUrlStr, newUrl);
    }

    // when a lister was stopped before the job emits the redirection signal, the old url will
    // also be in listersCurrentlyHolding
    const QList<KDirLister *> holders = oldDirData.listersCurrentlyHolding;
    foreach ( KDirLister *kdl, holders ) {
        kdl->d->jobStarted( job );
        // do it like when starting a new list-job that will redirect later
        // TODO: maybe don't emit started if there's an update running for newUrl already?
        emit kdl->started( oldUrl );

        kdl->d->redirect(oldUrl, newUrl);
    }

    DirItem *newDir = itemsInUse.value(newUrlStr);
    if ( newDir ) {
        kDebug(7004) << newUrl << "already in use";

        // only in this case there can newUrl already be in listersCurrentlyListing or listersCurrentlyHolding
        delete dir;

        // get the job if one's running for newUrl already (can be a list-job or an update-job), but
        // do not return this 'job', which would happen because of the use of redirectionURL()
        KIO::ListJob *oldJob = jobForUrl( newUrlStr, job );

        // listers of newUrl with oldJob: forget about the oldJob and use the already running one
        // which will be converted to an updateJob
        KDirListerCacheDirectoryData& newDirData = directoryData[newUrlStr];

        QList<KDirLister *>& curListers = newDirData.listersCurrentlyListing;
        if ( !curListers.isEmpty() ) {
            kDebug(7004) << "and it is currently listed";

            Q_ASSERT( oldJob );  // ?!

            foreach ( KDirLister *kdl, curListers ) { // listers of newUrl
                kdl->d->jobDone( oldJob );

                kdl->d->jobStarted( job );
                kdl->d->connectJob( job );
            }

            // append listers of oldUrl with newJob to listers of newUrl with oldJob
            foreach ( KDirLister *kdl, listers )
                curListers.append( kdl );
        } else {
            curListers = listers;
        }

        if ( oldJob )         // kill the old job, be it a list-job or an update-job
            killJob( oldJob );

        // holders of newUrl: use the already running job which will be converted to an updateJob
        QList<KDirLister *>& curHolders = newDirData.listersCurrentlyHolding;
        if ( !curHolders.isEmpty() ) {
            kDebug(7004) << "and it is currently held.";

            foreach ( KDirLister *kdl, curHolders ) {  // holders of newUrl
                kdl->d->jobStarted( job );
                emit kdl->started( newUrl );
            }

            // append holders of oldUrl to holders of newUrl
            foreach ( KDirLister *kdl, holders )
                curHolders.append( kdl );
        } else {
            curHolders = holders;
        }


        // emit old items: listers, holders. NOT: newUrlListers/newUrlHolders, they already have them listed
        // TODO: make this a separate method?
        foreach ( KDirLister *kdl, listers + holders ) {
            if ( kdl->d->rootFileItem.isNull() && kdl->d->url == newUrl )
                kdl->d->rootFileItem = newDir->rootItem;

            kdl->d->addNewItems( newDir->lstItems );
            kdl->d->emitItems();
        }
    } else if ( (newDir = itemsCached.take( newUrlStr )) ) {
        kDebug(7004) << newUrl << "is unused, but already in the cache.";

        delete dir;
        itemsInUse.insert( newUrlStr, newDir );
        KDirListerCacheDirectoryData& newDirData = directoryData[newUrlStr];
        newDirData.listersCurrentlyListing = listers;
        newDirData.listersCurrentlyHolding = holders;

        // emit old items: listers, holders
        foreach ( KDirLister *kdl, listers + holders ) {
            if ( kdl->d->rootFileItem.isNull() && kdl->d->url == newUrl )
                kdl->d->rootFileItem = newDir->rootItem;

            kdl->d->addNewItems( newDir->lstItems );
            kdl->d->emitItems();
        }
    } else {
        kDebug(7004) << newUrl << "has not been listed yet.";

        dir->rootItem = KFileItem();
        dir->lstItems.clear();
        dir->redirect( newUrl );
        itemsInUse.insert( newUrlStr, dir );
        KDirListerCacheDirectoryData& newDirData = directoryData[newUrlStr];
        newDirData.listersCurrentlyListing = listers;
        newDirData.listersCurrentlyHolding = holders;

        if ( holders.isEmpty() ) {
#ifdef DEBUG_CACHE
            printDebug();
#endif
            return; // only in this case the job doesn't need to be converted,
        }
    }

    // make the job an update job
    job->disconnect( this );

    connect( job, SIGNAL(entries( KIO::Job *, const KIO::UDSEntryList & )),
             this, SLOT(slotUpdateEntries( KIO::Job *, const KIO::UDSEntryList & )) );
    connect( job, SIGNAL(result( KJob * )),
             this, SLOT(slotUpdateResult( KJob * )) );

    // FIXME: autoUpdate-Counts!!

#ifdef DEBUG_CACHE
    printDebug();
#endif
}

void KDirListerCache::renameDir( const KUrl &oldUrl, const KUrl &newUrl )
{
    kDebug(7004) << oldUrl << "->" << newUrl;
    const QString oldUrlStr = oldUrl.url(KUrl::RemoveTrailingSlash);
    const QString newUrlStr = newUrl.url(KUrl::RemoveTrailingSlash);

    // Not enough. Also need to look at any child dir, even sub-sub-sub-dir.
    //DirItem *dir = itemsInUse.take( oldUrlStr );
    //emitRedirections( oldUrl, url );

    typedef QPair<QString, DirItem *> ItemToInsert;
    QLinkedList<ItemToInsert> itemsToInsert;

    // Look at all dirs being listed/shown
    QHash<QString, DirItem *>::iterator itu = itemsInUse.begin();
    const QHash<QString, DirItem *>::iterator ituend = itemsInUse.end();
    bool goNext;
    while ( itu != ituend ) {
        goNext = true;
        DirItem *dir = itu.value();
        KUrl oldDirUrl ( itu.key() );
        //kDebug(7004) << "itemInUse:" << oldDirUrl;
        // Check if this dir is oldUrl, or a subfolder of it
        if ( oldUrl.isParentOf( oldDirUrl ) ) {
            // TODO should use KUrl::cleanpath like isParentOf does
            QString relPath = oldDirUrl.path().mid( oldUrl.path().length() );

            KUrl newDirUrl( newUrl ); // take new base
            if ( !relPath.isEmpty() )
                newDirUrl.addPath( relPath ); // add unchanged relative path
            //kDebug(7004) << "new url=" << newDirUrl;

            // Update URL in dir item and in itemsInUse
            dir->redirect( newDirUrl );
            itu = itemsInUse.erase( itu ); // implies ++itu

            itemsToInsert.append(qMakePair(newDirUrl.url(KUrl::RemoveTrailingSlash), dir));
            goNext = false; // because of the implied ++itu above
            // Rename all items under that dir

            for ( KFileItemList::iterator kit = dir->lstItems.begin(), kend = dir->lstItems.end();
                  kit != kend ; ++kit )
            {
                const KUrl oldItemUrl ((*kit).url());
                const QString oldItemUrlStr( oldItemUrl.url(KUrl::RemoveTrailingSlash) );
                KUrl newItemUrl( oldItemUrl );
                newItemUrl.setPath( newDirUrl.path() );
                newItemUrl.addPath( oldItemUrl.fileName() );
                kDebug(7004) << "renaming" << oldItemUrlStr << "to" << newItemUrl.url();
                (*kit).setUrl( newItemUrl );
            }
            emitRedirections( oldDirUrl, newDirUrl );
        }
        if ( goNext )
            ++itu;
    }

    // Do the inserts out of the loop to avoid messing up iterators
    foreach(const ItemToInsert& i, itemsToInsert) {
        itemsInUse.insert(i.first, i.second);
    }

    // Is oldUrl a directory in the cache?
    // Remove any child of oldUrl from the cache - even if the renamed dir itself isn't in it!
    removeDirFromCache( oldUrl );
    // TODO rename, instead.
}

// helper for renameDir, not used for redirections from KIO::listDir().
void KDirListerCache::emitRedirections( const KUrl &oldUrl, const KUrl &newUrl )
{
    kDebug(7004) << oldUrl << "->" << newUrl;
    const QString oldUrlStr = oldUrl.url(KUrl::RemoveTrailingSlash);
    const QString newUrlStr = newUrl.url(KUrl::RemoveTrailingSlash);

    KIO::ListJob *job = jobForUrl( oldUrlStr );
    if ( job )
        killJob( job );

    // Check if we were listing this dir. Need to abort and restart with new name in that case.
    DirectoryDataHash::iterator dit = directoryData.find(oldUrlStr);
    if ( dit == directoryData.end() )
        return;
    const QList<KDirLister *> listers = (*dit).listersCurrentlyListing;
    const QList<KDirLister *> holders = (*dit).listersCurrentlyHolding;

    KDirListerCacheDirectoryData& newDirData = directoryData[newUrlStr];

    // Tell the world that the job listing the old url is dead.
    foreach ( KDirLister *kdl, listers ) {
        if ( job )
            kdl->d->jobDone( job );

        emit kdl->canceled( oldUrl );
    }
    newDirData.listersCurrentlyListing += listers;

    // Check if we are currently displaying this directory (odds opposite wrt above)
    foreach ( KDirLister *kdl, holders ) {
        if ( job )
            kdl->d->jobDone( job );
    }
    newDirData.listersCurrentlyHolding += holders;
    directoryData.erase(dit);

    if ( !listers.isEmpty() ) {
        updateDirectory( newUrl );

        // Tell the world about the new url
        foreach ( KDirLister *kdl, listers )
            emit kdl->started( newUrl );
    }

    // And notify the dirlisters of the redirection
    foreach ( KDirLister *kdl, holders ) {
        // ### consider replacing this block with kdl->redirect(oldUrl, newUrl)...
        KUrl::List& lstDirs = kdl->d->lstDirs;
        lstDirs[ lstDirs.indexOf( oldUrl ) ] = newUrl;

        if ( lstDirs.count() == 1 )
            emit kdl->redirection( newUrl );

        emit kdl->redirection( oldUrl, newUrl );
    }
}

void KDirListerCache::removeDirFromCache( const KUrl& dir )
{
    kDebug(7004) << dir;
    const QList<QString> cachedDirs = itemsCached.keys(); // seems slow, but there's no qcache iterator...
    foreach(const QString& cachedDir, cachedDirs) {
        if ( dir.isParentOf( KUrl( cachedDir ) ) )
            itemsCached.remove( cachedDir );
    }
}

void KDirListerCache::slotUpdateEntries( KIO::Job* job, const KIO::UDSEntryList& list )
{
    jobs[static_cast<KIO::ListJob*>(job)] += list;
}

void KDirListerCache::slotUpdateResult( KJob * j )
{
    Q_ASSERT( j );
    KIO::ListJob *job = static_cast<KIO::ListJob *>( j );

    KUrl jobUrl (joburl( job ));
    jobUrl.adjustPath(KUrl::RemoveTrailingSlash);  // need remove trailing slashes again, in case of redirections
    QString jobUrlStr (jobUrl.url());

    kDebug(7004) << "finished update" << jobUrl;

    KDirListerCacheDirectoryData& dirData = directoryData[jobUrlStr];
    // Collect the dirlisters which were listing the URL using that ListJob
    // plus those that were already holding that URL - they all get updated.
    dirData.moveListersWithoutCachedItemsJob();
    QList<KDirLister *> listers = dirData.listersCurrentlyHolding;
    listers += dirData.listersCurrentlyListing;

    // once we are updating dirs that are only in the cache this will fail!
    Q_ASSERT( !listers.isEmpty() );

    if ( job->error() ) {
        foreach ( KDirLister* kdl, listers ) {
            kdl->d->jobDone( job );

            //don't bother the user
            //kdl->handleError( job );

            emit kdl->canceled( jobUrl );
            if ( kdl->d->numJobs() == 0 ) {
                kdl->d->complete = true;
                emit kdl->canceled();
            }
        }

        jobs.remove( job );

        // TODO: if job is a parent of one or more
        // of the pending urls we should cancel them
        processPendingUpdates();
        return;
    }

    DirItem *dir = itemsInUse.value(jobUrlStr, 0);
    Q_ASSERT(dir);
    dir->complete = true;


    // check if anyone wants the mimetypes immediately
    bool delayedMimeTypes = true;
    foreach ( KDirLister *kdl, listers )
        delayedMimeTypes &= kdl->d->delayedMimeTypes;

    QHash<QString, KFileItem*> fileItems; // fileName -> KFileItem*

    // Unmark all items in url
    for ( KFileItemList::iterator kit = dir->lstItems.begin(), kend = dir->lstItems.end() ; kit != kend ; ++kit )
    {
        (*kit).unmark();
        fileItems.insert( (*kit).name(), &*kit );
    }

    KIO::UDSEntryList buf = jobs.value( job );
    KIO::UDSEntryList::const_iterator it = buf.begin();
    const KIO::UDSEntryList::const_iterator end = buf.end();
    for ( ; it != end; ++it )
    {
        // Form the complete url
        KFileItem item( *it, jobUrl, delayedMimeTypes, true );

        const QString name = item.name();
        Q_ASSERT( !name.isEmpty() );

        // we duplicate the check for dotdot here, to avoid iterating over
        // all items again and checking in matchesFilter() that way.
        if ( name.isEmpty() || name == ".." )
            continue;

        if ( name == "." )
        {
            // if the update was started before finishing the original listing
            // there is no root item yet
            if ( dir->rootItem.isNull() )
            {
                dir->rootItem = item;

                foreach ( KDirLister *kdl, listers )
                    if ( kdl->d->rootFileItem.isNull() && kdl->d->url == jobUrl )
                        kdl->d->rootFileItem = dir->rootItem;
            }
            continue;
        }

        // Find this item
        if (KFileItem* tmp = fileItems.value(item.name()))
        {
            // check if something changed for this file
            if ( !tmp->cmp( item ) )
            {
                foreach ( KDirLister *kdl, listers )
                    kdl->d->aboutToRefreshItem( *tmp );

                // kDebug(7004) << "slotUpdateResult: file changed: " << tmp->name();

                const KFileItem oldItem = *tmp;
                *tmp = item;
                foreach ( KDirLister *kdl, listers )
                    kdl->d->addRefreshItem( oldItem, *tmp );
            }
            tmp->mark();
        }
        else // this is a new file
        {
            // kDebug(7004) << "slotUpdateResult: new file: " << name;

            KFileItem pitem(item);
            pitem.mark();
            dir->lstItems.append( pitem );

            foreach ( KDirLister *kdl, listers )
                kdl->d->addNewItem( pitem );
        }
    }

    jobs.remove( job );

    deleteUnmarkedItems( listers, dir->lstItems );

    foreach ( KDirLister *kdl, listers ) {
        kdl->d->emitItems();

        kdl->d->jobDone( job );

        emit kdl->completed( jobUrl );
        if ( kdl->d->numJobs() == 0 )
        {
            kdl->d->complete = true;
            emit kdl->completed();
        }
    }

    // TODO: hmm, if there was an error and job is a parent of one or more
    // of the pending urls we should cancel it/them as well
    processPendingUpdates();
}

// private

KIO::ListJob *KDirListerCache::jobForUrl( const QString& url, KIO::ListJob *not_job )
{
  KIO::ListJob *job;
  QMap< KIO::ListJob *, KIO::UDSEntryList >::const_iterator it = jobs.begin();
  while ( it != jobs.end() )
  {
    job = it.key();
    if ( joburl( job ).url(KUrl::RemoveTrailingSlash) == url && job != not_job )
       return job;
    ++it;
  }
  return 0;
}

const KUrl& KDirListerCache::joburl( KIO::ListJob *job )
{
  if ( job->redirectionUrl().isValid() )
     return job->redirectionUrl();
  else
     return job->url();
}

void KDirListerCache::killJob( KIO::ListJob *job )
{
  jobs.remove( job );
  job->disconnect( this );
  job->kill();
}

void KDirListerCache::deleteUnmarkedItems( const QList<KDirLister *>& listers, KFileItemList &lstItems )
{
    KFileItemList deletedItems;
    // Find all unmarked items and delete them
    QMutableListIterator<KFileItem> kit(lstItems);
    while (kit.hasNext()) {
        const KFileItem item = kit.next();
        if (!item.isMarked()) {
            //kDebug() << item->name();
            deletedItems.append(item);
            kit.remove();
        }
    }
    if (!deletedItems.isEmpty())
        itemsDeleted(listers, deletedItems);
}

void KDirListerCache::itemsDeleted(const QList<KDirLister *>& listers, const KFileItemList& deletedItems)
{
    Q_FOREACH(KDirLister *kdl, listers) {
        kdl->d->emitItemsDeleted(deletedItems);
    }

    Q_FOREACH(const KFileItem& item, deletedItems) {
        if (item.isDir())
            deleteDir(item.url());
    }
}

void KDirListerCache::deleteDir( const KUrl& dirUrl )
{
    //kDebug() << dirUrl;
    // unregister and remove the children of the deleted item.
    // Idea: tell all the KDirListers that they should forget the dir
    //       and then remove it from the cache.

    // Separate itemsInUse iteration and calls to forgetDirs (which modify itemsInUse)
    KUrl::List affectedItems;

    QHash<QString, DirItem *>::iterator itu = itemsInUse.begin();
    const QHash<QString, DirItem *>::iterator ituend = itemsInUse.end();
    for ( ; itu != ituend; ++itu ) {
        const KUrl deletedUrl( itu.key() );
        if ( dirUrl.isParentOf( deletedUrl ) ) {
            affectedItems.append(deletedUrl);
        }
    }

    foreach(const KUrl& deletedUrl, affectedItems) {
        const QString deletedUrlStr = deletedUrl.url();
        // stop all jobs for deletedUrlStr
        DirectoryDataHash::iterator dit = directoryData.find(deletedUrlStr);
        if (dit != directoryData.end()) {
            // we need a copy because stop modifies the list
            QList<KDirLister *> listers = (*dit).listersCurrentlyListing;
            foreach ( KDirLister *kdl, listers )
                stop( kdl, deletedUrl );
            // tell listers holding deletedUrl to forget about it
            // this will stop running updates for deletedUrl as well

            // we need a copy because forgetDirs modifies the list
            QList<KDirLister *> holders = (*dit).listersCurrentlyHolding;
            foreach ( KDirLister *kdl, holders ) {
                // lister's root is the deleted item
                if ( kdl->d->url == deletedUrl )
                {
                    // tell the view first. It might need the subdirs' items (which forgetDirs will delete)
                    if ( !kdl->d->rootFileItem.isNull() ) {
                        emit kdl->deleteItem( kdl->d->rootFileItem );
                        emit kdl->itemsDeleted(KFileItemList() << kdl->d->rootFileItem);
                    }
                    forgetDirs( kdl );
                    kdl->d->rootFileItem = KFileItem();
                }
                else
                {
                    const bool treeview = kdl->d->lstDirs.count() > 1;
                    if ( !treeview )
                    {
                        emit kdl->clear();
                        kdl->d->lstDirs.clear();
                    }
                    else
                        kdl->d->lstDirs.removeAll( deletedUrl );

                    forgetDirs( kdl, deletedUrl, treeview );
                }
            }
        }

        // delete the entry for deletedUrl - should not be needed, it's in
        // items cached now
        int count = itemsInUse.remove( deletedUrlStr );
        Q_ASSERT( count == 0 );
        Q_UNUSED( count ); //keep gcc "unused variable" complaining quiet when in release mode
    }

    // remove the children from the cache
    removeDirFromCache( dirUrl );
}

// delayed updating of files, FAM is flooding us with events
void KDirListerCache::processPendingUpdates()
{
    foreach(const QString& file, pendingUpdates) {
        kDebug(7004) << file;
        KUrl u(file);
        KFileItem *item = findByUrl( 0, u ); // search all items
        if ( item ) {
            // we need to refresh the item, because e.g. the permissions can have changed.
            aboutToRefreshItem( *item );
            KFileItem oldItem = *item;
            item->refresh();
            emitRefreshItem( oldItem, *item );
        }
    }
    pendingUpdates.clear();
}

#ifndef NDEBUG
void KDirListerCache::printDebug()
{
    kDebug(7004) << "Items in use:";
    QHash<QString, DirItem *>::const_iterator itu = itemsInUse.begin();
    const QHash<QString, DirItem *>::const_iterator ituend = itemsInUse.end();
    for ( ; itu != ituend ; ++itu ) {
        kDebug(7004) << "   " << itu.key() << "URL:" << itu.value()->url
                     << "rootItem:" << ( !itu.value()->rootItem.isNull() ? itu.value()->rootItem.url() : KUrl() )
                     << "autoUpdates refcount:" << itu.value()->autoUpdates
                     << "complete:" << itu.value()->complete
                     << QString("with %1 items.").arg(itu.value()->lstItems.count());
    }

    kDebug(7004) << "Directory data:";
    DirectoryDataHash::const_iterator dit = directoryData.begin();
    for ( ; dit != directoryData.end(); ++dit )
    {
        QString list;
        foreach ( KDirLister* listit, (*dit).listersCurrentlyListing )
            list += " 0x" + QString::number( (qlonglong)listit, 16 );
        kDebug(7004) << "  " << dit.key() << (*dit).listersCurrentlyListing.count() << "listers:" << list;
        foreach ( KDirLister* listit, (*dit).listersCurrentlyListing ) {
            if (listit->d->m_cachedItemsJob) {
                kDebug(7004) << "  Lister" << listit << "has CachedItemsJob" << listit->d->m_cachedItemsJob;
            }
        }

        list.clear();
        foreach ( KDirLister* listit, (*dit).listersCurrentlyHolding )
            list += " 0x" + QString::number( (qlonglong)listit, 16 );
        kDebug(7004) << "  " << dit.key() << (*dit).listersCurrentlyHolding.count() << "holders:" << list;
    }

    QMap< KIO::ListJob *, KIO::UDSEntryList >::Iterator jit = jobs.begin();
    kDebug(7004) << "Jobs:";
    for ( ; jit != jobs.end() ; ++jit )
        kDebug(7004) << "   " << jit.key() << "listing" << joburl( jit.key() ) << ":" << (*jit).count() << "entries.";

    kDebug(7004) << "Items in cache:";
    const QList<QString> cachedDirs = itemsCached.keys();
    foreach(const QString& cachedDir, cachedDirs) {
        DirItem* dirItem = itemsCached.object(cachedDir);
        kDebug(7004) << "   " << cachedDir << "rootItem:"
                     << (!dirItem->rootItem.isNull() ? dirItem->rootItem.url().prettyUrl() : QString("NULL") )
                     << "with" << dirItem->lstItems.count() << "items.";
    }
}
#endif

/*********************** -- The new KDirLister -- ************************/


KDirLister::KDirLister( QObject* parent )
    : QObject(parent), d(new Private(this))
{
    //kDebug(7003) << "+KDirLister";

    d->complete = true;

    setAutoUpdate( true );
    setDirOnlyMode( false );
    setShowingDotFiles( false );

    setAutoErrorHandlingEnabled( true, 0 );
}

KDirLister::~KDirLister()
{
    //kDebug(7003) << "-KDirLister";

    // Stop all running jobs
    if (!kDirListerCache.isDestroyed()) {
        stop();
        kDirListerCache->forgetDirs( this );
    }

    delete d;
}

bool KDirLister::openUrl( const KUrl& _url, OpenUrlFlags _flags )
{
    // emit the current changes made to avoid an inconsistent treeview
    if ( d->changes != NONE && ( _flags & Keep ) )
        emitChanges();

    d->changes = NONE;

    return kDirListerCache->listDir( this, _url, _flags & Keep, _flags & Reload );
}

void KDirLister::stop()
{
    kDirListerCache->stop( this );
}

void KDirLister::stop( const KUrl& _url )
{
    kDirListerCache->stop( this, _url );
}

bool KDirLister::autoUpdate() const
{
    return d->autoUpdate;
}

void KDirLister::setAutoUpdate( bool _enable )
{
    if ( d->autoUpdate == _enable )
        return;

    d->autoUpdate = _enable;
    kDirListerCache->setAutoUpdate( this, _enable );
}

bool KDirLister::showingDotFiles() const
{
  return d->isShowingDotFiles;
}

void KDirLister::setShowingDotFiles( bool _showDotFiles )
{
  if ( d->isShowingDotFiles == _showDotFiles )
    return;

  d->isShowingDotFiles = _showDotFiles;
  d->changes ^= DOT_FILES;
}

bool KDirLister::dirOnlyMode() const
{
  return d->dirOnlyMode;
}

void KDirLister::setDirOnlyMode( bool _dirsOnly )
{
  if ( d->dirOnlyMode == _dirsOnly )
    return;

  d->dirOnlyMode = _dirsOnly;
  d->changes ^= DIR_ONLY_MODE;
}

bool KDirLister::autoErrorHandlingEnabled() const
{
  return d->autoErrorHandling;
}

void KDirLister::setAutoErrorHandlingEnabled( bool enable, QWidget* parent )
{
  d->autoErrorHandling = enable;
  d->errorParent = parent;
}

KUrl KDirLister::url() const
{
  return d->url;
}

KUrl::List KDirLister::directories() const
{
  return d->lstDirs;
}

void KDirLister::emitChanges()
{
  if ( d->changes == NONE )
    return;

  for ( KUrl::List::Iterator it = d->lstDirs.begin();
        it != d->lstDirs.end(); ++it )
  {
    KFileItemList deletedItems;

    const KFileItemList* itemList = kDirListerCache->itemsForDir( *it );
    KFileItemList::const_iterator kit = itemList->begin();
    const KFileItemList::const_iterator kend = itemList->end();
    for ( ; kit != kend; ++kit )
    {
      if ( (*kit).text() == "." || (*kit).text() == ".." )
        continue;

      bool oldMime = true, newMime = true;

      if ( d->changes & MIME_FILTER )
      {
        const QString mimetype = (*kit).mimetype();
        oldMime = doMimeFilter( mimetype, d->oldMimeFilter )
                && d->doMimeExcludeFilter( mimetype, d->oldMimeExcludeFilter );
        newMime = doMimeFilter( mimetype, d->mimeFilter )
                && d->doMimeExcludeFilter( mimetype, d->mimeExcludeFilter );

        if ( oldMime && !newMime )
        {
          deletedItems.append(*kit);
          continue;
        }
      }

      if ( d->changes & DIR_ONLY_MODE )
      {
        // the lister switched to dirOnlyMode
        if ( d->dirOnlyMode )
        {
          if ( !(*kit).isDir() )
          {
              deletedItems.append(*kit);
          }
        }
        else if ( !(*kit).isDir() )
          d->addNewItem( *kit );

        continue;
      }

      if ( (*kit).isHidden() )
      {
        if ( d->changes & DOT_FILES )
        {
          // the lister switched to dot files mode
          if ( d->isShowingDotFiles )
            d->addNewItem( *kit );
          else
          {
              deletedItems.append(*kit);
          }

          continue;
        }
      }
      else if ( d->changes & NAME_FILTER )
      {
        bool oldName = (*kit).isDir() ||
                       d->oldFilters.isEmpty() ||
                       doNameFilter( (*kit).text(), d->oldFilters );

        bool newName = (*kit).isDir() ||
                       d->lstFilters.isEmpty() ||
                       doNameFilter( (*kit).text(), d->lstFilters );

        if ( oldName && !newName )
        {
          deletedItems.append(*kit);
          continue;
        }
        else if ( !oldName && newName )
          d->addNewItem( *kit );
      }

      if ( (d->changes & MIME_FILTER) && !oldMime && newMime )
        d->addNewItem( *kit );
    }

    if (!deletedItems.isEmpty()) {
        emit itemsDeleted(deletedItems);
        // for compat
        Q_FOREACH(const KFileItem& item, deletedItems)
            emit deleteItem(item);
    }
    d->emitItems();
  }

  d->changes = NONE;
}

void KDirLister::updateDirectory( const KUrl& _u )
{
  kDirListerCache->updateDirectory( _u );
}

bool KDirLister::isFinished() const
{
  return d->complete;
}

KFileItem KDirLister::rootItem() const
{
  return d->rootFileItem;
}

KFileItem KDirLister::findByUrl( const KUrl& _url ) const
{
  KFileItem *item = kDirListerCache->findByUrl( this, _url );
  if (item) {
      return *item;
  } else {
      return KFileItem();
  }
}

KFileItem KDirLister::findByName( const QString& _name ) const
{
  return kDirListerCache->findByName( this, _name );
}


// ================ public filter methods ================ //

void KDirLister::setNameFilter( const QString& nameFilter )
{
  if ( !(d->changes & NAME_FILTER) )
  {
    d->oldFilters = d->lstFilters;
  }

  d->lstFilters.clear();

  d->nameFilter = nameFilter;

  // Split on white space
  const QStringList list = nameFilter.split( ' ', QString::SkipEmptyParts );
  for ( QStringList::const_iterator it = list.begin(); it != list.end(); ++it )
    d->lstFilters.append( QRegExp(*it, Qt::CaseInsensitive, QRegExp::Wildcard ) );

  d->changes |= NAME_FILTER;
}

QString KDirLister::nameFilter() const
{
  return d->nameFilter;
}

void KDirLister::setMimeFilter( const QStringList& mimeFilter )
{
  if ( !(d->changes & MIME_FILTER) )
    d->oldMimeFilter = d->mimeFilter;

  if ( mimeFilter.contains("application/octet-stream") ) // all files
    d->mimeFilter.clear();
  else
    d->mimeFilter = mimeFilter;

  d->changes |= MIME_FILTER;
}

void KDirLister::setMimeExcludeFilter( const QStringList& mimeExcludeFilter )
{
  if ( !(d->changes & MIME_FILTER) )
    d->oldMimeExcludeFilter = d->mimeExcludeFilter;

  d->mimeExcludeFilter = mimeExcludeFilter;
  d->changes |= MIME_FILTER;
}


void KDirLister::clearMimeFilter()
{
  if ( !(d->changes & MIME_FILTER) )
  {
    d->oldMimeFilter = d->mimeFilter;
    d->oldMimeExcludeFilter = d->mimeExcludeFilter;
  }
  d->mimeFilter.clear();
  d->mimeExcludeFilter.clear();
  d->changes |= MIME_FILTER;
}

QStringList KDirLister::mimeFilters() const
{
  return d->mimeFilter;
}

bool KDirLister::matchesFilter( const QString& name ) const
{
  return doNameFilter( name, d->lstFilters );
}

bool KDirLister::matchesMimeFilter( const QString& mime ) const
{
  return doMimeFilter( mime, d->mimeFilter ) && d->doMimeExcludeFilter(mime,d->mimeExcludeFilter);
}

// ================ protected methods ================ //

bool KDirLister::matchesFilter( const KFileItem& item ) const
{
  Q_ASSERT( !item.isNull() );

  if ( item.text() == ".." )
    return false;

  if ( !d->isShowingDotFiles && item.isHidden() )
    return false;

  if ( item.isDir() || d->lstFilters.isEmpty() )
    return true;

  return matchesFilter( item.text() );
}

bool KDirLister::matchesMimeFilter( const KFileItem& item ) const
{
  Q_ASSERT( !item.isNull() );
  // Don't lose time determining the mimetype if there is no filter
  if ( d->mimeFilter.isEmpty() && d->mimeExcludeFilter.isEmpty() )
      return true;
  return matchesMimeFilter( item.mimetype() );
}

bool KDirLister::doNameFilter( const QString& name, const QList<QRegExp>& filters ) const
{
  for ( QList<QRegExp>::const_iterator it = filters.begin(); it != filters.end(); ++it )
    if ( (*it).exactMatch( name ) )
      return true;

  return false;
}

bool KDirLister::doMimeFilter( const QString& mime, const QStringList& filters ) const
{
  if ( filters.isEmpty() )
    return true;

  const KMimeType::Ptr mimeptr = KMimeType::mimeType(mime);
  if ( !mimeptr )
    return false;

  //kDebug(7004) << "doMimeFilter: investigating: "<<mimeptr->name();
  QStringList::const_iterator it = filters.begin();
  for ( ; it != filters.end(); ++it )
    if ( mimeptr->is(*it) )
      return true;
    //else   kDebug(7004) << "doMimeFilter: compared without result to  "<<*it;

  return false;
}

bool KDirLister::Private::doMimeExcludeFilter( const QString& mime, const QStringList& filters ) const
{
  if ( filters.isEmpty() )
    return true;

  QStringList::const_iterator it = filters.begin();
  for ( ; it != filters.end(); ++it )
    if ( (*it) == mime )
      return false;

  return true;
}

void KDirLister::handleError( KIO::Job *job )
{
  if ( d->autoErrorHandling )
    job->uiDelegate()->showErrorMessage();
}


// ================= private methods ================= //

void KDirLister::Private::addNewItem( const KFileItem &item )
{
  if ( ( dirOnlyMode && !item.isDir() ) || !m_parent->matchesFilter( item ) )
    return; // No reason to continue... bailing out here prevents a mimetype scan.

  if ( m_parent->matchesMimeFilter( item ) )
  {
    if ( !lstNewItems )
    {
      lstNewItems = new KFileItemList;
    }

    Q_ASSERT( !item.isNull() );
    lstNewItems->append( item );            // items not filtered
  }
  else
  {
    if ( !lstMimeFilteredItems ) {
      lstMimeFilteredItems = new KFileItemList;
    }

    Q_ASSERT( !item.isNull() );
    lstMimeFilteredItems->append( item );   // only filtered by mime
  }
}

void KDirLister::Private::addNewItems( const KFileItemList& items )
{
  // TODO: make this faster - test if we have a filter at all first
  // DF: was this profiled? The matchesFoo() functions should be fast, w/o filters...
  // Of course if there is no filter and we can do a range-insertion instead of a loop, that might be good.
  KFileItemList::const_iterator kit = items.begin();
  const KFileItemList::const_iterator kend = items.end();
  for ( ; kit != kend; ++kit )
    addNewItem( *kit );
}

void KDirLister::Private::aboutToRefreshItem( const KFileItem &item )
{
  // The code here follows the logic in addNewItem
  if ( ( dirOnlyMode && !item.isDir() ) || !m_parent->matchesFilter( item ) )
    refreshItemWasFiltered = true;
  else if ( !m_parent->matchesMimeFilter( item ) )
    refreshItemWasFiltered = true;
  else
    refreshItemWasFiltered = false;
}

void KDirLister::Private::addRefreshItem( const KFileItem& oldItem, const KFileItem& item )
{
  bool isExcluded = (dirOnlyMode && !item.isDir()) || !m_parent->matchesFilter( item );

  if ( !isExcluded && m_parent->matchesMimeFilter( item ) )
  {
    if ( refreshItemWasFiltered )
    {
      if ( !lstNewItems ) {
        lstNewItems = new KFileItemList;
      }

      Q_ASSERT( !item.isNull() );
      lstNewItems->append( item );
    }
    else
    {
      if ( !lstRefreshItems ) {
        lstRefreshItems = new QList<QPair<KFileItem,KFileItem> >;
      }

      Q_ASSERT( !item.isNull() );
      lstRefreshItems->append( qMakePair(oldItem, item) );
    }
  }
  else if ( !refreshItemWasFiltered )
  {
    if ( !lstRemoveItems ) {
      lstRemoveItems = new KFileItemList;
    }

    // notify the user that the mimetype of a file changed that doesn't match
    // a filter or does match an exclude filter
    Q_ASSERT( !item.isNull() );
    lstRemoveItems->append( item );
  }
}

void KDirLister::Private::emitItems()
{
  KFileItemList *tmpNew = lstNewItems;
  lstNewItems = 0;

  KFileItemList *tmpMime = lstMimeFilteredItems;
  lstMimeFilteredItems = 0;

  QList<QPair<KFileItem, KFileItem> > *tmpRefresh = lstRefreshItems;
  lstRefreshItems = 0;

  KFileItemList *tmpRemove = lstRemoveItems;
  lstRemoveItems = 0;

  if ( tmpNew )
  {
    emit m_parent->newItems( *tmpNew );
    delete tmpNew;
  }

  if ( tmpMime )
  {
    emit m_parent->itemsFilteredByMime( *tmpMime );
    delete tmpMime;
  }

  if ( tmpRefresh )
  {
    emit m_parent->refreshItems( *tmpRefresh );
    delete tmpRefresh;
  }

  if ( tmpRemove )
  {
      emit m_parent->itemsDeleted( *tmpRemove );
      delete tmpRemove;
  }
}

void KDirLister::Private::emitDeleteItem( const KFileItem &item )
{
  if ( ( dirOnlyMode && !item.isDir() ) || !m_parent->matchesFilter( item ) )
    return; // No reason to continue... bailing out here prevents a mimetype scan.
  if ( m_parent->matchesMimeFilter( item ) )
  {
    emit m_parent->deleteItem( item );
  }
}

void KDirLister::Private::emitItemsDeleted(const KFileItemList &_items)
{
    KFileItemList items = _items;
    QMutableListIterator<KFileItem> it(items);
    while (it.hasNext()) {
        const KFileItem& item = it.next();
        if ((dirOnlyMode && !item.isDir())
            || !m_parent->matchesFilter(item)
            || !m_parent->matchesMimeFilter(item) ) { // do this one last
            it.remove();
        } else {
            // for compat
            emit m_parent->deleteItem(item);
        }
    }
    if (!items.isEmpty())
        emit m_parent->itemsDeleted(items);
}

// ================ private slots ================ //

void KDirLister::Private::_k_slotInfoMessage( KJob *, const QString& message )
{
  emit m_parent->infoMessage( message );
}

void KDirLister::Private::_k_slotPercent( KJob *job, unsigned long pcnt )
{
  jobData[static_cast<KIO::ListJob *>(job)].percent = pcnt;

  int result = 0;

  KIO::filesize_t size = 0;

  QMap< KIO::ListJob *, Private::JobData >::Iterator dataIt = jobData.begin();
  while ( dataIt != jobData.end() )
  {
    result += (*dataIt).percent * (*dataIt).totalSize;
    size += (*dataIt).totalSize;
    ++dataIt;
  }

  if ( size != 0 )
    result /= size;
  else
    result = 100;
  emit m_parent->percent( result );
}

void KDirLister::Private::_k_slotTotalSize( KJob *job, qulonglong size )
{
  jobData[static_cast<KIO::ListJob *>(job)].totalSize = size;

  KIO::filesize_t result = 0;
  QMap< KIO::ListJob *, Private::JobData >::Iterator dataIt = jobData.begin();
  while ( dataIt != jobData.end() )
  {
    result += (*dataIt).totalSize;
    ++dataIt;
  }

  emit m_parent->totalSize( result );
}

void KDirLister::Private::_k_slotProcessedSize( KJob *job, qulonglong size )
{
  jobData[static_cast<KIO::ListJob *>(job)].processedSize = size;

  KIO::filesize_t result = 0;
  QMap< KIO::ListJob *, Private::JobData >::Iterator dataIt = jobData.begin();
  while ( dataIt != jobData.end() )
  {
    result += (*dataIt).processedSize;
    ++dataIt;
  }

  emit m_parent->processedSize( result );
}

void KDirLister::Private::_k_slotSpeed( KJob *job, unsigned long spd )
{
  jobData[static_cast<KIO::ListJob *>(job)].speed = spd;

  int result = 0;
  QMap< KIO::ListJob *, Private::JobData >::Iterator dataIt = jobData.begin();
  while ( dataIt != jobData.end() )
  {
    result += (*dataIt).speed;
    ++dataIt;
  }

  emit m_parent->speed( result );
}

uint KDirLister::Private::numJobs()
{
  return jobData.count();
}

void KDirLister::Private::jobDone( KIO::ListJob *job )
{
  jobData.remove( job );
}

void KDirLister::Private::jobStarted( KIO::ListJob *job )
{
  Private::JobData data;
  data.speed = 0;
  data.percent = 0;
  data.processedSize = 0;
  data.totalSize = 0;

  jobData.insert( job, data );
  complete = false;
}

void KDirLister::Private::connectJob( KIO::ListJob *job )
{
  m_parent->connect( job, SIGNAL(infoMessage( KJob *, const QString&, const QString& )),
                     m_parent, SLOT(_k_slotInfoMessage( KJob *, const QString& )) );
  m_parent->connect( job, SIGNAL(percent( KJob *, unsigned long )),
                     m_parent, SLOT(_k_slotPercent( KJob *, unsigned long )) );
  m_parent->connect( job, SIGNAL(totalSize( KJob *, qulonglong )),
                     m_parent, SLOT(_k_slotTotalSize( KJob *, qulonglong )) );
  m_parent->connect( job, SIGNAL(processedSize( KJob *, qulonglong )),
                     m_parent, SLOT(_k_slotProcessedSize( KJob *, qulonglong )) );
  m_parent->connect( job, SIGNAL(speed( KJob *, unsigned long )),
                     m_parent, SLOT(_k_slotSpeed( KJob *, unsigned long )) );
}

void KDirLister::setMainWindow( QWidget *window )
{
  d->window = window;
}

QWidget *KDirLister::mainWindow()
{
  return d->window;
}

KFileItemList KDirLister::items( WhichItems which ) const
{
    return itemsForDir( url(), which );
}

KFileItemList KDirLister::itemsForDir( const KUrl& dir, WhichItems which ) const
{
    KFileItemList *allItems = kDirListerCache->itemsForDir( dir );
    if ( !allItems )
        return KFileItemList();

    if ( which == AllItems )
        return *allItems;
    else // only items passing the filters
    {
        KFileItemList result;
        KFileItemList::const_iterator kit = allItems->begin();
        const KFileItemList::const_iterator kend = allItems->end();
        for ( ; kit != kend; ++kit )
        {
            KFileItem item = *kit;
            bool isExcluded = (d->dirOnlyMode && !item.isDir()) || !matchesFilter( item );
            if ( !isExcluded && matchesMimeFilter( item ) )
                result.append( item );
        }
        return result;
    }
}

bool KDirLister::delayedMimeTypes() const
{
    return d->delayedMimeTypes;
}

void KDirLister::setDelayedMimeTypes( bool delayedMimeTypes )
{
    d->delayedMimeTypes = delayedMimeTypes;
}

// called by KDirListerCache::slotRedirection
void KDirLister::Private::redirect( const KUrl& oldUrl, const KUrl& newUrl )
{
    if ( url.equals( oldUrl, KUrl::CompareWithoutTrailingSlash ) ) {
        rootFileItem = KFileItem();
        url = newUrl;
    }

    kDebug() << "LSTDIRS" << lstDirs;
    kDebug() << "OLD" << oldUrl;
    kDebug() << "NEW" << newUrl;
    kDebug() << "IDX" << lstDirs.indexOf( oldUrl );
    kDebug() << "LSTCOUNT" << lstDirs.count();
    kDebug() << oldUrl.toEncoded();
    kDebug() << lstDirs.at(0).toEncoded();
    lstDirs[ lstDirs.indexOf( oldUrl ) ] = newUrl;

    if ( lstDirs.count() == 1 ) {
        emit m_parent->clear();
        emit m_parent->redirection( newUrl );
        emit m_parent->redirection( oldUrl, newUrl );
    } else {
        emit m_parent->clear( oldUrl );
        emit m_parent->redirection( oldUrl, newUrl );
    }
}

void KDirListerCacheDirectoryData::moveListersWithoutCachedItemsJob()
{
    // Move dirlisters from listersCurrentlyListing to listersCurrentlyHolding,
    // but not those that are still waiting on a CachedItemsJob...
    // Unit-testing note:
    // Run kdirmodeltest in valgrind to hit the case where an update
    // is triggered while a lister has a CachedItemsJob (different timing...)
    QMutableListIterator<KDirLister *> lister_it(listersCurrentlyListing);
    while (lister_it.hasNext()) {
        KDirLister* kdl = lister_it.next();
        if (!kdl->d->m_cachedItemsJob) {
            Q_ASSERT(!listersCurrentlyHolding.contains(kdl));
            // OK, move this lister from "currently listing" to "currently holding".
            listersCurrentlyHolding.append(kdl);
            lister_it.remove();
        }
    }
}

KFileItem KDirLister::cachedItemForUrl(const KUrl& url)
{
    return kDirListerCache->itemForUrl(url);
}

#include "kdirlister.moc"
#include "kdirlister_p.moc"
