/*
   Copyright (C) 2000-2002 Stephan Kulow <coolo@kde.org>
   Copyright (C) 2000-2002 David Faure <faure@kde.org>
   Copyright (C) 2000-2002 Waldo Bastian <bastian@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <config.h>

#include <qglobal.h> //for Q_OS_XXX
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
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

#ifdef USE_POSIX_ACL
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

#include <q3valuelist.h>
#include <qregexp.h>

#include <kdebug.h>
#include <kurl.h>
#include <kinstance.h>
#include <ksimpleconfig.h>
#include <ktempfile.h>
#include <klocale.h>
#include <qfile.h>
#include <q3strlist.h>
#include "file.h"
#include <limits.h>
#include <kprocess.h>
#include <kmountpoint.h>
#include <kstandarddirs.h>

#ifdef HAVE_VOLMGT
#include <volmgt.h>
#include <sys/mnttab.h>
#endif

#include <kstandarddirs.h>
#include <kio/ioslave_defaults.h>
#include <kde_file.h>
#include <kglobal.h>
#include <kmimetype.h>

using namespace KIO;

#define MAX_IPC_SIZE (1024*32)

static QString testLogFile( const char *_filename );
#ifdef USE_POSIX_ACL
static bool isExtendedACL(  acl_t p_acl );
static void appendACLAtoms( const Q3CString & path, UDSEntry& entry, 
                            mode_t type, bool withACL );
#endif

extern "C" { KDE_EXPORT int kdemain(int argc, char **argv); }

int kdemain( int argc, char **argv )
{
  KLocale::setMainCatalog("kdelibs");
  KInstance instance( "kio_file" );
  ( void ) KGlobal::locale();

  kdDebug(7101) << "Starting " << getpid() << endl;

  if (argc != 4)
  {
     fprintf(stderr, "Usage: kio_file protocol domain-socket1 domain-socket2\n");
     exit(-1);
  }

  FileProtocol slave(argv[2], argv[3]);
  slave.dispatchLoop();

  kdDebug(7101) << "Done" << endl;
  return 0;
}


FileProtocol::FileProtocol( const Q3CString &pool, const Q3CString &app ) : SlaveBase( "file", pool, app )
{
    usercache.setAutoDelete( true );
    groupcache.setAutoDelete( true );
}


int FileProtocol::setACL( const char *path, mode_t perm, bool directoryDefault )
{
    int ret = 0;
#ifdef USE_POSIX_ACL

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
        acl = acl_from_text( ACLString.latin1() );
        if ( acl_valid( acl ) == 0 ) { // let's be safe
            ret = acl_set_file( path, ACL_TYPE_ACCESS, acl );
            ssize_t size = acl_size( acl );
            kdDebug(7101) << "Set ACL on: " << path << " to: " << acl_to_text( acl, &size ) << endl;
        }
        acl_free( acl );
        if ( ret != 0 ) return ret; // better stop trying right away
    }

    if ( directoryDefault && !defaultACLString.isEmpty() ) {
        if ( defaultACLString == "ACL_DELETE" ) {
            // user told us to delete the default ACL, do so
            ret += acl_delete_def_file( path );
        } else {
            acl_t acl = acl_from_text( defaultACLString.latin1() );
            if ( acl_valid( acl ) == 0 ) { // let's be safe
                ret += acl_set_file( path, ACL_TYPE_DEFAULT, acl );
                ssize_t size = acl_size( acl );
                kdDebug(7101) << "Set Default ACL on: " << path << " to: " << acl_to_text( acl, &size ) << endl;
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

void FileProtocol::chmod( const KURL& url, int permissions )
{
    Q3CString _path( QFile::encodeName(url.path()) );
    /* FIXME: Should be atomic */
    if ( ::chmod( _path.data(), permissions ) == -1 || 
        ( setACL( _path.data(), permissions, false ) == -1 ) ||
        /* if not a directory, cannot set default ACLs */
        ( setACL( _path.data(), permissions, true ) == -1 && errno != ENOTDIR ) ) {

        switch (errno) {
            case EPERM:
            case EACCES:
                error( KIO::ERR_ACCESS_DENIED, url.path() );
                break;
            case ENOTSUP:
                error( KIO::ERR_UNSUPPORTED_ACTION, url.path() );
                break;
            case ENOSPC:
                error( KIO::ERR_DISK_FULL, url.path() );
                break;
            default:
        error( KIO::ERR_CANNOT_CHMOD, url.path() );
        }
    } else
        finished();
}

void FileProtocol::mkdir( const KURL& url, int permissions )
{
    Q3CString _path( QFile::encodeName(url.path()));

    kdDebug(7101) << "mkdir(): " << _path << ", permission = " << permissions << endl;

    KDE_struct_stat buff;
    if ( KDE_stat( _path.data(), &buff ) == -1 ) {
        if ( ::mkdir( _path.data(), 0777 /*umask will be applied*/ ) != 0 ) {
            if ( errno == EACCES ) {
          error( KIO::ERR_ACCESS_DENIED, url.path() );
          return;
            } else if ( errno == ENOSPC ) {
          error( KIO::ERR_DISK_FULL, url.path() );
          return;
            } else {
          error( KIO::ERR_COULD_NOT_MKDIR, url.path() );
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
        kdDebug(7101) << "ERR_DIR_ALREADY_EXIST" << endl;
        error( KIO::ERR_DIR_ALREADY_EXIST, url.path() );
        return;
    }
    error( KIO::ERR_FILE_ALREADY_EXIST, url.path() );
    return;
}

void FileProtocol::get( const KURL& url )
{
    if (!url.isLocalFile()) {
        KURL redir(url);
	redir.setProtocol(config()->readEntry("DefaultRemoteProtocol", "smb"));
	redirection(redir);
	finished();
	return;
    }

    Q3CString _path( QFile::encodeName(url.path()));
    KDE_struct_stat buff;
    if ( KDE_stat( _path.data(), &buff ) == -1 ) {
        if ( errno == EACCES )
           error( KIO::ERR_ACCESS_DENIED, url.path() );
        else
           error( KIO::ERR_DOES_NOT_EXIST, url.path() );
        return;
    }

    if ( S_ISDIR( buff.st_mode ) ) {
        error( KIO::ERR_IS_DIRECTORY, url.path() );
        return;
    }
    if ( !S_ISREG( buff.st_mode ) ) {
        error( KIO::ERR_CANNOT_OPEN_FOR_READING, url.path() );
        return;
    }

    int fd = KDE_open( _path.data(), O_RDONLY);
    if ( fd < 0 ) {
        error( KIO::ERR_CANNOT_OPEN_FOR_READING, url.path() );
        return;
    }

#ifdef HAVE_FADVISE
    posix_fadvise( fd, 0, 0, POSIX_FADV_SEQUENTIAL);
#endif

    // Determine the mimetype of the file to be retrieved, and emit it.
    // This is mandatory in all slaves (for KRun/BrowserRun to work).
    KMimeType::Ptr mt = KMimeType::findByURL( url, buff.st_mode, true /* local URL */ );
    emit mimeType( mt->name() );

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
                kdDebug( 7101 ) << "Resume offset: " << KIO::number(offset) << endl;
            }
        }
    }

    totalSize( buff.st_size );

    char buffer[ MAX_IPC_SIZE ];
    QByteArray array;

    while( 1 )
    {
       int n = ::read( fd, buffer, MAX_IPC_SIZE );
       if (n == -1)
       {
          if (errno == EINTR)
              continue;
          error( KIO::ERR_COULD_NOT_READ, url.path());
          close(fd);
          return;
       }
       if (n == 0)
          break; // Finished

       array.setRawData(buffer, n);
       data( array );
       array.resetRawData(buffer, n);

       processed_size += n;
       processedSize( processed_size );

       //kdDebug( 7101 ) << "Processed: " << KIO::number (processed_size) << endl;
    }

    data( QByteArray() );

    close( fd );

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

void FileProtocol::put( const KURL& url, int _mode, bool _overwrite, bool _resume )
{
    QString dest_orig = url.path();
    Q3CString _dest_orig( QFile::encodeName(dest_orig));

    kdDebug(7101) << "put(): " << dest_orig << ", mode=" << _mode << endl;

    QString dest_part( dest_orig );
    dest_part += QLatin1String(".part");
    Q3CString _dest_part( QFile::encodeName(dest_part));

    KDE_struct_stat buff_orig;
    bool bOrigExists = (KDE_lstat( _dest_orig.data(), &buff_orig ) != -1);
    bool bPartExists = false;
    bool bMarkPartial = config()->readBoolEntry("MarkPartial", true);

    if (bMarkPartial)
    {
        KDE_struct_stat buff_part;
        bPartExists = (KDE_stat( _dest_part.data(), &buff_part ) != -1);

        if (bPartExists && !_resume && !_overwrite && buff_part.st_size > 0 && S_ISREG(buff_part.st_mode))
        {
            kdDebug(7101) << "FileProtocol::put : calling canResume with "
                          << KIO::number(buff_part.st_size) << endl;

            // Maybe we can use this partial file for resuming
            // Tell about the size we have, and the app will tell us
            // if it's ok to resume or not.
            _resume = canResume( buff_part.st_size );

            kdDebug(7101) << "FileProtocol::put got answer " << _resume << endl;
        }
    }

    if ( bOrigExists && !_overwrite && !_resume)
    {
        if (S_ISDIR(buff_orig.st_mode))
            error( KIO::ERR_DIR_ALREADY_EXIST, dest_orig );
        else
            error( KIO::ERR_FILE_ALREADY_EXIST, dest_orig );
        return;
    }

    int result;
    QString dest;
    Q3CString _dest;

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
                    kdDebug(7101) << "Appending .part extension to " << dest_orig << endl;
                    dest = dest_part;
                    if ( bPartExists && !_resume )
                    {
                        kdDebug(7101) << "Deleting partial file " << dest_part << endl;
                        remove( _dest_part.data() );
                        // Catch errors when we try to open the file.
                    }
                }
                else
                {
                    dest = dest_orig;
                    if ( bOrigExists && !_resume )
                    {
                        kdDebug(7101) << "Deleting destination file " << dest_orig << endl;
                        remove( _dest_orig.data() );
                        // Catch errors when we try to open the file.
                    }
                }

                _dest = QFile::encodeName(dest);

                if ( _resume )
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
                    kdDebug(7101) << "####################### COULD NOT WRITE " << dest << " _mode=" << _mode << endl;
                    kdDebug(7101) << "errno==" << errno << "(" << strerror(errno) << ")" << endl;
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
                  kdWarning(7101) << "Couldn't write. Error:" << strerror(errno) << endl;
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
        kdDebug(7101) << "Error during 'put'. Aborting." << endl;

        if (fd != -1)
        {
          close(fd);

          KDE_struct_stat buff;
          if (bMarkPartial && KDE_stat( _dest.data(), &buff ) == 0)
          {
            int size = config()->readNumEntry("MinimumKeepSize", DEFAULT_MINIMUM_KEEP_SIZE);
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

    if ( close(fd) )
    {
        kdWarning(7101) << "Error when closing file descriptor:" << strerror(errno) << endl;
        error( KIO::ERR_COULD_NOT_WRITE, dest_orig);
        return;
    }

    // after full download rename the file back to original name
    if ( bMarkPartial )
    {
        // If the original URL is a symlink and we were asked to overwrite it,
        // remove the symlink first. This ensures that we do not overwrite the
        // current source if the symlink points to it.
        if( _overwrite && S_ISLNK( buff_orig.st_mode ) )
          remove( _dest_orig.data() );

        if ( ::rename( _dest.data(), _dest_orig.data() ) )
        {
            kdWarning(7101) << " Couldn't rename " << _dest << " to " << _dest_orig << endl;
            error( KIO::ERR_CANNOT_RENAME_PARTIAL, dest_orig );
            return;
        }
    }

    // set final permissions
    if ( _mode != -1 && !_resume )
    {
        if (::chmod(_dest_orig.data(), _mode) != 0)
        {
            // couldn't chmod. Eat the error if the filesystem apparently doesn't support it.
            if ( KIO::testFileSystemFlag( _dest_orig, KIO::SupportsChmod ) )
                 warning( i18n( "Could not change permissions for\n%1" ).arg( dest_orig ) );
        }
    }

    // We have done our job => finish
    finished();
}


void FileProtocol::copy( const KURL &src, const KURL &dest,
                         int _mode, bool _overwrite )
{
    kdDebug(7101) << "copy(): " << src << " -> " << dest << ", mode=" << _mode << endl;

    Q3CString _src( QFile::encodeName(src.path()));
    Q3CString _dest( QFile::encodeName(dest.path()));
    KDE_struct_stat buff_src;
#ifdef USE_POSIX_ACL
    acl_t acl;
#endif

    if ( KDE_stat( _src.data(), &buff_src ) == -1 ) {
        if ( errno == EACCES )
           error( KIO::ERR_ACCESS_DENIED, src.path() );
        else
           error( KIO::ERR_DOES_NOT_EXIST, src.path() );
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
           error( KIO::ERR_DIR_ALREADY_EXIST, dest.path() );
           return;
        }

        if (!_overwrite)
        {
           error( KIO::ERR_FILE_ALREADY_EXIST, dest.path() );
           return;
        }

        // If the destination is a symlink and overwrite is TRUE,
        // remove the symlink first to prevent the scenario where
        // the symlink actually points to current source!
        if (_overwrite && S_ISLNK(buff_dest.st_mode))
        {
            kdDebug(7101) << "copy(): LINK DESTINATION" << endl;
            remove( _dest.data() );
        }
    }

    int src_fd = KDE_open( _src.data(), O_RDONLY);
    if ( src_fd < 0 ) {
	error( KIO::ERR_CANNOT_OPEN_FOR_READING, src.path() );
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
	kdDebug(7101) << "###### COULD NOT WRITE " << dest.url() << endl;
        if ( errno == EACCES ) {
            error( KIO::ERR_WRITE_ACCESS_DENIED, dest.path() );
        } else {
            error( KIO::ERR_CANNOT_OPEN_FOR_WRITING, dest.path() );
        }
        close(src_fd);
        return;
    }

#ifdef HAVE_FADVISE
    posix_fadvise(dest_fd,0,0,POSIX_FADV_SEQUENTIAL);
#endif

#ifdef USE_POSIX_ACL
    acl = acl_get_fd(src_fd);
    if ( acl && !isExtendedACL( acl ) ) {
        kdDebug(7101) << _dest.data() << " doesn't have extended ACL" << endl;
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
            n = ::sendfile( dest_fd, src_fd, &sf, MAX_IPC_SIZE );
            processed_size = sf;
            if ( n == -1 && errno == EINVAL ) { //not all filesystems support sendfile()
                kdDebug(7101) << "sendfile() not supported, falling back " << endl;
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
            kdDebug(7101) << "sendfile() error:" << strerror(errno) << endl;
            if ( errno == ENOSPC ) // disk full
            {
                error( KIO::ERR_DISK_FULL, dest.path());
                remove( _dest.data() );
            }
            else {
                error( KIO::ERR_SLAVE_DEFINED,
                        i18n("Cannot copy file from %1 to %2. (Errno: %3)")
                        .arg( src.path() ).arg( dest.path() ).arg( errno ) );
            }
          } else
#endif
          error( KIO::ERR_COULD_NOT_READ, src.path());
          close(src_fd);
          close(dest_fd);
#ifdef USE_POSIX_ACL
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
           close(src_fd);
           close(dest_fd);

           if ( errno == ENOSPC ) // disk full
           {
              error( KIO::ERR_DISK_FULL, dest.path());
              remove( _dest.data() );
           }
           else
           {
              kdWarning(7101) << "Couldn't write[2]. Error:" << strerror(errno) << endl;
              error( KIO::ERR_COULD_NOT_WRITE, dest.path());
           }
#ifdef USE_POSIX_ACL
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

    close( src_fd );

    if (close( dest_fd))
    {
        kdWarning(7101) << "Error when closing file descriptor[2]:" << strerror(errno) << endl;
        error( KIO::ERR_COULD_NOT_WRITE, dest.path());
#ifdef USE_POSIX_ACL
        if (acl) acl_free(acl);
#endif
        return;
    }

    // set final permissions
    if ( _mode != -1 )
    {
        if ( (::chmod(_dest.data(), _mode) != 0)
#ifdef USE_POSIX_ACL
          || (acl && acl_set_file(_dest.data(), ACL_TYPE_ACCESS, acl) != 0)
#endif
        )
       {
        // Eat the error if the filesystem apparently doesn't support chmod.
        if ( KIO::testFileSystemFlag( _dest, KIO::SupportsChmod ) )
            warning( i18n( "Could not change permissions for\n%1" ).arg( dest.path() ) );
       }
    }
#ifdef USE_POSIX_ACL
    if (acl) acl_free(acl);
#endif

    // copy access and modification time
    struct utimbuf ut;
    ut.actime = buff_src.st_atime;
    ut.modtime = buff_src.st_mtime;
    if ( ::utime( _dest.data(), &ut ) != 0 )
    {
        kdWarning() << QString::fromLatin1("Couldn't preserve access and modification time for\n%1").arg( dest.path() ) << endl;
    }

    processedSize( buff_src.st_size );
    finished();
}

void FileProtocol::rename( const KURL &src, const KURL &dest,
                           bool _overwrite )
{
    Q3CString _src( QFile::encodeName(src.path()));
    Q3CString _dest( QFile::encodeName(dest.path()));
    KDE_struct_stat buff_src;
    if ( KDE_lstat( _src.data(), &buff_src ) == -1 ) {
        if ( errno == EACCES )
           error( KIO::ERR_ACCESS_DENIED, src.path() );
        else
           error( KIO::ERR_DOES_NOT_EXIST, src.path() );
        return;
    }

    KDE_struct_stat buff_dest;
    bool dest_exists = ( KDE_stat( _dest.data(), &buff_dest ) != -1 );
    if ( dest_exists )
    {
        if (S_ISDIR(buff_dest.st_mode))
        {
           error( KIO::ERR_DIR_ALREADY_EXIST, dest.path() );
           return;
        }

        if (!_overwrite)
        {
           error( KIO::ERR_FILE_ALREADY_EXIST, dest.path() );
           return;
        }
    }

    if ( ::rename( _src.data(), _dest.data()))
    {
        if (( errno == EACCES ) || (errno == EPERM)) {
            error( KIO::ERR_ACCESS_DENIED, dest.path() );
        }
        else if (errno == EXDEV) {
           error( KIO::ERR_UNSUPPORTED_ACTION, QLatin1String("rename"));
        }
        else if (errno == EROFS) { // The file is on a read-only filesystem
           error( KIO::ERR_CANNOT_DELETE, src.path() );
        }
        else {
           error( KIO::ERR_CANNOT_RENAME, src.path() );
        }
        return;
    }

    finished();
}

void FileProtocol::symlink( const QString &target, const KURL &dest, bool overwrite )
{
    // Assume dest is local too (wouldn't be here otherwise)
    if ( ::symlink( QFile::encodeName( target ), QFile::encodeName( dest.path() ) ) == -1 )
    {
        // Does the destination already exist ?
        if ( errno == EEXIST )
        {
            if ( overwrite )
            {
                // Try to delete the destination
                if ( unlink( QFile::encodeName( dest.path() ) ) != 0 )
                {
                    error( KIO::ERR_CANNOT_DELETE, dest.path() );
                    return;
                }
                // Try again - this won't loop forever since unlink succeeded
                symlink( target, dest, overwrite );
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

void FileProtocol::del( const KURL& url, bool isfile)
{
    Q3CString _path( QFile::encodeName(url.path()));
    /*****
     * Delete files
     *****/

    if (isfile) {
	kdDebug( 7101 ) <<  "Deleting file "<< url.url() << endl;

	// TODO deletingFile( source );

	if ( unlink( _path.data() ) == -1 ) {
            if ((errno == EACCES) || (errno == EPERM))
               error( KIO::ERR_ACCESS_DENIED, url.path());
            else if (errno == EISDIR)
               error( KIO::ERR_IS_DIRECTORY, url.path());
            else
               error( KIO::ERR_CANNOT_DELETE, url.path() );
	    return;
	}
    } else {

      /*****
       * Delete empty directory
       *****/

      kdDebug( 7101 ) << "Deleting directory " << url.url() << endl;

      if ( ::rmdir( _path.data() ) == -1 ) {
	if ((errno == EACCES) || (errno == EPERM))
	  error( KIO::ERR_ACCESS_DENIED, url.path());
	else {
	  kdDebug( 7101 ) << "could not rmdir " << perror << endl;
	  error( KIO::ERR_COULD_NOT_RMDIR, url.path() );
	  return;
	}
      }
    }

    finished();
}


QString FileProtocol::getUserName( uid_t uid ) 
{
    QString *temp;
    temp = usercache.find( uid );
    if ( !temp ) {
        struct passwd *user = getpwuid( uid );
        if ( user ) {
            usercache.insert( uid, new QString(QString::fromLatin1(user->pw_name)) );
            return QString::fromLatin1( user->pw_name );
        }
        else
            return QString::number( uid );
    }
    else
        return *temp;
}

QString FileProtocol::getGroupName( gid_t gid ) 
{
    QString *temp;
    temp = groupcache.find( gid );
    if ( !temp ) {
        struct group *grp = getgrgid( gid );
        if ( grp ) {
            groupcache.insert( gid, new QString(QString::fromLatin1(grp->gr_name)) );
            return QString::fromLatin1( grp->gr_name );
        }
        else
            return QString::number( gid );
    }
    else
        return *temp;
}



bool FileProtocol::createUDSEntry( const QString & filename, const Q3CString & path, UDSEntry & entry, 
                                   short int details, bool withACL )
{
#ifndef USE_POSIX_ACL
    Q_UNUSED(withACL);
#endif
    assert(entry.count() == 0); // by contract :-)
    // Note: details = 0 (only "file or directory or symlink or doesn't exist") isn't implemented
    // because there's no real performance penalty in kio_file for returning the complete
    // details. Please consider doing it in your kioslave if you're using this one as a model :)
    UDSAtom atom;
    atom.m_uds = KIO::UDS_NAME;
    atom.m_str = filename;
    entry.append( atom );

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

            atom.m_uds = KIO::UDS_LINK_DEST;
            atom.m_str = QFile::decodeName( buffer2 );
            entry.append( atom );

            // A symlink -> follow it only if details>1
            if ( details > 1 && KDE_stat( path.data(), &buff ) == -1 ) {
                // It is a link pointing to nowhere
                type = S_IFMT - 1;
                access = S_IRWXU | S_IRWXG | S_IRWXO;

                atom.m_uds = KIO::UDS_FILE_TYPE;
                atom.m_long = type;
                entry.append( atom );

                atom.m_uds = KIO::UDS_ACCESS;
                atom.m_long = access;
                entry.append( atom );

                atom.m_uds = KIO::UDS_SIZE;
                atom.m_long = 0L;
                entry.append( atom );

                goto notype;

            }
        }
    } else {
        // kdWarning() << "lstat didn't work on " << path.data() << endl;
        return false;
    }

    type = buff.st_mode & S_IFMT; // extract file type
    access = buff.st_mode & 07777; // extract permissions

    atom.m_uds = KIO::UDS_FILE_TYPE;
    atom.m_long = type;
    entry.append( atom );

    atom.m_uds = KIO::UDS_ACCESS;
    atom.m_long = access;
    entry.append( atom );

    atom.m_uds = KIO::UDS_SIZE;
    atom.m_long = buff.st_size;
    entry.append( atom );

#ifdef USE_POSIX_ACL
    /* Append an atom indicating whether the file has extended acl information
     * and if withACL is specified also one with the acl itself. If it's a directory
     * and it has a default ACL, also append that. */
    appendACLAtoms( path, entry, type, withACL );
#endif

 notype:
    atom.m_uds = KIO::UDS_MODIFICATION_TIME;
    atom.m_long = buff.st_mtime;
    entry.append( atom );

    atom.m_uds = KIO::UDS_USER;
    atom.m_str = getUserName( buff.st_uid );
    entry.append( atom );

    atom.m_uds = KIO::UDS_GROUP;
    atom.m_str = getGroupName( buff.st_gid );
    entry.append( atom );

    atom.m_uds = KIO::UDS_ACCESS_TIME;
    atom.m_long = buff.st_atime;
    entry.append( atom );

    // Note: buff.st_ctime isn't the creation time !
    // We made that mistake for KDE 2.0, but it's in fact the
    // "file status" change time, which we don't care about.

    return true;
}

void FileProtocol::stat( const KURL & url )
{
    if (!url.isLocalFile()) {
        KURL redir(url);
	redir.setProtocol(config()->readEntry("DefaultRemoteProtocol", "smb"));
	redirection(redir);
	kdDebug(7101) << "redirecting to " << redir.url() << endl;
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
    Q3CString _path( QFile::encodeName(url.path(-1)));

    QString sDetails = metaData(QLatin1String("details"));
    int details = sDetails.isEmpty() ? 2 : sDetails.toInt();
    kdDebug(7101) << "FileProtocol::stat details=" << details << endl;

    UDSEntry entry;
    if ( !createUDSEntry( url.fileName(), _path, entry, details, true /*with acls*/ ) )
    {
        error( KIO::ERR_DOES_NOT_EXIST, url.path(-1) );
        return;
    }
#if 0
///////// debug code
    KIO::UDSEntry::ConstIterator it = entry.begin();
    for( ; it != entry.end(); it++ ) {
        switch ((*it).m_uds) {
            case KIO::UDS_FILE_TYPE:
                kdDebug(7101) << "File Type : " << (mode_t)((*it).m_long) << endl;
                break;
            case KIO::UDS_ACCESS:
                kdDebug(7101) << "Access permissions : " << (mode_t)((*it).m_long) << endl;
                break;
            case KIO::UDS_USER:
                kdDebug(7101) << "User : " << ((*it).m_str.ascii() ) << endl;
                break;
            case KIO::UDS_GROUP:
                kdDebug(7101) << "Group : " << ((*it).m_str.ascii() ) << endl;
                break;
            case KIO::UDS_NAME:
                kdDebug(7101) << "Name : " << ((*it).m_str.ascii() ) << endl;
                //m_strText = decodeFileName( (*it).m_str );
                break;
            case KIO::UDS_URL:
                kdDebug(7101) << "URL : " << ((*it).m_str.ascii() ) << endl;
                break;
            case KIO::UDS_MIME_TYPE:
                kdDebug(7101) << "MimeType : " << ((*it).m_str.ascii() ) << endl;
                break;
            case KIO::UDS_LINK_DEST:
                kdDebug(7101) << "LinkDest : " << ((*it).m_str.ascii() ) << endl;
                break;
            case KIO::UDS_EXTENDED_ACL:
                kdDebug(7101) << "Contains extended ACL " << endl;
                break;
        }
        }
    MetaData::iterator it1 = mOutgoingMetaData.begin();
    for ( ; it1 != mOutgoingMetaData.end(); it1++ ) {
        kdDebug(7101) << it1.key() << " = " << it1.data() << endl;
    }
/////////
#endif
    statEntry( entry );

    finished();
}

void FileProtocol::listDir( const KURL& url)
{
    kdDebug(7101) << "========= LIST " << url.url() << " =========" << endl;
    if (!url.isLocalFile()) {
        KURL redir(url);
	redir.setProtocol(config()->readEntry("DefaultRemoteProtocol", "smb"));
	redirection(redir);
	kdDebug(7101) << "redirecting to " << redir.url() << endl;
	finished();
	return;
    }

    Q3CString _path( QFile::encodeName(url.path()));

    KDE_struct_stat buff;
    if ( KDE_stat( _path.data(), &buff ) == -1 ) {
	error( KIO::ERR_DOES_NOT_EXIST, url.path() );
	return;
    }

    if ( !S_ISDIR( buff.st_mode ) ) {
	error( KIO::ERR_IS_FILE, url.path() );
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
                   i18n( "No media in device for %1" ).arg( url.path() ) );
            break;
#endif
        default:
            error( KIO::ERR_CANNOT_ENTER_DIRECTORY, url.path() );
            break;
        }
	return;
    }

    // Don't make this a QStringList. The locale file name we get here
    // should be passed intact to createUDSEntry to avoid problems with
    // files where QFile::encodeName(QFile::decodeName(a)) != a.
    Q3StrList entryNames;

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
    Q3StrListIterator it(entryNames);
    for (; it.current(); ++it) {
        entry.clear();
        if ( createUDSEntry( QFile::decodeName(*it), 
                             *it /* we can use the filename as relative path*/, 
                             entry, 2, true ) )
          listEntry( entry, false);
        else
          ;//Well, this should never happen... but with wrong encoding names
    }

    listEntry( entry, true ); // ready

    kdDebug(7101) << "============= COMPLETED LIST ============" << endl;

    chdir(path_buffer);

    finished();
}

/*
void FileProtocol::testDir( const QString& path )
{
    Q3CString _path( QFile::encodeName(path));
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
	Q_INT8 iRo;

	stream >> iRo >> fstype >> dev >> point;

	bool ro = ( iRo != 0 );

	kdDebug(7101) << "MOUNTING fstype=" << fstype << " dev=" << dev << " point=" << point << " ro=" << ro << endl;
	bool ok = pmount( dev );
	if (ok)
	    finished();
	else
	    mount( ro, fstype.ascii(), dev, point );

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
  kdDebug(7101) << "FileProtocol::slotProcessedSize (" << (unsigned int) bytes << ")" << endl;
  processedSize( bytes );
}

// Connected to KShred
void FileProtocol::slotInfoMessage( const QString & msg )
{
  kdDebug(7101) << "FileProtocol::slotInfoMessage (" << msg << ")" << endl;
  infoMessage( msg );
}

void FileProtocol::mount( bool _ro, const char *_fstype, const QString& _dev, const QString& _point )
{
    kdDebug(7101) << "FileProtocol::mount _fstype=" << _fstype << endl;
    Q3CString buffer;

#ifdef HAVE_VOLMGT
	/*
	 *  support for Solaris volume management
	 */
	QString err;
	Q3CString devname = QFile::encodeName( _dev );

	if( volmgt_running() ) {
//		kdDebug(7101) << "VOLMGT: vold ok." << endl;
		if( volmgt_check( devname.data() ) == 0 ) {
			kdDebug(7101) << "VOLMGT: no media in "
					<< devname.data() << endl;
			err = i18n("No Media inserted or Media not recognized.");
			error( KIO::ERR_COULD_NOT_MOUNT, err );
			return;
		} else {
			kdDebug(7101) << "VOLMGT: " << devname.data()
				<< ": media ok" << endl;
			finished();
			return;
		}
	} else {
		err = i18n("\"vold\" is not running.");
		kdDebug(7101) << "VOLMGT: " << err << endl;
		error( KIO::ERR_COULD_NOT_MOUNT, err );
		return;
	}
#else


    KTempFile tmpFile;
    Q3CString tmpFileC = QFile::encodeName(tmpFile.name());
    const char *tmp = tmpFileC.data();
    Q3CString dev;
    if ( _dev.startsWith( "LABEL=" ) ) { // turn LABEL=foo into -L foo (#71430)
        QString labelName = _dev.mid( 6 );
        dev = "-L ";
        dev += QFile::encodeName( KProcess::quote( labelName ) ); // is it correct to assume same encoding as filesystem?
    } else if ( _dev.startsWith( "UUID=" ) ) { // and UUID=bar into -U bar
        QString uuidName = _dev.mid( 5 );
        dev = "-U ";
        dev += QFile::encodeName( KProcess::quote( uuidName ) );
    }
    else
        dev = QFile::encodeName( KProcess::quote(_dev) ); // get those ready to be given to a shell

    Q3CString point = QFile::encodeName( KProcess::quote(_point) );
    bool fstype_empty = !_fstype || !*_fstype;
    Q3CString fstype = KProcess::quote(_fstype).latin1(); // good guess
    Q3CString readonly = _ro ? "-r" : "";
    QString epath = QLatin1String(getenv("PATH"));
    QString path = QLatin1String("/sbin:/bin");
    if(!epath.isEmpty())
        path += QLatin1String(":") + epath;
    QString mountProg = KGlobal::dirs()->findExe("mount", path);
    if (mountProg.isEmpty()){
      error( KIO::ERR_COULD_NOT_MOUNT, i18n("Could not find program \"mount\""));
      return;
    }

    // Two steps, in case mount doesn't like it when we pass all options
    for ( int step = 0 ; step <= 1 ; step++ )
    {
        // Mount using device only if no fstype nor mountpoint (KDE-1.x like)
        if ( !_dev.isEmpty() && _point.isEmpty() && fstype_empty )
            buffer.sprintf( "%s %s 2>%s", mountProg.latin1(), dev.data(), tmp );
        else
          // Mount using the mountpoint, if no fstype nor device (impossible in first step)
          if ( !_point.isEmpty() && _dev.isEmpty() && fstype_empty )
            buffer.sprintf( "%s %s 2>%s", mountProg.latin1(), point.data(), tmp );
          else
            // mount giving device + mountpoint but no fstype
            if ( !_point.isEmpty() && !_dev.isEmpty() && fstype_empty )
              buffer.sprintf( "%s %s %s %s 2>%s", mountProg.latin1(), readonly.data(), dev.data(), point.data(), tmp );
            else
              // mount giving device + mountpoint + fstype
#if defined(__svr4__) && defined(__sun__) // MARCO for Solaris 8 and I
                // believe this is true for SVR4 in general
                buffer.sprintf( "%s -F %s %s %s %s 2>%s"
				mountProg.latin1()
                                fstype.data()
                                _ro ? "-oro" : ""
                                dev.data()
                                point.data()
                                tmp );
#else
              buffer.sprintf( "%s %s -t %s %s %s 2>%s", mountProg.latin1(), readonly.data(),
                              fstype.data(), dev.data(), point.data(), tmp );
#endif

        kdDebug(7101) << buffer << endl;

        int mount_ret = system( buffer.data() );

        QString err = testLogFile( tmp );
        if ( err.isEmpty() && mount_ret == 0)
        {
            finished();
            return;
        }
        else
        {
            // Didn't work - or maybe we just got a warning
            QString mp = KIO::findDeviceMountPoint( _dev );
            // Is the device mounted ?
            if ( !mp.isEmpty() && mount_ret == 0)
            {
                kdDebug(7101) << "mount got a warning: " << err << endl;
                warning( err );
                finished();
                return;
            }
            else
            {
                if ( (step == 0) && !_point.isEmpty())
                {
                    kdDebug(7101) << err << endl;
                    kdDebug(7101) << "Mounting with those options didn't work, trying with only mountpoint" << endl;
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
    Q3CString buffer;

    KTempFile tmpFile;
    Q3CString tmpFileC = QFile::encodeName(tmpFile.name());
    QString err;
    const char *tmp = tmpFileC.data();

#ifdef HAVE_VOLMGT
	/*
	 *  support for Solaris volume management
	 */
	char *devname;
	char *ptr;
	FILE *mnttab;
	struct mnttab mnt;

	if( volmgt_running() ) {
		kdDebug(7101) << "VOLMGT: looking for "
			<< _point.toLocal8Bit() << endl;

		if( (mnttab = KDE_fopen( MNTTAB, "r" )) == NULL ) {
			err = "couldn't open mnttab";
			kdDebug(7101) << "VOLMGT: " << err << endl;
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
			kdDebug(7101) << "VOLMGT: "
				<< QFile::encodeName(_point).data()
				<< ": " << err << endl;
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
                Q3CString qdevname(QFile::encodeName(KProcess::quote(QFile::decodeName(Q3CString(devname)))).data());
		buffer.sprintf( "/usr/bin/eject %s 2>%s", qdevname.data(), tmp );
		kdDebug(7101) << "VOLMGT: eject " << qdevname << endl;

		/*
		 *  from eject(1): exit status == 0 => need to manually eject
		 *                 exit status == 4 => media was ejected
		 */
//		if( WEXITSTATUS( system( buffer.toLocal8Bit() )) == 4 ) {
		if( WEXITSTATUS( system( buffer.data() )) == 4 ) {  // Fix for QString -> Q3CString?
			/*
			 *  this is not an error, so skip "testLogFile()"
			 *  to avoid wrong/confusing error popup
			 */
			unlink( tmp );
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
		kdDebug(7101) << "VOLMGT: " << err << endl;
		error( KIO::ERR_COULD_NOT_UNMOUNT, err );
		return;
	}
#else
    QString epath = getenv("PATH");
    QString path = QLatin1String("/sbin:/bin");
    if (!epath.isEmpty())
       path += ":" + epath;
    QString umountProg = KGlobal::dirs()->findExe("umount", path);

    if (umountProg.isEmpty()) {
        error( KIO::ERR_COULD_NOT_UNMOUNT, i18n("Could not find program \"umount\""));
        return;
    }
    buffer.sprintf( "%s %s 2>%s", umountProg.latin1(), QFile::encodeName(KProcess::quote(_point)).data(), tmp );
    system( buffer.data() );
#endif /* HAVE_VOLMGT */

    err = testLogFile( tmp );
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
    QString epath = getenv("PATH");
    QString path = QLatin1String("/sbin:/bin");
    if (!epath.isEmpty())
        path += ":" + epath;
    QString pmountProg = KGlobal::dirs()->findExe("pmount", path);

    if (pmountProg.isEmpty())
        return false;

    Q3CString buffer;
    buffer.sprintf( "%s %s", QFile::encodeName(pmountProg).data(),
                    QFile::encodeName(KProcess::quote(dev)).data() );

    int res = system( buffer.data() );

    return res==0;
}

bool FileProtocol::pumount(const QString &point)
{
    QString real_point = KStandardDirs::realPath(point);

    KMountPoint::List mtab = KMountPoint::currentMountPoints();

    KMountPoint::List::const_iterator it = mtab.begin();
    KMountPoint::List::const_iterator end = mtab.end();

    QString dev;

    for (; it!=end; ++it)
    {
        QString tmp = (*it)->mountedFrom();
        QString mp = (*it)->mountPoint();
        mp = KStandardDirs::realPath(mp);

        if (mp==real_point)
            dev = KStandardDirs::realPath(tmp);
    }

    if (dev.isEmpty()) return false;

    QString epath = getenv("PATH");
    QString path = QLatin1String("/sbin:/bin");
    if (!epath.isEmpty())
        path += ":" + epath;
    QString pumountProg = KGlobal::dirs()->findExe("pumount", path);

    if (pumountProg.isEmpty())
        return false;

    Q3CString buffer;
    buffer.sprintf( "%s %s", QFile::encodeName(pumountProg).data(),
                    QFile::encodeName(KProcess::quote(dev)).data() );

    int res = system( buffer.data() );

    return res==0;
}

/*************************************
 *
 * Utilities
 *
 *************************************/

static QString testLogFile( const char *_filename )
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
	result = i18n("Could not read %1").arg(QFile::decodeName(_filename));
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
#ifdef USE_POSIX_ACL

static bool isExtendedACL( acl_t acl )
{
    return ( acl_equiv_mode( acl, 0 ) != 0 );
}

static void appendACLAtoms( const Q3CString & path, UDSEntry& entry, mode_t type, bool withACL )
{
    // first check for a noop
    if ( acl_extended_file( path.data() ) == 0 ) return;

    acl_t acl = 0;
    acl_t defaultAcl = 0;
    UDSAtom atom;
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
      kdDebug(7101) << path.data() << " has extended ACL entries " << endl;
      atom.m_uds = KIO::UDS_EXTENDED_ACL;
      atom.m_long = 1;
      entry.append( atom );
    }
    if ( withACL ) {
        if ( acl ) {
            ssize_t size = acl_size( acl );
            atom.m_uds = KIO::UDS_ACL_STRING;
            atom.m_str = QString::fromLatin1( acl_to_text( acl, &size ) );
            entry.append( atom );
            kdDebug(7101) << path.data() << "ACL: " << atom.m_str << endl;
        }
        if ( defaultAcl ) {
            ssize_t size = acl_size( defaultAcl );
            atom.m_uds = KIO::UDS_DEFAULT_ACL_STRING;
            atom.m_str = QString::fromLatin1( acl_to_text( defaultAcl, &size ) );
            entry.append( atom );
            kdDebug(7101) << path.data() << "DEFAULT ACL: " << atom.m_str << endl;
        }
    }
    if ( acl ) acl_free( acl );
    if ( defaultAcl ) acl_free( defaultAcl );
}
#endif

#include "file.moc"
