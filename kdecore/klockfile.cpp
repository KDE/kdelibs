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
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <qfile.h>
#include <qtextstream.h>

#include <kapplication.h>
#include <kglobal.h>
#include <ktempfile.h>

// 30 seconds
KLockFile::KLockFile(const QString &file) : m_file(file), m_staleTime(30), m_isLocked(false), m_recoverLock(false)
{
  m_statBuf = (void *)malloc(sizeof(struct stat));
}

KLockFile::~KLockFile()
{
  unlock();
  free(m_statBuf);
}

void
KLockFile::setStaleTime(int _staleTime)
{
  m_staleTime = _staleTime;
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

  (*(uniqueFile.textStream())) << QString::number(getpid()) << endl
      << KGlobal::instance()->instanceName() << endl
      << hostname << endl;
  uniqueFile.close();
  
  QCString uniqueName = QFile::encodeName( uniqueFile.name() );
      
  // Create lock file
  result = ::link( uniqueName, lockFileName );
  if (result != 0)
     return KLockFile::LockError;

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
              
   // link to lock file
   if (::link(lckFile, tmpFile) != 0)
      return KLockFile::LockFail; // Try again later

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
  if (m_isLocked)
     return KLockFile::LockOK;

  KLockFile::LockResult result;     
  int hardErrors = 5;
  int n = 5;
  while(true)
  {
     struct stat st_buf;
     result = lockFile(m_file, st_buf);
     if (result == KLockFile::LockOK)
     {
        m_staleTimer = QTime();
        break;
     }
     else if (result == KLockFile::LockError)
     {
        m_staleTimer = QTime();
        if (--hardErrors == 0)
        {
           break;
        }
     }
     else // KLockFile::Fail
     {
        if (!m_staleTimer.isNull() && !statResultIsEqual(*(struct stat*)m_statBuf, st_buf))
           m_staleTimer = QTime();
           
        if (!m_staleTimer.isNull())
        {
           bool isStale = false;
           if ((m_pid != -1) && !m_hostname.isEmpty())
           {
              // Check if hostname is us
              char hostname[256];
              hostname[0] = 0;
              gethostname(hostname, 255);
              hostname[255] = 0;
              
              if (m_hostname == hostname)
              {
                 // Check if pid still exists
                 int res = ::kill(m_pid, 0);
                 if ((res == -1) && (errno == ESRCH))
                    isStale = true;
              }
           }
           if (m_staleTimer.elapsed() > (m_staleTime*1000))
              isStale = true;
           
           if (isStale)
           {
              if ((options & LockForce) == 0)
                 return KLockFile::LockStale;
                 
              result = deleteStaleLock(m_file, *((struct stat*)m_statBuf));

              if (result == KLockFile::LockOK)
              {
                 // Lock deletion successful
                 m_staleTimer = QTime();
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
           memcpy(m_statBuf, &st_buf, sizeof(struct stat));
           m_staleTimer.start();
           
           m_pid = -1;
           m_hostname = QString::null;
           m_instance = QString::null;
        
           QFile file(m_file);
           if (file.open(IO_ReadOnly))
           {
              QTextStream ts(&file);
              if (!ts.atEnd())
                 m_pid = ts.readLine().toInt();
              if (!ts.atEnd())
                 m_instance = ts.readLine();
              if (!ts.atEnd())
                 m_hostname = ts.readLine();
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
     
     select(0, 0, 0, 0, &tv);
  }
  if (result == LockOK)
     m_isLocked = true;
  return result;
}
   
bool KLockFile::isLocked() const
{
  return m_isLocked;
}
   
void KLockFile::unlock()
{
  if (m_isLocked)
  {
     ::unlink(QFile::encodeName(m_file));
     m_isLocked = false;
  }
}

bool KLockFile::getLockInfo(int &pid, QString &hostname)
{
  if (m_pid == -1)
     return false;
  pid = m_pid;
  hostname = m_hostname;
  return true;
}
