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

#define CLUE_DEBUG

#ifdef GrayScale
#undef GrayScale
#endif

#ifdef Color
#undef Color
#endif

#include "khtml.h"

#include "khtmlcache.h"
#include "khtmldata.h"
#include "khtmliter.h"
#include "khtmlchain.h"
#include "khtmljscript.h"
#include "khtmltoken.h"
#include "khtmlparser.h"
#include "khtmlobj.h"
#include "khtmlclue.h"
#include "khtmlform.h"
#include "khtmlframe.h"
#include "khtmltable.h"
#include "khtmlview.h"
#include "khtmlsavedpage.h"
#include "khtmltags.h"
#include "khtmlembed.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>

#include <qimage.h>
#include <qregexp.h>
#include <qkeycode.h>
#include <qprinter.h>
#include <qdrawutil.h>
#include <qdragobject.h>
#include <qstrlist.h>

#include <kurl.h>
#include <kimgio.h>
#include <kstddirs.h>
#include <kglobal.h>

#include <X11/Xlib.h>

// Debug functions
void debugT( const char *msg , ...);
void debugM( const char *msg , ...);

#define PRINTING_MARGIN		36	// printed margin in 1/72in units
#define TIMER_INTERVAL		30	// ms between parser parses

HTMLPendingFile::HTMLPendingFile()
{
}

HTMLPendingFile::HTMLPendingFile( QString _url, HTMLFileRequester *_obj )
{
  m_strURL = _url;
  m_lstClients.append( _obj );
}

//
// KHTMLWidget 
//
///////////////////////////

KHTMLWidget::KHTMLWidget( QWidget *parent, const char *name, const char * )
    : QWidget( parent, name, WPaintClever )
{
    jsEnvironment = 0;      
    leftBorder    = LEFT_BORDER;
    rightBorder   = RIGHT_BORDER;
    topBorder     = TOP_BORDER;
    bottomBorder  = BOTTOM_BORDER;
    x_offset      = 0;
    y_offset      = 0;
    clue          = 0;
    background    = 0;
    ht            = 0;
    allocator     = 0;
    settings      = 0;
    colorContext  = 0;
    pressed       = false;
    //pressedURL    = "";
    //pressedTarget = "";
    //actualURL     = "";
    //baseURL       = "";
    bIsSelected   = false;
    selectedFrame = 0;
    htmlView      = 0;
    bIsFrameSet   = false;
    bIsFrame      = false;
    frameSet      = 0;
    bFramesComplete = false;
    painter       = 0;
    //overURL       = "";
    granularity   = 1000;
    linkCursor    = arrowCursor;
    bIsTextSelected = false;

    parser = 0;

    mapPendingFiles.setAutoDelete( true );
    framesetList.setAutoDelete( true );
    frameList.setAutoDelete( false ); 
    embededFrameList.setAutoDelete( false ); 
    formList.setAutoDelete( true );
    mapList.setAutoDelete( true );

    defaultSettings = new HTMLSettings;

    cache = new KHTMLCache(this);

    QPalette pal = palette().copy();
    QColorGroup cg = pal.normal();
    QColorGroup newGroup( cg.foreground(), lightGray, cg.light(),
	    cg.dark(), cg.mid(), cg.text(), lightGray );
    pal.setNormal( newGroup );
    setPalette( pal );

    setBackgroundColor( lightGray );

    QString f =  locate("data", "khtml/pics/khtml_dnd.xpm");
    dndDefaultPixmap.load( f.data() );
    
    kimgioRegister();

    setMouseTracking( true );    

    connect( &updateTimer, SIGNAL( timeout() ), SLOT( slotUpdate() ) );
    connect( &autoScrollYTimer, SIGNAL(timeout()), SLOT( slotAutoScrollY() ) );

    textFindIter = 0;
}
     
void KHTMLWidget::addHref( QString _url )
{ 
    // allow doubles in there... The call to QStringList::contains()
    // just uses up too much CPU time for some pages.
    //if (!usedHrefURLs.contains( _url))
    //{
    usedHrefURLs.append( _url );
    //}
}


//
// Cache handling
//
///////////////////////////////////////////////////



void KHTMLWidget::requestImage( HTMLObject *obj, QString _url )
{ 
    cache->requestImage( obj, _url ); 
    if (!usedImageURLs.contains( _url ))
    {
        usedImageURLs.append( _url );
    }
}

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
  
  if ( mapPendingFiles.count() == 0 && !parser )
  {
    emit documentDone();
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

  if ( mapPendingFiles.count() == 0 && !parser )
  {
    emit documentDone();
    cache->flush();
  }
}

void KHTMLWidget::slotFormSubmitted( QString _method, QString _url, const char *_data, QString _target )
{
    emit formSubmitted( _method, _url, _data, _target );
}

//
// Event Handling
//
/////////////////

void KHTMLWidget::mousePressEvent( QMouseEvent *_mouse )
{
    if ( clue == 0 )
	return;

    // Make this frame the active one
    if ( bIsFrame && !bIsSelected )
	htmlView->setSelected( TRUE );

    if ( clue->mouseEvent( _mouse->x() + x_offset - leftBorder,
	    _mouse->y() + y_offset - topBorder, _mouse->button(),
	    _mouse->state() ) )
	return;
    
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
	selectPt1.setX( _mouse->pos().x() + x_offset - leftBorder );
	selectPt1.setY( _mouse->pos().y() + y_offset - topBorder );
    }
    press_x = _mouse->pos().x();
    press_y = _mouse->pos().y();    
	    
    HTMLObject *obj;

    obj = clue->checkPoint( _mouse->pos().x() + x_offset - leftBorder,
	    _mouse->pos().y() + y_offset - topBorder );

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
}

void KHTMLWidget::mouseDoubleClickEvent( QMouseEvent *_mouse )
{
    if ( clue == 0 )
	return;
    
    HTMLObject *obj;
    
    obj = clue->checkPoint( _mouse->pos().x() + x_offset - leftBorder,
	_mouse->pos().y() + y_offset - topBorder );
    
    if ( obj != 0)
	if ( obj->getURL().length() )
		emit doubleClick( obj->getURL(), _mouse->button() );
}

void KHTMLWidget::mouseMoveEvent( QMouseEvent * _mouse )
{
    if ( clue == 0 )
	return;

    if ( !pressed )
    {
	// Look wether the cursor is over an URL.
	HTMLObject *obj=clue->checkPoint(_mouse->pos().x()+x_offset-leftBorder,
	    _mouse->pos().y() + y_offset - topBorder );
	if ( obj != 0 )
	{
	    if ( obj->getURL().length() )
	    {
		if ( overURL != obj->getURL() )
		{
		    setCursor( linkCursor );
		    overURL = obj->getURL();
		    emit onURL( overURL );
		}
	    }
	    else if ( overURL != "" )
	    {
		setCursor( arrowCursor );
		emit onURL( 0 );
		overURL = "";
	    }
	}
	else if ( overURL != "" )
	{
	    setCursor( arrowCursor );
	    emit onURL( 0 );
	    overURL = "";
	}
	return;
    }

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
	selectPt2.setX( point.x() + x_offset - leftBorder );
	selectPt2.setY( point.y() + y_offset - topBorder );
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
	
	QStrList urls;
	urls.append(pressedURL.data());

	QUrlDrag *ud = new QUrlDrag(urls, this);
	ud->setPixmap(dndDefaultPixmap);
	ud->dragCopy();
    }
}

void KHTMLWidget::mouseReleaseEvent( QMouseEvent * _mouse )
{
    if ( clue == 0 )
	return;

    if ( pressed )
    {
	// in case we started an autoscroll in MouseMove event
	stopAutoScrollY();
	disconnect( this, SLOT( slotUpdateSelectText(int) ) );
    }

    // Used to prevent mouseMoveEvent from initiating a drag before
    // the mouse is pressed again.
    pressed = false;

    if ( clue->mouseEvent( _mouse->x() + x_offset - leftBorder,
	    _mouse->y() + y_offset - topBorder, _mouse->button(),
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
    if ( pressedURL.isEmpty() )
	return;

    // if ( pressedURL.data()[0] == '#' )
    //	gotoAnchor( pressedURL.data() + 1 );
    // else
    if ( _mouse->button() != RightButton )
    {
//	printf("pressedURL='%s'\n",pressedURL.data());
	emit URLSelected( pressedURL.data(), _mouse->button(), pressedTarget.data() );
	// required for backward compatability
	emit URLSelected( pressedURL.data(), _mouse->button() );
    }
}

//
// Selection
//
/////////////////////////////

/*
 * Checks out wether there is a URL under the point p and returns a pointer
 * to this URL or 0 if there is none.
 */
QString KHTMLWidget::getURL( QPoint &p )
{
    if ( clue == 0 )
	return 0;
    
    HTMLObject *obj;
    
    obj = clue->checkPoint( p.x() + x_offset, p.y() + y_offset );
    
    if ( obj != 0)
	if ( obj->getURL().length() )
	    return obj->getURL();
    
    return 0;
}

void KHTMLWidget::select( QPainter * , QRect &_rect )
{
    if ( clue == 0 )
	return;
    
    QRect r = _rect;

    r.moveBy( -x_offset, -y_offset );
    
    int tx = -x_offset + leftBorder;
    int ty = -y_offset + topBorder;

    HTMLChain chain;

    clue->select( this, &chain, r, tx, ty );
}

void KHTMLWidget::selectText( int _x1, int _y1, int _x2, int _y2 )
{
    if ( clue == 0 )
	return;

    int tx = -x_offset + leftBorder;
    int ty = -y_offset + topBorder;

    HTMLChain chain;

    bIsTextSelected = clue->selectText(this,&chain,_x1, _y1, _x2, _y2, tx, ty);
}

void KHTMLWidget::findTextBegin()
{
    if ( clue == 0 )
	return;

    findTextEnd();

    textFindIter = new HTMLListIterator( clue );
}

bool KHTMLWidget::findTextNext( const QRegExp &exp )
{
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
	if ( y < y_offset || y > y_offset + height() - obj->getHeight() )
	{
	    gotoXY( x_offset, y - 40 );
	}
	paintSingleObject( obj );
    }
    else
    {
	// end of document reached.
	findTextEnd();
    }

    return ( obj != 0 );
}

void KHTMLWidget::findTextEnd()
{
    if ( textFindIter )
    {
	delete textFindIter;
	textFindIter = 0;
	selectText( 0, 0, 0, 0 );	// deselect all text
    }
}

void KHTMLWidget::paintEvent( QPaintEvent* _pe )
{
    bool newPainter = FALSE;

    if ( clue == 0 )
	return;

    if ( painter == 0 )
    {
	painter = new QPainter;
	painter->begin( this );
	newPainter = TRUE;
    }

    painter->setBackgroundColor( settings->bgColor );

    positionFormElements();

    if (background)
    {
        background->print( painter, _pe->rect().x() + x_offset,
	    _pe->rect().y() + y_offset,
	    _pe->rect().width(), _pe->rect().height(), -x_offset, -y_offset, false );
    }

    // painter->translate( x_offset, -y_offset );    
    int tx = -x_offset + leftBorder;
    int ty = -y_offset + topBorder;

    clue->print( painter, _pe->rect().x() - tx, _pe->rect().y() - ty,
	    _pe->rect().width(), _pe->rect().height(), tx, ty, false );
    
    if ( bIsSelected )
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

void KHTMLWidget::resizeEvent( QResizeEvent* _re )
{
	if ( clue == 0 )
	    return;

	if ( isFrameSet() )
	{
	    framesetList.getFirst()->setGeometry( 0, 0, width(), height() );
	}            
	else if ( clue && _re->oldSize().width() != _re->size().width() )
	{
	    calcSize();
	    calcAbsolutePos();
	    positionFormElements();
	}

	emit resized( _re->size() );
}

void KHTMLWidget::keyPressEvent( QKeyEvent *_ke )
{
    switch ( _ke->key() )
    {
	case Key_Down:
	    {
		if ( docHeight() < height() ) break;
		int newY = y_offset + 20;
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
	      int newY = y_offset + height() - 20;
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
		int newY = y_offset - 20;
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
		int newY = y_offset - height() + 20;
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
		int newX = x_offset + 20;
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
		int newX = x_offset - 20;
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
	_obj->print( painter, absx - x_offset + leftBorder - _obj->getXPos(),
	    absy - y_offset + topBorder - (_obj->getYPos()-_obj->getAscent()) );
    }
    else
    {
	int tx = -x_offset + leftBorder;
	int ty = -y_offset + topBorder;
	
	clue->print( painter, _obj, x_offset, y_offset,
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
            background->print( painter, x + x_offset, y + y_offset, 
			w, h, -x_offset, -y_offset, false );
        }
	int tx = -x_offset + leftBorder;
	int ty = -y_offset + topBorder;

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

void KHTMLWidget::slotUpdate()
{
    repaint( false );
}

void KHTMLWidget::calcAbsolutePos()
{
    if ( clue )
	clue->calcAbsolutePos( 0, 0 );
}

void KHTMLWidget::getSelected( QStringList &_list )
{
    if ( clue == 0 )
	return;
    
    clue->getSelected( _list );
}

void KHTMLWidget::getSelectedText( QString &_str )
{
    if ( clue == 0 )
	return;
    
    clue->getSelectedText( _str, false );
}

void KHTMLWidget::getAllText( QString &_str )
{
    if ( clue == 0 )
	return;
    
    clue->getSelectedText( _str, true );
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
	clue->setMaxWidth( pgWidth );
	clue->calcSize();
	clue->setPos( 0, clue->getAscent() );
	calcAbsolutePos();

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
	calcAbsolutePos();

	if ( newPainter )
	{
	    painter->end();
	    delete painter;
	    painter = 0;
	}
    }
}

//
// Parsing
//
////////////////////////////

void KHTMLWidget::begin( QString _url, int _x_offset, int _y_offset )
{
    debug("KHTMLWidget::begin(....)");

    bIsFrameSet = FALSE;
    // bIsFrame = FALSE;
    bFramesComplete = FALSE;
    framesetList.clear();
    frameList.clear();
    embededFrameList.clear();
    usedImageURLs.clear();
    usedHrefURLs.clear();
    
    findTextEnd();

    if ( bIsTextSelected )
    {
	bIsTextSelected = false;
	emit textSelected( false );
    }
    
    if ( frameSet )
    {
	delete frameSet;
	frameSet = 0;
    }

    x_offset = _x_offset;
    y_offset = _y_offset;

    emit scrollHorz( x_offset );
    emit scrollVert( y_offset );
    
    stopParser();
    
    reference = QString::null;
    actualURL = "";
    baseURL = "";

    if ( !_url.isEmpty() )
    {
        actualURL = _url;
        reference = actualURL.ref();
        setBaseURL( _url);

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

    // Clear several administration lists
    formList.clear();
    mapList.clear();

    // Delete all HTMLObjects on the page
    if (clue)
	delete clue;
    
    if (background)
        delete background;
    background = 0;


    // Free their storage
    if ( allocator )
    {
	delete allocator;
	allocator = 0;
    }

    // Delete the tokenizer including all tokens and strings.                              
    if ( ht )
	delete ht;

    // Lets start again
    ht = new HTMLTokenizer( );
    ht->begin( );
    
//HIGHLY experimental
    //ht->setPlainText();

    emit documentStarted();

    writing = true;
}

void KHTMLWidget::write( const char *_str)
{
    if ( _str == 0 )
	return;
    
    ht->write( _str );

    // If the timer has been stopped while waiting for more html,
    // start it again.
    if ( parser && timerId == 0 )
	timerId = startTimer( TIMER_INTERVAL );
}

void KHTMLWidget::end()
{
    writing = false;
    if ( ht )
	ht->end();
}

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

    parser = new KHTMLParser( this, ht, painter, settings, 0 /* &formData */, allocator );

    setBGColor( settings->bgColor );

    // this will call timerEvent which in turn calls parseBody
    timerId = startTimer( TIMER_INTERVAL );
}

void KHTMLWidget::setBackground(HTMLBackground *_background)
{
    background = _background;
}

void KHTMLWidget::stopParser()
{
    if ( !parser )
	return;

    if ( timerId != 0 )
	killTimer( timerId );
    
    delete parser;
    parser = 0;
}

void KHTMLWidget::timerEvent( QTimerEvent * )
{
  /*** DEBUG ***/
  printf("############### timerEvent ##############\n");
  /*** END DEBUG ***/

    static const uint end[] = { ID_BODY + ID_CLOSE_TAG, 
				ID_HTML + ID_CLOSE_TAG, 0 }; 

    if ( !painter )
    {
      printf("############### Dont have a painter ##############\n");
      return;
    }
    
    debugM("Killing timer\n");
    killTimer( timerId );
    timerId = 0;

    debugM("Has more tokens?\n");
    if ( !ht->hasMoreTokens() && writing )
    {
      printf("############### waiting for more ##############\n");
      return;
    }
    
    debugM("Storing font info\n");
    const QFont &oldFont = painter->font();

    debugM("Getting height\n");
    int lastHeight = docHeight();

    parseCount = granularity;

  /*** DEBUG ***/
  printf("############### parsing body ##############\n");
  /*** END DEBUG ***/

    if ( parser->parseBody( clue, end, TRUE ) )
	stopParser();
    else if ( !ht->hasMoreTokens() && !writing )
        stopParser();

    calcSize();
	
    calcAbsolutePos();

    debugM("Restoring font\n");
    painter->setFont( oldFont );

    debugM("Synchronizing painter's background\n");
    // FE: synchronize painter's backgroundColor
    painter->setBackgroundColor( settings->bgColor );

    // If the visible rectangle was not filled before the parsing and
    // if we have something to display in the visible area now then repaint.
    if ( lastHeight - y_offset < height() * 2 && docHeight() - y_offset > 0 )
	scheduleUpdate();

    if (!reference.isNull())
    {
    	if (gotoAnchor())
    	{
    	    reference = QString::null;
    	}
    }

    debugM("Parsin is over?\n");
    // Parsing is over ?
    if ( !parser )
    {
#ifdef CLUE_DEBUG
	printf("---------------------- background ----------------------------\n");
        if (background)
            background->printDebug( true, 0, true);
        else
            printf("No background.\n");
	printf("------------------ clue + object list ------------------------\n");
	clue->printDebug(true, 0, true);
	printf("--------------------------------------------------------------\n");
#endif
        debugM("Yes\n");
	debug( "Parsing done" );

	// Is y_offset too big ?
	if ( docHeight() - y_offset < height() )
	{
	    y_offset = docHeight() - height();
	    if ( y_offset < 0 )
		y_offset = 0;
	}
	// Adjust the scrollbar
	emit scrollVert( y_offset );

	// Is x_offset too big ?
	if ( docWidth() - x_offset < width() )
	{
	    x_offset = docWidth() - width();
	    if ( x_offset < 0 )
		x_offset = 0;
	}	    
	// Adjust the scrollbar
	emit scrollHorz( x_offset );

	painter->end();
	delete painter;
	painter = 0;

	// Did we finish the job or are still pictures missing ?
	// XXXX if ( waitingFileList.count() == 0 && bgPixmapURL.isEmpty() )
	if ( mapPendingFiles.isEmpty() )
	{
	    emit documentDone();
	}

	/*** DEBUG ***/
	printf("############### Showing frames ##############\n");
	/*** END DEBUG ***/

	// Now it is time to tell all frames what they should do
	KHTMLView *v;
	KHTMLWidget *w;
	for ( w = frameList.first(); w != 0; w = frameList.next() )
	{
	    v = w->getView();
	    if ( !v->getCookie().isNull() )
		v->openURL( v->getCookie() );
	    v->show();
	}
	KHTMLEmbededWidget* e;
	for( e = embededFrameList.first(); e != 0L; e = embededFrameList.next() )
	  e->show();

	HTMLFrameSet *s;
	for ( s = framesetList.first(); s != 0; s = framesetList.next() )
	{
	    s->show();
	}
	if ( ( s = framesetList.getFirst() ) )
	    s->setGeometry( 0, 0, width(), height() );
    }
    else{
      /*** DEBUG ***/
      printf("############### More timerEvents ##############\n");
      /*** END DEBUG ***/
      
	timerId = startTimer( TIMER_INTERVAL );
    }	
}

void KHTMLWidget::calcSize()
{
    if ( clue == 0 )
	return;

    clue->reset();
    
    int _max_width = width() - leftBorder - rightBorder;
    int _min_width = clue->calcMinWidth();
    
    if (_min_width > _max_width)
    {
        _max_width = _min_width;
    }
    
    clue->setMaxWidth( _max_width );
    clue->calcSize();
    clue->setPos( 0, clue->getAscent() );

    emit documentChanged();
}


void KHTMLWidget::slotScrollVert( int _val )
{
    int ofs = 0;
    int diff = y_offset - _val;
    
    if ( !isUpdatesEnabled() )
	return;

    if ( clue == 0 )
	return;

    if ( diff == 0 )
	return;

    if (bIsSelected)
    {
        ofs = 2;
    }

    y_offset = _val;

    if (diff < 0)
    {
    	diff = -diff;
    	if (diff < height() - ofs - ofs)
    	{
	    bitBlt( this, ofs, ofs, 
	            this, ofs, ofs + diff, 
	            width()-ofs-ofs, height()-diff-ofs-ofs);    
	}
	else
	{
	   diff = height() - ofs - ofs;
	}
	repaint( ofs, height() - ofs - diff, 
		 width()-ofs-ofs, diff, false);
    }
    else {
	if (diff < height() - ofs - ofs)
	{
	    bitBlt( this, ofs, ofs + diff,
	    	    this, ofs, ofs,
	    	    width()-ofs-ofs, height()-diff-ofs-ofs);
	}
	else
	{
	    diff = height() - ofs - ofs;
	}
	repaint( ofs, ofs, width()-ofs-ofs, diff, false);
    }
}

void KHTMLWidget::slotScrollHorz( int _val )
{
    int ofs = 0;
    int diff = x_offset - _val;

    if ( !isUpdatesEnabled() )
	return;

    if ( clue == 0 )
	return;

    if ( diff == 0)
	return;
    
    if (bIsSelected)
    {
        ofs = 2;
    }

    x_offset = _val;

    if (diff < 0)
    {
    	diff = -diff;
    	if (diff < width() - ofs - ofs)
    	{
	    bitBlt( this, ofs, ofs, 
	            this, ofs + diff, ofs, 
	            width()-diff-ofs-ofs, height()-ofs-ofs);    
	}
	else
	{
	   diff = width() - ofs - ofs;
	}
	repaint( width() - ofs - diff, ofs, 
		 diff, height() - ofs - ofs, false);
    }
    else {
	if (diff < width() - ofs - ofs)
	{
	    bitBlt( this, ofs + diff, ofs,
	    	    this, ofs, ofs,
	    	    width()-diff-ofs-ofs, height()-ofs-ofs);
	}
	else
	{
	    diff = width() - ofs - ofs;
	}
	repaint( ofs, ofs, diff, height()-ofs-ofs, false);
    }
}

void KHTMLWidget::positionFormElements()
{
    HTMLForm *f;

    for ( f = formList.first(); f != 0; f = formList.next() )
    {
	f->position( x_offset - leftBorder, y_offset - topBorder,
	    width(), height() );
    }
}

bool KHTMLWidget::gotoAnchor( )
{
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

    return TRUE;
}

bool KHTMLWidget::gotoXY( int _x_offset, int _y_offset )
{
    if ( clue == 0 )
	return FALSE;

    emit scrollVert( _y_offset );

    emit scrollHorz( _x_offset );

    return TRUE;
}


bool KHTMLWidget::gotoAnchor( QString _name )
{
    reference = _name;
    
    if (gotoAnchor())
    {
       reference = QString::null;
       return TRUE;
    }
    return FALSE;
}

void KHTMLWidget::autoScrollY( int _delay, int _dy )
{
    if ( clue == 0 )
	    return;

    if ( _dy == 0 || ( _dy < 0 && y_offset == 0 ) ||
	 ( _dy > 0 && y_offset >= docHeight() - height() - 1 ) )
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
	if ( ( autoScrollDY > 0 && y_offset < docHeight() - height() - 1 ) ||
		 ( autoScrollDY < 0 && y_offset > 0 ) )
	{
		int newY = y_offset + autoScrollDY;
		if ( newY > docHeight() - height()  - 1)
			newY = docHeight() - height() - 1;
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
	if ( pressed )
	{
		QPoint point = QCursor::pos();
		point = mapFromGlobal( point );
		if ( point.y() > height() )
			point.setY( height() );
		else if ( point.y() < 0 )
			point.setY( 0 );
		selectPt2.setX( point.x() + x_offset - leftBorder );
		selectPt2.setY( point.y() + y_offset - topBorder );
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
}

void KHTMLWidget::select( QPainter *, bool _select )
{
    if ( clue == 0 )
	return;
    
    int tx = -x_offset + leftBorder;
    int ty = -y_offset + topBorder;

    HTMLChain chain;

    clue->select( this, &chain, _select, tx, ty );
}

void KHTMLWidget::selectByURL( QPainter *, QString _url, bool _select )
{
    if ( clue == 0 )
	return;

    int tx = -x_offset + leftBorder;
    int ty = -y_offset + topBorder;

    HTMLChain chain;

    clue->selectByURL( this, &chain, _url, _select, tx, ty );
}

void KHTMLWidget::select( QPainter *, QRegExp& _pattern, bool _select )
{
    if ( clue == 0 )
	return;

    int tx = -x_offset + leftBorder;
    int ty = -y_offset + topBorder;

    HTMLChain chain;

    clue->select( this, &chain, _pattern, _select, tx, ty );
}

int KHTMLWidget::docWidth() const
{
    if ( bIsFrameSet )
	return width();
    
    if ( clue )
	return clue->getWidth() + leftBorder + rightBorder;
    else
	return leftBorder + rightBorder;
}

int KHTMLWidget::docHeight() const
{
    if ( bIsFrameSet )
	return height();
  
    if ( clue )
	return clue->getHeight() + topBorder + bottomBorder;

    return 0;
}

void KHTMLWidget::setSelected( bool _active )
{
    if ( _active == bIsSelected )
    return;
  
  bIsSelected = _active;

  if ( _active )
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
}

void KHTMLWidget::setIsFrameSet( bool _b )
{
  bIsFrameSet = _b;
}

void KHTMLWidget::setIsFrame( bool _b )
{
  bIsFrame = _b;
}

void KHTMLWidget::slotFrameSelected( KHTMLView *_view )
{
  if ( selectedFrame && selectedFrame != _view )
    selectedFrame->setSelected( FALSE );
  selectedFrame = _view;
}

KHTMLView* KHTMLWidget::getSelectedFrame()
{
  if ( isFrame() && isSelected() )
    return htmlView;

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
    
  return 0;
}

      
KHTMLWidget::~KHTMLWidget()
{
    if ( parser )
    	delete parser;

    if ( painter )
    {
	painter->end();
	delete painter;
    }
    if (clue)
	delete clue;

    if (background)
        delete background;

    if (ht)
	delete ht;
    if (allocator)
	delete allocator;
    

/*
    if ( colorContext )
    {
	QColor::leaveAllocContext();
	QColor::destroyAllocContext( colorContext );
    }
*/
    delete defaultSettings;

    if ( jsEnvironment )
	delete jsEnvironment;            

    delete cache;
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

HTMLMap *KHTMLWidget::getMap( QString mapurl )
{
    HTMLMap *map;

    for ( map = mapList.first(); map != 0; map = mapList.next() )
    {
        if ( map->mapURL() == mapurl )
            return map;
    }
    return 0;
}

/*
 * Add a image map
 */

void KHTMLWidget::addMap( QString mapUrl)
{
    mapList.append( new HTMLMap( this, mapUrl) );
}

/*
 * Get last image map
 */
HTMLMap * KHTMLWidget::lastMap()
{
    if (mapList.isEmpty())
    	return 0;
    	
    return mapList.getLast();
}
                                    

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

void KHTMLWidget::addForm( HTMLForm *_form )
{
    formList.append( _form );
    connect( _form, 
             SIGNAL( submitted( QString, QString, 
				const char *, QString ) ),
             SLOT( slotFormSubmitted( QString, QString, 
				      const char *, QString ) ) 
           );
}

void KHTMLWidget::setNewTitle( QString _title)
{
    emit setTitle( _title );
}
     
void KHTMLWidget::setBGColor( const QColor &_bgColor)
{
 
    if (settings->bgColor.red() != _bgColor.red() ||
	settings->bgColor.green() != _bgColor.green() ||
	settings->bgColor.blue() != _bgColor.blue() )
    {
       settings->bgColor = _bgColor;
       QPalette pal = palette().copy();
       QColorGroup cg = pal.normal();
       QColorGroup newGroup( cg.foreground(), settings->bgColor,
           cg.light(), cg.dark(), cg.mid(), cg.text(), settings->bgColor );
       pal.setNormal( newGroup );
       setPalette( pal );
       setBackgroundColor( settings->bgColor );
    }
}                   

bool KHTMLWidget::setCharset(QString /*name*/, bool /*override*/){
    // ###### FIXME: decoder

}


JSEnvironment* KHTMLWidget::getJSEnvironment()
{
    if ( jsEnvironment == 0 )
	jsEnvironment = new JSEnvironment( this );
    
    return jsEnvironment;
}

JSWindowObject* KHTMLWidget::getJSWindowObject()
{
    return getJSEnvironment()->getJSWindowObject();
}


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
    p->xOffset = x_offset;
    p->yOffset = y_offset;
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

HTMLPageInfo *
KHTMLWidget::getPageInfo()
{
    HTMLPageInfo *pageInfo = new HTMLPageInfo();
    pageInfo->imageURLs = usedImageURLs;
    pageInfo->hrefURLs = usedHrefURLs;
    return pageInfo;
}

#include "khtml.moc"

