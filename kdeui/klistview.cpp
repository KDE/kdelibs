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
#include <qcursor.h>
#include <qtooltip.h>

#include <kglobalsettings.h>
#include <kcursor.h>
#include <kapp.h>
#include <kipc.h>
#include <kdebug.h>

#define private public
#include <qlistview.h>
#undef private

#include "klistview.h"
#include "klistviewlineedit.h"

#include <X11/Xlib.h>

class KListView::Tooltip : public QToolTip
{
public:
  Tooltip (KListView* parent, QToolTipGroup* group = 0L);
  virtual ~Tooltip () {}

protected:
  /**
   * Reimplemented from QToolTip for internal reasons.
   */
  virtual void maybeTip (const QPoint&);

private:
  KListView* mParent;
};

KListView::Tooltip::Tooltip (KListView* parent, QToolTipGroup* group)
  : QToolTip (parent, group),
	mParent (parent)
{
}

void KListView::Tooltip::maybeTip (const QPoint&)
{
  // FIXME
}

class KListView::KListViewPrivate
{
public:
  bool bUseSingle;
  bool bChangeCursorOverItem;

  QListViewItem* pCurrentItem;
  bool cursorInExecuteArea;

  QTimer autoSelect;
  int autoSelectDelay;

  QCursor oldCursor;

  QPoint pressPos;
  QPoint startDragPos;
  KListViewLineEdit *editor;
  QValueList<int> renameable;

  bool itemsMovable;
  bool itemsRenameable;
  bool dragEnabled;
  bool autoOpen;
  bool dropVisualizer;
  bool dropHighlighter;
  bool createChildren;

  int tooltipColumn;

  SelectionModeExt selectionMode;
  int contextMenuKey;
  bool showContextMenusOnPress;

  bool wasShiftEvent;
};


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
  : QListView( parent, name ),
	d (new KListViewPrivate()),
	mDropVisualizerWidth (4)
{
  {
	d->editor=new KListViewLineEdit(this);
	d->itemsMovable=true;
	d->itemsRenameable=false;
	d->dragEnabled=false;
	d->autoOpen=true;
	d->tooltipColumn=0;
	d->dropVisualizer=true;
	d->dropHighlighter=true;
	d->createChildren=true;
	d->selectionMode = Single;
	d->contextMenuKey = KGlobalSettings::contextMenuKey ();
	d->showContextMenusOnPress = KGlobalSettings::showContextMenusOnPress ();
	
	connect(d->editor, SIGNAL(done(QListViewItem*,int)), this, SLOT(doneEditing(QListViewItem*,int)));
  }

//   setAcceptDrops(true);
  setDragAutoScroll(true);
  d->oldCursor = viewport()->cursor();
  connect( this, SIGNAL( onViewport() ),
		   this, SLOT( slotOnViewport() ) );
  connect( this, SIGNAL( onItem( QListViewItem * ) ),
		   this, SLOT( slotOnItem( QListViewItem * ) ) );

  connect (this, SIGNAL(contentsMoving(int,int)),
		   this, SLOT(cleanDropVisualizer()));  

  slotSettingsChanged(KApplication::SETTINGS_MOUSE);
  connect( kapp, SIGNAL( settingsChanged(int) ), SLOT( slotSettingsChanged(int) ) );
  kapp->addKipcEventMask( KIPC::SettingsChanged );
  
  d->pCurrentItem = 0L;

  connect(&d->autoSelect, SIGNAL( timeout() ),
		  this, SLOT( slotAutoSelect() ) );

  // context menu handling
  if (d->showContextMenusOnPress)
	{
	  kdDebug() << "connectiong to pressed" << endl;
	  connect (this, SIGNAL (rightButtonPressed (QListViewItem*, const QPoint&, int)),
			   this, SLOT (emitContextMenu (QListViewItem*, const QPoint&, int)));
	}
  else
	{
	  kdDebug () << "connecting to clicked" <<endl;
	  connect (this, SIGNAL (rightButtonClicked (QListViewItem*, const QPoint&, int)),
			   this, SLOT (emitContextMenu (QListViewItem*, const QPoint&, int)));
	}

  connect (this, SIGNAL (menuShortCutPressed (KListView*, QListViewItem*)),
		   this, SLOT (emitContextMenu (KListView*, QListViewItem*)));
}



KListView::~KListView()
{
  delete d->editor;
  delete d;
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
  if ( item && (d->autoSelectDelay > -1) && d->bUseSingle ) {
    d->autoSelect.start( d->autoSelectDelay, true );
    d->pCurrentItem = item;
  }
}

void KListView::slotOnViewport()
{
  if ( d->bChangeCursorOverItem )
    viewport()->setCursor( d->oldCursor );

  d->autoSelect.stop();
  d->pCurrentItem = 0L;
}

void KListView::slotSettingsChanged(int category)
{
//   if (category != KApplication::SETTINGS_MOUSE && category != KApplication::SETTINGS_POPUPMENU)
// 	return;
  switch (category)
	{
	case KApplication::SETTINGS_MOUSE:
	  d->bUseSingle = KGlobalSettings::singleClick();
	  
	  disconnect (this, SIGNAL (mouseButtonClicked (int, QListViewItem*, const QPoint &, int)),
				  this, SLOT (slotMouseButtonClicked (int, QListViewItem*, const QPoint &, int)));
	  
	  //       disconnect( this, SIGNAL( doubleClicked( QListViewItem *,
	  // 					       const QPoint &, int ) ),
	  // 		  this, SLOT( slotExecute( QListViewItem *,
	  // 					   const QPoint &, int ) ) );
	  
	  if( d->bUseSingle )
		{
		  connect (this, SIGNAL (mouseButtonClicked (int, QListViewItem*, const QPoint &, int)),
				   this, SLOT (slotMouseButtonClicked( int, QListViewItem*, const QPoint &, int)));
		}
	  else
		{
		  //       connect( this, SIGNAL( doubleClicked( QListViewItem *,
		  // 					    const QPoint &, int ) ),
		  // 	       this, SLOT( slotExecute( QListViewItem *,
		  // 					const QPoint &, int ) ) );
		}
	  
	  d->bChangeCursorOverItem = KGlobalSettings::changeCursorOverIcon();
	  d->autoSelectDelay = KGlobalSettings::autoSelectDelay();
	  
	  if( !d->bUseSingle || !d->bChangeCursorOverItem )
		viewport()->setCursor( d->oldCursor );
	  
	  break;
	  
	case KApplication::SETTINGS_POPUPMENU:
	  // context menu settings
	  d->contextMenuKey = KGlobalSettings::contextMenuKey ();
	  d->showContextMenusOnPress = KGlobalSettings::showContextMenusOnPress ();
	  
	  if (d->showContextMenusOnPress)
		{
		  kdDebug () << "reconnecting to pressed" <<endl;
		  
		  disconnect (0L, 0L, this, SLOT (emitContextMenu (QListViewItem*, const QPoint&, int)));
		  
		  connect (this, SIGNAL (rightButtonPressed (QListViewItem*, const QPoint&, int)),
			   this, SLOT (emitContextMenu (QListViewItem*, const QPoint&, int)));
		}
	  else
		{
		  
		  kdDebug () << "reconnecting to clicked" <<endl;
		  disconnect (0L, 0L, this, SLOT (emitContextMenu (QListViewItem*, const QPoint&, int)));
		  
		  connect (this, SIGNAL (rightButtonClicked (QListViewItem*, const QPoint&, int)),
				   this, SLOT (emitContextMenu (QListViewItem*, const QPoint&, int)));
		}
	  break;

	default:
	  break;
	}
}

void KListView::slotAutoSelect()
{
  if (!isActiveWindow())
	{
	  d->autoSelect.stop();
	  return;
	}

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
  setCurrentItem( d->pCurrentItem );

  if( d->pCurrentItem ) {
    //Shift pressed?
    if( (keybstate & ShiftMask) ) {
      bool block = signalsBlocked();
      blockSignals( true );

      //No Ctrl? Then clear before!
      if( !(keybstate & ControlMask) )
		clearSelection();

      bool select = !d->pCurrentItem->isSelected();
      bool update = viewport()->isUpdatesEnabled();
      viewport()->setUpdatesEnabled( false );

      bool down = previousItem->itemPos() < d->pCurrentItem->itemPos();
      QListViewItemIterator lit( down ? previousItem : d->pCurrentItem );
      for ( ; lit.current(); ++lit ) {
		if ( down && lit.current() == d->pCurrentItem ) {
		  d->pCurrentItem->setSelected( select );
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
		emit selectionChanged( d->pCurrentItem );
    }
    else if( (keybstate & ControlMask) )
      setSelected( d->pCurrentItem, !d->pCurrentItem->isSelected() );
    else {
      bool block = signalsBlocked();
      blockSignals( true );

      if( !d->pCurrentItem->isSelected() )
		clearSelection();

      blockSignals( block );

      setSelected( d->pCurrentItem, true );
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

    d->autoSelect.stop();

    //Don´t emit executed if in SC mode and Shift or Ctrl are pressed
    if( !( d->bUseSingle && ((keybstate & ShiftMask) || (keybstate & ControlMask)) ) ) {
      emit executed( item );
      emit executed( item, pos, c );
    }
  }
}

void KListView::focusOutEvent( QFocusEvent *fe )
{
  cleanDropVisualizer();

  d->autoSelect.stop();
  
  QListView::focusOutEvent( fe );
}

void KListView::leaveEvent( QEvent *e )
{
  d->autoSelect.stop();

  QListView::leaveEvent( e );
}

void KListView::contentsMousePressEvent( QMouseEvent *e )
{
  if( (selectionModeExt() == Extended) && (e->state() & ShiftButton) && !(e->state() & ControlButton) ) {
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
      d->pressPos = p;
    }
  }

// If the row was already selected, create an editor widget.
	QListViewItem *at=itemAt(p);
	if (at && at->isSelected() && itemsRenameable())
		rename(at, 0); // TODO

  QListView::contentsMousePressEvent( e );
}

void KListView::contentsMouseMoveEvent( QMouseEvent *e )
{
  QPoint vp = contentsToViewport(e->pos());
  QListViewItem *item = itemAt( vp );

  //do we process cursor changes at all?
  if ( item && d->bChangeCursorOverItem && d->bUseSingle ) {
    //Cursor moved on a new item or in/out the execute area
    if( (item != d->pCurrentItem) ||
		(isExecuteArea(vp) != d->cursorInExecuteArea) ) {

      d->cursorInExecuteArea = isExecuteArea(vp);

      if( d->cursorInExecuteArea ) //cursor moved in execute area
		viewport()->setCursor( KCursor().handCursor() );
      else //cursor moved out of execute area
		viewport()->setCursor( d->oldCursor );
    }
  }

  QListView::contentsMouseMoveEvent( e );

  // I have just started to move my mouse..
  if ((e->state() == LeftButton) && d->startDragPos.isNull())
	{
	  d->startDragPos = e->pos();
	}
  else // Now, I may begin dragging!
	if ((e->state() == LeftButton) && dragEnabled())
	  {
		// Have we moved the mouse far enough?
		if ((d->startDragPos - e->pos()).manhattanLength() > QApplication::startDragDistance())
		  {
			d->startDragPos = e->pos();
		
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

    if( (e->button() == LeftButton) && !d->bUseSingle )
      emitExecute( item, e->globalPos(), col );
  }
}

void KListView::slotMouseButtonClicked( int btn, QListViewItem *item, const QPoint &pos, int c )
{
  if( (btn == LeftButton) && item )
    emitExecute( item, pos, c );
}

void KListView::contentsDropEvent (QDropEvent* e)
{
  cleanDropVisualizer ();

  if (acceptDrag (e))
	{
	  QListViewItem *afterme;
	  QListViewItem *parent;
	  findDrop(e->pos(), parent, afterme);

	  emit dropped (e, afterme);
	}
}

void KListView::contentsDragMoveEvent(QDragMoveEvent *event)
{
  if (acceptDrops() && acceptDrag (event))
	{
	  //Clean up the view
	  QListViewItem *afterme;
	  QListViewItem *parent;
	  findDrop(event->pos(), parent, afterme);
	  
	  if (dropVisualizer())
		{
		  QRect tmpRect = drawDropVisualizer (0, parent, afterme);

		  if (tmpRect != mOldDropVisualizer)
			{
			  cleanDropVisualizer();

			  QPainter painter(viewport());
			  mOldDropVisualizer = drawDropVisualizer(&painter,parent, afterme);
			}
		}
	}
  else
	{
	  event->ignore();
	}
}

void KListView::contentsDragLeaveEvent (QDragLeaveEvent*)
{
    cleanDropVisualizer();
}

void KListView::cleanDropVisualizer()
{
    if (mOldDropVisualizer.isValid())
	  {
		viewport()->repaint (mOldDropVisualizer, true);
		mOldDropVisualizer = QRect();
	  }
}

void KListView::findDrop(const QPoint &pos, QListViewItem *&parent, QListViewItem *&after)
{ 
  after = itemAt (contentsToViewport(pos));
  
  if (after)
	{
      if (!(pos.x() > header()->cellPos( header()->mapToActual( 0 ) ) +
			treeStepSize() * (after->depth() + ( rootIsDecorated() ? 1 : 0) ) + itemMargin() ||
			pos.x() < header()->cellPos (header()->mapToActual (0))))
		{
		  after = 0L;
		}
  
	  if (after && !below (after, pos))
		after = after->itemAbove();
	}
  else
	{
	  after = lastItem();
	}

  if (after)
	parent = after->parent();
}


void KListView::contentsMouseReleaseEvent( QMouseEvent *e )
{
  d->pressPos = QPoint ();
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


void KListView::moveItem(QListViewItem *item, QListViewItem *parent, QListViewItem *after)
{
	// Basically reimplementing the QListViewItem(QListViewItem*, QListViewItem*) constructor
	// in here, without ever deleting the item.
	if (item->parent())
		item->parent()->takeItem(item);
	else
		takeItem(item);
		
	if (parent)
		parent->insertItem(item);
	else
		insertItem(item);
	
	if (after)
		item->moveToJustAfter(after);
}

void KListView::contentsDragEnterEvent(QDragEnterEvent *event)
{
  acceptDrag (event); // acceptDrag should call "accept"
}

void KListView::setDropVisualizerWidth (int w)
{
  mDropVisualizerWidth = w > 0 ? w : 1;
}

QRect KListView::drawDropVisualizer(QPainter *p, QListViewItem */*parent*/,
								   QListViewItem *after)
{
  QRect insertmarker;

  if (after)
	{
	  insertmarker = itemRect (after);

	  insertmarker.setLeft (0);
	  insertmarker.setRight (viewport()->width());
	  insertmarker.setTop (insertmarker.bottom() - mDropVisualizerWidth/2 + 1);
	  insertmarker.setBottom (insertmarker.bottom() + mDropVisualizerWidth/2);
	}
  else
	{
	  insertmarker = QRect (0, 0, viewport()->width(), mDropVisualizerWidth/2);
	}

  if (p)
	p->fillRect(insertmarker, Dense4Pattern);
  
  return insertmarker;
}

QRect KListView::drawItemHighlighter(QPainter */*painter*/, QListViewItem */*item*/)
{
  return QRect(0,0,0,0);
}

void KListView::cleanItemHighlighter ()
{
  // FIXME
}

void KListView::rename(QListViewItem *item, int c)
{
	d->editor->load(item,c);

}

bool KListView::isRenameable (int col) const
{
	return d->renameable.contains(col);
}

void KListView::setRenameable (int col, bool yesno)
{
	if (col>=header()->count()) return;

	d->renameable.remove(col);
	if (yesno && d->renameable.find(col)==d->renameable.end())
		d->renameable+=col;
	else if (!yesno && d->renameable.find(col)!=d->renameable.end())
		d->renameable.remove(col);
}

void KListView::doneEditing(QListViewItem *item, int row)
{
	emit itemRenamed(item, item->text(row), row);
	emit itemRenamed(item);
}

bool KListView::acceptDrag(QDropEvent*) const
{
	return true;
}


void KListView::setCreateChildren(bool b)
{
	d->createChildren=b;
}

bool KListView::createChildren() const
{
	return d->createChildren;
}


int KListView::tooltipColumn() const
{
	return d->tooltipColumn;
}

void KListView::setTooltipColumn(int column)
{
	d->tooltipColumn=column;
}

void KListView::setDropHighlighter(bool b)
{
	d->dropHighlighter=b;
}

bool KListView::dropHighlighter() const
{
	return d->dropHighlighter;
}

bool KListView::showTooltip(QListViewItem *item, const QPoint &, int column) const
{
	return ((tooltip(item, column).length()>0) && (column==tooltipColumn()));
}

QString KListView::tooltip(QListViewItem *item, int column) const
{
	return item->text(column);
}

void KListView::keyPressEvent (QKeyEvent* e)
{
  if (e->key() == d->contextMenuKey)
	{
	  emit menuShortCutPressed (this, currentItem());
	  return;
	}
	
  if (d->selectionMode != Konqueror)
	QListView::keyPressEvent (e);
  else
	konquerorKeyPressEvent (e);
}

void KListView::konquerorKeyPressEvent (QKeyEvent* e)
{
  if ((e->state()==ShiftButton) && (e->key()!=Key_Shift) &&
	  (e->key()!=Key_Control) && (e->key()!=Key_Meta) &&
	  (e->key()!=Key_Alt) && (!d->wasShiftEvent)) 
	selectAll(FALSE);
  
  d->wasShiftEvent = e->state() == ShiftButton;

  // We are only interested in the insert key here
  QListViewItem* item = currentItem();
  //insert without modifiers toggles the selection of the current item and moves to the next
  if (item==0) return;
  QListViewItem* nextItem = 0L;
  int items = 0;

  if (((e->key() == Key_Enter)|| (e->key() == Key_Return)) && (e->state() == ControlButton))
	{
      QListViewItem* item = currentItem();

      if ( !item->isSelected() )
		{
		  QListViewItemIterator it (item);
		  for( ; it.current(); it++ )
            if ( it.current()->isSelected() )
			  setSelected( it.current(), false );
		  setSelected( item, true );
		}

      QPoint p (width() / 2, height() / 2 );
      p = mapToGlobal( p );
	  //      popupMenu( p );
      return;
   }
  
   switch (e->key())
	 {
	 case Key_Escape:
	   selectAll(FALSE);
	   break;

	 case Key_Space:
	   //toggle selection of current item
	   item->setSelected(!item->isSelected());
	   item->repaint();
	   emit selectionChanged();
	   break;

	 case Key_Insert:
	   //toggle selection of current item and move to the next item
	   item->setSelected(!item->isSelected());
	   nextItem=item->itemBelow();
	   if (nextItem!=0)
		 {
		   setCurrentItem(nextItem);
		   ensureItemVisible(nextItem);
		 }
	   else item->repaint();
	   emit selectionChanged();
	   break;

	 case Key_Down:
	   //toggle selection of current item and move to the next item
	   if ((e->state()==ShiftButton) || (e->state()==(ControlButton|ShiftButton)))
         item->setSelected(!item->isSelected());
	   nextItem=item->itemBelow();
	   if (nextItem!=0)
		 {
		   setCurrentItem(nextItem);
		   ensureItemVisible(nextItem);
		 }
	   else item->repaint();
	   if ((e->state()==ShiftButton) || (e->state()==(ControlButton|ShiftButton)))
		 {
		   emit selectionChanged();
		 }
	   break;

	 case Key_Up:
	   //move to the prev. item and toggle selection of this one
	   nextItem=item->itemAbove();
	   if (nextItem==0) break;
	   if ((e->state()==ShiftButton) || (e->state()==(ControlButton|ShiftButton)))
         nextItem->setSelected(!nextItem->isSelected());
	   setCurrentItem(nextItem);
	   ensureItemVisible(nextItem);
	   if ((e->state()==ShiftButton) || (e->state()==(ControlButton|ShiftButton)))
		 {
		   emit selectionChanged();
		 }
	   break;

	 case Key_End:
	   //move to the last item and toggle selection of all items inbetween
	   nextItem=item;
	   
	   while(nextItem!=0)
		 {
		   if ((e->state()==ShiftButton) || (e->state()==(ControlButton|ShiftButton)))
			 nextItem->setSelected(!nextItem->isSelected());
		   if (nextItem->itemBelow()==0)
			 {
			   nextItem->repaint();
			   ensureItemVisible(nextItem);
			   setCurrentItem(nextItem);
			 }
		   nextItem=nextItem->itemBelow();
		 }
	   if ((e->state()==ShiftButton) || (e->state()==(ControlButton|ShiftButton)))
		 {
		   emit selectionChanged();
		 }
	   break;

	 case Key_Home:
	   //move to the last item and toggle selection of all items inbetween
	   nextItem=item;
	   
	   while(nextItem!=0)
		 {
		   if ((e->state()==ShiftButton) || (e->state()==(ControlButton|ShiftButton)))
			 nextItem->setSelected(!nextItem->isSelected());
		   if (nextItem->itemAbove()==0)
			 {
			   nextItem->repaint();
			   ensureItemVisible(nextItem);
			   setCurrentItem(nextItem);
			 }
		   nextItem=nextItem->itemAbove();
		 }
	   if ((e->state()==ShiftButton) || (e->state()==(ControlButton|ShiftButton)))
		 {
		   emit selectionChanged();
		 }
	   break;

	 case Key_Next:
	   items=visibleHeight()/item->height();
	   nextItem=item;
	   for (int i=0; i<items; i++)
		 {
		   if ((e->state()==ShiftButton) || (e->state()==(ControlButton|ShiftButton)))
			 nextItem->setSelected(!nextItem->isSelected());
		   //the end
		   if ((i==items-1) || (nextItem->itemBelow()==0))

			 {
			   if ((e->state()==ShiftButton) || (e->state()==(ControlButton|ShiftButton)))
				 nextItem->setSelected(!nextItem->isSelected());
			   nextItem->repaint();
			   ensureItemVisible(nextItem);
			   setCurrentItem(nextItem);
			   if ((e->state()==ShiftButton) || (e->state()==(ControlButton|ShiftButton)))
				 {
				   emit selectionChanged();
				 }
			   return;
			 }
		   nextItem=nextItem->itemBelow();
		 }
	   break;

	 case Key_Prior:
	   items=visibleHeight()/item->height();
	   nextItem=item;
	   for (int i=0; i<items; i++)
		 {
		   if ((nextItem!=item) &&((e->state()==ShiftButton) || (e->state()==(ControlButton|ShiftButton))))
			 nextItem->setSelected(!nextItem->isSelected());
		   //the end
		   if ((i==items-1) || (nextItem->itemAbove()==0))

			 {
			   nextItem->repaint();
			   ensureItemVisible(nextItem);
			   setCurrentItem(nextItem);
			   if ((e->state()==ShiftButton) || (e->state()==(ControlButton|ShiftButton)))
				 {
				   emit selectionChanged();
				 }
			   return;
			 }
		   nextItem=nextItem->itemAbove();
		 }
	   break;

	 case Key_Minus:
       if ( item->isOpen() )
		 setOpen( item, FALSE );
       break;
	 case Key_Plus:
       if (  !item->isOpen() && (item->isExpandable() || item->childCount()) )
		 setOpen( item, TRUE );
       break;
	 default:
	   QListView::keyPressEvent (e);
	   break;
	 }
}

void KListView::setSelectionModeExt (SelectionModeExt mode)
{
  d->selectionMode = mode;

  switch (mode)
	{
	case Single:
	case Multi:
	case Extended:
	case NoSelection:
	  setSelectionMode (static_cast<QListView::SelectionMode>(static_cast<int>(mode)));
	  break;

	default:
	  break;
	}
}

KListView::SelectionModeExt KListView::selectionModeExt () const
{
  return d->selectionMode;
}

void KListView::emitContextMenu (KListView*, QListViewItem* i)
{
  QPoint p;

  kdDebug () << "KListView::emitContextMenu " << i << endl;

  if (i)
	p = viewport()->mapToGlobal(itemRect(i).center());
  else
	p = mapToGlobal(rect().center());

  kdDebug() << "at: x " << p.x() << " y " << p.y() << endl;

  emit contextMenu (this, i, p);
}

void KListView::emitContextMenu (QListViewItem* i, const QPoint& p, int)
{
  dumpObjectInfo();
  kdDebug () << "KListView::emitContextMenu " << i << " at: x " << p.x() << " y " << p.y()<< endl;

  emit contextMenu (this, i, p);
}

#include "klistviewlineedit.moc"
