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
// Februar 2002 - Add file watching and remote mount check for STAT
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
#include <kconfig.h>
#include <kglobal.h>

#include "kdirwatch.h"
#include "kdirwatch_p.h"
#include "global.h" //  KIO::probably_slow_mounted

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

  char c = 0;
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
 *   The polling frequency is determined from global kconfig
 *   settings, defaulting to 500 ms for local directories
 *   and 5000 ms for remote mounts
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

KDirWatchPrivate::KDirWatchPrivate()
{
  dwp_self = this;

  timer = new QTimer(this);
  connect (timer, SIGNAL(timeout()), this, SLOT(slotRescan()));
  freq = 3600000; // 1 hour as upper bound
  statEntries = 0;
  delayRemove = false;

  KConfigGroup config(KGlobal::config(), QCString("DirWatch"));
  m_nfsPollInterval = config.readNumEntry("NFSPollInterval", 5000);
  m_PollInterval = config.readNumEntry("PollInterval", 500);

  QString available("Stat");

#ifdef HAVE_FAM
  // It's possible that FAM server can't be started
  if (FAMOpen(&fc) ==0) {
    available += ", FAM";
    use_fam=true;
    sn = new QSocketNotifier( FAMCONNECTION_GETFD(&fc),
			      QSocketNotifier::Read, this);
    connect( sn, SIGNAL(activated(int)),
 	     this, SLOT(famEventReceived()) );
  }
  else {
    kdDebug(7001) << "Can't use FAM (fam daemon not running?)" << endl;
    use_fam=false;
  }
#endif

#ifdef HAVE_DNOTIFY
  supports_dnotify = true; // not guilty until proven guilty
  available += ", DNotify";

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
#endif

  kdDebug(7001) << "Available methods: " << available << endl;
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

// set polling frequency for a entry and adjust global freq if needed
void KDirWatchPrivate::useFreq(Entry* e, int newFreq)
{
  e->freq = newFreq;

  // a reasonable frequency for the global polling timer
  if (e->freq < freq) {
    freq = e->freq;
    if (timer->isActive()) timer->changeInterval(freq);
    kdDebug(7001) << "Global Poll Freq is now " << freq << " msec" << endl;
  }
}


#if defined(HAVE_FAM)
// setup FAM notification, returns false if not possible
bool KDirWatchPrivate::useFAM(Entry* e)
{
  if (!use_fam) return false;

  e->m_mode = FAMMode;

  if (e->isDir) {
    if (e->m_status == NonExistent) {
      // If the directory does not exist we watch the parent directory
      addEntry(0, QDir::cleanDirPath(e->path+"/.."), e, true);
    }
    else {
      int res =FAMMonitorDirectory(&fc, QFile::encodeName(e->path),
				   &(e->fr), e);
      if (res<0) {
	e->m_mode = UnknownMode;
	use_fam=false;
	return false;
      }
      kdDebug(7001) << " Setup FAM (Req "
		    << FAMREQUEST_GETREQNUM(&(e->fr))
		    << ") for " << e->path << endl;
    }
  }
  else {
    if (e->m_status == NonExistent) {
      // If the file does not exist we watch the directory
      addEntry(0, QFileInfo(e->path).dirPath(true), e, true);
    }
    else {
      int res = FAMMonitorFile(&fc, QFile::encodeName(e->path),
			       &(e->fr), e);
      if (res<0) {
	e->m_mode = UnknownMode;
	use_fam=false;
	return false;
      }

      kdDebug(7001) << " Setup FAM (Req "
		    << FAMREQUEST_GETREQNUM(&(e->fr))
		    << ") for " << e->path << endl;
    }
  }

  // handle FAM events to avoid deadlock
  // (FAM sends back all files in a directory when monitoring)
  famEventReceived();

  return true;
}
#endif


#ifdef HAVE_DNOTIFY
// setup DNotify notification, returns false if not possible
bool KDirWatchPrivate::useDNotify(Entry* e)
{
  e->dn_fd = 0;
  if (!supports_dnotify) return false;

  e->m_mode = DNotifyMode;

  if (e->isDir) {
    e->dn_dirty = false;
    if (e->m_status == Normal) {
      int fd = open(QFile::encodeName(e->path).data(), O_RDONLY);
      if (fd<0) {
	e->m_mode = UnknownMode;
	return false;
      }

      int mask = DN_DELETE|DN_CREATE|DN_RENAME|DN_MULTISHOT;
      // if dependant is a file watch, we check for MODIFY & ATTRIB too
      for(Entry* dep=e->m_entries.first();dep;dep=e->m_entries.next())
	if (!dep->isDir) { mask |= DN_MODIFY|DN_ATTRIB; break; }

      if(fcntl(fd, F_SETSIG, SIGRTMIN) < 0 ||
	 fcntl(fd, F_NOTIFY, mask) < 0) {

	kdDebug(7001) << "Not using Linux Directory Notifications."
		      << endl;
	supports_dnotify = false;
	::close(fd);
	e->m_mode = UnknownMode;
	return false;
      }

      fd_Entry.replace(fd, e);
      e->dn_fd = fd;

      kdDebug(7001) << " Setup DNotify (fd " << fd
		    << ") for " << e->path << endl;
    }
    else { // NotExisting
      addEntry(0, QDir::cleanDirPath(e->path+"/.."), e, true);
    }
  }
  else { // File
    // we always watch the directory (DNOTIFY can't watch files alone)
    // this notifies us about changes of files therein
    addEntry(0, QFileInfo(e->path).dirPath(true), e, true);
  }

  return true;
}
#endif


bool KDirWatchPrivate::useStat(Entry* e)
{
  if (KIO::probably_slow_mounted(e->path))
    useFreq(e, m_nfsPollInterval);
  else
    useFreq(e, m_PollInterval);

  if (e->m_mode != StatMode) {
    e->m_mode = StatMode;
    statEntries++;

    if ( statEntries == 1 ) {
      // if this was first STAT entry (=timer was stopped)
      timer->start(freq);      // then start the timer
      kdDebug(7001) << " Started Polling Timer, freq " << freq << endl;
    }
  }

  kdDebug(7001) << " Setup Stat (freq " << e->freq
		<< ") for " << e->path << endl;

  return true;
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
  if (path.startsWith("/dev/") || (path == "/dev"))
    return; // Don't even go there.

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

  // we have a new path to watch

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
     e->m_entries.append(sub_entry);
  else
    e->addClient(instance);

  kdDebug(7001) << "Added " << (e->isDir ? "Dir ":"File ") << path
		<< (e->m_status == NonExistent ? " NotExisting" : "")
		<< (sub_entry ? QString(" for %1").arg(sub_entry->path) : QString(""))
		<< (instance ? QString(" [%1]").arg(instance->name()) : QString(""))
		<< endl;


  // now setup the notification method
  e->m_mode = UnknownMode;
  e->msecLeft = 0;

#if defined(HAVE_FAM)
  if (useFAM(e)) return;
#endif

#ifdef HAVE_DNOTIFY
  if (useDNotify(e)) return;
#endif

  useStat(e);
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

  if (e->m_clients.count() || e->m_entries.count())
    return;

  if (delayRemove) {
    removeList.append(e);
    // now e->isValid() is false
    return;
  }

#ifdef HAVE_FAM
  if (e->m_mode == FAMMode) {
    if ( e->m_status == Normal) {
      FAMCancelMonitor(&fc, &(e->fr) );
      kdDebug(7001) << "Cancelled FAM (Req "
		    << FAMREQUEST_GETREQNUM(&(e->fr))
		    << ") for " << e->path << endl;
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
  if (e->m_mode == DNotifyMode) {
    if (!e->isDir) {
      removeEntry(0, QFileInfo(e->path).dirPath(true), e);
    }
    else { // isDir
      // must close the FD.
      if ( e->m_status == Normal) {
	if (e->dn_fd) {
	  ::close(e->dn_fd);
	  fd_Entry.remove(e->dn_fd);

	  kdDebug(7001) << "Cancelled DNotify (fd " << e->dn_fd
			<< ") for " << e->path << endl;
	  e->dn_fd = 0;

	}
      }
      else {
	removeEntry(0, QDir::cleanDirPath(e->path+"/.."), e);
      }
    }
  }
#endif

  if (e->m_mode == StatMode) {
    statEntries--;
    if ( statEntries == 0 ) {
      timer->stop(); // stop timer if lists are empty
      kdDebug(7001) << " Stopped Polling Timer" << endl;
    }
  }

  kdDebug(7001) << "Removed " << (e->isDir ? "Dir ":"File ") << e->path
		<< (sub_entry ? QString(" for %1").arg(sub_entry->path) : QString(""))
		<< (instance ? QString(" [%1]").arg(instance->name()) : QString(""))
		<< endl;
  m_mapEntries.remove( e->path ); // <e> not valid any more
}


/* Called from KDirWatch destructor:
 * remove <instance> as client from all entries
 */
void KDirWatchPrivate::removeEntries( KDirWatch* instance )
{
  QPtrList<Entry> list;
  int minfreq = 3600000;

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
    else if ( (*it).freq < minfreq) minfreq = (*it).freq;
  }

  for(Entry* e=list.first();e;e=list.next())
    removeEntry(instance, e->path, 0);

  if (minfreq > freq) {
    // we can decrease the global polling frequency
    freq = minfreq;
    if (timer->isActive()) timer->changeInterval(freq);
    kdDebug(7001) << "Poll Freq now " << freq << " msec" << endl;
  }
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
    e->msecLeft = 0;
    ev = scanEntry(e);
  }
  emitEvent(e,ev);

  return true;
}

// instance ==0: stop scanning for all instances
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
//
int KDirWatchPrivate::scanEntry(Entry* e)
{
#ifdef HAVE_FAM
  // we do not stat entries using FAM
  if (e->m_mode == FAMMode) return NoChange;
#endif

  // Shouldn't happen: Ignore "unknown" notification method
  if (e->m_mode == UnknownMode) return NoChange;

#ifdef HAVE_DNOTIFY
  if (e->m_mode == DNotifyMode) {
    // we know nothing has changed, no need to stat
    if(!e->dn_dirty) return NoChange;
    e->dn_dirty = false;
  }
#endif

  if (e->m_mode == StatMode) {
    // only scan if timeout on entry timer happens;
    // e.g. when using 500msec global timer, a entry
    // with freq=5000 is only watched every 10th time

    e->msecLeft -= freq;
    if (e->msecLeft>0) return NoChange;
    e->msecLeft += e->freq;
  }


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
void KDirWatchPrivate::emitEvent(Entry* e, int event, const QString &fileName)
{
  QString path = e->path;
  if (!fileName.isEmpty()) {
    if (fileName[0] == '/')
      path = fileName;
    else
      path += "/" + fileName;
  }

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
      c->instance->setDeleted(path);
      // emit only Deleted event...
      continue;
    }

    if (event & Created) {
      c->instance->setCreated(path);
      // possible emit Change event after creation
    }

    if (event & Changed)
      c->instance->setDirty(path);
  }
}


/* Scan all entries to be watched for changes. This is done regularly
 * when polling and once after a DNOTIFY signal. This is NOT used by FAM.
 */
void KDirWatchPrivate::slotRescan()
{
  EntryMap::Iterator it;

  // We delay deletions of entries this way.
  // removeDir(), when called in slotDirty(), can cause a crash otherwise
  delayRemove = true;

#ifdef HAVE_DNOTIFY
  QPtrList<Entry> dList, cList;

  // for DNotify method,
  // progate dirty flag to dependant entries (e.g. file watches)
  it = m_mapEntries.begin();
  for( ; it != m_mapEntries.end(); ++it )
    if ( ((*it).m_mode == DNotifyMode) && (*it).dn_dirty )
      (*it).propagate_dirty();

#endif

  it = m_mapEntries.begin();
  for( ; it != m_mapEntries.end(); ++it ) {
    // we don't check invalid entries (i.e. remove delayed)
    if (!(*it).isValid()) continue;

    int ev = scanEntry( &(*it) );

#ifdef HAVE_DNOTIFY
    if ((*it).m_mode == DNotifyMode) {
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
	  if (! useDNotify( &(*it) )) {
	    // if DNotify setup fails...
	    useStat( &(*it) );
	  }
	}
      }
    }
#endif

    if ( ev != NoChange )
      emitEvent( &(*it), ev);
  }

  Entry* e;

#ifdef HAVE_DNOTIFY
  // Scan parent of deleted directories for new creation
  for(e=dList.first();e;e=dList.next())
    addEntry(0, QDir::cleanDirPath( e->path+"/.."), e, true);

  // Remove watch of parent of new created directories
  for(e=cList.first();e;e=cList.next())
    removeEntry(0, QDir::cleanDirPath( e->path+"/.."), e);
#endif

  // Really remove entries which were marked to be removed
  delayRemove = false;
  for(e=removeList.first();e;e=removeList.next())
    removeEntry(0, e->path, 0);
  removeList.clear();
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

      // Replace all FAMMode entries with DNotify/Stat
      EntryMap::Iterator it;
      it = m_mapEntries.begin();
      for( ; it != m_mapEntries.end(); ++it )
	if ((*it).m_mode == FAMMode && (*it).m_clients.count()>0) {
#ifdef HAVE_DNOTIFY
	  if (useDNotify( &(*it) )) continue;
#endif
	  useStat( &(*it) );
	}
    }
    else
      checkFAMEvent(&fe);
  }
}

void KDirWatchPrivate::checkFAMEvent(FAMEvent* fe)
{
  // Don't be too verbose ;-)
  if ((fe->code == FAMExists) ||
      (fe->code == FAMEndExist) ||
      (fe->code == FAMAcknowledge)) return;

  // $HOME/.X.err grows with debug output, so don't notify change
  if ( *(fe->filename) == '.') {
    if (strncmp(fe->filename, ".X.err", 6) == 0) return;
    if (strncmp(fe->filename, ".xsession-errors", 16) == 0) return;
  }

  Entry* e = 0;
  EntryMap::Iterator it = m_mapEntries.begin();
  for( ; it != m_mapEntries.end(); ++it )
    if (FAMREQUEST_GETREQNUM(&( (*it).fr )) ==
       FAMREQUEST_GETREQNUM(&(fe->fr)) ) {
      e = &(*it);
      break;
    }

  // Entry* e = static_cast<Entry*>(fe->userdata);

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
		<< ", Req " << FAMREQUEST_GETREQNUM(&(fe->fr))
		<< ")" << endl;

  if (!e) {
    // this happens e.g. for FAMAcknowledge after deleting a dir...
    //    kdDebug(7001) << "No entry for FAM event ?!" << endl;
    return;
  }

  if (e->m_status == NonExistent) {
    kdDebug(7001) << "FAM event for nonExistent entry " << e->path << endl;
    return;
  }

  if (e->isDir)
    switch (fe->code)
    {
      case FAMDeleted:
       // file absolute: watched dir
        if (fe->filename[0] == '/')
        {
          // a watched directory was deleted

          e->m_status = NonExistent;
          FAMCancelMonitor(&fc, &(e->fr) ); // needed ?
          kdDebug(7001) << "Cancelled FAMReq "
                        << FAMREQUEST_GETREQNUM(&(e->fr))
                        << " for " << e->path << endl;
          // Scan parent for a new creation
          addEntry(0, QDir::cleanDirPath( e->path+"/.."), e, true);
        }
        emitEvent(e, Deleted, QFile::decodeName(fe->filename));
        break;

      case FAMCreated: {
          // check for creation of a directory we have to watch
          Entry *sub_entry = e->m_entries.first();
          for(;sub_entry; sub_entry = e->m_entries.next())
            if (sub_entry->path == e->path + "/" + fe->filename) break;
          if (sub_entry && sub_entry->isDir) {
            QString path = e->path;
            removeEntry(0,e->path,sub_entry); // <e> can be invalid here!!
            sub_entry->m_status = Normal;
            if (!useFAM(sub_entry))
              useStat(sub_entry);
        
            emitEvent(sub_entry, Created);
          }
          else emitEvent(e, Created, QFile::decodeName(fe->filename));
          break;
        }

      case FAMChanged:
        emitEvent(e, Changed, QFile::decodeName(fe->filename));

      default:
        break;
    }
  else switch (fe->code)
    {
      case FAMCreated: emitEvent(e, Created);
                       break;
      case FAMDeleted: emitEvent(e, Deleted);
                       break;
      case FAMChanged: emitEvent(e, Changed);
                       break;
    }
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
      kdDebug(7001) << "  " << e->path << " ("
		    << ((e->m_status==Normal)?"":"Nonexistent ")
		    << (e->isDir ? "Dir":"File") << ", using "
		    << ((e->m_mode == FAMMode) ? "FAM" :
			(e->m_mode == DNotifyMode) ? "DNotify" :
			(e->m_mode == StatMode) ? "Stat" : "Unknown Method")
		    << ")" << endl;

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

KDirWatch::KDirWatch (QObject* parent, const char* name)
  : QObject(parent,name)
{
  if (!name) {
    static int nameCounter = 0;

    nameCounter++;
    setName(QString("KDirWatch-%1").arg(nameCounter).ascii());
  }

  if (!dwp_self)
    dwp_self = new KDirWatchPrivate;
  d = dwp_self;

  _isStopped = false;
}

KDirWatch::~KDirWatch()
{
  if (d) d->removeEntries(this);
  // we don't remove singleton KDirWatchPrivate
}


// TODO: add watchFiles/recursive support
void KDirWatch::addDir( const QString& _path,
			bool watchFiles, bool recursive)
{
  if (watchFiles || recursive) {
    kdDebug(7001) << "addDir - recursive/watchFiles not supported in KDE 3.0"
		  << endl;
  }
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
    kdDebug(7001) << "KDirWatch not used" << endl;
    return;
  }
  dwp_self->statistics();
}


void KDirWatch::setCreated( const QString & _file )
{
  kdDebug(7001) << name() << " emitting created " << _file << endl;
  emit created( _file );
}

void KDirWatch::setDirty( const QString & _file )
{
  kdDebug(7001) << name() << " emitting dirty " << _file << endl;
  emit dirty( _file );
}

void KDirWatch::setDeleted( const QString & _file )
{
  kdDebug(7001) << name() << " emitting deleted " << _file << endl;
  emit deleted( _file );
}

#include "kdirwatch.moc"
#include "kdirwatch_p.moc"

//sven

// vim: sw=2 ts=8 et
