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

#include "klockfile.h"

#include <config.h>

#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <QtCore/QDate>
#include <QtCore/QFile>
#include <QtCore/QTextIStream>

#include "krandom.h"
#include "kglobal.h"
#include "kcomponentdata.h"
#include "ktemporaryfile.h"
#include "kde_file.h"

// TODO: http://www.spinnaker.de/linux/nfs-locking.html

class KLockFile::Private
{
public:
    Private(const KComponentData &c)
        : componentData(c)
    {
    }

    QString file;
    int staleTime;
    bool isLocked;
    bool recoverLock;
    bool linkCountSupport;
    QTime staleTimer;
    KDE_struct_stat statBuf;
    int pid;
    QString hostname;
    QString instance;
    QString lockRecoverFile;
    KComponentData componentData;
};


// 30 seconds
KLockFile::KLockFile(const QString &file, const KComponentData &componentData)
    : d(new Private(componentData))
{
  d->file = file;
  d->staleTime = 30;
  d->isLocked = false;
  d->recoverLock = false;
  d->linkCountSupport = true;
}

KLockFile::~KLockFile()
{
  unlock();
  delete d;
}

int
KLockFile::staleTime() const
{
  return d->staleTime;
}


void
KLockFile::setStaleTime(int _staleTime)
{
  d->staleTime = _staleTime;
}

static bool operator==( const KDE_struct_stat &st_buf1,
			const KDE_struct_stat &st_buf2)
{
#define FIELD_EQ(what)       (st_buf1.what == st_buf2.what)
  return FIELD_EQ(st_dev) && FIELD_EQ(st_ino) &&
         FIELD_EQ(st_uid) && FIELD_EQ(st_gid) && FIELD_EQ(st_nlink);
#undef FIELD_EQ
}

static bool operator!=( const KDE_struct_stat& st_buf1,
			const KDE_struct_stat& st_buf2 )
{
  return !(st_buf1 == st_buf2);
}

static bool testLinkCountSupport(const QByteArray &fileName)
{
   KDE_struct_stat st_buf;
   int result = -1;
   // Check if hardlinks raise the link count at all?
   if(!::link( fileName, fileName+".test" )) {
     result = KDE_lstat( fileName, &st_buf );
     ::unlink( fileName+".test" );
   }
   return (result < 0 || ((result == 0) && (st_buf.st_nlink == 2)));
}

static KLockFile::LockResult lockFile(const QString &lockFile, KDE_struct_stat &st_buf,
        bool &linkCountSupport, const KComponentData &componentData)
{
  QByteArray lockFileName = QFile::encodeName( lockFile );
  int result = KDE_lstat( lockFileName, &st_buf );
  if (result == 0)
     return KLockFile::LockFail;

  KTemporaryFile uniqueFile(componentData);
  uniqueFile.setFileTemplate(lockFile);
  if (!uniqueFile.open())
     return KLockFile::LockError;
  uniqueFile.setPermissions(QFile::ReadUser|QFile::WriteUser|QFile::ReadGroup|QFile::ReadOther);

  char hostname[256];
  hostname[0] = 0;
  gethostname(hostname, 255);
  hostname[255] = 0;
  QString componentName = componentData.componentName();

  QTextStream stream(&uniqueFile);
  stream << QString::number(getpid()) << endl
      << componentName << endl
      << hostname << endl;
  stream.flush();

  QByteArray uniqueName = QFile::encodeName( uniqueFile.fileName() );

  // Create lock file
  result = ::link( uniqueName, lockFileName );
  if (result != 0)
     return KLockFile::LockError;

  if (!linkCountSupport)
     return KLockFile::LockOK;

  KDE_struct_stat st_buf2;
  result = KDE_lstat( uniqueName, &st_buf2 );
  if (result != 0)
     return KLockFile::LockError;

  result = KDE_lstat( lockFileName, &st_buf );
  if (result != 0)
     return KLockFile::LockError;

  if (st_buf != st_buf2 || S_ISLNK(st_buf.st_mode) || S_ISLNK(st_buf2.st_mode))
  {
     // SMBFS supports hardlinks by copying the file, as a result the above test will always fail
     // cifs increases link count artifically but the inodes are still different
     if ((st_buf2.st_nlink > 1 ||
         ((st_buf.st_nlink == 1) && (st_buf2.st_nlink == 1))) && (st_buf.st_ino != st_buf2.st_ino))
     {
        linkCountSupport = testLinkCountSupport(uniqueName);
        if (!linkCountSupport)
           return KLockFile::LockOK; // Link count support is missing... assume everything is OK.
     }
     return KLockFile::LockFail;
  }

  return KLockFile::LockOK;
}

static KLockFile::LockResult deleteStaleLock(const QString &lockFile, KDE_struct_stat &st_buf, bool &linkCountSupport, const KComponentData &componentData)
{
   // This is dangerous, we could be deleting a new lock instead of
   // the old stale one, let's be very careful

   // Create temp file
   KTemporaryFile *ktmpFile = new KTemporaryFile(componentData);
   ktmpFile->setFileTemplate(lockFile);
   if (!ktmpFile->open())
      return KLockFile::LockError;

   QByteArray lckFile = QFile::encodeName(lockFile);
   QByteArray tmpFile = QFile::encodeName(ktmpFile->fileName());
   delete ktmpFile;

   // link to lock file
   if (::link(lckFile, tmpFile) != 0)
      return KLockFile::LockFail; // Try again later

   // check if link count increased with exactly one
   // and if the lock file still matches
   KDE_struct_stat st_buf1;
   KDE_struct_stat st_buf2;
   memcpy(&st_buf1, &st_buf, sizeof(KDE_struct_stat));
   st_buf1.st_nlink++;
   if ((KDE_lstat(tmpFile, &st_buf2) == 0) && st_buf1 == st_buf2)
   {
      if ((KDE_lstat(lckFile, &st_buf2) == 0) && st_buf1 == st_buf2)
      {
         // - - if yes, delete lock file, delete temp file, retry lock
         qWarning("WARNING: deleting stale lockfile %s", lckFile.data());
         ::unlink(lckFile);
         ::unlink(tmpFile);
         return KLockFile::LockOK;
      }
   }

   // SMBFS supports hardlinks by copying the file, as a result the above test will always fail
   if (linkCountSupport)
   {
      linkCountSupport = testLinkCountSupport(tmpFile);
   }

   if (!linkCountSupport)
   {
      // Without support for link counts we will have a little race condition
      qWarning("WARNING: deleting stale lockfile %s", lckFile.data());
      ::unlink(tmpFile);
      if (::unlink(lckFile) < 0) {
          qWarning("WARNING: Problem deleting stale lockfile %s: %s", lckFile.data(),
                  strerror(errno));
          return KLockFile::LockFail;
      }
      return KLockFile::LockOK;
   }

   // Failed to delete stale lock file
   qWarning("WARNING: Problem deleting stale lockfile %s", lckFile.data());
   ::unlink(tmpFile);
   return KLockFile::LockFail;
}


KLockFile::LockResult KLockFile::lock(LockFlags options)
{
  if (d->isLocked)
     return KLockFile::LockOK;

  KLockFile::LockResult result;
  int hardErrors = 5;
  int n = 5;
  while(true)
  {
     KDE_struct_stat st_buf;
     result = lockFile(d->file, st_buf, d->linkCountSupport, d->componentData);
     if (result == KLockFile::LockOK)
     {
        d->staleTimer = QTime();
        break;
     }
     else if (result == KLockFile::LockError)
     {
        d->staleTimer = QTime();
        if (--hardErrors == 0)
        {
           break;
        }
     }
     else // KLockFile::Fail -- there is already such a file present (e.g. left by a crashed app)
     {
        if (!d->staleTimer.isNull() && d->statBuf != st_buf)
           d->staleTimer = QTime();

        if (d->staleTimer.isNull())
        {
           memcpy(&(d->statBuf), &st_buf, sizeof(KDE_struct_stat));
           d->staleTimer.start();

           d->pid = -1;
           d->hostname.clear();
           d->instance.clear();

           QFile file(d->file);
           if (file.open(QIODevice::ReadOnly))
           {
              QTextStream ts(&file);
              if (!ts.atEnd())
                 d->pid = ts.readLine().toInt();
              if (!ts.atEnd())
                 d->instance = ts.readLine();
              if (!ts.atEnd())
                 d->hostname = ts.readLine();
           }
        }

        bool isStale = false;
        if ((d->pid > 0) && !d->hostname.isEmpty())
        {
           // Check if hostname is us
           char hostname[256];
           hostname[0] = 0;
           gethostname(hostname, 255);
           hostname[255] = 0;

           if (d->hostname == QLatin1String(hostname))
           {
              // Check if pid still exists
              int res = ::kill(d->pid, 0);
              if ((res == -1) && (errno == ESRCH))
                 isStale = true;
           }
        }
        if (d->staleTimer.elapsed() > (d->staleTime*1000))
           isStale = true;

        if (isStale)
        {
           if ((options & ForceFlag) == 0)
              return KLockFile::LockStale;

           result = deleteStaleLock(d->file, d->statBuf, d->linkCountSupport, d->componentData);

           if (result == KLockFile::LockOK)
           {
              // Lock deletion successful
              d->staleTimer = QTime();
              continue; // Now try to get the new lock
           }
           else if (result != KLockFile::LockFail)
           {
              return result;
           }
        }
     }

     if (options & NoBlockFlag)
        break;

     struct timeval tv;
     tv.tv_sec = 0;
     tv.tv_usec = n*((KRandom::random() % 200)+100);
     if (n < 2000)
        n = n * 2;

     select(0, 0, 0, 0, &tv);
  }
  if (result == LockOK)
     d->isLocked = true;
  return result;
}

bool KLockFile::isLocked() const
{
  return d->isLocked;
}

void KLockFile::unlock()
{
  if (d->isLocked)
  {
     ::unlink(QFile::encodeName(d->file));
     d->isLocked = false;
  }
}

bool KLockFile::getLockInfo(int &pid, QString &hostname, QString &appname)
{
  if (d->pid == -1)
     return false;
  pid = d->pid;
  hostname = d->hostname;
  appname = d->instance;
  return true;
}
