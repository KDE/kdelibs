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

class KDirLister::KDirListerPrivate
{
public:
    KDirListerPrivate() { }
    KURL::List lstPendingUpdates;
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
  // Check for dir in m_lstDirs
  for ( KURL::List::Iterator it = m_lstDirs.begin(); it != m_lstDirs.end(); ++it )
    if ( dir.cmp( (*it), true /* ignore trailing slash */ ) )
    {
      updateDirectory( dir );
      break;
    }
}

void KDirLister::openURL( const KURL& _url, bool _showDotFiles, bool _keep )
{
  if ( _url.isMalformed() )
  {
    QString tmp = i18n("Malformed URL\n%1").arg(_url.url());
    KMessageBox::error( (QWidget*)0L, tmp);
    return;
  }

  m_isShowingDotFiles = _showDotFiles;

  // Stop running jobs, if any
  stop();

  // Complete switch, don't keep previous URLs
  if ( !_keep )
    forgetDirs();

  // Automatic updating of directories ?
  if ( _url.isLocalFile() )
  {
    //kdDebug(7003) << "adding " << _url.path() << endl;
    kdirwatch->addDir( _url.path() );
    if ( !_keep ) // already done if keep == true
    {
      connect( kdirwatch, SIGNAL( dirty( const QString& ) ),
               this, SLOT( slotDirectoryDirty( const QString& ) ) );
      connect( kdirwatch, SIGNAL( fileDirty( const QString& ) ),
               this, SLOT( slotFileDirty( const QString& ) ) );
    }
  }
  m_lstDirs.append( _url );

  m_bComplete = false;

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
    emit completed();
}


void KDirLister::slotEntries( KIO::Job*, const KIO::UDSEntryList& entries )
{
  KFileItemList lstNewItems;
  KIO::UDSEntryListConstIterator it = entries.begin();
  KIO::UDSEntryListConstIterator end = entries.end();

  // avoid creating these QStrings again and again
  static const QString dot = QString::fromLatin1(".");

  for (; it != end; ++it) {
    QString name;

    // Find out about the name
    KIO::UDSEntry::ConstIterator entit = (*it).begin();
    for( ; entit != (*it).end(); ++entit )
      if ( (*entit).m_uds == KIO::UDS_NAME )
        name = (*entit).m_str;

    assert( !name.isEmpty() );

    if ( name == dot )
    {
      if ( !m_rootFileItem ) // only if we didn't keep the previous dir
      {
        m_rootFileItem = createFileItem( *it, m_url, m_bDelayedMimeTypes );
      }
    }
    else
    {
      //kdDebug(7003)<< "Adding " << u.url() << endl;
      KFileItem* item = createFileItem( *it, m_url, m_bDelayedMimeTypes,
					true );
      assert( item != 0L );

      if ( (m_bDirOnlyMode && !S_ISDIR( item->mode() )) || !matchesFilter( item ))
      {
        delete item;
        continue;
      }

      lstNewItems.append( item );
      m_lstFileItems.append( item );

    }
  }
  if (!lstNewItems.isEmpty())
      emit newItems( lstNewItems );
}

void KDirLister::slotRedirection( KIO::Job *, const KURL & url )
{
  kdDebug(7003) << "KDirLister::slotRedirection " << url.url() << endl;
  m_url = url;
  emit redirection( url );
}

void KDirLister::updateDirectory( const KURL& _dir )
{
  kdDebug(7003) << "KDirLister::updateDirectory( " << _dir.url() << " )" << endl;
  if ( !m_bComplete )
  {
    d->lstPendingUpdates.append( _dir );
    return;
  }

  // Stop running jobs
  stop();

  m_bComplete = false;
  m_buffer.clear();

  m_url = _dir;
  m_job = KIO::listDir( m_url, false /* no default GUI */ );
  connect( m_job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList&)),
           SLOT( slotUpdateEntries( KIO::Job*, const KIO::UDSEntryList&)));
  connect( m_job, SIGNAL( result( KIO::Job * ) ),
	   SLOT( slotUpdateResult( KIO::Job * ) ) );

  kdDebug(7003) << "update started in " << m_url.url() << endl;

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

  KFileItemList lstNewItems;
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
      //kdDebug(7003) << "slotUpdateFinished : unmarking " << (*kit)->url().url() << endl;
      (*kit)->unmark();
    } else
      (*kit)->mark(); // keep the other items
  }

  QValueListIterator<KIO::UDSEntry> it = m_buffer.begin();
  for( ; it != m_buffer.end(); ++it )
  {
    QString name;

    // Find out about the name
    KIO::UDSEntry::Iterator it2 = (*it).begin();
    for( ; it2 != (*it).end(); it2++ )
      if ( (*it2).m_uds == KIO::UDS_NAME )
        name = (*it2).m_str;

    assert( !name.isEmpty() );

    if ( name == "." || name == ".." )
      continue;

    if ( m_isShowingDotFiles || name[0]!='.' )
    {
      // Form the complete url
      KURL u( m_url );
      u.addPath( name );
      //kdDebug(7003) << "slotUpdateFinished : found " << name << endl;

      // Find this icon
      bool done = false;
      QListIterator<KFileItem> kit ( m_lstFileItems );
      for( ; kit.current() && !done; ++kit )
      {
        if ( u == (*kit)->url() )
        {
          //kdDebug(7003) << "slotUpdateFinished : keeping " << name << endl;
          (*kit)->mark();
          done = true;
        }
      }

      if ( !done )
      {
        //kdDebug(7003) << "slotUpdateFinished : inserting " << name << endl;
        KFileItem* item = createFileItem( *it, u, m_bDelayedMimeTypes );
	
	if ( m_bDirOnlyMode && !S_ISDIR( item->mode() ) )
	{
	  delete item;
	  continue;
	}
	
        m_lstFileItems.append( item );
        lstNewItems.append( item );
        item->mark();
      }
    }
  }

  emit newItems( lstNewItems );

  deleteUnmarkedItems();

  m_buffer.clear();

  emit completed();

  // continue with pending updates
  // as this will result in a recursive loop it's sufficient to only
  // take the first entry
  pendingIt = d->lstPendingUpdates.begin();
  if ( pendingIt != d->lstPendingUpdates.end() )
    updateDirectory( *pendingIt );
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

KFileItem* KDirLister::find( const KURL& _url )
{
  QListIterator<KFileItem> it = m_lstFileItems;
  for( ; it.current(); ++it )
  {
    if ( (*it)->url() == _url )
      return (*it);
  }

  return 0L;
}

void KDirLister::forgetDirs()
{
  for ( KURL::List::Iterator it = m_lstDirs.begin(); it != m_lstDirs.end(); ++it ) {
    if ( (*it).isLocalFile() )
    {
      kdDebug(7003) << "forgetting about " << (*it).path() << endl;
      kdirwatch->removeDir( (*it).path() );
    }
  }
  m_lstDirs.clear();
  kdirwatch->disconnect( this );
}

KFileItem * KDirLister::createFileItem( const KIO::UDSEntry& entry,
					const KURL& url,
					bool determineMimeTypeOnDemand,
					bool urlIsDirectory )
{
    return new KFileItem( entry, url, determineMimeTypeOnDemand,
			  urlIsDirectory );
}

bool KDirLister::matchesFilter( const KFileItem *item )
{
    assert( item != 0L );

    static const QString dotdot = QString::fromLatin1("..");
    if ( item->text() == dotdot )
	return false;

    if ( !m_isShowingDotFiles && item->text()[0] == '.' )
	return false;
	
    if (item->isDir() || m_lstFilters.isEmpty())
	return true;

    return matchesFilter( item->text() );
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

void KDirLister::setNameFilter(const QString& nameFilter)
{
    m_lstFilters.clear();

    if ( !nameFilter.isEmpty() ) {
        // Split on white space
        char *s = qstrdup(nameFilter.latin1());
        char *g = strtok(s, " ");
        while (g) {
            m_lstFilters.append(new QRegExp(QString::fromLatin1(g), false, true ));
            g = strtok(0, " ");
        }
        delete [] s;
    }
}

void KDirLister::FilesAdded( const KURL & directory )
{
  //kdDebug(7003) << "FilesAdded " << directory.url() << " - we are showing " << m_lstDirs.first().url() << endl;
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
    //kdDebug(7003) << "FilesRemoved: " << (*it).url() << endl;
    // For each file removed: look in m_lstFileItems to see if we know it,
    // and if found, unmark it (for deletion)
    kit.toFirst();
    for( ; kit.current(); ++kit )
    {
      if ( (*kit)->url().cmp( (*it), true /* ignore trailing slash */ ) )
      {
        //kdDebug(7003) << "FilesRemoved : unmarking " << (*kit)->url().url() << endl;
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
          kdDebug(7003) << "emit closeView" << endl;
          stop();
          emit closeView();
          return;
        }
      }
    }
  }

  deleteUnmarkedItems();
}

#include "kdirlister.moc"
