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

//#define CLUE_DEBUG

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

#include <kurl.h>
#include <kapp.h>
#include <kcharsets.h>

#include <X11/Xlib.h>

// Debug functions
void debugT( const char *msg , ...);
void debugM( const char *msg , ...);

#define PRINTING_MARGIN		36	// printed margin in 1/72in units
#define TIMER_INTERVAL		30	// ms between parser parses

HTMLPendingFile::HTMLPendingFile()
{
}

HTMLPendingFile::HTMLPendingFile( const char *_url, HTMLObject *_obj )
{
  m_strURL = _url;
  m_lstClients.append( _obj );
}

//
// KHTMLWidget 
//
///////////////////////////

KHTMLWidget::KHTMLWidget( QWidget *parent, const char *name, const char * )
    : KDNDWidget( parent, name, WPaintClever )
{
    jsEnvironment = 0;      
    leftBorder    = LEFT_BORDER;
    rightBorder   = RIGHT_BORDER;
    topBorder     = TOP_BORDER;
    bottomBorder  = BOTTOM_BORDER;
    x_offset      = 0;
    y_offset      = 0;
    clue          = 0;
    ht            = 0;
    settings      = 0;
    colorContext  = 0;
    pressed       = false;
    pressedURL    = "";
    pressedTarget = "";
    actualURL     = "";
    baseURL       = "";
    bIsSelected   = false;
    selectedFrame = 0;
    htmlView      = 0;
    bIsFrameSet   = false;
    bIsFrame      = false;
    frameSet      = 0;
    bFramesComplete = false;
    painter       = 0;
    overURL       = "";
    granularity   = 600;
    linkCursor    = arrowCursor;
    bIsTextSelected = false;

    parser = 0;

    mapPendingFiles.setAutoDelete( true );
    framesetList.setAutoDelete( false );
    frameList.setAutoDelete( false ); 
    // XXX waitingFileList.setAutoDelete( false );
    formList.setAutoDelete( true );
    mapList.setAutoDelete( true );

    parsedURLs.setAutoDelete( false );
    parsedTargets.setAutoDelete( false );

    defaultSettings = new HTMLSettings;

    cache = new KHTMLCache(this);

    QPalette pal = palette().copy();
    QColorGroup cg = pal.normal();
    QColorGroup newGroup( cg.foreground(), lightGray, cg.light(),
	    cg.dark(), cg.mid(), cg.text(), lightGray );
    pal.setNormal( newGroup );
    setPalette( pal );

    setBackgroundColor( lightGray );

    QString f = kapp->kde_datadir().copy();
    f += "/khtmlw/pics/khtmlw_dnd.xpm";
    dndDefaultPixmap.load( f.data() );
    
    registerFormats();

    setMouseTracking( true );    

    connect( &updateTimer, SIGNAL( timeout() ), SLOT( slotUpdate() ) );
    connect( &autoScrollYTimer, SIGNAL(timeout()), SLOT( slotAutoScrollY() ) );

    textFindIter = 0;
}

//
// Cache handling
//
///////////////////////////////////////////////////



void KHTMLWidget::requestImage( HTMLObject *obj, const char *_url )
{ 
    cache->requestImage( obj, _url); 
}

void KHTMLWidget::preloadImage( const char *_filename)
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
                                              
void KHTMLWidget::requestFile( HTMLObject *_obj, const char *_url, 
			       bool update )
{
  printf("==== REQUEST %s  ====\n", _url );
  
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

void KHTMLWidget::cancelRequestFile( HTMLObject *_obj )
{
  QStrList lst;
  
  QDictIterator<HTMLPendingFile> it( mapPendingFiles );
  for( ; it.current(); ++it )
  {
    it.current()->m_lstClients.removeRef( _obj );
    if ( it.current()->m_lstClients.count() == 0 )
    {
      emit cancelFileRequest( it.current()->m_strURL );
      lst.append( it.currentKey() );
    }
  }
  
  const char* u;
  for( u = lst.first(); u != 0L; u = lst.next() )
    mapPendingFiles.remove( u );
}

void KHTMLWidget::cancelRequestFile( const char *_url )
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

void KHTMLWidget::requestBackgroundImage( const char *_url )
{
    bgPixmapURL = _url;
    bgPixmapURL.detach();
    emit fileRequest( _url );
}

void KHTMLWidget::data( const char *_url, const char *_data, int _len, bool _eof )
{
  bool do_update = false;
  
  HTMLPendingFile *p = mapPendingFiles[ _url ];
  if ( !p )
    return;
  
  if ( !p->m_buffer.isOpen() )
    p->m_buffer.open( IO_WriteOnly );
  p->m_buffer.writeBlock( _data, _len );
  if ( _eof )
  {    
    p->m_buffer.close();

    HTMLObject* o;
    for( o = p->m_lstClients.first(); o != 0L; o = p->m_lstClients.next() )
      if ( o->fileLoaded( _url, p->m_buffer ) )
	do_update = true;
    
    mapPendingFiles.remove( _url );
  }

  if ( do_update )
  {
    calcSize();
    calcAbsolutePos();
    scheduleUpdate( true );
  }
  
  if ( mapPendingFiles.count() == 0 && !parser )
  {
    emit documentDone();
    cache->flush();
  }
}

void KHTMLWidget::slotFileLoaded( const char *_url, const char *_filename )
{
  printf("///////// FileLoaded %s %s ////////////\n",_url,_filename );
  
  HTMLPendingFile *p = mapPendingFiles[ _url ];
  if ( !p )
  {
    if ( !bgPixmapURL.isEmpty() )
    {
	// Did the background image arrive ?
	if ( strcmp( bgPixmapURL, _url ) == 0 )
	{
	    bgPixmap.load( _filename );					
	    bgPixmapURL = 0;
	    scheduleUpdate( true );
	}
    }    
    return;
  }

  assert( !p->m_buffer.isOpen() );
  
  HTMLObject* o;
  for( o = p->m_lstClients.first(); o != 0L; o = p->m_lstClients.next() )
    o->fileLoaded( _url, _filename );
  
  mapPendingFiles.remove( _url );

  if ( mapPendingFiles.count() == 0 && !parser )
  {
    emit documentDone();
    cache->flush();
  }
}

void KHTMLWidget::slotFormSubmitted( const char *_method, const char *_url, const char *_data )
{
    emit formSubmitted( _method, _url, _data );
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
	if ( obj->getURL() != 0 )
	{
	    if (obj->getURL()[0] != 0)
	    {
		// Save data. Perhaps the user wants to start a drag.
		if ( _mouse->button() == LeftButton || _mouse->button() == MidButton )
		{
		    pressedURL = obj->getURL();
		    pressedURL.detach();
		    pressedTarget = obj->getTarget();
		    pressedTarget.detach();
		}
		
		// Does the parent want to process the event now ?
	        if ( htmlView )	
		{
		    if ( htmlView->mousePressedHook( obj->getURL(),
		    	    obj->getTarget(), _mouse, obj->isSelected() ) )
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
	if ( obj->getURL() != 0 )
	    if (obj->getURL()[0] != 0)
		emit doubleClick( obj->getURL(), _mouse->button() );
}

void KHTMLWidget::dndMouseMoveEvent( QMouseEvent * _mouse )
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
	    if ( obj->getURL() && obj->getURL()[0] != 0 )
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
    if ( abs( x - press_x ) > Dnd_X_Precision || abs( y - press_y ) > Dnd_Y_Precision && !drag )
    {
        // debugT(">>>>>>>>>>>>>>>> Starting DND <<<<<<<<<<<<<<<<<<<<<<<<\n");
	QPoint p = mapToGlobal( _mouse->pos() );

	// Does the parent want to process the event now ?
	if ( htmlView )
        {
	    if ( htmlView->dndHook( pressedURL.data(), p ) )
		return;
	}

	int dx = - dndDefaultPixmap.width() / 2;
	int dy = - dndDefaultPixmap.height() / 2;

	startDrag( new KDNDIcon( dndDefaultPixmap, p.x() + dx, p.y() + dy ),
		pressedURL.data(), pressedURL.length(), DndURL, dx, dy );
    }
}

void KHTMLWidget::dndMouseReleaseEvent( QMouseEvent * _mouse )
{
    if ( clue == 0 )
	return;

    if ( pressed )
    {
	// in case we started an autoscroll in MouseMove event
	stopAutoScrollY();
	disconnect( this, SLOT( slotUpdateSelectText(int) ) );
    }

    // Used to prevent dndMouseMoveEvent from initiating a drag before
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

void KHTMLWidget::dragEndEvent()
{
    // Used to prevent dndMouseMoveEvent from initiating a new drag before
    // the mouse is pressed again.
    pressed = false;
}


//
// Selection
//
/////////////////////////////

/*
 * Checks out wether there is a URL under the point p and returns a pointer
 * to this URL or 0 if there is none.
 */
const char* KHTMLWidget::getURL( QPoint &p )
{
    if ( clue == 0 )
	return 0;
    
    HTMLObject *obj;
    
    obj = clue->checkPoint( p.x() + x_offset, p.y() + y_offset );
    
    if ( obj != 0)
	if ( obj->getURL() && obj->getURL()[0] != 0 )
	    return obj->getURL();
    
    return 0;
}

void KHTMLWidget::select( QPainter * _painter, QRect &_rect )
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

    // painter->translate( x_offset, -y_offset );    
    int tx = -x_offset + leftBorder;
    int ty = -y_offset + topBorder;
    
    drawBackground( x_offset, y_offset, _pe->rect().x(),
	    _pe->rect().y(),
	    _pe->rect().width(), _pe->rect().height() );

    clue->print( painter, _pe->rect().x() - x_offset,
	    _pe->rect().y() + y_offset - topBorder,
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
      //-------------------------------------
      // KFM Extension
      //-------------------------------------
        case Key_Space:
	  {
	    cellSelected();
	    flushKeys();
	  }
	break;
        case Key_Escape:
	  {
	    cellContextMenu();
	    flushKeys();
	  }
	break;
        case Key_Return:
	  {
	    cellActivated();
	    flushKeys();
	  }
	break;	
      //-------------------------------------
      // End KFM Extension
      //-------------------------------------
	case Key_Down:
	    {
	        // KFM Extension
	        if ( !cellDown() )
		{    
		  if ( docHeight() < height() ) break;
		  int newY = y_offset + 20;
		  if ( newY > docHeight() - height() )
		    newY = docHeight() - height();
		  slotScrollVert( newY );
		  emit scrollVert( newY );
		}
		
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
	      // KFM Extension
	      if ( _ke->state() & AltButton )
	      {
		emit goUp();
              }
	      // KFM Extension
	      else if ( !cellUp() )
	      {  
		if ( docHeight() < height() ) break;
		int newY = y_offset - 20;
		if ( newY < 0 )
		  newY = 0;
		slotScrollVert( newY );
		emit scrollVert( newY );
	      }
	      
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
	        // KFM Extension
	        if ( _ke->state() & AltButton )
	        {
		  emit goRight();
                }
		// KFM Extension
		else if ( !cellRight() )
		{  
		  if ( docWidth() < width() ) break;
		  int newX = x_offset + 20;
		  if ( newX > docWidth() - width() )
		    newX = docWidth() - width();
		  slotScrollHorz( newX );
		  emit scrollHorz( newX );
		}
	      
		flushKeys();
	    }
	    break;

	case Key_Left:
	    {
	      // KFM Extension
	      if ( _ke->state() & AltButton )
	      {
		emit goLeft();
	      }
	      // KFM Extension
	      else if ( !cellLeft() )
	      {  
		if ( docWidth() < width() ) break;
		int newX = x_offset - 20;
		if ( newX < 0 )
		  newX = 0;
		slotScrollHorz( newX );
		emit scrollHorz( newX );
	      }
	      
	      flushKeys();
	    }
	    break;

	default:
	    KDNDWidget::keyPressEvent( _ke );
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
/*
	drawBackground( x_offset, y_offset, absx - x_offset + leftBorder,
		absy - y_offset + topBorder,
		_obj->getWidth(), _obj->getHeight() );
*/
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

	int tx = -x_offset + leftBorder;
	int ty = -y_offset + topBorder;

	bool db = bDrawBackground;
	bDrawBackground = true;
	drawBackground( x_offset, y_offset, x, y, w, h );
	bDrawBackground = db;

	_chain->current()->print( painter, _chain, x - x_offset - leftBorder,
		y + y_offset - topBorder, w, h, tx, ty );
    
	if ( newPainter )
	{
	    painter->end();
	    delete painter;
	    painter = 0;
	}
    }
}

void KHTMLWidget::scheduleUpdate( bool clear )
{
    if ( clear )
	bDrawBackground = true;

    if ( !updateTimer.isActive() )
    {
	bDrawBackground = clear;
	updateTimer.start( 100, true );
    }
}

void KHTMLWidget::slotUpdate()
{
    repaint( false );

    // If we aren't parsing anymore then the background should always be
    // drawn.
    if ( !parser )
	bDrawBackground = true;
}

void KHTMLWidget::calcAbsolutePos()
{
    if ( clue )
	clue->calcAbsolutePos( 0, 0 );
}

void KHTMLWidget::getSelected( QStrList &_list )
{
    if ( clue == 0 )
	return;
    
    clue->getSelected( _list );
}

void KHTMLWidget::getSelectedText( QString &_str )
{
    if ( clue == 0 )
	return;
    
    clue->getSelectedText( _str );
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

void KHTMLWidget::begin( const char *_url, int _x_offset, int _y_offset )
{
    
    bIsFrameSet = FALSE;
    // bIsFrame = FALSE;
    bFramesComplete = FALSE;
    framesetList.clear();
    frameList.clear();
    
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
    
    if ( !bgPixmapURL.isEmpty() )
	emit cancelFileRequest( bgPixmapURL );
    bgPixmapURL = 0;
    
    stopParser();
    
    reference = 0;
    actualURL = "";
    baseURL = "";
    charsetName = "";

    if ( _url != 0 )
    {
        actualURL = _url;
        reference = actualURL.reference();
        setBaseURL( _url);
    }

    
    if ( painter )
    {
	painter->end();
	delete painter;
	painter = 0;
    }

    // Clear several administration lists
    parsedTargets.clear();
    parsedURLs.clear();
    formList.clear();
    mapList.clear();

    // Delete all HTMLObjects on the page
    if (clue)
	delete clue;

    // Delete the tokenizer including all tokens and strings.                              
    if ( ht )
	delete ht;

    // Lets start again
    ht = new HTMLTokenizer( );
    ht->begin();

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


    clue = new HTMLClueV(); 
    clue->setVAlign( HTMLClue::Top );
    clue->setHAlign( HTMLClue::Left );


    parser = new KHTMLParser( this, ht, painter, settings );

    if (!charsetName.isEmpty())
    	parser->setCharset( charsetName.data() );

    if ( !bgPixmap.isNull() )
	bgPixmap.resize( -1, -1 );

    // clear page
    bDrawBackground = true;
    setBGColor( settings->bgColor );
    drawBackground( x_offset, y_offset, 0, 0, width(), height() );

    // this will call timerEvent which in turn calls parseBody
    timerId = startTimer( TIMER_INTERVAL );
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
    debugM("Timer event\n");
    static const char *end[] = { "</body>", 0 }; 

    if ( !painter )
	return;

    debugM("Killing timer\n");
    killTimer( timerId );
    timerId = 0;

    debugM("Has more tokens?\n");
    if ( !ht->hasMoreTokens() && writing )
	return;

    debugM("Storing font info\n");
    const QFont &oldFont = painter->font();

    debugM("Getting height\n");
    int lastHeight = docHeight();

    parseCount = granularity;
    debugM("Parsing body height\n");
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
	scheduleUpdate( false );

    if (!reference.isNull())
    {
    	if (gotoAnchor())
    	{
    	    reference = 0;
    	}
    }

    debugM("Parsin is over?\n");
    // Parsing is over ?
    if ( !parser )
    {
#ifdef CLUE_DEBUG
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
	if ( mapPendingFiles.isEmpty() && bgPixmapURL.isEmpty() )
	{
	    emit documentDone();
	}

	// Now it is time to tell all frames what they should do
	KHTMLView *v;
	KHTMLWidget *w;
	for ( w = frameList.first(); w != 0; w = frameList.next() )
	{
	    v = w->getView();
	    if ( v->getCookie() )
		v->openURL( v->getCookie() );
	    v->show();
	}

	HTMLFrameSet *s;
	for ( s = framesetList.first(); s != 0; s = framesetList.next() )
	{
	    s->show();
	}
	if ( ( s = framesetList.getFirst() ) )
	    s->setGeometry( 0, 0, width(), height() );
	bDrawBackground = true;
    }
    else{
        debugM("No\n");
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

    bDrawBackground = true;

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
    
    bDrawBackground = true;

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

void KHTMLWidget::drawBackground( int _xval, int _yval, int _x, int _y,
	int _w, int _h )
{
	if ( !bDrawBackground )
	    return;

	if ( bgPixmap.isNull() )
	{
		painter->eraseRect( _x, _y, _w, _h );
		return;
	}

	// if the background pixmap is transparent we must erase the bg
	if ( bgPixmap.mask() )
	    painter->eraseRect( _x, _y, _w, _h );

	int pw = bgPixmap.width();
	int ph = bgPixmap.height();

	int xOrigin = _x/pw*pw - _xval%pw;
	int yOrigin = _y/ph*ph - _yval%ph;

	painter->setClipRect( _x, _y, _w, _h );
	painter->setClipping( TRUE );

	for ( int yp = yOrigin; yp < _y + _h; yp += ph )
	{
		for ( int xp = xOrigin; xp < _x + _w; xp += pw )
		{
			painter->drawPixmap( xp, yp, bgPixmap );
		}
	}

	painter->setClipping( FALSE );
}

bool KHTMLWidget::gotoAnchor( )
{
    if ( clue == 0 )
	return FALSE;

    QPoint p( 0, 0 );
    
    HTMLAnchor *anchor = clue->findAnchor( reference.data(), &p );
    if ( anchor == 0 )
	return FALSE;

    // Is there more HTML to be expected?
    if (parser)	
    {
	// Check if the reference can be located at the top of the screen 
        if (p.y() > docHeight() - height()  - 1)
             return FALSE;
    }

    emit scrollVert( p.y() );

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


bool KHTMLWidget::gotoAnchor( const char *_name )
{
    reference = _name;
    
    if (gotoAnchor())
    {
       reference = 0;
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

void KHTMLWidget::select( QPainter *_painter, bool _select )
{
    if ( clue == 0 )
	return;
    
    int tx = -x_offset + leftBorder;
    int ty = -y_offset + topBorder;

    HTMLChain chain;

    clue->select( this, &chain, _select, tx, ty );
}

void KHTMLWidget::selectByURL( QPainter *_painter, const char *_url, bool _select )
{
    if ( clue == 0 )
	return;

    int tx = -x_offset + leftBorder;
    int ty = -y_offset + topBorder;

    HTMLChain chain;

    clue->selectByURL( this, &chain, _url, _select, tx, ty );
}

void KHTMLWidget::select( QPainter *_painter, QRegExp& _pattern, bool _select )
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
    if (ht)
	delete ht;
    

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
KHTMLWidget::setFontSizes(const int *newFontSizes)
{
    defaultSettings->setFontSizes(newFontSizes);
}

void
KHTMLWidget::getFontSizes(int *newFontSizes)
{
    defaultSettings->getFontSizes(newFontSizes);
}

void
KHTMLWidget::resetFontSizes(void)
{
    defaultSettings->resetFontSizes();
}

void 
KHTMLWidget::setStandardFont( const char *name )
{	
    defaultSettings->fontBaseFace = name; 
}

void 
KHTMLWidget::setFixedFont( const char *name )
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

//-----------------------------------------------------------
// FUNCTIONS used for KFM Extension
//-----------------------------------------------------------

bool KHTMLWidget::cellUp()
{
  if ( clue == 0 || parser )
    return true;
  
  QList<HTMLCellInfo> list;
  list.setAutoDelete( true );
  
  clue->findCells( -x_offset + leftBorder, -y_offset + topBorder, list );

  // A usual HTML page ?
  if ( list.isEmpty() )
    return false;
  
  HTMLCellInfo *curr = 0;
  HTMLCellInfo *next = 0;
  
  // Find current marker
  HTMLCellInfo *info;
  for ( info = list.first(); info != 0; info = list.next() )
  {
    if ( info->pCell->isMarked() )
    {
      curr = info;
      break;
    }
  }

  if ( curr == 0 )
    next = list.first();
  else
  { 
    while( list.current() )
    {
      if ( list.current()->baseAbs < curr->baseAbs )
	break;
      list.prev();
    }

    if ( list.current() == 0 )
      return true;
    
    HTMLCellInfo *inf; 
    int diff = 0xFFFFFFF;
    for ( inf = list.current(); inf != 0; inf = list.prev() )
    {
      int i = curr->xAbs - inf->xAbs;
      if ( i < 0 ) i *= -1;
      if ( i < diff )
      {
	diff = i;
	next = inf;
      }
    }
  }
  
  if ( next == 0 )
    return false;
  
  bool new_painter = false;
  if ( painter == 0 )
  {
    new_painter = true;
    painter = new QPainter;
    painter->begin( this );
  }

  if ( curr )
    curr->pCell->setMarker( painter, next->tx, next->ty, false );
  next->pCell->setMarker( painter, next->tx, next->ty, true );

  if ( new_painter )
  {
    painter->end();
    delete painter;
    painter = 0;
  }

  if ( next->ty + next->pCell->getYPos() - next->pCell->getAscent() < 0 )
    emit scrollVert( y_offset + ( next->ty + next->pCell->getYPos() - next->pCell->getAscent() ) );

  emit onURL( next->pCell->getURL() );

  return true;
}

bool KHTMLWidget::cellDown()
{
  if ( clue == 0 || parser )
    return true;
  
  QList<HTMLCellInfo> list;
  list.setAutoDelete( true );
  
  clue->findCells( -x_offset + leftBorder, -y_offset + topBorder, list );

  // A usual HTML page ?
  if ( list.isEmpty() )
    return false;
  
  HTMLCellInfo *curr = 0;
  HTMLCellInfo *next = 0;
  
  // Find current marker
  HTMLCellInfo *info;
  for ( info = list.first(); info != 0; info = list.next() )
  {
    if ( info->pCell->isMarked() )
    {
      curr = info;
      break;
    }
  }

  if ( curr == 0 )
    next = list.first();
  else
  { 
    while( list.current() )
    {
      if ( list.current()->baseAbs > curr->baseAbs )
	break;
      list.next();
    }

    if ( list.current() == 0 )
      return false;
    
    HTMLCellInfo *inf; 
    int diff = 0xFFFFFFF;
    for ( inf = list.current(); inf != 0; inf = list.next() )
    {
      int i = curr->xAbs - inf->xAbs;
      if ( i < 0 ) i *= -1;
      if ( i < diff )
      {
	diff = i;
	next = inf;
      }
    }
  }
  
  if ( next == 0 )
    return false;
  
  bool new_painter = false;
  if ( painter == 0 )
  {
    new_painter = true;
    painter = new QPainter;
    painter->begin( this );
  }

  if ( curr )
    curr->pCell->setMarker( painter, next->tx, next->ty, false );
  next->pCell->setMarker( painter, next->tx, next->ty, true );

  if ( new_painter )
  {
    painter->end();
    delete painter;
    painter = 0;
  }

  if ( next->ty + next->pCell->getYPos() + next->pCell->getDescent() > height() )
    emit scrollVert( y_offset + ( next->ty + next->pCell->getYPos() + next->pCell->getDescent() - height() ) );

  emit onURL( next->pCell->getURL() );

  return true;
}

bool KHTMLWidget::cellLeft()
{
  if ( clue == 0 || parser )
    return true;
  
  QList<HTMLCellInfo> list;
  list.setAutoDelete( true );
  
  clue->findCells( -x_offset + leftBorder, -y_offset + topBorder, list );

  if ( list.isEmpty() )
    return false;
  
  HTMLCellInfo *curr = 0;
  HTMLCellInfo *next = 0;
  
  // Find current marker
  HTMLCellInfo *info;
  for ( info = list.first(); info != 0; info = list.next() )
  {
    if ( info->pCell->isMarked() )
    {
      curr = info;
      break;
    }
  }

  if ( curr == 0 )
    next = list.first();
  else
    next = list.prev();
  
  if ( next == 0 )
    return false;
  
  bool new_painter = false;
  if ( painter == 0 )
  {
    new_painter = true;
    painter = new QPainter;
    painter->begin( this );
  }

  if ( curr )
    curr->pCell->setMarker( painter, next->tx, next->ty, false );
  next->pCell->setMarker( painter, next->tx, next->ty, true );

  if ( new_painter )
  {
    painter->end();
    delete painter;
    painter = 0;
  }

  if ( next->ty + next->pCell->getYPos() - next->pCell->getAscent() < 0 )
    emit scrollVert( y_offset + ( next->ty + next->pCell->getYPos() - next->pCell->getAscent() ) );

  emit onURL( next->pCell->getURL() );

  return true;
}

bool KHTMLWidget::cellRight()
{
  if ( clue == 0 || parser )
    return true;
  
  QList<HTMLCellInfo> list;
  list.setAutoDelete( true );
  
  clue->findCells( -x_offset + leftBorder, -y_offset + topBorder, list );

  if ( list.isEmpty() )
    return false;
  
  HTMLCellInfo *curr = 0;
  HTMLCellInfo *next = 0;
  
  // Find current marker
  HTMLCellInfo *info;
  for ( info = list.first(); info != 0; info = list.next() )
  {
    if ( info->pCell->isMarked() )
    {
      curr = info;
      break;
    }
  }

  if ( curr == 0 )
    next = list.first();
  else
    next = list.next();
  
  if ( next == 0 )
    return false;
  
  bool new_painter = false;
  if ( painter == 0 )
  {
    new_painter = true;
    painter = new QPainter;
    painter->begin( this );
  }

  if ( curr )
    curr->pCell->setMarker( painter, next->tx, next->ty, false );
  next->pCell->setMarker( painter, next->tx, next->ty, true );

  if ( new_painter )
  {
    painter->end();
    delete painter;
    painter = 0;
  }

  emit onURL( next->pCell->getURL() );
  
  if ( next->ty + next->pCell->getYPos() + next->pCell->getDescent() > height() )
    emit scrollVert( y_offset + ( next->ty + next->pCell->getYPos() + next->pCell->getDescent() - height() ) );

  return true;
}

void KHTMLWidget::cellSelected()
{
  if ( clue == 0 || parser )
    return;
  
  QList<HTMLCellInfo> list;
  list.setAutoDelete( true );
  
  clue->findCells( -x_offset + leftBorder, -y_offset + topBorder, list );

  if ( list.isEmpty() )
    return;
  
  HTMLCellInfo *curr = 0;
  
  // Find current marker
  HTMLCellInfo *info;
  for ( info = list.first(); info != 0; info = list.next() )
  {
    if ( info->pCell->isMarked() )
    {
      curr = info;
      break;
    }
  }

  if ( curr == 0 )
    return;
  if ( curr->pCell->getURL() == 0 )
    return;
  
  QStrList urllist;
  getSelected( urllist );

  bool mode = true;
  if ( urllist.find( curr->pCell->getURL() ) != -1 )
    mode = false;
  
  selectByURL( 0, curr->pCell->getURL(), mode );
}

void KHTMLWidget::cellActivated()
{
  if ( clue == 0 || parser )
    return;
  
  QList<HTMLCellInfo> list;
  list.setAutoDelete( true );
  
  clue->findCells( -x_offset + leftBorder, -y_offset + topBorder, list );

  if ( list.isEmpty() )
    return;
  
  HTMLCellInfo *curr = 0;
  
  // Find current marker
  HTMLCellInfo *info;
  for ( info = list.first(); info != 0; info = list.next() )
  {
    if ( info->pCell->isMarked() )
    {
      curr = info;
      break;
    }
  }

  if ( curr == 0 )
    return;
  if ( curr->pCell->getURL() == 0 )
    return;

  emit URLSelected( curr->pCell->getURL(), LeftButton, curr->pCell->getTarget() );
}

void KHTMLWidget::cellContextMenu()
{
  if ( clue == 0 || parser )
    return;
  
  QList<HTMLCellInfo> list;
  list.setAutoDelete( true );
  
  clue->findCells( -x_offset + leftBorder, -y_offset + topBorder, list );

  if ( list.isEmpty() )
    return;
  
  HTMLCellInfo *curr = 0;
  
  // Find current marker
  HTMLCellInfo *info;
  for ( info = list.first(); info != 0; info = list.next() )
  {
    if ( info->pCell->isMarked() )
    {
      curr = info;
      break;
    }
  }

  if ( curr == 0 )
    return;

//  printf("curr->url='%s'\n",curr->pCell->getURL());
  
  if ( curr->pCell->getURL() == 0 )
    return;

  QPoint p( curr->tx, curr->ty );
  
  emit popupMenu( curr->pCell->getURL(), mapToGlobal( p ) );
}

//-----------------------------------------------------------
// End KFM Extensions
//-----------------------------------------------------------

HTMLMap *KHTMLWidget::getMap( const char *mapurl )
{
    HTMLMap *map;

    for ( map = mapList.first(); map != 0; map = mapList.next() )
    {
        if ( strcasecmp( map->mapURL(), mapurl ) == 0 )
            return map;
    }
    return 0;
}

/*
 * Add a image map
 */

void KHTMLWidget::addMap( const char *mapUrl)
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

bool KHTMLWidget::URLVisited( const char *_url )
{
    if ( htmlView )
	return htmlView->URLVisited( _url );

    return false;
}

void KHTMLWidget::setBaseURL( const char *_url)
{
    baseURL = _url;
    baseURL.setReference( 0 );
    baseURL.setSearchPart( 0 );
            
    QString p = baseURL.httpPath();
               
    if ( p.length() > 0 )
    {
        int pos = p.findRev( '/' );
        if ( pos >= 0 )
            p.truncate( pos );
    }
    p += "/";
    baseURL.setPath( p );
}

void KHTMLWidget::addParsedTarget( const char *_target)
{
    parsedTargets.append( _target );
}
    
void KHTMLWidget::addParsedURL( const char *_url)
{
    parsedURLs.append( _url );
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

void KHTMLWidget::addFrame( HTMLFrameSet *_frameSet, const char *_name,
                            bool _scrolling, bool _resize,
                            int _frameborder, 
                            int _marginwidth, int _marginheight,
                            const char *_src)
{
    // Create the frame,
    KHTMLView *frame = htmlView->newView( _frameSet, _name );
    frame->setIsFrame( TRUE );
    frame->setScrolling( _scrolling );
    frame->setAllowResize( _resize );
    frame->setFrameBorder( _frameborder );
    frame->setMarginWidth( _marginwidth );
    frame->setMarginHeight( _marginheight );
    _frameSet->append( frame );

    if ( _src )
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

void KHTMLWidget::addForm( HTMLForm *_form )
{
    formList.append( _form );
    connect( _form, 
             SIGNAL( submitted( const char *, const char *, const char * ) ),
             SLOT( slotFormSubmitted( const char *, const char *, const char * ) ) 
           );
}

void KHTMLWidget::setNewTitle( const char *_title)
{
    emit setTitle( _title );
}
     

void KHTMLWidget::setBGImage( const char *_url)
{
    KURL kurl( baseURL, _url );

    if ( strcmp( kurl.protocol(), "file" ) == 0 )
    {
        bgPixmap.load( kurl.path() );
        scheduleUpdate( true );
    }
    else
    {
        requestBackgroundImage( kurl.url() );
    }
}     

void KHTMLWidget::setBGColor( const QColor &_bgColor)
{
 
    if ( bDrawBackground ||
       settings->bgColor.red() != _bgColor.red() ||
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

bool KHTMLWidget::setCharset(const char *name){
    charsetName = name;
    return true; // Always true
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
                                                            
#include "khtml.moc"

