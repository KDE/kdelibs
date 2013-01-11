/*
   This file is part of the KDE libraries
   Copyright (c) 2004 Waldo Bastian <bastian@kde.org>
   Copyright (c) 2011 David Faure <faure@kde.org>

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

#include <config-klockfile.h>

#include <sys/types.h>
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <qplatformdefs.h> // QT_STATBUF, QT_LSTAT, QT_OPEN
#include <QtCore/QDate>
#include <QtCore/QFile>
#include <QCoreApplication>
#include <QTextStream>
#include <QTemporaryFile>

#include "krandom.h"
#include "kfilesystemtype_p.h"

#include <unistd.h>
#include <fcntl.h>

// Related reading:
// http://www.spinnaker.de/linux/nfs-locking.html
// http://en.wikipedia.org/wiki/File_locking
// http://apenwarr.ca/log/?m=201012

// Related source code:
// * lockfile-create, from the lockfile-progs package, uses the link() trick from lockFileWithLink
// below, so it works over NFS but fails on FAT32 too.
// * the flock program, which uses flock(LOCK_EX), works on local filesystems (including FAT32),
//    but not NFS.
//  Note about flock: don't unlink, it creates a race. http://world.std.com/~swmcd/steven/tech/flock.html

// fcntl(F_SETLK) is not a good solution.
// It locks other processes but locking out other threads must be done by hand,
// and worse, it unlocks when just reading the file in the same process (!).
// See the apenwarr.ca article above.

// open(O_EXCL) seems to be the best solution for local files (on all filesystems),
// it only fails over NFS (at least with old NFS servers).
// See http://www.informit.com/guides/content.aspx?g=cplusplus&seqNum=144

// Conclusion: we use O_EXCL by default, and the link() trick over NFS.

class KLockFile::Private
{
public:
    Private(const QString& componentName)
        : staleTime(30), // 30 seconds
          isLocked(false),
          linkCountSupport(true),
          mustCloseFd(false),
          m_pid(-1),
          m_componentName(componentName)
    {
    }

    // The main method
    KLockFile::LockResult lockFile(QT_STATBUF &st_buf);

    // Two different implementations
    KLockFile::LockResult lockFileOExcl(QT_STATBUF &st_buf);
    KLockFile::LockResult lockFileWithLink(QT_STATBUF &st_buf);

    KLockFile::LockResult deleteStaleLock();
    KLockFile::LockResult deleteStaleLockWithLink();

    void writeIntoLockFile(QFile& file);
    void readLockFile();
    bool isNfs() const;

    QFile m_file;
    QString m_fileName;
    int staleTime;
    bool isLocked;
    bool linkCountSupport;
    bool mustCloseFd;
    QTime staleTimer;
    QT_STATBUF statBuf;
    int m_pid;
    QString m_hostname;
    QString m_componentName; // as set for this instance
    QString m_componentNameFromFile; // as read from the lock file
};


KLockFile::KLockFile(const QString &file, const QString &componentName)
    : d(new Private(componentName))
{
  d->m_fileName = file;
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

static bool operator==( const QT_STATBUF &st_buf1,
            const QT_STATBUF &st_buf2)
{
#define FIELD_EQ(what)       (st_buf1.what == st_buf2.what)
  return FIELD_EQ(st_dev) && FIELD_EQ(st_ino) &&
         FIELD_EQ(st_uid) && FIELD_EQ(st_gid) && FIELD_EQ(st_nlink);
#undef FIELD_EQ
}

static bool operator!=( const QT_STATBUF& st_buf1,
            const QT_STATBUF& st_buf2 )
{
  return !(st_buf1 == st_buf2);
}

static bool testLinkCountSupport(const QByteArray &fileName)
{
   QT_STATBUF st_buf;
   int result = -1;
   // Check if hardlinks raise the link count at all?
   if(!::link( fileName.data(), QByteArray(fileName+".test").data() )) {
     result = QT_LSTAT( fileName.data(), &st_buf );
     ::unlink( QByteArray(fileName+".test").data() );
   }
   return (result < 0 || ((result == 0) && (st_buf.st_nlink == 2)));
}

void KLockFile::Private::writeIntoLockFile(QFile& file)
{
  file.setPermissions(QFile::ReadUser|QFile::WriteUser|QFile::ReadGroup|QFile::ReadOther);

  char hostname[256];
  hostname[0] = 0;
  gethostname(hostname, 255);
  hostname[255] = 0;
  m_hostname = QString::fromLocal8Bit(hostname);
  if (m_componentName.isEmpty() && QCoreApplication::instance()) // TODO Qt5: should be fixed by new Q_GLOBAL_STATIC: qcoreappdata() was dangling, in kconfigtest testSyncOnExit.
      m_componentName = QCoreApplication::applicationName();

  m_pid = getpid();

  file.write(QByteArray::number(m_pid) + '\n');
  file.write(m_componentName.toUtf8() + '\n');
  file.write(hostname);
  file.flush();
}

void KLockFile::Private::readLockFile()
{
    m_pid = -1;
    m_hostname.clear();
    m_componentNameFromFile.clear();

    QFile file(m_fileName);
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream ts(&file);
        if (!ts.atEnd())
            m_pid = ts.readLine().toInt();
        if (!ts.atEnd())
            m_componentNameFromFile = ts.readLine();
        if (!ts.atEnd())
            m_hostname = ts.readLine();
    }
}

KLockFile::LockResult KLockFile::Private::lockFileWithLink(QT_STATBUF &st_buf)
{
  const QByteArray lockFileName = QFile::encodeName( m_fileName );
  int result = QT_LSTAT( lockFileName.data(), &st_buf );
  if (result == 0) {
     return KLockFile::LockFail;
  }

  QTemporaryFile uniqueFile;
  uniqueFile.setFileTemplate(m_fileName);
  if (!uniqueFile.open())
     return KLockFile::LockError;

  writeIntoLockFile(uniqueFile);

  QByteArray uniqueName = QFile::encodeName( uniqueFile.fileName() );

  // Create lock file
  result = ::link( uniqueName.data(), lockFileName.data() );
  if (result != 0)
     return KLockFile::LockError;

  if (!linkCountSupport)
     return KLockFile::LockOK;

  QT_STATBUF st_buf2;
  result = QT_LSTAT( uniqueName.data(), &st_buf2 );
  if (result != 0)
     return KLockFile::LockError;

  result = QT_LSTAT( lockFileName.data(), &st_buf );
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

bool KLockFile::Private::isNfs() const
{
    const KFileSystemType::Type fsType = KFileSystemType::fileSystemType(m_fileName);
    return fsType == KFileSystemType::Nfs;
}

KLockFile::LockResult KLockFile::Private::lockFile(QT_STATBUF &st_buf)
{
    if (isNfs()) {
        return lockFileWithLink(st_buf);
    }

    return lockFileOExcl(st_buf);
}

KLockFile::LockResult KLockFile::Private::lockFileOExcl(QT_STATBUF &st_buf)
{
    const QByteArray lockFileName = QFile::encodeName( m_fileName );

    int fd = QT_OPEN(lockFileName.constData(), O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (fd < 0) {
        if (errno == EEXIST) {
            // File already exists
            if (QT_LSTAT(lockFileName.constData(), &st_buf) != 0) { // caller wants stat buf details
                // File got deleted meanwhile! Clear struct rather than leaving it unset.
                st_buf.st_dev = 0;
                st_buf.st_ino = 0;
                st_buf.st_uid = 0;
                st_buf.st_gid = 0;
                st_buf.st_nlink = 0;
            }
            return LockFail;
        } else {
            return LockError;
        }
    }
    // We hold the lock, continue.
    if (!m_file.open(fd, QIODevice::WriteOnly)) {
        return LockError;
    }
    mustCloseFd = true;
    writeIntoLockFile(m_file);

    // stat to get the modification time
    const int result = QT_LSTAT(QFile::encodeName(m_fileName).data(), &st_buf);
    if (result != 0)
        return KLockFile::LockError;
    return KLockFile::LockOK;
}

KLockFile::LockResult KLockFile::Private::deleteStaleLock()
{
    if (isNfs())
        return deleteStaleLockWithLink();

    // I see no way to prevent the race condition here, where we could
    // delete a new lock file that another process just got after we
    // decided the old one was too stale for us too.
    qWarning("WARNING: deleting stale lockfile %s", qPrintable(m_fileName));
    QFile::remove(m_fileName);
    return LockOK;
}

KLockFile::LockResult KLockFile::Private::deleteStaleLockWithLink()
{
    // This is dangerous, we could be deleting a new lock instead of
    // the old stale one, let's be very careful

    // Create temp file
    QTemporaryFile *ktmpFile = new QTemporaryFile;
    ktmpFile->setFileTemplate(m_fileName);
    if (!ktmpFile->open()) {
        delete ktmpFile;
        return KLockFile::LockError;
    }

    const QByteArray lckFile = QFile::encodeName(m_fileName);
    const QByteArray tmpFile = QFile::encodeName(ktmpFile->fileName());
    delete ktmpFile;

   // link to lock file
   if (::link(lckFile.data(), tmpFile.data()) != 0)
      return KLockFile::LockFail; // Try again later

   // check if link count increased with exactly one
   // and if the lock file still matches
   QT_STATBUF st_buf1;
   QT_STATBUF st_buf2;
   memcpy(&st_buf1, &statBuf, sizeof(QT_STATBUF));
   st_buf1.st_nlink++;
   if ((QT_LSTAT(tmpFile.data(), &st_buf2) == 0) && st_buf1 == st_buf2)
   {
      if ((QT_LSTAT(lckFile.data(), &st_buf2) == 0) && st_buf1 == st_buf2)
      {
         // - - if yes, delete lock file, delete temp file, retry lock
         qWarning("WARNING: deleting stale lockfile %s", lckFile.data());
         ::unlink(lckFile.data());
         ::unlink(tmpFile.data());
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
      ::unlink(tmpFile.data());
      if (::unlink(lckFile.data()) < 0) {
          qWarning("WARNING: Problem deleting stale lockfile %s: %s", lckFile.data(),
                  strerror(errno));
          return KLockFile::LockFail;
      }
      return KLockFile::LockOK;
   }

   // Failed to delete stale lock file
   qWarning("WARNING: Problem deleting stale lockfile %s", lckFile.data());
   ::unlink(tmpFile.data());
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
        QT_STATBUF st_buf;
        // Try to create the lock file
        result = d->lockFile(st_buf);

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
           memcpy(&(d->statBuf), &st_buf, sizeof(QT_STATBUF));
           d->staleTimer.start();

           d->readLockFile();
        }

        bool isStale = false;
        if ((d->m_pid > 0) && !d->m_hostname.isEmpty())
        {
           // Check if hostname is us
           char hostname[256];
           hostname[0] = 0;
           gethostname(hostname, 255);
           hostname[255] = 0;

           if (d->m_hostname == QString::fromLocal8Bit(hostname))
           {
              // Check if pid still exists
              int res = ::kill(d->m_pid, 0);
              if ((res == -1) && (errno == ESRCH))
                  isStale = true; // pid does not exist
           }
        }
        if (d->staleTimer.elapsed() > (d->staleTime*1000))
           isStale = true;

        if (isStale)
        {
           if ((options & ForceFlag) == 0)
              return KLockFile::LockStale;

           result = d->deleteStaleLock();

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
      ::unlink(QFile::encodeName(d->m_fileName).data());
      if (d->mustCloseFd) {
         close(d->m_file.handle());
         d->mustCloseFd = false;
     }
     d->m_file.close();
     d->m_pid = -1;
     d->isLocked = false;
  }
}

bool KLockFile::getLockInfo(int &pid, QString &hostname, QString &appname)
{
  if (d->m_pid == -1)
     return false;
  pid = d->m_pid;
  hostname = d->m_hostname;
  appname = d->m_componentNameFromFile;
  return true;
}
