//-----------------------------------------------------------------------------
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

//#define CLUE_DEBUG

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
    if ( url.isNull() )
	return;
    if ( url[0] == 0 )
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
    if ( _select == isSelected() || url.data() == 0 || url[0] == 0 )
	return;
	
    setSelected( _select );

    _painter->eraseRect( x + _tx, y - ascent + _ty, width, ascent+descent );
    print( _painter, _tx, _ty );
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
    descent = _painter->fontMetrics().descent();
    width = _painter->fontMetrics().width( (const char*)_text );
    url = _url;
    url.detach();
    target = _target;
    target.detach();
}

HTMLText::HTMLText( const HTMLFont *_font, QPainter *_painter ) : HTMLObject()
{
    text = " ";
    font = _font;
    ascent = _painter->fontMetrics().ascent();
    descent = _painter->fontMetrics().descent();
    width = _painter->fontMetrics().width( (const char*)text );
    setSeparator( true );
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
 
    const QPen & pen = _painter->pen();
    
    _painter->setFont( *font );
    
    if ( isSelected() )
    {
	_painter->fillRect( x + _tx, y - ascent + _ty, width, ascent + descent, black );
	_painter->setPen( white );
    }
    else
    {
	_painter->setPen( font->textColor() );
	if ( !url.isNull() && url.data()[0] != 0 )	
	    _painter->setPen( font->linkColor() );
    }

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

    if ( isSelected() )
	_painter->setPen( pen );
}

//-----------------------------------------------------------------------------

HTMLRule::HTMLRule( int _max_width, int _width, int _percent, int _size,
	HAlign _align, bool _shade )
	: HTMLObject()
{
	if ( _size < 1 )
		_size = 1;
    ascent = 6 + _size;
    descent = 6;
    max_width = _max_width;
	width = _max_width;
	length = _width;
	percent = _percent;
	align = _align;
	shade = _shade;

	if ( percent )
	{
		length = max_width * percent / 100;
		setFixedWidth( false );
	}
}

void HTMLRule::setMaxWidth( int _max_width )
{
	max_width = _max_width;
	width = _max_width;

	if ( percent )
		length = max_width * percent / 100;
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

	int xp, yp = y + _ty;

	switch ( align )
	{
		case Left:
			xp = x + _tx;
			break;

		case Right:
			xp = x + _tx + max_width - length;
			break;

		default:
			xp = x + _tx + ((max_width-length)>>1);
	}
 
 	if ( shade )
		qDrawShadeLine( _painter, xp, yp, xp + length, yp,
			colorGrp, TRUE, 1, ascent-7 );
	else
		_painter->fillRect( xp, yp, length, ascent-6, QBrush(black) );
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

HTMLImage::HTMLImage( KHTMLWidget *widget, const char *_filename, const char* _url, const char *_target,
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
	    htmlWidget->requestImage( this, imageURL.data() );
	    synchron = FALSE;
	    // If we have to wait for the image...
	    if ( pixmap == 0L )
	    {
		// Make shure that we dont get broken values here
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
    if ( percent )
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

void HTMLImage::imageLoaded( const char *_filename )
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
	if ( percent )
		return 1;

	return width;
}

int HTMLImage::calcPreferredWidth()
{
	return width;
}

void HTMLImage::setMaxWidth( int _max_width )
{
	if ( percent )
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

HTMLTableCell::HTMLTableCell( int _x, int _y, int _max_width, int _percent,
	int rs, int cs, int pad )
	 : HTMLClueV( _x, _y, _max_width, _percent )
{
	rspan = rs;
	cspan = cs;
	padding = pad;
}

void HTMLTableCell::setMaxWidth( int _max_width )
{
	HTMLObject *obj;

	// We allow fixed width cells to be resized in a table
	max_width = _max_width;

    if ( percent )
		width = _max_width * percent / 100;
	else if ( !isFixedWidth() )
		width = max_width;

	for ( obj = list.first(); obj != 0L; obj = list.next() )
	{
		obj->setMaxWidth( width );
	}
}

bool HTMLTableCell::print( QPainter *_painter, int _x, int _y, int _width,
	int _height, int _tx, int _ty, bool toPrinter )
{
	if ( _y + _height < y - getAscent() || _y > y )
		return false;
    
	if ( !isPrinting() )
		return false;
    
 	if ( bg.isValid() )
	{
		int top = _y - ( y - getAscent() );
		int bottom = top + _height;
		if ( top < -padding )
			top = -padding;
		if ( bottom > getAscent() + padding )
			bottom = getAscent() + padding;

 		QBrush brush( bg );
		_painter->fillRect( _tx + x - padding, _ty + y - ascent + top,
			getMaxWidth() + padding * 2, bottom - top, brush );
	}

	return HTMLClueV::print( _painter, _x, _y, _width, _height, _tx, _ty, toPrinter );
}

//-----------------------------------------------------------------------------

HTMLTable::HTMLTable( int _x, int _y, int _max_width, int _width, int _percent,
	int _padding, int _spacing, int _border ) : HTMLObject()
{
    x = _x;
    y = _y;
    max_width = _max_width;
	width = _width;
	percent = _percent;
	padding = _padding;
	spacing = _spacing;
	border  = _border;
	caption = NULL;

	setFixedWidth( false );
	row = 0;
	col = 0;

	totalCols = 1;	// this should be expanded to the maximum number of cols
					// by the first row parsed
	totalRows = 5;	// allocate five rows initially

	cells = new HTMLTableCell ** [totalRows];

	for ( unsigned int r = 0; r < totalRows; r++ )
	{
		cells[r] = new HTMLTableCell * [totalCols];
		memset( cells[r], 0, totalCols * sizeof( HTMLTableCell * ) );
	}

	if ( percent )
		width = max_width * percent / 100;
	else if ( width == 0 )
		width = max_width;
	else
		setFixedWidth( TRUE );
}

void HTMLTable::startRow()
{
	col = 0;
}

void HTMLTable::addCell( HTMLTableCell *cell )
{
	while ( col < totalCols && cells[row][col] != NULL )
		col++;
	setCells( row, col, cell );

	col++;
}

void HTMLTable::endRow()
{
	row++;
}

void HTMLTable::setCells( unsigned int r, unsigned int c, HTMLTableCell *cell )
{
	unsigned int endRow = r + cell->rowSpan();
	unsigned int endCol = c + cell->colSpan();

	if ( endCol > totalCols )
		addColumns( endCol - totalCols );

	if ( endRow >= totalRows )
		addRows( endRow - totalRows + 10 );

	for ( ; r < endRow; r++ )
	{
		for ( unsigned int tc = c; tc < endCol; tc++ )
		{
			cells[r][tc] = cell;
		}
	}
}

void HTMLTable::addRows( int num )
{
	HTMLTableCell ***newRows = new HTMLTableCell ** [totalRows + num];
	memcpy( newRows, cells, totalRows * sizeof(HTMLTableCell **) );
	delete [] cells;
	cells = newRows;

	for ( unsigned int r = totalRows; r < totalRows + num; r++ )
	{
		cells[r] = new HTMLTableCell * [totalCols];
		memset( cells[r], 0, totalCols * sizeof( HTMLTableCell * ) );
	}

	totalRows += num;
}

void HTMLTable::addColumns( int num )
{
	HTMLTableCell **newCells;

	for ( unsigned int r = 0; r < totalRows; r++ )
	{
		newCells = new HTMLTableCell * [totalCols+num];
		memcpy( newCells, cells[r], totalCols * sizeof( HTMLTableCell * ) );
		memset( newCells + totalCols, 0, num * sizeof( HTMLTableCell * ) );
		delete [] cells[r];
		cells[r] = newCells;
	}

	totalCols += num;
}

void HTMLTable::endTable()
{
	calcColumnWidths();
}

void HTMLTable::calcAbsolutePos( int _x, int _y )
{
	int lx = _x + x;
	int ly = _y + y - ascent;
	HTMLTableCell *cell;

 	unsigned int r, c;

	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			if ( ( cell = cells[r][c] ) == NULL )
				continue;
			if ( c < totalCols - 1 && cell == cells[r][c+1] )
				continue;
			if ( r < row - 1 && cells[r+1][c] == cell )
				continue;

			cell->calcAbsolutePos( lx, ly );
		}
	}
}

HTMLAnchor* HTMLTable::findAnchor( const char *_name, QPoint *_p )
{
    HTMLAnchor *ret;
	HTMLTableCell *cell;

    _p->setX( _p->x() + x );
    _p->setY( _p->y() + y - ascent );
 
 	unsigned int r, c;

	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			if ( ( cell = cells[r][c] ) == NULL )
				continue;
			if ( c < totalCols - 1 && cell == cells[r][c+1] )
				continue;
			if ( r < row - 1 && cells[r+1][c] == cell )
				continue;

			ret = cell->findAnchor( _name, _p );
			if ( ret != NULL )
				return ret;
		}
	}
	
    _p->setX( _p->x() - x );
    _p->setY( _p->y() - y + ascent );

	return NULL;
}

void HTMLTable::reset()
{
	unsigned int r, c;
	HTMLTableCell *cell;

	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			if ( ( cell = cells[r][c] ) == NULL )
				continue;
			if ( c < totalCols - 1 && cell == cells[r][c+1] )
				continue;
			if ( r < row - 1 && cells[r+1][c] == cell )
				continue;

			cell->reset();
		}
	}
}

void HTMLTable::calcSize( HTMLClue * )
{
	unsigned int r, c;
	HTMLTableCell *cell;

	// Attempt to get sensible cell widths
	optimiseCellWidth();

	// set cell widths and then calculate cell sizes
	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			if ( ( cell = cells[r][c] ) == NULL )
				continue;
			if ( c < totalCols - 1 && cell == cells[r][c+1] )
				continue;
			if ( r < row - 1 && cell == cells[r+1][c] )
				continue;

			cell->setMaxWidth( columnOpt[c+1] -
				 columnOpt[ c-cell->colSpan()+1 ] - spacing -
				 padding - padding - 1 );
			cell->calcSize( NULL );
		}
	}

	if ( caption )
	{
		caption->setMaxWidth( columnOpt[ totalCols ] + border );
		caption->calcSize();
		if ( capAlign == HTMLClue::Top )
			caption->setPos( 0, caption->getAscent() );
	}

	// We have the cell sizes now, so calculate the vertical positions
	calcRowHeights();

	// set cell positions
	for ( r = 0; r < row; r++ )
	{
		int cellHeight;

		ascent = rowHeights[r+1] - padding - spacing;
		if ( caption && capAlign == HTMLClue::Top )
			ascent += caption->getHeight();

		for ( c = 0; c < totalCols; c++ )
		{
			if ( ( cell = cells[r][c] ) == NULL )
				continue;
			if ( c < totalCols - 1 && cell == cells[r][c+1] )
				continue;
			if ( r < row - 1 && cell == cells[r+1][c] )
				continue;

			cell->setPos( columnOpt[ c-cell->colSpan() + 1 ] + padding,
				ascent - cell->getDescent() );

			cellHeight = rowHeights[r+1] - rowHeights[r-cell->rowSpan()+1] -
				padding - padding - spacing;
			cell->setMaxAscent( cellHeight );
		}
	}

	if ( caption && capAlign == HTMLClue::Bottom )
		caption->setPos( 0, rowHeights[ row ] + border + caption->getAscent() );

	width = columnOpt[ totalCols ] + border;
	ascent = rowHeights[ row ] + border;
	if ( caption )
		ascent += caption->getHeight();
}

// Both the minimum and preferred column sizes are calculated here.
// The hard part is choosing the actual sizes based on these two.
void HTMLTable::calcColumnWidths()
{
	unsigned int r, c;
	int borderExtra = border ? 1 : 0;

	QArray<bool> fixed( totalCols );
	fixed.fill( false );

	columnPos.resize( totalCols+1 );
	columnPos[0] = border + spacing;

	columnPrefPos.resize( totalCols+1 );
	columnPrefPos[0] = border + spacing;

	for ( c = 0; c < totalCols; c++ )
	{
		columnPos[c+1] = 0;
		columnPrefPos[c+1] = 0;

		for ( r = 0; r < row; r++ )
		{
			HTMLTableCell *cell = cells[r][c];
			int colPos;

			if ( c < totalCols - 1 && cells[r][c+1] == cell )
				continue;
			if ( r < row - 1 && cells[r+1][c] == cell )
				continue;

			if (NULL == cell)
			        continue; 

/*
			// calculate minimum pos
			if ( cell->isFixedWidth() )
			{
				colPos = columnPos[ c - cell->colSpan() + 1 ] +
					cell->getWidth() + padding +
					padding + spacing + borderExtra;
			}
			else
			{
*/
				colPos = columnPos[ c - cell->colSpan() + 1 ] +
						cell->calcMinWidth() + padding + padding + spacing +
						borderExtra;
//			}
			if ( columnPos[c + 1] < colPos )
				columnPos[c + 1] = colPos;

			if ( fixed[c] )
				continue;

			// calculate preferred pos
/*
			if ( cell->getPercent() )
			{
				colPos = columnPrefPos[ c - cell->colSpan() + 1 ] +
					( max_width * cell->getPercent() / 100 ) + padding +
					padding + spacing + borderExtra + 1;
				fixed[c] = true;
				columnPrefPos[c + 1] = colPos;
			}
			else
*/
			if ( cell->isFixedWidth() )
			{
				colPos = columnPrefPos[ c - cell->colSpan() + 1 ] +
					cell->getWidth() + padding +
					padding + spacing + borderExtra + 1;
				fixed[c] = true;
				columnPrefPos[c + 1] = colPos;
			}
			else
			{
				colPos = columnPrefPos[ c - cell->colSpan() + 1 ] +
					cell->calcPreferredWidth() + padding + padding +
					spacing + borderExtra + 1;
				if ( columnPrefPos[c + 1] < colPos )
					columnPrefPos[c + 1] = colPos;
			}
		}
	}
}

// Use the minimum and preferred cell widths to produce an optimum
// cell spacing.  When this has been done columnOpt contains
// the optimised cell widths.
void HTMLTable::optimiseCellWidth()
{
	unsigned int c;
	int tableWidth = width - border;

	columnOpt = columnPos.copy();

	if ( columnPrefPos[totalCols] > tableWidth )
	{
		int extra = tableWidth - columnPos[totalCols];

		if ( extra > 0 )
		{
			for ( c = 1; c <= totalCols; c++ )
			{
				int addSize = extra / (totalCols-c+1);
				if ( addSize > columnPrefPos[c] - columnPos[c] )
					addSize = columnPrefPos[c] - columnPos[c];

				for ( unsigned int c1 = c; c1 <= totalCols; c1++ )
				{
					columnOpt[c1] += addSize;
				}
				extra -= addSize;
			}
		}
	}
	else if ( percent || isFixedWidth() )
	{
		columnPos = columnPrefPos;
		int extra = tableWidth - columnPrefPos[totalCols];
		int addSize = extra / totalCols;

		for ( c = 1; c <= totalCols; c++ )
		{
			for ( unsigned int c1 = c; c1 <= totalCols; c1++ )
				columnOpt[c1] += addSize;
		}
	}
	else
	{
		columnOpt = columnPrefPos.copy();
	}
}

void HTMLTable::calcRowHeights()
{
	unsigned int r, c;
	int borderExtra = border ? 1 : 0;
	HTMLTableCell *cell;

	rowHeights.resize( row+1 );
	rowHeights[0] = border + spacing;

	for ( r = 0; r < row; r++ )
	{
		rowHeights[r+1] = 0;
		for ( c = 0; c < totalCols; c++ )
		{
			if ( ( cell = cells[r][c] ) == NULL )
				continue;
			if ( c < totalCols - 1 && cell == cells[r][c+1] )
				continue;
			if ( r < row - 1 && cells[r+1][c] == cell )
				continue;

			int rowPos = rowHeights[ r - cell->rowSpan() + 1 ] +
				cell->getHeight() + padding + padding + spacing + borderExtra;

			if ( rowPos > rowHeights[r+1] )
				rowHeights[r+1] = rowPos;
		}
	}
}

int HTMLTable::calcMinWidth()
{
	return columnPos[totalCols] + border;
}

int HTMLTable::calcPreferredWidth()
{
	return columnPrefPos[totalCols] + border;
}

void HTMLTable::setMaxWidth( int _max_width )
{
	max_width = _max_width;

	if ( percent )
		width = max_width * percent / 100;
	else if ( !isFixedWidth() )
		width = max_width;

	calcColumnWidths();
}

void HTMLTable::setMaxAscent( int _a )
{
	ascent = _a;
}

HTMLObject *HTMLTable::checkPoint( int _x, int _y )
{
	unsigned int r, c;
    HTMLObject *obj;
	HTMLTableCell *cell;

    if ( _x < x || _x > x + width || _y > y + descent || _y < y - ascent)
	return 0L;

	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			if ( ( cell = cells[r][c] ) == NULL )
				continue;

			if ( c < totalCols - 1 && cell == cells[r][c+1] )
				continue;
			if ( r < row - 1 && cells[r+1][c] == cell )
				continue;

			if ((obj = cell->checkPoint( _x-x, _y-(y - ascent) )) != 0L)
				return obj;
		}
	}

	return NULL;
}

void HTMLTable::selectByURL( QPainter *_painter, const char *_url, bool _select, int _tx, int _ty )
{
	unsigned int r, c;
	HTMLTableCell *cell;

    _tx += x;
    _ty += y - ascent;

	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			if ( ( cell = cells[r][c] ) == NULL )
				continue;

			if ( c < totalCols - 1 && cell == cells[r][c+1] )
				continue;
			if ( r < row - 1 && cells[r+1][c] == cell )
				continue;

			cell->selectByURL( _painter, _url, _select, _tx, _ty );
		}
	}
}

void HTMLTable::select( QPainter *_painter, QRegExp& _pattern, bool _select, int _tx, int _ty )
{
	unsigned int r, c;
	HTMLTableCell *cell;

    _tx += x;
    _ty += y - ascent;

	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			if ( ( cell = cells[r][c] ) == NULL )
				continue;

			if ( c < totalCols - 1 && cell == cells[r][c+1] )
				continue;
			if ( r < row - 1 && cells[r+1][c] == cell )
				continue;

			cell->select( _painter, _pattern, _select, _tx, _ty );
		}
	}
}

void HTMLTable::select( QPainter *_painter, bool _select, int _tx, int _ty )
{
	unsigned int r, c;
	HTMLTableCell *cell;

    _tx += x;
    _ty += y - ascent;

	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			if ( ( cell = cells[r][c] ) == NULL )
				continue;

			if ( c < totalCols - 1 && cell == cells[r][c+1] )
				continue;
			if ( r < row - 1 && cells[r+1][c] == cell )
				continue;

			cell->select( _painter, _select, _tx, _ty );
		}
	}
}

void HTMLTable::select( QPainter *_painter, QRect & _rect, int _tx, int _ty )
{
	unsigned int r, c;
	HTMLTableCell *cell;

    _tx += x;
    _ty += y - ascent;

	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			if ( ( cell = cells[r][c] ) == NULL )
				continue;

			if ( c < totalCols - 1 && cell == cells[r][c+1] )
				continue;
			if ( r < row - 1 && cells[r+1][c] == cell )
				continue;

			cell->select( _painter, _rect, _tx, _ty );
		}
	}
}

void HTMLTable::select( bool _select )
{
	unsigned int r, c;
	HTMLTableCell *cell;

	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			if ( ( cell = cells[r][c] ) == NULL )
				continue;

			if ( c < totalCols - 1 && cell == cells[r][c+1] )
				continue;
			if ( r < row - 1 && cells[r+1][c] == cell )
				continue;

			cell->select( _select );
		}
	}
}

void HTMLTable::getSelected( QStrList &_list )
{
	unsigned int r, c;
	HTMLTableCell *cell;

	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			if ( ( cell = cells[r][c] ) == NULL )
				continue;

			if ( c < totalCols - 1 && cell == cells[r][c+1] )
				continue;
			if ( r < row - 1 && cells[r+1][c] == cell )
				continue;

			cell->getSelected( _list );
		}
	}
}

int HTMLTable::findPageBreak( int _y )
{
	if ( _y > y )
		return -1;

	unsigned int r, c;
	HTMLTableCell *cell;
	int pos, minpos = 0x7FFFFFFF;
	QArray<bool> colsDone( totalCols );
	colsDone.fill( false );

	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			if ( ( cell = cells[r][c] ) == NULL )
				continue;

			if ( c < totalCols - 1 && cell == cells[r][c+1] )
				continue;
			if ( r < row - 1 && cells[r+1][c] == cell )
				continue;
			if ( colsDone[c] )
				continue;

			pos = cell->findPageBreak( _y - ( y - ascent ) );
			if ( pos >= 0 && pos < minpos )
			{
				colsDone[c] = true;
				minpos = pos;
			}
		}

	}

	if ( minpos != 0x7FFFFFFF )
		return ( minpos + y - ascent );

	return -1;
}

bool HTMLTable::print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty, bool toPrinter )
{
    if ( _y + _height < y - getAscent() || _y > y )
	return false;
    
    if ( !isPrinting() )
	return false;
    
    _tx += x;
    _ty += y - ascent;
    
	unsigned int r, c;
	HTMLTableCell *cell;
	QArray<bool> colsDone( totalCols );
	colsDone.fill( false );

	if ( caption )
	{
		caption->print( _painter, _x - x, _y - (y - ascent),
			_width, _height, _tx, _ty, toPrinter );
	}

	// draw the cells
	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			if ( ( cell = cells[r][c] ) == NULL )
				continue;
			if ( c < totalCols - 1 && cell == cells[r][c+1] )
				continue;
			if ( r < row - 1 && cells[r+1][c] == cell )
				continue;
			if ( colsDone[c] )
				continue;
			if ( cell->print( _painter, _x - x, _y - (y - ascent),
					_width, _height, _tx, _ty, toPrinter ) )
				colsDone[c] = true;
		}
	}

	// draw the border - needs work to print to printer
	if ( border && !toPrinter )
	{
		int capOffset = 0;
		if ( caption && capAlign == HTMLClue::Top )
			capOffset = caption->getHeight();
		QColorGroup colorGrp( black, lightGray, white, darkGray, gray,
			black, white );
		qDrawShadePanel( _painter, _tx, _ty + capOffset, width,
			rowHeights[row] + border, colorGrp, false, border );

		// draw borders around each cell
		for ( r = 0; r < row; r++ )
		{
			for ( c = 0; c < totalCols; c++ )
			{
				if ( ( cell = cells[r][c] ) == NULL )
					continue;
				if ( c < totalCols - 1 && cell == cells[r][c+1] )
					continue;
				if ( r < row - 1 && cells[r+1][c] == cell )
					continue;

				qDrawShadePanel(_painter,
					_tx + columnOpt[c-cell->colSpan()+1],
					_ty + rowHeights[r-cell->rowSpan()+1] + capOffset,
					columnOpt[c+1] - columnOpt[c - cell->colSpan() + 1] -
					spacing,
					rowHeights[r+1] - rowHeights[r-cell->rowSpan()+1]-spacing,
					colorGrp, TRUE, 1 );
			}
		}
	}

	for ( c = 0; c < totalCols; c++ )
	{
		if ( colsDone[c] == true )
			return true;
	}

	return false;
}

void HTMLTable::print( QPainter *_painter, int _tx, int _ty )
{
    print( _painter, 0, 0, 0xFFFF, 0xFFFF, _tx, _ty );
}

HTMLTable::~HTMLTable()
{
	unsigned int r, c;
	HTMLTableCell *cell;

	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			if ( ( cell = cells[r][c] ) == NULL )
				continue;
			if ( c < totalCols - 1 && cell == cells[r][c+1] )
				continue;
			if ( r < row - 1 && cells[r+1][c] == cell )
				continue;
			delete cell;
		}

		delete [] cells[r];
	}

	delete [] cells;

	delete caption;
}

//-----------------------------------------------------------------------------

HTMLClue::HTMLClue( int _x, int _y, int _max_width, int _percent )
	 : HTMLObject()
{
    x = _x;
    y = _y;
    max_width = _max_width;
	percent = _percent;
    valign = Bottom;
    halign = Left;
    list.setAutoDelete( TRUE );

	if ( percent )
	{
		width = max_width * percent / 100;
		setFixedWidth( false );
	}
	else
		width = max_width;

	prevCalcObj = -1;
}

void HTMLClue::reset()
{
	HTMLObject *obj;

	for ( obj = list.first(); obj != 0L; obj = list.next() )
		obj->reset();

	prevCalcObj = -1;
}

void HTMLClue::calcAbsolutePos( int _x, int _y )
{
	HTMLObject *obj;

	int lx = _x + x;
	int ly = _y + y - ascent;

	for ( obj = list.first(); obj != 0L; obj = list.next() )
		obj->calcAbsolutePos( lx, ly );
}

HTMLAnchor* HTMLClue::findAnchor( const char *_name, QPoint *_p )
{
    HTMLObject *obj;
    HTMLAnchor *ret;

    _p->setX( _p->x() + x );
    _p->setY( _p->y() + y - ascent );
    
    for ( obj = list.first(); obj != 0L; obj = list.next() )
    {
	ret = obj->findAnchor( _name, _p );
	if ( ret != 0L )
	    return ret;
    }
    
    _p->setX( _p->x() - x );
    _p->setY( _p->y() - y + ascent );

    return 0L;
}

void HTMLClue::getSelected( QStrList &_list )
{
    HTMLObject *obj;

    for ( obj = list.first(); obj != 0L; obj = list.next() )
    {
	obj->getSelected( _list );
    }
}

void HTMLClue::select( bool _select )
{
    HTMLObject *obj;

    for ( obj = list.first(); obj != 0L; obj = list.next() )
    {
	obj->select( _select );
    }
}

void HTMLClue::select( QPainter *_painter, QRegExp& _pattern, bool _select, int _tx, int _ty )
{
    HTMLObject *obj;

    _tx += x;
    _ty += y - ascent;

    for ( obj = list.first(); obj != 0L; obj = list.next() )
    {
	obj->select( _painter, _pattern, _select, _tx, _ty );
    }
}

void HTMLClue::select( QPainter *_painter, bool _select, int _tx, int _ty )
{
    HTMLObject *obj;

    _tx += x;
    _ty += y - ascent;

    for ( obj = list.first(); obj != 0L; obj = list.next() )
    {
	obj->select( _painter, _select, _tx, _ty );
    }
}

void HTMLClue::select( QPainter *_painter, QRect & _rect, int _tx, int _ty )
{
    HTMLObject *obj;

    QRect r( x + _tx, y - ascent + _ty, width, ascent + descent );

    _tx += x;
    _ty += y - ascent;
    
    if ( _rect.contains( r ) )
      for ( obj = list.first(); obj != 0L; obj = list.next() )
      {
	obj->select( _painter, TRUE, _tx, _ty );
      }
    else if ( !_rect.intersects( r ) )
      for ( obj = list.first(); obj != 0L; obj = list.next() )
      {
	obj->select( _painter, FALSE, _tx, _ty );
      }
    else
      for ( obj = list.first(); obj != 0L; obj = list.next() )
      {
	obj->select( _painter, _rect, _tx, _ty );
      }
}

void HTMLClue::selectByURL( QPainter *_painter, const char *_url, bool _select, int _tx, int _ty )
{
    HTMLObject *obj;

    // _painter->translate( (float)x, (float)( y - ascent ) );
    _tx += x;
    _ty += y - ascent;

    for ( obj = list.first(); obj != 0L; obj = list.next() )
    {
	obj->selectByURL( _painter, _url, _select, _tx, _ty );
    }

    // _painter->translate( (float)-x, (float)-( y - ascent ) );
}

HTMLObject* HTMLClue::checkPoint( int _x, int _y )
{
    HTMLObject *obj;
    HTMLObject *obj2;

    if ( _x < x || _x > x + width || _y > y + descent || _y < y - ascent)
	return 0L;

    for ( obj = list.first(); obj != 0L; obj = list.next() )
    {
	if ((obj2 = obj->checkPoint( _x - x, _y - (y - ascent) )) != 0L)
	    return obj2;
    }

    return 0L;
}

void HTMLClue::calcSize( HTMLClue * )
{
	HTMLObject *obj;

	// If we have already called calcSize for the children, then just
	// continue from the last object done in previous call.
	if ( prevCalcObj >= 0 )
		obj = list.at( prevCalcObj );	// this will always find an object
	else
	{
//		x = y = ascent = 0;
		ascent = 0;
		obj = list.first();
	}

	while ( obj != NULL )
	{
		obj->calcSize( this );
		obj = list.next();
	}

	// remember this object so that we can start from here next time
	// we are called.
	list.last();
	prevCalcObj = list.at();
}

void HTMLClue::recalcBaseSize( QPainter *_painter )
{
	HTMLObject *obj;

	for ( obj = list.first(); obj != NULL; obj = list.next() )
		obj->recalcBaseSize( _painter );
}

int HTMLClue::calcMinWidth()
{
    HTMLObject *obj;
	int minWidth = 0;

    for ( obj = list.first(); obj != 0L; obj = list.next() )
    {
	int w = obj->calcMinWidth();
	if ( w > minWidth )
		minWidth = w;
    }

	return minWidth;
}

int HTMLClue::calcPreferredWidth()
{
	if ( isFixedWidth() )
		return width;

	HTMLObject *obj;
	int prefWidth = 0;

    for ( obj = list.first(); obj != 0L; obj = list.next() )
    {
	int w = obj->calcPreferredWidth();
	if ( w > prefWidth )
		prefWidth = w;
    }

	return prefWidth;
}

void HTMLClue::setMaxAscent( int _a )
{
    HTMLObject *obj;

    if ( valign == VCenter )
    {
	for ( obj = list.first(); obj != 0L; obj = list.next() )
	    obj->setYPos( obj->getYPos() + ( _a - ascent )/2 );
    }
	else if ( valign == Bottom )
	{
	for ( obj = list.first(); obj != 0L; obj = list.next() )
	    obj->setYPos( obj->getYPos() + _a - ascent );
	}

	ascent = _a;
}

int HTMLClue::findPageBreak( int _y )
{
	if ( _y > y )
		return -1;

	HTMLObject *obj;
	int pos;

    for ( obj = list.first(); obj != 0L; obj = list.next() )
    {
		if ( !obj->isAligned() )
		{
			pos = obj->findPageBreak( _y - ( y - ascent ) );
			if ( pos >= 0 )
				return ( pos + y - ascent );
		}
    }

	return -1;
}

bool HTMLClue::print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty, bool toPrinter )
{
    if ( _y + _height < y - getAscent() || _y > y )
	return false;
    
    if ( !isPrinting() )
	return false;
    
    HTMLObject *obj;

    _tx += x;
    _ty += y - ascent;

#ifdef CLUE_DEBUG
	// draw rectangles around clues - for debugging
	QBrush brush;
	_painter->setBrush( brush );
	_painter->drawRect( _tx, _ty, width, getHeight() );
#endif

    for ( obj = list.first(); obj != 0L; obj = list.next() )
    {
		if ( !obj->isAligned() )
		{
			if ( obj->print( _painter, _x - x, _y - (y - getHeight()),
					_width, _height, _tx, _ty, toPrinter ) && toPrinter )
				return true;
		}
    }

	return false;
}

void HTMLClue::print( QPainter *_painter, int _tx, int _ty )
{
    print( _painter, 0, 0, 0xFFFF, 0xFFFF, _tx, _ty );
}

void HTMLClue::print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty, HTMLObject *_obj )
{
    if ( _y + _height < y - getAscent() || _y > y )
	return;
    
    if ( !isPrinting() )
	return;
    
    HTMLObject *obj;

    _tx += x;
    _ty += y - ascent;
    
    for ( obj = list.first(); obj != 0L; obj = list.next() )
    {
	if ( obj == _obj )
	{
	    obj->print( _painter, _x - x, _y - (y - getHeight()), _width, _height, _tx, _ty );
	    return;
	}
    }
}

//-----------------------------------------------------------------------------

void HTMLClueV::reset()
{
	HTMLClue::reset();

	alignLeftList.clear();
	alignRightList.clear();
}

void HTMLClueV::setMaxWidth( int _max_width )
{
	HTMLObject *obj;

	if ( !isFixedWidth() )
	{
		max_width = _max_width;
		if ( percent )
			width = _max_width * percent / 100;
		else
			width = max_width;
	}

	for ( obj = list.first(); obj != 0L; obj = list.next() )
	{
		obj->setMaxWidth( width );
	}
}

HTMLObject* HTMLClueV::checkPoint( int _x, int _y )
{
    HTMLObject *obj2;

	if ( ( obj2 = HTMLClue::checkPoint( _x, _y ) ) != NULL )
		return obj2;

    if ( _x < x || _x > x + width || _y > y + descent || _y < y - ascent)
	return 0L;

	HTMLClueAligned *clue;
	for ( clue=alignLeftList.first(); clue!=NULL; clue=alignLeftList.next() )
	{
		if ((obj2 = clue->checkPoint( _x - x - clue->parent()->getXPos(),
				 _y - (y - ascent) - ( clue->parent()->getYPos() -
				 clue->parent()->getAscent() ) )) != 0L)
			return obj2;
	}
	for ( clue=alignRightList.first(); clue!=NULL; clue=alignRightList.next() )
	{
		if ((obj2 = clue->checkPoint( _x - x - clue->parent()->getXPos(),
				 _y - (y - ascent) - ( clue->parent()->getYPos() -
				 clue->parent()->getAscent() ) )) != 0L)
			return obj2;
	}

    return 0L;
}

void HTMLClueV::calcSize( HTMLClue *parent )
{
	HTMLObject *obj;
	int lmargin = parent ? parent->getLeftMargin( getYPos() ) : 0;

	// If we have already called calcSize for the children, then just
	// continue from the last object done in previous call.
	if ( prevCalcObj >= 0 )
	{
		obj = list.at( prevCalcObj );	// This will always find an object.
		ascent -= obj->getHeight();	// Otherwise it will be added twice.
	}
	else
	{
		ascent = descent = 0;
		obj = list.first();
	}

	while ( obj != NULL )
	{
		// Set an initial ypos so that the alignment stuff knows where
		// the top of this object is
		obj->setYPos( ascent );
		obj->calcSize( this );
		if ( obj->getWidth() > width )
			width = obj->getWidth();
		ascent += obj->getHeight();
		obj->setPos( lmargin, ascent - obj->getDescent() );
		obj = list.next();
	}

	// remember this object so that we can start from here next time
	// we are called.
	list.last();
	prevCalcObj = list.at();
/*
	if (max_width != 0 && width < max_width)
		width = max_width;
*/
	if ( halign == HCenter )
	{
		for ( obj = list.first(); obj != 0L; obj = list.next() )
			obj->setXPos( lmargin + (width - obj->getWidth()) / 2 );
	}
	else if ( halign == Right )
	{
		for ( obj = list.first(); obj != 0L; obj = list.next() )
			obj->setXPos( lmargin + width - obj->getWidth() );
	}

	HTMLClueAligned *clue;
	for ( clue=alignLeftList.first(); clue!=NULL; clue=alignLeftList.next() )
	{
		if ( clue->getYPos() + clue->parent()->getYPos() > ascent )
			ascent = clue->getYPos() + clue->parent()->getYPos();
	}
	for ( clue=alignRightList.first(); clue!=NULL; clue=alignRightList.next() )
	{
		if ( clue->getYPos() + clue->parent()->getYPos() > ascent )
			ascent = clue->getYPos() + clue->parent()->getYPos();
	}
}

bool HTMLClueV::print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty, bool toPrinter )
{
	bool rv = HTMLClue::print( _painter, _x, _y, _width, _height, _tx, _ty, toPrinter );

	// print aligned objects
    if ( _y + _height < y - getAscent() || _y > y )
	return rv;
    
    if ( !isPrinting() )
	return rv;

    _tx += x;
    _ty += y - ascent;

    HTMLClueAligned *clue;

    for ( clue=alignLeftList.first(); clue != 0L; clue=alignLeftList.next() )
    {
	clue->print( _painter, _tx + clue->parent()->getXPos(),
		_ty + clue->parent()->getYPos() - clue->parent()->getAscent() );
    }

    for ( clue=alignRightList.first(); clue != 0L; clue=alignRightList.next() )
    {
	clue->print( _painter, _tx + clue->parent()->getXPos(),
		_ty + clue->parent()->getYPos() - clue->parent()->getAscent() );
    }

	return rv;
}

int HTMLClueV::getLeftMargin( int _y )
{
	int margin = 0;
	HTMLClueAligned *clue;

	for ( clue=alignLeftList.first(); clue!=NULL; clue=alignLeftList.next() )
	{
		if ( clue->getYPos() - clue->getAscent() + clue->parent()->getYPos() -
				clue->parent()->getAscent() < _y &&
				clue->getYPos() + clue->parent()->getYPos() -
				clue->parent()->getAscent() > _y )
			margin = clue->getXPos() + clue->getWidth();
	}

	return margin;
}

int HTMLClueV::getRightMargin( int _y )
{
	int margin = max_width;
	HTMLClueAligned *clue;

	for ( clue=alignRightList.first(); clue!=NULL; clue=alignRightList.next() )
	{
		if ( clue->getYPos() - clue->getAscent() + clue->parent()->getYPos() -
				clue->parent()->getAscent() < _y &&
				clue->getYPos() + clue->parent()->getYPos() -
				clue->parent()->getAscent() > _y )
			margin = clue->getXPos();
	}

	return margin;
}

//-----------------------------------------------------------------------------

void HTMLClueH::setMaxWidth( int _w )
{
	HTMLObject *obj;
	max_width = _w;

	// first calculate width minus fixed width objects
    for ( obj = list.first(); obj != 0L; obj = list.next() )
    {
		if ( obj->getPercent() == 0 )	// i.e. fixed width objects
			_w -= obj->getWidth();
    }

	// now call setMaxWidth for variable objects
    for ( obj = list.first(); obj != 0L; obj = list.next() )
		if ( obj->getPercent() > 0 )
			obj->setMaxWidth( _w );
}

void HTMLClueH::calcSize( HTMLClue *parent )
{
	// make sure children are properly sized
	setMaxWidth( max_width );

    HTMLClue::calcSize( this );
    
    HTMLObject *obj;
	int lmargin = parent->getLeftMargin( getYPos() );

    width = lmargin;
    descent = 0;
    ascent = 0;

	int a = 0;
	int d = 0;
    for ( obj = list.first(); obj != 0L; obj = list.next() )
    {
	obj->setXPos( width );
	width += obj->getWidth();
	if ( obj->getAscent() > a )
	    a = obj->getAscent();
	if ( obj->getDescent() > d )
	    d = obj->getDescent();
    }

	ascent = a+d;

	switch ( valign )
	{
		case Top:
			for ( obj = list.first(); obj != 0L; obj = list.next() )
				obj->setYPos( obj->getAscent() );
			break;

		case VCenter:
			for ( obj = list.first(); obj != 0L; obj = list.next() )
				obj->setYPos( ascent/2 );
			break;

		default:
			for ( obj = list.first(); obj != 0L; obj = list.next() )
				obj->setYPos( ascent - d );
    }
}

int HTMLClueH::calcMinWidth()
{
    HTMLObject *obj;
	int minWidth = 0;

	for ( obj = list.first(); obj != 0L; obj = list.next() )
		minWidth += obj->calcMinWidth();
	
	return minWidth;
}
 
int HTMLClueH::calcPreferredWidth()
{
    HTMLObject *obj;
	int prefWidth = 0;

	for ( obj = list.first(); obj != 0L; obj = list.next() )
		prefWidth += obj->calcPreferredWidth();
	
	return prefWidth;
}
    
//-----------------------------------------------------------------------------

void HTMLClueFlow::calcSize( HTMLClue *parent )
{
	HTMLClue::calcSize( this );

	HTMLObject *obj;
	int lmargin, rmargin;

	ascent = 0;
	descent = 0;
	width = 0;
	lmargin = parent->getLeftMargin( getYPos() );
	rmargin = parent->getRightMargin( getYPos() );
	int w = lmargin;
	int a = 0;
	int d = 0;

	QListIterator<HTMLObject> line( list );
	QListIterator<HTMLObject> it( list );    

	for ( ; it.current() ; ++it )
	{
		obj = it.current();

		// This left or right aligned with other objects to wrap around it
		if ( obj->isAligned() )
		{
			HTMLClueAligned *c = (HTMLClueAligned *)obj;
			if ( c->getHAlign() == Left )
				parent->appendLeftAligned( c );
			else
				parent->appendRightAligned( c );
//			if ( it.current() == list.getFirst() )
			if ( w == lmargin )
			{
				if ( c->getHAlign() == Left )
					c->setPos( lmargin,
						ascent + c->getAscent() );
				else
					c->setPos( rmargin - c->getWidth(),
						ascent + c->getAscent() );
				while ( it.current() != line.current() )
					++line;
				++line;
				lmargin = parent->getLeftMargin( getYPos() + 1 );
				rmargin = parent->getRightMargin( getYPos() + 1 );
				w = lmargin;
			}
		}
		// Object exceeds max_width => new line containing only this object
		else if ( obj->getWidth() > rmargin && !obj->isSeparator() &&
			!obj->isNewline() && rmargin != 0)
		{
			if ( w != 0)
			{
				ascent += a + d;
				y += a + d;
			}

			if ( w > width )
				width = w;

			while ( line.current() != it.current() )
			{
				if ( line.current()->isAligned() )
				{
					HTMLClue *c = (HTMLClue *)line.current();
					if ( c->getHAlign() == Left )
					{
						int margin = parent->getLeftMargin( getYPos() + 1 );
						c->setPos( margin, ascent + c->getAscent() );
					}
					else
					{
						int margin = parent->getRightMargin( getYPos() + 1 );
						c->setPos( margin - c->getWidth(),
							ascent + c->getAscent() );
					}
				}
				else
				{
					line.current()->setYPos( ascent - d );
					line.current()->setMaxAscent( a );
					if ( halign == HCenter )
						line.current()->setXPos( line.current()->getXPos()+
							( rmargin - w )/2 );
					else if ( halign == Right )
						line.current()->setXPos( line.current()->getXPos()+
							( rmargin - w ) );
				}
				++line;
			}

			lmargin = parent->getLeftMargin( getYPos() + 1 );
			rmargin = parent->getRightMargin( getYPos() + 1 );

			ascent += obj->getHeight();
			obj->setPos( lmargin, ascent - obj->getDescent() );
			++line;

			if ( obj->getWidth() > width )
				width = obj->getWidth();
			w = lmargin;
			d = 0;
			a = 0;
		}
		// Object fits in this line and is no newline
		else if ( w + obj->getWidth() <= rmargin && rmargin != 0 &&
			!obj->isNewline() )
		{
			if ( obj->getAscent() > a )
				a = obj->getAscent();

			if ( obj->getDescent() > d )
				d = obj->getDescent();

			obj->setXPos( w );

			// skip a space at the start of a line
			if ( w != lmargin || !obj->isSeparator() )
			{
				w += obj->getWidth();
				if ( w > width)
					width = w;
			}
		}
		// Object does not fit in the line but is smaller than max_width or
		// it is a newline
		else 
		{
			// Perhaps the newline is the only object in this line...
			if ( obj->isNewline() )
			{
				if ( obj->getAscent() > a )
					a = obj->getAscent();
				if ( obj->getDescent() > d )
					d = obj->getDescent();
			}

			ascent += a + d;
			y += a + d;

			while ( line.current() != it.current() )
			{
				if ( line.current()->isAligned() )
				{
					HTMLClue *c = (HTMLClue *)line.current();
					if ( c->getHAlign() == Left )
					{
						int margin = parent->getLeftMargin( getYPos() + 1 );
						c->setPos( margin, ascent + c->getAscent() );
					}
					else
					{
						int margin = parent->getRightMargin( getYPos() + 1 );
						c->setPos( margin - c->getWidth(),
							ascent + c->getAscent() );
					}
				}
				else
				{
					line.current()->setYPos( ascent - d );
					line.current()->setMaxAscent( a );
					if ( halign == HCenter )
						line.current()->setXPos( line.current()->getXPos()+
							( rmargin - w )/2 );
					else if ( halign == Right )
						line.current()->setXPos( line.current()->getXPos()+
							( rmargin - w ) );
				}
				++line;
			}

			lmargin = parent->getLeftMargin( getYPos() + 1 );
			rmargin = parent->getRightMargin( getYPos() + 1 );

			// Do not print newlines or separators at the beginning of a line.
			if ( obj->isSeparator() || obj->isNewline() )
			{
				w = lmargin;
				a = 0;
				d = 0;
			}
			else
			{
				w = obj->getWidth() + lmargin;
				a = obj->getAscent();
				d = obj->getDescent();
				if ( w > width)
					width = w;
			}

			obj->setXPos( lmargin );
		}

	}

	if (w != 0)
	{
		ascent += a + d;
		y += a + d;

		while ( line.current() != it.current() )
		{
			if ( line.current()->isAligned() )
			{
				HTMLClue *c = (HTMLClue *)line.current();
				if ( c->getHAlign() == HTMLClue::Left )
				{
					int margin = parent->getLeftMargin( getYPos() + 1 );
					c->setPos( margin, ascent + c->getAscent() );
				}
				else
				{
					int margin = parent->getRightMargin( getYPos() + 1 );
					c->setPos( margin - c->getWidth(),
						ascent + c->getAscent() );
				}
			}
			else
			{	
				line.current()->setYPos( ascent - d );
				line.current()->setMaxAscent( a );
				if ( halign == HCenter )
					line.current()->setXPos( line.current()->getXPos()+
						( rmargin - w )/2 );
				else if ( halign == Right )
					line.current()->setXPos( line.current()->getXPos()+
						( rmargin - w ) );
			}
			++line;
		}

		if ( w > width )
			width = w;
	}    

	if ( width < max_width )
		width = max_width;
}

int HTMLClueFlow::findPageBreak( int _y )
{
	if ( _y > y )
		return -1;

	HTMLObject *obj;
	int pos, minpos, yp;

    for ( obj = list.first(); obj != 0L; obj = list.next() )
    {
		yp = obj->getYPos();
		minpos = yp;

		while ( obj && obj->getYPos() == yp )
		{
			if ( !obj->isAligned() )
			{
				pos = obj->findPageBreak( _y - ( y - ascent ) );
				if ( pos >= 0 && pos < minpos )
					minpos = pos;
			}
			obj = list.next();
		}
		if ( minpos != yp )
			return ( minpos + y - ascent );
    }

	return -1;
}

int HTMLClueFlow::calcPreferredWidth()
{
	HTMLObject *obj;
	int maxw = 0, w = 0;

    for ( obj = list.first(); obj != 0L; obj = list.next() )
    {
		if ( !obj->isNewline() )
		{
			if ( !obj->isAligned() )
				w += obj->calcPreferredWidth();
		}
		else
		{
			if ( w > maxw )
				maxw = w;
			w = 0;
		}
    }

	if ( w > maxw )
		maxw = w;

	return maxw;
}

void HTMLClueFlow::setMaxWidth( int _max_width )
{
	max_width = _max_width;

	HTMLObject *obj;

    for ( obj = list.first(); obj != 0L; obj = list.next() )
    {
		obj->setMaxWidth( max_width );
	}
}

//-----------------------------------------------------------------------------


void HTMLClueAligned::setMaxWidth( int _max_width )
{
	max_width = _max_width;

	HTMLObject *obj;

    for ( obj = list.first(); obj != 0L; obj = list.next() )
    {
		obj->setMaxWidth( max_width );
	}
}

// HTMLClueAligned behaves like a HTMLClueV
//
void HTMLClueAligned::calcSize( HTMLClue * )
{
    HTMLClue::calcSize( this );

    HTMLObject *obj;

	width = 0;
	ascent = ALIGN_BORDER;
	descent = 0;

    for ( obj = list.first(); obj != 0L; obj = list.next() )
    {
	if ( obj->getWidth() > width )
	     width = obj->getWidth();
	ascent += obj->getHeight();
	obj->setPos( ALIGN_BORDER, ascent - obj->getDescent() );
    }

	ascent += ALIGN_BORDER;
	width += (ALIGN_BORDER*2);
}

//-----------------------------------------------------------------------------

