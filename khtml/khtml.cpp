/*
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
//----------------------------------------------------------------------------
//
// KDE HTML Widget -- Main Widget
// $Id$

#define BORDER 5

// width defined in qscrollbar.cpp
// unfortunatly, I couldn't find a way to get this value directly, since
// QScrollView::verticalScrollBar()->width() gives a wrong value, as
// long as the scrollbar wasn't shown
#define SCROLLBARWIDTH 16

#ifdef GrayScale
#undef GrayScale
#endif

#ifdef Color
#undef Color
#endif

#include "khtml.h"

#include "khtmlio.h"
#include "khtmldecoder.h"
#include "khtmldata.h"

#include "html_element.h"
#include "html_elementimpl.h"
#include "html_baseimpl.h"
#include "html_documentimpl.h"

#include <assert.h>
#include <stdio.h>
#include <qpainter.h>

#include <kimgio.h>
#include <kstddirs.h>
#include <kglobal.h>

#define PRINTING_MARGIN		36	// printed margin in 1/72in units
#define TIMER_INTERVAL		30	// ms between parser parses

KHTMLWidget::KHTMLWidget( QWidget *parent, const char *name )
    : QScrollView( parent, name, WPaintClever )
{
    decoder = 0;

    settings      = 0;
    colorContext  = 0;
    pressed       = false;
    _isFrame      = false;
    painter       = 0;
    granularity   = 1000;
    linkCursor    = arrowCursor;
    bIsTextSelected = false;
    _width = width()- SCROLLBARWIDTH - 10;

    mapPendingFiles.setAutoDelete( true );

    defaultSettings = new HTMLSettings;

    cache = new KHTMLCache(this);

    QPalette pal = palette().copy();
    QColorGroup cg = pal.normal();
    QColorGroup newGroup( cg.foreground(), lightGray, cg.light(),
	    cg.dark(), cg.mid(), cg.text(), lightGray );
    pal.setNormal( newGroup );
    setPalette( pal );

    setBackgroundColor( lightGray );

    QString f =  locate("data", "khtml/pics/khtml_dnd.png");
    dndDefaultPixmap.load( f.data() );

    kimgioRegister();

    // initialize QScrollview
    enableClipper(true);
    setMouseTracking( true );
    viewport()->setMouseTracking(true);

    document = 0;

    connect( &autoScrollYTimer, SIGNAL(timeout()), SLOT( slotAutoScrollY() ) );
}

KHTMLWidget::~KHTMLWidget()
{
    if ( painter )
    {
  	painter->end();
  	delete painter;
    }

    if(document)
    {
	document->detach();
	document->deref();
    }

    if(defaultSettings) delete defaultSettings;
    if(cache) delete cache;
    if(decoder) delete decoder;
}

//
// Cache handling
//
///////////////////////////////////////////////////


void KHTMLWidget::preloadImage( QString _filename)
{
    KHTMLCache::preload( _filename, KHTMLCache::Persistent );
}

void KHTMLWidget::setCacheSize( int size )
{
    KHTMLCache::setSize( size );
}

int KHTMLWidget::cacheSize()
{
    return KHTMLCache::size();
}


//
// File handling
//
///////////////////////////////////////////////////
void KHTMLWidget::requestFile( HTMLFileRequester *_obj, QString _url,
			       bool )
{
  printf("==== REQUEST %s  ====\n", _url.latin1() );

  HTMLPendingFile *p = mapPendingFiles[ _url ];
  if ( p )
  {
    p->m_lstClients.append( _obj );
    return;
  }

  p = new HTMLPendingFile( _url, _obj );
  mapPendingFiles.insert( _url, p );

  emit fileRequest( _url );
}

void KHTMLWidget::cancelRequestFile( HTMLFileRequester *_obj )
{
  QDictIterator<HTMLPendingFile> it( mapPendingFiles );
  for( ; it.current(); ++it )
  {
    it.current()->m_lstClients.removeRef( _obj );
    if ( it.current()->m_lstClients.count() == 0 )
    {
      emit cancelFileRequest( it.current()->m_strURL );
      mapPendingFiles.remove( it.currentKey() );
    }
  }
}

void KHTMLWidget::cancelRequestFile( QString _url )
{
  mapPendingFiles.remove( _url );
  emit cancelFileRequest( _url );
}

void KHTMLWidget::cancelAllRequests()
{
  QDictIterator<HTMLPendingFile> it( mapPendingFiles );
  for( ; it.current(); ++it )
    emit cancelFileRequest( it.current()->m_strURL );
  mapPendingFiles.clear();
}

void KHTMLWidget::data( QString _url, const char *_data, int _len, bool _eof )
{
  HTMLPendingFile *p = mapPendingFiles[ _url ];
  if ( !p )
    return;

  if ( !p->m_buffer.isOpen() )
    p->m_buffer.open( IO_WriteOnly );
  p->m_buffer.writeBlock( _data, _len );

  HTMLFileRequester* o;
  for( o = p->m_lstClients.first(); o != 0L; o = p->m_lstClients.next() )
    o->fileLoaded( _url, p->m_buffer, _eof );

  if ( _eof )
    {
    p->m_buffer.close();
    mapPendingFiles.remove( _url );
  }
  else
    return;

  /// ### FIXME: parser has to finish too...
  if ( mapPendingFiles.count() == 0 )
  {
      //emit documentDone();
      cache->flush();
  }
}

void KHTMLWidget::slotFileLoaded( QString _url, QString _filename )
{
  printf("khtml: FileLoaded %s %s\n",
	 _url.latin1(), _filename.latin1() );

  HTMLPendingFile *p = mapPendingFiles[ _url ];
  if ( !p )
    return;

  assert( !p->m_buffer.isOpen() );

  HTMLFileRequester* o;
  for( o = p->m_lstClients.first(); o != 0L; o = p->m_lstClients.next() )
    o->fileLoaded( _url, _filename );

  mapPendingFiles.remove( _url );

  /// ### FIXME: parser has to finish too...
  if ( mapPendingFiles.count() == 0 )
  {
      //emit documentDone();
      cache->flush();
  }
}

void KHTMLWidget::slotFormSubmitted( QString _method, QString _url, const char *_data, QString _target )
{
    emit formSubmitted( _method, _url, _data, _target );
}

//
// Painting
//
///////////////

void KHTMLWidget::resizeEvent ( QResizeEvent * event )
{
    printf("resizeEvent\n");
    QScrollView::resizeEvent(event);
    layout();
    emit resized( event->size() );
}

void KHTMLWidget::drawContents ( QPainter * p, int clipx,
				 int clipy, int clipw, int cliph )

{
    if(!document) return;
    NodeImpl *body = document->body();
    if(!body) return;
    // ### fix this for frames...
    body->print(p, clipx, clipy, clipw, cliph, 0, 0);
}

void KHTMLWidget::paintElement( NodeImpl *e, bool recursive )
{
    int xPos, yPos;
    e->getAbsolutePosition(xPos, yPos);
    int yOff = contentsY();
    if(yOff > yPos+e->getDescent() ||
       yOff+visibleHeight() < yPos-e->getAscent())
	return;

    QWidget *vp = viewport();
    QPainter p(vp);
    int xOff = contentsX()+vp->x();
    yOff += vp->y();
    p.translate( -xOff, -yOff );
    if(recursive)
	e->print(&p, xOff, yOff, vp->width(), vp->height(),
		 xPos - e->getXPos(), yPos - e->getYPos());
    else
	e->printObject( &p , xOff, yOff, vp->width(), vp->height(),
			xPos , yPos );
}

void KHTMLWidget::layout()
{
    //### take care of frmaes (hide scrollbars,...)
    
    if(!document) return;
    NodeImpl *body = document->body();
    if(!body) return;

    int w = width() - SCROLLBARWIDTH - 5;
    if(w < _width-5 || w > _width + 10)
    {
	printf("layouting document\n");

	_width = w;

	document->setAvailableWidth(_width);
	document->layout(true);
	resizeContents(document->getWidth(), document->getHeight());
	viewport()->repaint(true);
    }
}


//
// Event Handling
//
/////////////////

void KHTMLWidget::viewportMousePressEvent( QMouseEvent *_mouse )
{
    if(!document) return;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    printf("\nmousePressEvent: x=%d, y=%d\n", xm, ym);


    // Make this frame the active one
    if ( _isFrame && !_isSelected )
    {
	// find top level frame
	KHTMLWidget *w = this;
	while(w->isFrame())
	    w = static_cast<KHTMLWidget *>(w->parentWidget());
	w->setSelected(this);
    }

    DOMString url;
    document->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MousePress, 0, 0, url );

    if(url != 0)
    {
	printf("mouseEvent: overURL %s\n", url.string().latin1());
	pressedURL = url.string();
    }
    else
	pressedURL = QString::null;

    if ( _mouse->button() == LeftButton || _mouse->button() == MidButton )
    {
    	pressed = TRUE;
	// deselect all currently selected text
	if ( bIsTextSelected )
	{
	    bIsTextSelected = false;
	    selectText( 0, 0, 0, 0 );	// deselect all text
	    emit textSelected( false );
	}
	// start point for text selection
	selectPt1.setX( _mouse->pos().x() + contentsX());
	selectPt1.setY( _mouse->pos().y() + contentsY());
    }
    press_x = _mouse->pos().x();
    press_y = _mouse->pos().y();
	
#if 0
    HTMLObject *obj;

    obj = clue->checkPoint( _mouse->pos().x() + contentsX() - leftBorder,
	    _mouse->pos().y() + contentsY() - topBorder );

    pressedURL = "";
    pressedTarget = "";

    if ( obj != 0)
    {
	if ( obj->getURL().length() )
	{
	    // Save data. Perhaps the user wants to start a drag.
	    if ( _mouse->button() == LeftButton || _mouse->button() == MidButton )
	    {
		pressedURL = obj->getURL();
		pressedTarget = obj->getTarget();
	    }
	
	    // Does the parent want to process the event now ?
	    if ( htmlView )	
	    {
		if ( htmlView->mousePressedHook( obj->getURL(), obj->getTarget(),
						 _mouse, obj->isSelected() ) )
		    return;
	    }
	
	    if ( _mouse->button() == RightButton )
	    {
		emit popupMenu(obj->getURL(),mapToGlobal( _mouse->pos() ));
		return;
	    }
	    return;
	}
    }

    if ( htmlView )	
      if ( htmlView->mousePressedHook( 0, 0L, _mouse, FALSE ) )
	return;
    if ( _mouse->button() == RightButton )
	emit popupMenu( 0, mapToGlobal( _mouse->pos() ) );
#endif
}

void KHTMLWidget::viewportMouseDoubleClickEvent( QMouseEvent *_mouse )
{
    if(!document) return;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    printf("\nmouseDblClickEvent: x=%d, y=%d\n", xm, ym);

    DOMString url;
    document->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MouseDblClick, 0, 0, url );

    if ( url.length() )
	emit doubleClick( url.string(), _mouse->button() );
}

void KHTMLWidget::viewportMouseMoveEvent( QMouseEvent * _mouse )
{
    if(!document) return;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    DOMString url;
    document->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MouseMove, 0, 0, url );

    if ( !pressed && url.length() )
    {
	QString surl = url.string();
	if ( overURL.isEmpty() )
	{
	    setCursor( linkCursor );
	    overURL = surl;
	    emit onURL( overURL );
	}
	else if ( overURL != surl )
	{
	    emit onURL( surl );
	    overURL = surl;
	}
	return;
    }
    else if( overURL.length() && !url.length() )
    {
	setCursor( arrowCursor );
	emit onURL( 0 );
	overURL = "";
    }

#if 0
    // debugT(">>>>>>>>>>>>>>>>>>> Move detected <<<<<<<<<<<<<<<<<<<\n");

    // Does the parent want to process the event now ?
    if ( htmlView )
    {
	if ( htmlView->mouseMoveHook( _mouse ) )
	    return;
    }

    // text selection
    if ( pressed && pressedURL.isEmpty() )
    {
	QPoint point = _mouse->pos();
	if ( point.y() > height() )
	    point.setY( height() );
	else if ( point.y() < 0 )
	    point.setY( 0 );
	selectPt2.setX( point.x() + contentsX() - leftBorder );
	selectPt2.setY( point.y() + contentsY() - topBorder );
	if ( selectPt2.y() < selectPt1.y() )
	{
	    selectText( selectPt2.x(), selectPt2.y(),
		selectPt1.x(), selectPt1.y() );
	}
	else
	{
	    selectText( selectPt1.x(), selectPt1.y(),
		selectPt2.x(), selectPt2.y() );
	}

	// Do we need to scroll because the user has moved the mouse
	// outside the widget bounds?
	if ( _mouse->pos().y() > height() )
	{
	    autoScrollY( 100, 20 );
	    connect( this, SIGNAL( scrollVert(int) ),
		SLOT( slotUpdateSelectText(int) ) );
	}
	else if ( _mouse->pos().y() < 0 )
	{
	    autoScrollY( 100, -20 );
	    connect( this, SIGNAL( scrollVert(int) ),
		SLOT( slotUpdateSelectText(int) ) );
	}
	else
	{
	    stopAutoScrollY();
	    disconnect( this, SLOT( slotUpdateSelectText(int) ) );
	}
    }

    // Drags are only started with the left mouse button ...
    // if ( _mouse->button() != LeftButton )
    // return;

    // debugT("Testing pressedURL.isEmpty()\n");
    if ( pressedURL.isEmpty() )
	return;

    int x = _mouse->pos().x();
    int y = _mouse->pos().y();

    // debugT("Testing Drag\n");

    // Did the user start a drag?
    if ( abs( x - press_x ) > 5 || abs( y - press_y ) > 5 )
    {
	// Does the parent want to process the event now ?
	if ( htmlView )
        {
	    if ( htmlView->dndHook( pressedURL.data() ) )
		return;
	}
	
	QStrList uris;
	uris.append(pressedURL.data());

	QUriDrag *ud = new QUriDrag(uris, this);
	ud->setPixmap(dndDefaultPixmap);
	ud->dragCopy();
    }
#endif
}

void KHTMLWidget::viewportMouseReleaseEvent( QMouseEvent * _mouse )
{
    if ( !document ) return;

    if ( pressed )
    {
	// in case we started an autoscroll in MouseMove event
	stopAutoScrollY();
	disconnect( this, SLOT( slotUpdateSelectText(int) ) );
    }

    // Used to prevent mouseMoveEvent from initiating a drag before
    // the mouse is pressed again.
    pressed = false;

    int xm, ym;
    viewportToContents(_mouse->x(), _mouse->y(), xm, ym);

    printf("\nmouseReleaseEvent: x=%d, y=%d\n", xm, ym);

    DOMString url;
    document->mouseEvent( xm, ym, _mouse->stateAfter(), DOM::NodeImpl::MouseRelease, 0, 0, url );

#if 0
    if ( clue->mouseEvent( _mouse->x() + contentsX() - leftBorder,
	    _mouse->y() + contentsY() - topBorder, _mouse->button(),
	    _mouse->state() ) )
	return;

    // Does the parent want to process the event now ?
    if ( htmlView )
    {
      if ( htmlView->mouseReleaseHook( _mouse ) )
	return;
    }

    // emit textSelected() if necessary
    if ( _mouse->button() == LeftButton || _mouse->button() == MidButton )
    {
	if ( bIsTextSelected )
	{
	    debugM( "Text Selected\n" );
	    emit textSelected( true );
	}
    }
    if ( clue == 0 )
	return;
#endif
    if ( pressedURL.isNull() )
	return;

    // if ( pressedURL.data()[0] == '#' )
    //	gotoAnchor( pressedURL.data() + 1 );
    // else
    if ( _mouse->button() != RightButton )
    {
	printf("pressedURL='%s'\n",pressedURL.data());
	emit URLSelected( pressedURL.data(), _mouse->button(), pressedTarget.data() );
    }
}
//
// Selection
//
/////////////////////////////
#if 0
/*
 * Checks out wether there is a URL under the point p and returns a pointer
 * to this URL or 0 if there is none.
 */
QString KHTMLWidget::getURL( QPoint &p )
{
    if ( clue == 0 )
	return 0;

    HTMLObject *obj;

    obj = clue->checkPoint( p.x() + contentsX(), p.y() + contentsY() );

    if ( obj != 0)
	if ( obj->getURL().length() )
	    return obj->getURL();

    return 0;
}
#endif

void KHTMLWidget::select( QRect &/*_rect*/ )
{
#if 0
    if ( clue == 0 )
	return;

    QRect r = _rect;

    r.moveBy( -contentsX(), -contentsY() );

    int tx = -contentsX() + leftBorder;
    int ty = -contentsY() + topBorder;

    HTMLChain chain;

    clue->select( this, &chain, r, tx, ty );
#endif
}

void KHTMLWidget::selectText( int /*_x1*/, int /*_y1*/, int /*_x2*/, int /*_y2*/ )
{
#if 0
    if ( clue == 0 )
	return;

    int tx = -contentsX() + leftBorder;
    int ty = -contentsY() + topBorder;

    HTMLChain chain;

    bIsTextSelected = clue->selectText(this,&chain,_x1, _y1, _x2, _y2, tx, ty);
#endif
}

void KHTMLWidget::findTextBegin()
{
#if 0
    if ( clue == 0 )
	return;

    findTextEnd();

    textFindIter = new HTMLListIterator( clue );
#endif
}

bool KHTMLWidget::findTextNext( const QRegExp &/*exp*/ )
{
#if 0
    if ( clue == 0 )
	return false;

    HTMLObject *obj;

    if ( !textFindIter )
	findTextBegin();

    selectText( 0, 0, 0, 0 );	// deselect all text

    // loop until we match the regexp, or reach the end of the document.
    do
    {
	obj = textFindIter->current();
	textFindIter->next();
    }
    while ( obj && !obj->selectText( exp ) );

    if ( obj )
    {
	// We found a match - highlight
	int x = 0, y = 0;

	clue->getObjectPosition( obj, x, y );
	if ( y < contentsY() || y > contentsY() + height() - obj->getHeight() )
	{
	    void ensureVisible ( int x, int y );
	}
	paintSingleObject( obj );
    }
    else
    {
	// end of document reached.
	findTextEnd();
    }

    return ( obj != 0 );
#endif
    return false;
}

void KHTMLWidget::findTextEnd()
{
#if 0
    if ( textFindIter )
    {
	delete textFindIter;
	textFindIter = 0;
	selectText( 0, 0, 0, 0 );	// deselect all text
    }
#endif
}

#if 0
void KHTMLWidget::paintEvent( QPaintEvent* _pe )
{
    bool newPainter = FALSE;

    HTMLBodyElementImpl *body = document->body();
    if ( body == 0 )
	return;

    body->paint
    if ( painter == 0 )
    {
	painter = new QPainter;
	painter->begin( this );
	newPainter = TRUE;
    }

    painter->setBackgroundColor( settings->bgColor );

    if (background)
    {
        background->print( painter, _pe->rect().x() + contentsX(),
	    _pe->rect().y() + contentsY(),
	    _pe->rect().width(), _pe->rect().height(), -contentsX(), -contentsY(), false );
    }

    // painter->translate( contentsX(), -contentsY() );
    int tx = -contentsX() + leftBorder;
    int ty = -contentsY() + topBorder;

    clue->print( painter, _pe->rect().x() - tx, _pe->rect().y() - ty,
	    _pe->rect().width(), _pe->rect().height(), tx, ty, false );

    if ( _isSelected )
    {
	QPen pen = painter->pen();
	painter->setPen( black );
	QBrush brush;
	painter->setBrush( brush );
	painter->drawRect( 0, 0, width(), height() );
	painter->drawRect( 1, 1, width() - 2, height() - 2 );
	painter->setPen( pen );
    }

    if ( newPainter )
    {
	painter->end();
	delete painter;
	painter = 0;
    }
}
#endif

#if 0
void KHTMLWidget::keyPressEvent( QKeyEvent *_ke )
{
    switch ( _ke->key() )
    {
	case Key_Down:
	    {
		if ( docHeight() < height() ) break;
		int newY = contentsY() + 20;
		if ( newY > docHeight() - height() )
		    newY = docHeight() - height();
		slotScrollVert( newY );
		emit scrollVert( newY );
		
		flushKeys();
	    }
	    break;

	case Key_Next:
	    {
	      if ( docHeight() < height() ) break;
	      int newY = contentsY() + height() - 20;
	      if ( newY > docHeight() - height() )
		newY = docHeight() - height();
	      slotScrollVert( newY );
	      emit scrollVert( newY );
		
	      flushKeys();
	    }
	    break;

	case Key_Up:
	    {
		if ( docHeight() < height() ) break;
		int newY = contentsY() - 20;
		if ( newY < 0 )
		    newY = 0;
		slotScrollVert( newY );
		emit scrollVert( newY );
	
		flushKeys();
	    }
	    break;

	case Key_Prior:
	    {
		if ( docHeight() < height() ) break;
		int newY = contentsY() - height() + 20;
		if ( newY < 0 )
		    newY = 0;
		slotScrollVert( newY );
		emit scrollVert( newY );

		flushKeys();
	    }
	    break;

	case Key_Right:
	    {
		if ( docWidth() < width() ) break;
		int newX = contentsX() + 20;
		if ( newX > docWidth() - width() )
		    newX = docWidth() - width();
		slotScrollHorz( newX );
		emit scrollHorz( newX );
		
		flushKeys();
	    }
	    break;

	case Key_Left:
	    {
		if ( docWidth() < width() ) break;
		int newX = contentsX() - 20;
		if ( newX < 0 )
		  newX = 0;
		slotScrollHorz( newX );
		emit scrollHorz( newX );

		flushKeys();
	    }
	    break;

	default:
	    QWidget::keyPressEvent( _ke );
    }
}

// Little routine from Alessandro Russo to flush extra keypresses from
// the event queue
void KHTMLWidget::flushKeys()
{
    XEvent ev_return;
    Display *dpy = qt_xdisplay();
    while ( XCheckTypedEvent( dpy, KeyPress, &ev_return ) );
}

void KHTMLWidget::paintSingleObject( HTMLObject *_obj )
{
    bool newPainter = FALSE;

//    if ( parser )
//	return;

    if ( clue == 0 )
	return;

    if ( painter == 0 )
    {
	painter = new QPainter;
	painter->begin( this );
	newPainter = TRUE;
    }

    int absx, absy;


    if ( ( absx = _obj->getAbsX() ) >= 0 )
    {
	absy = _obj->getAbsY();
	_obj->print( painter, absx - contentsX() + leftBorder - _obj->getXPos(),
	    absy - contentsY() + topBorder - (_obj->getYPos()-_obj->getAscent()) );
    }
    else
    {
	int tx = -contentsX() + leftBorder;
	int ty = -contentsY() + topBorder;
	
	clue->print( painter, _obj, contentsX(), contentsY(),
		     width(), height(), tx, ty );
    }

    if ( newPainter )
    {
	painter->end();
	delete painter;
	painter = 0;
    }
}

void KHTMLWidget::paint( HTMLChain *_chain, int x, int y, int w, int h )
{
    bool newPainter = FALSE;

    if ( clue == 0 )
	return;

    _chain->first();

    if ( _chain->current() )
    {
	if ( painter == 0 )
	{
	    painter = new QPainter;
	    painter->begin( this );
	    newPainter = TRUE;
	}
        if (background)
        {
            background->print( painter, x + contentsX(), y + contentsY(),
			w, h, -contentsX(), -contentsY(), false );
        }
	int tx = -contentsX() + leftBorder;
	int ty = -contentsY() + topBorder;

	_chain->current()->print( painter, _chain, x - tx,
		y - ty, w, h, tx, ty );

	if ( newPainter )
	{
	    painter->end();
	    delete painter;
	    painter = 0;
	}
    }
}

void KHTMLWidget::scheduleUpdate()
{
    if ( !updateTimer.isActive() )
      updateTimer.start( 100, true );
}

#endif

void KHTMLWidget::getSelected( QStringList &/*_list*/ )
{
#if 0
    if ( clue == 0 )
	return;

    clue->getSelected( _list );
#endif
}

void KHTMLWidget::getSelectedText( QString &/*_str*/ )
{
#if 0
    if ( clue == 0 )
	return;

    clue->getSelectedText( _str, false );
#endif
}

#if 0
void KHTMLWidget::getFrameSetText( QString &_str, HTMLFrameSet *f)
{
    for( QWidget *w = f->widgetList.first(); w != 0;
         w = f->widgetList.next() )
    {
        if ( w->inherits( "KHTMLView" ) )
        {
            KHTMLView *v = (KHTMLView*)w;
            _str += "\n[FRAME]\n";
            v->getAllText( _str );
	}
	else if(strcmp(w->className(),"HTMLFrameSet") == 0 )
	{
	    HTMLFrameSet *frameSet = (HTMLFrameSet *)w;
            getFrameSetText( _str, frameSet);
	}
    }
}
#endif

void KHTMLWidget::getAllText( QString &/*_str*/ )
{
#if 0
    if ( clue )
        clue->getSelectedText( _str, true );

    if(isFrameSet() && frameSet)
    {
        getFrameSetText( _str, frameSet );
    }
#endif
}

// Print the current document to the printer.
// This currently prints the entire document without releasing control
// to the event loop.  This isn't a problem for small documents, but
// may be annoying for very large documents.  If this is changed in
// the future, it should be noted that this widget CANNOT be redrawn
// while it is printing as its layout is recalculated to suit the paper
// size it is being printed on.
//
void KHTMLWidget::print()
{
#if 0
    float scalers[] = { 1.1, 1.0, 0.9, 0.75, 0.6, 0.4 };
    QPrinter printer;

    if ( printer.setup( 0 ) )
    {
	bool newPainter = false;
	int pgWidth = 595, pgHeight = 842;

	switch ( printer.pageSize() )
	{
	    case QPrinter::A4:
		pgWidth = 595;
		pgHeight = 842;
		break;

	    case QPrinter::B5:
		pgWidth = 516;
		pgHeight = 729;
		break;

	    case QPrinter::Letter:
		pgWidth = 612;
		pgHeight = 792;
		break;

	    case QPrinter::Legal:
		pgWidth = 612;
		pgHeight = 1008;
		break;

	    case QPrinter::Executive:
		pgWidth = 540;
		pgHeight = 720;
		break;

            default:
	      assert( 0 );
	}

	if ( printer.orientation() == QPrinter::Landscape )
	{
	    int tmp = pgWidth;
	    pgWidth = pgHeight;
	    pgHeight = tmp;
	}

	pgWidth -= ( 2*PRINTING_MARGIN );
	pgHeight -= ( 2*PRINTING_MARGIN );

	pgWidth = (int) ((float)pgWidth / scalers[ settings->fontBaseSize ] );
	pgHeight = (int) ((float)pgHeight / scalers[ settings->fontBaseSize ] );

	int margin = (int)((float)PRINTING_MARGIN /
		scalers[ settings->fontBaseSize ] );

	QPainter prPainter;
	prPainter.begin( &printer );

	prPainter.scale( scalers[ settings->fontBaseSize ],
		scalers[ settings->fontBaseSize ] );

	clue->recalcBaseSize( &prPainter );
	clue->reset();
	clue->setAvailableWidth( pgWidth );
	clue->calcSize();
	clue->setPos( 0, clue->getAscent() );

	unsigned numBreaks = 1;
	int pos = 0;
	QArray<int> breaks( 10 );
	breaks[0] = 0;

	do
	{
	    debugM( "Break pos = %d\n", pos );
	    pos = clue->findPageBreak( pos + pgHeight );
	    if ( pos >= 0 )
	    {
		breaks[ numBreaks ] =  pos;
		numBreaks++;
		if ( numBreaks == breaks.size() )
		    breaks.resize( numBreaks + 10 );
	    }
	}
	while ( pos > 0 );

	for ( unsigned b = 0; b < numBreaks; b++ )
	{
	    int printHeight;
	    if ( b < numBreaks - 1 )
		printHeight = breaks[b+1] - breaks[b];
	    else
		printHeight = pgHeight;
	    clue->print( &prPainter, 0, breaks[b], pgWidth, printHeight,
	    margin, margin-breaks[b], true );
	    if ( b < numBreaks - 1 )
	    {
		printer.newPage();
	    }
	}

	prPainter.end();

	if ( painter == 0 )
	{
	    painter = new QPainter;
	    painter->begin( this );
	    newPainter = TRUE;
	}

	clue->recalcBaseSize( painter );
	calcSize();

	if ( newPainter )
	{
	    painter->end();
	    delete painter;
	    painter = 0;
	}
    }
#endif
}
//
// Parsing
//
////////////////////////////

// ### fix offset
void KHTMLWidget::begin( QString _url, int /*_x_offset*/, int /*_y_offset*/ )
{
    debug("KHTMLWidget::begin(....)");

    //bIsFrameSet = FALSE;
    // bIsFrame = FALSE;
    //bFramesComplete = FALSE;
    //framesetList.clear();
    //frameList.clear();
    //embededFrameList.clear();
    //usedImageURLs.clear();
    //usedHrefURLs.clear();

    //findTextEnd();

    if ( bIsTextSelected )
    {
	bIsTextSelected = false;
	emit textSelected( false );
    }

    //emit scrollHorz( _x_offset );
    //emit scrollVert( _y_offset );

    stopParser();

//    reference = QString::null;
//    actualURL = "";
//    baseURL = "";

    if ( !_url.isEmpty() )
    {
        actualURL = _url;
//        reference = actualURL.ref();
//        setBaseURL( _url);

        // Set a default title
        KURL title(_url);
        title.setRef(QString::null);
        title.setQuery(QString::null);
        emit setTitle( title.url().data() );
    }
    else
    {
        emit setTitle( "* Unknown *" );
    }

    if ( painter )
    {
	painter->end();
	delete painter;
	painter = 0;
    }

    if(document)
    {
	document->detach();
	document->deref();
    }
    document = new HTMLDocumentImpl(this, cache);
    document->ref();
    document->open();
    // clear widget
    resizeContents(0, 0);
    setBackgroundMode(PaletteBackground);
    viewport()->repaint(true);

    emit documentStarted();

    writing = true;
}

void KHTMLWidget::write( const char *_str)
{
    if(!decoder) decoder = new KHTMLDecoder();

    if ( _str == 0 )
	return;

    QString decoded = decoder->decode( _str );
    document->write( decoded );
}

void KHTMLWidget::write( const QString &_str)
{
    if ( _str == QString::null )
	return;

    document->write( _str );

}

void KHTMLWidget::end()
{
    writing = false;
    document->close();
    //viewport()->repaint(true);
}

#if 0
void KHTMLWidget::parse()
{
//    emit documentStarted();

    // Dont parse an existing framed document twice.
    // If parse is called two times after begin() then
    // the second call is ususally done because the widget
    // has been resized.
    if (parser)
        return;

/*
    if ( bIsFrameSet && frameSet )
    {
	frameSet->resize( width(), height() );
	return;
    }
*/
    // if there is no tokenizer then no html has been added
    if ( !ht )
	return;
	
    painter = new QPainter();
    painter->begin( this );

/*
    if ( colorContext )
    {
	QColor::leaveAllocContext();
	QColor::destroyAllocContext( colorContext );
    }

    colorContext = QColor::enterAllocContext();
*/

    if ( settings )
    	delete settings;

    settings = new HTMLSettings( *defaultSettings);

    allocator = new HTMLAllocator( 128*1024 ); // Allocate in chunks of 128K

    clue = new(allocator) HTMLClueV();
    clue->setVAlign( HTMLClue::Top );
    clue->setHAlign( HTMLClue::Left );

    parser = new KHTMLParser( this, ht, painter, settings, 0, allocator );

    setBackgroundColor( settings->bgColor );

    // this will call timerEvent which in turn calls parseBody
    timerId = startTimer( TIMER_INTERVAL );
}

void KHTMLWidget::setBackground(HTMLBackground *_background)
{
    background = _background;
}
#endif

void KHTMLWidget::stopParser()
{
#if 0
    if ( !parser )
	return;

    if ( timerId != 0 )
	killTimer( timerId );

    delete parser;
    parser = 0;
#endif
}

void KHTMLWidget::calcSize()
{
#if 0
    if ( clue == 0 )
	return;

    clue->reset();

    int _max_width = width() - leftBorder - rightBorder;
    int _min_width = clue->calcMinWidth();

    if (_min_width > _max_width)
    {
        _max_width = _min_width;
    }

    clue->setAvailableWidth( _max_width );
    clue->calcSize();
    clue->setPos( 0, clue->getAscent() );

    emit documentChanged();
#endif
}


void KHTMLWidget::slotScrollVert( int _val )
{
    if(_val < 0) _val = 0;
    if(_val > docHeight()) _val = docHeight();

    setContentsPos(contentsX(), _val);
}

void KHTMLWidget::slotScrollHorz( int _val )
{
    if(_val < 0) _val = 0;
    if(_val > docWidth()) _val = docWidth();

    setContentsPos(_val, contentsY());
}

bool KHTMLWidget::gotoAnchor( QString /*reference*/)
{
#if 0
    int _x = 0;
    int _y = 0;

    if ( clue == 0 )
	return FALSE;

    HTMLAnchor *anchor = clue->findAnchor( reference, _x, _y );
    if ( anchor == 0 )
	return FALSE;

    // Is there more HTML to be expected?
    if (parser)	
    {
	// Check if the reference can be located at the top of the screen
        if (_y > docHeight() - height()  - 1)
             return FALSE;
    }

    emit scrollVert( _y );

#endif
    return TRUE;
}

void KHTMLWidget::autoScrollY( int _delay, int _dy )
{
    if ( _dy == 0 || ( _dy < 0 && contentsY() == 0 ) ||
	 ( _dy > 0 && contentsY() >= docHeight() - contentsHeight() - 1 ) )
    {
	stopAutoScrollY();
    }
    else if ( !autoScrollYTimer.isActive() )
    {
	autoScrollYDelay = _delay;
	autoScrollYTimer.start( _delay, true );
	autoScrollDY = _dy;
    }
}

void KHTMLWidget::stopAutoScrollY()
{
    if ( autoScrollYTimer.isActive() )
	autoScrollYTimer.stop();
}

void KHTMLWidget::slotAutoScrollY()
{
	if ( ( autoScrollDY > 0 &&
	       contentsY() < docHeight() - contentsHeight() - 1 ) ||
	     ( autoScrollDY < 0 && contentsY() > 0 ) )
	{
		int newY = contentsY() + autoScrollDY;
		if ( newY > docHeight() - contentsHeight()  - 1)
			newY = docHeight() - contentsHeight() - 1;
		else if ( newY < 0 )
			newY = 0;
		slotScrollVert( newY );
		emit scrollVert( newY );
		autoScrollYTimer.start( autoScrollYDelay, true );
	}
}

// used to update the selection when the user has caused autoscrolling
// by dragging the mouse out of the widget bounds.
void KHTMLWidget::slotUpdateSelectText( int )
{
#if 0
	if ( pressed )
	{
		QPoint point = QCursor::pos();
		point = mapFromGlobal( point );
		if ( point.y() > height() )
			point.setY( height() );
		else if ( point.y() < 0 )
			point.setY( 0 );
		selectPt2.setX( point.x() + contentsX() - leftBorder );
		selectPt2.setY( point.y() + contentsY() - topBorder );
		if ( selectPt2.y() < selectPt1.y() )
		{
			selectText( selectPt2.x(), selectPt2.y(),
				selectPt1.x(), selectPt1.y() );
		}
		else
		{
			selectText( selectPt1.x(), selectPt1.y(),
				selectPt2.x(), selectPt2.y() );
		}
	}
#endif
}

void KHTMLWidget::select( QPainter *, bool /*_select*/ )
{
#if 0
    if ( clue == 0 )
	return;

    int tx = -contentsX() + leftBorder;
    int ty = -contentsY() + topBorder;

    HTMLChain chain;

    clue->select( this, &chain, _select, tx, ty );
#endif
}

void KHTMLWidget::selectByURL( QPainter *, QString /*_url*/, bool /*_select*/ )
{
#if 0
    if ( clue == 0 )
	return;

    int tx = -contentsX() + leftBorder;
    int ty = -contentsY() + topBorder;

    HTMLChain chain;

    clue->selectByURL( this, &chain, _url, _select, tx, ty );
#endif
}

void KHTMLWidget::select( QPainter *, QRegExp& /*_pattern*/, bool /*_select*/ )
{
#if 0
    if ( clue == 0 )
	return;

    int tx = -contentsX() + leftBorder;
    int ty = -contentsY() + topBorder;

    HTMLChain chain;

    clue->select( this, &chain, _pattern, _select, tx, ty );
#endif
}

int KHTMLWidget::docWidth() const
{
    return document->getWidth();
}

int KHTMLWidget::docHeight() const
{
    return document->getHeight();
}

void KHTMLWidget::setSelected( KHTMLWidget *w )
{
    if ( !w)
	return;

  _isSelected = true;

  //### go through all child widgets and select/deselct them...
  
#if 0

  if ( !w )
  {
      bool isOld = TRUE;
      if ( !painter )
      {
	  painter = new QPainter;
	  painter->begin( this );
	  isOld = FALSE;
      }

      QPen pen = painter->pen();
      painter->setPen( black );
      painter->drawRect( 0, 0, width(), height() );
      painter->drawRect( 1, 1, width() - 2, height() - 2 );
      painter->setPen( pen );

      if ( !isOld )
      {	
	  painter->end();
	  delete painter;
	  painter = 0;
      }
  }
  else
    repaint( false );
#endif
}

#if 0
void KHTMLWidget::slotFrameSelected( KHTMLView *_view )
{
  if ( selectedFrame && selectedFrame != _view )
    selectedFrame->setSelected( FALSE );
  selectedFrame = _view;
}
#endif

KHTMLWidget* KHTMLWidget::getSelectedFrame()
{
#if 0
    if ( isFrame() && isSelected() )
    return this;

  if ( isFrameSet() )
  {
    HTMLFrameSet *f;
    for ( f = framesetList.first(); f != 0; f = framesetList.next() )
    {
      KHTMLView *v = f->getSelectedFrame();
      if ( v )
	return v;
    }
  }
#endif
  return 0;
}

void
KHTMLWidget::setFontSizes(const int *newFontSizes, const int *newFixedFontSizes)
{
    defaultSettings->setFontSizes(newFontSizes, newFixedFontSizes);
}

void
KHTMLWidget::getFontSizes(int *newFontSizes, int *newFixedFontSizes)
{
    defaultSettings->getFontSizes(newFontSizes, newFixedFontSizes);
}

void
KHTMLWidget::resetFontSizes(void)
{
    defaultSettings->resetFontSizes();
}

void
KHTMLWidget::setStandardFont( QString name )
{	
    defaultSettings->fontBaseFace = name;
}

void
KHTMLWidget::setFixedFont( QString name )
{	
    defaultSettings->fixedFontFace = name;
}

void
KHTMLWidget::setDefaultBGColor( const QColor &col )
{	
    defaultSettings->bgColor = col;
}

void
KHTMLWidget::setDefaultTextColors( const QColor &normal, const QColor &link,
				   const QColor &vlink )
{
    defaultSettings->fontBaseColor = normal;
    defaultSettings->linkColor = link;
    defaultSettings->vLinkColor = vlink;
}

void
KHTMLWidget::setUnderlineLinks( bool ul )
{
    defaultSettings->underlineLinks = ul;
}

#if 0

//
// Parser Interface Functions
//
/////////////////////////////

bool KHTMLWidget::URLVisited( QString _url )
{
    if ( htmlView )
	return htmlView->URLVisited( _url );

    return false;
}

void KHTMLWidget::setBaseURL( QString _url)
{
    baseURL = _url;
    baseURL.setRef( QString::null );
    baseURL.setQuery( QString::null );

    QString p = baseURL.path();
       // was httpPath(); is it really important NOT to decode it ? (David)

    if ( p.length() > 0 )
    {
        int pos = p.findRev( '/' );
        if ( pos >= 0 )
            p.truncate( pos );
    }
    p += "/";
    baseURL.setPath( p );
}

void KHTMLWidget::addFrameSet( HTMLFrameSet *_frameSet )
{
    if (!htmlView)
	return;

    htmlView->setIsFrameSet( true );
    framesetList.append( _frameSet );
}

void KHTMLWidget::showFrameSet( HTMLFrameSet *_frameSet )
{
    if (!htmlView)
	return;

    if ( selectedFrame )
        selectedFrame->setSelected( TRUE );

    _frameSet->parse();
    _frameSet->setGeometry( 0, 0, width(), height() );
    _frameSet->show();
}

void KHTMLWidget::addFrame( HTMLFrameSet *_frameSet, QString _name,
                            bool _scrolling, bool _resize,
                            int _frameborder,
                            int _marginwidth, int _marginheight,
                            QString _src)
{
    // Create the frame,
    KHTMLView *frame = htmlView->newView( _frameSet, _name.ascii() );
    frame->setIsFrame( TRUE );
    frame->setScrolling( _scrolling );
    frame->setAllowResize( _resize );
    frame->setFrameBorder( _frameborder );
    frame->setMarginWidth( _marginwidth );
    frame->setMarginHeight( _marginheight );
    _frameSet->append( frame );

    if ( !_src.isEmpty() )
    {
        // Determine the complete URL for this widget
	KURL u( baseURL, _src );
	connect( frame, SIGNAL( frameSelected( KHTMLView * ) ),
	         this, SLOT( slotFrameSelected( KHTMLView * ) ) );

        selectedFrame = frame;
	// Tell the new widget what it should show
	// frame->openURL( u.url().data() );
	frame->setCookie( u.url().data() );

	KHTMLView *top = frame->findView( "_top" );
	if ( top )
	{
	    connect( frame, SIGNAL( documentStarted( KHTMLView * ) ),
	             top, SLOT( slotDocumentStarted( KHTMLView * ) ) );
	    connect( frame, SIGNAL( documentDone( KHTMLView * ) ),
			  top, SLOT( slotDocumentDone( KHTMLView * ) ) );
	}
    }
    // html->show();
    // Add frame to list
    frameList.append( frame->getKHTMLWidget() );
}

void KHTMLWidget::addEmbededFrame( HTMLFrameSet *_frameSet, KHTMLEmbededWidget* _embed )
{
  _frameSet->append( _embed );

  embededFrameList.append( _embed );
}

#endif

void KHTMLWidget::setNewTitle( QString _title)
{
    emit setTitle( _title );
}

bool KHTMLWidget::setCharset(QString /*name*/, bool /*override*/){
    // ###### FIXME: decoder

}

bool KHTMLWidget::isFrameSet()
{
    return false;
}


#if 0
SavedPage *
KHTMLWidget::saveYourself(SavedPage *p)
{
    if( !p )
	p = new SavedPage();
    if( bIsFrame && htmlView )
    {
	// we need to store the frame info
	p->isFrame = true;
	p->scrolling = htmlView->getScrolling();
	p->frameborder = htmlView->getFrameBorder();
	p->marginwidth = leftBorder;
	p->marginheight = topBorder;
	p->allowresize = htmlView->allowResize();
    }
    p->url = getDocumentURL().url();
    p->xOffset = contentsX();
    p->yOffset = contentsY();
    p->forms = new QStringList();
    for ( HTMLForm *f = formList.first(); f != 0; f = formList.next() )
    {
	f->saveForm( p->forms );
    }

    if(isFrameSet() && !parser && frameSet)
	buildFrameTree(p, frameSet);

    return p;
}

void
KHTMLWidget::buildFrameTree(SavedPage *p, HTMLFrameSet *f)
{
    p->isFrameSet = true;
    //ok... now we need to store the content of all frames...
    p->frames = new QList<SavedPage>;
    p->frames->setAutoDelete(true);
    p->frameLayout = new FrameLayout;
    p->frameLayout->rows = f->rows;
    p->frameLayout->cols = f->cols;
    p->frameLayout->frameBorder = f->frameBorder;
    //FIXME
    //p->title = title;
    p->title = "";

    QWidget *w;
    for( (w = f->widgetList.first()); w != 0;
	 (w = f->widgetList.next()) )
    {
	if ( w->inherits( "KHTMLView" ) )
	{
	    KHTMLView *v = (KHTMLView*)w;
		SavedPage *s = v->saveYourself();
		p->frames->append( s );
	}
	else if(strcmp(w->className(),"HTMLFrameSet") == 0 )
	{
	    HTMLFrameSet *f = (HTMLFrameSet *)w;
	    SavedPage *s = new SavedPage;
	    buildFrameTree(s, f);
	    p->frames->append( s );
	}
    }
}

void
KHTMLWidget::restore(SavedPage *p)
{
    if( !p->isFrameSet )
    {
	// the easy part...
	if(htmlView)
	{
	    printf("restoring view\n");
	    printf("framename = %s\n",p->frameName.data());
	    printf("url = %s\n",p->url.data());
	    htmlView->openURL( p->url );
	    htmlView->setIsFrame( p->isFrame );
	    if( p->isFrame )
	    {
		htmlView->setFrameName( p->frameName );
		htmlView->setScrolling( p->scrolling );
		htmlView->setAllowResize( p->allowresize );
		htmlView->setFrameBorder( p->frameborder );
		htmlView->setMarginWidth( p->marginwidth );
		htmlView->setMarginHeight( p->marginheight );
	    }
	    htmlView->restorePosition(p->xOffset, p->yOffset);
	}
	else
	{
	    printf("NO VIEW!!!!\n");
	    emit URLSelected( p->url, LeftButton, 0L );
	}
    }
    else
    {
	// dirty hack, to get kfm to display the right url in the lineedit...
	htmlView->openURL( ("restored:" + p->url) );

	// we construct a html sequence, which represents the frameset to see
	QString s = "<html><head><title>\n";
	s += p->title;
	s += "</title></head><body>\n";
	buildFrameSet(p, &s);
	s += "</body></html>\n";

	printf("restoring frameset:\n%s\n", s.data());
	begin();
	parse();
	write(s.latin1());
	end();
	
	actualURL = p->url;
	reference = actualURL.ref();
	setBaseURL( p->url);
    }

}

void
KHTMLWidget::buildFrameSet(SavedPage *p, QString *s)
{
    QString tmp;

    if(!p->isFrameSet) return;

    QString aStr = "<frameset";
    if(!p->frameLayout->rows.isEmpty())
	aStr += " ROWS=\"" + p->frameLayout->rows + "\"";
    if(!p->frameLayout->cols.isEmpty())
	aStr += " COLS=\"" + p->frameLayout->cols + "\"";
    tmp.sprintf(" FRAMEBORDER=%d",p->frameLayout->frameBorder);
    aStr += tmp;
    if(!p->frameLayout->allowResize)
	aStr += " NORESIZE";
    aStr += ">\n";
    *s += aStr;

    SavedPage *sp;
    for( (sp = p->frames->first()); sp != 0; (sp = p->frames->next()) )
    {
	if(sp->isFrameSet)
	    buildFrameSet(sp, s);
	else
	{
	    aStr = "<frame src=\"";
	    aStr += sp->url;
	    aStr += "\" name=\"";
	    aStr += sp->frameName;
	    aStr += "\">";
	    // FIXME: other options...
	    aStr += "\n";
	    *s += aStr;
	}
    }
    *s += "</frameset>";
}

QString
KHTMLWidget::getBackground()
{
  if ( !background )
    return QString::null;

  return background->getURL().string();
}

HTMLPageInfo *
KHTMLWidget::getPageInfo()
{
    HTMLPageInfo *pageInfo = new HTMLPageInfo();
    pageInfo->imageURLs = usedImageURLs;
    pageInfo->hrefURLs = usedHrefURLs;
    return pageInfo;
}
#endif

#include "khtml.moc"

