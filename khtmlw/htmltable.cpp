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

#include "htmlchain.h"
#include "htmlobj.h"
#include "htmlclue.h"
#include "htmltable.h"
#include "html.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <qimage.h>
#include <qdrawutil.h>

//#define CLUE_DEBUG
//#define DEBUG_ALIGN

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

    max_width = _max_width;

    if (max_width > 0)
    {
	if ( percent > 0 )
            width = _max_width * percent / 100;
	else if ( !isFixedWidth() )
	    width = max_width;
    }

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

    if ( isFixedWidth() )
    {
        if (width > minWidth)
            minWidth = width;
    }
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

void HTMLTableCell::print( QPainter *_painter, HTMLChain *_chain, int _x,
	int _y, int _width, int _height, int _tx, int _ty )
{
    if ( bg.isValid() )
    {
	    int top = _y - ( y - getAscent() );
	    int bottom = top + _height;
	    if ( top < -padding )
		    top = -padding;
	    if ( bottom > getAscent() + padding )
		    bottom = getAscent() + padding;

	    int left = _x - x;
	    int right = left + _width;
	    if ( left < -padding )
		left = -padding;
	    if ( right > width + padding )
		right = width + padding;

	    QBrush brush( bg );
	    _painter->fillRect( _tx + x + left, _ty + y - ascent + top,
		    right - left, bottom - top, brush );
    }

    HTMLClue::print( _painter, _chain, _x, _y, _width, _height, _tx, _ty );
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
    {
//WABA does this make sense? 
    	max_width = width; 
	setFixedWidth( TRUE );
    }
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
//WABA: Why not? Let's give crappy HTML a chance
//    if ( row != 0 && endCol > totalCols )
//	endCol = totalCols;

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
//    totalRows = row;

    // calculate min/max widths
    calcColInfo();

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

bool HTMLTable::getObjectPosition( const HTMLObject *objp, int &xp, int &yp )
{
    HTMLTableCell *cell;

    xp += x;
    yp += (y - ascent);

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

	    if ( cell->getObjectPosition( objp, xp, yp ) )
		return true;
	}
    }
    
    xp -= x;
    yp -= (y - ascent);

    return false;
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
    calcColInfo();

    // recalculate min/max widths
    calcColumnWidths();

    // If it doesn't fit... MAKE IT FIT!
    for ( c = 0; c < totalCols; c++ )
    {
        if (columnPos[c+1] > max_width-border)
             columnPos[c+1] = max_width-border;
    }

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
	    {
		continue;
	    }
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
		colPos = cell->calcPreferredWidth();
		colPos += columnPrefPos[indx] + padding + padding + 
			  spacing + borderExtra;
		if ( columnPrefPos[c + 1] < colPos )
		    columnPrefPos[c + 1] = colPos;
	    }

	    if ( columnPrefPos[c + 1] < columnPos[c + 1] )
		columnPrefPos[c + 1] = columnPos[c + 1];
	}
	if (columnPrefPos[c + 1] <= columnPrefPos[c])
	    columnPrefPos[c + 1] = columnPrefPos[c]+1;
    }
}

// Use the minimum and preferred cell widths to produce an optimum
// cell spacing.  When this has been done columnOpt contains
// the optimum cell widths.
void HTMLTable::optimiseCellWidth()
{
    int tableWidth = width - border;
    int addSize = 0;
	
    columnOpt = columnPos.copy();

    if (tableWidth > columnPos[ totalCols ] )
    {
	// We have some space to spare
	addSize = tableWidth - columnPos[ totalCols];

	if ((percent <= 0) && (!isFixedWidth()))
	{
	    // Variable width Table, 
	    if (columnPrefPos[totalCols] < tableWidth)
	    {
	        // don't scale beyond preferred width.
	        addSize = columnPrefPos[totalCols] - 
	                  columnPos[ totalCols];
	    }
	}
    }
    
    if (addSize > 0)
    {
        scaleColumns(0, totalCols-1, addSize);
    }
}

void HTMLTable::scaleColumns(unsigned int c_start, unsigned int c_end, int tooAdd)
{
    unsigned int r, c;
    int colspan;
    int addSize;
    int minWidth, prefWidth;
    int totalAllowed, totalRequested;
    int borderExtra = ( border == 0 ) ? 0 : 1;
    int tableWidth = width - border;
     
    int *prefColumnWidth;

    // satisfy fixed width cells
    for ( colspan = 0; colspan <= 1; colspan++)
    {
        for ( r = 0; r < totalRows; r++ )
        {
            for ( c = c_start; c <= c_end; c++ )
            {
                HTMLTableCell *cell = cells[r][c];
                
                if (cell == 0L)
                    continue;
                
                if ( r < totalRows - 1 && cells[r+1][c] == cell )
                    continue;

		/* Fixed cells only */
                if ( !cell->isFixedWidth() )
                    continue;

                if (colspan == 0)	
                {
                    // colSpan == 1
                    if (cell->colSpan() != 1)
                        continue;
                }
                else
                {
                    // colSpan > 1
                    if (cell->colSpan() <= 1)
                        continue;
                    if ( c < totalCols - 1 && cells[r][c+1] == cell )
                        continue;
                }                                                                  

                minWidth = columnOpt[c+1] - columnOpt[c+1-cell->colSpan()];
                prefWidth = cell->getWidth() + padding +
                            padding + spacing + borderExtra;
                if (prefWidth <= minWidth)
                    continue;

                addSize = (prefWidth - minWidth);

                tooAdd -= addSize;
                                
                if (colspan == 0)
                {
                    unsigned int c1;
                    // Just add this to the column size
                    for ( c1 = c+1; c1 <= totalCols; c1++ )
                        columnOpt[c1] += addSize;
                }
                else
                {
                    unsigned int c_b = c+1-cell->colSpan();

                    // Some end-conditions are required here to prevent looping
                    if (c_b < c_start)
                        continue;
                    if ((c_b == c_start) && (c == c_end))
                        continue;

                    // scale the columns covered by 'cell' first
                    scaleColumns(c_b, c, addSize);

                } // if colspan
            } // for c
        } // for r
    } // for colspan        

    // satisfy percentage width cells
        
    for ( r = 0; r < totalRows; r++ )
    {
        totalRequested = 0;

        if (tooAdd <= 0)                // No space left!
        {
            return;
        }

        /* first calculate how much we would like to add in this row */
        for ( c = c_start; c <= c_end; c++ )
        {
            HTMLTableCell *cell = cells[r][c];

            if (cell == 0L)
                continue;
            if ( r < totalRows - 1 && cells[r+1][c] == cell )
                continue;
            if ( c < totalCols - 1 && cells[r][c+1] == cell )
                continue;
                        
            /* Percentage cells only */
            if ( cell->getPercent() <= 0 )
                continue;

            // Only cells with a colspan which fits within c_begin .. c_start
            if (cell->colSpan() > 1)
            {
                unsigned int c_b = c+1-cell->colSpan();

                if (c_b < c_start)
                    continue;
                if ((c_b == c_start) && (c == c_end))
                    continue;
            }

            minWidth = columnOpt[c+1] - columnOpt[c+1-cell->colSpan()];
            prefWidth = tableWidth * cell->getPercent() / 100 + padding +
                        padding + spacing + borderExtra;

            if (prefWidth <= minWidth)
                continue;
                                        
            totalRequested += (prefWidth - minWidth);

        } // for c
                
        if (totalRequested == 0)  // Nothing to do
            continue;

        totalAllowed = tooAdd;
                
        // Do the actual adjusting of the percentage cells
                
        for ( colspan = 0; colspan <= 1; colspan++)
        {
            for ( c = c_start; c <= c_end; c++ )
            {
                HTMLTableCell *cell = cells[r][c];
                                
                if (cell == 0L)
                    continue;
                if ( c < totalCols - 1 && cells[r][c+1] == cell )
                    continue;
                if ( r < totalRows - 1 && cells[r+1][c] == cell )
                    continue;
                                
                /* Percentage cells only */
                if ( cell->getPercent() <= 0 )
                    continue;

                // Only cells with a colspan which fits within c_begin .. c_start
                if (cell->colSpan() > 1)
                {
                    unsigned int c_b = c+1-cell->colSpan();

                    if (colspan == 0)
                        continue;
        
                    if (c_b < c_start)
                        continue;
                    if ((c_b == c_start) && (c == c_end))
                        continue;
                }
                else
                {
                    if (colspan != 0)
                        continue;
                }
            
                minWidth = columnOpt[c+1] - columnOpt[c+1-cell->colSpan()];
                prefWidth = tableWidth * cell->getPercent() / 100 + padding +
                            padding + spacing + borderExtra;

                if (prefWidth <= minWidth)
                    continue;
                                        
                addSize = (prefWidth - minWidth);
                       
                if (totalRequested > totalAllowed) // We can't honour the request, scale it
                {
                    addSize = addSize * totalAllowed / totalRequested;
                    totalRequested -= (prefWidth - minWidth);
                    totalAllowed -= addSize;
                }

                tooAdd -= addSize;

                if (colspan == 0)
                {
                    unsigned int c1;
                    // Just add this to the column size
                    for ( c1 = c+1; c1 <= totalCols; c1++ )
                         columnOpt[c1] += addSize;
                }
                else
                {
                    unsigned int c_b = c+1-cell->colSpan();
                    // Some end-conditions are required here to prevent looping
                    if (c_b < c_start)
                        continue;
                    if ((c_b == c_start) && (c == c_end))
                        continue;
                                        
                    // scale the columns covered by 'cell' first
                    scaleColumns(c_b, c, addSize);
                } // if colspan
            } // for c
        } // for colspan        
    } // for r

    // This should not be calculated on a row by row basis
        
    totalRequested = 0;

    if (tooAdd <= 0)                // No space left!
    {
        return;
    }

    prefColumnWidth = new int [totalCols];
        
    /* first calculate how much we would like to add in each column */
    for ( c = c_start; c <= c_end; c++ )
    {
        minWidth = columnOpt[c+1] - columnOpt[c];
        prefColumnWidth[c] = minWidth;
        for ( r = 0; r < totalRows; r++ )
        {
            int prefCellWidth;
            HTMLTableCell *cell = cells[r][c];
                              
            if (cell == 0L)
                continue;
            if ( r < totalRows - 1 && cells[r+1][c] == cell )
                continue;

            // Parse colspan-cells in each colomn!

            if ( c < totalCols - 1 && cells[r][c+1] == cell )
            {        
                // Find the last cell
                unsigned int c1 = c+2;
                while ((c1 < totalCols) && (cells[r][c1] == cell))
                    c1++;
                               
                minWidth = columnOpt[c1] - columnOpt[c1 + 1 - cell->colSpan()];
            }
            else
            {        
                minWidth = columnOpt[c+1] - columnOpt[c];
            }                 

            if (cell->isFixedWidth())
            { // fixed width
                prefCellWidth = cell->getWidth() + padding +
                                padding + spacing + borderExtra;
            }
            else if (cell->getPercent() > 0)
            { // percentage width
                prefCellWidth = tableWidth * cell->getPercent() / 100 + padding +
                                padding + spacing + borderExtra;
            }
            else
            { // variable width
                prefCellWidth = cell->calcPreferredWidth() + padding +
                                padding + spacing + borderExtra;
            }
            prefCellWidth = prefCellWidth / cell->colSpan();

            if (prefCellWidth > prefColumnWidth[c])
                prefColumnWidth[c] = prefCellWidth;
        }
        
        if (prefColumnWidth[c] > minWidth)
        {
            totalRequested += (prefColumnWidth[c] - minWidth);
        }
        else
        {
            prefColumnWidth[c] = 0;
        }

    } // for c
                
    if (totalRequested > 0)  // Nothing to do
    {
        totalAllowed = tooAdd;
                
        // Do the actual adjusting of the variable width cells
                
        for ( c = c_start; c <= c_end; c++ )
        {
            unsigned int c1;
                       
            minWidth = columnOpt[c+1] - columnOpt[c];
            prefWidth = prefColumnWidth[c];
            if (prefWidth <= minWidth)
                continue;
                                        
            addSize = (prefWidth - minWidth);
                                
            if (totalRequested > totalAllowed) // We can't honour the request, scale it
            {        
                addSize = addSize * totalAllowed / totalRequested;
                totalRequested -= (prefWidth - minWidth);
                totalAllowed -= addSize;
            }
                                        
            tooAdd -= addSize;
                                
            // Just add this to the column size
            for ( c1 = c+1; c1 <= totalCols; c1++ )
                columnOpt[c1] += addSize;
        } // for c
    }
    delete [] prefColumnWidth;

    // Spread the remaining space equally across all columns
    if (tooAdd > 0)
    {        
        for( c = c_start; c <= c_end; c++)
        {
            unsigned int c1;

            addSize = tooAdd / (1 + c_end - c);
            tooAdd -= addSize;
            for ( c1 = c+1; c1 <= totalCols; c1++ )
            {
                columnOpt[c1] += addSize;
            }
        }
    }        
}

// New table layout function
void HTMLTable::addRowInfo(int _row, int _colInfoIndex)
{
    rowInfo[_row].entry[rowInfo[_row].nrEntries++] = _colInfoIndex;
}

// New table layout function
int HTMLTable::addColInfo(int _startCol, int _colSpan, 
                          int _minSize, int _prefSize, 
                          int _maxSize, ColType _colType)
{
    unsigned int indx;

    // Is there already some info present?
    for(indx = 0; indx < totalColInfos; indx++)
    {
       if ((colInfo[indx].startCol == _startCol) &&
           (colInfo[indx].colSpan == _colSpan))
           break;
    } 
    if (indx == totalColInfos)
    {
       // No colInfo present allocate some
       totalColInfos++;
       if (totalColInfos >= colInfo.size())
       {
           colInfo.resize( colInfo.size() + totalCols );
       }
       colInfo[indx].startCol = _startCol;
       colInfo[indx].colSpan = _colSpan;
       colInfo[indx].minSize = _minSize;
       colInfo[indx].prefSize = _prefSize;
       colInfo[indx].maxSize = _maxSize;
       colInfo[indx].colType = _colType;
    }
    else
    {
       if (_minSize > colInfo[indx].minSize)
           colInfo[indx].minSize = _minSize;

       // Fixed < Percent < Variable 
       if (_colType < colInfo[indx].colType)
       {
           colInfo[indx].prefSize = _prefSize;
       }
       else if (_colType == colInfo[indx].colType)
       {
           if (_prefSize > colInfo[indx].prefSize)
               colInfo[indx].prefSize = _prefSize;
       }
    }
    return (indx); /* Return the ColInfo Index */
}
                        
// New table layout function
//
// Both the minimum and preferred column sizes are calculated here.
// The hard part is choosing the actual sizes based on these two.
void HTMLTable::calcColInfo()
{
    unsigned int r, c;
    int borderExtra = ( border == 0 ) ? 0 : 1;

    // Allocate some memory for column info
    colInfo.resize( totalCols*2 );
    rowInfo = (RowInfo_t *) malloc( totalRows * sizeof(RowInfo_t) );
    totalColInfos = 0;
    
    for ( r = 0; r < totalRows; r++ )
    {
	rowInfo[r].entry = (int *) malloc( totalCols * sizeof(int));
	rowInfo[r].nrEntries = 0;
        for ( c = 0; c < totalCols; c++ )
	{
	    HTMLTableCell *cell = cells[r][c];
	    int            min_size;
	    int            pref_size;
	    int            colInfoIndex;
	    ColType        col_type;

	    if ( cell == 0 )
		continue; 
	    if ( (c > 0) && (cells[r][c-1] == cell) )
		continue;
	    if ( (r > 0) && (cells[r-1][c] == cell) )
		continue;

	    // calculate minimum size
	    min_size = cell->calcMinWidth() + padding + padding + 
	              spacing + borderExtra;

	    // calculate preferred pos
	    if ( cell->getPercent() > 0 )
	    {
		pref_size = ( max_width * cell->getPercent() / 100 ) + 
		           padding + padding + spacing + borderExtra;
		col_type = Percent;
	    }
	    else if ( cell->isFixedWidth() )
	    {
		pref_size = cell->getWidth() + padding + padding + 
		            spacing + borderExtra;
		col_type = Fixed;
	    }
	    else
	    {
		pref_size = cell->calcPreferredWidth() + 
		            padding + padding + spacing + borderExtra;
		col_type = Variable;
	    }
	    colInfoIndex = addColInfo(c, cell->colSpan(), min_size, 
	                              pref_size, max_width, col_type);
	    addRowInfo(r, colInfoIndex);
	}
    }

    // Remove redundant rows
    unsigned int i,j;
    unsigned int totalRowInfos;
    totalRowInfos = 1;
    for(i = 1; i < totalRows; i++)
    {
    	bool unique = TRUE;
    	for(j = 0; (j < totalRowInfos) && (unique == TRUE); j++)
    	{
    	    unsigned k;
    	    if (rowInfo[i].nrEntries == rowInfo[j].nrEntries)
    	        unique = FALSE;
    	    else
    	    {
    	        bool match = TRUE;
    	        k = rowInfo[i].nrEntries;
    	        while (k--)
    	        {
    	            if (rowInfo[i].entry[k] != rowInfo[j].entry[k])
    	            {
    	            	match = FALSE;
    	            	break;
    	            }
    	        }
    	        if (match)
    	            unique = FALSE;
    	    }
    	}
    	if (!unique)
    	{
    	     free( rowInfo[i].entry);
    	}
    	else 
    	{
    	    if (totalRowInfos != i)
    	    {
    	        rowInfo[totalRowInfos].entry = rowInfo[i].entry;
    	        rowInfo[totalRowInfos].nrEntries = rowInfo[i].nrEntries;
    	    }
    	    totalRowInfos++;
    	}
    }

    // Calculate pref width and min width for each row
    
    _minWidth = 0;
    _prefWidth = 0;
    for(i = 0; i < totalRowInfos; i++)
    {
        int min = 0;
        int pref = 0;
        for(j = 0; j < rowInfo[i].nrEntries; j++)
        {
           int index = rowInfo[i].entry[j];
           min += colInfo[index].minSize;
           pref += colInfo[index].prefSize;
	}
	rowInfo[i].minSize = min;
	rowInfo[i].prefSize = pref;
	if (_minWidth < min)
	{
	    _minWidth = min;
	}
	if (_prefWidth < pref)
	{
	    _prefWidth = pref;
	}
    }
    
    // DEBUG: Show the results :)
#if 0    
    printf("---- %d ----\n", totalColInfos);
    for(i = 0; i < totalColInfos; i++)
    {
        printf("col #%d: %d - %d, min: %3d pref: %3d max: %3d type: %d\n",
                 i,
                 colInfo[i].startCol, colInfo[i].colSpan,
                 colInfo[i].minSize, colInfo[i].prefSize,
                 colInfo[i].maxSize, (int) colInfo[i].colType);
    }
    for(i = 0; i < totalRowInfos; i++)
    {
        printf("row #%d: ", i);
        for(j = 0; j < rowInfo[i].nrEntries; j++)
        {
           if (j == 0)
              printf("%d", rowInfo[i].entry[j]);
           else
              printf("- %d", rowInfo[i].entry[j]);
        } 
        printf(" ! %d : %d\n", rowInfo[i].minSize, rowInfo[i].prefSize);
    }
#endif
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
//    return columnPos[totalCols] + border;
    return _minWidth;
}

int HTMLTable::calcPreferredWidth()
{
//    return columnPrefPos[totalCols] + border;
    return _prefWidth;
}

void HTMLTable::setMaxWidth( int _max_width )
{
    if (!isFixedWidth())
    {
        max_width = _max_width;

        if ( percent > 0 )
            width = max_width * percent / 100;
        else 
            width = max_width;
        calcColumnWidths();
    }
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

HTMLObject *HTMLTable::mouseEvent( int _x, int _y, int button, int state )
{
    unsigned int r, c;
    HTMLObject *obj;
    HTMLTableCell *cell;

    if ( _x < x || _x > x + width || _y > y + descent || _y < y - ascent)
	return 0;

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

	    if ( ( obj = cell->mouseEvent( _x-x, _y-(y - ascent), button,
		    state ) ) != 0 )
		return obj;
	}
    }

    return 0;
}

void HTMLTable::selectByURL( KHTMLWidget *_htmlw, HTMLChain *_chain,
    const char *_url, bool _select, int _tx, int _ty )
{
    unsigned int r, c;
    HTMLTableCell *cell;

    _tx += x;
    _ty += y - ascent;

    _chain->push( this );

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

	    cell->selectByURL( _htmlw, _chain, _url, _select, _tx, _ty );
	}
    }

    _chain->pop();
}

void HTMLTable::select( KHTMLWidget *_htmlw, HTMLChain *_chain,
    QRegExp& _pattern, bool _select, int _tx, int _ty )
{
    unsigned int r, c;
    HTMLTableCell *cell;

    _tx += x;
    _ty += y - ascent;

    _chain->push( this );

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

	    cell->select( _htmlw, _chain, _pattern, _select, _tx, _ty );
	}
    }

    _chain->pop();
}

void HTMLTable::select( KHTMLWidget *_htmlw, HTMLChain *_chain,
    bool _select, int _tx, int _ty )
{
    unsigned int r, c;
    HTMLTableCell *cell;

    _tx += x;
    _ty += y - ascent;

    _chain->push( this );

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

	    cell->select( _htmlw, _chain, _select, _tx, _ty );
	}
    }

    _chain->pop();
}

void HTMLTable::select( KHTMLWidget *_htmlw, HTMLChain *_chain,
    QRect & _rect, int _tx, int _ty )
{
    unsigned int r, c;
    HTMLTableCell *cell;

    _tx += x;
    _ty += y - ascent;

    _chain->push( this );

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

	    cell->select( _htmlw, _chain, _rect, _tx, _ty );
	}
    }

    _chain->pop();
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

bool HTMLTable::selectText( KHTMLWidget *_htmlw, HTMLChain *_chain,
	int _x1, int _y1, int _x2, int _y2, int _tx, int _ty )
{
    bool isSel = false;
    unsigned int r, c;
    HTMLTableCell *cell;

    _tx += x;
    _ty += y - ascent;

    _chain->push( this );

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
		isSel = cell->selectText( _htmlw, _chain, 0, _y1 - ( y - ascent ),
			width + 1, _y2 - ( y - ascent ), _tx, _ty ) || isSel;
	    }
	    else if ( _y1 < y - ascent )
	    {
		isSel = cell->selectText( _htmlw, _chain, 0, _y1 - ( y - ascent ),
			_x2 - x, _y2 - ( y - ascent ), _tx, _ty ) || isSel;
	    }
	    else if ( _y2 > y )
	    {
		isSel = cell->selectText( _htmlw, _chain, _x1 - x,
			_y1 - ( y - ascent ), width + 1, _y2 - ( y - ascent ),
			_tx, _ty ) || isSel;
	    }
	    else if ( (_x1 - x < cell->getXPos() + cell->getWidth() &&
			_x2 - x > cell->getXPos() ) )
	    {
		isSel = cell->selectText( _htmlw, _chain, _x1 - x,
			_y1 - ( y - ascent ), _x2 - x, _y2 - ( y - ascent ),
			_tx, _ty ) || isSel;
	    }
	    else
	    {
		cell->selectText( _htmlw, _chain, 0, 0, 0, 0, _tx, _ty );
	    }
	}
    }

    _chain->pop();

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

void HTMLTable::findCells( int _tx, int _ty, QList<HTMLCellInfo> &_list )
{
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
	    cell->findCells( _tx, _ty, _list );
	}
    }
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

void HTMLTable::print( QPainter *_painter, HTMLChain *_chain,int _x,
	int _y, int _width, int _height, int _tx, int _ty )
{
    _tx += x;
    _ty += y - ascent;

    _chain->next();

    if ( _chain->current() )
    {
	_chain->current()->print( _painter, _chain, _x - x,
	    _y - (y - getHeight()), _width, _height, _tx, _ty );
    }
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

