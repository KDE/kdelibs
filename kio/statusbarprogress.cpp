// $Id$

#include <qtooltip.h>

#include <kapp.h>
#include <klocale.h>

#include "jobclasses.h"
#include "statusbarprogress.h"

StatusbarProgress::StatusbarProgress( QWidget* parent, bool button )
  : ProgressBase( parent ) {

  m_bShowButton = button;
  m_bOnlyClean = true;  // we don't want to delete this widget, only clean

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
}


void StatusbarProgress::setJob( KIO::Job *job ) {
  ProgressBase::setJob( job );

  connect( m_pButton, SIGNAL( clicked() ), this, SLOT( stop() ) );
  mode = Progress;
  setMode();
}


void StatusbarProgress::setMode() {
  switch ( mode ) {
  case None:
    if ( m_bShowButton ) {
      m_pButton->hide();
    }
    m_pProgressBar->hide();
    m_pLabel->hide();
    break;

  case Label:
    if ( m_bShowButton ) {
      m_pButton->show();
    }
    m_pProgressBar->hide();
    m_pLabel->show();
    break;

  case Progress:
    if ( m_bShowButton ) {
      m_pButton->show();
    }
    m_pProgressBar->show();
    m_pLabel->hide();
    break;
  }
}


void StatusbarProgress::clean() {
  m_pJob = 0L;
  m_pProgressBar->setValue( 0 );
  m_pLabel->clear();

  mode = None;
  setMode();
}


void StatusbarProgress::slotTotalSize( KIO::Job*, unsigned long _size ) {
  m_iTotalSize = _size;
}

void StatusbarProgress::slotPercent( KIO::Job*, unsigned long _percent ) {
  m_pProgressBar->setValue( _percent );
}


void StatusbarProgress::slotSpeed( KIO::Job*, unsigned long _bytes_per_second ) {
  if ( _bytes_per_second == 0 ) {
    m_pLabel->setText( i18n( " Stalled ") );
  } else {
    m_pLabel->setText( i18n( " %1/s ").arg( KIO::convertSize( _bytes_per_second )) );
  }
}


bool StatusbarProgress::eventFilter( QObject *, QEvent *ev ) {
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

#include "statusbarprogress.moc"
