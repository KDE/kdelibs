// $Id$

#include "kio_job.h"
#include <qpushbutton.h>

#include <kapp.h>
#include <klocale.h>
#include <kwm.h>

#include "kio_simpleprogress_dlg.h"


KIOSimpleProgressDlg::KIOSimpleProgressDlg( KIOJob* _job, bool m_bStartIconified )
  : QDialog( 0L ) {

  m_pJob = _job;
  connect( m_pJob, SIGNAL( sigSpeed( int, unsigned long ) ),
	   SLOT( slotSpeed( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigTotalSize( int, unsigned long ) ),
	   SLOT( slotTotalSize( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigTotalFiles( int, unsigned long ) ),
	   SLOT( slotTotalFiles( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigTotalDirs( int, unsigned long ) ),
	   SLOT( slotTotalDirs( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigPercent( int, unsigned long ) ),
	   SLOT( slotPercent( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigProcessedSize( int, unsigned long ) ),
	   SLOT( slotProcessedSize( int, unsigned long ) ) );
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

  m_pProgressBar = new KProgress( 0, 100, 0, KProgress::Horizontal, this );
  m_pLine1 = new QLabel( this );
  m_pLine2 = new QLabel( this );
  m_pLine3 = new QLabel( this );
  m_pLine4 = new QLabel( this );
  m_pLine5 = new QLabel( this );
  m_pLine6 = new QLabel( this );

  m_pLayout = new QVBoxLayout( this, 10, 0 );
  m_pLayout->addStrut( 360 );	// makes dlg at least that wide
  if ( m_pLine1 != 0L ) {
    m_pLine1->setFixedHeight( 20 );
    m_pLayout->addWidget( m_pLine1 );
  }

  if ( m_pLine2 != 0L ) {
    m_pLine2->setFixedHeight( 20 );
    m_pLayout->addWidget( m_pLine2 );
  }

  if ( m_pLine3 != 0L ) {
    m_pLine3->setFixedHeight( 20 );
    m_pLayout->addWidget( m_pLine3 );
  }

  if ( m_pProgressBar != 0L ) {
    m_pProgressBar->setFixedHeight( 20 );
    m_pLayout->addSpacing( 10 );
    m_pLayout->addWidget( m_pProgressBar );
  }

  if ( m_pLine4 != 0L ) {
    m_pLine4->setFixedHeight( 20 );
    m_pLayout->addWidget( m_pLine4 );
  }

  if ( m_pLine5 != 0L ) {
    m_pLine5->setFixedHeight( 20 );
    m_pLayout->addWidget( m_pLine5 );
  }

  if ( m_pLine6 != 0L ) {
    m_pLine5->setFixedHeight( 20 );
    m_pLayout->addWidget( m_pLine6 );
  }

  QPushButton *pb = new QPushButton( i18n("Cancel"), this );
  pb->setFixedSize( pb->sizeHint() );
  connect( pb, SIGNAL( clicked() ), SLOT( done() ) );
  m_pLayout->addSpacing( 10 );
  m_pLayout->addWidget( pb );

  m_pLayout->addStretch( 10 );
  m_pLayout->activate();
  resize( sizeHint() );

  this->show();

  if ( m_bStartIconified ) {
    KWM::setIconify( this->winId(), true );
  }
}


void KIOSimpleProgressDlg::done( int ) {
  if ( m_pJob ) {
    m_pJob->kill();
  }
//   hide(); !!! test closing
}


void KIOSimpleProgressDlg::slotTotalSize( int, unsigned long _bytes ) {
  m_iTotalSize = _bytes;
}


void KIOSimpleProgressDlg::slotTotalFiles( int, unsigned long _files ) {
  m_iTotalFiles = _files;
}


void KIOSimpleProgressDlg::slotTotalDirs( int, unsigned long _dirs ) {
  m_iTotalDirs = _dirs;
}


void KIOSimpleProgressDlg::slotPercent( int, unsigned long _percent ) {
  QString tmp;

  tmp = i18n( "%1% of %2 ").arg( _percent ).arg( KIOJob::convertSize(m_iTotalSize));
  m_pProgressBar->setValue( _percent );
  setCaption( tmp );
}


void KIOSimpleProgressDlg::slotProcessedSize( int, unsigned long _bytes ) {
  QString tmp;

  tmp = i18n( "%1 from %2 ").arg( KIOJob::convertSize(_bytes) ).arg( KIOJob::convertSize(m_iTotalSize));
  m_pLine4->setText( tmp );
}


void KIOSimpleProgressDlg::slotProcessedDirs( int, unsigned long _dirs ) {
  m_pLine1->setText( i18n("%1/%2 directories created").arg( _dirs ).arg( m_iTotalDirs ) );
}


void KIOSimpleProgressDlg::slotProcessedFiles( int, unsigned long _files ) {
  m_pLine1->setText( i18n("%1/%2 files").arg( _files ).arg( m_iTotalFiles ) );
}


void KIOSimpleProgressDlg::slotSpeed( int, unsigned long _bytes_per_second ) {
  if ( _bytes_per_second == 0 ) {
    m_pLine5->setText( i18n( "Stalled") );
  } else {
    m_pLine5->setText( i18n( "%1/s %2").arg( KIOJob::convertSize( _bytes_per_second )).arg( m_pJob->getRemainingTime().toString()) );
  }
}


void KIOSimpleProgressDlg::slotScanningDir( int , const char *_dir) {
  m_pLine2->setText( i18n("Scanning %1").arg( _dir ) );
}


void KIOSimpleProgressDlg::slotCopyingFile( int, const char *_from, const char *_to ) {
  m_pLine1->setText( i18n("Copying") );
  m_pLine2->setText( i18n("From : %1").arg( _from ) );
  m_pLine3->setText( i18n("To : %1").arg( _to ) );
}


void KIOSimpleProgressDlg::slotMakingDir( int, const char *_dir ) {
  m_pLine2->setText( i18n("Creating dir %1").arg( _dir ) );
}


void KIOSimpleProgressDlg::slotGettingFile( int, const char *_url ) {
  m_pLine1->setText( i18n("Fetching file") );
  m_pLine2->setText( _url );
}


void KIOSimpleProgressDlg::slotDeletingFile( int, const char *_url ) {
  m_pLine1->setText( i18n("Deleting file") );
  m_pLine2->setText( _url );
}


void KIOSimpleProgressDlg::slotCanResume( int, bool _resume ) {
  if ( _resume ) {
    m_pLine6->setText( i18n("Resumable") );
  } else {
    m_pLine6->setText( i18n("Not resumable") );
  }
}


#include "kio_simpleprogress_dlg.moc"
