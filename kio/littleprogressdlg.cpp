
#include <kapp.h>
#include <klocale.h>
#include <kwm.h>

#include "kio_job.h"
#include "kio_littleprogress_dlg.h"

KIOLittleProgressDlg::KIOLittleProgressDlg( QWidget* parent ) : QWidget( parent )
{
  m_pJob = 0L;
  m_iPercent = 0;
  
  QFontMetrics fm = fontMetrics();
  int w = fm.width( " 999.9 kB/s 00:00:01 " ) + 8;
  int h = fm.height() + 3;

  m_pProgressBar = new KProgress( 0, 100, 0, KProgress::Horizontal, this );
  m_pProgressBar->setFrameStyle( QFrame::Box | QFrame::Raised );
  m_pProgressBar->setLineWidth( 1 );
  m_pProgressBar->setBackgroundMode( QWidget::PaletteBackground );
  m_pProgressBar->setBarColor( Qt::blue );
  m_pProgressBar->setGeometry( 0, 1, w, h - 1 );
  m_pProgressBar->installEventFilter( this );

  m_pLabel = new QLabel( "", this );
  m_pLabel->setFrameStyle( QFrame::Box | QFrame::Raised );
  m_pLabel->setGeometry( 0, 1, w, h - 1 );
  m_pLabel->installEventFilter( this );

  m_pMenu = new QPopupMenu();

  mode = false;
  setMode( true );

  resize( w, h );

  this->show();
}


void KIOLittleProgressDlg::setJob( KIOJob *job )
{
  m_pJob = job;
  m_pMenu->clear();
  m_pMenu->insertItem( i18n("Cancel"), m_pJob, SLOT(slotCancel()) );
  setMode( mode );
}

void KIOLittleProgressDlg::setMode( bool _mode )
{
  if ( _mode ) {
    m_pProgressBar->hide();
    m_pLabel->show();
  }
  else {
    m_pProgressBar->show();
    m_pLabel->hide();
  }
}


void KIOLittleProgressDlg::clean()
{
  m_pJob = 0L;
  m_pProgressBar->setValue( 0 );
  m_pLabel->clear();
  setMode( true );
}


void KIOLittleProgressDlg::processedSize()
{
  if ( m_pJob->m_iProcessedSize == 0 || m_pJob->m_iTotalSize == 0 )
    return;
  
  int old = m_iPercent;

  m_iPercent = (int)(( (float)m_pJob->m_iProcessedSize / (float)m_pJob->m_iTotalSize ) * 100.0);

  if ( m_iPercent == old )
    return;

  m_pProgressBar->setValue( m_iPercent );
}


void KIOLittleProgressDlg::speed()
{
  if ( m_pJob == 0L )
    return;

  if ( m_pJob->m_iProcessedSize == 0 )
    return;
  
  if ( m_pJob->m_iSpeed == 0 )
    m_pLabel->setText( i18n( " Stalled ") );
  else
    m_pLabel->setText( i18n( " %1/s %2 ").arg( KIOJob::convertSize( m_pJob->m_iSpeed )).arg( m_pJob->m_RemainingTime.toString()) );
}


bool KIOLittleProgressDlg::eventFilter( QObject *, QEvent *ev ) {

  if ( ev->type() == QEvent::MouseButtonPress ) {
    QMouseEvent *e = (QMouseEvent*)ev;

    if ( e->button() == LeftButton ) {    // toggle view on left mouse button

      if ( m_pJob == 0L ) // only toggle when we have something to display
	return true;

      mode = ! mode;
      setMode( mode );
      return true;
    }
    else if ( e->button() == RightButton ) {  // open popup menu on right mouse button

      if ( m_pJob == 0L ) // only popup when we have something to cancel
	return true;
      
      m_pMenu->move(-1000,-1000);
      m_pMenu->show();
      m_pMenu->hide();
      QRect g = KWM::geometry( this->winId() );
      m_pMenu->popup(QPoint( g.x(), g.y() - m_pMenu->height()));
	
      m_pMenu->exec();
      return true;
    }
  }

  return false;
}


#include "kio_littleprogress_dlg.moc"
