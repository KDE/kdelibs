// $Id$

#include <kwm.h>

#include "job.h"
#include "progressbase.h"

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
  connect( m_pJob, SIGNAL( sigTotalFiles( int, unsigned int ) ),
	   SLOT( slotTotalFiles( int, unsigned int ) ) );
  connect( m_pJob, SIGNAL( sigTotalDirs( int, unsigned int ) ),
	   SLOT( slotTotalDirs( int, unsigned int ) ) );
  connect( m_pJob, SIGNAL( sigProcessedSize( int, unsigned long ) ),
	   SLOT( slotProcessedSize( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigPercent( int, unsigned long ) ),
	   SLOT( slotPercent( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigProcessedFiles( int, unsigned int ) ),
	   SLOT( slotProcessedFiles( int, unsigned int ) ) );
  connect( m_pJob, SIGNAL( sigProcessedDirs( int, unsigned int ) ),
	   SLOT( slotProcessedDirs( int, unsigned int ) ) );
  connect( m_pJob, SIGNAL( sigCopying( int, const KURL& , const KURL& ) ),
	   SLOT( slotCopyingFile( int, const KURL&, const KURL& ) ) );
  connect( m_pJob, SIGNAL( sigScanningDir( int, const KURL& ) ),
	   SLOT( slotScanningDir( int, const KURL& ) ) );
  connect( m_pJob, SIGNAL( sigMakingDir( int, const KURL& ) ),
	   SLOT( slotMakingDir( int, const KURL& ) ) );
  connect( m_pJob, SIGNAL( sigGettingFile( int, const KURL& ) ),
	   SLOT( slotGettingFile( int, const KURL& ) ) );
  connect( m_pJob, SIGNAL( sigDeletingFile( int, const KURL& ) ),
	   SLOT( slotDeletingFile( int, const KURL& ) ) );
  connect( m_pJob, SIGNAL( sigCanResume( int, bool ) ),
 	   SLOT( slotCanResume( int, bool ) ) );
}


void KIOProgressBase::refill() {
    // TODO
#if 0
  // refill the values with values from m_pJob
  switch ( m_pJob->cmd() ) {
  case KIOProtocol::KIO::Command::CMD_COPY:
  case KIOProtocol::CMD_MCOPY:
    slotCopyingFile( m_pJob->m_id, m_pJob->m_strFrom, m_pJob->m_strTo );
    slotCanResume( m_pJob->m_id, m_pJob->m_bCanResume );
    break;

  case KIOProtocol::CMD_DEL:
    slotDeletingFile( m_pJob->m_id, m_pJob->m_strFrom );
    break;

  case KIOProtocol::CMD_MKDIR:
    slotMakingDir( m_pJob->m_id, m_pJob->m_strTo );
    break;

  case KIOProtocol::CMD_GET:
    slotGettingFile( m_pJob->m_id, m_pJob->m_strFrom );
    break;
  }

  slotTotalSize( m_pJob->m_id, m_pJob->m_iTotalSize );
  slotTotalFiles( m_pJob->m_id, m_pJob->m_iTotalFiles );
  slotTotalDirs( m_pJob->m_id, m_pJob->m_iTotalDirs );

  slotPercent( m_pJob->m_id, m_pJob->m_iPercent );

  if ( m_pJob->m_iTotalDirs > 1 ) {
    slotProcessedDirs( m_pJob->m_id, m_pJob->m_iProcessedDirs );
  }

  if ( m_pJob->m_iTotalFiles > 1 ) {
    slotProcessedFiles( m_pJob->m_id, m_pJob->m_iProcessedFiles );
  }

  slotSpeed( m_pJob->m_id, m_pJob->m_iSpeed );
#endif
}


void KIOProgressBase::setJob( KIOJob *job ) {
  if ( m_pJob ) {
    disconnect( m_pJob ); // completely forget about that job
  }

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

#include "progressbase.moc"

