/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2003 Joseph Wenninger <jowenn@kde.org>
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

#include "ktempdir.h"

#include <config.h>

#include <sys/types.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#ifdef HAVE_TEST
#include <test.h>
#endif
#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#include <QtCore/QDir>

#include "kglobal.h"
#include "krandom.h"
#include "kcomponentdata.h"
#include "kstandarddirs.h"
#include <kdebug.h>
#include "kde_file.h"

#ifdef Q_WS_WIN
#include <QtCore/QVarLengthArray>
#include <windows.h>
#include <shellapi.h>
extern QString mkdtemp_QString (const QString &_template);
#endif

#ifdef _WIN32_WCE
#include <shellapi.h>
#endif

class KTempDir::Private
{
public:
    int error;
    QString tmpName;
    bool exists;
    bool autoRemove;

    Private()
    {
        autoRemove = true;
        exists = false;
        error=0;
    }
};

KTempDir::KTempDir(const QString &directoryPrefix, int mode) : d(new Private)
{
    (void) create( directoryPrefix.isEmpty() ? KStandardDirs::locateLocal("tmp", KGlobal::mainComponent().componentName()) : directoryPrefix , mode);
}

bool KTempDir::create(const QString &directoryPrefix, int mode)
{
   (void) KRandom::random();

#ifdef Q_WS_WIN
   const QString nme = directoryPrefix + "XXXXXX";
   const QString realName = mkdtemp_QString(nme);
   if(realName.isEmpty())
   {
       kWarning(180) << "KTempDir: Error trying to create " << nme
		      << ": " << ::strerror(errno) << endl;
       d->error = errno;
       d->tmpName.clear();
       return false;
   }

   // got a return value != 0
   d->tmpName = realName + '/';
   kDebug(180) << "KTempDir: Temporary directory created :" << d->tmpName
	        << endl;
   mode_t umsk = KGlobal::umask();
   KDE::chmod(nme, mode&(~umsk));

   // Success!
   d->exists = true;
#else
   QByteArray nme = QFile::encodeName(directoryPrefix) + "XXXXXX";
   char *realName;
   if((realName=mkdtemp(nme.data())) == 0)
   {
       // Recreate it for the warning, mkdtemps emptied it
       nme = QFile::encodeName(directoryPrefix) + "XXXXXX";
       kWarning(180) << "KTempDir: Error trying to create " << nme.data()
		      << ": " << ::strerror(errno) << endl;
       d->error = errno;
       d->tmpName.clear();
       return false;
   }

   // got a return value != 0
   QByteArray realNameStr(realName);
   d->tmpName = QFile::decodeName(realNameStr)+'/';
   kDebug(180) << "KTempDir: Temporary directory created :" << d->tmpName
	        << endl;

   mode_t umsk = KGlobal::umask();
   if(chmod(nme, mode&(~umsk)) < 0) {
       kWarning(180) << "KTempDir: Unable to change permissions on" << d->tmpName
                     << ":" << ::strerror(errno);
       d->error = errno;
       d->tmpName.clear();
       (void) ::rmdir(realName); // Cleanup created directory
       return false;
   }

   // Success!
   d->exists = true;

   // Set uid/gid (necessary for SUID programs)
   if(chown(nme, getuid(), getgid()) < 0) {
       // Just warn, but don't failover yet
       kWarning(180) << "KTempDir: Unable to change owner on" << d->tmpName
                     << ":" << ::strerror(errno);
   }

#endif
   return true;
}

KTempDir::~KTempDir()
{
    if (d->autoRemove) {
        unlink();
    }

    delete d;
}

int KTempDir::status() const
{
    return d->error;
}

QString KTempDir::name() const
{
    return d->tmpName;
}

bool KTempDir::exists() const
{
    return d->exists;
}

void KTempDir::setAutoRemove(bool autoRemove)
{
    d->autoRemove = autoRemove;
}

bool KTempDir::autoRemove() const
{
    return d->autoRemove;
}

void KTempDir::unlink()
{
    if (!d->exists) return;
    if (KTempDir::removeDir(d->tmpName))
        d->error=0;
    else
        d->error=errno;
    d->exists=false;
}

#ifndef Q_WS_WIN
// Auxiliary recursive function for removeDirs
static bool rmtree(const QByteArray& name)
{
    //kDebug(180) << "Checking directory for remove" << name;
    KDE_struct_stat st;
    if ( KDE_lstat( name.data(), &st ) == -1 ) // Do not dereference symlink!
        return false;
    if ( S_ISDIR( st.st_mode ) )
    {
        // This is a directory, so process it
        //kDebug(180) << "File" << name << "is DIRECTORY!";
        KDE_struct_dirent* ep;
        DIR* dp = ::opendir( name.data() );
        if ( !dp )
            return false;
        while ( ( ep = KDE_readdir( dp ) ) )
        {
            //kDebug(180) << "CHECKING" << name << "/" << ep->d_name;
            if ( !qstrcmp( ep->d_name, "." ) || !qstrcmp( ep->d_name, ".." ) )
                continue;
            QByteArray newName( name );
            newName += '/';
            newName += ep->d_name;
            /*
             * Be defensive and close the directory.
             *
             * Potential problems:
             * - opendir/readdir/closedir is not re-entrant
             * - unlink and rmdir invalidates a opendir/readdir/closedir
             * - limited number of file descriptors for opendir/readdir/closedir
             */
            if ( ::closedir( dp ) ) {
                kDebug(180) << "Error closing" << name;
                return false;
            }
            // Recurse!
            //kDebug(180) << "RECURSE: " << newName;
            if ( ! rmtree( newName ) )
                return false;
            // We have to re-open the directory before continuing
            dp = ::opendir( name.data() );
            if ( !dp )
                return false;
        }
        if ( ::closedir( dp ) ) {
            kDebug(180) << "Error closing" << name;
            return false;
        }
        //kDebug(180) << "RMDIR dir " << name;
        return ! ::rmdir( name );
    }
    else
    {
        // This is a non-directory file, so remove it
        //kDebug(180) << "KTempDir: unlinking file" << name;
        return ! ::unlink( name );
    }
}
#endif

bool KTempDir::removeDir( const QString& path )
{
    //kDebug(180) << path;
    if ( !QFile::exists( path ) )
        return true; // The goal is that there is no directory

#ifdef Q_WS_WIN
    QVarLengthArray<WCHAR, MAX_PATH> name;
    name.resize( path.length() + 2 ); // double null terminated!
    memcpy( name.data(), path.utf16(), path.length() * sizeof(WCHAR) );
    name[path.length()     ] = 0;
    name[path.length() + 1 ] = 0;
    if(path.endsWith('/') || path.endsWith('\\'))
      name[path.length() - 1 ] = 0;
    SHFILEOPSTRUCTW fileOp;
    memset(&fileOp, 0, sizeof(SHFILEOPSTRUCTW) );
    fileOp.wFunc = FO_DELETE;
    fileOp.pFrom = (LPCWSTR)name.constData();
    fileOp.fFlags = FOF_NOCONFIRMATION | FOF_SILENT;
#ifdef _WIN32_WCE
    // FOF_NOERRORUI is not defined in wince
#else
    fileOp.fFlags |= FOF_NOERRORUI;
#endif
    errno = SHFileOperationW( &fileOp );
    return (errno == 0);
#else
    const QByteArray cstr( QFile::encodeName( path ) );
    return rmtree( cstr );
#endif
}

