/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qtooltip.h>
#include <qlayout.h>
#include <qwidgetstack.h>

#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>

#include "jobclasses.h"
#include "statusbarprogress.h"

namespace KIO {

StatusbarProgress::StatusbarProgress( QWidget* parent, bool button )
  : ProgressBase( parent ) {

  m_bShowButton = button;

  // only clean this dialog
  setOnlyClean(true);
  // TODO : is this really needed ?
  setStopOnClose(false);

  int w = fontMetrics().width( " 999.9 kB/s 00:00:01 " ) + 8;
  box = new QHBoxLayout( this, 0, 0 );

  m_pButton = new QPushButton( "X", this );
  box->addWidget( m_pButton  );
  stack = new QWidgetStack( this );
  box->addWidget( stack );
  connect( m_pButton, SIGNAL( clicked() ), this, SLOT( slotStop() ) );

  m_pProgressBar = new KProgress( this );
  m_pProgressBar->setFrameStyle( QFrame::Box | QFrame::Raised );
  m_pProgressBar->setLineWidth( 1 );
  m_pProgressBar->setBackgroundMode( QWidget::PaletteBackground );
  m_pProgressBar->installEventFilter( this );
  m_pProgressBar->setMinimumWidth( w );
  stack->addWidget( m_pProgressBar, 1 );

  m_pLabel = new QLabel( "", this );
  m_pLabel->setAlignment( AlignHCenter | AlignVCenter );
  m_pLabel->installEventFilter( this );
  m_pLabel->setMinimumWidth( w );
  stack->addWidget( m_pLabel, 2 );
  setMinimumSize( sizeHint() );

  mode = None;
  setMode();
}


void StatusbarProgress::setJob( KIO::Job *job )
{
  ProgressBase::setJob( job );

  mode = Progress;
  setMode();
}


void StatusbarProgress::setMode() {
  switch ( mode ) {
  case None:
    if ( m_bShowButton ) {
      m_pButton->hide();
    }
    stack->hide();
    break;

  case Label:
    if ( m_bShowButton ) {
      m_pButton->show();
    }
    stack->show();
    stack->raiseWidget( m_pLabel );
    break;

  case Progress:
    if ( m_bShowButton ) {
      m_pButton->show();
    }
    stack->show();
    stack->raiseWidget( m_pProgressBar );
    break;
  }
}


void StatusbarProgress::slotClean() {
  // we don't want to delete this widget, only clean
  m_pProgressBar->setValue( 0 );
  m_pLabel->clear();

  mode = None;
  setMode();
}


void StatusbarProgress::slotTotalSize( KIO::Job*, KIO::filesize_t size ) {
  m_iTotalSize = size;
}

void StatusbarProgress::slotPercent( KIO::Job*, unsigned long percent ) {
  m_pProgressBar->setValue( percent );
}


void StatusbarProgress::slotSpeed( KIO::Job*, unsigned long bytes_per_second ) {
  if ( bytes_per_second == 0 ) {
    m_pLabel->setText( i18n( " Stalled ") );
  } else {
    m_pLabel->setText( i18n( " %1/s ").arg( KIO::convertSize( bytes_per_second )) );
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

void StatusbarProgress::virtual_hook( int id, void* data )
{ ProgressBase::virtual_hook( id, data ); }

} /* namespace */
#include "statusbarprogress.moc"
