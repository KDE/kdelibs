// $Id$

#include "kio_job.h"
#include <qpushbutton.h>
#include <qtimer.h>
#include <qlayout.h>

#include <kapp.h>
#include <kdialog.h>
#include <klocale.h>
#include <kwm.h>

#include "kio_simpleprogress_dlg.h"


KIOSimpleProgressDlg::KIOSimpleProgressDlg()
  : KIOProgressBase() {

  QVBoxLayout *topLayout = new QVBoxLayout( this, KDialog::marginHint(),
					    KDialog::spacingHint() );
  topLayout->addStrut( 360 );	// makes dlg at least that wide

  QGridLayout *grid = new QGridLayout(3, 3);
  topLayout->addLayout(grid);
  grid->setColStretch(2, 1);
  grid->addColSpacing(1, KDialog::spacingHint());
  // filenames or action name
  grid->addWidget(new QLabel(i18n("Source:"), this), 0, 0);
  
  sourceLabel = new QLabel(this);
  grid->addWidget(sourceLabel, 0, 2);

  grid->addWidget(new QLabel(i18n("Destination:"), this), 1, 0);

  destLabel = new QLabel(this);
  grid->addWidget(destLabel, 1, 2);
  
  topLayout->addSpacing( 10 );

  progressLabel = new QLabel(this);
  grid->addWidget(progressLabel, 2, 2);

  m_pProgressBar = new KProgress(0, 100, 0, KProgress::Horizontal, this);
  topLayout->addWidget( m_pProgressBar );

  // processed info
  QHBoxLayout *hBox = new QHBoxLayout();
  topLayout->addLayout(hBox);

  speedLabel = new QLabel(this);
  hBox->addWidget(speedLabel, 1);

  sizeLabel = new QLabel(this);
  hBox->addWidget(sizeLabel);
  
  resumeLabel = new QLabel(this);
  hBox->addWidget(resumeLabel);

  hBox = new QHBoxLayout();
  topLayout->addLayout(hBox);
  
  hBox->addStretch(1);

  QPushButton *pb = new QPushButton( i18n("Cancel"), this );
  connect( pb, SIGNAL( clicked() ), SLOT( stop() ) );
  hBox->addWidget( pb );

  resize( sizeHint() );
}


void KIOSimpleProgressDlg::slotTotalSize( int, unsigned long _bytes ) 
{
  m_iTotalSize = _bytes;
}


void KIOSimpleProgressDlg::slotTotalFiles( int, unsigned long _files ) 
{
  m_iTotalFiles = _files;
}


void KIOSimpleProgressDlg::slotTotalDirs( int, unsigned long _dirs ) 
{
  m_iTotalDirs = _dirs;
}


void KIOSimpleProgressDlg::slotPercent( int, unsigned long _percent ) 
{
  QString tmp(i18n( "%1% of %2 ").arg( _percent ).arg( KIOJob::convertSize(m_iTotalSize)));
  m_pProgressBar->setValue( _percent );
  switch(mode) {
  case Copy:
    tmp.append(i18n(" (Copying)"));
    break;
  case Delete:
    tmp.append(i18n(" (Deleting)"));
    break;
  case Create:
    tmp.append(i18n(" (Creating)"));
    break;
  case Scan:
    tmp.append(i18n(" (Scanning)"));
    break;
  case Fetch:
    tmp.append(i18n(" (Fetching)"));
    break;
  }

  setCaption( tmp );
}


void KIOSimpleProgressDlg::slotProcessedSize( int, unsigned long _bytes ) {
  QString tmp;

  tmp = i18n( "%1 of %2 ").arg( KIOJob::convertSize(_bytes) ).arg( KIOJob::convertSize(m_iTotalSize));
  sizeLabel->setText( tmp );
}


void KIOSimpleProgressDlg::slotProcessedDirs( int, unsigned long _dirs ) 
{
  m_iProcessedDirs = _dirs;

  QString tmps;
  tmps = i18n("%1 / %2 directories  ").arg( m_iProcessedDirs ).arg( m_iTotalDirs );
  tmps += i18n("%1 / %2 files").arg( m_iProcessedFiles ).arg( m_iTotalFiles );
  progressLabel->setText( tmps );
}


void KIOSimpleProgressDlg::slotProcessedFiles( int, unsigned long _files ) 
{
  m_iProcessedFiles = _files;

  QString tmps;
  if ( m_iTotalDirs > 1 ) {
    tmps = i18n("%1 / %2 directories  ").arg( m_iProcessedDirs ).arg( m_iTotalDirs );
  }
  tmps += i18n("%1 / %2 files").arg( m_iProcessedFiles ).arg( m_iTotalFiles );
  progressLabel->setText( tmps );
}


void KIOSimpleProgressDlg::slotSpeed( int, unsigned long _bytes_per_second ) 
{
  if ( _bytes_per_second == 0 ) {
    speedLabel->setText( i18n( "Stalled") );
  } else {
    speedLabel->setText( i18n( "%1/s %2").arg( KIOJob::convertSize( _bytes_per_second )).arg( m_pJob->getRemainingTime().toString()) );
  }
}


void KIOSimpleProgressDlg::slotScanningDir( int , const char *_dir) 
{
  setCaption(i18n("Scanning %1").arg( _dir ) );
  mode = Scan;
}


void KIOSimpleProgressDlg::slotCopyingFile( int, const char *_from, 
					    const char *_to ) 
{
  setCaption(i18n("Copy file(s) progress"));
  mode = Copy;
  sourceLabel->setText( _from );
  destLabel->setText( _to );
}


void KIOSimpleProgressDlg::slotMakingDir( int, const char *_dir ) 
{
  setCaption(i18n("Creating directory"));
  mode = Create;
  sourceLabel->setText( _dir );
}


void KIOSimpleProgressDlg::slotGettingFile( int, const char *_url ) 
{
  setCaption(i18n("Fetch file(s) progress"));
  mode = Fetch;
  sourceLabel->setText( _url );
}


void KIOSimpleProgressDlg::slotDeletingFile( int, const char *_url ) 
{
  setCaption(i18n("Delete file(s) progress"));
  mode = Delete;
  sourceLabel->setText( _url );
}


void KIOSimpleProgressDlg::slotCanResume( int, bool _resume )
{
  if ( _resume ) {
    resumeLabel->setText( i18n("Resumable") );
  } else {
    resumeLabel->setText( i18n("Not resumable") );
  }
}


#include "kio_simpleprogress_dlg.moc"
