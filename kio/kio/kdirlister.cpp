/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
                 2000 Carsten Pfeiffer <pfeiffer@kde.org>
                 2001, 2002 Michael Brade <brade@kde.org>

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

#include "kdirlister.h"

#include <qregexp.h>
#include <qptrlist.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kio/job.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kstaticdeleter.h>
#include <kdebugclasses.h>

#include "kdirlister_p.h"

#include <assert.h>

KDirListerCache* KDirListerCache::s_pSelf = 0;
static KStaticDeleter<KDirListerCache> sd_KDirListerCache;

// Enable this to get printDebug() called often, to see the contents of the cache
//#define DEBUG_CACHE

// Make really sure it doesn't get activated in the final build
#ifdef NDEBUG
#undef DEBUG_CACHE
#endif

KDirListerCache::KDirListerCache( int maxCount )
  : itemsCached( maxCount )
{
  kdDebug(7004) << "+KDirListerCache" << endl;

  itemsInUse.setAutoDelete( false );
  itemsCached.setAutoDelete( true );
  urlsCurrentlyListed.setAutoDelete( true );
  urlsCurrentlyHeld.setAutoDelete( true );

  connect( kdirwatch, SIGNAL( dirty( const QString& ) ),
           this, SLOT( slotFileDirty( const QString& ) ) );
  connect( kdirwatch, SIGNAL( created( const QString& ) ),
           this, SLOT( slotFileCreated( const QString& ) ) );
  connect( kdirwatch, SIGNAL( deleted( const QString& ) ),
           this, SLOT( slotFileDeleted( const QString& ) ) );
}

KDirListerCache::~KDirListerCache()
{
  kdDebug(7004) << "-KDirListerCache" << endl;

  itemsInUse.setAutoDelete( true );
  itemsInUse.clear();
  itemsCached.clear();
  urlsCurrentlyListed.clear();
  urlsCurrentlyHeld.clear();

  kdirwatch->disconnect( this );
}

// setting _reload to true will emit the old files and
// call updateDirectory
void KDirListerCache::listDir( KDirLister* lister, const KURL& _u,
                               bool _keep, bool _reload )
{
  // like this we don't have to worry about trailing slashes any further
  KURL _url = _u;
  _url.cleanPath(); // kill consecutive slashes
  _url.adjustPath(-1);

#ifdef DEBUG_CACHE
  printDebug();
#endif
  kdDebug(7004) << k_funcinfo << lister << " url=" << _url
                << " keep=" << _keep << " reload=" << _reload << endl;
  /*if ( !_keep ) {
    for ( KURL::List::Iterator it = lister->d->lstDirs.begin();
          it != lister->d->lstDirs.end(); ++it )
          kdDebug(7004) << " Dir for this lister:" << (*it) << endl;
  }*/
  if ( !_keep )
  {
    // stop any running jobs for lister
    stop( lister );

    // clear our internal list for lister
    forgetDirs( lister );

    lister->d->rootFileItem = 0;
  }
  else if ( lister->d->lstDirs.contains( _url ) )
  {
    // stop the job listing _url for this lister
    stop( lister, _url );

    // clear _url for lister
    forgetDirs( lister, _url, true );

    if ( lister->d->url == _url )
      lister->d->rootFileItem = 0;
  }

  lister->d->lstDirs.append( _url );

  if ( lister->d->url.isEmpty() || !_keep ) // set toplevel URL only if not set yet
    lister->d->url = _url;

  DirItem *itemU = itemsInUse[_url.url()];
  DirItem *itemC;

  if ( !urlsCurrentlyListed[_url.url()] )
  {
    // if there is an update running for _url already we get into
    // the following case - it will just be restarted by updateDirectory().

    if ( itemU )
    {
      kdDebug(7004) << "listDir: Entry already in use: " << _url << endl;

      bool oldState = lister->d->complete;
      lister->d->complete = false;

      emit lister->started( _url );

      if ( !lister->d->rootFileItem && lister->d->url == _url )
        lister->d->rootFileItem = itemU->rootItem;

      lister->addNewItems( *(itemU->lstItems) );
      lister->emitItems();

      lister->d->complete = oldState;

      emit lister->completed( _url );
      if ( lister->d->complete )
        emit lister->completed();

      // _url is already in use, so there is already an entry in urlsCurrentlyHeld
      assert( urlsCurrentlyHeld[_url.url()] );
      urlsCurrentlyHeld[_url.url()]->append( lister );

      if ( _reload || !itemU->complete )
        updateDirectory( _url );
    }
    else if ( !_reload && (itemC = itemsCached.take( _url.url() )) )
    {
      kdDebug(7004) << "listDir: Entry in cache: " << _url << endl;

      itemC->decAutoUpdate();
      itemsInUse.insert( _url.url(), itemC );
      itemU = itemC;

      bool oldState = lister->d->complete;
      lister->d->complete = false;

      emit lister->started( _url );

      if ( !lister->d->rootFileItem && lister->d->url == _url )
        lister->d->rootFileItem = itemC->rootItem;

      lister->addNewItems( *(itemC->lstItems) );
      lister->emitItems();

      lister->d->complete = oldState;

      emit lister->completed( _url );
      if ( lister->d->complete )
        emit lister->completed();

      Q_ASSERT( !urlsCurrentlyHeld[_url.url()] );
      QPtrList<KDirLister> *list = new QPtrList<KDirLister>;
      list->append( lister );
      urlsCurrentlyHeld.insert( _url.url(), list );

      if ( !itemC->complete )
        updateDirectory( _url );
    }
    else  // dir not in cache or _reload is true
    {
      kdDebug(7004) << "listDir: Entry not in cache or reloaded: " << _url << endl;

      QPtrList<KDirLister> *list = new QPtrList<KDirLister>;
      list->append( lister );
      urlsCurrentlyListed.insert( _url.url(), list );

      itemsCached.remove( _url.url() );
      itemU = new DirItem( _url );
      itemsInUse.insert( _url.url(), itemU );

//        // we have a limit of MAX_JOBS_PER_LISTER concurrently running jobs
//        if ( lister->d->numJobs >= MAX_JOBS_PER_LISTER )
//        {
//          lstPendingUpdates.append( _url );
//        }
//        else
//        {

      if ( lister->d->url == _url )
        lister->d->rootFileItem = 0;

      lister->d->complete = false;
      lister->d->numJobs++;

      KIO::ListJob* job = KIO::listDir( _url, false /* no default GUI */ );
      jobs.insert( job, QValueList<KIO::UDSEntry>() );

      connect( job, SIGNAL( entries( KIO::Job *, const KIO::UDSEntryList & ) ),
               this, SLOT( slotEntries( KIO::Job *, const KIO::UDSEntryList & ) ) );
      connect( job, SIGNAL( result( KIO::Job * ) ),
               this, SLOT( slotResult( KIO::Job * ) ) );
      connect( job, SIGNAL( redirection( KIO::Job *, const KURL & ) ),
               this, SLOT( slotRedirection( KIO::Job *, const KURL & ) ) );

      connect( job, SIGNAL( infoMessage( KIO::Job *, const QString& ) ),
               lister, SLOT( slotInfoMessage( KIO::Job *, const QString& ) ) );
      connect( job, SIGNAL( percent( KIO::Job *, unsigned long ) ),
               lister, SLOT( slotPercent( KIO::Job *, unsigned long ) ) );
      connect( job, SIGNAL( totalSize( KIO::Job *, KIO::filesize_t ) ),
               lister, SLOT( slotTotalSize( KIO::Job *, KIO::filesize_t ) ) );
      connect( job, SIGNAL( processedSize( KIO::Job *, KIO::filesize_t ) ),
               lister, SLOT( slotProcessedSize( KIO::Job *, KIO::filesize_t ) ) );
      connect( job, SIGNAL( speed( KIO::Job *, unsigned long ) ),
               lister, SLOT( slotSpeed( KIO::Job *, unsigned long ) ) );

      emit lister->started( _url );

//        }
    }
  }
  else
  {
    kdDebug(7004) << k_funcinfo << "Entry currently being listed: " << _url << endl;

    emit lister->started( _url );

    lister->d->complete = false;
    lister->d->numJobs++;
    urlsCurrentlyListed[_url.url()]->append( lister );

    QMap< KIO::ListJob *, QValueList<KIO::UDSEntry> >::Iterator it = jobs.begin();
    //kdDebug(7004) << "KDirListerCache::listDir " << jobs.count() << " jobs" << endl;
    while ( it != jobs.end() )
    {
      //kdDebug(7004) << "KDirListerCache::listDir looking at job for " << it.key()->url() << endl;
      if ( it.key()->url() == _url )
        break;

      ++it;
    }
    Q_ASSERT( it != jobs.end() );

    KIO::ListJob *job = it.key();
    connect( job, SIGNAL( infoMessage( KIO::Job *, const QString& ) ),
             lister, SLOT( slotInfoMessage( KIO::Job *, const QString& ) ) );
    connect( job, SIGNAL( percent( KIO::Job *, unsigned long ) ),
             lister, SLOT( slotPercent( KIO::Job *, unsigned long ) ) );
    connect( job, SIGNAL( totalSize( KIO::Job *, KIO::filesize_t ) ),
             lister, SLOT( slotTotalSize( KIO::Job *, KIO::filesize_t ) ) );
    connect( job, SIGNAL( processedSize( KIO::Job *, KIO::filesize_t ) ),
             lister, SLOT( slotProcessedSize( KIO::Job *, KIO::filesize_t ) ) );
    connect( job, SIGNAL( speed( KIO::Job *, unsigned long ) ),
             lister, SLOT( slotSpeed( KIO::Job *, unsigned long ) ) );

    Q_ASSERT( itemU );

    if ( !lister->d->rootFileItem && lister->d->url == _url )
      lister->d->rootFileItem = itemU->rootItem;

    lister->addNewItems( *(itemU->lstItems) );
    lister->emitItems();
  }

  // automatic updating of directories
  if ( lister->d->autoUpdate )
    itemU->incAutoUpdate();
}

void KDirListerCache::stop( KDirLister *lister )
{
#ifdef DEBUG_CACHE
  printDebug();
#endif
  kdDebug(7004) << k_funcinfo << "lister: " << lister << endl;
  bool stopped = false;

  QDictIterator< QPtrList<KDirLister> > it( urlsCurrentlyListed );
  while ( it.current() )
  {
    if ( it.current()->findRef( lister ) > -1 )
    {
      // lister is listing url
      QString url = it.currentKey();

      //kdDebug(7004) << k_funcinfo << " found lister in list - for " << url << endl;
      bool ret = it.current()->removeRef( lister );
      Q_ASSERT(ret);
      lister->d->numJobs--;

      // move lister to urlsCurrentlyHeld
      QPtrList<KDirLister> *holders = urlsCurrentlyHeld[url];
      if ( !holders )
      {
        holders = new QPtrList<KDirLister>;
        holders->append( lister );
        urlsCurrentlyHeld.insert( url, holders );
      }
      else
        holders->append( lister );

      emit lister->canceled( KURL( url ) );

      //kdDebug(7004) << "KDirListerCache::stop(lister) remaining list: " << it.current()->count() << " listers" << endl;
      // find and kill the job
      if ( it.current()->isEmpty() )
      {
        urlsCurrentlyListed.remove( url );
        killJob( url );
      }

      stopped = true;
    }
    else
      ++it;
  }

  if ( stopped )
  {
    emit lister->canceled();
    lister->d->complete = true;
  }

  // this is wrong if there is still an update running!
  //Q_ASSERT( lister->d->complete );
}

void KDirListerCache::stop( KDirLister *lister, const KURL& _u )
{
  QString urlStr( _u.url(-1) );
  KURL _url( urlStr );

  // TODO: consider to stop all the "child jobs" of _url as well
  kdDebug(7004) << k_funcinfo << lister << " url=" << _url << endl;

  kdDebug(7004) << "removing listed from urlsCurrentlyListed" << endl;
  QPtrList<KDirLister> *listers = urlsCurrentlyListed[urlStr];
  if ( !listers || !listers->removeRef( lister ) )
    return;

  // move lister to urlsCurrentlyHeld
  QPtrList<KDirLister> *holders = urlsCurrentlyHeld[_url.url()];
  if ( !holders )
  {
    holders = new QPtrList<KDirLister>;
    holders->append( lister );
    urlsCurrentlyHeld.insert( urlStr, holders );
  }
  else
    holders->append( lister );

  lister->d->numJobs--;
  emit lister->canceled( _url );

  if ( listers->isEmpty() )   // kill the job
  {
    urlsCurrentlyListed.remove( urlStr );
    killJob( urlStr );
  }

  if ( lister->d->numJobs == 0 )
  {
    lister->d->complete = true;

    // we killed the last job for lister
    emit lister->canceled();
  }
}

void KDirListerCache::setAutoUpdate( KDirLister *lister, bool enable )
{
  // IMPORTANT: this method does not check for the current autoUpdate state!

  for ( KURL::List::Iterator it = lister->d->lstDirs.begin();
        it != lister->d->lstDirs.end(); ++it )
  {
    if ( enable )
      itemsInUse[(*it).url()]->incAutoUpdate();
    else
      itemsInUse[(*it).url()]->decAutoUpdate();
  }
}

void KDirListerCache::forgetDirs( KDirLister *lister )
{
  kdDebug(7004) << k_funcinfo << lister << endl;

  // clear lister->d->lstDirs before calling forgetDirs(), so that
  // it doesn't contain things that itemsInUse doesn't. When emitting
  // the canceled signals, lstDirs must not contain anything that
  // itemsInUse does not contain. (otherwise it might crash in findByName()).
  KURL::List lstDirsCopy = lister->d->lstDirs;
  lister->d->lstDirs.clear();
  
  for ( KURL::List::Iterator it = lstDirsCopy.begin();
        it != lstDirsCopy.end(); ++it )
  {
    forgetDirs( lister, *it, false );
  }

  emit lister->clear();
}

void KDirListerCache::forgetDirs( KDirLister *lister, const KURL& url, bool notify )
{
  kdDebug(7004) << k_funcinfo << lister << " url: " << url << endl;

  QString urlStr = url.url(-1);
  QPtrList<KDirLister> *holders = urlsCurrentlyHeld[urlStr];
  Q_ASSERT( holders );
  holders->removeRef( lister );

  DirItem *item = itemsInUse[urlStr];
  Q_ASSERT( item );

  if ( holders->isEmpty() )
  {
    urlsCurrentlyHeld.remove( urlStr ); // this deletes the (empty) holders list
    if ( !urlsCurrentlyListed[urlStr] )
    {
      // item not in use anymore -> move into cache if complete
      itemsInUse.remove( urlStr );

      // this job is a running update
      if ( killJob( urlStr ) )
      {
        kdDebug(7004) << k_funcinfo << "Killing update job for " << urlStr << endl;

        lister->d->numJobs--;

        emit lister->canceled( url );
        if ( lister->d->numJobs == 0 )
        {
          lister->d->complete = true;
          emit lister->canceled();
        }
      }

      if ( notify )
      {
        lister->d->lstDirs.remove( url.url() );
        emit lister->clear( url );
      }

      if ( item->complete )
      {
        kdDebug(7004) << k_funcinfo << lister << " item moved into cache: " << url << endl;
        itemsCached.insert( urlStr, item ); // TODO: may return false!!

        // watch cached directories
        item->incAutoUpdate();
      }
      else {
        delete item;
        item = 0;
      }
    }
  }

  if ( item && lister->d->autoUpdate )
    item->decAutoUpdate();
}

void KDirListerCache::updateDirectory( const KURL& _dir )
{
  kdDebug(7004) << k_funcinfo << _dir << endl;

  QString urlStr = _dir.url(-1);
  if ( !itemsInUse[urlStr] )
  {
    DirItem *item = itemsCached[urlStr];
    if ( item )
    {
      item->complete = false;
      item->decAutoUpdate();
      kdDebug(7004) << k_funcinfo << "directory " << _dir << " not in use, marked dirty." << endl;
    }
    else
      kdDebug(7004) << k_funcinfo << "aborted, directory " << _dir << " not in cache." << endl;

    return;
  }

  // A job can be running to
  //   - only list a new directory: the listers are in urlsCurrentlyListed
  //   - only update a directory: the listers are in urlsCurrentlyHeld
  //   - update a currently running listing: the listers are in urlsCurrently

  // restart the job for _dir if it is running already
  bool killed = killJob( urlStr );

  // we don't need to emit canceled signals since we only replaced the job,
  // the listing is continuing.

  QPtrList<KDirLister> *listers = urlsCurrentlyListed[urlStr];
  QPtrList<KDirLister> *holders = urlsCurrentlyHeld[urlStr];

  Q_ASSERT( !listers || ( listers && killed ) );

  KIO::ListJob *job = KIO::listDir( _dir, false /* no default GUI */ );
  jobs.insert( job, QValueList<KIO::UDSEntry>() );

  connect( job, SIGNAL( entries( KIO::Job *, const KIO::UDSEntryList & ) ),
           this, SLOT( slotUpdateEntries( KIO::Job *, const KIO::UDSEntryList & ) ) );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           this, SLOT( slotUpdateResult( KIO::Job * ) ) );

  kdDebug(7004) << k_funcinfo << "update started in " << _dir << endl;

  if ( !killed && holders )
    for ( KDirLister *kdl = holders->first(); kdl; kdl = holders->next() )
    {
      kdl->d->numJobs++;
      kdl->d->complete = false;
      emit kdl->started( _dir );
    }
}

KFileItemList* KDirListerCache::itemsForDir( const KURL &_dir ) const
{
  QString urlStr = _dir.url(-1);
  DirItem *item = itemsInUse[ urlStr ];
  if ( !item )
    item = itemsCached[ urlStr ];
  return item ? item->lstItems : 0;
}

KFileItem* KDirListerCache::findByName( const KDirLister *lister, const QString& _name ) const
{
  Q_ASSERT( lister );

  for ( KURL::List::Iterator it = lister->d->lstDirs.begin();
        it != lister->d->lstDirs.end(); ++it )
  {
    KFileItemListIterator kit( *itemsInUse[(*it).url()]->lstItems );
    for ( ; kit.current(); ++kit )
      if ( (*kit)->name() == _name )
        return (*kit);
  }

  return 0L;
}

KFileItem* KDirListerCache::findByURL( const KDirLister *lister, const KURL& _u ) const
{
  QString _url = _u.url(-1);

  KURL parentDir( _url );
  parentDir.setPath( parentDir.directory() );

  // If lister is set, check that it contains this dir
  if ( lister && !lister->d->lstDirs.contains( parentDir ) )
      return 0L;

  KFileItemList* itemList = itemsForDir( parentDir );
  if ( itemList )
  {
    KFileItemListIterator kit( *itemList );
    for ( ; kit.current(); ++kit )
      if ( (*kit)->url() == _url )
        return (*kit);
  }
  return 0L;
}

void KDirListerCache::FilesAdded( const KURL &dir )
{
  kdDebug(7004) << k_funcinfo << dir << endl;
  updateDirectory( dir );
}

void KDirListerCache::FilesRemoved( const KURL::List &fileList )
{
  kdDebug(7004) << k_funcinfo << endl;
  KURL::List::ConstIterator it = fileList.begin();
  for ( ; it != fileList.end() ; ++it )
  {
    // emit the deleteItem signal if this file was shown in any view
    KFileItem* fileitem = 0L;
    KURL parentDir( *it );
    parentDir.setPath( parentDir.directory() );
    KFileItemList* lstItems = itemsForDir( parentDir );
    if ( lstItems )
    {
      KFileItem* fit = lstItems->first();
      for ( ; fit; fit = lstItems->next() )
        if ( fit->url() == *it ) {
          fileitem = fit;
          lstItems->take(); // remove fileitem from list
          break;
        }
    }

    // in case of a dir, check if we have any known children, there's much to do in that case
    // (stopping jobs, removing dirs from cache etc.)
    deleteDir( *it );

    // now remove the (dir)item itself
    if ( fileitem )
    {
      QPtrList<KDirLister> *listers = urlsCurrentlyHeld[parentDir.url()];
      if ( listers )
        for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
          kdl->emitDeleteItem( fileitem );
      delete fileitem;
    }
  }
}

void KDirListerCache::FilesChanged( const KURL::List &fileList )
{
  kdDebug(7004) << k_funcinfo << "only half implemented" << endl;
  KURL::List::ConstIterator it = fileList.begin();
  for ( ; it != fileList.end() ; ++it )
  {
      kdDebug(7004) << "KDirListerCache::FilesChanged " << *it << endl;
      KFileItem* fileitem = findByURL( 0, *it );
      if ( fileitem )
      {
          // we need to refresh the item, because e.g. the permissions can have changed.
          fileitem->refresh();
          emitRefreshItem( fileitem );
      }
      else
          kdDebug(7004) << "item not found" << endl;
  }
  // ## TODO problems with current jobs listing/updating that dir
  // ( see kde-2.2.2's kdirlister )
}

void KDirListerCache::FileRenamed( const KURL &src, const KURL &dst )
{
  kdDebug(7004) << k_funcinfo << src.prettyURL() << " -> " << dst.prettyURL() << endl;
#ifdef DEBUG_CACHE
  printDebug();
#endif

  // Somehow this should only be called if src is a dir. But how could we know if it is?
  // (Note that looking into itemsInUse isn't good enough. One could rename a subdir in a view.)
  renameDir( src, dst );

  QString oldUrl = src.url(-1);
  // Now update the KFileItem representing that file or dir (not exclusive with the above!)
  KFileItem* fileitem = findByURL( 0, oldUrl );
  if ( fileitem )
  {
    fileitem->setURL( dst );
    fileitem->refreshMimeType();

    emitRefreshItem( fileitem );
  }
#ifdef DEBUG_CACHE
  printDebug();
#endif
}

void KDirListerCache::emitRefreshItem( KFileItem* fileitem )
{
  // Look whether this item was shown in any view, i.e. held by any dirlister
  KURL parentDir( fileitem->url() );
  parentDir.setPath( parentDir.directory() );
  QString parentDirURL = parentDir.url();
  QPtrList<KDirLister> *listers = urlsCurrentlyHeld[parentDirURL];
  if ( listers )
    for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
    {
      kdl->addRefreshItem( fileitem );
      kdl->emitItems();
    }

  // Also look in urlsCurrentlyListed, in case the user manages to rename during a listing
  listers = urlsCurrentlyListed[parentDirURL];
  if ( listers )
    for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
    {
      kdl->addRefreshItem( fileitem );
      kdl->emitItems();
    }
}

KDirListerCache* KDirListerCache::self()
{
  if ( !s_pSelf )
    s_pSelf = sd_KDirListerCache.setObject( s_pSelf, new KDirListerCache );

  return s_pSelf;
}

// private slots

void KDirListerCache::slotFileDirty( const QString& _file )
{
  kdDebug(7004) << k_funcinfo << _file << endl;

  KURL u;
  u.setPath( _file );
  KFileItem *item = findByURL( 0, u ); // search all items
  if ( item && !item->isDir() )
  {
    // we need to refresh the item, because e.g. the permissions can have changed.
    item->refresh();
    emitRefreshItem( item );
  }
  else
    slotURLDirty( u );
}

void KDirListerCache::slotFileCreated( const QString& _file )
{
  // XXX: how to avoid a complete rescan here?
  KURL u;
  u.setPath( _file );
  u.setPath( u.directory() );
  FilesAdded( u );
}

void KDirListerCache::slotFileDeleted( const QString& _file )
{
  KURL u;
  u.setPath( _file );
  FilesRemoved( u );
}

void KDirListerCache::slotURLDirty( const KURL& _dir )
{
  //kdDebug(7004) << k_funcinfo << _dir.prettyURL() << endl;

  // this already checks for _dir being used
  updateDirectory( _dir );
}

void KDirListerCache::slotEntries( KIO::Job *job, const KIO::UDSEntryList &entries )
{
  KURL url = static_cast<KIO::ListJob *>(job)->url();
  url.adjustPath(-1);
  QString urlStr = url.url();

  kdDebug(7004) << k_funcinfo << "new entries for " << url << endl;

  DirItem *dir = itemsInUse[urlStr];
  Q_ASSERT( dir );

  QPtrList<KDirLister> *listers = urlsCurrentlyListed[urlStr];
  Q_ASSERT( listers );
  Q_ASSERT( !listers->isEmpty() );

  // check if anyone wants the mimetypes immediately
  bool delayedMimeTypes = true;
  for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
    delayedMimeTypes &= kdl->d->delayedMimeTypes;

  // avoid creating these QStrings again and again
  static const QString& dot = KGlobal::staticQString(".");
  static const QString& dotdot = KGlobal::staticQString("..");

  KIO::UDSEntryListConstIterator it = entries.begin();
  KIO::UDSEntryListConstIterator end = entries.end();

  for ( ; it != end; ++it )
  {
    QString name;

    // find out about the name
    KIO::UDSEntry::ConstIterator entit = (*it).begin();
    for( ; entit != (*it).end(); ++entit )
      if ( (*entit).m_uds == KIO::UDS_NAME )
      {
        name = (*entit).m_str;
        break;
      }

    Q_ASSERT( !name.isEmpty() );
    if ( name.isEmpty() )
      continue;

    if ( name == dot )
    {
      Q_ASSERT( !dir->rootItem );
      dir->rootItem = new KFileItem( *it, url, delayedMimeTypes, true  );

      for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
        if ( !kdl->d->rootFileItem && kdl->d->url == url )
          kdl->d->rootFileItem = dir->rootItem;
    }
    else if ( name != dotdot )
    {
      //kdDebug(7004)<< "Adding " << url.prettyURL() << endl;

      KFileItem* item = new KFileItem( *it, url, delayedMimeTypes, true );
      Q_ASSERT( item );

      dir->lstItems->append( item );

      for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
        kdl->addNewItem( item );
    }
  }

  for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
    kdl->emitItems();
}

void KDirListerCache::slotResult( KIO::Job* j )
{
  Q_ASSERT( j );
  KIO::ListJob *job = static_cast<KIO::ListJob *>( j );
  jobs.remove( job );

  KURL jobUrl = job->url();
  jobUrl.adjustPath(-1);  // need remove trailing slashes again, in case of redirections
  QString jobUrlStr = jobUrl.url();

  kdDebug(7004) << k_funcinfo << "finished listing " << jobUrl << endl;
#ifdef DEBUG_CACHE
  printDebug();
#endif

  QPtrList<KDirLister> *listers = urlsCurrentlyListed.take( jobUrlStr );
  Q_ASSERT( listers );

  KDirLister *kdl;

  if ( job->error() )
  {
    for ( kdl = listers->first(); kdl; kdl = listers->next() )
    {
      kdl->handleError( job );
      emit kdl->canceled( jobUrl );
      if ( --kdl->d->numJobs == 0 )
      {
        kdl->d->complete = true;
        emit kdl->canceled();
      }
    }
  }
  else
  {
    DirItem *dir = itemsInUse[jobUrlStr];
    Q_ASSERT( dir );
    dir->complete = true;

    for ( kdl = listers->first(); kdl; kdl = listers->next() )
    {
      emit kdl->completed( jobUrl );
      if ( --kdl->d->numJobs == 0 )
      {
        kdl->d->complete = true;
        emit kdl->completed();
      }
    }
  }

  // move the directory to the held directories
  Q_ASSERT( !urlsCurrentlyHeld[jobUrlStr] );
  urlsCurrentlyHeld.insert( jobUrlStr, listers );

  // TODO: hmm, if there was an error and job is a parent of one or more
  // of the pending urls we should cancel it/them as well
  processPendingUpdates();

#ifdef DEBUG_CACHE
  printDebug();
#endif
}

void KDirListerCache::slotRedirection( KIO::Job *job, const KURL &url )
{
  Q_ASSERT( job );
  KURL oldUrl = static_cast<KIO::ListJob *>( job )->url();
  kdDebug(7004) << k_funcinfo << oldUrl.prettyURL() << " -> " << url.prettyURL() << endl;

  // I don't think there can be dirItems that are childs of oldUrl.
  // Am I wrong here? And even if so, we don't need to delete them, right?
  // DF: redirection happens before listDir emits any item. Makes little sense otherwise.

  DirItem *dir = itemsInUse.take( oldUrl.url() );
  Q_ASSERT( dir );

  QPtrList<KDirLister> *listers = urlsCurrentlyListed.take( oldUrl.url() );
  Q_ASSERT( listers );
  Q_ASSERT( !listers->isEmpty() );

  for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
  {
    if ( kdl->d->url.cmp( oldUrl, true ) )
    {
      kdl->d->rootFileItem = 0;
      kdl->d->url = url;
    }

    *kdl->d->lstDirs.find( oldUrl ) = url;

    if ( kdl->d->lstDirs.count() == 1 )
    {
      emit kdl->clear();
      emit kdl->redirection( url );
      emit kdl->redirection( oldUrl, url );
    }
    else
    {
      emit kdl->clear( oldUrl );
      emit kdl->redirection( oldUrl, url );
    }
  }

  delete dir->rootItem;
  dir->rootItem = 0;
  dir->lstItems->clear();
  itemsInUse.insert( url.url(-1), dir );
  urlsCurrentlyListed.insert( url.url(-1), listers );
}

void KDirListerCache::renameDir( const KURL &oldUrl, const KURL &newUrl )
{
  kdDebug(7004) << k_funcinfo << oldUrl.prettyURL() << " -> " << newUrl.prettyURL() << endl;
  QString oldUrlStr = oldUrl.url(-1);
  QString newUrlStr = newUrl.url(-1);

  // Not enough. Also need to look at any child dir, even sub-sub-sub-dir.
  //DirItem *dir = itemsInUse.take( oldUrlStr );
  //emitRedirections( oldUrl, url );

  // Look at all dirs being listed/shown
  QDictIterator<DirItem> itu( itemsInUse );
  bool goNext;
  while ( itu.current() )
  {
    goNext = true;
    DirItem* dir = itu.current();
    KURL oldDirUrl = itu.currentKey();
    //kdDebug(7004) << "itemInUse: " << oldDirUrl.prettyURL() << endl;
    // Check if this dir is oldUrl, or a subfolder of it
    if ( oldUrl.isParentOf( oldDirUrl ) )
    {
      QString relPath = oldDirUrl.path().mid( oldUrl.path().length() ); // ### should use KURL::cleanpath like isParentOf does

      KURL newDirUrl( newUrl ); // take new base
      if ( !relPath.isEmpty() )
        newDirUrl.addPath( relPath ); // add unchanged relative path
      //kdDebug(7004) << "KDirListerCache::renameDir new url=" << newDirUrl.prettyURL() << endl;

      // Update URL in root item and in itemsInUse
      if ( dir->rootItem )
        dir->rootItem->setURL( newDirUrl );
      dir->url = newDirUrl;
      itemsInUse.remove( itu.currentKey() ); // implies ++itu
      itemsInUse.insert( newDirUrl.url(-1), dir );
      goNext = false; // because of the implied ++itu above
      if ( dir->lstItems )
      {
        // Rename all items under that dir
        KFileItemListIterator kit( *dir->lstItems );
        for ( ; kit.current(); ++kit )
        {
          KURL oldItemUrl = (*kit)->url();
          QString oldItemUrlStr( oldItemUrl.url(-1) );
          KURL newItemUrl( oldItemUrl );
          newItemUrl.setPath( newDirUrl.path() );
          newItemUrl.addPath( oldItemUrl.fileName() );
          kdDebug(7004) << "KDirListerCache::renameDir renaming " << oldItemUrlStr << " to " << newItemUrl.url() << endl;
          (*kit)->setURL( newItemUrl );
        }
      }
      emitRedirections( oldDirUrl, newDirUrl );
    }
    if (goNext)
      ++itu;
  }

  // Is oldUrl a directory in the cache?
  // Remove any child of oldUrl from the cache - even if the renamed dir itself isn't in it!
  removeDirFromCache( oldUrl );
  // TODO rename, instead.
}

void KDirListerCache::emitRedirections( const KURL &oldUrl, const KURL &url )
{
  kdDebug(7004) << k_funcinfo << oldUrl.prettyURL() << " -> " << url.prettyURL() << endl;
  QString oldUrlStr = oldUrl.url(-1);
  QString urlStr = url.url(-1);
  // Check if we were listing this dir. Need to abort and restart with new name in that case.
  QPtrList<KDirLister> *listers = urlsCurrentlyListed.take( oldUrlStr );
  if ( listers )
  {
    killJob( oldUrlStr );
    urlsCurrentlyListed.insert( urlStr, listers );
    updateDirectory( url );
    // not sure if we should emit canceled( oldUrl ) and started( dst ),
    // updateDirectory won't do it which means we will get a completed without
    // a different argument... I'd do this as well.
    for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
    {
      emit kdl->canceled( oldUrl );
      emit kdl->started( url );
    }
  }

  // Check if we are currently displaying this directory (odds opposite wrt above)
  // Update urlsCurrentlyHeld dict with new URL
  QPtrList<KDirLister> *holders = urlsCurrentlyHeld.take( oldUrlStr );
  if ( holders )
  {
    urlsCurrentlyHeld.insert( url.url(-1), holders );
    // And notify the dirlisters of the redirection
    for ( KDirLister *kdl = holders->first(); kdl; kdl = holders->next() )
    {
      *kdl->d->lstDirs.find( oldUrl ) = url;
      if ( kdl->d->lstDirs.count() == 1 )
      {
        emit kdl->redirection( url );
      }
      emit kdl->redirection( oldUrl, url );
    }
  }
}

void KDirListerCache::removeDirFromCache( const KURL& dir )
{
  kdDebug(7004) << "KDirListerCache::removeDirFromCache " << dir.prettyURL() << endl;
  QCacheIterator<DirItem> itc( itemsCached );
  while ( itc.current() )
  {
    if ( dir.isParentOf( KURL( itc.currentKey() ) ) )
      itemsCached.remove( itc.currentKey() );
    else
      ++itc;
  }
}

void KDirListerCache::slotUpdateEntries( KIO::Job* job, const KIO::UDSEntryList& list )
{
  jobs[static_cast<KIO::ListJob*>(job)] += list;
}

void KDirListerCache::slotUpdateResult( KIO::Job * j )
{
  Q_ASSERT( j );
  KIO::ListJob *job = static_cast<KIO::ListJob *>( j );

  KURL jobUrl = job->url();
  jobUrl.adjustPath(-1);  // need remove trailing slashes again, in case of redirections
  QString jobUrlStr = jobUrl.url();

  kdDebug(7004) << k_funcinfo << "finished update " << jobUrl << endl;

  KDirLister *kdl;

  QPtrList<KDirLister> *listers = urlsCurrentlyHeld[jobUrlStr];
  QPtrList<KDirLister> *tmpLst = urlsCurrentlyListed.take( jobUrlStr );

  if ( tmpLst )
  {
    if ( listers )
      for ( kdl = tmpLst->first(); kdl; kdl = tmpLst->next() )
      {
        Q_ASSERT( listers->containsRef( kdl ) == 0 );
        listers->append( kdl );
      }
    else
    {
      listers = tmpLst;
      urlsCurrentlyHeld.insert( jobUrlStr, listers );
    }
  }

  // once we are updating dirs that are only in the cache this will fail!
  Q_ASSERT( listers );

  if ( job->error() )
  {
    for ( kdl = listers->first(); kdl; kdl = listers->next() )
    {
      //don't bother the user
      //kdl->handleError( job );

      emit kdl->canceled( jobUrl );
      if ( --kdl->d->numJobs == 0 )
      {
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

  DirItem *dir = itemsInUse[jobUrlStr];
  dir->complete = true;


  // check if anyone wants the mimetypes immediately
  bool delayedMimeTypes = true;
  for ( kdl = listers->first(); kdl; kdl = listers->next() )
    delayedMimeTypes &= kdl->d->delayedMimeTypes;

  // should be enough to get reasonable speed in most cases
  QDict<KFileItem> fileItems( 9973 );

  KFileItemListIterator kit ( *(dir->lstItems) );

  // Unmark all items in url
  for ( ; kit.current(); ++kit )
  {
    //kdDebug(7004) << "slotUpdateResult : unmarking " << (*kit)->url().prettyURL() << endl;
    (*kit)->unmark();
    fileItems.insert( (*kit)->url().url(), *kit );
  }

  static const QString& dot = KGlobal::staticQString(".");
  static const QString& dotdot = KGlobal::staticQString("..");

  KFileItem *item, *tmp;

  QValueList<KIO::UDSEntry> buf = jobs[job];
  QValueListIterator<KIO::UDSEntry> it = buf.begin();
  for ( ; it != buf.end(); ++it )
  {
    QString name;

    // Find out about the name
    KIO::UDSEntry::Iterator it2 = (*it).begin();
    for ( ; it2 != (*it).end(); it2++ )
      if ( (*it2).m_uds == KIO::UDS_NAME )
      {
        name = (*it2).m_str;
        break;
      }

    Q_ASSERT( !name.isEmpty() );

    // we duplicate the check for dotdot here, to avoid iterating over
    // all items again and checking in matchesFilter() that way.
    if ( name.isEmpty() || name == dotdot )
      continue;

    if ( name == dot )
    {
      // if the update was started before finishing the original listing
      // there is no root item yet
      if ( !dir->rootItem )
      {
        dir->rootItem = new KFileItem( *it, jobUrl, delayedMimeTypes, true  );

        for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
          if ( !kdl->d->rootFileItem && kdl->d->url == jobUrl )
            kdl->d->rootFileItem = dir->rootItem;
      }

      continue;
    }

    // Form the complete url
    KURL u( jobUrl );
    u.addPath( name );
    //kdDebug(7004) << "slotUpdateResult : found " << name << endl;

    // Find this item
    bool found = false;
    if ( (tmp = fileItems[u.url()]) )
    {
      tmp->mark();
      found = true;
    }

    item = new KFileItem( *it, jobUrl, delayedMimeTypes, true  );

    if ( found )
    {
      // check if something changed for this file
      if ( !tmp->cmp( *item ) )
      {
        //kdDebug(7004) << "slotUpdateResult: file changed: " << tmp->name() << endl;
        tmp->assign( *item );

        for ( kdl = listers->first(); kdl; kdl = listers->next() )
          kdl->addRefreshItem( tmp );
      }
      delete item;  // gmbl, this is the most often case... IMPORTANT TODO: speed it up somehow!
    }
    else // this is a new file
    {
      //kdDebug(7004) << "slotUpdateResult: new file: " << name << endl;

      item->mark();
      dir->lstItems->append( item );

      for ( kdl = listers->first(); kdl; kdl = listers->next() )
        kdl->addNewItem( item );
    }
  }

  jobs.remove( job );

  deleteUnmarkedItems( listers, dir->lstItems );

  for ( kdl = listers->first(); kdl; kdl = listers->next() )
  {
    kdl->emitItems();

    emit kdl->completed( jobUrl );
    if ( --kdl->d->numJobs == 0 )
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

bool KDirListerCache::killJob( const QString& _url )
{
  KIO::ListJob *job;
  QMap< KIO::ListJob *, QValueList<KIO::UDSEntry> >::Iterator it = jobs.begin();
  while ( it != jobs.end() )
  {
    job = it.key();
    if ( job->url().url(-1) == _url )
    {
      jobs.remove( it );
      job->disconnect( this );
      job->kill();
      return true;
    }
    ++it;
  }
  return false;
}

void KDirListerCache::deleteUnmarkedItems( QPtrList<KDirLister> *listers, KFileItemList *lstItems )
{
  // Find all unmarked items and delete them
  KFileItem* item;
  lstItems->first();
  while ( (item = lstItems->current()) )
    if ( !item->isMarked() )
    {
      for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
        kdl->emitDeleteItem( item );

      deleteDir( item->url() );

      // finally actually delete the item
      lstItems->take();
      delete item;
    }
    else
      lstItems->next();
}

void KDirListerCache::deleteDir( const KURL& dirUrl )
{
  // unregister and remove the childs of the deleted item.
  // Idea: tell all the KDirListers that they should forget the dir
  //       and then remove it from the cache.

  QDictIterator<DirItem> itu( itemsInUse );
  while ( itu.current() )
  {
    KURL deletedUrl = itu.currentKey();
    if ( dirUrl.isParentOf( deletedUrl ) )
    {
      // stop all jobs for deletedUrl

      QPtrList<KDirLister> *kdls = urlsCurrentlyListed[deletedUrl.url()];
      if ( kdls )  // yeah, I lack good names
      {
        // we need a copy because stop modifies the list
        kdls = new QPtrList<KDirLister>( *kdls );
        for ( KDirLister *kdl = kdls->first(); kdl; kdl = kdls->next() )
          stop( kdl, deletedUrl );

        delete kdls;
      }

      // tell listers holding deletedUrl to forget about it
      // this will stop running updates for deletedUrl as well

      kdls = urlsCurrentlyHeld[deletedUrl.url()];
      if ( kdls )
      {
        // we need a copy because forgetDirs modifies the list
        kdls = new QPtrList<KDirLister>( *kdls );

        for ( KDirLister *kdl = kdls->first(); kdl; kdl = kdls->next() )
        {
          // lister's root is the deleted item
          if ( kdl->d->url == deletedUrl )
          {
            forgetDirs( kdl );
            emit kdl->deleteItem( kdl->d->rootFileItem );
            kdl->d->rootFileItem = 0;
          }
          else
          {
            bool treeview = kdl->d->lstDirs.count() > 1;
            forgetDirs( kdl, deletedUrl, treeview );
            if ( !treeview )
            {
              kdl->d->lstDirs.clear();
              emit kdl->clear();
            }
          }
        }

        delete kdls;
      }

      // delete the entry for deletedUrl - should not be needed, it's in
      // items cached now

      DirItem *dir = itemsInUse.take( deletedUrl.url() );
      Q_ASSERT( !dir );
    }
    else
      ++itu;
  }

  // remove the children from the cache
  removeDirFromCache( dirUrl );
}

void KDirListerCache::processPendingUpdates()
{
  // TODO
}

#ifndef NDEBUG
void KDirListerCache::printDebug()
{
  kdDebug(7004) << "Items in use: " << endl;
  QDictIterator<DirItem> itu( itemsInUse );
  for ( ; itu.current() ; ++itu ) {
      kdDebug(7004) << "   " << itu.currentKey() << "  URL: " << itu.current()->url
                    << " rootItem: " << ( itu.current()->rootItem ? itu.current()->rootItem->url() : QString("NULL") )
                    << " autoUpdates refcount: " << itu.current()->autoUpdates
                    << " complete: " << itu.current()->complete
                  << ( itu.current()->lstItems ? QString(" with %1 items.").arg(itu.current()->lstItems->count()) : QString(" lstItems=NULL") ) << endl;
  }

  kdDebug(7004) << "urlsCurrentlyHeld: " << endl;
  QDictIterator< QPtrList<KDirLister> > it( urlsCurrentlyHeld );
  for ( ; it.current() ; ++it )
  {
    QString list;
    for ( QPtrListIterator<KDirLister> listit( *it.current() ); listit.current(); ++listit )
      list += " 0x" + QString::number( (long)listit.current(), 16 );
    kdDebug(7004) << "   " << it.currentKey() << "  " << it.current()->count() << " listers: " << list << endl;
  }

  kdDebug(7004) << "urlsCurrentlyListed: " << endl;
  QDictIterator< QPtrList<KDirLister> > it2( urlsCurrentlyListed );
  for ( ; it2.current() ; ++it2 )
  {
    QString list;
    for ( QPtrListIterator<KDirLister> listit( *it2.current() ); listit.current(); ++listit )
      list += " 0x" + QString::number( (long)listit.current(), 16 );
    kdDebug(7004) << "   " << it2.currentKey() << "  " << it2.current()->count() << " listers: " << list << endl;
  }

  QMap< KIO::ListJob *, QValueList<KIO::UDSEntry> >::Iterator jit = jobs.begin();
  kdDebug(7004) << "Jobs: " << endl;
  for ( ; jit != jobs.end() ; ++jit )
    kdDebug(7004) << "   " << jit.key() << " listing " << jit.key()->url().prettyURL() << ": " << (*jit).count() << " entries." << endl;

  kdDebug(7004) << "Items in cache: " << endl;
  QCacheIterator<DirItem> itc( itemsCached );
  for ( ; itc.current() ; ++itc )
    kdDebug(7004) << "   " << itc.currentKey() << "  rootItem: "
                  << ( itc.current()->rootItem ? itc.current()->rootItem->url().prettyURL() : QString("NULL") )
                  << ( itc.current()->lstItems ? QString(" with %1 items.").arg(itc.current()->lstItems->count()) : QString(" lstItems=NULL") ) << endl;
}
#endif

/*********************** -- The new KDirLister -- ************************/


KDirLister::KDirLister( bool _delayedMimeTypes )
{
  kdDebug(7003) << "+KDirLister" << endl;

  d = new KDirListerPrivate;

  d->complete = true;
  d->delayedMimeTypes = _delayedMimeTypes;

  setAutoUpdate( true );
  setDirOnlyMode( false );
  setShowingDotFiles( false );

  setAutoErrorHandlingEnabled( true, 0 );

  connect( this, SIGNAL( completed() ), SLOT( slotClearState() ) );
  connect( this, SIGNAL( canceled() ), SLOT( slotClearState() ) );
  connect( this, SIGNAL( completed( const KURL& ) ), SLOT( slotJobToBeKilled( const KURL& ) ) );
  connect( this, SIGNAL( canceled( const KURL& ) ), SLOT( slotJobToBeKilled( const KURL& ) ) );
}

KDirLister::~KDirLister()
{
  kdDebug(7003) << "-KDirLister" << endl;

  // Stop all running jobs
  stop();
  s_pCache->forgetDirs( this );

  delete d;
}

bool KDirLister::openURL( const KURL& _url, bool _keep, bool _reload )
{
  if ( !validURL( _url ) )
    return false;

  kdDebug(7003) << k_funcinfo << _url.prettyURL()
                << " keep=" << _keep << " reload=" << _reload << endl;

  // emit the current changes made to avoid an inconsistent treeview
  if ( d->changes != NONE && _keep )
    emitChanges();

  d->changes = NONE;

  s_pCache->listDir( this, _url, _keep, _reload );

  return true;
}

void KDirLister::stop()
{
  kdDebug(7003) << k_funcinfo << endl;
  s_pCache->stop( this );
}

void KDirLister::stop( const KURL& _url )
{
  kdDebug(7003) << k_funcinfo << _url.prettyURL() << endl;
  s_pCache->stop( this, _url );
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
  s_pCache->setAutoUpdate( this, _enable );
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

const KURL& KDirLister::url() const
{
  return d->url;
}

void KDirLister::emitChanges()
{
  if ( d->changes == NONE )
    return;

  static const QString& dot = KGlobal::staticQString(".");
  static const QString& dotdot = KGlobal::staticQString("..");

  for ( KURL::List::Iterator it = d->lstDirs.begin();
        it != d->lstDirs.end(); ++it )
  {
    KFileItemListIterator kit( *s_pCache->itemsForDir( *it ) );
    for ( ; kit.current(); ++kit )
    {
      if ( (*kit)->text() == dot || (*kit)->text() == dotdot )
        continue;

      bool oldMime = true, newMime = true;

      if ( d->changes & MIME_FILTER )
      {
        oldMime = doMimeFilter( (*kit)->mimetype(), d->oldMimeFilter )
		 && doMimeExcludeFilter( (*kit)->mimetype(), d->oldMimeExcludeFilter );
        newMime = doMimeFilter( (*kit)->mimetype(), d->mimeFilter ) 
		&& doMimeExcludeFilter( (*kit)->mimetype(), d->mimeExcludeFilter );

        if ( oldMime && !newMime )
        {
          emit deleteItem( *kit );
          continue;
        }
      }

      if ( d->changes & DIR_ONLY_MODE )
      {
        // the lister switched to dirOnlyMode
        if ( d->dirOnlyMode )
        {
          if ( !(*kit)->isDir() )
            emit deleteItem( *kit );
        }
        else if ( !(*kit)->isDir() )
          addNewItem( *kit );

        continue;
      }

      if ( (*kit)->text()[0] == dot )
      {
        if ( d->changes & DOT_FILES )
        {
          // the lister switched to dot files mode
          if ( d->isShowingDotFiles )
            addNewItem( *kit );
          else
            emit deleteItem( *kit );

          continue;
        }
      }
      else if ( d->changes & NAME_FILTER )
      {
        bool oldName = (*kit)->isDir() ||
                       d->oldFilters.isEmpty() ||
                       doNameFilter( (*kit)->text(), d->oldFilters );

        bool newName = (*kit)->isDir() ||
                       d->lstFilters.isEmpty() ||
                       doNameFilter( (*kit)->text(), d->lstFilters );

        if ( oldName && !newName )
        {
          emit deleteItem( *kit );
          continue;
        }
        else if ( !oldName && newName )
          addNewItem( *kit );
      }

      if ( (d->changes & MIME_FILTER) && !oldMime && newMime )
        addNewItem( *kit );
    }

    emitItems();
  }

  d->changes = NONE;
}

void KDirLister::updateDirectory( const KURL& _u )
{
  s_pCache->updateDirectory( _u );
}

bool KDirLister::isFinished() const
{
  return d->complete;
}

KFileItem* KDirLister::rootItem() const
{
  return d->rootFileItem;
}

KFileItem* KDirLister::findByURL( const KURL& _url ) const
{
  return s_pCache->findByURL( this, _url );
}

KFileItem* KDirLister::findByName( const QString& _name ) const
{
  return s_pCache->findByName( this, _name );
}

#ifndef KDE_NO_COMPAT
KFileItem* KDirLister::find( const KURL& _url ) const
{
  return findByURL( _url );
}
#endif


// ================ public filter methods ================ //

void KDirLister::setNameFilter( const QString& nameFilter )
{
  if ( !(d->changes & NAME_FILTER) )
  {
    d->oldFilters = d->lstFilters;
    d->lstFilters.setAutoDelete( false );
  }

  d->lstFilters.clear();
  d->lstFilters.setAutoDelete( true );

  d->nameFilter = nameFilter;

  // Split on white space
  QStringList list = QStringList::split( ' ', nameFilter );
  for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
    d->lstFilters.append( new QRegExp(*it, false, true ) );

  d->changes |= NAME_FILTER;
}

const QString& KDirLister::nameFilter() const
{
  return d->nameFilter;
}

void KDirLister::setMimeFilter( const QStringList& mimeFilter )
{
  if ( !(d->changes & MIME_FILTER) )
    d->oldMimeFilter = d->mimeFilter;

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

const QStringList& KDirLister::mimeFilters() const
{
  return d->mimeFilter;
}

bool KDirLister::matchesFilter( const QString& name ) const
{
  return doNameFilter( name, d->lstFilters );
}

bool KDirLister::matchesMimeFilter( const QString& mime ) const
{
  return doMimeFilter( mime, d->mimeFilter ) && doMimeExcludeFilter(mime,d->mimeExcludeFilter);
}

// ================ protected methods ================ //

bool KDirLister::matchesFilter( const KFileItem *item ) const
{
  Q_ASSERT( item );
  static const QString& dotdot = KGlobal::staticQString("..");

  if ( item->text() == dotdot )
    return false;

  if ( !d->isShowingDotFiles && item->text()[0] == '.' )
    return false;

  if ( item->isDir() || d->lstFilters.isEmpty() )
    return true;

  return matchesFilter( item->text() );
}

bool KDirLister::matchesMimeFilter( const KFileItem *item ) const
{
  Q_ASSERT( item );
  return matchesMimeFilter( item->mimetype() );
}

bool KDirLister::doNameFilter( const QString& name, const QPtrList<QRegExp>& filters ) const
{
  for ( QPtrListIterator<QRegExp> it( filters ); it.current(); ++it )
    if ( it.current()->exactMatch( name ) )
      return true;

  return false;
}

bool KDirLister::doMimeFilter( const QString& mime, const QStringList& filters ) const
{
  if ( filters.isEmpty() )
    return true;

  QStringList::ConstIterator it = filters.begin();
  for ( ; it != filters.end(); ++it )
    if ( (*it) == mime )
      return true;

  return false;
}

bool KDirLister::doMimeExcludeFilter( const QString& mime, const QStringList& filters ) const
{
  if ( filters.isEmpty() )
    return true;

  QStringList::ConstIterator it = filters.begin();
  for ( ; it != filters.end(); ++it )
    if ( (*it) == mime )
      return false;

  return true;
}


bool KDirLister::validURL( const KURL& _url ) const
{
  if ( _url.isMalformed() )
  {
    if ( d->autoErrorHandling )
    {
      QString tmp = i18n("Malformed URL\n%1").arg( _url.prettyURL() );
      KMessageBox::error( d->errorParent, tmp );
    }
    return false;
  }

  // TODO: verify that this is really a directory?

  return true;
}

void KDirLister::handleError( KIO::Job *job )
{
  if ( d->autoErrorHandling )
    job->showErrorDialog( d->errorParent );
}


// ================= private methods ================= //

void KDirLister::addNewItem( const KFileItem *item )
{
  bool isNameFilterMatch = (d->dirOnlyMode && !item->isDir()) || !matchesFilter( item );
  bool isMimeFilterMatch = !matchesMimeFilter( item );

  if ( !isNameFilterMatch && !isMimeFilterMatch )
  {
    if ( !d->lstNewItems )
      d->lstNewItems = new KFileItemList;

    d->lstNewItems->append( item );            // items not filtered
  }
  else if ( !isNameFilterMatch )
  {
    if ( !d->lstMimeFilteredItems )
      d->lstMimeFilteredItems = new KFileItemList;

    d->lstMimeFilteredItems->append( item );   // only filtered by mime
  }
}

void KDirLister::addNewItems( const KFileItemList& items )
{
  // TODO: make this faster - test if we have a filter at all first
  for ( KFileItemListIterator kit( items ); kit.current(); ++kit )
    addNewItem( *kit );
}

void KDirLister::addRefreshItem( const KFileItem *item )
{
  bool isNameFilterMatch = (d->dirOnlyMode && !item->isDir()) || !matchesFilter( item );
  bool isMimeFilterMatch = !matchesMimeFilter( item );

  if ( !isNameFilterMatch && !isMimeFilterMatch )
  {
    if ( !d->lstRefreshItems )
      d->lstRefreshItems = new KFileItemList;

    d->lstRefreshItems->append( item );
  }
}

void KDirLister::emitItems()
{
  KFileItemList *tmpNew = d->lstNewItems;
  d->lstNewItems = 0;

  KFileItemList *tmpMime = d->lstMimeFilteredItems;
  d->lstMimeFilteredItems = 0;

  KFileItemList *tmpRefresh = d->lstRefreshItems;
  d->lstRefreshItems = 0;

  if ( tmpNew )
  {
    emit newItems( *tmpNew );
    delete tmpNew;
  }

  if ( tmpMime )
  {
    emit itemsFilteredByMime( *tmpMime );
    delete tmpMime;
  }

  if ( tmpRefresh )
  {
    emit refreshItems( *tmpRefresh );
    delete tmpRefresh;
  }
}

void KDirLister::emitDeleteItem( KFileItem *item )
{
  bool isNameFilterMatch = (d->dirOnlyMode && !item->isDir()) || !matchesFilter( item );
  bool isMimeFilterMatch = !matchesMimeFilter( item );

  if ( !isNameFilterMatch && !isMimeFilterMatch )
    emit deleteItem( item );
}


// ================ private slots ================ //

void KDirLister::slotInfoMessage( KIO::Job *, const QString& message )
{
  emit infoMessage( message );
}

void KDirLister::slotPercent( KIO::Job *job, unsigned long pcnt )
{
  d->jobData[static_cast<KIO::ListJob*>(job)].percent = pcnt;

  int result = 0;

  KIO::filesize_t size = 0;

  QMap< KIO::ListJob *, KDirListerPrivate::JobData >::Iterator dataIt = d->jobData.begin();
  while ( dataIt != d->jobData.end() )
  {
    result += (*dataIt).percent * (*dataIt).totalSize;
    size += (*dataIt).totalSize;
    ++dataIt;
  }

  if ( size != 0 )
    result /= size;
  else
    result = 100;
  emit percent( result );
}

void KDirLister::slotTotalSize( KIO::Job *job, KIO::filesize_t size )
{
  d->jobData[static_cast<KIO::ListJob*>(job)].totalSize = size;

  KIO::filesize_t result = 0;
  QMap< KIO::ListJob *, KDirListerPrivate::JobData >::Iterator dataIt = d->jobData.begin();
  while ( dataIt != d->jobData.end() )
  {
    result += (*dataIt).totalSize;
    ++dataIt;
  }

  emit totalSize( result );
}

void KDirLister::slotProcessedSize( KIO::Job *job, KIO::filesize_t size )
{
  d->jobData[static_cast<KIO::ListJob*>(job)].processedSize = size;

  KIO::filesize_t result = 0;
  QMap< KIO::ListJob *, KDirListerPrivate::JobData >::Iterator dataIt = d->jobData.begin();
  while ( dataIt != d->jobData.end() )
  {
    result += (*dataIt).processedSize;
    ++dataIt;
  }

  emit processedSize( result );
}

void KDirLister::slotSpeed( KIO::Job *job, unsigned long spd )
{
  d->jobData[static_cast<KIO::ListJob*>(job)].speed = spd;

  int result = 0;
  QMap< KIO::ListJob *, KDirListerPrivate::JobData >::Iterator dataIt = d->jobData.begin();
  while ( dataIt != d->jobData.end() )
  {
    result += (*dataIt).speed;
    ++dataIt;
  }

  emit speed( result );
}

void KDirLister::slotJobToBeKilled( const KURL& url )
{
  KIO::ListJob *job;
  QMap< KIO::ListJob *, KDirListerPrivate::JobData >::Iterator dataIt = d->jobData.begin();
  while ( dataIt != d->jobData.end() )
  {
    job = dataIt.key();
    if ( job->url().cmp(url, true) )
    {
      d->jobData.remove( dataIt );
      return;
    }
    ++dataIt;
  }
}

void KDirLister::slotClearState()
{
  d->jobData.clear();
}


// to keep BC changes

void KDirLister::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kdirlister.moc"
#include "kdirlister_p.moc"
