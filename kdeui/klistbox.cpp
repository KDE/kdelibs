/* This file is part of the KDE libraries
   Copyright (C) 2000 Reginald Stadlbauer <reggie@kde.org>

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

#include <kglobalsettings.h>
#include <kcursor.h>
#include <kapplication.h>
#include <kipc.h>
#include <kdebug.h>

#include "klistbox.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

KListBox::KListBox( QWidget *parent, const char *name, WFlags f )
    : QListBox( parent, name, f )
{
    oldCursor = viewport()->cursor();
    connect( this, SIGNAL( onViewport() ),
	     this, SLOT( slotOnViewport() ) );
    connect( this, SIGNAL( onItem( QListBoxItem * ) ),
	     this, SLOT( slotOnItem( QListBoxItem * ) ) );
    slotSettingsChanged(KApplication::SETTINGS_MOUSE);
    if (kapp)
    {
        connect( kapp, SIGNAL( settingsChanged(int) ), SLOT( slotSettingsChanged(int) ) );
        kapp->addKipcEventMask( KIPC::SettingsChanged );
    }

    m_pCurrentItem = 0L;

    m_pAutoSelect = new QTimer( this );
    connect( m_pAutoSelect, SIGNAL( timeout() ),
    	     this, SLOT( slotAutoSelect() ) );
}

void KListBox::slotOnItem( QListBoxItem *item )
{
    if ( item && m_bChangeCursorOverItem && m_bUseSingle )
        viewport()->setCursor( KCursor().handCursor() );

    if ( item && (m_autoSelectDelay > -1) && m_bUseSingle ) {
      m_pAutoSelect->start( m_autoSelectDelay, true );
      m_pCurrentItem = item;
    }
}

void KListBox::slotOnViewport()
{
    if ( m_bChangeCursorOverItem )
        viewport()->setCursor( oldCursor );

    m_pAutoSelect->stop();
    m_pCurrentItem = 0L;
}


void KListBox::slotSettingsChanged(int category)
{
    if (category != KApplication::SETTINGS_MOUSE)
        return;
    m_bUseSingle = KGlobalSettings::singleClick();

    disconnect( this, SIGNAL( mouseButtonClicked( int, QListBoxItem *,
						  const QPoint & ) ),
		this, SLOT( slotMouseButtonClicked( int, QListBoxItem *,
						    const QPoint & ) ) );
//         disconnect( this, SIGNAL( doubleClicked( QListBoxItem *, 
// 						 const QPoint & ) ),
// 		    this, SLOT( slotExecute( QListBoxItem *, 
// 					     const QPoint & ) ) );

    if( m_bUseSingle )
    {
      connect( this, SIGNAL( mouseButtonClicked( int, QListBoxItem *, 
						 const QPoint & ) ),
	       this, SLOT( slotMouseButtonClicked( int, QListBoxItem *,
						   const QPoint & ) ) );
    }
    else
    {
//         connect( this, SIGNAL( doubleClicked( QListBoxItem *, 
// 					      const QPoint & ) ),
//                  this, SLOT( slotExecute( QListBoxItem *, 
// 					  const QPoint & ) ) );
    }

    m_bChangeCursorOverItem = KGlobalSettings::changeCursorOverIcon();
    m_autoSelectDelay = KGlobalSettings::autoSelectDelay();

    if( !m_bUseSingle || !m_bChangeCursorOverItem )
	viewport()->setCursor( oldCursor );
}

void KListBox::slotAutoSelect()
{
  // check that the item still exists
  if( index( m_pCurrentItem ) == -1 )
    return;

  //Give this widget the keyboard focus.
  if( !hasFocus() )
    setFocus();

#ifdef Q_WS_X11 //FIXME
  Window root;
  Window child;
  int root_x, root_y, win_x, win_y;
  uint keybstate;
  XQueryPointer( qt_xdisplay(), qt_xrootwin(), &root, &child,
		 &root_x, &root_y, &win_x, &win_y, &keybstate );
#endif

  QListBoxItem* previousItem = item( currentItem() ); 
  setCurrentItem( m_pCurrentItem );

  if( m_pCurrentItem ) {
#ifndef Q_WS_QWS //FIXME
    //Shift pressed?
    if( (keybstate & ShiftMask) ) {
#endif
      bool block = signalsBlocked();
      blockSignals( true );

#ifndef Q_WS_QWS //FIXME
      //No Ctrl? Then clear before!
      if( !(keybstate & ControlMask) )  
	clearSelection(); 
#endif

      bool select = !m_pCurrentItem->isSelected();
      bool update = viewport()->isUpdatesEnabled();
      viewport()->setUpdatesEnabled( false );

      bool down = index( previousItem ) < index( m_pCurrentItem );
      QListBoxItem* it = down ? previousItem : m_pCurrentItem;
      for (;it ; it = it->next() ) {
	if ( down && it == m_pCurrentItem ) {
	  setSelected( m_pCurrentItem, select );
	  break;
	}
	if ( !down && it == previousItem ) {
	  setSelected( previousItem, select );
	  break;
	}
	setSelected( it, select );
      }
      
      blockSignals( block );
      viewport()->setUpdatesEnabled( update );
      triggerUpdate( false );

      emit selectionChanged();

      if( selectionMode() == QListBox::Single )
	emit selectionChanged( m_pCurrentItem );
    }
#ifndef Q_WS_QWS //FIXME
    else if( (keybstate & ControlMask) )
      setSelected( m_pCurrentItem, !m_pCurrentItem->isSelected() );
#endif
    else {
      bool block = signalsBlocked();
      blockSignals( true );

      if( !m_pCurrentItem->isSelected() )
	clearSelection(); 

      blockSignals( block );

      setSelected( m_pCurrentItem, true );
    }
#ifndef Q_WS_QWS //FIXME
  }
  else
    kdDebug() << "That´s not supposed to happen!!!!" << endl;
#endif
}

void KListBox::emitExecute( QListBoxItem *item, const QPoint &pos )
{
#ifdef Q_WS_X11 //FIXME
  Window root;
  Window child;
  int root_x, root_y, win_x, win_y;
  uint keybstate;
  XQueryPointer( qt_xdisplay(), qt_xrootwin(), &root, &child,
		 &root_x, &root_y, &win_x, &win_y, &keybstate );
#endif
    
  m_pAutoSelect->stop();
  
#ifndef Q_WS_QWS //FIXME
  //Don´t emit executed if in SC mode and Shift or Ctrl are pressed
  if( !( m_bUseSingle && ((keybstate & ShiftMask) || (keybstate & ControlMask)) ) ) {
#endif
    emit executed( item );
    emit executed( item, pos );
#ifndef Q_WS_QWS //FIXME
  }
#endif
}

//
// 2000-16-01 Espen Sand
// This widget is used in dialogs. It should ignore
// F1 (and combinations) and Escape since these are used
// to start help or close the dialog. This functionality
// should be done in QListView but it is not (at least now)
//
void KListBox::keyPressEvent(QKeyEvent *e)
{
  if( e->key() == Key_Escape )
  {
    e->ignore();
  }
  else if( e->key() == Key_F1 )
  {
    e->ignore();
  }
  else
  {
    QListBox::keyPressEvent(e);
  }
}

void KListBox::focusOutEvent( QFocusEvent *fe )
{
  m_pAutoSelect->stop();

  QListBox::focusOutEvent( fe );
}

void KListBox::leaveEvent( QEvent *e ) 
{
  m_pAutoSelect->stop();

  QListBox::leaveEvent( e );
}

void KListBox::contentsMousePressEvent( QMouseEvent *e )
{
  if( (selectionMode() == Extended) && (e->state() & ShiftButton) && !(e->state() & ControlButton) ) {
    bool block = signalsBlocked();
    blockSignals( true );

    clearSelection();

    blockSignals( block );
  }

  QListBox::contentsMousePressEvent( e );
}

void KListBox::contentsMouseDoubleClickEvent ( QMouseEvent * e )
{
  QListBox::contentsMouseDoubleClickEvent( e );

  QListBoxItem* item = itemAt( e->pos() );

  if( item ) {
    emit doubleClicked( item, e->globalPos() );

    if( (e->button() == LeftButton) && !m_bUseSingle )
      emitExecute( item, e->globalPos() );
  }
}

void KListBox::slotMouseButtonClicked( int btn, QListBoxItem *item, const QPoint &pos )
{
  if( (btn == LeftButton) && item )
    emitExecute( item, pos );
}

#include "klistbox.moc"
