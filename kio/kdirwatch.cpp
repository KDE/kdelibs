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

#include <config.h>

#ifdef HAVE_DNOTIFY
#include <fcntl.h>
#include <signal.h>
#include <qintdict.h>
#endif

#include <qtimer.h>
#include <qstringlist.h>
#include <qfile.h>
#include <qdir.h>
#include <qsocketnotifier.h>

#include <kapp.h>
#include <kdebug.h>

#include "kdirwatch.h"

#ifdef HAVE_FAM
#include <fam.h>
#endif

enum directoryStatus { Normal = 0, NonExistent };

class KDirWatchPrivate
{
public:
  KDirWatchPrivate() { }
  ~KDirWatchPrivate() { }
  struct Entry
  {
    time_t m_ctime;
    int m_clients;
    directoryStatus m_status;
#ifdef HAVE_FAM
    FAMRequest fr;
    QString watchPath;
#endif
#ifdef HAVE_DNOTIFY
    int dn_fd;
    bool dn_dirty;
#endif
  };

  typedef QMap<QString,Entry> EntryMap;

  QTimer *timer;
  EntryMap m_mapDirs;

  int freq;

  static KDirWatch* s_pSelf;

#ifdef HAVE_FAM
  QSocketNotifier *sn;
  FAMConnection fc;
  bool use_fam;
  bool emitEvents;
#endif
#ifdef HAVE_DNOTIFY
  bool supports_dnotify;
  bool sighandler_installed;
#endif
};

#define NO_NOTIFY (time_t) 0

KDirWatch* KDirWatch::s_pSelf = 0L;
#ifdef HAVE_DNOTIFY
static QIntDict<KDirWatchPrivate::Entry>* fd_Entry = 0L;
#endif

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

KDirWatch* KDirWatch::self()
{
  if ( !s_pSelf )
    s_pSelf = new KDirWatch;
  return s_pSelf;
}

KDirWatch::KDirWatch (int _freq)
{
  d = new KDirWatchPrivate;
  d->timer = new QTimer(this);
  connect (d->timer, SIGNAL(timeout()), this, SLOT(slotRescan()));

  d->freq = _freq;

#ifdef HAVE_FAM
  // It's possible that FAM server can't be started
  if (FAMOpen(&(d->fc)) ==0) {
    kdDebug(7001) << "KDirWatch: Using FAM" << endl;
    d->use_fam=true;
    d->emitEvents = true;
    d->sn = new QSocketNotifier( FAMCONNECTION_GETFD(&(d->fc)),
                                 QSocketNotifier::Read, this);
    connect( d->sn, SIGNAL(activated(int)),
 	     this, SLOT(famEventReceived()) );
  }
  else {
    kdDebug(7001) << "KDirWatch: Can't use FAM" << endl;
    d->use_fam=false;
  }
#endif
#ifdef HAVE_DNOTIFY
#ifdef HAVE_FAM
  d->supports_dnotify = !d->use_fam;
#else
  d->supports_dnotify = true; // not guilty until proven guilty
#endif
  d->sighandler_installed = false;
  if(!fd_Entry) fd_Entry = new QIntDict<KDirWatchPrivate::Entry>(31);
#endif
}

KDirWatch::~KDirWatch()
{
  d->timer->stop();

  // don't leak FD's etc.
  KDirWatchPrivate::EntryMap::Iterator it = d->m_mapDirs.begin();
  while(it != d->m_mapDirs.end()) {
    removeDir(it.key());
    it = d->m_mapDirs.begin();
  }

#ifdef HAVE_FAM
  if (d->use_fam) {
    FAMClose(&(d->fc));
    kdDebug(7001) << "KDirWatch deleted (FAM closed)" << endl;
  }
#endif

  delete d; d = 0;
}

#ifdef HAVE_FAM
static QString findWatchPath(const QString &path)
{
    struct stat statbuff;
    QString watchPath = path;
    while (stat(QFile::encodeName(watchPath), &statbuff) == -1)
      watchPath = QDir::cleanDirPath(watchPath+"/..");
    return watchPath;
}
#endif

#ifdef HAVE_DNOTIFY
static void dnotify_handler(int, siginfo_t *si, void *)
{
  KDirWatchPrivate::Entry* e = fd_Entry->find(si->si_fd);

  if(!e || e->dn_fd != si->si_fd) {
    qDebug("fatal error in KDirWatch");
  } else if(e->m_ctime != NO_NOTIFY)
    e->dn_dirty = true;
}
#endif

void KDirWatch::addDir( const QString& _path )
{
  QString path = _path;

  if ( path.length() > 1 && path.right(1) == "/" )
    path.truncate( path.length() - 1 );

  KDirWatchPrivate::EntryMap::Iterator it = d->m_mapDirs.find( path );
  if ( it != d->m_mapDirs.end() )
  {
    (*it).m_clients++;
    return;
  }

  struct stat statbuff;
  KDirWatchPrivate::Entry e;
  e.m_clients = 1;
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

#ifdef HAVE_FAM
  QString famPath = path;
  if (e.m_status == NonExistent)
  {
    // If the directory does not exist we watch the first parent directory
    // that DOES exist and wait for the directory to be created.
    famPath = findWatchPath(famPath);
    e.watchPath = famPath;
  }
  if (d->use_fam) {
    FAMMonitorDirectory(&(d->fc), QFile::encodeName(famPath), &(e.fr), 0);
    kdDebug(7001) << "KDirWatch added " <<
    	  QFile::encodeName(path) << " -> FAMReq " << FAMREQUEST_GETREQNUM(&(e.fr)) << endl;
  }
#endif

#ifdef HAVE_DNOTIFY
  e.dn_fd = 0;
#endif

  d->m_mapDirs.insert( path, e );

#ifdef HAVE_DNOTIFY
  if(e.m_status == Normal && d->supports_dnotify) {
    if(!d->sighandler_installed) {
      struct sigaction act;
      act.sa_sigaction = dnotify_handler;
      sigemptyset(&act.sa_mask);
      act.sa_flags = SA_SIGINFO;
#ifdef SA_RESTART
      act.sa_flags |= SA_RESTART;
#endif
      sigaction(SIGRTMIN, &act, NULL);
      d->sighandler_installed = true;
    }

    int fd = open(QFile::encodeName(path).data(), O_RDONLY);
    if(!fd ||
       fcntl(fd, F_SETSIG, SIGRTMIN) < 0 ||
       fcntl(fd, F_NOTIFY, DN_DELETE|DN_CREATE|DN_MULTISHOT) < 0) {
      if(fd) {
        d->supports_dnotify = false;
        ::close(fd);
      }
    }
    else {
      KDirWatchPrivate::Entry* e = &d->m_mapDirs[path];
      fd_Entry->replace(fd, e);
      e->dn_fd = fd;
      e->dn_dirty = false;
    }
  }
#endif

#ifdef HAVE_FAM
  // if FAM server can't be used, fall back to good old timer...
  if (!d->use_fam)
#endif
  if ( d->m_mapDirs.count() == 1 ) // if this was first entry (=timer was stopped)
    d->timer->start(d->freq);      // then start the timer
}

time_t KDirWatch::ctime( const QString &_path )
{
  if ( d->m_mapDirs.isEmpty() )
    return 0;

  QString path = _path;

  if ( path.right(1) == "/" )
    path.truncate( path.length() - 1 );

  KDirWatchPrivate::EntryMap::Iterator it = d->m_mapDirs.find( path );
  if ( it == d->m_mapDirs.end() )
    return 0;

  return (*it).m_ctime;
}

void KDirWatch::removeDir( const QString& _path )
{
  if ( d->m_mapDirs.isEmpty() )
    return;

  QString path = _path;

  if ( path.right(1) == "/" )
    path.truncate( path.length() - 1 );

  KDirWatchPrivate::EntryMap::Iterator it = d->m_mapDirs.find( path );
  if ( it == d->m_mapDirs.end() )
    return;

  (*it).m_clients--;
  if ( (*it).m_clients > 0 )
    return;

#ifdef HAVE_FAM
  if (d->use_fam) {
    FAMCancelMonitor(&(d->fc), &((*it).fr) );
    // kdDebug(7001) << "KDirWatch deleted: " <<
    // QFile::encodeName(path) << " (FAMReq " << FAMREQUEST_GETREQNUM(&((*it).fr)) << ")" << endl;
  }
#endif

#ifdef HAVE_DNOTIFY
  // must close the FD.
  if ((*it).dn_fd)
  {
    ::close((*it).dn_fd);
    fd_Entry->remove((*it).dn_fd);
    (*it).dn_fd = 0;
  }
#endif

  d->m_mapDirs.remove( it );

#ifdef HAVE_FAM
  if (!d->use_fam)
#endif
  if( d->m_mapDirs.isEmpty() )
    d->timer->stop(); // stop timer if list empty
}

bool KDirWatch::stopDirScan( const QString& _path )
{
  if ( d->m_mapDirs.isEmpty() )
    return false;

  QString path = _path;
  if ( path.right(1) == "/" )
    path.truncate( path.length() - 1 );

  KDirWatchPrivate::EntryMap::Iterator it = d->m_mapDirs.find( path );
  if ( it == d->m_mapDirs.end() )
    return false;

  (*it).m_ctime = NO_NOTIFY;
  (*it).m_status = Normal;

#ifdef HAVE_FAM
  if (d->use_fam) {
    FAMSuspendMonitor(&(d->fc), &((*it).fr) );
  }
#endif

  return true;
}

bool KDirWatch::restartDirScan( const QString& _path )
{
  if ( d->m_mapDirs.isEmpty() )
    return false;

  QString path = _path;
  if ( path.right(1) == "/" )
    path.truncate( path.length() - 1 );

  KDirWatchPrivate::EntryMap::Iterator it = d->m_mapDirs.find( path );
  if ( it == d->m_mapDirs.end() )
    return false;

  struct stat statbuff;
  stat( QFile::encodeName(path), &statbuff );
  if (stat( QFile::encodeName(path), &statbuff ) == 0)
  {
     (*it).m_ctime = statbuff.st_ctime;
     (*it).m_status = Normal;
  }
  else
  {
     (*it).m_ctime = NO_NOTIFY;
     (*it).m_status = NonExistent;
  }

#ifdef HAVE_FAM
  if (d->use_fam) {
    FAMResumeMonitor(&(d->fc), &((*it).fr) );
  }
#endif

  return true;
}

void KDirWatch::stopScan()
{
#ifdef HAVE_FAM
  if (d->use_fam)
    d->emitEvents = false;
  else
#endif
  d->timer->stop();
}

void KDirWatch::startScan( bool notify, bool skippedToo )
{
  if (!notify)
    resetList(skippedToo);
#ifdef HAVE_FAM
  if (d->use_fam)
    d->emitEvents = true;
  else
#endif
  d->timer->start(d->freq);
}

// Protected:

void KDirWatch::resetList( bool skippedToo )
{
  if ( d->m_mapDirs.isEmpty() )
    return;

  KDirWatchPrivate::EntryMap::Iterator it = d->m_mapDirs.begin();
  for( ; it != d->m_mapDirs.end(); ++it )
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

void KDirWatch::slotRescan()
{
  QStringList del;

  KDirWatchPrivate::EntryMap::Iterator it = d->m_mapDirs.begin();
  for( ; it != d->m_mapDirs.end(); ++it )
  {
#ifdef HAVE_DNOTIFY
    // we know nothing has changed, no need to stat
    if((*it).dn_fd && !(*it).dn_dirty)
      continue;
#endif

    struct stat statbuff;
    if ( stat( QFile::encodeName(it.key()), &statbuff ) == -1 )
    {
      if ((*it).m_status == Normal)
      {
         kdDebug(7001) << "KDirWatch emitting deleted " << it.key() << endl;
         emit deleted( it.key() );
         del.append( it.key() );
      }
#ifdef HAVE_DNOTIFY
      // must close the FD.
      if ((*it).dn_fd)
      {
        ::close((*it).dn_fd);
        fd_Entry->remove((*it).dn_fd);
        (*it).dn_fd = 0;
      }
#endif
      continue; // iterator incremented
    }
    if ( ((statbuff.st_ctime != (*it).m_ctime) &&
          ((*it).m_ctime != NO_NOTIFY)) ||
          ((*it).m_status == NonExistent) )
    {
      (*it).m_ctime = statbuff.st_ctime;
      (*it).m_status = Normal;
      kdDebug(7001) << "KDirWatch emitting dirty " << it.key() << endl;
      emit dirty( it.key() );
    }
#ifdef HAVE_DNOTIFY
    if((*it).dn_fd) (*it).dn_dirty = false;
#endif
  }

  QStringList::Iterator it2 = del.begin();
  for( ; it2 != del.end(); ++it2 )
    d->m_mapDirs.remove( *it2 );
}

bool KDirWatch::contains( const QString& _path ) const
{
  QString path = _path;
  if ( path.right(1) == "/" )
    path.truncate( path.length() - 1 );

  return d->m_mapDirs.contains( path );
}

void KDirWatch::setFileDirty( const QString & _file )
{
  emit fileDirty( _file );
}

#ifdef HAVE_FAM
void KDirWatch::famEventReceived()
{
  if (!d->use_fam || !d->emitEvents) return;

  FAMEvent fe;

  if (FAMNextEvent(&(d->fc), &fe) == -1)
  {
    kdWarning(7001) << "FAM connection problem, switching to polling." << endl;
    d->use_fam = false;
    delete d->sn; d->sn = 0;
    startScan(false, false);
    return;
  }

  int reqNum = FAMREQUEST_GETREQNUM(&(fe.fr));

  // Don't be too verbose ;-)
  if ((fe.code == FAMExists) || (fe.code == FAMEndExist)) return;

  kdDebug(7001) << "KDirWatch processing FAM event ("
                << ((fe.code == FAMChanged) ? "FAMChanged" :
                    (fe.code == FAMDeleted) ? "FAMDeleted" :
                    (fe.code == FAMStartExecuting) ? "FAMStartExecuting" :
                    (fe.code == FAMStopExecuting) ? "FAMStopExecuting" :
                    (fe.code == FAMCreated) ? "FAMCreated" :
                    (fe.code == FAMMoved) ? "FAMMoved" :
                    (fe.code == FAMAcknowledge) ? "FAMAcknowledge" :
                    (fe.code == FAMExists) ? "FAMExists" :
                    (fe.code == FAMEndExist) ? "FAMEndExist" : "Unknown Code")
                << ", " << &(fe.filename[0]) << ", Req " << reqNum << ")" << endl;

  if (fe.code == FAMDeleted) {
    // WABA: We ignore changes to ".directory*" files because they
    // tend to be generated as a result of 'dirty' events. Which
    // leads to a never ending stream of cause & result.
    if (strncmp(fe.filename, ".directory", 10) == 0) return;
    KDirWatchPrivate::EntryMap::Iterator it = d->m_mapDirs.begin();
    for( ; it != d->m_mapDirs.end(); ++it )
      if ( FAMREQUEST_GETREQNUM( &((*it).fr) ) == reqNum ) {
        if ((*it).m_status == NonExistent) return; // Ignore
	if (fe.filename[0] == '/') {
	  kdDebug(7001) << "KDirWatch emitting deleted " << it.key() << endl;
	  emit deleted ( it.key() );
          (*it).m_status = NonExistent;
//	  d->m_mapDirs.remove( it.key() );
	}
	else {
          kdDebug(7001) << "KDirWatch emitting dirty " << it.key() << endl;
	  emit dirty( it.key() );
	}
	return;
      }
  }
  else if ((fe.code == FAMChanged) || (fe.code == FAMCreated)) {
    // WABA: We ignore changes to ".directory*" files because they
    // tend to be generated as a result of 'dirty' events. Which
    // leads to a never ending stream of cause & result.
    if (strncmp(fe.filename, ".directory", 10) == 0) return;
    KDirWatchPrivate::EntryMap::Iterator it = d->m_mapDirs.begin();
    for( ; it != d->m_mapDirs.end();++it)
      if ( FAMREQUEST_GETREQNUM( &((*it).fr) ) == reqNum ) {
        QString path = it.key();
        const KDirWatchPrivate::Entry &e = (*it);

        if (e.m_status == NonExistent)
        {
           // A change occurred in the parent of a non-existing directory.
           // check if we need to update the directory to watch and
           // if the directory we are interested in happened to be created.
           QString watchPath = findWatchPath(path);
           if (watchPath != e.watchPath)
           {
              removeDir(path); // This deletes 'e' and invalidates 'it'!!
              addDir(path);
           }
           if (path != watchPath)
              return;
           // If the watch path is the same as the path,
           // we have been created!
        }
        QString filename = path + "/" + fe.filename;
        QFileInfo info(filename);
        if (info.isDir() || fe.code == FAMCreated)
        {
            kdDebug(7001) << "KDirWatch emitting dirty " << path << endl;
            emit dirty( path );
        }
        else
        {
            kdDebug(7001) << "KDirWatch emitting fileDirty " << filename << endl;
            emit fileDirty(filename);
        }
        return;
      }
  }
}
#else
void KDirWatch::famEventReceived() {}
#endif

#include "kdirwatch.moc"

//sven
