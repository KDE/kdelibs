/* 
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)
              (C) 1998 Waldo Bastian (bastian@kde.org)

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
// KDE HTML Widget -- Tables
// $Id$

#include <kurl.h>

#include "khtmlchain.h"
#include "khtmliter.h"
#include "khtmlobj.h"
#include "khtmlclue.h"
#include "khtmltable.h"
#include "khtml.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <qimage.h>
#include <qdrawutil.h>

//#define CLUE_DEBUG
//#define DEBUG_ALIGN

//-----------------------------------------------------------------------------

HTMLTableCell::HTMLTableCell( int _percent, int _width, int rs, int cs, int pad )
	 : HTMLClueV( _percent, _width )
{
	rspan = rs;
	cspan = cs;
	padding = pad;
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
			max_width + padding * 2, bottom - top, brush );
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

void HTMLTableCell::setMaxWidth( int _max_width)
{
    // HTMLTableCells don't do their own width-calculation.
    // It is done by HTMLTable instead.
    // We just do whatever the HTMLTable wants.
    max_width = _max_width;
}
                                                        
void HTMLTableCell::calcSize( HTMLClue *parent )
{
    HTMLClueV::calcSize(parent);

    // HTMLTableCells don't do their own width-calculation.
    // It is done by HTMLTable instead.
    // We just do whatever the HTMLTable wants.
    width = max_width;
}
//-----------------------------------------------------------------------------

HTMLTable::HTMLTable( int _percent, int _width,
	int _padding, int _spacing, int _border ) : HTMLObject()
{
    percent = _percent;
    if (percent == 0)
        fixed_width = _width;
    else
        fixed_width = UNDEFINED;
                         
    padding = _padding;
    spacing = _spacing;
    border  = _border;
    caption = 0L;

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
}

HTMLIterator *HTMLTable::getIterator()
{ 
    return totalRows ? new HTMLTableIterator( this ) : 0; 
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
    calcColInfo(1);

//    calcColumnWidths();
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
    calcColInfo(1);
}

void HTMLTable::calcSize( HTMLClue * )
{
    unsigned int r, c;
    int indx;
    HTMLTableCell *cell;

    // recalculate min/max widths
//    calcColumnWidths();

    // Do the final layout based on max_width
    calcColInfo(2);

    // If it doesn't fit... MAKE IT FIT!
    for ( c = 0; c < totalCols; c++ )
    {
        if (columnPos[c+1] > max_width-border)
        {
             printf("WARNING: Column too small!\n");
             columnPos[c+1] = max_width-border;
	}
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
	    cell->calcSize();
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

    // This is the one and only place 
    // where the actual size of the table is set!!!!
    width = columnOpt[ totalCols ] + border;
    ascent = rowHeights[ totalRows ] + border;
    if ( caption )
	ascent += caption->getHeight();
}

// Use the minimum and preferred cell widths to produce an optimum
// cell spacing.  When this has been done columnOpt contains
// the optimum cell widths.
void HTMLTable::optimiseCellWidth()
{
    int tableWidth;
    int addSize = 0;
	
    if (percent == UNDEFINED)
    {
        tableWidth = max_width;
    }
    else if (percent == 0)
    {
    	// Fixed with
        tableWidth = fixed_width;
    }	
    else
    {
        tableWidth = (percent * max_width) / 100;
    }	

    if (tableWidth < min_width)
        tableWidth = min_width;

    tableWidth -= border;
	
    columnOpt = columnPos.copy();

    if (tableWidth > columnPos[ totalCols ] )
    {
	// We have some space to spare
	addSize = tableWidth - columnPos[ totalCols];

	if (percent == UNDEFINED)
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
                if ( cell->getPercent() != 0 )
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
            int cellPercent;
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
            cellPercent = cell->getPercent();      

            if (cellPercent == UNDEFINED)
            { // variable width
                prefCellWidth = cell->calcPreferredWidth() + padding +
                                padding + spacing + borderExtra;
            }
            else if (cellPercent == 0)
            { // fixed width
                prefCellWidth = cell->getWidth() + padding +
                                padding + spacing + borderExtra;
            }
            else 
            { // percentage width
                prefCellWidth = tableWidth * cell->getPercent() / 100 + padding +
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

    if (_startCol + _colSpan > (int) totalCols)
    	_colSpan = totalCols - _startCol;

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
void HTMLTable::calcColInfo( int pass )
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
	    int            cellPercent;
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
	
	    cellPercent = cell->getPercent();

	    // calculate preferred pos
	    if ( (pass == 2) && (cellPercent > 0) )
	    {
		pref_size = ( max_width * cell->getPercent() / 100 ) + 
		           padding + padding + spacing + borderExtra;
		col_type = Percent;
	    }
	    else if ( cellPercent == 0 )
	    {
	    	// Fixed size, preffered width == min width
		pref_size = min_size;
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
    unsigned int i;
    unsigned int totalRowInfos;
    totalRowInfos = 1;
    for(i = 1; i < totalRows; i++)
    {
    	bool unique = TRUE;
    	for(unsigned int j = 0; (j < totalRowInfos) && (unique == TRUE); j++)
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
    
    int maxColSpan = 0;
    for(i = 0; i < totalRowInfos; i++)
    {
        int min = 0;
        int pref = 0;
        for(int j = 0; j < rowInfo[i].nrEntries; j++)
        {
           int index = rowInfo[i].entry[j];
           
           // Prefered size is at least the minimum size
	   if (colInfo[index].minSize > colInfo[index].prefSize)
	   	colInfo[index].prefSize = colInfo[index].minSize;
           if (colInfo[index].colSpan > maxColSpan)
	        maxColSpan = colInfo[index].colSpan;

           min += colInfo[index].minSize;
           pref += colInfo[index].prefSize;
	}
	rowInfo[i].minSize = min;
	rowInfo[i].prefSize = pref;
    }

    printf("maxColSpan = %d\n", maxColSpan);

    columnPos.resize( totalCols + 1 );
    columnPrefPos.resize( totalCols + 1 );
    columnPos.fill( 0 );
    columnPrefPos.fill( 0 );
    colType.resize( totalCols + 1 );
    colType.fill(Variable);
    
    columnPos[0] = border + spacing;
    columnPrefPos[0] = border + spacing;
    // Calculate minimum widths for each column.
    for(int col_span = 1; col_span <= maxColSpan; col_span++)
    {
        for(i = 0; i < totalRowInfos; i++)
        {
            for(int j = 0; j < rowInfo[i].nrEntries; j++)
            {
                int index = rowInfo[i].entry[j];
                if (colInfo[index].colSpan != col_span)
                    continue;
		int currMinSize = 0;
		int currPrefSize = 0;
                int nonFixedCount = 0;
		int kol = colInfo[index].startCol;
                bool isFixed = (colInfo[index].colType == Fixed);
                
		// Update minimum sizes
                for (int k = col_span; k; k--)
                {
                    currMinSize += columnPos[kol + k];
                    if ( colType[ kol + k ] != Fixed )
                    	nonFixedCount++;
		}                

		if (currMinSize < colInfo[index].minSize)
		{
		    currMinSize = colInfo[index].minSize - currMinSize;
printf("MinSize: IsFixed %d nonFixedCount %d\n", isFixed, nonFixedCount);
		    if ( (nonFixedCount == 0) || 
		         (nonFixedCount == col_span) )
		    {
printf("Colspan = %d Spread %d pixels about all columns\n", col_span, currMinSize);		    	
		    	// Spread extra width across all columns equally
                        for (int k = col_span; k; k--)
                        {
                    	    int delta = currMinSize / k;
                    	    columnPos[kol + k] += delta;

                            // Make sure columnPrefPos > columnPos
                    	    if (columnPos[kol + k] >
                    	        columnPrefPos[kol + k])
                    	    {
                    	    	columnPrefPos[kol + k] = columnPos[kol + k];
                    	    }
                    	    if (isFixed)
                    	    {
                    	        colType[kol + k ] = Fixed;
                    	    }
                    	    currMinSize -= delta;
            	        }
            	    }
		    else
		    {
printf("Colspan = %d Spread %d pixels about all non-fixed columns\n", col_span, currMinSize);		    	
		    	// Spread extra width across all non-fixed columns 
                        for (int k = col_span; k; k--)
                        {
                            if ( colType[kol + k ] != Fixed )
                            {
                    	        int delta = currMinSize / nonFixedCount;
                    	        nonFixedCount--;
                    	        columnPos[kol + k] += delta;

                                // Make sure prefPos > minPos
                    	        if (columnPos[kol + k] >
                    	            columnPrefPos[kol + k])
                    	        {
                    	            columnPrefPos[kol + k] = 
                    	                              columnPos[kol + k];
                    	        }
                    	        currMinSize -= delta;
	 			if (isFixed)
                    	        {
                    	            colType[kol + k ] = Fixed;
                    	        }
                    	    }
            	        }
		    
		    }
		}

		// Update preferred sizes
		nonFixedCount = 0;
                for (int k = col_span; k; k--)
                {
                    currPrefSize += columnPrefPos[kol + k];
                    if ( colType[ kol + k ] != Fixed )
                    	nonFixedCount++;
		}                

		if (currPrefSize < colInfo[index].prefSize)
		{
		    currPrefSize = colInfo[index].prefSize - currPrefSize;
		    if ( (nonFixedCount == 0) || 
		         (nonFixedCount == col_span) )
		    {
		    	// Spread extra width across all columns equally
                        for (int k = col_span; k; k--)
                        {
                            int delta = currPrefSize / k;
                    	    columnPrefPos[kol + k] += delta;
                    	    currPrefSize -= delta;
                        }
            	    }
            	    else
            	    {
		    	// Spread extra width across all non-fixed columns 
                        for (int k = col_span; k; k--)
                        {
                            if ( colType[kol + k ] != Fixed )
                            {
                    	        int delta = currMinSize / nonFixedCount;
                    	        nonFixedCount--;
                    	        columnPrefPos[kol + k] += delta;
                    	        currPrefSize -= delta;
                    	    }
            	        }
            	    }
		}
            }
    	}
    }

    // Cummulate
    for(i = 1; i <= totalCols; i++)
    {
    	columnPos[i] += columnPos[i-1];
    	columnPrefPos[i] += columnPrefPos[i-1];
    }
    min_width = columnPos[totalCols]+border+spacing;
    pref_width = columnPrefPos[totalCols]+border+spacing;

    if ( percent == 0 )
    {
	// Fixed width: Our minimum width is at least our fixed width 
        if (fixed_width > min_width)
            min_width = fixed_width;
    }

    // DEBUG: Show the results :)
#if 1
    printf("--PASS %d --\n", pass);
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
        for(unsigned int j = 0; j < (unsigned int) rowInfo[i].nrEntries; j++)
        {
           if (j == 0)
              printf("%d", rowInfo[i].entry[j]);
           else
              printf("- %d", rowInfo[i].entry[j]);
        } 
        printf(" ! %d : %d\n", rowInfo[i].minSize, rowInfo[i].prefSize);
    }
    for(i = 0; i < totalCols; i++)
    {
    	printf("Col %d: %d - %d\n", i, columnPos[i+1]-columnPos[i], 
    		columnPrefPos[i+1]-columnPrefPos[i]);
    }
    printf("min = %d, pref = %d\n", min_width, pref_width);
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
    return min_width;
}

int HTMLTable::calcPreferredWidth()
{
    return pref_width;
}

void HTMLTable::setMaxWidth( int _max_width )
{
    max_width = _max_width;
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

