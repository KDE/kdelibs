
#include <kapp.h>
#include <kwm.h>

#include "kio_job.h"
#include "kio_littleprogress_dlg.h"

QString convertSize( int size );

KIOLittleProgressDlg::KIOLittleProgressDlg( QWidget* parent ) : QWidget( parent )
{
  m_iTotalSize = 0;
  m_iProcessedSize = 0;
  m_pJob = 0L;
  
  QFontMetrics fm = fontMetrics();
  int w = fm.width( " 100 %  134.5 kB/s " ) + 8;
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
  m_pProgressBar->setValue( 0 );
  m_pLabel->clear();
  setMode( true );
}


void KIOLittleProgressDlg::totalSize( unsigned long _bytes )
{
  m_iTotalSize = _bytes;
}


void KIOLittleProgressDlg::processedSize( unsigned long _bytes )
{
  if ( _bytes == 0 || m_iTotalSize == 0 )
    return;
  
  m_iProcessedSize = _bytes;
  
  int old = m_iPercent;

  m_iPercent = (int)(( (float)_bytes / (float)m_iTotalSize ) * 100.0);

  if ( m_iPercent == old )
    return;

  QString tmp;
  tmp.sprintf(" %d %%  %s/s ", m_iPercent, convertSize( m_iSpeed ).ascii() );

  m_pLabel->setText( tmp );
  m_pProgressBar->setValue( m_iPercent );

}


void KIOLittleProgressDlg::speed( unsigned long _bytes_per_second )
{
  if ( m_iProcessedSize == 0 )
    return;
  
  m_iSpeed = _bytes_per_second;

  QString tmp;

  if ( m_iSpeed == 0 )
    tmp.sprintf( i18n( " %d %%  Stalled "), m_iPercent );
  else
    tmp.sprintf( " %d %%  %s/s ", m_iPercent, convertSize( m_iSpeed ).ascii() );

  m_pLabel->setText( tmp );
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


//
//  Helper functions
//

QString convertSize( int size ) {
  float fsize;
  QString s;
  if ( size > 1048576 ){
    fsize = (float) size / (float) 1048576;
    s.sprintf ( "%.1f MB", fsize);
  } else if ( size > 1024 ){
    fsize = (float) size / (float) 1024;
    s.sprintf ( "%.1f kB", fsize);
  } else {
    s.sprintf ( "%d B", size);
  }
  return s;
}

#include "kio_littleprogress_dlg.moc"
