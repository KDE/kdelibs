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
//
// KDE HTML Widget
//

#include <kurl.h>

#include "htmlobj.h"
#include "htmlclue.h"
#include "ampseq.h"
#include "html.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <qimage.h>
#include <qdrawutl.h>

//#define CLUE_DEBUG

//-----------------------------------------------------------------------------

HTMLTableCell::HTMLTableCell( int _x, int _y, int _max_width, int _percent,
	int rs, int cs, int pad )
	 : HTMLClueV( _x, _y, _max_width, _percent )
{
	rspan = rs;
	cspan = cs;
	padding = pad;
	width = _max_width;
}

void HTMLTableCell::setMaxWidth( int _max_width )
{
    HTMLObject *obj;

    // We allow fixed width cells to be resized in a table
//    width = max_width = _max_width;
    max_width = _max_width;
/*
    if ( percent > 0 )
	width = _max_width * percent / 100;
    else if ( !isFixedWidth() )
	width = max_width;
*/
    for ( obj = head; obj != 0; obj = obj->next() )
	obj->setMaxWidth( max_width );
}

int HTMLTableCell::calcMinWidth()
{
    HTMLObject *obj;
    int minWidth = 0;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	int w = obj->calcMinWidth();
	if ( w > minWidth )
	    minWidth = w;
    }

//    if ( isFixedWidth() )
//	return width > minWidth ? width : minWidth;

    return minWidth;
}

bool HTMLTableCell::print( QPainter *_painter, int _x, int _y, int _width,
	int _height, int _tx, int _ty, bool toPrinter )
{
	if ( _y + _height < y - getAscent() || _y > y )
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
    caption = 0L;

    setFixedWidth( false );
    row = 0;
    col = 0;

    totalCols = 1;	// this should be expanded to the maximum number of cols
				    // by the first row parsed
    totalRows = 1;
    allocRows = 5;	// allocate five rows initially

    cells = new HTMLTableCell ** [allocRows];

    for ( unsigned int r = 0; r < allocRows; r++ )
    {
	cells[r] = new HTMLTableCell * [totalCols];
	memset( cells[r], 0, totalCols * sizeof( HTMLTableCell * ) );
    }

    if ( percent > 0 )
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
    while ( col < totalCols && cells[row][col] != 0L )
	col++;
    setCells( row, col, cell );

    col++;
}

void HTMLTable::endRow()
{
    while ( col < totalCols && cells[row][col] != 0 )
	col++;
    if ( col )
	row++;
}

void HTMLTable::setCells( unsigned int r, unsigned int c, HTMLTableCell *cell )
{
    unsigned int endRow = r + cell->rowSpan();
    unsigned int endCol = c + cell->colSpan();

    // The first row sets the number of columns.  Do not allow subsequent
    // rows to change the number of columns.
    if ( row != 0 && endCol > totalCols )
	endCol = totalCols;

    if ( endCol > totalCols )
	addColumns( endCol - totalCols );

    if ( endRow >= allocRows )
	addRows( endRow - allocRows + 10 );

    if ( endRow > totalRows )
	totalRows = endRow;

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
    HTMLTableCell ***newRows = new HTMLTableCell ** [allocRows + num];
    memcpy( newRows, cells, allocRows * sizeof(HTMLTableCell **) );
    delete [] cells;
    cells = newRows;

    for ( unsigned int r = allocRows; r < allocRows + num; r++ )
    {
	cells[r] = new HTMLTableCell * [totalCols];
	memset( cells[r], 0, totalCols * sizeof( HTMLTableCell * ) );
    }

    allocRows += num;
}

void HTMLTable::addColumns( int num )
{
    HTMLTableCell **newCells;

    for ( unsigned int r = 0; r < allocRows; r++ )
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
    totalRows = row;
    calcColumnWidths();
}

void HTMLTable::calcAbsolutePos( int _x, int _y )
{
    int lx = _x + x;
    int ly = _y + y - ascent;
    HTMLTableCell *cell;

    unsigned int r, c;

    for ( r = 0; r < totalRows; r++ )
    {
	for ( c = 0; c < totalCols; c++ )
	{
	    if ( ( cell = cells[r][c] ) == 0 )
		continue;
	    if ( c < totalCols - 1 && cell == cells[r][c+1] )
		continue;
	    if ( r < totalRows - 1 && cells[r+1][c] == cell )
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

    for ( r = 0; r < totalRows; r++ )
    {
	for ( c = 0; c < totalCols; c++ )
	{
	    if ( ( cell = cells[r][c] ) == 0 )
		continue;
	    if ( c < totalCols - 1 && cell == cells[r][c+1] )
		continue;
	    if ( r < totalRows - 1 && cells[r+1][c] == cell )
		continue;

	    ret = cell->findAnchor( _name, _p );
	    if ( ret != 0 )
		return ret;
	}
    }
	
    _p->setX( _p->x() - x );
    _p->setY( _p->y() - y + ascent );

    return 0;
}

void HTMLTable::reset()
{
    unsigned int r, c;
    HTMLTableCell *cell;

    for ( r = 0; r < totalRows; r++ )
    {
	for ( c = 0; c < totalCols; c++ )
	{
	    if ( ( cell = cells[r][c] ) == 0 )
		continue;
	    if ( c < totalCols - 1 && cell == cells[r][c+1] )
		continue;
	    if ( r < totalRows - 1 && cells[r+1][c] == cell )
		continue;

	    cell->reset();
	}
    }
}

void HTMLTable::calcSize( HTMLClue * )
{
    unsigned int r, c;
    int indx;
    HTMLTableCell *cell;

    // recalculate min/max widths
    calcColumnWidths();

    // Attempt to get sensible cell widths
    optimiseCellWidth();

    // set cell widths and then calculate cell sizes
    for ( r = 0; r < totalRows; r++ )
    {
	for ( c = 0; c < totalCols; c++ )
	{
	    if ( ( cell = cells[r][c] ) == 0 )
		continue;
	    if ( c < totalCols - 1 && cell == cells[r][c+1] )
		continue;
	    if ( r < totalRows - 1 && cell == cells[r+1][c] )
		continue;

	    if ( ( indx = c-cell->colSpan()+1 ) < 0 )
		indx = 0;

	    cell->setMaxWidth( columnOpt[c+1] - columnOpt[ indx ] - spacing -
		 padding - padding );
	    cell->calcSize( 0 );
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
    for ( r = 0; r < totalRows; r++ )
    {
	int cellHeight;

	ascent = rowHeights[r+1] - padding - spacing;
	if ( caption && capAlign == HTMLClue::Top )
	    ascent += caption->getHeight();

	for ( c = 0; c < totalCols; c++ )
	{
	    if ( ( cell = cells[r][c] ) == 0 )
		continue;
	    if ( c < totalCols - 1 && cell == cells[r][c+1] )
		continue;
	    if ( r < totalRows - 1 && cell == cells[r+1][c] )
		continue;

	    if ( ( indx = c-cell->colSpan()+1 ) < 0 )
		indx = 0;

	    cell->setPos( columnOpt[indx] + padding,
		ascent - cell->getDescent() );

	    if ( ( indx = r-cell->rowSpan()+1 ) < 0 )
		indx = 0;

	    cellHeight = rowHeights[r+1] - rowHeights[indx] -
		padding - padding - spacing;
	    cell->setMaxAscent( cellHeight );
	}
    }

    if ( caption && capAlign == HTMLClue::Bottom )
	caption->setPos( 0, rowHeights[ totalRows ] + border + caption->getAscent() );

    width = columnOpt[ totalCols ] + border;
    ascent = rowHeights[ totalRows ] + border;
    if ( caption )
	ascent += caption->getHeight();
}

// Both the minimum and preferred column sizes are calculated here.
// The hard part is choosing the actual sizes based on these two.
void HTMLTable::calcColumnWidths()
{
    unsigned int r, c;
    int indx, borderExtra = ( border == 0 ) ? 0 : 1;

    colType.resize( totalCols+1 );
    colType.fill( Variable );

    columnPos.resize( totalCols+1 );
    columnPos[0] = border + spacing;

    columnPrefPos.resize( totalCols+1 );
    columnPrefPos[0] = border + spacing;

    colSpan.resize( totalCols+1 );
    colSpan.fill( 1 );

    for ( c = 0; c < totalCols; c++ )
    {
	columnPos[c+1] = 0;
	columnPrefPos[c+1] = 0;

	for ( r = 0; r < totalRows; r++ )
	{
	    HTMLTableCell *cell = cells[r][c];
	    int colPos;

	    if ( cell == 0 )
		continue; 
	    if ( c < totalCols - 1 && cells[r][c+1] == cell )
		continue;
	    if ( r < totalRows - 1 && cells[r+1][c] == cell )
		continue;

	    if ( ( indx = c-cell->colSpan()+1 ) < 0 )
		indx = 0;

	    // calculate minimum pos.
	    colPos = columnPos[indx] + cell->calcMinWidth() +
		    padding + padding + spacing + borderExtra;

	    if ( columnPos[c + 1] < colPos )
		columnPos[c + 1] = colPos;

	    if ( colType[c + 1] != Variable )
		continue;

	    // calculate preferred pos

	    if ( cell->getPercent() > 0 )
	    {
		colPos = columnPrefPos[indx] +
			( max_width * cell->getPercent() / 100 ) + padding +
			padding + spacing + borderExtra;
		colType[c + 1] = Percent;
		colSpan[c + 1] = cell->colSpan();
		columnPrefPos[c + 1] = colPos;
	    }
	    else if ( cell->isFixedWidth() )
	    {
		colPos = columnPrefPos[indx] + cell->getWidth() + padding +
			padding + spacing + borderExtra;
		colType[c + 1] = Fixed;
		colSpan[c + 1] = cell->colSpan();
		columnPrefPos[c + 1] = colPos;
	    }
	    else
	    {
		colPos = columnPrefPos[indx] + cell->calcPreferredWidth() +
			padding + padding + spacing + borderExtra;
		if ( columnPrefPos[c + 1] < colPos )
		    columnPrefPos[c + 1] = colPos;
	    }

	    if ( columnPrefPos[c + 1] < columnPos[c + 1] )
		columnPrefPos[c + 1] = columnPos[c + 1];
	}
    }
/*
    debug( "Cell widths" );
    for ( c = 0; c <= totalCols; c++ )
	debug( "cell min: %d, cell pref: %d", columnPos[c], columnPrefPos[c] );
*/
}

// Use the minimum and preferred cell widths to produce an optimum
// cell spacing.  When this has been done columnOpt contains
// the optimum cell widths.
void HTMLTable::optimiseCellWidth()
{
    unsigned int c, c1;
    int tableWidth = width - border;
    int totalPref = 0, totalMin = 0, totalPc = 0;

    columnOpt = columnPos.copy();

    if ( ( ( percent > 0 || isFixedWidth() ) &&
	    tableWidth > columnPos[ totalCols ] ) ||
	    columnPrefPos[totalCols] > tableWidth )
    {
	int addSize;

	// satisfy fixed width cells
	for ( c = 1; c <= totalCols; c++ )
	{
	    int prefWidth = columnPrefPos[c] - columnPrefPos[c-colSpan[c]];
	    int minWidth = columnPos[c] - columnPos[c-colSpan[c]];

	    if ( colType[c] == Fixed && prefWidth > minWidth )
	    {
		addSize = prefWidth - minWidth;

		for ( c1 = c; c1 <= totalCols; c1++ )
		    columnOpt[c1] += addSize;
	    }

	    if ( colType[c] == Percent && prefWidth > minWidth )
		totalPc += ( prefWidth - minWidth );
	}

	int extra = tableWidth - columnOpt[totalCols];

	// add space to percent width columns
	if ( extra > 0 )
	{
	    if ( extra > totalPc )
		extra = totalPc;
	    for ( c = 1; c <= totalCols; c++ )
	    {
		int prefWidth = columnPrefPos[c] - columnPrefPos[c-colSpan[c]];
		int minWidth = columnOpt[c] - columnOpt[c-colSpan[c]];

		if ( colType[c] != Percent )
		    continue;

		if ( prefWidth > minWidth )
		{
		    addSize = (prefWidth-minWidth) * extra / totalPc;
		    for ( c1 = c; c1 <= totalCols; c1++ )
			    columnOpt[c1] += addSize;
		}
	    }
	}
	
	extra = tableWidth - columnOpt[totalCols];

	// add space to variable width columns
	if ( extra > 0 )
	{
	    QArray<int> prefWidth( totalCols+1 );
	    QArray<int> minWidth( totalCols+1 );

	    for ( c = 1; c <= totalCols; c++ )
	    {
		if ( colType[c] != Variable )
		    continue;

		prefWidth[c] = columnPrefPos[c] - columnPrefPos[c-colSpan[c]];
		minWidth[c] = columnOpt[c] - columnOpt[c-colSpan[c]];

		if ( prefWidth[c] > tableWidth )
		    prefWidth[c] = tableWidth;
		if ( prefWidth[c] > minWidth[c] )
		{
		    totalPref += prefWidth[c];
		    totalMin += minWidth[c];
		}
	    }
	    for ( c = 1; c <= totalCols; c++ )
	    {
		if ( colType[c] != Variable )
			continue;

		if ( prefWidth[c] > minWidth[c] )
		{
		    addSize = prefWidth[c] * extra / totalPref;

		    if ( percent <= 0 && !isFixedWidth() &&
			    minWidth[c] + addSize > prefWidth[c] )
		    {
			addSize = prefWidth[c] - minWidth[c];
		    }

		    for ( c1 = c; c1 <= totalCols; c1++ )
			columnOpt[c1] += addSize;
		}
	    }
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
    int indx, borderExtra = border ? 1 : 0;
    HTMLTableCell *cell;

    rowHeights.resize( totalRows+1 );
    rowHeights[0] = border + spacing;

    for ( r = 0; r < totalRows; r++ )
    {
	rowHeights[r+1] = 0;
	for ( c = 0; c < totalCols; c++ )
	{
	    if ( ( cell = cells[r][c] ) == 0 )
		continue;
	    if ( c < totalCols - 1 && cell == cells[r][c+1] )
		continue;
	    if ( r < totalRows - 1 && cells[r+1][c] == cell )
		continue;

	    if ( ( indx = r - cell->rowSpan() + 1 ) < 0 )
		indx = 0;

	    int rowPos = rowHeights[ indx ] + cell->getHeight() +
		padding + padding + spacing + borderExtra;

	    if ( rowPos > rowHeights[r+1] )
		rowHeights[r+1] = rowPos;
	}

	if ( rowHeights[r+1] < rowHeights[r] )
	    rowHeights[r+1] = rowHeights[r];
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

	if ( percent > 0 )
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

    for ( r = 0; r < totalRows; r++ )
    {
	for ( c = 0; c < totalCols; c++ )
	{
	    if ( ( cell = cells[r][c] ) == 0 )
		continue;

	    if ( c < totalCols - 1 && cell == cells[r][c+1] )
		continue;
	    if ( r < totalRows - 1 && cells[r+1][c] == cell )
		continue;

	    if ((obj = cell->checkPoint( _x-x, _y-(y - ascent) )) != 0)
		return obj;
	}
    }

    return 0L;
}

void HTMLTable::selectByURL( QPainter *_painter, const char *_url, bool _select,
int _tx, int _ty )
{
    unsigned int r, c;
    HTMLTableCell *cell;

    _tx += x;
    _ty += y - ascent;

    for ( r = 0; r < totalRows; r++ )
    {
	for ( c = 0; c < totalCols; c++ )
	{
	    if ( ( cell = cells[r][c] ) == 0 )
		continue;

	    if ( c < totalCols - 1 && cell == cells[r][c+1] )
		continue;
	    if ( r < totalRows - 1 && cells[r+1][c] == cell )
		continue;

	    cell->selectByURL( _painter, _url, _select, _tx, _ty );
	}
    }
}

void HTMLTable::select( QPainter *_painter, QRegExp& _pattern, bool _select, int
_tx, int _ty )
{
    unsigned int r, c;
    HTMLTableCell *cell;

    _tx += x;
    _ty += y - ascent;

    for ( r = 0; r < totalRows; r++ )
    {
	for ( c = 0; c < totalCols; c++ )
	{
	    if ( ( cell = cells[r][c] ) == 0 )
		continue;

	    if ( c < totalCols - 1 && cell == cells[r][c+1] )
		continue;
	    if ( r < totalRows - 1 && cells[r+1][c] == cell )
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

    for ( r = 0; r < totalRows; r++ )
    {
	for ( c = 0; c < totalCols; c++ )
	{
	    if ( ( cell = cells[r][c] ) == 0 )
		continue;

	    if ( c < totalCols - 1 && cell == cells[r][c+1] )
		continue;
	    if ( r < totalRows - 1 && cells[r+1][c] == cell )
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

    for ( r = 0; r < totalRows; r++ )
    {
	for ( c = 0; c < totalCols; c++ )
	{
	    if ( ( cell = cells[r][c] ) == 0 )
		continue;

	    if ( c < totalCols - 1 && cell == cells[r][c+1] )
		continue;
	    if ( r < totalRows - 1 && cells[r+1][c] == cell )
		continue;

	    cell->select( _painter, _rect, _tx, _ty );
	}
    }
}

void HTMLTable::select( bool _select )
{
    unsigned int r, c;
    HTMLTableCell *cell;

    for ( r = 0; r < totalRows; r++ )
    {
	for ( c = 0; c < totalCols; c++ )
	{
	    if ( ( cell = cells[r][c] ) == 0 )
		continue;

	    if ( c < totalCols - 1 && cell == cells[r][c+1] )
		continue;
	    if ( r < totalRows - 1 && cells[r+1][c] == cell )
		continue;

	    cell->select( _select );
	}
    }
}

bool HTMLTable::selectText( QPainter *_painter, int _x1, int _y1,
	int _x2, int _y2, int _tx, int _ty )
{
    bool isSel = false;
    unsigned int r, c;
    HTMLTableCell *cell;

    _tx += x;
    _ty += y - ascent;

    for ( r = 0; r < totalRows; r++ )
    {
	for ( c = 0; c < totalCols; c++ )
	{
	    if ( ( cell = cells[r][c] ) == 0 )
		continue;

	    if ( c < totalCols - 1 && cell == cells[r][c+1] )
		continue;
	    if ( r < totalRows - 1 && cells[r+1][c] == cell )
		continue;

	    if ( _y1 < y - ascent && _y2 > y )
	    {
		isSel = cell->selectText( _painter, 0, _y1 - ( y - ascent ),
			width + 1, _y2 - ( y - ascent ), _tx, _ty ) || isSel;
	    }
	    else if ( _y1 < y - ascent )
	    {
		isSel = cell->selectText( _painter, 0, _y1 - ( y - ascent ),
			_x2 - x, _y2 - ( y - ascent ), _tx, _ty ) || isSel;
	    }
	    else if ( _y2 > y )
	    {
		isSel = cell->selectText( _painter, _x1 - x,
			_y1 - ( y - ascent ), width + 1, _y2 - ( y - ascent ),
			_tx, _ty ) || isSel;
	    }
	    else if ( (_x1 - x < cell->getXPos() + cell->getWidth() &&
			_x2 - x > cell->getXPos() ) )
	    {
		isSel = cell->selectText( _painter, _x1 - x,
			_y1 - ( y - ascent ), _x2 - x, _y2 - ( y - ascent ),
			_tx, _ty ) || isSel;
	    }
	    else
	    {
		cell->selectText( _painter, 0, 0, 0, 0, _tx, _ty );
	    }
	}
    }

    return isSel;
}

void HTMLTable::getSelected( QStrList &_list )
{
    unsigned int r, c;
    HTMLTableCell *cell;

    for ( r = 0; r < totalRows; r++ )
    {
	for ( c = 0; c < totalCols; c++ )
	{
	    if ( ( cell = cells[r][c] ) == 0 )
		continue;

	    if ( c < totalCols - 1 && cell == cells[r][c+1] )
		continue;
	    if ( r < totalRows - 1 && cells[r+1][c] == cell )
		continue;

	    cell->getSelected( _list );
	}
    }
}

void HTMLTable::getSelectedText( QString &_str )
{
    unsigned int r, c;
    HTMLTableCell *cell;

    for ( r = 0; r < totalRows; r++ )
    {
	for ( c = 0; c < totalCols; c++ )
	{
	    if ( ( cell = cells[r][c] ) == 0 )
		continue;

	    if ( c < totalCols - 1 && cell == cells[r][c+1] )
		continue;
	    if ( r < totalRows - 1 && cells[r+1][c] == cell )
		continue;

	    cell->getSelectedText( _str );
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

    for ( r = 0; r < totalRows; r++ )
    {
	for ( c = 0; c < totalCols; c++ )
	{
	    if ( ( cell = cells[r][c] ) == 0 )
		continue;

	    if ( c < totalCols - 1 && cell == cells[r][c+1] )
		continue;
	    if ( r < totalRows - 1 && cells[r+1][c] == cell )
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

    _tx += x;
    _ty += y - ascent;

    unsigned int r, c;
    int cindx, rindx;
    HTMLTableCell *cell;
    QArray<bool> colsDone( totalCols );
    colsDone.fill( false );

    if ( caption )
    {
	caption->print( _painter, _x - x, _y - (y - ascent),
	    _width, _height, _tx, _ty, toPrinter );
    }

    // draw the cells
    for ( r = 0; r < totalRows; r++ )
    {
	for ( c = 0; c < totalCols; c++ )
	{
	    if ( ( cell = cells[r][c] ) == 0 )
		continue;
	    if ( c < totalCols - 1 && cell == cells[r][c+1] )
		continue;
	    if ( r < totalRows - 1 && cells[r+1][c] == cell )
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
	    rowHeights[totalRows] + border, colorGrp, false, border );

	// draw borders around each cell
	for ( r = 0; r < totalRows; r++ )
	{
	    for ( c = 0; c < totalCols; c++ )
	    {
		if ( ( cell = cells[r][c] ) == 0 )
		    continue;
		if ( c < totalCols - 1 && cell == cells[r][c+1] )
		    continue;
		if ( r < totalRows - 1 && cells[r+1][c] == cell )
		    continue;

		if ( ( cindx = c-cell->colSpan()+1 ) < 0 )
		    cindx = 0;
		if ( ( rindx = r-cell->rowSpan()+1 ) < 0 )
		    rindx = 0;

		qDrawShadePanel(_painter,
		    _tx + columnOpt[cindx],
		    _ty + rowHeights[rindx] + capOffset,
		    columnOpt[c+1] - columnOpt[cindx] - spacing,
		    rowHeights[r+1] - rowHeights[rindx] - spacing,
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

void HTMLTable::print( QPainter *_painter, HTMLObject *_obj, int _x, int _y, int _width, int _height, int _tx, int _ty )
{
    if ( _y + _height < y - getAscent() || _y > y )
	return;

    _tx += x;
    _ty += y - ascent;

    unsigned int r, c;
    HTMLTableCell *cell;

    for ( r = 0; r < totalRows; r++ )
    {
	for ( c = 0; c < totalCols; c++ )
	{
	    if ( ( cell = cells[r][c] ) == 0 )
		continue;
	    if ( c < totalCols - 1 && cell == cells[r][c+1] )
		continue;
	    if ( r < totalRows - 1 && cells[r+1][c] == cell )
		continue;
	    cell->print( _painter, _obj, _x - x, _y - (y - ascent),
		_width, _height, _tx, _ty );
	}
    }
}

void HTMLTable::print( QPainter *_painter, int _tx, int _ty )
{
    print( _painter, 0, 0, 0xFFFF, 0xFFFF, _tx, _ty, false );
}

HTMLTable::~HTMLTable()
{
    unsigned int r, c;
    HTMLTableCell *cell;

    for ( r = 0; r < totalRows; r++ )
    {
	for ( c = 0; c < totalCols; c++ )
	{
	    if ( ( cell = cells[r][c] ) == 0 )
		continue;
	    if ( c < totalCols - 1 && cell == cells[r][c+1] )
		continue;
	    if ( r < totalRows - 1 && cells[r+1][c] == cell )
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
    head = tail = curr = 0;

    if ( percent > 0 )
    {
	width = max_width * percent / 100;
	setFixedWidth( false );
    }
    else if ( percent < 0 )
    {
	width = max_width;
	setFixedWidth( false );
    }
    else
	width = max_width;
}

HTMLClue::~HTMLClue()
{
    while ( head )
    {
	curr = head->next();
	delete head;
	head = curr;
    }
}

void HTMLClue::reset()
{
    HTMLObject *obj;

    for ( obj = head; obj != 0; obj = obj->next() )
	obj->reset();

    curr = 0;
}

void HTMLClue::calcAbsolutePos( int _x, int _y )
{
    HTMLObject *obj;

    int lx = _x + x;
    int ly = _y + y - ascent;

    for ( obj = head; obj != 0; obj = obj->next() )
	obj->calcAbsolutePos( lx, ly );
}

HTMLAnchor* HTMLClue::findAnchor( const char *_name, QPoint *_p )
{
    HTMLObject *obj;
    HTMLAnchor *ret;

    _p->setX( _p->x() + x );
    _p->setY( _p->y() + y - ascent );
    
    for ( obj = head; obj != 0; obj = obj->next() )
    {
	ret = obj->findAnchor( _name, _p );
	if ( ret != 0 )
	    return ret;
    }
    
    _p->setX( _p->x() - x );
    _p->setY( _p->y() - y + ascent );

    return 0;
}

void HTMLClue::getSelected( QStrList &_list )
{
    HTMLObject *obj;

    for ( obj = head; obj != 0; obj = obj->next() )
	obj->getSelected( _list );
}

void HTMLClue::getSelectedText( QString &_str )
{
    HTMLObject *obj;

    for ( obj = head; obj != 0; obj = obj->next() )
	obj->getSelectedText( _str );
}

void HTMLClue::select( bool _select )
{
    HTMLObject *obj;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	obj->select( _select );
    }
}

void HTMLClue::select( QPainter *_painter, QRegExp& _pattern, bool _select, int _tx, int _ty )
{
    HTMLObject *obj;

    _tx += x;
    _ty += y - ascent;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	obj->select( _painter, _pattern, _select, _tx, _ty );
    }
}

void HTMLClue::select( QPainter *_painter, bool _select, int _tx, int _ty )
{
    HTMLObject *obj;

    _tx += x;
    _ty += y - ascent;

    for ( obj = head; obj != 0; obj = obj->next() )
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
    {
	for ( obj = head; obj != 0; obj = obj->next() )
	    obj->select( _painter, TRUE, _tx, _ty );
    }
    else if ( !_rect.intersects( r ) )
    {
	for ( obj = head; obj != 0; obj = obj->next() )
	    obj->select( _painter, FALSE, _tx, _ty );
    }
    else
    {
	for ( obj = head; obj != 0; obj = obj->next() )
	    obj->select( _painter, _rect, _tx, _ty );
    }
}

void HTMLClue::selectByURL( QPainter *_painter, const char *_url, bool _select, int _tx, int _ty )
{
    HTMLObject *obj;

    _tx += x;
    _ty += y - ascent;

    for ( obj = head; obj != 0; obj = obj->next() )
	obj->selectByURL( _painter, _url, _select, _tx, _ty );
}

bool HTMLClue::selectText( QPainter *_painter, int _x1, int _y1,
	int _x2, int _y2, int _tx, int _ty )
{
    HTMLObject *obj;
    bool isSel = false;

    _tx += x;
    _ty += y - ascent;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	isSel = obj->selectText( _painter, _x1 - x, _y1 - ( y - ascent ),
		_x2 - x, _y2 - ( y - ascent ), _tx, _ty ) || isSel;
    }

    return isSel;
}

HTMLObject* HTMLClue::checkPoint( int _x, int _y )
{
    HTMLObject *obj;
    HTMLObject *obj2;

    if ( _x < x || _x > x + width || _y > y + descent || _y < y - ascent)
	return 0L;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	if ((obj2 = obj->checkPoint( _x - x, _y - (y - ascent) )) != 0L)
	    return obj2;
    }

    return 0;
}

void HTMLClue::calcSize( HTMLClue * )
{
    // If we have already called calcSize for the children, then just
    // continue from the last object done in previous call.
    if ( !curr )
    {
	ascent = 0;
	curr = head;
    }

    while ( curr != 0 )
    {
	curr->calcSize( this );
	curr = curr->next();
    }

    // remember the last object so that we can start from here next time
    // we are called.
    curr = tail;
}

void HTMLClue::recalcBaseSize( QPainter *_painter )
{
    HTMLObject *obj;

    for ( obj = head; obj != 0; obj = obj->next() )
	obj->recalcBaseSize( _painter );
}

int HTMLClue::calcMinWidth()
{
    HTMLObject *obj;
    int minWidth = 0;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	int w = obj->calcMinWidth();
	if ( w > minWidth )
	    minWidth = w;
    }

    if ( isFixedWidth() )
	return width > minWidth ? width : minWidth;

    return minWidth;
}

int HTMLClue::calcPreferredWidth()
{
    if ( isFixedWidth() )
	return width;

    HTMLObject *obj;
    int prefWidth = 0;

    for ( obj = head; obj != 0; obj = obj->next() )
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
	for ( obj = head; obj != 0; obj = obj->next() )
	    obj->setYPos( obj->getYPos() + ( _a - ascent )/2 );
    }
    else if ( valign == Bottom )
    {
	for ( obj = head; obj != 0; obj = obj->next() )
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

    for ( obj = head; obj != 0; obj = obj->next() )
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
    
    HTMLObject *obj;

    _tx += x;
    _ty += y - ascent;

#ifdef CLUE_DEBUG
    // draw rectangles around clues - for debugging
    QBrush brush;
    _painter->setBrush( brush );
    _painter->drawRect( _tx, _ty, width, getHeight() );
#endif

    for ( obj = head; obj != 0; obj = obj->next() )
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
    print( _painter, 0, 0, 0xFFFF, 0xFFFF, _tx, _ty, false );
}

void HTMLClue::print( QPainter *_painter, HTMLObject *_obj, int _x, int _y, int _width, int _height, int _tx, int _ty )
{
    if ( _y + _height < y - getAscent() || _y > y )
	return;
    
    HTMLObject *obj;

    _tx += x;
    _ty += y - ascent;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	if ( obj == _obj )
	{
	    obj->print( _painter, _x - x, _y - (y - getHeight()), _width,	
		_height, _tx, _ty, false );
	    return;
	}
	else
	    obj->print( _painter, _obj, _x - x, _y - (y - getHeight()),
		_width, _height, _tx, _ty );
    }
}

//-----------------------------------------------------------------------------

HTMLClueV::HTMLClueV( int _x, int _y, int _max_width, int _percent )
    : HTMLClue( _x, _y, _max_width, _percent )
{
    alignLeftList  = 0;
    alignRightList = 0;
}

void HTMLClueV::reset()
{
    HTMLClue::reset();

    alignLeftList  = 0;
    alignRightList = 0;
}

void HTMLClueV::setMaxWidth( int _max_width )
{
    HTMLObject *obj;

    if ( !isFixedWidth() )
    {
	max_width = _max_width;
	if ( percent > 0 )
	    width = _max_width * percent / 100;
	else
	    width = max_width;
    }

    for ( obj = head; obj != 0; obj = obj->next() )
	obj->setMaxWidth( width );
}

HTMLObject* HTMLClueV::checkPoint( int _x, int _y )
{
    HTMLObject *obj2;

    if ( ( obj2 = HTMLClue::checkPoint( _x, _y ) ) != 0L )
	    return obj2;

    if ( _x < x || _x > x + width || _y > y + descent || _y < y - ascent)
	return 0L;

    HTMLClueAligned *clue;
    for ( clue = alignLeftList; clue != 0; clue = clue->nextClue() )
    {
	if ((obj2 = clue->checkPoint( _x - x - clue->parent()->getXPos(),
		 _y - (y - ascent) - ( clue->parent()->getYPos() -
		 clue->parent()->getAscent() ) )) != 0L)
	    return obj2;
    }
    for ( clue = alignRightList; clue != 0; clue = clue->nextClue() )
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
	int lmargin = parent ? parent->getLeftMargin( getYPos() ) : 0;

	// If we have already called calcSize for the children, then just
	// continue from the last object done in previous call.
	if ( curr )
	{
	    ascent = 0;
	    // get the current ascent not including curr
	    HTMLObject *obj = head;
	    while ( obj != curr )
	    {
		ascent += obj->getHeight();
		obj = obj->next();
	    }
	    // remove any aligned objects previously added by the current
	    // object.
	    removeAlignedByParent( curr );
	}
	else
	{
	    ascent = descent = 0;
	    curr = head;
	}

	while ( curr != 0 )
	{
	    // Set an initial ypos so that the alignment stuff knows where
	    // the top of this object is
	    curr->setYPos( ascent );
	    curr->calcSize( this );
	    if ( curr->getWidth() > width )
		    width = curr->getWidth();
	    ascent += curr->getHeight();
	    curr->setPos( lmargin, ascent - curr->getDescent() );
	    curr = curr->next();
	}

	// remember the last object so that we can start from here next time
	// we are called.
	curr = tail;
/*
	if (max_width != 0 && width < max_width)
		width = max_width;
*/
	HTMLObject *obj;

	if ( halign == HCenter )
	{
	    for ( obj = head; obj != 0; obj = obj->next() )
		obj->setXPos( lmargin + (width - obj->getWidth()) / 2 );
	}
	else if ( halign == Right )
	{
	    for ( obj = head; obj != 0; obj = obj->next() )
		obj->setXPos( lmargin + width - obj->getWidth() );
	}

	HTMLClueAligned *clue;
	for ( clue = alignLeftList; clue != 0; clue = clue->nextClue() )
	{
	    if ( clue->getYPos() + clue->parent()->getYPos() > ascent )
		ascent = clue->getYPos() + clue->parent()->getYPos();
	}
	for ( clue = alignRightList; clue != 0; clue = clue->nextClue() )
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
    
    _tx += x;
    _ty += y - ascent;

    HTMLClueAligned *clue;

    for ( clue = alignLeftList; clue != 0; clue = clue->nextClue() )
    {
	clue->print( _painter, _tx + clue->parent()->getXPos(),
		_ty + clue->parent()->getYPos() - clue->parent()->getAscent() );
    }

    for ( clue = alignRightList; clue != 0; clue = clue->nextClue() )
    {
	clue->print( _painter, _tx + clue->parent()->getXPos(),
		_ty + clue->parent()->getYPos() - clue->parent()->getAscent() );
    }

    return rv;
}

void HTMLClueV::appendLeftAligned( HTMLClueAligned *_clue )
{
    if ( !alignLeftList )
    {
	alignLeftList = _clue;
    }
    else
    {
	HTMLClueAligned *obj = alignLeftList;
	while ( obj->nextClue() )
	{
	    if ( obj == _clue )
		return;
	    obj = obj->nextClue();
	}
	if ( obj == _clue )
	    return;
	obj->setNextClue( _clue );
	_clue->setNextClue( 0 );
    }
}

void HTMLClueV::appendRightAligned( HTMLClueAligned *_clue )
{
    if ( !alignRightList )
    {
	alignRightList = _clue;
    }
    else
    {
	HTMLClueAligned *obj = alignRightList;
	while ( obj->nextClue() )
	{
	    if ( obj == _clue )
		return;
	    obj = obj->nextClue();
	}
	if ( obj == _clue )
	    return;
	obj->setNextClue( _clue );
	_clue->setNextClue( 0 );
    }
}

// This is a horrible hack so that the progressive size calculation is
// not stuffed up by aligned clues added in a previous pass on the
// current clue
//
void HTMLClueV::removeAlignedByParent( HTMLObject *p )
{
    HTMLClueAligned *tmp, *obj;

    tmp = 0;
    obj = alignLeftList;

    while ( obj )
    {
	if ( obj->parent() == p )
	{
	    if ( tmp )
	    {
		tmp->setNextClue( obj->nextClue() );
		tmp = obj;
	    }
	    else
	    {
		alignLeftList = obj->nextClue();
		tmp = 0;
	    }
	}
	else
	    tmp = obj;
	obj = obj->nextClue();
    }

    tmp = 0;
    obj = alignRightList;

    while ( obj )
    {
	if ( obj->parent() == p )
	{
	    if ( tmp )
	    {
		tmp->setNextClue( obj->nextClue() );
		tmp = obj;
	    }
	    else
	    {
		alignRightList = obj->nextClue();
		tmp = 0;
	    }
	}
	else
	    tmp = obj;
	obj = obj->nextClue();
    }
}

int HTMLClueV::getLeftMargin( int _y )
{
    int margin = 0;
    HTMLClueAligned *clue;

    for ( clue = alignLeftList; clue != 0; clue = clue->nextClue() )
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

    for ( clue = alignRightList; clue != 0; clue = clue->nextClue() )
    {
	if ( clue->getYPos()-clue->getAscent()+clue->parent()->getYPos() -
		clue->parent()->getAscent() < _y &&
		clue->getYPos() + clue->parent()->getYPos() -
		clue->parent()->getAscent() > _y )
	    margin = clue->getXPos();
    }

    return margin;
}

int HTMLClueV::getLeftClear( int _y )
{
    int clearPos = _y;
    HTMLClueAligned *clue;

    for ( clue = alignLeftList; clue != 0; clue = clue->nextClue() )
    {
	if ( clue->getYPos() - clue->getAscent() + clue->parent()->getYPos() -
		    clue->parent()->getAscent() < _y &&
		    clue->getYPos() + clue->parent()->getYPos() -
		    clue->parent()->getAscent() > _y )
	{
	    int cp = clue->getYPos() + clue->parent()->getYPos() -
		clue->parent()->getAscent();
	    if ( cp > clearPos )
		clearPos = cp;
	}
    }

    return clearPos;
}

int HTMLClueV::getRightClear( int _y )
{
    int clearPos = _y;
    HTMLClueAligned *clue;

    for ( clue = alignRightList; clue != 0; clue = clue->nextClue() )
    {
	if ( clue->getYPos()-clue->getAscent()+clue->parent()->getYPos() -
		clue->parent()->getAscent() < _y &&
		clue->getYPos() + clue->parent()->getYPos() -
		clue->parent()->getAscent() > _y )
	{
	    int cp = clue->getYPos() + clue->parent()->getYPos() -
		clue->parent()->getAscent();
	    if ( cp > clearPos )
		clearPos = cp;
	}
    }

    return clearPos;
}

//-----------------------------------------------------------------------------

bool HTMLClueH::selectText( QPainter *_painter, int _x1, int _y1,
	int _x2, int _y2, int _tx, int _ty )
{
    HTMLObject *obj;
    bool isSel = false;
    int a = 0, d = 0;

    if ( !head )
	return false;

    _tx += x;
    _ty += y - ascent;

    // get max ascent and descent
    for ( obj = head; obj != 0; obj = obj->next() )
    {
	if ( obj->getAscent() > a )
	    a = obj->getAscent();
	if ( obj->getDescent() > d )
	    d = obj->getDescent();
    }

    int rely1 = _y1 - ( y - ascent );
    int rely2 = _y2 - ( y - ascent );
    int ypos = head->getYPos();

    if ( rely1 > ypos - a && rely1 < ypos + d )
	rely1 = ypos-1;

    if ( rely2 > ypos - a && rely2 < ypos + d )
	rely2 = ypos;

    // (de)select objects
    for ( obj = head; obj != 0; obj = obj->next() )
    {
	if ( obj->getObjectType() == Clue )
	    isSel = obj->selectText( _painter, _x1 - x, _y1 - (y-ascent),
		    _x2 - x, _y2 - ( y - ascent ), _tx, _ty ) || isSel;
	else
	    isSel = obj->selectText( _painter, _x1 - x, rely1,
		    _x2 - x, rely2, _tx, _ty ) || isSel;
    }

    return isSel;
}

void HTMLClueH::setMaxWidth( int _w )
{
    HTMLObject *obj;
    max_width = _w;

    // first calculate width minus fixed width objects
    for ( obj = head; obj != 0; obj = obj->next() )
    {
	if ( obj->getPercent() == 0 )	// i.e. fixed width objects
	    _w -= obj->getWidth();
    }

    // now call setMaxWidth for variable objects
    for ( obj = head; obj != 0; obj = obj->next() )
	if ( obj->getPercent() > 0 )
	    obj->setMaxWidth( _w - indent );
}

void HTMLClueH::calcSize( HTMLClue *parent )
{
    // make sure children are properly sized
    setMaxWidth( max_width );

    HTMLClue::calcSize( this );
    
    HTMLObject *obj;
    int lmargin = 0;
    
    if ( parent )
	lmargin = parent->getLeftMargin( getYPos() );

    width = lmargin + indent;
    descent = 0;
    ascent = 0;

    int a = 0;
    int d = 0;
    for ( obj = head; obj != 0; obj = obj->next() )
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
	    for ( obj = head; obj != 0; obj = obj->next() )
		obj->setYPos( obj->getAscent() );
	    break;

	case VCenter:
	    for ( obj = head; obj != 0; obj = obj->next() )
		obj->setYPos( ascent/2 );
	    break;

	default:
	    for ( obj = head; obj != 0; obj = obj->next() )
		obj->setYPos( ascent - d );
    }
}

int HTMLClueH::calcMinWidth()
{
    HTMLObject *obj;
    int minWidth = 0;

    for ( obj = head; obj != 0; obj = obj->next() )
	minWidth += obj->calcMinWidth();
    
    return minWidth + indent;
}
 
int HTMLClueH::calcPreferredWidth()
{
    HTMLObject *obj;
    int prefWidth = 0;

    for ( obj = head; obj != 0; obj = obj->next() )
	prefWidth += obj->calcPreferredWidth();
	
    return prefWidth + indent;
}
    
//-----------------------------------------------------------------------------

// process one line at a time, making sure that all objects on a line
// are able to be selected if the cursor is within the maximum
// ascent and descent of the line.
//
bool HTMLClueFlow::selectText( QPainter *_painter, int _x1, int _y1,
	int _x2, int _y2, int _tx, int _ty )
{
    HTMLObject *lineEnd = head, *obj = head;
    bool isSel = false;
    int ypos, a, d, rely1, rely2;

    _tx += x;
    _ty += y - ascent;

    while ( lineEnd )
    {
	ypos = lineEnd->getYPos();
	a = lineEnd->getAscent();
	d = lineEnd->getDescent();

	while ( lineEnd && lineEnd->getYPos() == ypos )
	{
	    if ( lineEnd->getAscent() > a )
		a = lineEnd->getAscent();
	    if ( lineEnd->getDescent() > d )
		d = lineEnd->getDescent();
	    lineEnd = lineEnd->next();
	}

	rely1 = _y1 - ( y - ascent );
	rely2 = _y2 - ( y - ascent );

	if ( rely1 > ypos - a && rely1 < ypos + d )
	    rely1 = ypos-1;

	if ( rely2 > ypos - a && rely2 < ypos + d )
	    rely2 = ypos;

	while ( obj != lineEnd )
	{
	    if ( obj->getObjectType() == Clue )
		isSel = obj->selectText( _painter, _x1 - x, _y1 - (y-ascent),
			_x2 - x, _y2 - ( y - ascent ), _tx, _ty ) || isSel;
	    else
		isSel = obj->selectText( _painter, _x1 - x, rely1,
			_x2 - x, rely2, _tx, _ty ) || isSel;
	    obj = obj->next();
	}
    }

    return isSel;
}

void HTMLClueFlow::getSelectedText( QString &_str )
{
    HTMLObject *obj;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	if ( obj != head || !obj->isSeparator() )
	    obj->getSelectedText( _str );
    }

    if ( tail && tail->isSelected() )
	_str += '\n';
}

// MRJ - 26/10/97:  I've given this a rewrite so that its layout is done
// correctly, i.e. break ONLY on separators and newlines instead of breaking
// after any object.  This is much nicer and smaller code now also.  Sorry
// if I broke something.
// 
void HTMLClueFlow::calcSize( HTMLClue *parent )
{
//    HTMLClue::calcSize( this );

    HTMLObject *obj = head;
    HTMLObject *line = head;
    HTMLVSpace::Clear clear = HTMLVSpace::CNone;;
    int lmargin, rmargin;

    ascent = 0;
    descent = 0;
    width = 0;
    lmargin = parent->getLeftMargin( getYPos() );
    if ( indent > lmargin )
	lmargin = indent;
    rmargin = parent->getRightMargin( getYPos() );
    int w = lmargin;
    int a = 0;
    int d = 0;
    int extra;

    bool newLine = false;

    while ( obj != 0 )
    {
	// If we get a newline object, set newLine=true so that the current
	// line will be aligned, and the next line prepared.
	if ( obj->isNewline() )
	{
	    if ( obj->getAscent() > a )
		a = obj->getAscent();
	    if ( obj->getDescent() > d )
		d = obj->getDescent();
	    newLine = true;
	    HTMLVSpace *vs = (HTMLVSpace *)obj;
	    clear = vs->clear();
	    obj = obj->next();
	}
	// add a separator
	else if ( obj->isSeparator() )
	{
	    if ( obj->getAscent() > a )
		a = obj->getAscent();

	    if ( obj->getDescent() > d )
		d = obj->getDescent();

	    obj->setXPos( w );

	    // skip a space at the start of a line
	    if ( w != lmargin )
		w += obj->getWidth();

	    obj = obj->next();
	}
	// a left or right aligned object is not added in this line.  It
	// is added to our parent's list of aligned objects and will be
	// taken into account in subsequent get*Margin() calls.
	else if ( obj->isAligned() )
	{
	    obj->calcSize();
	    HTMLClueAligned *c = (HTMLClueAligned *)obj;
	    if ( c->getHAlign() == Left )
		    parent->appendLeftAligned( c );
	    else
		    parent->appendRightAligned( c );
	    if ( w == lmargin )
	    {
		if ( c->getHAlign() == Left )
		    c->setPos( lmargin,
			ascent + c->getAscent() );
		else
		    c->setPos( rmargin - c->getWidth(),
			ascent + c->getAscent() );
		while ( obj != line )
		    line = line->next();
		line = line->next();
		lmargin = parent->getLeftMargin( getYPos() + 1 );
		if ( indent > lmargin )
		    lmargin = indent;
		rmargin = parent->getRightMargin( getYPos() + 1 );
		w = lmargin;
	    }
	    obj = obj->next();
	}
	// This is a normal object.  We must add all objects upto the next
	// separator/newline/aligned object.
	else
	{
	    int runWidth = 0;
	    HTMLObject *run = obj;
	    while ( run && !run->isSeparator() && !run->isNewline() &&
		    !run->isAligned() )
	    {
		run->setMaxWidth( rmargin - lmargin );
		run->calcSize();
		runWidth += run->getWidth();
		run = run->next();
	    }

	    // if these objects do not fit in the current line and we are
	    // not at the start of a line then end the current line in
	    // preparation to add this run in the next pass.
	    if ( w > lmargin && w + runWidth > rmargin )
	    {
		newLine = true;
	    }
	    else
	    {
		// If this run cannot fit in the allowed area.  Break into
		// individual objects.
		if ( runWidth > rmargin - lmargin )
		    run = obj->next();

		while ( obj != run )
		{
		    if ( obj->getAscent() > a )
			a = obj->getAscent();

		    if ( obj->getDescent() > d )
			d = obj->getDescent();

		    obj->setXPos( w );

		    w += obj->getWidth();

		    obj = obj->next();
		}
	    }
	}

	// if we need a new line, or all objects have been processed
	// and need to be aligned.
	if ( newLine || !obj )
	{
	    ascent += a + d;
	    y += a + d;

	    if ( w > width )
		width = w;

	    if ( halign == HCenter )
	    {
		extra = ( rmargin - w ) / 2;
		if ( extra < 0 )
		    extra = 0;
	    }
	    else if ( halign == Right )
	    {
	        extra = rmargin - w;
		if ( extra < 0 )
		    extra = 0;
	    }

	    while ( line != obj )
	    {
		if ( line->isAligned() )
		{
		    HTMLClue *c = (HTMLClue *)line;
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
		    line->setYPos( ascent - d );
		    line->setMaxAscent( a );
		    if ( halign == HCenter || halign == Right )
		    {
			line->setXPos( line->getXPos() + extra );
		    }
		}
		line = line->next();
	    }

	    int oldy = y;

	    if ( clear == HTMLVSpace::Left || clear == HTMLVSpace::All )
	    {
		y = parent->getLeftClear( getYPos() + 1 );
		ascent += y-oldy;
	    }
	    if ( clear == HTMLVSpace::Right || clear == HTMLVSpace::All )
	    {
		y = parent->getRightClear( getYPos() + 1 );
		ascent += y-oldy;
	    }

	    lmargin = parent->getLeftMargin( getYPos() + 1 );
	    if ( indent > lmargin )
		lmargin = indent;
	    rmargin = parent->getRightMargin( getYPos() + 1 );

	    w = lmargin;
	    d = 0;
	    a = 0;

	    newLine = false;
	    clear = HTMLVSpace::CNone;
	}
    }

    if ( width < max_width )
	    width = max_width;
}

#if 0
void HTMLClueFlow::calcSize( HTMLClue *parent )
{
    HTMLClue::calcSize( this );

    HTMLObject *obj = head;
    HTMLObject *line = head;
    int lmargin, rmargin;

    ascent = 0;
    descent = 0;
    width = 0;
    lmargin = parent->getLeftMargin( getYPos() );
    if ( indent > lmargin )
	lmargin = indent;
    rmargin = parent->getRightMargin( getYPos() );
    int w = lmargin;
    int a = 0;
    int d = 0;

    for ( ; obj != 0; obj = obj->next() )
    {
	// This left or right aligned with other objects to wrap around it
	if ( obj->isAligned() )
	{
	    HTMLClueAligned *c = (HTMLClueAligned *)obj;
	    if ( c->getHAlign() == Left )
		    parent->appendLeftAligned( c );
	    else
		    parent->appendRightAligned( c );
	    if ( w == lmargin )
	    {
		if ( c->getHAlign() == Left )
		    c->setPos( lmargin,
			ascent + c->getAscent() );
		else
		    c->setPos( rmargin - c->getWidth(),
			ascent + c->getAscent() );
		while ( obj != line )
		    line = line->next();
		line = line->next();
		lmargin = parent->getLeftMargin( getYPos() + 1 );
		if ( indent > lmargin )
		    lmargin = indent;
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

	    while ( line != obj )
	    {
		if ( line->isAligned() )
		{
		    HTMLClue *c = (HTMLClue *)line;
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
		    line->setYPos( ascent - d );
		    line->setMaxAscent( a );
		    if ( halign == HCenter )
			line->setXPos( line->getXPos()+
			    ( rmargin - w )/2 );
		    else if ( halign == Right )
			line->setXPos( line->getXPos()+
			    ( rmargin - w ) );
		}
		line = line->next();
	    }

	    lmargin = parent->getLeftMargin( getYPos() + 1 );
	    if ( indent > lmargin )
		lmargin = indent;
	    rmargin = parent->getRightMargin( getYPos() + 1 );

	    ascent += obj->getHeight();
	    obj->setPos( lmargin, ascent - obj->getDescent() );
	    line = line->next();

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

	    while ( line != obj )
	    {
		if ( line->isAligned() )
		{
		    HTMLClue *c = (HTMLClue *)line;
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
		    line->setYPos( ascent - d );
		    line->setMaxAscent( a );
		    if ( halign == HCenter )
			line->setXPos( line->getXPos()+ ( rmargin - w )/2 );
		    else if ( halign == Right )
			line->setXPos( line->getXPos()+ ( rmargin - w ) );
		}
		line = line->next();
	    }

	    lmargin = parent->getLeftMargin( getYPos() + 1 );
	    if ( indent > lmargin )
		lmargin = indent;
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

	while ( line != obj )
	{
	    if ( line->isAligned() )
	    {
		HTMLClue *c = (HTMLClue *)line;
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
		line->setYPos( ascent - d );
		line->setMaxAscent( a );
		if ( halign == HCenter )
		    line->setXPos( line->getXPos() + ( rmargin - w )/2 );
		else if ( halign == Right )
		    line->setXPos( line->getXPos() + ( rmargin - w ) );
	    }
	    line = line->next();
	}

	if ( w > width )
		width = w;
    }    

    if ( width < max_width )
	    width = max_width;
}
#endif

int HTMLClueFlow::findPageBreak( int _y )
{
    if ( _y > y )
	return -1;

    HTMLObject *obj;
    int pos, minpos, yp;

    for ( obj = head; obj != 0; obj = obj->next() )
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
	    obj = obj->next();
	}
	if ( minpos != yp )
	    return ( minpos + y - ascent );
    }

    return -1;
}

int HTMLClueFlow::calcMinWidth()
{
    HTMLObject *obj = head;
    int minWidth = 0;
    int ow, runWidth = 0;

    while ( obj )
    {
	if ( obj->isSeparator() || obj->isNewline() )
	{
	    runWidth = 0;
	}
	else
	{
	    ow = obj->calcMinWidth();

	    // we try not to grow larger than max_width by breaking at
	    // object boundaries if necessary.
	    if ( runWidth + ow > max_width )
		runWidth = 0;

	    runWidth += ow;

	    if ( runWidth > minWidth )
		minWidth = runWidth;
	}

	obj = obj->next();
    }

    if ( isFixedWidth() )
	return ( width > minWidth ? width : minWidth ) + indent;

    return minWidth + indent;
}

int HTMLClueFlow::calcPreferredWidth()
{
    HTMLObject *obj;
    int maxw = 0, w = 0;

    for ( obj = head; obj != 0; obj = obj->next() )
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

    return maxw + indent;
}

void HTMLClueFlow::setMaxWidth( int _max_width )
{
    max_width = _max_width;

    HTMLObject *obj;

    for ( obj = head; obj != 0; obj = obj->next() )
	obj->setMaxWidth( max_width - indent );
}


//-----------------------------------------------------------------------------


void HTMLClueAligned::setMaxWidth( int _max_width )
{
    max_width = _max_width;

    HTMLObject *obj;

    for ( obj = head; obj != 0; obj = obj->next() )
	obj->setMaxWidth( max_width );
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

    for ( obj = head; obj != 0; obj = obj->next() )
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

