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
    if ( _select == isSelected() || url[0] == 0 )
	return;
	
    setSelected( _select );

    _painter->eraseRect( x + _tx, y - ascent + _ty, width, ascent+descent );
    print( _painter, _tx, _ty );
}

//-----------------------------------------------------------------------------

HTMLVSpace::HTMLVSpace( int _vspace ) : HTMLObject()
{
    ascent = _vspace;    
    descent = 0;
    width = 1;
}

//-----------------------------------------------------------------------------

HTMLText::HTMLText( const char* _text, const HTMLFont *_font, QPainter *_painter, const char *_url ) : HTMLObject()
{
    text = _text;
    font = _font;
    ascent = _painter->fontMetrics().ascent();
    descent = _painter->fontMetrics().descent();
    width = _painter->fontMetrics().width( (const char*)_text );
    url = _url;
    url.detach();
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

void HTMLText::print( QPainter *_painter, int, int _y, int, int _height, int _tx, int _ty )
{
    if ( _y + _height < y - getAscent() || _y > y + getDescent() )
	return;

    print( _painter, _tx, _ty );
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

void HTMLRule::print( QPainter *_painter, int, int _y, int, int _height, int _tx, int _ty )
{
    if ( _y + _height < y - getAscent() || _y > y + getDescent() )
	return;

    print( _painter, _tx, _ty );
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

void HTMLBullet::print( QPainter *_painter, int, int _y, int, int _height, int _tx, int _ty )
{
    if ( _y + _height < y - getAscent() || _y > y + getDescent() )
	return;

    print( _painter, _tx, _ty );
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
	  pCache->setAutoDelete( true );
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
	{
	  pCache = new QList<HTMLCachedImage>;
	  pCache->setAutoDelete( true );
	}

  pCache->append( new HTMLCachedImage( _filename ) );
}

HTMLImage::HTMLImage( KHTMLWidget *widget, const char *_filename, const char* _url, int _max_width,
		      int _width, int _height, int _percent )
{
	pCache = new QList<HTMLCachedImage>;
	pCache->setAutoDelete( true );

    pixmap = 0L;

    htmlWidget = widget;
    
    url = _url;
    url.detach();
    
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

void HTMLImage::print( QPainter *_painter, int, int _y, int, int _height, int _tx, int _ty )
{
    if ( _y + _height < y - getAscent() || _y > y + getDescent() )
	return;

    print( _painter, _tx, _ty );
}

void HTMLImage::print( QPainter *_painter, int _tx, int _ty )
{
    if ( pixmap == 0L )
	return;

	if ( (width != pixmap->width() || ascent != pixmap->height() ) &&
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

	delete pCache;
	pCache = NULL;
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

HTMLTableCell::HTMLTableCell( int _x, int _y, int _max_width, int _width,
	int _percent, int rs, int cs, int pad )
	 : HTMLClueV( _x, _y, _max_width, _percent )
{
	rspan = rs;
	cspan = cs;
	padding = pad;

	if ( _width )
	{
		width = _width;
		setFixedWidth( true );
	}
	else
		setFixedWidth( false );
}

void HTMLTableCell::print( QPainter *_painter, int _x, int _y, int _width,
	int _height, int _tx, int _ty )
{
	if ( _y + _height < y - getAscent() || _y > y )
		return;
    
	if ( !isPrinting() )
		return;
    
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

	HTMLClueV::print( _painter, _x, _y, _width, _height, _tx, _ty );
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

 	unsigned int r, c;

	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			cells[r][c]->calcAbsolutePos( lx, ly );
		}
	}
}

HTMLAnchor* HTMLTable::findAnchor( const char *_name, QPoint *_p )
{
    HTMLAnchor *ret;

    _p->setX( _p->x() + x );
    _p->setY( _p->y() + y - ascent );
 
 	unsigned int r, c;

	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			ret = cells[r][c]->findAnchor( _name, _p );
			if ( ret != NULL )
				return ret;
		}
	}
	
    _p->setX( _p->x() - x );
    _p->setY( _p->y() - y + ascent );

	return NULL;
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
			cell = cells[r][c];

			if ( c < totalCols - 1 && cell == cells[r][c+1] )
				continue;
			if ( r < row - 1 && cell == cells[r+1][c] )
				continue;

			cell->setMaxWidth( columnPos[c+1] -
				 columnPos[ c-cell->colSpan()+1 ] - spacing -
				 padding - padding - 1 );
			cell->calcSize( NULL );
		}
	}

	// We have the cell sizes now, so calculate the vertical positions
	calcRowHeights();

	// set cell positions
	for ( r = 0; r < row; r++ )
	{
		int cellHeight;

		ascent = rowHeights[r+1] - padding - spacing;

		for ( c = 0; c < totalCols; c++ )
		{
			cell = cells[r][c];

			if ( c < totalCols - 1 && cell == cells[r][c+1] )
				continue;
			if ( r < row - 1 && cell == cells[r+1][c] )
				continue;

			cell->setPos( columnPos[ c-cell->colSpan() + 1 ] + padding,
				ascent - cell->getDescent() );

			cellHeight = rowHeights[r+1] - rowHeights[r-cell->rowSpan()+1] -
				padding - padding - spacing;
			cell->setMaxAscent( cellHeight );
		}
	}

	width = columnPos[ totalCols ] + border;
	ascent = rowHeights[ row ] + border;
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
			if ( cell->getPercent() )
			{
				colPos = columnPrefPos[ c - cell->colSpan() + 1 ] +
					( max_width * cell->getPercent() / 100 ) + padding +
					padding + spacing + borderExtra + 1;
				fixed[c] = true;
				columnPrefPos[c + 1] = colPos;
			}
			else if ( cell->isFixedWidth() )
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
// cell spacing.  When this has been done columnPos contains
// the optimised cell widths.
void HTMLTable::optimiseCellWidth()
{
	unsigned int c;
	int tableWidth = width;

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
					columnPos[c1] += addSize;
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
				columnPos[c1] += addSize;
		}
	}
	else
	{
		columnPos = columnPrefPos;
	}
}

void HTMLTable::calcRowHeights()
{
	unsigned int r, c;
	int borderExtra = border ? 1 : 0;

	rowHeights.resize( row+1 );
	rowHeights[0] = border + spacing;

	for ( r = 0; r < row; r++ )
	{
		rowHeights[r+1] = 0;
		for ( c = 0; c < totalCols; c++ )
		{
			HTMLTableCell *cell = cells[r][c];

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

    if ( _x < x || _x > x + width || _y > y + descent || _y < y - ascent)
	return 0L;

	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			if ((obj = cells[r][c]->checkPoint( _x-x, _y-(y - ascent) )) != 0L)
				return obj;
		}
	}

	return NULL;
}

void HTMLTable::selectByURL( QPainter *_painter, const char *_url, bool _select, int _tx, int _ty )
{
	unsigned int r, c;

    _tx += x;
    _ty += y - ascent;

	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			cells[r][c]->selectByURL( _painter, _url, _select, _tx, _ty );
		}
	}
}

void HTMLTable::select( QPainter *_painter, QRegExp& _pattern, bool _select, int _tx, int _ty )
{
	unsigned int r, c;

    _tx += x;
    _ty += y - ascent;

	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			cells[r][c]->select( _painter, _pattern, _select, _tx, _ty );
		}
	}
}

void HTMLTable::select( QPainter *_painter, bool _select, int _tx, int _ty )
{
	unsigned int r, c;

    _tx += x;
    _ty += y - ascent;

	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			cells[r][c]->select( _painter, _select, _tx, _ty );
		}
	}
}

void HTMLTable::select( bool _select )
{
	unsigned int r, c;

	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			cells[r][c]->select( _select );
		}
	}
}

void HTMLTable::getSelected( QStrList &_list )
{
	unsigned int r, c;

	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			cells[r][c]->getSelected( _list );
		}
	}
}

void HTMLTable::print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty )
{
    if ( _y + _height < y - getAscent() || _y > y )
	return;
    
    if ( !isPrinting() )
	return;
    
    _tx += x;
    _ty += y - ascent;
    
	unsigned int r, c;

	// draw the cells
	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			HTMLTableCell *cell = cells[r][c];

			if ( c < totalCols - 1 && cell == cells[r][c+1] )
				continue;
			if ( r < row - 1 && cells[r+1][c] == cell )
				continue;
			cell->print( _painter, _x - x, _y - (y - getHeight()), _width, _height, _tx, _ty );
		}
	}

	// draw the border
	if ( border )
	{
		QColorGroup colorGrp( black, lightGray, white, darkGray, gray,
			black, white );
		qDrawShadePanel( _painter, _tx, _ty, width, ascent, colorGrp, false, border );

		// draw borders around each cell
		for ( r = 0; r < row; r++ )
		{
			for ( c = 0; c < totalCols; c++ )
			{
				HTMLTableCell *cell = cells[r][c];

				if ( c < totalCols - 1 && cell == cells[r][c+1] )
					continue;
				if ( r < row - 1 && cells[r+1][c] == cell )
					continue;

				qDrawShadePanel(_painter,
					_tx + columnPos[c-cell->colSpan()+1],
					_ty + rowHeights[r-cell->rowSpan()+1],
					columnPos[c+1] - columnPos[c - cell->colSpan() + 1] -
					spacing,
					rowHeights[r+1] - rowHeights[r-cell->rowSpan()+1]-spacing,
					colorGrp, TRUE, 1 );
			}
		}
	}
}

void HTMLTable::print( QPainter *_painter, int _tx, int _ty )
{
    print( _painter, 0, 0, 0xFFFF, 0xFFFF, _tx, _ty );
}

HTMLTable::~HTMLTable()
{
	unsigned int r, c;

	for ( r = 0; r < row; r++ )
	{
		for ( c = 0; c < totalCols; c++ )
		{
			HTMLTableCell *cell = cells[r][c];
			if ( c < totalCols - 1 && cell == cells[r][c+1] )
				continue;
			if ( r < row - 1 && cells[r+1][c] == cell )
				continue;
			delete cell;
		}

		delete [] cells[r];
	}

	delete [] cells;
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

    //_painter->translate( (float)x, (float)( y - ascent ) );
    _tx += x;
    _ty += y - ascent;

    for ( obj = list.first(); obj != 0L; obj = list.next() )
    {
	obj->select( _painter, _select, _tx, _ty );
    }

    // _painter->translate( (float)-x, (float)-( y - ascent ) );
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
		obj = list.first();

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

void HTMLClue::print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty )
{
    if ( _y + _height < y - getAscent() || _y > y )
	return;
    
    if ( !isPrinting() )
	return;
    
    HTMLObject *obj;

    _tx += x;
    _ty += y - ascent;
/*
	// draw rectangles around clues - for debugging
	_painter->setBrush( QBrush() );
	_painter->drawRect( _tx, _ty, width, getHeight() );
*/
    for ( obj = list.first(); obj != 0L; obj = list.next() )
    {
		if ( !obj->isAligned() )
	obj->print( _painter, _x - x, _y - (y - getHeight()), _width, _height, _tx, _ty );
    }
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

void HTMLClueV::setMaxWidth( int _max_width )
{
	HTMLObject *obj;

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
	int lmargin = parent ? parent->getLeftMargin( this, 0 ) : 0;

	// If we have already called calcSize for the children, then just
	// continue from the last object done in previous call.
	if ( prevCalcObj >= 0 )
	{
		obj = list.at( prevCalcObj );	// This will always find an object.
		ascent -= obj->getHeight();	// Otherwise it will be added twice.
	}
	else
		obj = list.first();

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

	if (max_width != 0 && width < max_width)
		width = max_width;

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

void HTMLClueV::print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty )
{
	HTMLClue::print( _painter, _x, _y, _width, _height, _tx, _ty );

	// print aligned objects
    if ( _y + _height < y - getAscent() || _y > y )
	return;
    
    if ( !isPrinting() )
	return;

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
}

int HTMLClueV::getLeftMargin( HTMLClue *child, int _y )
{
	int margin = 0;
	HTMLClueAligned *clue;

	_y += child->getYPos();

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

int HTMLClueV::getRightMargin( HTMLClue *child, int _y )
{
	int margin = max_width;
	HTMLClueAligned *clue;

	_y += child->getYPos();

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
	int lmargin = parent->getLeftMargin( this, 0 );

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
	lmargin = parent->getLeftMargin( this, 0 );
	rmargin = parent->getRightMargin( this, 0 );
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
			if ( it.current() == list.getFirst() )
			{
				if ( c->getHAlign() == Left )
					c->setPos( lmargin,
						ascent + c->getAscent() );
				else
					c->setPos( rmargin - c->getWidth(),
						ascent + c->getAscent() );
				++line;
				lmargin = parent->getLeftMargin( this, 1 );
				rmargin = parent->getRightMargin( this, 1 );
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
						int margin = parent->getLeftMargin( this, 1 );
						c->setPos( margin, ascent + c->getAscent() );
					}
					else
					{
						int margin = parent->getRightMargin( this, 1 );
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

			lmargin = parent->getLeftMargin( this, 1 );
			rmargin = parent->getRightMargin( this, 1 );

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

			w += obj->getWidth();
			if ( w > width)
				width = w;
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
						int margin = parent->getLeftMargin( this, 1 );
						c->setPos( margin, ascent + c->getAscent() );
					}
					else
					{
						int margin = parent->getRightMargin( this, 1 );
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

			lmargin = parent->getLeftMargin( this, 1 );
			rmargin = parent->getRightMargin( this, 1 );

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
					int margin = parent->getLeftMargin( this, 1 );
					c->setPos( margin, ascent + c->getAscent() );
				}
				else
				{
					int margin = parent->getRightMargin( this, 1 );
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

HTMLTokenizer::HTMLTokenizer( const char *str )
{
    int c;

	const char *src = str;

    for ( c = 0; *src != '\0'; c++, src++ )
    {
	if ( *src == '<' || *src == '>' || *src == ' ' || *src == '\n' )
	    c += 3;
	else if ( *src == '\t' )
	    c += TAB_SIZE;
    }
    
    buffer = new char[ c  + 1 ];
	char *dest = buffer;

    src = str;
    bool tag = false;
    // To avoid multiple spaces.
    bool space = false;
    bool pre = false;
    // If we have <pre> and get a '\t' we need to know
    // in which row we are in order to calculate the next
    // tabulators position.
    int pre_pos = 0;

    while ( *src != 0 )
    {
	if ( *src == '&' )
	{
	    if ( pre )
		pre_pos ++;	    
	    space = false;

	    // Is the string long enough?
	    if ( *(src+1) != '\0' && *(src+2) != '\0' )
	    {
		// Special character by number?
		if ( *(src + 1) == '#' )
		{
		    char *endptr;
		    int z = (int) strtol( src+2, &endptr, 10 );
		    *dest++ = z;
			src = endptr;
		    // Skip a trailing ';' ?
		    if ( *src == ';' )
				src++;
		}
		// Special character ?
		else if ( isalpha( *(src + 1) ) )
		{
		    int tmpcnt;
		    
		    for ( tmpcnt = 0; tmpcnt < NUM_AMPSEQ; tmpcnt++ ) 
			{
				if ( strncmp( AmpSequences[ tmpcnt ].tag, src+1,
					 strlen( AmpSequences[ tmpcnt ].tag ) ) == 0 )
				{
					*dest++ = AmpSequences[ tmpcnt ].value;
					src += strlen( AmpSequences[ tmpcnt ].tag ) + 1;
					if ( *src == ';' )
						src++;
					break;
				}
			}

			if ( tmpcnt == NUM_AMPSEQ )
			*dest++ = *src++;
		}
		else
		    *dest++ = *src++;
	    }
	    else
		*dest++ = *src++;
	}
	else if ( *src == '<' )
	{
	    if ( strncasecmp( src, "<pre>", 5 ) == 0 )
	    {
		pre_pos = 0;
		pre = TRUE;
	    }
	    else if ( strncasecmp( src, "</pre>", 6 ) == 0 )
		pre = false;
		else if ( strncasecmp( src, "<!--", 4 ) == 0 )
		{
			src += 4;

			while ( *src )
			{
				if ( strncasecmp( src, "-->", 3 ) == 0 )
				{
					src += 3;
					break;
				}
				src++;
			}
			continue;
		}

	    space = false;

	    if ( dest > buffer )
		*dest++ = 0;
	    *dest++ = TAG_ESCAPE;
	    *dest++ = '<';
	    tag = true;
	    src++;
	}
	else if ( *src == '>' )
	{
	    space = false;

	    *dest++ = '>';
	    *dest++ = 0;
	    tag = false;
	    src++;
	}
	else if ( !tag && pre && ( *src == ' ' || *src == '\t' || *src == '\n' || *src == 13 ) )
	{
	    // For every line break in <pre> insert a the tag '\n'.
	    if ( *src == '\n' )
	    {
		*dest++ = 0;
		*dest++ = TAG_ESCAPE;
		*dest++ = '\n';
		*dest++ = 0;
		pre_pos = 0; 
	    }
	    else if ( *src == '\t' )
	    {
		int p = TAB_SIZE - ( pre_pos % TAB_SIZE );
		for ( int x = 0; x < p; x++ )
		    *dest++ = ' ';
	    }
	    else if ( *src == ' ' )
	    {
		pre_pos ++;
		*dest++ = ' ';
		space = TRUE;
	    }
	    src++;
	}
	else if ( !tag && ( *src == ' ' || *src == '\t' || *src == '\n' || *src == 13 ) )
	{
	    if ( !space )
	    {
// MRJ - taking line this out nearly halves mem usage and makes almost no
// difference to output
//		*dest++ = 0;
		*dest++ = ' ';
		*dest++ = 0;
	    }
	    src++;
	    space = TRUE;
	}
	else
	{
	    space = false;
	    if ( pre )
		pre_pos++;
	    
	    *dest++ = *src++;
	}
    }

    end = dest;
    *end = 0;

    pos = buffer;
}

const char* HTMLTokenizer::nextToken()
{
    if ( pos == NULL )
	return 0L;

    char *ret = pos;
    pos += strlen( ret ) + 1;
    if ( pos >= end )
	pos = NULL;

    return ret;
}

bool HTMLTokenizer::hasMoreTokens()
{
    if ( pos == NULL )
	return false;
    return true;
}

HTMLTokenizer::~HTMLTokenizer()
{
    if ( buffer != 0L )
		delete [] buffer;
}

//-----------------------------------------------------------------------------

StringTokenizer::StringTokenizer( const QString &_str, const char *_separators )
{
    QString str = _str.simplifyWhiteSpace();
    int c;

    const char *separators = _separators;
    const char *src = str.data();

    for ( c = 0; *src != '\0'; c++, src++ )
    {
	const char *s = separators;
	while( *s != 0 )
	{
	    if ( *src == *s )
		c++;
	    s++;
	}
    }
    
    buffer = new char[ c + 1 ];

    src = str.data();
	end = buffer;
    bool quoted = false;
    
    for ( ; *src != '\0'; src++ )
    {
	char *x = strchr( separators, *src );
	if ( *src == '\"' )
	    quoted = !quoted;
	else if ( x != 0L && !quoted )
	    *end++ = 0;
	else
	    *end++ = *src;
    }

    *end = 0;

    pos = buffer;
}

const char* StringTokenizer::nextToken()
{
    if ( pos == NULL )
	return 0L;

    char *ret = pos;
    pos += strlen( ret ) + 1;
    if ( pos >= end )
	pos = NULL;

    return ret;
}

bool StringTokenizer::hasMoreTokens()
{
    if ( pos == NULL )
	return false;
    return true;
}

StringTokenizer::~StringTokenizer()
{
    if ( buffer != 0L )
	delete [] buffer;
}

