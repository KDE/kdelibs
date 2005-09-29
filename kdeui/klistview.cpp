/* This file is part of the KDE libraries
   Copyright (C) 2000 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2000,2003 Charles Samuels <charles@kde.org>
   Copyright (C) 2000 Peter Putzer

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

#include <qdragobject.h>
#include <qtimer.h>
#include <qheader.h>
#include <qcursor.h>
#include <qtooltip.h>
#include <qstyle.h>
#include <qpainter.h>

#include <kglobalsettings.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kapplication.h>
#include <kipc.h>
#include <kdebug.h>

#include "klistview.h"
#include "klistviewlineedit.h"

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
  KListViewPrivate (KListView* listview)
    : pCurrentItem (0),
      autoSelectDelay(0),
      dragOverItem(0),
      dragDelay (KGlobalSettings::dndEventDelay()),
      editor (new KListViewLineEdit (listview)),
      cursorInExecuteArea(false),
      itemsMovable (true),
      selectedBySimpleMove(false),
      selectedUsingMouse(false),
      itemsRenameable (false),
      validDrag (false),
      dragEnabled (false),
      autoOpen (true),
      disableAutoSelection (false),
      dropVisualizer (true),
      dropHighlighter (false),
      createChildren (true),
      pressedOnSelected (false),
      wasShiftEvent (false),
      fullWidth (false),
      sortAscending(true),
      tabRename(true),
      sortColumn(0),
      selectionDirection(0),
      tooltipColumn (0),
      selectionMode (Single),
      contextMenuKey (KGlobalSettings::contextMenuKey()),
      showContextMenusOnPress (KGlobalSettings::showContextMenusOnPress()),
      mDropVisualizerWidth (4),
      paintAbove (0),
      paintCurrent (0),
      paintBelow (0),
      painting (false),
      shadeSortColumn(KGlobalSettings::shadeSortColumn())
  {
      renameable.append(0);
      connect(editor, SIGNAL(done(QListViewItem*,int)), listview, SLOT(doneEditing(QListViewItem*,int)));
  }

  ~KListViewPrivate ()
  {
    delete editor;
  }

  QListViewItem* pCurrentItem;

  QTimer autoSelect;
  int autoSelectDelay;

  QTimer dragExpand;
  QListViewItem* dragOverItem;
  QPoint dragOverPoint;

  QPoint startDragPos;
  int dragDelay;

  KListViewLineEdit *editor;
  QValueList<int> renameable;

  bool cursorInExecuteArea:1;
  bool bUseSingle:1;
  bool bChangeCursorOverItem:1;
  bool itemsMovable:1;
  bool selectedBySimpleMove : 1;
  bool selectedUsingMouse:1;
  bool itemsRenameable:1;
  bool validDrag:1;
  bool dragEnabled:1;
  bool autoOpen:1;
  bool disableAutoSelection:1;
  bool dropVisualizer:1;
  bool dropHighlighter:1;
  bool createChildren:1;
  bool pressedOnSelected:1;
  bool wasShiftEvent:1;
  bool fullWidth:1;
  bool sortAscending:1;
  bool tabRename:1;

  int sortColumn;

  //+1 means downwards (y increases, -1 means upwards, 0 means not selected), aleXXX
  int selectionDirection;
  int tooltipColumn;

  SelectionModeExt selectionMode;
  int contextMenuKey;
  bool showContextMenusOnPress;

  QRect mOldDropVisualizer;
  int mDropVisualizerWidth;
  QRect mOldDropHighlighter;
  QListViewItem *afterItemDrop;
  QListViewItem *parentItemDrop;

  QListViewItem *paintAbove;
  QListViewItem *paintCurrent;
  QListViewItem *paintBelow;
  bool painting:1;
  bool shadeSortColumn:1;

  QColor alternateBackground;
};


KListViewLineEdit::KListViewLineEdit(KListView *parent)
        : KLineEdit(parent->viewport()), item(0), col(0), p(parent)
{
        setFrame( false );
        hide();
        connect( parent, SIGNAL( selectionChanged() ), SLOT( slotSelectionChanged() ));
}

KListViewLineEdit::~KListViewLineEdit()
{
}

QListViewItem *KListViewLineEdit::currentItem() const
{
	return item;
}

void KListViewLineEdit::load(QListViewItem *i, int c)
{
        item=i;
        col=c;

        QRect rect(p->itemRect(i));
        setText(item->text(c));
        home( true );

        int fieldX = rect.x() - 1;
        int fieldW = p->columnWidth(col) + 2;

        QHeader* const pHeader = p->header();

        const int pos = pHeader->mapToIndex(col);
        for ( int index = 0; index < pos; ++index )
            fieldX += p->columnWidth( pHeader->mapToSection( index ));

        if ( col == 0 ) {
            int d = i->depth() + (p->rootIsDecorated() ? 1 : 0);
            d *= p->treeStepSize();
            fieldX += d;
            fieldW -= d;
        }

        if ( i->pixmap( col ) ) {// add width of pixmap
            int d = i->pixmap( col )->width();
            fieldX += d;
            fieldW -= d;
        }

        setGeometry(fieldX, rect.y() - 1, fieldW, rect.height() + 2);
        show();
        setFocus();
}

/*	Helper functions to for
 *	tabOrderedRename functionality.
 */

static int nextCol (KListView *pl, QListViewItem *pi, int start, int dir)
{
	if (pi)
	{
		//	Find the next renameable column in the current row
		for (; ((dir == +1) ? (start < pl->columns()) : (start >= 0)); start += dir)
			if (pl->isRenameable(start))
				return start;
	}

	return -1;
}

static QListViewItem *prevItem (QListViewItem *pi)
{
	QListViewItem *pa = pi->itemAbove();

	/*	Does what the QListViewItem::previousSibling()
	 *	of my dreams would do.
	 */
	if (pa && pa->parent() == pi->parent())
		return pa;

	return 0;
}

static QListViewItem *lastQChild (QListViewItem *pi)
{
	if (pi)
	{
		/*	Since there's no QListViewItem::lastChild().
		 *	This finds the last sibling for the given
		 *	item.
		 */
		for (QListViewItem *pt = pi->nextSibling(); pt; pt = pt->nextSibling())
			pi = pt;
	}

	return pi;
}

void KListViewLineEdit::selectNextCell (QListViewItem *pitem, int column, bool forward)
{
	const int ncols = p->columns();
	const int dir = forward ? +1 : -1;
	const int restart = forward ? 0 : (ncols - 1);
	QListViewItem *top = (pitem && pitem->parent())
		? pitem->parent()->firstChild()
		: p->firstChild();
	QListViewItem *pi = pitem;

	terminate();		//	Save current changes

	do
	{
		/*	Check the rest of the current row for an editable column,
		 *	if that fails, check the entire next/previous row. The
		 *	last case goes back to the first item in the current branch
		 *	or the last item in the current branch depending on the
		 *	direction.
		 */
		if ((column = nextCol(p, pi, column + dir, dir)) != -1 ||
			(column = nextCol(p, (pi = (forward ? pi->nextSibling() : prevItem(pi))), restart, dir)) != -1 ||
			(column = nextCol(p, (pi = (forward ? top : lastQChild(pitem))), restart, dir)) != -1)
		{
			if (pi)
			{
				p->setCurrentItem(pi);		//	Calls terminate
				p->rename(pi, column);

				/*	Some listviews may override rename() to
				 *	prevent certain items from being renamed,
				 *	if this is done, [m_]item will be NULL
				 *	after the rename() call... try again.
				 */
				if (!item)
					continue;

				break;
			}
		}
	}
	while (pi && !item);
}

#ifdef KeyPress
#undef KeyPress
#endif

bool KListViewLineEdit::event (QEvent *pe)
{
	if (pe->type() == QEvent::KeyPress)
	{
		QKeyEvent *k = (QKeyEvent *) pe;

	    if ((k->key() == Qt::Key_Backtab || k->key() == Qt::Key_Tab) &&
			p->tabOrderedRenaming() && p->itemsRenameable() &&
			!(k->state() & ControlButton || k->state() & AltButton))
		{
			selectNextCell(item, col,
				(k->key() == Key_Tab && !(k->state() & ShiftButton)));
			return true;
	    }
	}

	return KLineEdit::event(pe);
}

void KListViewLineEdit::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter )
		terminate(true);
	else if(e->key() == Qt::Key_Escape)
		terminate(false);
        else if (e->key() == Qt::Key_Down || e->key() == Qt::Key_Up)
        {
		terminate(true);
                KLineEdit::keyPressEvent(e);
        }
	else
		KLineEdit::keyPressEvent(e);
}

void KListViewLineEdit::terminate()
{
    terminate(true);
}

void KListViewLineEdit::terminate(bool commit)
{
    if ( item )
    {
        //kdDebug() << "KListViewLineEdit::terminate " << commit << endl;
        if (commit)
            item->setText(col, text());
        int c=col;
        QListViewItem *i=item;
        col=0;
        item=0;
        p->setFocus();// will call focusOutEvent, that's why we set item=0 before
        hide();
        if (commit)
            emit done(i,c);
    }
}

void KListViewLineEdit::focusOutEvent(QFocusEvent *ev)
{
    QFocusEvent * focusEv = static_cast<QFocusEvent*>(ev);
    // Don't let a RMB close the editor
    if (focusEv->reason() != QFocusEvent::Popup && focusEv->reason() != QFocusEvent::ActiveWindow)
        terminate(true);
    else
        KLineEdit::focusOutEvent(ev);
}

void KListViewLineEdit::paintEvent( QPaintEvent *e )
{
    KLineEdit::paintEvent( e );

    if ( !frame() ) {
        QPainter p( this );
        p.setClipRegion( e->region() );
        p.drawRect( rect() );
    }
}

// selection changed -> terminate. As our "item" can be already deleted,
// we can't call terminate(false), because that would emit done() with
// a dangling pointer to "item".
void KListViewLineEdit::slotSelectionChanged()
{
    item = 0;
    col = 0;
    hide();
}


KListView::KListView( QWidget *parent, const char *name )
  : QListView( parent, name ),
        d (new KListViewPrivate (this))
{
  setDragAutoScroll(true);

  connect( this, SIGNAL( onViewport() ),
                   this, SLOT( slotOnViewport() ) );
  connect( this, SIGNAL( onItem( QListViewItem * ) ),
                   this, SLOT( slotOnItem( QListViewItem * ) ) );

  connect (this, SIGNAL(contentsMoving(int,int)),
                   this, SLOT(cleanDropVisualizer()));
  connect (this, SIGNAL(contentsMoving(int,int)),
                   this, SLOT(cleanItemHighlighter()));

  slotSettingsChanged(KApplication::SETTINGS_MOUSE);
  if (kapp)
  {
    connect( kapp, SIGNAL( settingsChanged(int) ), SLOT( slotSettingsChanged(int) ) );
    kapp->addKipcEventMask( KIPC::SettingsChanged );
  }

  connect(&d->autoSelect, SIGNAL( timeout() ),
                  this, SLOT( slotAutoSelect() ) );
  connect(&d->dragExpand, SIGNAL( timeout() ),
                  this, SLOT( slotDragExpand() ) );

  // context menu handling
  if (d->showContextMenusOnPress)
        {
          connect (this, SIGNAL (rightButtonPressed (QListViewItem*, const QPoint&, int)),
                           this, SLOT (emitContextMenu (QListViewItem*, const QPoint&, int)));
        }
  else
        {
          connect (this, SIGNAL (rightButtonClicked (QListViewItem*, const QPoint&, int)),
                           this, SLOT (emitContextMenu (QListViewItem*, const QPoint&, int)));
        }

  connect (this, SIGNAL (menuShortCutPressed (KListView*, QListViewItem*)),
                   this, SLOT (emitContextMenu (KListView*, QListViewItem*)));
  d->alternateBackground = KGlobalSettings::alternateBackgroundColor();
}

KListView::~KListView()
{
  delete d;
}

bool KListView::isExecuteArea( const QPoint& point )
{
  QListViewItem* item = itemAt( point );
  if ( item ) {
    return isExecuteArea( point.x(), item );
  }

  return false;
}

bool KListView::isExecuteArea( int x )
{
  return isExecuteArea( x, 0 );
}

bool KListView::isExecuteArea( int x, QListViewItem* item )
{
  if( allColumnsShowFocus() )
    return true;
  else {
    int offset = 0;


    int width = columnWidth( 0 );

    QHeader* const thisHeader = header();
    const int pos = thisHeader->mapToIndex( 0 );

    for ( int index = 0; index < pos; ++index )
      offset += columnWidth( thisHeader->mapToSection( index ) );

    x += contentsX(); // in case of a horizontal scrollbar

    if ( item )
    {
	width = treeStepSize()*( item->depth() + ( rootIsDecorated() ? 1 : 0 ) );
	width += itemMargin();
	int ca = AlignHorizontal_Mask & columnAlignment( 0 );
	if ( ca == AlignLeft || ca == AlignAuto ) {
	    width += item->width( fontMetrics(), this, 0 );
	    if ( width > columnWidth( 0 ) )
		width = columnWidth( 0 );
	}
    }

    return ( x > offset && x < ( offset + width ) );
  }
}

void KListView::slotOnItem( QListViewItem *item )
{
  QPoint vp = viewport()->mapFromGlobal( QCursor::pos() );
  if ( item && isExecuteArea( vp.x() ) && (d->autoSelectDelay > -1) && d->bUseSingle ) {
    d->autoSelect.start( d->autoSelectDelay, true );
    d->pCurrentItem = item;
  }
}

void KListView::slotOnViewport()
{
  if ( d->bChangeCursorOverItem )
    viewport()->unsetCursor();

  d->autoSelect.stop();
  d->pCurrentItem = 0L;
}

void KListView::slotSettingsChanged(int category)
{
  switch (category)
  {
  case KApplication::SETTINGS_MOUSE:
    d->dragDelay =  KGlobalSettings::dndEventDelay();
    d->bUseSingle = KGlobalSettings::singleClick();

    disconnect(this, SIGNAL (mouseButtonClicked (int, QListViewItem*, const QPoint &, int)),
               this, SLOT (slotMouseButtonClicked (int, QListViewItem*, const QPoint &, int)));

    if( d->bUseSingle )
      connect (this, SIGNAL (mouseButtonClicked (int, QListViewItem*, const QPoint &, int)),
               this, SLOT (slotMouseButtonClicked( int, QListViewItem*, const QPoint &, int)));

    d->bChangeCursorOverItem = KGlobalSettings::changeCursorOverIcon();
    if ( !d->disableAutoSelection )
      d->autoSelectDelay = KGlobalSettings::autoSelectDelay();

    if( !d->bUseSingle || !d->bChangeCursorOverItem )
       viewport()->unsetCursor();

    break;

  case KApplication::SETTINGS_POPUPMENU:
    d->contextMenuKey = KGlobalSettings::contextMenuKey ();
    d->showContextMenusOnPress = KGlobalSettings::showContextMenusOnPress ();

    if (d->showContextMenusOnPress)
    {
      disconnect (0L, 0L, this, SLOT (emitContextMenu (QListViewItem*, const QPoint&, int)));

      connect(this, SIGNAL (rightButtonPressed (QListViewItem*, const QPoint&, int)),
              this, SLOT (emitContextMenu (QListViewItem*, const QPoint&, int)));
    }
    else
    {
      disconnect (0L, 0L, this, SLOT (emitContextMenu (QListViewItem*, const QPoint&, int)));

      connect(this, SIGNAL (rightButtonClicked (QListViewItem*, const QPoint&, int)),
              this, SLOT (emitContextMenu (QListViewItem*, const QPoint&, int)));
    }
    break;

  default:
    break;
  }
}

void KListView::slotAutoSelect()
{
  // check that the item still exists
  if( itemIndex( d->pCurrentItem ) == -1 )
    return;

  if (!isActiveWindow())
        {
          d->autoSelect.stop();
          return;
        }

  //Give this widget the keyboard focus.
  if( !hasFocus() )
    setFocus();

  ButtonState keybstate = KApplication::keyboardMouseState();

  QListViewItem* previousItem = currentItem();
  setCurrentItem( d->pCurrentItem );

  if( d->pCurrentItem ) {
    //Shift pressed?
    if( (keybstate & Qt::ShiftButton) ) {
      bool block = signalsBlocked();
      blockSignals( true );

      //No Ctrl? Then clear before!
      if( !(keybstate & Qt::ControlButton) )
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
    else if( (keybstate & KApplication::ControlModifier) )
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
    kdDebug() << "KListView::slotAutoSelect: That´s not supposed to happen!!!!" << endl;
}

void KListView::slotHeaderChanged()
{

  const int colCount = columns();
  if (d->fullWidth && colCount)
  {
    int w = 0;
    const int lastColumn = colCount - 1;
    for (int i = 0; i < lastColumn; ++i) w += columnWidth(i);
    setColumnWidth( lastColumn, viewport()->width() - w - 1 );
  }
}

void KListView::emitExecute( QListViewItem *item, const QPoint &pos, int c )
{
    if( isExecuteArea( viewport()->mapFromGlobal(pos) ) ) {
	d->validDrag=false;

        // Double click mode ?
        if ( !d->bUseSingle )
        {
            viewport()->unsetCursor();
            emit executed( item );
            emit executed( item, pos, c );
        }
        else
        {
            ButtonState keybstate = KApplication::keyboardMouseState();

            d->autoSelect.stop();

            //Don´t emit executed if in SC mode and Shift or Ctrl are pressed
            if( !( ((keybstate & Qt::ShiftButton) || (keybstate & Qt::ControlButton)) ) ) {
                viewport()->unsetCursor();
                emit executed( item );
                emit executed( item, pos, c );
            }
        }
    }
}

void KListView::focusInEvent( QFocusEvent *fe )
{
 //   kdDebug()<<"KListView::focusInEvent()"<<endl;
  QListView::focusInEvent( fe );
  if ((d->selectedBySimpleMove)
      && (d->selectionMode == FileManager)
      && (fe->reason()!=QFocusEvent::Popup)
      && (fe->reason()!=QFocusEvent::ActiveWindow)
      && (currentItem()))
  {
      currentItem()->setSelected(true);
      currentItem()->repaint();
      emit selectionChanged();
  };
}

void KListView::focusOutEvent( QFocusEvent *fe )
{
  cleanDropVisualizer();
  cleanItemHighlighter();

  d->autoSelect.stop();

  if ((d->selectedBySimpleMove)
      && (d->selectionMode == FileManager)
      && (fe->reason()!=QFocusEvent::Popup)
      && (fe->reason()!=QFocusEvent::ActiveWindow)
      && (currentItem())
      && (!d->editor->isVisible()))
  {
      currentItem()->setSelected(false);
      currentItem()->repaint();
      emit selectionChanged();
  };

  QListView::focusOutEvent( fe );
}

void KListView::leaveEvent( QEvent *e )
{
  d->autoSelect.stop();

  QListView::leaveEvent( e );
}

bool KListView::event( QEvent *e )
{
  if (e->type() == QEvent::ApplicationPaletteChange)
    d->alternateBackground=KGlobalSettings::alternateBackgroundColor();

  return QListView::event(e);
}

void KListView::contentsMousePressEvent( QMouseEvent *e )
{
  if( (selectionModeExt() == Extended) && (e->state() & ShiftButton) && !(e->state() & ControlButton) )
  {
    bool block = signalsBlocked();
    blockSignals( true );

    clearSelection();

    blockSignals( block );
  }
  else if ((selectionModeExt()==FileManager) && (d->selectedBySimpleMove))
  {
     d->selectedBySimpleMove=false;
     d->selectedUsingMouse=true;
     if (currentItem())
     {
        currentItem()->setSelected(false);
        currentItem()->repaint();
//        emit selectionChanged();
     }
  }

  QPoint p( contentsToViewport( e->pos() ) );
  QListViewItem *at = itemAt (p);

  // true if the root decoration of the item "at" was clicked (i.e. the +/- sign)
  bool rootDecoClicked = at
           && ( p.x() <= header()->cellPos( header()->mapToActual( 0 ) ) +
                treeStepSize() * ( at->depth() + ( rootIsDecorated() ? 1 : 0) ) + itemMargin() )
           && ( p.x() >= header()->cellPos( header()->mapToActual( 0 ) ) );

  if (e->button() == LeftButton && !rootDecoClicked)
  {
    //Start a drag
    d->startDragPos = e->pos();

    if (at)
    {
      d->validDrag = true;
      d->pressedOnSelected = at->isSelected();
    }
  }

  QListView::contentsMousePressEvent( e );
}

void KListView::contentsMouseMoveEvent( QMouseEvent *e )
{
  if (!dragEnabled() || d->startDragPos.isNull() || !d->validDrag)
      QListView::contentsMouseMoveEvent (e);

  QPoint vp = contentsToViewport(e->pos());
  QListViewItem *item = itemAt( vp );

  //do we process cursor changes at all?
  if ( item && d->bChangeCursorOverItem && d->bUseSingle )
    {
      //Cursor moved on a new item or in/out the execute area
      if( (item != d->pCurrentItem) ||
          (isExecuteArea(vp) != d->cursorInExecuteArea) )
        {
          d->cursorInExecuteArea = isExecuteArea(vp);

          if( d->cursorInExecuteArea ) //cursor moved in execute area
            viewport()->setCursor( KCursor::handCursor() );
          else //cursor moved out of execute area
            viewport()->unsetCursor();
        }
    }

  bool dragOn = dragEnabled();
  QPoint newPos = e->pos();
  if (dragOn && d->validDrag &&
      (newPos.x() > d->startDragPos.x()+d->dragDelay ||
       newPos.x() < d->startDragPos.x()-d->dragDelay ||
       newPos.y() > d->startDragPos.y()+d->dragDelay ||
       newPos.y() < d->startDragPos.y()-d->dragDelay))
    //(d->startDragPos - e->pos()).manhattanLength() > QApplication::startDragDistance())
    {
      QListView::contentsMouseReleaseEvent( 0 );
      startDrag();
      d->startDragPos = QPoint();
      d->validDrag = false;
    }
}

void KListView::contentsMouseReleaseEvent( QMouseEvent *e )
{
  if (e->button() == LeftButton)
  {
    // If the row was already selected, maybe we want to start an in-place editing
    if ( d->pressedOnSelected && itemsRenameable() )
    {
      QPoint p( contentsToViewport( e->pos() ) );
      QListViewItem *at = itemAt (p);
      if ( at )
      {
        // true if the root decoration of the item "at" was clicked (i.e. the +/- sign)
        bool rootDecoClicked =
                  ( p.x() <= header()->cellPos( header()->mapToActual( 0 ) ) +
                    treeStepSize() * ( at->depth() + ( rootIsDecorated() ? 1 : 0) ) + itemMargin() )
               && ( p.x() >= header()->cellPos( header()->mapToActual( 0 ) ) );

        if (!rootDecoClicked)
        {
          int col = header()->mapToLogical( header()->cellAt( p.x() ) );
          if ( d->renameable.contains(col) )
            rename(at, col);
        }
      }
    }

    d->pressedOnSelected = false;
    d->validDrag = false;
    d->startDragPos = QPoint();
  }
  QListView::contentsMouseReleaseEvent( e );
}

void KListView::contentsMouseDoubleClickEvent ( QMouseEvent *e )
{
  // We don't want to call the parent method because it does setOpen,
  // whereas we don't do it in single click mode... (David)
  //QListView::contentsMouseDoubleClickEvent( e );
  if ( !e || e->button() != LeftButton )
    return;

  QPoint vp = contentsToViewport(e->pos());
  QListViewItem *item = itemAt( vp );
  emit QListView::doubleClicked( item ); // we do it now

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
    emitExecute(item, pos, c);
}

void KListView::contentsDropEvent(QDropEvent* e)
{
  cleanDropVisualizer();
  cleanItemHighlighter();
  d->dragExpand.stop();

  if (acceptDrag (e))
  {
    e->acceptAction();
    QListViewItem *afterme;
    QListViewItem *parent;

    findDrop(e->pos(), parent, afterme);

    if (e->source() == viewport() && itemsMovable())
        movableDropEvent(parent, afterme);
    else
    {
        emit dropped(e, afterme);
        emit dropped(this, e, afterme);
        emit dropped(e, parent, afterme);
        emit dropped(this, e, parent, afterme);
    }
  }
}

void KListView::movableDropEvent (QListViewItem* parent, QListViewItem* afterme)
{
  QPtrList<QListViewItem> items, afterFirsts, afterNows;
  QListViewItem *current=currentItem();
  bool hasMoved=false;
  for (QListViewItem *i = firstChild(), *iNext=0; i; i = iNext)
  {
    iNext=i->itemBelow();
    if (!i->isSelected())
      continue;

    // don't drop an item after itself, or else
    // it moves to the top of the list
    if (i==afterme)
      continue;

    i->setSelected(false);

    QListViewItem *afterFirst = i->itemAbove();

        if (!hasMoved)
        {
                emit aboutToMove();
                hasMoved=true;
        }

    moveItem(i, parent, afterme);

    // ###### This should include the new parent !!! -> KDE 3.0
    // If you need this right now, have a look at keditbookmarks.
    emit moved(i, afterFirst, afterme);

    items.append (i);
    afterFirsts.append (afterFirst);
    afterNows.append (afterme);

    afterme = i;
  }
  clearSelection();
  for (QListViewItem *i=items.first(); i; i=items.next() )
    i->setSelected(true);
  if (current)
    setCurrentItem(current);

  emit moved(items,afterFirsts,afterNows);

  if (firstChild())
    emit moved();
}

void KListView::contentsDragMoveEvent(QDragMoveEvent *event)
{
  if (acceptDrag(event))
  {
    event->acceptAction();
    //Clean up the view

    findDrop(event->pos(), d->parentItemDrop, d->afterItemDrop);
    QPoint vp = contentsToViewport( event->pos() );
    QListViewItem *item = isExecuteArea( vp ) ? itemAt( vp ) : 0L;

    if ( item != d->dragOverItem )
    {
      d->dragExpand.stop();
      d->dragOverItem = item;
      d->dragOverPoint = vp;
      if ( d->dragOverItem && d->dragOverItem->isExpandable() && !d->dragOverItem->isOpen() )
        d->dragExpand.start( QApplication::startDragTime(), true );
    }
    if (dropVisualizer())
    {
      QRect tmpRect = drawDropVisualizer(0, d->parentItemDrop, d->afterItemDrop);
      if (tmpRect != d->mOldDropVisualizer)
      {
        cleanDropVisualizer();
        d->mOldDropVisualizer=tmpRect;
        viewport()->repaint(tmpRect);
      }
    }
    if (dropHighlighter())
    {
      QRect tmpRect = drawItemHighlighter(0, itemAt( vp ));
      if (tmpRect != d->mOldDropHighlighter)
      {
        cleanItemHighlighter();
        d->mOldDropHighlighter=tmpRect;
        viewport()->repaint(tmpRect);
      }
    }
  }
  else
      event->ignore();
}

void KListView::slotDragExpand()
{
  if ( itemAt( d->dragOverPoint ) == d->dragOverItem )
    d->dragOverItem->setOpen( true );
}

void KListView::contentsDragLeaveEvent (QDragLeaveEvent*)
{
  d->dragExpand.stop();
  cleanDropVisualizer();
  cleanItemHighlighter();
}

void KListView::cleanDropVisualizer()
{
  if (d->mOldDropVisualizer.isValid())
  {
    QRect rect=d->mOldDropVisualizer;
    d->mOldDropVisualizer = QRect();
    viewport()->repaint(rect, true);
  }
}

int KListView::depthToPixels( int depth )
{
    return treeStepSize() * ( depth + (rootIsDecorated() ? 1 : 0) ) + itemMargin();
}

void KListView::findDrop(const QPoint &pos, QListViewItem *&parent, QListViewItem *&after)
{
	QPoint p (contentsToViewport(pos));

	// Get the position to put it in
	QListViewItem *atpos = itemAt(p);

	QListViewItem *above;
	if (!atpos) // put it at the end
		above = lastItem();
	else
	{
		// Get the closest item before us ('atpos' or the one above, if any)
		if (p.y() - itemRect(atpos).topLeft().y() < (atpos->height()/2))
			above = atpos->itemAbove();
		else
			above = atpos;
	}

	if (above)
	{
		// if above has children, I might need to drop it as the first item there

		if (above->firstChild() && above->isOpen())
		{
			parent = above;
			after = 0;
			return;
		}

      // Now, we know we want to go after "above". But as a child or as a sibling ?
      // We have to ask the "above" item if it accepts children.
      if (above->isExpandable())
      {
          // The mouse is sufficiently on the right ? - doesn't matter if 'above' has visible children
          if (p.x() >= depthToPixels( above->depth() + 1 ) ||
              (above->isOpen() && above->childCount() > 0) )
          {
              parent = above;
              after = 0L;
              return;
          }
      }

      // Ok, there's one more level of complexity. We may want to become a new
      // sibling, but of an upper-level group, rather than the "above" item
      QListViewItem * betterAbove = above->parent();
      QListViewItem * last = above;
      while ( betterAbove )
      {
          // We are allowed to become a sibling of "betterAbove" only if we are
          // after its last child
          if ( !last->nextSibling() )
          {
              if (p.x() < depthToPixels ( betterAbove->depth() + 1 ))
                  above = betterAbove; // store this one, but don't stop yet, there may be a better one
              else
                  break; // not enough on the left, so stop
              last = betterAbove;
              betterAbove = betterAbove->parent(); // up one level
          } else
              break; // we're among the child of betterAbove, not after the last one
      }
  }
  // set as sibling
  after = above;
  parent = after ? after->parent() : 0L ;
}

QListViewItem* KListView::lastChild () const
{
  QListViewItem* lastchild = firstChild();

  if (lastchild)
        for (; lastchild->nextSibling(); lastchild = lastchild->nextSibling());

  return lastchild;
}

QListViewItem *KListView::lastItem() const
{
  QListViewItem* last = lastChild();

  for (QListViewItemIterator it (last); it.current(); ++it)
    last = it.current();

  return last;
}

KLineEdit *KListView::renameLineEdit() const
{
  return d->editor;
}

void KListView::startDrag()
{
  QDragObject *drag = dragObject();

  if (!drag)
        return;

  if (drag->drag() && drag->target() != viewport())
    emit moved();
}

QDragObject *KListView::dragObject()
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

QPtrList<QListViewItem> KListView::selectedItems() const
{
  return selectedItems(true);
}

QPtrList<QListViewItem> KListView::selectedItems(bool includeHiddenItems) const
{
  QPtrList<QListViewItem> list;

  // Using selectionMode() instead of selectionModeExt() since for the cases that
  // we're interested in selectionMode() should work for either variety of the
  // setSelectionMode().

  switch(selectionMode())
  {
  case NoSelection:
      break;
  case Single:
      if(selectedItem() && (includeHiddenItems || selectedItem()->isVisible()))
          list.append(selectedItem());
      break;
  default:
  {
      int flags = QListViewItemIterator::Selected;
      if (!includeHiddenItems)
      {
        flags |= QListViewItemIterator::Visible;
      }

      QListViewItemIterator it(const_cast<KListView *>(this), flags);

      for(; it.current(); ++it)
          list.append(it.current());

      break;
  }
  }

  return list;
}


void KListView::moveItem(QListViewItem *item, QListViewItem *parent, QListViewItem *after)
{
  // sanity check - don't move a item into its own child structure
  QListViewItem *i = parent;
  while(i)
    {
      if(i == item)
        return;
      i = i->parent();
    }

  if (after)
  {
      item->moveItem(after);
      return;
  }

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
}

void KListView::contentsDragEnterEvent(QDragEnterEvent *event)
{
  if (acceptDrag (event))
    event->accept();
}

void KListView::setDropVisualizerWidth (int w)
{
  d->mDropVisualizerWidth = w > 0 ? w : 1;
}

QRect KListView::drawDropVisualizer(QPainter *p, QListViewItem *parent,
                                    QListViewItem *after)
{
    QRect insertmarker;

    if (!after && !parent)
        insertmarker = QRect (0, 0, viewport()->width(), d->mDropVisualizerWidth/2);
    else
    {
        int level = 0;
        if (after)
        {
            QListViewItem* it = 0L;
            if (after->isOpen())
            {
                // Look for the last child (recursively)
                it = after->firstChild();
                if (it)
                    while (it->nextSibling() || it->firstChild())
                        if ( it->nextSibling() )
                            it = it->nextSibling();
                        else
                            it = it->firstChild();
            }

            insertmarker = itemRect (it ? it : after);
            level = after->depth();
        }
        else if (parent)
        {
            insertmarker = itemRect (parent);
            level = parent->depth() + 1;
        }
        insertmarker.setLeft( treeStepSize() * ( level + (rootIsDecorated() ? 1 : 0) ) + itemMargin() );
        insertmarker.setRight (viewport()->width());
        insertmarker.setTop (insertmarker.bottom() - d->mDropVisualizerWidth/2 + 1);
        insertmarker.setBottom (insertmarker.bottom() + d->mDropVisualizerWidth/2);
    }

    // This is not used anymore, at least by KListView itself (see viewportPaintEvent)
    // Remove for KDE 4.0.
    if (p)
        p->fillRect(insertmarker, Dense4Pattern);

    return insertmarker;
}

QRect KListView::drawItemHighlighter(QPainter *painter, QListViewItem *item)
{
  QRect r;

  if (item)
  {
    r = itemRect(item);
    r.setLeft(r.left()+(item->depth()+(rootIsDecorated() ? 1 : 0))*treeStepSize());
    if (painter)
      style().drawPrimitive(QStyle::PE_FocusRect, painter, r, colorGroup(),
                            QStyle::Style_FocusAtBorder, colorGroup().highlight());
  }

  return r;
}

void KListView::cleanItemHighlighter ()
{
  if (d->mOldDropHighlighter.isValid())
  {
    QRect rect=d->mOldDropHighlighter;
    d->mOldDropHighlighter = QRect();
    viewport()->repaint(rect, true);
  }
}

void KListView::rename(QListViewItem *item, int c)
{
  if (d->renameable.contains(c))
  {
    ensureItemVisible(item);
    d->editor->load(item,c);
  }
}

bool KListView::isRenameable (int col) const
{
  return d->renameable.contains(col);
}

void KListView::setRenameable (int col, bool renameable)
{
  if (col>=header()->count()) return;

  d->renameable.remove(col);
  if (renameable)
    d->renameable+=col;
}

void KListView::doneEditing(QListViewItem *item, int row)
{
  emit itemRenamed(item, item->text(row), row);
  emit itemRenamed(item);
}

bool KListView::acceptDrag(QDropEvent* e) const
{
  return acceptDrops() && itemsMovable() && (e->source()==viewport());
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
        return ((column==tooltipColumn()) && !tooltip(item, column).isEmpty());
}

QString KListView::tooltip(QListViewItem *item, int column) const
{
        return item->text(column);
}

void KListView::setTabOrderedRenaming(bool b)
{
	d->tabRename = b;
}

bool KListView::tabOrderedRenaming() const
{
	return d->tabRename;
}

void KListView::keyPressEvent (QKeyEvent* e)
{
  //don't we need a contextMenuModifier too ? (aleXXX)
  if (e->key() == d->contextMenuKey)
        {
          emit menuShortCutPressed (this, currentItem());
          return;
        }

  if (d->selectionMode != FileManager)
        QListView::keyPressEvent (e);
  else
        fileManagerKeyPressEvent (e);
}

void KListView::activateAutomaticSelection()
{
   d->selectedBySimpleMove=true;
   d->selectedUsingMouse=false;
   if (currentItem())
   {
      currentItem()->setSelected(true);
      currentItem()->repaint();
      emit selectionChanged();
   };
}

void KListView::deactivateAutomaticSelection()
{
   d->selectedBySimpleMove=false;
}

bool KListView::automaticSelection() const
{
   return d->selectedBySimpleMove;
}

void KListView::fileManagerKeyPressEvent (QKeyEvent* e)
{
   //don't care whether it's on the keypad or not
    int e_state=(e->state() & ~Keypad);

    int oldSelectionDirection(d->selectionDirection);

    if ((e->key()!=Key_Shift) && (e->key()!=Key_Control)
        && (e->key()!=Key_Meta) && (e->key()!=Key_Alt))
    {
       if ((e_state==ShiftButton) && (!d->wasShiftEvent) && (!d->selectedBySimpleMove))
          selectAll(false);
       d->selectionDirection=0;
       d->wasShiftEvent = (e_state == ShiftButton);
    };

    //d->wasShiftEvent = (e_state == ShiftButton);


    QListViewItem* item = currentItem();
    if (!item) return;

    QListViewItem* repaintItem1 = item;
    QListViewItem* repaintItem2 = 0L;
    QListViewItem* visItem = 0L;

    QListViewItem* nextItem = 0L;
    int items = 0;

    bool shiftOrCtrl((e_state==ControlButton) || (e_state==ShiftButton));
    int selectedItems(0);
    for (QListViewItem *tmpItem=firstChild(); tmpItem; tmpItem=tmpItem->nextSibling())
       if (tmpItem->isSelected()) selectedItems++;

    if (((!selectedItems) || ((selectedItems==1) && (d->selectedUsingMouse)))
        && (e_state==NoButton)
        && ((e->key()==Key_Down)
        || (e->key()==Key_Up)
        || (e->key()==Key_Next)
        || (e->key()==Key_Prior)
        || (e->key()==Key_Home)
        || (e->key()==Key_End)))
    {
       d->selectedBySimpleMove=true;
       d->selectedUsingMouse=false;
    }
    else if (selectedItems>1)
       d->selectedBySimpleMove=false;

    bool emitSelectionChanged(false);

    switch (e->key())
    {
    case Key_Escape:
       selectAll(false);
       emitSelectionChanged=true;
       break;

    case Key_Space:
       //toggle selection of current item
       if (d->selectedBySimpleMove)
          d->selectedBySimpleMove=false;
       item->setSelected(!item->isSelected());
       emitSelectionChanged=true;
       break;

    case Key_Insert:
       //toggle selection of current item and move to the next item
       if (d->selectedBySimpleMove)
       {
          d->selectedBySimpleMove=false;
          if (!item->isSelected()) item->setSelected(true);
       }
       else
       {
          item->setSelected(!item->isSelected());
       };

       nextItem=item->itemBelow();

       if (nextItem)
       {
          repaintItem2=nextItem;
          visItem=nextItem;
          setCurrentItem(nextItem);
       };
       d->selectionDirection=1;
       emitSelectionChanged=true;
       break;

    case Key_Down:
       nextItem=item->itemBelow();
       //toggle selection of current item and move to the next item
       if (shiftOrCtrl)
       {
          d->selectionDirection=1;
          if (d->selectedBySimpleMove)
             d->selectedBySimpleMove=false;
          else
          {
             if (oldSelectionDirection!=-1)
             {
                item->setSelected(!item->isSelected());
                emitSelectionChanged=true;
             };
          };
       }
       else if ((d->selectedBySimpleMove) && (nextItem))
       {
          item->setSelected(false);
          emitSelectionChanged=true;
       };

       if (nextItem)
       {
          if (d->selectedBySimpleMove)
             nextItem->setSelected(true);
          repaintItem2=nextItem;
          visItem=nextItem;
          setCurrentItem(nextItem);
       };
       break;

    case Key_Up:
       nextItem=item->itemAbove();
       d->selectionDirection=-1;
       //move to the prev. item and toggle selection of this one
       // => No, can't select the last item, with this. For symmetry, let's
       // toggle selection and THEN move up, just like we do in down (David)
       if (shiftOrCtrl)
       {
          if (d->selectedBySimpleMove)
             d->selectedBySimpleMove=false;
          else
          {
             if (oldSelectionDirection!=1)
             {
                item->setSelected(!item->isSelected());
                emitSelectionChanged=true;
             };
          }
       }
       else if ((d->selectedBySimpleMove) && (nextItem))
       {
          item->setSelected(false);
          emitSelectionChanged=true;
       };

       if (nextItem)
       {
          if (d->selectedBySimpleMove)
             nextItem->setSelected(true);
          repaintItem2=nextItem;
          visItem=nextItem;
          setCurrentItem(nextItem);
       };
       break;

    case Key_End:
       //move to the last item and toggle selection of all items inbetween
       nextItem=item;
       if (d->selectedBySimpleMove)
          item->setSelected(false);
       if (shiftOrCtrl)
          d->selectedBySimpleMove=false;

       while(nextItem)
       {
          if (shiftOrCtrl)
             nextItem->setSelected(!nextItem->isSelected());
          if (!nextItem->itemBelow())
          {
             if (d->selectedBySimpleMove)
                nextItem->setSelected(true);
             repaintItem2=nextItem;
             visItem=nextItem;
             setCurrentItem(nextItem);
          }
          nextItem=nextItem->itemBelow();
       }
       emitSelectionChanged=true;
       break;

    case Key_Home:
       // move to the first item and toggle selection of all items inbetween
       nextItem = firstChild();
       visItem = nextItem;
       repaintItem2 = visItem;
       if (d->selectedBySimpleMove)
          item->setSelected(false);
       if (shiftOrCtrl)
       {
          d->selectedBySimpleMove=false;

          while ( nextItem != item )
          {
             nextItem->setSelected( !nextItem->isSelected() );
             nextItem = nextItem->itemBelow();
          }
          item->setSelected( !item->isSelected() );
       }
       setCurrentItem( firstChild() );
       emitSelectionChanged=true;
       break;

    case Key_Next:
       items=visibleHeight()/item->height();
       nextItem=item;
       if (d->selectedBySimpleMove)
          item->setSelected(false);
       if (shiftOrCtrl)
       {
          d->selectedBySimpleMove=false;
          d->selectionDirection=1;
       };

       for (int i=0; i<items; i++)
       {
          if (shiftOrCtrl)
             nextItem->setSelected(!nextItem->isSelected());
          //the end
          if ((i==items-1) || (!nextItem->itemBelow()))

          {
             if (shiftOrCtrl)
                nextItem->setSelected(!nextItem->isSelected());
             if (d->selectedBySimpleMove)
                nextItem->setSelected(true);
             ensureItemVisible(nextItem);
             setCurrentItem(nextItem);
             update();
             if ((shiftOrCtrl) || (d->selectedBySimpleMove))
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
       if (d->selectedBySimpleMove)
          item->setSelected(false);
       if (shiftOrCtrl)
       {
          d->selectionDirection=-1;
          d->selectedBySimpleMove=false;
       };

       for (int i=0; i<items; i++)
       {
          if ((nextItem!=item) &&(shiftOrCtrl))
             nextItem->setSelected(!nextItem->isSelected());
          //the end
          if ((i==items-1) || (!nextItem->itemAbove()))

          {
             if (d->selectedBySimpleMove)
                nextItem->setSelected(true);
             ensureItemVisible(nextItem);
             setCurrentItem(nextItem);
             update();
             if ((shiftOrCtrl) || (d->selectedBySimpleMove))
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
          setOpen( item, false );
       break;
    case Key_Plus:
       if (  !item->isOpen() && (item->isExpandable() || item->childCount()) )
          setOpen( item, true );
       break;
    default:
       bool realKey = ((e->key()!=Key_Shift) && (e->key()!=Key_Control)
                        && (e->key()!=Key_Meta) && (e->key()!=Key_Alt));

       bool selectCurrentItem = (d->selectedBySimpleMove) && (item->isSelected());
       if (realKey && selectCurrentItem)
          item->setSelected(false);
       //this is mainly for the "goto filename beginning with pressed char" feature (aleXXX)
       QListView::SelectionMode oldSelectionMode = selectionMode();
       setSelectionMode (QListView::Multi);
       QListView::keyPressEvent (e);
       setSelectionMode (oldSelectionMode);
       if (realKey && selectCurrentItem)
       {
          currentItem()->setSelected(true);
          emitSelectionChanged=true;
       }
       repaintItem2=currentItem();
       if (realKey)
          visItem=currentItem();
       break;
    }

    if (visItem)
       ensureItemVisible(visItem);

    QRect ir;
    if (repaintItem1)
       ir = ir.unite( itemRect(repaintItem1) );
    if (repaintItem2)
       ir = ir.unite( itemRect(repaintItem2) );

    if ( !ir.isEmpty() )
    {                 // rectangle to be repainted
       if ( ir.x() < 0 )
          ir.moveBy( -ir.x(), 0 );
       viewport()->repaint( ir, false );
    }
    /*if (repaintItem1)
       repaintItem1->repaint();
    if (repaintItem2)
       repaintItem2->repaint();*/
    update();
    if (emitSelectionChanged)
       emit selectionChanged();
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

    case FileManager:
        setSelectionMode (QListView::Extended);
        break;

    default:
        kdWarning () << "Warning: illegal selection mode " << int(mode) << " set!" << endl;
        break;
    }
}

KListView::SelectionModeExt KListView::selectionModeExt () const
{
  return d->selectionMode;
}

int KListView::itemIndex( const QListViewItem *item ) const
{
    if ( !item )
        return -1;

    if ( item == firstChild() )
        return 0;
    else {
        QListViewItemIterator it(firstChild());
        uint j = 0;
        for (; it.current() && it.current() != item; ++it, ++j );

        if( !it.current() )
          return -1;

        return j;
    }
}

QListViewItem* KListView::itemAtIndex(int index)
{
   if (index<0)
      return 0;

   int j(0);
   for (QListViewItemIterator it=firstChild(); it.current(); ++it)
   {
      if (j==index)
         return it.current();
      ++j;
   };
   return 0;
}


void KListView::emitContextMenu (KListView*, QListViewItem* i)
{
  QPoint p;

  if (i)
        p = viewport()->mapToGlobal(itemRect(i).center());
  else
        p = mapToGlobal(rect().center());

  emit contextMenu (this, i, p);
}

void KListView::emitContextMenu (QListViewItem* i, const QPoint& p, int)
{
  emit contextMenu (this, i, p);
}

void KListView::setAcceptDrops (bool val)
{
  QListView::setAcceptDrops (val);
  viewport()->setAcceptDrops (val);
}

int KListView::dropVisualizerWidth () const
{
        return d->mDropVisualizerWidth;
}


void KListView::viewportPaintEvent(QPaintEvent *e)
{
  d->paintAbove = 0;
  d->paintCurrent = 0;
  d->paintBelow = 0;
  d->painting = true;

  QListView::viewportPaintEvent(e);

  if (d->mOldDropVisualizer.isValid() && e->rect().intersects(d->mOldDropVisualizer))
    {
      QPainter painter(viewport());

      // This is where we actually draw the drop-visualizer
      painter.fillRect(d->mOldDropVisualizer, Dense4Pattern);
    }
  if (d->mOldDropHighlighter.isValid() && e->rect().intersects(d->mOldDropHighlighter))
    {
      QPainter painter(viewport());

      // This is where we actually draw the drop-highlighter
      style().drawPrimitive(QStyle::PE_FocusRect, &painter, d->mOldDropHighlighter, colorGroup(),
                            QStyle::Style_FocusAtBorder);
    }
  d->painting = false;
}

void KListView::setFullWidth()
{
  setFullWidth(true);
}

void KListView::setFullWidth(bool fullWidth)
{
  d->fullWidth = fullWidth;
  header()->setStretchEnabled(fullWidth, columns()-1);
}

bool KListView::fullWidth() const
{
  return d->fullWidth;
}

int KListView::addColumn(const QString& label, int width)
{
  int result = QListView::addColumn(label, width);
  if (d->fullWidth) {
    header()->setStretchEnabled(false, columns()-2);
    header()->setStretchEnabled(true, columns()-1);
  }
  return result;
}

int KListView::addColumn(const QIconSet& iconset, const QString& label, int width)
{
  int result = QListView::addColumn(iconset, label, width);
  if (d->fullWidth) {
    header()->setStretchEnabled(false, columns()-2);
    header()->setStretchEnabled(true, columns()-1);
  }
  return result;
}

void KListView::removeColumn(int index)
{
  QListView::removeColumn(index);
  if (d->fullWidth && index == columns()) header()->setStretchEnabled(true, columns()-1);
}

void KListView::viewportResizeEvent(QResizeEvent* e)
{
  QListView::viewportResizeEvent(e);
}

const QColor &KListView::alternateBackground() const
{
  return d->alternateBackground;
}

void KListView::setAlternateBackground(const QColor &c)
{
  d->alternateBackground = c;
  repaint();
}

void KListView::setShadeSortColumn(bool shadeSortColumn)
{
  d->shadeSortColumn = shadeSortColumn;
  repaint();
}

bool KListView::shadeSortColumn() const
{
  return d->shadeSortColumn;
}

void KListView::saveLayout(KConfig *config, const QString &group) const
{
  KConfigGroupSaver saver(config, group);
  QStringList widths, order;

  const int colCount = columns();
  QHeader* const thisHeader = header();
  for (int i = 0; i < colCount; ++i)
  {
    widths << QString::number(columnWidth(i));
    order << QString::number(thisHeader->mapToIndex(i));
  }
  config->writeEntry("ColumnWidths", widths);
  config->writeEntry("ColumnOrder", order);
  config->writeEntry("SortColumn", d->sortColumn);
  config->writeEntry("SortAscending", d->sortAscending);
}

void KListView::restoreLayout(KConfig *config, const QString &group)
{
  KConfigGroupSaver saver(config, group);
  QStringList cols = config->readListEntry("ColumnWidths");
  int i = 0;
  { // scope the iterators
    QStringList::ConstIterator it = cols.constBegin();
    const QStringList::ConstIterator itEnd = cols.constEnd();
    for (; it != itEnd; ++it)
      setColumnWidth(i++, (*it).toInt());
  }

  // move sections in the correct sequence: from lowest to highest index position
  // otherwise we move a section from an index, which modifies
  // all index numbers to the right of the moved one
  cols = config->readListEntry("ColumnOrder");
  const int colCount = columns();
  for (i = 0; i < colCount; ++i)   // final index positions from lowest to highest
  {
    QStringList::ConstIterator it = cols.constBegin();
    const QStringList::ConstIterator itEnd = cols.constEnd();

    int section = 0;
    for (; (it != itEnd) && ((*it).toInt() != i); ++it, ++section) ;

    if ( it != itEnd ) {
      // found the section to move to position i
      header()->moveSection(section, i);
    }
  }

  if (config->hasKey("SortColumn"))
    setSorting(config->readNumEntry("SortColumn"), config->readBoolEntry("SortAscending", true));
}

void KListView::setSorting(int column, bool ascending)
{
  QListViewItem *selected = 0;

  if (selectionMode() == QListView::Single) {
    selected = selectedItem();
    if (selected && !selected->isVisible())
      selected = 0;
  }
  else if (selectionMode() != QListView::NoSelection) {
    QListViewItem *item = firstChild();
    while (item && !selected) {
      if (item->isSelected() && item->isVisible())
	selected = item;
      item = item->itemBelow();
    }
  }

  d->sortColumn = column;
  d->sortAscending = ascending;
  QListView::setSorting(column, ascending);

  if (selected)
    ensureItemVisible(selected);

  QListViewItem* item = firstChild();
  while ( item ) {
    KListViewItem *kItem = dynamic_cast<KListViewItem*>(item);
    if (kItem) kItem->m_known = false;
    item = item->itemBelow();
  }
}

int KListView::columnSorted(void) const
{
  return d->sortColumn;
}

bool KListView::ascendingSort(void) const
{
  return d->sortAscending;
}

void KListView::takeItem(QListViewItem *item)
{
  if(item && item == d->editor->currentItem())
    d->editor->terminate();

  QListView::takeItem(item);
}

void KListView::disableAutoSelection()
{
  if ( d->disableAutoSelection )
    return;

  d->disableAutoSelection = true;
  d->autoSelect.stop();
  d->autoSelectDelay = -1;
}

void KListView::resetAutoSelection()
{
  if ( !d->disableAutoSelection )
    return;

  d->disableAutoSelection = false;
  d->autoSelectDelay = KGlobalSettings::autoSelectDelay();
}

void KListView::doubleClicked( QListViewItem *item, const QPoint &pos, int c )
{
  emit QListView::doubleClicked( item, pos, c );
}

KListViewItem::KListViewItem(QListView *parent)
  : QListViewItem(parent)
{
  init();
}

KListViewItem::KListViewItem(QListViewItem *parent)
  : QListViewItem(parent)
{
  init();
}

KListViewItem::KListViewItem(QListView *parent, QListViewItem *after)
  : QListViewItem(parent, after)
{
  init();
}

KListViewItem::KListViewItem(QListViewItem *parent, QListViewItem *after)
  : QListViewItem(parent, after)
{
  init();
}

KListViewItem::KListViewItem(QListView *parent,
    QString label1, QString label2, QString label3, QString label4,
    QString label5, QString label6, QString label7, QString label8)
  : QListViewItem(parent, label1, label2, label3, label4, label5, label6, label7, label8)
{
  init();
}

KListViewItem::KListViewItem(QListViewItem *parent,
    QString label1, QString label2, QString label3, QString label4,
    QString label5, QString label6, QString label7, QString label8)
  : QListViewItem(parent, label1, label2, label3, label4, label5, label6, label7, label8)
{
  init();
}

KListViewItem::KListViewItem(QListView *parent, QListViewItem *after,
    QString label1, QString label2, QString label3, QString label4,
    QString label5, QString label6, QString label7, QString label8)
  : QListViewItem(parent, after, label1, label2, label3, label4, label5, label6, label7, label8)
{
  init();
}

KListViewItem::KListViewItem(QListViewItem *parent, QListViewItem *after,
    QString label1, QString label2, QString label3, QString label4,
    QString label5, QString label6, QString label7, QString label8)
  : QListViewItem(parent, after, label1, label2, label3, label4, label5, label6, label7, label8)
{
  init();
}

KListViewItem::~KListViewItem()
{
  if(listView())
    emit static_cast<KListView *>(listView())->itemRemoved(this);
}

void KListViewItem::init()
{
  m_odd = m_known = false;
  KListView *lv = static_cast<KListView *>(listView());
  setDragEnabled( dragEnabled() || lv->dragEnabled() );
  emit lv->itemAdded(this);
}

void KListViewItem::insertItem(QListViewItem *item)
{
  QListViewItem::insertItem(item);
  if(listView())
    emit static_cast<KListView *>(listView())->itemAdded(item);
}

void KListViewItem::takeItem(QListViewItem *item)
{
  QListViewItem::takeItem(item);
  if(listView())
    emit static_cast<KListView *>(listView())->itemRemoved(item);
}

const QColor &KListViewItem::backgroundColor()
{
  if (isAlternate())
    return static_cast< KListView* >(listView())->alternateBackground();
  return listView()->viewport()->colorGroup().base();
}

QColor KListViewItem::backgroundColor(int column)
{
  KListView* view = static_cast< KListView* >(listView());
  QColor color = isAlternate() ?
                 view->alternateBackground() :
                 view->viewport()->colorGroup().base();

  // calculate a different color if the current column is sorted (only if more than 1 column)
  if ( (view->columns() > 1) && view->shadeSortColumn() && (column == view->columnSorted()) )
  {
    if ( color == Qt::black )
      color = QColor(55, 55, 55);  // dark gray
    else
    {
      int h,s,v;
      color.hsv(&h, &s, &v);
      if ( v > 175 )
        color = color.dark(104);
      else
        color = color.light(120);
    }
  }

  return color;
}

bool KListViewItem::isAlternate()
{
  KListView* const lv = static_cast<KListView *>(listView());
  if (lv && lv->alternateBackground().isValid())
  {
    KListViewItem *above;

    KListView::KListViewPrivate* const lvD = lv->d;

    // Ok, there's some weirdness here that requires explanation as this is a
    // speed hack.  itemAbove() is a O(n) operation (though this isn't
    // immediately clear) so we want to call it as infrequently as possible --
    // especially in the case of painting a cell.
    //
    // So, in the case that we *are* painting a cell:  (1) we're assuming that
    // said painting is happening top to bottem -- this assumption is present
    // elsewhere in the implementation of this class, (2) itemBelow() is fast --
    // roughly constant time.
    //
    // Given these assumptions we can do a mixture of caching and telling the
    // next item that the when that item is the current item that the now
    // current item will be the item above it.
    //
    // Ideally this will make checking to see if the item above the current item
    // is the alternate color a constant time operation rather than 0(n).

    if (lvD->painting) {
      if (lvD->paintCurrent != this)
      {
        lvD->paintAbove = lvD->paintBelow == this ? lvD->paintCurrent : itemAbove();
        lvD->paintCurrent = this;
        lvD->paintBelow = itemBelow();
      }

      above = dynamic_cast<KListViewItem *>(lvD->paintAbove);
    }
    else
    {
      above = dynamic_cast<KListViewItem *>(itemAbove());
    }

    m_known = above ? above->m_known : true;
    if (m_known)
    {
       m_odd = above ? !above->m_odd : false;
    }
    else
    {
       KListViewItem *item;
       bool previous = true;
       if (parent())
       {
          item = dynamic_cast<KListViewItem *>(parent());
          if (item)
             previous = item->m_odd;
          item = dynamic_cast<KListViewItem *>(parent()->firstChild());
       }
       else
       {
          item = dynamic_cast<KListViewItem *>(lv->firstChild());
       }

       while(item)
       {
          item->m_odd = previous = !previous;
          item->m_known = true;
          item = dynamic_cast<KListViewItem *>(item->nextSibling());
       }
    }
    return m_odd;
  }
  return false;
}

void KListViewItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
{
  QColorGroup _cg = cg;
  const QPixmap *pm = listView()->viewport()->backgroundPixmap();

  if (pm && !pm->isNull())
  {
    _cg.setBrush(QColorGroup::Base, QBrush(backgroundColor(column), *pm));
    QPoint o = p->brushOrigin();
    p->setBrushOrigin( o.x()-listView()->contentsX(), o.y()-listView()->contentsY() );
  }
  else
  {
    _cg.setColor((listView()->viewport()->backgroundMode() == Qt::FixedColor) ?
                 QColorGroup::Background : QColorGroup::Base,
                 backgroundColor(column));
  }
  QListViewItem::paintCell(p, _cg, column, width, alignment);
}

void KListView::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "klistview.moc"
#include "klistviewlineedit.moc"

// vim: noet
