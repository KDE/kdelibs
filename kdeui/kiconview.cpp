/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include <qtimer.h>

#include "kiconview.h"
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kapp.h>
#include <kipc.h>
#include <kcursor.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

class KIconView::KIconViewPrivate
{
public:
    KIconViewPrivate() {
	mode = KIconView::Execute;
	doAutoSelect = TRUE;
    }
    KIconView::Mode mode;
    bool doAutoSelect;
};

KIconView::KIconView( QWidget *parent, const char *name, WFlags f )
    : QIconView( parent, name, f )
{
    d = new KIconViewPrivate;

    oldCursor = viewport()->cursor();
    connect( this, SIGNAL( onViewport() ),
	     this, SLOT( slotOnViewport() ) );
    connect( this, SIGNAL( onItem( QIconViewItem * ) ),
             this, SLOT( slotOnItem( QIconViewItem * ) ) );
    slotSettingsChanged( KApplication::SETTINGS_MOUSE );
    connect( kapp, SIGNAL( settingsChanged(int) ), SLOT( slotSettingsChanged(int) ) );
    kapp->addKipcEventMask( KIPC::SettingsChanged );

    m_pCurrentItem = 0L;

    m_pAutoSelect = new QTimer( this );
    connect( m_pAutoSelect, SIGNAL( timeout() ),
    	     this, SLOT( slotAutoSelect() ) );
}

KIconView::~KIconView()
{
    delete d;
}


void KIconView::setMode( KIconView::Mode mode )
{
    d->mode = mode;
}

KIconView::Mode KIconView::mode() const
{
    return d->mode;
}


int KIconView::itemIndex( const QIconViewItem *item ) const
{
    if ( !item )
	return -1;

    if ( item == firstItem() )
	return 0;
    else if ( item == lastItem() )
	return count() - 1;
    else {
	QIconViewItem *i = firstItem();
	int j = 0;
	while ( i && i != item ) {
	    i = i->nextItem();
	    ++j;
	}

	return i ? j : -1;
    }
}

void KIconView::slotOnItem( QIconViewItem *item )
{
    if ( item && m_bChangeCursorOverItem && m_bUseSingle )
        viewport()->setCursor( KCursor().handCursor() );

    if ( item && (m_autoSelectDelay > -1) && m_bUseSingle ) {
      m_pAutoSelect->start( m_autoSelectDelay, true );
      m_pCurrentItem = item;
    }
}

void KIconView::slotOnViewport()
{
    if ( m_bChangeCursorOverItem )
        viewport()->setCursor( oldCursor );

    m_pAutoSelect->stop();
    m_pCurrentItem = 0L;
}

void KIconView::slotSettingsChanged(int category)
{
    if ( category != KApplication::SETTINGS_MOUSE )
      return;
    m_bUseSingle = KGlobalSettings::singleClick();
    //kdDebug() << "KIconView::slotSettingsChanged for mouse, usesingle=" << m_bUseSingle << endl;

    disconnect( this, SIGNAL( mouseButtonClicked( int, QIconViewItem *,
						  const QPoint & ) ),
		this, SLOT( slotMouseButtonClicked( int, QIconViewItem *,
						    const QPoint & ) ) );
//         disconnect( this, SIGNAL( doubleClicked( QIconViewItem *,
// 						 const QPoint & ) ),
// 		    this, SLOT( slotExecute( QIconViewItem *,
// 					     const QPoint & ) ) );

    if( m_bUseSingle ) {
      connect( this, SIGNAL( mouseButtonClicked( int, QIconViewItem *,
						 const QPoint & ) ),
	       this, SLOT( slotMouseButtonClicked( int, QIconViewItem *,
						   const QPoint & ) ) );
    }
    else {
//         connect( this, SIGNAL( doubleClicked( QIconViewItem *,
// 					      const QPoint & ) ),
//                  this, SLOT( slotExecute( QIconViewItem *,
// 					  const QPoint & ) ) );
    }

    m_bChangeCursorOverItem = KGlobalSettings::changeCursorOverIcon();
    m_autoSelectDelay = m_bUseSingle ? KGlobalSettings::autoSelectDelay() : -1;

    if( !m_bUseSingle || !m_bChangeCursorOverItem )
        viewport()->setCursor( oldCursor );
}

void KIconView::slotAutoSelect()
{
  // check that the item still exists
  if( itemIndex( m_pCurrentItem ) == -1 || !d->doAutoSelect )
    return;

  //Give this widget the keyboard focus.
  if( !hasFocus() )
    setFocus();

#ifdef Q_WS_X11
  //FIXME(E): Implement for Qt Embedded
  Window root;
  Window child;
  int root_x, root_y, win_x, win_y;
  uint keybstate;
  XQueryPointer( qt_xdisplay(), qt_xrootwin(), &root, &child,
		 &root_x, &root_y, &win_x, &win_y, &keybstate );
#endif

  QIconViewItem* previousItem = currentItem();
  setCurrentItem( m_pCurrentItem );

  if( m_pCurrentItem ) {
    //Shift pressed?
#ifdef Q_WS_X11 //FIXME
    if( (keybstate & ShiftMask) ) {
      //Temporary implementaion of the selection until QIconView supports it
      bool block = signalsBlocked();
      blockSignals( true );

      //No Ctrl? Then clear before!
      if( !(keybstate & ControlMask) )
	clearSelection();

      bool select = !m_pCurrentItem->isSelected();
      bool update = viewport()->isUpdatesEnabled();
      viewport()->setUpdatesEnabled( false );

      //Calculate the smallest rectangle that contains the current Item
      //and the one that got the autoselect event
      QRect r;
      QRect redraw;
      if ( previousItem )
	r = QRect( QMIN( previousItem->x(), m_pCurrentItem->x() ),
		   QMIN( previousItem->y(), m_pCurrentItem->y() ),
		   0, 0 );
      else
	r = QRect( 0, 0, 0, 0 );
      if ( previousItem->x() < m_pCurrentItem->x() )
	r.setWidth( m_pCurrentItem->x() - previousItem->x() + m_pCurrentItem->width() );
      else
	r.setWidth( previousItem->x() - m_pCurrentItem->x() + previousItem->width() );
      if ( previousItem->y() < m_pCurrentItem->y() )
	r.setHeight( m_pCurrentItem->y() - previousItem->y() + m_pCurrentItem->height() );
      else
	r.setHeight( previousItem->y() - m_pCurrentItem->y() + previousItem->height() );
      r = r.normalize();

      //Check for each item whether it is within the rectangle.
      //If yes, select it
      for( QIconViewItem* i = firstItem(); i; i = i->nextItem() ) {
	if( i->intersects( r ) ) {
	  redraw = redraw.unite( i->rect() );
	  setSelected( i, select, true );
	}
      }

      blockSignals( block );
      viewport()->setUpdatesEnabled( update );
      repaintContents( redraw, false );

      emit selectionChanged();

      if( selectionMode() == QIconView::Single )
	emit selectionChanged( m_pCurrentItem );

      //setSelected( m_pCurrentItem, true, (keybstate & ControlMask), (keybstate & ShiftMask) );
    }
    else if( (keybstate & ControlMask) )
      setSelected( m_pCurrentItem, !m_pCurrentItem->isSelected(), true );
    else
#endif
      setSelected( m_pCurrentItem, true );
  }
#ifndef Q_WS_QWS //FIXME: Remove #if as soon as the stuff above is implemented
  else
    kdDebug() << "KIconView: That's not supposed to happen!!!!" << endl;
#endif
}

void KIconView::emitExecute( QIconViewItem *item, const QPoint &pos )
{
  if ( d->mode != Execute )
  {
    // kdDebug() << "KIconView::emitExecute : not in execute mode !" << endl;
    return;
  }

#ifdef Q_WS_X11 //FIXME
  Window root;
  Window child;
  int root_x, root_y, win_x, win_y;
  uint keybstate;
  XQueryPointer( qt_xdisplay(), qt_xrootwin(), &root, &child,
		 &root_x, &root_y, &win_x, &win_y, &keybstate );
#endif

  m_pAutoSelect->stop();

  //Don´t emit executed if in SC mode and Shift or Ctrl are pressed
#ifdef Q_WS_X11 //FIXME
  if( !( m_bUseSingle && ((keybstate & ShiftMask) || (keybstate & ControlMask)) ) ) {
    setSelected( item, false );
    emit executed( item );
    emit executed( item, pos );
  }
#endif
}

void KIconView::focusOutEvent( QFocusEvent *fe )
{
  m_pAutoSelect->stop();

  QIconView::focusOutEvent( fe );
}

void KIconView::leaveEvent( QEvent *e )
{
  m_pAutoSelect->stop();

  QIconView::leaveEvent( e );
}

void KIconView::contentsMousePressEvent( QMouseEvent *e )
{
  if( (selectionMode() == Extended) && (e->state() & ShiftButton) && !(e->state() & ControlButton) ) {
    bool block = signalsBlocked();
    blockSignals( true );

    clearSelection();

    blockSignals( block );
  }

  QIconView::contentsMousePressEvent( e );
  d->doAutoSelect = FALSE;
}

void KIconView::contentsMouseDoubleClickEvent ( QMouseEvent * e )
{
  QIconView::contentsMouseDoubleClickEvent( e );

  QIconViewItem* item = findItem( e->pos() );

  if( item ) {
    if( (e->button() == LeftButton) && !m_bUseSingle )
      emitExecute( item, e->globalPos() );

    emit doubleClicked( item, e->globalPos() );
  }
}

void KIconView::slotMouseButtonClicked( int btn, QIconViewItem *item, const QPoint &pos )
{
  //kdDebug() << " KIconView::slotMouseButtonClicked() item=" << item << endl;
  if( (btn == LeftButton) && item )
    emitExecute( item, pos );
}

void KIconView::contentsMouseReleaseEvent( QMouseEvent *e )
{
    d->doAutoSelect = TRUE;
    QIconView::contentsMouseReleaseEvent( e );
}

#include "kiconview.moc"
