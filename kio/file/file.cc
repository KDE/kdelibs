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
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include <qvaluelist.h>

#include "../kio_rename_dlg.h"
#include "../kio_skip_dlg.h"
#include <kurl.h>
#include <kprotocolmanager.h>

#include <iostream.h>

#include "file.h"

QString testLogFile( const char *_filename );
int check( KIOConnection *_con );

void sigchld_handler( int );
void sigsegv_handler( int );

int main( int , char ** )
{
  signal(SIGCHLD, KIOProtocol::sigchld_handler);
#ifdef NDEBUG
  signal(SIGSEGV, IOProtocol::sigsegv_handler);
#endif
  qDebug( "kio_file : Starting");

  KIOConnection parent( 0, 1 );
  
  FileProtocol file( &parent );
  file.dispatchLoop();

  qDebug( "kio_file : Done" );
}

FileProtocol::FileProtocol( KIOConnection *_conn ) : KIOProtocol( _conn )
{
  m_cmd = CMD_NONE;
  m_bIgnoreJobErrors = false;
}


void FileProtocol::slotMkdir( const char *_url, int _mode )
{
  KURL usrc( _url );
  if ( usrc.isMalformed() ) {
    error( ERR_MALFORMED_URL, strdup(_url) );
    m_cmd = CMD_NONE;
    return;
  }

  if ( !usrc.isLocalFile() ) {
    error( ERR_INTERNAL, "kio_file got non local name in mkdir command" );
    m_cmd = CMD_NONE;
    return;
  }

  struct stat buff;
  if ( stat( usrc.path(), &buff ) == -1 ) {
    if ( ::mkdir( usrc.path(), S_IRWXU ) != 0 ) {
      if ( errno == EACCES ) {
	error( ERR_ACCESS_DENIED, strdup(_url) );
	m_cmd = CMD_NONE;
	return;
      } else {
	error( ERR_COULD_NOT_MKDIR, strdup(_url) );
	m_cmd = CMD_NONE;
	return;
      }
    } else {
      if ( _mode != -1 )
	if ( chmod( usrc.path(), _mode ) == -1 ) {
	  error( ERR_CANNOT_CHMOD, strdup(_url) );
	  m_cmd = CMD_NONE;
	  return;
	}

      finished();
      return;
    }
  }

  if ( S_ISDIR( buff.st_mode ) ) {
    error( ERR_DOES_ALREADY_EXIST, strdup(_url) );
    m_cmd = CMD_NONE;
    return;
  }

  error( ERR_COULD_NOT_MKDIR, strdup(_url) );
  m_cmd = CMD_NONE;
  return;
}


void FileProtocol::slotCopy( QStringList& _source, const char *_dest )
{
  doCopy( _source, _dest, false );
}


void FileProtocol::slotCopy( const char* _source, const char *_dest )
{
  QStringList lst;
  lst.append( _source );
  
  doCopy( lst, _dest, true );
}


void FileProtocol::slotMove( QStringList& _source, const char *_dest )
{
  doCopy( _source, _dest, false, true );
}


void FileProtocol::slotMove( const char* _source, const char *_dest )
{
  QStringList lst;
  lst.append( _source );
  
  doCopy( lst, _dest, true, true );
}


void FileProtocol::doCopy( QStringList& _source, const char *_dest, bool _rename, bool _move )
{
  if ( _rename )
    assert( _source.count() == 1 );

  qDebug( "kio_file : Making copy to %s", _dest );

  // Check whether the URLs are wellformed
  QStringList::Iterator source_files_it = _source.begin();
  while (source_files_it != _source.end()) {    
    qDebug( "kio_file : Checking %s", (*source_files_it).ascii() );
    KURL usrc(*source_files_it);
    if ( usrc.isMalformed() ) {
      error( ERR_MALFORMED_URL, *source_files_it );
      m_cmd = CMD_NONE;
      return;
    }
    if ( !usrc.isLocalFile() ) {
      error( ERR_INTERNAL, "kio_file got non local file as source in copy command" );
      m_cmd = CMD_NONE;
      return;
    }
    ++source_files_it;
  }

  qDebug( "kio_file : All URLs ok %s", _dest );

  // Make a copy of the parameter. if we do IPC calls from here, then we overwrite
  // our argument. This is tricky! ( but saves memory and speeds things up )
  QString dest = _dest;

  // Check wellformedness of the destination
  KURL udest( dest );
  if ( udest.isMalformed() ) {
    error( ERR_MALFORMED_URL, dest );
    m_cmd = CMD_NONE;
    return;
  }

  qDebug( "kio_file : Dest ok %s", dest.ascii() );

  // Find IO server for destination
  QString exec = KProtocolManager::self().executable( udest.protocol() );
  if ( exec.isEmpty() ) {
    error( ERR_UNSUPPORTED_PROTOCOL, udest.protocol() );
    m_cmd = CMD_NONE;
    return;
  }

  // Is the left most protocol a filesystem protocol ?
  if ( KProtocolManager::self().outputType( udest.protocol() ) != KProtocolManager::T_FILESYSTEM )
  {
    error( ERR_PROTOCOL_IS_NOT_A_FILESYSTEM, udest.protocol() );
    m_cmd = CMD_NONE;
    return;
  }

  qDebug( "kio_file : IO server ok %s", dest.ascii() );

  // Get a list of all source files and directories
  QValueList<Copy> files;
  QValueList<CopyDir> dirs;
  int size = 0;
  qDebug( "kio_file : Iterating" );

  source_files_it = _source.begin();
  qDebug( "kio_file : Looping" );
  while ( source_files_it != _source.end()) {
    qDebug( "kio_file : Executing %s", (*source_files_it).ascii() );
    KURL usrc( (*source_files_it).ascii() );
    qDebug( "kio_file : Parsed URL" );
    // Did an error occur ?
    int s;
    if ( ( s = listRecursive( usrc.path(), files, dirs, _rename ) ) == -1 ) {
      // Error message is already sent
      m_cmd = CMD_NONE;
      return;
    }
    // Sum up the total amount of bytes we have to copy
    size += s;
    ++source_files_it;
  }

  qDebug( "kio_file : Recursive 1 %s", dest.data() );

  // Check whether we do not copy a directory in itself or one of its subdirectories
  struct stat buff2;
  if ( udest.isLocalFile() && stat( udest.path(), &buff2 ) == 0 ) {
    bool b_error = false;
    for ( source_files_it = _source.begin(); source_files_it != _source.end(); ++source_files_it ) {    
      KURL usrc( (*source_files_it).ascii() );

      struct stat buff1;
      // Can we stat both the source, too ? ( Should always be the case )
      if ( stat( usrc.path(), &buff1 ) == 0 ) {
	bool b_error = false;
	// Are source and dest equal ? => error
	if ( buff1.st_ino == buff2.st_ino )
	  b_error = true;
      }
    }

    if ( !b_error ) {
      // Iterate over all subdirectories
      QValueList<CopyDir>::Iterator it = dirs.begin();
      for( ; it != dirs.end() && !b_error; it++ )
	if ( buff2.st_ino == (*it).m_ino )
	  b_error = true;
    }

    // Do we have a cyclic copy now ? => error
    if ( b_error ) {
      error( ERR_CYCLIC_COPY, *source_files_it );
      m_cmd = CMD_NONE;
      return;
    }
  }

  qDebug( "kio_file : Recursive ok %s", dest.data() );

  m_cmd = CMD_GET;

  // Start a server for the destination protocol
  KIOSlave slave( exec );
  if ( slave.pid() == -1 ) {
    error( ERR_CANNOT_LAUNCH_PROCESS, exec );
    m_cmd = CMD_NONE;
    return;
  }

  // Put a protocol on top of the job
  FileIOJob job( &slave, this );

  qDebug( "kio_file : Job started ok %s", dest.ascii() );

  // Tell our client what we 'r' gonna do
  totalSize( size );
  totalFiles( files.count() );
  totalDirs( dirs.count() );

  int processed_files = 0;
  int processed_dirs = 0;
  int processed_size = 0;

  // Replace the relative destinations with absolut destinations
  // by prepending the destinations path
  QString tmp1 = udest.path( 1 );
  // Strip '/'
  QString tmp1_stripped = udest.path( -1 );

  QValueList<CopyDir>::Iterator dir_it = dirs.begin();
  while (dir_it != dirs.end()) {
    QString tmp2 = (*dir_it).m_strRelDest;
    if ( _rename )
      (*dir_it).m_strRelDest = tmp1_stripped;
    else
      (*dir_it).m_strRelDest = tmp1;
    (*dir_it).m_strRelDest += tmp2;
    dir_it++;
  }
  QValueList<Copy>::Iterator fit = files.begin();
  for( ; fit != files.end(); fit++ ) {
    QString tmp2 = (*fit).m_strRelDest;
    if ( _rename ) // !!! && fit->m_strRelDest == "" )
      (*fit).m_strRelDest = tmp1_stripped;
    else
      (*fit).m_strRelDest = tmp1;
    (*fit).m_strRelDest += tmp2;
  }

  qDebug( "kio_file : Destinations ok %s", dest.data() );

  /*****
   * Make directories
   *****/

  m_bIgnoreJobErrors = true;
  bool overwrite_all = false;
  bool auto_skip = false;
  QStringList skip_list;
  QStringList overwrite_list;
  // Create all directories
  dir_it = dirs.begin();
  while (dir_it != dirs.end()) {
    // Repeat until we got no error
    do {
      job.clearError();

      KURL ud( dest );
      ud.setPath( (*dir_it).m_strRelDest );

      QString d = ud.url();

      // Is this URL on the skip list ?
      bool skip = false;
      QStringList::Iterator sit = skip_list.begin();
      for( ; sit != skip_list.end() && !skip; sit++ )
	// Is d a subdirectory of *sit ?
	if ( strncmp( *sit, d, (*sit).length() ) == 0 )
	  skip = true;

      if ( skip )
	continue;

      // Is this URL on the overwrite list ?
      bool overwrite = false;
      QStringList::Iterator oit = overwrite_list.begin();
      for( ; oit != overwrite_list.end() && !overwrite; oit++ )
	if ( strncmp( *oit, d, (*oit).length() ) == 0 )
	  overwrite = true;

      if ( overwrite )
	continue;

      // Tell what we are doing
      makingDir( d );

      // qDebug( "kio_file : Making remote dir %s", d );
      // Create the directory
      job.mkdir( d, (*dir_it).m_mode );
      while( !job.hasFinished() )
	job.dispatch();

      // Did we have an error ?
      if ( job.hasError() ) {
	// Can we prompt the user and ask for a solution ?
	if ( /* m_bGUI && */ job.errorId() == ERR_DOES_ALREADY_EXIST )
	{    
	  QString old_path = ud.path( 1 );
	  QString old_url = ud.url( 1 );
	  // Should we skip automatically ?
	  if ( auto_skip ) {
	    job.clearError();
	    // We dont want to copy files in this directory, so we put it on the skip list.
	    skip_list.append( old_url );
	    continue;
	  } else if ( overwrite_all ) {
	    job.clearError();
	    continue;
	  }

	  /* RenameDlg_Mode m = (RenameDlg_Mode)( M_SINGLE | M_OVERWRITE );
	  if ( dirs.count() > 1 )
	    m = (RenameDlg_Mode)(M_MULTI | M_SKIP | M_OVERWRITE ); */
	  RenameDlg_Mode m = (RenameDlg_Mode)( M_MULTI | M_SKIP | M_OVERWRITE );
	  QString tmp2 = ud.url();
	  QString n;
	  RenameDlg_Result r = open_RenameDlg( (*dir_it).m_strAbsSource, tmp2, m, n );
	  if ( r == R_CANCEL ) {
	    error( ERR_USER_CANCELED, "" );
	    m_cmd = CMD_NONE;
	    return;
	  } else if ( r == R_RENAME ) {
	    KURL u( n );
	    // The Dialog should have checked this.
	    if ( u.isMalformed() )
	      assert( 0 );
	    // The new path with trailing '/'
	    QString tmp3 = u.path( 1 );
	    renamed( tmp3 );
	    ///////
	    // Replace old path with tmp3 
	    ///////
	    QValueList<CopyDir>::Iterator dir_it2 = dir_it;
	    // Change the current one and strip the trailing '/'
	    (*dir_it2).m_strRelDest = u.path( -1 );
	    // Change the name of all subdirectories
	    dir_it2++;
	    for( ; dir_it2 != dirs.end(); dir_it2++ )
	      if ( strncmp( (*dir_it2).m_strRelDest, old_path, old_path.length() ) == 0 )
		(*dir_it2).m_strRelDest.replace( 0, old_path.length(), tmp3 );
	    // Change all filenames
	    QValueList<Copy>::Iterator fit2 = files.begin();
	    for( ; fit2 != files.end(); fit2++ )
	      if ( strncmp( (*fit2).m_strRelDest, old_path, old_path.length() ) == 0 )
		(*fit2).m_strRelDest.replace( 0, old_path.length(), tmp3 );
	    // Dont clear error => we will repeat the current command
	  } else if ( r == R_SKIP ) {
	    // Skip all files and directories that start with 'old_url'
	    skip_list.append( old_url );
	    // Clear the error => The current command is not repeated => skipped
	    job.clearError();
	  } else if ( r == R_AUTO_SKIP ) {
	    // Skip all files and directories that start with 'old_url'
	    skip_list.append( old_url );
	    // Clear the error => The current command is not repeated => skipped
	    job.clearError();
	    auto_skip = true;
	  } else if ( r == R_OVERWRITE ) {
	    // Dont bother for subdirectories
	    overwrite_list.append( old_url );
	    // Clear the error => The current command is not repeated => we will
	    // overwrite every file in this directory or any of its subdirectories
	    job.clearError();
	  } else if ( r == R_OVERWRITE_ALL ) {
	    job.clearError();
	    overwrite_all = true;
	  } else
	    assert( 0 );
	}
	// No need to ask the user, so raise an error
	else {
	  error( job.errorId(), job.errorText() );
	  m_cmd = CMD_NONE;
	  return;
	}
      }
    }
    while( job.hasError() );
      
    processedDirs( ++processed_dirs );
    ++dir_it;
  }

  qDebug( "kio_file : Created directories %s", dest.data() );

  /*****
   * Copy files
   *****/

  time_t t_start = time( 0L );
  time_t t_last = t_start;

  fit = files.begin();
  for( ; fit != files.end(); fit++ ) { 

    bool overwrite = false;
    bool skip_copying = false;

    // Repeat until we got no error
    do { 
      job.clearError();

      KURL ud( dest );
      ud.setPath( (*fit).m_strRelDest );
      QString d = ud.url();

      // Is this URL on the skip list ?
      bool skip = false;
      QStringList::Iterator sit = skip_list.begin();
      for( ; sit != skip_list.end() && !skip; sit++ )
	// Is 'd' a file in directory '*sit' or one of its subdirectories ?
	if ( strncmp( *sit, d, (*sit).length() ) == 0 )
	  skip = true;

      if ( skip )
	continue;

      QString realpath = "file:"; realpath += (*fit).m_strAbsSource;
      copyingFile( realpath, d );

      // qDebug( "kio_file : Writing to %s", d );

      // Is this URL on the overwrite list ?
      QStringList::Iterator oit = overwrite_list.begin();
      for( ; oit != overwrite_list.end() && !overwrite; oit++ )
	if ( strncmp( *oit, d, (*oit).length() ) == 0 )
	  overwrite = true;

      job.put( d, (*fit).m_mode, overwrite_all || overwrite,
	       false, (*fit).m_size );

      while( !job.isReady() && !job.hasFinished() )
	job.dispatch();

      // Did we have an error ?
      if ( job.hasError() ) {
	int currentError = job.errorId();

	qDebug("################# COULD NOT PUT %d", currentError);
	// if ( /* m_bGUI && */ job.errorId() == ERR_WRITE_ACCESS_DENIED )
	if ( /* m_bGUI && */ currentError != ERR_DOES_ALREADY_EXIST &&
			     currentError != ERR_DOES_ALREADY_EXIST_FULL )
	{
	  // Should we skip automatically ?
	  if ( auto_skip ) {
	    job.clearError();
	    skip_copying = true;
	    continue;
	  }
	  QString tmp2 = ud.url();
	  SkipDlg_Result r;
	  r = open_SkipDlg( tmp2, ( files.count() > 1 ) );
	  if ( r == S_CANCEL ) {
	    error( ERR_USER_CANCELED, "" );
	    m_cmd = CMD_NONE;
	    return;
	  } else if ( r == S_SKIP ) {
	    // Clear the error => The current command is not repeated => skipped
	    job.clearError();
	    skip_copying = true;
	    continue;
	  } else if ( r == S_AUTO_SKIP ) {
	    // Clear the error => The current command is not repeated => skipped
	    job.clearError();
	    skip_copying = true;
	    continue;
	  } else
	    assert( 0 );
	}
	// Can we prompt the user and ask for a solution ?
	else if ( /* m_bGUI && */ currentError == ERR_DOES_ALREADY_EXIST ||
				  currentError == ERR_DOES_ALREADY_EXIST_FULL )
	{    
	  // Should we skip automatically ?
	  if ( auto_skip ) {
	    job.clearError();
	    continue;
	  }

	  RenameDlg_Mode m = (RenameDlg_Mode)( M_SINGLE | M_OVERWRITE );
	  if ( files.count() > 1 )
	    m = (RenameDlg_Mode)( M_MULTI | M_SKIP | M_OVERWRITE );

	  QString tmp2 = ud.url().data();
	  QString n;
	  RenameDlg_Result r = open_RenameDlg((*fit).m_strAbsSource, tmp2, m, n );

	  if ( r == R_CANCEL ) 
	  {
	    error( ERR_USER_CANCELED, "" );
	    m_cmd = CMD_NONE;
	    return;
	  }
	  else if ( r == R_RENAME )
	  {
	    KURL u( n );
	    // The Dialog should have checked this.
	    if ( u.isMalformed() )
	      assert( 0 );
	    renamed( u.path( -1 ) );
	    // Change the destination name of the current file
	    (*fit).m_strRelDest = u.path( -1 );
	    // Dont clear error => we will repeat the current command
	  } else if ( r == R_SKIP ) {
	    // Clear the error => The current command is not repeated => skipped
	    job.clearError();
	  }
	  else if ( r == R_AUTO_SKIP )
	  {
	    // Clear the error => The current command is not repeated => skipped
	    job.clearError();
	    auto_skip = true;
	  }
	  else if ( r == R_OVERWRITE )
	  {
	    overwrite = true;
	    // Dont clear error => we will repeat the current command
	  }
	  else if ( r == R_OVERWRITE_ALL )
	  {
	    overwrite_all = true;
	    // Dont clear error => we will repeat the current command
	  }
	  else
	    assert( 0 );
	}
	// No need to ask the user, so raise an error
	else
	{    
	  error( currentError, job.errorText() );
	  m_cmd = CMD_NONE;
	  return;
	}
      }
    }
    while( job.hasError() );

    if ( skip_copying )
      continue;

    //qDebug( "kio_file : Opening %s", (*fit).m_strAbsSource );
    
    FILE *f = fopen( (*fit).m_strAbsSource, "rb" );
    if ( f == 0L ) {
      error( ERR_CANNOT_OPEN_FOR_READING, (*fit).m_strAbsSource );
      m_cmd = CMD_NONE;
      return;
    }

    char buffer[ 4096 ];
    while( !feof( f ) ) {
      int n = fread( buffer, 1, 2048, f );

      // !!! slow down loop for local testing
//        for ( int tmpi = 0; tmpi < 800000; tmpi++ ) ;

      job.data( buffer, n );
      processed_size += n;
      time_t t = time( 0L );
      if ( t - t_last >= 1 )
      {
	processedSize( processed_size );
	speed( processed_size / ( t - t_start ) );
	t_last = t;
      }
      
      // Check parent
      while ( check( connection() ) )
	dispatch();
      // Check for error messages from slave
      while ( check( &slave ) )
	job.dispatch();

      // An error ?
      if ( job.hasFinished() )
      {
	fclose( f );
	m_cmd = CMD_NONE;
	finished();
	return;
      }
    }

    job.dataEnd();
  
    fclose( f );

    while( !job.hasFinished() )
      job.dispatch();

    time_t t = time( 0L );
    
    processedSize( processed_size );
    if ( t - t_start >= 1 )
    {
      speed( processed_size / ( t - t_start ) );
      t_last = t;
    }
    processedFiles( ++processed_files );
  }

  qDebug( "kio_file : Copied files %s", dest.data() );

  if ( _move ) {
    slotDel( _source );
  }
  else
  {
    finished();
    m_cmd = CMD_NONE;
  }    
}

  
void FileProtocol::slotGet( const char *_url )
{
  KURL usrc( _url );
  if ( usrc.isMalformed() ) {
    error( ERR_MALFORMED_URL, strdup(_url) );
    m_cmd = CMD_NONE;
    return;
  }

  if ( !usrc.isLocalFile() ) {
    error( ERR_INTERNAL, "kio_file got non local file in get command" );
    m_cmd = CMD_NONE;
    return;
  }

  struct stat buff;
  if ( stat( usrc.path(), &buff ) == -1 ) {
    error( ERR_DOES_NOT_EXIST, strdup(_url) );
    m_cmd = CMD_NONE;
    return;
  }

  if ( S_ISDIR( buff.st_mode ) ) {
    error( ERR_IS_DIRECTORY, strdup(_url) );
    m_cmd = CMD_NONE;
    return;
  }

  m_cmd = CMD_GET;
  
  FILE *f = fopen( usrc.path(), "rb" );
  if ( f == 0L ) {
    error( ERR_CANNOT_OPEN_FOR_READING, strdup(_url) );
    m_cmd = CMD_NONE;
    return;
  }

  ready();

  gettingFile( _url );
  
  totalSize( buff.st_size );  
  int processed_size = 0;
  time_t t_start = time( 0L );
  time_t t_last = t_start;
  
  char buffer[ 4096 ];
  while( !feof( f ) )
  {
    int n = fread( buffer, 1, 2048, f );
    data( buffer, n );

    processed_size += n;
    time_t t = time( 0L );
    if ( t - t_last >= 1 )
    {
      processedSize( processed_size );
      speed( processed_size / ( t - t_start ) );
      t_last = t;
    }
  }

  dataEnd();
  
  fclose( f );

  processedSize( buff.st_size );
  time_t t = time( 0L );
  if ( t - t_start >= 1 )
    speed( processed_size / ( t - t_start ) );

  finished();
  m_cmd = CMD_NONE;
}


void FileProtocol::slotGetSize( const char *_url )
{
  m_cmd = CMD_GET_SIZE;
  
  KURL usrc( _url );
  if ( usrc.isMalformed() ) {
    error( ERR_MALFORMED_URL, strdup(_url) );
    m_cmd = CMD_NONE;
    return;
  }

  if ( !usrc.isLocalFile() ) {
    error( ERR_INTERNAL, "kio_file got non local file in get size command" );
    m_cmd = CMD_NONE;
    return;
  }

  struct stat buff;
  if ( stat( usrc.path(), &buff ) == -1 ) {
    error( ERR_DOES_NOT_EXIST, strdup(_url) );
    m_cmd = CMD_NONE;
    return;
  }

  if ( S_ISDIR( buff.st_mode ) )  { // !!! needed ?
    error( ERR_IS_DIRECTORY, strdup(_url) );
    m_cmd = CMD_NONE;
    return;
  }

  totalSize( buff.st_size );
  
  finished();
  m_cmd = CMD_NONE;
}


void FileProtocol::slotPut( const char *_url, int _mode, bool _overwrite, bool _resume, int _size )
{
  QString url_orig = _url;
  QString url_part = url_orig + ".part";

  KURL udest_orig( url_orig );
  KURL udest_part( url_part );

  bool m_bMarkPartial = KProtocolManager::self().markPartial();

  if ( udest_orig.isMalformed() ) {
    error( ERR_MALFORMED_URL, url_orig );
    finished();
    m_cmd = CMD_NONE;
    return;
  }

  if ( !udest_orig.isLocalFile() ) {
    error( ERR_INTERNAL, "kio_file got non local file as destination in put command" );
    finished();
    m_cmd = CMD_NONE;
    return;
  }

  m_cmd = CMD_PUT;

  struct stat buff;

  if ( stat( udest_orig.path(), &buff ) != -1 ) {

    // if original file exists but we are using mark partial -> rename it to XXX.part
    if ( m_bMarkPartial )
      rename ( udest_orig.path(), udest_part.path() );

    if ( !_overwrite && !_resume ) {
      if ( buff.st_size == _size )
	error( ERR_DOES_ALREADY_EXIST_FULL, udest_orig.path() );
      else
	error( ERR_DOES_ALREADY_EXIST, udest_orig.path() );
      
      finished();
      m_cmd = CMD_NONE;
      return;
    }
  } else if ( stat( udest_part.path(), &buff ) != -1 ) {
    // if file with extension .part exists but we are not using mark partial
    // -> rename XXX.part to original name
    if ( ! m_bMarkPartial )
      rename ( udest_part.path(), udest_orig.path() );

    if ( !_overwrite && !_resume ) {
	if ( buff.st_size == _size )
	  error( ERR_DOES_ALREADY_EXIST_FULL, udest_orig.path() );
	else
	  error( ERR_DOES_ALREADY_EXIST, udest_orig.path() );

	finished();
	m_cmd = CMD_NONE;
	return;
      }
  }

  KURL udest;

  // if we are using marking of partial downloads -> add .part extension
  if ( m_bMarkPartial ) {
    qDebug( "kio_file : Adding .part extension to %s", udest_orig.path().ascii() );
    udest = udest_part;
  } else
    udest = udest_orig;


  /* if ( access( udest.path(), W_OK ) == -1 )
  {
    qDebug("Write Access denied for '%s' %d",udest.path(),errno );
    
    error( ERR_WRITE_ACCESS_DENIED, strdup(_url) );
    finished();
    m_cmd = CMD_NONE;
    return;
  } */

  if ( _resume )
    m_fPut = fopen( udest.path(), "ab" );  // append if resuming
  else
    m_fPut = fopen( udest.path(), "wb" );

  if ( m_fPut == 0L ) {
    qDebug( "kio_file : ####################### COULD NOT WRITE %s", udest.path().ascii() );
    if ( errno == EACCES )
      error( ERR_WRITE_ACCESS_DENIED, udest.path() );
    else
      error( ERR_CANNOT_OPEN_FOR_WRITING, udest.path() );
    m_cmd = CMD_NONE;
    finished();
    return;
  }

  // We are ready for receiving data
  ready();
  
  // Loop until we got 'dataEnd'
  while ( m_cmd == CMD_PUT && dispatch() );

  fclose( m_fPut );

  if ( stat( udest.path(), &buff ) != -1 ) {

    // If the given file size is 0 and the downloaded size is not 0,
    // then we assume we got the whole file.
    if ( buff.st_size == _size || ( buff.st_size && !_size ) ) {

      // after full download rename the file back to original name
      if ( m_bMarkPartial ) {
	if ( rename( udest.path(), udest_orig.path() ) ) {
	    error( ERR_CANNOT_RENAME, udest_orig.path() );
	    m_cmd = CMD_NONE;
	    finished();
	    return;
	  }
      }

      // do chmod only after full download
      if ( _mode != -1 ) {
	if ( chmod( udest_orig.path(), _mode ) == -1 ) {
	    error( ERR_CANNOT_CHMOD, udest_orig.path() );
	    m_cmd = CMD_NONE;
	    finished();
	    return;
	  }
      }
    } // if the size is less then minimum -> delete the file
    else if ( buff.st_size < KProtocolManager::self().minimumKeepSize() ) {
	remove( udest.path() );
    }
  }

  // We have done our job => finish
  finished();

  m_cmd = CMD_NONE;
}


void FileProtocol::slotDel( QStringList& _source )
{
  // Check whether the URLs are wellformed
  QStringList::Iterator source_it = _source.begin();
  for( ; source_it != _source.end(); ++source_it ) {    
    qDebug( "kio_file : Checking %s", (*source_it).ascii() );
    KURL usrc( *source_it );
    if ( usrc.isMalformed() ) {
      error( ERR_MALFORMED_URL, *source_it );
      m_cmd = CMD_NONE;
      return;
    }
    if ( !usrc.isLocalFile() ) {
      error( ERR_MALFORMED_URL, "kio_file got non local file in delete command" );
      m_cmd = CMD_NONE;
      return;
    }
  }

  qDebug( "kio_file : All URLs ok" );

  // Get a list of all source files and directories
  QValueList<Copy> fs;
  QValueList<CopyDir> ds;
  int size = 0;
  qDebug( "kio_file : Iterating" );

  source_it = _source.begin();
  qDebug( "kio_file : Looping" );
  for( ; source_it != _source.end(); ++source_it ) {
    struct stat stat_buf;
    qDebug( "kio_file : Checking %s", (*source_it).ascii() );
    KURL victim( (*source_it) );
    int s;
    if ( ( s = listRecursive( victim.path(), fs, ds, false ) ) == -1 ) {
      // Error message is already sent
      m_cmd = CMD_NONE;
      return;
    }
    // Sum up the total amount of bytes we have to copy
    size += s;
    qDebug( "kio_file : Parsed URL OK and added to appropiate list" );
  }

  qDebug( "kio_file : Recursive ok" );

  if ( fs.count() == 1 )
    m_cmd = CMD_DEL;
  else
    m_cmd = CMD_MDEL;

  // Tell our client what we 'r' gonna do
  totalSize( size );
  totalFiles( fs.count() );
  totalDirs( ds.count() );

  /*****
   * Delete files
   *****/

  QValueList<Copy>::Iterator fit = fs.begin();
  for( ; fit != fs.end(); fit++ ) { 

    QString filename = (*fit).m_strAbsSource;
    qDebug( "kio_file : Deleting file %s", filename.ascii() );

    deletingFile( filename );

    if ( unlink( filename ) == -1 ) {
      error( ERR_CANNOT_DELETE, filename );
      m_cmd = CMD_NONE;
      return;
    }
  }

  /*****
   * Delete empty directories
   *****/

  QValueList<CopyDir>::Iterator dit = ds.fromLast();
  for( ; dit != ds.end(); dit-- ) { 

    QString dirname = (*dit).m_strAbsSource;
    qDebug( "kio_file : Deleting directory %s", dirname.ascii() );

    deletingFile( dirname );

    if ( rmdir( dirname ) == -1 ) {
      error( ERR_COULD_NOT_RMDIR, dirname );
      m_cmd = CMD_NONE;
      return;
    }
  }

  finished();
  
  m_cmd = CMD_NONE;
}

void FileProtocol::slotListDir( const char *_url )
{
  KURL usrc( _url );
  if ( usrc.isMalformed() ) {
    error( ERR_MALFORMED_URL, strdup(_url) );
    m_cmd = CMD_NONE;
    return;
  }

  if ( !usrc.isLocalFile() ) {
    error( ERR_INTERNAL, "kio_file got non local file in list command" );
    m_cmd = CMD_NONE;
    return;
  }

  struct stat buff;
  if ( stat( usrc.path(), &buff ) == -1 ) {
    error( ERR_DOES_NOT_EXIST, strdup(_url) );
    m_cmd = CMD_NONE;
    return;
  }

  if ( !S_ISDIR( buff.st_mode ) ) {
    error( ERR_IS_FILE, strdup(_url) );
    m_cmd = CMD_NONE;
    return;
  }

  m_cmd = CMD_LIST;
  
  DIR *dp = 0L;
  struct dirent *ep;

  QString dir = usrc.path( 0 );
  dp = opendir( dir.ascii() );
  if ( dp == 0L ) {
    error( ERR_CANNOT_ENTER_DIRECTORY, strdup(_url) );
    m_cmd = CMD_NONE;
    return;
  }

  QValueList<KUDSEntry> entries;

  KUDSEntry entry;
  KUDSAtom atom;
  while ( ( ep = readdir( dp ) ) != 0L ) {
    if ( strcmp( ep->d_name, "." ) == 0 || strcmp( ep->d_name, ".." ) == 0 )
      continue;

    // qDebug( "kio_file : Listing %s", ep->d_name );

    entry.clear();

    atom.m_uds = UDS_NAME;
    atom.m_str = ep->d_name;
    entry.append( atom );
  
    QString tmp = usrc.path( 1 );
    tmp += ep->d_name;

    QString slink = "";
    mode_t type;
    mode_t access;
 
    struct stat buff;
    struct stat lbuff;
    if ( stat( tmp.ascii(), &buff ) == -1 )  {
      // A link poiting to nowhere ?
      if ( lstat( tmp.ascii(), &lbuff ) == -1 ) {
	// Should never happen
	error( ERR_DOES_NOT_EXIST, tmp );
	m_cmd = CMD_NONE;
	return;
      }

      // It is a link pointing to nowhere
      type = S_IFMT - 1;
      access = S_IRWXU | S_IRWXG | S_IRWXO;
    } else {
      lstat( tmp.ascii(), &lbuff );
      type = buff.st_mode;
      access = buff.st_mode;
      // Is it a link
      if ( S_ISLNK( lbuff.st_mode ) ) {  
	// type |= S_IFLNK; No !! This screws S_ISDIR and friends. (David)
        // caller should check UDS_LINK_DEST instead
	char buffer2[ 1000 ];
	int n = readlink( tmp.ascii(), buffer2, 1000 );
	if ( n != -1 ) {
	  buffer2[ n ] = 0;
	  slink = buffer2;
	}
      }
      else
	type &= S_IFMT;
    }
    
    struct passwd * user = getpwuid( buff.st_uid );
    struct group * grp = getgrgid( buff.st_gid );

    atom.m_uds = UDS_FILE_TYPE;
    atom.m_long = type;
    entry.append( atom );

    atom.m_uds = UDS_SIZE;
    atom.m_long = buff.st_size;
    entry.append( atom );

    atom.m_uds = UDS_MODIFICATION_TIME;
    atom.m_long = buff.st_mtime;
    entry.append( atom );

    atom.m_uds = UDS_ACCESS;
    atom.m_long = access;
    entry.append( atom );

    atom.m_uds = UDS_USER;
    atom.m_str = (( user != 0L ) ? user->pw_name : "???" );
    entry.append( atom );

    atom.m_uds = UDS_GROUP;
    atom.m_str = (( grp != 0L ) ? grp->gr_name : "???" );
    entry.append( atom );

    atom.m_uds = UDS_LINK_DEST;
    atom.m_str = slink;
    entry.append( atom );

    atom.m_uds = UDS_ACCESS_TIME;
    atom.m_long = buff.st_atime;
    entry.append( atom );    

    atom.m_uds = UDS_CREATION_TIME;
    atom.m_long = buff.st_ctime;
    entry.append( atom );
//    listEntry( entry );
    entries.append( entry );
  }
  
  closedir( dp );
  
  totalFiles( entries.count() );
  
  QValueList<KUDSEntry>::Iterator it = entries.begin();
  QValueList<KUDSEntry>::Iterator end = entries.end();
  int i = 0;
  for (; it != end; ++it )
  {
    listEntry( *it );
    processedFiles( ++i );
  }
  
  m_cmd = CMD_NONE;

  finished();
}


void FileProtocol::slotTestDir( const char *_url )
{
  KURL usrc( _url );
  if ( usrc.isMalformed() ) {
    error( ERR_MALFORMED_URL, strdup(_url) );
    m_cmd = CMD_NONE;
    return;
  }

  if ( !usrc.isLocalFile() ) {
    error( ERR_INTERNAL, "kio_file got non local file in testdir command" );
    m_cmd = CMD_NONE;
    return;
  }

  struct stat buff;
  if ( stat( usrc.path(), &buff ) == -1 ) {
    error( ERR_DOES_NOT_EXIST, strdup(_url) );
    m_cmd = CMD_NONE;
    return;
  }

  if ( S_ISDIR( buff.st_mode ) )
    isDirectory();
  else
    isFile();

  finished();
}

void FileProtocol::slotMount( bool _ro, const char *_fstype, const char* _dev, const char *_point )
{
  char buffer[ 1024 ];

  int t = (int)time( 0L );
    
  // Look in /etc/fstab ?
  if ( *_fstype == 0 || *_point == 0 )
    sprintf( buffer, "mount %s 2>/tmp/mnt%i",_dev, t );
  else if ( _ro )
    sprintf( buffer, "mount -rt %s %s %s 2>/tmp/mnt%i",_fstype, _dev, _point, t );
  else
    sprintf( buffer, "mount -t %s %s %s 2>/tmp/mnt%i",_fstype, _dev, _point, t );
  
  system( buffer );
  
  sprintf( buffer, "/tmp/mnt%i", t );
  
  QString err = testLogFile( buffer );
  if ( err.isEmpty() ) {
    finished();
    return;
  }

  error( ERR_COULD_NOT_MOUNT, err );
  m_cmd = CMD_NONE;

  return;
}


void FileProtocol::slotUnmount( const char *_point )
{
  char buffer[ 1024 ];

  int t = (int)time( 0L );
    
  sprintf( buffer, "umount %s 2>/tmp/mnt%i",_point, t );
  system( buffer );

  sprintf( buffer, "/tmp/mnt%i", t );

  QString err = testLogFile( buffer );
  if ( err.isEmpty() ) {
    finished();
    return;
  }

  error( ERR_COULD_NOT_MOUNT, err );
  m_cmd = CMD_NONE;

  return;
}


void FileProtocol::slotData( void *_p, int _len )
{
  switch( m_cmd )
    {
    case CMD_PUT:
      fwrite( _p, 1, _len, m_fPut );
      break;
    }
}

void FileProtocol::slotDataEnd()
{
  switch( m_cmd )
    {
    case CMD_PUT:  
      m_cmd = CMD_NONE;
    }
}

long FileProtocol::listRecursive( const char *_path, QValueList<Copy>&
				_files, QValueList<CopyDir>& _dirs, bool _rename )
{
  struct stat buff;

  // Check whether we have to copy the complete directory tree beginning by its root.
  int len = strlen( _path );
  while( len >= 1 && _path[ len -1 ] == '/' )
    len--;
  if ( len == 1 && _path[ 0 ] == '/' )
  {
    if ( stat( "/", &buff ) == -1 )
    {
      error( ERR_DOES_NOT_EXIST, "/" );
      return -1;
    }

    CopyDir c;
    c.m_strAbsSource = _path;
    if ( _rename )
      c.m_strRelDest = "";
    else
      c.m_strRelDest = "Root";
    c.m_ino = buff.st_ino;
    c.m_mode = buff.st_mode;
    _dirs.append( c );
    
    return listRecursive2( "/", c.m_strRelDest, _files, _dirs );
  }
  
  QString p( _path );
  p.truncate( len );
  qDebug( "kio_file : ########## RECURSIVE LISTING %s", p.ascii() );
  
  if ( stat( p, &buff ) == -1 ) {
    error( ERR_DOES_NOT_EXIST, p );
    return -1;
  }

  KURL u( p );
  // Should be checked before, but who knows
  if ( u.isMalformed() )
    assert( 0 );

  // Is the source not a directory ? => so just copy it and we are done.
  if ( !S_ISDIR( buff.st_mode ) ) {
    QString fname;
    if ( _rename )
      fname = "";
    else {
      fname = u.filename();
      // Should be impossible, but who knows ...
      if ( fname.isEmpty() )
	assert( 0 );
    }

    Copy c;
    c.m_strAbsSource = p;
    c.m_strRelDest = fname;
    c.m_mode = buff.st_mode;
    c.m_size = buff.st_size;
    _files.append( c );
    return buff.st_size;
  }

  // The source is a directory. So we have to go into recursion here.
  QString tmp1;
  if ( _rename )
    tmp1 = u.path( 0 );
  else {
    tmp1 = u.directory( true );
    tmp1 += "/";
  }
  QString tmp2;
  if ( _rename )
    tmp2 = "";
  else
    tmp2 = u.filename();
  CopyDir c;
  c.m_strAbsSource = p;
  c.m_strRelDest = tmp2;
  c.m_mode = buff.st_mode;
  c.m_ino = buff.st_ino;
  _dirs.append( c );
  qDebug( "kio_file : ########### STARTING RECURSION with %s and %s",tmp1.ascii(), tmp2.ascii() );

  return listRecursive2( tmp1, tmp2, _files, _dirs );
}

long FileProtocol::listRecursive2( const char *_abs_path, const char *_rel_path,
				   QValueList<Copy>& _files, QValueList<CopyDir>& _dirs )
{
  long size = 0;
  
  QString p = _abs_path;
  p += _rel_path;

  DIR *dp = 0L;
  struct dirent *ep;

  scanningDir( p );
  
  dp = opendir( p );
  if ( dp == 0L )
  {
    error( ERR_CANNOT_ENTER_DIRECTORY, p );
    return -1;
  }
    
  while ( ( ep = readdir( dp ) ) != 0L )
  {
    if ( strcmp( ep->d_name, "." ) == 0 || strcmp( ep->d_name, ".." ) == 0 )
      continue;
    
    QString p2 = p;
    p2 += "/";
    p2 += ep->d_name;
  
    struct stat buff;
    if ( stat( p2, &buff ) == -1 )
    {
      // Should never happen
      error( ERR_DOES_NOT_EXIST, p );
      return -1;
    }

    QString tmp = _rel_path;
//     if ( tmp != "" )
      tmp += "/";
    tmp += ep->d_name;
  
    if ( !S_ISDIR( buff.st_mode ) )
    {
      Copy c;
      c.m_strAbsSource = p2;
      c.m_strRelDest = tmp;
      c.m_mode = buff.st_mode & ( S_ISUID | S_ISGID | S_IFLNK | S_IRWXU | S_IRWXG | S_IRWXO );
      c.m_size = buff.st_size;
      _files.append( c );
      size += buff.st_size;
    } else {
      // Did we scan this directory already ?
      // This may happen because a link goes backward in the directory tree
      QValueList<CopyDir>::Iterator it = _dirs.begin();
      for( ; it != _dirs.end(); it++ )
	if ( (*it).m_ino == buff.st_ino ) {
	  error( ERR_CYCLIC_LINK, p2 );
	  return -1;
	}
      
      CopyDir c;
      c.m_strAbsSource = p2;
      c.m_strRelDest = tmp;
      c.m_mode = buff.st_mode;
      c.m_ino = buff.st_ino;
      _dirs.append( c );

      long s;
      if ( ( s = listRecursive2( _abs_path, tmp, _files, _dirs ) ) == -1 )
	return -1;
      size += s;
    }
  }
  
  closedir( dp );
  
  return size;
}

void FileProtocol::jobError( int _errid, const char *_txt )
{
  if ( !m_bIgnoreJobErrors )
    error( _errid, _txt );
}

/*************************************
 *
 * FileIOJob
 *
 *************************************/

FileIOJob::FileIOJob( KIOConnection *_conn, FileProtocol *_File ) : KIOJobBase( _conn )
{
  m_pFile = _File;
}
  
void FileIOJob::slotError( int _errid, const char *_txt )
{
  KIOJobBase::slotError( _errid, _txt );
  m_pFile->jobError( _errid, _txt );
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
    result = "Could not read ";
    result += _filename;
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


int check( KIOConnection *_con )
{
  int err;
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  fd_set rfds;
  FD_ZERO( &rfds );
  FD_SET( _con->inFD(), &rfds );
  
again:
  if ( ( err = select( _con->inFD(), &rfds, 0L, 0L, &tv ) ) == -1 && errno == EINTR )
    goto again;

  // No error and something to read ?
  if ( err != -1 && err != 0 )
    return 1;
  
  return 0;
}


void openFileManagerWindow( const char * )
{
  assert( 0 );
}
