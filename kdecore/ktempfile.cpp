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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include <config.h>

#include <sys/types.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef HAVE_TEST
#include <test.h>
#endif
#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp"
#endif

#include <qdatetime.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qtextstream.h>

#include "kglobal.h"
#include "krandom.h"
#include "kinstance.h"
#include "ktempfile.h"
#include "kstandarddirs.h"
#include "kde_file.h"
#include "kdebug.h"

/* antlarr: KDE 4: make the parameters const QString & */
KTempFile::KTempFile(QString filePrefix, QString fileExtension, int mode):d(0)
{
   bAutoDelete = false;
   mFd = -1;
   mStream = 0;
   mFile = 0;
   mTextStream = 0;
   mDataStream = 0;
   mError = 0;
   bOpen = false;
   if (fileExtension.isEmpty())
      fileExtension = ".tmp";
   if (filePrefix.isEmpty())
   {
      filePrefix = locateLocal("tmp", KGlobal::instance()->instanceName());
   }
   (void) create(filePrefix, fileExtension, mode);
}

KTempFile::KTempFile(bool):d(0)
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
KTempFile::create(const QString &filePrefix, const QString &fileExtension,
		  int mode)
{
   // make sure the random seed is randomized
   (void) KRandom::random();

   QByteArray ext = QFile::encodeName(fileExtension);
   QByteArray nme = QFile::encodeName(filePrefix) + "XXXXXX" + ext;
   if((mFd = mkstemps(nme.data(), ext.length())) < 0)
   {
       // Recreate it for the warning, mkstemps emptied it
       QByteArray nme = QFile::encodeName(filePrefix) + "XXXXXX" + ext;
       kdWarning() << "KTempFile: Error trying to create " << nme << ": " << strerror(errno) << endl;
       mError = errno;
       mTmpName = QString::null;
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

   // Set close on exec
   fcntl(mFd, F_SETFD, FD_CLOEXEC);
   
   return true;
}

KTempFile::~KTempFile()
{
   close();
   if (bAutoDelete)
      unlink();
}

int
KTempFile::status() const
{
   return mError;
}

QString
KTempFile::name() const
{
   return mTmpName;
}

int
KTempFile::handle() const
{
   return mFd;
}

FILE *
KTempFile::fstream()
{
   if (mStream) return mStream;
   if (mFd < 0) return 0;

   // Create a stream
   mStream = KDE_fdopen(mFd, "r+");
   if (!mStream) {
     kdWarning() << "KTempFile: Error trying to open " << mTmpName << ": " << strerror(errno) << endl;
     mError = errno;
   }
   return mStream;
}

QFile *
KTempFile::file()
{
   if (mFile) return mFile;
   if ( !fstream() ) return 0;

   mFile = new QFile();
   mFile->setName( name() );
   mFile->open(QIODevice::ReadWrite, mStream);
   return mFile;
}

QTextStream *
KTempFile::textStream()
{
   if (mTextStream) return mTextStream;
   if ( !file() ) return 0; // Initialize mFile

   mTextStream = new QTextStream( mFile );
   return mTextStream;
}

QDataStream *
KTempFile::dataStream()
{
   if (mDataStream) return mDataStream;
   if ( !file() ) return 0;  // Initialize mFile

   mDataStream = new QDataStream( mFile );
   return mDataStream;
}

void
KTempFile::unlink()
{
   if (!mTmpName.isEmpty())
      QFile::remove( mTmpName );
   mTmpName = QString::null;
}

#if defined(_POSIX_SYNCHRONIZED_IO) && _POSIX_SYNCHRONIZED_IO > 0
#define FDATASYNC fdatasync
#else
#define FDATASYNC fsync
#endif

bool
KTempFile::sync()
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
         kdWarning() << "KTempFile: Error trying to flush " << mTmpName << ": " << strerror(errno) << endl;
         mError = errno;
      }
   }

   if (mFd >= 0)
   {
      result = FDATASYNC(mFd);
      if (result)
      {
         kdWarning() << "KTempFile: Error trying to sync " << mTmpName << ": " << strerror(errno) << endl;
         mError = errno;
      }
   }

   return (mError == 0);
}

#undef FDATASYNC

bool
KTempFile::close()
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
         kdWarning() << "KTempFile: Error trying to close " << mTmpName << ": " << strerror(errno) << endl;
         mError = errno;
      }
   }


   if (mFd >= 0)
   {
      result = ::close(mFd);
      mFd = -1;
      if (result != 0) {
         kdWarning() << "KTempFile: Error trying to close " << mTmpName << ": " << strerror(errno) << endl;
         mError = errno;
      }
   }

   bOpen = false;
   return (mError == 0);
}

