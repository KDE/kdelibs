//
// KDE HTML Widget
//

#include <kurl.h>

#include "htmlobj.h"
#include "ampseq.h"
#include "html.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <qimage.h>
#include <qdrawutl.h>

// This will be constructed once and NEVER deleted.
QList<HTMLCachedImage>* HTMLImage::pCache = NULL;
int HTMLObject::objCount = 0;

//-----------------------------------------------------------------------------

HTMLObject::HTMLObject()
{
	flags = 0;
	setFixedWidth( true );
	setPrinting( true );
    max_width = 0;
    width = 0;
    ascent = 0;
    descent = 0;
	percent = 0;
	objCount++;
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

  //    {
  //      printf("Comparing Image %i %i %i %i\n",_rect.left(), _rect.top(), _rect.right(), _rect.bottom() );
  //      printf("          with %i %i %i %i\n",r.left(), r.top(), r.right(), r.bottom() );    
  //    }
  
  if ( _rect.contains( r ) )
    s2 = TRUE;
  else
    s2 = FALSE;
  
  if ( s != s2 )
    select( _painter, s2, _tx, _ty );
}

void HTMLObject::getSelected( QStrList &_list )
{
    if ( isSelected() && url[0] != 0 )
    {
	char *s;
	
    for ( s = _list.first(); s != 0L; s = _list.next() )
	if ( strcmp( url, s ) == 0 )
	    return;
	
	_list.append( url.data() );
    }
}

void HTMLObject::selectByURL( QPainter *_painter, const char *_url, bool _select, int _tx, int _ty )
{
    if ( _url == url )
    {
	setSelected( _select );

	_painter->eraseRect( x + _tx, y - ascent + _ty, width, ascent+descent );
	print( _painter, _tx, _ty );
    }
}

void HTMLObject::select( QPainter *_painter, QRegExp& _pattern, bool _select, int _tx, int _ty )
{
    if ( url.isEmpty() )
	return;
    
    KURL u( url.data() );
    QString filename = u.filename();
    
    if ( filename.find( _pattern ) != -1 )
    {
	setSelected( _select );

	_painter->eraseRect( x + _tx, y - ascent + _ty, width, ascent+descent );
	print( _painter, _tx, _ty );
    }	
}

void HTMLObject::select( QPainter *_painter, bool _select, int _tx, int _ty )
{
    if ( _select == isSelected() || url.isEmpty() )
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

HTMLVSpace::HTMLVSpace( int _vspace ) : HTMLObject()
{
    ascent = _vspace;    
    descent = 0;
    width = 1;
}

//-----------------------------------------------------------------------------

HTMLText::HTMLText( const char* _text, const HTMLFont *_font, QPainter *_painter,
		    const char *_url, const char *_target ) : HTMLObject()
{
    text = _text;
    font = _font;
    ascent = _painter->fontMetrics().ascent();
    descent = _painter->fontMetrics().descent()+1;
    width = _painter->fontMetrics().width( (const char*)_text );
    url = _url;
    url.detach();
    target = _target;
    target.detach();
	selStart = 0;
	selEnd = strlen( text );
}

HTMLText::HTMLText( const HTMLFont *_font, QPainter *_painter ) : HTMLObject()
{
    text = " ";
    font = _font;
    ascent = _painter->fontMetrics().ascent();
    descent = _painter->fontMetrics().descent();
    width = _painter->fontMetrics().width( (const char*)text );
    setSeparator( true );
	selStart = 0;
	selEnd = strlen( text );
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

	do
	{
		charWidth = _painter->fontMetrics().width( text[ index ] );
		if ( xp + charWidth/2 >= _xpos )
			break;
		xp += charWidth;
		index++;
	}
	while ( index < len );

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
			for ( int i = selStart; i < selEnd; i++ )
				_str += text[ i ];
		}
	}
}

void HTMLText::recalcBaseSize( QPainter *_painter )
{
	const QFont &oldFont = _painter->font();
	_painter->setFont( *font );
	ascent = _painter->fontMetrics().ascent();
	descent = _painter->fontMetrics().descent();
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
	if ( !isPrinting() )
		return;
 
	_painter->setPen( font->textColor() );
    _painter->setFont( *font );
    
    if ( isSelected() && _painter->device()->devType() != PDT_PRINTER )
    {
		if ( !url.isEmpty() )	
			_painter->setPen( font->linkColor() );
		_painter->drawText( x + _tx, y + _ty, text, selStart );
		int fillStart = _painter->fontMetrics().width( text, selStart );
		int fillEnd = _painter->fontMetrics().width( text, selEnd );
		_painter->fillRect( x + fillStart + _tx, y - ascent + _ty,
			fillEnd - fillStart, ascent + descent, black );
		_painter->setPen( white );
		_painter->drawText( x + _tx + fillStart, y + _ty, text + selStart,
			selEnd - selStart );
		_painter->setPen( font->textColor() );
		if ( !url.isEmpty() )	
			_painter->setPen( font->linkColor() );
		_painter->drawText( x + _tx + fillEnd, y + _ty, text + selEnd );
    }
    else
    {
		if ( !url.isEmpty() )	
			_painter->setPen( font->linkColor() );

#if QT_VERSION < 130  // remove when Qt-1.3 is released
		if ( _painter->device()->devType() == PDT_PRINTER )
		{
			QString escText = "";
			const char *p = text;

			while ( *p != '\0' )
			{
				if ( *p == '(' || *p == ')' || *p == '\\' )
					escText += '\\';
				escText += *p++;
			}
			_painter->drawText( x + _tx, y + _ty, escText );
		}
		else
#endif
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
	if ( !isPrinting() )
		return;

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
    if ( !isPrinting() )
	return;

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
	const char* _url, const char *_target,
	int _max_width, int _width, int _height, int _percent )
{
	if ( pCache == NULL )
		pCache = new QList<HTMLCachedImage>;

    pixmap = 0L;

    htmlWidget = widget;
    
    url = _url;
    url.detach();
    target = _target;
    target.detach();
    
    cached = TRUE;

    // An assumption
    predefinedWidth = TRUE;
    predefinedHeight = TRUE;
    
    percent = _percent;
    max_width = _max_width;
    ascent = _height;
    descent = 0;
    width = _width;
    
    if ( _filename[0] != '/' )
    {
	KURL kurl( _filename );
	if ( kurl.isMalformed() )
	    return;
	if ( strcmp( kurl.protocol(), "file" ) == 0 )
	{
	    pixmap = HTMLImage::findImage( kurl.path() );
	    if ( pixmap == 0L )
	    {
		pixmap = new QPixmap();
		pixmap->load( kurl.path() );	    
		cached = false;
	    }
	}
	else 
	{
	    imageURL = _filename;
	    imageURL.detach();
	    synchron = TRUE;
	    htmlWidget->requestFile( this, imageURL.data() );
	    synchron = FALSE;
	    // If we have to wait for the image...
	    if ( pixmap == 0L )
	    {
		// Make sure that we dont get broken values here
		if ( ascent == -1 )
		{
		    predefinedHeight = FALSE;
		    ascent = 32;
		}
		if ( width == -1 )
		{
		    predefinedWidth = FALSE;
		    width = 32;
		}
	    }
	    return;
	}
    }
    else
    {
	pixmap = HTMLImage::findImage( _filename );
	if ( pixmap == 0L )
	{
	    pixmap = new QPixmap();
	    pixmap->load( _filename );
	    cached = false;
	}
    }
    
    // Is the image available ?
    if ( pixmap == NULL || pixmap->isNull() )
	return;

    init();
}

void HTMLImage::init()
{
    if ( percent > 0 )
    {
	width = max_width * percent / 100;
	ascent = pixmap->height() * width / pixmap->width();
	setFixedWidth( false );
    }
    else if ( width != -1 )
    {
	if ( ascent == -1 )
	    ascent = pixmap->height() * width / pixmap->width();
    }
    else
	width = pixmap->width();
    
    if ( ascent == -1 )
    	ascent = pixmap->height();
}

void HTMLImage::fileLoaded( const char *_filename )
{
    pixmap = new QPixmap();
    pixmap->load( _filename );	    
    cached = false;

    if ( pixmap == NULL || pixmap->isNull() )
	return;

    init();

    // We knew the size during the HTML parsing ?
    if ( predefinedWidth && predefinedHeight && !synchron )
	htmlWidget->paintSingleObject( this );
    else if ( !synchron )
    {
	if ( !predefinedWidth )
	    width = -1;
	if ( !predefinedHeight )
	    ascent = -1;
	
	htmlWidget->parseAfterLastImage();
    }
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
	{
		max_width = _max_width;
		width = max_width * percent / 100;
		ascent = pixmap->height() * width / pixmap->width();
	}
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
    if ( pixmap == 0L || pixmap->isNull() )
	{
		QColorGroup colorGrp( black, lightGray, white, darkGray, gray,
			black, white );
		qDrawShadePanel( _painter, x + _tx, y - ascent + _ty, width, ascent,
			colorGrp, true, 1 );
	}
	else if ( (width != pixmap->width() || ascent != pixmap->height() ) &&
		pixmap->width() != 0 && pixmap->height() != 0 )
	{
		QWMatrix matrix;
		matrix.scale( (float)width/pixmap->width(),
			(float)ascent/pixmap->height() );
		QPixmap tp = pixmap->xForm( matrix );
	    _painter->drawPixmap( QPoint( x + _tx, y - ascent + _ty ), tp );
	}
	else
	    _painter->drawPixmap( QPoint( x + _tx, y - ascent + _ty ), *pixmap );
}

HTMLImage::~HTMLImage()
{
    if ( pixmap && !cached )
	delete pixmap;
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
	url = _url;
	htmlWidget = w;

	if ( url.contains( ':' ) )
		htmlWidget->requestFile( this, url );
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

			StringTokenizer st( buffer, " " );

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
		const char* _url, const char *_target,
		int _max_width, int _width, int _height, int _percent )
	: HTMLImage( widget, _filename, _url, _target, _max_width, _width,
		_height, _percent )
{
	type = ClientSide;
	serverurl = _url;
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
						url = area->getURL();
						target = area->getTarget();
						return this;
					}
					else
					{
						url.resize( 0 );
						target.resize( 0 );
					}
				}
			}
			else
			{
				QString coords;
				coords.sprintf( "?%d,%d", _x - x, _y - ( y -ascent ) );
				url = serverurl + coords;
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

