#include "kio_job.h"
#include "kio_progress_dlg.h"
#include "kio_manager.h"

#include <qsocknot.h>
#include <qdialog.h>
#include <qpushbutton.h>

#include <k2url.h>
#include <kapp.h>
#include <kwm.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/wait.h>

#include <string>
#include <iostream>

#ifdef __FreeBSD__
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#endif      

int KIOJob::s_id = 1;
map<int,KIOJob*>* KIOJob::s_mapJobs = 0L;

KIOJob::KIOJob() : QObject(), IOJob( 0L )
{
  m_id = ++s_id;

  assert( s_mapJobs );
  (*s_mapJobs)[ m_id ] = this;
  
  m_bAutoDelete = true;
  m_bGUI = true;
  m_bStartIconified = false;
  m_bCacheToPool = true;

  m_iTotalSize = 0;
  m_iTotalFiles = 0;
  m_iTotalDirs = 0;
  m_iProcessedSize = 0;

  m_pCopyProgressDlg = 0L;
  m_pDialog = 0L;
  m_pSlave = 0L;
  m_pNotifier = 0L;
  m_bPreGet = false;
  m_iPreGetBufferSize = 0;
  m_iPreGetBufferMaxSize = 0;
  m_pPreGetBuffer = 0L;
  m_bPreGetFinished = false;
}


KIOJob::~KIOJob()
{
  if ( m_pPreGetBuffer )
    delete [] m_pPreGetBuffer;
  if ( m_pDialog )
    delete m_pDialog;
  
  clean();  
}


void KIOJob::initStatic()
{
  if ( !s_mapJobs )
    s_mapJobs = new map<int,KIOJob*>;
}


KIOJob* KIOJob::find( int _id )
{
  map<int,KIOJob*>::iterator it = s_mapJobs->find( _id );
  if ( it == s_mapJobs->end() )
    return 0L;
  
  return it->second;
}


void KIOJob::kill( bool quiet )
{
  if ( !quiet )
    emit sigCanceled( m_id );

  clean();
  
  // Time to die ...
  delete this;
  return;
}


void KIOJob::clean()
{
  assert( s_mapJobs );
  if ( m_id )
  {    
    assert( s_mapJobs->find( m_id ) != s_mapJobs->end() );
    s_mapJobs->erase( m_id );
    m_id = 0;
  }
  
  if ( m_pNotifier )
  {
    m_pNotifier->setEnabled( false );
    delete m_pNotifier;
    m_pNotifier = 0L;
  }
  
  if ( m_pCopyProgressDlg )
  {
    delete m_pCopyProgressDlg;
    m_pCopyProgressDlg = 0L;
  }

  cerr << "clean()\n";

  // Do not putback the slave into the pool because we may have
  // died in action. This means that the slave is in an undefined
  // state. If the job has finished successfully then
  // 'slotFinished' already handed the slave back to the pool.
  if ( m_pSlave )
  {    
    delete m_pSlave;
    m_pSlave = 0L;
  }   
}


// close progress dialog if it exists
void KIOJob::hideGUI() {
  if ( m_pCopyProgressDlg ){
    delete m_pCopyProgressDlg;
    m_pCopyProgressDlg = 0L;
    m_bGUI = false;
  }
}


// open progress dialog if it exists, otherwise create it
void KIOJob::showGUI() {
  if (! m_pCopyProgressDlg ){
    m_pCopyProgressDlg = new KIOCopyProgressDlg( this, m_bStartIconified );

    m_pCopyProgressDlg->copyingFile( m_strFrom.c_str(), m_strTo.c_str() );
    m_pCopyProgressDlg->totalSize( m_iTotalSize );
    m_pCopyProgressDlg->totalFiles( m_iTotalFiles );
    m_pCopyProgressDlg->totalDirs( m_iTotalDirs );
    m_pCopyProgressDlg->processedSize( m_iProcessedSize );
  }

  m_bGUI = true;
}


// iconify progress dialog if it exists
void KIOJob::iconifyGUI() {
  if ( !m_pCopyProgressDlg )
    return;

  KWM::setIconify( m_pCopyProgressDlg->winId(), true );
}


bool KIOJob::mount( bool _ro, const char *_fstype, const char* _dev, const char *_point )
{
  string error;
  int errid;
  if ( !createSlave( "file", errid, error ) )
  {
    slotError( errid, error.c_str() );
    return false;
  }
  
  if ( m_bGUI )
  {
    QString buffer;
    buffer.sprintf( i18n("Mounting %s ..."), _dev );
    m_pDialog = createDialog( buffer );
  }
  
  return IOJob::mount( _ro, _fstype, _dev, _point );  
}


bool KIOJob::unmount( const char *_point )
{
  string error;
  int errid;
  if ( !createSlave( "file", errid, error ) )
  {
    slotError( errid, error.c_str() );
    return false;
  }
  
  if ( m_bGUI )
  {
    QString buffer;
    buffer.sprintf( i18n("Unmounting %s ..."), _point );
    m_pDialog = createDialog( buffer );
  }
  
  return IOJob::unmount( _point );  
}


bool KIOJob::copy( const char *_source, const char *_dest )
{
  list<K2URL> lst;
  if ( !K2URL::split( _source, lst ) )
  {
    slotError( ERR_MALFORMED_URL, _source );
    return false;
  }

  string protocol = lst.back().protocol();

  string error;
  int errid = 0;
  if ( !createSlave( protocol.c_str(), errid, error ) )
  {
    slotError( errid, error.c_str() );
    return false;
  }
  
  if ( m_bGUI )
  {
    m_pCopyProgressDlg = new KIOCopyProgressDlg( this, m_bStartIconified );
  }
  
  return IOJob::copy( _source, _dest );
}


bool KIOJob::copy( QStrList& _source, const char *_dest )
{
  list<string> stlurls;
  const char *s;
  for( s = _source.first(); s != 0; s = _source.next() )
    stlurls.push_back( s );

  return copy( stlurls, _dest );
}


bool KIOJob::copy( list<string>& _source, const char *_dest )
{
  assert( !m_pSlave );

  string protocol;
  string host;
  list<string>::iterator it = _source.begin();
  for( ; it != _source.end(); ++it )
  {    
    list<K2URL> lst;
    if ( !K2URL::split( it->c_str(), lst ) )
    {
      slotError( ERR_MALFORMED_URL, it->c_str() );
      return false;
    }
    if ( protocol.empty() )
    {
      protocol = lst.back().protocol();
      host = lst.back().host();
    }
    // Still the same host and protocol ?
    else if ( protocol != lst.back().protocol() || host != lst.back().host() )
    {
      // URGENTLY TODO: extract these sources and start a second copy command with them
      assert( 0 );
    }
  }
  
  string error;
  int errid = 0;
  if ( !createSlave( protocol.c_str(), errid, error ) )
  {
    slotError( errid, error.c_str() );
    return false;
  }
  
  if ( m_bGUI )
  {
    m_pCopyProgressDlg = new KIOCopyProgressDlg( this, m_bStartIconified );
  }
  
  return IOJob::copy( _source, _dest );
}


bool KIOJob::testDir( const char *_url )
{
  assert( !m_pSlave );

  list<K2URL> lst;
  if ( !K2URL::split( _url, lst ) )
  {
    slotError( ERR_MALFORMED_URL, _url );
    return false;
  }

  string error;
  int errid;
  if ( !createSlave( lst.back().protocol(), errid, error ) )
  {
    slotError( errid, error.c_str() );
    return false;
  }

  if ( m_bGUI )
  {
    m_pCopyProgressDlg = new KIOCopyProgressDlg( this, m_bStartIconified );
  }

  return IOJob::testDir( _url );
}


bool KIOJob::get( const char *_url )
{
  assert( !m_pSlave );

  list<K2URL> lst;
  if ( !K2URL::split( _url, lst ) )
  {
    slotError( ERR_MALFORMED_URL, _url );
    return false;
  }

  string error;
  int errid;
  if ( !createSlave( lst.back().protocol(), errid, error ) )
  {
    slotError( errid, error.c_str() );
    return false;
  }

  if ( m_bGUI )
  {
    m_pCopyProgressDlg = new KIOCopyProgressDlg( this, m_bStartIconified );
  }

  return IOJob::get( _url );
}


bool KIOJob::preget( const char *_url, int _max_size )
{
  m_bPreGet = true;
  m_iPreGetBufferMaxSize = _max_size;
  
  return get( _url );
}


bool KIOJob::getSize( const char *_url )
{
  assert( !m_pSlave );

  list<K2URL> lst;
  if ( !K2URL::split( _url, lst ) )
  {
    slotError( ERR_MALFORMED_URL, _url );
    return false;
  }

  string error;
  int errid;
  if ( !createSlave( lst.back().protocol(), errid, error ) )
  {
    slotError( errid, error.c_str() );
    return false;
  }

  return IOJob::getSize( _url );
}


void KIOJob::cont()
{
  if ( !m_strPreGetMimeType.empty() )
    emit sigMimeType( m_id, m_strPreGetMimeType.c_str() );
  if ( m_pPreGetBuffer )
    emit sigData( m_id, m_pPreGetBuffer, m_iPreGetBufferSize );

  if ( m_pNotifier )
    m_pNotifier->setEnabled( true );

  if ( m_bPreGetFinished )
    slotFinished();  
}


bool KIOJob::listDir( const char *_url )
{
  assert( !m_pSlave );

  list<K2URL> lst;
  if ( !K2URL::split( _url, lst ) )
  {
    slotError( ERR_MALFORMED_URL, _url );
    return false;
  }

  string error;
  int errid;
  if ( !createSlave( lst.back().protocol(), errid, error ) )
  {
    slotError( errid, error.c_str() );
    return false;
  }

  return IOJob::listDir( _url );
}


void KIOJob::slotIsDirectory()
{
  emit sigIsDirectory( m_id );
}


void KIOJob::slotIsFile()
{
  emit sigIsFile( m_id );
}


void KIOJob::slotData( void *_p, int _len )
{
  if ( m_bPreGet )
  {
    int len = _len + m_iPreGetBufferSize;
    char* p = new char[ len ];
    if ( m_pPreGetBuffer )
    {	
      memcpy( p, m_pPreGetBuffer, m_iPreGetBufferSize );
      delete [] m_pPreGetBuffer;
    }
    
    memcpy( p + m_iPreGetBufferSize, (char*)_p, _len );
    m_pPreGetBuffer = p;
    m_iPreGetBufferSize += _len;
    if ( m_iPreGetBufferSize >= m_iPreGetBufferMaxSize )
    {
      if ( m_pNotifier )
	m_pNotifier->setEnabled( false );
      emit sigPreData( m_id, m_pPreGetBuffer, m_iPreGetBufferSize );
      m_bPreGet = false;
    }
    
    return;
  }
  
  emit sigData( m_id, (const char*)_p, _len );
}


void KIOJob::slotListEntry( UDSEntry& _entry )
{
  emit sigListEntry( m_id, _entry );
}


void KIOJob::slotFinished()
{
  if ( m_bPreGet )
  {
    m_bPreGet = false;
    if ( m_pPreGetBuffer )
      emit sigPreData( m_id, m_pPreGetBuffer, m_iPreGetBufferSize );
    else
      emit sigPreData( m_id, 0L, 0L );
    m_bPreGetFinished = true;
    return;
  }

  // If someone tries to delete us because we emitted sigFinished
  // he wont have look. One only stores the id of the job. And since
  // we remove the id from the map NOW, nobody gets the pointer to this
  // object => nobody can delete it. We delete this object at the end
  // of this function anyway.
  assert( s_mapJobs );
  s_mapJobs->erase( m_id );

  emit sigFinished( m_id );
  m_id = 0;
  
  // Put the slave back to the pool
  if ( m_pSlave )
  {  
    // Delete the notifier NOW. One never know what happens ....
    if ( m_pNotifier )
    {
      m_pNotifier->setEnabled( false );
      delete m_pNotifier;
      m_pNotifier = 0L;
    }

    if ( m_bCacheToPool )
      KIOSlavePool::self()->addSlave( m_pSlave, m_strSlaveProtocol.c_str() );
    else
      delete m_pSlave;

    m_pSlave = 0L;
  }
  
  clean();
  
  if ( m_bAutoDelete )
  {
    delete this;
    return;
  }
}


void KIOJob::slotError( int _errid, const char *_txt )
{
  cerr << "KIOJob::slotError " << _errid << "   " << _txt << endl;
  IOJob::slotError( _errid, _txt );
  
  // If someone tries to delete us because we emitted sigError
  // he wont have look. One only stores the id of the job. And since
  // we remove the id from the map NOW, nobody gets the pointer to this
  // object => nobody can delete it. We delete this object at the end
  // of this function anyway.
  assert( s_mapJobs );
  s_mapJobs->erase( m_id );

  emit sigError( m_id, _errid, _txt );
  m_id = 0;

  // NOTE: This may be dangerous. I really hope that the
  // slaves are still in a good shape after reporting an error.
  // Put the slave back to the pool
  if ( m_pSlave )
  {  
    // Delete the notifier NOW. One never know what happens ....
    if ( m_pNotifier )
    {
      m_pNotifier->setEnabled( false );
      delete m_pNotifier;
      m_pNotifier = 0L;
    }

    if ( m_bCacheToPool )
      KIOSlavePool::self()->addSlave( m_pSlave, m_strSlaveProtocol.c_str() );
    else
      delete m_pSlave;

    m_pSlave = 0L;
  }

  clean();

  if ( m_bAutoDelete )
  {
    delete this;
    return;
  }
}


void KIOJob::slotRenamed( const char *_new )
{
  emit sigRenamed( m_id, _new );
}


void KIOJob::slotCanResume( bool _resume )
{
  emit sigCanResume( m_id, _resume );
}


void KIOJob::slotTotalSize( unsigned long _bytes )
{
  m_iTotalSize = _bytes;
  if ( ( m_cmd == CMD_COPY || m_cmd == CMD_COPY_SINGLE || m_cmd == CMD_GET )
       && m_pCopyProgressDlg )
    m_pCopyProgressDlg->totalSize( _bytes );
  
  emit sigTotalSize( m_id, _bytes );
  cerr << "TotalSize " << _bytes << endl;
}


void KIOJob::slotTotalFiles( unsigned long _files )
{
  m_iTotalFiles = _files;
  if ( m_cmd == CMD_COPY && m_pCopyProgressDlg )
    m_pCopyProgressDlg->totalFiles( _files );

  emit sigTotalFiles( m_id, _files );
  cerr << "TotalFiles " << _files << endl;
}


void KIOJob::slotTotalDirs( unsigned long _dirs )
{
  m_iTotalDirs = _dirs;
  if ( m_cmd == CMD_COPY && m_pCopyProgressDlg )
    m_pCopyProgressDlg->totalDirs( _dirs );

  emit sigTotalDirs( m_id, _dirs );
  cerr << "TotalDirs " << _dirs << endl;
}


void KIOJob::slotProcessedSize( unsigned long _bytes )
{
  m_iProcessedSize = _bytes;
  if ( ( m_cmd == CMD_COPY || m_cmd == CMD_COPY_SINGLE || m_cmd == CMD_GET )
       && m_pCopyProgressDlg )
    m_pCopyProgressDlg->processedSize( _bytes );

  emit sigProcessedSize( m_id, _bytes );
  cerr << "ProcessedSize " << _bytes << endl;
}


void KIOJob::slotProcessedFiles( unsigned long _files )
{
  if ( m_cmd == CMD_COPY && m_pCopyProgressDlg )
    m_pCopyProgressDlg->processedFiles( _files );

  emit sigProcessedFiles( m_id, _files );
  cerr << "ProcessedFiles " << _files << endl;
}


void KIOJob::slotProcessedDirs( unsigned long _dirs )
{
  if ( m_cmd == CMD_COPY && m_pCopyProgressDlg )
    m_pCopyProgressDlg->processedDirs( _dirs );

  emit sigProcessedDirs( m_id, _dirs );
  cerr << "ProcessedDirs " << _dirs << endl;
}


void KIOJob::slotScanningDir( const char *_dir )
{
  if ( m_cmd == CMD_COPY && m_pCopyProgressDlg )
    m_pCopyProgressDlg->scanningDir( _dir );

  cerr << "ScanningDir " << _dir << endl;
}


void KIOJob::slotSpeed( unsigned long _bytes_per_second )
{
  if ( ( m_cmd == CMD_COPY || m_cmd == CMD_COPY_SINGLE || m_cmd == CMD_GET )
       && m_pCopyProgressDlg )
    m_pCopyProgressDlg->speed( _bytes_per_second );

  emit sigSpeed( m_id, _bytes_per_second );
  cerr << "Speed " << _bytes_per_second << endl;
}


void KIOJob::slotCopyingFile( const char *_from, const char *_to )
{
  m_strFrom = _from;
  m_strTo = _to;

  if ( ( m_cmd == CMD_COPY || m_cmd == CMD_COPY_SINGLE ) && m_pCopyProgressDlg )
    m_pCopyProgressDlg->copyingFile( _from, _to );

  emit sigCopying( m_id, _from, _to );
  cerr << "CopyingFile " << _from << " -> " << _to << endl;
}


void KIOJob::slotMakingDir( const char *_dir )
{
  if ( m_cmd == CMD_COPY && m_pCopyProgressDlg )
    m_pCopyProgressDlg->makingDir( _dir );

  cerr << "MakingDir " << _dir << endl;
}


void KIOJob::slotGettingFile( const char *_url )
{
  if ( m_cmd == CMD_GET && m_pCopyProgressDlg )
    m_pCopyProgressDlg->gettingFile( _url );

  cerr << "GettingFile " << _url << endl;
}


void KIOJob::slotMimeType( const char *_type )
{
  if ( m_bPreGet )
  {    
    m_strPreGetMimeType = _type;
    m_bPreGet = false;
    if ( m_pNotifier )
      m_pNotifier->setEnabled( false );
  }
  
  emit sigMimeType( m_id, _type );
  cerr << "MimeType " << _type << endl;
}


void KIOJob::slotRedirection( const char *_url )
{
  emit sigRedirection( m_id, _url );
}


void KIOJob::slotCancel()
{
  emit sigCanceled( m_id );

  clean();
  
  if ( m_bAutoDelete )
    delete this;
}


void KIOJob::connectSlave( Slave *_s )
{
  setConnection( _s );
  m_pNotifier = new QSocketNotifier( _s->inFD(), QSocketNotifier::Read, this );
  connect( m_pNotifier, SIGNAL( activated( int ) ), this, SLOT( slotDispatch( int ) ) );
}


Slave* KIOJob::createSlave( const char *_protocol, int& _error, string& _error_text )
{
  Slave *s = KIOSlavePool::self()->slave( _protocol );
  if ( s )
  {
    m_pSlave = s;
    m_strSlaveProtocol = _protocol;
    connectSlave( s );
    return s;
  }
  
  string exec = ProtocolManager::self()->find( _protocol );
  cerr << "TRYING TO START" << exec << endl;
  
  if ( exec.empty() )
  {
    _error = ERR_UNSUPPORTED_PROTOCOL;
    _error_text = _protocol;
    return 0L;
  }
  
  s = new Slave( exec.c_str() );
  if ( s->pid() == -1 )
  {
    _error = ERR_CANNOT_LAUNCH_PROCESS;
    _error_text = exec;
    return 0L;
  }

  m_pSlave = s;
  m_strSlaveProtocol = _protocol;
  connectSlave( s );
  return s;
}


void KIOJob::slotDispatch( int )
{
  if ( !dispatch() )
  {    
    if ( m_pNotifier )
    {
      m_pNotifier->setEnabled( false );
      delete m_pNotifier;
      m_pNotifier = 0L;

      // Remove the dead slave now, to avoid that it is
      // putback in the pool.
      if ( m_pSlave )
      {    
	delete m_pSlave;
	m_pSlave = 0L;
      }
      
      slotError( ERR_SLAVE_DIED, m_strSlaveProtocol.c_str() );
      slotFinished();
    }
  }
}


QDialog* KIOJob::createDialog( const char *_text )
{
  QDialog* dlg = new QDialog;
  QVBoxLayout* layout = new QVBoxLayout( dlg, 10, 0 );
  layout->addStrut( 360 );	// makes dlg at least that wide

  QLabel *line1 = new QLabel( _text, dlg );
  line1->setFixedHeight( 20 );
  layout->addWidget( line1 );

  QPushButton *pb = new QPushButton( i18n("Cancel"), dlg );
  pb->setFixedSize( pb->sizeHint() );
  connect( pb, SIGNAL( clicked() ), this, SLOT( slotCancel() ) );
  layout->addSpacing( 10 );
  layout->addWidget( pb );

  layout->addStretch( 10 );
  layout->activate();
  dlg->resize( dlg->sizeHint() );

  dlg->show();

  return dlg;
}


/***************************************************************
 *
 * KIOSlavePool
 *
 ***************************************************************/

KIOSlavePool* KIOSlavePool::s_pSelf = 0L;

Slave* KIOSlavePool::slave( const char *_protocol )
{
  multimap<string,Entry>::iterator it = m_mapSlaves.find( _protocol );
  if ( it == m_mapSlaves.end() )
    return 0L;

  Slave* s = it->second.m_pSlave;
  m_mapSlaves.erase( it );

  return s;
}

void KIOSlavePool::addSlave( Slave *_slave, const char *_protocol )
{
  if ( m_mapSlaves.size() == 6 )
    eraseOldest();
  
  Entry e;
  e.m_time = time( 0L );
  e.m_pSlave = _slave;
  m_mapSlaves.insert( multimap<string,Entry>::value_type( _protocol, e ) );
}

void KIOSlavePool::eraseOldest()
{
  assert( m_mapSlaves.size() >= 1 );
  
  multimap<string,Entry>::iterator oldie = m_mapSlaves.begin();

  multimap<string,Entry>::iterator it = oldie;
  it++;
  for( ; it != m_mapSlaves.end(); it++ )
    if ( oldie->second.m_time > it->second.m_time )
      oldie = it;
  
  m_mapSlaves.erase( oldie );
}
    
KIOSlavePool* KIOSlavePool::self()
{
  if ( !s_pSelf )
    s_pSelf = new KIOSlavePool;
  return s_pSelf;
}

/***************************************************************
 *
 * Utility functions
 *
 ***************************************************************/

QString KIOJob::findDeviceMountPoint( const char *_device, const char *_file )
{
#ifdef __FreeBSD__
  if( !strcmp( "/etc/mtab", _file ) )
  {
    struct statfs *buf;
    long fsno;
    int flags = MNT_WAIT;
	
    fsno = getfsstat( NULL, 0, flags );
    buf = (struct statfs *)malloc(fsno * sizeof( struct statfs ) );
    if( getfsstat(buf, fsno*sizeof( struct statfs ), flags) == -1 )
    {
      free(buf);
      return QString(NULL);
    }
    else
    {
      int i;
      for( i = 0; i < fsno; i++ )
	if( !strcmp(buf[i].f_mntfromname, _device ) )
	{
	  QString tmpstr((const char *)buf[i].f_mntonname);
	  free(buf);
	  return tmpstr;
	}
    }
  }
    
#endif /* __FreeBSD__ */             

  // Get the real device name, not some link.
  char buffer[1024];
  QString tmp;
    
  struct stat lbuff;
  lstat( _device, &lbuff );

  // Perhaps '_device' is just a link ?
  const char *device2 = _device;
    
  if ( S_ISLNK( lbuff.st_mode ) )
  {
    int n = readlink( _device, buffer, 1022 );
    if ( n > 0 )
    {
      buffer[ n ] = 0;
      if ( buffer[0] == '/' )
	device2 = buffer;
      else
      {
	tmp = "/dev/";
	tmp += buffer;
	device2 = tmp.data();
      }
    }
  }
    
  int len = strlen( _device );
  int len2 = strlen( device2 );
      
  FILE *f;
  f = fopen( _file, "rb" );
  if ( f != 0L )
  {
    char buff[ 1024 ];
    
    while ( !feof( f ) )
    {
      buff[ 0 ] = 0;
      // Read a line
      fgets( buff, 1023, f );
      // Is it the device we are searching for ?
      if ( strncmp( buff, _device, len ) == 0 && ( buff[len] == ' ' || buff[len] == '\t' ) )
      {
	// Skip all spaces
	while( buff[ len ] == ' ' || buff[ len ] == '\t' )
	  len++;
		    
	char *p = strchr( buff + len, ' ' );
	if ( p != 0L )
	{
	  *p = 0;
	  fclose( f );
	  return QString( buff + len );
	}
      }
      else if ( strncmp( buff, device2, len2 ) == 0 && ( buff[len2] == ' ' || buff[len2] == '\t' ) )
      {
	// Skip all spaces
	while( buff[ len2 ] == ' ' || buff[ len2 ] == '\t' )
	  len2++;
	
	char *p = strchr( buff + len2, ' ' );
	if ( p != 0L )
	{
	  *p = 0;
	  fclose( f );
	  return QString( buff + len2 );
	}	      
      }
    }
    
    fclose( f );
  }
  
  return QString();
}
  
#include "kio_job.moc"
