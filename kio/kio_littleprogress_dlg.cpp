// $Id$

#include <qtooltip.h>

#include <kapp.h>
#include <klocale.h>

#include "kio_job.h" 
#include "kio_littleprogress_dlg.h"

KIOLittleProgressDlg::KIOLittleProgressDlg( QWidget* parent ) 
  : QWidget( parent ) {

  m_pJob = 0L;
  
  QFontMetrics fm = fontMetrics();
  int w_offset = fm.width( "x" ) + 10;
  int w = fm.width( " 999.9 kB/s 00:00:01 " ) + 8;
  int h = fm.height() + 3;

  m_pButton = new QPushButton( "x", this );
  m_pButton->setGeometry( 0, 1, w_offset, h - 1);
  QToolTip::add( m_pButton, i18n("Cancel job") );
  
  m_pProgressBar = new KProgress( 0, 100, 0, KProgress::Horizontal, this );
  m_pProgressBar->setFrameStyle( QFrame::Box | QFrame::Raised );
  m_pProgressBar->setLineWidth( 1 );
  m_pProgressBar->setBackgroundMode( QWidget::PaletteBackground );
  m_pProgressBar->setBarColor( Qt::blue );
  m_pProgressBar->setGeometry( w_offset, 1, w + w_offset, h - 1 );
  m_pProgressBar->installEventFilter( this );

  m_pLabel = new QLabel( "", this );
  m_pLabel->setFrameStyle( QFrame::Box | QFrame::Raised );
  m_pLabel->setGeometry( w_offset, 1, w + w_offset, h - 1 );
  m_pLabel->installEventFilter( this );

  mode = None;
  setMode();

  resize( w + w_offset, h );

  this->show();
}


void KIOLittleProgressDlg::setJob( KIOJob *job ) {
  m_pJob = job;
  connect( m_pJob, SIGNAL( sigSpeed( int, unsigned long ) ),
	   SLOT( slotSpeed( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigTotalSize( int, unsigned long ) ),
	   SLOT( slotTotalSize( int, unsigned long ) ) );
  connect( m_pJob, SIGNAL( sigPercent( int, unsigned long ) ),
	   SLOT( slotPercent( int, unsigned long ) ) );

  connect( m_pButton, SIGNAL( clicked() ), m_pJob, SLOT(slotCancel()) );
  mode = Progress;
  setMode();
}


void KIOLittleProgressDlg::setMode() {
  switch ( mode ) {
  case None:
    m_pButton->hide();
    m_pProgressBar->hide();
    m_pLabel->hide();
    break;

  case Label:
    m_pButton->show();
    m_pProgressBar->hide();
    m_pLabel->show();
    break;

  case Progress:
    m_pButton->show();
    m_pProgressBar->show();
    m_pLabel->hide();
    break;
  }
}


void KIOLittleProgressDlg::clean() {
  m_pJob = 0L;
  m_pProgressBar->setValue( 0 );
  m_pLabel->clear();

  mode = None;
  setMode();
}


void KIOLittleProgressDlg::slotTotalSize( int, unsigned long _size ) {
  m_iTotalSize = _size;
}

void KIOLittleProgressDlg::slotPercent( int, unsigned long _percent ) {
  m_pProgressBar->setValue( _percent );
}


void KIOLittleProgressDlg::slotSpeed( int, unsigned long _bytes_per_second ) {
  if ( _bytes_per_second == 0 ) {
    m_pLabel->setText( i18n( " Stalled ") );
  } else {
    m_pLabel->setText( i18n( " %1/s %2 ").arg( KIOJob::convertSize( _bytes_per_second )).arg( m_pJob->getRemainingTime().toString()) );
  }
}


bool KIOLittleProgressDlg::eventFilter( QObject *, QEvent *ev ) {
  if ( ! m_pJob ) { // don't react when there isn't any job doing IO
    return true;
  }

  if ( ev->type() == QEvent::MouseButtonPress ) {
    QMouseEvent *e = (QMouseEvent*)ev;

    if ( e->button() == LeftButton ) {    // toggle view on left mouse button
      if ( mode == Label ) {
	mode = Progress;
      } else if ( mode == Progress ) {
	mode = Label;
      }
      setMode();
      return true;

    }
  }

  return false;
}


#include "kio_littleprogress_dlg.moc"
