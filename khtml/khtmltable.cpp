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

//#define TABLE_DEBUG

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
#include <assert.h>

#include <qimage.h>
#include <qdrawutil.h>

//#define CLUE_DEBUG

#define FOR_EACH_CELL(r,c,cell) \
    for ( unsigned int r = 0; r < totalRows; r++ )                    \
    {                                                                 \
        for ( unsigned int c = 0; c < totalCols; c++ )                \
        {                                                             \
            HTMLTableCell *cell = cells[r][c];                        \
            if (!cell)                                                \
                continue;                                             \
	    if ( (c < totalCols - 1) && (cell == cells[r][c+1]) )     \
	        continue;                                             \
	    if ( (r < totalRows - 1) && (cells[r+1][c] == cell) )     \
	        continue;                                             

#define END_FOR_EACH } }

//-----------------------------------------------------------------------------

HTMLTableCell::HTMLTableCell( int _percent, int _width, int rs, int cs, int pad )
	 : HTMLClueV()
{
    rspan = rs;
    cspan = cs;
    padding = pad;

    percent = _percent;
    if (percent == 0)
        fixed_width = _width;
    else
        fixed_width = UNDEFINED;
}

bool HTMLTableCell::print( QPainter *_painter, int _x, int _y, int _width,
	int _height, int _tx, int _ty, bool toPrinter )
{
	if ( (_y + _height < y - getAscent() - padding) || 
	     (_y > y + padding )
	   )
	{
            return false;
        }
    
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
			width + padding * 2, bottom - top, brush );
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
    width = _max_width;
}
                                                        
void HTMLTableCell::calcSize( HTMLClue *parent )
{
    HTMLClueV::calcSize(parent);
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
    calcColInfoI();
}

void HTMLTable::calcAbsolutePos( int _x, int _y )
{
    int lx = _x + x;
    int ly = _y + y - ascent;

    FOR_EACH_CELL( r, c, cell)
    {
        cell->calcAbsolutePos( lx, ly );
    }
    END_FOR_EACH
}

bool HTMLTable::getObjectPosition( const HTMLObject *objp, int &xp, int &yp )
{
    xp += x;
    yp += (y - ascent);

    FOR_EACH_CELL( r, c, cell)
    {
        if ( cell->getObjectPosition( objp, xp, yp ) )
            return true;
    }
    END_FOR_EACH
    
    xp -= x;
    yp -= (y - ascent);

    return false;
}

HTMLAnchor* HTMLTable::findAnchor( QString _name, int &_x, int &_y )
{
    HTMLAnchor *ret;

    _x += x;
    _y += y - ascent;
 
    FOR_EACH_CELL( r, c, cell)
    {
        ret = cell->findAnchor( _name, _x, _y );
        if (ret)
	    return ret;
    }
    END_FOR_EACH
	
    _x -= x;
    _y -= y - ascent;

    return 0;
}

void HTMLTable::reset()
{
    FOR_EACH_CELL( r, c, cell)
    {
        cell->reset();
    }
    END_FOR_EACH

    calcColInfoI();
}

void HTMLTable::calcSize( HTMLClue * )
{
    int indx;

    // Do the final layout based on width
    calcColInfoII();

    // If it doesn't fit... MAKE IT FIT!
    for ( unsigned int c = 0; c < totalCols; c++ )
    {
        if (columnPos[c+1] > width-border)
        {
             printf("WARNING: Column too small!\n");
             columnPos[c+1] = width-border;
	}
    }

    // set cell widths and then calculate cell sizes
    FOR_EACH_CELL( r, c, cell)
    {
	if ( ( indx = c-cell->colSpan()+1 ) < 0 )
	    indx = 0;

	cell->setMaxWidth( columnPos[c+1] - columnPos[ indx ] - spacing -
	         padding - padding );
	cell->calcSize();
    }
    END_FOR_EACH

    if ( caption )
    {
	caption->setMaxWidth( columnPos[ totalCols ] + border );
	caption->calcSize();
	if ( capAlign == HTMLClue::Top )
	    caption->setPos( 0, caption->getAscent() );
    }

    // We have the cell sizes now, so calculate the vertical positions
    calcRowHeights();

    // set cell positions
    for ( unsigned int r = 0; r < totalRows; r++ )
    {
	int cellHeight;

	ascent = rowHeights[r+1] - padding - spacing;
	if ( caption && capAlign == HTMLClue::Top )
	    ascent += caption->getHeight();

	for ( unsigned int c = 0; c < totalCols; c++ )
	{
            HTMLTableCell *cell = cells[r][c];                        \
            if (!cell)                                                \
                continue;                                             \
	    if ( c < totalCols - 1 && cell == cells[r][c+1] )
		continue;
	    if ( r < totalRows - 1 && cell == cells[r+1][c] )
		continue;

	    if ( ( indx = c-cell->colSpan()+1 ) < 0 )
		indx = 0;

	    cell->setPos( columnPos[indx] + padding,
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
    width = columnPos[ totalCols ] + border;
    ascent = rowHeights[ totalRows ] + border;
    if ( caption )
	ascent += caption->getHeight();
}

// New table layout function
void HTMLTable::addRowInfo(int _row, int _colInfoIndex)
{
    rowInfo[_row].entry[rowInfo[_row].nrEntries++] = _colInfoIndex;
}

// New table layout function
int HTMLTable::addColInfo(int _startCol, int _colSpan, 
                          int _minSize, int _prefSize, 
                          ColType _colType)
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

void HTMLTable::addColMinWidth(int k, int delta, ColType t)
{     
#ifdef TABLE_DEBUG              	    
printf("Adding %d pixels to col %d: %d --> %d\n",
     delta, k, columnPos[k], columnPos[k]+delta);
#endif

    columnPos[k] += delta;

    // Make sure columnPrefPos > columnPos
    if (columnPos[k] >
        columnPrefPos[k])
    {
 	columnPrefPos[k] = columnPos[k];
    }
    // Fixed < Percent < Variable
    if (t < colType[k])
    {
       colType[ k ] = t;
    }
}

void HTMLTable::addColsMinWidthEqual(int kol, int span, int tooAdd, ColType t)
{
    // Spread extra width across all columns equally
    for (int k = span; k; k--)
    {
        int delta = tooAdd / k;
        addColMinWidth(kol+k, delta, t);
        tooAdd -= delta;
    }
}

void HTMLTable::addColsMinWidthVar(int kol, int span, int tooAdd, ColType t, int varCount)
{
    assert( varCount > 0);
    // Spread extra width across all variable columns 
    for (int k = span; k; k--)
    {
        if ( colType[kol + k ] == Variable )
        {
            int delta = tooAdd / varCount;
            addColMinWidth(kol+k, delta, t);
            varCount--;
            tooAdd -= delta;
        }
    }
}

void HTMLTable::addColsMinWidthNonFix(int kol, int span, int tooAdd, ColType t, int nonFixedCount)
{
    assert( nonFixedCount > 0);
    // Spread extra width across all non-fixed columns 
    for (int k = span; k; k--)
    {
        if ( colType[kol + k ] != Fixed )
        {
            int delta = tooAdd / nonFixedCount;
            addColMinWidth(kol+k, delta, t);
            nonFixedCount--;
            tooAdd -= delta;
        }
    }
}

int HTMLTable::addColsMinWidthPref(int kol, int span, int tooAdd)
{
    int prefDiff;    // The width we prefer to have extra
    int prefTooAdd;  // The actual extra width we can add

    prefDiff = 0;
    for (int k = span; k; k--)
    {
#ifdef TABLE_DEBUG
printf("  Col = %d, type = %d, pos = %d, pref = %d\n",
   kol+k, colType[kol+k], columnPos[kol+k], columnPrefPos[kol+k]);
#endif
        if ( colType[kol + k ] == Percent )
        {
            prefDiff += columnPrefPos[kol+k] - columnPos[kol+k];
        }
    }

    prefTooAdd = tooAdd;
    if (prefTooAdd > prefDiff)
    {
        prefTooAdd = prefDiff;
    }
    // extra width across all percent columns 
#ifdef TABLE_DEBUG
printf("Spreading %d pixels to percent columns according to pref\n", prefTooAdd);
#endif
    for (int k = span; k; k--)
    {
        if (prefDiff == 0)
            break;
                                
        if ( colType[kol + k ] == Percent )
        {
            int delta = prefTooAdd * 
      	        	(columnPrefPos[kol+k] - columnPos[kol+k])
                        / prefDiff;
            prefDiff -= (columnPrefPos[kol+k] - columnPos[kol+k]);
            addColMinWidth(kol+k, delta, Variable);
            tooAdd -= delta;
            prefTooAdd -= delta;
        }
    }

    prefDiff = 0;
    for (int k = span; k; k--)
    {
#ifdef TABLE_DEBUG

printf("  Col = %d, type = %d, pos = %d, pref = %d\n",
   kol+k, colType[kol+k], columnPos[kol+k], columnPrefPos[kol+k]);
#endif
        if ( colType[kol + k ] == Variable )
        {
            prefDiff += columnPrefPos[kol+k] - columnPos[kol+k];
        }
    }

    prefTooAdd = tooAdd;
    if (prefTooAdd > prefDiff)
    {
        prefTooAdd = prefDiff;
    }
    // extra width across all variable columns 
#ifdef TABLE_DEBUG
printf("Spreading %d pixels to variable columns according to pref\n", prefTooAdd);
#endif
    for (int k = span; k; k--)
    {
        if (prefDiff == 0)
            break;
                                
        if ( colType[kol + k ] == Variable )
        {
            int delta = prefTooAdd * 
      	        	(columnPrefPos[kol+k] - columnPos[kol+k])
                        / prefDiff;
            prefDiff -= (columnPrefPos[kol+k] - columnPos[kol+k]);
            addColMinWidth(kol+k, delta, Variable);
            tooAdd -= delta;
            prefTooAdd -= delta;
        }
    }

    // Return the width still left to add.
    return tooAdd;
}

void HTMLTable::addColsPrefWidthEqual(int kol, int span, int tooAdd)
{
    // Spread extra width across all columns equally
    for (int k = span; k; k--)
    {
        int delta = tooAdd / k;
        columnPrefPos[kol + k] += delta;
        tooAdd -= delta;
    }
}

void HTMLTable::addColsPrefWidthNonFix(int kol, int span, int tooAdd, int nonFixedCount)
{
    assert( nonFixedCount > 0);
    // Spread extra width across all non-fixed columns 
    for (int k = span; k; k--)
    {
        if ( colType[kol + k ] != Fixed )
        {
            int delta = tooAdd / nonFixedCount;
            nonFixedCount--;
            columnPrefPos[kol + k] += delta;
            tooAdd -= delta;
        }
    }
}
                        
void HTMLTable::addColsPrefWidthVar(int kol, int span, int tooAdd, int varCount)
{
    assert( varCount > 0);
    // Spread extra width across all non-fixed columns 
    for (int k = span; k; k--)
    {
        if ( colType[kol + k ] == Variable )
        {
            int delta = tooAdd / varCount;
            varCount--;
            columnPrefPos[kol + k] += delta;
            tooAdd -= delta;
        }
    }
}
                        
// New table layout function
//
// Both the minimum and preferred column sizes are calculated here.
// During this phase we are calculating width advices
// min_width and pref_idth are calculated here
void HTMLTable::calcColInfoI( void )
{
    unsigned int r, c;
    int borderExtra = ( border == 0 ) ? 0 : 1;

#ifdef TABLE_DEBUG
printf("START calcColInfoI() this = %p\n", this);
#endif

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
	    {
	        // Add dummy info
	        colInfoIndex = addColInfo(c, 1, 1, 1, Variable);
                addRowInfo(r, colInfoIndex);
		continue; 
            }
	    if ( (c > 0) && (cells[r][c-1] == cell) )
		continue;
	    if ( (r > 0) && (cells[r-1][c] == cell) )
		continue;

	    // calculate minimum size
	    min_size = cell->calcMinWidth() + padding + padding + 
	              spacing + borderExtra;
	
	    cellPercent = cell->getPercent();

	    // calculate preferred pos
	    if ( cellPercent == 0 )
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
	                              pref_size, col_type);
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
        for(int j = 0; j < rowInfo[i].nrEntries; j++)
        {
           int index = rowInfo[i].entry[j];
           
           // Prefered size is at least the minimum size
	   if (colInfo[index].minSize > colInfo[index].prefSize)
	   	colInfo[index].prefSize = colInfo[index].minSize;
           if (colInfo[index].colSpan > maxColSpan)
	        maxColSpan = colInfo[index].colSpan;
	}
    }

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
                ColType isFixed;
                if (colInfo[index].colType == Fixed)
                   isFixed = Fixed;
                else
                   isFixed = Variable;
                
		// Update minimum sizes
                for (int k = col_span; k; k--)
                {
                    currMinSize += columnPos[kol + k];
                    if ( colType[ kol + k ] != Fixed )
                    	nonFixedCount++;
		}                

		if (currMinSize < colInfo[index].minSize)
		{
		    int tooAdd = colInfo[index].minSize - currMinSize;
		    if ( (nonFixedCount == 0) || 
		         (nonFixedCount == col_span) )
		    {
		    	// Spread extra width across all columns equally
			addColsMinWidthEqual(kol, col_span, tooAdd, isFixed);                        
            	    }
		    else
		    {
		        // Spread extra width across all non-fixed width columns equally
		        addColsMinWidthNonFix(kol, col_span, tooAdd, isFixed, nonFixedCount);
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
		    int tooAdd = colInfo[index].prefSize - currPrefSize;
		    if ( (nonFixedCount == 0) || 
		         (nonFixedCount == col_span) )
		    {
		    	// Spread extra width across all columns equally
			addColsPrefWidthEqual(kol, col_span, tooAdd);                        
            	    }
            	    else
            	    {
		    	// Spread extra width across all non-fixed columns 
                        addColsPrefWidthNonFix(kol, col_span, tooAdd, nonFixedCount);
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
    min_width = columnPos[totalCols]+border;
    pref_width = columnPrefPos[totalCols]+border;

    if ( percent == 0 )
    {
	// Fixed width: Our minimum width is at least our fixed width 
        if (fixed_width > min_width)
            min_width = fixed_width;
    }
    if (pref_width < min_width)
        pref_width = min_width;

#ifdef TABLE_DEBUG
    printf("--PASS 1 : Calculating width advice --\n");
    printf("---- %d ----\n", totalColInfos);
    for(i = 0; i < totalColInfos; i++)
    {
        printf("col #%d: %d - %d, min: %3d pref: %3d type: %d\n",
                 i,
                 colInfo[i].startCol, colInfo[i].colSpan,
                 colInfo[i].minSize, colInfo[i].prefSize,
                 (int) colInfo[i].colType);
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
    }
    printf("min = %d, pref = %d\n", min_width, pref_width);
#endif
}

// New table layout function
//
// Both the minimum and preferred column sizes are calculated here.
// During this phase we are going to calculate the actual widths.
void HTMLTable::calcColInfoII(void)
{
    unsigned int r, c;
    int borderExtra = ( border == 0 ) ? 0 : 1;

#ifdef TABLE_DEBUG
printf("START calcColInfoII() this = %p\n", this);
#endif
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
	    {
	        // Add dummy info
	        colInfoIndex = addColInfo(c, 1, 1, 1, Variable);
                addRowInfo(r, colInfoIndex);
		continue; 
            }
	    if ( (c > 0) && (cells[r][c-1] == cell) )
		continue;
	    if ( (r > 0) && (cells[r-1][c] == cell) )
		continue;

	    // calculate minimum size
	    min_size = cell->calcMinWidth() + padding + padding + 
	              spacing + borderExtra;
	
	    cellPercent = cell->getPercent();

	    // calculate preferred pos
	    if ( cellPercent > 0 )
	    {
		pref_size = ( width * cell->getPercent() / 100 ) + 
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
	                              pref_size, col_type);
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

    int maxColSpan = 0;
    for(i = 0; i < totalRowInfos; i++)
    {
        for(int j = 0; j < rowInfo[i].nrEntries; j++)
        {
           int index = rowInfo[i].entry[j];
           
           // Prefered size is at least the minimum size
	   if (colInfo[index].minSize > colInfo[index].prefSize)
	   	colInfo[index].prefSize = colInfo[index].minSize;
           if (colInfo[index].colSpan > maxColSpan)
	        maxColSpan = colInfo[index].colSpan;
	}
    }
#ifdef TABLE_DEBUG
    printf("--PASS II --\n");
    printf("---- %d ----\n", totalColInfos);
    for(i = 0; i < totalColInfos; i++)
    {
        printf("col #%d: %d - %d, min: %3d pref: %3d type: %d\n",
                 i,
                 colInfo[i].startCol, colInfo[i].colSpan,
                 colInfo[i].minSize, colInfo[i].prefSize,
                 (int) colInfo[i].colType);
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
        printf("\n");
    }
    printf("maxColSpan = %d\n", maxColSpan);
#endif

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
                int fixedCount = 0;
                int percentCount = 0;
                int varCount = 0;
		int kol = colInfo[index].startCol;
                ColType isFixed = colInfo[index].colType;
                
		// Update minimum sizes
                for (int k = col_span; k; k--)
                {
                    currMinSize += columnPos[kol + k];
                    switch( colType[ kol + k ])
                    {
                    case Fixed:
                    	fixedCount++;
                    	break;
                    case Percent:
                        percentCount++;
                        break;
                    default:
                        varCount++;
                        break;
                    } 
		}                
#ifdef TABLE_DEBUG
printf("Col = %d span = %d, fix = %d, perc = %d, var = %d\n",
    kol, col_span, fixedCount, percentCount, varCount);
#endif 
		if (currMinSize < colInfo[index].minSize)
		{
		    int tooAdd = colInfo[index].minSize - currMinSize;
		    if ( (fixedCount == col_span) || 
		         (varCount == col_span) )
		    {
#ifdef TABLE_DEBUG
printf("Spreading %d pixels equally\n", tooAdd);
#endif
		    	// Spread extra width across all columns equally
                        addColsMinWidthEqual(kol, col_span, tooAdd, isFixed);
            	    }
		    else
		    {
#ifdef TABLE_DEBUG
printf("Spreading %d pixels\n", tooAdd);
#endif
                        // Look at the pref. widths first
                        tooAdd = addColsMinWidthPref(kol, col_span, tooAdd);            	        
            	        if (varCount > 0)
            	        {
		    	   // Spread extra width across all variable columns 
                           addColsMinWidthVar(kol, col_span, tooAdd, isFixed, varCount);
                    	}
                    	else
            	        {
		    	   // Spread extra width across all columns equally
                           addColsMinWidthEqual(kol, col_span, tooAdd, isFixed);
            	        }
		    }
		}

		// Update preferred sizes
		varCount = 0;
                for (int k = col_span; k; k--)
                {
                    currPrefSize += columnPrefPos[kol + k];
                    if ( colType[ kol + k ] == Variable )
                    	varCount++;
		}                

		if (currPrefSize < colInfo[index].prefSize)
		{
		    int tooAdd = colInfo[index].prefSize - currPrefSize;
		    if ( (varCount == 0) || 
		         (varCount == col_span) )
		    {
		    	// Spread extra width across all columns equally
			addColsPrefWidthEqual(kol, col_span, tooAdd);                        
            	    }
            	    else
            	    {
		    	// Spread extra width across all Variable columns 
                        addColsPrefWidthVar(kol, col_span, tooAdd, varCount);
            	    }
		}
            }
    	}
    }

    // Fill me up dear.
    {
        unsigned int fixedCount = 0;
        unsigned int percentCount = 0;
        unsigned int varCount = 0;
	int currMinSize = 0;
        for (int k = totalCols; k; k--)
        {
            currMinSize += columnPos[k];
            switch( colType[ k ])
            {
                case Fixed:
                    fixedCount++;
              	    break;
                case Percent:
                    percentCount++;
                    break;
                default:
                    varCount++;
                    break;
            } 
        }                

        if (currMinSize < width )
        {
             int tooAdd = width - currMinSize - columnPos[0] - border;
             if (fixedCount == totalCols)
             {
#ifdef TABLE_DEBUG
printf("Final: Spreading %d pixels equally\n", tooAdd);
#endif
                 // Spread extra width across all columns equally
                 addColsMinWidthEqual(0, totalCols, tooAdd, Variable);
             }
             else
             {
#ifdef TABLE_DEBUG
printf("Final: Spreading %d pixels\n", tooAdd);
#endif
                 // Look at the pref. widths first
                 tooAdd = addColsMinWidthPref(0, totalCols, tooAdd);            	        
                 if (varCount > 0)
                 {
	             // Spread extra width across all variable columns 
                     addColsMinWidthVar(0, totalCols, tooAdd, Variable, varCount);
                 }
                 else
            	 {
		     // Spread extra width across all columns equally
                     addColsMinWidthEqual(0, totalCols, tooAdd, Variable);
            	 }
             }
         }
#ifdef TABLE_DEBUG
printf("Done!\n");         
#endif
    }

    // Cummulate
    for(i = 1; i <= totalCols; i++)
    {
#ifdef TABLE_DEBUG
printf("Actual width col %d: %d\n", i, columnPos[i]);
#endif
    	columnPos[i] += columnPos[i-1];
    	columnPrefPos[i] += columnPrefPos[i-1];
    }

#ifdef TABLE_DEBUG
    printf("Predicted minimum width: %d\n", min_width);
    printf("Predicted pref width: %d\n", pref_width);
    printf("Width set: %d\n", width);
    printf("Actual width: %d\n", columnPos[totalCols]);
    // DEBUG: Show the results :)
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
    if (percent == UNDEFINED)
    {
        // Variable width
        width = _max_width;
        if (width > pref_width)
        {
            width = pref_width;
        }

        if (_max_width < min_width)
        {
            printf("Layout error in HTMLClue::setMaxWidth(this = %p): _max_width (%d) smaller than minimum width(%d)!\n", 
            this, _max_width, min_width);
        }
    }
    else if (percent == 0)
    {
    	// Fixed with
        width = fixed_width;
        if (_max_width < width)
        {
            printf("Layout error in HTMLClue::setMaxWidth(this = %p): _max_width (%d) smaller than fixed width (%d)!\n", 
            this, _max_width, fixed_width);
        }
    }	
    else
    {
        // Percentage width
        width = (percent * _max_width) / 100;
    }        

    if (width < min_width)
	width = min_width;
}

void HTMLTable::setMaxAscent( int _a )
{
    ascent = _a;
}

HTMLObject *HTMLTable::checkPoint( int _x, int _y )
{
    if ( (_x < x) || (_x > x + width) || 
         (_y > y + descent) || (_y < y - ascent)
       )
    {
	return 0L;
    }

    FOR_EACH_CELL(r, c, cell)
    {
        HTMLObject *obj = cell->checkPoint( _x-x, _y-(y - ascent) );
        if (obj)
	    return obj;
    }
    END_FOR_EACH

    return 0L;
}

HTMLObject *HTMLTable::mouseEvent( int _x, int _y, int button, int state )
{
    if ( (_x < x) || (_x > x + width) || 
         (_y > y + descent) || (_y < y - ascent)
       )
    {
	return 0;
    }

    FOR_EACH_CELL(r, c, cell)
    {
        HTMLObject *obj = cell->mouseEvent( _x-x, _y-(y - ascent), 
                                            button, state );
        if (obj)
	    return obj;
    }
    END_FOR_EACH

    return 0;
}

void HTMLTable::selectByURL( KHTMLWidget *_htmlw, HTMLChain *_chain,
    QString _url, bool _select, int _tx, int _ty )
{
    _tx += x;
    _ty += y - ascent;

    _chain->push( this );

    FOR_EACH_CELL(r, c, cell)
    {
	cell->selectByURL( _htmlw, _chain, _url, _select, _tx, _ty );
    }
    END_FOR_EACH

    _chain->pop();
}

void HTMLTable::select( KHTMLWidget *_htmlw, HTMLChain *_chain,
    QRegExp& _pattern, bool _select, int _tx, int _ty )
{
    _tx += x;
    _ty += y - ascent;

    _chain->push( this );

    FOR_EACH_CELL(r, c, cell)
    {
	cell->select( _htmlw, _chain, _pattern, _select, _tx, _ty );
    }
    END_FOR_EACH

    _chain->pop();
}

void HTMLTable::select( KHTMLWidget *_htmlw, HTMLChain *_chain,
    bool _select, int _tx, int _ty )
{
    _tx += x;
    _ty += y - ascent;

    _chain->push( this );

    FOR_EACH_CELL(r, c, cell)
    {
        cell->select( _htmlw, _chain, _select, _tx, _ty );
    }
    END_FOR_EACH

    _chain->pop();
}

void HTMLTable::select( KHTMLWidget *_htmlw, HTMLChain *_chain,
    QRect & _rect, int _tx, int _ty )
{
    _tx += x;
    _ty += y - ascent;

    _chain->push( this );

    FOR_EACH_CELL(r, c, cell)
    {
	cell->select( _htmlw, _chain, _rect, _tx, _ty );
    }
    END_FOR_EACH

    _chain->pop();
}

void HTMLTable::select( bool _select )
{
    FOR_EACH_CELL(r, c, cell)
    {
	cell->select( _select );
    }
    END_FOR_EACH
}

bool HTMLTable::selectText( KHTMLWidget *_htmlw, HTMLChain *_chain,
	int _x1, int _y1, int _x2, int _y2, int _tx, int _ty )
{
    bool isSel = false;

    _tx += x;
    _ty += y - ascent;

    _chain->push( this );

    FOR_EACH_CELL(r, c, cell)
    {
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
    END_FOR_EACH

    _chain->pop();

    return isSel;
}

void HTMLTable::getSelected( QStringList &_list )
{
    FOR_EACH_CELL(r, c, cell)
    {
        cell->getSelected( _list );
    }
    END_FOR_EACH
}

void HTMLTable::getSelectedText( QString &_str, bool getAll )
{
    FOR_EACH_CELL(r, c, cell)
    {
	cell->getSelectedText( _str, getAll );
    }
    END_FOR_EACH
}
int HTMLTable::findPageBreak( int _y )
{
    if ( _y > y )
	return -1;

    int pos, minpos = 0x7FFFFFFF;
    QArray<bool> colsDone( totalCols );
    colsDone.fill( false );

    FOR_EACH_CELL(r, c, cell)
    {
	if ( colsDone[c] )
	    continue;

	pos = cell->findPageBreak( _y - ( y - ascent ) );
	if ( pos >= 0 && pos < minpos )
	{
	    colsDone[c] = true;
	    minpos = pos;
	}
    }
    END_FOR_EACH

    if ( minpos != 0x7FFFFFFF )
	return ( minpos + y - ascent );

    return -1;
}

void HTMLTable::findCells( int _tx, int _ty, QList<HTMLCellInfo> &_list )
{
    _tx += x;
    _ty += y - ascent;

    FOR_EACH_CELL(r, c, cell)
    {
	cell->findCells( _tx, _ty, _list );
    }
    END_FOR_EACH
}

bool HTMLTable::print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty, bool toPrinter )
{
    if ( (_y + _height < y - ascent) || (_y > y + descent) )
	return false;

    _tx += x;
    _ty += y - ascent;

    int cindx, rindx;
    QArray<bool> colsDone( totalCols );
    colsDone.fill( false );

    if ( caption )
    {
	caption->print( _painter, _x - x, _y - (y - ascent),
	    _width, _height, _tx, _ty, toPrinter );
    }

    // draw the cells
    FOR_EACH_CELL(r, c, cell)
    {
//@@WABA: What does this do?
        if ( colsDone[c] )
            continue; // Next cell

        if ( cell->print( _painter, _x - x, _y - (y - ascent),
		    _width, _height, _tx, _ty, toPrinter ) )
	colsDone[c] = true;
    }
    END_FOR_EACH

    // draw the border - needs work to print to printer
    if ( border && !toPrinter )
    {
	int capOffset = 0;
	if ( caption && capAlign == HTMLClue::Top )
	    capOffset = caption->getHeight();
	QColorGroup colorGrp( Qt::black, Qt::lightGray, Qt::white, Qt::darkGray, Qt::gray,
	    Qt::black, Qt::white );
	qDrawShadePanel( _painter, _tx, _ty + capOffset, width,
	    rowHeights[totalRows] + border, colorGrp, false, border );

        FOR_EACH_CELL(r, c, cell)
        {
	    if ( ( cindx = c-cell->colSpan()+1 ) < 0 )
	        cindx = 0;
	    if ( ( rindx = r-cell->rowSpan()+1 ) < 0 )
	        rindx = 0;

	    qDrawShadePanel(_painter,
	         _tx + columnPos[cindx],
	         _ty + rowHeights[rindx] + capOffset,
	         columnPos[c+1] - columnPos[cindx] - spacing,
	         rowHeights[r+1] - rowHeights[rindx] - spacing,
	         colorGrp, TRUE, 1 );
	}
        END_FOR_EACH
    }

    for ( unsigned int c = 0; c < totalCols; c++ )
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
	    _y - (y - ascent), _width, _height, _tx, _ty );
    }
} 

void HTMLTable::print( QPainter *_painter, HTMLObject *_obj, int _x, int _y, int _width, int _height, int _tx, int _ty )
{
    if ( (_y + _height < y - ascent) || (_y > y + descent) )
	return;

    _tx += x;
    _ty += y - ascent;

    FOR_EACH_CELL(r, c, cell)
    {
	cell->print( _painter, _obj, _x - x, _y - (y - ascent),
		_width, _height, _tx, _ty );
    }
    END_FOR_EACH
}

void HTMLTable::print( QPainter *_painter, int _tx, int _ty )
{
    print( _painter, 0, 0, 0xFFFF, 0xFFFF, _tx, _ty, false );
}

HTMLTable::~HTMLTable()
{
    for ( unsigned int r = 0; r < totalRows; r++ )
    {
	for ( unsigned int c = 0; c < totalCols; c++ )
	{
            HTMLTableCell *cell = cells[r][c];
	    if (!cell)
		continue;
	    if ( (c < totalCols - 1) && (cell == cells[r][c+1]) )
		continue;
	    if ( (r < totalRows - 1) && (cells[r+1][c] == cell) )
		continue;
	    delete cell;
	}

	delete [] cells[r];
    }

    delete [] cells;

    delete caption;
}

void
HTMLTable::printDebug( bool propagate, int indent, bool printObjects )
{
    QString str = "   ";
    QString iStr = "";
    int i;
    for( i=0; i<indent; i++)
        iStr += str;

    printf((iStr + objectName()).ascii());
    printf("\n");

    if (!propagate) return;

    printf((iStr + "\\---> pos = (%d/%d)  size = (%d/%d)\n").ascii(),
	   x, y, width, ascent+descent);
    printf((iStr + " ---> size = %d rows,  %d cols\n").ascii(), totalRows, totalCols);
    printf((iStr + " ---> this = %p\n").ascii(), this);
    
    // ok... go through the children
    indent++;
    FOR_EACH_CELL(r, c, cell)
    {
        printf((iStr + "CELL[%d, %d]\n").ascii(), r,c);
        cell->printDebug( propagate, indent, printObjects);
    }
    END_FOR_EACH
}
