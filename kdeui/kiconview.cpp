#include <qtimer.h>

#include "kiconview.h"
#include <kglobalsettings.h>
#include <kcursor.h>

#include <X11/Xlib.h>

KIconView::KIconView( QWidget *parent, const char *name, WFlags f )
    : QIconView( parent, name, f )
{
    //initializing so that checkSettings() actually does something
    m_bUseSingle = !KGlobalSettings::singleClick();
    checkSettings();
    oldCursor = viewport()->cursor();
    m_bChangeCursorOverItem = true;
    connect( this, SIGNAL( onViewport() ),
	     this, SLOT( slotOnViewport() ) );
    connect( this, SIGNAL( onItem( QIconViewItem * ) ),
             this, SLOT( slotOnItem( QIconViewItem * ) ) );
    checkSettings();

    connect( this, SIGNAL( executed( QIconViewItem * ) ),
	     this, SLOT( slotExecute( QIconViewItem * ) ) );

    m_pCurrentItem = 0L;

    m_pAutoSelect = new QTimer( this );
    connect( m_pAutoSelect, SIGNAL( timeout() ),
    	     this, SLOT( slotAutoSelect() ) );
}

void KIconView::checkSettings()
{
  if( m_bUseSingle != KGlobalSettings::singleClick() ) {
    m_bUseSingle = KGlobalSettings::singleClick();
    if( m_bUseSingle )
      connect( this, SIGNAL( clicked( QIconViewItem * ) ),
	       this, SLOT( slotExecute( QIconViewItem * ) ) );
    else
      connect( this, SIGNAL( doubleClicked( QIconViewItem * ) ),
	       this, SLOT( slotExecute( QIconViewItem * ) ) );
  }
  
  m_bChangeCursorOverItem = KGlobalSettings::changeCursorOverIcon();
  m_autoSelectDelay = KGlobalSettings::autoSelectDelay();

  if( !m_bUseSingle || !m_bChangeCursorOverItem )
    viewport()->setCursor( oldCursor );
}

void KIconView::slotOnItem( QIconViewItem *item )
{
    checkSettings();
    if ( item && m_bChangeCursorOverItem && m_bUseSingle )
        viewport()->setCursor( KCursor().handCursor() );

    if ( item && (m_autoSelectDelay > -1) && m_bUseSingle ) {
      m_pAutoSelect->start( m_autoSelectDelay, true ); 
      m_pCurrentItem = item;
    }
}

void KIconView::slotOnViewport()
{
    checkSettings();
    if ( m_bChangeCursorOverItem )
        viewport()->setCursor( oldCursor );

    m_pAutoSelect->stop();
    m_pCurrentItem = 0L;
}

void KIconView::slotAutoSelect()
{
  Window root;
  Window child;
  int root_x, root_y, win_x, win_y;
  uint keybstate;
  XQueryPointer( qt_xdisplay(), qt_xrootwin(), &root, &child,
		 &root_x, &root_y, &win_x, &win_y, &keybstate );

  if( m_pCurrentItem ) {
    //Shift pressed?
    if( (keybstate & ShiftMask) ) {
      //No Ctrl? Then clear before!
      if( !(keybstate & ControlMask) )  
	clearSelection();

      //Temporary implementaion of the selection until QIconView supports it
      bool select = !m_pCurrentItem->isSelected();
      bool block = signalsBlocked();
      blockSignals( true );
      viewport()->setUpdatesEnabled( FALSE );

      //Calculate the smallest rectangle that contains the current Item 
      //and the one that got the autoselect event
      QRect r;
      QRect redraw;
      if ( currentItem() )
	r = QRect( QMIN( currentItem()->x(), m_pCurrentItem->x() ),
		   QMIN( currentItem()->y(), m_pCurrentItem->y() ),
		   0, 0 );
      else
	r = QRect( 0, 0, 0, 0 );
      if ( currentItem()->x() < m_pCurrentItem->x() )
	r.setWidth( m_pCurrentItem->x() - currentItem()->x() + m_pCurrentItem->width() );
      else
	r.setWidth( currentItem()->x() - m_pCurrentItem->x() + currentItem()->width() );
      if ( currentItem()->y() < m_pCurrentItem->y() )
	r.setHeight( m_pCurrentItem->y() - currentItem()->y() + m_pCurrentItem->height() );
      else
	r.setHeight( currentItem()->y() - m_pCurrentItem->y() + currentItem()->height() );
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
      viewport()->setUpdatesEnabled( true );
      repaintContents( redraw, false );
      emit selectionChanged();
      //setSelected( m_pCurrentItem, true, (keybstate & ControlMask), (keybstate & ShiftMask) );
    }
    else if( (keybstate & ControlMask) ) 
      setSelected( m_pCurrentItem, !m_pCurrentItem->isSelected(), true );
    else 
      setSelected( m_pCurrentItem, true );
  }
  else
    kdDebug() << "That´s not supposed to happen!!!!" << endl;

  setCurrentItem( m_pCurrentItem );
}

void KIconView::slotExecute( QIconViewItem *item )
{
  m_pAutoSelect->stop();
  
  emit executed( item );
}
