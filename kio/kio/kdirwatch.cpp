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
//#include <sys/stat.h>
#include <unistd.h>
//#include <time.h>
#include <fcntl.h>
#include <signal.h>
#endif

#include <assert.h>
#include <qdir.h>
#include <qfile.h>
#include <qintdict.h>
#include <qptrlist.h>
#include <qsocketnotifier.h>
#include <qstringlist.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kdebug.h>

#include "kdirwatch.h"
#include "kdirwatch_p.h"


#define NO_NOTIFY (time_t) 0

static KDirWatchPrivate* dwp_self = 0;

#ifdef HAVE_DNOTIFY

/* DNOTIFY signal handler
 *
 * As this is called asynchronously, only a flag is set and
 * a rescan is requested.
 * This is done by writing into a pipe to trigger a QSocketNotifier
 * watching on this pipe: a timer is started and after a timeout,
 * the rescan is done.
 */
void KDirWatchPrivate::dnotify_handler(int, siginfo_t *si, void *)
{
  Entry* e = (dwp_self) ? dwp_self->fd_Entry.find(si->si_fd) :0;

//  kdDebug(7001) << "DNOTIFY Handler: fd " << si->si_fd << " path "
//		<< QString(e ? e->path:"unknown") << endl;

  if(!e || e->dn_fd != si->si_fd) {
    qDebug("fatal error in KDirWatch");
  } else
    e->dn_dirty = true;

  char c;
  write(dwp_self->mPipe[1], &c, 1);
}
#endif


//
// Class KDirWatchPrivate (singleton)
//

/* All entries (files/directories) to be watched in the
 * application (coming from multiple KDirWatch instances)
 * are registered in a single KDirWatchPrivate instance.
 *
 * At the moment, the following methods for file watching
 * are supported:
 * - Polling: All files to be watched are polled regularly
 *   using stat (more precise: QFileInfo.lastModified()).
 *   The polling frequency is supplied in the *first* KDirWatch
 *   constructor, defaulting to 500 ms.
 *   This needs CPU time, especially when polling NFS dirs.
 * - FAM (File Alternation Monitor): first used on IRIX, SGI
 *   has ported this method to LINUX. It uses a kernel part
 *   (IMON, sending change events to /dev/imon) and a user
 *   level damon (fam), to which applications connect for
 *   notification of file changes. For NFS, the fam damon
 *   on the NFS server machine is used; if IMON is not built
 *   into the kernel, fam uses polling for local files.
 * - DNOTIFY: In late LINUX 2.3.x, directory notification was
 *   introduced. By opening a directory, you can request for
 *   UNIX signals to be sent to the process when a directory
 *   is changed.
 */

KDirWatchPrivate::KDirWatchPrivate(int _freq)
{
  dwp_self = this;

  timer = new QTimer(this);
  connect (timer, SIGNAL(timeout()), this, SLOT(slotRescan()));
  freq = _freq;

#ifdef HAVE_FAM
  // It's possible that FAM server can't be started
  if (FAMOpen(&fc) ==0) {
    kdDebug(7001) << "Using FAM" << endl;
    use_fam=true;
    sn = new QSocketNotifier( FAMCONNECTION_GETFD(&fc),
			      QSocketNotifier::Read, this);
    connect( sn, SIGNAL(activated(int)),
 	     this, SLOT(famEventReceived()) );
  }
  else {
    kdDebug(7001) << "Can't use FAM (fam damon not running?)" << endl;
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
     kdDebug(7001) << "Will use Linux Directory Notifications." << endl;

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


/* This should never be called, but doesn't harm */
KDirWatchPrivate::~KDirWatchPrivate()
{
  timer->stop();

  /* remove all entries being watched */
  removeEntries(0);

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

/* In DNOTIFY mode, only entries which are marked dirty are scanned.
 * We first need to mark all yet nonexistant, but possible created
 * entries as dirty...
 */
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

/* A KDirWatch instance is interested in getting events for
 * this file/Dir entry.
 */
void KDirWatchPrivate::Entry::addClient(KDirWatch* instance)
{
  Client* client = m_clients.first();
  for(;client; client = m_clients.next())
    if (client->instance == instance) break;
  
  if (client) {
    client->count++;
    return;
  }

  client = new Client;
  client->instance = instance;
  client->count = 1;
  client->watchingStopped = instance->isStopped();
  client->pending = NoChange;

  m_clients.append(client);  
}

void KDirWatchPrivate::Entry::removeClient(KDirWatch* instance)
{
  Client* client = m_clients.first();
  for(;client; client = m_clients.next())
    if (client->instance == instance) break;

  if (client) {
    client->count--;
    if (client->count == 0) {
      m_clients.removeRef(client);
      delete client;
    }
  }
}

/* get number of clients */
int KDirWatchPrivate::Entry::clients()
{
  int clients = 0;
  Client* client = m_clients.first();
  for(;client; client = m_clients.next())
    clients += client->count;

  return clients;
}


KDirWatchPrivate::Entry* KDirWatchPrivate::entry(const QString& _path)
{
// we only support absolute paths
  if (_path.left(1) != "/") {
    return 0;
  }

  QString path = _path;

  if ( path.length() > 1 && path.right(1) == "/" )
    path.truncate( path.length() - 1 );

  EntryMap::Iterator it = m_mapEntries.find( path );
  if ( it == m_mapEntries.end() )
    return 0;
  else
    return &(*it);
}

/* If <instance> !=0, this KDirWatch instance wants to watch at <_path>,
 * providing in <isDir> the type of the entry to be watched.
 * Sometimes, entries are dependant on each other: if <sub_entry> !=0,
 * this entry needs another entry to watch himself (when notExistent).
 */
void KDirWatchPrivate::addEntry(KDirWatch* instance, const QString& _path,
				Entry* sub_entry, bool isDir)
{
  QString path = _path;

  if ( path.length() > 1 && path.right(1) == "/" )
    path.truncate( path.length() - 1 );

  EntryMap::Iterator it = m_mapEntries.find( path );
  if ( it != m_mapEntries.end() )
  {
    if (sub_entry) {
       (*it).m_entries.append(sub_entry);
       kdDebug(7001) << "Added already watched Entry " << path 
		     << " (for " << sub_entry->path << ")" << endl;
    }
    else {
       (*it).addClient(instance);
       kdDebug(7001) << "Added already watched Entry " << path
		     << " (now " <<  (*it).clients() << " clients)"
		     << QString(" [%1]").arg(instance->name()) << endl;
    }
    return;
  }

  QFileInfo info(path);

  Entry newEntry;
  m_mapEntries.insert( path, newEntry );
  // the insert does a copy, so we have to use <e> now
  Entry* e = &(m_mapEntries[path]);

  if (info.exists()) {
    e->isDir = info.isDir();

    if (e->isDir && !isDir)
      qWarning("KDirWatch: %s is a directory. Use addDir!", path.ascii());
    else if (!e->isDir && isDir)
      qWarning("KDirWatch: %s is a file. Use addFile!", path.ascii());

    e->m_ctime = info.lastModified();
    e->m_status = Normal;
  }
  else {
    e->isDir = isDir;
    e->m_ctime = QDateTime(); // invalid
    e->m_status = NonExistent;
  }

  e->path = path;
  if (sub_entry)
  {
     e->m_entries.append(sub_entry);
  }
  else
  {
    e->addClient(instance);
  }

  kdDebug(7001) << "Added " << (e->isDir ? "Dir ":"File ") << path 
		<< QString((e->m_status==NonExistent)?" NotExisting":"")
		<< (sub_entry ? QString(" for %1").arg(sub_entry->path) : "")
		<< (instance ? QString(" [%1]").arg(instance->name()) : "")
		<< endl;

#if defined(HAVE_FAM)
  if (use_fam) {
    if (e->isDir) {
      if (e->m_status == NonExistent) {
	// If the directory does not exist we watch the parent directory
	addEntry(0, QDir::cleanDirPath(path+"/.."), e, true);
      }
      else {
	FAMMonitorDirectory(&fc, QFile::encodeName(path),
			    &(e->fr), 0);
	int reqnum = FAMREQUEST_GETREQNUM(&(e->fr));
	fr_Entry.replace(reqnum,e);
	kdDebug(7001) << " -> FAMReq " << reqnum << endl;
      }
    }
    else {
      if (e->m_status == NonExistent) {
	// If the file does not exist we watch the directory
	addEntry(0, info.dirPath(true), e, true);
      }
      else {
	FAMMonitorFile(&fc, QFile::encodeName(path),
		       &(e->fr), 0);
	int reqnum = FAMREQUEST_GETREQNUM(&(e->fr));
	fr_Entry.replace(reqnum,e);
	kdDebug(7001) << " -> FAMReq " << reqnum << endl;
      }
    }
  }
#endif

#ifdef HAVE_DNOTIFY
  e->dn_fd = 0;
  if(supports_dnotify) {
    if (e->isDir) {
      e->dn_dirty = false;
      if (e->m_status == Normal) {
	int fd = open(QFile::encodeName(path).data(), O_RDONLY);
	if(fd < 0 ||
	   fcntl(fd, F_SETSIG, SIGRTMIN) < 0 ||
	   fcntl(fd, F_NOTIFY,
		 DN_MODIFY|DN_DELETE|DN_CREATE|DN_RENAME|DN_ATTRIB|DN_MULTISHOT) < 0) {

	  if(fd >= 0) {
	    kdDebug(7001) << "Not using Linux Directory Notifications."
			  << endl;
	    supports_dnotify = false;
	    ::close(fd);
	  }
	}
	else {
          fd_Entry.replace(fd, e);
          e->dn_fd = fd;
	}
      }
      else { // NotExisting
	addEntry(0, QDir::cleanDirPath(path+"/.."), e, true);
      }
    }
    else { // File
      // we always watch the directory (DNOTIFY can't watch files alone)
      // this notifies us about changes of files therein
      addEntry(0, info.dirPath(true), e, true);
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
  if ( m_mapEntries.count() == 1 ) {
    // if this was first entry (=timer was stopped)
    timer->start(freq);      // then start the timer
  }
}


void KDirWatchPrivate::removeEntry( KDirWatch* instance, 
				    const QString& _path, Entry* sub_entry )
{
  Entry* e = entry(_path);
  if (!e) {
    qWarning("KDirWatch::removeDir can't handle '%s'", _path.ascii() );
    return;
  }

  if (sub_entry)
    e->m_entries.removeRef(sub_entry);
  else
    e->removeClient(instance);

  if ( e->m_clients.count() || e->m_entries.count())
    return;

#ifdef HAVE_FAM
  if (use_fam) {
    if ( e->m_status == Normal) {
      FAMCancelMonitor(&fc, &(e->fr) );
      int reqnum = FAMREQUEST_GETREQNUM(&(e->fr));
      fr_Entry.remove(reqnum);
      kdDebug(7001) << "Cancelled FAMReq " << reqnum << endl;
    }
    else {
      if (e->isDir)
	removeEntry(0, QDir::cleanDirPath(e->path+"/.."), e);
      else
	removeEntry(0, QFileInfo(e->path).dirPath(true), e);
    }
  }
#endif

#ifdef HAVE_DNOTIFY
  if (supports_dnotify) {
    if (!e->isDir) {
      removeEntry(0, QFileInfo(e->path).dirPath(true), e);
    }
    else { // isDir
      // must close the FD.
      if ( e->m_status == Normal) {
	if (e->dn_fd) {
	  ::close(e->dn_fd);
	  fd_Entry.remove(e->dn_fd);
	  e->dn_fd = 0;
	}
      }
      else {
	removeEntry(0, QDir::cleanDirPath(e->path+"/.."), e);
      }
    }
  }
#endif

  kdDebug(7001) << "Removed " << (e->isDir ? "Dir ":"File ") << e->path
		<< (sub_entry ? QString(" for %1").arg(sub_entry->path) : "")
		<< (instance ? QString(" [%1]").arg(instance->name()) : "")
		<< endl;
  m_mapEntries.remove( e->path ); // <e> not valid any more

#ifdef HAVE_FAM
  if (!use_fam)
#endif
  if ( m_mapEntries.count() == 0 )
    timer->stop(); // stop timer if lists are empty
}


/* Called from KDirWatch destructor:
 * remove <instance> as client from all entries
 */
void KDirWatchPrivate::removeEntries( KDirWatch* instance )
{
  QPtrList<Entry> list;

  // put all entries where instance is a client in list
  EntryMap::Iterator it = m_mapEntries.begin();
  for( ; it != m_mapEntries.end(); ++it ) {
    Client* c = (*it).m_clients.first();
    for(;c;c=(*it).m_clients.next())
      if (c->instance == instance) break;
    if (c) {
      c->count = 1; // forces deletion of instance as client
      list.append(&(*it));
    }
  }

  for(Entry* e=list.first();e;e=list.next())
    removeEntry(instance, e->path, 0);
}

// instance ==0: stop scanning for all instances
bool KDirWatchPrivate::stopEntryScan( KDirWatch* instance, Entry* e)
{
  int stillWatching = 0;
  Client* c = e->m_clients.first();
  for(;c;c=e->m_clients.next()) {
    if (!instance || instance == c->instance)
      c->watchingStopped = true;
    else if (!c->watchingStopped)
      stillWatching += c->count;
  }

  kdDebug(7001) << instance->name() << " stopped scanning " << e->path
		<< " (now " << stillWatching << " watchers)" << endl;

  if (stillWatching == 0) {
    // if nobody is interested, we don't watch
    e->m_ctime = QDateTime(); // invalid
    //    e->m_status = Normal;
  }
  return true;
}

// instance ==0: start scanning for all instances
bool KDirWatchPrivate::restartEntryScan( KDirWatch* instance, Entry* e,
					 bool notify)
{
  int wasWatching = 0, newWatching = 0;
  Client* c = e->m_clients.first();
  for(;c;c=e->m_clients.next()) {
    if (!c->watchingStopped)
      wasWatching += c->count;
    else if (!instance || instance == c->instance) {
      c->watchingStopped = false;
      newWatching += c->count;
    }
  }
  if (newWatching == 0)
    return false;

  kdDebug(7001) << instance->name() << " restarted scanning " << e->path
		<< " (now " << wasWatching+newWatching << " watchers)" << endl;

  // restart watching and emit pending events

  int ev = NoChange;
  if (wasWatching == 0) {
    if (!notify) {
      QFileInfo info(e->path);
      if (info.exists()) {
	e->m_ctime = info.lastModified();
	e->m_status = Normal;
      }
      else {
	e->m_ctime = QDateTime(); // invalid
	e->m_status = NonExistent;
      }
    }
    ev = scanEntry(e);
  }
  emitEvent(e,ev);

  return true;
}

// instance ==0: start scanning for all instances
void KDirWatchPrivate::stopScan(KDirWatch* instance)
{
  EntryMap::Iterator it = m_mapEntries.begin();
  for( ; it != m_mapEntries.end(); ++it )
    stopEntryScan(instance, &(*it));
}


void KDirWatchPrivate::startScan(KDirWatch* instance,
				 bool notify, bool skippedToo )
{
  if (!notify)
    resetList(instance,skippedToo);

  EntryMap::Iterator it = m_mapEntries.begin();
  for( ; it != m_mapEntries.end(); ++it )
    restartEntryScan(instance, &(*it), notify);

  // timer should still be running when in polling mode
}


// clear all pending events, also from stopped
void KDirWatchPrivate::resetList( KDirWatch* instance,
				  bool skippedToo )
{
  EntryMap::Iterator it = m_mapEntries.begin();
  for( ; it != m_mapEntries.end(); ++it ) {

    Client* c = (*it).m_clients.first();
    for(;c;c=(*it).m_clients.next())
      if (!c->watchingStopped || skippedToo)
	c->pending = NoChange;
  }
}

// Return event happened on <e>
int KDirWatchPrivate::scanEntry(Entry* e)
{
#ifdef HAVE_DNOTIFY
  if (supports_dnotify) { 
    // we know nothing has changed, no need to stat
    if(!e->dn_dirty) return NoChange;
    e->dn_dirty = false;
  }
#endif

  QFileInfo info(e->path);
  if (info.exists()) {

    if (e->m_status == NonExistent) {
      e->m_ctime = info.lastModified();
      e->m_status = Normal;
      return Created;
    }

    if ( e->m_ctime.isValid() && 
	 (info.lastModified() != e->m_ctime) ) {
      e->m_ctime = info.lastModified();
      return Changed;
    }

    return NoChange;
  }
  
  // dir/file doesn't exist

  if (!e->m_ctime.isValid())
    return NoChange;

  e->m_ctime = QDateTime(); // invalid
  e->m_status = NonExistent;

  return Deleted;
}

/* Notify all interested KDirWatch instances about a given event on an entry
 * and stored pending events. When watching is stopped, the event is
 * added to the pending events.
 */
void KDirWatchPrivate::emitEvent(Entry* e, int event)
{
  Client* c = e->m_clients.first();
  for(;c;c=e->m_clients.next()) {
    if (c->instance==0 || c->count==0) continue;

    if (c->watchingStopped) {
      // add event to pending...
      if (event == Changed) 
	c->pending |= event;
      else if (event == Created || event == Deleted)
	c->pending = event;
      continue;
    }
    // not stopped
    if (event == NoChange || event == Changed)
      event |= c->pending;
    c->pending = NoChange;
    if (event == NoChange) continue;

    if (event & Deleted) {
      if (e->isDir)
	c->instance->setDirDeleted(e->path);
      else
	c->instance->setFileDeleted(e->path);
      // emit only Deleted event...
      continue;
    }

    if (event & Created) {
      if (e->isDir)
	c->instance->setDirCreated(e->path);
      else
	c->instance->setFileCreated(e->path);
      // possible emit Change event after creation
    }

    if (event & Changed) {
      if (e->isDir)
	c->instance->setDirDirty(e->path);
      else
	c->instance->setFileDirty(e->path);      
    }
  }
}


/* Scan all entries to be watched for changes. This is done regularly
 * when polling and once after a DNOTIFY signal. This is NOT used by FAM.
 */
void KDirWatchPrivate::slotRescan()
{
  EntryMap::Iterator it;

#ifdef HAVE_DNOTIFY
  QPtrList<Entry> dList, cList;

  if (supports_dnotify)
  {
    it = m_mapEntries.begin();
    for( ; it != m_mapEntries.end(); ++it )
    {
      if((*it).dn_dirty)
        (*it).propagate_dirty();
    }
  }
#endif

  it = m_mapEntries.begin();
  for( ; it != m_mapEntries.end(); ++it ) {
    int ev = scanEntry( &(*it) );

#ifdef HAVE_DNOTIFY
    if (supports_dnotify) {
      if ((*it).isDir && (ev == Deleted)) {
	dList.append( &(*it) );
      
	// must close the FD.
	if ((*it).dn_fd) {
	  ::close((*it).dn_fd);
	  fd_Entry.remove((*it).dn_fd);
	  (*it).dn_fd = 0;
	}
      }

      else if ((*it).isDir && (ev == Created)) {
	// For created, but yet without DNOTIFYing ...
	if ( (*it).dn_fd == 0) {
	  cList.append( &(*it) );
	  int fd = open(QFile::encodeName( (*it).path).data(), O_RDONLY);
	  if(fd>=0) {
	    if (fcntl(fd, F_SETSIG, SIGRTMIN) < 0 ||
		fcntl(fd, F_NOTIFY, DN_DELETE|DN_CREATE|
		      DN_RENAME|DN_ATTRIB|DN_MULTISHOT) < 0)
	      ::close(fd);
	    else {
	      fd_Entry.replace(fd, &(*it));
	      (*it).dn_fd = fd;
	    }
	  }
	}
      }
    }
#endif

    emitEvent( &(*it), ev);    
  }

#ifdef HAVE_DNOTIFY
  // Scan parent of deleted directories for new creation
  Entry* e;
  for(e=dList.first();e;e=dList.next())
    addEntry(0, QDir::cleanDirPath( e->path+"/.."), e, true);

  // Remove watch of parent of new created directories
  for(e=cList.first();e;e=cList.next())
    removeEntry(0, QDir::cleanDirPath( e->path+"/.."), e);
#endif
      
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

      // This is simple done by starting the polling timer...
      if ( m_mapEntries.count()>0 ) 
	timer->start(freq);

    }
    else 
      checkFAMEvent(&fe);
  }
}

void KDirWatchPrivate::checkFAMEvent(FAMEvent* fe)
{
  // Don't be too verbose ;-)
  if ((fe->code == FAMExists) || (fe->code == FAMEndExist)) return;

  int reqNum = FAMREQUEST_GETREQNUM(&(fe->fr));
  Entry* e = fr_Entry.find(reqNum);

  kdDebug(7001) << "Processing FAM event ("
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
  
  if (!e) {
    // this happens e.g. for FAMAcknowledge after deleting a dir...
    //    kdDebug(7001) << "No entry for FAM event ?!" << endl;
    return;
  }

  if (e->m_status == NonExistent) {
    kdDebug(7001) << "FAM event for nonExistent entry " << e->path << endl;
    return;
  }

  // Was a watched file changed?
  if ( !e->isDir ) {
    if (fe->code == FAMDeleted) emitEvent(e, Deleted);
    else if (fe->code == FAMChanged) emitEvent(e, Changed);
    else if (fe->code == FAMCreated) emitEvent(e, Created);
    return;
  }

  if (fe->code != FAMCreated &&
      fe->code != FAMDeleted &&
      fe->code != FAMChanged) return;

  // if fe->filename is relative, a file in a watched dir was
  // changed, deleted or created
  if ( fe->filename[0] != '/') {

    // WABA: We ignore changes to ".directory*" files because they
    // tend to be generated as a result of 'dirty' events. Which
    // leads to a never ending stream of cause & result.
    if (strncmp(fe->filename, ".directory", 10) == 0) return;

    emitEvent(e, Changed);

    if (fe->code == FAMCreated) {
      // check for creation of a directory we have to watch
      Entry *sub_entry = e->m_entries.first();
      for(;sub_entry; sub_entry = e->m_entries.next())
	if (sub_entry->path == e->path + "/" + fe->filename) break;
      if (sub_entry) {
	// only possible for a FAMCreated event
	QString path = e->path;
	removeEntry(0,e->path,sub_entry); // <e> can be invalid here!!
	FAMMonitorDirectory(&fc,
			    QFile::encodeName(sub_entry->path),
			    &(sub_entry->fr), 0);
	int newReqNum = FAMREQUEST_GETREQNUM(&(sub_entry->fr));
	fr_Entry.replace(newReqNum, sub_entry);
	sub_entry->m_status = Normal;
	kdDebug(7001) << "Added " << sub_entry->path 
		      << " -> FAMReq" << newReqNum << endl;
	
	emitEvent(sub_entry, Created);
      }
    }

    return;
  }
  
  if (fe->code == FAMDeleted) {
    // a watched directory was deleted

    e->m_status = NonExistent;
    FAMCancelMonitor(&fc, &(e->fr) ); // needed ?
    fr_Entry.remove(reqNum);
    kdDebug(7001) << "Cancelled FAMReq " << reqNum
		  << " for " << e->path << endl;
    // Scan parent for a new creation
    addEntry(0, QDir::cleanDirPath( e->path+"/.."), e, true);
    emitEvent(e, Deleted);
    return;
  }

  // a watched directory was changed
  emitEvent(e, Changed);
}
#else
void KDirWatchPrivate::famEventReceived() {}
#endif


void KDirWatchPrivate::statistics()
{
  EntryMap::Iterator it;

  kdDebug(7001) << "Entries watched:" << endl;
  if (m_mapEntries.count()==0) {
    kdDebug(7001) << "  None." << endl;
  }
  else {
    it = m_mapEntries.begin();
    for( ; it != m_mapEntries.end(); ++it ) {
      Entry* e = &(*it);
      kdDebug(7001) << "  " << e->path
		    << ((e->m_status==Normal)?" (":" (Nonexistent ")
		    << (e->isDir ? "Dir)":"File)") << endl;

      Client* c = e->m_clients.first();
      for(;c; c = e->m_clients.next()) {
	QString pending;
	if (c->watchingStopped) {
	  if (c->pending & Deleted) pending += "deleted ";
	  if (c->pending & Created) pending += "created ";
	  if (c->pending & Changed) pending += "changed ";
	  if (!pending.isEmpty()) pending = " (pending: " + pending + ")";
	  pending = ", stopped" + pending;
	}
	kdDebug(7001) << "    by " << c->instance->name()
		      << " (" << c->count << " times)" 
		      << pending << endl;
      }
      if (e->m_entries.count()>0) {
	kdDebug(7001) << "    dependent entries:" << endl;
	Entry* d = e->m_entries.first();
	for(;d; d = e->m_entries.next()) {
	  kdDebug(7001) << "      " << d->path << endl;
	}
      }  
    }
  }
}


//
// Class KDirWatch
//

KDirWatch* KDirWatch::s_pSelf = 0L;

KDirWatch* KDirWatch::self()
{
  if ( !s_pSelf ) {
    s_pSelf = new KDirWatch;
  }
    
  return s_pSelf;
}

KDirWatch::KDirWatch (int _freq)
{
  static int nameCounter = 0;

  nameCounter++;
  _name = QString("KDirWatch-%1").arg(nameCounter);

  if (!dwp_self)
    dwp_self = new KDirWatchPrivate(_freq);
  d = dwp_self;

  _isStopped = false;
}

KDirWatch::~KDirWatch()
{  
  if (d) d->removeEntries(this);
  // we don't remove singleton KDirWatchPrivate 
}

void KDirWatch::addDir( const QString& _path )
{
  if (d) d->addEntry(this, _path, 0, true);
}

void KDirWatch::addFile( const QString& _path )
{
  if (d) d->addEntry(this, _path, 0, false);
}

QDateTime KDirWatch::ctime( const QString &_path )
{
  KDirWatchPrivate::Entry* e = d->entry(_path);

  if (!e)
    return QDateTime();
  else
    return e->m_ctime;
}

void KDirWatch::removeDir( const QString& _path )
{
  if (d) d->removeEntry(this, _path, 0);
}

void KDirWatch::removeFile( const QString& _path )
{
  if (d) d->removeEntry(this, _path, 0);
}

bool KDirWatch::stopDirScan( const QString& _path )
{
  if (d) {
    KDirWatchPrivate::Entry *e = d->entry(_path);
    if (e && e->isDir) return d->stopEntryScan(this, e);
  }
  return false;
}

bool KDirWatch::restartDirScan( const QString& _path )
{
  if (d) {
    KDirWatchPrivate::Entry *e = d->entry(_path);
    if (e && e->isDir)
      // restart without notifying pending events
      return d->restartEntryScan(this, e, false);
  }
  return false;
}

void KDirWatch::stopScan()
{
  if (d) d->stopScan(this);
  _isStopped = true;
}

void KDirWatch::startScan( bool notify, bool skippedToo )
{
  _isStopped = false;
  if (d) d->startScan(this, notify, skippedToo);
}


bool KDirWatch::contains( const QString& _path ) const
{
  KDirWatchPrivate::Entry* e = d->entry(_path);
  if (!e)
     return false;

  KDirWatchPrivate::Client* c = e->m_clients.first();
  for(;c;c=e->m_clients.next())
    if (c->instance == this) return true;

  return false;
}

void KDirWatch::statistics()
{
  if (!dwp_self) {
    kdDebug(7001) << "No KDirWatchPrivate object (KDirWatch not used)"
		  << endl;
    return;
  }
  dwp_self->statistics();
}


void KDirWatch::setFileCreated( const QString & _file )
{
  kdDebug(7001) << name() << " emitting fileCreated " << _file << endl;
  emit fileCreated( _file );
}

void KDirWatch::setFileDirty( const QString & _file )
{
  kdDebug(7001) << name() << " emitting fileDirty " << _file << endl;
  emit fileDirty( _file );
}

void KDirWatch::setFileDeleted( const QString & _file )
{
  kdDebug(7001) << name() << " emitting fileDeleted " << _file << endl;
  emit fileDeleted( _file );
}

void KDirWatch::setDirCreated( const QString & _path )
{
  kdDebug(7001) << name() << " emitting created dir " << _path << endl;
  emit created( _path );
}

void KDirWatch::setDirDirty( const QString & _path )
{
  kdDebug(7001) << name() << " emitting dirty dir " << _path << endl;
  emit dirty( _path );
}

void KDirWatch::setDirDeleted( const QString & _path )
{
  kdDebug(7001) << name() << " emitting deleted dir " << _path << endl;
  emit deleted( _path );
}


#include "kdirwatch.moc"
#include "kdirwatch_p.moc"

//sven
