// $Id$

#include <config.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
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

#include <qvaluelist.h>

#include <kshred.h>
#include <kdebug.h>
#include <kurl.h>
#include <kinstance.h>
#include <ksimpleconfig.h>
#include <ktempfile.h>
#include <klocale.h>
#include <qfile.h>
#include <qstrlist.h>
#include "file.h"
#include <limits.h>

#ifdef HAVE_VOLMGT
#include <volmgt.h>
#include <sys/mnttab.h>
#endif

#include <kio/ioslave_defaults.h>
#include <kglobal.h>

#ifdef _LFS64_LARGEFILE
#define KDE_stat		::stat64
#define KDE_lstat		::lstat64
#define KDE_fstat		::fstat64
#define KDE_open		::open64
#define KDE_lseek		::lseek64
#define KDE_readdir		::readdir64
#define KDE_struct_stat 	struct stat64
#define KDE_struct_dirent	struct dirent64
#else
#define KDE_stat		::stat
#define KDE_lstat		::lstat
#define KDE_fstat		::fstat
#define KDE_open		::open
#define KDE_lseek		::lseek
#define KDE_readdir		::readdir
#define KDE_struct_stat 	struct stat
#define KDE_struct_dirent	struct dirent
#endif

#ifdef _LFS64_STDIO
#define KDE_fopen		fopen64
#else
#define KDE_fopen		fopen
#endif

using namespace KIO;

#define MAX_IPC_SIZE (1024*32)

QString testLogFile( const char *_filename );

extern "C" { int kdemain(int argc, char **argv); }

int kdemain( int argc, char **argv )
{
  KLocale::setMainCatalogue("kdelibs");
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


FileProtocol::FileProtocol( const QCString &pool, const QCString &app ) : SlaveBase( "file", pool, app )
{
    usercache.setAutoDelete( TRUE );
    groupcache.setAutoDelete( TRUE );
}

void FileProtocol::chmod( const KURL& url, int permissions )
{
    QCString _path( QFile::encodeName(url.path()));
    if ( ::chmod( _path.data(), permissions ) == -1 )
        error( KIO::ERR_CANNOT_CHMOD, url.path() );
    else
        finished();
}

void FileProtocol::mkdir( const KURL& url, int permissions )
{
    QCString _path( QFile::encodeName(url.path()));
    KDE_struct_stat buff;
    if ( KDE_stat( _path.data(), &buff ) == -1 ) {
	if ( ::mkdir( _path.data(), 0777 /*umask will be applied*/ ) != 0 ) {
	    if ( errno == EACCES ) {
		error( KIO::ERR_ACCESS_DENIED, url.path() );
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
	kdDebug() << "ERR_DIR_ALREADY_EXIST" << endl;
	error( KIO::ERR_DIR_ALREADY_EXIST, url.path() );
	return;
    }
    error( KIO::ERR_FILE_ALREADY_EXIST, url.path() );
    return;
}

void FileProtocol::get( const KURL& url )
{
    QCString _path( QFile::encodeName(url.path()));
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

    totalSize( buff.st_size );
    int processed_size = 0;
    time_t t_start = time( 0L );
    time_t t_last = t_start;

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
       time_t t = time( 0L );
       if ( t - t_last >= 1 )
       {
          processedSize( processed_size );
          speed( processed_size / ( t - t_start ) );
          t_last = t;
       }
    }

    data( QByteArray() );

    close( fd );

    processedSize( buff.st_size );
    time_t t = time( 0L );
    if ( t - t_start >= 1 )
	speed( processed_size / ( t - t_start ) );

    finished();
}

static int
write_all(int fd, const char *buf, size_t len)
{
   while (len > 0)
   {
      int written = write(fd, buf, len);
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
    QCString _dest_orig( QFile::encodeName(dest_orig));
    kdDebug( 7101 ) << "Put " << dest_orig << endl;
    QString dest_part( dest_orig );
    dest_part += QString::fromLatin1(".part");
    QCString _dest_part( QFile::encodeName(dest_part));

    bool bMarkPartial = config()->readBoolEntry("MarkPartial", true);

    KDE_struct_stat buff_orig;
    bool orig_exists = ( KDE_stat( _dest_orig.data(), &buff_orig ) != -1 );
    if ( orig_exists &&  !_overwrite && !_resume)
    {
        if (S_ISDIR(buff_orig.st_mode))
           error( KIO::ERR_DIR_ALREADY_EXIST, dest_orig );
        else
           error( KIO::ERR_FILE_ALREADY_EXIST, dest_orig );
        return;
    }

    QString dest;
    if (bMarkPartial)
    {
        kdDebug(7101) << "Appending .part extension to " << dest_orig << endl;
        dest = dest_part;

        KDE_struct_stat buff_part;
        bool part_exists = ( KDE_stat( _dest_part.data(), &buff_part ) != -1 );
        if ( part_exists && !_resume && buff_part.st_size > 0 )
        {
            kdDebug() << "FileProtocol::put : calling canResume with " << (unsigned long)buff_part.st_size << endl;
             // Maybe we can use this partial file for resuming
             // Tell about the size we have, and the app will tell us
             // if it's ok to resume or not.
             _resume = canResume( buff_part.st_size );

             kdDebug() << "FileProtocol::put got answer " << _resume << endl;

             if (!_resume)
             {
                 kdDebug(7101) << "Deleting partial file " << dest_part << endl;
                 if ( ! remove( _dest_part.data() ) ) {
                     part_exists = false;
                 } else {
                     error( KIO::ERR_CANNOT_DELETE_PARTIAL, dest_part );
                     return;
                 }
             }
        }
    }
    else
    {
       dest = dest_orig;
       if ( orig_exists && !_resume )
        {
             kdDebug(7101) << "Deleting destination file " << dest_part << endl;
             remove( _dest_orig.data() );
             // Catch errors when we try to open the file.
        }
    }
    QCString _dest( QFile::encodeName(dest));

    int fd;

    if ( _resume ) {
        fd = KDE_open( _dest.data(), O_RDWR );  // append if resuming
        KDE_lseek(fd, 0, SEEK_END); // Seek to end
    } else {
        // WABA: Make sure that we keep writing permissions ourselves,
        // otherwise we can be in for a surprise on NFS.
        mode_t initialMode;
        if (_mode != -1)
           initialMode = _mode | S_IWUSR | S_IRUSR;
        else
           initialMode = 0666;

        fd = KDE_open(_dest.data(), O_CREAT | O_TRUNC | O_WRONLY, initialMode);
    }

    if ( fd < 0 ) {
	kdDebug(7101) << "####################### COULD NOT WRITE " << dest << _mode << endl;
	kdDebug(7101) << "errno==" << errno << "(" << strerror(errno) << ")" << endl;
        if ( errno == EACCES ) {
            error( KIO::ERR_WRITE_ACCESS_DENIED, dest );
        } else {
            error( KIO::ERR_CANNOT_OPEN_FOR_WRITING, dest );
        }
        return;
    }

    int result;
    // Loop until we got 0 (end of data)
    do
    {
      QByteArray buffer;
      dataReq(); // Request for data
      result = readData( buffer );
      if (result > 0)
      {
         if (write_all( fd, buffer.data(), buffer.size()))
         {
            if ( errno == ENOSPC ) // disk full
            {
              error( KIO::ERR_DISK_FULL, dest_orig);
              result = -2; // means: remove dest file
            }
            else
            {
              error( KIO::ERR_COULD_NOT_WRITE, dest_orig);
              result = -1;
            }
         }
      }
    }
    while ( result > 0 );


    if (result != 0)
    {
        close(fd);
	kdDebug(7101) << "Error during 'put'. Aborting." << endl;
        if (result == -2)
        {
	   remove(_dest.data());
        } else if (bMarkPartial)
        {
           KDE_struct_stat buff;
           if (( KDE_stat( _dest.data(), &buff ) == -1 ) ||
               ( buff.st_size < config()->readNumEntry("MinimumKeepSize", DEFAULT_MINIMUM_KEEP_SIZE) ))
           {
	       remove(_dest.data());
           }
        }
        ::exit(255);
    }

    if ( close(fd) )
    {
        error( KIO::ERR_COULD_NOT_WRITE, dest_orig);
        return;
    }

    // after full download rename the file back to original name
    if ( bMarkPartial )
    {
       if ( ::rename( _dest.data(), _dest_orig.data() ) )
       {
           kdWarning(7101) << " Couldn't rename " << _dest << " to " << _dest_orig << endl;
           error( KIO::ERR_CANNOT_RENAME_PARTIAL, dest_orig );
           return;
       }
    }

    // set final permissions, if the file was just created
    if ( _mode != -1 && !orig_exists )
    {
       if (::chmod(_dest_orig.data(), _mode) != 0)
       {
           warning( i18n( "Could not change permissions for\n%1" ).arg( dest_orig ) );
       }
    }

    // We have done our job => finish
    finished();
}


void FileProtocol::copy( const KURL &src, const KURL &dest,
                         int _mode, bool _overwrite )
{
    QCString _src( QFile::encodeName(src.path()));
    QCString _dest( QFile::encodeName(dest.path()));
    KDE_struct_stat buff_src;
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

    int src_fd = KDE_open( _src.data(), O_RDONLY);
    if ( src_fd < 0 ) {
	error( KIO::ERR_CANNOT_OPEN_FOR_READING, src.path() );
	return;
    }

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

    totalSize( buff_src.st_size );
    int processed_size = 0;
    time_t t_start = time( 0L );
    time_t t_last = t_start;

    char buffer[ MAX_IPC_SIZE ];
    QByteArray array;

    while( 1 )
    {
       int n = ::read( src_fd, buffer, MAX_IPC_SIZE );
       if (n == -1)
       {
          if (errno == EINTR)
              continue;
          error( KIO::ERR_COULD_NOT_READ, src.path());
          close(src_fd);
          close(dest_fd);
          return;
       }
       if (n == 0)
          break; // Finished

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
              error( KIO::ERR_COULD_NOT_WRITE, dest.path());
          }
          return;
       }

       processed_size += n;
       time_t t = time( 0L );
       if ( t - t_last >= 1 )
       {
          processedSize( processed_size );
	  speed( processed_size / ( t - t_start ) );
	  t_last = t;
       }
    }

    close( src_fd );

    if (close( dest_fd))
    {
        error( KIO::ERR_COULD_NOT_WRITE, dest.path());
        return;
    }

    // set final permissions
    if ( _mode != -1 )
    {
       if (::chmod(_dest.data(), _mode) != 0)
       {
           warning( i18n( "Could not change permissions for\n%1" ).arg( dest.path() ) );
       }
    }

    // copy access and modification time
    struct utimbuf ut;
    ut.actime = buff_src.st_atime;
    ut.modtime = buff_src.st_mtime;
    if ( ::utime( _dest.data(), &ut ) != 0 )
        kdWarning() << QString::fromLatin1("Couldn't preserve access and modification time for\n%1").arg( dest.path() ) << endl;

    processedSize( buff_src.st_size );
    time_t t = time( 0L );
    if ( t - t_start >= 1 )
	speed( processed_size / ( t - t_start ) );

    finished();
}

void FileProtocol::rename( const KURL &src, const KURL &dest,
                           bool _overwrite )
{
    QCString _src( QFile::encodeName(src.path()));
    QCString _dest( QFile::encodeName(dest.path()));
    KDE_struct_stat buff_src;
    if ( KDE_stat( _src.data(), &buff_src ) == -1 ) {
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
           error( KIO::ERR_UNSUPPORTED_ACTION, QString::fromLatin1("rename"));
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
            // Some error occured while we tried to symlink
            error( KIO::ERR_CANNOT_SYMLINK, dest.path() );
            return;
        }
    }
    finished();
}

void FileProtocol::del( const KURL& url, bool isfile)
{
    QCString _path( QFile::encodeName(url.path()));
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

bool FileProtocol::createUDSEntry( const QString & filename, const QCString & path, UDSEntry & entry  )
{
    assert(entry.count() == 0); // by contract :-)
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
		atom.m_str = QString::fromLocal8Bit( buffer2 );
		entry.append( atom );

		// A link poiting to nowhere ?
		if ( KDE_stat( path.data(), &buff ) == -1 ) {
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
            kdWarning() << "lstat didn't work on " << path.data() << endl;
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

    notype:
	atom.m_uds = KIO::UDS_MODIFICATION_TIME;
	atom.m_long = buff.st_mtime;
	entry.append( atom );

	atom.m_uds = KIO::UDS_USER;
	uid_t uid = buff.st_uid;
	QString *temp = usercache.find( uid );

	if ( !temp ) {
	    struct passwd *user = getpwuid( uid );
	    if ( user ) {
		usercache.insert( uid, new QString(QString::fromLatin1(user->pw_name)) );
		atom.m_str = user->pw_name;
	    }
	    else
		atom.m_str = QString::number( uid );
	}
	else
	    atom.m_str = *temp;
	entry.append( atom );

	atom.m_uds = KIO::UDS_GROUP;
	gid_t gid = buff.st_gid;
	temp = groupcache.find( gid );
	if ( !temp ) {
	    struct group *grp = getgrgid( gid );
	    if ( grp ) {
		groupcache.insert( gid, new QString(QString::fromLatin1(grp->gr_name)) );
		atom.m_str = grp->gr_name;
	    }
	    else
		atom.m_str = QString::number( gid );
	}
	else
	    atom.m_str = *temp;
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
    /* directories may not have a slash at the end if
     * we want to stat() them; it requires that we
     * change into it .. which may not be allowed
     * stat("/is/unaccessible")  -> rwx------
     * stat("/is/unaccessible/") -> EPERM            H.Z.
     * This is the reason for the -1
     */
    QCString _path( QFile::encodeName(url.path(-1)));
    KDE_struct_stat buff;
    if ( KDE_lstat( _path.data(), &buff ) == -1 ) {
	error( KIO::ERR_DOES_NOT_EXIST, url.path(-1) );
	return;
    }

    UDSEntry entry;
    if ( !createUDSEntry( url.fileName(), _path, entry ) )
    {
	// Should never happen
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
        }
    }
/////////
#endif
    statEntry( entry );

    finished();
}

void FileProtocol::listDir( const KURL& url)
{
    QCString _path( QFile::encodeName(url.path()));
    kdDebug(7101) << "========= LIST " << url.url() << " =========" << endl;

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
	error( KIO::ERR_CANNOT_ENTER_DIRECTORY, url.path() );
	return;
    }

    // Don't make this a QStringList. The locale file name we get here
    // should be passed intact to createUDSEntry to avoid problems with
    // files where QFile::encodeName(QFile::decodeName(a)) != a.
    QStrList entryNames;

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
    chdir( _path.data() );

    UDSEntry entry;
    QStrListIterator it(entryNames);
    for (; it.current(); ++it) {
        entry.clear();
        if ( createUDSEntry( QFile::decodeName(*it), *it /* we can use the filename as relative path*/, entry ) )
          listEntry( entry, false);
        else
          ;//Well, this should never happen... but with wrong encoding names
    }

    listEntry( entry, true ); // ready

    kdDebug(7101) << "============= COMPLETED LIST ============" << endl;

    chdir(path_buffer);

    finished();

    kdDebug(7101) << "=============== BYE ===========" << endl;
}

/*
void FileProtocol::testDir( const QString& path )
{
    QCString _path( QFile::encodeName(path));
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
    QDataStream stream(data, IO_ReadOnly);

    stream >> tmp;
    switch (tmp) {
    case 1:
      {
	QString fstype, dev, point;
	Q_INT8 iRo;

	stream >> iRo >> fstype >> dev >> point;

	bool ro = ( iRo != 0 );

	kdDebug(7101) << "MOUNTING fstype=" << fstype << " dev=" << dev << " point=" << point << " ro=" << ro << endl;
	mount( ro, fstype.ascii(), dev, point );

      }
      break;
    case 2:
      {
	QString point;
	stream >> point;
	unmount( point );
      }
      break;

    case 3:
    {
      QString filename;
      stream >> filename;
      KShred shred( filename );
      connect( &shred, SIGNAL( processedSize( unsigned long ) ),
               this, SLOT( slotProcessedSize( unsigned long ) ) );
      connect( &shred, SIGNAL( infoMessage( const QString & ) ),
               this, SLOT( slotInfoMessage( const QString & ) ) );
      if (!shred.shred())
          error( KIO::ERR_CANNOT_DELETE, filename );
      else
          finished();
      break;
    }
    default:
      break;
    }
}

// Connected to KShred
void FileProtocol::slotProcessedSize( unsigned long bytes )
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

#ifndef HAVE_VOLMGT
static QString shellQuote( const QString &_str )
{
    // Credits to Walter, says Bernd G. :)
    QString str(_str);
    str.replace(QRegExp(QString::fromLatin1("'")), QString::fromLatin1("'\"'\"'"));
    return QString::fromLatin1("'")+str+'\'';
}
#endif /* ! HAVE_VOLMGT */

void FileProtocol::mount( bool _ro, const char *_fstype, const QString& _dev, const QString& _point )
{
    kdDebug(7101) << "FileProtocol::mount _fstype=" << _fstype << endl;
    QString buffer;

#ifdef HAVE_VOLMGT
	/*
	 *  support for Solaris volume management
	 */
	QString err;
	QCString devname = QFile::encodeName( _dev );

	if( volmgt_running() ) {
//		kdDebug(7101) << "VOLMGT: vold ok." << endl;
		if( volmgt_check( devname.data() ) == 0 ) {
			kdDebug(7101) << "VOLMGT: no media in "
					<< devname.data() << endl;
			err = "No Media inserted or Media not recognized.";
			error( KIO::ERR_COULD_NOT_MOUNT, err );
			return;
		} else {
			kdDebug(7101) << "VOLMGT: " << devname.data()
				<< ": media ok" << endl;
			finished();
			return;
		}
	} else {
		err = "\"vold\" is not running.";
		kdDebug(7101) << "VOLMGT: " << err << endl;
		error( KIO::ERR_COULD_NOT_MOUNT, err );
		return;
	}
#else
    KTempFile tmpFile;
    QCString tmpFileC = QFile::encodeName(tmpFile.name());
    const char *tmp = tmpFileC.data();
    QCString dev = QFile::encodeName( shellQuote(_dev) ); // get those ready to be given to a shell
    QCString point = QFile::encodeName( shellQuote(_point) );
    QCString fstype = _fstype;
    QCString readonly = _ro ? "-r" : "";

    // Two steps, in case mount doesn't like it when we pass all options
    for ( int step = 0 ; step <= 1 ; step++ )
    {
        // Mount using device only if no fstype nor mountpoint (KDE-1.x like)
        if ( !_dev.isEmpty() && _point.isEmpty() && fstype.isEmpty() )
            buffer.sprintf( "mount %s 2>%s", dev.data(), tmp );
        else
          // Mount using the mountpoint, if no fstype nor device (impossible in first step)
          if ( !_point.isEmpty() && _dev.isEmpty() && fstype.isEmpty() )
            buffer.sprintf( "mount %s 2>%s", point.data(), tmp );
          else
            // mount giving device + mountpoint but no fstype
            if ( !_point.isEmpty() && !_dev.isEmpty() && fstype.isEmpty() )
              buffer.sprintf( "mount %s %s %s 2>%s", readonly.data(), dev.data(), point.data(), tmp );
            else
              // mount giving device + mountpoint + fstype
              buffer.sprintf( "mount %s -t %s %s %s 2>%s", readonly.data(),
                              fstype.data(), dev.data(), point.data(), tmp );

        kdDebug(7101) << buffer << endl;

        system( buffer.ascii() );

        QString err = testLogFile( tmp );
        if ( err.isEmpty() )
        {
            finished();
            return;
        }
        else
        {
            // Didn't work - or maybe we just got a warning
            QString mp = KIO::findDeviceMountPoint( _dev );
            // Is the device mounted ?
            if ( !mp.isNull() )
            {
                kdDebug(7101) << "mount got a warning: " << err << endl;
                warning( err );
                finished();
                return;
            }
            else
            {
                if ( step == 0 )
                {
                    kdDebug(7101) << err << endl;
                    kdDebug(7101) << "Mounting with those options didn't work, trying with only mountpoint" << endl;
                    fstype = "";
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
    QString buffer;

    KTempFile tmpFile;
    QCString tmpFileC = QFile::encodeName(tmpFile.name());
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
			<< _point.local8Bit() << endl;

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
			if( strcmp( _point.local8Bit(), mnt.mnt_mountp ) == 0 ){
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
		buffer.sprintf( "/usr/bin/eject %s 2>%s", devname, tmp );
		kdDebug(7101) << "VOLMGT: eject " << devname << endl;

		/*
		 *  from eject(1): exit status == 0 => need to manually eject
		 *                 exit status == 4 => media was ejected
		 */
		if( WEXITSTATUS( system( buffer.local8Bit() )) == 4 ) {
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
		err = "\"vold\" is not running.";
		kdDebug(7101) << "VOLMGT: " << err << endl;
		error( KIO::ERR_COULD_NOT_UNMOUNT, err );
		return;
	}
#else
    buffer.sprintf( "umount %s 2>%s", QFile::encodeName(_point).data(), tmp );
    system( buffer.ascii() );
#endif /* HAVE_VOLMGT */

    err = testLogFile( tmp );
    if ( err.isEmpty() )
	finished();
    else
        error( KIO::ERR_COULD_NOT_UNMOUNT, err );
}

/*************************************
 *
 * Utilities
 *
 *************************************/

QString testLogFile( const char *_filename )
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

#include "file.moc"
