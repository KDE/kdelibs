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

#include <qtimer.h>
#include <qdatetime.h>
#include <qmap.h>

#define kdirwatch KDirWatch::self()

class KDirWatchPrivate;

 /**
  * Watch directories and files for changes.
  * The watched directories or files doesn't have to exist yet.
  *
  * When a watched directory is changed, i.e. when files therein are
  * created or deleted, KDirWatch will emit the signal @ref dirty().
  *
  * When a watched, but previously not existing directory gets created,
  * KDirWatch will emit the signal @ref created().
  *
  * When a watched directory gets deleted, KDirWatch will emit the
  * signal @ref deleted(). The directory is still watched for new
  * creation.
  *
  * When a watched file is changed, i.e. attributes changed or written
  * to, KDirWatch will emit the signal @ref fileDirty().
  *
  * Scanning of particular directories or files can be stopped temporarily
  * and restarted. The whole class can be stopped and restarted.
  * Directories and files can be added/removed from list in any state.
  *
  * The implementation uses the FAM service when available;
  * if FAM is not available, the DNOTIFY functionality is used on LINUX.
  * As a last resort, a regular polling for change of modification times
  * is done; note that this is a bad solution when watching NFS mounted
  * directories.
  *
  * @short Class for watching directory and file changes.
  * @author Sven Radej <sven@lisa.exp.univie.ac.at>
  */
class KDirWatch : public QObject
{
  Q_OBJECT
    
  public:
   /**
    * Constructor.
    *
    * Scanning begins immediatly when a dir/file watch
    * is added.
    * The default scan frequency is 500 ms.
    */
   KDirWatch ( int freq = 500 );

   /**
    * Destructor.
    *
    * Stops scanning and cleans up.
    */
   ~KDirWatch();

   /**
    * Set the name of this KDirWatch instance
    * Used in output of @ref statistics()
    */
   void setName(const QString& name) { _name = name; }

   /**
    * Get the name of this KDirWatch instance
    * Used in output of @ref statistics()
    */
   const QString& name() { return _name; }

   /**
    * Adds a directory to be watched.
    *
    */
   void addDir(const QString& path);

   /**
    * Adds a file to be watched.
    *
    */
   void addFile(const QString& file);

   /**
    * Returns the time the directory/file was last changed.
    */
   QDateTime ctime(const QString& path);

   /**
    * Removes a directory from the list of scanned directories.
    *
    * If specified path is not in the list this does nothing.
    */
   void removeDir(const QString& path);

   /**
    * Removes a file from the list of watched files.
    *
    * If specified path is not in the list this does nothing.
    */
   void removeFile(const QString& file);

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

   /**
    * Is scanning stopped?
    * After creation of KDirWatch, this is true.
    */
   bool isStopped() { return _isStopped; }

   /**
    * Check if a directory is being watched by this KDirWatch instance
    */
   bool contains( const QString& path ) const;

   /**
    * Dump statistic information about all KDirWatch instances.
    * This checks for consistency, too.
    */
   static void statistics();

   /** @ref created() */
   void setDirCreated( const QString& );
   /** @ref dirty() */
   void setDirDirty( const QString& );
   /** @ref deleted() */
   void setDirDeleted( const QString& );
   /** @ref fileCreated() */
   void setFileCreated( const QString& );
   /** @ref fileDirty() */
   void setFileDirty( const QString& );
   /** @ref fileDeleted() */
   void setFileDeleted( const QString& );

   /**
    * The KDirWatch instance usually globally used in an application.
    * However, you can create an arbitrary number of KDirWatch instances
    * aside from this one.
    */
   static KDirWatch* self();
    
 signals:

   /**
    * Emitted when a watched directory is changed, i.e. files
    * therein are created, deleted or changed.
    *
    * The new ctime is set
    * before the signal is emitted.
    */
   void dirty (const QString&);

   /**
    * Emitted when a watched directory is created.
    */
   void created (const QString&);
     
   /**
    * Emitted when a watched directory is deleted.
    *
    * The directory is still watched for new creation.
    */
   void deleted (const QString&);
     
   
   /**
    * Emitted when KDirWatch learns that the file
    * @p _file has changed.
    *
    * This happens for instance when a .desktop file 
    * gets a new icon. One has to call 
    * @ref setFileDirty() or watch the file for changes
    * for this signal to be emitted.
    *
    * Note that KDirNotify is network transparent and
    * broadcasts to all processes, so it sort of supersedes this.
    */
   void fileDirty (const QString&);

   /**
    * Emitted when a watched file is created.
    */
   void fileCreated (const QString&);

   /**
    * Emitted when a watched file is deleted.
    */
   void fileDeleted (const QString&);

 private:
   bool _isStopped;
   QString _name;
   
   KDirWatchPrivate *d;
   static KDirWatch* s_pSelf;
};

#endif
