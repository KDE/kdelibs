// $Id$

#include <kwin.h>
#include <kwm.h>

#include "jobclasses.h"
#include "progressbase.h"

KIOProgressBase::KIOProgressBase( QWidget *parent )
  : QWidget( parent ) {
  m_bOnlyClean = false;
  m_bStopOnClose = true;
}


void KIOProgressBase::Connect() {
  connect( m_pJob, SIGNAL( speed( KIO::Job*, unsigned long ) ),
	   SLOT( slotSpeed( KIO::Job*, unsigned long ) ) );

  connect( m_pJob, SIGNAL( totalSize( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalSize( KIO::Job*, unsigned long ) ) );
  connect( m_pJob, SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );
  connect( m_pJob, SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );

  connect( m_pJob, SIGNAL( processedSize( KIO::Job*, unsigned long ) ),
	   SLOT( slotProcessedSize( KIO::Job*, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigPercent( KIO::Job*, unsigned long ) ),
	   SLOT( slotPercent( KIO::Job*, unsigned long ) ) );

  connect( m_pJob, SIGNAL( processedFiles( KIO::Job*, unsigned long ) ),
	   SLOT( slotProcessedFiles( KIO::Job*, unsigned long ) ) );
  connect( m_pJob, SIGNAL( processedDirs( KIO::Job*, unsigned long ) ),
	   SLOT( slotProcessedDirs( KIO::Job*, unsigned long ) ) );

  connect( m_pJob, SIGNAL( copying( KIO::Job*, const KURL& , const KURL& ) ),
	   SLOT( slotCopyingFile( KIO::Job*, const KURL&, const KURL& ) ) );
  connect( m_pJob, SIGNAL( moving( KIO::Job*, const KURL& , const KURL& ) ),
	   SLOT( slotMovingFile( KIO::Job*, const KURL&, const KURL& ) ) );
  connect( m_pJob, SIGNAL( deletingFile( KIO::Job*, const KURL& ) ),
	   SLOT( slotDeletingFile( KIO::Job*, const KURL& ) ) );
  connect( m_pJob, SIGNAL( creatingDir( KIO::Job*, const KURL& ) ),
 	   SLOT( slotCreatingDir( KIO::Job*, const KURL& ) ) );

//   connect( m_pJob, SIGNAL( sigGettingFile( KIO::Job*, const KURL& ) ),
// 	   SLOT( slotGettingFile( KIO::Job*, const KURL& ) ) );

  connect( m_pJob, SIGNAL( canResume( KIO::Job*, bool ) ),
 	   SLOT( slotCanResume( KIO::Job*, bool ) ) );
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
    slotCreatingDir( m_pJob->m_id, m_pJob->m_strTo );
    break;

  case KIOProtocol::CMD_GET:
    slotGettingFile( m_pJob->m_id, m_pJob->m_strFrom );
    break;
  }

  slotTotalSize( m_pJob, m_pJob->m_iTotalSize );
  slotTotalFiles( m_pJob, m_pJob->m_iTotalFiles );
  slotTotalDirs( m_pJob, m_pJob->m_iTotalDirs );

  slotPercent( m_pJob, m_pJob->m_iPercent );

  if ( m_pJob->m_iTotalDirs > 1 ) {
    slotProcessedDirs( m_pJob, m_pJob->m_iProcessedDirs );
  }

  if ( m_pJob->m_iTotalFiles > 1 ) {
    slotProcessedFiles( m_pJob, m_pJob->m_iProcessedFiles );
  }

  slotSpeed( m_pJob->m_id, m_pJob->m_iSpeed );
#endif
}


void KIOProgressBase::setJob( KIO::Job *job ) {
  if ( m_pJob ) {
    disconnect( m_pJob ); // completely forget about that job
  }

  m_pJob = job;
  Connect();
}

/*
void KIOProgressBase::iconify( bool mode ) {
  KWM::setIconify( this->winId(), mode );
}
*/

void KIOProgressBase::closeEvent( QCloseEvent* ) {
  if ( m_bStopOnClose ) {
    stop();
  } else { // we have to do the cleaning ourselves
    if ( m_bOnlyClean ) {
      clean();
    } else {
      delete this;
      // code doesn't seem relevant anymore, correct if wrong - PGB
      //if ( m_pJob ) {
      //    m_pJob->m_pProgressDlg = 0L;
      //}
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

