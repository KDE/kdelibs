 /*
  This file is or will be part of KDE desktop environment

  Copyright 1998 Sven Radej <sven@lisa.exp.univie.ac.at>

  It is licensed under GPL version 2.

  If it is part of KDE libraries than this file is licensed under
  LGPL version 2.
 */

#include <qtimer.h>
#include <qstrlist.h>

#include <kapp.h>
#include <kdebug.h>

#include "kdirwatch.h"

#include <iostream.h>

#define NO_NOTIFY (time_t) 0

KDirWatch* KDirWatch::s_pSelf = 0L;

 // CHANGES:
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
}

KDirWatch::~KDirWatch()
{
  timer->stop();
  delete timer;
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
  m_mapDirs.insert( path, e );

  if ( m_mapDirs.count() == 1 ) // if this was first entry (=timer was stopped)
    timer->start(freq);      // then start the timer
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

  m_mapDirs.remove( it );

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
  return true;
}

void KDirWatch::stopScan()
{
  timer->stop();
}

void KDirWatch::startScan( bool notify, bool skippedToo )
{
  if (!notify)
    resetList(skippedToo);
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
      kdebug( KDEBUG_INFO, 7001, "Deleting %s", it.key().ascii() );
      emit deleted( it.key() );
      del.append( it.key() );
      continue; // iterator incremented
    }
    if ( statbuff.st_ctime != (*it).m_ctime &&
         (*it).m_ctime != NO_NOTIFY)
    {
      (*it).m_ctime = statbuff.st_ctime;
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

/*******************************************************************/
/*
int main (int argc, char **argv)
{

  kdebug( KDEBUG_INFO, 7001,"You must create directory test in your home");
  kdebug( KDEBUG_INFO, 7001,"Directory test will be watched, but skipped");
  kdebug( KDEBUG_INFO, 7001,"When test is changed, you will be notified on console");
  kdebug( KDEBUG_INFO, 7001,"Open kfms on home and test and move/copy files between them and root");
  kdebug( KDEBUG_INFO, 7001,"Note that there will allways be output for test");

  KDirWatch *dirwatch;

  KApplication a(argc, argv);
  dirwatch = new KDirWatch();

  QString home = getenv ("HOME");
  QString desk = getenv ("HOME");
  home.prepend("file:");
  desk.prepend("file:");
  desk.append("/Desktop/");
  home.append("/");
  kdebug( KDEBUG_INFO, 7001,"Watching:");
  kdebug( KDEBUG_INFO, 7001,home.data());
  kdebug( KDEBUG_INFO, 7001,desk.data());
  dirwatch->addDirListEntry(home.data());
  home.append("test/");
  dirwatch->addDirListEntry(home.data());
  dirwatch->addDirListEntry(desk.data());
  kdebug( KDEBUG_INFO, 7001,"Watching: (but skipped)");
  kdebug( KDEBUG_INFO, 7001,home.data());

  dirwatch->startScan();
  if (!dirwatch->stopDirScan(home.data()))
    kdebug( KDEBUG_ERROR, 7001,"stopDirScan: error");

  return a.exec();
}
*/
#include "kdirwatch.moc"

//sven
