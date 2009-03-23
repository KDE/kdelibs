/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 1999 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "k3tempfile.h"

#include <config.h>

#include <sys/types.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp"
#endif

#include <QtCore/QCharRef>
#include <QtCore/QDate>
#include <QtCore/QFile>
#include <QtCore/QDataStream>
#include <QtCore/QTextIStream>

#include "kglobal.h"
#include "krandom.h"
#include "kcomponentdata.h"
#include "kstandarddirs.h"
#include "kde_file.h"
#include "kdebug.h"

class K3TempFile::Private
{
public:
   int _Error;
#define mError d->_Error
   QString _TmpName;
#define mTmpName d->_TmpName
   int _Fd;
#define mFd d->_Fd
   FILE *_Stream;
#define mStream d->_Stream
   QFile *_File;
#define mFile d->_File
   QTextStream *_TextStream;
#define mTextStream d->_TextStream
   QDataStream *_DataStream;
#define mDataStream d->_DataStream
   bool _Open;
#define bOpen d->_Open
   bool _AutoDelete;
#define bAutoDelete d->_AutoDelete
};

K3TempFile::K3TempFile(const QString& filePrefix,
                     const QString& fileExtension, int mode)
 : d(new Private)
{
   bAutoDelete = false;
   mFd = -1;
   mStream = 0;
   mFile = 0;
   mTextStream = 0;
   mDataStream = 0;
   mError = 0;
   bOpen = false;
   QString extension = fileExtension;
   QString prefix = filePrefix;
   if (extension.isEmpty())
      extension = QLatin1String(".tmp");
   if (prefix.isEmpty())
   {
      prefix = KStandardDirs::locateLocal("tmp", KGlobal::mainComponent().componentName());
   }
   (void) create(prefix, extension, mode);
}

K3TempFile::K3TempFile(bool):d(new Private)
{
   bAutoDelete = false;
   mFd = -1;
   mStream = 0;
   mFile = 0;
   mTextStream = 0;
   mDataStream = 0;
   mError = 0;
   bOpen = false;
}

bool
K3TempFile::create(const QString &filePrefix, const QString &fileExtension,
		  int mode)
{
   // make sure the random seed is randomized
   (void) KRandom::random();

   QByteArray ext = QFile::encodeName(fileExtension);
   QByteArray nme = QFile::encodeName(filePrefix) + "XXXXXX" + ext;
   if((mFd = mkstemps(nme.data(), ext.length())) < 0)
   {
       // Recreate it for the warning, mkstemps emptied it
       nme = QFile::encodeName(filePrefix) + "XXXXXX" + ext;
       kWarning() << "K3TempFile: Error trying to create " << nme << ": " << strerror(errno);
       mError = errno;
       mTmpName.clear();
       return false;
   }

   // got a file descriptor. nme contains the name
   mTmpName = QFile::decodeName(nme);
   mode_t tmp = 0;
   mode_t umsk = umask(tmp);
   umask(umsk);
   fchmod(mFd, mode&(~umsk));

   // Success!
   bOpen = true;

   // Set uid/gid (necessary for SUID programs)
   fchown(mFd, getuid(), getgid());

#ifndef Q_WS_WIN
   // Set close on exec
   fcntl(mFd, F_SETFD, FD_CLOEXEC);
#endif
   return true;
}

K3TempFile::~K3TempFile()
{
   close();
   if (bAutoDelete)
      unlink();
   delete d;
}

int
K3TempFile::status() const
{
   return mError;
}

QString
K3TempFile::name() const
{
   return mTmpName;
}

int
K3TempFile::handle() const
{
   return mFd;
}

FILE *
K3TempFile::fstream()
{
   if (mStream) return mStream;
   if (mFd < 0) return 0;

   // Create a stream
   mStream = KDE_fdopen(mFd, "r+");
   if (!mStream) {
     kWarning() << "K3TempFile: Error trying to open " << mTmpName << ": " << strerror(errno);
     mError = errno;
   }
   return mStream;
}

QFile *
K3TempFile::file()
{
   if (mFile) return mFile;
   if ( !fstream() ) return 0;

   mFile = new QFile();
   mFile->setFileName( name() );
   mFile->open(mStream, QIODevice::ReadWrite);
   return mFile;
}

QTextStream *
K3TempFile::textStream()
{
   if (mTextStream) return mTextStream;
   if ( !file() ) return 0; // Initialize mFile

   mTextStream = new QTextStream( mFile );
   return mTextStream;
}

QDataStream *
K3TempFile::dataStream()
{
   if (mDataStream) return mDataStream;
   if ( !file() ) return 0;  // Initialize mFile

   mDataStream = new QDataStream( mFile );
   return mDataStream;
}

void
K3TempFile::unlink()
{
   if (!mTmpName.isEmpty())
      QFile::remove( mTmpName );
   mTmpName.clear();
}

#if defined(_POSIX_SYNCHRONIZED_IO) && _POSIX_SYNCHRONIZED_IO > 0
#define FDATASYNC fdatasync
#else
#define FDATASYNC fsync
#endif

bool
K3TempFile::sync()
{
   int result = 0;

   if (mStream)
   {
      do {
         result = fflush(mStream); // We need to flush first otherwise fsync may not have our data
      }
      while ((result == -1) && (errno == EINTR));

      if (result)
      {
         kWarning() << "K3TempFile: Error trying to flush " << mTmpName << ": " << strerror(errno);
         mError = errno;
      }
   }

   if (mFd >= 0)
   {
      if( qgetenv( "KDE_EXTRA_FSYNC" ) == "1" )
      {
         result = FDATASYNC(mFd);
         if (result)
         {
            kWarning() << "K3TempFile: Error trying to sync " << mTmpName << ": " << strerror(errno);
            mError = errno;
         }
      }
   }

   return (mError == 0);
}

#undef FDATASYNC

bool
K3TempFile::close()
{
   int result = 0;
   delete mTextStream; mTextStream = 0;
   delete mDataStream; mDataStream = 0;
   delete mFile; mFile = 0;

   if (mStream)
   {
      result = ferror(mStream);
      if (result)
         mError = ENOSPC; // Assume disk full.

      result = fclose(mStream);
      mStream = 0;
      mFd = -1;
      if (result != 0) {
         kWarning() << "K3TempFile: Error trying to close " << mTmpName << ": " << strerror(errno);
         mError = errno;
      }
   }


   if (mFd >= 0)
   {
      result = ::close(mFd);
      mFd = -1;
      if (result != 0) {
         kWarning() << "K3TempFile: Error trying to close " << mTmpName << ": " << strerror(errno);
         mError = errno;
      }
   }

   bOpen = false;
   return (mError == 0);
}

void
K3TempFile::setAutoDelete(bool autoDelete)
{
   bAutoDelete = autoDelete;
}

void
K3TempFile::setError(int error)
{
   mError = error;
}

bool
K3TempFile::isOpen() const
{
   return bOpen;
}

#undef mError
#undef mTmpName
#undef mFd
#undef mStream
#undef mFile
#undef mTextStream
#undef mDataStream
#undef bOpen
#undef bAutoDelete
