// -*- c-basic-offset: 2 -*-
/* This file is part of the KDE libraries
   Copyright (C) 1998 Sven Radej <sven@lisa.exp.univie.ac.at>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


// CHANGES:
// Mar 30, 2001 - Native support for Linux dir change notification.
// Jan 28, 2000 - Usage of FAM service on IRIX (Josef.Weidendorfer@in.tum.de)
// May 24. 1998 - List of times introduced, and some bugs are fixed. (sven)
// May 23. 1998 - Removed static pointer - you can have more instances.
// It was Needed for KRegistry. KDirWatch now emits signals and doesn't
// call (or need) KFM. No more URL's - just plain paths. (sven)
// Mar 29. 1998 - added docs, stop/restart for particular Dirs and
// deep copies for list of dirs. (sven)
// Mar 28. 1998 - Created.  (sven)


#include <config.h>

#ifdef HAVE_DNOTIFY
#include <fcntl.h>
#include <signal.h>
#include <qintdict.h>
#endif

#include <qptrlist.h>
#include <qtimer.h>
#include <qstringlist.h>
#include <qfile.h>
#include <qdir.h>
#include <qsocketnotifier.h>

#include <kapplication.h>
#include <kdebug.h>

#include "kdirwatch.h"
#include "kdirwatch_p.h"


#define NO_NOTIFY (time_t) 0


#ifdef HAVE_DNOTIFY
static KDirWatchPrivate* dwp = 0;
void KDirWatchPrivate::dnotify_handler(int, siginfo_t *si, void *)
{
  Entry* e = (dwp) ? dwp->fd_Entry.find(si->si_fd) :0;

  if(!e || e->dn_fd != si->si_fd) {
    qDebug("fatal error in KDirWatch");
  } else
    e->dn_dirty = true;

  char c;
  write(dwp->mPipe[1], &c, 1);
}
#endif


//
// Class KDirWatchPrivate (singleton)
//

KDirWatchPrivate::KDirWatchPrivate(int _freq)
{
  timer = new QTimer(this);
  connect (timer, SIGNAL(timeout()), this, SLOT(slotRescan()));
  freq = _freq;

#ifdef HAVE_FAM
  // It's possible that FAM server can't be started
  if (FAMOpen(&fc) ==0) {
    kdDebug(7001) << "KDirWatch: Using FAM" << endl;
    use_fam=true;
    emitEvents = true;
    sn = new QSocketNotifier( FAMCONNECTION_GETFD(&fc),
			      QSocketNotifier::Read, this);
    connect( sn, SIGNAL(activated(int)),
 	     this, SLOT(famEventReceived()) );
  }
  else {
    kdDebug(7001) << "KDirWatch: Can't use FAM" << endl;
    use_fam=false;
  }
#endif

#ifdef HAVE_DNOTIFY
#ifdef HAVE_FAM
  supports_dnotify = !use_fam;
#else
  supports_dnotify = true; // not guilty until proven guilty
#endif
  if (supports_dnotify)
  {
     kdDebug(7001) << "KDirWatch: Will use Linux Directory Notifications when available." << endl;

     dwp = this; // for dnotify_handler access to this

     pipe(mPipe);
     mSn = new QSocketNotifier( mPipe[0], QSocketNotifier::Read, this);
     connect(mSn, SIGNAL(activated(int)), this, SLOT(slotActivated()));
     connect(&mTimer, SIGNAL(timeout()), this, SLOT(slotRescan()));
     struct sigaction act;
     act.sa_sigaction = KDirWatchPrivate::dnotify_handler;
     sigemptyset(&act.sa_mask);
     act.sa_flags = SA_SIGINFO;
#ifdef SA_RESTART
     act.sa_flags |= SA_RESTART;
#endif
     sigaction(SIGRTMIN, &act, NULL);
  }
#endif

}



KDirWatchPrivate::~KDirWatchPrivate()
{
  timer->stop();

  // don't leak FD's etc.
  EntryMap::Iterator it = m_mapDirs.begin();
  while(it != m_mapDirs.end()) {
    (*it).m_clients = 1;
    (*it).m_entries.clear();
    removeDir(it.key(),0);
    it = m_mapDirs.begin();
  }

#ifdef HAVE_FAM
  if (use_fam) {
    FAMClose(&fc);
    kdDebug(7001) << "KDirWatch deleted (FAM closed)" << endl;
  }
#endif

}

#ifdef HAVE_DNOTIFY
void KDirWatchPrivate::slotActivated()
{
   char dummy_buf[100];
   read(mPipe[0], &dummy_buf, 100);
   
   if (!mTimer.isActive())
      mTimer.start(200, true);
}

void KDirWatchPrivate::Entry::propagate_dirty()
{
  Entry* sub_entry;
  for(sub_entry = m_entries.first(); sub_entry; sub_entry = m_entries.next())
  {
     if (!sub_entry->dn_dirty)
     {
        sub_entry->dn_dirty = true;
        sub_entry->propagate_dirty();
     }
  }
}

#else // !HAVE_DNOTIFY
// slots always have to be defined... 
void KDirWatchPrivate::slotActivated() {}
#endif


KDirWatchPrivate::Entry* KDirWatchPrivate::entry(const QString& _path)
{
// we only support absolute paths
  assert( _path.left(1) == "/");

  QString path = _path;

  if ( path.length() > 1 && path.right(1) == "/" )
    path.truncate( path.length() - 1 );

  EntryMap::Iterator it = m_mapDirs.find( path );
  if ( it == m_mapDirs.end() )
    return 0;
  else
    return &(*it);
}

void KDirWatchPrivate::addDir( const QString& _path, Entry* sub_entry )
{
  QString path = _path;

  if ( path.length() > 1 && path.right(1) == "/" )
    path.truncate( path.length() - 1 );

  EntryMap::Iterator it = m_mapDirs.find( path );
  if ( it != m_mapDirs.end() )
  {
    if (sub_entry) {
       (*it).m_entries.append(sub_entry);
       kdDebug(7001) << "Added already watched " << path 
		     << " (for " << sub_entry->path << ")" << endl;
    }
    else {
       (*it).m_clients++;
       kdDebug(7001) << "Added already watched " << path
		     << " (" <<  (*it).m_clients << " clients)" << endl;
    }
    return;
  }


  struct stat statbuff;
  Entry e;

  e.path = path;
  if (sub_entry)
  {
     e.m_entries.append(sub_entry);
     e.m_clients = 0;
  }
  else
  {
     e.m_clients = 1;
  }

  if (stat( QFile::encodeName(path), &statbuff ) == 0)
  {
     e.m_ctime = statbuff.st_ctime;
     e.m_status = Normal;
  }
  else
  {
     e.m_ctime = NO_NOTIFY;
     e.m_status = NonExistent;
  }

  kdDebug(7001) << "Added " << path 
		<< QString((e.m_status==NonExistent)?" NotExisting":"")
		<< (sub_entry ? QString(" for %1").arg(sub_entry->path) : "")
		<< endl;

#ifdef HAVE_DNOTIFY
  e.dn_fd = 0;
#endif

  m_mapDirs.insert( path, e );
  // the insert does a copy, so to modify e we have to use ep
  Entry* ep = &(m_mapDirs[path]);

#if defined(HAVE_FAM)
  if (use_fam) {
    if (e.m_status == NonExistent) {
      // If the directory does not exist we watch the parent directory
      addDir(QDir::cleanDirPath(path+"/.."), ep);
    }
    else {
      FAMMonitorDirectory(&fc, QFile::encodeName(path),
			  &(ep->fr), 0);
      kdDebug(7001) << " -> FAMReq " << FAMREQUEST_GETREQNUM(&(ep->fr))
		    << endl;
    }
  }
#endif

#ifdef HAVE_DNOTIFY
  if(supports_dnotify) {
    ep->dn_dirty = false;
    if (ep->m_status == Normal)
    {
       int fd = open(QFile::encodeName(path).data(), O_RDONLY);
       if(fd < 0 ||
            fcntl(fd, F_SETSIG, SIGRTMIN) < 0 ||
            fcntl(fd, F_NOTIFY,
		  DN_DELETE|DN_CREATE|DN_RENAME|DN_ATTRIB|DN_MULTISHOT) < 0)
       {
          if(fd >= 0) {
             kdDebug(7001) << "KDirWatch: Not using Linux Directory Notifications." << endl;
             supports_dnotify = false;
             ::close(fd);
          }
       }
       else
       {
          fd_Entry.replace(fd, ep);
          ep->dn_fd = fd;
       }
    }
    if(supports_dnotify) {
      if (path != "/")
      {
         addDir(QDir::cleanDirPath(path+"/.."), ep);
      }
    }
  }
#endif

#ifdef HAVE_FAM
  // if FAM server can't be used, fall back to good old timer...
  if (!use_fam)
#endif
#ifdef HAVE_DNOTIFY
  // if DNotify  can't be used, fall back to good old timer...
  if (!supports_dnotify)
#endif
  if ( m_mapDirs.count() == 1 ) // if this was first entry (=timer was stopped)
  {
    timer->start(freq);      // then start the timer
  }

}



void KDirWatchPrivate::removeDir( const QString& _path, Entry* sub_entry )
{
  Entry* e = entry(_path);
  if (!e) return;

  if (sub_entry)
    e->m_entries.removeRef(sub_entry);
  else
    e->m_clients--;

  if ( (e->m_clients > 0 ) || e->m_entries.count())
    return;
//qWarning("Removing... %s", path.latin1());

#ifdef HAVE_FAM
  if (use_fam) {
    if ( e->m_status == Normal) {
      FAMCancelMonitor(&fc, &(e->fr) );
      kdDebug(7001) << "Cancelled FAMReq "
		    << FAMREQUEST_GETREQNUM(&(e->fr)) << endl;
    }
    else {
      removeDir(QDir::cleanDirPath(e->path+"/.."), e);
    }
  }
#endif

#ifdef HAVE_DNOTIFY
  if (supports_dnotify)
  {
    // must close the FD.
    if (e->dn_fd)
    {
      ::close(e->dn_fd);
      fd_Entry.remove(e->dn_fd);
      e->dn_fd = 0;
    }
    if (e->path != "/")
    {
      removeDir(QDir::cleanDirPath(e->path+"/.."), e);
    }
  }
#endif

  kdDebug(7001) << "Removed " << e->path
		<< (sub_entry ? QString(" for %1").arg(sub_entry->path) : "")
		<< endl;
  m_mapDirs.remove( e->path ); // <e> not valid any more

#ifdef HAVE_FAM
  if (!use_fam)
#endif
  if( m_mapDirs.isEmpty() )
    timer->stop(); // stop timer if list empty
}

bool KDirWatchPrivate::stopDirScan( const QString& _path )
{
  Entry* e = entry(_path);
  if (!e) return false;

#ifdef HAVE_FAM
  if (use_fam && (e->m_status == Normal)) {
    FAMSuspendMonitor(&fc, &(e->fr) );
  }
#endif

  e->m_ctime = NO_NOTIFY;
  e->m_status = Normal;

  return true;
}

bool KDirWatchPrivate::restartDirScan( const QString& _path )
{
  Entry* e = entry(_path);
  if (!e) return false;

  struct stat statbuff;
  stat( QFile::encodeName(e->path), &statbuff );
  if (stat( QFile::encodeName(e->path), &statbuff ) == 0)
  {
     e->m_ctime = statbuff.st_ctime;
     e->m_status = Normal;
  }
  else
  {
     e->m_ctime = NO_NOTIFY;
     e->m_status = NonExistent;
  }

#ifdef HAVE_FAM
  if (use_fam && (e->m_status == Normal)) {
    FAMResumeMonitor(&fc, &(e->fr) );
  }
#endif

  return true;
}

void KDirWatchPrivate::stopScan()
{
#ifdef HAVE_FAM
  if (use_fam)
    emitEvents = false;
  else
#endif
  timer->stop();
}


void KDirWatchPrivate::startScan( bool notify, bool skippedToo )
{
  if (!notify)
    resetList(skippedToo);
#ifdef HAVE_FAM
  if (use_fam)
    emitEvents = true;
  else
#endif
  timer->start(freq);
}

void KDirWatchPrivate::resetList( bool skippedToo )
{
  if ( m_mapDirs.isEmpty() )
    return;

  EntryMap::Iterator it = m_mapDirs.begin();
  for( ; it != m_mapDirs.end(); ++it )
  {
    if ( (*it).m_ctime != NO_NOTIFY || skippedToo)
    {
      struct stat statbuff;
      if (stat( QFile::encodeName(it.key()), &statbuff ) == 0)
      {
         (*it).m_ctime = statbuff.st_ctime;
         (*it).m_status = Normal;
      }
      else
      {
         (*it).m_ctime = NO_NOTIFY;
         (*it).m_status = NonExistent;
      }
    }
  }
}

void KDirWatchPrivate::slotRescan()
{
  QStringList del;

  EntryMap::Iterator it;

#ifdef HAVE_DNOTIFY
  if (supports_dnotify)
  {
    it = m_mapDirs.begin();
    for( ; it != m_mapDirs.end(); ++it )
    {
      if((*it).dn_dirty)
        (*it).propagate_dirty();
    }
  }
#endif

  it = m_mapDirs.begin();
  for( ; it != m_mapDirs.end(); ++it )
  {
    const QString &path = it.key();
#ifdef HAVE_DNOTIFY
    if (supports_dnotify)
    { 
      // we know nothing has changed, no need to stat
      if(!(*it).dn_dirty)
        continue;
      (*it).dn_dirty = false;
    }
#endif

    struct stat statbuff;
    if ( stat( QFile::encodeName(path), &statbuff ) == -1 )
    {
      if ((*it).m_status == Normal)
      {
         if ((*it).m_clients)
         {
            kdDebug(7001) << "KDirWatch emitting deleted " << path << endl;
            emit deleted( path );
         }
         del.append( path );
#ifdef HAVE_DNOTIFY
         if (supports_dnotify)
         {
           // must close the FD.
           if ((*it).dn_fd)
           {
             ::close((*it).dn_fd);
             fd_Entry.remove((*it).dn_fd);
             (*it).dn_fd = 0;
           }
         }
#endif
      }
      continue; // iterator incremented
    }
    if ( ((statbuff.st_ctime != (*it).m_ctime) &&
          ((*it).m_ctime != NO_NOTIFY)) ||
          ((*it).m_status == NonExistent) )
    {
      (*it).m_ctime = statbuff.st_ctime;
      (*it).m_status = Normal;
      if ((*it).m_clients)
      {
          kdDebug(7001) << "KDirWatch emitting dirty " << path << endl;
          emit dirty( path );
      }
    }
  }

  QStringList::Iterator it2;
  it2  = del.begin();
  for( ; it2 != del.end(); ++it2 )
    removeDir( *it2, 0 );
}

#ifdef HAVE_FAM
void KDirWatchPrivate::famEventReceived()
{
  static FAMEvent fe;

  while(use_fam && FAMPending(&fc)) {
    if (FAMNextEvent(&fc, &fe) == -1) {
      kdWarning(7001) << "FAM connection problem, switching to polling."
		      << endl;
      use_fam = false;
      delete sn; sn = 0;
      startScan(false, false);
    }
    else if (emitEvents) checkFAMEvent(&fe);
  }
}

void KDirWatchPrivate::checkFAMEvent(FAMEvent* fe)
{
  int reqNum = FAMREQUEST_GETREQNUM(&(fe->fr));

  // Don't be too verbose ;-)
  if ((fe->code == FAMExists) || (fe->code == FAMEndExist)) return;

  kdDebug(7001) << "KDirWatch processing FAM event ("
		<< ((fe->code == FAMChanged) ? "FAMChanged" :
		    (fe->code == FAMDeleted) ? "FAMDeleted" :
		    (fe->code == FAMStartExecuting) ? "FAMStartExecuting" :
		    (fe->code == FAMStopExecuting) ? "FAMStopExecuting" :
		    (fe->code == FAMCreated) ? "FAMCreated" :
		    (fe->code == FAMMoved) ? "FAMMoved" :
		    (fe->code == FAMAcknowledge) ? "FAMAcknowledge" :
		    (fe->code == FAMExists) ? "FAMExists" :
		    (fe->code == FAMEndExist) ? "FAMEndExist" : "Unknown Code")
		<< ", " << fe->filename
		<< ", Req " << reqNum << ")" << endl;
  
  if (fe->code == FAMDeleted) {
    // WABA: We ignore changes to ".directory*" files because they
    // tend to be generated as a result of 'dirty' events. Which
    // leads to a never ending stream of cause & result.
    if (strncmp(fe->filename, ".directory", 10) == 0) return;

    EntryMap::Iterator it = m_mapDirs.begin();
    for( ; it != m_mapDirs.end(); ++it )
      if ( FAMREQUEST_GETREQNUM( &((*it).fr) ) == reqNum ) break;
    if (it == m_mapDirs.end()) return;

    if ((*it).m_status == NonExistent) return;
    
    // If the watched dir was deleted, fe->filename is absolute
    // otherwise, a file in a watched dir was deleted -> dir is dirty
    if (fe->filename[0] != '/') {
      kdDebug(7001) << "KDirWatch emitting dirty " << it.key() << endl;
      emit dirty( it.key() );      
      return;
    }

    (*it).m_status = NonExistent;
    FAMCancelMonitor(&fc, &((*it).fr) ); // needed ?
    kdDebug(7001) << "Cancelled FAMReq "
		  << FAMREQUEST_GETREQNUM(&((*it).fr))
		  << " for " << (*it).path << endl;
    // Scan parent for a new creation
    addDir(QDir::cleanDirPath( (*it).path+"/.."), &(*it));
    if ((*it).m_clients>0) {
      kdDebug(7001) << "KDirWatch emitting deleted "
		    << it.key() << endl;
      emit deleted ( it.key() );
    }
  }
  else if ((fe->code == FAMChanged) || (fe->code == FAMCreated)) {
    // WABA: We ignore changes to ".directory*" files because they
    // tend to be generated as a result of 'dirty' events. Which
    // leads to a never ending stream of cause & result.
    if (strncmp(fe->filename, ".directory", 10) == 0) return;

    EntryMap::Iterator it = m_mapDirs.begin();
    for( ; it != m_mapDirs.end();++it)
      if ( FAMREQUEST_GETREQNUM( &((*it).fr) ) == reqNum ) break;
    if (it == m_mapDirs.end()) return;
 
    Entry &e = (*it);
    if (e.m_status == NonExistent) return;

    QString path = it.key();
    Entry *sub_entry = e.m_entries.first();
    for(;sub_entry; sub_entry = e.m_entries.next())
      if (sub_entry->path == path + "/" + fe->filename) break;

    if (sub_entry) {
      // only possible for a FAMCreated event
      removeDir(path,sub_entry);
      FAMMonitorDirectory(&fc,
			  QFile::encodeName(sub_entry->path),
			  &(sub_entry->fr), 0);
      sub_entry->m_status = Normal;
      kdDebug(7001) << "Added " << sub_entry->path 
		    << " -> FAMReq" 
		    << FAMREQUEST_GETREQNUM(&(sub_entry->fr))
		    << endl;

      // if entry was removed, <e> is invalid -> return
      if (!m_mapDirs.contains( path )) return;
    }

    // if there aren't clients, we continue
    if (!e.m_clients) return;

    QString filename = path;
    // if a monitored file/dir has changed, fe->filename contains the
    // whole absolute path
    if (fe->filename != path) {
      if (filename.right(1) != "/") filename += "/";
      filename += fe->filename;
    }
    QFileInfo info(filename);
    if (info.isDir() || fe->code == FAMCreated) {
      kdDebug(7001) << "KDirWatch emitting dirty " << path << endl;
      emit dirty( path );
    }
    else {
      kdDebug(7001) << "KDirWatch emitting fileDirty " << filename << endl;
      emit fileDirty(filename);
    }
  }
}
#else
void KDirWatchPrivate::famEventReceived() {}
#endif




//
// Class KDirWatch
//

KDirWatch* KDirWatch::s_pSelf = 0L;

KDirWatch* KDirWatch::self()
{
  if ( !s_pSelf )
    s_pSelf = new KDirWatch;
  return s_pSelf;
}

KDirWatch::KDirWatch (int _freq)
{
  d = new KDirWatchPrivate(_freq);
  
  // connect signals from KDirWatchPrivate to KDirWatch
  connect( d, SIGNAL(dirty(const QString&)),
	   this, SIGNAL(dirty(const QString&)) );
  connect( d, SIGNAL(fileDirty(const QString&)),
	   this, SIGNAL(fileDirty(const QString&)) );
  connect( d, SIGNAL(deleted(const QString&)),
	   this, SIGNAL(deleted(const QString&)) );
}

KDirWatch::~KDirWatch()
{  
  delete d; d = 0;
}

void KDirWatch::addDir( const QString& _path )
{
  d->addDir(_path, 0);
}

time_t KDirWatch::ctime( const QString &_path )
{
  KDirWatchPrivate::Entry* e = d->entry(_path);

  if (!e)
    return 0;
  else
    return e->m_ctime;
}

void KDirWatch::removeDir( const QString& _path )
{
  if (d) d->removeDir(_path, 0);
}

bool KDirWatch::stopDirScan( const QString& _path )
{
  if (d) return d->stopDirScan(_path);
  return false;
}

bool KDirWatch::restartDirScan( const QString& _path )
{
  if (d) return d->stopDirScan(_path);
  return false;
}

void KDirWatch::stopScan()
{
  if (d) d->stopScan();
}

void KDirWatch::startScan( bool notify, bool skippedToo )
{
  if (d) d->startScan(notify,skippedToo);
}


bool KDirWatch::contains( const QString& _path ) const
{
  return d->entry(_path) ? true:false;
}

void KDirWatch::setFileDirty( const QString & _file )
{
  emit fileDirty( _file );
}


#include "kdirwatch.moc"
#include "kdirwatch_p.moc"

//sven
