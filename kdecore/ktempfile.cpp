/*
 *
 *  This file is part of the KDE libraries
 *  Copyright (c) 1999 Waldo Bastian <bastian@kde.org>
 *
 * $Id$
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
#include <qfile.h>

#include "kapp.h"
#include "ktempfile.h"


KTempFile::KTempFile(QString filePrefix, QString fileExtension, int mode)
{
   bAutoDelete = false;
   mFd = -1;
   mError = 0;
   bOpen = false;
   int maxTries = 3;
   if (fileExtension.isEmpty())
      fileExtension = ".tmp";
   if (filePrefix.isEmpty())
      filePrefix = "/tmp/kde";


#ifdef __FreeBSD__
   mTmpName = filePrefix+QString("XXXX")+fileExtension;

   // I use a malloc'd buffer because mkstemps will modify it,
   // something that would otherwise be a bad idea with say, a QString
   char *mktmpName = (char *)malloc(mTmpName.length());
   strcpy(mktmpName, mTmpName.ascii());
   mFd = mkstemps(mktmpName, fileExtension.length());

   if (mFd == -1) {
            mError = errno;
            mTmpName = QString::null;
            return;
   }

   // Apply the possibly more restrictive mode
   fchmod(mFd, mode);

   mTmpName = mktmpName;
   free(mktmpName);

#else

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
#endif

   // Success!
   bOpen = true;

   qIODevice.setName(mTmpName);
   if (!qIODevice.open(mFd, IO_ReadWrite)) {
      printf("AHHH\n");
   }
   setDevice(&qIODevice);

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

   if (qIODevice.isOpen())
     qIODevice.close();

   if (mFd >= 0) {
      result = ::close(mFd);
      mFd = -1;
   }

   if (result != 0)
      mError = errno;


   bOpen = false;
   return (mError == 0);
}

