 /*
  This file is or will be part of KDE desktop environment

  Copyright 1998 Sven Radej <sven@lisa.exp.univie.ac.at>

  It is licensed under GPL version 2.
  
  If it is part of KDE libraries than this file is licensed under
  LGPL version 2.
 */

#ifndef _KDIRWATCH_H
#define _KDIRWATCH_H

#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <qtimer.h>
#include <qmap.h>

#define kdirwatch KDirWatch::self()

 /**
  * Class for watching directory changes. It uses stat (2) and
  * compares stored and actual changed time of directories. If
  * there is a difference notifies for change. Directories can be
  * added, removed from the list and scanning of particular directories
  * can be stoped and restarted. The whole class can be stoped and
  * restarted. Directories can be added/removed from list in
  * any state.
  * When watched directory is changed, KDirWatch will emit signal 'dirty'.
  *
  * If a watched directory gets deleted, KDirwatch will remove it from
  * the list, and  emit a signal 'deleted'.
  * @short Class for watching directory changes.
  * @author Sven Radej <sven@lisa.exp.univie.ac.at>
  */
class KDirWatch : public QObject
{
  Q_OBJECT
    
  public:
   /**
    * Constructor. Does not begin with scanning until @ref startScan
    * is called. Default frequency is 1000 ms. The created list of
    * directories has deep copies.
    */
   KDirWatch ( int freq = 1000 );

   /**
    * Destructor. Stops scanning and cleans up.
    */
   ~KDirWatch();

   /**
    * Adds directory to list of directories to be watched. (The list
    * makes deep copies).
    */
   void addDir(const QString& path);

   /**
    * Removes directory from list of scanned directories. If specified
    * path is not in the list, does nothing.
    */
   void removeDir(const QString& path);

   /**
    * Stops scanning for specified path. Does not delete dir from list,
    * just skips it. Call this function when you make an huge operation
    * on this directory (copy/move big files or lot of files). When finished,
    * call @ref #restartDirScan (path).
    * Returns 'false' if specified path is not in list, 'true' otherwise.
    */
   bool stopDirScan(const QString& path);

   /**
    * Restarts scanning for specified path. Resets ctime. It doesn't notify
    * the change, since ctime value is reset. Call it when you are finished
    * with big operations on that path, *and* when *you* have refreshed that
    * path.
    * Returns 'false' if specified path is not in list, 'true' otherwise.
    */
   bool restartDirScan(const QString& path);

   /**
    * Starts scanning of all dirs in list. If notify is true, all changed
    * dirs (since @ref #stopScan call) will be notified for refresh. If
    * notify is false, all ctimes will be reset (except those who are stopped,
    * but only if skippedToo is false) and changed dirs won't be
    * notified. You can start scanning even if the list is empty. First call
    * should be called with 'false' or else all dirs in list will be notified.
    * Note that direcories that were.
    * If 'skippedToo' is true, the skipped dirs, (scanning of which was
    * stopped with @ref #stopDirScan ) will be reset and notified for change.
    * Otherwise, stopped dirs will continue to be unnotified.
    */
   void startScan( bool notify=false, bool skippedToo=false );

   /**
    * Stops scanning of all dirs in list. List is not cleared, just the
    * timer is stopped.
    */
   void stopScan();
  
   bool contains( const QString& path ) const;
  
   /** @see signal veryDirty */
   void setVeryDirty( const QString & dir );

   static KDirWatch* self();
    
 signals:

   /**
    * This signal is emmited when directory is changed. The new ctime is set
    * before the signal is emited.
    */
   void dirty (const QString& dir);
   
   /**
    * This signal is emmited when KDirWatch learns that the directory
    * needs to be completely reloaded. This happens for instance a
    * .desktop file gets a new icon - but this isn't automatic.
    * Call setVeryDirty().
    */
   void veryDirty (const QString& dir);

   /**
    * This signal is emmited when directory is deleted. When you receive
    * this signal, directory is not yet deleted from the list. You will
    * receive this signal only once, because one directory cannot be
    * deleted more than once. Please, forget the last^H^H^H^Hprevious
    * sentence.
    */
   void deleted (const QString& dir);
     
 protected:
   void resetList (bool reallyall);
   
 protected slots:
   void slotRescan();
   
 private:
  struct Entry
  {
    time_t m_ctime;
    int m_clients;
  };
  
  QTimer *timer;
  QMap<QString,Entry> m_mapDirs;

  int freq;
  struct stat statbuff;

  static KDirWatch* s_pSelf;
};

#endif
