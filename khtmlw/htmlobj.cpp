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
//-----------------------------------------------------------------------------
//
// KDE HTML Widget

#include <kurl.h>
#include <kapp.h>

#include "htmlobj.h"
#include "html.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>

#include <qfile.h>
#include <qimage.h>
#include <qdrawutl.h>
#include <qmovie.h>

#include "htmlobj.moc"

// This will be constructed once and NEVER deleted.
QList<HTMLCachedImage>* HTMLImage::pCache = 0L;
int HTMLObject::objCount = 0;

//-----------------------------------------------------------------------------

HTMLObject::HTMLObject()
{
    flags = 0;
    setFixedWidth( true );
    max_width = 0;
    width = 0;
    ascent = 0;
    descent = 0;
    percent = 0;
    objCount++;
    nextObj = 0;
    x = 0;
    y = 0;
}

HTMLObject* HTMLObject::checkPoint( int _x, int _y )
{
    if ( _x >= x && _x < x + width )
	if ( _y > y - ascent && _y < y + descent + 1 )
	    return this;
    
    return 0L;
}

void HTMLObject::select( QPainter *_painter, QRect &_rect, int _tx, int _ty )
{
    QRect r( x + _tx, y - ascent + _ty, width, ascent + descent );

    bool s = isSelected();
    bool s2;

    if ( _rect.contains( r ) )
	s2 = TRUE;
    else
	s2 = FALSE;

    if ( s != s2 )
	select( _painter, s2, _tx, _ty );
}

void HTMLObject::getSelected( QStrList &_list )
{
    if ( &_list == 0L )
    {
	debugM( "HTMLObject::getSelected(): _list is null\n" );
	return;
    }

    const char *u = getURL();

    if ( u != 0 && *u != '\0' && isSelected() )
    {
	char *s;
	
	for ( s = _list.first(); s != 0L; s = _list.next() )
	    if ( strcmp( u, s ) == 0 )
		return;
	
	_list.append( u );
    }
}

void HTMLObject::selectByURL( QPainter *_painter, const char *_url, bool _select, int _tx, int _ty )
{
    const char *u = getURL();

    if ( u == 0 )
	return;

    if ( strcmp( _url, u ) == 0 )
	select( _painter, _select, _tx, _ty );
}

void HTMLObject::select( QPainter *_painter, QRegExp& _pattern, bool _select, int _tx, int _ty )
{
    const char *u = getURL();

    if ( u == 0 || *u == '\0' )
	return;
    
    KURL ku( u );
    QString filename = ku.filename();
    
    if ( filename.find( _pattern ) != -1 )
	select( _painter, _select, _tx, _ty );
}

void HTMLObject::select( QPainter *_painter, bool _select, int _tx, int _ty )
{
    const char *u = getURL();
    if ( u == 0 || *u == '\0' || _select == isSelected() )
	return;
	
    setSelected( _select );

    _painter->eraseRect( x + _tx, y - ascent + _ty, width, ascent+descent );
    print( _painter, _tx, _ty );
}

bool HTMLObject::selectText( QPainter *_painter, int _x1, int _y1,
	int _x2, int _y2, int _tx, int _ty )
{
    bool selectIt = false;

    if ( _y1 >= y + descent || _y2 <= y - ascent )
	    selectIt = false;
    // start and end are on this line
    else if ( _y1 >= y - ascent && _y2 <= y + descent )
    {
	if ( _x1 > _x2 )
	{
	    int tmp = _x1;
	    _x1 = _x2;
	    _x2 = tmp;
	}
	if ( _x1 < x + width/2 && _x2 > x + width/2 && _x2 - _x1 > width/2 )
	    selectIt = true;
    }
    // starts on this line and extends past it.
    else if ( _y1 >= y - ascent && _y2 > y + descent )
    {
	if ( _x1 < x + width/2 )
	    selectIt = true;
    }
    // starts before this line and ends on it.
    else if ( _y1 < y - ascent && _y2 <= y + descent )
    {
	if ( _x2 > x + width/2 )
	    selectIt = true;
    }
    // starts before and ends after this line
    else if ( _y1 < y - ascent && _y2 > y + descent )
    {
	selectIt = true;
    }

    if ( selectIt != isSelected() )
    {
	setSelected( selectIt );
	_painter->eraseRect( x + _tx, y - ascent + _ty, width, ascent+descent );
	print( _painter, _tx, _ty );
    }

    return selectIt;
}

int HTMLObject::findPageBreak( int _y )
{
    if ( _y < y + descent )
	return ( y - ascent );

    return -1;
}

//-----------------------------------------------------------------------------

HTMLVSpace::HTMLVSpace( int _vspace, Clear c ) : HTMLObject()
{
    setNewline( true );
    ascent = _vspace;    
    descent = 0;
    width = 1;
    cl = c;
}

void HTMLVSpace::getSelectedText( QString &_str )
{
    if ( isSelected() )
	_str += '\n';
}

//-----------------------------------------------------------------------------

HTMLText::HTMLText(const char* _text, const HTMLFont *_font, QPainter *_painter
                   ,bool _autoDelete)
    : HTMLObject()
{
    autoDelete=_autoDelete;
    text = _text;
    font = _font;
    ascent = _painter->fontMetrics().ascent();
    descent = _painter->fontMetrics().descent()+1;
    width = _painter->fontMetrics().width( (const char*)_text );
    selStart = 0;
    selEnd = strlen( text );
}

HTMLText::HTMLText( const HTMLFont *_font, QPainter *_painter ) : HTMLObject()
{
    autoDelete=false;
    text = "";
    font = _font;
    ascent = _painter->fontMetrics().ascent();
    descent = _painter->fontMetrics().descent() + 1;
    width = 0;
    setSeparator( true );
    selStart = 0;
    selEnd = 0;
}

bool HTMLText::selectText( QPainter *_painter, int _x1, int _y1,
	int _x2, int _y2, int _tx, int _ty )
{
    bool selectIt = false;
    int oldSelStart = selStart;
    int oldSelEnd = selEnd;

    if ( _y1 >= y + descent || _y2 <= y - ascent )
	    selectIt = false;
    // start and end are on this line
    else if ( _y1 >= y - ascent && _y2 <= y + descent )
    {
	if ( _x1 > _x2 )
	{
	    int tmp = _x1;
	    _x1 = _x2;
	    _x2 = tmp;
	}
	if ( _x1 < x + width && _x2 > x )
	{
	    selectIt = true;
	    selStart = 0;
	    if ( _x1 > x )
		selStart = getCharIndex( _painter, _x1 - x );
	    selEnd = strlen( text );
	    if ( _x2 < x + width )
		selEnd = getCharIndex( _painter, _x2 - x );
	}
    }
    // starts on this line and extends past it.
    else if ( _y1 >= y - ascent && _y2 > y + descent )
    {
	if ( _x1 < x + width )
	{
	    selectIt = true;
	    selStart = 0;
	    if ( _x1 > x )
		selStart = getCharIndex( _painter, _x1 - x );
	    selEnd = strlen( text );
	}
    }
    // starts before this line and ends on it.
    else if ( _y1 < y - ascent && _y2 <= y + descent )
    {
	if ( _x2 > x )
	{
	    selectIt = true;
	    selStart = 0;
	    selEnd = strlen( text );
	    if ( _x2 < x + width )
		selEnd = getCharIndex( _painter, _x2 - x );
	}
    }
    // starts before and ends after this line
    else if ( _y1 < y - ascent && _y2 > y + descent )
    {
	selectIt = true;
	selStart = 0;
	selEnd = strlen( text );
    }

    if ( selectIt && selStart == selEnd )
	selectIt = false;

    if ( selectIt != isSelected() || oldSelStart != selStart ||
	oldSelEnd != selEnd )
    {
	setSelected( selectIt );
	_painter->eraseRect( x + _tx, y - ascent + _ty, width, ascent+descent );
	print( _painter, _tx, _ty );
    }

    return selectIt;
}

// get the index of the character at _xpos.
//
int HTMLText::getCharIndex( QPainter *_painter, int _xpos )
{
    int charWidth, index = 0, xp = 0, len = strlen( text );

    _painter->setFont( *font );

    while ( index < len )
    {
	charWidth = _painter->fontMetrics().width( text[ index ] );
	if ( xp + charWidth/2 >= _xpos )
	    break;
	xp += charWidth;
	index++;
    }

    return index;
}

void HTMLText::getSelectedText( QString &_str )
{
    if ( isSelected() )
    {
	if ( isNewline() )
	    _str += '\n';
	else
	{
	    int i = selStart;

	    // skip white space at the start of a line.
	    if ( !_str.isEmpty() && _str[ _str.length() - 1 ] == '\n' )
	    {
		while ( text[ i ] == ' ' )
		    i++;
	    }

	    while ( i < selEnd )
	    {
		_str += text[ i ];
		i++;
	    }
	}
    }
}

void HTMLText::recalcBaseSize( QPainter *_painter )
{
    const QFont &oldFont = _painter->font();
    _painter->setFont( *font );
    ascent = _painter->fontMetrics().ascent();
    descent = _painter->fontMetrics().descent() + 1;
    width = _painter->fontMetrics().width( (const char*)text );
    _painter->setFont( oldFont );
}

bool HTMLText::print( QPainter *_painter, int, int _y, int, int _height, int _tx, int _ty, bool toPrinter )
{
    if ( _y + _height < y - getAscent() || _y > y + getDescent() )
	return false;

    if ( toPrinter )
    {
	if ( _y + _height < y + descent )
	    return true;
	if ( isPrinted() )
	    return false;
	setPrinted( true );
    }

    print( _painter, _tx, _ty );

    return false;
}

void HTMLText::print( QPainter *_painter, int _tx, int _ty )
{
    if ( isNewline() )
	return;
 
    _painter->setPen( font->textColor() );
    _painter->setFont( *font );
    
    if ( isSelected() && _painter->device()->devType() != PDT_PRINTER )
    {
	_painter->drawText( x + _tx, y + _ty, text, selStart );
	int fillStart = _painter->fontMetrics().width( text, selStart );
	int fillEnd = _painter->fontMetrics().width( text, selEnd );
	_painter->fillRect( x + fillStart + _tx, y - ascent + _ty,
		fillEnd - fillStart, ascent + descent, kapp->selectColor );
	_painter->setPen( kapp->selectTextColor );
	_painter->drawText( x + _tx + fillStart, y + _ty, text + selStart,
		selEnd - selStart );
	_painter->setPen( font->textColor() );
	_painter->drawText( x + _tx + fillEnd, y + _ty, text + selEnd );
    }
    else
    {
	_painter->drawText( x + _tx, y + _ty, text );
    }
}

//-----------------------------------------------------------------------------

HTMLRule::HTMLRule( int _max_width, int _percent, int _size, bool _shade )
	: HTMLObject()
{
    if ( _size < 1 )
	_size = 1;
    ascent = 6 + _size;
    descent = 6;
    max_width = _max_width;
    width = _max_width;
    percent = _percent;
    shade = _shade;

    if ( percent > 0 )
    {
	width = max_width * percent / 100;
	setFixedWidth( false );
    }
}

int HTMLRule::calcMinWidth()
{
    if ( isFixedWidth() )
	return width;
    
    return 1;
}

void HTMLRule::setMaxWidth( int _max_width )
{
    if ( !isFixedWidth() )
    {
	max_width = _max_width;
	if ( percent > 0 )
	    width = _max_width * percent / 100;
	else
	    width = max_width;
    }
}

bool HTMLRule::print( QPainter *_painter, int, int _y, int, int _height, int _tx, int _ty, bool toPrinter )
{
    if ( _y + _height < y - getAscent() || _y > y + getDescent() )
	return false;

    if ( toPrinter )
    {
	if ( _y + _height <= y + descent )
		return true;
	if ( isPrinted() )
		return false;
	setPrinted( true );
    }

    print( _painter, _tx, _ty );

    return false;
}

void HTMLRule::print( QPainter *_painter, int _tx, int _ty )
{
    QColorGroup colorGrp( black, lightGray, white, darkGray, gray,
	    black, white );

    int xp = x + _tx, yp = y + _ty;

    if ( shade )
	qDrawShadeLine( _painter, xp, yp, xp + width, yp,
		colorGrp, TRUE, 1, ascent-7 );
    else
	_painter->fillRect( xp, yp, width, ascent-6, QBrush(black) );
}

//-----------------------------------------------------------------------------

HTMLBullet::HTMLBullet( int _height, int _level, const QColor &col )
	: HTMLObject(), color( col )
{
    ascent = _height;
    descent = 0;
    width = 14;
    level = _level;
}

bool HTMLBullet::print( QPainter *_painter, int, int _y, int, int _height, int _tx, int _ty, bool toPrinter )
{
    if ( _y + _height < y - getAscent() || _y > y + getDescent() )
	return false;

	if ( toPrinter )
	{
		if ( _y + _height <= y + descent )
			return true;
		if ( isPrinted() )
			return false;
		setPrinted( true );
	}

    print( _painter, _tx, _ty );

	return false;
}

void HTMLBullet::print( QPainter *_painter, int _tx, int _ty )
{
	int yp = y + _ty - 9;
	int xp = x + _tx + 2;

	_painter->setPen( QPen( color ) );

	switch ( level )
	{
		case 1:
			_painter->setBrush( QBrush( color ) );
			_painter->drawEllipse( xp, yp, 7, 7 );
			break;

		case 2:
			_painter->setBrush( QBrush() );
			_painter->drawEllipse( xp, yp, 7, 7 );
			break;

		case 3:
			_painter->setBrush( QBrush( color ) );
			_painter->drawRect( xp, yp, 7, 7 );
			break;

		default:
			_painter->setBrush( QBrush() );
			_painter->drawRect( xp, yp, 7, 7 );
	}
}

//-----------------------------------------------------------------------------

HTMLCachedImage::HTMLCachedImage( const char *_filename )
{
    pixmap = new QPixmap();
    pixmap->load( _filename );
    filename = _filename;
}

QPixmap* HTMLImage::findImage( const char *_filename )
{
	// Since this method is static, it is possible that pCache has not
	// yet been initialized. Better be careful.
	if( !pCache )
	{
		pCache = new QList<HTMLCachedImage>;
		return 0l;
	}

    HTMLCachedImage *img;
    for ( img = pCache->first(); img != 0L; img = pCache->next() )
    {
	if ( strcmp( _filename, img->getFileName() ) == 0 )
	    return img->getPixmap();
    }
    
    return 0L;
}

void HTMLImage::cacheImage( const char *_filename )
{
	// Since this method is static, it is possible that pCache has not
	// yet been initialized. Better be careful.
	if( !pCache )
		pCache = new QList<HTMLCachedImage>;

	pCache->append( new HTMLCachedImage( _filename ) );
}

HTMLImage::HTMLImage( KHTMLWidget *widget, const char *_filename,
	char *_url, char *_target,
	int _max_width, int _width, int _height, int _percent, int bdr )
    : QObject(), HTMLObject()
{
    if ( pCache == 0 )
	pCache = new QList<HTMLCachedImage>;

    pixmap = 0;
    movie = 0;
    overlay = 0;

    htmlWidget = widget;
    
    url = _url;
    target = _target;
    
    cached = TRUE;

    predefinedWidth = ( _width < 0 && !_percent ) ? false : true;
    predefinedHeight = _height < 0 ? false : true;

    border = bdr;
    percent = _percent;
    max_width = _max_width;
    ascent = _height;
    descent = 0;
    width = _width;

    absX = -1;
    absY = -1;
    
    if ( _filename[0] != '/' )
    {
	KURL kurl( _filename );
	if ( kurl.isMalformed() )
	    return;
	if ( strcmp( kurl.protocol(), "file" ) == 0 )
	{
	    pixmap = HTMLImage::findImage( kurl.path() );
	    if ( pixmap == 0 )
	    {
// We cannot use QMovie here, because we will load thumbnails which
// are not in GIF format!

//#ifdef USE_QMOVIE
//		if ( strstr( kurl.path(), ".gif" ) != 0 )
//		{
//		    movie = new QMovie( kurl.path(), 8192 );
//		    movie->connectUpdate( this, SLOT( movieUpdated( const QRect &) ) );
//		}
//		else
//		{
//#endif
		    pixmap = new QPixmap();
		    pixmap->load( kurl.path() );	    
//#ifdef USE_QMOVIE
//		}
//#endif
		cached = false;
	    }
	}
	else 
	{
	    debugM( "Requesting image" );
	    imageURL = _filename;
	    imageURL.detach();
	    synchron = TRUE;
	    htmlWidget->requestFile( this, imageURL.data() );
	    synchron = FALSE;
	}
    }
    else
    {
	pixmap = HTMLImage::findImage( _filename );
	if ( pixmap == 0 )
	{
	    pixmap = new QPixmap();
	    pixmap->load( _filename );
	    cached = false;
	}
    }

    // Is the image available ?
    if ( pixmap == 0 || pixmap->isNull() )
    {
	if ( !predefinedWidth && !percent )
	    width = 32;
	if ( !predefinedHeight && !percent )
	    ascent = 32;
    }
    else
	init();
}

void HTMLImage::init()
{
    if ( percent > 0 )
    {
	width = (int)max_width * (int)percent / 100;
	if ( !predefinedHeight )
	    ascent = pixmap->height() * width / pixmap->width();
	setFixedWidth( false );
    }
    else
    {
	if ( !predefinedWidth )
	    width = pixmap->width();

	if ( !predefinedHeight )
	    ascent = pixmap->height();

	if ( predefinedWidth && !predefinedHeight )
	    ascent = pixmap->height() * width / pixmap->width();
    }

    width += border*2;
    ascent += border*2;
}

void HTMLImage::setOverlay( const char *_ol )
{
    // overlays must be cached
    overlay = HTMLImage::findImage( _ol );
}

void HTMLImage::fileLoaded( const char *_filename )
{
#ifdef USE_QMOVIE
    FILE *f = fopen( _filename, "rb" );
    char buffer[ 4 ];
    int n = fread( buffer, 1, 3, f );
    if ( n >= 0 )
      buffer[ n ] = 0;
    else
      buffer[0] = 0;
    fclose( f );
    if ( strcmp( buffer, "GIF" ) == 0 )
    {
	movie = new QMovie( _filename, 8192 );
	movie->connectUpdate( this, SLOT( movieUpdated( const QRect &) ) );
    }
    else
    {
#endif
	pixmap = new QPixmap();
	pixmap->load( _filename );	    
	cached = false;

	if ( pixmap == 0 || pixmap->isNull() )
	    return;

	init();

	debugM( "Loaded Image: %s, %d, %d, %d\n",  imageURL.data(), predefinedWidth,
		    predefinedHeight, synchron );

	// We knew the size during the HTML parsing ?
	if ( predefinedWidth && predefinedHeight && !synchron )
	    htmlWidget->paintSingleObject( this );
	else if ( !synchron )
	{
	    htmlWidget->calcSize();
	    htmlWidget->calcAbsolutePos();
	    htmlWidget->scheduleUpdate( true );
	}
#ifdef USE_QMOVIE
    }
#endif
}

int HTMLImage::calcMinWidth()
{
    if ( percent > 0 )
	return 1;

    return width;
}

int HTMLImage::calcPreferredWidth()
{
    return width;
}

void HTMLImage::setMaxWidth( int _max_width )
{
    if ( percent > 0 )
	max_width = _max_width;

    if ( pixmap == 0 || pixmap->isNull() )
	return;

    if ( percent > 0 )
    {
	width = (int)max_width * (int)percent / 100;
	if ( !predefinedHeight )
	    ascent = pixmap->height() * width / pixmap->width() + border * 2;
	width += border * 2;
    }
}

void HTMLImage::select( QPainter *_painter, bool _select, int _tx, int _ty )
{
    const char *u = getURL();
    if ( u == 0 || *u == '\0' || _select == isSelected() )
	return;
	
    setSelected( _select );

//    _painter->eraseRect( x + _tx, y - ascent + _ty, width, ascent+descent );
    print( _painter, _tx, _ty );
}

bool HTMLImage::print( QPainter *_painter, int, int _y, int, int _height, int _tx, int _ty, bool toPrinter )
{
    if ( _y + _height < y - getAscent() || _y > y + getDescent() )
	return false;

    if ( toPrinter )
    {
	if ( _y + _height <= y + descent )
	    return true;
	if ( isPrinted() )
	    return false;
	setPrinted( true );
    }

    print( _painter, _tx, _ty );

    return false;
}

void HTMLImage::print( QPainter *_painter, int _tx, int _ty )
{
    const QPixmap *pixptr = pixmap;
    QRect rect( 0, 0, width - border*2, ascent - border*2 );

#ifdef USE_QMOVIE
    if ( movie && pixmap )
    {
	pixptr = &movie->framePixmap();
	rect = movie->getValidRect();
    }
#endif

    if ( !movie && pixmap )
    {
	if ( predefinedWidth )
	    rect.setWidth( pixmap->width() );

	if ( predefinedHeight )
	    rect.setHeight( pixmap->height() );
    }

    if ( pixptr == 0 || pixptr->isNull() )
    {
	if ( !predefinedWidth || !predefinedHeight )
	{
	    QColorGroup colorGrp( black, lightGray, white, darkGray, gray,
		    black, white );
	    qDrawShadePanel( _painter, x + _tx, y - ascent + _ty, width, ascent,
		    colorGrp, true, 1 );
	}
    }
    else
    {
	QPixmap pm = *pixptr;

	if ( isSelected() )
	{
	    QPainter p( &pm );
//	    p.setRasterOp( NotEraseROP );
//	    p.fillRect( 0, 0, pm.width(), pm.height(), blue );
	    QBrush b( kapp->selectColor, Dense4Pattern );
	    p.fillRect( 0, 0, pm.width(), pm.height(), b );
	}

	if ( (width - border*2 != pixptr->width() ||
	    ascent - border*2 != pixptr->height() ) &&
	    pixptr->width() != 0 && pixptr->height() != 0 )
	{
	    QWMatrix matrix;
	    matrix.scale( (float)(width-border*2)/pixptr->width(),
		    (float)(ascent-border*2)/pixptr->height() );
	    QPixmap tp = pm.xForm( matrix );
	    rect.setRight( rect.width() * (width-border*2)/pixptr->width() );
	    rect.setBottom( rect.height() * (ascent-border*2)/pixptr->height());
	    _painter->drawPixmap( QPoint( x + _tx + border,
		    y - ascent + _ty + border ), tp, rect );
	}
	else
	    _painter->drawPixmap( QPoint( x + _tx + border,
		    y - ascent + _ty + border ), pm, rect );
	
	if ( overlay )
	    _painter->drawPixmap( QPoint( x + _tx + border,
		    y - ascent + _ty + border ), *overlay, rect );
    }

    if ( border )
    {
	QPen pen( borderColor );
	_painter->setPen( pen );
	QBrush brush;
	_painter->setBrush( brush );	// null brush
	for ( int i = 0; i < border; i++ )
	    _painter->drawRect( x + _tx + i, y - ascent + _ty + i,
		width - i*2, ascent - i*2 );
    }
}

void HTMLImage::calcAbsolutePos( int _x, int _y )
{
    absX = _x + x;
    absY = _y + y - ascent;
}

int HTMLImage::getAbsX()
{
    return absX;
}

int HTMLImage::getAbsY()
{
    return absY;
}

void HTMLImage::movieUpdated( const QRect & )
{
#ifdef USE_QMOVIE
    if ( !pixmap )
    {
	pixmap = new QPixmap;
	*pixmap = movie->framePixmap();
	init();
	if ( !predefinedWidth || !predefinedHeight )
	{
	    htmlWidget->calcSize();
	    htmlWidget->calcAbsolutePos();
	    htmlWidget->scheduleUpdate( true );
	    return;
	}
    }
    htmlWidget->paintSingleObject( this );
#endif
}

HTMLImage::~HTMLImage()
{
    if ( !imageURL.isEmpty() && !pixmap )
	htmlWidget->cancelRequestFile( this );

    if ( pixmap && !cached )
	delete pixmap;
#ifdef USE_QMOVIE
    if ( movie )
    {
	movie->disconnectUpdate( this, 0 );
    	delete movie;
    }
#endif
}

//----------------------------------------------------------------------------

HTMLArea::HTMLArea( const QPointArray &_points, const char *_url,
	const char *_target )
{
	region = QRegion( _points );
	url = _url;
	target = _target;
}

HTMLArea::HTMLArea( const QRect &_rect, const char *_url, const char *_target )
{
	region = QRegion( _rect );
	url = _url;
	target = _target;
}

HTMLArea::HTMLArea( int _x, int _y, int _r, const char *_url,
	const char *_target )
{
	QRect r( _x - _r, _y - _r, _r * 2, _y * 2 );

	region = QRegion( r, QRegion::Ellipse );
	url = _url;
	target = _target;
}

//----------------------------------------------------------------------------

HTMLMap::HTMLMap( KHTMLWidget *w, const char *_url )
	: HTMLObject()
{
    areas.setAutoDelete( true );
    mapurl = _url;
    htmlWidget = w;

    if ( strchr ( mapurl, ':' ) )
	htmlWidget->requestFile( this, mapurl );
}

HTMLMap::~HTMLMap()
{
    if ( strchr ( mapurl, ':' ) )
	htmlWidget->cancelRequestFile( this );
}

// The external map has been downloaded
void HTMLMap::fileLoaded( const char *_filename )
{
    QFile file( _filename );
    QString buffer;
    QString href;
    QString coords;
    HTMLArea::Shape shape = HTMLArea::Rect;
    char ch;

    if ( file.open( IO_ReadOnly ) )
    {
	while ( !file.atEnd() )
	{
	    // read in a line
	    buffer.data()[0] = '\0';
	    do
	    {
		ch = file.getch();
		if ( ch != '\n' && ch != -1 );
		    buffer += ch;
	    }
	    while ( ch != '\n' && ch != -1 );

	    // comment?
	    if ( buffer[0] == '#' )
		continue;

	    StringTokenizer st;
	    st.tokenize( buffer, " " );

	    // get shape
	    const char *p = st.nextToken();

	    if ( strncasecmp( p, "rect", 4 ) == 0 )
		shape = HTMLArea::Rect;
	    else if ( strncasecmp( p, "poly", 4 ) == 0 )
		shape = HTMLArea::Poly;
	    else if ( strncasecmp( p, "circle", 6 ) == 0 )
		shape = HTMLArea::Circle;

	    // get url
	    p = st.nextToken();

	    if ( *p == '#' )
	    {// reference
		KURL u( htmlWidget->getDocumentURL() );
		u.setReference( p + 1 );
		href = u.url();
	    }
	    else if ( strchr( p, ':' ) )
	    {// full URL
		href =  p;
	    }
	    else
	    {// relative URL
		KURL u2( htmlWidget->getBaseURL(), p );
		href = u2.url();
	    }

	    // read coords and create object
	    HTMLArea *area = 0;

	    switch ( shape )
	    {
		case HTMLArea::Rect:
		    {
			p = st.nextToken();
			int x1, y1, x2, y2;
			sscanf( p, "%d,%d,%d,%d", &x1, &y1, &x2, &y2 );
			QRect rect( x1, y1, x2-x1, y2-y1 );
			area = new HTMLArea( rect, href, "" );
			printf( "Area Rect %d, %d, %d, %d\n", x1, y1, x2, y2 );
		    }
		    break;

		case HTMLArea::Circle:
		    {
			p = st.nextToken();
			int xc, yc, rc;
			sscanf( p, "%d,%d,%d", &xc, &yc, &rc );
			area = new HTMLArea( xc, yc, rc, href, "" );
			printf( "Area Circle %d, %d, %d\n", xc, yc, rc );
		    }
		    break;

		case HTMLArea::Poly:
		    {
			printf( "Area Poly " );
			int count = 0, x, y;
			QPointArray parray;
			while ( st.hasMoreTokens() )
			{
			    p = st.nextToken();
			    sscanf( p, "%d,%d", &x, &y );
			    parray.resize( count + 1 );
			    parray.setPoint( count, x, y );
			    printf( "%d, %d  ", x, y );
			    count++;
			}
			printf( "\n" );
			if ( count > 2 )
			    area = new HTMLArea( parray, href, "" );
		    }
		    break;
	    }

	    if ( area )
		addArea( area );
	}
    }
}

const HTMLArea *HTMLMap::containsPoint( int _x, int _y )
{
    const HTMLArea *area;

    for ( area = areas.first(); area != 0; area = areas.next() )
    {
	if ( area->contains( QPoint( _x, _y ) ) )
	    return area;
    }

    return 0;
}

//----------------------------------------------------------------------------

HTMLImageMap::HTMLImageMap( KHTMLWidget *widget, const char *_filename,
	    char *_url, char *_target,
	    int _max_width, int _width, int _height, int _percent, int bdr )
    : HTMLImage( widget, _filename, _url, _target, _max_width, _width,
	    _height, _percent, bdr )
{
    type = ClientSide;
    serverurl = _url;
    serverurl.detach();
}

HTMLObject* HTMLImageMap::checkPoint( int _x, int _y )
{
    if ( _x >= x && _x < x + width )
    {
	if ( _y > y - ascent && _y < y + descent + 1 )
	{
	    if ( type == ClientSide )
	    {
		HTMLMap *map = htmlWidget->getMap( mapurl );
		if ( map )
		{
		    const HTMLArea *area = map->containsPoint( _x - x,
			_y - ( y -ascent ) );
		    if ( area )
		    {
			strcpy( url, area->getURL() );
			strcpy( target, area->getTarget() );
			return this;
		    }
		    else
		    {
			*url = '\0';
			*target = '\0';
		    }
		}
	    }
	    else
	    {
		QString coords;
		coords.sprintf( "?%d,%d", _x - x, _y - ( y -ascent ) );
		strcpy( url, serverurl );
		strcat( url, coords );
		return this;
	    }
	}
    }

    return 0;
}

//-----------------------------------------------------------------------------

HTMLAnchor* HTMLAnchor::findAnchor( const char *_name, QPoint *_p )
{
    if ( strcmp( _name, name ) == 0 )
    {
	_p->setX( _p->x() + x );
	_p->setY( _p->y() + y );
	return this;
    }
    
    return 0L;
}

void HTMLAnchor::setMaxAscent( int _a )
{
    y -= _a;
}

//-----------------------------------------------------------------------------

