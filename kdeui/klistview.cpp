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
#include <klineedit.h>

#include "klistviewlineedit.h"
#include "klistview.h"

#include <X11/Xlib.h>

class KListView::KListViewPrivate
{
public:
  QRect *invalidateRect;
  QPoint *pressPos;
  QPoint *startDragPos;
  KListViewLineEdit *editor;
  QValueList<int> renameable;

  bool itemsMovable;
  bool itemsRenameable;
  bool dragEnabled;
  bool autoOpen;
  bool dropVisualizer;
};

class QListViewItemHack
{
public:
	QListViewItemHack();
	void moveItem(QListViewItemHack *after, bool child);

private:
    int ownHeight;
    int maybeTotalHeight;
    int nChildren;

    uint lsc: 14;
    uint lso: 1;
    uint open : 1;
    uint selected : 1;
    uint selectable: 1;
    uint configured: 1;
    uint expandable: 1;
    uint is_root: 1;

    QListViewItem * parentItem;
    QListViewItem * siblingItem;
    QListViewItem * childItem;

    void * columns;
};

void QListViewItemHack::moveItem(QListViewItemHack */*after*/, bool /*child*/)
{

}

KListViewLineEdit::KListViewLineEdit(KListView *parent)
	: KLineEdit(parent->viewport()), item(0), col(0), p(parent)
{
	hide();
}

KListViewLineEdit::~KListViewLineEdit()
{
}

void KListViewLineEdit::load(QListViewItem *i, int c)
{
	item=i;
	col=c;
	
	QRect rect(p->itemRect(i));
	setText(item->text(c));
	int fieldX = - p->contentsX();
	
	for(int i = 0;i< c;i++)
		fieldX += p->columnWidth(i);
	
	setGeometry(rect.x(), rect.y(), p->columnWidth(c)+2, rect.height() + 2);
	show();
	setFocus();
	grabMouse();

}

void KListViewLineEdit::keyPressEvent(QKeyEvent *e)
{
	QLineEdit::keyPressEvent(e);
	
	if(e->key() == Qt::Key_Return)
		terminate();
}

void KListViewLineEdit::terminate()
{
	item->setText(col, text());
	hide();
	int c=col;
	QListViewItem *i=item;
	col=0;
	item=0;
	releaseMouse();
	emit done(i,c);
}

void KListViewLineEdit::focusOutEvent(QFocusEvent *)
{
	terminate();
}

KListView::KListView( QWidget *parent, const char *name )
    : QListView( parent, name )
{
	{
		d=new KListViewPrivate;
		d->invalidateRect=0;
		d->pressPos=0;
		d->startDragPos=0;
		d->editor=new KListViewLineEdit(this);
		d->itemsMovable=true;
		d->itemsRenameable=false;
		d->dragEnabled=false;
		d->autoOpen=true;
		connect(d->editor, SIGNAL(done(QListViewItem*,int)), this, SLOT(doneEditing(QListViewItem*,int)));
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
    delete d->startDragPos;
    delete d->pressPos;
    delete d->invalidateRect;
    delete d->editor;
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

// If the row was already selected, create an editor widget.
	QListViewItem *at=itemAt(p);
	if (at && at->isSelected())
		rename(at, 0); // TODO

  QListView::contentsMousePressEvent( e );
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
				// do something
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
    QListViewItem *afterme=findDrop(event->pos());

    QRect *rectTemp=new QRect(drawDropVisualizer(0,0, afterme));

    if (d->invalidateRect && (*d->invalidateRect)!=*rectTemp)
    {
        cleanRect();
        QPainter painter(viewport());
        drawDropVisualizer(&painter,0, afterme);
    }

    delete d->invalidateRect;
    d->invalidateRect=rectTemp;


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
	delete d->invalidateRect;
	d->invalidateRect=0;
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

void KListView::setItemsMovable(bool b)
{
	d->itemsMovable=b;
}

bool KListView::itemsMovable() const
{
	return d->itemsMovable;
}

void KListView::setItemsRenameable(bool b)
{
	d->itemsRenameable=b;
}

bool KListView::itemsRenameable() const
{
	return d->itemsRenameable;
}


void KListView::setDragEnabled(bool b)
{
	d->dragEnabled=b;
}

bool KListView::dragEnabled() const
{
	return d->dragEnabled;
}

void KListView::setAutoOpen(bool b)
{
	d->autoOpen=b;
}

bool KListView::autoOpen() const
{
	return d->autoOpen;
}

bool KListView::dropVisualizer() const
{
	return d->dropVisualizer;
}

void KListView::setDropVisualizer(bool b)
{
	d->dropVisualizer=b;
}

QList<QListViewItem> KListView::selectedItems() const
{
	QList<QListViewItem> list;
	for (QListViewItem *i=firstChild(); i!=0; i=i->itemBelow())
		if (i->isSelected()) list.append(i);
	return list;
}

void KListView::moveItem(QListViewItem *item, QListViewItem *after, bool child)
{
// unimplemented
	QListViewItemHack *hi=(QListViewItemHack*)item;
	hi->moveItem((QListViewItemHack*)after,child);
}

void KListView::dragEnterEvent(QDragEnterEvent *event)
{
	QListView::dragEnterEvent(event);
	event->accept(event->source()==viewport());
}

QRect KListView::drawDropVisualizer(QPainter *painter, int depth, QListViewItem *after)
{
	QRect rect(depth*treeStepSize(), itemRect(after).bottom(), width(), 2);

	if (painter)
	{	
		QColor barcolor(foregroundColor());	
		painter->setPen(barcolor);
		painter->drawRect(rect);
	}
	return rect;
}

void KListView::rename(QListViewItem *item, int c)
{
	d->editor->load(item,c);

}

bool KListView::getRenameableRow(int row) const
{
	return (bool)d->renameable.contains(row);
}

void KListView::setRenameableRow(int row, bool yesno)
{
	if (row>=header()->count()) return;

	d->renameable.remove(row);
	if (yesno && d->renameable.find(row)==d->renameable.end())
		d->renameable+=row;
	else if (!yesno && d->renameable.find(row)!=d->renameable.end())
		d->renameable.remove(row);
}

void KListView::doneEditing(QListViewItem *item, int row)
{
	emit itemRenamed(item, item->text(row), row);
	emit itemRenamed(item);
}



#include "klistviewlineedit.moc"
