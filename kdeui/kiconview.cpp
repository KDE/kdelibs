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
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kapplication.h>
#include <kipc.h>
#include <kcursor.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

class KIconView::KIconViewPrivate
{
public:
    KIconViewPrivate() {
	mode = KIconView::Execute;
	doAutoSelect = TRUE;
        fm = 0L;
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

    oldCursor = viewport()->cursor();
    connect( this, SIGNAL( onViewport() ),
	     this, SLOT( slotOnViewport() ) );
    connect( this, SIGNAL( onItem( QIconViewItem * ) ),
             this, SLOT( slotOnItem( QIconViewItem * ) ) );
    slotSettingsChanged( KApplication::SETTINGS_MOUSE );
    connect( kapp, SIGNAL( settingsChanged(int) ), SLOT( slotSettingsChanged(int) ) );
    kapp->addKipcEventMask( KIPC::SettingsChanged );

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


int KIconView::itemIndex( const QIconViewItem *item ) const
{
    if ( !item )
	return -1;

    if ( item == firstItem() )
	return 0;
    else if ( item == lastItem() )
	return count() - 1;
    else {
	QIconViewItem *i = firstItem();
	int j = 0;
	while ( i && i != item ) {
	    i = i->nextItem();
	    ++j;
	}

	return i ? j : -1;
    }
}

void KIconView::slotOnItem( QIconViewItem *item )
{
    if ( item && m_bChangeCursorOverItem && m_bUseSingle )
        viewport()->setCursor( KCursor().handCursor() );

    if ( item && (m_autoSelectDelay > -1) && m_bUseSingle ) {
      m_pAutoSelect->start( m_autoSelectDelay, true );
      m_pCurrentItem = item;
    }
}

void KIconView::slotOnViewport()
{
    if ( m_bChangeCursorOverItem )
        viewport()->setCursor( oldCursor );

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
        viewport()->setCursor( oldCursor );
}

void KIconView::slotAutoSelect()
{
  // check that the item still exists
  if( itemIndex( m_pCurrentItem ) == -1 || !d->doAutoSelect )
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
      //Temporary implementaion of the selection until QIconView supports it
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

  //Don´t emit executed if in SC mode and Shift or Ctrl are pressed
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

QFontMetrics *KIconView::itemFontMetrics() const
{
    if (!d->fm) {
        // QIconView creates one too, but we can't access it
        d->fm = new QFontMetrics( font() );
    }
    return d->fm;
}

QBitmap KIconView::itemMask( QPixmap *pix ) const
{
    // Yet more code duplication from QIconView, due to QIconView::mask() being private
    QBitmap m;
    if ( d->maskCache.find( QString::number( pix->serialNumber() ), m ) )
	return m;
    m = pix->createHeuristicMask();
    d->maskCache.insert( QString::number( pix->serialNumber() ), m );
    return m;
}

/////////////

void KIconViewItem::init()
{
    m_wordWrap = 0L;
    calcRect();
}

KIconViewItem::~KIconViewItem()
{
    delete m_wordWrap;
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
        itemIconRect = QRect( 0, ( height - itemIconRect.height() ) / 2,
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

static QPixmap *kiv_buffer_pixmap = 0;
static QCleanupHandler<QPixmap> kiv_cleanup_pixmap;

// Yay let's keep duplicating stuff, we love that so much...
static QPixmap *get_kiv_buffer_pixmap( const QSize &s )
{
    if ( !kiv_buffer_pixmap ) {
        kiv_buffer_pixmap = new QPixmap( s );
        kiv_cleanup_pixmap.add( &kiv_buffer_pixmap );
        return kiv_buffer_pixmap;
    }

    kiv_buffer_pixmap->resize( s );
    return kiv_buffer_pixmap;
}

void KIconViewItem::paintItem( QPainter *p, const QColorGroup &cg )
{
    QIconView* view = iconView();
    Q_ASSERT( view );
    if ( !view )
        return;
    // No word-wrap ? Call the default paintItem in that case
    // (because we don't have access to calcTmpText()).
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

    // Yet more code duplication from QIconView
    // just to replace p->drawText() with m_wordWrap->drawText() !

    p->save();

    if ( isSelected() ) {
	p->setPen( cg.highlightedText() );
    } else {
	p->setPen( cg.text() );
    }

#ifndef QT_NO_PICTURE
    if ( picture() ) {
	QPicture *pic = picture();
	if ( isSelected() ) {
	    p->fillRect( pixmapRect( FALSE ), QBrush( cg.highlight(), QBrush::Dense4Pattern) );
	}
	p->drawPicture( x()-pic->boundingRect().x(), y()-pic->boundingRect().y(), *pic );
	if ( isSelected() ) {
	    p->fillRect( textRect( FALSE ), cg.highlight() );
	    p->setPen( QPen( cg.highlightedText() ) );
	} else if ( view->itemTextBackground() != NoBrush )
	    p->fillRect( textRect( FALSE ), view->itemTextBackground() );

	int align = view->itemTextPos() == QIconView::Bottom ? AlignHCenter : AlignAuto;
        //align |= WordBreak /*| BreakAnywhere*/;
	//p->drawText( textRect( FALSE ), align, text() );
        m_wordWrap->drawText( p, textX, textY, align );
	p->restore();
	return;
    }
#endif
    // ### get rid of code duplication
    if ( view->itemTextPos() == QIconView::Bottom ) {
	int w = pixmap()->width();

	if ( isSelected() ) {
	    QPixmap *pix = pixmap();
	    if ( pix && !pix->isNull() ) {
		QPixmap *buffer = get_kiv_buffer_pixmap( pix->size() );
		QBitmap mask = kview->itemMask( pix );

		QPainter p2( buffer );
		p2.fillRect( pix->rect(), white );
		p2.drawPixmap( 0, 0, *pix );
		p2.end();
		buffer->setMask( mask );
		p2.begin( buffer );
		p2.fillRect( pix->rect(), QBrush( cg.highlight(), QBrush::Dense4Pattern) );
		p2.end();
		QRect cr = pix->rect();
		p->drawPixmap( x() + ( width() - w ) / 2, y(), *buffer, 0, 0, cr.width(), cr.height() );
	    }
	} else {
	    p->drawPixmap( x() + ( width() - w ) / 2, y(), *pixmap() );
	}

	p->save();
	if ( isSelected() ) {
	    p->fillRect( textRect( FALSE ), cg.highlight() );
	    p->setPen( QPen( cg.highlightedText() ) );
	} else if ( view->itemTextBackground() != NoBrush )
	    p->fillRect( textRect( FALSE ), view->itemTextBackground() );

	//int align = AlignHCenter | WordBreak /*| BreakAnywhere*/;
	//p->drawText( textRect( FALSE ), align, text() );
        m_wordWrap->drawText( p, textX, textY, AlignHCenter );

	p->restore();
    } else {
	int h = pixmap()->height();

	if ( isSelected() ) {
	    QPixmap *pix = pixmap();
	    if ( pix && !pix->isNull() ) {
		QPixmap *buffer = get_kiv_buffer_pixmap( pix->size() );
		QBitmap mask = kview->itemMask( pix );

		QPainter p2( buffer );
		p2.fillRect( pix->rect(), white );
		p2.drawPixmap( 0, 0, *pix );
		p2.end();
		buffer->setMask( mask );
		p2.begin( buffer );
		p2.fillRect( pix->rect(), QBrush( cg.highlight(), QBrush::Dense4Pattern) );
		p2.end();
		QRect cr = pix->rect();
		p->drawPixmap( x() , y() + ( height() - h ) / 2, *buffer, 0, 0, cr.width(), cr.height() );
	    }
	} else {
	    p->drawPixmap( x() , y() + ( height() - h ) / 2, *pixmap() );
	}

	p->save();
	if ( isSelected() ) {
	    p->fillRect( textRect( FALSE ), cg.highlight() );
	    p->setPen( QPen( cg.highlightedText() ) );
	} else if ( view->itemTextBackground() != NoBrush )
	    p->fillRect( textRect( FALSE ), view->itemTextBackground() );

	//int align = AlignAuto | WordBreak /*| BreakAnywhere*/;
	//p->drawText( textRect( FALSE ), align, text() );
        m_wordWrap->drawText( p, textX, textY, AlignAuto );

	p->restore();
    }

    p->restore();
}

////////////////

KWordWrap* KWordWrap::formatText( QFontMetrics &fm, const QRect & r, int /*flags*/, const QString & str, int len )
{
    // The wordwrap algorithm
    // The variable names and the global shape of the algorithm are inspired
    // from QTextFormatterBreakWords::format().
    //kdDebug() << "KWordWrap::formatText " << str << " r=" << r.x() << "," << r.y() << " " << r.width() << "x" << r.height() << endl;
    KWordWrap* kw = new KWordWrap;
    if ( len == -1 )
        kw->m_text = str;
    else
        kw->m_text = str.left( len );
    int height = fm.height();
    if ( len == -1 )
        len = str.length();
    int lastBreak = -1;
    int lineWidth = 0;
    int x = 0;
    int y = 0;
    int w = r.width();
    int textwidth = 0;
    for ( int i = 0 ; i < len; ++i )
    {
        QChar c = str[i];
        int ww = fm.charWidth( str, i );
        bool isBreakable = c.isSpace() || c.isPunct() || c.isSymbol();
        /*kdDebug() << "c='" << QString(c) << "' i=" << i << "/" << len
                  << " x=" << x << " ww=" << ww << " w=" << w
                  << " lastBreak=" << lastBreak << " isBreakable=" << isBreakable << endl;*/
        int breakAt = -1;
        if ( x + ww > w && lastBreak != -1 ) // time to break and we know where
            breakAt = lastBreak;
        if ( x + ww > w - 4 && lastBreak == -1 ) // time to break but found nowhere [-> break here]
            breakAt = i;
        if ( i == len - 2 && x + ww + fm.charWidth( str, i+1 ) > w ) // don't leave the last char alone
            breakAt = lastBreak == -1 ? i - 1 : lastBreak;
        if ( breakAt != -1 )
        {
            //kdDebug() << "KWordWrap::formatText breaking after " << breakAt << endl;
            kw->m_breakPositions.append( breakAt );
            int thisLineWidth = lastBreak == -1 ? x + ww : lineWidth;
            kw->m_lineWidths.append( thisLineWidth );
            textwidth = QMAX( textwidth, thisLineWidth );
            x = 0;
            y += height;
            if ( lastBreak != -1 )
            {
                // Breakable char was found, restart from there
                i = lastBreak;
                lastBreak = -1;
                continue;
            }
        } else if ( isBreakable )
        {
            lastBreak = i;
            lineWidth = x + ww;
        }
        x += ww;
    }
    textwidth = QMAX( textwidth, x );
    kw->m_lineWidths.append( x );
    y += height;
    //kdDebug() << "KWordWrap::formatText boundingRect:" << r.x() << "," << r.y() << " " << textwidth << "x" << y << endl;
    kw->m_boundingRect.setRect( 0, 0, textwidth, y );
    return kw;
}

QString KWordWrap::wrappedString() const
{
    // We use the calculated break positions to insert '\n' into the string
    QString ws;
    int start = 0;
    QValueList<int>::ConstIterator it = m_breakPositions.begin();
    for ( ; it != m_breakPositions.end() ; ++it )
    {
        int end = (*it);
        ws += m_text.mid( start, end - start + 1 ) + '\n';
        start = end + 1;
    }
    ws += m_text.mid( start );
    return ws;
}

void KWordWrap::drawText( QPainter *painter, int textX, int textY, int flags ) const
{
    //kdDebug() << "KWordWrap::drawText text=" << wrappedString() << " x=" << textX << " y=" << textY << endl;
    // We use the calculated break positions to draw the text line by line using QPainter
    int start = 0;
    int y = 0;
    QFontMetrics fm = painter->fontMetrics();
    int height = fm.height(); // line height
    int ascent = fm.ascent();
    int maxwidth = m_boundingRect.width();
    QValueList<int>::ConstIterator it = m_breakPositions.begin();
    QValueList<int>::ConstIterator itw = m_lineWidths.begin();
    for ( ; it != m_breakPositions.end() ; ++it, ++itw )
    {
        int end = (*it);
        int x = textX;
        if ( flags & Qt::AlignHCenter )
            x += ( maxwidth - *itw ) / 2;
        else if ( flags & Qt::AlignRight )
            x += maxwidth - *itw;
        painter->drawText( x, textY + y + ascent, m_text.mid( start, end - start + 1 ) );
        y += height;
        start = end + 1;
    }
    // Draw the last line
    int x = textX;
    if ( flags & Qt::AlignHCenter )
        x += ( maxwidth - *itw ) / 2;
    else if ( flags & Qt::AlignRight )
        x += maxwidth - *itw;
    painter->drawText( x, textY + y + ascent, m_text.mid( start ) );
}

#include "kiconview.moc"
