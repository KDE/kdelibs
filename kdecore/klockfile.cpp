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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <klockfile.h>

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

#include <qfile.h>
#include <qtextstream.h>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <ktempfile.h>

// TODO: http://www.spinnaker.de/linux/nfs-locking.html
// TODO: Make regression test

class KLockFile::KLockFilePrivate {
public:
   QString file;
   int staleTime;
   bool isLocked;
   bool recoverLock;
   QTime staleTimer;
   struct stat statBuf;
   int pid;
   QString hostname;
   QString instance;
   QString lockRecoverFile;
};


// 30 seconds
KLockFile::KLockFile(const QString &file)
{
  d = new KLockFilePrivate();
  d->file = file;
  d->staleTime = 30;
  d->isLocked = false;
  d->recoverLock = false;
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

static bool statResultIsEqual(struct stat &st_buf1, struct stat &st_buf2)
{
#define FIELD_EQ(what)       (st_buf1.what == st_buf2.what)
  return FIELD_EQ(st_dev) && FIELD_EQ(st_ino) && 
         FIELD_EQ(st_uid) && FIELD_EQ(st_gid) && FIELD_EQ(st_nlink);
#undef FIELD_EQ
}

static KLockFile::LockResult lockFile(const QString &lockFile, struct stat &st_buf)
{
  QCString lockFileName = QFile::encodeName( lockFile );
  int result = ::lstat( lockFileName, &st_buf );
  if (result == 0)
     return KLockFile::LockFail;
  
  KTempFile uniqueFile(lockFile, QString::null, 0644);
  uniqueFile.setAutoDelete(true);
  if (uniqueFile.status() != 0)
     return KLockFile::LockError;

  char hostname[256];
  hostname[0] = 0;
  gethostname(hostname, 255);
  hostname[255] = 0;
  QCString instanceName = KCmdLineArgs::appName();

  (*(uniqueFile.textStream())) << QString::number(getpid()) << endl
      << instanceName << endl
      << hostname << endl;
  uniqueFile.close();
  
  QCString uniqueName = QFile::encodeName( uniqueFile.name() );
      
#ifdef Q_OS_UNIX
  // Create lock file
  result = ::link( uniqueName, lockFileName );
  if (result != 0)
     return KLockFile::LockError;

#else
  //TODO for win32
  return KLockFile::LockOK;
#endif

  struct stat st_buf2;
  result = ::lstat( uniqueName, &st_buf2 );
  if (result != 0)
     return KLockFile::LockError;

  result = ::lstat( lockFileName, &st_buf );
  if (result != 0)
     return KLockFile::LockError;

  if (!statResultIsEqual(st_buf, st_buf2) || S_ISLNK(st_buf.st_mode) || S_ISLNK(st_buf2.st_mode))
     return KLockFile::LockFail;

  return KLockFile::LockOK;
}

static KLockFile::LockResult deleteStaleLock(const QString &lockFile, struct stat &st_buf)
{
   // This is dangerous, we could be deleting a new lock instead of
   // the old stale one, let's be very careful
              
   // Create temp file
   KTempFile ktmpFile(lockFile);
   if (ktmpFile.status() != 0)
      return KLockFile::LockError;
              
   QCString lckFile = QFile::encodeName( lockFile );
   QCString tmpFile = QFile::encodeName(ktmpFile.name());
   ktmpFile.close();
   ktmpFile.unlink();
              
#ifdef Q_OS_UNIX
   // link to lock file
   if (::link(lckFile, tmpFile) != 0)
      return KLockFile::LockFail; // Try again later
#else
   //TODO for win32
   return KLockFile::LockOK;
#endif

   // check if link count increased with exactly one
   // and if the lock file still matches
   struct stat st_buf1;
   struct stat st_buf2;
   memcpy(&st_buf1, &st_buf, sizeof(struct stat));
   st_buf1.st_nlink++;
   if ((lstat(tmpFile, &st_buf2) == 0) && statResultIsEqual(st_buf1, st_buf2))
   {
      if ((lstat(lckFile, &st_buf2) == 0) && statResultIsEqual(st_buf1, st_buf2))
      {
         // - - if yes, delete lock file, delete temp file, retry lock
         qWarning("WARNING: deleting stale lockfile %s", lckFile.data());
         ::unlink(lckFile);
         ::unlink(tmpFile);
         return KLockFile::LockOK;
      }
   }
   // Failed to delete stale lock file
   qWarning("WARNING: Problem deleting stale lockfile %s", lckFile.data());
   ::unlink(tmpFile);
   return KLockFile::LockFail;
}


KLockFile::LockResult KLockFile::lock(int options)
{
  if (d->isLocked)
     return KLockFile::LockOK;

  KLockFile::LockResult result;     
  int hardErrors = 5;
  int n = 5;
  while(true)
  {
     struct stat st_buf;
     result = lockFile(d->file, st_buf);
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
     else // KLockFile::Fail
     {
        if (!d->staleTimer.isNull() && !statResultIsEqual(d->statBuf, st_buf))
           d->staleTimer = QTime();
           
        if (!d->staleTimer.isNull())
        {
           bool isStale = false;
           if ((d->pid > 0) && !d->hostname.isEmpty())
           {
              // Check if hostname is us
              char hostname[256];
              hostname[0] = 0;
              gethostname(hostname, 255);
              hostname[255] = 0;
              
              if (d->hostname == hostname)
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
              if ((options & LockForce) == 0)
                 return KLockFile::LockStale;
                 
              result = deleteStaleLock(d->file, d->statBuf);

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
        else
        {
           memcpy(&(d->statBuf), &st_buf, sizeof(struct stat));
           d->staleTimer.start();
           
           d->pid = -1;
           d->hostname = QString::null;
           d->instance = QString::null;
        
           QFile file(d->file);
           if (file.open(IO_ReadOnly))
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
     }
        
     if ((options & LockNoBlock) != 0)
        break;
     
     struct timeval tv;
     tv.tv_sec = 0;
     tv.tv_usec = n*((KApplication::random() % 200)+100);
     if (n < 2000)
        n = n * 2;
     
#ifdef Q_OS_UNIX
     select(0, 0, 0, 0, &tv);
#else
     //TODO for win32
#endif
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
