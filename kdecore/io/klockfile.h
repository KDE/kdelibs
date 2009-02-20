/*
   This file is part of the KDE libraries
   Copyright (c) 2004 Waldo Bastian <bastian@kde.org>

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

#ifndef KLOCKFILE_H
#define KLOCKFILE_H

#include <kdecore_export.h>
#include <ksharedptr.h>
#include <kglobal.h>

class QString;

/**
 * \class KLockFile klockfile.h <KLockFile>
 * 
 * The KLockFile class provides NFS safe lockfiles.
 *
 * @author Waldo Bastian <bastian@kde.org>
 */
class KDECORE_EXPORT KLockFile : public KShared
{
public:
   typedef KSharedPtr<KLockFile> Ptr;

   explicit KLockFile(const QString &file, const KComponentData &componentName = KGlobal::mainComponent());

   /**
    * Destroys the object, releasing the lock if held
    **/
   ~KLockFile();

   /**
    * Possible return values of the lock function.
    */
   enum LockResult {
     /**
      * Lock was acquired successfully
      */
     LockOK = 0,

     /**
      * The lock could not be acquired because it is held by another process
      */
     LockFail,

     /**
      * The lock could not be acquired due to an error
      */
     LockError,

     /**
      * A stale lock has been detected
      */
     LockStale
   };

   enum LockFlag {
     /**
      * Return immediately, do not wait for the lock to become available
      */
     NoBlockFlag = 1,

     /**
      * Automatically remove a lock when a lock is detected that is stale
      * for more than staleTime() seconds, or if the process that created it
      * is not running anymore.
      */
     ForceFlag = 2
   };
   Q_DECLARE_FLAGS(LockFlags, LockFlag)

   /**
    * Attempt to acquire the lock
    *
    * @param flags A set of @ref LockFlag values OR'ed together.
    */
   LockResult lock(LockFlags flags=LockFlags());

   /**
    * Returns whether the lock is held or not
    */
   bool isLocked() const;

   /**
    * Release the lock
    */
   void unlock();

   /**
    * Return the time in seconds after which a lock is considered stale
    * The default is 30.
    */
   int staleTime() const;

   /**
    * Set the time in seconds after which a lock is considered stale
    */
   void setStaleTime(int _staleTime);

   /**
    * Returns the pid, hostname and appname of the process holding
    * the lock after the lock functon has returned with LockStale.
    * @returns false if the pid and hostname could not be determined
    */
   bool getLockInfo(int &pid, QString &hostname, QString &appname);

private:
   class Private;
   Private *const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KLockFile::LockFlags)

#endif
