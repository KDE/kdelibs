/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
                 2000 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include <kapp.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <klocale.h>
#include <kio/job.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kglobalsettings.h>

template class QList<QRegExp>;

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
  setAutoUpdate( true );
  d->urlChanged = false;
}

KDirLister::~KDirLister()
{
  // Stop running jobs
  stop();
  delete m_rootFileItem;
  forgetDirs();
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
  m_isShowingDotFiles = _showDotFiles;

  // Stop running jobs, if any
  stop();

  // Complete switch, don't keep previous URLs
  if ( !_keep )
    forgetDirs();

  // Automatic updating of directories ?
  if ( d->autoUpdate && _url.isLocalFile() )
  {
    //kdDebug(7003) << "adding to kdirwatch " << kdirwatch << " " << _url.path() << endl;
    kdirwatch->addDir( _url.path() );
  }
  m_lstDirs.append( _url );

  m_bComplete = false;

  d->urlChanged = false;
  m_url = _url; // keep a copy
  m_job = KIO::listDir( m_url, false /* no default GUI */ );
  connect( m_job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList&)),
           SLOT( slotEntries( KIO::Job*, const KIO::UDSEntryList&)));
  connect( m_job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotResult( KIO::Job * ) ) );
  connect( m_job, SIGNAL( redirection( KIO::Job *, const KURL & ) ),
           this, SLOT( slotRedirection( KIO::Job *, const KURL & ) ) );

  emit started( m_url.url() );
  if ( !_keep )
  {
    emit clear();
    m_lstFileItems.clear(); // clear our internal list
    delete m_rootFileItem;
    m_rootFileItem = 0L;
  }
}

void KDirLister::stop()
{
  // Stop running jobs
  if ( m_job )
  {
    m_job->disconnect( this );
    m_job->kill();
    m_job = 0;
  }
  m_bComplete = true;
}

void KDirLister::slotResult( KIO::Job * job )
{
  m_job = 0;
  m_bComplete = true;
  if (job && job->error())
  {
    job->showErrorDialog();
    emit canceled();
  } else
  {
      emit completed();
      processPendingUpdates();
  }
}


void KDirLister::slotEntries( KIO::Job*, const KIO::UDSEntryList& entries )
{
  KFileItemList lstNewItems, lstFilteredItems;
  KIO::UDSEntryListConstIterator it = entries.begin();
  KIO::UDSEntryListConstIterator end = entries.end();

  // avoid creating these QStrings again and again
  static const QString& dot = KGlobal::staticQString(".");

  for (; it != end; ++it) {
    QString name;

    // Find out about the name
    KIO::UDSEntry::ConstIterator entit = (*it).begin();
    for( ; entit != (*it).end(); ++entit )
      if ( (*entit).m_uds == KIO::UDS_NAME )
        name = (*entit).m_str;

    ASSERT( !name.isEmpty() );
    if ( name.isEmpty() )
      continue;

    if ( name == dot )
    {
      if ( !m_rootFileItem ) // only if we didn't keep the previous dir
      {
        m_rootFileItem = createFileItem( *it, m_url, m_bDelayedMimeTypes );
      }
    }
    else
    {
      //kdDebug(7003)<< "Adding " << u.prettyURL() << endl;
      KFileItem* item = createFileItem( *it, m_url, m_bDelayedMimeTypes);
      assert( item != 0L );
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
      lstNewItems.append( item );
      m_lstFileItems.append( item );
    }
  }
  if (!lstNewItems.isEmpty())
    emit newItems( lstNewItems );
  if ( !lstFilteredItems.isEmpty() )
    emit itemsFilteredByMime( lstFilteredItems );
}

void KDirLister::slotRedirection( KIO::Job *, const KURL & url )
{
  kdDebug(7003) << "KDirLister::slotRedirection " << url.prettyURL() << endl;
  KURL oldUrl = m_url;
  m_url = url;
  if ( m_lstDirs.count() == 1 )
  {
      //kdDebug( 7003 ) << "setting first URL to " << url.prettyURL() << endl;
      m_lstDirs.first() = m_url;
  }
  if ( !m_lstFileItems.isEmpty() )
  {
      //kdDebug( 7003 ) << "getting rid of current stuff by emitting clear" << endl;
      emit clear();
  }
  emit redirection( url );
  emit redirection( oldUrl, url );
}

void KDirLister::updateDirectory( const KURL& _dir )
{
  kdDebug(7003) << "KDirLister::updateDirectory( " << _dir.prettyURL() << " )" << endl;
  if ( !m_bComplete )
  {
    //kdDebug(7003) << "KDirLister::updateDirectory -> appending to pending updates " << endl;
    d->lstPendingUpdates.append( _dir );
    return;
  }

  // Stop running jobs
  stop();

  m_bComplete = false;
  m_buffer.clear();

  d->urlChanged = false;
  m_url = _dir;
  m_job = KIO::listDir( m_url, false /* no default GUI */ );
  connect( m_job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList&)),
           SLOT( slotUpdateEntries( KIO::Job*, const KIO::UDSEntryList&)));
  connect( m_job, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotUpdateResult( KIO::Job * ) ) );

  kdDebug(7003) << "update started in " << m_url.prettyURL() << endl;

  emit started( m_url.url() );
}

void KDirLister::slotUpdateResult( KIO::Job * job )
{
  m_job = 0;
  m_bComplete = true;
  if (job->error())
  {
    //don't bother the user
    //job->showErrorDialog();
    //emit canceled();
    return;
  }

  KFileItemList lstNewItems, lstFilteredItems;
  KFileItemList lstRefreshItems;
  KURL::List::Iterator pendingIt = d->lstPendingUpdates.find( m_url );
  if ( pendingIt != d->lstPendingUpdates.end() )
    d->lstPendingUpdates.remove( pendingIt );

  // Unmark all items whose path is m_url
  QString sPath = m_url.path( 1 ); // with trailing slash
  QListIterator<KFileItem> kit ( m_lstFileItems );
  for( ; kit.current(); ++kit )
  {
    if ( (*kit)->url().directory( false /* keep trailing slash */, false ) == sPath )
    {
      //kdDebug(7003) << "slotUpdateFinished : unmarking " << (*kit)->url().prettyURL() << endl;
      (*kit)->unmark();
    } else
      (*kit)->mark(); // keep the other items
  }

  static const QString& dot = KGlobal::staticQString(".");
  static const QString& dotdot = KGlobal::staticQString("..");
  QValueListIterator<KIO::UDSEntry> it = m_buffer.begin();
  for( ; it != m_buffer.end(); ++it )
  {
    QString name;

    // Find out about the name
    KIO::UDSEntry::Iterator it2 = (*it).begin();
    for( ; it2 != (*it).end(); it2++ )
      if ( (*it2).m_uds == KIO::UDS_NAME )
        name = (*it2).m_str;

    ASSERT( !name.isEmpty() );

    // we duplicate the check for dotdot here, to avoid iterating over
    // all items in m_lstFileItems and checking in matchesFilter() that way.
    if ( name.isEmpty() || name == dot || name == dotdot )
      continue;

    if ( m_isShowingDotFiles || name[0]!='.' )
    {
      // Form the complete url
      KURL u( m_url );
      u.addPath( name );
      //kdDebug(7003) << "slotUpdateFinished : found " << name << endl;

      // Find this item
      bool found = false;
      QListIterator<KFileItem> kit ( m_lstFileItems );
      for( ; kit.current(); ++kit )
      {
        if ( u == (*kit)->url() )
        {
          //kdDebug(7003) << "slotUpdateFinished : keeping " << name << endl;
          (*kit)->mark();
          found = true;
          break;
        }
      }

      KFileItem* item = createFileItem( *it, m_url, m_bDelayedMimeTypes );

      if ( found )
      {
          assert(kit.current());
          // Check if something changed for this file
          if ( ! (*kit)->cmp( *item ) )
          {
              (*kit)->assign( *item );
              (*kit)->mark(); // just in case
              lstRefreshItems.append( (*kit) );
              kdDebug(7003) << "slotUpdateFinished : This file has changed : " << (*kit)->name() << endl;
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
        m_lstFileItems.append( item );
        lstNewItems.append( item );
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

  deleteUnmarkedItems();

  m_buffer.clear();

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
        kdDebug(7003) << "KDirLister::processPendingUpdates pending update in " << (*pendingIt).prettyURL() << endl;
        updateDirectory( *pendingIt );
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
  QList<KFileItem> lst;
  QListIterator<KFileItem> kit ( m_lstFileItems );
  for( ; kit.current(); ++kit )
  {
    if ( !(*kit)->isMarked() )
    {
      //kdDebug(7003) << "Removing " << (*kit)->text() << endl;
      lst.append( *kit );
    }
  }

  KFileItem* kci;
  for( kci = lst.first(); kci != 0L; kci = lst.next() )
  {
    emit deleteItem( kci );
    m_lstFileItems.remove( kci );
  }
}

void KDirLister::slotUpdateEntries( KIO::Job*, const KIO::UDSEntryList& list )
{
  m_buffer += list;
}

void KDirLister::setShowingDotFiles( bool _showDotFiles )
{
  if ( m_isShowingDotFiles != _showDotFiles )
  {
    m_isShowingDotFiles = _showDotFiles;
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
  QListIterator<KFileItem> it = m_lstFileItems;
  for( ; it.current(); ++it )
  {
    if ( (*it)->url() == _url )
      return (*it);
  }

  return 0L;
}

KFileItem* KDirLister::findByName( const QString& name ) const
{
  QListIterator<KFileItem> it = m_lstFileItems;
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
    for (QListIterator<QRegExp> it(m_lstFilters); it.current(); ++it)
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
    d->mimeFilter = QStringList::split(' ', mimefilter);
}

// ## the deprecated one
const QString& KDirLister::mimeFilter() const
{
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
  ASSERT( !fileList.isEmpty() );
  // Mark all items
  QListIterator<KFileItem> kit ( m_lstFileItems );
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

  deleteUnmarkedItems();
}

void KDirLister::FilesChanged( const KURL::List & fileList )
{
  KURL::List dirs;
  QListIterator<KFileItem> kit ( m_lstFileItems );
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
  // So we need to abort any running update, and relaunch it.
  if ( d->autoUpdate && !m_bComplete )
  {
      stop();
      KURL::List::ConstIterator it = dirs.begin();
      for ( ; it != dirs.end() ; ++it )
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
        QListIterator<KFileItem> kit ( m_lstFileItems );
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

    d->urlChanged |= !(url == m_url && m_bComplete);
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
    else
	openURL( m_url, showingDotFiles() );
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
  return true;
}


#include "kdirlister.moc"
