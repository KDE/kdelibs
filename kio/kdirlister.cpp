/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
                 2000 Carsten Pfeiffer <pfeiffer@kde.org>
                 2001 Michael Brade <brade@informatik.uni-muenchen.de>

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

#include <assert.h>

#include <qdir.h>
#include <qmap.h>
#include <qdict.h>

#include <kapp.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <klocale.h>
#include <kio/job.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kglobalsettings.h>

template class QPtrList<QRegExp>;

class KDirLister::KDirListerPrivate
{
public:
    KDirListerPrivate() { autoUpdate=false; }
    KURL::List lstPendingUpdates;
    bool autoUpdate;
    bool urlChanged;
    QString nameFilter;
    QStringList mimeFilter;
    QString deprecated_mimeFilter;
    QMap< KIO::ListJob *, QValueList<KIO::UDSEntry> > jobs;
    static const uint MAX_JOBS = 5;
};

KDirLister::KDirLister( bool _delayedMimeTypes )
{
  d = new KDirListerPrivate;
  m_bComplete = true;
  m_job = 0L;
  m_lstFileItems.setAutoDelete( true );
  m_lstFilters.setAutoDelete( true );
  m_rootFileItem = 0L;
  m_bDirOnlyMode = false;
  m_bDelayedMimeTypes = _delayedMimeTypes;
  m_isShowingDotFiles = false;
  d->urlChanged = false;
  setAutoUpdate( true );
}

KDirLister::~KDirLister()
{
  // Stop all running jobs
  stop();
  forgetDirs();
  delete m_rootFileItem;
  delete d;
}

void KDirLister::slotFileDirty( const QString& _file )
{
  //kdDebug(7003) << "KDirLister::slotFileDirty " << _file << endl;
  KURL u;
  u.setPath( _file );
  KFileItem * item = find( u.url() );
  if ( item ) {
    // We need to refresh the item, because i.e. the permissions can have changed.
    item->refresh();
    KFileItemList lst;
    lst.append( item );
    emit refreshItems( lst );
  }
}

void KDirLister::slotDirectoryDirty( const QString& _dir )
{
  // _dir does not contain a trailing slash
  //kdDebug(7003) << "KDirLister::slotDirectoryDirty( " << _dir << " )" << endl;
  KURL url;
  url.setPath( _dir );
  slotURLDirty( url );
}

void KDirLister::slotURLDirty( const KURL & dir )
{
  //kdDebug(7003) << "KDirLister::slotURLDirty " << dir.prettyURL() << endl;
  // Check for dir in m_lstDirs
  for ( KURL::List::Iterator it = m_lstDirs.begin(); it != m_lstDirs.end(); ++it )
    if ( dir.cmp( (*it), true /* ignore trailing slash */ ) )
    {
      updateDirectory( *it );
      break;
    }
}

void KDirLister::openURL( const KURL& _url, bool _showDotFiles, bool _keep )
{
  if ( !validURL( _url ) )
    return;

  kdDebug(7003) << "KDirLister::openURL " << _url.prettyURL() << " keep=" << _keep << endl;

  // Complete switch, don't keep previous URLs
  if ( !_keep )
  {
    // Stop running jobs, if any
    stop();

    // clear our internal list
    forgetDirs();
    m_lstFileItems.clear();
    delete m_rootFileItem;
    m_rootFileItem = 0L;

    emit clear();
  }
  else if ( m_lstDirs.contains( _url ) )
  {
    updateDirectory( _url );
    return;
  }

  // TODO: hmm, this means that we could change (!) this setting with _keep == true
  // what about setShowingDotFiles?
  m_isShowingDotFiles = _showDotFiles;

  m_lstDirs.append( _url );

  // Automatic updating of directories ?
  if ( d->autoUpdate && _url.isLocalFile() )
  {
    //kdDebug(7003) << "adding to kdirwatch " << kdirwatch << " " << _url.path() << endl;
    kdirwatch->addDir( _url.path() );
  }

  // we have a limit of MAX_JOBS concurrently running jobs
  if ( d->jobs.count() >= d->MAX_JOBS )
  {
    d->lstPendingUpdates.append( _url );
    return;
  }

  if ( !_keep )
    m_url = _url;

  m_bComplete = false;
  d->urlChanged = false;

  m_job = KIO::listDir( _url, false /* no default GUI */ );
  d->jobs.insert( m_job, QValueList<KIO::UDSEntry>() );

  connect( m_job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ),
           SLOT( slotEntries( KIO::Job*, const KIO::UDSEntryList& ) ) );
  connect( m_job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotResult( KIO::Job * ) ) );
  connect( m_job, SIGNAL( redirection( KIO::Job *, const KURL & ) ),
           this, SLOT( slotRedirection( KIO::Job *, const KURL & ) ) );

  emit started( _url.url() );
}

const KURL& KDirLister::url() const
{
  // ### DON'T CHANGE THIS METHOD BEFORE KDE 3!!
  return m_url;
}

void KDirLister::stop()
{
  // Stop all running jobs
  uint dirs = m_lstDirs.count();
  KIO::ListJob* job;
  QMap< KIO::ListJob *, QValueList<KIO::UDSEntry> >::Iterator it = d->jobs.begin();
  while ( it != d->jobs.end() )
  {
    job = it.key();
    if ( dirs > 1 )
      emit canceled( job->url() );
    job->disconnect( this );
    job->kill();
    ++it;
  }

  if ( !d->jobs.isEmpty() )
  {
    emit canceled();
    d->jobs.clear();
  }

  m_job = 0L;
  m_bComplete = true;
}

void KDirLister::stop( const KURL& _url )
{
  // TODO: consider to stop all the "child jobs" of _url as well
  bool jobsRunning = (!d->jobs.isEmpty());
  KIO::ListJob *job;
  QMap< KIO::ListJob *, QValueList<KIO::UDSEntry> >::Iterator it = d->jobs.begin();
  while ( it != d->jobs.end() )
  {
    job = it.key();
    if ( job->url().cmp( _url, true ) )
    {
      if ( m_job == job )
        m_job = 0L;

      d->jobs.remove( it );
      job->disconnect( this );
      job->kill();
      emit canceled( _url );
      break;
    }
    else
      ++it;
  }

  if ( d->jobs.isEmpty() )    // m_job already 0L
  {
    m_bComplete = true;
    if ( jobsRunning )  // we really killed a job
      emit canceled();
  }
  else if ( !m_job )
    m_job = d->jobs.begin().key();
}

void KDirLister::slotResult( KIO::Job* j )
{
  assert( j );
  KIO::ListJob *job = static_cast<KIO::ListJob *>( j );
  d->jobs.remove( job );

  if ( d->jobs.isEmpty() )
  {
    m_job = 0;
    m_bComplete = true;
  }
  else if ( m_job == job )
    m_job = d->jobs.begin().key();

  if ( job->error() )
  {
    job->showErrorDialog();

    emit canceled( job->url() );
    if ( m_bComplete )
      emit canceled();
  }
  else
  {
    emit completed( job->url() );
    if ( m_bComplete )
      emit completed();
  }

  // TODO: hmm, if there was an error and job is a parent of one or more
  // of the pending urls we should cancel it/them as well
  processPendingUpdates();
}

void KDirLister::slotEntries( KIO::Job* job, const KIO::UDSEntryList& entries )
{
  KFileItemList lstNewItems, lstFilteredItems;
  KIO::UDSEntryListConstIterator it = entries.begin();
  KIO::UDSEntryListConstIterator end = entries.end();

  const KURL& url = static_cast<KIO::ListJob *>(job)->url();

  // avoid creating these QStrings again and again
  static const QString& dot = KGlobal::staticQString(".");

  for (; it != end; ++it) {
    QString name;

    // Find out about the name
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
      if ( !m_rootFileItem && url == m_url ) // only if we didn't keep the previous dir
      {
        m_rootFileItem = createFileItem( *it, url, m_bDelayedMimeTypes );
      }
    }
    else
    {
      //kdDebug(7003)<< "Adding " << url.prettyURL() << endl;
      KFileItem* item = createFileItem( *it, url, m_bDelayedMimeTypes);
      assert( item != 0L );
      bool isNamedFilterMatch = ((m_bDirOnlyMode && !item->isDir()) ||
                                 !matchesFilter( item ));
      bool isMimeFilterMatch = !matchesMimeFilter( item );
      if ( isNamedFilterMatch || isMimeFilterMatch )
      {
        // save only files that are filtered out by mime
        if ( !isNamedFilterMatch && isMimeFilterMatch )
          lstFilteredItems.append( item );
        else
          delete item;
        continue;
      }

      lstNewItems.append( item );
      m_lstFileItems.append( item );
    }
  }

  if ( !lstNewItems.isEmpty() )
    emit newItems( lstNewItems );
  if ( !lstFilteredItems.isEmpty() )
    emit itemsFilteredByMime( lstFilteredItems );
}

void KDirLister::slotRedirection( KIO::Job *job, const KURL & url )
{
  kdDebug(7003) << "KDirLister::slotRedirection " << url.prettyURL() << endl;

  KURL oldUrl;
  if ( job )
  {
    oldUrl = static_cast<KIO::ListJob *>( job )->url();
    if ( m_url.cmp( oldUrl, true ) )
      m_url = url;
  }
  else
  {
    oldUrl = m_url;
    m_url = url;
  }

  *m_lstDirs.find( oldUrl ) = url;

  if ( m_lstDirs.count() == 1 )
  {
    m_lstFileItems.clear();
    emit clear();
    emit redirection( url );
  }
  else
  {
    for ( KFileItem *item = m_lstFileItems.first(); item; item = m_lstFileItems.next() )
    {
      if ( oldUrl.isParentOf( item->url() ) && (!oldUrl.cmp(item->url())) )
        item->unmark();
      else
        item->mark();
    }
    deleteUnmarkedItems();

    emit redirection( oldUrl, url );
  }
}

void KDirLister::updateDirectory( const KURL& _dir )
{
  kdDebug(7003) << "KDirLister::updateDirectory( " << _dir.prettyURL() << " )" << endl;

  // if there's an update running for this _dir restart it
  stop( _dir );

  // we have a limit of MAX_JOBS concurrently running jobs
  if ( d->jobs.count() >= d->MAX_JOBS )
  {
    if ( d->lstPendingUpdates.find( _dir ) == d->lstPendingUpdates.end() )
    {
      //kdDebug(7003) << "KDirLister::updateDirectory -> appending to pending updates" << endl;
      d->lstPendingUpdates.append( _dir );
    }
    else
    {
      //kdDebug(7003) << "KDirLister::updateDirectory -> discarding, already queued" << endl;
    }

    return;
  }

  m_bComplete = false;
  d->urlChanged = false;

  KIO::ListJob* job = KIO::listDir( _dir, false /* no default GUI */ );
  d->jobs.insert( job, QValueList<KIO::UDSEntry>() );

  if ( !m_job )
    m_job = job;

  connect( job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ),
           SLOT( slotUpdateEntries( KIO::Job*, const KIO::UDSEntryList& ) ) );
  connect( job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotUpdateResult( KIO::Job * ) ) );

  kdDebug(7003) << "update started in " << _dir.prettyURL() << endl;

  emit started( _dir.url() );
}

void KDirLister::slotUpdateEntries( KIO::Job* job, const KIO::UDSEntryList& list )
{
  d->jobs[static_cast<KIO::ListJob*>(job)] += list;
}

void KDirLister::slotUpdateResult( KIO::Job * j )
{
  assert( j );
  KIO::ListJob *job = static_cast<KIO::ListJob *>( j );

  if ( d->jobs.count() == 1 )
  {
    m_job = 0;
    m_bComplete = true;
  }
  else if ( m_job == job )
  {
    if ( job == d->jobs.end().key() )
      m_job = d->jobs.begin().key();
    else
      m_job = d->jobs.end().key();
  }

  if ( job->error() )
  {
    //don't bother the user
    //job->showErrorDialog();
    //if ( m_lstDirs.count() > 1 )
    //  emit canceled( job->url() );
    //if ( m_bComplete )
    //  emit canceled();
    // TODO: if job is a parent of one or more
    // of the pending urls we should cancel them
    processPendingUpdates();
    return;
  }

  const KURL& url = job->url();

  QDict<KFileItem> fileItems( 9973 );
  KFileItemList lstNewItems, lstFilteredItems;
  KFileItemList lstRefreshItems;

  // Unmark all items whose path is url
  QString sPath = url.path( 1 ); // with trailing slash
  KFileItemListIterator kit ( m_lstFileItems );
  for ( ; kit.current(); ++kit )
  {
    if ( (*kit)->url().directory( false /* keep trailing slash */, false ) == sPath )
    {
      //kdDebug(7003) << "slotUpdateFinished : unmarking " << (*kit)->url().prettyURL() << endl;
      (*kit)->unmark();
    } else
      (*kit)->mark(); // keep the other items

    fileItems.insert( (*kit)->url().prettyURL(), *kit );
  }

  static const QString& dot = KGlobal::staticQString(".");
  static const QString& dotdot = KGlobal::staticQString("..");
  QValueList<KIO::UDSEntry> buf = d->jobs[job];
  QValueListIterator<KIO::UDSEntry> it = buf.begin();
  for( ; it != buf.end(); ++it )
  {
    QString name;

    // Find out about the name
    KIO::UDSEntry::Iterator it2 = (*it).begin();
    for( ; it2 != (*it).end(); it2++ )
      if ( (*it2).m_uds == KIO::UDS_NAME )
      {
        name = (*it2).m_str;
        break;
      }

    Q_ASSERT( !name.isEmpty() );

    // we duplicate the check for dotdot here, to avoid iterating over
    // all items in m_lstFileItems and checking in matchesFilter() that way.
    if ( name.isEmpty() || name == dot || name == dotdot )
      continue;

    if ( m_isShowingDotFiles || name[0]!='.' )
    {
      // Form the complete url
      KURL u( url );
      u.addPath( name );
      //kdDebug(7003) << "slotUpdateFinished : found " << name << endl;

      // Find this item
      bool found = false;
      KFileItem *tmp;
      if ( (tmp = fileItems[u.prettyURL()]) )
      {
        tmp->mark();
        found = true;
      }

      KFileItem* item = createFileItem( *it, url, m_bDelayedMimeTypes );

      if ( found )
      {
          assert(tmp);
          // Check if something changed for this file
          if ( !tmp->cmp( *item ) )
          {
              tmp->assign( *item );
              lstRefreshItems.append( tmp );
              kdDebug(7003) << "slotUpdateFinished : This file has changed : " << tmp->name() << endl;
          }
          delete item;
      }
      else // This is a new file
      {
        //kdDebug(7003) << "slotUpdateFinished : inserting " << name << endl;
        bool isNamedFilterMatch = ((m_bDirOnlyMode && !item->isDir()) ||
                                   !matchesFilter( item ));
        bool isMimeFilterMatch = !matchesMimeFilter( item );
        if ( isNamedFilterMatch || isMimeFilterMatch )
        {
          if ( !isNamedFilterMatch && isMimeFilterMatch )
            lstFilteredItems.append( item );
          else
            delete item;
          continue;
        }

        //kdDebug(7003) << "slotUpdateFinished : URL= " << item->url().prettyURL() << endl;
        lstNewItems.append( item );
        m_lstFileItems.append( item );
        item->mark();
      }
    }
  }

  if ( !lstNewItems.isEmpty() )
      emit newItems( lstNewItems );
  if ( !lstRefreshItems.isEmpty() )
      emit refreshItems( lstRefreshItems );
  if ( !lstFilteredItems.isEmpty() )
      emit itemsFilteredByMime( lstFilteredItems );

  // unmark the childs
  kit.toFirst();
  for ( ; kit.current(); ++kit )
    if ( !(*kit)->isMarked() )
    {
      KFileItemListIterator kit2( m_lstFileItems );
      for ( ; kit2.current(); ++kit2 )
        if ( (*kit)->url().isParentOf( (*kit2)->url() ) )
          (*kit2)->unmark();
    }

  deleteUnmarkedItems();

  d->jobs.remove( job );

  if ( m_lstDirs.count() > 1 )
    emit completed( job->url() );
  if ( m_bComplete )
    emit completed();

  processPendingUpdates();
}

void KDirLister::processPendingUpdates()
{
  // continue with pending updates
  KURL::List::Iterator pendingIt = d->lstPendingUpdates.begin();
  while ( pendingIt != d->lstPendingUpdates.end() )
  {
    // Check for dir in m_lstDirs - this may have changed since the time
    // we registered the update
    for ( KURL::List::Iterator it = m_lstDirs.begin(); it != m_lstDirs.end(); ++it )
      if ( (*pendingIt).cmp( (*it), true /* ignore trailing slash */ ) )
      {
        kdDebug(7003) << "KDirLister::processPendingUpdates: pending update in " << (*pendingIt).prettyURL() << endl;
        KURL copy = KURL( *pendingIt );  // keep a copy to prevent a crash
        d->lstPendingUpdates.remove( pendingIt );
        updateDirectory( copy );
        return;
      }
    // Drop this update, we're not interested anymore
    d->lstPendingUpdates.remove( pendingIt );
    pendingIt = d->lstPendingUpdates.begin();
  }
}

void KDirLister::deleteUnmarkedItems()
{
  // Find all unmarked items and delete them
  KFileItem* item;
  m_lstFileItems.first();
  while ( (item = m_lstFileItems.current()) )
    if ( !item->isMarked() )
    {
      // unregister and remove the deleted child folders
      // (the child items should have been unmarked already)
      KURL::List::Iterator it = m_lstDirs.begin();
      while ( it != m_lstDirs.end() )
        if ( item->url().isParentOf( *it ) )
        {
          if ( (*it).isLocalFile() )
          {
            //kdDebug(7003) << "forgetting about " << (*it).path() << endl;
            kdirwatch->removeDir( (*it).path() );
          }

          it = m_lstDirs.remove( it );
        }
        else
          ++it;

      m_lstFileItems.take();
      emit deleteItem( item );
      delete item;
    }
    else
      m_lstFileItems.next();
}

void KDirLister::setShowingDotFiles( bool _showDotFiles )
{
  if ( m_isShowingDotFiles != _showDotFiles )
  {
    m_isShowingDotFiles = _showDotFiles;

    if ( !_showDotFiles )
    {
      bool found = false;
      KFileItemListIterator it( m_lstFileItems );
      for ( ; it.current(); ++it )
        if ( (*it)->text()[0] == '.' )
        {
          (*it)->unmark();
          found = true;
        }
        else
          (*it)->mark();

      if ( found )
        deleteUnmarkedItems();
    }
    else
      for ( KURL::List::Iterator it = m_lstDirs.begin(); it != m_lstDirs.end(); ++it )
        updateDirectory( *it ); // update all directories
  }
}

bool KDirLister::showingDotFiles() const
{
  return m_isShowingDotFiles;
}

KFileItem* KDirLister::find( const KURL& _url ) const
{
  KFileItemListIterator it = m_lstFileItems;
  for( ; it.current(); ++it )
  {
    if ( (*it)->url() == _url )
      return (*it);
  }

  return 0L;
}

KFileItem* KDirLister::findByName( const QString& name ) const
{
  KFileItemListIterator it = m_lstFileItems;
  for( ; it.current(); ++it )
  {
    if ( (*it)->name() == name )
      return (*it);
  }

  return 0L;
}

void KDirLister::forgetDirs()
{
  for ( KURL::List::Iterator it = m_lstDirs.begin(); it != m_lstDirs.end(); ++it ) {
    if ( (*it).isLocalFile() )
    {
      //kdDebug(7003) << "forgetting about " << (*it).path() << endl;
      kdirwatch->removeDir( (*it).path() );
    }
  }
  m_lstDirs.clear();
}

KFileItem * KDirLister::createFileItem( const KIO::UDSEntry& entry,
                                        const KURL& url,
                                        bool determineMimeTypeOnDemand )
{
    return new KFileItem( entry, url, determineMimeTypeOnDemand,
                          true /* url is the directory */ );
}

bool KDirLister::matchesFilter( const KFileItem *item ) const
{
    assert( item != 0L );
    static const QString& dotdot = KGlobal::staticQString("..");

    if ( item->text() == dotdot )
        return false;

    if ( !m_isShowingDotFiles && item->text()[0] == '.' )
        return false;

    if (item->isDir() || m_lstFilters.isEmpty())
        return true;

    return matchesFilter( item->text() );
}

bool KDirLister::matchesMimeFilter( const KFileItem *item ) const
{
    assert( item != 0L );

    return matchesMimeFilter( item->mimetype() );
}

bool KDirLister::matchesFilter(const QString& name) const
{
    bool matched = false;
    for (QPtrListIterator<QRegExp> it(m_lstFilters); it.current(); ++it)
        if ( it.current()->match( name ) != -1 ) {
            matched = true;
            break;
        }

    return matched;
}

bool KDirLister::matchesMimeFilter(const QString& mime) const
{
    if ( d->mimeFilter.isEmpty() )
        return true;

    QStringList::Iterator it = d->mimeFilter.begin();
    for ( ; it != d->mimeFilter.end(); ++it )
        if ( (*it) == mime )
            return true;

    return false;
}

void KDirLister::setNameFilter(const QString& nameFilter)
{
    m_lstFilters.clear();
    d->urlChanged = true;
    d->nameFilter = nameFilter;

    QStringList list = QStringList::split(' ', nameFilter);

    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
    {
        // Split on white space
        m_lstFilters.append(new QRegExp(*it, false, true ));
    }
}

void KDirLister::setMimeFilter( const QStringList& mimefilter )
{
    d->mimeFilter = mimefilter;
}

QStringList KDirLister::mimeFilters() const
{
    return d->mimeFilter;
}

void KDirLister::clearMimeFilter()
{
    d->mimeFilter.clear();
}

// ## the deprecated one
void KDirLister::setMimeFilter( const QString& mimefilter )
{
    //kdWarning(7003) << "This is the deprecated setMimeFilter( " << mimefilter << " )!!!" << endl;
    d->mimeFilter = QStringList::split(' ', mimefilter);
}

// ## the deprecated one
const QString& KDirLister::mimeFilter() const
{
    //kdWarning(7003) << "This is the deprecated mimeFilter()!!!" << endl;
    d->deprecated_mimeFilter = QString::null;
    QStringList::ConstIterator it = d->mimeFilter.begin();
    while ( it != d->mimeFilter.end() ) {
        if ( it != d->mimeFilter.begin() )
            d->deprecated_mimeFilter += ' ';
        d->deprecated_mimeFilter += *it;

        ++it;
    }

    return d->deprecated_mimeFilter;
}

const QString& KDirLister::nameFilter() const
{
    return d->nameFilter;
}

void KDirLister::FilesAdded( const KURL & directory )
{
  kdDebug(7003) << "FilesAdded " << directory.prettyURL() << " - we are showing " << m_url.prettyURL() << endl;
  slotURLDirty( directory );
}

void KDirLister::FilesRemoved( const KURL::List & fileList )
{
  Q_ASSERT( !fileList.isEmpty() );
  // Mark all items
  KFileItemListIterator kit ( m_lstFileItems );
  for( ; kit.current(); ++kit )
    (*kit)->mark();

  KURL::List::ConstIterator it = fileList.begin();
  for ( ; it != fileList.end() ; ++it )
  {
    //kdDebug(7003) << "FilesRemoved: " << (*it).prettyURL() << endl;
    // For each file removed: look in m_lstFileItems to see if we know it,
    // and if found, unmark it (for deletion)
    kit.toFirst();
    for( ; kit.current(); ++kit )
    {
      if ( (*kit)->url().cmp( (*it), true /* ignore trailing slash */ ) )
      {
        //kdDebug(7003) << "FilesRemoved : unmarking " << (*kit)->url().prettyURL() << endl;
        (*kit)->unmark();
        break;
      }
    }

    if ( !kit.current() ) // we didn't find it
    {
      // maybe it's the dir we're listing (or a parent of it) ?
      // Check for dir in m_lstDirs, if in single-URL mode
      // Do NOT do this if more than one dir. A tree view is never deleted.
      if ( m_lstDirs.count() == 1 )
      {
        if ( (*it).isParentOf(m_lstDirs.first()) )
        {
          kdDebug( 7003 ) << (*it).prettyURL() << " is a parent of " << m_lstDirs.first().prettyURL() << endl;
          //kdDebug(7003) << "emit closeView" << endl;
          stop();
          emit closeView();
          return;
        }
      }
    }
  }

  // unmark the childs
  kit.toFirst();
  for ( ; kit.current(); ++kit )
    if ( !(*kit)->isMarked() )
    {
      KFileItemListIterator kit2( m_lstFileItems );
      for ( ; kit2.current(); ++kit2 )
        if ( (*kit)->url().isParentOf( (*kit2)->url() ) )
          (*kit2)->unmark();
    }

  deleteUnmarkedItems();
}

void KDirLister::FilesChanged( const KURL::List & fileList )
{
  KURL::List dirs;
  KFileItemListIterator kit ( m_lstFileItems );
  KURL::List::ConstIterator it = fileList.begin();
  for ( ; it != fileList.end() ; ++it )
  {
    kit.toFirst();
    for( ; kit.current(); ++kit )
    {
      if ( (*kit)->url().cmp( (*it), true /* ignore trailing slash */ ) )
      {
        (*kit)->refresh();
        KFileItemList lst;
        lst.append( *kit );
        emit refreshItems( lst );
        KURL dir(*it);
        dir.setPath( (*it).directory() );
        if ( !dirs.contains( dir ) )
          dirs.append(dir);
      }
    }
  }
  // Tricky. If an update is running at the same time, it might have
  // listed those files before, and emit outdated info (reverting this method's work)
  // So we need to abort any running update, and relaunch it. (David)
  //
  // even more tricky: we need to relaunch even if there is a new listjob
  // running, not just an update. (Michael)
  //
  // And _even_ more tricky: if the changed item isn't in our list yet, then skip
  // this code (otherwise we abort but don't update anything!) (David)
  //
  // Ok, finally the most tricky thingie (gets really out of hand now :): only
  // stop jobs listing one of the 'dirs', and only update the directory, if
  // the job was really running (David, Michael)
  if ( !m_bComplete )
  {
    KURL::List dirsToBeUpdated;

    it = dirs.begin();
    for ( ; it != dirs.end() ; ++it )
    {
      QMap< KIO::ListJob *, QValueList<KIO::UDSEntry> >::Iterator job = d->jobs.begin();
      for ( ; job != d->jobs.end(); ++job )
        if ( job.key()->url().cmp( (*it), true ) )
          dirsToBeUpdated.append( (*it) );
    }

    it = dirsToBeUpdated.begin();
    for ( ; it != dirsToBeUpdated.end() ; ++it )
      updateDirectory( (*it ) );
  }
}

void KDirLister::FileRenamed( const KURL &src, const KURL &dst )
{
    kdDebug( 7003 ) << "FileRenamed " << src.prettyURL() << " -- " << dst.prettyURL() << endl;
    if ( m_rootFileItem )
        kdDebug( 7003 ) << "root url is " << m_rootFileItem->url().prettyURL() << endl;

    if ( m_rootFileItem && m_rootFileItem->url() == src )
    {
        m_rootFileItem->setURL( dst );
        slotRedirection( 0L, dst );
    }
    else
    {
        KFileItemListIterator kit ( m_lstFileItems );
        for( ; kit.current(); ++kit )
        {
          if ( (*kit)->url().cmp( src, true /* ignore trailing slash */ ) )
          {
              kdDebug( 7003 ) << "Found item, renamed it" << endl;
              (*kit)->setURL( dst );
              (*kit)->refreshMimeType(); // The new name might lead to a new mimetype
              KFileItemList lst;
              lst.append( *kit );
              emit refreshItems( lst );

              KURL dir( dst );
              dir.setPath( dst.directory() );
              if ( !m_bComplete )
              {
                QMap< KIO::ListJob *, QValueList<KIO::UDSEntry> >::Iterator job = d->jobs.begin();
                for ( ; job != d->jobs.end(); ++job )
                  if ( job.key()->url().cmp( dir, true ) )
                  {
                    updateDirectory( dir );
                    break;
                  }
              }
              break;
          }
        }
    }
}

void KDirLister::setAutoUpdate( bool enable )
{
    if ( d->autoUpdate == enable )
        return;

    d->autoUpdate = enable;

    for ( KURL::List::Iterator it = m_lstDirs.begin(); it != m_lstDirs.end();
          ++it ) {
        if ( (*it).isLocalFile() ) {
            if ( enable )
                kdirwatch->addDir( (*it).path() );
            else
                kdirwatch->removeDir( (*it).path() );
        }
    }

    if ( enable ) {
      connect( kdirwatch, SIGNAL( dirty( const QString& ) ),
               this, SLOT( slotDirectoryDirty( const QString& ) ) );
      connect( kdirwatch, SIGNAL( fileDirty( const QString& ) ),
               this, SLOT( slotFileDirty( const QString& ) ) );
    }
    else
        kdirwatch->disconnect( this );
}

bool KDirLister::autoUpdate() const
{
    return d->autoUpdate;
}

bool KDirLister::setURL( const KURL& url )
{
   if ( !validURL( url ) )
        return false;

    d->urlChanged |= !(url.cmp( m_url, true ) && m_bComplete);
    stop();
    forgetDirs();
    m_url = url;
    return true;
}

void KDirLister::listDirectory()
{
    if ( m_bComplete && !d->urlChanged ) {
        emit clear();
        emit newItems( m_lstFileItems );
        emit completed();
    }
    else {
        openURL( m_url, showingDotFiles() );
    }
}

void KDirLister::setURLDirty( bool dirty )
{
    d->urlChanged = dirty;
}

bool KDirLister::validURL( const KURL& url ) const
{
  if ( url.isMalformed() )
  {
    QString tmp = i18n("Malformed URL\n%1").arg(url.url());
    KMessageBox::error( (QWidget*)0L, tmp);
    return false;
  }

  // TODO: perhaps verify that this is really a directory?

  return true;
}


#include "kdirlister.moc"
