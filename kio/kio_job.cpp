// $Id$

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <assert.h>
#include <dirent.h>
#include <fstab.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_SYS_MNTTAB_H
#include <sys/mnttab.h>
#endif
#ifdef HAVE_MNTENT_H
#include <mntent.h>
#endif

#include <qsocketnotifier.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtimer.h>

#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <kwm.h>
#include <kdebug.h>
#include <kprotocolmanager.h>
#include <kdialog.h>

#include "kio_job.h"
#include "kio_simpleprogress_dlg.h"
#include "kio_listprogress_dlg.h"

/**
 * Maximum number of slaves kept around in KIOSlavePool.
 */
#define MAX_SLAVES           6

int KIOJob::s_id = 0;
KIOJob::jobDict *KIOJob::s_allJobs = 0L;
KIOListProgressDlg* KIOJob::m_pListProgressDlg = 0L;

KIOJob::KIOJob(const char *name)
  : QObject(0, name), KIOJobBase( 0L ) {

  m_id = ++s_id;

  if (!s_allJobs) {
    initStatic();
  }

  s_allJobs->insert( m_id, this);

  m_bAutoDelete = true;
  m_iGUImode = SIMPLE; // default is a simple progress dialog
  m_bCacheToPool = true;

  m_iTotalSize = 0;
  m_iTotalFiles = 0;
  m_iTotalDirs = 0;
  m_iProcessedSize = 0;
  m_iProcessedFiles = 0;
  m_iProcessedDirs = 0;
  m_iSpeed = 0;
  m_bStalled = false;
  m_iPercent = 0;

  m_pProgressDlg = 0L;

  m_pDialog = 0L;
  m_pSlave = 0L;
  m_bPreGet = false;
  m_iPreGetBufferSize = 0;
  m_iPreGetBufferMaxSize = 0;
  m_pPreGetBuffer = 0L;
  m_bPreGetFinished = false;
}


KIOJob::~KIOJob() {
  if ( m_pPreGetBuffer ) {
    delete [] m_pPreGetBuffer;
  }

  if ( m_pDialog ) {
    delete m_pDialog;
  }

  clean();
}


void KIOJob::initStatic() {
  if ( !s_allJobs ) {
    s_allJobs = new jobDict;
  }

  if ( !m_pListProgressDlg ) {
    m_pListProgressDlg = new KIOListProgressDlg;
  }
}


KIOJob *KIOJob::find( int _id ) {
  return s_allJobs->find( _id );
}


void KIOJob::kill( bool quiet ) {
  if ( !quiet ) {
    emit sigCanceled( m_id );
  }

  clean();

  // Time to die ...
  delete this;
}


void KIOJob::clean() {
  assert( s_allJobs );
  if ( m_id ) {
    assert( s_allJobs->find( m_id ) != 0 );
    s_allJobs->remove( m_id );
    m_id = 0;
  }

  if ( m_pProgressDlg ) {
    if ( m_pProgressDlg->onlyClean() ) {
      m_pProgressDlg->clean();
    } else {
      delete m_pProgressDlg;
      m_pProgressDlg = 0L;
    }
  }

  // Do not putback the slave into the pool because we may have
  // died in action. This means that the slave is in an undefined
  // state. If the job has finished successfully then
  // 'slotFinished' already handed the slave back to the pool.
  if ( m_pSlave ) {
    delete m_pSlave;
    m_pSlave = 0L;
  }
}


void KIOJob::createGUI() {
  switch ( m_iGUImode ) {
  case LIST:
    if ( !m_pListProgressDlg )
      return;
    m_pListProgressDlg->addJob( this );
    m_pListProgressDlg->show();
    break;

  case SIMPLE:
  case CUSTOM:
    if ( !m_pProgressDlg )
      setGUImode( (KIOJob::GUImode)m_iGUImode );
    m_pProgressDlg->setJob( this );
    // instead of showing immediately, we fire off a one shot timer to
    // show ourselves after 1.5 seconds.  This avoids massive window creation/
    // destruction on single file copies or other short operations.
    QTimer::singleShot(1500, m_pProgressDlg, SLOT(show()));

    break;
  }
}


void KIOJob::setProgressDlg( KIOProgressBase *dlg ) {
  m_pProgressDlg = dlg;
  m_pProgressDlg->setJob( this );
}


void KIOJob::setGUImode( GUImode _mode ) {
  if ( m_iGUImode == _mode && m_pProgressDlg ) {
    return;
  }

  m_iGUImode = _mode;
  if ( m_pProgressDlg ) {
    delete m_pProgressDlg;
  }

  if ( m_iGUImode == SIMPLE ) {  // create a default dialog
    m_pProgressDlg = new KIOSimpleProgressDlg();
  }
}


bool KIOJob::mount( bool _ro, const char *_fstype, const char* _dev, const char *_point ) {
  QString error;
  int errid;
  if ( !createSlave( "file", errid, error ) ) {
    slotError( errid, error.ascii() );
    return false;
  }

  if ( m_iGUImode != NONE )
    m_pDialog = createDialog( i18n("Mounting %1...").arg(_dev));

  return KIOJobBase::mount( _ro, _fstype, _dev, _point );
}


bool KIOJob::unmount( const char *_point ) {
  QString error;
  int errid;
  if ( !createSlave( "file", errid, error ) ) {
    slotError( errid, error.data() );
    return false;
  }

  if ( m_iGUImode != NONE )
    m_pDialog = createDialog( i18n("Unmounting %1...").arg(_point));

  return KIOJobBase::unmount( _point );
}


bool KIOJob::copy( const char *_source, const char *_dest, bool _move ) {
  KURL u( _source );
  if ( u.isMalformed() ) {
    slotError( ERR_MALFORMED_URL, _source );
    return false;
  }

  QString error;
  int errid = 0;
  if ( !createSlave( u.protocol().ascii(), u.host().ascii(),
		     u.user().ascii(), u.pass().ascii(), errid, error ) ) {
    slotError( errid, error.data() );
    return false;
  }

  createGUI();

  if ( _move ) {
    return KIOJobBase::move( _source, _dest );
  } else {
    return KIOJobBase::copy( _source, _dest );
  }
}


bool KIOJob::copy( QStringList& _source, const char *_dest, bool _move ) {
  assert( !m_pSlave );

  QString protocol, host, user, pass;
  QStringList::Iterator it = _source.begin();
  for( ; it != _source.end(); ++it ) {
    KURL u( (*it) );
    if ( u.isMalformed() ) {
      slotError( ERR_MALFORMED_URL, (*it).ascii() );
      return false;
    }

    if ( protocol.isEmpty() ) {
      protocol = u.protocol();
      host = u.host();
      user = u.user();
      pass = u.pass();
    } else if ( protocol != u.protocol() || host != u.host() ||
		user != u.host() || pass != u.pass() ) {     // Still the same host and protocol ?
      // URGENTLY TODO: extract these sources and start a second copy command with them
      ASSERT( 0 );
    }
  }

  QString error;
  int errid = 0;
  if ( !createSlave( protocol.ascii(), host.ascii(),
		     user.ascii(), pass.ascii(), errid, error ) ) {
    slotError( errid, error.ascii() );
    return false;
  }

  createGUI();

  if ( _move ) {
    return KIOJobBase::move( _source, _dest );
  } else {
    return KIOJobBase::copy( _source, _dest );
  }
}


bool KIOJob::move( const char *_source, const char *_dest ) {
  return copy( _source, _dest, true );
}


bool KIOJob::move( QStringList& _source, const char *_dest ) {
  return copy( _source, _dest, true );
}


bool KIOJob::del( const char *_source ) {
  KURL u( _source );
  if ( u.isMalformed() ) {
    slotError( ERR_MALFORMED_URL, _source );
    return false;
  }

  QString error;
  int errid = 0;
  if ( !createSlave( u.protocol().ascii(), u.host().ascii(),
		     u.user().ascii(), u.pass().ascii(), errid, error ) ) {
    slotError( errid, error.ascii() );
    return false;
  }

  createGUI();

  return KIOJobBase::del( _source );
}


bool KIOJob::del( QStringList& _source ) {
  assert( !m_pSlave );

  QString protocol;
  QString host;
  QString user;
  QString pass;
  QStringList::Iterator it = _source.begin();
  for( ; it != _source.end(); ++it ) {
    KURL u( (*it) );
    if ( u.isMalformed() ) {
      slotError( ERR_MALFORMED_URL, (*it).ascii() );
      return false;
    }

    if ( protocol.isEmpty() ) {
      protocol = u.protocol();
      host = u.host();
      user = u.user();
      pass = u.pass();
    }
    // Still the same host and protocol ?
    else if ( protocol != u.protocol() || host != u.host() ||
	      user != u.user() || pass != u.pass() ) {
      // URGENTLY TODO: extract these sources and start a second copy command with them
      ASSERT( 0 );
    }
  }

  QString error;
  int errid = 0;
  if ( !createSlave( protocol.ascii(), host.ascii(),
		     user.ascii(), pass.ascii(), errid, error ) ) {
    slotError( errid, error.ascii() );
    return false;
  }

  createGUI();

  return KIOJobBase::del( _source );
}


bool KIOJob::testDir( const char *_url ) {
  ASSERT( !m_pSlave );

  KURL u( _url );
  if ( u.isMalformed() ) {
    slotError( ERR_MALFORMED_URL, _url );
    return false;
  }

  QString error;
  int errid;
  if ( !createSlave( u.protocol().ascii(), u.host().ascii(),
		     u.user().ascii(), u.pass().ascii(), errid, error ) ) {
    slotError( errid, error.ascii() );
    return false;
  }

  createGUI();

  return KIOJobBase::testDir( _url );
}


bool KIOJob::get( const char *_url ) {
  ASSERT( !m_pSlave );

  KURL u( _url );
  if ( u.isMalformed() ) {
    slotError( ERR_MALFORMED_URL, _url );
    return false;
  }

  QString error;
  int errid;
  if ( !createSlave( u.protocol().ascii(), u.host().ascii(),
		     u.user().ascii(), u.pass().ascii(), errid, error ) ) {
    slotError( errid, error.ascii() );
    return false;
  }

  createGUI();

  return KIOJobBase::get( _url );
}


bool KIOJob::preget( const char *_url, int _max_size ) {
  m_bPreGet = true;
  m_iPreGetBufferMaxSize = _max_size;

  return get( _url );
}


bool KIOJob::getSize( const char *_url ) {
  assert( !m_pSlave );

  KURL u( _url );
  if ( u.isMalformed() ) {
    slotError( ERR_MALFORMED_URL, _url );
    return false;
  }

  QString error;
  int errid;
  if ( !createSlave( u.protocol().ascii(), u.host().ascii(),
		     u.user().ascii(), u.pass().ascii(), errid, error ) ) {
    slotError( errid, error.ascii() );
    return false;
  }

  return KIOJobBase::getSize( _url );
}


bool KIOJob::put( const char *_url, int _mode, bool _overwrite, bool _resume,
		  int _len ) {
  ASSERT( !m_pSlave );

  KURL u( _url );
  if ( u.isMalformed() ) {
    slotError( ERR_MALFORMED_URL, _url );
    return false;
  }

  QString error;
  int errid;
  if ( !createSlave( u.protocol().ascii(), u.host().ascii(),
		     u.user().ascii(), u.pass().ascii(), errid, error ) ) {
    slotError( errid, error.ascii() );
    return false;
  }

  createGUI();

  return KIOJobBase::put( _url, _mode, _overwrite, _resume, _len);
}


bool KIOJob::mkdir( const char *_url, int _mode ) {
  assert( !m_pSlave );

  KURL u( _url );

  QString error;
  int errid = 0;
  if ( !createSlave( u.protocol().ascii(), u.host().ascii(),
		     u.user().ascii(), u.pass().ascii(), errid, error ) ) {
    slotError( errid, error.ascii() );
    return false;
  }

  createGUI();

  return KIOJobBase::mkdir( _url, _mode );
}


void KIOJob::cont() {
  if ( !m_strPreGetMimeType.isEmpty() )
    emit sigMimeType( m_id, m_strPreGetMimeType.data() );
  if ( m_pPreGetBuffer )
    emit sigData( m_id, m_pPreGetBuffer, m_iPreGetBufferSize );

  m_pSlave->resume();

  if ( m_bPreGetFinished )
    slotFinished();
}


bool KIOJob::listDir( const char *_url ) {
  ASSERT( !m_pSlave );

  KURL u( _url );
  if ( u.isMalformed() ) {
    slotError( ERR_MALFORMED_URL, _url );
    return false;
  }

  QString error;
  int errid;
  if ( !createSlave( u.protocol().ascii(), u.host().ascii(),
		     u.user().ascii(), u.pass().ascii(), errid, error ) ) {
    slotError( errid, error.ascii() );
    return false;
  }

  return KIOJobBase::listDir( _url );
}


void KIOJob::slotIsDirectory() {
  emit sigIsDirectory( m_id );
}


void KIOJob::slotIsFile() {
  emit sigIsFile( m_id );
}


void KIOJob::slotData( void *_p, int _len ) {
  if ( m_bPreGet ) {
    int len = _len + m_iPreGetBufferSize;
    char* p = new char[ len ];
    if ( m_pPreGetBuffer ) {	
      memcpy( p, m_pPreGetBuffer, m_iPreGetBufferSize );
      delete [] m_pPreGetBuffer;
    }

    memcpy( p + m_iPreGetBufferSize, (char*)_p, _len );
    m_pPreGetBuffer = p;
    m_iPreGetBufferSize += _len;
    if ( m_iPreGetBufferSize >= m_iPreGetBufferMaxSize ) {
      m_pSlave->suspend();
      emit sigPreData( m_id, m_pPreGetBuffer, m_iPreGetBufferSize );
      m_bPreGet = false;
    }

    return;
  }

  emit sigData( m_id, (const char*)_p, _len );
}


void KIOJob::slotListEntry( const KUDSEntry& _entry ) {
  emit sigListEntry( m_id, _entry );
}


void KIOJob::slotFinished() {
  if ( m_bPreGet ) {
    m_bPreGet = false;
    if ( m_pPreGetBuffer ) {
      emit sigPreData( m_id, m_pPreGetBuffer, m_iPreGetBufferSize );
    } else {
      emit sigPreData( m_id, 0L, 0L );
    }
    m_bPreGetFinished = true;
    return;
  }

  // If someone tries to delete us because we emitted sigFinished
  // he wont have look. One only stores the id of the job. And since
  // we remove the id from the map NOW, nobody gets the pointer to this
  // object => nobody can delete it. We delete this object at the end
  // of this function anyway.
  assert( s_allJobs );
  s_allJobs->remove( m_id );

  // Put the slave back to the pool
  if ( m_pSlave ) {
    disconnectSlave( m_pSlave );

    if ( m_bCacheToPool ) {
      KIOSlavePool::self()->addSlave( m_pSlave, m_strSlaveProtocol.data(),
				      m_strSlaveHost.data(),
				      m_strSlaveUser.data(),
				      m_strSlavePass.data() );
    } else {
      delete m_pSlave;
    }

    m_pSlave = 0L;
  }

  emit sigFinished( m_id );
  m_id = 0;

  clean();

  if ( m_bAutoDelete ) {
    delete this;
    return;
  }
}


void KIOJob::slotError( int _errid, const char *_txt ) {
  if ( _errid == ERR_WARNING ) {
    //this is very tricky, because we rely on the slots connected to sigError
    //to check that this is only a warning on continue to proceed normally.
    //otherwise we might run into trouble...
    emit sigError( m_id, _errid, _txt );
    return;
  }

  KIOJobBase::slotError( _errid, _txt );

  // If someone tries to delete us because we emitted sigError
  // he wont have look. One only stores the id of the job. And since
  // we remove the id from the map NOW, nobody gets the pointer to this
  // object => nobody can delete it. We delete this object at the end
  // of this function anyway.
  assert( s_allJobs );
  s_allJobs->remove( m_id );

  emit sigError( m_id, _errid, _txt );
  m_id = 0;

  // NOTE: This may be dangerous. I really hope that the
  // slaves are still in a good shape after reporting an error.
  // Put the slave back to the pool
  if ( m_pSlave ) {
    disconnectSlave( m_pSlave );

    if ( m_bCacheToPool ) {
      KIOSlavePool::self()->addSlave( m_pSlave, m_strSlaveProtocol.data(),
				      m_strSlaveHost.data(),
				      m_strSlaveUser.data(),
				      m_strSlavePass.data() );
    } else {
      delete m_pSlave;
    }

    m_pSlave = 0L;
  }

  clean();

  if ( m_bAutoDelete ) {
    delete this;
    return;
  }
}


void KIOJob::slotReady() {
  m_bIsReady = true;

  emit sigReady( m_id );
}


void KIOJob::slotRenamed( const char *_new ) {
  m_strTo = _new;

  emit sigRenamed( m_id, _new );
}


void KIOJob::slotCanResume( bool _resume ) {
  m_bCanResume = _resume;

  emit sigCanResume( m_id, _resume );
}


void KIOJob::slotTotalSize( unsigned long _bytes ) {
  m_iTotalSize = _bytes;

  emit sigTotalSize( m_id, _bytes );
  kdebug( KDEBUG_INFO, 7007, "TotalSize %ld", _bytes );
}


void KIOJob::slotTotalFiles( unsigned long _files ) {
  m_iTotalFiles = _files;

  emit sigTotalFiles( m_id, _files );
  kdebug( KDEBUG_INFO, 7007, "TotalFiles %ld", _files );
}


void KIOJob::slotTotalDirs( unsigned long _dirs ) {
  m_iTotalDirs = _dirs;

  emit sigTotalDirs( m_id, _dirs );
  kdebug( KDEBUG_INFO, 7007, "TotalDirs %ld", _dirs );
}


void KIOJob::slotProcessedSize( unsigned long _bytes ) {
  uint old = m_iPercent;

  m_iProcessedSize = _bytes;

  if ( m_iTotalSize != 0 ) {
    m_iPercent = (int)(( (float)m_iProcessedSize / (float)m_iTotalSize ) * 100.0);
    if ( m_iPercent != old ) {
      emit sigPercent( m_id, m_iPercent );
    }
  }

  emit sigProcessedSize( m_id, _bytes );
}


void KIOJob::slotProcessedFiles( unsigned long _files ) {
  m_iProcessedFiles = _files;

  emit sigProcessedFiles( m_id, _files );
  kdebug( KDEBUG_INFO, 7007, "ProcessedFiles %ld", _files );
}


void KIOJob::slotProcessedDirs( unsigned long _dirs ) {
  m_iProcessedDirs = _dirs;

  emit sigProcessedDirs( m_id, _dirs );
  kdebug( KDEBUG_INFO, 7007, "ProcessedDirs %ld", _dirs );
}


void KIOJob::slotSpeed( unsigned long _bytes_per_second ) {
  m_iSpeed = _bytes_per_second;

  if ( _bytes_per_second == 0 ) {
    m_bStalled = true;
  } else {
    m_bStalled = false;

    if (m_iTotalSize >= m_iProcessedSize)
    {
       int secs = ( m_iTotalSize - m_iProcessedSize ) / _bytes_per_second;

       m_RemainingTime = QTime().addSecs(secs);
    }
  }

  emit sigSpeed( m_id, _bytes_per_second );
}


void KIOJob::slotScanningDir( const char *_dir ) {
  m_strFrom = _dir;

  emit sigScanningDir( m_id, _dir );
  kdebug( KDEBUG_INFO, 7007, "ScanningDir %s", _dir );
}


void KIOJob::slotMakingDir( const char *_dir ) {
  m_strTo = _dir;

  emit sigMakingDir( m_id, _dir );
  kdebug( KDEBUG_INFO, 7007, "MakingDir %s", _dir );
}


void KIOJob::slotCopyingFile( const char *_from, const char *_to ) {
  m_strFrom = _from;
  m_strTo = _to;

  emit sigCopying( m_id, _from, _to );
  kdebug( KDEBUG_INFO, 7007, "CopyingFile %s -> %s", _from,  _to );
}


void KIOJob::slotGettingFile( const char *_url ) {
  m_strFrom = _url;

  emit sigGettingFile( m_id, _url );
  kdebug( KDEBUG_INFO, 7007, "GettingFile %s", _url );
}


void KIOJob::slotDeletingFile( const char *_url ) {
  m_strFrom = _url;

  emit sigDeletingFile( m_id, _url );
  kdebug( KDEBUG_INFO, 7007, "DeletingFile %s", _url );
}


void KIOJob::slotMimeType( const char *_type ) {
  if ( m_bPreGet ) {
    m_strPreGetMimeType = _type;
    m_bPreGet = false;
    m_pSlave->suspend();
  }

  emit sigMimeType( m_id, _type );
  kdebug( KDEBUG_INFO, 7007, "MimeType %s", _type );
}


void KIOJob::slotRedirection( const char *_url ) {
  emit sigRedirection( m_id, _url );
}


void KIOJob::slotCancel() {
  emit sigCanceled( m_id );

  clean();

  if ( m_bAutoDelete ) {
    delete this;
  }
}

void KIOJob::slotSlaveDied( KProcess *)
{
  assert( m_pSlave);
  kdebug( KDEBUG_INFO, 7007, "Slave died, pid = %ld", m_pSlave->getPid() );
}


void KIOJob::connectSlave( KIOSlave *_s ) {
  setConnection( _s );
  connect(_s, SIGNAL( receivedStdout( int, int &)), this, SLOT( slotDispatch( int, int & )) );
  connect(_s, SIGNAL( processExited(KProcess *)), this, SLOT( slotSlaveDied(KProcess *)));
  _s->resume();
}

void KIOJob::disconnectSlave( KIOSlave *_s ) {
  _s->suspend();
  disconnect(_s, SIGNAL( receivedStdout( int, int &)), this, SLOT( slotDispatch( int, int & )) );
  disconnect(_s, SIGNAL( processExited(KProcess *)), this, SLOT( slotSlaveDied(KProcess *)));
}


KIOSlave* KIOJob::createSlave( const char *_protocol, int& _error, QString& _error_text ) {
  KIOSlave *s = KIOSlavePool::self()->slave( _protocol );
  if ( s ) {
    m_pSlave = s;
    m_strSlaveProtocol = _protocol;
    connectSlave( s );
    return s;
  }

  QString exec = KProtocolManager::self().executable( _protocol );
  kdebug( KDEBUG_INFO, 7007, "TRYING TO START %s", exec.data() );

  if ( exec.isEmpty() ) {
    _error = ERR_UNSUPPORTED_PROTOCOL;
    _error_text = _protocol;
    return 0L;
  }

  s = new KIOSlave( exec.data() );
  if ( !s->isRunning() ) {
    _error = ERR_CANNOT_LAUNCH_PROCESS;
    _error_text = exec;
    return 0L;
  }

  m_pSlave = s;
  m_strSlaveProtocol = _protocol;
  connectSlave( s );
  return s;
}


KIOSlave* KIOJob::createSlave( const char *_protocol, const char *_host,
			       const char *_user, const char *_pass,
			       int& _error, QString& _error_text ) {

  // no host, nor user, nor pass : wrong method
  if (!_host && !_user && !_pass) {
    return createSlave( _protocol, _error, _error_text );
  }

  KIOSlave *s = KIOSlavePool::self()->slave( _protocol, _host, _user, _pass );
  debug("KIOJob::createSlave : Slave got");
  if ( s ) {
    m_pSlave = s;
    m_strSlaveProtocol = _protocol;
    m_strSlaveHost = _host;
    m_strSlaveUser = _user;
    m_strSlavePass = _pass;
    debug(" m_strSlavePass ok ");
    connectSlave( s );
    return s;
  }

  QString exec = KProtocolManager::self().executable( _protocol );
  kdebug( KDEBUG_INFO, 7007, "TRYING TO START %s", exec.data() );

  if ( exec.isEmpty() ) {
    _error = ERR_UNSUPPORTED_PROTOCOL;
    _error_text = _protocol;
    return 0L;
  }

  s = new KIOSlave( exec.data() );
  if ( !s->isRunning() ) {
    _error = ERR_CANNOT_LAUNCH_PROCESS;
    _error_text = exec;
    return 0L;
  }

  debug(" trying m_strSlave...Pass");
  m_pSlave = s;
  m_strSlaveProtocol = _protocol;
  m_strSlaveHost = _host;
  m_strSlaveUser = _user;
  m_strSlavePass = _pass;

  debug(" m_strSlavePass ok (1)");
  connectSlave( s );
  return s;
}


void KIOJob::slotDispatch( int, int &result ) {
  result = 1;
  if ( !dispatch() ) {
    result = -1;

    // Get rid of the slave. It's no good any longer.
    m_pSlave->closeStdout();
    delete m_pSlave;
    m_pSlave = 0;
    bool deleteFlag = m_bAutoDelete;
    m_bAutoDelete = false;
    slotError( ERR_SLAVE_DIED, m_strSlaveProtocol.data() );
    m_bAutoDelete = deleteFlag;
    slotFinished();
  }
}


QDialog* KIOJob::createDialog( const QString &_text ) {
  QDialog* dlg = new QDialog;
  QVBoxLayout* layout = new QVBoxLayout( dlg, KDialog::marginHint(),
					 KDialog::spacingHint() );
  layout->addStrut( 360 );	// makes dlg at least that wide

  QLabel *line1 = new QLabel( _text, dlg );
  layout->addWidget( line1 );

  layout->addSpacing( KDialog::spacingHint() );

  QHBoxLayout *hBox = new QHBoxLayout();
  layout->addLayout(hBox);

  hBox->addStretch(1);

  QPushButton *pb = new QPushButton( i18n("Cancel"), dlg );
  connect( pb, SIGNAL( clicked() ), this, SLOT( slotCancel() ) );
  hBox->addWidget( pb );

  layout->addStretch( 10 );

  dlg->resize( dlg->sizeHint() );

  // instead of showing immediately, we fire off a one shot timer to
  // show ourselves after 1.5 seconds.  This avoids massive window creation/
  // destruction on single file copies or other short operations.

  QTimer::singleShot(1500, dlg, SLOT(show()));

  return dlg;
}


QString KIOJob::convertSize( unsigned long size )
{
    float fsize;
    QString s;
    // Giga-byte
    if ( size >= 1073741824 )
    {
        fsize = (float) size / (float) 1073741824;
        s = i18n( "%1 GB" ).arg( KGlobal::locale()->formatNumber(fsize, 0));
    }
    // Mega-byte
    else if ( size >= 1048576 )
    {
        fsize = (float) size / (float) 1048576;
        s = i18n( "%1 MB" ).arg( KGlobal::locale()->formatNumber(fsize, 0));
    }
    // Kilo-byte
    else if ( size > 1024 )
    {
        fsize = (float) size / (float) 1024;
        s = i18n( "%1 KB" ).arg( KGlobal::locale()->formatNumber(fsize, 0));
    }
    // Just a byte
    else
    {
        s = i18n( "%1 B" ).arg( KGlobal::locale()->formatNumber(size, 0));
    }
    return s;
}


/***************************************************************
 *
 * KIOSlavePool
 *
 ***************************************************************/

KIOSlavePool* KIOSlavePool::s_pSelf = 0L;


KIOSlave* KIOSlavePool::slave( const char *_protocol)
{
  Entry *entry = m_allSlaves.first();
  for(; entry; entry = m_allSlaves.next())
  {
     if (entry->m_protocol == _protocol)
     {
        kdebug( KDEBUG_INFO, 7007, "Found matching slave - protocol (%s)", _protocol );
        break;
     }
  }

  if (!entry)
  {
     kdebug( KDEBUG_INFO, 7007, "No matching slave, no matching protocol - protocol (%s)", _protocol );
     return 0L;
  }

  m_allSlaves.removeRef( entry );

  assert(entry != 0);

  KIOSlave *s = entry->m_pSlave;
  delete entry;

  disconnect( s, SIGNAL( processExited(KProcess *)), this, SLOT( slotSlaveDied(KProcess *)));

  return s;
}

KIOSlave* KIOSlavePool::slave( const char *_protocol, const char *_host,
			       const char *_user, const char *_pass)
{
  Entry *entry = m_allSlaves.first();
  Entry *protEntry = 0;
  for(; entry; entry = m_allSlaves.next())
  {
     if (entry->m_protocol == _protocol)
     {
        protEntry = entry;
        if((entry->m_host == _host) &&
           (entry->m_user == _user) &&
           (entry->m_pass == _pass) )
        {
           kdebug( KDEBUG_INFO, 7007, "Found matching slave, total match - protocol (%s)", _protocol );
           break;
        }
     }
  }

  if (!entry)
  {
     if (!protEntry)
     {
        kdebug( KDEBUG_INFO, 7007, "No matching slave, no matching protocol - protocol (%s)", _protocol );
        return 0L;
     }
     kdebug( KDEBUG_INFO, 7007, "Found matching slave, protocol only - protocol (%s)", _protocol );
     entry = protEntry;
  }

  m_allSlaves.removeRef( entry );

  assert(entry != 0);

  KIOSlave *s = entry->m_pSlave;
  delete entry;

  disconnect( s, SIGNAL( processExited(KProcess *)), this, SLOT( slotSlaveDied(KProcess *)));

  return s;
}


void KIOSlavePool::addSlave( KIOSlave *_slave, const char *_protocol, const char *_host,
			     const char *_user, const char *_pass )
{
  Entry *entry = new Entry();
  entry->m_time = time( 0L );
  entry->m_pSlave = _slave;
  entry->m_protocol = _protocol;
  entry->m_host = _host;
  entry->m_user = _user;
  entry->m_pass = _pass;

  if (m_allSlaves.count() >= MAX_SLAVES )
  {
     Entry *entry = m_allSlaves.first();
     Entry *oldest = entry;
     for(entry = m_allSlaves.next(); entry; entry = m_allSlaves.next())
     {
        if (oldest->m_time > entry->m_time)
           oldest = entry;
     }
     m_allSlaves.removeRef(oldest);
     delete oldest->m_pSlave;
     delete oldest;
     kdebug( KDEBUG_INFO, 7007, "oldest slave removed - protocol = %s", _protocol );
  }
  m_allSlaves.append(entry);
  connect( _slave, SIGNAL( processExited(KProcess *)), this, SLOT( slotSlaveDied(KProcess *)));
}


void KIOSlavePool::slotSlaveDied(KProcess *proc) {
   kdebug( KDEBUG_INFO, 7007, "Slave died from KIOSlavePool" );

   Entry *entry = m_allSlaves.first();
   for(; entry; entry = m_allSlaves.next())
   {
      assert(entry->m_pSlave);

      if (entry->m_pSlave->getPid() == proc->getPid())
         break;
   }
   if (!entry)
   {
       kdebug( KDEBUG_ERROR, 7007, "Unknown slave died from KIOSlavePool!" );
       return;
   }
   m_allSlaves.removeRef(entry);
   kdebug( KDEBUG_INFO, 7007, "Slave died from KIOSlavePool - protocol = %s host = %s",
		entry->m_protocol.ascii(), entry->m_host.ascii() );
   delete entry->m_pSlave;
   delete entry;
}


KIOSlavePool* KIOSlavePool::self() {
  if ( !s_pSelf ) {
    s_pSelf = new KIOSlavePool;
  }

  return s_pSelf;
}


/***************************************************************
 *
 * Utility functions
 *
 ***************************************************************/

#ifdef _PATH_MOUNTED
// On some Linux, MNTTAB points to /etc/fstab !
# undef MNTTAB
# define MNTTAB _PATH_MOUNTED
#else
# ifndef MNTTAB
#  ifdef MTAB_FILE
#   define MNTTAB MTAB_FILE
#  else
#   define MNTTAB "/etc/mnttab"
#  endif
# endif
#endif

// hopefully there are only two kind of APIs. If not we need a configure test
#ifdef HAVE_SETMNTENT
#define SETMNTENT setmntent
#define ENDMNTENT endmntent
#define STRUCT_MNTENT struct mntent *
#define STRUCT_SETMNTENT FILE *
#define GETMNTENT(file, var) ((var = getmntent(file)) != NULL)
#define MOUNTPOINT(var) var->mnt_dir
#define MOUNTTYPE(var) var->mnt_type
#define FSNAME(var) var->mnt_fsname
#elif BSD
#define SETMNTENT(x, y) setfsent()
#define ENDMNTENT(x) /* nope */
#define STRUCT_MNTENT struct fstab *
#define STRUCT_SETMNTENT int
#define GETMNTENT(file, var) ((var=getfsent()) != NULL)
#define MOUNTPOINT(var) var->fs_file
#define MOUNTTYPE(var) var->fs_vftype
#define FSNAME(var) var->fs_spec
#else /* no setmntent and no BSD */
#define SETMNTENT fopen
#define ENDMNTENT fclose
#define STRUCT_MNTENT struct mnttab
#define STRUCT_SETMNTENT FILE *
#define GETMNTENT(file, var) (getmntent(file, &var) == 0)
#define MOUNTPOINT(var) var.mnt_mountp
#define MOUNTTYPE(var) var.mnt_fstype
#define FSNAME(var) var.mnt_special
#endif

QString KIOJob::findDeviceMountPoint( const char *filename )
{
    STRUCT_SETMNTENT	mtab;
    char    		realname[MAXPATHLEN];

    /* If the path contains symlinks, get the real name */
    if (realpath(filename, realname) == 0) {
	if (strlen(filename) >= sizeof(realname))
	    return QString::null;
	strcpy(realname, filename);
    }

    /* Get the list of mounted file systems */

    if ((mtab = SETMNTENT(MNTTAB, "r")) == 0) {
	perror("setmntent");
	return QString::null;
    }

    /* Loop over all file systems and see if we can find our
     * mount point.
     * Note that this is the mount point with the longest match.
     * XXX: Fails if me->mnt_dir is not a realpath but goes
     * through a symlink, e.g. /foo/bar where /foo is a symlink
     * pointing to /local/foo.
     *
     * How kinky can you get with a filesystem?
     */

    int max = 0;
    STRUCT_MNTENT me;

    QString result;

    while (true) {
      if (!GETMNTENT(mtab, me))
	break;

      int  length = strlen(FSNAME(me));

      if (!strncmp(FSNAME(me), realname, length)
	  && length > max) {
	max = length;
	if (length == 1 || realname[length] == '/' || realname[length] == '\0') {
	
	  result = MOUNTPOINT(me);
	}
      }
    }

    ENDMNTENT(mtab);
    return result;
}

#include "kio_job.moc"
