#include "krun.h"
#include "kservices.h"
#include "kmimetypes.h"
#include "kmimemagic.h"
#include "kio_job.h"
#include "kio_error.h"
#include "kio_manager.h"

#include <qmsgbox.h>

#include <k2url.h>
#include <kapp.h>
#include <kstring.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// This function is implemeneted in kfmgui.cc and in kfmlib
extern void openFileManagerWindow( const char *_url );

bool KRun::runURL( const char *_url, const char *_mimetype )
{
  
  if ( strcmp( _mimetype, "text/html" ) == 0 )
  {
    openFileManagerWindow( _url );
    return true;
  }
  else if ( strcmp( _mimetype, "inode/directory" ) == 0 )
  {
    openFileManagerWindow( _url );
    return true;
  }
  else if ( strcmp( _mimetype, "inode/directory-locked" ) == 0 )
  {
    QString tmp;
    ksprintf( &tmp, i18n("Can not enter\n%s\nAccess denied"), _url );
    QMessageBox::critical( 0L, i18n("Error"), tmp, i18n("Ok") );
    return false;
  }
  else if ( strcmp( _mimetype, "application/x-kdelnk" ) == 0 )
  {
    K2URL u( _url );
    if ( u.isLocalFile() )
      return KDELnkMimeType::run( _url, true );
  }
  
  // Get all services for this mime type
  list<KService::Offer> offers;
  KService::findServiceByServiceType( _mimetype, offers );

  if ( offers.size() == 0 || !offers.front().allowAsDefault() )
  {
    kdebug( KDEBUG_INFO, 7010, "No Offers" );
    // HACK TODO: OpenWith
    return false;
  }
  
  QStrList lst;
  lst.append( _url );
  // Choose the first service from the list and do the job
  return KRun::run( offers.front().m_pService, lst );
}

void KRun::shellQuote( string &_str )
{
  int i = 0;
  while( ( i = _str.find( '\'', i ) ) != -1 )
  {    
    _str.replace( i, 1, "'\''" );
    i += 4;
  }
  
  _str.insert( 0, "'" );
  _str += "'";
}

bool KRun::run( KService* _service, QStrList& _urls )
{
  QString icon = _service->icon();
  QString miniicon = _service->icon();
  QString name = _service->name();

  return run( _service->exec(), _urls, name, icon, miniicon );
}

bool KRun::run( const char *_exec, QStrList& _urls, const char *_name, const char *_icon,
		  const char *_mini_icon, const char *_kdelnk_file )
{
  bool b_local_files = true;
  
  string U = "",F = "",D = "",N = "";
  
  const char* it = _urls.first();
  for( ; it != 0L; it = _urls.next() )
  {
    K2URL url( it );
    if ( url.isMalformed() )
    {
      string tmp = i18n( "Malformed URL" );
      tmp += "\n";
      tmp += it;
      QMessageBox::critical( 0L, i18n( "KFM Error" ), tmp.c_str(), i18n( "Ok" ) );
      return false;
    }

    if ( !url.isLocalFile() )
      b_local_files = false;

    string tmp = it;
    shellQuote( tmp );
    U += tmp;
    U += " ";
    tmp = url.directory();
    shellQuote( tmp );
    D += tmp;
    D += " ";
    tmp = url.filename();
    shellQuote( tmp );
    N += tmp;
    N += " ";
    tmp = url.path( -1 );
    shellQuote( tmp );
    F += tmp;
    F += " ";
  }
  
  string exec = _exec;
  bool b_local_app = false;
  if ( exec.find( "%u" ) == string::npos )
    b_local_app = true;
    
  // Did the user forget to append something like '%f' ?
  // If so, then assume that '%f' is the right joice => the application
  // accepts only local files.
  if ( exec.find( "%f" ) == string::npos && exec.find( "%u" ) == string::npos && exec.find( "%n" ) == string::npos &&
       exec.find( "%d" ) == string::npos && exec.find( "%F" ) == string::npos && exec.find( "%U" ) == string::npos &&
       exec.find( "%N" ) == string::npos && exec.find( "%D" ) == string::npos )
    exec += " %f";
  
  // Can we pass multiple files on the command line or do we have to start the application for every single file ?
  bool b_allow_multiple = false;
  if ( exec.find( "%F" ) != string::npos || exec.find( "%U" ) != string::npos || exec.find( "%N" ) != string::npos ||
       exec.find( "%D" ) != string::npos )
    b_allow_multiple = true;

  int pos;
  
  string name = _name;
  shellQuote( name );
  while ( ( pos = exec.find( "%c" ) ) != (int)string::npos )
    exec.replace( pos, 2, name );

  string icon = _icon;
  shellQuote( icon );
  if ( !icon.empty() )
    icon.insert( 0, "-icon " );
  while ( ( pos = exec.find( "%i" ) ) != (int)string::npos )
    exec.replace( pos, 2, icon );

  string mini_icon = _mini_icon;
  shellQuote( mini_icon );
  if ( !mini_icon.empty() )
    mini_icon.insert( 0, "-miniicon " );
  while ( ( pos = exec.find( "%m" ) ) != (int)string::npos )
    exec.replace( pos, 2, mini_icon );

  if ( !_kdelnk_file )
    _kdelnk_file = "";
  while ( ( pos = exec.find( "%k" ) ) != (int)string::npos )
    exec.replace( pos, 2, _kdelnk_file );

  // The application accepts only local files ?
  if ( b_local_app && !b_local_files )
  {
    return runOldApplication( _exec, _urls, b_allow_multiple );
  }
  
  if ( b_allow_multiple || _urls.isEmpty() )
  {	
    while ( ( pos = exec.find( "%f" )) != (int)string::npos )
      exec.replace( pos, 2, "" );
    while ( ( pos = exec.find( "%n" )) != (int)string::npos )
      exec.replace( pos, 2, "" );
    while ( ( pos = exec.find( "%d" )) != (int)string::npos )
      exec.replace( pos, 2, "" );
    while ( ( pos = exec.find( "%u" )) != (int)string::npos )
      exec.replace( pos, 2, "" );

    while ( ( pos = exec.find( "%F" )) != (int)string::npos )
      exec.replace( pos, 2, F );
    while ( ( pos = exec.find( "%N" )) != (int)string::npos )
      exec.replace( pos, 2, N );
    while ( ( pos = exec.find( "%D" )) != (int)string::npos )
      exec.replace( pos, 2, N );
    while ( ( pos = exec.find( "%U" )) != (int)string::npos )
      exec.replace( pos, 2, U );

    return run( exec.c_str() );
  }
  
  it = _urls.first();
  for( ; it != 0L; it = _urls.next() )
  {
    string e = exec;
    K2URL url( it );
    assert( !url.isMalformed() );
    string f = url.path( -1 );
    shellQuote( f );
    string d = url.directory();
    shellQuote( d );
    string n = url.filename();
    shellQuote( n );
    string u = it;
    shellQuote( u );
   
    while ( ( pos = e.find( "%f" )) != (int)string::npos )
      e.replace( pos, 2, f );
    while ( ( pos = e.find( "%n" )) != (int)string::npos )
      e.replace( pos, 2, n );
    while ( ( pos = e.find( "%d" )) != (int)string::npos )
      e.replace( pos, 2, d );
    while ( ( pos = e.find( "%u" )) != (int)string::npos )
      e.replace( pos, 2, u );

    return run( e.c_str() );
  }
  
  return true;
}

bool KRun::run( const char *_cmd )
{
  kdebug( KDEBUG_INFO, 7010, "Running %s", _cmd );
  
  QString exec = _cmd;
  exec += " &";
  system( exec );

  return true;
}

bool KRun::runOldApplication( const char *_exec, QStrList& _urls, bool _allow_multiple )
{
  char **argv = 0L;
  
  QString kfmexec = kapp->kde_bindir();
  kfmexec += "/kfmexec";
  
  if ( _allow_multiple )
  {
    argv = new char*[ _urls.count() + 3 ];
    argv[ 0 ] = (char*)kfmexec.data();

    int i = 1;
    const char* s;
    for( s = _urls.first(); s != 0L; s = _urls.next() )
      argv[ i++ ] = (char*)s;
    argv[ i ] = 0;
      
    int pid;
    if ( ( pid = fork() ) == 0 )
    {    
      execvp( argv[0], argv );
      exit(1);
    }
  }
  else
  {
    const char* s;
    for( s = _urls.first(); s != 0L; s = _urls.next() )
    {
      argv = new char*[ 3 ];
      argv[ 0 ] = (char*)kfmexec.data();
      argv[ 1 ] = (char*)s;
      argv[ 2 ] = 0;
      
      int pid;
      if ( ( pid = fork() ) == 0 )
      {    
	execvp( argv[0], argv );
	exit(1);
      }
    }
  }
  
  return true;
}

KRun::KRun( const char *_url, mode_t _mode, bool _is_local_file, bool _auto_delete )
{
  m_bFault = false;
  m_bAutoDelete = _auto_delete;
  m_bFinished = false;
  m_jobId = 0;
  m_strURL = _url;
  m_bScanFile = false;
  m_bIsDirectory = false;
  m_bIsLocalFile = _is_local_file;
  m_mode = _mode;

  // Start the timer. This means we will return to the event
  // loop and do initialization afterwards.
  // Reason: We must complete the constructor before we do anything else.
  m_bInit = true;
  connect( &m_timer, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
  m_timer.start( 0, true );
}

void KRun::init()
{
  kdebug( KDEBUG_INFO, 7010, "INIT called" );
  
  K2URL url( m_strURL );
  
  if ( !m_bIsLocalFile && url.isLocalFile() )
    m_bIsLocalFile = true;
  
  if ( m_bIsLocalFile )
  {
    if ( m_mode == 0 )
    {
      struct stat buff;
      if ( stat( url.path(), &buff ) == -1 )
      {
	// HACK: Use general error message function ( yet to do )
	kioErrorDialog( ERR_DOES_NOT_EXIST, m_strURL );
	m_bFault = true;
	m_bFinished = true;
	m_timer.start( 0, true );
	return;
      }
      m_mode = buff.st_mode;
    }

    KMimeType* mime = KMimeType::findByURL( url, m_mode, m_bIsLocalFile );
    kdebug( KDEBUG_INFO, 7010, "MIME TYPE is %s", mime->mimeType() );
    assert( mime );
    foundMimeType( mime->mimeType() );
    return;
  }

  // Did we already get the information that it is a directory ?
  if ( S_ISDIR( m_mode ) )
  {
    foundMimeType( "inode/directory" );
    return;
  }

  // Lets see wether it is a directory
  K2URLList lst;
  K2URL::split( m_strURL, lst );
  if ( !ProtocolManager::self()->supportsListing( lst.back().protocol() ) )
  {
    kdebug( KDEBUG_INFO, 7010, "##### NOT A DIRECTORY" );
    // No support for listing => we can scan the file
    scanFile();
    return;
  }

  kdebug( KDEBUG_INFO, 7010, "##### TESTING DIRECTORY" );
  
  // It may be a directory
  KIOJob* job = new KIOJob();
  connect( job, SIGNAL( sigIsDirectory( int ) ), this, SLOT( slotIsDirectory( int ) ) );
  connect( job, SIGNAL( sigIsFile( int ) ), this, SLOT( slotIsFile( int ) ) );
  connect( job, SIGNAL( sigFinished( int ) ), this, SLOT( slotFinished( int ) ) );
  connect( job, SIGNAL( sigError( int, int, const char* ) ),
	   this, SLOT( slotError( int, int, const char* ) ) );

  job->setAutoDelete( false );
  m_jobId = job->id();
  job->enableGUI( false );
  job->testDir( m_strURL );
}

KRun::~KRun()
{
  m_timer.stop();
  killJob();
}

void KRun::scanFile()
{
  kdebug( KDEBUG_INFO, 7010, "###### Scanning file %s", m_strURL.data() );
  
  KIOJob* job = new KIOJob();
  connect( job, SIGNAL( sigMimeType( int, const char* ) ), this, SLOT( slotMimeType( int, const char* ) ) );
  connect( job, SIGNAL( sigPreData( int, const char*, int ) ),
	   this, SLOT( slotPreData( int, const char*, int ) ) );
  connect( job, SIGNAL( sigError( int, int, const char* ) ),
	   this, SLOT( slotError( int, int, const char* ) ) );
  job->setAutoDelete( false );
  m_jobId = job->id();
  job->enableGUI( false );
  job->preget( m_strURL, 2048 );
}

void KRun::slotTimeout()
{
  if ( m_bInit )
  {
    m_bInit = false;
    init();
    return;
  }
  
  if ( m_bFault )
    emit error();
  if ( m_bFinished )
    emit finished();

  if ( m_bScanFile )
  {
    m_bScanFile = false;
    scanFile();
    return;
  }
  else if ( m_bIsDirectory )
  {    
    m_bIsDirectory = false;
    foundMimeType( "inode/directory" );
    return;
  }
  
  if ( m_bAutoDelete )
  {
    delete this;
    return;
  }
}

void KRun::slotIsDirectory( int /* _id */ )
{
  m_bIsDirectory = true;
}

void KRun::slotIsFile( int /* _id */ )
{
  // Ok, when the job is finished we want to scan the file.
  // But we wait until the job is finished => we can reuse the
  // protocol server
  m_bScanFile = true;
}

void KRun::slotFinished( int /* _id */ )
{
  kdebug( KDEBUG_INFO, 7010, "####### FINISHED" );

  if ( m_bFault )
  {    
    m_bFinished = true;

    if ( m_bAutoDelete )
    {
      delete this;
      return;
    }
    return;
  }
  
  // Start the timer. Once we get the timer event this
  // protocol server is back in the pool and we can reuse it.
  // This gives better performance then starting a new slave
  if ( m_bScanFile || m_bIsDirectory )
    m_timer.start( 0, true );
}

void KRun::slotError( int, int _errid, const char *_errortext )
{
  kdebug( KDEBUG_ERROR, 7010,"######## ERROR %d %s", _errid, _errortext );
  // HACK
  // Display error message
  kioErrorDialog( _errid, _errortext );

  m_bFault = true;
  // m_timer.start( 0, true ); 
}

void KRun::slotMimeType( int, const char *_type )
{
  kdebug( KDEBUG_INFO, 7010, "######## MIMETYPE %s", _type );
  foundMimeType( _type );
}

void KRun::slotPreData( int, const char *_data, int _len )
{
  kdebug( KDEBUG_INFO, 7010, "Got pre data" );
  KMimeMagicResult* result = KMimeMagic::self()->findBufferType( _data, _len );

  // If we still did not find it, we must assume the default mime type
  if ( !result || !result->mimeType() || result->mimeType()[0] == 0 )
    foundMimeType( "application/octet-stream" );
  else
    foundMimeType( result->mimeType() );
}

void KRun::foundMimeType( const char *_type )
{
  kdebug( KDEBUG_INFO, 7010, "Resulting mime type is %s", _type );
  
  if ( strcmp( _type, "application/x-gzip" ) == 0 )
  {
    list<K2URL> lst;
    if ( !K2URL::split( m_strURL, lst ) )
    {
      string tmp = i18n( "Malformed URL" );
      tmp += "\n";
      tmp += m_strURL;
      QMessageBox::critical( 0L, i18n( "Error" ), tmp.c_str(), i18n( "Ok" ) );
      return;
    }

    string tmp = lst.back().ref();
    string ref;
    if ( tmp.empty() )
      ref = "gzip:/decompress";
    else
    {
      string ref = "gzip:/decompress#";
      ref += tmp;
    }
    lst.back().setRef( ref.c_str() );
    
    // Create the new URL
    K2URL::join( lst, tmp );
    m_strURL = tmp.c_str();
    
    kdebug( KDEBUG_INFO, 7010, "Now trying with %s", m_strURL.data() );
    
    killJob();

    KIOJob* job = new KIOJob();
    connect( job, SIGNAL( sigMimeType( int, const char* ) ), this, SLOT( slotMimeType( int, const char* ) ) );
    connect( job, SIGNAL( sigPreData( int, const char*, int ) ),
	     this, SLOT( slotPreData( int, const char*, int ) ) );
    job->setAutoDelete( false );
    m_jobId = job->id();
    job->enableGUI( false );
    job->preget( m_strURL, 2048 );
    return;
  }

  KRun::runURL( m_strURL, _type );

  if ( m_bAutoDelete )
  {
    delete this;
    return;
  }
  /*
  m_bFinished = true;
  m_timer.start( 0, true ); */
}

void KRun::killJob()
{
  if ( m_jobId )
  {
    KIOJob* job = KIOJob::find( m_jobId );
    if ( job )
      job->kill();
    m_jobId = 0;
  }
}

#include "krun.moc"
