// $Id$

#include <kwm.h>

#include "kio_job.h"
#include "kio_progressbase.h"

KIOProgressBase::KIOProgressBase( QWidget *parent )
  : QWidget( parent ) {
  m_bOnlyClean = false;
  m_bStopOnClose = true;
}


void KIOProgressBase::Connect() {
  connect( m_pJob, SIGNAL( sigSpeed( int, unsigned long ) ),
	   SLOT( slotSpeed( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigTotalSize( int, unsigned long ) ),
	   SLOT( slotTotalSize( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigTotalFiles( int, unsigned long ) ),
	   SLOT( slotTotalFiles( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigTotalDirs( int, unsigned long ) ),
	   SLOT( slotTotalDirs( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigProcessedSize( int, unsigned long ) ),
	   SLOT( slotProcessedSize( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigPercent( int, unsigned long ) ),
	   SLOT( slotPercent( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigProcessedFiles( int, unsigned long ) ),
	   SLOT( slotProcessedFiles( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigProcessedDirs( int, unsigned long ) ),
	   SLOT( slotProcessedDirs( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigCopying( int, const char*, const char* ) ),
	   SLOT( slotCopyingFile( int, const char*, const char* ) ) );
  connect( m_pJob, SIGNAL( sigScanningDir( int, const char* ) ),
	   SLOT( slotScanningDir( int, const char* ) ) );
  connect( m_pJob, SIGNAL( sigMakingDir( int, const char* ) ),
	   SLOT( slotMakingDir( int, const char* ) ) );
  connect( m_pJob, SIGNAL( sigGettingFile( int, const char* ) ),
	   SLOT( slotGettingFile( int, const char* ) ) );
  connect( m_pJob, SIGNAL( sigDeletingFile( int, const char* ) ),
	   SLOT( slotDeletingFile( int, const char* ) ) );
  connect( m_pJob, SIGNAL( sigCanResume( int, bool ) ),
 	   SLOT( slotCanResume( int, bool ) ) );
}


void KIOProgressBase::refill() {
  // refill the values with values from m_pJob
  switch ( m_pJob->m_cmd ) {
  case KIO::CMD_COPY:
  case KIO::CMD_MCOPY:
    slotCopyingFile( m_pJob->m_id, m_pJob->m_strFrom.ascii(), m_pJob->m_strTo.ascii() );
    slotCanResume( m_pJob->m_id, m_pJob->m_bCanResume );
    break;
    
  case KIO::CMD_DEL:
  case KIO::CMD_MDEL:
    slotDeletingFile( m_pJob->m_id, m_pJob->m_strFrom.ascii() );
    break;
    
  case KIO::CMD_MKDIR:
    slotMakingDir( m_pJob->m_id, m_pJob->m_strTo.ascii() );
    break;
    
  case KIO::CMD_GET:
    slotGettingFile( m_pJob->m_id, m_pJob->m_strFrom.ascii() );
    break;
  }
  
  slotTotalSize( m_pJob->m_id, m_pJob->m_iTotalSize );
  slotTotalFiles( m_pJob->m_id, m_pJob->m_iTotalFiles );
  slotTotalDirs( m_pJob->m_id, m_pJob->m_iTotalDirs );
  
  slotPercent( m_pJob->m_id, m_pJob->m_iPercent );
  slotProcessedDirs( m_pJob->m_id, m_pJob->m_iProcessedDirs );
  slotProcessedFiles( m_pJob->m_id, m_pJob->m_iProcessedFiles );
  
  slotSpeed( m_pJob->m_id, m_pJob->m_iSpeed );
}


void KIOProgressBase::setJob( KIOJob *job ) {
  m_pJob = job;
  Connect();
}


void KIOProgressBase::iconify( bool mode ) {
  KWM::setIconify( this->winId(), mode );
}


void KIOProgressBase::closeEvent( QCloseEvent* ) {
  if ( m_bStopOnClose ) {
    stop();
  } else { // we have to do the cleaning ourselves
    if ( m_bOnlyClean ) {
      clean();
    } else {
      delete this;
      if ( m_pJob ) {
	m_pJob->m_pProgressDlg = 0L;
      }
    }
  }
}


// this will kill job and subsequently also delete or clean this dialog
void KIOProgressBase::stop() {
  if ( m_pJob ) {
    m_pJob->kill();
  }
}
