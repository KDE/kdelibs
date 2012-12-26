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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "k3iconview.h"

#include <QtCore/QTimer>
#include <QPainter>
#include <QPixmapCache>
#include <QActionEvent>

#include "kwordwrap.h"
#include <kconfig.h>
#include <kdebug.h>
#include <kglobalsettings.h>

#include <kcursor.h>
#include <QApplication>

class K3IconView::K3IconViewPrivate
{
public:
    K3IconViewPrivate() {
        mode = K3IconView::Execute;
        fm = 0L;
        doAutoSelect = true;
        textHeight = 0;
        dragHoldItem = 0L;
    }
    K3IconView::Mode mode;
    bool doAutoSelect;
    QFontMetrics *fm;
    QPixmapCache maskCache;
    int textHeight;
    Q3IconViewItem *dragHoldItem;
    QTimer dragHoldTimer;
    QTimer doubleClickIgnoreTimer;
};

K3IconView::K3IconView( QWidget *parent, const char *name, Qt::WindowFlags f )
    : Q3IconView( parent, name, f )
{
    d = new K3IconViewPrivate;

    connect( this, SIGNAL(onViewport()),
             this, SLOT(slotOnViewport()) );
    connect( this, SIGNAL(onItem(Q3IconViewItem*)),
             this, SLOT(slotOnItem(Q3IconViewItem*)) );
    slotSettingsChanged( KGlobalSettings::SETTINGS_MOUSE );
    connect( KGlobalSettings::self(), SIGNAL(settingsChanged(int)), SLOT(slotSettingsChanged(int)) );

    m_pCurrentItem = 0L;

    m_pAutoSelect = new QTimer( this );
    connect( m_pAutoSelect, SIGNAL(timeout()),
             this, SLOT(slotAutoSelect()) );

    connect( &d->dragHoldTimer, SIGNAL(timeout()), this, SLOT(slotDragHoldTimeout()) );
}

K3IconView::~K3IconView()
{
    delete d->fm;
    delete d;
}


void K3IconView::setMode( K3IconView::Mode mode )
{
    d->mode = mode;
}

K3IconView::Mode K3IconView::mode() const
{
    return d->mode;
}

void K3IconView::slotOnItem( Q3IconViewItem *item )
{
    if ( item ) {
        if ( m_bUseSingle ) {
            if ( m_bChangeCursorOverItem )
                viewport()->setCursor(QCursor(Qt::PointingHandCursor));

            if ( (m_autoSelectDelay > -1) ) {
                m_pAutoSelect->setSingleShot( true );
                m_pAutoSelect->start( m_autoSelectDelay );
            }
        }
        m_pCurrentItem = item;
    }
}

void K3IconView::slotOnViewport()
{
    if ( m_bUseSingle && m_bChangeCursorOverItem )
        viewport()->unsetCursor();

    m_pAutoSelect->stop();
    m_pCurrentItem = 0L;
}

void K3IconView::slotSettingsChanged(int category)
{
    if ( category != KGlobalSettings::SETTINGS_MOUSE )
      return;
    m_bUseSingle = KGlobalSettings::singleClick();
    //kDebug() << "K3IconView::slotSettingsChanged for mouse, usesingle=" << m_bUseSingle;

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

void K3IconView::slotAutoSelect()
{
  // check that the item still exists
  if( index( m_pCurrentItem ) == -1 || !d->doAutoSelect )
    return;

  //Give this widget the keyboard focus.
  if( !hasFocus() )
    setFocus();

  Qt::KeyboardModifiers keybstate = QApplication::keyboardModifiers();
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
      bool update = viewport()->updatesEnabled();
      viewport()->setUpdatesEnabled( false );

      //Calculate the smallest rectangle that contains the current Item
      //and the one that got the autoselect event
      QRect r;
      QRect redraw;
      if ( previousItem ) {
        r = QRect( qMin( previousItem->x(), m_pCurrentItem->x() ),
            qMin( previousItem->y(), m_pCurrentItem->y() ),
            0, 0 );
        if ( previousItem->x() < m_pCurrentItem->x() )
          r.setWidth( m_pCurrentItem->x() - previousItem->x() + m_pCurrentItem->width() );
        else
          r.setWidth( previousItem->x() - m_pCurrentItem->x() + previousItem->width() );
        if ( previousItem->y() < m_pCurrentItem->y() )
          r.setHeight( m_pCurrentItem->y() - previousItem->y() + m_pCurrentItem->height() );
        else
          r.setHeight( previousItem->y() - m_pCurrentItem->y() + previousItem->height() );
        r = r.normalized();
      }
      else
	r = QRect( 0, 0, 0, 0 );

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
    kDebug() << "K3IconView: That's not supposed to happen!!!!";
}

void K3IconView::emitExecute( Q3IconViewItem *item, const QPoint &pos )
{
  if ( d->mode != Execute )
  {
    // kDebug() << "K3IconView::emitExecute : not in execute mode !";
    return;
  }

  Qt::KeyboardModifiers keybstate = QApplication::keyboardModifiers();

  m_pAutoSelect->stop();

  //Don't emit executed if in SC mode and Shift or Ctrl are pressed
  if( !( m_bUseSingle && ((keybstate & Qt::ShiftModifier) || (keybstate & Qt::ControlModifier)) ) ) {
    setSelected( item, false );
    viewport()->unsetCursor();
    emit executed( item );
    emit executed( item, pos );
  }
}

void K3IconView::updateDragHoldItem( QDropEvent *e )
{
  Q3IconViewItem *item = findItem( e->pos() );

  if ( d->dragHoldItem != item)
  {
    d->dragHoldItem = item;
    if( item  )
    {
      d->dragHoldTimer.setSingleShot( true );
      d->dragHoldTimer.start( 1000 );
    }
    else
    {
      d->dragHoldTimer.stop();
    }
  }
}

void K3IconView::focusOutEvent( QFocusEvent *fe )
{
  m_pAutoSelect->stop();

  Q3IconView::focusOutEvent( fe );
}

void K3IconView::leaveEvent( QEvent *e )
{
  m_pAutoSelect->stop();

  Q3IconView::leaveEvent( e );
}

void K3IconView::contentsMousePressEvent( QMouseEvent *e )
{
  if( (selectionMode() == Extended) && (e->modifiers() & Qt::ShiftModifier) && !(e->modifiers() & Qt::ControlModifier) ) {
    bool block = signalsBlocked();
    blockSignals( true );

    clearSelection();

    blockSignals( block );
  }

  Q3IconView::contentsMousePressEvent( e );
  d->doAutoSelect = false;
}

void K3IconView::contentsMouseDoubleClickEvent ( QMouseEvent * e )
{
  Q3IconView::contentsMouseDoubleClickEvent( e );

  Q3IconViewItem* item = findItem( e->pos() );

  if( item ) {
    if( (e->button() == Qt::LeftButton) && !m_bUseSingle )
      emitExecute( item, e->globalPos() );

    emit doubleClicked( item, e->globalPos() );
  }
  d->doubleClickIgnoreTimer.setSingleShot(true);
  d->doubleClickIgnoreTimer.start(0);
}

void K3IconView::slotMouseButtonClicked( int btn, Q3IconViewItem *item, const QPoint &pos )
{
  //kDebug() << " K3IconView::slotMouseButtonClicked() item=" << item;
  if( d->doubleClickIgnoreTimer.isActive() )
    return; // Ignore double click

  if( (btn == Qt::LeftButton) && item )
    emitExecute( item, pos );
}

void K3IconView::contentsMouseReleaseEvent( QMouseEvent *e )
{
    d->doAutoSelect = true;
    Q3IconView::contentsMouseReleaseEvent( e );
}

void K3IconView::contentsDragEnterEvent( QDragEnterEvent *e )
{
    updateDragHoldItem( e );
    Q3IconView::contentsDragEnterEvent( e );
}

void K3IconView::contentsDragLeaveEvent( QDragLeaveEvent *e )
{
    d->dragHoldTimer.stop();
    d->dragHoldItem = 0L;
    Q3IconView::contentsDragLeaveEvent( e );
}


void K3IconView::contentsDragMoveEvent( QDragMoveEvent *e )
{
    updateDragHoldItem( e );
    Q3IconView::contentsDragMoveEvent( e );
}

void K3IconView::contentsDropEvent( QDropEvent* e )
{
    d->dragHoldTimer.stop();
    Q3IconView::contentsDropEvent( e );
}

void K3IconView::slotDragHoldTimeout()
{
    Q3IconViewItem *tmp = d->dragHoldItem;
    d->dragHoldItem = 0L;

    emit held( tmp );
}

void K3IconView::takeItem( Q3IconViewItem * item )
{
    if ( item == d->dragHoldItem )
    {
        d->dragHoldTimer.stop();
        d->dragHoldItem = 0L;
    }

    Q3IconView::takeItem( item );
}

void K3IconView::cancelPendingHeldSignal()
{
    d->dragHoldTimer.stop();
    d->dragHoldItem = 0L;
}

void K3IconView::wheelEvent( QWheelEvent *e )
{
    if (horizontalScrollBar() && (arrangement() == Q3IconView::TopToBottom)) {
        QWheelEvent ce(e->pos(), e->delta(), e->buttons(), e->modifiers(), Qt::Horizontal);
        QApplication::sendEvent( horizontalScrollBar(), &ce);
	if (ce.isAccepted()) {
            e->accept();
	    return;
	}
    }
    Q3IconView::wheelEvent(e);
}

void K3IconView::setFont( const QFont &font )
{
    delete d->fm;
    d->fm = 0L;
    Q3IconView::setFont( font );
}

QFontMetrics *K3IconView::itemFontMetrics() const
{
    if (!d->fm) {
        // QIconView creates one too, but we can't access it
        d->fm = new QFontMetrics( font() );
    }
    return d->fm;
}

QPixmap K3IconView::selectedIconPixmap( QPixmap *pix, const QColor &col ) const
{
    QPixmap m;
    if ( d->maskCache.find( QString::number( pix->serialNumber() ), m ) )
	return m;
    m = *pix;
    {
        QPainter p(&m);
        QColor h = col;
        h.setAlphaF(0.5);
        p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        p.fillRect(m.rect(), h);
        p.end();
    }
    d->maskCache.insert( QString::number( pix->serialNumber() ), m );
    return m;
}

int K3IconView::iconTextHeight() const
{
    return d->textHeight > 0 ? d->textHeight : ( wordWrapIconText() ? 99 : 1 );
}

void K3IconView::setIconTextHeight( int n )
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

struct K3IconViewItem::K3IconViewItemPrivate
{
    QSize m_pixmapSize;
};

void K3IconViewItem::init()
{
    m_wordWrap = 0L;
    d = 0L;
    calcRect();
}

K3IconViewItem::~K3IconViewItem()
{
    delete m_wordWrap;
    delete d;
}

void K3IconViewItem::calcRect( const QString& text_ )
{
    Q_ASSERT( iconView() );
    if ( !iconView() )
        return;
    delete m_wordWrap;
    m_wordWrap = 0L;
#ifndef NDEBUG // be faster for the end-user, such a bug will have been fixed before hand :)
    if ( !qobject_cast<K3IconView*>(iconView()) )
    {
        kWarning() << "K3IconViewItem used in a " << iconView()->metaObject()->className() << " !!";
        return;
    }
#endif
    //kDebug() << "K3IconViewItem::calcRect - " << text();
    K3IconView *view = static_cast<K3IconView *>(iconView());
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
    int nbLines = static_cast<K3IconView*>( iconView() )->iconTextHeight();
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

void K3IconViewItem::paintItem( QPainter *p, const QColorGroup &cg )
{
    Q3IconView* view = iconView();
    Q_ASSERT( view );
    if ( !view )
        return;
#ifndef NDEBUG // be faster for the end-user, such a bug will have been fixed before hand :)
    if ( !qobject_cast<K3IconView*>(view) )
    {
        kWarning() << "K3IconViewItem used in a " << view->metaObject()->className() << " !!";
        return;
    }
#endif

    p->save();

    paintPixmap(p, cg);
    paintText(p, cg);

    p->restore();
}

KWordWrap * K3IconViewItem::wordWrap()
{
    return m_wordWrap;
}

void K3IconViewItem::paintPixmap( QPainter *p, const QColorGroup &cg )
{
    K3IconView *kview = static_cast<K3IconView *>(iconView());

#ifndef QT_NO_PICTURE
    if ( picture() ) {
	QPicture *pic = picture();
	if ( isSelected() ) {
            // TODO something as nice as selectedIconPixmap if possible ;)
	    p->fillRect( pixmapRect( false ), QBrush( cg.color(QPalette::Highlight), Qt::Dense4Pattern) );
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
            QPixmap selectedPix = kview->selectedIconPixmap( pix, cg.color( QPalette::Highlight ) );
            p->drawPixmap( iconX, iconY, selectedPix );
        } else {
            p->drawPixmap( iconX, iconY, *pix );
        }
    }
}

void K3IconViewItem::paintText( QPainter *p, const QColorGroup &cg )
{
    int textX = textRect( false ).x() + 2;
    int textY = textRect( false ).y();

    if ( isSelected() ) {
        p->fillRect( textRect( false ), cg.color( QPalette::Highlight ) );
        p->setPen( QPen( cg.color( QPalette::HighlightedText ) ) );
    } else {
        if ( iconView()->itemTextBackground() != Qt::NoBrush )
            p->fillRect( textRect( false ), iconView()->itemTextBackground() );
        p->setPen( cg.color( QPalette::Text ) );
    }

    int align = iconView()->itemTextPos() == Q3IconView::Bottom ? Qt::AlignHCenter : Qt::AlignLeft;
    m_wordWrap->drawText( p, textX, textY, align | KWordWrap::Truncate );
}

QSize K3IconViewItem::pixmapSize() const
{
    return d ? d->m_pixmapSize : QSize( 0, 0 );
}

void K3IconViewItem::setPixmapSize( const QSize& size )
{
    if ( !d )
        d = new K3IconViewItemPrivate;

    d->m_pixmapSize = size;
}

#include "moc_k3iconview.cpp"
