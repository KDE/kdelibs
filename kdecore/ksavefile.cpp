/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Waldo Bastian <bastian@kde.org>

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

#include <config.h>

#include <sys/types.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_TEST
#include <test.h>
#endif

#include <qdatetime.h>
#include <qdir.h>

#include "kapplication.h"
#include "ksavefile.h"

KSaveFile::KSaveFile(const QString &filename, int mode)
 : mTempFile(true)
{
   // we only check here if the directory can be written to
   // the actual filename isn't written to, but replaced later
   // with the contents of our tempfile
   if (!checkAccess(filename, W_OK))
   {
      mTempFile.setError(EACCES);
      return;
   }

   if (mTempFile.create(filename, QString::fromLatin1(".new"), mode))
   {
      mFileName = filename; // Set filename upon success

      // if we're overwriting an existing file, ensure temp file's
      // permissions are the same as existing file so the existing
      // file's permissions are preserved
      struct stat stat_buf;
      if ((stat(QFile::encodeName(filename), &stat_buf)==0) 
          && (stat_buf.st_uid == getuid())
          && (stat_buf.st_gid == getgid()))
      {
         fchmod(mTempFile.handle() , stat_buf.st_mode);
      }
   }
   return;
}

KSaveFile::~KSaveFile()
{
   close();
}

QString
KSaveFile::name() const
{
   return mFileName;
}

void
KSaveFile::abort()
{
   mTempFile.unlink();
   mTempFile.close();
}

bool
KSaveFile::close()
{
   if (mTempFile.name().isEmpty())
      return false; // Save was aborted already
   if (mTempFile.close())
   {
      QDir dir;
      bool result = dir.rename( mTempFile.name(), mFileName);
      if ( result )
      {
         return true; // Success!
      }
      mTempFile.setError(errno);
   }

   // Something went wrong, make sure to delete the interim file.
   mTempFile.unlink();
   return false;
}

static int
write_all(int fd, const char *buf, size_t len)
{
   while (len > 0)
   {
      int written = write(fd, buf, len);
      if (written < 0)
      {
          if (errno == EINTR)
             continue;
          return -1;
      }
      buf += written;
      len -= written;
   }
   return 0;
}

bool KSaveFile::backupFile( const QString& qFilename, const QString& backupDir,
                            const QString& backupExtension)
{
   QCString cFilename = QFile::encodeName(qFilename);
   const char *filename = cFilename.data();

   int fd = open( filename, O_RDONLY);
   if (fd < 0)
      return false;

   struct stat buff;
   if ( fstat( fd, &buff) < 0 )
   {
      ::close( fd );
      return false;
   }

   QCString cBackup;
   if ( backupDir.isEmpty() )
       cBackup = cFilename;
   else
   {
       QCString nameOnly;
       int slash = cFilename.findRev('/');
       if (slash < 0)
	   nameOnly = cFilename;
       else
	   nameOnly = cFilename.mid(slash + 1);
       cBackup = QFile::encodeName(backupDir);
       if ( backupDir[backupDir.length()-1] != '/' )
           cBackup += '/';
       cBackup += nameOnly;
   }
   cBackup += QFile::encodeName(backupExtension);
   const char *backup = cBackup.data();
   int permissions = buff.st_mode & 07777;

   if ( stat( backup, &buff) == 0)
   {
      if ( unlink( backup ) != 0 )
      {
         ::close(fd);
         return false;
      }
   }

   mode_t old_umask = umask(0);
   int fd2 = open( backup, O_WRONLY | O_CREAT | O_EXCL, permissions | S_IWUSR);
   umask(old_umask);

   if ( fd2 < 0 )
   {
      ::close(fd);
      return false;
   }

    char buffer[ 32*1024 ];

    while( 1 )
    {
       int n = ::read( fd, buffer, 32*1024 );
       if (n == -1)
       {
          if (errno == EINTR)
              continue;
          ::close(fd);
          ::close(fd2);
          return false;
       }
       if (n == 0)
          break; // Finished

       if (write_all( fd2, buffer, n))
       {
          ::close(fd);
          ::close(fd2);
          return false;
       }
    }

    ::close( fd );

    if (::close(fd2))
        return false;
    return true;
}
