#include "main.h"

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>
#include <k2url.h>
#include <kio_manager.h>
#include <kio_rename_dlg.h>
#include <kio_skip_dlg.h>

void sig_handler( int );
void sig_handler2( int );
int check( Connection *_con );

int main( int argc, char **argv )
{
  signal(SIGCHLD,sig_handler);
  signal(SIGSEGV,sig_handler2);

  ProtocolManager manager;

  Connection parent( 0, 1 );
  
  FtpProtocol ftp( &parent );
  ftp.dispatchLoop();

  debug( "kio_ftp : Done" );
}

void sig_handler2( int )
{
  debug( "kio_ftp : ###############SEG FTP#############" );
  exit(1);
}

void sig_handler( int )
{
  int pid;
  int status;
    
  while( 1 )
  {
    pid = waitpid( -1, &status, WNOHANG );
    if ( pid <= 0 )
    {
      // Reinstall signal handler, since Linux resets to default after
      // the signal occured ( BSD handles it different, but it should do
      // no harm ).
      signal( SIGCHLD, sig_handler );
      return;
    }
  }
}


/*
 * We'll use an alarm that will set this flag when transfer has timed out
 */
char sigbreak = 0;

void sig_alarm(int signo)
{
  sigbreak = 1;
}            

void setup_alarm(unsigned int timeout)
{
  sigbreak = 0;
  alarm(timeout);
  signal(SIGALRM, sig_alarm);
}


FtpProtocol::FtpProtocol( Connection *_conn ) : IOProtocol( _conn )
{
  m_cmd = CMD_NONE;
  m_bIgnoreJobErrors = false;
  m_bCanResume = true; // most of ftp servers support resuming ?
}


void FtpProtocol::slotMkdir( const char *_url, int _mode )
{
  string url = _url;
  
  K2URL usrc( _url );
  if ( usrc.isMalformed() )
  {
    error( ERR_MALFORMED_URL, url.c_str() );
    m_cmd = CMD_NONE;
    return;
  }

  if ( strcmp( usrc.protocol(), "ftp" ) != 0L )
  {
    error( ERR_INTERNAL, "kio_ftp got non ftp file in mkdir command" );
    m_cmd = CMD_NONE;
    return;
  }

  // Connect to the ftp server
  if ( ! ftp.isConnected() )
    if ( !ftp.ftpConnect( usrc ) )
      {
	error( ftp.error(), ftp.errorText() );
	m_cmd = CMD_NONE;
	return;
      }

  FtpEntry* e = ftp.ftpStat( usrc );

  if ( !e  ) {
    if ( ! ftp.ftpMkdir( usrc.path() ) )
      {
	error( ERR_COULD_NOT_MKDIR, url.c_str() );
	m_cmd = CMD_NONE;
	return;
      }
    else
      {
	// set the desired attributes for dir  !!! how ?
// 	if ( _mode != -1 )
// 	  if ( chmod( usrc.path(), _mode ) == -1 )
// 	    {
// 	      error( ERR_CANNOT_CHMOD, url.c_str() );
// 	      m_cmd = CMD_NONE;
// 	      return;
// 	    }

	finished();
	return;
      }
  }

  if ( S_ISDIR( e->type ) )  // !!! ok ?
  {
    error( ERR_DOES_ALREADY_EXIST, url.c_str() );
    m_cmd = CMD_NONE;
    return;
  }
    
  error( ERR_COULD_NOT_MKDIR, url.c_str() );
  m_cmd = CMD_NONE;
  return;
}


void FtpProtocol::slotCopy( list<string>& _source, const char *_dest )
{
  doCopy( _source, _dest, false );
}


void FtpProtocol::slotCopy( const char* _source, const char *_dest )
{
  list<string> lst;
  lst.push_back( _source );
  
  doCopy( lst, _dest, true );
}


void FtpProtocol::slotMove( list<string>& _source, const char *_dest )
{
  doCopy( _source, _dest, false, true );
}


void FtpProtocol::slotMove( const char* _source, const char *_dest )
{
  list<string> lst;
  lst.push_back( _source );
  
  doCopy( lst, _dest, true, true );
}


void FtpProtocol::doCopy( list<string>& _source, const char *_dest, bool _rename, bool _move )
{
  if ( _rename )
    assert( _source.size() == 1 );
  
  debug( "kio_ftp : Making copy to %s", _dest );
  
  // Check wether the URLs are wellformed
  list<string>::iterator soit = _source.begin();
  for( ; soit != _source.end(); ++soit )
  {    
    debug( "kio_ftp : Checking %s", soit->c_str() );
    K2URL usrc( *soit );
    if ( usrc.isMalformed() )
    {
      error( ERR_MALFORMED_URL, soit->c_str() );
      m_cmd = CMD_NONE;
      return;
    }
    if ( strcmp( usrc.protocol(), "ftp" ) != 0L )
    {
      error( ERR_INTERNAL, "kio_ftp got non ftp file as source in copy command" );
      m_cmd = CMD_NONE;
      return;
    }
  }

  debug( "kio_ftp : All URLs ok %s", _dest );

  // Make a copy of the parameter. if we do IPC calls from here, then we overwrite
  // our argument. This is tricky! ( but saves memory and speeds things up )
  string dest = _dest;
  
  // Check wellformedness of the destination
  K2URL udest( dest );
  if ( udest.isMalformed() )
  {
    error( ERR_MALFORMED_URL, dest.c_str() );
    m_cmd = CMD_NONE;
    return;
  }

  debug( "kio_ftp : Dest ok %s", dest.c_str() );

  // Extract destinations right most protocol
  list<K2URL> lst;
  if ( !K2URL::split( dest.c_str(), lst )  )
  {
    error( ERR_MALFORMED_URL, dest.c_str() );
    m_cmd = CMD_NONE;
    return;
  }

  // Find IO server for destination
  string exec = ProtocolManager::self()->find( lst.back().protocol() );

  if ( exec.empty() )
  {
    error( ERR_UNSUPPORTED_PROTOCOL, lst.back().protocol() );
    m_cmd = CMD_NONE;
    return;
  }

  // Is the right most protocol a filesystem protocol ?
  if ( ProtocolManager::self()->outputType( lst.back().protocol() ) != ProtocolManager::T_FILESYSTEM )
  {
    error( ERR_PROTOCOL_IS_NOT_A_FILESYSTEM, lst.back().protocol() );
    m_cmd = CMD_NONE;
    return;
  }
      
  debug( "kio_ftp : IO server ok %s", dest.c_str() );

  // Connect to the ftp server
  K2URL usrc( _source.front() );
  if ( !ftp.ftpConnect( usrc ) )
  {
    error( ftp.error(), ftp.errorText() );
    m_cmd = CMD_NONE;
    return;
  }
  
  // Find out, whether we are logged anonymously or not
  // Authorization has been already checked with ftp.ftpConnect( usrc )
  bool b_user = false;
  if ( usrc.hasUser() )
    b_user = true;

  // Get a list of all source files and directories
  list<Copy> files;
  list<CopyDir> dirs;
  int size = 0;
  debug( "kio_ftp : Iterating" );

  soit = _source.begin();
  debug( "kio_ftp : Looping" );
  for( ; soit != _source.end(); ++soit )
  {    
    debug( "kio_ftp : Executing %s", soit->c_str() );
    K2URL usrc( *soit );
    debug( "kio_ftp : Parsed URL" );
    // Did an error occur ?
    int s;
    if ( ( s = listRecursive( usrc.path(), files, dirs, _rename ) ) == -1 )
    {
      ftp.ftpDisconnect();
      // Error message is already sent
      m_cmd = CMD_NONE;
      return;
    }
    // Sum up the total amount of bytes we have to copy
    size += s;
  }

  debug( "kio_ftp : Recursive 1 %s", dest.c_str() );

  /*
  // Check wether we do not copy a directory in itself or one of its subdirectories
  struct stat buff2;
  if ( udest.isLocalFile() && stat( udest.path(), &buff2 ) == 0 )
  {
    bool b_error = false;
    for( soit = _source.begin(); soit != _source.end(); ++soit )
    {    
      K2URL usrc( *soit );  

      struct stat buff1;
      // Can we stat both the source, too ? ( Should always be the case )
      if ( stat( usrc.path(), &buff1 ) == 0 )
      {
	bool b_error = false;
	// Are source and dest equal ? => error
	if ( buff1.st_ino == buff2.st_ino )
	  b_error = true;
      }
    }
  
    if ( !b_error )
    {
      // Iterate over all subdirectories
      list<CopyDir>::iterator it = dirs.begin();
      for( ; it != dirs.end() && !b_error; it++ )
	if ( buff2.st_ino == it->m_ino )
	  b_error = true;
    }

    // Do we have a cylic copy now ? => error
    if ( b_error )
    {
      error( ERR_CYCLIC_COPY, soit->c_str() );
      m_cmd = CMD_NONE;
      return;
    }
  }
  */

  debug( "kio_ftp : Recursive ok %s", dest.c_str() );

  m_cmd = CMD_GET;
  
  // Start a server for the destination protocol
  Slave slave( exec.c_str() );
  if ( slave.pid() == -1 )
  {
    error( ERR_CANNOT_LAUNCH_PROCESS, exec.c_str() );
    ftp.ftpDisconnect();
    m_cmd = CMD_NONE;
    return;
  }
  
  // Put a protocol on top of the job
  FtpIOJob job( &slave, this );

  debug( "kio_ftp : Job started ok %s", dest.c_str() );

  // Tell our client what we 'r' gonna do
  totalSize( size );
  totalFiles( files.size() );
  totalDirs( dirs.size() );
  
  int processed_files = 0;
  int processed_dirs = 0;
  int processed_size = 0;
  
  // Replace the relative destinations with absolut destinations
  // by prepending the destinations path
  string tmp1 = lst.back().path( 1 );
  // Strip '/'
  string tmp1_stripped = lst.back().path( -1 );
  list<CopyDir>::iterator dit = dirs.begin();
  for( ; dit != dirs.end(); dit++ )
  {
    string tmp2 = dit->m_strRelDest;
    dit->m_strRelDest = tmp1;
    dit->m_strRelDest += tmp2;
  }
  list<Copy>::iterator fit = files.begin();
  for( ; fit != files.end(); fit++ )
  {
    string tmp2 = fit->m_strRelDest;
    if ( _rename && fit->m_strRelDest == "" )
      fit->m_strRelDest = tmp1_stripped;
    else
      fit->m_strRelDest = tmp1;
    fit->m_strRelDest += tmp2;
  }
  
  debug( "kio_ftp : Destinations ok %s", dest.c_str() );

  /*****
   * Make directories
   *****/
  
  m_bIgnoreJobErrors = true;
  bool overwrite_all = false;
  bool auto_skip = false;
  bool resume_all = false;
  list<string> skip_list;
  list<string> overwrite_list;
  
  // Create all directories
  dit = dirs.begin();
  for( ; dit != dirs.end(); dit++ )
  { 
    // Repeat until we got no error
    do
    {
      job.clearError();

      list<K2URL> l( lst );
      l.back().setPath( dit->m_strRelDest.c_str() );

      string d;
      list<K2URL>::iterator it = l.begin();
      for( ; it != l.end(); it++ )
	d += it->url();

      // Is this URL on the skip list ?
      bool skip = false;
      list<string>::iterator sit = skip_list.begin();
      for( ; sit != skip_list.end() && !skip; sit++ )
	// Is d a subdirectory of *sit ?
	if ( strncmp( sit->c_str(), d.c_str(), sit->size() ) == 0 )
	  skip = true;
      
      if ( skip )
	continue;

      // Is this URL on the overwrite list ?
      bool overwrite = false;
      list<string>::iterator oit = overwrite_list.begin();
      for( ; oit != overwrite_list.end() && !overwrite; oit++ )
	if ( strncmp( oit->c_str(), d.c_str(), oit->size() ) == 0 )
	  overwrite = true;
      
      if ( overwrite )
	continue;
      
      // Tell what we are doing
      makingDir( d.c_str() );
      
      // debug( "kio_ftp : Making remote dir %s", d );
      // Create the directory
      job.mkdir( d.c_str(), dit->m_access );
      while( !job.hasFinished() )
	job.dispatch();

      // Did we have an error ?
      if ( job.hasError() )
      {
	// Can we prompt the user and ask for a solution ?
	if ( /* m_bGUI && */ job.errorId() == ERR_DOES_ALREADY_EXIST )
	{    
	  string old_path = l.back().path( 1 );
	  string old_url = l.back().url( 1 );
	  // Should we skip automatically ?
	  if ( auto_skip )
	  {
	    job.clearError();
	    // We dont want to copy files in this directory, so we put it on the skip list.
	    skip_list.push_back( old_url );
	    continue;
	  }
	  else if ( overwrite_all )
	  {    
	    job.clearError();
	    continue;
	  }

	  string n;
	  /* RenameDlg_Mode m = (RenameDlg_Mode)( M_SINGLE | M_OVERWRITE );
	  if ( dirs.size() > 1 )
	    m = (RenameDlg_Mode)(M_MULTI | M_SKIP | M_OVERWRITE ); */
	  RenameDlg_Mode m = (RenameDlg_Mode)( M_MULTI | M_SKIP | M_OVERWRITE );
	  string tmp2 = l.back().url();
	  RenameDlg_Result r = open_RenameDlg( dit->m_strAbsSource.c_str(), tmp2.c_str(), m, n );
	  if ( r == R_CANCEL ) 
	  {
	    ftp.ftpDisconnect();
	    error( ERR_USER_CANCELED, "" );
	    m_cmd = CMD_NONE;
	    return;
	  }
	  else if ( r == R_RENAME )
	  {
	    K2URL u( n.c_str() );
	    // The Dialog should have checked this.
	    if ( u.isMalformed() )
	      assert( 0 );
	    // The new path with trailing '/'
	    string tmp3 = u.path( 1 );
	    ///////
	    // Replace old path with tmp3 
	    ///////
	    list<CopyDir>::iterator dit2 = dit;
	    // Change the current one and strip the trailing '/'
	    dit2->m_strRelDest = u.path( -1 );
	    // Change the name of all subdirectories
	    dit2++;
	    for( ; dit2 != dirs.end(); dit2++ )
	      if ( strncmp( dit2->m_strRelDest.c_str(), old_path.c_str(), old_path.size() ) == 0 )
		dit2->m_strRelDest.replace( 0, old_path.size(), tmp3 );
	    // Change all filenames
	    list<Copy>::iterator fit2 = files.begin();
	    for( ; fit2 != files.end(); fit2++ )
	      if ( strncmp( fit2->m_strRelDest.c_str(), old_path.c_str(), old_path.size() ) == 0 )
		fit2->m_strRelDest.replace( 0, old_path.size(), tmp3 );
	    // Dont clear error => we will repeat the current command
	  }
	  else if ( r == R_SKIP )
	  {
	    // Skip all files and directories that start with 'old_url'
	    skip_list.push_back( old_url );
	    // Clear the error => The current command is not repeated => skipped
	    job.clearError();
	  }
	  else if ( r == R_AUTO_SKIP )
	  {
	    // Skip all files and directories that start with 'old_url'
	    skip_list.push_back( old_url );
	    // Clear the error => The current command is not repeated => skipped
	    job.clearError();
	    auto_skip = true;
	  }
	  else if ( r == R_OVERWRITE )
	  {
	    // Dont bother for subdirectories
	    overwrite_list.push_back( old_url );
	    // Clear the error => The current command is not repeated => we will
	    // overwrite every file in this directory or any of its subdirectories
	    job.clearError();
	  }
	  else if ( r == R_OVERWRITE_ALL )
	  {
	    job.clearError();
	    overwrite_all = true;
	  }
	  else
	    assert( 0 );
	}
	// No need to ask the user, so raise an error
	else
	{    
	  ftp.ftpDisconnect();
	  error( job.errorId(), job.errorText() );
	  m_cmd = CMD_NONE;
	  return;
	}
      }
    }
    while( job.hasError() );
      
    processedDirs( ++processed_dirs );
  }

  debug( "kio_ftp : Created directories %s", dest.c_str() );
  

  /*****
   * Server check - supports resuming ?
   *****/

  if ( !ftp.ftpResume( 0 ) )
    m_bCanResume = false;

  /*****
   * Copy files
   *****/

  time_t t_start = time( 0L );
  time_t t_last = t_start;

  fit = files.begin();
  for( ; fit != files.end(); fit++ ) { 

    bool overwrite = false;
    bool skip_copying = false;
    bool resume = false;
    unsigned long offset = 0;

    // Repeat until we got no error
    do
    { 
      job.clearError();

      list<K2URL> l( lst );
      l.back().setPath( fit->m_strRelDest.c_str() );
    
      // debug( "kio_ftp : ########### SET Path to '%s'", fit->m_strRelDest.c_str() );
      
      string d;
      list<K2URL>::iterator it = l.begin();
      for( ; it != l.end(); it++ )
	d += it->url();

      // Is this URL on the skip list ?
      bool skip = false;
      list<string>::iterator sit = skip_list.begin();
      for( ; sit != skip_list.end() && !skip; sit++ )
	// Is 'd' a file in directory '*sit' or one of its subdirectories ?
	if ( strncmp( sit->c_str(), d.c_str(), sit->size() ) == 0 )
	  skip = true;
    
      if ( skip )
	continue;
    
      // emit sigCanResume( m_bCanResume )
      canResume( m_bCanResume );

      string realpath = "ftp:"; realpath += fit->m_strAbsSource;
      copyingFile( realpath.c_str(), d.c_str() );
    
      // debug( "kio_ftp : Writing to %s", d );
       
      // Is this URL on the overwrite list ?
      list<string>::iterator oit = overwrite_list.begin();
      for( ; oit != overwrite_list.end() && !overwrite; oit++ )
	if ( strncmp( oit->c_str(), d.c_str(), oit->size() ) == 0 )
	  overwrite = true;
      
      // implicitly set permissions rw-r--r-- for anonymous ftp
      int md = -1;
      // but when it's not anonymous ftp, set permissions as in original source
      if ( b_user )
	md = fit->m_access;

      job.put( d.c_str(), md, overwrite_all || overwrite,
		 resume_all || resume, fit->m_size );

      while( !job.isReady() && !job.hasFinished() )
	job.dispatch();

      // Did we have an error ?
      if ( job.hasError() ) {
	int currentError = job.errorId();

	debug("################# COULD NOT PUT %d",currentError);
	// if ( /* m_bGUI && */ job.errorId() == ERR_WRITE_ACCESS_DENIED )
	if ( /* m_bGUI && */  currentError != ERR_DOES_ALREADY_EXIST &&
			      currentError != ERR_DOES_ALREADY_EXIST_FULL )
	{
	  // Should we skip automatically ?
	  if ( auto_skip )
	  {
	    job.clearError();
	    skip_copying = true;
	    continue;
	  }
	  string tmp2 = l.back().url();
	  SkipDlg_Result r;
	  r = open_SkipDlg( tmp2.c_str(), ( files.size() > 1 ) );
	  if ( r == S_CANCEL )
	  {
	    ftp.ftpDisconnect();
	    error( ERR_USER_CANCELED, "" );
	    m_cmd = CMD_NONE;
	    return;
	  }
	  else if ( r == S_SKIP )
	  {
	    // Clear the error => The current command is not repeated => skipped
	    job.clearError();
	    skip_copying = true;
	    continue;
	  }
	  else if ( r == S_AUTO_SKIP )
	  {
	    // Clear the error => The current command is not repeated => skipped
	    job.clearError();
	    skip_copying = true;
	    continue;
	  }
	  else
	    assert( 0 );
	}
	// Can we prompt the user and ask for a solution ?
	else if ( /* m_bGUI && */ currentError == ERR_DOES_ALREADY_EXIST ||
		  currentError == ERR_DOES_ALREADY_EXIST_FULL )
	{    
	  // Should we skip automatically ?
	  if ( auto_skip )
	  {
	    job.clearError();
	    continue;
	  }

	  RenameDlg_Result r;
	  string n;

	  if ( ProtocolManager::self()->getAutoResume() && m_bCanResume &&
		    currentError != ERR_DOES_ALREADY_EXIST_FULL ) {
	    r = R_RESUME_ALL;
	  }
	  else {
	    RenameDlg_Mode m;

	    // ask for resume only if transfer can be resumed and if it is not
	    // already fully downloaded
	    if ( files.size() > 1 ){
	      if ( m_bCanResume && currentError != ERR_DOES_ALREADY_EXIST_FULL )
		m = (RenameDlg_Mode)(M_MULTI | M_SKIP | M_OVERWRITE | M_RESUME);
	      else
		m = (RenameDlg_Mode)(M_MULTI | M_SKIP | M_OVERWRITE);
	    } else {
	      if ( m_bCanResume && currentError != ERR_DOES_ALREADY_EXIST_FULL )
		m = (RenameDlg_Mode)( M_SINGLE | M_OVERWRITE | M_RESUME);
	      else
		m = (RenameDlg_Mode)( M_SINGLE | M_OVERWRITE);
	    }

	    string tmp2 = l.back().url();
	    r = open_RenameDlg( fit->m_strAbsSource.c_str(), tmp2.c_str(), m, n );
	  }

	  if ( r == R_CANCEL ) 
	  {
	    ftp.ftpDisconnect();
	    error( ERR_USER_CANCELED, "" );
	    m_cmd = CMD_NONE;
	    return;
	  }
	  else if ( r == R_RENAME )
	  {
	    K2URL u( n.c_str() );
	    // The Dialog should have checked this.
	    if ( u.isMalformed() )
	      assert( 0 );
	    // Change the destination name of the current file
	    fit->m_strRelDest = u.path( -1 );
	    // Dont clear error => we will repeat the current command
	  }
	  else if ( r == R_SKIP )
	  {
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
	  else if ( r == R_RESUME )
	  {
	    resume = true;
	    offset = getOffset( l.back().url() );
	    // Dont clear error => we will repeat the current command
	  }
	  else if ( r == R_RESUME_ALL )
	  {
	    resume_all = true;
	    offset = getOffset( l.back().url() );
	    // Dont clear error => we will repeat the current command
	  }
	  else
	    assert( 0 );
	}
	// No need to ask the user, so raise an error
	else
	{    
	  ftp.ftpDisconnect();
	  error( currentError, job.errorText() );
	  m_cmd = CMD_NONE;
	  return;
	}
      }
    }
    while( job.hasError() );

    if ( skip_copying )
      continue;

    // add the offset to processed size
    if ( offset > 0 ) {
      processed_size += offset;
      debug( "kio_ftp : Offset = %ld", offset );
    }

    K2URL tmpurl( "ftp:/" );
    tmpurl.setPath( fit->m_strAbsSource.c_str() );

    debug( "kio_ftp : Opening %s", fit->m_strAbsSource.c_str() );
   
    if ( !ftp.ftpOpen( tmpurl, Ftp::READ, offset ) )
    {
      ftp.ftpDisconnect();
      error( ftp.error(), ftp.errorText() );
      m_cmd = CMD_NONE;
      return;
    }

    // we want to know the processed size even before we read something
    // especially in case we overwrite file => this cleans size to zero
    processedSize( processed_size );

    char buffer[ 4096 ];
    int n;
    do {
      setup_alarm( ProtocolManager::self()->getReadTimeout() ); // start timeout
      n = ftp.read( buffer, 2048 );

      // !!! slow down loop for local testing
      for ( int tmpi = 0; tmpi < 800000; tmpi++ ) ;

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
	ftp.ftpClose();
	ftp.ftpDisconnect();
	m_cmd = CMD_NONE;
	finished();
	return;
      }
    } while ( n > 0 );

    job.dataEnd();
  
    ftp.ftpClose();

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

  debug( "kio_ftp : Copied files %s", dest.c_str() );
  
  // slotDel() handles disconnecting by itself
  if ( _move ) {
    slotDel( _source );
  } else
    ftp.ftpDisconnect();

  finished();

  m_cmd = CMD_NONE;
}


void FtpProtocol::slotGetSize( const char* _url ) {
  
  m_cmd = CMD_GET_SIZE;
  
  // Check wether URL is wellformed
  K2URL usrc( _url );
  if ( usrc.isMalformed() )
    {
      error( ERR_MALFORMED_URL, _url );
      m_cmd = CMD_NONE;
      return;
    }
  if ( strcmp( usrc.protocol(), "ftp" ) != 0L )
    {
      error( ERR_INTERNAL, "kio_ftp got non ftp file as source in copy command" );
      m_cmd = CMD_NONE;
      return;
    }

  debug( "kio_ftp : URL is ok " );

  if ( !ftp.ftpConnect( usrc ) )
  {
    error( ftp.error(), ftp.errorText() );
    m_cmd = CMD_NONE;
    return;
  }
  
  // Get a list of all source files and directories
  list<Copy> files;
  list<CopyDir> dirs;

  debug( "kio_ftp : Executing %s", _url );
  // Did an error occur ?
  int s;
  if ( ( s = listRecursive( usrc.path(), files, dirs, false ) ) == -1 )
    {
      ftp.ftpDisconnect();
      // Error message is already sent
      m_cmd = CMD_NONE;
      return;
    }

  // Tell our client what we 'r' gonna do
  totalSize( s );

  ftp.ftpDisconnect();

  finished();

  m_cmd = CMD_NONE;
}


void FtpProtocol::slotPut( const char *_url, int _mode, bool _overwrite, bool _resume, int _size )
{
  string url_orig = _url;
  string url_part = url_orig + ".part";

  K2URL udest_orig( url_orig );
  K2URL udest_part( url_part );

  bool m_bMarkPartial = ProtocolManager::self()->getMarkPartial();

  if ( udest_orig.isMalformed() )
  {
    error( ERR_MALFORMED_URL, url_orig.c_str() );
    finished();
    m_cmd = CMD_NONE;
    return;
  }

  if ( strcmp( udest_orig.protocol(), "ftp" ) != 0L )
  {
    error(ERR_INTERNAL,"kio_ftp got non ftp file as as destination in put command" );
    finished();
    m_cmd = CMD_NONE;
    return;
  }
  
  m_cmd = CMD_PUT;

  // Connect to the ftp server
  if ( !ftp.ftpConnect( udest_orig ) )
  {
    error( ftp.error(), ftp.errorText() );
    finished();
    m_cmd = CMD_NONE;
    return;
  }

  // Find out, whether we are logged anonymously or not
  // Authorization has been already checked with ftp.ftpConnect( usrc )
  bool b_user = false;
  if ( udest_orig.hasUser() )
    b_user = true;

  FtpEntry* e;

  if ( ( e = ftp.ftpStat( udest_orig ) ) ) {
    // if original file exists, but we are using mark partial -> rename it to XXX.part
    if ( m_bMarkPartial )
      ftp.ftpRename( udest_orig.path(), udest_part.path() );

    if ( !_overwrite && !_resume ) {
      if ( e->size == _size )
	error( ERR_DOES_ALREADY_EXIST_FULL, udest_orig.path() );
      else
	error( ERR_DOES_ALREADY_EXIST, udest_orig.path() );
      
      ftp.ftpDisconnect();
      finished();
      m_cmd = CMD_NONE;
      return;
    }
  } else if ( ( e = ftp.ftpStat( udest_part ) ) ) {
    // if file with extension .part exists but we are not using mark partial
    // -> rename XXX.part to original name
    if ( ! m_bMarkPartial )
      ftp.ftpRename( udest_part.path(), udest_orig.path() );

    if ( !_overwrite && !_resume )
      {
	if ( e->size == _size )
	  error( ERR_DOES_ALREADY_EXIST_FULL, udest_orig.path() );
	else
	  error( ERR_DOES_ALREADY_EXIST, udest_orig.path() );

	ftp.ftpDisconnect();
	finished();
	m_cmd = CMD_NONE;
	return;
      }
  }

  K2URL udest;

  // if we are using marking of partial downloads -> add .part extension
  if ( m_bMarkPartial ) {
    debug( "kio_ftp : Adding .part extension to %s", udest_orig.path() );
    udest = udest_part;
  } else
    udest = udest_orig;


  /* if ( access( udest.path(), W_OK ) == -1 )
  {
    debug("Write Access denied for '%s' %d",udest.path(), errno );
    
    error( ERR_WRITE_ACCESS_DENIED, url.c_str() );
    finished();
    m_cmd = CMD_NONE;
    return;
  } */

  unsigned long offset = 0;

  // set the mode according to offset
  if ( _resume ) {
    offset = e->size;
    debug( "kio_ftp : Offset = %ld", offset );
  }

  if ( !ftp.ftpOpen( udest, Ftp::WRITE, offset ) ) {
    debug( "kio_ftp : ####################### COULD NOT WRITE %s", udest.path() );

    ftp.ftpDisconnect();
    error( ftp.error(), ftp.errorText() );
    finished();
    m_cmd = CMD_NONE;
    return;
  }

  // We are ready for receiving data
  ready();
  
  // Loop until we got 'dataEnd'
  while ( m_cmd == CMD_PUT && dispatch() );

//   if ( (e = ftp.ftpStat( udest )) ) {
//     if ( e->size == _size ) {

//       // after full download rename the file back to original name
//       if ( m_bMarkPartial ) {
// 	if ( ftp.ftpRename( udest.path(), udest_orig.path() ) )
// 	  {
// 	    error( ERR_CANNOT_RENAME, udest_orig.path() );
// 	    ftp.ftpDisconnect();
// 	    finished();
// 	    m_cmd = CMD_NONE;
// 	    return;
// 	  }
//       }

//       // do chmod only after full download
	// set the desired attributes for dir  !!! how ?
//       if ( _mode != -1 ) {
// 	if ( chmod( udest_orig.path(), _mode ) == -1 )
// 	  {
// 	    error( ERR_CANNOT_CHMOD, udest_orig.path() );
// 	    ftp.ftpDisconnect();
// 	    finished();
// 	    m_cmd = CMD_NONE;
// 	    return;
// 	  }
//       }

//     } // if the size is less then minimum -> delete the file
//     else if ( e->size < ProtocolManager::self()->getMinimumKeepSize() ) {
// 	ftp.ftpDelete( udest.path() );
//     }
//   }

  ftp.ftpClose();
  ftp.ftpDisconnect();
  
  // We have done our job => finish
  finished();
  
  m_cmd = CMD_NONE;
}


void FtpProtocol::slotDel( const char *_url )
{
  debug( "kio_ftp : Deleting file %s", _url );

  K2URL usrc( _url );
  if ( usrc.isMalformed() )
    {
      error( ERR_MALFORMED_URL, _url );
      m_cmd = CMD_NONE;
      return;
    }

  if ( strcmp( usrc.protocol(), "ftp" ) != 0L )
    {
      error(ERR_INTERNAL,"kio_ftp got non ftp file for delete command" );
      m_cmd = CMD_NONE;
      return;
    }

  // Connect to the ftp server
  if ( ! ftp.isConnected() )
    if ( !ftp.ftpConnect( usrc ) )
      {
	error( ftp.error(), ftp.errorText() );
	m_cmd = CMD_NONE;
	return;
      }

  FtpEntry* e = ftp.ftpStat( usrc );

  if ( !e  ) {
    error( ERR_DOES_NOT_EXIST, _url );
    m_cmd = CMD_NONE;
    return;
  }

  m_cmd = CMD_DEL;

  deletingFile( _url );

  totalSize( e->size );
  totalFiles( 1 );
  totalDirs( 0 );

  if ( ftp.ftpDelete( usrc.path() ) )
    {
      error( ERR_CANNOT_DELETE, _url );
      m_cmd = CMD_NONE;
      return;
    }

  ftp.ftpDisconnect();

  finished();
  
  m_cmd = CMD_NONE;
}


void FtpProtocol::slotDel( list<string>& _source )
{
  m_cmd = CMD_MDEL;

  // Check wether the URLs are wellformed
  list<string>::iterator soit = _source.begin();
  for( ; soit != _source.end(); ++soit )
  {    
    debug( "kio_ftp : Checking %s", soit->c_str() );
    K2URL usrc( *soit );

    if ( usrc.isMalformed() ) {
      error( ERR_MALFORMED_URL, soit->c_str() );
      m_cmd = CMD_NONE;
      return;
    }

    if ( strcmp( usrc.protocol(), "ftp" ) != 0L ) {
      error(ERR_INTERNAL,"kio_ftp got non ftp file for delete command" );
      error( ERR_INTERNAL, "kio_file got non local file as source in copy command" );
      m_cmd = CMD_NONE;
      return;
    }

    FtpEntry* e = ftp.ftpStat( usrc );

    if ( !e  ) {
      error( ERR_DOES_NOT_EXIST, usrc.path() );
      m_cmd = CMD_NONE;
      return;
    }

    debug( "kio_ftp : Deleting %s", usrc.path() );

    deletingFile( usrc.path() );
  
    if ( ftp.ftpDelete( usrc.path() ) )
      {
	error( ERR_CANNOT_DELETE, usrc.path() );
	m_cmd = CMD_NONE;
	return;
      }
  }

  finished();
  
  m_cmd = CMD_NONE;
}


void FtpProtocol::slotData( void *_p, int _len )
{
  switch( m_cmd )
    {
    case CMD_PUT:
      ftp.write( _p, _len );
      break;
    }
}


void FtpProtocol::slotDataEnd()
{
  switch( m_cmd )
    {
    case CMD_PUT:
      m_cmd = CMD_NONE;
    }
}


long FtpProtocol::listRecursive( const char *_path, list<Copy>& _files, list<CopyDir>& _dirs, bool _rename )
{
  m_bAutoSkip = false;
  
  // Check wether we have to copy the complete directory tree beginning by its root.
  int len = strlen( _path );
  while( len >= 1 && _path[ len - 1 ] == '/' )
    len--;
  if ( len == 0 )
  {
    CopyDir c;
    c.m_strAbsSource = _path;
    if ( _rename )
      c.m_strRelDest = "";
    else
      c.m_strRelDest = "";
    c.m_access = S_IRWXU | S_IRWXO | S_IRWXG;
    c.m_type = S_IFDIR;
    _dirs.push_back( c );
    
    return listRecursive2( "/", c.m_strRelDest.c_str(), _files, _dirs );
  }
  
  string p;
  p.assign( _path, len );
  debug( "kio_ftp : ########## RECURSIVE LISTING %s", p.c_str() );
  
  K2URL tmpurl( "ftp:/" );
  tmpurl.setPath( p.c_str() );
  FtpEntry* e = ftp.ftpStat( tmpurl );
  if ( !e )
  {
    error( ERR_DOES_NOT_EXIST, p.c_str() );
    return -1;
  }
  
  K2URL u( p.c_str() );
  // Should be checked before, but who knows
  if ( u.isMalformed() )
    assert( 0 );

  // Is the source not a directory ? => so just copy it and we are done.
  if ( !S_ISDIR( e->type ) )
  {
    debug( "kio_ftp : not a dir" );
    string fname;
    if ( _rename )
      fname = "";
    else
    {
      fname = u.filename();
      // Should be impossible, but who knows ...
      if ( fname.empty() )
	assert( 0 );
    }

    Copy c;
    c.m_strAbsSource = p;
    c.m_strRelDest = fname;
    c.m_access = e->access;
    c.m_type = e->type;
    c.m_size = e->size;
    _files.push_back( c );
    return e->size;
  }

  // The source is a directory. So we have to go into recursion here.
  string tmp1;
  if ( _rename )
    tmp1 = u.path( 1 );
  else
  {    
    tmp1 = u.directory( true );
    tmp1 += "/";
  }
  string tmp2;
  if ( _rename )
    tmp2 = "";
  else
    tmp2 = u.filename();
  CopyDir c;
  c.m_strAbsSource = p;
  c.m_strRelDest = tmp2;
  c.m_access = e->access;
  c.m_type = e->type;
  _dirs.push_back( c );
  debug( "kio_ftp : ########### STARTING RECURSION with %s and %s",tmp1.c_str(), tmp2.c_str() );
  
  return listRecursive2( tmp1.c_str(), tmp2.c_str(), _files, _dirs );
}


long FtpProtocol::listRecursive2( const char *_abs_path, const char *_rel_path,
				  list<Copy>& _files, list<CopyDir>& _dirs )
{
  long size = 0;
  
  string p = _abs_path;
  p += _rel_path;

  // scanningDir( p.c_str() );
  debug( "kio_ftp : ###### Scanning Dir %s", p.c_str() );
  
  K2URL tmpurl( "ftp:/" );
  tmpurl.setPath( p.c_str() );
  if ( !ftp.ftpOpenDir( tmpurl ) )
  {
    if ( m_bAutoSkip )
      return 0;

    SkipDlg_Result result = open_SkipDlg( p.c_str(), true );
    if ( result == S_CANCEL )
    {
      // error( ERR_CANNOT_ENTER_DIRECTORY, p.c_str() );
      return -1;
    }
    else if ( result == S_AUTO_SKIP )
      m_bAutoSkip = true;
    return 0;
  }

  list<string> recursion;

  debug( "kio_ftp : ##Listing" );
  
  FtpEntry *e;
  while ( ( e = ftp.readdir() ) != 0L )
  {
    debug( "kio_ftp : #%s", e->name.c_str() );
    
    if ( e->name == "." || e->name == ".." )
      continue;
    
    string p2 = p;
    p2 += "/";
    p2 += e->name;
  
    string tmp = _rel_path;
    if ( tmp != "" )
      tmp += "/";
    tmp += e->name;
  
    if ( !S_ISDIR( e->type ) )
    {
      debug( "kio_ftp : Appending '%s' '%s'", p2.c_str(), tmp.c_str() );
      Copy c;
      c.m_strAbsSource = p2;
      c.m_strRelDest = tmp;
      c.m_access = e->access;
      c.m_type = e->type;
      c.m_size = e->size;
      _files.push_back( c );
      size += e->size;
    }
    else
    {
      CopyDir c;
      c.m_strAbsSource = p2;
      c.m_strRelDest = tmp;
      c.m_access = e->access;
      c.m_type = e->type;
      _dirs.push_back( c );

      recursion.push_back( tmp );
    }
  }

  if ( !ftp.ftpCloseDir() )
  {
    // error( ERR_COULD_NOT_CLOSEDIR, p.c_str() );
    return -1;
  }
  
  list<string>::iterator it = recursion.begin();
  for( ; it != recursion.end(); ++it )
  {    
    long s;
    if ( ( s = listRecursive2( _abs_path, it->c_str(), _files, _dirs ) ) == -1 )
      return -1;
    size += s;
  }
  
  return size;
}


void FtpProtocol::slotListDir( const char *_url )
{
  debug( "kio_ftp : =============== LIST %s ===============", _url  );
  
  string url = _url;
  
  K2URL usrc( _url );
  if ( usrc.isMalformed() )
  {
    error( ERR_MALFORMED_URL, url.c_str() );
    m_cmd = CMD_NONE;
    return;
  }

  if ( strcmp( usrc.protocol(), "ftp" ) != 0L )
  {
    error( ERR_INTERNAL, "kio_ftp got non ftp file as source in copy command" );
    m_cmd = CMD_NONE;
    return;
  }

  /*
  struct stat buff;
  if ( stat( usrc.path(), &buff ) == -1 )
  {
    error( ERR_DOES_NOT_EXIST, url.c_str() );
    m_cmd = CMD_NONE;
    return;
  }

  if ( !S_ISDIR( buff.st_mode ) )
  {
    error( ERR_IS_FILE, url.c_str() );
    m_cmd = CMD_NONE;
    return;
  } */

  m_cmd = CMD_LIST;
  
  if ( !ftp.opendir( usrc ) )
  {
    error( ERR_CANNOT_ENTER_DIRECTORY, url.c_str() );
    m_cmd = CMD_NONE;
    return;
  }

  FtpEntry* e;
  while ( ( e = ftp.readdir() ) )
  {
    if ( e->name == "." || e->name == ".." )
      continue;

    debug( "kio_ftp : Listing %s", e->name.c_str() );

    UDSEntry entry;
    UDSAtom atom;
    atom.m_uds = UDS_NAME;
    atom.m_str = e->name;
    entry.push_back( atom );
      
    atom.m_uds = UDS_FILE_TYPE;
    atom.m_long = e->type;
    entry.push_back( atom );
    atom.m_uds = UDS_SIZE;
    atom.m_long = e->size;
    entry.push_back( atom );
    /* atom.m_uds = UDS_MODIFICATION_TIME;
    atom.m_long = buff.st_mtime;
    entry.push_back( atom ); */
    atom.m_uds = UDS_ACCESS;
    atom.m_long = e->access;
    entry.push_back( atom );
    atom.m_uds = UDS_USER;
    atom.m_str = e->owner;
    entry.push_back( atom );
    atom.m_uds = UDS_GROUP;
    atom.m_str = e->group;
    entry.push_back( atom );
    atom.m_uds = UDS_LINK_DEST;
    atom.m_str = e->link;
    entry.push_back( atom );
    /* atom.m_uds = UDS_ACCESS_TIME;
    atom.m_long = buff.st_atime;
    entry.push_back( atom );    
    atom.m_uds = UDS_CREATION_TIME;
    atom.m_long = buff.st_ctime;
    entry.push_back( atom ); */

    listEntry( entry );
  }

  debug( "kio_ftp : ============= COMPLETED LIST ============" );
 
  ftp.closedir();
  
  debug( "kio_ftp : ============= COMPLETED LIST 2 ============" );

  m_cmd = CMD_NONE;

  finished();

  debug( "kio_ftp : =============== BYE ===========" );
}


void FtpProtocol::slotTestDir( const char *_url )
{
  string url = _url;
  
  K2URL usrc( _url );
  if ( usrc.isMalformed() )
  {
    error( ERR_MALFORMED_URL, url.c_str() );
    m_cmd = CMD_NONE;
    return;
  }
    
  if ( strcmp( usrc.protocol(), "ftp" ) != 0L )
  {
    error( ERR_INTERNAL, "kio_ftp got non ftp file as source in copy command" );
    m_cmd = CMD_NONE;
    return;
  }
  debug( "kio_ftp : =============== slotTestDir ==============" );
  FtpEntry* e;
  if ( !( e = ftp.stat( usrc ) ) )
  {
    debug( "kio_ftp : =========== ERROR ========" );
    error( ERR_DOES_NOT_EXIST, url.c_str() );
    m_cmd = CMD_NONE;
    return;
  }
  
  if ( S_ISDIR( e->type ) )
  {
    debug( "kio_ftp : ========== DONE DIR ========= %s", _url );    
    isDirectory();
  }
  else
  {
    debug( "kio_ftp : ========== DONE FILE ========= %s", _url );
    isFile();
  }

  finished();
}


void FtpProtocol::jobError( int _errid, const char *_txt )
{
  if ( !m_bIgnoreJobErrors )
    error( _errid, _txt );
}


// Ftp* FtpProtocol::getFtp() {
//   return ftp;
// }


/*************************************
 *
 * FtpIOJob
 *
 *************************************/

FtpIOJob::FtpIOJob( Connection *_conn, FtpProtocol *_Ftp ) : IOJob( _conn )
{
  m_pFtp = _Ftp;
}
  

void FtpIOJob::slotError( int _errid, const char *_txt )
{
  IOJob::slotError( _errid, _txt );
  m_pFtp->jobError( _errid, _txt );
}


int check( Connection *_con )
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
