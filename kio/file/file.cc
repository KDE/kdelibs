// $Id$

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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
#include <unistd.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <iostream.h>

#include <qvaluelist.h>

#include <kshred.h>
#include <kdebug.h>
#include <kurl.h>
#include <kprotocolmanager.h>
#include <kinstance.h>
#include <ktempfile.h>
#include <klocale.h>
#include <qfile.h>
#include "file.h"
#include <limits.h>

using namespace KIO;

#define MAX_IPC_SIZE (1024*32)

QString testLogFile( const char *_filename );

extern "C" { int kdemain(int argc, char **argv); }

int kdemain( int argc, char **argv )
{
  KInstance instance( "kio_file" );

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

void FileProtocol::chmod( const QString& path, int permissions )
{
    QCString _path( QFile::encodeName(path));
    if ( ::chmod( _path.data(), permissions ) == -1 )
        error( KIO::ERR_CANNOT_CHMOD, path );
    else
        finished();
}

void FileProtocol::mkdir( const QString& path, int permissions )
{
    QCString _path( QFile::encodeName(path));
    struct stat buff;
    if ( ::stat( _path.data(), &buff ) == -1 ) {
	if ( ::mkdir( _path.data(), S_IRWXU ) != 0 ) {
	    if ( errno == EACCES ) {
		error( KIO::ERR_ACCESS_DENIED, path );
		return;
	    } else {
		error( KIO::ERR_COULD_NOT_MKDIR, path );
		return;
	    }
	} else {
	    if ( permissions != -1 )
	        chmod( _path.data(), permissions );
	    else
	        finished();
	    return;
	}
    }

    if ( S_ISDIR( buff.st_mode ) ) {
	debug("ERR_DIR_ALREADY_EXIST");
	error( KIO::ERR_DIR_ALREADY_EXIST, path );
	return;
    }
    error( KIO::ERR_FILE_ALREADY_EXIST, path );
    return;
}

void FileProtocol::get( const QString& path, const QString& /*query*/, bool /* reload */)
{
    QCString _path( QFile::encodeName(path));
    struct stat buff;
    if ( ::stat( _path.data(), &buff ) == -1 ) {
        if ( errno == EACCES )
           error( KIO::ERR_ACCESS_DENIED, path );
        else
           error( KIO::ERR_DOES_NOT_EXIST, path );
	return;
    }

    if ( S_ISDIR( buff.st_mode ) ) {
	error( KIO::ERR_IS_DIRECTORY, path );
	return;
    }

    int fd = open( _path.data(), O_RDONLY);
    if ( fd < 0 ) {
	error( KIO::ERR_CANNOT_OPEN_FOR_READING, path );
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
          error( KIO::ERR_COULD_NOT_READ, path);
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

void FileProtocol::put( const QString& dest_orig, int _mode, bool _overwrite, bool _resume )
{
    QCString _dest_orig( QFile::encodeName(dest_orig));
    kdDebug( 7101 ) << "Put " << dest_orig << endl;
    QString dest_part( dest_orig );
    dest_part += QString::fromLatin1(".part");
    QCString _dest_part( QFile::encodeName(dest_part));

    bool bMarkPartial = KProtocolManager::self().markPartial();

    struct stat buff_orig;
    bool orig_exists = ( ::stat( _dest_orig.data(), &buff_orig ) != -1 );
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

        struct stat buff_part;
        bool part_exists = ( ::stat( _dest_part.data(), &buff_part ) != -1 );
        if ( part_exists && !_resume )
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
        fd = open( _dest.data(), O_RDWR );  // append if resuming
        lseek(fd, 0, SEEK_END); // Seek to end
    } else {
        // WABA: Make sure that we keep writing permissions ourselves,
        // otherwise we can be in for a surprise on NFS.
        mode_t initialMode;
        if (_mode != -1)
           initialMode = _mode | S_IWUSR | S_IRUSR;
        else
           initialMode = 0666;

        fd = open(_dest.data(), O_CREAT | O_TRUNC | O_WRONLY, initialMode);
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
            result = -1;
            error( KIO::ERR_COULD_NOT_WRITE, dest_orig);
         }
      }
    }
    while ( result > 0 );


    if (result != 0)
    {
        close(fd);
	kdDebug(7101) << "Error during 'put'. Aborting." << endl;
        if (bMarkPartial)
        {
           struct stat buff;
           if (( ::stat( _dest.data(), &buff ) == -1 ) ||
               ( buff.st_size < KProtocolManager::self().minimumKeepSize() ))
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
           error( KIO::ERR_CANNOT_RENAME_PARTIAL, dest_orig );
           return;
       }
    }

    // set final permissions
    if ( _mode != -1 )
    {
       if (::chmod(_dest_orig.data(), _mode) != 0)
       {
           error( KIO::ERR_CANNOT_CHMOD, dest_orig );
           return;
       }
    }

    // We have done our job => finish
    finished();
}


void FileProtocol::copy( const QString &src, const QString &dest,
                         int _mode, bool _overwrite )
{
    QCString _src( QFile::encodeName(src));
    QCString _dest( QFile::encodeName(dest));
    struct stat buff_src;
    if ( ::stat( _src.data(), &buff_src ) == -1 ) {
        if ( errno == EACCES )
           error( KIO::ERR_ACCESS_DENIED, src );
        else
           error( KIO::ERR_DOES_NOT_EXIST, src );
	return;
    }

    if ( S_ISDIR( buff_src.st_mode ) ) {
	error( KIO::ERR_IS_DIRECTORY, src );
	return;
    }

    struct stat buff_dest;
    bool dest_exists = ( ::stat( _dest.data(), &buff_dest ) != -1 );
    if ( dest_exists )
    {
        if (S_ISDIR(buff_dest.st_mode))
        {
           error( KIO::ERR_DIR_ALREADY_EXIST, dest );
           return;
        }

        if (!_overwrite)
        {
           error( KIO::ERR_FILE_ALREADY_EXIST, dest );
           return;
        }
    }

    int src_fd = open( _src.data(), O_RDONLY);
    if ( src_fd < 0 ) {
	error( KIO::ERR_CANNOT_OPEN_FOR_READING, src );
	return;
    }

    // WABA: Make sure that we keep writing permissions ourselves,
    // otherwise we can be in for a surprise on NFS.
    mode_t initialMode;
    if (_mode != -1)
       initialMode = _mode | S_IWUSR;
    else
       initialMode = 0666;

    int dest_fd = open(_dest.data(), O_CREAT | O_TRUNC | O_WRONLY, initialMode);
    if ( dest_fd < 0 ) {
	kdDebug(7101) << "####################### COULD NOT WRITE " << dest << endl;
        if ( errno == EACCES ) {
            error( KIO::ERR_WRITE_ACCESS_DENIED, dest );
        } else {
            error( KIO::ERR_CANNOT_OPEN_FOR_WRITING, dest );
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
          error( KIO::ERR_COULD_NOT_READ, src);
          close(src_fd);
          close(dest_fd);
          return;
       }
       if (n == 0)
          break; // Finished

       if (write_all( dest_fd, buffer, n))
       {
          error( KIO::ERR_COULD_NOT_WRITE, dest);
          close(src_fd);
          close(dest_fd);
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
        error( KIO::ERR_COULD_NOT_WRITE, dest);
        return;
    }

    // set final permissions
    if ( _mode != -1 )
    {
       if (::chmod(_dest.data(), _mode) != 0)
       {
           error( KIO::ERR_CANNOT_CHMOD, dest );
           return;
       }
    }

    processedSize( buff_src.st_size );
    time_t t = time( 0L );
    if ( t - t_start >= 1 )
	speed( processed_size / ( t - t_start ) );

    finished();
}

void FileProtocol::rename( const QString &src, const QString &dest,
                           bool _overwrite )
{
    QCString _src( QFile::encodeName(src));
    QCString _dest( QFile::encodeName(dest));
    struct stat buff_src;
    if ( ::stat( _src.data(), &buff_src ) == -1 ) {
        if ( errno == EACCES )
           error( KIO::ERR_ACCESS_DENIED, src );
        else
           error( KIO::ERR_DOES_NOT_EXIST, src );
	return;
    }

    struct stat buff_dest;
    bool dest_exists = ( ::stat( _dest.data(), &buff_dest ) != -1 );
    if ( dest_exists )
    {
        if (S_ISDIR(buff_dest.st_mode))
        {
           error( KIO::ERR_DIR_ALREADY_EXIST, dest );
           return;
        }

        if (!_overwrite)
        {
           error( KIO::ERR_FILE_ALREADY_EXIST, dest );
           return;
        }
    }

    if ( ::rename( _src.data(), _dest.data()))
    {
        if (( errno == EACCES ) || (errno == EPERM)) {
            error( KIO::ERR_ACCESS_DENIED, dest );
        }
        else if (errno == EXDEV) {
           error( KIO::ERR_UNSUPPORTED_ACTION, "rename");
        }
        else {
           error( KIO::ERR_CANNOT_RENAME, src );
        }
        return;
    }

    finished();
}


void FileProtocol::del( const QString& path, bool isfile)
{
    QCString _path( QFile::encodeName(path));
    /*****
     * Delete files
     *****/

    if (isfile) {
	kdDebug( 7101 ) <<  "Deleting file "<< path << endl;
	
	// TODO deletingFile( source );
	
	if ( unlink( _path.data() ) == -1 ) {
            if ((errno == EACCES) || (errno == EPERM))
               error( KIO::ERR_ACCESS_DENIED, path);
            else if (errno == EISDIR)
               error( KIO::ERR_IS_DIRECTORY, path);
            else
               error( KIO::ERR_CANNOT_DELETE, path );
	    return;
	}
    } else {
	
	/*****
	 * Delete empty directory
	 *****/
	
	kdDebug(7101) << "Deleting directory " << path << endl;

	// TODO deletingFile( source );
	

      /*****
       * Delete empty directory
       *****/
	
      kdDebug( 7101 ) << "Deleting directory " << path << endl;

      if ( ::rmdir( _path.data() ) == -1 ) {
	if ((errno == EACCES) || (errno == EPERM))
	  error( KIO::ERR_ACCESS_DENIED, path);
	else {
	  kdDebug( 7101 ) << "could not rmdir " << perror << endl;
	  error( KIO::ERR_COULD_NOT_RMDIR, path );
	  return;
	}
      }
    }

    finished();
}

void FileProtocol::createUDSEntry( const QString & filename, const QString & path, UDSEntry & entry  )
{
    QCString _path( QFile::encodeName(path));
    assert(entry.count() == 0); // by contract :-)
    UDSAtom atom;
    atom.m_uds = KIO::UDS_NAME;
    atom.m_str = filename;
    entry.append( atom );
	
    mode_t type;
    mode_t access;
    struct stat buff;

	if ( lstat( _path.data(), &buff ) == 0 )  {
	
	    if (S_ISLNK(buff.st_mode)) {

		char buffer2[ 1000 ];
		int n = readlink( _path.data(), buffer2, 1000 );
		if ( n != -1 ) {
		    buffer2[ n ] = 0;
                }

		atom.m_uds = KIO::UDS_LINK_DEST;
		atom.m_str = buffer2;
		entry.append( atom );

		// A link poiting to nowhere ?
		if ( ::stat( _path.data(), &buff ) == -1 ) {
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
		usercache.insert( uid, new QString(user->pw_name) );
		atom.m_str = user->pw_name;
	    }
	    else
		atom.m_str = "???";
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
		groupcache.insert( gid, new QString(grp->gr_name) );
		atom.m_str = grp->gr_name;
	    }
	    else
		atom.m_str = "???";
	}
	else
	    atom.m_str = *temp;
	entry.append( atom );

	atom.m_uds = KIO::UDS_ACCESS_TIME;
	atom.m_long = buff.st_atime;
	entry.append( atom );

	atom.m_uds = KIO::UDS_CREATION_TIME;
	atom.m_long = buff.st_ctime;
	entry.append( atom );
}

void FileProtocol::stat( const QString & path, const QString& /*query*/ )
{
    QCString _path( QFile::encodeName(path));
    struct stat buff;
    if ( ::lstat( _path.data(), &buff ) == -1 ) {
	error( KIO::ERR_DOES_NOT_EXIST, path );
	return;
    }

    // Extract filename out of path
    KURL url;
    url.setPath(path);
    QString filename = url.fileName();

    UDSEntry entry;
    createUDSEntry( filename, path, entry );
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
    statEntry( entry );

    finished();
}

void FileProtocol::listDir( const QString& path, const QString& /*query*/ )
{
    QCString _path( QFile::encodeName(path));
    kdDebug(7101) << "=============== LIST " << path << " ===============" << endl;

    struct stat buff;
    if ( ::stat( _path.data(), &buff ) == -1 ) {
	error( KIO::ERR_DOES_NOT_EXIST, path );
	return;
    }

    if ( !S_ISDIR( buff.st_mode ) ) {
	error( KIO::ERR_IS_FILE, path );
	return;
    }

    DIR *dp = 0L;
    struct dirent *ep;

    dp = opendir( _path.data() );
    if ( dp == 0 ) {
	error( KIO::ERR_CANNOT_ENTER_DIRECTORY, path );
	return;
    }

    QStringList entryNames;

    while ( ( ep = readdir( dp ) ) != 0L )
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
    QStringList::Iterator it (entryNames.begin());

    for (; it != entryNames.end(); ++it) {
	entry.clear();
        createUDSEntry( *it, *it /* we can use the filename as relative path*/, entry );
	listEntry( entry, false);
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
    struct stat buff;
    if ( ::stat( _path.data(), &buff ) == -1 ) {
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
	
	kdDebug(7101) << "!!!!!!!!! MOUNTING " << fstype << " " << dev << " " << point << endl;
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
      assert(0);
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

void FileProtocol::mount( bool _ro, const char *_fstype, const QString& _dev, const QString& _point )
{
    kdDebug(7101) << "FileProtocol::mount" << endl;
    QString buffer;

    KTempFile tmpFile;
    QCString tmpFileC = QFile::encodeName(tmpFile.name());
    const char *tmp = tmpFileC.data();
    QCString dev = QFile::encodeName( _dev );
    QCString point = QFile::encodeName( _point );
    QCString fstype = _fstype;
    QCString readonly = _ro ? "-r" : "";

    // Two steps, in case mount doesn't like it when we pass all options
    for ( int step = 0 ; step <= 1 ; step++ )
    {
        // Mount using device only if no fstype nor mountpoint (KDE-1.x like)
        if ( !dev.isEmpty() && point.isEmpty() && fstype.isEmpty() )
            buffer.sprintf( "mount %s 2>%s", dev.data(), tmp );
        else
          // Mount using the mountpoint, if no fstype nor device (impossible in first step)
          if ( !point.isEmpty() && dev.isEmpty() && fstype.isEmpty() )
            buffer.sprintf( "mount %s 2>%s", point.data(), tmp );
          else
            // mount giving device + mountpoint but no fstype
            if ( !point.isEmpty() && !dev.isEmpty() && fstype.isEmpty() )
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
}


void FileProtocol::unmount( const QString& _point )
{
    QString buffer;

    KTempFile tmpFile;
    QCString tmpFileC = QFile::encodeName(tmpFile.name());
    const char *tmp = tmpFileC.data();

    buffer.sprintf( "umount %s 2>%s", QFile::encodeName(_point).data(), tmp );
    system( buffer.ascii() );

    QString err = testLogFile( tmp );
    if ( err.isEmpty() )
	finished();
    else
        error( KIO::ERR_COULD_NOT_MOUNT, err );
}

/*************************************
 *
 * Utilities
 *
 *************************************/

QString testLogFile( const char *_filename )
{
    char buffer[ 1024 ];
    struct stat buff;

    QString result;

    stat( _filename, &buff );
    int size = buff.st_size;
    if ( size == 0 ) {
	unlink( _filename );
	return result;
    }

    FILE * f = fopen( _filename, "rb" );
    if ( f == 0L ) {
	unlink( _filename );
	result = i18n("Could not read %1").arg(_filename);
	return result;
    }

    result = "";
    const char *p = "";
    while ( p != 0L ) {
	p = fgets( buffer, sizeof(buffer)-1, f );
	if ( p != 0L )
	    result += buffer;
    }

    fclose( f );

    unlink( _filename );

    return result;
}

#include "file.moc"
