/* This file is part of the KDE libraries
   Copyright (C) 2000 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2000 Charles Samuels <charles@kde.org>

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

#include <qdragobject.h>
#include <qtimer.h>
#include <qheader.h>

#include <kglobalsettings.h>
#include <kcursor.h>
#include <kapp.h>
#include <kipc.h>
#include <kdebug.h>

#include "klistview.h"

#include <X11/Xlib.h>

KListView::KListView( QWidget *parent, const char *name )
    : QListView( parent, name )
{
	{
		d=new KListViewPrivate;
		d->invalidateRect=0;
		d->pressPos=0;
		d->startDragPos=0;
	}

	setAcceptDrops(true);
    setDragAutoScroll(true);
    oldCursor = viewport()->cursor();
    connect( this, SIGNAL( onViewport() ),
	     this, SLOT( slotOnViewport() ) );
    connect( this, SIGNAL( onItem( QListViewItem * ) ),
	     this, SLOT( slotOnItem( QListViewItem * ) ) );

    slotSettingsChanged(KApplication::SETTINGS_MOUSE);
    connect( kapp, SIGNAL( settingsChanged(int) ), SLOT( slotSettingsChanged(int) ) );
    kapp->addKipcEventMask( KIPC::SettingsChanged );

    m_pCurrentItem = 0L;

    m_pAutoSelect = new QTimer( this );
    connect( m_pAutoSelect, SIGNAL( timeout() ),
    	     this, SLOT( slotAutoSelect() ) );
}

KListView::~KListView()
{
    delete d->invalidateRect;
}

bool KListView::isExecuteArea( const QPoint& point )
{
   if ( itemAt( point ) )
   {
     if( allColumnsShowFocus() )
       return true;
     else {

      int x = point.x();
      int pos = header()->mapToActual( 0 );
      int offset = 0;
      int width = columnWidth( pos );

      for ( int index = 0; index < pos; index++ )
         offset += columnWidth( index );

      return ( x > offset && x < ( offset + width ) );
     }
   }
   return false;
}

void KListView::slotOnItem( QListViewItem *item )
{
  if ( item && (m_autoSelectDelay > -1) && m_bUseSingle ) {
    m_pAutoSelect->start( m_autoSelectDelay, true );
    m_pCurrentItem = item;
  }
}

void KListView::slotOnViewport()
{
  if ( m_bChangeCursorOverItem )
    viewport()->setCursor( oldCursor );

  m_pAutoSelect->stop();
  m_pCurrentItem = 0L;
}

void KListView::slotSettingsChanged(int category)
{
    if (category != KApplication::SETTINGS_MOUSE)
        return;
    m_bUseSingle = KGlobalSettings::singleClick();

    disconnect( this, SIGNAL( mouseButtonClicked( int, QListViewItem *,
						  const QPoint &, int ) ),
		this, SLOT( slotMouseButtonClicked( int, QListViewItem *,
						    const QPoint &, int ) ) );
//       disconnect( this, SIGNAL( doubleClicked( QListViewItem *,
// 					       const QPoint &, int ) ),
// 		  this, SLOT( slotExecute( QListViewItem *,
// 					   const QPoint &, int ) ) );

    if( m_bUseSingle )
    {
      connect( this, SIGNAL( mouseButtonClicked( int, QListViewItem *,
						 const QPoint &, int ) ),
	       this, SLOT( slotMouseButtonClicked( int, QListViewItem *,
						   const QPoint &, int ) ) );
    }
    else
    {
//       connect( this, SIGNAL( doubleClicked( QListViewItem *,
// 					    const QPoint &, int ) ),
// 	       this, SLOT( slotExecute( QListViewItem *,
// 					const QPoint &, int ) ) );
    }

    m_bChangeCursorOverItem = KGlobalSettings::changeCursorOverIcon();
    m_autoSelectDelay = KGlobalSettings::autoSelectDelay();

    if( !m_bUseSingle || !m_bChangeCursorOverItem )
	viewport()->setCursor( oldCursor );
}

void KListView::slotAutoSelect()
{
  //Give this widget the keyboard focus.
  if( !hasFocus() )
    setFocus();

  Window root;
  Window child;
  int root_x, root_y, win_x, win_y;
  uint keybstate;
  XQueryPointer( qt_xdisplay(), qt_xrootwin(), &root, &child,
		 &root_x, &root_y, &win_x, &win_y, &keybstate );

  QListViewItem* previousItem = currentItem();
  setCurrentItem( m_pCurrentItem );

  if( m_pCurrentItem ) {
    //Shift pressed?
    if( (keybstate & ShiftMask) ) {
      bool block = signalsBlocked();
      blockSignals( true );

      //No Ctrl? Then clear before!
      if( !(keybstate & ControlMask) )
	clearSelection();

      bool select = !m_pCurrentItem->isSelected();
      bool update = viewport()->isUpdatesEnabled();
      viewport()->setUpdatesEnabled( false );

      bool down = previousItem->itemPos() < m_pCurrentItem->itemPos();
      QListViewItemIterator lit( down ? previousItem : m_pCurrentItem );
      for ( ; lit.current(); ++lit ) {
	if ( down && lit.current() == m_pCurrentItem ) {
	  m_pCurrentItem->setSelected( select );
	  break;
	}
	if ( !down && lit.current() == previousItem ) {
	  previousItem->setSelected( select );
	  break;
	}
	lit.current()->setSelected( select );
      }

      blockSignals( block );
      viewport()->setUpdatesEnabled( update );
      triggerUpdate();

      emit selectionChanged();

      if( selectionMode() == QListView::Single )
	emit selectionChanged( m_pCurrentItem );
    }
    else if( (keybstate & ControlMask) )
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

void KListView::emitExecute( QListViewItem *item, const QPoint &pos, int c )
{
  if( isExecuteArea( viewport()->mapFromGlobal(pos) ) ) {

    Window root;
    Window child;
    int root_x, root_y, win_x, win_y;
    uint keybstate;
    XQueryPointer( qt_xdisplay(), qt_xrootwin(), &root, &child,
		   &root_x, &root_y, &win_x, &win_y, &keybstate );

    m_pAutoSelect->stop();

    //Don´t emit executed if in SC mode and Shift or Ctrl are pressed
    if( !( m_bUseSingle && ((keybstate & ShiftMask) || (keybstate & ControlMask)) ) ) {
      emit executed( item );
      emit executed( item, pos, c );
    }
  }
}

void KListView::focusOutEvent( QFocusEvent *fe )
{
  m_pAutoSelect->stop();

  QListView::focusOutEvent( fe );
}

void KListView::leaveEvent( QEvent *e )
{
  m_pAutoSelect->stop();

  QListView::leaveEvent( e );
}

void KListView::contentsMousePressEvent( QMouseEvent *e )
{
  if( (selectionMode() == Extended) && (e->state() & ShiftButton) && !(e->state() & ControlButton) ) {
    bool block = signalsBlocked();
    blockSignals( true );

    clearSelection();

    blockSignals( block );
  }

  QListView::contentsMousePressEvent( e );

  QPoint p( contentsToViewport( e->pos() ) );
  QListViewItem *i = itemAt( p );
  if ( i )
  {
    // if the user clicked into the root decoration of the item, don't try to start a drag!
    if (p.x() > header()->cellPos( header()->mapToIndex( 0 ) ) +
        treeStepSize() * ( i->depth() + ( rootIsDecorated() ? 1 : 0) ) + itemMargin() ||
        p.x() < header()->cellPos( header()->mapToIndex( 0 ) ) )
    {
      delete d->pressPos;
      d->pressPos= new QPoint(p);
    }
  }
}

void KListView::contentsMouseMoveEvent( QMouseEvent *e )
{
  QPoint vp = contentsToViewport(e->pos());
  QListViewItem *item = itemAt( vp );

  //do we process cursor changes at all?
  if ( item && m_bChangeCursorOverItem && m_bUseSingle ) {
    //Cursor moved on a new item or in/out the execute area
    if( (item != m_pCurrentItem) ||
	(isExecuteArea(vp) != m_cursorInExecuteArea) ) {

      m_cursorInExecuteArea = isExecuteArea(vp);

      if( m_cursorInExecuteArea ) //cursor moved in execute area
	viewport()->setCursor( KCursor().handCursor() );
      else //cursor moved out of execute area
	viewport()->setCursor( oldCursor );
    }
  }

	QListView::contentsMouseMoveEvent( e );

	// I have just started to move my mouse..
	if ((e->state() == LeftButton) && !d->startDragPos)
	{
		d->startDragPos=new QPoint(e->pos());
	}
	else // Now, I may begin dragging!
		if (d->startDragPos && (e->state() == LeftButton) && dragEnabled())
	{
		// Have we moved the mouse far enough?
		if ((*d->startDragPos-e->pos()).manhattanLength() > QApplication::startDragDistance())
		{
            delete d->startDragPos;
			d->startDragPos= new QPoint(e->pos());
		
			startDrag();
		}
	}
}

void KListView::contentsMouseDoubleClickEvent ( QMouseEvent *e )
{
  QListView::contentsMouseDoubleClickEvent( e );

  QPoint vp = contentsToViewport(e->pos());
  QListViewItem *item = itemAt( vp );
  int col = item ? header()->mapToLogical( header()->cellAt( vp.x() ) ) : -1;

  if( item ) {
    emit doubleClicked( item, e->globalPos(), col );

    if( (e->button() == LeftButton) && !m_bUseSingle )
      emitExecute( item, e->globalPos(), col );
  }
}

void KListView::slotMouseButtonClicked( int btn, QListViewItem *item, const QPoint &pos, int c )
{
  if( (btn == LeftButton) && item )
    emitExecute( item, pos, c );
}

void KListView::dropEvent(QDropEvent* event)
{
    QListView::dropEvent(event);
    cleanRect();
    QListViewItem *afterme=findDrop(event->pos());
	
    if (event->source()==viewport()) // Moving an item
	{
		if (dragEnabled() && itemsMovable())
		{
			for (QListViewItem *i=firstChild(); i!=0; i=i->itemBelow())
			{
				if (!i->isSelected())
					continue;
				moveItem(i, afterme);
				afterme=i;
			}		
		}	
	}
    else
		dropEvent(event, this, afterme);
}

void KListView::dropEvent(QDropEvent *event, QListView *parent, QListViewItem *after)
{
    QString text;
    if (!QTextDrag::decode(event, text)) return;

    after=new QListViewItem(parent, after,text);
}

void KListView::dragMoveEvent(QDragMoveEvent *event)
{
    QListView::dragMoveEvent(event);
    if (!event->isAccepted()) return;

    //Clean up the view
    cleanRect();
	
    QListViewItem *afterme=findDrop(event->pos());
	
    d->invalidateRect=new QRect(0, itemRect(afterme).bottom(),
	                            width(), 2);
	
    repaintContents(*d->invalidateRect);
}

void KListView::dragLeaveEvent(QDragLeaveEvent *event)
{
    QListView::dragLeaveEvent(event);
    cleanRect();
}

void KListView::cleanRect()
{
    if (!d->invalidateRect) return;
    QRect *temp=d->invalidateRect;
    d->invalidateRect=0;
	
    viewport()->update(*temp);
	
    delete temp;
}

void KListView::viewportPaintEvent(QPaintEvent *event)
{
    QListView::viewportPaintEvent(event);
    QColor barcolor(foregroundColor());	
	
    if (d->invalidateRect)
	{
	    QPainter paint(viewport());
	    paint.setPen(barcolor);

	    paint.drawRect(*d->invalidateRect);
	}
}

QListViewItem* KListView::findDrop(const QPoint &_p)
{
	
	QPoint p(_p);
	// Move the point if the header is shown
	if (header()->isVisible())
		p.setY(p.y()-header()->height());
	
    // Get the position to put it in
    QListViewItem *afterme=0;
    QListViewItem *atpos(itemAt(p));
	
    if (!atpos) // put it at the end
		afterme=lastItem();
    else
	{ // get the one closer to me..
	  // That is, the space between two listviewitems
	  // Since this aims to be user-friendly :)
		int dropY=mapFromGlobal(p).y();
		int itemHeight=atpos->height();
		int topY=mapFromGlobal(itemRect(atpos).topLeft()).y();
		
		if ((dropY-topY)<itemHeight/2)
			afterme=atpos->itemAbove();	
		else
			afterme=atpos;
	}

    return afterme;
}


void KListView::contentsMouseReleaseEvent( QMouseEvent *e )
{
    delete d->pressPos;
    d->pressPos=0;
    QListView::contentsMouseReleaseEvent( e );
}

QListViewItem *KListView::lastItem() const
{
    QListViewItem *lastchild=firstChild();
    if (lastchild)
	for (;lastchild->nextSibling()!=0; lastchild=lastchild->nextSibling());
    return lastchild;
}

void KListView::startDrag()
{
	QDragObject *drag = dragObject();
	if ( !drag )
		return;

	if (drag->drag())
		if ( drag->target() != viewport() )
			emit moved();
}

QDragObject *KListView::dragObject() const
{
	if (!currentItem())
		return 0;
	
	return new QStoredDrag("application/x-qlistviewitem", viewport());
}

void KListView::setItemsMovable(bool)
{

}

bool KListView::itemsMovable() const
{
	return true;
}

void KListView::setItemsRenameable(bool)
{

}

bool KListView::itemsRenameable() const
{
	return true;
}


void KListView::setDragEnabled(bool)
{

}

bool KListView::dragEnabled() const
{
	return true;
}

QList<QListViewItem> KListView::selectedItems() const
{
	QList<QListViewItem> list;
	for (QListViewItem *i=firstChild(); i!=0; i=i->itemBelow())
		if (i->isSelected()) list.append(i);
	return list;
}

void KListView::moveItem(QListViewItem *item, QListViewItem *after)
{
// unimplemented
/*
<Don Sanders>
Alternatively you could use takeItem on all items beyond the 'after' item insert
the new item and then insert all the items you just took (uugh).
*/
}

void KListView::dragEnterEvent(QDragEnterEvent *event)
{
	QListView::dragEnterEvent(event);
	event->accept(event->source()==viewport());
}

