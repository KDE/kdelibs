// -*- c-basic-offset: 2 -*-
/* This file is part of the KDE libraries
   Copyright (C) 1998 Sven Radej <sven@lisa.exp.univie.ac.at>
   Copyright (C) 2006 Dirk Mueller <mueller@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


// CHANGES:
// Oct 4,  2005 - Inotify support (Dirk Mueller)
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

#include <config-kdirwatch.h>
#include <config.h>

#include <sys/stat.h>
#include <assert.h>
#include <qdir.h>
#include <qfile.h>
#include <qsocketnotifier.h>
#include <qstringlist.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstaticdeleter.h>
#include <kde_file.h>

// debug
#include <sys/ioctl.h>

#ifdef HAVE_SYS_INOTIFY_H
#include <unistd.h>
#include <fcntl.h>
#include <sys/inotify.h>

#ifndef IN_DONT_FOLLOW
#define IN_DONT_FOLLOW 0x02000000
#endif

#ifndef IN_ONLYDIR
#define IN_ONLYDIR 0x01000000
#endif

#endif

#include <sys/utsname.h>

#include "kdirwatch.h"
#include "kdirwatch_p.h"
#include "global.h" //  KIO::probably_slow_mounted

#define NO_NOTIFY (time_t) 0

static KDirWatchPrivate* dwp_self = 0;

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
 * - INOTIFY: In LINUX 2.6.13, inode change notification was
 *   introduced. You're now able to watch arbitrary inode's
 *   for changes, and even get notification when they're
 *   unmounted.
 */

KDirWatchPrivate::KDirWatchPrivate()
  : timer(),
    freq( 3600000 ), // 1 hour as upper bound
    statEntries( 0 ),
    m_ref( 0 ),
    delayRemove( false ),
    rescan_all( false ),
    rescan_timer()
{
  timer.setObjectName( "KDirWatchPrivate::timer" );
  connect (&timer, SIGNAL(timeout()), this, SLOT(slotRescan()));

  KConfigGroup config(KGlobal::config(), QLatin1String("DirWatch"));
  m_nfsPollInterval = config.readEntry("NFSPollInterval", 5000);
  m_PollInterval = config.readEntry("PollInterval", 500);

  QString available("Stat");

  // used for FAM
  rescan_timer.setObjectName( "KDirWatchPrivate::rescan_timer" );
  rescan_timer.setSingleShot( true );
  connect(&rescan_timer, SIGNAL(timeout()), this, SLOT(slotRescan()));

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
    kDebug(7001) << "Can't use FAM (fam daemon not running?)" << endl;
    use_fam=false;
  }
#endif

#ifdef HAVE_SYS_INOTIFY_H
  supports_inotify = true;

  m_inotify_fd = inotify_init();

  if ( m_inotify_fd <= 0 ) {
    kDebug(7001) << "Can't use Inotify, kernel doesn't support it" << endl;
    supports_inotify = false;
  }

  {
    struct utsname uts;
    int major, minor, patch;
    if (uname(&uts) < 0)
      supports_inotify = false; // *shrug*
    else if (sscanf(uts.release, "%d.%d.%d", &major, &minor, &patch) != 3)
      supports_inotify = false; // *shrug*
    else if( major * 1000000 + minor * 1000 + patch < 2006014 ) { // <2.6.14
      kDebug(7001) << "Can't use INotify, Linux kernel too old" << endl;
      supports_inotify = false;
    }
  }

  if ( supports_inotify ) {
    available += ", INotify";
    fcntl(m_inotify_fd, F_SETFD, FD_CLOEXEC);

    mSn = new QSocketNotifier( m_inotify_fd, QSocketNotifier::Read, this );
    connect( mSn, SIGNAL(activated( int )), this, SLOT( slotActivated() ) );
  }
#endif

  kDebug(7001) << "Available methods: " << available << endl;
}

/* This is called on app exit (KStaticDeleter) */
KDirWatchPrivate::~KDirWatchPrivate()
{
  timer.stop();

  /* remove all entries being watched */
  removeEntries(0);

#ifdef HAVE_FAM
  if (use_fam) {
    FAMClose(&fc);
  }
#endif
#ifdef HAVE_SYS_INOTIFY_H
  if ( supports_inotify )
    ::close( m_inotify_fd );
#endif
}

#include <stdlib.h>

void KDirWatchPrivate::slotActivated()
{

#ifdef HAVE_SYS_INOTIFY_H
  if ( !supports_inotify )
    return;

  int pending = -1;
  int offset = 0;
  char buf[4096];
  assert( m_inotify_fd > -1 );
  ioctl( m_inotify_fd, FIONREAD, &pending );

  while ( pending > 0 ) {

    if ( pending > (int)sizeof( buf ) )
      pending = sizeof( buf );

    pending = read( m_inotify_fd, buf, pending);

    while ( pending > 0 ) {
      struct inotify_event *event = (struct inotify_event *) &buf[offset];
      pending -= sizeof( struct inotify_event ) + event->len;
      offset += sizeof( struct inotify_event ) + event->len;

      QString path;
      QByteArray cpath(event->name, event->len);
      if(event->len)
        path = QFile::decodeName ( cpath );

      if ( path.length() && isNoisyFile( cpath ) )
        continue;

      // now we're in deep trouble of finding the
      // associated entries
      // for now, we suck and iterate
      for ( EntryMap::Iterator it = m_mapEntries.begin();
            it != m_mapEntries.end();  ) {
        Entry* e = &( *it );
        ++it;
        if ( e->wd == event->wd ) {
          e->dirty = true;

          if( event->mask & IN_DELETE_SELF) {
            kDebug(7001) << "-->got deleteself signal for " << e->path << endl;
            e->m_status = NonExistent;
            if (e->isDir)
              addEntry(0, QDir::cleanDirPath(e->path+"/.."), e, true);
            else
              addEntry(0, QFileInfo(e->path).dirPath(true), e, true);
          }
          if ( event->mask & IN_IGNORED ) {
            e->wd = 0;
          }
          if ( event->mask & (IN_CREATE|IN_MOVED_TO) ) {
            Entry* sub_entry = 0;
            Q_FOREACH(sub_entry, e->m_entries)
              if (sub_entry->path == e->path + "/" + path) break;

            if (sub_entry /*&& sub_entry->isDir*/) {
              removeEntry(0,e->path, sub_entry);
              KDE_struct_stat stat_buf;
              QByteArray tpath = QFile::encodeName(path);
              KDE_stat(tpath, &stat_buf);

              //sub_entry->isDir = S_ISDIR(stat_buf.st_mode);
              //sub_entry->m_ctime = stat_buf.st_ctime;
              //sub_entry->m_status = Normal;
              //sub_entry->m_nlink = stat_buf.st_nlink;

              if(!useINotify(sub_entry))
                useStat(sub_entry);
              sub_entry->dirty = true;
            }
          }

          if (!rescan_timer.isActive())
            rescan_timer.start(m_PollInterval); // singleshot

          break;
        }
      }
    }
  }
#endif
}

/* In FAM mode, only entries which are marked dirty are scanned.
 * We first need to mark all yet nonexistent, but possible created
 * entries as dirty...
 */
void KDirWatchPrivate::Entry::propagate_dirty()
{
  foreach(Entry *sub_entry, m_entries)
  {
     if (!sub_entry->dirty)
     {
        sub_entry->dirty = true;
        sub_entry->propagate_dirty();
     }
  }
}


/* A KDirWatch instance is interested in getting events for
 * this file/Dir entry.
 */
void KDirWatchPrivate::Entry::addClient(KDirWatch* instance)
{
  foreach(Client* client, m_clients) {
    if (client->instance == instance) {
      client->count++;
      return;
    }
  }

  Client* client = new Client;
  client->instance = instance;
  client->count = 1;
  client->watchingStopped = instance->isStopped();
  client->pending = NoChange;

  m_clients.append(client);
}

void KDirWatchPrivate::Entry::removeClient(KDirWatch* instance)
{
  QList<Client *>::iterator it = m_clients.begin();
  const QList<Client *>::iterator end = m_clients.end();
  for ( ; it != end ; ++it ) {
    Client* client = *it;
    if (client->instance == instance) {
      client->count--;
      if (client->count == 0) {
        m_clients.erase(it);
        delete client;
      }
      return;
    }
  }
}

/* get number of clients */
int KDirWatchPrivate::Entry::clients()
{
  int clients = 0;
  foreach(Client* client, m_clients)
    clients += client->count;

  return clients;
}


KDirWatchPrivate::Entry* KDirWatchPrivate::entry(const QString& _path)
{
// we only support absolute paths
  if (QDir::isRelativePath(_path)) {
    return 0;
  }

  QString path = _path;

  if ( path.length() > 1 && path.endsWith( QLatin1Char( '/' ) ) )
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
    if (timer.isActive()) timer.start(freq);
    kDebug(7001) << "Global Poll Freq is now " << freq << " msec" << endl;
  }
}


#if defined(HAVE_FAM)
// setup FAM notification, returns false if not possible
bool KDirWatchPrivate::useFAM(Entry* e)
{
  if (!use_fam) return false;

  // handle FAM events to avoid deadlock
  // (FAM sends back all files in a directory when monitoring)
  famEventReceived();

  e->m_mode = FAMMode;
  e->dirty = false;

  if (e->isDir) {
    if (e->m_status == NonExistent) {
      // If the directory does not exist we watch the parent directory
      addEntry(0, QDir::cleanPath(e->path+"/.."), e, true);
    }
    else {
      int res =FAMMonitorDirectory(&fc, QFile::encodeName(e->path),
				   &(e->fr), e);
      if (res<0) {
	e->m_mode = UnknownMode;
	use_fam=false;
	return false;
      }
      kDebug(7001) << " Setup FAM (Req "
		    << FAMREQUEST_GETREQNUM(&(e->fr))
		    << ") for " << e->path << endl;
    }
  }
  else {
    if (e->m_status == NonExistent) {
      // If the file does not exist we watch the directory
      addEntry(0, QFileInfo(e->path).absolutePath(), e, true);
    }
    else {
      int res = FAMMonitorFile(&fc, QFile::encodeName(e->path),
			       &(e->fr), e);
      if (res<0) {
	e->m_mode = UnknownMode;
	use_fam=false;
	return false;
      }

      kDebug(7001) << " Setup FAM (Req "
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

#ifdef HAVE_SYS_INOTIFY_H
// setup INotify notification, returns false if not possible
bool KDirWatchPrivate::useINotify( Entry* e )
{
  e->wd = 0;
  e->dirty = false;

  if (!supports_inotify) return false;

  e->m_mode = INotifyMode;

  if ( e->m_status == NonExistent ) {
    addEntry(0, QDir::cleanPath(e->path+"/.."), e, true);
    return true;
  }

  int mask = IN_DELETE|IN_DELETE_SELF|IN_CREATE|IN_MOVE|IN_MOVE_SELF|IN_DONT_FOLLOW;
  if(!e->isDir)
    mask |= IN_MODIFY|IN_ATTRIB;
  else
    mask |= IN_ONLYDIR;

  // if dependant is a file watch, we check for MODIFY & ATTRIB too
  foreach(Entry *dep, e->m_entries) {
    if (!dep->isDir) { mask |= IN_MODIFY|IN_ATTRIB; break; }
  }

  if ( ( e->wd = inotify_add_watch( m_inotify_fd,
                                    QFile::encodeName( e->path ), mask) ) > 0)
    return true;

  return false;
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
      timer.start(freq);      // then start the timer
      kDebug(7001) << " Started Polling Timer, freq " << freq << endl;
    }
  }

  kDebug(7001) << " Setup Stat (freq " << e->freq
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

  if ( path.length() > 1 && path.endsWith( QLatin1Char( '/' ) ) )
    path.truncate( path.length() - 1 );

  EntryMap::Iterator it = m_mapEntries.find( path );
  if ( it != m_mapEntries.end() )
  {
    if (sub_entry) {
       (*it).m_entries.append(sub_entry);
       kDebug(7001) << "Added already watched Entry " << path
		     << " (for " << sub_entry->path << ")" << endl;
#ifdef HAVE_SYS_INOTIFY
       Entry* e = &(*it);
       if( (e->m_mode == INotifyMode) && (e->wd > 0) ) {
         int mask = IN_DELETE|IN_DELETE_SELF|IN_CREATE|IN_MOVE|IN_MOVE_SELF|IN_DONT_FOLLOW;
         if(!e->isDir)
           mask |= IN_MODIFY|IN_ATTRIB;
         else
           mask |= IN_ONLYDIR;

         inotify_rm_watch (m_inotify_fd, e->wd);
         e->wd = inotify_add_watch( m_inotify_fd, QFile::encodeName( e->path ), mask);
       }
#endif
    }
    else {
       (*it).addClient(instance);
       kDebug(7001) << "Added already watched Entry " << path
		     << " (now " <<  (*it).clients() << " clients)"
		     << QString(" [%1]").arg(instance->objectName()) << endl;
    }
    return;
  }

  // we have a new path to watch

  KDE_struct_stat stat_buf;
  QByteArray tpath = QFile::encodeName(path);
  bool exists = (KDE_stat(tpath, &stat_buf) == 0);

  Entry newEntry;
  m_mapEntries.insert( path, newEntry );
  // the insert does a copy, so we have to use <e> now
  Entry* e = &(m_mapEntries[path]);

  if (exists) {
    e->isDir = S_ISDIR(stat_buf.st_mode);

    if (e->isDir && !isDir)
      qWarning("KDirWatch: %s is a directory. Use addDir!", qPrintable(path));
    else if (!e->isDir && isDir)
      qWarning("KDirWatch: %s is a file. Use addFile!", qPrintable(path));

    e->m_ctime = stat_buf.st_ctime;
    e->m_status = Normal;
    e->m_nlink = stat_buf.st_nlink;
  }
  else {
    e->isDir = isDir;
    e->m_ctime = invalid_ctime;
    e->m_status = NonExistent;
    e->m_nlink = 0;
  }

  e->path = path;
  if (sub_entry)
     e->m_entries.append(sub_entry);
  else
    e->addClient(instance);

  kDebug(7001) << "Added " << (e->isDir ? "Dir ":"File ") << path
		<< (e->m_status == NonExistent ? " NotExisting" : "")
		<< (sub_entry ? QString(" for %1").arg(sub_entry->path) : QString(""))
		<< (instance ? QString(" [%1]").arg(instance->objectName()) : QString(""))
		<< endl;


  // now setup the notification method
  e->m_mode = UnknownMode;
  e->msecLeft = 0;

  if ( isNoisyFile( tpath ) )
    return;

#if defined(HAVE_FAM)
  if (useFAM(e)) return;
#endif

#if defined(HAVE_SYS_INOTIFY_H)
  if (useINotify(e)) return;
#endif

  useStat(e);
}


void KDirWatchPrivate::removeEntry( KDirWatch* instance,
				    const QString& _path, Entry* sub_entry )
{
  kDebug(7001) << "KDirWatchPrivate::removeEntry for '" << _path << "' sub_entry: " << sub_entry << endl;
  Entry* e = entry(_path);
  if (!e) {
    kWarning(7001) << "KDirWatch::removeDir can't handle '" << _path << "'" << endl;
    return;
  }

  if (sub_entry)
    e->m_entries.removeAll(sub_entry);
  else
    e->removeClient(instance);

  if (e->m_clients.count() || e->m_entries.count())
    return;

  if (delayRemove) {
    // removeList is allowed to contain any entry at most once
    if (!removeList.contains(e))
      removeList.append(e);
    // now e->isValid() is false
    return;
  }

#ifdef HAVE_FAM
  if (e->m_mode == FAMMode) {
    if ( e->m_status == Normal) {
      FAMCancelMonitor(&fc, &(e->fr) );
      kDebug(7001) << "Cancelled FAM (Req "
		    << FAMREQUEST_GETREQNUM(&(e->fr))
		    << ") for " << e->path << endl;
    }
    else {
      if (e->isDir)
	removeEntry(0, QDir::cleanPath(e->path+"/.."), e);
      else
	removeEntry(0, QFileInfo(e->path).absolutePath(), e);
    }
  }
#endif

#ifdef HAVE_SYS_INOTIFY_H
  if (e->m_mode == INotifyMode) {
    if ( e->m_status == Normal ) {
      (void) inotify_rm_watch( m_inotify_fd, e->wd );
      kDebug(7001) << "Cancelled INotify (fd " <<
        m_inotify_fd << ", "  << e->wd <<
        ") for " << e->path << endl;
    }
    else {
      if (e->isDir)
        removeEntry(0, QDir::cleanDirPath(e->path+"/.."), e);
      else
        removeEntry(0, QFileInfo(e->path).dirPath(true), e);
    }
  }
#endif

  if (e->m_mode == StatMode) {
    statEntries--;
    if ( statEntries == 0 ) {
      timer.stop(); // stop timer if lists are empty
      kDebug(7001) << " Stopped Polling Timer" << endl;
    }
  }

  kDebug(7001) << "Removed " << (e->isDir ? "Dir ":"File ") << e->path
		<< (sub_entry ? QString(" for %1").arg(sub_entry->path) : QString(""))
		<< (instance ? QString(" [%1]").arg(instance->objectName()) : QString(""))
		<< endl;
  m_mapEntries.remove( e->path ); // <e> not valid any more
}


/* Called from KDirWatch destructor:
 * remove <instance> as client from all entries
 */
void KDirWatchPrivate::removeEntries( KDirWatch* instance )
{
  int minfreq = 3600000;

  QStringList pathList;
  // put all entries where instance is a client in list
  EntryMap::Iterator it = m_mapEntries.begin();
  for( ; it != m_mapEntries.end(); ++it ) {
    Client* c = 0;
    foreach(Client* client, (*it).m_clients) {
      if (client->instance == instance) {
        c = client;
        break;
      }
    }
    if (c) {
      c->count = 1; // forces deletion of instance as client
      pathList.append((*it).path);
    }
    else if ( (*it).m_mode == StatMode && (*it).freq < minfreq )
      minfreq = (*it).freq;
  }

  foreach(QString path, pathList)
    removeEntry(instance, path, 0);

  if (minfreq > freq) {
    // we can decrease the global polling frequency
    freq = minfreq;
    if (timer.isActive()) timer.start(freq);
    kDebug(7001) << "Poll Freq now " << freq << " msec" << endl;
  }
}

// instance ==0: stop scanning for all instances
bool KDirWatchPrivate::stopEntryScan( KDirWatch* instance, Entry* e)
{
  int stillWatching = 0;
  foreach(Client* client, e->m_clients) {
    if (!instance || instance == client->instance)
      client->watchingStopped = true;
    else if (!client->watchingStopped)
      stillWatching += client->count;
  }

  kDebug(7001) << (instance ? instance->objectName() : "all") << " stopped scanning " << e->path
		<< " (now " << stillWatching << " watchers)" << endl;

  if (stillWatching == 0) {
    // if nobody is interested, we don't watch
    if ( e->m_mode != INotifyMode )
    {
      e->m_ctime = invalid_ctime; // invalid
      e->m_status = NonExistent;
    }
    //    e->m_status = Normal;
  }
  return true;
}

// instance ==0: start scanning for all instances
bool KDirWatchPrivate::restartEntryScan( KDirWatch* instance, Entry* e,
					 bool notify)
{
  int wasWatching = 0, newWatching = 0;
  foreach(Client* client, e->m_clients) {
    if (!client->watchingStopped)
      wasWatching += client->count;
    else if (!instance || instance == client->instance) {
      client->watchingStopped = false;
      newWatching += client->count;
    }
  }
  if (newWatching == 0)
    return false;

  kDebug(7001) << (instance ? instance->objectName() : "all") << " restarted scanning " << e->path
		<< " (now " << wasWatching+newWatching << " watchers)" << endl;

  // restart watching and emit pending events

  int ev = NoChange;
  if (wasWatching == 0) {
    if (!notify) {
      KDE_struct_stat stat_buf;
      bool exists = (KDE_stat(QFile::encodeName(e->path), &stat_buf) == 0);
      if (exists) {
	e->m_ctime = stat_buf.st_ctime;
	e->m_status = Normal;
        e->m_nlink = stat_buf.st_nlink;
      }
      else {
	e->m_ctime = invalid_ctime;
	e->m_status = NonExistent;
        e->m_nlink = 0;
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
void KDirWatchPrivate::resetList( KDirWatch* /*instance*/,
				  bool skippedToo )
{
  EntryMap::Iterator it = m_mapEntries.begin();
  for( ; it != m_mapEntries.end(); ++it ) {

    foreach(Client* client, (*it).m_clients) {
      if (!client->watchingStopped || skippedToo)
	client->pending = NoChange;
    }
  }
}

// Return event happened on <e>
//
int KDirWatchPrivate::scanEntry(Entry* e)
{
#ifdef HAVE_FAM
  if (e->m_mode == FAMMode) {
    // we know nothing has changed, no need to stat
    if(!e->dirty) return NoChange;
    e->dirty = false;
  }
#endif

  // Shouldn't happen: Ignore "unknown" notification method
  if (e->m_mode == UnknownMode) return NoChange;

#if defined( HAVE_SYS_INOTIFY_H )
  if (e->m_mode == DNotifyMode || e->m_mode == INotifyMode ) {
    // we know nothing has changed, no need to stat
    if(!e->dirty) return NoChange;
    e->dirty = false;
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

  KDE_struct_stat stat_buf;
  bool exists = (KDE_stat(QFile::encodeName(e->path), &stat_buf) == 0);
  if (exists) {

    if (e->m_status == NonExistent) {
      e->m_ctime = stat_buf.st_ctime;
      e->m_status = Normal;
      e->m_nlink = stat_buf.st_nlink;
      return Created;
    }

    if ( (e->m_ctime != invalid_ctime) &&
	 ((stat_buf.st_ctime != e->m_ctime) ||
	  (stat_buf.st_nlink != (nlink_t) e->m_nlink)) ) {
      e->m_ctime = stat_buf.st_ctime;
      e->m_nlink = stat_buf.st_nlink;
      return Changed;
    }

    return NoChange;
  }

  // dir/file doesn't exist

  if (e->m_ctime == invalid_ctime) {
    e->m_nlink = 0;
    e->m_status = NonExistent;
    return NoChange;
  }

  e->m_ctime = invalid_ctime;
  e->m_nlink = 0;
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
    if (!QDir::isRelativePath(fileName))
      path = fileName;
    else
#ifdef Q_OS_UNIX
      path += '/' + fileName;
#elif defined(Q_WS_WIN)
      //current drive is passed instead of /
      path += QDir::currentPath().left(2) + '/' + fileName;
#endif
  }

  foreach(Client* c, e->m_clients)
  {
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

// Remove entries which were marked to be removed
void KDirWatchPrivate::slotRemoveDelayed()
{
  delayRemove = false;
  foreach(Entry* e, removeList)
    removeEntry(0, e->path, 0);
  removeList.clear();
}

/* Scan all entries to be watched for changes. This is done regularly
 * when polling. This is NOT used by FAM.
 */
void KDirWatchPrivate::slotRescan()
{
  EntryMap::Iterator it;

  // People can do very long things in the slot connected to dirty(),
  // like showing a message box. We don't want to keep polling during
  // that time, otherwise the value of 'delayRemove' will be reset.
  bool timerRunning = timer.isActive();
  if ( timerRunning )
    timer.stop();

  // We delay deletions of entries this way.
  // removeDir(), when called in slotDirty(), can cause a crash otherwise
  delayRemove = true;

  if (rescan_all)
  {
    // mark all as dirty
    it = m_mapEntries.begin();
    for( ; it != m_mapEntries.end(); ++it )
      (*it).dirty = true;
    rescan_all = false;
  }
  else
  {
    // progate dirty flag to dependant entries (e.g. file watches)
    it = m_mapEntries.begin();
    for( ; it != m_mapEntries.end(); ++it )
      if (((*it).m_mode == INotifyMode || (*it).m_mode == DNotifyMode) && (*it).dirty )
        (*it).propagate_dirty();
  }

#ifdef HAVE_SYS_INOTIFY_H
  QList<Entry*> dList, cList;
#endif

  it = m_mapEntries.begin();
  for( ; it != m_mapEntries.end(); ++it ) {
    // we don't check invalid entries (i.e. remove delayed)
    if (!(*it).isValid()) continue;

    int ev = scanEntry( &(*it) );

#ifdef HAVE_SYS_INOTIFY_H
    if ((*it).m_mode == INotifyMode) {
      if ( ev == Deleted ) {
        addEntry(0, QDir::cleanPath( ( *it ).path+"/.."), &*it, true);
      }
    }
    if ((*it).m_mode == INotifyMode && ev == Created && (*it).wd == 0) {
      cList.append( &(*it) );
      if (! useINotify( &(*it) )) {
        useStat( &(*it) );
      }
    }
#endif

    if ( ev != NoChange )
      emitEvent( &(*it), ev);
  }


  if ( timerRunning )
    timer.start(freq);

#ifdef HAVE_SYS_INOTIFY_H
  // Remove watch of parent of new created directories
  Q_FOREACH(Entry* e, cList)
    removeEntry(0, QDir::cleanDirPath( e->path+"/.."), e);
#endif

  QTimer::singleShot(0, this, SLOT(slotRemoveDelayed()));
}

bool KDirWatchPrivate::isNoisyFile( const char * filename )
{
  // $HOME/.X.err grows with debug output, so don't notify change
  if ( *filename == '.') {
    if (strncmp(filename, ".X.err", 6) == 0) return true;
    if (strncmp(filename, ".xsession-errors", 16) == 0) return true;
    // fontconfig updates the cache on every KDE app start
    // (inclusive kio_thumbnail slaves)
    if (strncmp(filename, ".fonts.cache", 12) == 0) return true;
  }

  return false;
}

#ifdef HAVE_FAM
void KDirWatchPrivate::famEventReceived()
{
  static FAMEvent fe;

  delayRemove = true;

  while(use_fam && FAMPending(&fc)) {
    if (FAMNextEvent(&fc, &fe) == -1) {
      kWarning(7001) << "FAM connection problem, switching to polling."
		      << endl;
      use_fam = false;
      delete sn; sn = 0;

      // Replace all FAMMode entries with DNotify/Stat
      EntryMap::Iterator it;
      it = m_mapEntries.begin();
      for( ; it != m_mapEntries.end(); ++it )
	if ((*it).m_mode == FAMMode && (*it).m_clients.count()>0) {
#ifdef HAVE_SYS_INOTIFY_H
	  if (useINotify( &(*it) )) continue;
#endif
	  useStat( &(*it) );
	}
    }
    else
      checkFAMEvent(&fe);
  }

  QTimer::singleShot(0, this, SLOT(slotRemoveDelayed()));
}

void KDirWatchPrivate::checkFAMEvent(FAMEvent* fe)
{
  // Don't be too verbose ;-)
  if ((fe->code == FAMExists) ||
      (fe->code == FAMEndExist) ||
      (fe->code == FAMAcknowledge)) return;

  if ( isNoisyFile( fe->filename ) )
    return;

  Entry* e = 0;
  EntryMap::Iterator it = m_mapEntries.begin();
  for( ; it != m_mapEntries.end(); ++it )
    if (FAMREQUEST_GETREQNUM(&( (*it).fr )) ==
       FAMREQUEST_GETREQNUM(&(fe->fr)) ) {
      e = &(*it);
      break;
    }

  // Entry* e = static_cast<Entry*>(fe->userdata);

#if 0 // #88538
  kDebug(7001) << "Processing FAM event ("
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
#endif

  if (!e) {
    // this happens e.g. for FAMAcknowledge after deleting a dir...
    //    kDebug(7001) << "No entry for FAM event ?!" << endl;
    return;
  }

  if (e->m_status == NonExistent) {
    kDebug(7001) << "FAM event for nonExistent entry " << e->path << endl;
    return;
  }

  // Delayed handling. This rechecks changes with own stat calls.
  e->dirty = true;
  if (!rescan_timer.isActive())
    rescan_timer.start(m_PollInterval); // singleshot

  // needed FAM control actions on FAM events
  if (e->isDir)
    switch (fe->code)
    {
      case FAMDeleted:
       // file absolute: watched dir
        if (!QDir::isRelativePath(fe->filename))
        {
          // a watched directory was deleted

          e->m_status = NonExistent;
          FAMCancelMonitor(&fc, &(e->fr) ); // needed ?
          kDebug(7001) << "Cancelled FAMReq "
                        << FAMREQUEST_GETREQNUM(&(e->fr))
                        << " for " << e->path << endl;
          // Scan parent for a new creation
          addEntry(0, QDir::cleanPath( e->path+"/.."), e, true);
        }
        break;

      case FAMCreated: {
          // check for creation of a directory we have to watch
          foreach(Entry *sub_entry, e->m_entries) {
            if (sub_entry->path == e->path + '/' + (const char *)fe->filename
                && sub_entry->isDir) {
              QString path = e->path;
              removeEntry(0,e->path,sub_entry); // <e> can be invalid here!!
              sub_entry->m_status = Normal;
              if (!useFAM(sub_entry))
#ifdef HAVE_SYS_INOTIFY_H
                if (!useINotify(sub_entry ))
#endif
                  useStat(sub_entry);
              break;
            }
          }
          break;

        }

      default:
        break;
    }
}
#else
void KDirWatchPrivate::famEventReceived() {}
#endif


void KDirWatchPrivate::statistics()
{
  EntryMap::Iterator it;

  kDebug(7001) << "Entries watched:" << endl;
  if (m_mapEntries.count()==0) {
    kDebug(7001) << "  None." << endl;
  }
  else {
    it = m_mapEntries.begin();
    for( ; it != m_mapEntries.end(); ++it ) {
      Entry* e = &(*it);
      kDebug(7001) << "  " << e->path << " ("
		    << ((e->m_status==Normal)?"":"Nonexistent ")
		    << (e->isDir ? "Dir":"File") << ", using "
		    << ((e->m_mode == FAMMode) ? "FAM" :
                        (e->m_mode == INotifyMode) ? "INotify" :
			(e->m_mode == DNotifyMode) ? "DNotify" :
			(e->m_mode == StatMode) ? "Stat" : "Unknown Method")
		    << ")" << endl;

      foreach(Client* c, e->m_clients) {
	QString pending;
	if (c->watchingStopped) {
	  if (c->pending & Deleted) pending += "deleted ";
	  if (c->pending & Created) pending += "created ";
	  if (c->pending & Changed) pending += "changed ";
	  if (!pending.isEmpty()) pending = " (pending: " + pending + ')';
	  pending = ", stopped" + pending;
	}
	kDebug(7001) << "    by " << c->instance->objectName()
		      << " (" << c->count << " times)"
		      << pending << endl;
      }
      if (e->m_entries.count()>0) {
	kDebug(7001) << "    dependent entries:" << endl;
        foreach(Entry *d, e->m_entries) {
	  kDebug(7001) << "      " << d->path << endl;
	}
      }
    }
  }
}


//
// Class KDirWatch
//

static KStaticDeleter<KDirWatch> sd_dw;
KDirWatch* KDirWatch::s_pSelf = 0L;

KDirWatch* KDirWatch::self()
{
  if ( !s_pSelf ) {
    sd_dw.setObject( s_pSelf, new KDirWatch );
  }

  return s_pSelf;
}

bool KDirWatch::exists()
{
  return s_pSelf != 0;
}

KDirWatch::KDirWatch (QObject* parent)
  : QObject(parent)
{
  static int nameCounter = 0;

  nameCounter++;
  setObjectName(QString("KDirWatch-%1").arg(nameCounter) );

  if (!dwp_self)
    dwp_self = new KDirWatchPrivate;
  d = dwp_self;
  d->ref();

  _isStopped = false;
}

KDirWatch::~KDirWatch()
{
  d->removeEntries(this);
  if ( d->deref() )
  {
    // delete it if it's the last one
    delete d;
    dwp_self = 0L;
  }
}


// TODO: add watchFiles/recursive support
void KDirWatch::addDir( const QString& _path,
			bool watchFiles, bool recursive)
{
  if (watchFiles || recursive) {
    kDebug(7001) << "addDir - recursive/watchFiles not supported yet in KDE 3.x" << endl;
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

  QDateTime result;
  result.setTime_t(e->m_ctime);
  return result;
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

  foreach(KDirWatchPrivate::Client* client, e->m_clients) {
    if (client->instance == this)
      return true;
  }

  return false;
}

void KDirWatch::statistics()
{
  if (!dwp_self) {
    kDebug(7001) << "KDirWatch not used" << endl;
    return;
  }
  dwp_self->statistics();
}


void KDirWatch::setCreated( const QString & _file )
{
  kDebug(7001) << objectName() << " emitting created " << _file << endl;
  emit created( _file );
}

void KDirWatch::setDirty( const QString & _file )
{
  kDebug(7001) << objectName() << " emitting dirty " << _file << endl;
  emit dirty( _file );
}

void KDirWatch::setDeleted( const QString & _file )
{
  kDebug(7001) << objectName() << " emitting deleted " << _file << endl;
  emit deleted( _file );
}

KDirWatch::Method KDirWatch::internalMethod()
{
#ifdef HAVE_FAM
  if (d->use_fam)
    return KDirWatch::FAM;
#endif
#ifdef HAVE_SYS_INOTIFY_H
  if (d->supports_inotify)
    return KDirWatch::INotify;
#endif
  return KDirWatch::Stat;
}


#include "kdirwatch.moc"
#include "kdirwatch_p.moc"

//sven

// vim: sw=2 ts=8 et
