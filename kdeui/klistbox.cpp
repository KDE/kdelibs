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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "config.h"

#include <qtimer.h>

#include <kglobalsettings.h>
#include <kcursor.h>
#include <kapplication.h>
#include <kipc.h> 
#include <kdebug.h>

#include "klistbox.h"
#include <QKeyEvent>

KListBox::KListBox( QWidget *parent, const char *name, Qt::WFlags f )
    : Q3ListBox( parent, name, f ), d(0)
{
    connect( this, SIGNAL( onViewport() ),
	     this, SLOT( slotOnViewport() ) );
    connect( this, SIGNAL( onItem( Q3ListBoxItem * ) ),
	     this, SLOT( slotOnItem( Q3ListBoxItem * ) ) );
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

void KListBox::slotOnItem( Q3ListBoxItem *item )
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
        viewport()->unsetCursor();

    m_pAutoSelect->stop();
    m_pCurrentItem = 0L;
}


void KListBox::slotSettingsChanged(int category)
{
    if (category != KApplication::SETTINGS_MOUSE)
        return;
    m_bUseSingle = KGlobalSettings::singleClick();

    disconnect( this, SIGNAL( mouseButtonClicked( int, Q3ListBoxItem *,
						  const QPoint & ) ),
		this, SLOT( slotMouseButtonClicked( int, Q3ListBoxItem *,
						    const QPoint & ) ) );
//         disconnect( this, SIGNAL( doubleClicked( QListBoxItem *, 
// 						 const QPoint & ) ),
// 		    this, SLOT( slotExecute( QListBoxItem *, 
// 					     const QPoint & ) ) );

    if( m_bUseSingle )
    {
      connect( this, SIGNAL( mouseButtonClicked( int, Q3ListBoxItem *, 
						 const QPoint & ) ),
	       this, SLOT( slotMouseButtonClicked( int, Q3ListBoxItem *,
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
        viewport()->unsetCursor();
}

void KListBox::slotAutoSelect()
{
  // check that the item still exists
  if( index( m_pCurrentItem ) == -1 )
    return;

  //Give this widget the keyboard focus.
  if( !hasFocus() )
    setFocus();

  Qt::ButtonState keybstate = QApplication::keyboardModifiers();

  Q3ListBoxItem* previousItem = item( currentItem() ); 
  setCurrentItem( m_pCurrentItem );

  if( m_pCurrentItem ) {
    //Shift pressed?
    if( (keybstate & Qt::ShiftModifier) ) {
      bool block = signalsBlocked();
      blockSignals( true );

      //No Ctrl? Then clear before!
      if( !(keybstate & Qt::ControlModifier) )  
	clearSelection(); 

      bool select = !m_pCurrentItem->isSelected();
      bool update = viewport()->isUpdatesEnabled();
      viewport()->setUpdatesEnabled( false );

      bool down = index( previousItem ) < index( m_pCurrentItem );
      Q3ListBoxItem* it = down ? previousItem : m_pCurrentItem;
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

      if( selectionMode() == Q3ListBox::Single )
	emit selectionChanged( m_pCurrentItem );
    }
    else if( (keybstate & Qt::ControlModifier) )
      setSelected( m_pCurrentItem, !m_pCurrentItem->isSelected() );
    else {
      bool block = signalsBlocked();
      blockSignals( true );

      if( !m_pCurrentItem->isSelected() )
	clearSelection(); 

      blockSignals( block );

      setSelected( m_pCurrentItem, true );
    }
  }
  else
    kdDebug() << "That´s not supposed to happen!!!!" << endl;
}

void KListBox::emitExecute( Q3ListBoxItem *item, const QPoint &pos )
{
  Qt::ButtonState keybstate = QApplication::keyboardModifiers();
    
  m_pAutoSelect->stop();
  
  //Don´t emit executed if in SC mode and Shift or Ctrl are pressed
  if( !( m_bUseSingle && ((keybstate & Qt::ShiftModifier) || (keybstate & Qt::ControlModifier)) ) ) {
    emit executed( item );
    emit executed( item, pos );
  }
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
  if( e->key() == Qt::Key_Escape )
  {
    e->ignore();
  }
  else if( e->key() == Qt::Key_F1 )
  {
    e->ignore();
  }
  else
  {
    Q3ListBox::keyPressEvent(e);
  }
}

void KListBox::focusOutEvent( QFocusEvent *fe )
{
  m_pAutoSelect->stop();

  Q3ListBox::focusOutEvent( fe );
}

void KListBox::leaveEvent( QEvent *e ) 
{
  m_pAutoSelect->stop();

  Q3ListBox::leaveEvent( e );
}

void KListBox::contentsMousePressEvent( QMouseEvent *e )
{
  if( (selectionMode() == Extended) && (e->state() & Qt::ShiftModifier) && !(e->state() & Qt::ControlModifier) ) {
    bool block = signalsBlocked();
    blockSignals( true );

    clearSelection();

    blockSignals( block );
  }

  Q3ListBox::contentsMousePressEvent( e );
}

void KListBox::contentsMouseDoubleClickEvent ( QMouseEvent * e )
{
  Q3ListBox::contentsMouseDoubleClickEvent( e );

  Q3ListBoxItem* item = itemAt( contentsToViewport( e->pos() ) );

  if( item ) {
    emit doubleClicked( item, e->globalPos() );

    if( (e->button() == Qt::LeftButton) && !m_bUseSingle )
      emitExecute( item, e->globalPos() );
  }
}

void KListBox::slotMouseButtonClicked( int btn, Q3ListBoxItem *item, const QPoint &pos )
{
  if( (btn == Qt::LeftButton) && item )
    emitExecute( item, pos );
}

void KListBox::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "klistbox.moc"
