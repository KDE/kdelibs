/* This file is part of the KDE libraries
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
// KDE HTML Widget

#include <kurl.h>
#include <kapp.h>
#include <kcharsets.h>

#include "html.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>

#include <qimage.h>
#include <qregexp.h>
#include <qkeycode.h>
#include <qprinter.h>
#include <qdrawutl.h>

#include <X11/Xlib.h>

#ifdef HAVE_LIBJPEG
#include "jpeg.h"
#endif

#define PRINTING_MARGIN		36	// printed margin in 1/72in units
#define INDENT_SIZE		30

#define TIMER_INTERVAL		30	// ms between parser parses

//----------------------------------------------------------------------------
// convert number to roman numerals
QString toRoman( int number, bool upper )
{
    QString roman;
    char ldigits[] = { 'i', 'v', 'x', 'l', 'c', 'd', 'm' };
    char udigits[] = { 'I', 'V', 'X', 'L', 'C', 'D', 'M' };
    char *digits = upper ? udigits : ldigits;
    int i, d = 0;

    do
    {   
	int num = number % 10;

	if ( num % 5 < 4 )
	    for ( i = num % 5; i > 0; i-- )
		roman.insert( 0, digits[ d ] );

	if ( num >= 4 && num <= 8)
	    roman.insert( 0, digits[ d+1 ] );

	if ( num == 9 )
	    roman.insert( 0, digits[ d+2 ] );

	if ( num % 5 == 4 )
	    roman.insert( 0, digits[ d ] );

	number /= 10;
	d += 2;
    }
    while ( number );

    return roman;
}

//----------------------------------------------------------------------------

HTMLFontManager* pFontManager = 0;

// Array of mark parser functions, e.g:
// <img ...  is processed by KHTMLWidget::parseI()
// </ul>     is processed by KHTMLWidget::parseU()
//
parseFunc KHTMLWidget::parseFuncArray[26] = {
	&KHTMLWidget::parseA,
	&KHTMLWidget::parseB,
	&KHTMLWidget::parseC,
	&KHTMLWidget::parseD,
	&KHTMLWidget::parseE,
	&KHTMLWidget::parseF,
	&KHTMLWidget::parseG,
	&KHTMLWidget::parseH,
	&KHTMLWidget::parseI,
	&KHTMLWidget::parseJ,
	&KHTMLWidget::parseK,
	&KHTMLWidget::parseL,
	&KHTMLWidget::parseM,
	&KHTMLWidget::parseN,
	&KHTMLWidget::parseO,
	&KHTMLWidget::parseP,
	&KHTMLWidget::parseQ,
	&KHTMLWidget::parseR,
	&KHTMLWidget::parseS,
	&KHTMLWidget::parseT,
	&KHTMLWidget::parseU,
	&KHTMLWidget::parseV,
	&KHTMLWidget::parseW,
	&KHTMLWidget::parseX,
	&KHTMLWidget::parseY,
	&KHTMLWidget::parseZ
};


KHTMLWidget::KHTMLWidget( QWidget *parent, const char *name, const char * )
    : KDNDWidget( parent, name, WPaintClever ), tempStrings( true ), parsedURLs( false ),
	parsedTargets( false )
{
    jsEnvironment = 0;      
    leftBorder    = LEFT_BORDER;
    rightBorder   = RIGHT_BORDER;
    topBorder     = TOP_BORDER;
    bottomBorder  = BOTTOM_BORDER;
    x_offset      = 0;
    y_offset      = 0;
    url           = 0;
    title         = "";    
    clue          = 0;
    italic        = false;
    weight        = QFont::Normal;
    stringTok     = 0;
    ht            = 0;
    colorContext  = 0;
    pressed       = false;
    pressedURL    = "";
    pressedTarget = "";
    actualURL     = "";
    baseURL       = "";
    baseTarget    = "";
    target        = 0;
    url           = 0;
    bIsSelected   = false;
    selectedFrame = 0;
    htmlView      = 0;
    bIsFrameSet   = false;
    bIsFrame      = false;
    frameSet      = 0;
    bFramesComplete = false;
    painter       = 0;
    parsing       = false;
    overURL       = "";
    granularity   = 600;
    linkCursor    = arrowCursor;
    bIsTextSelected = false;
    charsetConverter = 0;

    framesetStack.setAutoDelete( false );
    framesetList.setAutoDelete( false );
    frameList.setAutoDelete( false ); 
    waitingFileList.setAutoDelete( false );
    formList.setAutoDelete( true );
    listStack.setAutoDelete( true );
    glossaryStack.setAutoDelete( true );
    mapList.setAutoDelete( true );
    colorStack.setAutoDelete( true );

    /* parsedURLs.setAutoDelete( true );
    parsedTargets.setAutoDelete( true );
    
    standardFont = "times";
    fixedFont = "courier"; */

    defaultSettings = new HTMLSettings;
    settings = new HTMLSettings;

    QPalette pal = palette().copy();
    QColorGroup cg = pal.normal();
    QColorGroup newGroup( cg.foreground(), lightGray, cg.light(),
	    cg.dark(), cg.mid(), cg.text(), lightGray );
    pal.setNormal( newGroup );
    setPalette( pal );

    QString f = kapp->kdedir().copy();
    f += "/share/apps/khtmlw/pics/khtmlw_dnd.xpm";
    dndDefaultPixmap.load( f.data() );
    
    registerFormats();

    setMouseTracking( true );    

    connect( &updateTimer, SIGNAL( timeout() ), SLOT( slotUpdate() ) );

    if ( !pFontManager )
	pFontManager = new HTMLFontManager();
}

void KHTMLWidget::requestFile( HTMLObject *_obj, const char *_url )
{
    debug( "Requesting File: %s", _url );
    waitingFileList.append( _obj );
    emit fileRequest( _url );
}

void KHTMLWidget::cancelRequestFile( HTMLObject *_obj )
{
    if ( waitingFileList.findRef( _obj ) != -1 )
    {
	waitingFileList.removeRef( _obj );
	emit cancelFileRequest( _obj->requestedFile() );
    }
}

void KHTMLWidget::cancelAllRequests()
{
    HTMLObject *o;

    for ( o = waitingFileList.first(); o != 0; o = waitingFileList.next() )
	emit cancelFileRequest( o->requestedFile() );

    waitingFileList.clear();
}

void KHTMLWidget::requestBackgroundImage( const char *_url )
{
    bgPixmapURL = _url;
    bgPixmapURL.detach();
    emit fileRequest( _url );
}

void KHTMLWidget::slotFileLoaded( const char *_url, const char *_filename )
{
    QList<HTMLObject> del;
    del.setAutoDelete( FALSE );

    HTMLObject *p;    
    for ( p = waitingFileList.first(); p != 0L; p = waitingFileList.next() )
    {
	if ( strcmp( _url, p->requestedFile() ) == 0 )
	{
	    del.append( p );
	    p->fileLoaded( _filename );
	}
    }

    // Are we waiting for the background image ?
    if ( !bgPixmapURL.isEmpty() )
    {
	// Did the background image arrive ?
	if ( strcmp( bgPixmapURL, _url ) == 0 )
	{
	    bgPixmap.load( _filename );					
	    bgPixmapURL = 0L;
	    scheduleUpdate( true );
	}
    }    

    for ( p = del.first(); p != 0L; p = del.next() )
	waitingFileList.removeRef( p );

    if ( waitingFileList.count() == 0 )
    {
	if ( !parsing )
	{
	    emit documentDone();
	}
    }
}

void KHTMLWidget::slotFormSubmitted( const char *_method, const char *_url )
{
    emit formSubmitted( _method, _url );
}

void KHTMLWidget::mousePressEvent( QMouseEvent *_mouse )
{
    if ( clue == 0L )
	return;

    // Make this frame the active one
    if ( bIsFrame && !bIsSelected )
	htmlView->setSelected( TRUE );
    
    if ( _mouse->button() == LeftButton )
    {
    	pressed = TRUE;
	// deselect all currently selected text
	if ( bIsTextSelected )
	{
	    bIsTextSelected = false;
	    selectText( 0, 0, 0, 0, 0 );	// deselect all text
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

    if ( obj != 0L)
    {
	if ( obj->getURL() != 0 )
	{
	    if (obj->getURL()[0] != 0)
	    {
		// Save data. Perhaps the user wants to start a drag.
		if ( _mouse->button() == LeftButton )
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
      if ( htmlView->mousePressedHook( 0L, 0L, _mouse, FALSE ) )
	return;
    if ( _mouse->button() == RightButton )
	emit popupMenu( 0L, mapToGlobal( _mouse->pos() ) );    
}

void KHTMLWidget::mouseDoubleClickEvent( QMouseEvent *_mouse )
{
    if ( clue == 0L )
	return;
    
    HTMLObject *obj;
    
    obj = clue->checkPoint( _mouse->pos().x() + x_offset - leftBorder,
	_mouse->pos().y() + y_offset - topBorder );
    
    if ( obj != 0L)
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
	    selectText( 0, selectPt2.x(), selectPt2.y(),
		selectPt1.x(), selectPt1.y() );
	}
	else
	{
	    selectText( 0, selectPt1.x(), selectPt1.y(),
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

    // Does the parent want to process the event now ?
    if ( htmlView )
    {
      if ( htmlView->mouseReleaseHook( _mouse ) )
	return;
    }

    // emit textSelected() if necessary
    if ( _mouse->button() == LeftButton )
    {
	if ( bIsTextSelected )
	{
	    debugM( "Text Selected\n" );
	    emit textSelected( true );
	}
    }

    if ( clue == 0L )
	return;
    if ( pressedURL.isEmpty() )
	return;

    // if ( pressedURL.data()[0] == '#' )
    //	gotoAnchor( pressedURL.data() + 1 );
    // else
    if ( _mouse->button() != RightButton )
    {
	printf("pressedURL='%s'\n",pressedURL.data());
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

bool KHTMLWidget::URLVisited( const char *_url )
{
    if ( htmlView )
	return htmlView->URLVisited( _url );

    return false;
}

/*
 * Checks out wether there is a URL under the point p and returns a pointer
 * to this URL or 0L if there is none.
 */
const char* KHTMLWidget::getURL( QPoint &p )
{
    if ( clue == 0L )
	return 0L;
    
    HTMLObject *obj;
    
    obj = clue->checkPoint( p.x() + x_offset, p.y() + y_offset );
    
    if ( obj != 0L)
	if ( obj->getURL() && obj->getURL()[0] != 0 )
	    return obj->getURL();
    
    return 0L;
}

void KHTMLWidget::select( QPainter * _painter, QRect &_rect )
{
    if ( clue == 0L )
	return;
    
    bool newPainter = FALSE;

    QRect r = _rect;
    // r.setTop( r.top() + y_offset );
    // r.setLeft( r.left() + x_offset );
    
    int tx = -x_offset + leftBorder;
    int ty = -y_offset + topBorder;

    if ( _painter == 0L )
    {
	if ( painter == 0 )
	{
	    // debugT("New Painter for painting\n");
	    painter = new QPainter();
	    painter->begin( this );
	    newPainter = TRUE;
	}
    
	clue->select( painter, r, tx, ty );

	if ( newPainter )
	{
	    painter->end();
	    delete painter;
	    painter = 0;
	}
    }
    else
      clue->select( _painter, r, tx, ty );    
}

void KHTMLWidget::selectText( QPainter * _painter, int _x1, int _y1,
	int _x2, int _y2 )
{
    if ( clue == 0L )
	return;

    bool newPainter = FALSE;

    int tx = -x_offset + leftBorder;
    int ty = -y_offset + topBorder;

    if ( _painter == 0L )
    {
	if ( painter == 0 )
	{
	    painter = new QPainter();
	    painter->begin( this );
	    newPainter = TRUE;
	}
    
	bIsTextSelected = clue->selectText( painter, _x1, _y1, _x2, _y2, tx, ty );

	if ( newPainter )
	{
	    painter->end();
	    delete painter;
	    painter = 0;
	}
    }
    else
	bIsTextSelected = clue->selectText( _painter, _x1, _y1, _x2, _y2, tx, ty );
}

void KHTMLWidget::paintEvent( QPaintEvent* _pe )
{
    bool newPainter = FALSE;

    if ( clue == 0L )
	return;

    if ( painter == 0 )
    {
	painter = new QPainter;
	painter->begin( this );
	newPainter = TRUE;
    }

    painter->setBackgroundColor( backgroundColor() );

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
	if ( clue == 0L )
	    return;

	if ( isFrameSet() )
	{
	    framesetList.getFirst()->setGeometry( 0, 0, width(), height() );
	}            
	else if ( clue && _re->oldSize().width() != _re->size().width() )
	{
	    calcSize();
	    calcAbsolutePos();
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

//    if ( parsing )
//	return;
    
    if ( clue == 0L )
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
	int tx = x_offset + leftBorder;
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
    if ( !parsing )
	bDrawBackground = true;
}

void KHTMLWidget::calcAbsolutePos()
{
    if ( clue )
	clue->calcAbsolutePos( 0, 0 );
}

void KHTMLWidget::getSelected( QStrList &_list )
{
    if ( clue == 0L )
	return;
    
    clue->getSelected( _list );
}

void KHTMLWidget::getSelectedText( QString &_str )
{
    if ( clue == 0L )
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
	}

	if ( printer.orientation() == QPrinter::Landscape )
	{
	    int tmp = pgWidth;
	    pgWidth = pgHeight;
	    pgHeight = tmp;
	}

	pgWidth -= ( 2*PRINTING_MARGIN );
	pgHeight -= ( 2*PRINTING_MARGIN );

	QPainter prPainter;
	prPainter.begin( &printer );

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
	    PRINTING_MARGIN, PRINTING_MARGIN-breaks[b], true );
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

void KHTMLWidget::begin( const char *_url, int _x_offset, int _y_offset )
{
    emit documentStarted();
    
    bIsFrameSet = FALSE;
    // bIsFrame = FALSE;
    bFramesComplete = FALSE;
    framesetStack.clear();
    framesetList.clear();
    frameList.clear();

    if ( bIsTextSelected )
    {
	bIsTextSelected = false;
	emit textSelected( false );
    }
    
    if ( frameSet )
    {
	delete frameSet;
	frameSet = 0L;
    }

    x_offset = _x_offset;
    y_offset = _y_offset;

    emit scrollHorz( x_offset );
    emit scrollVert( y_offset );
    
    bgPixmapURL = 0;
    
    stopParser();
    
    if ( _url != 0L )
    {
      actualURL = _url;
      KURL u( actualURL );
      QString p = u.path();
      if ( p.length() == 0 )
      {
	u.setReference( "" );
	QString base( u.url() );
	base += "/";
	baseURL = base;
      }
      else if ( p.right(1) == "/" )
      {
	u.setReference( "" );
	baseURL = u.url();
      }
      else
      {
	u.setReference( "" );
	QString base( u.url() );
	int pos = base.findRev( '/' );
	if ( pos > 0 )
		base.truncate( pos );
	baseURL = base;
      }
    }

    baseTarget = "";

    if ( stringTok )
	delete stringTok;
    stringTok = new StringTokenizer;

    if ( ht != 0 )
	delete ht;
    ht = new HTMLTokenizer( this );
    ht->begin();

    writing = true;
}

void KHTMLWidget::write( const char *_str)
{
    if ( _str == 0L )
	return;
    
    ht->write( _str );

    // If the timer has been stopped while waiting for more html,
    // start it again.
    if ( parsing && timerId == 0 )
	timerId = startTimer( TIMER_INTERVAL );
}

void KHTMLWidget::end()
{
    writing = false;
    if ( ht )
	ht->end();
}

// changes a current font
// needed for headings, and whenever the font family change is necessery
void KHTMLWidget::selectFont( const char *_fontfamily, int _fontsize, int _weight, bool _italic )
{
    if ( _fontsize < 0 )
	_fontsize = 0;
    else if ( _fontsize >= MAXFONTSIZES )
	_fontsize = MAXFONTSIZES - 1;

    HTMLFont f( _fontfamily, _fontsize, _weight, _italic,
            font_stack.top()->charset());
    f.setTextColor( *(colorStack.top()) );
    const HTMLFont *fp = pFontManager->getFont( f );

    font_stack.push( fp );
    painter->setFont( *(font_stack.top()) );
}

void KHTMLWidget::selectFont( int _relative_font_size )
{
    int fontsize = settings->fontBaseSize + _relative_font_size;

    if ( !currentFont() )
    {
	fontsize = settings->fontBaseSize;
	debug( "aarrrgh - no font" );
    }

    if ( fontsize < 0 )
	fontsize = 0;
    else if ( fontsize >= MAXFONTSIZES )
	fontsize = MAXFONTSIZES - 1;

    HTMLFont f( font_stack.top()->family(), fontsize, weight,
	italic,  font_stack.top()->charset() );

    f.setUnderline( underline );
    f.setStrikeOut( strikeOut );
    f.setTextColor( *(colorStack.top()) );

    const HTMLFont *fp = pFontManager->getFont( f );

    font_stack.push( fp );
    painter->setFont( *(font_stack.top()) );
}

void KHTMLWidget::selectFont()
{
    int fontsize;
	
    if ( currentFont() )
	fontsize = currentFont()->size();
    else
    {
	fontsize = settings->fontBaseSize;
	debug( "aarrrgh - no font" );
    }

    HTMLFont f( font_stack.top()->family(), fontsize, weight,
	italic,  font_stack.top()->charset() );

    f.setUnderline( underline );
    f.setStrikeOut( strikeOut );
    f.setTextColor( *(colorStack.top()) );

    const HTMLFont *fp = pFontManager->getFont( f );

    font_stack.push( fp );
    painter->setFont( *(font_stack.top()) );
}

void KHTMLWidget::popFont()
{
    font_stack.pop();
    if ( font_stack.isEmpty() )
    {
	HTMLFont f( settings->fontBaseFace, settings->fontBaseSize );
	const HTMLFont *fp = pFontManager->getFont( f );
	font_stack.push( fp );
    }

    // we keep the current font color
    font_stack.top()->setTextColor( *(colorStack.top()) );

    painter->setFont( *(font_stack.top()) );
    weight = font_stack.top()->weight();
    italic = font_stack.top()->italic();
    underline = font_stack.top()->underline();
    strikeOut = font_stack.top()->strikeOut();
}

void KHTMLWidget::popColor()
{
    colorStack.remove();

    if ( colorStack.isEmpty() )
	colorStack.push( new QColor( settings->fontBaseColor ) );
}

void KHTMLWidget::parse()
{
//    emit documentStarted();
    
    // Dont parse an existing framed document twice.
    // If parse is called two times after begin() then
    // the second call is ususally done because the widget
    // has been resized.
    if ( bIsFrameSet && frameSet )
    {
	frameSet->resize( width(), height() );
	return;
    }
  
    // if there is no tokenizer then no html has been added
    if ( !ht )
	return;
	
    if ( !bgPixmapURL.isEmpty() )
	emit cancelFileRequest( bgPixmapURL );

    stopParser();
    
    if ( painter )
    {
	painter->end();
	delete painter;
    }
    painter = new QPainter();
    painter->begin( this );

    tempStrings.clear();

    char *str;
    for ( str = parsedURLs.first(); str; str = parsedURLs.next() )
	delete [] str;
    parsedURLs.clear();
    for ( str = parsedTargets.first(); str; str = parsedTargets.next() )
	delete [] str;
    parsedTargets.clear();

    // Initialize the font stack with the default font.
    italic = false;
    underline = false;
    strikeOut = false;
    weight = QFont::Normal;
    *settings = *defaultSettings;

    colorStack.clear();
    colorStack.push( new QColor( settings->fontBaseColor ) );
    
    font_stack.clear();
    HTMLFont f( settings->fontBaseFace, settings->fontBaseSize );
    f.setTextColor( settings->fontBaseColor );
    const HTMLFont *fp = pFontManager->getFont( f );
    font_stack.push( fp );

    // reset form related stuff
    formList.clear();
    form = 0;
    formSelect = 0;
    inOption = false;
    inTextArea = false;

    inTitle = false;
    bodyParsed = false;

    target = 0;
    url = 0;

    listStack.clear();
    glossaryStack.clear();
    mapList.clear();

    parsing = true;
    indent = 0;
    vspace_inserted = true;
    divAlign = HTMLClue::Left;

    // move to the first token
    ht->first();

    // clear page
    bDrawBackground = true;
    if ( bgPixmap.isNull() )
	painter->eraseRect( 0, 0, width(), height() );
    else
	drawBackground( x_offset, y_offset, 0, 0, width(), height() );

    if ( !bgPixmap.isNull() )
	bgPixmap.resize( -1, -1 );
    else
	bDrawBackground = false;
/*
    if ( colorContext )
    {
	QColor::leaveAllocContext();
	QColor::destroyAllocContext( colorContext );
    }

    colorContext = QColor::enterAllocContext();
*/
    if (clue)
	delete clue;
    clue = new HTMLClueV( 0, 0, width() - leftBorder - rightBorder );
    clue->setVAlign( HTMLClue::Top );
    clue->setHAlign( HTMLClue::Left );

    flow = 0;

    // this will call timerEvent which in turn calls parseBody
    timerId = startTimer( TIMER_INTERVAL );
}

void KHTMLWidget::stopParser()
{
    if ( !parsing )
	return;

    if ( timerId != 0 )
	killTimer( timerId );
    
    parsing = false;
}

void KHTMLWidget::timerEvent( QTimerEvent * )
{
    static const char *end[] = { "</body>", 0 }; 

    if ( !painter )
	return;

    killTimer( timerId );
    timerId = 0;

    if ( !ht->hasMoreTokens() && writing )
	return;

    const QFont &oldFont = painter->font();

    painter->setFont( *font_stack.top() );

    int lastHeight = docHeight();

    parseCount = granularity;
    if ( parseBody( clue, end, TRUE ) )
	stopParser();

    clue->calcSize();
    clue->setPos( 0, clue->getAscent() );
    calcAbsolutePos();

    painter->setFont( oldFont );

    // FE: synchronize painter's backgroundColor
    painter->setBackgroundColor( backgroundColor() );

    // If the visible rectangle was not filled before the parsing and
    // if we have something to display in the visible area now then repaint.
    if ( lastHeight - y_offset < height() * 2 && docHeight() - y_offset > 0 )
	scheduleUpdate( false );

    emit documentChanged();

    // Parsing is over ?
    if ( !parsing )
    {
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
	if ( waitingFileList.count() == 0 && bgPixmapURL.isEmpty() )
	{
	    emit documentDone();
	}

	// Now it is time to tell all frames what they should do
	KHTMLView *v;
	KHTMLWidget *w;
	for ( w = frameList.first(); w != 0L; w = frameList.next() )
	{
	    v = w->getView();
	    if ( v->getCookie() )
		v->openURL( v->getCookie() );
	    v->show();
	}

	HTMLFrameSet *s;
	for ( s = framesetList.first(); s != 0L; s = framesetList.next() )
	{
	    s->show();
	}
	if ( ( s = framesetList.getFirst() ) )
	    s->setGeometry( 0, 0, width(), height() );
    }
    else
	timerId = startTimer( TIMER_INTERVAL );
}

void KHTMLWidget::calcSize()
{
    if ( clue == 0 )
	return;

    clue->reset();
    clue->setMaxWidth( width() - leftBorder - rightBorder );
    clue->calcSize();
    clue->setPos( 0, clue->getAscent() );

    emit documentChanged();
}

bool KHTMLWidget::insertVSpace( HTMLClueV *_clue, bool _vspace_inserted )
{
    if ( !_vspace_inserted )
    {
	HTMLClueFlow *f = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
	_clue->append( f );
	HTMLVSpace *t = new HTMLVSpace( HTMLFont::pointSize( settings->fontBaseSize ) );
	f->append( t );
	flow = 0;
    }
    
    return true;
}

const char* KHTMLWidget::parseBody( HTMLClueV *_clue, const char *_end[], bool toplevel )
{
    const char *str;
    
    // Before one can write any objects in the body he usually adds a FlowBox
    // to _clue. Then put all texts, images etc. in the FlowBox.
    // If f == 0L, you have to create a new FlowBox, otherwise you can
    // use the one stored in f. Setting f to 0L means closing the FlowBox.

    while ( ht->hasMoreTokens() && parsing )
    {
	str = ht->nextToken();

	if ( *str == '\0' )
	    continue;

	if ( *str == ' ' && *(str+1) == '\0' )
	{
	    // if in* is set this text belongs in a form element
	    if ( inOption || inTextArea )
		formText += " ";
	    else if ( inTitle )
		title += " ";
	    else if ( flow != 0)
	    {
		HTMLText *t;
		if ( url || target )
		    t = new HTMLLinkText( " ", currentFont(), painter,
			url, target );
		else
		    t = new HTMLText( " ", currentFont(), painter );
		t->setSeparator( true );
		flow->append( t );
	    }
	}
	else if ( *str != TAG_ESCAPE )
	{
	    // if in* is set this text belongs in a form element
	    if ( inOption || inTextArea )
	    {
		formText += str;
	    }
	    else if ( inTitle )
	    {
		title += str;
	    }
	    else
	    {
		vspace_inserted = false;

		if ( flow == 0 )
		{
		    flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
		    flow->setIndent( indent );
		    flow->setHAlign( divAlign );
		    _clue->append( flow );
		}
		
    	        if (charsetConverter){
		   QList<KCharsetConversionResult> rl=
		           charsetConverter->multipleConvert(str);
		   KCharsetConversionResult *r;
		   for(r=rl.first();r;r=rl.next()){ 
		      	char *str1=r->copy();
		        HTMLFont f=*currentFont();
			debugM("%s(%s)\n",str1,r->charset());
			f.setCharset(r->charset());
	                const HTMLFont *fp = pFontManager->getFont( f );
		       
		   	if ( url || target )
		       		flow->append( new HTMLLinkText( str1, fp,
					painter, url, target,TRUE ) );
		   	else
		       		flow->append( new HTMLText( str1, fp,
				        painter,TRUE ) );
		  }		
		}
		else{
		  bool autoDelete;
	          const HTMLFont *fp = currentFont();
		  if (*str=='&'){ // we don't need converter for this
		     char *buffer=new char(strlen(str)+2); // buffer will never
		                                           // have to be longer
		     int l;
		     const char *str1;
		     KCharsets *charsets=KApplication::getKApplication()
		                                           ->getCharsets();
		     debugM("Token: %s\n",str);
		     const KCharsetConversionResult &r=charsets->convertTag(str,l);
		     str1=r;					 
	             debugM("Converted to: %s (length: %i)\n",str1,l);
		     if (str1 && l){
		       HTMLFont f=*fp;
		       if (r.charset().ok()){
		         debugM("Needed charset: %s\n",r.charset().name());
		         f.setCharset(r.charset());
			 debugM("Set font: %s\n",charsets->name(QFont(f)));
	                 fp = pFontManager->getFont( f );
			 debugM("Got font: %s\n",charsets->name(QFont(f)));
		       }  
		       strcpy(buffer,str1);
		       strcat(buffer,str+l);
		       str=buffer;
	               debugM("Result:%s\n",str);
		       autoDelete=TRUE;
		     }  
		  }
		  else autoDelete=FALSE;
		
		  if ( url || target )
		      flow->append( new HTMLLinkText( str, fp, painter,
		  	 url, target,autoDelete ) );
		  else
		      flow->append( new HTMLText( str, fp, painter,autoDelete ) );
		}      
	    }
	}
	else
	{
	    str++;

	    int i = 0;

	    while ( _end[i] != 0 )
	    {
		if ( strncasecmp( str, _end[i], strlen( _end[i] ) ) == 0 )
		{
		    return str;
		}
		i++;
	    }
	    
	    // The tag used for line break when we are in <pre>...</pre>
	    if ( *str == '\n' )
	    {
		// tack a space on the end to ensure the previous line is not
		// zero pixels high
		if ( flow && !flow->hasChildren() )
		    flow->append( new HTMLText( currentFont(), painter ) );
		flow = new HTMLClueH( 0, 0, _clue->getMaxWidth() );
		flow->setIndent( indent );
		_clue->append( flow );
	    }
	    else
		parseOneToken( _clue, str );
	}

	// perhaps we have the frame read complete. So skip the rest
	if ( bFramesComplete )
	{
	    stopParser();
	    return 0;
	}

	if ( toplevel )
	{
	    if ( parseCount <= 0 )
		return 0L;
	}
	parseCount--;
    }

    if ( !ht->hasMoreTokens() && toplevel && !writing )
	stopParser();
    
    return 0L;
}

const char *KHTMLWidget::parseOneToken( HTMLClueV *_clue, const char *str )
{
    if ( *str == '<' )
    {
	int indx;

	str++;

	if ( *str == '/' )
	    indx = *(str+1) - 'a';
	else
	    indx = *str - 'a';
	
	if ( indx >= 0 && indx < 26 )
	    (this->*(parseFuncArray[indx]))( _clue, str );
	
    }

    return 0;
}

// <a               </a>
// <address>        </address>
// <area            </area>
void KHTMLWidget::parseA( HTMLClueV *_clue, const char *str )
{
    if ( strncmp( str, "area", 4 ) == 0 )
    {
	if ( mapList.isEmpty() )
	    return;

	stringTok->tokenize( str + 5, " >" );

	QString href;
	QString coords;
	QString atarget = baseTarget.copy();
	HTMLArea::Shape shape = HTMLArea::Rect;

	while ( stringTok->hasMoreTokens() )
	{
	    const char* p = stringTok->nextToken();

	    if ( strncasecmp( p, "shape=", 6 ) == 0 )
	    {
		if ( strncasecmp( p+6, "rect", 4 ) == 0 )
		    shape = HTMLArea::Rect;
		else if ( strncasecmp( p+6, "poly", 4 ) == 0 )
		    shape = HTMLArea::Poly;
		else if ( strncasecmp( p+6, "circle", 6 ) == 0 )
		    shape = HTMLArea::Circle;
	    }
	    else if ( strncasecmp( p, "href=", 5 ) == 0 )
	    {
		p += 5;
		if ( *p == '#' )
		{// reference
		    KURL u( actualURL );
		    u.setReference( p + 1 );
		    href = u.url();
		}
		else if ( strchr( p, ':' ) )
		{// full URL
		    href =  p;
		}
		else
		{// relative URL
		    KURL u2( baseURL, p );
		    href = u2.url();
		}
	    }
	    else if ( strncasecmp( p, "target=", 7 ) == 0 )
	    {
		atarget = p+7;
	    }
	    else if ( strncasecmp( p, "coords=", 7 ) == 0 )
	    {
		coords = p+7;
	    }
	}

	if ( !coords.isEmpty() )
	{
	    HTMLArea *area = 0;

	    switch ( shape )
	    {
		case HTMLArea::Rect:
		    {
			int x1, y1, x2, y2;
			sscanf( coords, "%d,%d,%d,%d", &x1, &y1, &x2, &y2 );
			QRect rect( x1, y1, x2-x1, y2-y1 );
			area = new HTMLArea( rect, href, atarget );
			debugM( "Area Rect %d, %d, %d, %d\n", x1, y1, x2, y2 );
		    }
		    break;

		case HTMLArea::Circle:
		    {
			int xc, yc, rc;
			sscanf( coords, "%d,%d,%d", &xc, &yc, &rc );
			area = new HTMLArea( xc, yc, rc, href, atarget );
			debugM( "Area Circle %d, %d, %d\n", xc, yc, rc );
		    }
		    break;

		case HTMLArea::Poly:
		    {
			debugM( "Area Poly " );
			int count = 0, x, y;
			QPointArray parray;
			const char *ptr = coords;
			while ( ptr )
			{
			    x = atoi( ptr );
			    ptr = strchr( ptr, ',' );
			    if ( ptr )
			    {
				y = atoi( ++ptr );
				parray.resize( count + 1 );
				parray.setPoint( count, x, y );
				debugM( "%d, %d  ", x, y );
				count++;
				ptr = strchr( ptr, ',' );
				if ( ptr ) ptr++;
			    }
			}
			debugM( "\n" );
			if ( count > 2 )
				area = new HTMLArea( parray, href, atarget );
		    }
		    break;
	    }

	    if ( area )
		    mapList.getLast()->addArea( area );
	}
    }
    else if ( strncmp( str, "address", 7) == 0 )
    {
//	vspace_inserted = insertVSpace( _clue, vspace_inserted );
	flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
	flow->setHAlign( divAlign );
	_clue->append( flow );
	italic = TRUE;
	weight = QFont::Normal;
	selectFont();
    }
    else if ( strncmp( str, "/address", 8) == 0 )
    {
	popFont();
    }
    else if ( strncmp( str, "a ", 2 ) == 0 )
    {
	char tmpurl[1024];
	tmpurl[0] = '\0';
	target = 0;
	bool visited = false;
	const char *p;

	stringTok->tokenize( str + 2, " >" );

	while ( ( p = stringTok->nextToken() ) != 0 )
	{
	    if ( strncasecmp( p, "href=", 5 ) == 0 )
	    {
		p += 5;

		if ( *p == '#' )
		{// reference
		    KURL u( actualURL );
		    u.setReference( p + 1 );
		    strcpy( tmpurl, u.url() );
		}
		else if ( strchr( p, ':' ) )
		{// full URL
		    strcpy( tmpurl, p );
		}
		else
		{// relative URL
		    KURL u( baseURL );
		    KURL u2( baseURL, p );
		    strcpy( tmpurl, u2.url() );
		}

		visited = URLVisited( tmpurl );
	    }
	    else if ( strncasecmp( p, "name=", 5 ) == 0 )
	    {
		if ( flow == 0 )
		    _clue->append( new HTMLAnchor( p+5 ) );
		else
		    flow->append( new HTMLAnchor( p+5 ) );
	    }
	    else if ( strncasecmp( p, "target=", 7 ) == 0 )
	    {
		target = new char [ strlen( p+7 ) + 1 ];
		strcpy( target, p+7 );
		parsedTargets.append( target );
	    }
	}
	if ( !target && !baseTarget.isEmpty() )
	{
	    target = new char [ baseTarget.length()+1 ];
	    strcpy( target, baseTarget );
	    parsedTargets.append( target );
	}
	if ( tmpurl[0] != '\0' )
	{
	    vspace_inserted = false;
	    if ( visited )
		colorStack.push( new QColor( settings->vLinkColor ) );
	    else
		colorStack.push( new QColor( settings->linkColor ) );
	    if ( settings->underlineLinks )
		underline = true;
	    selectFont();
	    url = new char [ strlen( tmpurl ) + 1 ];
	    strcpy( url, tmpurl );
	    parsedURLs.append( url );
	}
    }
    else if ( strncmp( str, "/a", 2 ) == 0 )
    {
	closeAnchor();
    }
}

// <b>              </b>
// <base
// <basefont                        unimplemented
// <big>            </big>
// <blockquote>     </blockquote>
// <body
// <br
void KHTMLWidget::parseB( HTMLClueV *_clue, const char *str )
{
    if ( strncmp( str, "basefont", 8 ) == 0 )
    {
    }
    else if ( strncmp(str, "base", 4 ) == 0 )
    {
	stringTok->tokenize( str + 5, " >" );
	while ( stringTok->hasMoreTokens() )
	{
	    const char* token = stringTok->nextToken();
	    if ( strncasecmp( token, "target=", 7 ) == 0 )
	    {
		baseTarget = token+7;
	    }
	    else if ( strncasecmp( token, "href=", 5 ) == 0 )
	    {
		baseURL = token + 5;
	    }
	}
    }
    else if ( strncmp(str, "big", 3 ) == 0 )
    {
	selectFont( +2 );
    }
    else if ( strncmp(str, "/big", 4 ) == 0 )
    {
	popFont();
    }
    else if ( strncmp(str, "blockquote", 10 ) == 0 )
    {
	indent += INDENT_SIZE;

	flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
	flow->setIndent( indent );
	_clue->append( flow );
    }
    else if ( strncmp(str, "/blockquote", 11 ) == 0 )
    {
	indent -= INDENT_SIZE;
	flow = 0;
    }
    else if ( strncmp( str, "body", 4 ) == 0 )
    {
	if ( bodyParsed )
	    return;

	bodyParsed = true;
	bool bgColorSet = FALSE;
	bool bgPixmapSet = FALSE;
	stringTok->tokenize( str + 5, " >" );
	while ( stringTok->hasMoreTokens() )
	{
	    const char* token = stringTok->nextToken();
	    if ( strncasecmp( token, "bgcolor=", 8 ) == 0 )
	    {
		if ( *(token+8) != '#' && strlen( token+8 ) == 6 )
		{
		    QString col = "#";
		    col += token+8;
		    settings->bgColor.setNamedColor( col );
		}
		else
		    settings->bgColor.setNamedColor( token+8 );
		bgColorSet = TRUE;
	    }
	    else if ( strncasecmp( token, "background=", 11 ) == 0 )
	    {
		const char* filename = token + 11;
		KURL kurl( baseURL, filename );
		if ( strcmp( kurl.protocol(), "file" ) == 0 )
		{
		    bgPixmap.load( kurl.path() );
		    scheduleUpdate( true );
		}
		else
		    requestBackgroundImage( kurl.url() );
		
		if ( !bgPixmap.isNull() )
		    bgPixmapSet = TRUE;
	    }
	    else if ( strncasecmp( token, "text=", 5 ) == 0 )
	    {
		settings->fontBaseColor.setNamedColor( token+5 );
		*(colorStack.top()) = settings->fontBaseColor;
		font_stack.top()->setTextColor( settings->fontBaseColor );
	    }
	    else if ( strncasecmp( token, "link=", 5 ) == 0 )
	    {
		settings->linkColor.setNamedColor( token+5 );
	    }
	    else if ( strncasecmp( token, "vlink=", 6 ) == 0 )
	    {
		settings->vLinkColor.setNamedColor( token+6 );
	    }
	}

	if ( !bgColorSet && !bgPixmapSet )
	{
	    QPalette pal = palette().copy();
	    QColorGroup cg = pal.normal();
	    QColorGroup newGroup( cg.foreground(), defaultSettings->bgColor,
		    cg.light(), cg.dark(), cg.mid(), cg.text(),
		    defaultSettings->bgColor );
	    pal.setNormal( newGroup );
	    setPalette( pal );

	    // simply testing if QColor == QColor fails!?, so we must compare
	    // each RGB
	    if ( defaultSettings->bgColor.red() != backgroundColor().red() ||
		defaultSettings->bgColor.green() != backgroundColor().green() ||
		defaultSettings->bgColor.blue() != backgroundColor().blue() ||
		bDrawBackground )
    		setBackgroundColor( defaultSettings->bgColor );
	}
	else if ( bgColorSet && !bgPixmapSet )
	{
	    QPalette pal = palette().copy();
	    QColorGroup cg = pal.normal();
	    QColorGroup newGroup( cg.foreground(), settings->bgColor,
		cg.light(), cg.dark(), cg.mid(), cg.text(), settings->bgColor );
	    pal.setNormal( newGroup );
	    setPalette( pal );

	    if ( settings->bgColor.red() != backgroundColor().red() ||
		settings->bgColor.green() != backgroundColor().green() ||
		settings->bgColor.blue() != backgroundColor().blue() ||
		bDrawBackground )
    		setBackgroundColor( settings->bgColor );
	}

	if ( painter )
	    painter->setBackgroundColor( backgroundColor() );
    }
    else if ( strncmp( str, "br", 2 ) == 0 )
    {
	HTMLVSpace::Clear clear = HTMLVSpace::CNone;

	stringTok->tokenize( str + 3, " >" );
	while ( stringTok->hasMoreTokens() )
	{
	    const char* token = stringTok->nextToken();
	    if ( strncasecmp( token, "clear=", 6 ) == 0 )
	    {
		if ( strncasecmp( token+6, "left", 4 ) == 0 )
		    clear = HTMLVSpace::Left;
		else if ( strncasecmp( token+6, "right", 5 ) == 0 )
		    clear = HTMLVSpace::Right;
		else if ( strncasecmp( token+6, "all", 3 ) == 0 )
		    clear = HTMLVSpace::All;
	    }
	}

	HTMLVSpace *vs = new HTMLVSpace( currentFont()->pointSize(), clear );

	if ( flow != 0 )
	    flow->append( vs );
	else
	    _clue->append( vs );

	vspace_inserted = false;
    }
    else if ( strncmp(str, "b", 1 ) == 0 )
    {
	if ( str[1] == '>' || str[1] == ' ' )
	{
	    weight = QFont::Bold;
	    selectFont();
	}
    }
    else if ( strncmp(str, "/b", 2 ) == 0 )
    {
	popFont();
    }
}

// <center>         </center>
// <cite>           </cite>
// <code>           </code>
// <cell>           </cell>
// <comment>        </comment>      unimplemented
void KHTMLWidget::parseC( HTMLClueV *_clue, const char *str )
{
	if (strncmp( str, "center", 6 ) == 0)
	{
		divAlign = HTMLClue::HCenter;
		flow = 0;
	}
	else if (strncmp( str, "/center", 7 ) == 0)
	{
		divAlign = HTMLClue::Left;
		flow = 0;
	}
	else if (strncmp( str, "cell", 4 ) == 0)
	{
	    HTMLClue *f = flow;
	    if ( flow == 0L )
	    {
		flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
		flow->setHAlign( divAlign );
		_clue->append( flow );
		f = flow;
	    }

	    parseCell( flow, str );

	    flow = f;

	    HTMLText *t = new HTMLText( "", currentFont(), painter );
	    t->setSeparator( true );
	    flow->append( t );
	}
	else if (strncmp( str, "cite", 4 ) == 0)
	{
		italic = TRUE;
		weight = QFont::Normal;
		selectFont();
	}
	else if (strncmp( str, "/cite", 5) == 0)
	{
		popFont();
	}
	else if (strncmp(str, "code", 4 ) == 0 )
	{
		selectFont( settings->fixedFontFace, settings->fontBaseSize,
		    QFont::Normal, FALSE );
	}
	else if (strncmp(str, "/code", 5 ) == 0 )
	{
		popFont();
	}
}

// <dir             </dir>          partial
// <div             </div>
// <dl>             </dl>
// <dt>             </dt>
void KHTMLWidget::parseD( HTMLClueV *_clue, const char *str )
{
    if ( strncmp( str, "dir", 3 ) == 0 )
    {
	closeAnchor();
	listStack.push( new HTMLList( Dir ) );
	indent += INDENT_SIZE;
    }
    else if ( strncmp( str, "/dir", 4 ) == 0 )
    {
	if ( listStack.remove() )
	{
	    indent -= INDENT_SIZE;
	    flow = 0;
	}
    }
    else if ( strncmp( str, "div", 3 ) == 0 )
    {
	stringTok->tokenize( str + 4, " >" );
	while ( stringTok->hasMoreTokens() )
	{
	    const char* token = stringTok->nextToken();
	    if ( strncasecmp( token, "align=", 6 ) == 0 )
	    {
		if ( strcasecmp( token + 6, "right" ) == 0 )
		    divAlign = HTMLClue::Right;
		else if ( strcasecmp( token + 6, "center" ) == 0 )
		    divAlign = HTMLClue::HCenter;
		else
		    divAlign = HTMLClue::Left;
	    }
	}

	flow = 0;
    }
    else if ( strncmp( str, "/div", 4 ) == 0 )
    {
	divAlign = HTMLClue::Left;
	flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
	flow->setIndent( indent );
	flow->setHAlign( divAlign );
	_clue->append( flow );
    }
    else if ( strncmp( str, "dl", 2 ) == 0 )
    {
	vspace_inserted = insertVSpace( _clue, vspace_inserted );
	closeAnchor();
	if ( glossaryStack.top() )
	{
	    indent += INDENT_SIZE;
	}
	glossaryStack.push( new GlossaryEntry( GlossaryDL ) );
	flow = 0;
    }
    else if ( strncmp( str, "/dl", 3 ) == 0 )
    {
	if ( !glossaryStack.top() )
	    return;

	if ( *glossaryStack.top() == GlossaryDD )
	{
	    glossaryStack.remove();
	    indent -= INDENT_SIZE;
	}
	glossaryStack.remove();
	if ( glossaryStack.top() )
	    indent -= INDENT_SIZE;
	vspace_inserted = insertVSpace( _clue, vspace_inserted );
    }
    else if (strncmp( str, "dt", 2 ) == 0)
    {
	if ( !glossaryStack.top() )
	    return;

	if ( *glossaryStack.top() == GlossaryDD )
	{
	    glossaryStack.pop();
	    indent -= INDENT_SIZE;
	}
	vspace_inserted = false;
	flow = 0;

	flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
	flow->setIndent( indent );
	_clue->append( flow );
    }
    else if (strncmp( str, "dd", 2 ) == 0)
    {
	if ( !glossaryStack.top() )
	    return;

	if ( *glossaryStack.top() != GlossaryDD )
	{
	    glossaryStack.push( new GlossaryEntry( GlossaryDD ) );
	    indent += INDENT_SIZE;
	}
	flow = 0;

	flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
	flow->setIndent( indent );
	_clue->append( flow );
    }
}

// <em>             </em>
void KHTMLWidget::parseE( HTMLClueV *, const char *str )
{
	if ( strncmp( str, "em", 2 ) == 0 )
	{
		italic = TRUE;
		selectFont();
	}
	else if ( strncmp( str, "/em", 3 ) == 0 )
	{
		popFont();
	}
}

// <font>           </font>
// <form>           </form>         partial
// <frame           <frame>
// <frameset        </frameset>
void KHTMLWidget::parseF( HTMLClueV *, const char *str )
{
	if ( strncmp( str, "font", 4 ) == 0 )
	{
	    stringTok->tokenize( str + 5, " >" );
	    int newSize = currentFont()->size() - settings->fontBaseSize;
	    QString newFace;
	    QColor *color = new QColor( *(colorStack.top()) );
	    while ( stringTok->hasMoreTokens() )
	    {
		    const char* token = stringTok->nextToken();
		    if ( strncasecmp( token, "size=", 5 ) == 0 )
		    {
			    int num = atoi( token + 5 );
			    if ( *(token + 5) == '+' || *(token + 5) == '-' )
				newSize = num;
			    else
				newSize = num - settings->fontBaseSize;
		    }
		    else if ( strncasecmp( token, "color=", 6 ) == 0 )
		    {
			    color->setNamedColor( token+6 );
		    }
		    else if ( strncasecmp( token, "face=", 5 ) == 0 )
		    {
			// try to find a matching font in the font list.
			StringTokenizer st;
			st.tokenize( token+5, " ," );
			while ( st.hasMoreTokens() )
			{
			    const char *fname = st.nextToken();
			    QFont tryFont( fname );
			    QFontInfo fi( tryFont );
			    if ( strcmp( tryFont.family(), fi.family() ) == 0 )
			    {
				// we found a matching font
				newFace = fname;
				break;
			    }
			}
		    }
	    }
	    colorStack.push( color );
	    if ( !newFace.isEmpty() )
		selectFont( newFace, newSize + settings->fontBaseSize,
		    currentFont()->weight(), currentFont()->italic() );
	    else
		selectFont( newSize );
	}
	else if ( strncmp( str, "/font", 5 ) == 0 )
	{
		popColor();
		popFont();
	}
	else if ( strncmp( str, "frameset", 8 ) == 0 )
        {
	  if ( !htmlView )
	      return;
	  // Determine the parent for the frameset
	  QWidget *p = this;
	  if ( framesetStack.count() > 0 )
	    p = framesetStack.getLast();

	  htmlView->setIsFrameSet( TRUE );
	  HTMLFrameSet *f = new HTMLFrameSet( p, str );

	  // Append the new set to a parent set ?
	  if ( framesetStack.count() > 0 )
	      framesetStack.getLast()->append( f );

	  // Append the set to the stack
	  framesetStack.append( f );
	  framesetList.append( f );
	}
	else if ( strncmp( str, "/frameset", 9 ) == 0 )
        {
	  if ( !htmlView )
	      return;
	    // Is it the toplevel frameset ?
	    if ( framesetStack.count() == 1 )
	    {
		if ( selectedFrame )
		    selectedFrame->setSelected( TRUE );
		// debugT("Showing First frame\n");
		framesetStack.getFirst()->parse();
		frameSet = framesetStack.getFirst();
		frameSet->setGeometry( 0, 0, width(), height() );
		frameSet->show();
		framesetStack.removeLast();
	    }
	    else if ( bIsFrameSet )
	    {
		framesetStack.removeLast();
	    }
	}	
	else if ( strncmp( str, "frame", 5 ) == 0 )
        {
	    if ( !framesetStack.isEmpty() )
	    {
	      QString src;
	      QString name;
	      int marginwidth = leftBorder;
	      int marginheight = rightBorder;
	      // 0 = no, 1 = yes, 2 = auto
	      int scrolling = 2;
	      bool noresize = FALSE;
	      // -1 = default ( 5 )
	      int frameborder = -1;
	      
	      stringTok->tokenize( str + 6, " >" );
	      while ( stringTok->hasMoreTokens() )
		{
		  const char* token = stringTok->nextToken();
		  if ( strncasecmp( token, "SRC=", 4 ) == 0 )
		    {
		      src = token + 4;
		    }
		  else if ( strncasecmp( token, "NAME=", 5 ) == 0 )
		    {
		      name = token + 5;
		    }
		  else if ( strncasecmp( token, "MARGINWIDTH=", 12 ) == 0 )
		    {
		      marginwidth = atoi( token + 12 );
		    }
		  else if ( strncasecmp( token, "MARGINHEIGHT=", 13 ) == 0 )
		    {
		      marginheight = atoi( token + 13 );
		    }
		  else if ( strncasecmp( token, "FRAMEBORDER=", 12 ) == 0 )
		    {
		      frameborder = atoi( token + 12 );
		      if ( frameborder < 0 )
			frameborder = -1;
		    }
		  else if ( strncasecmp( token, "NORESIZE", 8 ) == 0 )
		    noresize = TRUE;
		  else if ( strncasecmp( token, "SCROLLING=", 10 ) == 0 )
		    {
		      if ( strncasecmp( token + 10, "yes", 3 ) == 0 )
			scrolling = 1;
		      if ( strncasecmp( token + 10, "no", 2 ) == 0 )
			scrolling = 0;
		      if ( strncasecmp( token + 10, "auto", 4 ) == 0 )
			scrolling = -1;
		    }
		}	      

	      // Determine the parent for the frame
	      QWidget *p = this;
	      if ( framesetStack.count() > 0 )
		p = framesetStack.getLast();
	      // Create the HTML widget
	      KHTMLView *html = htmlView->newView( p, name );
	      html->setIsFrame( TRUE );
	      html->setScrolling( scrolling );
	      html->setAllowResize( !noresize );
	      html->setFrameBorder( frameborder );
	      html->setMarginWidth( marginwidth );
	      html->setMarginHeight( marginheight );
	      if ( src.isEmpty() )
	      {
		  framesetStack.getLast()->append( html );
	      }
	      else
	      {
		  // Determine the complete URL for this widget
		  KURL u( baseURL, src.data() );
		  connect( html, SIGNAL( frameSelected( KHTMLView * ) ),
			   this, SLOT( slotFrameSelected( KHTMLView * ) ) );
		  framesetStack.getLast()->append( html );

		  selectedFrame = html;
		  // Tell the new widget what it should show
		  // html->openURL( u.url().data() );
		  html->setCookie( u.url().data() );   
	      }
	      // html->show();
	      // Add frame to list
	      frameList.append( html->getKHTMLWidget() );  
	    }
	}
	else if ( strncmp( str, "form", 4 ) == 0 )
	{
		QString action = "";
		QString method = "GET";

		stringTok->tokenize( str + 5, " >" );
		while ( stringTok->hasMoreTokens() )
		{
			const char* token = stringTok->nextToken();
			if ( strncasecmp( token, "action=", 7 ) == 0 )
			{
				action = token + 7;
			}
			else if ( strncasecmp( token, "method=", 7 ) == 0 )
			{
				if ( strncasecmp( token + 7, "post", 4 ) == 0 )
					method = "POST";
			}
		}

		form = new HTMLForm( action, method );
		formList.append( form );
		connect( form, SIGNAL( submitted( const char *, const char *) ),
				SLOT( slotFormSubmitted( const char *, const char * ) ) );
	}
	else if ( strncmp( str, "/form", 5 ) == 0 )
	{
		form = 0;
	}
}

// <grid>           </grid>
void KHTMLWidget::parseG( HTMLClueV *, const char * )
{
    /* if ( strncmp( str, "grid", 4 ) == 0 )
    {
	vspace_inserted = insertVSpace( _clue, vspace_inserted );
	flow = 0L;
	parseGrid( _clue, _clue->getMaxWidth(), str + 6 );
    } */
}

// <h[1-6]>         </h[1-6]>
// <hr
void KHTMLWidget::parseH( HTMLClueV *_clue, const char *str )
{
        if ( *(str)=='h' &&
	    ( *(str+1)=='1' || *(str+1)=='2' || *(str+1)=='3' ||
     	      *(str+1)=='4' || *(str+1)=='5' || *(str+1)=='6' ) )
	{
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		HTMLClue::HAlign align = divAlign;

		stringTok->tokenize( str + 3, " >" );
		while ( stringTok->hasMoreTokens() )
		{
			const char* token = stringTok->nextToken();
			if ( strncasecmp( token, "align=", 6 ) == 0 )
			{
				if ( strcasecmp( token + 6, "center" ) == 0 )
					align = HTMLClue::HCenter;
				else if ( strcasecmp( token + 6, "right" ) == 0 )
					align = HTMLClue::Right;
			}
		}
		// Start a new flow box
		if ( !flow )
		{
		    flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
		    flow->setIndent( indent );
		    _clue->append( flow );
		}
		flow->setHAlign( align );

		switch ( str[1] )
		{
			case '1':
				weight = QFont::Bold;
				selectFont( +3 );
				break;

			case '2':
				weight = QFont::Bold;
				italic = FALSE;
				selectFont( +2 );
				break;

			case '3':
				weight = QFont::Bold;
				italic = FALSE;
				selectFont( +1 );
				break;

			case '4':
				weight = QFont::Bold;
				italic = FALSE;
				selectFont( +0 );
				break;

			case '5':
				weight = QFont::Normal;
				italic = TRUE;
				selectFont( +0 );
				break;

			case '6':
				weight = QFont::Bold;
				italic = FALSE;
				selectFont( -1 );
				break;
		}
	}
	else if ( *str=='/' && *(str+1)=='h' &&
	    ( *(str+2)=='1' || *(str+2)=='2' || *(str+2)=='3' ||
 	      *(str+2)=='4' || *(str+2)=='5' || *(str+2)=='6' ))
	{
		// Insert a vertical space if this did not happen already.
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		// Restore the old font
		popFont();
	}
	else if ( strncmp(str, "hr", 2 ) == 0 )
	{
		int size = 1;
		int length = _clue->getMaxWidth();
		int percent = 100;
		HTMLClue::HAlign align = divAlign;
		HTMLClue::HAlign oldAlign = divAlign;
		bool shade = TRUE;

		if ( flow )
		    oldAlign = align = flow->getHAlign();

		stringTok->tokenize( str + 3, " >" );
		while ( stringTok->hasMoreTokens() )
		{
			const char* token = stringTok->nextToken();
			if ( strncasecmp( token, "align=", 6 ) == 0 )
			{
				if ( strncasecmp( token + 6, "left", 4 ) == 0 )
					align = HTMLRule::Left;
				else if ( strncasecmp( token + 6, "right", 5 ) == 0 )
					align = HTMLRule::Right;
				else
					align = HTMLRule::HCenter;
			}
			else if ( strncasecmp( token, "size=", 5 ) == 0 )
			{
				size = atoi( token+5 );
			}
			else if ( strncasecmp( token, "width=", 6 ) == 0 )
			{
				if ( strchr( token+6, '%' ) )
					percent = atoi( token+6 );
				else
				{
					length = atoi( token+6 );
					percent = 0;
				}
			}
			else if ( strncasecmp( token, "noshade", 6 ) == 0 )
			{
				shade = FALSE;
			}
		}

		flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
		flow->setIndent( indent );
		flow->setHAlign( align );
		_clue->append( flow );

		flow->append( new HTMLRule( length, percent, size, shade ) );

		flow = 0;

		vspace_inserted = false;
	}
}

// <i>              </i>
// <img                             partial
// <input                           partial
void KHTMLWidget::parseI( HTMLClueV *_clue, const char *str )
{
    if (strncmp( str, "img", 3 ) == 0)
    {
	vspace_inserted = FALSE;

	// Parse all arguments but delete '<' and '>' and skip 'cell'
	const char* filename = 0L;
	QString fullfilename;
	QString usemap;
	bool    ismap = false;
	int width = -1;
	int height = -1;
	int percent = 0;
	int border = url == 0 ? 0 : 2;
	HTMLClue::HAlign align = HTMLClue::HNone;

	stringTok->tokenize( str + 4, " >" );
	while ( stringTok->hasMoreTokens() )
	{
	    const char* token = stringTok->nextToken();
	    if (strncasecmp( token, "src=", 4 ) == 0)
	    filename = token + 4;
	    else if (strncasecmp( token, "width=", 6 ) == 0)
	    {
		if ( strchr( token + 6, '%' ) )
		    percent = atoi( token + 6 );
		else
		    width = atoi( token + 6 );
	    }
	    else if (strncasecmp( token, "height=", 7 ) == 0)
		height = atoi( token + 7 );
	    else if (strncasecmp( token, "border=", 7 ) == 0)
		border = atoi( token + 7 );
	    else if (strncasecmp( token, "align=", 6 ) == 0)
	    {
		if ( strcasecmp( token + 6, "left" ) == 0 )
		    align = HTMLClue::Left;
		else if ( strcasecmp( token + 6, "right" ) == 0 )
		    align = HTMLClue::Right;
	    }
	    else if ( strncasecmp( token, "usemap=", 7 ) == 0 )
	    {
		if ( *(token + 7 ) == '#' )
		    usemap = token + 7;
		else
		{
		    if ( strchr( token + 7, ':' ) )
		    {// full URL
			usemap = token + 7;
		    }
		    else
		    {// relative URL
			KURL u( baseURL, token + 7 );
			usemap = u.url();
		    }
		}
	    }
	    else if ( strncasecmp( token, "ismap", 5 ) == 0 )
	    {
		ismap = true;
	    }
	}
	// if we have a file name do it...
	if ( filename != 0L )
	{
	    KURL kurl( baseURL, filename );
	    // Do we need a new FlowBox ?
	    if ( flow == 0)
	    {
		flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
		flow->setIndent( indent );
		flow->setHAlign( divAlign );
		_clue->append( flow );
	    }

	    HTMLImage *image;

	    if ( usemap.isEmpty() && !ismap )
	    {
		image =  new HTMLImage( this, kurl.url(), url, target,
			 _clue->getMaxWidth(), width, height, percent, border );
	    }
	    else
	    {
		// allocate enough mem for any URL which might be in the
		// image map
		char *newurl = new char [1024];
		newurl[0] = '\0';
		if ( url )
		{
		    strcpy( newurl, url );
		    delete [] url;
		}
		url = newurl;
		parsedURLs.removeLast();
		parsedURLs.append( url );

		// allocate enough mem for any target which might be in the
		// image map
		char *newtarget = new char [256];
		newtarget[0] = '\0';

		if ( target )
		{
		    strcpy( newtarget, target );
		    delete [] target;
		}
		else
		    strcpy( newtarget, "" );

		target = newtarget;
		parsedTargets.removeLast();
		parsedTargets.append( target );

		image =  new HTMLImageMap( this, kurl.url(), url, target,
			 _clue->getMaxWidth(), width, height, percent, border );
		if ( !usemap.isEmpty() )
		    ((HTMLImageMap *)image)->setMapURL( usemap );
		else
		    ((HTMLImageMap *)image)->setMapType( HTMLImageMap::ServerSide );
	    }

	    image->setBorderColor( *(colorStack.top()) );

	    if ( align != HTMLClue::Left && align != HTMLClue::Right )
	    {
		flow->append( image );
	    }
	    // we need to put the image in a HTMLClueAligned
	    else
	    {
		HTMLClueAligned *aligned = new HTMLClueAligned (flow, 0, 0, _clue->getMaxWidth() );
		aligned->setHAlign( align );
		aligned->append( image );
		flow->append( aligned );
	    }
	} 
    } 
    else if (strncmp( str, "input", 5 ) == 0)
    {
	if ( form == 0 )
		return;
	if ( flow == 0 )
	{
	    flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
	    flow->setIndent( indent );
	    flow->setHAlign( divAlign );
	    _clue->append( flow );
	}
	parseInput( str + 6 );
    }
    else if ( strncmp(str, "i", 1 ) == 0 )
    {
	if ( str[1] == '>' || str[1] == ' ' )
	{
	    italic = TRUE;
	    selectFont();
	}
    }
    else if ( strncmp( str, "/i", 2 ) == 0 )
    {
	popFont();
    }
}

void KHTMLWidget::parseJ( HTMLClueV *, const char * )
{
}

// <kbd>            </kbd>
void KHTMLWidget::parseK( HTMLClueV *, const char *str )
{
	if ( strncmp(str, "kbd", 3 ) == 0 )
	{
		selectFont( settings->fixedFontFace, settings->fontBaseSize,
		    QFont::Normal, FALSE );
	}
	else if ( strncmp(str, "/kbd", 4 ) == 0 )
	{
		popFont();
	}
}

// <listing>        </listing>      unimplemented
// <li>
void KHTMLWidget::parseL( HTMLClueV *_clue, const char *str )
{
    if (strncmp( str, "link", 4 ) == 0)
    {
    }
    else if (strncmp( str, "li", 2 ) == 0)
    {
	closeAnchor();
	QString item;
	ListType listType = Unordered;
	ListNumType listNumType = Numeric;
	int listLevel = 1;
	int itemNumber = 1;
	int indentSize = INDENT_SIZE;
	if ( listStack.count() > 0 )
	{
	    listType = listStack.top()->type;
	    listNumType = listStack.top()->numType;
	    itemNumber = listStack.top()->itemNumber;
	    listLevel = listStack.count();
	    indentSize = indent;
	}
	HTMLClueFlow *f = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
	_clue->append( f );
	HTMLClueH *c = new HTMLClueH( 0, 0, _clue->getMaxWidth() );
	c->setVAlign( HTMLClue::Top );
	f->append( c );

	// fixed width spacer
	HTMLClueV *vc = new HTMLClueV( 0, 0, indentSize, 0 );
	vc->setVAlign( HTMLClue::Top );
	c->append( vc );

	switch ( listType )
	{
	    case Unordered:
		flow = new HTMLClueFlow( 0, 0, vc->getMaxWidth(), 0 );
		flow->setHAlign( HTMLClue::Right );
		vc->append( flow );
		flow->append( new HTMLBullet( font_stack.top()->pointSize(),
			listLevel, settings->fontBaseColor ) );
		break;

	    case Ordered:
		flow = new HTMLClueFlow( 0, 0, vc->getMaxWidth(), 0 );
		flow->setHAlign( HTMLClue::Right );
		vc->append( flow );
		switch ( listNumType )
		{
		    case LowRoman:
			item = toRoman( itemNumber, false );
			break;

		    case UpRoman:
			item = toRoman( itemNumber, true );
			break;

		    case LowAlpha:
			item += (char) ('a' + itemNumber - 1);
			break;

		    case UpAlpha:
			item += (char) ('A' + itemNumber - 1);
			break;

		    default:
			item.sprintf( "%2d", itemNumber );
		}
		item += ". ";
		tempStrings.append( item );
		flow->append(new HTMLText(tempStrings.getLast(),currentFont(),
			painter));
		break;

	    default:
		break;
	}

	vc = new HTMLClueV( 0, 0, _clue->getMaxWidth() - indentSize );
	c->append( vc );
	flow = new HTMLClueFlow( 0, 0, vc->getMaxWidth() );
	vc->append( flow );
	if ( listStack.count() > 0 )
		listStack.top()->itemNumber++;
    }
}

// <map             </map>
// <menu>           </menu>         partial
// <meta> 			    partial - only charset
void KHTMLWidget::parseM( HTMLClueV *_clue, const char *str )
{
	if (strncmp( str, "menu", 4 ) == 0)
	{
		closeAnchor();
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		listStack.push( new HTMLList( Menu ) );
		indent += INDENT_SIZE;
//		parseList( _clue, _clue->getMaxWidth(), Menu );
	}
	else if (strncmp( str, "/menu", 5 ) == 0)
	{
		if ( listStack.remove() )
		{
		    indent -= INDENT_SIZE;
		    flow = 0;
		}
	}
	else if ( strncmp( str, "map", 3 ) == 0 )
	{
		stringTok->tokenize( str + 4, " >" );
		while ( stringTok->hasMoreTokens() )
		{
			const char* token = stringTok->nextToken();
			if ( strncasecmp( token, "name=", 5 ) == 0)
			{
				QString mapurl = "#";
				mapurl += token+5;
				HTMLMap *map = new HTMLMap( this, mapurl );
				mapList.append( map );
				debugM( "Map: %s\n", (const char *)mapurl );
			}
		}
	}
	else if ( strncmp( str, "meta", 4 ) == 0 )
	{
                QString httpequiv;
                QString name;
		QString content;
		debugM("Parsing <META>: %s\n",str);
		stringTok->tokenize( str + 5, " >" );
		while ( stringTok->hasMoreTokens() )
	   	{
			const char* token = stringTok->nextToken();
			debugM("token: %s\n",token);
			if ( strncasecmp( token, "name=", 5 ) == 0)
				name=token+5;
			else if ( strncasecmp( token, "http-equiv=", 11 ) == 0)
				httpequiv=token+11;
			else if ( strncasecmp( token, "content=", 8 ) == 0)
				content=token+8;
                         
		}
		debugM( "Meta: name=%s httpequiv=%s content=%s\n",
                          (const char *)name,(const char *)httpequiv,(const char *)content );
		if ( !httpequiv.isEmpty() &&
		    strcasecmp(httpequiv,"content-type") == 0)
		{
			stringTok->tokenize( content, " >;" );
			while ( stringTok->hasMoreTokens() )
	   		{
				const char* token = stringTok->nextToken();
				debugM("token: %s\n",token);
				if ( strncasecmp( token, "charset=", 8 ) == 0)
				  	setCharset(token+8);
			}                         
		}
			 
	}
}

void KHTMLWidget::parseN( HTMLClueV *, const char * )
{
}

// <ol>             </ol>           partial
// <option
void KHTMLWidget::parseO( HTMLClueV *_clue, const char *str )
{
    if ( strncmp( str, "ol", 2 ) == 0 )
    {
	closeAnchor();
	if ( listStack.isEmpty() )
	    vspace_inserted = insertVSpace( _clue, vspace_inserted );
	ListNumType listNumType = Numeric;

	stringTok->tokenize( str + 3, " >" );
	while ( stringTok->hasMoreTokens() )
	{
		const char* token = stringTok->nextToken();
		if ( strncasecmp( token, "type=", 5 ) == 0 )
		{
			switch ( *(token+5) )
			{
				case 'i':
					listNumType = LowRoman;
					break;

				case 'I':
					listNumType = UpRoman;
					break;

				case 'a':
					listNumType = LowAlpha;
					break;

				case 'A':
					listNumType = UpAlpha;
					break;
			}
		}
	}

	listStack.push( new HTMLList( Ordered, listNumType ) );
	indent += INDENT_SIZE;
    }
    else if ( strncmp( str, "/ol", 3 ) == 0 )
    {
	if ( listStack.remove() )
	{
	    indent -= INDENT_SIZE;
	    if ( listStack.isEmpty() )
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
	}
    }
    else if ( strncmp( str, "option", 6 ) == 0 )
    {
	if ( !formSelect )
		return;

	QString value = "";
	bool selected = false;

	stringTok->tokenize( str + 7, " >" );
	while ( stringTok->hasMoreTokens() )
	{
		const char* token = stringTok->nextToken();
		if ( strncasecmp( token, "value=", 6 ) == 0 )
		{
			const char *p = token + 6;
			value = p;
		}
		else if ( strncasecmp( token, "selected", 8 ) == 0 )
		{
			selected = true;
		}
	}

	if ( inOption )
		formSelect->setText( formText );

	formSelect->addOption( value, selected );

	inOption = true;
	formText = "";
    }
    else if ( strncmp( str, "/option", 7 ) == 0 )
    {
	inOption = false;
    }
}

// <p
// <pre             </pre>
void KHTMLWidget::parseP( HTMLClueV *_clue, const char *str )
{
	if ( strncmp( str, "pre", 3 ) == 0 )
	{
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		flow = new HTMLClueH( 0, 0, _clue->getMaxWidth() );
		flow->setIndent( indent );
		_clue->append( flow );
		selectFont( settings->fixedFontFace, settings->fontBaseSize,
		    QFont::Normal, FALSE );
	}	
	else if ( strncmp( str, "/pre", 4 ) == 0 )
	{
		vspace_inserted = insertVSpace( _clue, false );
		popFont();
	}
	else if ( *str == 'p' && ( *(str+1) == ' ' || *(str+1) == '>' ) )
	{
		closeAnchor();
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
		HTMLClue::HAlign align = divAlign;

		stringTok->tokenize( str + 2, " >" );
		while ( stringTok->hasMoreTokens() )
		{
			const char* token = stringTok->nextToken();
			if ( strncasecmp( token, "align=", 6 ) == 0 )
			{
				if ( strcasecmp( token + 6, "center" ) == 0 )
					align = HTMLClue::HCenter;
				else if ( strcasecmp( token + 6, "right" ) == 0 )
					align = HTMLClue::Right;
				else if ( strcasecmp( token + 6, "left" ) == 0 )
					align = HTMLClue::Left;
			}
		}
		if ( flow == 0 )
		{
		    flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
		    flow->setIndent( indent );
		    _clue->append( flow );
		}
		flow->setHAlign( align );
	}
	else if ( *str == '/' && *(str+1) == 'p' &&
	    ( *(str+2) == ' ' || *(str+2) == '>' ) )
	{
	    vspace_inserted = insertVSpace( _clue, vspace_inserted );
	}
}

void KHTMLWidget::parseQ( HTMLClueV *, const char * )
{
}

void KHTMLWidget::parseR( HTMLClueV *, const char * )
{
}

// <s>              </s>
// <samp>           </samp>
// <select          </select>       partial
// <small>          </small>
// <strike>         </strike>
// <strong>         </strong>
// <sub>            </sub>          unimplemented
// <sup>            </sup>          unimplemented
void KHTMLWidget::parseS( HTMLClueV *_clue, const char *str )
{
	if ( strncmp(str, "samp", 4 ) == 0 )
	{
		selectFont( settings->fixedFontFace, settings->fontBaseSize,
		    QFont::Normal, FALSE );
	}
	else if ( strncmp(str, "/samp", 5 ) == 0)
	{
		popFont();
	}
	else if ( strncmp(str, "select", 6 ) == 0)
	{
		if ( !form )
			return;

		QString name = "";
		int size = 0;
		bool multi = false;

		stringTok->tokenize( str + 7, " >" );
		while ( stringTok->hasMoreTokens() )
		{
			const char* token = stringTok->nextToken();
			if ( strncasecmp( token, "name=", 5 ) == 0 )
			{
				const char *p = token + 5;
				name = p;
			}
			else if ( strncasecmp( token, "size=", 5 ) == 0 )
			{
				size = atoi( token + 5 );
			}
			else if ( strncasecmp( token, "multiple", 8 ) == 0 )
			{
				multi = true;
			}
		}

		formSelect = new HTMLSelect( this, name, size, multi );
		formSelect->setForm( form );
		form->addElement( formSelect );
		if ( flow == 0 )
		{
			flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
			flow->setIndent( indent );
			flow->setHAlign( divAlign );
			_clue->append( flow );
		}

		flow->append( formSelect );
	}
	else if ( strncmp(str, "/select", 7 ) == 0 )
	{
		if ( inOption )
			formSelect->setText( formText );

		formSelect = 0;
		inOption = false;
	}
	else if ( strncmp(str, "small", 5 ) == 0 )
	{
		selectFont( -1 );
	}
	else if ( strncmp(str, "/small", 6 ) == 0 )
	{
		popFont();
	}
	else if ( strncmp(str, "strong", 6 ) == 0 )
	{
		weight = QFont::Bold;
		selectFont();
	}
	else if ( strncmp(str, "/strong", 7 ) == 0 )
	{
		popFont();
	}
	else if ( strncmp( str, "strike", 6 ) == 0 )
	{
	    strikeOut = TRUE;
	    selectFont();
	}
	else if ( strncmp(str, "s", 1 ) == 0 )
	{
	    if ( str[1] == '>' || str[1] == ' ' )
	    {
		strikeOut = TRUE;
		selectFont();
	    }
	}
	else if ( strncmp(str, "/s", 2 ) == 0 )
	{
	    if ( str[2] == '>' || str[2] == ' ' ||
		strncmp( str+2, "trike", 5 ) == 0 )
	    {
		popFont();
	    }
	}
}

// <table           </table>        most
// <textarea        </textarea>
// <title>          </title>
// <tt>             </tt>
void KHTMLWidget::parseT( HTMLClueV *_clue, const char *str )
{
	if ( strncmp( str, "table", 5 ) == 0 )
	{
		closeAnchor();
		if ( !vspace_inserted || !flow )
		{
		    flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
		    flow->setIndent( indent );
		    flow->setHAlign( divAlign );
		    _clue->append( flow );
		}

		parseTable( flow, _clue->getMaxWidth(), str + 6 );

		flow = 0;
	}
	else if ( strncmp( str, "title", 5 ) == 0 )
	{
		title = "";
		inTitle = true;
	}
	else if ( strncmp( str, "/title", 6 ) == 0 )
	{
		emit setTitle( title.data() );
		inTitle = false;
	}
	else if ( strncmp( str, "textarea", 8 ) == 0 )
	{
		if ( !form )
			return;

		QString name = "";
		int rows = 5, cols = 40;

		stringTok->tokenize( str + 9, " >" );
		while ( stringTok->hasMoreTokens() )
		{
			const char* token = stringTok->nextToken();
			if ( strncasecmp( token, "name=", 5 ) == 0 )
			{
				const char *p = token + 5;
				if ( *p == '"' ) p++;
				name = p;
				if ( name[ name.length() - 1 ] == '"' )
					name.truncate( name.length() - 1 );
			}
			else if ( strncasecmp( token, "rows=", 5 ) == 0 )
			{
				rows = atoi( token + 5 );
			}
			else if ( strncasecmp( token, "cols=", 5 ) == 0 )
			{
				cols = atoi( token + 5 );
			}
		}

		formTextArea = new HTMLTextArea( this, name, rows, cols );
		formTextArea->setForm( form );
		form->addElement( formTextArea );
		if ( flow == 0 )
		{
			flow = new HTMLClueFlow( 0, 0, _clue->getMaxWidth() );
			flow->setIndent( indent );
			flow->setHAlign( divAlign );
			_clue->append( flow );
		}

		flow->append( formTextArea );

		formText = "";
		inTextArea = true;
	}
	else if ( strncmp( str, "/textarea", 9 ) == 0 )
	{
		if ( inTextArea )
			formTextArea->setText( formText );

		formTextArea = 0;
		inTextArea = false;
	}
	else if ( strncmp( str, "tt", 2 ) == 0 )
	{
		selectFont( settings->fixedFontFace, settings->fontBaseSize,
		    QFont::Normal, FALSE );
	}
	else if ( strncmp( str, "/tt", 3 ) == 0 )
	{
		popFont();
	}
}
 
// <u>              </u>
// <ul              </ul>
void KHTMLWidget::parseU( HTMLClueV *_clue, const char *str )
{
    if ( strncmp( str, "ul", 2 ) == 0 )
    {
	    closeAnchor();
	    if ( listStack.isEmpty() )
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
	    ListType type = Unordered;

	    stringTok->tokenize( str + 3, " >" );
	    while ( stringTok->hasMoreTokens() )
	    {
		    const char* token = stringTok->nextToken();
		    if ( strncasecmp( token, "plain", 5 ) == 0 )
			    type = UnorderedPlain;
	    }

	    listStack.push( new HTMLList( type ) );
	    indent += INDENT_SIZE;
	    flow = 0;
    }
    else if ( strncmp( str, "/ul", 3 ) == 0 )
    {
	if ( listStack.remove() )
	{
	    indent -= INDENT_SIZE;
	    if ( listStack.isEmpty() )
		vspace_inserted = insertVSpace( _clue, vspace_inserted );
	}
	flow = 0;
    }
    else if ( strncmp(str, "u", 1 ) == 0 )
    {
	if ( str[1] == '>' || str[1] == ' ' )
	{
	    underline = TRUE;
	    selectFont();
	}
    }
    else if ( strncmp( str, "/u", 2 ) == 0 )
    {
	    popFont();
    }
}

// <var>            </var>
void KHTMLWidget::parseV( HTMLClueV *, const char *str )
{
	if ( strncmp(str, "var", 3 ) == 0 )
	{
		italic = TRUE;
		selectFont();
	}
	else if ( strncmp( str, "/var", 4 ) == 0)
	{
		popFont();
	}
}

void KHTMLWidget::parseW( HTMLClueV *, const char *)
{
}

void KHTMLWidget::parseX( HTMLClueV *, const char * )
{
}

void KHTMLWidget::parseY( HTMLClueV *, const char * )
{
}

void KHTMLWidget::parseZ( HTMLClueV *, const char * )
{
}

const char* KHTMLWidget::parseCell( HTMLClue *_clue, const char *str )
{
    static const char *end[] = { "</cell>", 0 }; 
    
    HTMLClue::HAlign gridHAlign = HTMLClue::HCenter;// global align of all cells
    int cell_width = 90;

    stringTok->tokenize( str + 5, " >" );
    while ( stringTok->hasMoreTokens() )
    {
	const char* token = stringTok->nextToken();
	if ( strncasecmp( token, "width=", 6 ) == 0 )
	{
	    cell_width = atoi( token + 6 );
	}
	else if ( strncasecmp( token, "align=", 6 ) == 0 )
	{
	    if ( strncasecmp( token + 6, "left", 4 ) == 0 )
		gridHAlign = HTMLClue::Left;
	    else if ( strncasecmp( token + 6, "right", 5 ) == 0 )
		gridHAlign = HTMLClue::Right;
	}
    }
    
    HTMLClue::VAlign valign = HTMLClue::Top;
    HTMLClue::HAlign halign = gridHAlign;
    
    HTMLClueV *vc = new HTMLClueV( 0, 0, cell_width, 0 ); // fixed width
    _clue->append( vc );
    vc->setVAlign( valign );
    vc->setHAlign( halign );
	flow = 0;
    str = parseBody( vc, end );

    vc = new HTMLClueV( 0, 0, 10, 0 ); // fixed width
    _clue->append( vc );

    return str;
}

const char* KHTMLWidget::parseTable( HTMLClue *_clue, int _max_width,
	const char *attr )
{
    static const char *endth[] = { "</th", "<th", "<td", "<tr", "</table", 0 };
    static const char *endtd[] = { "</td", "<th", "<td", "<tr", "</table", 0 };
    static const char *endall[] = { "<td", "<tr", "<th", "</table", 0 };
    static const char *endcap[] = { "</caption>", "</table>", "<tr", "<td", "<th", 0 };    
    const char* str = 0;
    bool firstRow = true;
    bool tableTag = true;
    int padding = 1;
    int spacing = 2;
    int width = 0;
    int percent = 0;
    int border = 0;
    char has_cell = 0;
    HTMLClue::VAlign rowvalign = HTMLClue::VNone;
    HTMLClue::HAlign rowhalign = HTMLClue::HNone;
    HTMLClue::HAlign align = HTMLClue::HNone;
    HTMLClueV *caption = 0;
    HTMLTableCell *tmpCell = 0;
    HTMLClue::VAlign capAlign = HTMLClue::Bottom;
    HTMLClue::HAlign olddivalign = divAlign;
    HTMLClue *oldFlow = flow;
    QColor tableColor;
    QColor rowColor;

    stringTok->tokenize( attr, " >" );
    while ( stringTok->hasMoreTokens() )
    {
	const char* token = stringTok->nextToken();
	if ( strncasecmp( token, "cellpadding=", 12 ) == 0 )
	{
	    padding = atoi( token + 12 );
	}
	else if ( strncasecmp( token, "cellspacing=", 12 ) == 0 )
	{
	    spacing = atoi( token + 12 );
	}
	else if ( strncasecmp( token, "border", 6 ) == 0 )
	{
	    if ( *(token + 6) == '=' )
		border = atoi( token + 7 );
	    else
		border = 1;
	}
	else if ( strncasecmp( token, "width=", 6 ) == 0 )
	{
	    if ( strchr( token+6, '%' ) )
		percent = atoi( token + 6 );
	    else
		width = atoi( token + 6 );
	}
	else if (strncasecmp( token, "align=", 6 ) == 0)
	{
	    if ( strcasecmp( token + 6, "left" ) == 0 )
		align = HTMLClue::Left;
	    else if ( strcasecmp( token + 6, "right" ) == 0 )
		align = HTMLClue::Right;
	}
	else if ( strncasecmp( token, "bgcolor=", 8 ) == 0 )
	{
	    if ( *(token+8) != '#' && strlen( token+8 ) == 6 )
	    {
		QString col = "#";
		col += token+8;
		tableColor.setNamedColor( col );
	    }
	    else
		tableColor.setNamedColor( token+8 );
	    rowColor = tableColor;
	}
    }

    HTMLTable *table = new HTMLTable( 0, 0, _max_width, width, percent,
	 padding, spacing, border );
    //       _clue->append( table ); 
    // CC: Moved at the end since we might decide to discard the table while parsing...

    bool done = false;

    while ( !done && ht->hasMoreTokens() )
    {
	str = ht->nextToken();

	// Every tag starts with an escape character
	if ( str[0] == TAG_ESCAPE )
	{
	    str++;

	    tableTag = true;

	    do
	    {
		if ( strncmp( str, "<caption", 8 ) == 0 )
		{
		    stringTok->tokenize( str + 9, " >" );
		    while ( stringTok->hasMoreTokens() )
		    {
			const char* token = stringTok->nextToken();
			if ( strncasecmp( token, "align=", 6 ) == 0)
			{
			    if ( strncasecmp( token+6, "top", 3 ) == 0)
				capAlign = HTMLClue::Top;
			}
		    }
		    caption = new HTMLClueV( 0, 0, _clue->getMaxWidth() );
		    divAlign = HTMLClue::HCenter;
		    flow = 0;
		    str = parseBody( caption, endcap );
		    table->setCaption( caption, capAlign );
		    flow = 0;
		}

		if ( strncmp( str, "<tr", 3 ) == 0 )
		{
		    if ( !firstRow )
			table->endRow();
		    table->startRow();
		    firstRow = FALSE;
		    rowvalign = HTMLClue::VNone;
		    rowhalign = HTMLClue::HNone;
		    rowColor = tableColor;

		    stringTok->tokenize( str + 4, " >" );
		    while ( stringTok->hasMoreTokens() )
		    {
			const char* token = stringTok->nextToken();
			if ( strncasecmp( token, "valign=", 7 ) == 0)
			{
			    if ( strncasecmp( token+7, "top", 3 ) == 0)
				rowvalign = HTMLClue::Top;
			    else if ( strncasecmp( token+7, "bottom", 6 ) == 0)
				rowvalign = HTMLClue::Bottom;
			    else
				rowvalign = HTMLClue::VCenter;
			}
			else if ( strncasecmp( token, "align=", 6 ) == 0)
			{
			    if ( strncasecmp( token+6, "left", 4 ) == 0)
				rowhalign = HTMLClue::Left;
			    else if ( strncasecmp( token+6, "right", 5 ) == 0)
				rowhalign = HTMLClue::Right;
			    else
				rowhalign = HTMLClue::HCenter;
			}
			else if ( strncasecmp( token, "bgcolor=", 8 ) == 0 )
			{
			    if ( *(token+8) != '#' && strlen( token+8 ) == 6 )
			    {
				QString col = "#";
				col += token+8;
				rowColor.setNamedColor( col );
			    }
			    else
				rowColor.setNamedColor( token+8 );
			}
		    }

		    break;
		}
		else if (*str=='<' && *(str+1)=='t' && (*(str+2)=='d' ||
			 *(str+2)=='h'))
		//		else if ( strncmp( str, "<td", 3 ) == 0 ||
		//			strncmp( str, "<th", 3 ) == 0 )
		{
		    bool heading = false;

		    // if ( strncasecmp( str, "<th", 3 ) == 0 )
		    if (*(str+2)=='h')
			    heading = true;
		    // <tr> may not be specified for the first row
		    if ( firstRow )
		    {
			table->startRow();
			firstRow = FALSE;
		    }

		    int rowSpan = 1, colSpan = 1;
		    int width = _clue->getMaxWidth();
		    int percent = -1;
		    QColor bgcolor = rowColor;
		    HTMLClue::VAlign valign = (rowvalign == HTMLClue::VNone ?
					    HTMLClue::VCenter : rowvalign);

		    if ( heading )
			divAlign = (rowhalign == HTMLClue::HNone ? HTMLClue::HCenter :
			    rowhalign);
		    else
			divAlign = (rowhalign == HTMLClue::HNone ? HTMLClue::Left :
			    rowhalign);

		    stringTok->tokenize( str + 4, " >" );
		    while ( stringTok->hasMoreTokens() )
		    {
			const char* token = stringTok->nextToken();
			if ( strncasecmp( token, "rowspan=", 8 ) == 0)
			{
			    rowSpan = atoi( token+8 );
			    if ( rowSpan < 1 )
				rowSpan = 1;
			}
			else if ( strncasecmp( token, "colspan=", 8 ) == 0)
			{
			    colSpan = atoi( token+8 );
			    if ( colSpan < 1 )
				colSpan = 1;
			}
			else if ( strncasecmp( token, "valign=", 7 ) == 0)
			{
			    if ( strncasecmp( token+7, "top", 3 ) == 0)
				valign = HTMLClue::Top;
			    else if ( strncasecmp( token+7, "bottom", 6 ) == 0)
				valign = HTMLClue::Bottom;
			    else
				valign = HTMLClue::VCenter;
			}
			else if ( strncasecmp( token, "align=", 6 ) == 0)
			{
			    if ( strncasecmp( token+6, "center", 6 ) == 0)
				divAlign = HTMLClue::HCenter;
			    else if ( strncasecmp( token+6, "right", 5 ) == 0)
				divAlign = HTMLClue::Right;
			    else
				divAlign = HTMLClue::Left;
			}
			else if ( strncasecmp( token, "width=", 6 ) == 0 )
			{
			    if ( strchr( token + 6, '%' ) )
				percent = atoi( token + 6 );
			    else
			    {
				width = atoi( token + 6 );
				percent = 0;
			    }
			}
			else if ( strncasecmp( token, "bgcolor=", 8 ) == 0 )
			{
			    if ( *(token+8) != '#' && strlen( token+8 ) == 6 )
			    {
				QString col = "#";
				col += token+8;
				bgcolor.setNamedColor( col );
			    }
			    else
				bgcolor.setNamedColor( token+8 );
			}
		    }

		    HTMLTableCell *cell = new HTMLTableCell(0, 0, width, percent,
			rowSpan, colSpan, padding );
		    if ( bgcolor.isValid() )
			cell->setBGColor( bgcolor );
		    cell->setVAlign( valign );
		    table->addCell( cell );
		    has_cell = 1;
		    flow = 0;
		    if ( heading )
		    {
			weight = QFont::Bold;
			selectFont();
			str = parseBody( cell, endth );
			popFont();
		    }
		    else
			    str = parseBody( cell, endtd );
		    if ( str == 0 )
		    { 
			// CC: Close table description in case of a malformed table
			// before returning!
			if ( !firstRow )
			    table->endRow();
			table->endTable(); 
			delete table;
			divAlign = olddivalign;
			flow = oldFlow;
			delete tmpCell;
			return 0;
		    }
		}
		else if ( strncmp( str, "</table", 7 ) == 0 )
		{
		    closeAnchor();
		    done = true;
		    break;
		}
		else
		{
		    // catch-all for broken tables
      		  if ( *str != '<' || *(str+1) != '/' || *(str+2) != 't' ||
                      ( *(str+3)!='d' && *(str+3)!='h' && *(str+3)!='r' ) )
/*
		    if ( strncmp( str, "</td", 4 ) &&
			    strncmp( str, "</th", 4 ) &&
			    strncmp( str, "</tr", 4 ) )
*/
		    {
			flow = 0;
			if ( !tmpCell )
			{
			    tmpCell = new HTMLTableCell( 0, 0,
				_clue->getMaxWidth(), -1, 1, 1, padding );
			    if ( tableColor.isValid() )
				tmpCell->setBGColor( tableColor );
			}
    			parseOneToken( tmpCell, str );
			str = parseBody( tmpCell, endall );
			closeAnchor();
		    }
		    else
			tableTag = false;
		}
	    }
	    while ( str && tableTag );
	}
    }

    // Did we catch any illegal HTML
    if ( tmpCell )
    {
	// if no cells have been added then this must be a table with
	// one cell and no <tr, <td, etc.  I HATE people who abuse HTML
	// like this.
	if ( !has_cell )
	{
	    if ( firstRow )
	    {
		table->startRow();
		firstRow = FALSE;
	    }
	    table->addCell( tmpCell );
	    has_cell = 1;
	}
	else
	    delete tmpCell;
    }

    if (has_cell)
    {
	// CC: the ending "</table>" might be missing, so 
	// we close the table here... ;-) 
	if ( !firstRow )
	    table->endRow();
	table->endTable();
	if ( align != HTMLClue::Left && align != HTMLClue::Right )
	{
	    _clue->append ( table );
	}
	else
	{
	    HTMLClueAligned *aligned = new HTMLClueAligned (_clue, 0, 0,
		    _clue->getMaxWidth() );
	    aligned->setHAlign( align );
	    aligned->append( table );
	    _clue->append( aligned );
	}
    }
    else
    {
	// CC: last ressort -- remove tables that do not contain any cells
	delete table;
    }

    divAlign = olddivalign;
    flow = oldFlow;

    return str;
}

const char *KHTMLWidget::parseInput( const char *attr )
{
    enum InputType { CheckBox, Hidden, Radio, Reset, Submit, Text, Image,
	    Button, Password, Undefined };
    const char *p;
    HTMLInput *element = 0;
    InputType type = Text;
    QString name = "";
    QString value = "";
    bool checked = false;
    int size = 20;
    QList<JSEventHandler> *handlers = 0L;

    stringTok->tokenize( attr, " >" );
    while ( stringTok->hasMoreTokens() )
    {
	const char* token = stringTok->nextToken();
	if ( strncasecmp( token, "type=", 5 ) == 0 )
	{
	    p = token + 5;
	    if ( *p == '"' ) p++;
	    if ( strncasecmp( p, "checkbox", 8 ) == 0 )
		type = CheckBox;
	    else if ( strncasecmp( p, "password", 8 ) == 0 )
		type = Password;
	    else if ( strncasecmp( p, "hidden", 6 ) == 0 )
		type = Hidden;
	    else if ( strncasecmp( p, "radio", 5 ) == 0 )
		type = Radio;
	    else if ( strncasecmp( p, "reset", 5 ) == 0 )
		type = Reset;
	    else if ( strncasecmp( p, "submit", 5 ) == 0 )
		type = Submit;
	    else if ( strncasecmp( p, "button", 6 ) == 0 )
		type = Button;      
	    else if ( strncasecmp( p, "text", 5 ) == 0 )
		type = Text;
	    else if ( strncasecmp( p, "Image", 5 ) == 0 )
		type = Image;
	}
	else if ( strncasecmp( token, "name=", 5 ) == 0 )
	{
	    p = token + 5;
	    if ( *p == '"' ) p++;
	    name = p;
	    if ( name[ name.length() - 1 ] == '"' )
		name.truncate( name.length() - 1 );
	}
	else if ( strncasecmp( token, "value=", 6 ) == 0 )
	{
	    p = token + 6;
	    if ( *p == '"' ) p++;
	    value = p;
	    if ( value[ value.length() - 1 ] == '"' )
		value.truncate( value.length() - 1 );
	}
	else if ( strncasecmp( token, "size=", 5 ) == 0 )
	{
	    size = atoi( token + 5 );
	}
	else if ( strncasecmp( token, "checked", 7 ) == 0 )
	{
	    checked = true;
	}
	else if ( strncasecmp( token, "onClick=", 8 ) == 0 )
	{
	    QString code;
	    p = token + 8;
	    if ( *p == '"' ) p++;
	    code = p;
	    if ( code[ code.length() - 1 ] == '"' )
		code.truncate( value.length() - 1 );
	    if ( handlers == 0L )
	    {
		handlers = new QList<JSEventHandler>;
		handlers->setAutoDelete( TRUE );
	    }
	    handlers->append( new JSEventHandler( getJSEnvironment(), "onClick", code ) );
	}     
    }

    switch ( type )
    {
	case CheckBox:
	    element = new HTMLCheckBox( this, name, value, checked );
	    break;

	case Hidden:
	    element = new HTMLHidden( name, value );
	    break;

	case Radio:
	    element = new HTMLRadio( this, name, value, checked );
	    connect( element, SIGNAL(radioSelected(const char *, const char *)),
		form, SLOT(slotRadioSelected(const char *, const char *)));
	    connect( form, SIGNAL( radioSelected(const char*, const char *) ),
		element, SLOT(slotRadioSelected(const char *, const char *)));
	    break;

	case Reset:
	    element = new HTMLReset( this, value );
	    connect( element, SIGNAL( resetForm() ),
		    form, SLOT( slotReset() ) );
	    break;

	case Submit:
	    element = new HTMLSubmit( this, value );
	    connect( element, SIGNAL( submitForm() ),
		    form, SLOT( slotSubmit() ) );
	    break;

	case Button:
	    element = new HTMLButton( this, name, value, handlers );
	    break;

	case Text:
	case Password:
	    element = new HTMLTextInput( this, name, value, size,
		    (type == Password));
	    connect( element, SIGNAL( submitForm() ),
		    form, SLOT( slotSubmit() ) );
	    break;

	case Image:
	    break;

	case Undefined:
	    break;
    }

    if ( element )
    {
	element->setForm( form );

	// hidden elements are not added to the HTMLObject hierarchy
	if ( type == Hidden )
	{
	    form->addHidden( (HTMLHidden *)element );
	}
	else
	{
	    form->addElement( element );
	    flow->append( element );
	}
    }

    return 0;
}

void KHTMLWidget::slotScrollVert( int _val )
{
    if ( !isUpdatesEnabled() )
	return;

    if ( clue == 0 )
	return;

    if ( _val == y_offset )
	return;

    if ( abs( y_offset - _val ) < height() )
    {
	if ( bIsSelected )	
	{
	    bitBlt( this, 2, 2 + ( y_offset - _val ), this, 2, 2, width() - 4,
		height() - 4 );
	}
	else 
	{
	    bitBlt( this, 0, ( y_offset - _val ), this );
	}
    }

    bDrawBackground = true;

    if ( _val > y_offset)
    {
	int diff = _val - y_offset + 2;
	if ( diff > height() )
	    diff = height();
	y_offset = _val;
	// update region without clearing background
/*
	QPaintEvent *e = new QPaintEvent( QRect( 0, height() - diff,
	    width(), diff ) );
	QApplication::postEvent( this, e );
*/
	repaint( 0, height() - diff, width(), diff, false );
    }
    else
    {
	int diff = y_offset - _val + 2;
	if ( diff > height() )
	    diff = height();
	y_offset = _val;
	// update region without clearing background
/*
	QPaintEvent *e = new QPaintEvent( QRect(0, 0, width(), diff) );
	QApplication::postEvent( this, e );
*/
	repaint( 0, 0, width(), diff, false );
    }
}

void KHTMLWidget::slotScrollHorz( int _val )
{
	if ( !isUpdatesEnabled() )
		return;

	if ( clue == 0L )
		return;
    
	if ( bIsSelected )
	  bitBlt( this, 2 + x_offset - _val, 2, this, 2, 2, width() - 4, height() - 4 );
	else
	  bitBlt( this, x_offset - _val, 0, this );

	bDrawBackground = true;

	if ( _val > x_offset)
	{
		int diff = _val - x_offset + 2;
		x_offset = _val;
		repaint( width() - diff, 0, diff, height(), false );
	}
	else
	{
		int diff = x_offset - _val + 2;
		x_offset = _val;
		repaint( 0, 0, diff, height(), false );
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

bool KHTMLWidget::gotoAnchor( const char *_name )
{
    if ( clue == 0L )
	return FALSE;

    QPoint p( 0, 0 );
    
    HTMLAnchor *anchor = clue->findAnchor( _name, &p );
    if ( anchor == 0L )
	return FALSE;

    emit scrollVert( p.y() );

    // MRJ - I've turned this off for now.  It doesn't produce very nice
    // output.
//    emit scrollHorz( p.x() );

    return TRUE;
}

void KHTMLWidget::autoScrollY( int _delay, int _dy )
{
	if ( clue == 0 )
		return;

	if ( !autoScrollYTimer.isActive() )
	{
		connect( &autoScrollYTimer, SIGNAL(timeout()), SLOT( slotAutoScrollY() ) );
		autoScrollYDelay = _delay;
		autoScrollYTimer.start( _delay, true );
		autoScrollDY = _dy;
	}
}

void KHTMLWidget::stopAutoScrollY()
{
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
		mapFromGlobal( point );
		if ( point.y() > height() )
			point.setY( height() );
		else if ( point.y() < 0 )
			point.setY( 0 );
		selectPt2.setX( point.x() + x_offset - leftBorder );
		selectPt2.setY( point.y() + y_offset - topBorder );
		if ( selectPt2.y() < selectPt1.y() )
		{
			selectText( 0, selectPt2.x(), selectPt2.y(),
				selectPt1.x(), selectPt1.y() );
		}
		else
		{
			selectText( 0, selectPt1.x(), selectPt1.y(),
				selectPt2.x(), selectPt2.y() );
		}
	}
}

void KHTMLWidget::select( QPainter *_painter, bool _select )
{
    int tx = x_offset + leftBorder;
    int ty = -y_offset + topBorder;

    if ( clue == 0L )
	return;
    
    bool newPainter = FALSE;

    if ( clue == 0L )
	return;

    if ( _painter == 0L )
    {
	if ( painter == 0 )
	{
		painter = new QPainter();
		painter->begin( this );
		newPainter = TRUE;
	}

	clue->select( painter, _select, tx, ty );

	if ( newPainter )
	{
		painter->end();
		delete painter;
		painter = 0;
	}
    }
    else    
      clue->select( _painter, _select, tx, ty );
}

void KHTMLWidget::selectByURL( QPainter *_painter, const char *_url, bool _select )
{
    int tx = x_offset + leftBorder;
    int ty = -y_offset + topBorder;
    bool newPainter = FALSE;

    if ( clue == 0L )
	return;

    if ( _painter == 0L )
    {
	if ( painter == 0 )
	{
		painter = new QPainter();
		painter->begin( this );
		newPainter = TRUE;
	}

	clue->selectByURL( painter, _url, _select, tx, ty );

	if ( newPainter )
	{
		painter->end();
		delete painter;
		painter = 0;
	}
    }
    else    
      clue->selectByURL( _painter, _url, _select, tx, ty );
}

void KHTMLWidget::select( QPainter *_painter, QRegExp& _pattern, bool _select )
{
    int tx = x_offset + leftBorder;
    int ty = -y_offset + topBorder;
    bool newPainter = FALSE;

    if ( clue == 0L )
	return;

    if ( _painter == 0L )
    {
	if ( painter == 0 )
	{
		painter = new QPainter();
		painter->begin( this );
		newPainter = TRUE;
	}
    
	clue->select( painter, _pattern, _select, tx, ty );

	if ( newPainter )
	{
		painter->end();
		delete painter;
		painter = 0;
	}
    }
    else
	clue->select( _painter, _pattern, _select, tx, ty );
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
	  painter = 0L;
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
    for ( f = framesetList.first(); f != 0L; f = framesetList.next() )
    {
      KHTMLView *v = f->getSelectedFrame();
      if ( v )
	return v;
    }
  }
    
  return 0L;
}

JSEnvironment* KHTMLWidget::getJSEnvironment()
{
    if ( jsEnvironment == 0L )
	jsEnvironment = new JSEnvironment( this );
    
    return jsEnvironment;
}

JSWindowObject* KHTMLWidget::getJSWindowObject()
{
    return getJSEnvironment()->getJSWindowObject();
}
      
KHTMLWidget::~KHTMLWidget()
{
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
    delete settings;

    if ( stringTok )
	delete stringTok;

    font_stack.clear();
    
    char *str;
    for ( str = parsedURLs.first(); str; str = parsedURLs.next() )
	delete [] str;
    parsedURLs.clear();
    for ( str = parsedTargets.first(); str; str = parsedTargets.next() )
	delete [] str;
    parsedTargets.clear();

    if ( jsEnvironment )
	delete jsEnvironment;            
}

bool KHTMLWidget::setCharset(const char *name){

	KCharsets *charsets=kapp->getCharsets();
	if (!charsets->isDisplayable(name)){
		if (!charsets->isAvailable(name)){
			warning("Charset not available");
			return FALSE;
		}
		if (charsetConverter) delete charsetConverter;
		debugM("Initializing conversion from %s\n",name);
		charsetConverter=new KCharsetConverter(name
						,KCharsetConverter::AMP_SEQUENCES);
		if (!charsetConverter->ok()){
			warning("Couldn't initialize converter");
			delete charsetConverter;
			charsetConverter=0;
			return FALSE;
		}
	        name=charsetConverter->outputCharset();
	}
 	debugM("Setting charset to: %s\n",name);
        HTMLFont f( *font_stack.top());
	debugM("Original font: face: %s qtCharset: %i\n",QFont(f).family(),(int)QFont(f).charSet());
	f.setCharset(name);
	debugM("Changed font: face: %s qtCharset: %i\n",QFont(f).family(),(int)QFont(f).charSet());
        const HTMLFont *fp = pFontManager->getFont( f );
	debugM("Got font: %p\n",fp);
	debugM("Got font: face: %s qtCharset: %i\n",QFont(*fp).family(),(int)QFont(*fp).charSet());
        font_stack.push( fp );
	debugM("painter: %p\n",painter);
	debugM("Font stack top: %p\n",font_stack.top());
        if (painter) painter->setFont( *font_stack.top() );
	return TRUE;
}

#include "html.moc"

