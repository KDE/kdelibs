/*
   Copyright (C) 2000-2002 Stephan Kulow <coolo@kde.org>
   Copyright (C) 2000-2002 David Faure <faure@kde.org>
   Copyright (C) 2000-2002 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2006 Allan Sandfeld Jensen <sandfeld@kde.org>
   Copyright (C) 2007 Thiago Macieira <thiago@kde.org>
   Copyright (C) 2007 Christian Ehrlicher <ch.ehrlicher@gmx.de>

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

#include <windows.h>

#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFileInfo>

#include <config.h>

#include <kconfiggroup.h>
#include <kdebug.h>

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

void FileProtocol::listDir( const KUrl& url )
{
    kDebug(7101) << "========= LIST " << url.url() << " =========";

    if (!url.isLocalFile()) {
        KUrl redir(url);
        redir.setProtocol(config()->readEntry("DefaultRemoteProtocol", "smb"));
        redirection(redir);
        kDebug(7101) << "redirecting to " << redir.url();
        finished();
        return;
    }

    QDir dir( url.toLocalFile() );

    if ( !dir.exists() ) {
        kDebug(7101) << "========= ERR_DOES_NOT_EXIST  =========";
        error( KIO::ERR_DOES_NOT_EXIST, url.toLocalFile() );
        return;
    }

    if ( !dir.isReadable() ) {
        kDebug(7101) << "========= ERR_CANNOT_ENTER_DIRECTORY =========";
        error( KIO::ERR_CANNOT_ENTER_DIRECTORY, url.toLocalFile() );
        return;
    }
    QDirIterator it( dir );
    UDSEntry entry;
    while( it.hasNext() ) {
        it.next();
        QFileInfo fileInfo = it.fileInfo();

        entry.insert( KIO::UDSEntry::UDS_NAME, it.fileName() );
        if( fileInfo.isSymLink() ) {
            entry.insert( KIO::UDSEntry::UDS_TARGET_URL, fileInfo.symLinkTarget() );
/* TODO - or not useful on windows?
            if ( details > 1 ) {
                // It is a link pointing to nowhere
                type = S_IFMT - 1;
                access = S_IRWXU | S_IRWXG | S_IRWXO;

                entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, type );
                entry.insert( KIO::UDSEntry::UDS_ACCESS, access );
                entry.insert( KIO::UDSEntry::UDS_SIZE, 0LL );
                goto notype;

            }
*/
        }
        int type = S_IFREG;
        int access = 0;
        if( fileInfo.isDir() )
            type = S_IFDIR;
        else if( fileInfo.isSymLink() )
            type = S_IFLNK;
        if( fileInfo.isReadable() )
            access |= S_IRUSR;
        if( fileInfo.isWritable() )
            access |= S_IWUSR;
        if( fileInfo.isExecutable() )
            access |= S_IXUSR;

        entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, type );
        entry.insert( KIO::UDSEntry::UDS_ACCESS, access );
        entry.insert( KIO::UDSEntry::UDS_SIZE, fileInfo.size() );

        entry.insert( KIO::UDSEntry::UDS_MODIFICATION_TIME, fileInfo.lastModified().toTime_t() );
        entry.insert( KIO::UDSEntry::UDS_USER, fileInfo.owner() );
        entry.insert( KIO::UDSEntry::UDS_GROUP, fileInfo.group() );
        entry.insert( KIO::UDSEntry::UDS_ACCESS_TIME, fileInfo.lastRead().toTime_t() );

        listEntry( entry, false );
        entry.clear();
    }

    listEntry( entry, true ); // ready

    kDebug(7101) << "============= COMPLETED LIST ============";

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

void FileProtocol::del( const KUrl& url, bool isfile )
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

void FileProtocol::chown( const KUrl& url, const QString&, const QString& )
{
    error( KIO::ERR_CANNOT_CHOWN, url.toLocalFile() );
}
