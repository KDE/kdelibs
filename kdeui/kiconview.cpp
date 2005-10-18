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

#include "config.h"

#include <qtimer.h>
#include <qpainter.h>
#include <qpixmapcache.h>
#include <qevent.h>
#include <q3cleanuphandler.h>

#include "kiconview.h"
#include "kwordwrap.h"
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kapplication.h>
#include <kipc.h> 

#include <kcursor.h>
#include <kpixmap.h>
#include <kpixmapeffect.h>

class KIconView::KIconViewPrivate
{
public:
    KIconViewPrivate() {
        mode = KIconView::Execute;
        fm = 0L;
        doAutoSelect = true;
        textHeight = 0;
        dragHoldItem = 0L;
    }
    KIconView::Mode mode;
    bool doAutoSelect;
    QFontMetrics *fm;
    QPixmapCache maskCache;
    int textHeight;
    Q3IconViewItem *dragHoldItem;
    QTimer dragHoldTimer;
    QTimer doubleClickIgnoreTimer;
};

KIconView::KIconView( QWidget *parent, const char *name, Qt::WFlags f )
    : Q3IconView( parent, name, f )
{
    d = new KIconViewPrivate;

    connect( this, SIGNAL( onViewport() ),
             this, SLOT( slotOnViewport() ) );
    connect( this, SIGNAL( onItem( Q3IconViewItem * ) ),
             this, SLOT( slotOnItem( Q3IconViewItem * ) ) );
    slotSettingsChanged( KApplication::SETTINGS_MOUSE );
    if ( kapp ) { // maybe null when used inside designer
        connect( kapp, SIGNAL( settingsChanged(int) ), SLOT( slotSettingsChanged(int) ) );
        kapp->addKipcEventMask( KIPC::SettingsChanged );
    }

    m_pCurrentItem = 0L;

    m_pAutoSelect = new QTimer( this );
    connect( m_pAutoSelect, SIGNAL( timeout() ),
             this, SLOT( slotAutoSelect() ) );

    connect( &d->dragHoldTimer, SIGNAL(timeout()), this, SLOT(slotDragHoldTimeout()) );
}

KIconView::~KIconView()
{
    delete d->fm;
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

void KIconView::slotOnItem( Q3IconViewItem *item )
{
    if ( item ) {
        if ( m_bUseSingle ) {
            if ( m_bChangeCursorOverItem )
                viewport()->setCursor( KCursor().handCursor() );

            if ( (m_autoSelectDelay > -1) ) {
                m_pAutoSelect->start( m_autoSelectDelay, true );
            }
        }
        m_pCurrentItem = item;
    }
}

void KIconView::slotOnViewport()
{
    if ( m_bUseSingle && m_bChangeCursorOverItem )
        viewport()->unsetCursor();

    m_pAutoSelect->stop();
    m_pCurrentItem = 0L;
}

void KIconView::slotSettingsChanged(int category)
{
    if ( category != KApplication::SETTINGS_MOUSE )
      return;
    m_bUseSingle = KGlobalSettings::singleClick();
    //kdDebug() << "KIconView::slotSettingsChanged for mouse, usesingle=" << m_bUseSingle << endl;

    disconnect( this, SIGNAL( mouseButtonClicked( int, Q3IconViewItem *,
						  const QPoint & ) ),
		this, SLOT( slotMouseButtonClicked( int, Q3IconViewItem *,
						    const QPoint & ) ) );
//         disconnect( this, SIGNAL( doubleClicked( QIconViewItem *,
// 						 const QPoint & ) ),
// 		    this, SLOT( slotExecute( QIconViewItem *,
// 					     const QPoint & ) ) );

    if( m_bUseSingle ) {
      connect( this, SIGNAL( mouseButtonClicked( int, Q3IconViewItem *,
						 const QPoint & ) ),
	       this, SLOT( slotMouseButtonClicked( int, Q3IconViewItem *,
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
        viewport()->unsetCursor();
}

void KIconView::slotAutoSelect()
{
  // check that the item still exists
  if( index( m_pCurrentItem ) == -1 || !d->doAutoSelect )
    return;

  //Give this widget the keyboard focus.
  if( !hasFocus() )
    setFocus();

  Qt::ButtonState keybstate = QApplication::keyboardModifiers();
  Q3IconViewItem* previousItem = currentItem();
  setCurrentItem( m_pCurrentItem );

  if( m_pCurrentItem ) {
    //Shift pressed?
    if( (keybstate & Qt::ShiftModifier) ) {
      //Temporary implementation of the selection until QIconView supports it
      bool block = signalsBlocked();
      blockSignals( true );

      //No Ctrl? Then clear before!
      if( !(keybstate & Qt::ControlModifier) )
	clearSelection();

      bool select = !m_pCurrentItem->isSelected();
      bool update = viewport()->isUpdatesEnabled();
      viewport()->setUpdatesEnabled( false );

      //Calculate the smallest rectangle that contains the current Item
      //and the one that got the autoselect event
      QRect r;
      QRect redraw;
      if ( previousItem )
	r = QRect( qMin( previousItem->x(), m_pCurrentItem->x() ),
		   qMin( previousItem->y(), m_pCurrentItem->y() ),
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
      r = r.normalized();

      //Check for each item whether it is within the rectangle.
      //If yes, select it
      for( Q3IconViewItem* i = firstItem(); i; i = i->nextItem() ) {
	if( i->intersects( r ) ) {
	  redraw = redraw.unite( i->rect() );
	  setSelected( i, select, true );
	}
      }

      blockSignals( block );
      viewport()->setUpdatesEnabled( update );
      repaintContents( redraw, false );

      emit selectionChanged();

      if( selectionMode() == Q3IconView::Single )
	emit selectionChanged( m_pCurrentItem );

      //setSelected( m_pCurrentItem, true, (keybstate & ControlButton), (keybstate & ShiftButton) );
    }
    else if( (keybstate & Qt::ControlModifier) )
      setSelected( m_pCurrentItem, !m_pCurrentItem->isSelected(), true );
    else
      setSelected( m_pCurrentItem, true );
  }
  else
    kdDebug() << "KIconView: That's not supposed to happen!!!!" << endl;
}

void KIconView::emitExecute( Q3IconViewItem *item, const QPoint &pos )
{
  if ( d->mode != Execute )
  {
    // kdDebug() << "KIconView::emitExecute : not in execute mode !" << endl;
    return;
  }

  Qt::ButtonState keybstate = QApplication::keyboardModifiers();

  m_pAutoSelect->stop();

  //Don´t emit executed if in SC mode and Shift or Ctrl are pressed
  if( !( m_bUseSingle && ((keybstate & Qt::ShiftModifier) || (keybstate & Qt::ControlModifier)) ) ) {
    setSelected( item, false );
    viewport()->unsetCursor();
    emit executed( item );
    emit executed( item, pos );
  }
}

void KIconView::updateDragHoldItem( QDropEvent *e )
{
  Q3IconViewItem *item = findItem( e->pos() );

  if ( d->dragHoldItem != item)
  {
    d->dragHoldItem = item;
    if( item  )
    {
      d->dragHoldTimer.start( 1000, true );
    }
    else
    {
      d->dragHoldTimer.stop();
    }
  }
}

void KIconView::focusOutEvent( QFocusEvent *fe )
{
  m_pAutoSelect->stop();

  Q3IconView::focusOutEvent( fe );
}

void KIconView::leaveEvent( QEvent *e )
{
  m_pAutoSelect->stop();

  Q3IconView::leaveEvent( e );
}

void KIconView::contentsMousePressEvent( QMouseEvent *e )
{
  if( (selectionMode() == Extended) && (e->state() & Qt::ShiftModifier) && !(e->state() & Qt::ControlModifier) ) {
    bool block = signalsBlocked();
    blockSignals( true );

    clearSelection();

    blockSignals( block );
  }

  Q3IconView::contentsMousePressEvent( e );
  d->doAutoSelect = false;
}

void KIconView::contentsMouseDoubleClickEvent ( QMouseEvent * e )
{
  Q3IconView::contentsMouseDoubleClickEvent( e );

  Q3IconViewItem* item = findItem( e->pos() );

  if( item ) {
    if( (e->button() == Qt::LeftButton) && !m_bUseSingle )
      emitExecute( item, e->globalPos() );

    emit doubleClicked( item, e->globalPos() );
  }
  d->doubleClickIgnoreTimer.start(0, true);
}

void KIconView::slotMouseButtonClicked( int btn, Q3IconViewItem *item, const QPoint &pos )
{
  //kdDebug() << " KIconView::slotMouseButtonClicked() item=" << item << endl;
  if( d->doubleClickIgnoreTimer.isActive() )
    return; // Ignore double click
    
  if( (btn == Qt::LeftButton) && item )
    emitExecute( item, pos );
}

void KIconView::contentsMouseReleaseEvent( QMouseEvent *e )
{
    d->doAutoSelect = true;
    Q3IconView::contentsMouseReleaseEvent( e );
}

void KIconView::contentsDragEnterEvent( QDragEnterEvent *e )
{
    updateDragHoldItem( e );
    Q3IconView::contentsDragEnterEvent( e );
}

void KIconView::contentsDragLeaveEvent( QDragLeaveEvent *e )
{
    d->dragHoldTimer.stop();
    d->dragHoldItem = 0L;
    Q3IconView::contentsDragLeaveEvent( e );
}


void KIconView::contentsDragMoveEvent( QDragMoveEvent *e )
{
    updateDragHoldItem( e );
    Q3IconView::contentsDragMoveEvent( e );
}

void KIconView::contentsDropEvent( QDropEvent* e )
{
    d->dragHoldTimer.stop();
    Q3IconView::contentsDropEvent( e );
}

void KIconView::slotDragHoldTimeout()
{
    Q3IconViewItem *tmp = d->dragHoldItem;
    d->dragHoldItem = 0L;

    emit held( tmp );
}

void KIconView::takeItem( Q3IconViewItem * item )
{
    if ( item == d->dragHoldItem )
    {
        d->dragHoldTimer.stop();
        d->dragHoldItem = 0L;
    }

    Q3IconView::takeItem( item );
}

void KIconView::cancelPendingHeldSignal()
{
    d->dragHoldTimer.stop();
    d->dragHoldItem = 0L;
}

void KIconView::wheelEvent( QWheelEvent *e )
{
    if (horizontalScrollBar() && (arrangement() == Q3IconView::TopToBottom)) {
        QWheelEvent ce(e->pos(), e->delta(), e->state(), Qt::Horizontal);
        QApplication::sendEvent( horizontalScrollBar(), &ce);
	if (ce.isAccepted()) {
            e->accept();
	    return;
	}
    }
    Q3IconView::wheelEvent(e);
}

void KIconView::setFont( const QFont &font )
{
    delete d->fm;
    d->fm = 0L;
    Q3IconView::setFont( font );
}

QFontMetrics *KIconView::itemFontMetrics() const
{
    if (!d->fm) {
        // QIconView creates one too, but we can't access it
        d->fm = new QFontMetrics( font() );
    }
    return d->fm;
}

QPixmap KIconView::selectedIconPixmap( QPixmap *pix, const QColor &col ) const
{
    QPixmap m;
    if ( d->maskCache.find( QString::number( pix->serialNumber() ), m ) )
	return m;
    m = KPixmapEffect::selectedPixmap( KPixmap(*pix), col );
    d->maskCache.insert( QString::number( pix->serialNumber() ), m );
    return m;
}

int KIconView::iconTextHeight() const
{
    return d->textHeight > 0 ? d->textHeight : ( wordWrapIconText() ? 99 : 1 );
}

void KIconView::setIconTextHeight( int n )
{
    int oldHeight = iconTextHeight();
    if ( n > 1 )
        d->textHeight = n;
    else
        d->textHeight = 1;

    // so that Qt still shows the tooltip when even a wrapped text is too long
    setWordWrapIconText( false );

    // update view if needed
    if ( iconTextHeight() != oldHeight )
        setFont( font() );  // hack to recalc items
}

/////////////

struct KIconViewItem::KIconViewItemPrivate
{
    QSize m_pixmapSize;
};

void KIconViewItem::init()
{
    m_wordWrap = 0L;
    d = 0L;
    calcRect();
}

KIconViewItem::~KIconViewItem()
{
    delete m_wordWrap;
    delete d;
}

void KIconViewItem::calcRect( const QString& text_ )
{
    Q_ASSERT( iconView() );
    if ( !iconView() )
        return;
    delete m_wordWrap;
    m_wordWrap = 0L;
#ifndef NDEBUG // be faster for the end-user, such a bug will have been fixed before hand :)
    if ( !iconView()->inherits("KIconView") )
    {
        kdWarning() << "KIconViewItem used in a " << iconView()->metaObject()->className() << " !!" << endl;
        return;
    }
#endif
    //kdDebug() << "KIconViewItem::calcRect - " << text() << endl;
    KIconView *view = static_cast<KIconView *>(iconView());
    QRect itemIconRect = pixmapRect();
    QRect itemTextRect = textRect();
    QRect itemRect = rect();

    int pw = 0;
    int ph = 0;

#ifndef QT_NO_PICTURE
    if ( picture() ) {
        QRect br = picture()->boundingRect();
        pw = br.width() + 2;
        ph = br.height() + 2;
    } else
#endif
    {
        // Qt uses unknown_icon if no pixmap. Let's see if we need that - I doubt it
        if (!pixmap())
            return;
        pw = pixmap()->width() + 2;
        ph = pixmap()->height() + 2;
    }
    itemIconRect.setWidth( pw );
#if 1 // FIXME 
    // There is a bug in Qt which prevents the item from being placed
    // properly when the pixmapRect is not at the top of the itemRect, so we
    // have to increase the height of the pixmapRect and leave it at the top
    // of the itemRect...
    if ( d && !d->m_pixmapSize.isNull() )
        itemIconRect.setHeight( d->m_pixmapSize.height() + 2 );
    else
#endif
    itemIconRect.setHeight( ph );

    int tw = 0;
    if ( d && !d->m_pixmapSize.isNull() )
        tw = view->maxItemWidth() - ( view->itemTextPos() == Q3IconView::Bottom ? 0 :
                                      d->m_pixmapSize.width() + 2 );
    else
        tw = view->maxItemWidth() - ( view->itemTextPos() == Q3IconView::Bottom ? 0 :
                                      itemIconRect.width() );
    
    QFontMetrics *fm = view->itemFontMetrics();
    QString t;
    QRect r;
    
    // When is text_ set ? Doesn't look like it's ever set.
    t = text_.isEmpty() ? text() : text_;
    
    // Max text height
    int nbLines = static_cast<KIconView*>( iconView() )->iconTextHeight();
    int height = nbLines > 0 ? fm->height() * nbLines : 0xFFFFFFFF;
    
    // Should not be higher than pixmap if text is alongside icons
    if ( view->itemTextPos() != Q3IconView::Bottom ) {
        if ( d && !d->m_pixmapSize.isNull() )
            height = qMin( d->m_pixmapSize.height() + 2, height );
        else
            height = qMin( itemIconRect.height(), height );
        height = qMax( height, fm->height() );
    }
    
    // Calculate the word-wrap
    QRect outerRect( 0, 0, tw - 6, height );
    m_wordWrap = KWordWrap::formatText( *fm, outerRect, 0, t );
    r = m_wordWrap->boundingRect();

    int realWidth = qMax( qMin( r.width() + 4, tw ), fm->width( "X" ) );
    itemTextRect.setWidth( realWidth );
    itemTextRect.setHeight( r.height() );

    int w = 0;    int h = 0;    int y = 0;
    if ( view->itemTextPos() == Q3IconView::Bottom ) {
        // If the pixmap size has been specified, use it
        if ( d && !d->m_pixmapSize.isNull() )
        {
            w = qMax( itemTextRect.width(), d->m_pixmapSize.width() + 2 );
            h = itemTextRect.height() + d->m_pixmapSize.height() + 2 + 1;
#if 0 // FIXME 
            // Waiting for the qt bug to be solved, the pixmapRect must
            // stay on the top...
            y = d->m_pixmapSize.height() + 2 - itemIconRect.height();
#endif
        }
        else {
            w = qMax( itemTextRect.width(), itemIconRect.width() );
            h = itemTextRect.height() + itemIconRect.height() + 1;
        }

        itemRect.setWidth( w );
        itemRect.setHeight( h );
        int width = qMax( w, QApplication::globalStrut().width() ); // see QIconViewItem::width()
        int height = qMax( h, QApplication::globalStrut().height() ); // see QIconViewItem::height()
        itemTextRect = QRect( ( width - itemTextRect.width() ) / 2, height - itemTextRect.height(),
                              itemTextRect.width(), itemTextRect.height() );
        itemIconRect = QRect( ( width - itemIconRect.width() ) / 2, y,
                              itemIconRect.width(), itemIconRect.height() );
    } else {
        // If the pixmap size has been specified, use it
        if ( d && !d->m_pixmapSize.isNull() )
        {
            h = qMax( itemTextRect.height(), d->m_pixmapSize.height() + 2 );
#if 0 // FIXME 
            // Waiting for the qt bug to be solved, the pixmapRect must
            // stay on the top...
            y = ( d->m_pixmapSize.height() + 2 - itemIconRect.height() ) / 2;
#endif
        }
        else
            h = qMax( itemTextRect.height(), itemIconRect.height() );
        w = itemTextRect.width() + itemIconRect.width() + 1;

        itemRect.setWidth( w );
        itemRect.setHeight( h );
        int width = qMax( w, QApplication::globalStrut().width() ); // see QIconViewItem::width()
        int height = qMax( h, QApplication::globalStrut().height() ); // see QIconViewItem::height()

        itemTextRect = QRect( width - itemTextRect.width(), ( height - itemTextRect.height() ) / 2,
                              itemTextRect.width(), itemTextRect.height() );
        if ( itemIconRect.height() > itemTextRect.height() ) // icon bigger than text -> center vertically
            itemIconRect = QRect( 0, ( height - itemIconRect.height() ) / 2,
                                  itemIconRect.width(), itemIconRect.height() );
        else // icon smaller than text -> place in top or center with first line
	    itemIconRect = QRect( 0, qMax(( fm->height() - itemIconRect.height() ) / 2 + y, 0),
                                  itemIconRect.width(), itemIconRect.height() );
        if ( ( itemIconRect.height() <= 20 ) && ( itemTextRect.height() < itemIconRect.height() ) )
        {
            itemTextRect.setHeight( itemIconRect.height() - 2 );
            itemTextRect.setY( itemIconRect.y() );
        }
    }

    if ( itemIconRect != pixmapRect() )
        setPixmapRect( itemIconRect );
    if ( itemTextRect != textRect() )
        setTextRect( itemTextRect );
    if ( itemRect != rect() )
        setItemRect( itemRect );

    // Done by setPixmapRect, setTextRect and setItemRect !  [and useless if no rect changed]
    //view->updateItemContainer( this );

}

void KIconViewItem::paintItem( QPainter *p, const QColorGroup &cg )
{
    Q3IconView* view = iconView();
    Q_ASSERT( view );
    if ( !view )
        return;
#ifndef NDEBUG // be faster for the end-user, such a bug will have been fixed before hand :)
    if ( !view->inherits("KIconView") )
    {
        kdWarning() << "KIconViewItem used in a " << view->metaObject()->className() << " !!" << endl;
        return;
    }
#endif

    p->save();

    paintPixmap(p, cg);
    paintText(p, cg);

    p->restore();
}

KWordWrap * KIconViewItem::wordWrap()
{
    return m_wordWrap;
}

void KIconViewItem::paintPixmap( QPainter *p, const QColorGroup &cg )
{
    KIconView *kview = static_cast<KIconView *>(iconView());

#ifndef QT_NO_PICTURE
    if ( picture() ) {
	QPicture *pic = picture();
	if ( isSelected() ) {
            // TODO something as nice as selectedIconPixmap if possible ;)
	    p->fillRect( pixmapRect( false ), QBrush( cg.highlight(), Qt::Dense4Pattern) );
	}
	p->drawPicture( x()-pic->boundingRect().x(), y()-pic->boundingRect().y(), *pic );
    } else
#endif
    {
        int iconX = pixmapRect( false ).x();
        int iconY = pixmapRect( false ).y();

        QPixmap *pix = pixmap();
        if ( !pix || pix->isNull() )
            return;

#if 1 // FIXME 
        // Move the pixmap manually because the pixmapRect is at the
        // top of the itemRect
        // (won't be needed anymore in future versions of qt)
        if ( d && !d->m_pixmapSize.isNull() )
        {
            int offset = 0;
            if ( kview->itemTextPos() == Q3IconView::Bottom )
                offset = d->m_pixmapSize.height() - pix->height();
            else
                offset = ( d->m_pixmapSize.height() - pix->height() ) / 2;
            if ( offset > 0 )
                iconY += offset;
        }
#endif
        if ( isSelected() ) {
            QPixmap selectedPix = kview->selectedIconPixmap( pix, cg.highlight() );
            p->drawPixmap( iconX, iconY, selectedPix );
        } else {
            p->drawPixmap( iconX, iconY, *pix );
        }
    }
}

void KIconViewItem::paintText( QPainter *p, const QColorGroup &cg )
{
    int textX = textRect( false ).x() + 2;
    int textY = textRect( false ).y();

    if ( isSelected() ) {
        p->fillRect( textRect( false ), cg.highlight() );
        p->setPen( QPen( cg.highlightedText() ) );
    } else {
        if ( iconView()->itemTextBackground() != Qt::NoBrush )
            p->fillRect( textRect( false ), iconView()->itemTextBackground() );
        p->setPen( cg.text() );
    }

    int align = iconView()->itemTextPos() == Q3IconView::Bottom ? Qt::AlignHCenter : Qt::AlignLeft;
    m_wordWrap->drawText( p, textX, textY, align | KWordWrap::Truncate );
}

QSize KIconViewItem::pixmapSize() const
{
    return d ? d->m_pixmapSize : QSize( 0, 0 );
}

void KIconViewItem::setPixmapSize( const QSize& size )
{
    if ( !d )
        d = new KIconViewItemPrivate;

    d->m_pixmapSize = size;
}

void KIconView::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kiconview.moc"
