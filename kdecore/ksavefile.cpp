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
#include "ksavefile.h"

KSaveFile::KSaveFile(const QString &filename, int mode)
{
   mFd = -1;
   mStream = 0;
   mError = 0;
   bOpen = false;
   int maxTries = 3;
   if (!checkAccess(filename, W_OK)) 
   {
      mError = EACCES;
      return;
   }
    
   srand( QTime::currentTime().msecsTo(QTime()));
   int tries = 0;
   do {
      tries++;
      mTmpName = filename+QString(".%1.new").arg(rand());
      mFd = open(mTmpName.ascii(), O_WRONLY|O_CREAT|O_EXCL, mode);

      if ((mFd <= 0) && (tries >= maxTries))
      {
         mError = errno;
         return;
      }
   }
   while( mFd <= 0);

   // Success!
   bOpen = true;

   // Set uid/gid (neccesary for SUID programs)
   chown(mTmpName.ascii(), getuid(), getgid());

   mFileName = filename;
   return;
}

KSaveFile::~KSaveFile()
{
   close();
}

int
KSaveFile::status()
{
   return mError;
}

QString 
KSaveFile::name()
{
   return mFileName;
}

int 
KSaveFile::handle()
{
   return mFd;
}

FILE *
KSaveFile::fstream()
{
   if (mStream) return mStream;
   if (mFd < 0) return 0;

   // Create a stream
   mStream = fdopen(mFd, "w");
   if (!mStream)
     mError = errno;

   return mStream;
}

void
KSaveFile::abort()
{
   if (mStream)
   {
      fclose(mStream);
      mStream = 0;
      mFd = -1;
   }
   if (mFd >= 0)
   {
      ::close(mFd);
      mFd = -1;
   }
   if (bOpen)
   {
      unlink( mTmpName.ascii());
      bOpen = false;
   }
}

bool
KSaveFile::close()
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

   if (!bOpen)
      return true;    // File is already closed.

   if ( result == 0 )
   {
      result = rename( mTmpName.ascii(), mFileName.ascii());
      if ( result == 0 )
      {
         bOpen = false;
         return true; // Success!
      }
      mError = errno;
   }
    
   // Something went wrong, make sure to delete the interim file.
   unlink(mTmpName.ascii());
   return false;
}

