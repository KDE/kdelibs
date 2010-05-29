/*
 *
 *  This file is part of the KDE libraries
 *  Copyright (c) 2003 Joseph Wenninger <jowenn@kde.org>
 *
 * $Id: ktempdir.cpp 532292 2006-04-21 15:19:53Z savernik $
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

#include <config.h>

#include <sys/types.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

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
#include <qdir.h>

#include "kglobal.h"
#include "kapplication.h"
#include "kinstance.h"
#include "ktempdir.h"
#include "kstandarddirs.h"
#include "kprocess.h"
#include <kdebug.h>
#include "kde_file.h"

KTempDir::KTempDir(QString directoryPrefix, int mode)
{
   bAutoDelete = false;
   bExisting = false;
   mError=0;
   if (directoryPrefix.isEmpty())
   {
      directoryPrefix = locateLocal("tmp", KGlobal::instance()->instanceName());
   }
   (void) create(directoryPrefix , mode);
}

bool
KTempDir::create(const QString &directoryPrefix, int mode)
{
   // make sure the random seed is randomized
   (void) KApplication::random();

   QCString nme = QFile::encodeName(directoryPrefix) + "XXXXXX";
   char *realName;
   if((realName=mkdtemp(nme.data())) == 0)
   {
       // Recreate it for the warning, mkdtemps emptied it
       QCString nme = QFile::encodeName(directoryPrefix) + "XXXXXX";
       qWarning("KTempDir: Error trying to create %s: %s", nme.data(), strerror(errno));
       mError = errno;
       mTmpName = QString::null;
       return false;
   }

   // got a return value != 0
   QCString realNameStr(realName);
   mTmpName = QFile::decodeName(realNameStr)+"/";
   kdDebug(180) << "KTempDir: Temporary directory created :" << mTmpName << endl;
   mode_t tmp = 0;
   mode_t umsk = umask(tmp);
   umask(umsk);
   chmod(nme, mode&(~umsk));

   // Success!
   bExisting = true;

   // Set uid/gid (necessary for SUID programs)
   chown(nme, getuid(), getgid());
   return true;
}

KTempDir::~KTempDir()
{
   if (bAutoDelete)
      unlink();

// KTempDirPrivate doesn't exist, so it can't be deleted
//   delete d;
}

int
KTempDir::status() const
{
   return mError;
}

QString
KTempDir::name() const
{
   return mTmpName;
}

bool
KTempDir::existing() const
{
   return bExisting;
}

QDir *
KTempDir::qDir()
{
   if (bExisting) return new QDir(mTmpName);
   return 0;
}

void
KTempDir::unlink()
{
   if (!bExisting) return;
   if (KTempDir::removeDir(mTmpName))
      mError=0;
   else
      mError=errno;
   bExisting=false;
}

// Auxiliary recursive function for removeDirs
static bool
rmtree(const QCString& name)
{
    kdDebug() << "Checking directory for remove " << name << endl;
    KDE_struct_stat st;
    if ( KDE_lstat( name.data(), &st ) == -1 ) // Do not dereference symlink!
        return false;
    if ( S_ISDIR( st.st_mode ) )
    {
        // This is a directory, so process it
        kdDebug() << "File " << name << " is DIRECTORY!" << endl;
        KDE_struct_dirent* ep;
        DIR* dp = ::opendir( name.data() );
        if ( !dp )
            return false;
        while ( ( ep = KDE_readdir( dp ) ) )
        {
            kdDebug() << "CHECKING " << name << "/" << ep->d_name << endl;
            if ( !qstrcmp( ep->d_name, "." ) || !qstrcmp( ep->d_name, ".." ) )
                continue;
            QCString newName( name );
            newName += "/"; // Careful: do not add '/' instead or you get problems with Qt3.
            newName += ep->d_name;
            /*
             * Be defensive and close the directory.
             *
             * Potential problems:
             * - opendir/readdir/closedir is not re-entrant
             * - unlink and rmdir invalidates a opendir/readdir/closedir
             * - limited number of file descriptors for opendir/readdir/closedir
             */
            if ( ::closedir( dp ) )
                return false;
            // Recurse!
            kdDebug() << "RECURSE: " << newName << endl;
            if ( ! rmtree( newName ) )
                return false;
            // We have to re-open the directory before continuing
            dp = ::opendir( name.data() );
            if ( !dp )
                return false;
        }
        if ( ::closedir( dp ) )
            return false;
        kdDebug() << "RMDIR dir " << name << endl;
        return ! ::rmdir( name );
    }
    else
    {
         // This is a non-directory file, so remove it
         kdDebug() << "UNLINKING file " << name << endl;
         return ! ::unlink( name );
    }
}

bool
KTempDir::removeDir(const QString& path)
{
    kdDebug() << k_funcinfo << " " << path << endl;
    if ( !QFile::exists( path ) )
        return true; // The goal is that there is no directory

    const QCString cstr( QFile::encodeName( path ) );
    return rmtree( cstr );
}


