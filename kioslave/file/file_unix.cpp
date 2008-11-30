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

#include "file.h"

#include <config.h>

#include <QtCore/QFile>

#include <kde_file.h>
#include <kdebug.h>
#include <kconfiggroup.h>
#include <kmountpoint.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <utime.h>
#include <pwd.h>

using namespace KIO;

#define MAX_IPC_SIZE (1024*32)

static bool
same_inode(const KDE_struct_stat &src, const KDE_struct_stat &dest)
{
   if (src.st_ino == dest.st_ino &&
       src.st_dev == dest.st_dev)
     return true;

   return false;
}

extern int write_all(int fd, const char *buf, size_t len);

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

void FileProtocol::chown( const KUrl& url, const QString& owner, const QString& group )
{
    const QByteArray _path( QFile::encodeName(url.toLocalFile()) );
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
}
