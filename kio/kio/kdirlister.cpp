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
#include "kdirlister_p.h"

#include <qregexp.h>
#include <qptrlist.h>

#include <kapp.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <klocale.h>
#include <kio/job.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kstaticdeleter.h>


KDirListerCache* KDirListerCache::s_pSelf = 0;
static KStaticDeleter<KDirListerCache> sd_KDirListerCache;


KDirListerCache::KDirListerCache( int maxCount )
  : itemsCached( maxCount )
{
  kdDebug(7004) << "+KDirListerCache" << endl;

  itemsInUse.setAutoDelete( false );
  itemsCached.setAutoDelete( true );
  urlsCurrentlyListed.setAutoDelete( true );
  urlsCurrentlyHeld.setAutoDelete( true );

  connect( kdirwatch, SIGNAL( dirty( const QString& ) ),
           this, SLOT( slotDirectoryDirty( const QString& ) ) );
  connect( kdirwatch, SIGNAL( fileDirty( const QString& ) ),
           this, SLOT( slotFileDirty( const QString& ) ) );
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

// TODO: hmpf, setting _reload to true will emit the old files and
//       just then call updateDirectory
void KDirListerCache::listDir( KDirLister* lister, const KURL& _u,
                               bool _keep, bool _reload )
{
  // like this we don't have to worry about trailing slashes any further
  KURL _url( _u.url(-1) );

  kdDebug(7004) << k_funcinfo << lister << " url=" << _url.prettyURL()
                << " keep=" << _keep << " reload=" << _reload << endl;

  if ( !_keep )
  {
    // Stop running jobs for lister, if any
    stop( lister );

    // clear our internal list for lister
    forgetDirs( lister );

    lister->d->rootFileItem = 0;
  }
  else if ( lister->d->lstDirs.contains( _url ) )
  {
    updateDirectory( _url ); // for all listers
    return;
  }

  lister->d->lstDirs.append( _url );

  if ( lister->d->url.isEmpty() || !_keep ) // set toplevel URL only if not set yet
    lister->d->url = _url;

  lister->d->urlChanged = false;

  DirItem *itemU = itemsInUse[_url.url()];
  DirItem *itemC;

  if ( !urlsCurrentlyListed[_url.url()] )
  {
    if ( itemU )
    {
      kdDebug(7004) << "listDir: Entry already in use: " << _url.prettyURL() << endl;

      bool oldState = lister->d->complete;
      lister->d->complete = false;

      emit lister->started( _url );
      itemU->count++;

      if ( !lister->d->rootFileItem && lister->d->url == _url )
        lister->d->rootFileItem = itemU->rootItem;

      lister->addNewItems( *(itemU->lstItems) );
      lister->emitItems();

      lister->d->complete = oldState;

      emit lister->completed( _url );
      if ( lister->d->complete )
        emit lister->completed();

      // _url is already in use, so there is already an entry in urlsCurrentlyHeld
      urlsCurrentlyHeld[_url.url()]->append( lister );

      if ( _reload || !itemU->complete )
        updateDirectory( _url );    // hmm, is it better to do this first?
    }
    else if ( !_reload && (itemC = itemsCached.take( _url.url() )) )
    {
      kdDebug(7004) << "listDir: Entry in cache: " << _url.prettyURL() << endl;

      Q_ASSERT( itemC->complete );
      Q_ASSERT( itemC->count == 0 );
      Q_ASSERT( itemC->autoUpdates == 0 );

      itemC->count++;
      itemsInUse.insert( _url.url(), itemC );

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
    }
    else  // dir not in cache or _reload is true
    {
      kdDebug(7004) << "listDir: Entry not in cache or reloaded: " << _url.prettyURL() << endl;

      QPtrList<KDirLister> *list = new QPtrList<KDirLister>;
      list->append( lister );
      urlsCurrentlyListed.insert( _url.url(), list );

      itemsCached.remove( _url.url() );
      itemsInUse.insert( _url.url(), new DirItem );

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
    kdDebug(7004) << k_funcinfo << "Entry currently being listed: " << _url.prettyURL() << endl;

    emit lister->started( _url );

    lister->d->complete = false;
    lister->d->numJobs++;
    urlsCurrentlyListed[_url.url()]->append( lister );

    QMap< KIO::ListJob *, QValueList<KIO::UDSEntry> >::Iterator it = jobs.begin();
    while ( it != jobs.end() )
    {
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
    itemU->count++;

    if ( !lister->d->rootFileItem && lister->d->url == _url )
      lister->d->rootFileItem = itemU->rootItem;

    lister->addNewItems( *(itemU->lstItems) );
    lister->emitItems();
  }

  // Automatic updating of directories ?
  if ( lister->d->autoUpdate && _url.isLocalFile() &&
       itemsInUse[_url.url()]->autoUpdates++ == 0 )
  {
      kdDebug(7004) << "adding to kdirwatch " << kdirwatch << " " << _url.path() << endl;
      kdirwatch->addDir( _url.path() );
  }
}

// TODO: it does not yet stop running updates
void KDirListerCache::stop( KDirLister *lister )
{
  kdDebug(7004) << k_funcinfo << lister << endl;
  bool stopped = false;

  QDictIterator< QPtrList<KDirLister> > it( urlsCurrentlyListed );
  while ( it.current() )
  {
    if ( it.current()->findRef( lister ) > -1 )
    {
      // lister is listing url
      QString url = it.currentKey();

      it.current()->removeRef( lister );
      lister->d->numJobs--;

      // move lister to urlsCurrentlyHeld
      QPtrList<KDirLister> *listers = urlsCurrentlyHeld[url];
      if ( !listers )
      {
        listers = new QPtrList<KDirLister>;
        listers->append( lister );
        urlsCurrentlyHeld.insert( url, listers );
      }
      else
        listers->append( lister );

      // find and kill the job
      if ( it.current()->isEmpty() )
      {
        urlsCurrentlyListed.remove( url );

        KIO::ListJob *job;
        QMap< KIO::ListJob *, QValueList<KIO::UDSEntry> >::Iterator jobIt = jobs.begin();
        while ( jobIt != jobs.end() )
        {
          job = jobIt.key();
          if ( job->url().url() == url )
          {
            jobs.remove( jobIt );
            job->disconnect( this );
            job->kill();
            break;
          }
          ++jobIt;
        }
      }

      stopped = true;
      emit lister->canceled( KURL( url ) );
    }
    else
      ++it;
  }

  if ( stopped )
  {
    emit lister->canceled();
    lister->d->complete = true;
  }

  Q_ASSERT( lister->d->complete );
}

void KDirListerCache::stop( KDirLister *lister, const KURL& _u )
{
  KURL _url( _u.url(-1) );

  // TODO: consider to stop all the "child jobs" of _url as well
  kdDebug(7004) << k_funcinfo << lister << " url=" << _url.prettyURL() << endl;

  QPtrList<KDirLister> *listers = urlsCurrentlyListed[_url.url()];
  if ( !listers || !listers->removeRef( lister ) )
    return;

  lister->d->numJobs--;

  if ( listers->isEmpty() )   // kill the job
  {
    urlsCurrentlyListed.remove( _url.url() );

    KIO::ListJob *job;
    QMap< KIO::ListJob *, QValueList<KIO::UDSEntry> >::Iterator it = jobs.begin();
    while ( it != jobs.end() )
    {
      job = it.key();
      if ( job->url() == _url )
      {
        jobs.remove( it );
        job->disconnect( this );
        job->kill();
        break;
      }
      ++it;
    }
  }

  // move lister to urlsCurrentlyHeld
  listers = urlsCurrentlyHeld[_url.url()];
  if ( !listers )
  {
    listers = new QPtrList<KDirLister>;
    listers->append( lister );
    urlsCurrentlyHeld.insert( _url.url(), listers );
  }
  else
    listers->append( lister );

  emit lister->canceled( _url );

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
    if ( (*it).isLocalFile() )
    {
      if ( enable && itemsInUse[(*it).url()]->autoUpdates++ == 0 )
        kdirwatch->addDir( (*it).path() );
      else if ( --itemsInUse[(*it).url()]->autoUpdates == 0 )
        kdirwatch->removeDir( (*it).path() );
    }
  }
}

void KDirListerCache::forgetDirs( KDirLister *lister )
{
  kdDebug(7004) << k_funcinfo << lister << endl;
  DirItem* item;
  QPtrList<KDirLister> *listers;

  for ( KURL::List::Iterator it = lister->d->lstDirs.begin();
        it != lister->d->lstDirs.end(); ++it )
  {
    listers = urlsCurrentlyHeld[(*it).url()];
    Q_ASSERT( listers );
    listers->removeRef( lister );

    if ( listers->isEmpty() )
      urlsCurrentlyHeld.remove( (*it).url() );

    item = itemsInUse[(*it).url()];

    Q_ASSERT( item );
    Q_ASSERT( item->count );

    // one lister less holding this dir
    item->count--;

    if ( lister->d->autoUpdate && --item->autoUpdates == 0 && (*it).isLocalFile() )
    {
      kdDebug(7004) << "removing from kdirwatch " << kdirwatch << " " << (*it).path() << endl;
      kdirwatch->removeDir( (*it).path() );
    }

    // item not in use anymore -> move into cache if complete
    if ( item->count == 0 )
    {
      itemsInUse.remove( (*it).url() );
      if ( item->complete )
      {
        kdDebug(7004) << k_funcinfo << lister << " item moved into cache: " << (*it).prettyURL() << endl;
        itemsCached.insert( (*it).url(), item ); // TODO: may return false!!
      }
    }
  }

  lister->d->lstDirs.clear();
  emit lister->clear();
}

// NOTE: this *never* uses the cache!
void KDirListerCache::updateDirectory( const KURL& _dir )
{
  kdDebug(7004) << k_funcinfo << _dir.prettyURL() << endl;

  // TODO: if _dir is in itemsCached update it as well!
  if ( !itemsInUse[_dir.url()] )
  {
    kdDebug(7004) << k_funcinfo << "updateDirectory aborted, " << _dir.prettyURL() << " not in use!" << endl;
    return;
  }

  QPtrList<KDirLister> *listers = urlsCurrentlyListed[_dir.url()];

  // if there's a job running for _dir restart it
  KIO::ListJob *job;
  QMap< KIO::ListJob *, QValueList<KIO::UDSEntry> >::Iterator it = jobs.begin();
  while ( it != jobs.end() )
  {
    job = it.key();
    if ( job->url() == _dir )
    {
      if ( listers )
        for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
        {
          kdl->d->numJobs--;
          emit kdl->canceled( _dir );
          // and don't emit canceled() in any case as the
          // lister is not finished yet!
        }

      jobs.remove( it );
      job->disconnect( this );
      job->kill();
      break;
    }
    ++it;
  }

  // TODO: max-job-limit

  job = KIO::listDir( _dir, false /* no default GUI */ );
  jobs.insert( job, QValueList<KIO::UDSEntry>() );

  connect( job, SIGNAL( entries( KIO::Job *, const KIO::UDSEntryList & ) ),
           this, SLOT( slotUpdateEntries( KIO::Job *, const KIO::UDSEntryList & ) ) );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           this, SLOT( slotUpdateResult( KIO::Job * ) ) );

  // TODO: what about connecting the speed/prcessedSize... signals?

  kdDebug(7004) << k_funcinfo << "update started in " << _dir.prettyURL() << endl;

  if ( listers )
    for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
    {
      kdl->d->numJobs++;
      kdl->d->complete = false;
      emit kdl->started( _dir );
    }

  if ( (listers = urlsCurrentlyHeld[_dir.url()]) )
    for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
    {
      kdl->d->numJobs++;
      kdl->d->complete = false;
      emit kdl->started( _dir );
    }
}

KFileItem* KDirListerCache::findByName( const KDirLister *lister, const QString& _name ) const
{
  Q_ASSERT( lister );

  // TODO: make this faster
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

// if lister == 0 all items are searched
KFileItem* KDirListerCache::findByURL( const KDirLister *lister, const KURL& _u ) const
{
  KURL _url( _u.url(-1) );

  // TODO: this code could be improved :)

  if ( !lister || lister->d->lstDirs.contains( _url ) )
  {
    DirItem *item = itemsInUse[_url.url()];
    if ( item )
    {
      KFileItemListIterator kit( *item->lstItems );
      for ( ; kit.current(); ++kit )
      {
        if ( (*kit)->url() == _url )
          return (*kit);
      }
    }
    else if ( !lister )
    {
      item = itemsCached[_url.url()];
      if ( item )
      {
        KFileItemListIterator kit( *item->lstItems );
        for ( ; kit.current(); ++kit )
        {
          if ( (*kit)->url() == _url )
            return (*kit);
        }
      }
    }
  }

  return 0L;
}

void KDirListerCache::FilesAdded( const KURL & directory )
{
  kdWarning(7004) << k_funcinfo << "NOT IMPLEMENTED YET!" << endl;
}

void KDirListerCache::FilesRemoved( const KURL::List & fileList )
{
  kdWarning(7004) << k_funcinfo << "NOT IMPLEMENTED YET!" << endl;
}

void KDirListerCache::FilesChanged( const KURL::List & fileList )
{
  kdWarning(7004) << k_funcinfo << "NOT IMPLEMENTED YET!" << endl;
}

void KDirListerCache::FileRenamed( const KURL &src, const KURL &dst )
{
  kdWarning(7004) << k_funcinfo << "NOT IMPLEMENTED YET!" << endl;
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
  //kdDebug(7004) << k_funcinfo << _file << endl;

  KURL u;
  u.setPath( _file );
  KFileItem *item = findByURL( 0, u ); // search all items
  if ( item )
  {
    // we need to refresh the item, because e.g. the permissions can have changed.
    item->refresh();

    KFileItemList lst;
    lst.append( item );

    // TODO: use urlsCurrentlyListed as well?

    QPtrList<KDirLister> *listers = urlsCurrentlyHeld[u.url(-1)];
    if ( listers )
      for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
        emit kdl->refreshItems( lst );
  }
}

void KDirListerCache::slotDirectoryDirty( const QString& _dir )
{
  //kdDebug(7004) << k_funcinfo << _dir << endl;

  // _dir does not contain a trailing slash
  KURL url;
  url.setPath( _dir );
  slotURLDirty( url );
}

void KDirListerCache::slotURLDirty( const KURL& _dir )
{
  //kdDebug(7004) << k_funcinfo << _dir << endl;

  // this already checks for _dir being used
  updateDirectory( _dir );
}

void KDirListerCache::slotEntries( KIO::Job *job, const KIO::UDSEntryList &entries )
{
  const KURL &url = static_cast<KIO::ListJob *>(job)->url();

  kdDebug(7004) << k_funcinfo << "new entries for " << url.prettyURL() << endl;

  DirItem *dir = itemsInUse[url.url()];
  Q_ASSERT( dir );

  QPtrList<KDirLister> *listers = urlsCurrentlyListed[url.url()];
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

  kdDebug(7004) << k_funcinfo << "finished listing " << job->url().prettyURL() << endl;

  QPtrList<KDirLister> *listers = urlsCurrentlyListed.take( job->url().url() );
  Q_ASSERT( listers );

  KDirLister *kdl;

  if ( job->error() )
  {
    for ( kdl = listers->first(); kdl; kdl = listers->next() )
    {
      kdl->handleError( job );
      emit kdl->canceled( job->url() );
      if ( --kdl->d->numJobs == 0 )
      {
        kdl->d->complete = true;
        emit kdl->canceled();
      }
    }
  }
  else
  {
    DirItem *dir = itemsInUse[job->url().url()];
    Q_ASSERT( dir );
    dir->complete = true;

    for ( kdl = listers->first(); kdl; kdl = listers->next() )
    {
      emit kdl->completed( job->url() );
      if ( --kdl->d->numJobs == 0 )
      {
        kdl->d->complete = true;
        emit kdl->completed();
      }
    }
  }

  // move the directory to the held directories
  Q_ASSERT( !urlsCurrentlyHeld[job->url().url()] );
  urlsCurrentlyHeld.insert( job->url().url(), listers );

  // TODO: hmm, if there was an error and job is a parent of one or more
  // of the pending urls we should cancel it/them as well
  processPendingUpdates();
}

void KDirListerCache::slotRedirection( KIO::Job *job, const KURL &url )
{
  Q_ASSERT( job );
  KURL oldUrl = static_cast<KIO::ListJob *>( job )->url();

  kdDebug(7004) << k_funcinfo << oldUrl.prettyURL() << " -> " << url.prettyURL() << endl;

  // I don't think there can be dirItems that are childs of oldUrl.
  // Am I wrong here? And even if so, we don't need to delete them, right?

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
      for ( KFileItem *item = dir->lstItems->first(); item; item = dir->lstItems->next() )
        emit kdl->deleteItem( item );

      emit kdl->redirection( oldUrl, url );
    }
  }

  delete dir->rootItem;
  dir->rootItem = 0;
  dir->lstItems->clear();
  itemsInUse.insert( url.url(-1), dir );
  urlsCurrentlyListed.insert( url.url(-1), listers );
}

void KDirListerCache::slotUpdateEntries( KIO::Job* job, const KIO::UDSEntryList& list )
{
  jobs[static_cast<KIO::ListJob*>(job)] += list;
}

void KDirListerCache::slotUpdateResult( KIO::Job * j )
{
  Q_ASSERT( j );
  KIO::ListJob *job = static_cast<KIO::ListJob *>( j );

  const KURL& url = job->url();

  kdDebug(7004) << k_funcinfo << "finished update " << job->url().prettyURL() << endl;

  KDirLister *kdl;

  QPtrList<KDirLister> *listers = urlsCurrentlyHeld[url.url()];
  QPtrList<KDirLister> *tmpLst = urlsCurrentlyListed.take( url.url() );

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
      urlsCurrentlyHeld.insert( url.url(), listers );
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
      
      emit kdl->canceled( url );
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

  DirItem *dir = itemsInUse[url.url()];
  dir->complete = true;


  // check if anyone wants the mimetypes immediately
  bool delayedMimeTypes = true;
  for ( kdl = listers->first(); kdl; kdl = listers->next() )
    delayedMimeTypes &= kdl->d->delayedMimeTypes;

  // should be enough to get reasonable speed in most cases
  QDict<KFileItem> fileItems( 9973 );

  KFileItemList lstRefreshItems;
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
    // TODO: no need to update the root item??
    if ( name.isEmpty() || name == dot || name == dotdot )
      continue;

    // Form the complete url
    KURL u( url );
    u.addPath( name );
    //kdDebug(7004) << "slotUpdateResult : found " << name << endl;

    // Find this item
    bool found = false;
    if ( (tmp = fileItems[u.url()]) )
    {
      tmp->mark();
      found = true;
    }

    item = new KFileItem( *it, url, delayedMimeTypes, true  );

    if ( found )
    {
      // check if something changed for this file
      if ( !tmp->cmp( *item ) )
      {
        tmp->assign( *item );
        lstRefreshItems.append( tmp );
        kdDebug(7004) << "slotUpdateResult: file changed: " << tmp->name() << endl;
      }
      delete item;  // gmbl, this is the most often case... IMPORTANT TODO: speed it up somehow!
    }
    else // this is a new file
    {
      kdDebug(7004) << "slotUpdateResult: new file: " << name << endl;

      item->mark();
      dir->lstItems->append( item );

      for ( kdl = listers->first(); kdl; kdl = listers->next() )
        kdl->addNewItem( item );
    }
  }

  jobs.remove( job );

  deleteUnmarkedItems( listers, dir->lstItems, true );

  for ( kdl = listers->first(); kdl; kdl = listers->next() )
  {
    kdl->emitItems();

    if ( !lstRefreshItems.isEmpty() )
      emit kdl->refreshItems( lstRefreshItems );

    emit kdl->completed( url );
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

void KDirListerCache::deleteUnmarkedItems( QPtrList<KDirLister> *listers, KFileItemList *lstItems, bool really )
{
  // Find all unmarked items and delete them
  KFileItem* item;
  lstItems->first();
  while ( (item = lstItems->current()) )
    if ( !item->isMarked() )
    {
      for ( KDirLister *kdl = listers->first(); kdl; kdl = listers->next() )
        emit kdl->deleteItem( item );

      if ( really )
      {
        // unregister and remove the deleted child folders

/* TODO
   This needs still some work. Maybe every child-item of a deleted directory
   needs to be emitted? And maybe other listers holding the deleted dir
   need to be notified...
   hmm, and what about using KDirListerPrivate::lstDirs?

        // dunno if this is enough - use KURL::isParentOf()?
        if ( !itemsCached.remove( item->url().url(-1) ) )
        {
          DirItem *di;
          if ( (di = itemsInUse.take( item->url().url(-1) )) )
          {
            if ( item->url().isLocalFile() )
            {
              //kdDebug(7004) << "forgetting about " << item->url().path() << endl;
              kdirwatch->removeDir( item->url().path() );
            }

            delete di;
          }
        }
*/
        lstItems->take();
        delete item;
      }
    }
    else
      lstItems->next();
}

void KDirListerCache::processPendingUpdates()
{
  kdWarning(7004) << k_funcinfo << "NOT IMPLEMENTED YET!" << endl;
}



/*********************** -- The new KDirLister -- ************************/


KDirLister::KDirLister( bool _delayedMimeTypes )
{
  kdDebug(7003) << "+KDirLister" << endl;

  d = new KDirListerPrivate;

  d->complete = true;
  d->urlChanged = false;
  d->delayedMimeTypes = _delayedMimeTypes;

  d->autoErrorHandling = true;
  d->errorParent = 0;

  d->numJobs = 0;
  d->rootFileItem = 0;

  setAutoUpdate( true );
  setDirOnlyMode( false );
  setShowingDotFiles( false );

  connect( this, SIGNAL( completed() ), SLOT( slotClearState() ) );
  connect( this, SIGNAL( canceled() ), SLOT( slotClearState() ) );
  connect( this, SIGNAL( canceled( const KURL& ) ), SLOT( slotCanceledState( const KURL& ) ) );
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
}

bool KDirLister::autoErrorHandlingEnabled()
{
  return d->autoErrorHandling;
}

void KDirLister::setAutoErrorHandlingEnabled( bool enable, QWidget* parent )
{
  d->autoErrorHandling = enable;
  d->errorParent = parent;
}

void KDirLister::handleError( KIO::Job *job )
{
  if ( d->autoErrorHandling )
    job->showErrorDialog( d->errorParent );
}

const KURL& KDirLister::url() const
{
  return d->url;
}

void KDirLister::emitChanges()
{
  // FIXME TODO!!
}

void KDirLister::updateDirectory( const KURL& _u )
{
  KURL _dir( _u.url(-1) );
  s_pCache->updateDirectory( _dir );
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


// ================ filter methods ================ //

// public
void KDirLister::setNameFilter( const QString& nameFilter )
{
  d->lstFilters.clear();
  d->urlChanged = true;         // TODO do not reload the cache, but just emit the changed items!
  d->nameFilter = nameFilter;

  // Split on white space
  QStringList list = QStringList::split( ' ', nameFilter );
  for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
    d->lstFilters.append( new QRegExp(*it, false, true ) );
}

const QString& KDirLister::nameFilter() const
{
  return d->nameFilter;
}

void KDirLister::setMimeFilter( const QStringList& mimeFilter )
{
  d->mimeFilter = mimeFilter;
}

void KDirLister::clearMimeFilter()
{
  d->mimeFilter.clear();
}

const QStringList& KDirLister::mimeFilters() const
{
  return d->mimeFilter;
}

bool KDirLister::matchesFilter( const QString& name ) const
{
  bool matched = false;
  for ( QPtrListIterator<QRegExp> it( d->lstFilters ); it.current(); ++it )
    if ( it.current()->search( name ) != -1 )
    {
      matched = true;
      break;
    }

  return matched;
}

bool KDirLister::matchesMimeFilter( const QString& mime ) const
{
  if ( d->mimeFilter.isEmpty() )
    return true;

  QStringList::Iterator it = d->mimeFilter.begin();
  for ( ; it != d->mimeFilter.end(); ++it )
    if ( (*it) == mime )
      return true;

  return false;
}

// protected
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


// ================ protected methods ================ //

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

void KDirLister::addNewItem( const KFileItem *item )
{
  bool isNameFilterMatch = (d->dirOnlyMode && !item->isDir()) || !matchesFilter( item );
  bool isMimeFilterMatch = !matchesMimeFilter( item );

  if ( !isNameFilterMatch && !isMimeFilterMatch )
    d->lstNewItems.append( item );            // items not filtered
  else if ( !isNameFilterMatch )
    d->lstMimeFilteredItems.append( item );   // only filtered by mime
}

void KDirLister::addNewItems( const KFileItemList& items )
{
  // TODO: make this faster - test if we have a filter at all first
  for ( KFileItemListIterator kit( items ); kit.current(); ++kit )
    addNewItem( *kit );
}

void KDirLister::emitItems()
{
  if ( !d->lstNewItems.isEmpty() )
  {
    emit newItems( d->lstNewItems );
    d->lstNewItems.clear();
  }

  if ( !d->lstMimeFilteredItems.isEmpty() )
  {
    emit itemsFilteredByMime( d->lstMimeFilteredItems );
    d->lstMimeFilteredItems.clear();
  }
}


// ================ protected slots ================ //

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

  result /= size;
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

void KDirLister::slotCanceled( const KURL& url )
{
  KIO::ListJob *job;
  QMap< KIO::ListJob *, KDirListerPrivate::JobData >::Iterator dataIt = d->jobData.begin();
  while ( dataIt != d->jobData.end() )
  {
    job = dataIt.key();
    if ( job->url() == url )
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

#include "kdirlister.moc"
#include "kdirlister_p.moc"
