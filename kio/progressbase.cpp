// $Id$

#include "jobclasses.h"
#include "progressbase.h"

ProgressBase::ProgressBase( QWidget *parent )
  : QWidget( parent ) {
  m_bOnlyClean = false;
  m_bStopOnClose = true;
}


void ProgressBase::Connect() {
  connect( m_pJob, SIGNAL( totalSize( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalSize( KIO::Job*, unsigned long ) ) );
  connect( m_pJob, SIGNAL( totalFiles( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalFiles( KIO::Job*, unsigned long ) ) );
  connect( m_pJob, SIGNAL( totalDirs( KIO::Job*, unsigned long ) ),
	   SLOT( slotTotalDirs( KIO::Job*, unsigned long ) ) );

  connect( m_pJob, SIGNAL( processedSize( KIO::Job*, unsigned long ) ),
	   SLOT( slotProcessedSize( KIO::Job*, unsigned long ) ) );
  connect( m_pJob, SIGNAL( processedFiles( KIO::Job*, unsigned long ) ),
	   SLOT( slotProcessedFiles( KIO::Job*, unsigned long ) ) );
  connect( m_pJob, SIGNAL( processedDirs( KIO::Job*, unsigned long ) ),
	   SLOT( slotProcessedDirs( KIO::Job*, unsigned long ) ) );

  connect( m_pJob, SIGNAL( speed( KIO::Job*, unsigned long ) ),
	   SLOT( slotSpeed( KIO::Job*, unsigned long ) ) );
  connect( m_pJob, SIGNAL( percent( KIO::Job*, unsigned int ) ),
	   SLOT( slotPercent( KIO::Job*, unsigned int ) ) );

  connect( m_pJob, SIGNAL( copying( KIO::Job*, const KURL& , const KURL& ) ),
	   SLOT( slotCopyingFile( KIO::Job*, const KURL&, const KURL& ) ) );
  connect( m_pJob, SIGNAL( moving( KIO::Job*, const KURL& , const KURL& ) ),
	   SLOT( slotMovingFile( KIO::Job*, const KURL&, const KURL& ) ) );
  connect( m_pJob, SIGNAL( deletingFile( KIO::Job*, const KURL& ) ),
	   SLOT( slotDeletingFile( KIO::Job*, const KURL& ) ) );
  connect( m_pJob, SIGNAL( creatingDir( KIO::Job*, const KURL& ) ),
 	   SLOT( slotCreatingDir( KIO::Job*, const KURL& ) ) );

  connect( m_pJob, SIGNAL( canResume( KIO::Job*, bool ) ),
 	   SLOT( slotCanResume( KIO::Job*, bool ) ) );
}


void ProgressBase::setJob( KIO::Job *job ) {
  if ( m_pJob ) {
    disconnect( m_pJob ); // completely forget about that job
  }

  m_pJob = job;
  Connect();
}


void ProgressBase::closeEvent( QCloseEvent* ) {
  if ( m_bStopOnClose ) {
    stop();
  } else { // we have to do the cleaning ourselves
    if ( m_bOnlyClean ) {
      clean();
    } else {
      delete this;
    }
  }
}

// this will kill job and subsequently also delete or clean this dialog
void ProgressBase::stop() {
  if ( m_pJob ) {
    m_pJob->kill();
  }
}

#include "progressbase.moc"

