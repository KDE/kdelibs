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

#include <qtimer.h>
#include <qstrlist.h>
#include <qsocketnotifier.h>

#include <kapp.h>
#include <kdebug.h>

#include "kdirwatch.h"

#include <iostream.h>

#define NO_NOTIFY (time_t) 0

KDirWatch* KDirWatch::s_pSelf = 0L;

 // CHANGES:
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
  timer = new QTimer(this);
  connect (timer, SIGNAL(timeout()), this, SLOT(slotRescan()));

  freq = _freq;

#ifdef USE_FAM
  // It's possible that FAM server can't be started
  if (FAMOpen(&fc) ==0) {
    qDebug("KDirWatch: Using FAM");
    use_fam=1;
    emitEvents = true;
    sn = new QSocketNotifier( FAMCONNECTION_GETFD(&fc), 
			      QSocketNotifier::Read, this);
    connect( sn, SIGNAL(activated(int)),
	     this, SLOT(famEventReceived()) );
  }
  else {
    qDebug("KDirWatch: Can't use FAM");
    use_fam=0;
  }
#endif
}

KDirWatch::~KDirWatch()
{
  timer->stop();
  //  delete timer; timer was created with 'this' as parent!

#ifdef USE_FAM
  if (use_fam) {
    FAMClose(&fc);
    qDebug("KDirWatch deleted (FAM closed)");
  }
#endif
}

void KDirWatch::addDir( const QString& _path )
{
  QString path = _path;

  if ( path.right(1) == "/" )
    path.truncate( path.length() - 1 );

  QMap<QString,Entry>::Iterator it = m_mapDirs.find( path );
  if ( it != m_mapDirs.end() )
  {
    (*it).m_clients++;
    return;
  }

  stat( path.ascii(), &statbuff );
  Entry e;
  e.m_clients = 1;
  e.m_ctime = statbuff.st_ctime;

#ifdef USE_FAM
  if (use_fam) {
    FAMMonitorDirectory(&fc, path.ascii(), &(e.fr), 0);
    //    qDebug("KDirWatch added %s -> FAMReq %d", 
    //	   path.ascii(),  FAMREQUEST_GETREQNUM(&(e.fr)) );
  }
#endif

  m_mapDirs.insert( path, e );



#ifdef USE_FAM
  // if FAM server can't be used, fall back to good old timer...
  if (!use_fam)
#endif
  if ( m_mapDirs.count() == 1 ) // if this was first entry (=timer was stopped)
    timer->start(freq);      // then start the timer
}

time_t KDirWatch::ctime( const QString &_path )
{
  if ( m_mapDirs.isEmpty() )
    return 0;

  QString path = _path;

  if ( path.right(1) == "/" )
    path.truncate( path.length() - 1 );

  QMap<QString,Entry>::Iterator it = m_mapDirs.find( path );
  if ( it == m_mapDirs.end() )
    return 0;

  return (*it).m_ctime;
}

void KDirWatch::removeDir( const QString& _path )
{
  if ( m_mapDirs.isEmpty() )
    return;

  QString path = _path;

  if ( path.right(1) == "/" )
    path.truncate( path.length() - 1 );

  QMap<QString,Entry>::Iterator it = m_mapDirs.find( path );
  if ( it == m_mapDirs.end() )
    return;

  (*it).m_clients--;
  if ( (*it).m_clients > 0 )
    return;

#ifdef USE_FAM
  if (use_fam) {
    FAMCancelMonitor(&fc, &((*it).fr) );
    //    qDebug("KDirWatch deleted: %s (FAMReq %d)", 
    //	   path.ascii(),  FAMREQUEST_GETREQNUM(&((*it).fr)) );
  }
#endif

  m_mapDirs.remove( it );

#ifdef USE_FAM
  if (!use_fam)
#endif
  if( m_mapDirs.isEmpty() )
    timer->stop(); // stop timer if list empty
}

bool KDirWatch::stopDirScan( const QString& _path )
{
  if ( m_mapDirs.isEmpty() )
    return false;

  QString path = _path;
  if ( path.right(1) == "/" )
    path.truncate( path.length() - 1 );

  QMap<QString,Entry>::Iterator it = m_mapDirs.find( path );
  if ( it == m_mapDirs.end() )
    return false;

  (*it).m_ctime = NO_NOTIFY;

#ifdef USE_FAM
  if (use_fam) {
    FAMSuspendMonitor(&fc, &((*it).fr) );
  }
#endif

  return true;
}

bool KDirWatch::restartDirScan( const QString& _path )
{
  if ( m_mapDirs.isEmpty() )
    return false;

  QString path = _path;
  if ( path.right(1) == "/" )
    path.truncate( path.length() - 1 );

  QMap<QString,Entry>::Iterator it = m_mapDirs.find( path );
  if ( it == m_mapDirs.end() )
    return false;

  stat( path.ascii(), &statbuff );
  (*it).m_ctime = statbuff.st_ctime;

#ifdef USE_FAM
  if (use_fam) {
    FAMResumeMonitor(&fc, &((*it).fr) );
  }
#endif

  return true;
}

void KDirWatch::stopScan()
{
#ifdef USE_FAM
  if (use_fam)
    emitEvents = false;
  else
#endif
  timer->stop();
}

void KDirWatch::startScan( bool notify, bool skippedToo )
{
  if (!notify)
    resetList(skippedToo);
#ifdef USE_FAM
  if (use_fam)
    emitEvents = true;
  else
#endif
  timer->start(freq);
}

// Protected:

void KDirWatch::resetList( bool skippedToo )
{
  if ( m_mapDirs.isEmpty() )
    return;

  QMap<QString,Entry>::Iterator it = m_mapDirs.begin();
  for( ; it != m_mapDirs.end(); ++it )
  {
    if ( (*it).m_ctime != NO_NOTIFY || skippedToo )
    {
      stat( it.key().ascii(), &statbuff );
      (*it).m_ctime = statbuff.st_ctime;
    }
  }
}

void KDirWatch::slotRescan()
{
  QStringList del;

  QMap<QString,Entry>::Iterator it = m_mapDirs.begin();
  for( ; it != m_mapDirs.end(); ++it )
  {
    if ( stat( it.key().ascii(), &statbuff ) == -1 )
    {
      kdDebug(7001) << "Deleting " << it.key() << endl;
      emit deleted( it.key() );
      del.append( it.key() );
      continue; // iterator incremented
    }
    if ( statbuff.st_ctime != (*it).m_ctime &&
         (*it).m_ctime != NO_NOTIFY)
    {
      (*it).m_ctime = statbuff.st_ctime;
      qDebug("KDirWatch emitting dirty");
      emit dirty( it.key() );
    }
  }

  QStringList::Iterator it2 = del.begin();
  for( ; it2 != del.end(); ++it2 )
    m_mapDirs.remove( *it2 );
}

bool KDirWatch::contains( const QString& _path ) const
{
  QString path = _path;
  if ( path.right(1) == "/" )
    path.truncate( path.length() - 1 );

  return m_mapDirs.contains( path );
}

void KDirWatch::setFileDirty( const QString & _file )
{
  emit fileDirty( _file );
}

#ifdef USE_FAM
void KDirWatch::famEventReceived()
{
  if (!use_fam || !emitEvents) return;
  
  FAMNextEvent(&fc, &fe);

  int reqNum = FAMREQUEST_GETREQNUM(&(fe.fr));

  // Don't be too verbose ;-)
  if (fe.code == FAMExists || fe.code == FAMEndExist) return;

  qDebug("KDirWatch processing FAM event (%s, %s, Req %d)", 
	 (fe.code == FAMChanged) ? "FAMChanged" : 
	 (fe.code == FAMDeleted) ? "FAMDeleted" : 
	 (fe.code == FAMStartExecuting) ? "FAMStartExecuting" : 
	 (fe.code == FAMStopExecuting) ? "FAMStopExecuting" : 
	 (fe.code == FAMCreated) ? "FAMCreated" : 
	 (fe.code == FAMMoved) ? "FAMMoved" : 
	 (fe.code == FAMAcknowledge) ? "FAMAcknowledge" : 
	 (fe.code == FAMExists) ? "FAMExists" : 
	 (fe.code == FAMEndExist) ? "FAMEndExist" : "Unkown Code",
	 &(fe.filename[0]), reqNum );
	 
  if (fe.code == FAMDeleted) {
    QMap<QString,Entry>::Iterator it = m_mapDirs.begin();
    for( ; it != m_mapDirs.end(); ++it )
      if ( FAMREQUEST_GETREQNUM( &((*it).fr) ) == reqNum ) {
	if (fe.filename[0] == '/') {
	  qDebug("KDirWatch emitting deleted");
	  emit deleted ( it.key() );
	  m_mapDirs.remove( it.key() );
	}
	else {
	  qDebug("KDirWatch emitting dirty");
	  emit dirty( it.key() );
	}
	return;
      }
  }
  else if (fe.code == FAMChanged || fe.code == FAMCreated) {
    QMap<QString,Entry>::Iterator it = m_mapDirs.begin();
    for( ; it != m_mapDirs.end(); ++it )
      if ( FAMREQUEST_GETREQNUM( &((*it).fr) ) == reqNum ) {
	qDebug("KDirWatch emitting dirty");
	emit dirty( it.key() );
	return;
      }
  } 
}
#else
void KDirWatch::famEventReceived() {}
#endif

#include "kdirwatch.moc"

//sven
