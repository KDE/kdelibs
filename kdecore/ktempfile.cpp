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

#include "config.h"

#include <sys/types.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_TEST
#include <test.h>
#endif

#include <stdlib.h>

#include <qdatetime.h>
#include "kapp.h"
#include "ktempfile.h"


KTempFile::KTempFile(QString filePrefix, QString fileExtension, int mode)
{
   bAutoDelete = false;
   mFd = -1;
   mStream = 0;
   mError = 0;
   bOpen = false;
   int maxTries = 3;
   if (fileExtension.isEmpty())
      fileExtension = ".tmp";
   if (filePrefix.isEmpty())
      filePrefix = "/tmp/kde";


   srand( QTime::currentTime().msecsTo(QTime()));
   int tries = 0;
   do {
      tries++;
      mTmpName = filePrefix+QString(".%1").arg(rand())+fileExtension;

      if (checkAccess(mTmpName, W_OK))
      {
         mFd = open(mTmpName.ascii(), O_RDWR|O_CREAT|O_EXCL, mode);
         if ((mFd <= 0) && (tries >= maxTries))
         {
            mError = errno;
            mTmpName = QString::null;
            return;
         }
      }
      else 
      {
         if (tries >= maxTries)
         {
            mError = EACCES;
            mTmpName = QString::null;
            return;
         }
      }
   }
   while( mFd <= 0);

   // Success!
   bOpen = true;

   // Set uid/gid (neccesary for SUID programs)
   chown(mTmpName.ascii(), getuid(), getgid());
   return;
}

KTempFile::~KTempFile()
{
   close();
   if (bAutoDelete)
      unlink();
}

int
KTempFile::status()
{
   return mError;
}

QString 
KTempFile::name()
{
   return mTmpName;
}

int 
KTempFile::handle()
{
   return mFd;
}

FILE *
KTempFile::fstream()
{
   if (mStream) return mStream;
   if (mFd < 0) return 0;

   // Create a stream
   mStream = fdopen(mFd, "r+");
   if (!mStream)
     mError = errno;

   return mStream;
}

void
KTempFile::unlink()
{
   if (!mTmpName.isEmpty())
      ::unlink( mTmpName.ascii());
}

bool
KTempFile::close()
{
   int result = 0;
   if (mStream)
   {
      result = fclose(mStream);
      mStream = 0;
      mFd = -1;
   }
   if (mFd >= 0)
   {
      result = ::close(mFd);
      mFd = -1;
   }

   if (result != 0)
      mError = errno;

   bOpen = false;
   return (mError == 0);
}

