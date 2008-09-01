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

//sendfile has different semantics in different platforms
#if defined HAVE_SENDFILE && defined Q_OS_LINUX
#define USE_SENDFILE 1
#endif

#ifdef USE_SENDFILE
#include <sys/sendfile.h>
#endif

#ifdef HAVE_POSIX_ACL
#include <sys/acl.h>
#include <acl/libacl.h>
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

#ifdef HAVE_VOLMGT
#include <volmgt.h>
#include <sys/mnttab.h>
#endif

#include <kio/ioslave_defaults.h>
#include <kde_file.h>
#include <kglobal.h>
#include <kmimetype.h>

using namespace KIO;

#define MAX_IPC_SIZE (1024*32)

static QString testLogFile( const QByteArray&_filename );
#ifdef HAVE_POSIX_ACL
static bool isExtendedACL(  acl_t p_acl );
static void appendACLAtoms( const QByteArray & path, UDSEntry& entry,
                            mode_t type, bool withACL );
#endif

extern "C" int KDE_EXPORT kdemain( int argc, char **argv )
{
  QCoreApplication app( argc, argv ); // needed for QSocketNotifier
  KComponentData componentData( "kio_file", "kdelibs4" );
  ( void ) KGlobal::locale();

  kDebug(7101) << "Starting " << getpid();

  if (argc != 4)
  {
     fprintf(stderr, "Usage: kio_file protocol domain-socket1 domain-socket2\n");
     exit(-1);
  }

  FileProtocol slave(argv[2], argv[3]);
  slave.dispatchLoop();

  kDebug(7101) << "Done";
  return 0;
}

FileProtocol::FileProtocol( const QByteArray &pool, const QByteArray &app )
    : SlaveBase( "file", pool, app ), openFd(-1)
{
}

FileProtocol::~FileProtocol()
{
}

int FileProtocol::setACL( const char *path, mode_t perm, bool directoryDefault )
{
    int ret = 0;
#ifdef HAVE_POSIX_ACL

    const QString ACLString = metaData( "ACL_STRING" );
    const QString defaultACLString = metaData( "DEFAULT_ACL_STRING" );
    // Empty strings mean leave as is
    if ( !ACLString.isEmpty() ) {
        acl_t acl = 0;
        if ( ACLString == "ACL_DELETE" ) {
            // user told us to delete the extended ACL, so let's write only
            // the minimal (UNIX permission bits) part
            acl = acl_from_mode( perm );
        }
        acl = acl_from_text( ACLString.toLatin1() );
        if ( acl_valid( acl ) == 0 ) { // let's be safe
            ret = acl_set_file( path, ACL_TYPE_ACCESS, acl );
            ssize_t size = acl_size( acl );
            kDebug(7101) << "Set ACL on: " << path << " to: " << acl_to_text( acl, &size );
        }
        acl_free( acl );
        if ( ret != 0 ) return ret; // better stop trying right away
    }

    if ( directoryDefault && !defaultACLString.isEmpty() ) {
        if ( defaultACLString == "ACL_DELETE" ) {
            // user told us to delete the default ACL, do so
            ret += acl_delete_def_file( path );
        } else {
            acl_t acl = acl_from_text( defaultACLString.toLatin1() );
            if ( acl_valid( acl ) == 0 ) { // let's be safe
                ret += acl_set_file( path, ACL_TYPE_DEFAULT, acl );
                ssize_t size = acl_size( acl );
                kDebug(7101) << "Set Default ACL on: " << path << " to: " << acl_to_text( acl, &size );
            }
            acl_free( acl );
        }
    }
#else
    Q_UNUSED(path);
    Q_UNUSED(perm);
    Q_UNUSED(directoryDefault);
#endif
    return ret;
}

void FileProtocol::chmod( const KUrl& url, int permissions )
{
    QByteArray _path( QFile::encodeName(url.toLocalFile()) );
    /* FIXME: Should be atomic */
    if ( ::chmod( _path.data(), permissions ) == -1 ||
        ( setACL( _path.data(), permissions, false ) == -1 ) ||
        /* if not a directory, cannot set default ACLs */
        ( setACL( _path.data(), permissions, true ) == -1 && errno != ENOTDIR ) ) {

        switch (errno) {
            case EPERM:
            case EACCES:
                error( KIO::ERR_ACCESS_DENIED, _path );
                break;
#if defined(ENOTSUP)
            case ENOTSUP: // from setACL since chmod can't return ENOTSUP
                error( KIO::ERR_UNSUPPORTED_ACTION, i18n( "Setting ACL for %1" ,  url.path() ) );
                break;
#endif
            case ENOSPC:
                error( KIO::ERR_DISK_FULL, _path );
                break;
            default:
        error( KIO::ERR_CANNOT_CHMOD, _path );
        }
    } else
        finished();
}

void FileProtocol::chown( const KUrl& url, const QString& owner, const QString& group )
{
    QByteArray _path( QFile::encodeName(url.toLocalFile()) );
#ifdef Q_WS_WIN
    error( KIO::ERR_CANNOT_CHOWN, _path );
#else

    uid_t uid;
    gid_t gid;

    // get uid from given owner
    {
        struct passwd *p = ::getpwnam(owner.toAscii());

        if ( ! p ) {
            error( KIO::ERR_SLAVE_DEFINED,
                   i18n( "Could not get user id for given user name %1", owner ) );
            return;
        }

        uid = p->pw_uid;
    }

    // get gid from given group
    {
        struct group *p = ::getgrnam(group.toAscii());

        if ( ! p ) {
            error( KIO::ERR_SLAVE_DEFINED,
                   i18n( "Could not get group id for given group name %1", group ) );
            return;
        }

        gid = p->gr_gid;
    }

    if ( ::chown(_path, uid, gid) == -1 ) {
        switch ( errno ) {
            case EPERM:
            case EACCES:
                error( KIO::ERR_ACCESS_DENIED, _path );
                break;
            case ENOSPC:
                error( KIO::ERR_DISK_FULL, _path );
                break;
            default:
                error( KIO::ERR_CANNOT_CHOWN, _path );
        }
    } else
        finished();
#endif
}

void FileProtocol::setModificationTime( const KUrl& url, const QDateTime& mtime )
{
    const QByteArray path = QFile::encodeName(url.toLocalFile());
    KDE_struct_stat statbuf;
    if (KDE_lstat(path, &statbuf) == 0) {
        struct utimbuf utbuf;
        utbuf.actime = statbuf.st_atime; // access time, unchanged
        utbuf.modtime = mtime.toTime_t(); // modification time
        if (utime(path, &utbuf) != 0) {
            // TODO: errno could be EACCES, EPERM, EROFS
            error(KIO::ERR_CANNOT_SETTIME, path);
        } else {
            finished();
        }
    } else {
        error( KIO::ERR_DOES_NOT_EXIST, path );
    }
}

void FileProtocol::mkdir( const KUrl& url, int permissions )
{
    QByteArray _path( QFile::encodeName(url.toLocalFile()));

    kDebug(7101) << "mkdir(): " << _path << ", permission = " << permissions;

    KDE_struct_stat buff;
    if ( KDE_stat( _path.data(), &buff ) == -1 ) {
        if ( KDE_mkdir( _path.data(), 0777 /*umask will be applied*/ ) != 0 ) {
            if ( errno == EACCES ) {
          error( KIO::ERR_ACCESS_DENIED, _path );
          return;
            } else if ( errno == ENOSPC ) {
          error( KIO::ERR_DISK_FULL, _path );
          return;
            } else {
          error( KIO::ERR_COULD_NOT_MKDIR, _path );
          return;
            }
        } else {
            if ( permissions != -1 )
                chmod( url, permissions );
            else
                finished();
            return;
        }
    }

    if ( S_ISDIR( buff.st_mode ) ) {
        kDebug(7101) << "ERR_DIR_ALREADY_EXIST";
        error( KIO::ERR_DIR_ALREADY_EXIST, _path );
        return;
    }
    error( KIO::ERR_FILE_ALREADY_EXIST, _path );
    return;
}

void FileProtocol::get( const KUrl& url )
{
    if (!url.isLocalFile()) {
        KUrl redir(url);
	redir.setProtocol(config()->readEntry("DefaultRemoteProtocol", "smb"));
	redirection(redir);
	finished();
	return;
    }

    QByteArray _path( QFile::encodeName(url.toLocalFile()));
    KDE_struct_stat buff;
    if ( KDE_stat( _path.data(), &buff ) == -1 ) {
        if ( errno == EACCES )
           error( KIO::ERR_ACCESS_DENIED, _path );
        else
           error( KIO::ERR_DOES_NOT_EXIST, _path );
        return;
    }

    if ( S_ISDIR( buff.st_mode ) ) {
        error( KIO::ERR_IS_DIRECTORY, _path );
        return;
    }
    if ( !S_ISREG( buff.st_mode ) ) {
        error( KIO::ERR_CANNOT_OPEN_FOR_READING, _path );
        return;
    }

    int fd = KDE_open( _path.data(), O_RDONLY);
    if ( fd < 0 ) {
        error( KIO::ERR_CANNOT_OPEN_FOR_READING, _path );
        return;
    }

#ifdef HAVE_FADVISE
    posix_fadvise( fd, 0, 0, POSIX_FADV_SEQUENTIAL);
#endif

    // Determine the mimetype of the file to be retrieved, and emit it.
    // This is mandatory in all slaves (for KRun/BrowserRun to work)
    // In real "remote" slaves, this is usually done using findByNameAndContent
    // after receiving some data. But we don't know how much data the mimemagic rules
    // need, so for local files, better use findByUrl with localUrl=true.
    KMimeType::Ptr mt = KMimeType::findByUrl( url, buff.st_mode, true /* local URL */ );
    emit mimeType( mt->name() );
    // Emit total size AFTER mimetype
    totalSize( buff.st_size );

    KIO::filesize_t processed_size = 0;

    QString resumeOffset = metaData("resume");
    if ( !resumeOffset.isEmpty() )
    {
        bool ok;
        KIO::fileoffset_t offset = resumeOffset.toLongLong(&ok);
        if (ok && (offset > 0) && (offset < buff.st_size))
        {
            if (KDE_lseek(fd, offset, SEEK_SET) == offset)
            {
                canResume ();
                processed_size = offset;
                kDebug( 7101 ) << "Resume offset: " << KIO::number(offset);
            }
        }
    }

    char buffer[ MAX_IPC_SIZE ];
    QByteArray array;

    while( 1 )
    {
       int n = ::read( fd, buffer, MAX_IPC_SIZE );
       if (n == -1)
       {
          if (errno == EINTR)
              continue;
          error( KIO::ERR_COULD_NOT_READ, _path );
          ::close(fd);
          return;
       }
       if (n == 0)
          break; // Finished

       array = QByteArray::fromRawData(buffer, n);
       data( array );
       array.clear();

       processed_size += n;
       processedSize( processed_size );

       //kDebug( 7101 ) << "Processed: " << KIO::number (processed_size);
    }

    data( QByteArray() );

    ::close( fd );

    processedSize( buff.st_size );
    finished();
}

static int
write_all(int fd, const char *buf, size_t len)
{
   while (len > 0)
   {
      ssize_t written = write(fd, buf, len);
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

void FileProtocol::open(const KUrl &url, QIODevice::OpenMode mode)
{
    kDebug(7101) << "FileProtocol::open " << url.url();

    openPath = QFile::encodeName(url.toLocalFile());
    KDE_struct_stat buff;
    if ( KDE_stat( openPath.data(), &buff ) == -1 ) {
        if ( errno == EACCES )
           error( KIO::ERR_ACCESS_DENIED, openPath );
        else
           error( KIO::ERR_DOES_NOT_EXIST, openPath );
        return;
    }

    if ( S_ISDIR( buff.st_mode ) ) {
        error( KIO::ERR_IS_DIRECTORY, openPath );
        return;
    }
    if ( !S_ISREG( buff.st_mode ) ) {
        error( KIO::ERR_CANNOT_OPEN_FOR_READING, openPath );
        return;
    }

    int flags = 0;
    if (mode & QIODevice::ReadOnly) {
        if (mode & QIODevice::WriteOnly) {
            flags = O_RDWR | O_CREAT;
        } else {
            flags = O_RDONLY;
        }
    } else if (mode & QIODevice::WriteOnly) {
        flags = O_WRONLY | O_CREAT;
    }

    if (mode & QIODevice::Append) {
        flags |= O_APPEND;
    } else if (mode & QIODevice::Truncate) {
        flags |= O_TRUNC;
    }

    int fd = KDE_open( openPath.data(), flags);
    if ( fd < 0 ) {
        error( KIO::ERR_CANNOT_OPEN_FOR_READING, openPath );
        return;
    }
    // Determine the mimetype of the file to be retrieved, and emit it.
    // This is mandatory in all slaves (for KRun/BrowserRun to work).
    // If we're not opening the file ReadOnly or ReadWrite, don't attempt to
    // read the file and send the mimetype.
    if (mode & QIODevice::ReadOnly){
        KMimeType::Ptr mt = KMimeType::findByUrl( url, buff.st_mode, true /* local URL */ );
        emit mimeType( mt->name() );
   }

    totalSize( buff.st_size );
    position( 0 );

    emit opened();
    openFd = fd;
}

void FileProtocol::read(KIO::filesize_t bytes)
{
    kDebug( 7101 ) << "File::open -- read";
    Q_ASSERT(openFd != -1);

    QVarLengthArray<char> buffer(bytes);
    while (true) {
        int res;
        do {
            res = ::read(openFd, buffer.data(), bytes);
        } while (res == -1 && errno == EINTR);

        if (res > 0) {
            QByteArray array = array.fromRawData(buffer.data(), res);
            data( array );
            bytes -= res;
        } else {
            // empty array designates eof
            data(QByteArray());
            if (res != 0) {
                error(KIO::ERR_COULD_NOT_READ, openPath);
                close();
            }
            break;
        }
        if (bytes <= 0) break;
    }
}

void FileProtocol::write(const QByteArray &data)
{
    kDebug( 7101 ) << "File::open -- write";
    Q_ASSERT(openFd != -1);

    if (write_all(openFd, data.constData(), data.size())) {
        if (errno == ENOSPC) { // disk full
            error( KIO::ERR_DISK_FULL, openPath );
            close();
        } else {
            kWarning(7101) << "Couldn't write. Error:" << strerror(errno);
            error( KIO::ERR_COULD_NOT_WRITE, openPath );
            close();
        }
    } else {
        written(data.size());
    }
}

void FileProtocol::seek(KIO::filesize_t offset)
{
    kDebug( 7101 ) << "File::open -- seek";
    Q_ASSERT(openFd != -1);

    int res = KDE_lseek(openFd, offset, SEEK_SET);
    if (res != -1) {
        position( offset );
    } else {
        error(KIO::ERR_COULD_NOT_SEEK, openPath );
        close();
    }
}

void FileProtocol::close()
{
    kDebug( 7101 ) << "File::open -- close ";
    Q_ASSERT(openFd != -1);

    ::close( openFd );
    openFd = -1;
    openPath.clear();

    finished();
}

static bool
same_inode(const KDE_struct_stat &src, const KDE_struct_stat &dest)
{
   if (src.st_ino == dest.st_ino &&
       src.st_dev == dest.st_dev)
     return true;

   return false;
}

void FileProtocol::put( const KUrl& url, int _mode, KIO::JobFlags _flags )
{
    QString dest_orig = url.toLocalFile();
    QByteArray _dest_orig( QFile::encodeName(dest_orig));

    kDebug(7101) << "put(): " << dest_orig << ", mode=" << _mode;

    QString dest_part( dest_orig );
    dest_part += QLatin1String(".part");
    QByteArray _dest_part( QFile::encodeName(dest_part));

    KDE_struct_stat buff_orig;
    bool bOrigExists = (KDE_lstat( _dest_orig.data(), &buff_orig ) != -1);
    bool bPartExists = false;
    bool bMarkPartial = config()->readEntry("MarkPartial", true);

    if (bMarkPartial)
    {
        KDE_struct_stat buff_part;
        bPartExists = (KDE_stat( _dest_part.data(), &buff_part ) != -1);

        if (bPartExists && !(_flags & KIO::Resume) && !(_flags & KIO::Overwrite) && buff_part.st_size > 0 && S_ISREG(buff_part.st_mode))
        {
            kDebug(7101) << "FileProtocol::put : calling canResume with "
                          << KIO::number(buff_part.st_size);

            // Maybe we can use this partial file for resuming
            // Tell about the size we have, and the app will tell us
            // if it's ok to resume or not.
            _flags |= canResume( buff_part.st_size ) ? KIO::Resume : KIO::DefaultFlags;

            kDebug(7101) << "FileProtocol::put got answer " << (_flags & KIO::Resume);
        }
    }

    if ( bOrigExists && !(_flags & KIO::Overwrite) && !(_flags & KIO::Resume))
    {
        if (S_ISDIR(buff_orig.st_mode))
            error( KIO::ERR_DIR_ALREADY_EXIST, dest_orig );
        else
            error( KIO::ERR_FILE_ALREADY_EXIST, dest_orig );
        return;
    }

    int result;
    QString dest;
    QByteArray _dest;

    int fd = -1;

    // Loop until we got 0 (end of data)
    do
    {
        QByteArray buffer;
        dataReq(); // Request for data
        result = readData( buffer );

        if (result >= 0)
        {
            if (dest.isEmpty())
            {
                if (bMarkPartial)
                {
                    kDebug(7101) << "Appending .part extension to " << dest_orig;
                    dest = dest_part;
                    if ( bPartExists && !(_flags & KIO::Resume) )
                    {
                        kDebug(7101) << "Deleting partial file " << dest_part;
                        remove( _dest_part.data() );
                        // Catch errors when we try to open the file.
                    }
                }
                else
                {
                    dest = dest_orig;
                    if ( bOrigExists && !(_flags & KIO::Resume) )
                    {
                        kDebug(7101) << "Deleting destination file " << dest_orig;
                        remove( _dest_orig.data() );
                        // Catch errors when we try to open the file.
                    }
                }

                _dest = QFile::encodeName(dest);

                if ( (_flags & KIO::Resume) )
                {
                    fd = KDE_open( _dest.data(), O_RDWR );  // append if resuming
                    KDE_lseek(fd, 0, SEEK_END); // Seek to end
                }
                else
                {
                    // WABA: Make sure that we keep writing permissions ourselves,
                    // otherwise we can be in for a surprise on NFS.
                    mode_t initialMode;
                    if (_mode != -1)
                        initialMode = _mode | S_IWUSR | S_IRUSR;
                    else
                        initialMode = 0666;

                    fd = KDE_open(_dest.data(), O_CREAT | O_TRUNC | O_WRONLY, initialMode);
                }

                if ( fd < 0 )
                {
                    kDebug(7101) << "####################### COULD NOT WRITE " << dest << " _mode=" << _mode;
                    kDebug(7101) << "errno==" << errno << "(" << strerror(errno) << ")";
                    if ( errno == EACCES )
                        error( KIO::ERR_WRITE_ACCESS_DENIED, dest );
                    else
                        error( KIO::ERR_CANNOT_OPEN_FOR_WRITING, dest );
                    return;
                }
            }

            if (write_all( fd, buffer.data(), buffer.size()))
            {
                if ( errno == ENOSPC ) // disk full
                {
                  error( KIO::ERR_DISK_FULL, dest_orig);
                  result = -2; // means: remove dest file
                }
                else
                {
                  kWarning(7101) << "Couldn't write. Error:" << strerror(errno);
                  error( KIO::ERR_COULD_NOT_WRITE, dest_orig);
                  result = -1;
                }
            }
        }
    }
    while ( result > 0 );

    // An error occurred deal with it.
    if (result < 0)
    {
        kDebug(7101) << "Error during 'put'. Aborting.";

        if (fd != -1)
        {
          ::close(fd);

          KDE_struct_stat buff;
          if (bMarkPartial && KDE_stat( _dest.data(), &buff ) == 0)
          {
            int size = config()->readEntry("MinimumKeepSize", DEFAULT_MINIMUM_KEEP_SIZE);
            if (buff.st_size <  size)
              remove(_dest.data());
          }
        }

        ::exit(255);
    }

    if ( fd == -1 ) // we got nothing to write out, so we never opened the file
    {
        finished();
        return;
    }

    if ( ::close(fd) )
    {
        kWarning(7101) << "Error when closing file descriptor:" << strerror(errno);
        error( KIO::ERR_COULD_NOT_WRITE, dest_orig);
        return;
    }

    // after full download rename the file back to original name
    if ( bMarkPartial )
    {
        // If the original URL is a symlink and we were asked to overwrite it,
        // remove the symlink first. This ensures that we do not overwrite the
        // current source if the symlink points to it.
        if( (_flags & KIO::Overwrite) && S_ISLNK( buff_orig.st_mode ) )
          remove( _dest_orig.data() );

#ifdef Q_OS_WIN
        if ( MoveFileExA( _dest.data(),
                          _dest_orig.data(),
                          MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED ) == 0 )
#else
        if ( KDE_rename( _dest.data(), _dest_orig.data() ) )
#endif
        {
            kWarning(7101) << " Couldn't rename " << _dest << " to " << _dest_orig;
            error( KIO::ERR_CANNOT_RENAME_PARTIAL, dest_orig );
            return;
        }
    }

    // set final permissions
    if ( _mode != -1 && !(_flags & KIO::Resume) )
    {
        if (::chmod(_dest_orig.data(), _mode) != 0)
        {
            // couldn't chmod. Eat the error if the filesystem apparently doesn't support it.
            KMountPoint::Ptr mp = KMountPoint::currentMountPoints().findByPath(_dest_orig);
            if (mp && mp->testFileSystemFlag(KMountPoint::SupportsChmod))
                 warning( i18n( "Could not change permissions for\n%1" ,  dest_orig ) );
        }
    }

    // set modification time
    const QString mtimeStr = metaData( "modified" );
    if ( !mtimeStr.isEmpty() ) {
        QDateTime dt = QDateTime::fromString( mtimeStr, Qt::ISODate );
        if ( dt.isValid() ) {
            KDE_struct_stat dest_statbuf;
            if (KDE_stat( _dest_orig.data(), &dest_statbuf ) == 0) {
                struct utimbuf utbuf;
                utbuf.actime = dest_statbuf.st_atime; // access time, unchanged
                utbuf.modtime = dt.toTime_t(); // modification time
                utime( _dest_orig.data(), &utbuf );
            }
        }

    }

    // We have done our job => finish
    finished();
}

#ifndef Q_OS_WIN

void FileProtocol::copy( const KUrl &src, const KUrl &dest,
                         int _mode, JobFlags _flags )
{
    kDebug(7101) << "copy(): " << src << " -> " << dest << ", mode=" << _mode;

    QByteArray _src( QFile::encodeName(src.toLocalFile()));
    QByteArray _dest( QFile::encodeName(dest.toLocalFile()));
    KDE_struct_stat buff_src;
#ifdef HAVE_POSIX_ACL
    acl_t acl;
#endif

    if ( KDE_stat( _src.data(), &buff_src ) == -1 ) {
        if ( errno == EACCES )
           error( KIO::ERR_ACCESS_DENIED, _src );
        else
           error( KIO::ERR_DOES_NOT_EXIST, _src );
	return;
    }

    if ( S_ISDIR( buff_src.st_mode ) ) {
	error( KIO::ERR_IS_DIRECTORY, src.path() );
	return;
    }
    if ( S_ISFIFO( buff_src.st_mode ) || S_ISSOCK ( buff_src.st_mode ) ) {
	error( KIO::ERR_CANNOT_OPEN_FOR_READING, src.path() );
	return;
    }

    KDE_struct_stat buff_dest;
    bool dest_exists = ( KDE_lstat( _dest.data(), &buff_dest ) != -1 );
    if ( dest_exists )
    {
        if (S_ISDIR(buff_dest.st_mode))
        {
           error( KIO::ERR_DIR_ALREADY_EXIST, _dest );
           return;
        }

	if ( same_inode( buff_dest, buff_src) )
	{
	    error( KIO::ERR_IDENTICAL_FILES, _dest );
	    return;
	}

        if (!(_flags & KIO::Overwrite))
        {
           error( KIO::ERR_FILE_ALREADY_EXIST, _dest );
           return;
        }

        // If the destination is a symlink and overwrite is TRUE,
        // remove the symlink first to prevent the scenario where
        // the symlink actually points to current source!
        if ((_flags & KIO::Overwrite) && S_ISLNK(buff_dest.st_mode))
        {
            //kDebug(7101) << "copy(): LINK DESTINATION";
            remove( _dest.data() );
        }
    }

    int src_fd = KDE_open( _src.data(), O_RDONLY);
    if ( src_fd < 0 ) {
	error( KIO::ERR_CANNOT_OPEN_FOR_READING, _src );
	return;
    }

#ifdef HAVE_FADVISE
    posix_fadvise(src_fd,0,0,POSIX_FADV_SEQUENTIAL);
#endif
    // WABA: Make sure that we keep writing permissions ourselves,
    // otherwise we can be in for a surprise on NFS.
    mode_t initialMode;
    if (_mode != -1)
       initialMode = _mode | S_IWUSR;
    else
       initialMode = 0666;

    int dest_fd = KDE_open(_dest.data(), O_CREAT | O_TRUNC | O_WRONLY, initialMode);
    if ( dest_fd < 0 ) {
	kDebug(7101) << "###### COULD NOT WRITE " << dest.url();
        if ( errno == EACCES ) {
            error( KIO::ERR_WRITE_ACCESS_DENIED, _dest );
        } else {
            error( KIO::ERR_CANNOT_OPEN_FOR_WRITING, _dest );
        }
        ::close(src_fd);
        return;
    }

#ifdef HAVE_FADVISE
    posix_fadvise(dest_fd,0,0,POSIX_FADV_SEQUENTIAL);
#endif

#ifdef HAVE_POSIX_ACL
    acl = acl_get_fd(src_fd);
    if ( acl && !isExtendedACL( acl ) ) {
        kDebug(7101) << _dest.data() << " doesn't have extended ACL";
        acl_free( acl );
        acl = NULL;
    }
#endif
    totalSize( buff_src.st_size );

    KIO::filesize_t processed_size = 0;
    char buffer[ MAX_IPC_SIZE ];
    int n;
#ifdef USE_SENDFILE
    bool use_sendfile=buff_src.st_size < 0x7FFFFFFF;
#endif
    while( 1 )
    {
#ifdef USE_SENDFILE
       if (use_sendfile) {
            off_t sf = processed_size;
            n = KDE_sendfile( dest_fd, src_fd, &sf, MAX_IPC_SIZE );
            processed_size = sf;
            if ( n == -1 && ( errno == EINVAL || errno == ENOSYS ) ) { //not all filesystems support sendfile()
                kDebug(7101) << "sendfile() not supported, falling back ";
                use_sendfile = false;
            }
       }
       if (!use_sendfile)
#endif
        n = ::read( src_fd, buffer, MAX_IPC_SIZE );

       if (n == -1)
       {
          if (errno == EINTR)
              continue;
#ifdef USE_SENDFILE
          if ( use_sendfile ) {
            kDebug(7101) << "sendfile() error:" << strerror(errno);
            if ( errno == ENOSPC ) // disk full
            {
                error( KIO::ERR_DISK_FULL, _dest );
                remove( _dest.data() );
            }
            else {
                error( KIO::ERR_SLAVE_DEFINED,
                        i18n("Cannot copy file from %1 to %2. (Errno: %3)",
                          src.toLocalFile(), dest.toLocalFile(), errno ) );
            }
          } else
#endif
          error( KIO::ERR_COULD_NOT_READ, _src );
          ::close(src_fd);
          ::close(dest_fd);
#ifdef HAVE_POSIX_ACL
          if (acl) acl_free(acl);
#endif
          return;
       }
       if (n == 0)
          break; // Finished
#ifdef USE_SENDFILE
       if ( !use_sendfile ) {
#endif
         if (write_all( dest_fd, buffer, n))
         {
           ::close(src_fd);
           ::close(dest_fd);

           if ( errno == ENOSPC ) // disk full
           {
              error( KIO::ERR_DISK_FULL, _dest );
              remove( _dest.data() );
           }
           else
           {
              kWarning(7101) << "Couldn't write[2]. Error:" << strerror(errno);
              error( KIO::ERR_COULD_NOT_WRITE, _dest );
           }
#ifdef HAVE_POSIX_ACL
           if (acl) acl_free(acl);
#endif
           return;
         }
         processed_size += n;
#ifdef USE_SENDFILE
       }
#endif
       processedSize( processed_size );
    }

    ::close( src_fd );

    if (::close( dest_fd))
    {
        kWarning(7101) << "Error when closing file descriptor[2]:" << strerror(errno);
        error( KIO::ERR_COULD_NOT_WRITE, _dest );
#ifdef HAVE_POSIX_ACL
        if (acl) acl_free(acl);
#endif
        return;
    }

    // set final permissions
    if ( _mode != -1 )
    {
        if ( (::chmod(_dest.data(), _mode) != 0)
#ifdef HAVE_POSIX_ACL
          || (acl && acl_set_file(_dest.data(), ACL_TYPE_ACCESS, acl) != 0)
#endif
        )
       {
           KMountPoint::Ptr mp = KMountPoint::currentMountPoints().findByPath(_dest);
           // Eat the error if the filesystem apparently doesn't support chmod.
           if ( mp && mp->testFileSystemFlag( KMountPoint::SupportsChmod ) )
               warning( i18n( "Could not change permissions for\n%1" ,  dest.toLocalFile() ) );
       }
    }
#ifdef HAVE_POSIX_ACL
    if (acl) acl_free(acl);
#endif

    // copy access and modification time
    struct utimbuf ut;
    ut.actime = buff_src.st_atime;
    ut.modtime = buff_src.st_mtime;
    if ( ::utime( _dest.data(), &ut ) != 0 )
    {
        kWarning() << QString::fromLatin1("Couldn't preserve access and modification time for\n%1").arg( _dest.data() );
    }

    processedSize( buff_src.st_size );
    finished();
}

void FileProtocol::rename( const KUrl &src, const KUrl &dest,
                           KIO::JobFlags _flags )
{
    char off_t_should_be_64_bits[sizeof(off_t) >= 8 ? 1 : -1]; (void) off_t_should_be_64_bits;
    QByteArray _src(QFile::encodeName(src.toLocalFile()));
    QByteArray _dest(QFile::encodeName(dest.toLocalFile()));
    KDE_struct_stat buff_src;
    if ( KDE_lstat( _src.data(), &buff_src ) == -1 ) {
        if ( errno == EACCES )
           error( KIO::ERR_ACCESS_DENIED, _src );
        else
           error( KIO::ERR_DOES_NOT_EXIST, _src );
        return;
    }

    KDE_struct_stat buff_dest;
    // stat symlinks here (lstat, not stat), to avoid ERR_IDENTICAL_FILES when replacing symlink
    // with its target (#169547)
    bool dest_exists = ( KDE_lstat( _dest.data(), &buff_dest ) != -1 );
    if ( dest_exists )
    {
        if (S_ISDIR(buff_dest.st_mode))
        {
           error( KIO::ERR_DIR_ALREADY_EXIST, _dest );
           return;
        }

	if ( same_inode( buff_dest, buff_src) )
	{
	    error( KIO::ERR_IDENTICAL_FILES, _dest );
	    return;
	}

        if (!(_flags & KIO::Overwrite))
        {
           error( KIO::ERR_FILE_ALREADY_EXIST, _dest );
           return;
        }
    }

    if ( KDE_rename( _src.data(), _dest.data()))
    {
        if (( errno == EACCES ) || (errno == EPERM)) {
            error( KIO::ERR_ACCESS_DENIED, _dest );
        }
        else if (errno == EXDEV) {
           error( KIO::ERR_UNSUPPORTED_ACTION, QLatin1String("rename"));
        }
        else if (errno == EROFS) { // The file is on a read-only filesystem
           error( KIO::ERR_CANNOT_DELETE, _src );
        }
        else {
           error( KIO::ERR_CANNOT_RENAME, _src );
        }
        return;
    }

    finished();
}

void FileProtocol::symlink( const QString &target, const KUrl &dest, KIO::JobFlags flags )
{
    // Assume dest is local too (wouldn't be here otherwise)
    if ( ::symlink( QFile::encodeName( target ), QFile::encodeName( dest.path() ) ) == -1 )
    {
        // Does the destination already exist ?
        if ( errno == EEXIST )
        {
            if ( (flags & KIO::Overwrite) )
            {
                // Try to delete the destination
                if ( unlink( QFile::encodeName( dest.path() ) ) != 0 )
                {
                    error( KIO::ERR_CANNOT_DELETE, dest.path() );
                    return;
                }
                // Try again - this won't loop forever since unlink succeeded
                symlink( target, dest, flags );
            }
            else
            {
                KDE_struct_stat buff_dest;
                KDE_lstat( QFile::encodeName( dest.path() ), &buff_dest );
                if (S_ISDIR(buff_dest.st_mode))
                    error( KIO::ERR_DIR_ALREADY_EXIST, dest.path() );
                else
                    error( KIO::ERR_FILE_ALREADY_EXIST, dest.path() );
                return;
            }
        }
        else
        {
            // Some error occurred while we tried to symlink
            error( KIO::ERR_CANNOT_SYMLINK, dest.path() );
            return;
        }
    }
    finished();
}

void FileProtocol::del( const KUrl& url, bool isfile)
{
    QByteArray _path( QFile::encodeName(url.toLocalFile()));
    /*****
     * Delete files
     *****/

    if (isfile) {
	kDebug( 7101 ) <<  "Deleting file "<< url.url();

	// TODO deletingFile( source );

	if ( unlink( _path.data() ) == -1 ) {
            if ((errno == EACCES) || (errno == EPERM))
               error( KIO::ERR_ACCESS_DENIED, _path );
            else if (errno == EISDIR)
               error( KIO::ERR_IS_DIRECTORY, _path );
            else
               error( KIO::ERR_CANNOT_DELETE, _path );
	    return;
	}
    } else {

      /*****
       * Delete empty directory
       *****/

      kDebug( 7101 ) << "Deleting directory " << url.url();

      if ( ::rmdir( _path.data() ) == -1 ) {
	if ((errno == EACCES) || (errno == EPERM))
	  error( KIO::ERR_ACCESS_DENIED, _path );
	else {
	  kDebug( 7101 ) << "could not rmdir " << perror;
	  error( KIO::ERR_COULD_NOT_RMDIR, _path );
	  return;
	}
      }
    }

    finished();
}

#endif  // Q_OS_WIN

QString FileProtocol::getUserName( uid_t uid ) const
{
    if ( !mUsercache.contains( uid ) ) {
        struct passwd *user = getpwuid( uid );
        if ( user ) {
            mUsercache.insert( uid, QString::fromLatin1(user->pw_name) );
        }
        else
            return QString::number( uid );
    }
    return mUsercache[uid];
}

QString FileProtocol::getGroupName( gid_t gid ) const
{
    if ( !mGroupcache.contains( gid ) ) {
        struct group *grp = getgrgid( gid );
        if ( grp ) {
            mGroupcache.insert( gid, QString::fromLatin1(grp->gr_name) );
        }
        else
            return QString::number( gid );
    }
    return mGroupcache[gid];
}

bool FileProtocol::createUDSEntry( const QString & filename, const QByteArray & path, UDSEntry & entry,
                                   short int details, bool withACL )
{
#ifndef HAVE_POSIX_ACL
    Q_UNUSED(withACL);
#endif
    assert(entry.count() == 0); // by contract :-)
    // entry.reserve( 8 ); // speed up QHash insertion

    // Note: details = 0 (only "file or directory or symlink or doesn't exist") isn't implemented
    // because there's no real performance penalty in kio_file for returning the complete
    // details. Please consider doing it in your kioslave if you're using this one as a model :)

    entry.insert( KIO::UDSEntry::UDS_NAME, filename );

    mode_t type;
    mode_t access;
    KDE_struct_stat buff;

    if ( KDE_lstat( path.data(), &buff ) == 0 )  {

        if (S_ISLNK(buff.st_mode)) {

            char buffer2[ 1000 ];
            int n = readlink( path.data(), buffer2, 1000 );
            if ( n != -1 ) {
                buffer2[ n ] = 0;
            }

            entry.insert( KIO::UDSEntry::UDS_LINK_DEST, QFile::decodeName( buffer2 ) );

            // A symlink -> follow it only if details>1
            if ( details > 1 && KDE_stat( path.data(), &buff ) == -1 ) {
                // It is a link pointing to nowhere
                type = S_IFMT - 1;
                access = S_IRWXU | S_IRWXG | S_IRWXO;

                entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, type );
                entry.insert( KIO::UDSEntry::UDS_ACCESS, access );
                entry.insert( KIO::UDSEntry::UDS_SIZE, 0LL );
                goto notype;

            }
        }
    } else {
        // kWarning() << "lstat didn't work on " << path.data();
        return false;
    }

    type = buff.st_mode & S_IFMT; // extract file type
    access = buff.st_mode & 07777; // extract permissions

    entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, type );
    entry.insert( KIO::UDSEntry::UDS_ACCESS, access );

    entry.insert( KIO::UDSEntry::UDS_SIZE, buff.st_size );

#ifdef HAVE_POSIX_ACL
    /* Append an atom indicating whether the file has extended acl information
     * and if withACL is specified also one with the acl itself. If it's a directory
     * and it has a default ACL, also append that. */
    appendACLAtoms( path, entry, type, withACL );
#endif

 notype:
    entry.insert( KIO::UDSEntry::UDS_MODIFICATION_TIME, buff.st_mtime );
    entry.insert( KIO::UDSEntry::UDS_USER, getUserName( buff.st_uid ) );
    entry.insert( KIO::UDSEntry::UDS_GROUP, getGroupName( buff.st_gid ) );
    entry.insert( KIO::UDSEntry::UDS_ACCESS_TIME, buff.st_atime );

    // Note: buff.st_ctime isn't the creation time !
    // We made that mistake for KDE 2.0, but it's in fact the
    // "file status" change time, which we don't care about.

    return true;
}

void FileProtocol::stat( const KUrl & url )
{
    if (!url.isLocalFile()) {
        KUrl redir(url);
	redir.setProtocol(config()->readEntry("DefaultRemoteProtocol", "smb"));
	redirection(redir);
	kDebug(7101) << "redirecting to " << redir.url();
	finished();
	return;
    }

    /* directories may not have a slash at the end if
     * we want to stat() them; it requires that we
     * change into it .. which may not be allowed
     * stat("/is/unaccessible")  -> rwx------
     * stat("/is/unaccessible/") -> EPERM            H.Z.
     * This is the reason for the -1
     */
#ifdef Q_WS_WIN
    QByteArray _path( QFile::encodeName(url.toLocalFile()) );
#else
    QByteArray _path( QFile::encodeName(url.path(KUrl::RemoveTrailingSlash)));
#endif
    QString sDetails = metaData(QLatin1String("details"));
    int details = sDetails.isEmpty() ? 2 : sDetails.toInt();
    kDebug(7101) << "FileProtocol::stat details=" << details;

    UDSEntry entry;
    if ( !createUDSEntry( url.fileName(), _path, entry, details, true /*with acls*/ ) )
    {
        error( KIO::ERR_DOES_NOT_EXIST, _path );
        return;
    }
#if 0
///////// debug code
    MetaData::iterator it1 = mOutgoingMetaData.begin();
    for ( ; it1 != mOutgoingMetaData.end(); it1++ ) {
        kDebug(7101) << it1.key() << " = " << it1.data();
    }
/////////
#endif
    statEntry( entry );

    finished();
}

#ifndef Q_OS_WIN
void FileProtocol::listDir( const KUrl& url)
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
    QByteArray _path( QFile::encodeName(url.toLocalFile()) );
    KDE_struct_stat buff;
    if ( KDE_stat( _path.data(), &buff ) == -1 ) {
	error( KIO::ERR_DOES_NOT_EXIST, _path );
	return;
    }

    if ( !S_ISDIR( buff.st_mode ) ) {
	error( KIO::ERR_IS_FILE, _path );
	return;
    }

    DIR *dp = 0L;
    KDE_struct_dirent *ep;

    dp = opendir( _path.data() );
    if ( dp == 0 ) {
        switch (errno)
        {
#ifdef ENOMEDIUM
	case ENOMEDIUM:
            error( ERR_SLAVE_DEFINED,
                   i18n( "No media in device for %1", url.toLocalFile() ) );
            break;
#else
        case ENOENT: // just to avoid the warning
#endif
        default:
            error( KIO::ERR_CANNOT_ENTER_DIRECTORY, _path );
            break;
        }
	return;
    }

    // Don't make this a QStringList. The locale file name we get here
    // should be passed intact to createUDSEntry to avoid problems with
    // files where QFile::encodeName(QFile::decodeName(a)) != a.
    QList<QByteArray> entryNames;
    while ( ( ep = KDE_readdir( dp ) ) != 0L )
	entryNames.append( ep->d_name );

    closedir( dp );
    totalSize( entryNames.count() );

    /* set the current dir to the path to speed up
       in not having to pass an absolute path.
       We restore the path later to get out of the
       path - the kernel wouldn't unmount or delete
       directories we keep as active directory. And
       as the slave runs in the background, it's hard
       to see for the user what the problem would be */
    char path_buffer[PATH_MAX];
    getcwd(path_buffer, PATH_MAX - 1);
    if ( chdir( _path.data() ) )  {
        if (errno == EACCES)
            error(ERR_ACCESS_DENIED, _path);
        else
            error(ERR_CANNOT_ENTER_DIRECTORY, _path);
        finished();
    }

    UDSEntry entry;
    QList<QByteArray>::ConstIterator it = entryNames.constBegin();
    QList<QByteArray>::ConstIterator end = entryNames.constEnd();
    for (; it != end; ++it) {
        entry.clear();
        if ( createUDSEntry( QFile::decodeName(*it),
                             *it /* we can use the filename as relative path*/,
                             entry, 2, true ) )
          listEntry( entry, false);
    }

    listEntry( entry, true ); // ready

    kDebug(7101) << "============= COMPLETED LIST ============";

    chdir(path_buffer);
    finished();
}
#endif  // !Q_OS_WIN

/*
void FileProtocol::testDir( const QString& path )
{
    QByteArray _path( QFile::encodeName(path));
    KDE_struct_stat buff;
    if ( KDE_stat( _path.data(), &buff ) == -1 ) {
	error( KIO::ERR_DOES_NOT_EXIST, path );
	return;
    }

    if ( S_ISDIR( buff.st_mode ) )
	isDirectory();
    else
	isFile();

    finished();
}
*/

void FileProtocol::special( const QByteArray &data)
{
    int tmp;
    QDataStream stream(data);

    stream >> tmp;
    switch (tmp) {
    case 1:
      {
	QString fstype, dev, point;
	qint8 iRo;

	stream >> iRo >> fstype >> dev >> point;

	bool ro = ( iRo != 0 );

	kDebug(7101) << "MOUNTING fstype=" << fstype << " dev=" << dev << " point=" << point << " ro=" << ro;
	bool ok = pmount( dev );
	if (ok)
	    finished();
	else
	    mount( ro, fstype.toAscii(), dev, point );

      }
      break;
    case 2:
      {
	QString point;
	stream >> point;
	bool ok = pumount( point );
	if (ok)
	    finished();
	else
	    unmount( point );
      }
      break;

    default:
      break;
    }
}

// Connected to KShred
void FileProtocol::slotProcessedSize( KIO::filesize_t bytes )
{
  kDebug(7101) << "FileProtocol::slotProcessedSize (" << (unsigned int) bytes << ")";
  processedSize( bytes );
}

// Connected to KShred
void FileProtocol::slotInfoMessage( const QString & msg )
{
  kDebug(7101) << "FileProtocol::slotInfoMessage (" << msg << ")";
  infoMessage( msg );
}

void FileProtocol::mount( bool _ro, const char *_fstype, const QString& _dev, const QString& _point )
{
    kDebug(7101) << "FileProtocol::mount _fstype=" << _fstype;

#ifdef HAVE_VOLMGT
	/*
	 *  support for Solaris volume management
	 */
	QString err;
	QByteArray devname = QFile::encodeName( _dev );

	if( volmgt_running() ) {
//		kDebug(7101) << "VOLMGT: vold ok.";
		if( volmgt_check( devname.data() ) == 0 ) {
			kDebug(7101) << "VOLMGT: no media in "
					<< devname.data();
			err = i18n("No Media inserted or Media not recognized.");
			error( KIO::ERR_COULD_NOT_MOUNT, err );
			return;
		} else {
			kDebug(7101) << "VOLMGT: " << devname.data()
				<< ": media ok";
			finished();
			return;
		}
	} else {
		err = i18n("\"vold\" is not running.");
		kDebug(7101) << "VOLMGT: " << err;
		error( KIO::ERR_COULD_NOT_MOUNT, err );
		return;
	}
#else


    KTemporaryFile tmpFile;
    tmpFile.setAutoRemove(false);
    tmpFile.open();
    QByteArray tmpFileName = QFile::encodeName(tmpFile.fileName());
    QByteArray dev;
    if ( _dev.startsWith( "LABEL=" ) ) { // turn LABEL=foo into -L foo (#71430)
        QString labelName = _dev.mid( 6 );
        dev = "-L ";
        dev += QFile::encodeName( KShell::quoteArg( labelName ) ); // is it correct to assume same encoding as filesystem?
    } else if ( _dev.startsWith( "UUID=" ) ) { // and UUID=bar into -U bar
        QString uuidName = _dev.mid( 5 );
        dev = "-U ";
        dev += QFile::encodeName( KShell::quoteArg( uuidName ) );
    }
    else
        dev = QFile::encodeName( KShell::quoteArg(_dev) ); // get those ready to be given to a shell

    QByteArray point = QFile::encodeName( KShell::quoteArg(_point) );
    bool fstype_empty = !_fstype || !*_fstype;
    QByteArray fstype = KShell::quoteArg(_fstype).toLatin1(); // good guess
    QByteArray readonly = _ro ? "-r" : "";
    QString epath = QString::fromLocal8Bit(qgetenv("PATH"));
    QString path = QLatin1String("/sbin:/bin");
    if(!epath.isEmpty())
        path += QLatin1String(":") + epath;
    QByteArray mountProg = KGlobal::dirs()->findExe("mount", path).toLocal8Bit();
    if (mountProg.isEmpty()){
      error( KIO::ERR_COULD_NOT_MOUNT, i18n("Could not find program \"mount\""));
      return;
    }
    QByteArray buffer = mountProg + ' ';

    // Two steps, in case mount doesn't like it when we pass all options
    for ( int step = 0 ; step <= 1 ; step++ )
    {
        // Mount using device only if no fstype nor mountpoint (KDE-1.x like)
        if ( !_dev.isEmpty() && _point.isEmpty() && fstype_empty )
            buffer += dev;
        else
          // Mount using the mountpoint, if no fstype nor device (impossible in first step)
          if ( !_point.isEmpty() && _dev.isEmpty() && fstype_empty )
              buffer += point;
          else
            // mount giving device + mountpoint but no fstype
            if ( !_point.isEmpty() && !_dev.isEmpty() && fstype_empty )
                buffer += readonly + ' ' + dev + ' ' + point;
            else
              // mount giving device + mountpoint + fstype
#if defined(__svr4__) && defined(Q_OS_SOLARIS) // MARCO for Solaris 8 and I
                // believe this is true for SVR4 in general
                buffer += "-F " + fstype + ' ' + (_ro ? "-oro" : "") + ' ' + dev + ' ' + point;
#else
                buffer += readonly + " -t " + fstype + ' ' + dev + ' ' + point;
#endif
        buffer += " 2>" + tmpFileName;
        kDebug(7101) << buffer;

        int mount_ret = system( buffer.constData() );

        QString err = testLogFile( tmpFileName );
        if ( err.isEmpty() && mount_ret == 0)
        {
            finished();
            return;
        }
        else
        {
            // Didn't work - or maybe we just got a warning
            KMountPoint::Ptr mp = KMountPoint::currentMountPoints().findByDevice( _dev );
            // Is the device mounted ?
            if ( mp && mount_ret == 0)
            {
                kDebug(7101) << "mount got a warning: " << err;
                warning( err );
                finished();
                return;
            }
            else
            {
                if ( (step == 0) && !_point.isEmpty())
                {
                    kDebug(7101) << err;
                    kDebug(7101) << "Mounting with those options didn't work, trying with only mountpoint";
                    fstype = "";
                    fstype_empty = true;
                    dev = "";
                    // The reason for trying with only mountpoint (instead of
                    // only device) is that some people (hi Malte!) have the
                    // same device associated with two mountpoints
                    // for different fstypes, like /dev/fd0 /mnt/e2floppy and
                    // /dev/fd0 /mnt/dosfloppy.
                    // If the user has the same mountpoint associated with two
                    // different devices, well they shouldn't specify the
                    // mountpoint but just the device.
                }
                else
                {
                    error( KIO::ERR_COULD_NOT_MOUNT, err );
                    return;
                }
            }
        }
    }
#endif /* ! HAVE_VOLMGT */
}


void FileProtocol::unmount( const QString& _point )
{
    QByteArray buffer;

    KTemporaryFile tmpFile;
    tmpFile.setAutoRemove(false);
    tmpFile.open();
    QByteArray tmpFileName = QFile::encodeName(tmpFile.fileName());
    QString err;

#ifdef HAVE_VOLMGT
	/*
	 *  support for Solaris volume management
	 */
	char *devname;
	char *ptr;
	FILE *mnttab;
	struct mnttab mnt;

	if( volmgt_running() ) {
		kDebug(7101) << "VOLMGT: looking for "
			<< _point.toLocal8Bit();

		if( (mnttab = KDE_fopen( MNTTAB, "r" )) == NULL ) {
			err = "could not open mnttab";
			kDebug(7101) << "VOLMGT: " << err;
			error( KIO::ERR_COULD_NOT_UNMOUNT, err );
			return;
		}

		/*
		 *  since there's no way to derive the device name from
		 *  the mount point through the volmgt library (and
		 *  media_findname() won't work in this case), we have to
		 *  look ourselves...
		 */
		devname = NULL;
		rewind( mnttab );
		while( getmntent( mnttab, &mnt ) == 0 ) {
			if( strcmp( _point.toLocal8Bit(), mnt.mnt_mountp ) == 0 ){
				devname = mnt.mnt_special;
				break;
			}
		}
		fclose( mnttab );

		if( devname == NULL ) {
			err = "not in mnttab";
			kDebug(7101) << "VOLMGT: "
				<< QFile::encodeName(_point).data()
				<< ": " << err;
			error( KIO::ERR_COULD_NOT_UNMOUNT, err );
			return;
		}

		/*
		 *  strip off the directory name (volume name)
		 *  the eject(1) command will handle unmounting and
		 *  physically eject the media (if possible)
		 */
		ptr = strrchr( devname, '/' );
		*ptr = '\0';
                QByteArray qdevname(QFile::encodeName(KShell::quoteArg(QFile::decodeName(QByteArray(devname)))).data());
		buffer = "/usr/bin/eject " + qdevname + " 2>" + tmpFileName;
		kDebug(7101) << "VOLMGT: eject " << qdevname;

		/*
		 *  from eject(1): exit status == 0 => need to manually eject
		 *                 exit status == 4 => media was ejected
		 */
		if( WEXITSTATUS( system( buffer.constData() )) == 4 ) {
			/*
			 *  this is not an error, so skip "testLogFile()"
			 *  to avoid wrong/confusing error popup
			 */
			QFile::remove( tmpFileName );
			finished();
			return;
		}
	} else {
		/*
		 *  eject(1) should do its job without vold(1M) running,
		 *  so we probably could call eject anyway, but since the
		 *  media is mounted now, vold must've died for some reason
		 *  during the user's session, so it should be restarted...
		 */
		err = i18n("\"vold\" is not running.");
		kDebug(7101) << "VOLMGT: " << err;
		error( KIO::ERR_COULD_NOT_UNMOUNT, err );
		return;
	}
#else
    QString epath = QString::fromLocal8Bit(qgetenv("PATH"));
    QString path = QLatin1String("/sbin:/bin");
    if (!epath.isEmpty())
       path += ':' + epath;
    QByteArray umountProg = KGlobal::dirs()->findExe("umount", path).toLocal8Bit();

    if (umountProg.isEmpty()) {
        error( KIO::ERR_COULD_NOT_UNMOUNT, i18n("Could not find program \"umount\""));
        return;
    }
    buffer = umountProg + QFile::encodeName(KShell::quoteArg(_point)) + " 2>" + tmpFileName;
    system( buffer.constData() );
#endif /* HAVE_VOLMGT */

    err = testLogFile( tmpFileName );
    if ( err.isEmpty() )
        finished();
    else
        error( KIO::ERR_COULD_NOT_UNMOUNT, err );
}

/*************************************
 *
 * pmount handling
 *
 *************************************/

bool FileProtocol::pmount(const QString &dev)
{
    QString epath = QString::fromLocal8Bit(qgetenv("PATH"));
    QString path = QLatin1String("/sbin:/bin");
    if (!epath.isEmpty())
        path += ':' + epath;
    QString pmountProg = KGlobal::dirs()->findExe("pmount", path);

    if (pmountProg.isEmpty())
        return false;

    QByteArray buffer = QFile::encodeName(pmountProg) + ' ' +
                        QFile::encodeName(KShell::quoteArg(dev));

    int res = system( buffer.constData() );

    return res==0;
}

bool FileProtocol::pumount(const QString &point)
{
    KMountPoint::Ptr mp = KMountPoint::currentMountPoints(KMountPoint::NeedRealDeviceName).findByPath(point);
    if (!mp)
        return false;
    QString dev = mp->realDeviceName();
    if (dev.isEmpty()) return false;

    QString epath = QString::fromLocal8Bit(qgetenv("PATH"));
    QString path = QLatin1String("/sbin:/bin");
    if (!epath.isEmpty())
        path += ':' + epath;
    QString pumountProg = KGlobal::dirs()->findExe("pumount", path);

    if (pumountProg.isEmpty())
        return false;

    QByteArray buffer = QFile::encodeName(pumountProg);
    buffer += ' ';
    buffer += QFile::encodeName(KShell::quoteArg(dev));

    int res = system( buffer.data() );

    return res==0;
}

/*************************************
 *
 * Utilities
 *
 *************************************/

static QString testLogFile( const QByteArray& _filename )
{
    char buffer[ 1024 ];
    KDE_struct_stat buff;

    QString result;

    KDE_stat( _filename, &buff );
    int size = buff.st_size;
    if ( size == 0 ) {
	unlink( _filename );
	return result;
    }

    FILE * f = KDE_fopen( _filename, "rb" );
    if ( f == 0L ) {
	unlink( _filename );
	result = i18n("Could not read %1", QFile::decodeName(_filename));
	return result;
    }

    result = "";
    const char *p = "";
    while ( p != 0L ) {
	p = fgets( buffer, sizeof(buffer)-1, f );
	if ( p != 0L )
	    result += QString::fromLocal8Bit(buffer);
    }

    fclose( f );

    unlink( _filename );

    return result;
}

/*************************************
 *
 * ACL handling helpers
 *
 *************************************/
#ifdef HAVE_POSIX_ACL

static bool isExtendedACL( acl_t acl )
{
    return ( acl_equiv_mode( acl, 0 ) != 0 );
}

static void appendACLAtoms( const QByteArray & path, UDSEntry& entry, mode_t type, bool withACL )
{
    // first check for a noop
    if ( acl_extended_file( path.data() ) == 0 ) return;

    acl_t acl = 0;
    acl_t defaultAcl = 0;
    bool isDir = S_ISDIR( type );
    // do we have an acl for the file, and/or a default acl for the dir, if it is one?
    acl = acl_get_file( path.data(), ACL_TYPE_ACCESS );
    /* Sadly libacl does not provided a means of checking for extended ACL and default
     * ACL separately. Since a directory can have both, we need to check again. */
    if ( isDir ) {
        if ( acl ) {
            if ( !isExtendedACL( acl ) ) {
                acl_free( acl );
                acl = 0;
            }
        }
        defaultAcl = acl_get_file( path.data(), ACL_TYPE_DEFAULT );
    }
    if ( acl || defaultAcl ) {
      kDebug(7101) << path.data() << " has extended ACL entries ";
      entry.insert( KIO::UDSEntry::UDS_EXTENDED_ACL, 1 );
    }
    if ( withACL ) {
        if ( acl ) {
            ssize_t size = acl_size( acl );
            const QString str = QString::fromLatin1( acl_to_text( acl, &size ) );
            entry.insert( KIO::UDSEntry::UDS_ACL_STRING, str );
            kDebug(7101) << path.data() << "ACL: " << str;
        }
        if ( defaultAcl ) {
            ssize_t size = acl_size( defaultAcl );
            const QString str = QString::fromLatin1( acl_to_text( defaultAcl, &size ) );
            entry.insert( KIO::UDSEntry::UDS_DEFAULT_ACL_STRING, str );
            kDebug(7101) << path.data() << "DEFAULT ACL: " << str;
        }
    }
    if ( acl ) acl_free( acl );
    if ( defaultAcl ) acl_free( defaultAcl );
}
#endif

#include "file.moc"
