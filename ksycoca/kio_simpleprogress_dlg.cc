#include "kio_job.h"
#include <qpushbutton.h>

#include <kapp.h>
#include <klocale.h>
#include <kwm.h>

#include "kio_simpleprogress_dlg.h"


KIOSimpleProgressDlg::KIOSimpleProgressDlg( KIOJob* _job, bool m_bStartIconified ) : QDialog( 0L )
{
  m_pJob = _job;
  m_iPercent = 0;

  m_pProgressBar = new KProgress( 0, 100, 0, KProgress::Horizontal, this );
  m_pLine1 = new QLabel( this );
  m_pLine2 = new QLabel( this );
  m_pLine3 = new QLabel( this );
  m_pLine4 = new QLabel( this );
  m_pLine5 = new QLabel( this );

  m_pLayout = new QVBoxLayout( this, 10, 0 );
  m_pLayout->addStrut( 360 );	// makes dlg at least that wide
  if ( m_pLine1 != 0L )
  {
    m_pLine1->setFixedHeight( 20 );
    m_pLayout->addWidget( m_pLine1 );
  }

  if ( m_pLine2 != 0L )
  {
    m_pLine2->setFixedHeight( 20 );
    m_pLayout->addWidget( m_pLine2 );
  }

  if ( m_pLine3 != 0L )
  {
    m_pLine3->setFixedHeight( 20 );
    m_pLayout->addWidget( m_pLine3 );
  }

  if ( m_pProgressBar != 0L )
  {
    m_pProgressBar->setFixedHeight( 20 );
    m_pLayout->addSpacing( 10 );
    m_pLayout->addWidget( m_pProgressBar );
  }

  if ( m_pLine4 != 0L )
  {
    m_pLine4->setFixedHeight( 20 );
    m_pLayout->addWidget( m_pLine4 );
  }

  if ( m_pLine5 != 0L )
  {
    m_pLine5->setFixedHeight( 20 );
    m_pLayout->addWidget( m_pLine5 );
  }

  QPushButton *pb = new QPushButton( i18n("Cancel"), this );
  pb->setFixedSize( pb->sizeHint() );
  connect( pb, SIGNAL( clicked() ), m_pJob, SLOT( slotCancel() ) );
  m_pLayout->addSpacing( 10 );
  m_pLayout->addWidget( pb );

  m_pLayout->addStretch( 10 );
  m_pLayout->activate();
  resize( sizeHint() );

  this->show();

  if ( m_bStartIconified )
    KWM::setIconify( this->winId(), true );
}


void KIOSimpleProgressDlg::done( int ) 
{
  if ( m_pJob )
    m_pJob->m_pSimpleProgressDlg = 0L;

  hide();
}


void KIOSimpleProgressDlg::processedSize()
{
  if ( m_pJob->m_iProcessedSize == 0 || m_pJob->m_iTotalSize == 0 )
    return;
  
  int old = m_iPercent;

  m_iPercent = (int)(( (float)m_pJob->m_iProcessedSize / (float)m_pJob->m_iTotalSize ) * 100.0);

  if ( m_iPercent == old )
    return;

  QString tmp = i18n( "%1 % of %2 ").arg( m_iPercent ).arg( KIOJob::convertSize(m_pJob->m_iTotalSize));

  m_pLine4->setText( tmp );
  m_pProgressBar->setValue( m_iPercent );
  setCaption( tmp );
}


void KIOSimpleProgressDlg::processedDirs()
{
  m_pLine1->setText( i18n("%1/%2 directories created").arg((int)m_pJob->m_iProcessedDirs).arg((int)m_pJob->m_iTotalDirs ) );
}


void KIOSimpleProgressDlg::processedFiles()
{
  m_pLine1->setText( i18n("%1/%2 files").arg( (int)m_pJob->m_iProcessedFiles ).arg( (int)m_pJob->m_iTotalFiles ) );
}


void KIOSimpleProgressDlg::speed()
{
  if ( m_pJob->m_iProcessedSize == 0 )
    return;
  
  if ( m_pJob->m_iSpeed == 0 )
    m_pLine5->setText( i18n( "Stalled") );
  else
    m_pLine5->setText( i18n( "%1/s %2").arg( KIOJob::convertSize( m_pJob->m_iSpeed )).arg( m_pJob->m_RemainingTime.toString()) );
  
}


void KIOSimpleProgressDlg::scanningDir()
{
  m_pLine2->setText( i18n("Scanning %1").arg( m_pJob->m_strFrom ) );
}


void KIOSimpleProgressDlg::copyingFile()
{
  m_pLine1->setText( i18n("Copying") );
  m_pLine2->setText( i18n("From : %1").arg( m_pJob->m_strFrom ) );
  m_pLine3->setText( i18n("To : %1").arg( m_pJob->m_strTo ) );
}


void KIOSimpleProgressDlg::makingDir()
{
  m_pLine2->setText( i18n("Creating dir %1").arg( m_pJob->m_strTo ) );
}


void KIOSimpleProgressDlg::gettingFile()
{
  m_pLine1->setText( i18n("Fetching file") );
  m_pLine2->setText( m_pJob->m_strFrom );
}


void KIOSimpleProgressDlg::deletingFile()
{
  m_pLine1->setText( i18n("Deleting file") );
  m_pLine2->setText( m_pJob->m_strTo );
}


#include "kio_simpleprogress_dlg.moc"
