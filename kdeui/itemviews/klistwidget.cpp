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
#include <kdebug.h>

#include <QtCore/QTimer>
#include <QKeyEvent>
#include <QApplication>

class KListWidget::KListWidgetPrivate
{
    public:
        KListWidgetPrivate(KListWidget *q)
            : q(q),
              m_pCurrentItem(0)
        {
        }

        void emitExecute( QListWidgetItem *item, const QPoint &pos );

        void _k_slotItemEntered(QListWidgetItem*);
        void _k_slotOnViewport();
        void _k_slotSettingsChanged(int);
        void _k_slotAutoSelect();

        KListWidget *q;
        bool m_bUseSingle : 1;
        bool m_bChangeCursorOverItem : 1;

        QListWidgetItem* m_pCurrentItem;
        QTimer* m_pAutoSelect;
        int m_autoSelectDelay;
};

KListWidget::KListWidget( QWidget *parent )
    : QListWidget(parent), d(new KListWidgetPrivate(this))
{
    connect( this, SIGNAL( viewportEntered() ),
             this, SLOT( _k_slotOnViewport() ) );
    connect( this, SIGNAL( itemEntered( QListWidgetItem * ) ),
             this, SLOT( _k_slotItemEntered( QListWidgetItem * ) ) );
    d->_k_slotSettingsChanged(KGlobalSettings::SETTINGS_MOUSE);
    connect( KGlobalSettings::self(), SIGNAL( settingsChanged(int) ), SLOT( _k_slotSettingsChanged(int) ) );

    d->m_pAutoSelect = new QTimer( this );
    connect( d->m_pAutoSelect, SIGNAL( timeout() ),
             this, SLOT( _k_slotAutoSelect() ) );
}

KListWidget::~KListWidget()
{
    delete d;
}

void KListWidget::KListWidgetPrivate::_k_slotItemEntered( QListWidgetItem *item )
{
    if ( item && m_bChangeCursorOverItem && m_bUseSingle )
        q->viewport()->setCursor( QCursor( Qt::OpenHandCursor ) );

    if ( item && (m_autoSelectDelay > -1) && m_bUseSingle ) {
      m_pAutoSelect->setSingleShot( true );
      m_pAutoSelect->start( m_autoSelectDelay );
      m_pCurrentItem = item;
    }
}

void KListWidget::KListWidgetPrivate::_k_slotOnViewport()
{
    if ( m_bChangeCursorOverItem )
        q->viewport()->unsetCursor();

    m_pAutoSelect->stop();
    m_pCurrentItem = 0;
}


void KListWidget::KListWidgetPrivate::_k_slotSettingsChanged(int category)
{
    if (category != KGlobalSettings::SETTINGS_MOUSE)
        return;
    m_bUseSingle = KGlobalSettings::singleClick();

    q->disconnect(q, SIGNAL(itemClicked( QListWidgetItem *)));
    q->disconnect(q, SIGNAL(itemDoubleClicked( QListWidgetItem *)));

    if( m_bUseSingle )
    {
        q->connect(q, SIGNAL(itemClicked(QListWidgetItem *)),
                   SIGNAL(executed(QListWidgetItem *)));
    }
    else
    {
        q->connect(q, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
                   SIGNAL(executed(QListWidgetItem *)));
    }

    m_bChangeCursorOverItem = KGlobalSettings::changeCursorOverIcon();
    m_autoSelectDelay = KGlobalSettings::autoSelectDelay();

    if( !m_bUseSingle || !m_bChangeCursorOverItem )
        q->viewport()->unsetCursor();
}

void KListWidget::KListWidgetPrivate::_k_slotAutoSelect()
{
  // check that the item still exists
  if( q->row( m_pCurrentItem ) == -1 )
    return;

  //Give this widget the keyboard focus.
  if( !q->hasFocus() )
    q->setFocus();

  Qt::KeyboardModifiers keybstate = QApplication::keyboardModifiers();

  QListWidgetItem* previousItem = q->currentItem();
  q->setCurrentItem( m_pCurrentItem );

  if( m_pCurrentItem ) {
    //Shift pressed?
    if( (keybstate & Qt::ShiftModifier) ) {
      bool block = q->signalsBlocked();
      q->blockSignals( true );

      //No Ctrl? Then clear before!
      if( !(keybstate & Qt::ControlModifier) )
            q->clearSelection();

      bool select = !m_pCurrentItem->isSelected();
      bool update = q->viewport()->updatesEnabled();
      q->viewport()->setUpdatesEnabled( false );

      bool down = q->row( previousItem ) < q->row( m_pCurrentItem );
      QListWidgetItem* it = down ? previousItem : m_pCurrentItem;

      for (int i = q->row(it) ; i < q->count() ; i++ ) {
        if ( down && q->item(i) == m_pCurrentItem ) {
            m_pCurrentItem->setSelected(select);
            break;
        }

        if ( !down && q->item(i) == previousItem ) {
            previousItem->setSelected(select);
            break;
        }
        it->setSelected(select);
    }

      q->blockSignals( block );
      q->viewport()->setUpdatesEnabled( update );

      emit q->itemSelectionChanged();

      if( q->selectionMode() == QAbstractItemView::SingleSelection )
        q->emit itemSelectionChanged();
    }
    else if( (keybstate & Qt::ControlModifier) )
      m_pCurrentItem->setSelected(!m_pCurrentItem->isSelected());
    else {
      bool block = q->signalsBlocked();
      q->blockSignals( true );

      if( !m_pCurrentItem->isSelected() )
        q->clearSelection();

      q->blockSignals( block );

      m_pCurrentItem->setSelected(true);
    }
  }
  else
    kDebug() << "That's not supposed to happen!!!!";
}

void KListWidget::KListWidgetPrivate::emitExecute( QListWidgetItem *item, const QPoint &pos )
{
  Qt::KeyboardModifiers keybstate = QApplication::keyboardModifiers();

  m_pAutoSelect->stop();

  //Don't emit executed if in SC mode and Shift or Ctrl are pressed
  if( !( m_bUseSingle && ((keybstate & Qt::ShiftModifier) || (keybstate & Qt::ControlModifier)) ) ) {
    emit q->executed( item );
    emit q->executed( item, pos );
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
  d->m_pAutoSelect->stop();

  QListWidget::focusOutEvent( fe );
}

void KListWidget::leaveEvent( QEvent *e )
{
  d->m_pAutoSelect->stop();

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

    if( (e->button() == Qt::LeftButton) && !d->m_bUseSingle )
      d->emitExecute( item, e->globalPos() );
  }
}

#include "klistwidget.moc"
