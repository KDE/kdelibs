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

#include "klistwidget.h"

#include <kglobalsettings.h>
#include <kcursor.h>
#include <kdebug.h>

#include <qtimer.h>
#include <QKeyEvent>
#include <QApplication>

KListWidget::KListWidget( QWidget *parent )
    : QListWidget(parent), d(0)
{
    connect( this, SIGNAL( viewportEntered() ),
	     this, SLOT( slotOnViewport() ) );
    connect( this, SIGNAL( itemEntered( QListWidgetItem * ) ),
	     this, SLOT( slotItemEntered( QListWidgetItem * ) ) );
    slotSettingsChanged(KGlobalSettings::SETTINGS_MOUSE);
    connect( KGlobalSettings::self(), SIGNAL( settingsChanged(int) ), SLOT( slotSettingsChanged(int) ) );

    m_pCurrentItem = 0L;

    m_pAutoSelect = new QTimer( this );
    connect( m_pAutoSelect, SIGNAL( timeout() ),
    	     this, SLOT( slotAutoSelect() ) );
}

void KListWidget::slotItemEntered( QListWidgetItem *item )
{
    if ( item && m_bChangeCursorOverItem && m_bUseSingle )
        viewport()->setCursor( KCursor().handCursor() );

    if ( item && (m_autoSelectDelay > -1) && m_bUseSingle ) {
      m_pAutoSelect->setSingleShot( true );
      m_pAutoSelect->start( m_autoSelectDelay );
      m_pCurrentItem = item;
    }
}

void KListWidget::slotOnViewport()
{
    if ( m_bChangeCursorOverItem )
        viewport()->unsetCursor();

    m_pAutoSelect->stop();
    m_pCurrentItem = 0L;
}


void KListWidget::slotSettingsChanged(int category)
{
    if (category != KGlobalSettings::SETTINGS_MOUSE)
        return;
    m_bUseSingle = KGlobalSettings::singleClick();

    disconnect( this, SIGNAL( mouseButtonClicked( int, QListWidgetItem *,
						  const QPoint & ) ),
		this, SLOT( slotMouseButtonClicked( int, QListWidgetItem *,
						    const QPoint & ) ) );
//         disconnect( this, SIGNAL( doubleClicked( QListBoxItem *,
// 						 const QPoint & ) ),
// 		    this, SLOT( slotExecute( QListBoxItem *,
// 					     const QPoint & ) ) );

    if( m_bUseSingle )
    {
      connect( this, SIGNAL( mouseButtonClicked( int, QListWidgetItem *,
						 const QPoint & ) ),
	       this, SLOT( slotMouseButtonClicked( int, QListWidgetItem *,
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

void KListWidget::slotAutoSelect()
{
  // check that the item still exists
  if( row( m_pCurrentItem ) == -1 )
    return;

  //Give this widget the keyboard focus.
  if( !hasFocus() )
    setFocus();

  Qt::KeyboardModifiers keybstate = QApplication::keyboardModifiers();

  QListWidgetItem* previousItem = currentItem();
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
      bool update = viewport()->updatesEnabled();
      viewport()->setUpdatesEnabled( false );

      bool down = row( previousItem ) < row( m_pCurrentItem );
      QListWidgetItem* it = down ? previousItem : m_pCurrentItem;
      
      for (int i = row(it) ; i < count() ; i++ ) {
	    if ( down && item(i) == m_pCurrentItem ) {
	        m_pCurrentItem->setSelected(select); 
	        break;
	    }

	    if ( !down && item(i) == previousItem ) {
	        previousItem->setSelected(select);
	        break;
	    }
	    it->setSelected(select);
    }

      blockSignals( block );
      viewport()->setUpdatesEnabled( update );

      emit itemSelectionChanged();

      if( selectionMode() == QAbstractItemView::SingleSelection )
	emit itemSelectionChanged();
    }
    else if( (keybstate & Qt::ControlModifier) )
      m_pCurrentItem->setSelected(!m_pCurrentItem->isSelected());
    else {
      bool block = signalsBlocked();
      blockSignals( true );

      if( !m_pCurrentItem->isSelected() )
	clearSelection();

      blockSignals( block );

      m_pCurrentItem->setSelected(true);
    }
  }
  else
    kDebug() << "That's not supposed to happen!!!!" << endl;
}

void KListWidget::emitExecute( QListWidgetItem *item, const QPoint &pos )
{
  Qt::KeyboardModifiers keybstate = QApplication::keyboardModifiers();

  m_pAutoSelect->stop();

  //Don't emit executed if in SC mode and Shift or Ctrl are pressed
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
void KListWidget::keyPressEvent(QKeyEvent *e)
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
    QListWidget::keyPressEvent(e);
  }
}

void KListWidget::focusOutEvent( QFocusEvent *fe )
{
  m_pAutoSelect->stop();

  QListWidget::focusOutEvent( fe );
}

void KListWidget::leaveEvent( QEvent *e )
{
  m_pAutoSelect->stop();

  QListWidget::leaveEvent( e );
}

void KListWidget::mousePressEvent( QMouseEvent *e )
{
  if( (selectionMode() == QAbstractItemView::ExtendedSelection) && (e->modifiers() & Qt::ShiftModifier) && !(e->modifiers() & Qt::ControlModifier) ) {
    bool block = signalsBlocked();
    blockSignals( true );

    clearSelection();

    blockSignals( block );
  }

  QListWidget::mousePressEvent( e );
}

void KListWidget::mouseDoubleClickEvent ( QMouseEvent * e )
{
  QListWidget::mouseDoubleClickEvent( e );

  QListWidgetItem* item = itemAt( e->pos() );

  if( item ) {
    emit doubleClicked( item, e->globalPos() );

    if( (e->button() == Qt::LeftButton) && !m_bUseSingle )
      emitExecute( item, e->globalPos() );
  }
}

void KListWidget::slotMouseButtonClicked( int btn, QListWidgetItem *item, const QPoint &pos )
{
  if( (btn == Qt::LeftButton) && item )
    emitExecute( item, pos );
}

#include "klistwidget.moc"
