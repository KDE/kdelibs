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
#include <qtimer.h>
#include <qpainter.h>
#include <qpixmapcache.h>
#include <qcleanuphandler.h>

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

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

class KIconView::KIconViewPrivate
{
public:
    KIconViewPrivate() {
        mode = KIconView::Execute;
        fm = 0L;
        doAutoSelect = true;
    }
    KIconView::Mode mode;
    bool doAutoSelect;
    QFontMetrics *fm;
    QPixmapCache maskCache;
};

KIconView::KIconView( QWidget *parent, const char *name, WFlags f )
    : QIconView( parent, name, f )
{
    d = new KIconViewPrivate;

    connect( this, SIGNAL( onViewport() ),
             this, SLOT( slotOnViewport() ) );
    connect( this, SIGNAL( onItem( QIconViewItem * ) ),
             this, SLOT( slotOnItem( QIconViewItem * ) ) );
    slotSettingsChanged( KApplication::SETTINGS_MOUSE );
    if ( kapp ) { // maybe null when used inside designer
        connect( kapp, SIGNAL( settingsChanged(int) ), SLOT( slotSettingsChanged(int) ) );
        kapp->addKipcEventMask( KIPC::SettingsChanged );
    }

    m_pCurrentItem = 0L;

    m_pAutoSelect = new QTimer( this );
    connect( m_pAutoSelect, SIGNAL( timeout() ),
             this, SLOT( slotAutoSelect() ) );
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

void KIconView::slotOnItem( QIconViewItem *item )
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

    disconnect( this, SIGNAL( mouseButtonClicked( int, QIconViewItem *,
						  const QPoint & ) ),
		this, SLOT( slotMouseButtonClicked( int, QIconViewItem *,
						    const QPoint & ) ) );
//         disconnect( this, SIGNAL( doubleClicked( QIconViewItem *,
// 						 const QPoint & ) ),
// 		    this, SLOT( slotExecute( QIconViewItem *,
// 					     const QPoint & ) ) );

    if( m_bUseSingle ) {
      connect( this, SIGNAL( mouseButtonClicked( int, QIconViewItem *,
						 const QPoint & ) ),
	       this, SLOT( slotMouseButtonClicked( int, QIconViewItem *,
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

#ifdef Q_WS_X11
  //FIXME(E): Implement for Qt Embedded
  Window root;
  Window child;
  int root_x, root_y, win_x, win_y;
  uint keybstate;
  XQueryPointer( qt_xdisplay(), qt_xrootwin(), &root, &child,
		 &root_x, &root_y, &win_x, &win_y, &keybstate );
#endif

  QIconViewItem* previousItem = currentItem();
  setCurrentItem( m_pCurrentItem );

  if( m_pCurrentItem ) {
    //Shift pressed?
#ifdef Q_WS_X11 //FIXME
    if( (keybstate & ShiftMask) ) {
      //Temporary implementation of the selection until QIconView supports it
      bool block = signalsBlocked();
      blockSignals( true );

      //No Ctrl? Then clear before!
      if( !(keybstate & ControlMask) )
	clearSelection();

      bool select = !m_pCurrentItem->isSelected();
      bool update = viewport()->isUpdatesEnabled();
      viewport()->setUpdatesEnabled( false );

      //Calculate the smallest rectangle that contains the current Item
      //and the one that got the autoselect event
      QRect r;
      QRect redraw;
      if ( previousItem )
	r = QRect( QMIN( previousItem->x(), m_pCurrentItem->x() ),
		   QMIN( previousItem->y(), m_pCurrentItem->y() ),
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
      viewport()->setUpdatesEnabled( update );
      repaintContents( redraw, false );

      emit selectionChanged();

      if( selectionMode() == QIconView::Single )
	emit selectionChanged( m_pCurrentItem );

      //setSelected( m_pCurrentItem, true, (keybstate & ControlMask), (keybstate & ShiftMask) );
    }
    else if( (keybstate & ControlMask) )
      setSelected( m_pCurrentItem, !m_pCurrentItem->isSelected(), true );
    else
#endif
      setSelected( m_pCurrentItem, true );
  }
#ifndef Q_WS_QWS //FIXME: Remove #if as soon as the stuff above is implemented
  else
    kdDebug() << "KIconView: That's not supposed to happen!!!!" << endl;
#endif
}

void KIconView::emitExecute( QIconViewItem *item, const QPoint &pos )
{
  if ( d->mode != Execute )
  {
    // kdDebug() << "KIconView::emitExecute : not in execute mode !" << endl;
    return;
  }

#ifdef Q_WS_X11 //FIXME
  Window root;
  Window child;
  int root_x, root_y, win_x, win_y;
  uint keybstate;
  XQueryPointer( qt_xdisplay(), qt_xrootwin(), &root, &child,
		 &root_x, &root_y, &win_x, &win_y, &keybstate );
#endif

  m_pAutoSelect->stop();

  //Don�t emit executed if in SC mode and Shift or Ctrl are pressed
#ifdef Q_WS_X11 //FIXME
  if( !( m_bUseSingle && ((keybstate & ShiftMask) || (keybstate & ControlMask)) ) ) {
    setSelected( item, false );
    emit executed( item );
    emit executed( item, pos );
  }
#endif
}

void KIconView::focusOutEvent( QFocusEvent *fe )
{
  m_pAutoSelect->stop();

  QIconView::focusOutEvent( fe );
}

void KIconView::leaveEvent( QEvent *e )
{
  m_pAutoSelect->stop();

  QIconView::leaveEvent( e );
}

void KIconView::contentsMousePressEvent( QMouseEvent *e )
{
  if( (selectionMode() == Extended) && (e->state() & ShiftButton) && !(e->state() & ControlButton) ) {
    bool block = signalsBlocked();
    blockSignals( true );

    clearSelection();

    blockSignals( block );
  }

  QIconView::contentsMousePressEvent( e );
  d->doAutoSelect = FALSE;
}

void KIconView::contentsMouseDoubleClickEvent ( QMouseEvent * e )
{
  QIconView::contentsMouseDoubleClickEvent( e );

  QIconViewItem* item = findItem( e->pos() );

  if( item ) {
    if( (e->button() == LeftButton) && !m_bUseSingle )
      emitExecute( item, e->globalPos() );

    emit doubleClicked( item, e->globalPos() );
  }
}

void KIconView::slotMouseButtonClicked( int btn, QIconViewItem *item, const QPoint &pos )
{
  //kdDebug() << " KIconView::slotMouseButtonClicked() item=" << item << endl;
  if( (btn == LeftButton) && item )
    emitExecute( item, pos );
}

void KIconView::contentsMouseReleaseEvent( QMouseEvent *e )
{
    d->doAutoSelect = TRUE;
    QIconView::contentsMouseReleaseEvent( e );
}

void KIconView::setFont( const QFont &font )
{
    delete d->fm;
    d->fm = 0L;
    QIconView::setFont( font );
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

/////////////

struct KIconViewItem::KIconViewItemPrivate
{
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
    // No word-wrap ? Call the default calcRect in that case.
    if ( !iconView()->wordWrapIconText() )
    {
        QIconViewItem::calcRect( text_ );
        return;
    }
#ifndef NDEBUG // be faster for the end-user, such a bug will have been fixed before hand :)
    if ( !iconView()->inherits("KIconView") )
    {
        kdWarning() << "KIconViewItem used in a " << iconView()->className() << " !!" << endl;
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
    itemIconRect.setHeight( ph );
    //kdDebug() << "KIconViewItem::calcRect itemIconRect[tmp]=" << itemIconRect.x() << "," << itemIconRect.y()
    //          << " " << itemIconRect.width() << "x" << itemIconRect.height() << endl;

    // When is text_ set ? Doesn't look like it's ever set.
    QString t = text_.isEmpty() ? text() : text_;

    int tw = 0;
    int th = 0;
    QFontMetrics *fm = view->itemFontMetrics();
    QRect outerRect( 0, 0, view->maxItemWidth() -
                     ( view->itemTextPos() == QIconView::Bottom ? 0 :
                       pixmapRect().width() ), 0xFFFFFFFF );
    // Calculate the word-wrap
    m_wordWrap = KWordWrap::formatText( *fm, outerRect, AlignHCenter | WordBreak /*| BreakAnywhere*/, t );
    QRect r = m_wordWrap->boundingRect();
    r.setWidth( r.width() + 4 );
    // [Non-word-wrap code removed]

    if ( r.width() > view->maxItemWidth() -
         ( view->itemTextPos() == QIconView::Bottom ? 0 :
           pixmapRect().width() ) )
        r.setWidth( view->maxItemWidth() - ( view->itemTextPos() == QIconView::Bottom ? 0 :
                                                   pixmapRect().width() ) );

    tw = r.width();
    th = r.height();
    int minw = fm->width( "X" );
    if ( tw < minw )
        tw = minw;

    itemTextRect.setWidth( tw );
    itemTextRect.setHeight( th );
    //kdDebug() << "KIconViewItem::calcRect itemTextRect[tmp]=" << itemTextRect.x() << "," << itemTextRect.y()
    //          << " " << itemTextRect.width() << "x" << itemTextRect.height() << endl;

    // All this code isn't related to the word-wrap algo...
    // Sucks that we have to duplicate it.
    int w = 0;    int h = 0;
    if ( view->itemTextPos() == QIconView::Bottom ) {
        w = QMAX( itemTextRect.width(), itemIconRect.width() );
        h = itemTextRect.height() + itemIconRect.height() + 1;

        itemRect.setWidth( w );
        itemRect.setHeight( h );
        int width = QMAX( w, QApplication::globalStrut().width() ); // see QIconViewItem::width()
        int height = QMAX( h, QApplication::globalStrut().height() ); // see QIconViewItem::height()
        itemTextRect = QRect( ( width - itemTextRect.width() ) / 2, height - itemTextRect.height(),
                              itemTextRect.width(), itemTextRect.height() );
        itemIconRect = QRect( ( width - itemIconRect.width() ) / 2, 0,
                              itemIconRect.width(), itemIconRect.height() );
    } else {
        h = QMAX( itemTextRect.height(), itemIconRect.height() );
        w = itemTextRect.width() + itemIconRect.width() + 1;

        itemRect.setWidth( w );
        itemRect.setHeight( h );
        int width = QMAX( w, QApplication::globalStrut().width() ); // see QIconViewItem::width()
        int height = QMAX( h, QApplication::globalStrut().height() ); // see QIconViewItem::height()

        itemTextRect = QRect( width - itemTextRect.width(), ( height - itemTextRect.height() ) / 2,
                              itemTextRect.width(), itemTextRect.height() );
        if ( itemIconRect.height() > itemTextRect.height() ) // icon bigger than text -> center vertically
            itemIconRect = QRect( 0, ( height - itemIconRect.height() ) / 2,
                                  itemIconRect.width(), itemIconRect.height() );
        else // icon smaller than text -> center with first line
            itemIconRect = QRect( 0, ( fm->height() - itemIconRect.height() ) / 2,
                                  itemIconRect.width(), itemIconRect.height() );
    }
#if 0
    kdDebug() << "KIconViewItem::calcRect itemIconRect=" << itemIconRect.x() << "," << itemIconRect.y()
              << " " << itemIconRect.width() << "x" << itemIconRect.height() << endl;
    kdDebug() << "KIconViewItem::calcRect itemTextRect=" << itemTextRect.x() << "," << itemTextRect.y()
              << " " << itemTextRect.width() << "x" << itemTextRect.height() << endl;
    kdDebug() << "KIconViewItem::calcRect itemRect=" << itemRect.x() << "," << itemRect.y()
              << " " << itemRect.width() << "x" << itemRect.height() << endl;
    kdDebug() << "KIconViewItem::calcRect - DONE" << endl;
#endif

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
    QIconView* view = iconView();
    Q_ASSERT( view );
    if ( !view )
        return;
    // No word-wrap ? Call the default paintItem in that case
    // (because we don't have access to calcTmpText()).
    // ################ This prevents the use of KPixmapEffect::selectedPixmap
    // This really needs to be opened up in qt.
    if ( !view->wordWrapIconText() )
    {
        QIconViewItem::paintItem( p, cg );
        return;
    }
#ifndef NDEBUG // be faster for the end-user, such a bug will have been fixed before hand :)
    if ( !view->inherits("KIconView") )
    {
        kdWarning() << "KIconViewItem used in a " << view->className() << " !!" << endl;
        return;
    }
#endif
    if ( !m_wordWrap )
    {
        kdWarning() << "KIconViewItem::paintItem called but wordwrap not ready - calcRect not called, or aborted!" << endl;
        return;
    }
    KIconView *kview = static_cast<KIconView *>(iconView());
    int textX = textRect( FALSE ).x();
    int textY = textRect( FALSE ).y();
    int iconX = pixmapRect( FALSE ).x();
    int iconY = pixmapRect( FALSE ).y();

    p->save();

#ifndef QT_NO_PICTURE
    if ( picture() ) {
	QPicture *pic = picture();
	if ( isSelected() ) {
            // TODO something as nice as selectedIconPixmap if possible ;)
	    p->fillRect( pixmapRect( FALSE ), QBrush( cg.highlight(), QBrush::Dense4Pattern) );
	}
	p->drawPicture( x()-pic->boundingRect().x(), y()-pic->boundingRect().y(), *pic );
    } else
#endif
    {
        QPixmap *pix = pixmap();
        if ( isSelected() ) {
            if ( pix && !pix->isNull() ) {
                QPixmap selectedPix = kview->selectedIconPixmap( pix, cg.highlight() );
                p->drawPixmap( iconX, iconY, selectedPix );
            }
        } else {
            p->drawPixmap( iconX, iconY, *pix );
        }
    }

    if ( isSelected() ) {
        p->fillRect( textRect( FALSE ), cg.highlight() );
        p->setPen( QPen( cg.highlightedText() ) );
    } else {
        if ( view->itemTextBackground() != NoBrush )
            p->fillRect( textRect( FALSE ), view->itemTextBackground() );
	p->setPen( cg.text() );
    }

    int align = view->itemTextPos() == QIconView::Bottom ? AlignHCenter : AlignAuto;
    m_wordWrap->drawText( p, textX, textY, align );

    p->restore();
}

void KIconView::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kiconview.moc"
