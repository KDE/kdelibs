/*
   Copyright (C) 2000-2002 Stephan Kulow <coolo@kde.org>
   Copyright (C) 2000-2002 David Faure <faure@kde.org>
   Copyright (C) 2000-2002 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2006 Allan Sandfeld Jensen <sandfeld@kde.org>
   Copyright (C) 2007 Thiago Macieira <thiago@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#define _WIN32_WINNT 0x0500

#include "file.h"

#include <config.h>
#include <config-acl.h>

#include <QtCore/QBool> //for Q_OS_XXX
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <utime.h>
#include <unistd.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <QtCore/QByteRef>
#include <QtCore/QDate>
#include <QtCore/QVarLengthArray>
#include <QtCore/QCoreApplication>
#include <QtCore/QRegExp>
#include <QtCore/QFile>
#ifdef Q_WS_WIN
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <windows.h>
#endif

#include <kdebug.h>
#include <kurl.h>
#include <kcomponentdata.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <ktemporaryfile.h>
#include <klocale.h>
#include <limits.h>
#include <kshell.h>
#include <kmountpoint.h>
#include <kstandarddirs.h>

#include <kio/ioslave_defaults.h>
#include <kde_file.h>
#include <kglobal.h>
#include <kmimetype.h>

using namespace KIO;

static DWORD CALLBACK CopyProgressRoutine(
    LARGE_INTEGER TotalFileSize,
    LARGE_INTEGER TotalBytesTransferred,
    LARGE_INTEGER StreamSize,
    LARGE_INTEGER StreamBytesTransferred,
    DWORD dwStreamNumber,
    DWORD dwCallbackReason,
    HANDLE hSourceFile,
    HANDLE hDestinationFile,
    LPVOID lpData
) {
    FileProtocol *f = reinterpret_cast<FileProtocol*>(lpData);
    f->processedSize( TotalBytesTransferred.QuadPart );
    return PROGRESS_CONTINUE;
}

void FileProtocol::copy( const KUrl &src, const KUrl &dest,
                         int _mode, JobFlags _flags )
{
    kDebug(7101) << "copy(): " << src << " -> " << dest << ", mode=" << _mode;

    QFileInfo _src(src.toLocalFile());
    QFileInfo _dest(dest.toLocalFile());
    DWORD dwFlags = COPY_FILE_FAIL_IF_EXISTS;

    if( _src == _dest ) {
	    error( KIO::ERR_IDENTICAL_FILES, _dest.filePath() );
	    return;
    }

    if( !_src.exists() ) {
        error( KIO::ERR_DOES_NOT_EXIST, _src.filePath() );
        return;
    }

    if ( _src.isDir() ) {
        error( KIO::ERR_IS_DIRECTORY, _src.filePath() );
        return;
    }

    if( _dest.exists() ) {
        if( _dest.isDir() ) {
           error( KIO::ERR_DIR_ALREADY_EXIST, _dest.filePath() );
           return;
        }

        if (!(_flags & KIO::Overwrite))
        {
           error( KIO::ERR_FILE_ALREADY_EXIST, _dest.filePath() );
           return;
        }

        dwFlags = 0;
    }

    if ( CopyFileExW( ( LPCWSTR ) _src.filePath().utf16(),
                      ( LPCWSTR ) _dest.filePath().utf16(),
                      CopyProgressRoutine,
                      ( LPVOID ) this,
                      FALSE,
                      dwFlags) == 0 )
    {
        DWORD dwLastErr = GetLastError();
        if ( dwLastErr == ERROR_FILE_NOT_FOUND )
            error( KIO::ERR_DOES_NOT_EXIST, _src.filePath() );
        else if ( dwLastErr == ERROR_ACCESS_DENIED )
            error( KIO::ERR_ACCESS_DENIED, _dest.filePath() );
        else {
            error( KIO::ERR_CANNOT_RENAME, _src.filePath() );
            kDebug( 7101 ) <<  "Copying file "
                           << _src.filePath()
                           << " failed ("
                           << dwLastErr << ")";
        }
        return;
    }

    finished();
}

void FileProtocol::rename( const KUrl &src, const KUrl &dest,
                           KIO::JobFlags _flags )
{
    kDebug(7101) << "rename(): " << src << " -> " << dest;

    QFileInfo _src(src.toLocalFile());
    QFileInfo _dest(dest.toLocalFile());
    DWORD dwFlags = 0;

    if( _src == _dest ) {
	    error( KIO::ERR_IDENTICAL_FILES, _dest.filePath() );
	    return;
    }

    if( !_src.exists() ) {
        error( KIO::ERR_DOES_NOT_EXIST, _src.filePath() );
        return;
    }

    if( _dest.exists() ) {
        if( _dest.isDir() ) {
           error( KIO::ERR_DIR_ALREADY_EXIST, _dest.filePath() );
           return;
        }

        if (!(_flags & KIO::Overwrite))
        {
           error( KIO::ERR_FILE_ALREADY_EXIST, _dest.filePath() );
           return;
        }
        
        dwFlags = MOVEFILE_REPLACE_EXISTING;
    }

    if ( MoveFileExW( ( LPCWSTR ) _src.filePath().utf16(),
                      ( LPCWSTR ) _dest.filePath().utf16(), dwFlags) == 0 )
    {
        DWORD dwLastErr = GetLastError();
        if ( dwLastErr == ERROR_FILE_NOT_FOUND )
            error( KIO::ERR_DOES_NOT_EXIST, _src.filePath() );
        else if ( dwLastErr == ERROR_ACCESS_DENIED )
            error( KIO::ERR_ACCESS_DENIED, _dest.filePath() );
        else {
            error( KIO::ERR_CANNOT_RENAME, _src.filePath() );
            kDebug( 7101 ) <<  "Renaming file "
                           << _src.filePath()
                           << " failed ("
                           << dwLastErr << ")";
        }
        return;
    }

    finished();
}

void FileProtocol::symlink( const QString &target, const KUrl &dest, KIO::JobFlags flags )
{
    // no symlink on windows for now
    // vista provides a CreateSymbolicLink() function. for now use ::copy
    FileProtocol::copy( target, dest, 0, flags );
}

void FileProtocol::del( const KUrl& url, bool isfile)
{
    QString _path( url.toLocalFile() );
    /*****
     * Delete files
     *****/

    if (isfile) {
        kDebug( 7101 ) << "Deleting file " << _path;
        
        if( DeleteFileW( ( LPCWSTR ) _path.utf16() ) == 0 ) {
            DWORD dwLastErr = GetLastError();
            if ( dwLastErr == ERROR_PATH_NOT_FOUND )
                error( KIO::ERR_DOES_NOT_EXIST, _path );
            else if( dwLastErr == ERROR_ACCESS_DENIED )
                error( KIO::ERR_ACCESS_DENIED, _path );
            else {
                error( KIO::ERR_CANNOT_DELETE, _path );
                kDebug( 7101 ) <<  "Deleting file "
                               << _path
                               << " failed ("
                               << dwLastErr << ")";
            }
        }
    } else {
        kDebug( 7101 ) << "Deleting directory " << url.url();
        if( RemoveDirectoryW( ( LPCWSTR ) _path.utf16() ) == 0 ) {
            DWORD dwLastErr = GetLastError();
            if ( dwLastErr == ERROR_FILE_NOT_FOUND )
                error( KIO::ERR_DOES_NOT_EXIST, _path );
            else if( dwLastErr == ERROR_ACCESS_DENIED )
                error( KIO::ERR_ACCESS_DENIED, _path );
            else {
                error( KIO::ERR_CANNOT_DELETE, _path );
                kDebug( 7101 ) <<  "Deleting directory "
                               << _path
                               << " failed ("
                               << dwLastErr << ")";
            }
        }
    }
    finished();
}
