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
#ifndef _KDIRWATCH_H
#define _KDIRWATCH_H

#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <qtimer.h>
#include <qmap.h>

#define kdirwatch KDirWatch::self()

class KDirWatchPrivate;

 /**
  * Watch directories for changes.
  *
  * It uses stat(2) and
  * compares stored and actual changed time of directories. If
  * there is a difference it notifies about the change. Directories can be
  * added, removed from the list and scanning of particular directories
  * can be stopped and restarted. The whole class can be stopped and
  * restarted. Directories can be added/removed from list in
  * any state.
  * When a watched directory is changed, KDirWatch will emit
  * the signal @ref dirty().
  *
  * If a watched directory gets deleted, KDirwatch will remove it from
  * the list, and  emit the signal @ref deleted().
  *
  * It's possible to watch a directory that doesn't exist yet.
  * KDirWatch will emit a @ref dirty() signal when it is created.
  *
  * @short Class for watching directory changes.
  * @author Sven Radej <sven@lisa.exp.univie.ac.at>
  */
class KDirWatch : public QObject
{
  Q_OBJECT
    
  public:
   /**
    * Constructor.
    *
    * Does not begin scanning until @ref startScan()
    * is called. Default frequency is 500 ms.
    */
   KDirWatch ( int freq = 500 );

   /**
    * Destructor.
    *
    * Stops scanning and cleans up.
    */
   ~KDirWatch();

   /**
    * Adds a directory to the list of directories to be watched.
    *
    */
   void addDir(const QString& path);

   /**
    * Returns the time the directory was last changed.
    */
   time_t ctime(const QString& path);

   /**
    * Removes a directory from the list of scanned directories.
    *
    * If specified path is not in the list this does nothing.
    */
   void removeDir(const QString& path);

   /**
    * Stops scanning the specified path.
    *
    * The @p path is not deleted from the interal just, it is just skipped.
    * Call this function when you perform an huge operation
    * on this directory (copy/move big files or many files). When finished,
    * call @ref restartDirScan (path).
    * Returns @p false if specified path is not in list, @p true otherwise.
    */
   bool stopDirScan(const QString& path);

   /**
    * Restarts scanning for specified path.
    *
    * Resets ctime. It doesn't notify
    * the change (by emitted a signal), since the ctime value is reset.
    *
    * Call it when you are finished with big operations on that path,
    * @em and when @em you have refreshed that path.  Returns @p false
    * if specified path is not in list, @p true otherwise.  
    */
   bool restartDirScan(const QString& path);

   /**
    * Starts scanning of all dirs in list.
    *
    * If notify is @p true, all changed directories (since @ref
    * stopScan() call) will be notified for refresh. If notify is
    * @p false, all ctimes will be reset (except those who are stopped,
    * but only if @p skippedToo is @p false) and changed dirs won't be
    * notified. You can start scanning even if the list is
    * empty. First call should be called with @p false or else all 
    * directories 
    * in list will be notified.  If
    * @p skippedToo is true, the skipped directoris (scanning of which was
    * stopped with @ref stopDirScan() ) will be reset and notified
    * for change.  Otherwise, stopped directories will continue to be
    * unnotified.
    */
   void startScan( bool notify=false, bool skippedToo=false );

   /**
    * Stops scanning of all directories in internal list.
    *
    * The timer is stopped, but the list is not cleared.
    */
   void stopScan();
  
   bool contains( const QString& path ) const;
  
   /** @ref fileDirty() */
   void setFileDirty( const QString & _file );

   static KDirWatch* self();
    
 signals:

   /**
    * Emitted when a directory is changed.
    *
    * The new ctime is set
    * before the signal is emited.
    */
   void dirty (const QString& dir);
   
   /**
    * Emitted when KDirWatch learns that the file
    * @p _file has changed.
    *
    * This happens for instance when a .desktop file 
    * gets a new icon - but this isn't automatic, one has to call 
    * @ref setFileDirty() for this signal to be emitted.
    *
    * Note that KDirNotify is network transparent and
    * broadcasts to all processes, so it sort of supersedes this.
    */
   void fileDirty (const QString& _file);

   /**
    * Emitted when directory is deleted.
    *
    * When you receive this signal, the directory is not yet
    * deleted from the list. However, it will be removed from the
    * notification list afterwards automatically. 
    */
   void deleted (const QString& dir);
     
 protected:
   void resetList (bool reallyall);
   
 protected slots:
   void slotRescan();
   void famEventReceived();
   
 private:
  KDirWatchPrivate *d;
  static KDirWatch* s_pSelf;
};

#endif
