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

#include <kcoreaddons_export.h>
#include <QtCore/QString>
#include <QtCore/QFlags>

class QString;

/**
 * \class KLockFile klockfile.h <KLockFile>
 *
 * The KLockFile class provides NFS safe lockfiles.
 *
 * @author Waldo Bastian <bastian@kde.org>
 * @deprecated since 5.0, please use QLockFile instead
 */
class KCOREADDONS_EXPORT KLockFile
{
public:

   /**
    * Constructor
    * @deprecated since 5.0, use QLockFile(file), drop the component name.
    */
   explicit KLockFile(const QString &file, const QString &componentName = QString());

   /**
    * Destroys the object, releasing the lock if held
    */
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
    * @deprecated since 5.0
    * KLockFile::lock() --> QLockFile::lock(). Possibly after setStaleLockTime(0), but
    *                                  only for the case of protecting a resource for a very long time.
    * KLockFile::lock(NoBlockFlag) --> QLockFile::tryLock(). Possibly after setStaleLockTime(0), but
    *                                  only for the case of protecting a resource for a very long time.
    * KLockFile::lock(ForceFlag) --> QLockFile::lock().
    * KLockFile::lock(NoBlockFlag|ForceFlag) --> QLockFile::tryLock().
    * Note that the return value is now simply a bool (success/failure).
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
    * @deprecated since 5.0. WARNING: QLockFile::staleLockTime() is in ms, so divide the result by 1000.
    */
   int staleTime() const;

   /**
    * Set the time in seconds after which a lock is considered stale
    * @deprecated since 5.0. WARNING: QLockFile::setStaleLockTime() is in ms, so multiply the argument by 1000.
    */
   void setStaleTime(int _staleTime);

   /**
    * Returns the pid, hostname and appname of the process holding
    * the lock after the lock functon has returned with LockStale.
    * @returns false if the pid and hostname could not be determined
    * @deprecated since 5.0. Use QLockFile::getLockInfo(qint64 *pid, QString *hostname, QString *appname)
    */
   bool getLockInfo(int &pid, QString &hostname, QString &appname);

private:
   class Private;
   Private *const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KLockFile::LockFlags)

#endif
