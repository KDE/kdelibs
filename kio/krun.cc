// $Id$

// comment the following line out if you want debug output
#define NO_DEBUG

#ifdef NO_DEBUG
#ifdef NDEBUG
#undef NO_DEBUG
#else
#undef NDEBUG
#define NDEBUG
#endif
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kio_job.h"
#include "krun.h"
#include "kservices.h"
#include "kuserprofile.h"
#include "kmimetypes.h"
#include "kmimemagic.h"
#include "kio_error.h"

#include <qmessagebox.h>

#include <kurl.h>
#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kprotocolmanager.h>
#include <kstddirs.h>

KFileManager * KFileManager::pFileManager = 0L;
KServiceProvider * KServiceProvider::pServiceProvider = 0L;

bool KRun::runURL( const char *_url, const char *_mimetype )
{

  if ( strcmp( _mimetype, "text/html" ) == 0 )
  {
    KFileManager::getFileManager()->openFileManagerWindow( _url );
    return true;
  }
  else if ( strcmp( _mimetype, "inode/directory" ) == 0 )
  {
    KFileManager::getFileManager()->openFileManagerWindow( _url );
    return true;
  }
  else if ( strcmp( _mimetype, "inode/directory-locked" ) == 0 )
  {
    QString tmp = i18n("Can not enter\n%1\nAccess denied").arg(_url);
    QMessageBox::critical( 0L, i18n("Error"), tmp, i18n("OK") );
    return false;
  }
  else if ( strcmp( _mimetype, "application/x-desktop" ) == 0 )
  {
    KURL u( _url );
    if ( u.isLocalFile() )
      return KDEDesktopMimeType::run( _url, true );
  }

  QStringList lst;
  lst.append( _url );

  const KSharedPtr<KService> offer = KServiceProvider::getServiceProvider()->service( _mimetype );
  
  if ( !offer )
  {
    //HACK TODO: OpenWith
    return false;
  }
  
  return KRun::run( *offer, lst );
}

void KRun::shellQuote( QString &_str )
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

bool KRun::run( const KService& _service, QStringList& _urls )
{
  QString icon = _service.icon();
  QString miniicon = _service.icon();
  QString name = _service.name();

  return run( _service.exec(), _urls, name, icon, miniicon );
}

bool KRun::run( const QString& _exec, QStringList& _urls, const QString& _name,
		const QString& _icon, const QString& _mini_icon, const QString& _desktop_file )
{
  bool b_local_files = true;

  QString U = "",F = "",D = "",N = "";

  QStringList::Iterator it = _urls.begin();
  for( ; it != _urls.end(); ++it )
  {
    KURL url( *it );
    if ( url.isMalformed() )
    {
      QString tmp = i18n( "Malformed URL" );
      tmp += "\n";
      tmp += *it;
      QMessageBox::critical( 0L, i18n( "KFM Error" ), tmp, i18n( "OK" ) );
      return false;
    }

    if ( !url.isLocalFile() )
      b_local_files = false;

    QString tmp = *it;
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

  QString exec = _exec;
  bool b_local_app = false;
  if ( exec.find( "%u" ) == -1 )
    b_local_app = true;

  // Did the user forget to append something like '%f' ?
  // If so, then assume that '%f' is the right choice => the application
  // accepts only local files.
  if ( exec.find( "%f" ) == -1 && exec.find( "%u" ) == -1 && exec.find( "%n" ) == -1 &&
       exec.find( "%d" ) == -1 && exec.find( "%F" ) == -1 && exec.find( "%U" ) == -1 &&
       exec.find( "%N" ) == -1 && exec.find( "%D" ) == -1 )
    exec += " %f";

  // Can we pass multiple files on the command line or do we have to start the application for every single file ?
  bool b_allow_multiple = false;
  if ( exec.find( "%F" ) != -1 || exec.find( "%U" ) != -1 || exec.find( "%N" ) != -1 ||
       exec.find( "%D" ) != -1 )
    b_allow_multiple = true;

  int pos;

  QString name = _name;
  shellQuote( name );
  while ( ( pos = exec.find( "%c" ) ) != -1 )
    exec.replace( pos, 2, name );

  QString icon = _icon;
  shellQuote( icon );
  if ( !icon.isEmpty() )
    icon.insert( 0, "-icon " );
  while ( ( pos = exec.find( "%i" ) ) != -1 )
    exec.replace( pos, 2, icon );

  QString mini_icon = _mini_icon;
  shellQuote( mini_icon );
  if ( !mini_icon.isEmpty() )
    mini_icon.insert( 0, "-miniicon " );
  while ( ( pos = exec.find( "%m" ) ) != -1 )
    exec.replace( pos, 2, mini_icon );

  while ( ( pos = exec.find( "%k" ) ) != -1 )
    exec.replace( pos, 2, _desktop_file );

  // The application accepts only local files ?
  if ( b_local_app && !b_local_files )
  {
    return runOldApplication( _exec, _urls, b_allow_multiple );
  }

  if ( b_allow_multiple || _urls.isEmpty() )
  {	
    while ( ( pos = exec.find( "%f" )) != -1 )
      exec.replace( pos, 2, "" );
    while ( ( pos = exec.find( "%n" )) != -1 )
      exec.replace( pos, 2, "" );
    while ( ( pos = exec.find( "%d" )) != -1 )
      exec.replace( pos, 2, "" );
    while ( ( pos = exec.find( "%u" )) != -1 )
      exec.replace( pos, 2, "" );

    while ( ( pos = exec.find( "%F" )) != -1 )
      exec.replace( pos, 2, F );
    while ( ( pos = exec.find( "%N" )) != -1 )
      exec.replace( pos, 2, N );
    while ( ( pos = exec.find( "%D" )) != -1 )
      exec.replace( pos, 2, N );
    while ( ( pos = exec.find( "%U" )) != -1 )
      exec.replace( pos, 2, U );

    return run( exec );
  }

  it = _urls.begin();
  for( ; it != _urls.end(); ++it )
  {
    QString e = exec;
    KURL url( *it );
    ASSERT( !url.isMalformed() );
    QString f ( url.path( -1 ) );
    shellQuote( f );
    QString d ( url.directory() );
    shellQuote( d );
    QString n ( url.filename() );
    shellQuote( n );
    QString u ( *it );
    shellQuote( u );

    while ( ( pos = e.find( "%f" )) != -1 )
      e.replace( pos, 2, f );
    while ( ( pos = e.find( "%n" )) != -1 )
      e.replace( pos, 2, n );
    while ( ( pos = e.find( "%d" )) != -1 )
      e.replace( pos, 2, d );
    while ( ( pos = e.find( "%u" )) != -1 )
      e.replace( pos, 2, u );

    return run( e );
  }

  return true;
}

bool KRun::run( const QString& _cmd )
{
  kdebug( KDEBUG_INFO, 7010, "Running %s", _cmd.ascii() );

  QString exec = _cmd;
  exec += " &";
  system( exec.ascii() );

  return true;
}

bool KRun::runOldApplication( const QString& , QStringList& _urls, bool _allow_multiple )
{
  char **argv = 0L;

  // find kfmexec in $PATH
  QString kfmexec = KStandardDirs::findExe( "kfmexec" );
  
  if ( _allow_multiple )
  {
    argv = new char*[ _urls.count() + 3 ];
    argv[ 0 ] = (char*)kfmexec.data();

    int i = 1;
    QStringList::Iterator it = _urls.begin();
    for( ; it != _urls.end(); ++it )
      argv[ i++ ] = (char*)(*it).ascii();
    argv[ i ] = 0;

    int pid;
    if ( ( pid = fork() ) == 0 )
    {
      execvp( argv[0], argv );
      _exit(1);
    }
  }
  else
  {
    QStringList::Iterator it = _urls.begin();
    for( ; it != _urls.end(); ++it )
    {
      argv = new char*[ 3 ];
      argv[ 0 ] = (char*)kfmexec.data();
      argv[ 1 ] = (char*)(*it).ascii();
      argv[ 2 ] = 0;

      int pid;
      if ( ( pid = fork() ) == 0 )
      {
	execvp( argv[0], argv );
	_exit(1);
      }
    }
  }

  return true;
}

KRun::KRun( const QString& _url, mode_t _mode, bool _is_local_file, bool _auto_delete )
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

  KURL url( m_strURL );

  if ( !m_bIsLocalFile && url.isLocalFile() )
    m_bIsLocalFile = true;

  if ( m_bIsLocalFile )
  {
    if ( m_mode == 0 )
    {
      struct stat buff;
      if ( stat( url.path().ascii(), &buff ) == -1 )
      {
	// HACK: Use general error message function ( yet to do )
	kioErrorDialog( ERR_DOES_NOT_EXIST, m_strURL.ascii() );
	m_bFault = true;
	m_bFinished = true;
	m_timer.start( 0, true );
	return;
      }
      m_mode = buff.st_mode;
    }

    KMimeType* mime = KMimeType::findByURL( url, m_mode, m_bIsLocalFile );
    assert( mime );
    kdebug( KDEBUG_INFO, 7010, "MIME TYPE is %s", mime->mimeType().ascii() );
    foundMimeType( mime->mimeType().ascii() );
    return;
  }

  // Did we already get the information that it is a directory ?
  if ( S_ISDIR( m_mode ) )
  {
    foundMimeType( "inode/directory" );
    return;
  }

  // Let's see whether it is a directory
  if ( !KProtocolManager::self().supportsListing( url.protocol() ) )
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
  job->setGUImode( KIOJob::NONE );
  job->testDir( m_strURL.ascii() );
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
  job->setGUImode( KIOJob::NONE );
  job->preget( m_strURL.ascii(), 2048 );
}

void KRun::slotTimeout()
{
  if ( m_bInit )
  {
    m_bInit = false;
    init();
    return;
  }

  if ( m_bFault ){
      emit error();
  }
  if ( m_bFinished ){
      emit finished();
  }

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
  if ( !result || result->mimeType().isEmpty())
    foundMimeType( "application/octet-stream" );
  else
    foundMimeType( result->mimeType().ascii() );
}

void KRun::foundMimeType( const char *_type )
{
  kdebug( KDEBUG_INFO, 7010, "Resulting mime type is %s", _type );

  // Automatically unzip stuff
  if ( strcmp( _type, "application/x-gzip" ) == 0 )
  {
    KURL::List lst = KURL::split( m_strURL );
    if ( lst.isEmpty() )
    {
      QString tmp = i18n( "Malformed URL" );
      tmp += "\n";
      tmp += m_strURL;
      QMessageBox::critical( 0L, i18n( "Error" ), tmp, i18n( "OK" ) );
      return;
    }

    lst.prepend( KURL( "gzip:/decompress" ) );
    // Move the HTML style reference to the leftmost URL
    KURL::List::Iterator it = lst.begin();
    ++it;
    (*lst.begin()).setRef( (*it).ref() );
    (*it).setRef( QString::null );

    // Create the new URL
    m_strURL = KURL::join( lst );

    kdebug( KDEBUG_INFO, 7010, "Now trying with %s", m_strURL.ascii() );

    killJob();

    KIOJob* job = new KIOJob();
    connect( job, SIGNAL( sigMimeType( int, const char* ) ), this, SLOT( slotMimeType( int, const char* ) ) );
    connect( job, SIGNAL( sigPreData( int, const char*, int ) ),
	     this, SLOT( slotPreData( int, const char*, int ) ) );
    job->setAutoDelete( false );
    m_jobId = job->id();
    job->setGUImode( KIOJob::NONE );
    job->preget( m_strURL.ascii(), 2048 );
    return;
  }

  if (KRun::runURL( m_strURL.ascii(), _type )){
      emit finished(); // tell owner that we finished (David)
  }
  else{
      emit error();
  }

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

/****************/
bool KFileManager::openFileManagerWindow( const char *_url )
{
  QString cmd = "kfmclient openURL ";
  cmd += _url;
  system( cmd.ascii() );
  return true; // assume kfmclient succeeded
}

const KSharedPtr<KService> KServiceProvider::service( const char *mime_type )
{
  KServiceTypeProfile::OfferList offers = KServiceTypeProfile::offers( mime_type );

  if ( offers.count() == 0 || !(*offers.begin()).allowAsDefault() )
  {
    kdebug( KDEBUG_INFO, 7010, "No Offers" );
    return 0L;
  }

  KService *s = (KService*)( *offers.begin() ).service();
  s->ref();
  return KSharedPtr<KService>( s );
}

#include "krun.moc"

#ifdef NO_DEBUG
#undef NO_DEBUG
#undef NDEBUG
#endif
