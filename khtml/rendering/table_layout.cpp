/*
 * This file is part of the HTML rendering engine for KDE.
 *
 * Copyright (C) 2002 Lars Knoll (knoll@kde.org)
 *           (C) 2002 Dirk Mueller (mueller@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */
#include <table_layout.h>

#include <render_table.h>
using namespace khtml;

//#define DEBUG_LAYOUT

/*
  The text below is from the CSS 2.1 specs.

  Fixed table layout
  ------------------

  With this (fast) algorithm, the horizontal layout of the table does
  not depend on the contents of the cells; it only depends on the
  table's width, the width of the columns, and borders or cell
  spacing.

  The table's width may be specified explicitly with the 'width'
  property. A value of 'auto' (for both 'display: table' and 'display:
  inline-table') means use the automatic table layout algorithm.

  In the fixed table layout algorithm, the width of each column is
  determined as follows:

    1. A column element with a value other than 'auto' for the 'width'
    property sets the width for that column.

    2.Otherwise, a cell in the first row with a value other than
    'auto' for the 'width' property sets the width for that column. If
    the cell spans more than one column, the width is divided over the
    columns.

    3. Any remaining columns equally divide the remaining horizontal
    table space (minus borders or cell spacing).

  The width of the table is then the greater of the value of the
  'width' property for the table element and the sum of the column
  widths (plus cell spacing or borders). If the table is wider than
  the columns, the extra space should be distributed over the columns.


  In this manner, the user agent can begin to lay out the table once
  the entire first row has been received. Cells in subsequent rows do
  not affect column widths. Any cell that has content that overflows
  uses the 'overflow' property to determine whether to clip the
  overflow content.

_____________________________________________________

  This is not quite true when comparing to IE. IE always honours
  table-layout:fixed and treats a variable table width as 100%. Makes
  a lot of sense, and is implemented here the same way.

*/

FixedTableLayout::FixedTableLayout( RenderTable *table )
    : TableLayout ( table )
{
}

FixedTableLayout::~FixedTableLayout()
{
}

int FixedTableLayout::calcWidthArray( int tableWidth )
{
    int usedWidth = 0;

    // iterate over all <col> elements
    RenderObject *child = table->firstChild();
    int cCol = 0;
    int nEffCols = table->numEffCols();
    width.resize( nEffCols );
    width.fill( -1 );

#ifdef DEBUG_LAYOUT
    qDebug("FixedTableLayout::calcWidthArray( %d )", tableWidth );
    qDebug("    col elements:");
#endif

    Length grpWidth;
    while ( child ) {
	if ( child->isTableCol() ) {
	    RenderTableCol *col = static_cast<RenderTableCol *>(child);
	    int span = col->span();
	    if ( col->firstChild() ) {
		grpWidth = col->style()->width();
	    } else {
		Length w = col->style()->width();
		if ( w.isVariable() )
		    w = grpWidth;
		int effWidth = -1;
		if ( w.type == Fixed && w.value > 0 )
		    effWidth = w.value;
		else if ( w.type == Percent && w.value > 0 && tableWidth != 0 )
		    effWidth = w.value*tableWidth/100;
#ifdef DEBUG_LAYOUT
		qDebug("    col element: effCol=%d, span=%d: %d",  cCol, span, effWidth);
#endif
		if ( effWidth != -1 ) {
		    int usedSpan = 0;
		    int i = 0;
		    while ( usedSpan < span ) {
			if( cCol + i >= nEffCols ) {
			    table->appendColumn( span - usedSpan );
			    nEffCols++;
			    width.resize( nEffCols );
			}
			int eSpan = table->spanOfEffCol( cCol+i );
			int w = effWidth*eSpan/span;
			width[cCol+i] = w;
			usedWidth += w;
			usedSpan += eSpan;
			i++;
		    }
		    cCol += i;
		} else {
		    int usedSpan = 0;
		    int i = 0;
		    while ( usedSpan < span ) {
			Q_ASSERT( cCol + i < nEffCols );
			usedSpan += table->spanOfEffCol( cCol );
			i++;
		    }
		    cCol += i;
		}
	    }
	} else {
	    break;
	}

	RenderObject *next = child->firstChild();
	if ( !next )
	    next = child->nextSibling();
	if ( !next && child->parent()->isTableCol() ) {
	    next = child->parent()->nextSibling();
	    grpWidth = Length();
	}
	child = next;
    }

#ifdef DEBUG_LAYOUT
    qDebug("    first row:");
#endif
    // iterate over the first row in case some are unspecified.
    RenderTableSection *section = table->head;
    if ( !section )
	section = table->firstBody;
    if ( !section )
	section = table->foot;
    if ( section ) {
	cCol = 0;
	// get the first cell in the first row
	child = section->firstChild()->firstChild();
	while ( child ) {
	    if ( child->isTableCell() ) {
		RenderTableCell *cell = static_cast<RenderTableCell *>(child);
		Length w = cell->style()->width();
		int span = cell->colSpan();
		int effWidth = -1;
		if ( w.type == Fixed && w.value > 0 )
		    effWidth = w.value;
		else if ( w.type == Percent && w.value > 0 && tableWidth != 0 )
		    effWidth = w.value*tableWidth/100;
#ifdef DEBUG_LAYOUT
		qDebug("    table cell: effCol=%d, span=%d: %d",  cCol, span, effWidth);
#endif
		if ( effWidth != -1 ) {
		    int usedSpan = 0;
		    int i = 0;
		    while ( usedSpan < span ) {
			Q_ASSERT( cCol + i < nEffCols );
			if ( width[cCol+i] == -1 ) {
			    int eSpan = table->spanOfEffCol( cCol+i );
			    int w = effWidth*eSpan/span;
			    width[cCol+i] = w;
			    usedWidth += w;
			    usedSpan += eSpan;
#ifdef DEBUG_LAYOUT
			    qDebug("    giving %d px to effCol %d (span=%d)", width[cCol+i], cCol+i, eSpan);
#endif
			} else {
#ifdef DEBUG_LAYOUT
			    qDebug("    width of col %d already defined (span=%d)", cCol, table->spanOfEffCol( cCol ) );
#endif
			    usedSpan += table->spanOfEffCol( cCol );
			}
			i++;
		    }
		    cCol += i;
		} else {
		    int usedSpan = 0;
		    int i = 0;
		    while ( usedSpan < span ) {
			Q_ASSERT( cCol + i < nEffCols );
			usedSpan += table->spanOfEffCol( cCol );
			i++;
		    }
		    cCol += i;
		}
	    } else {
		Q_ASSERT( false );
	    }
	    child = child->nextSibling();
	}
    }

    return usedWidth;

}

void FixedTableLayout::calcMinMaxWidth()
{
    // we might want to wait until we have all of the first row before
    // layouting for the first time.

    // only need to calculate the minimum width as the sum of the
    // cols/cells with a fixed width.
    //
    // The maximum width is QMAX( minWidth, tableWidth ) if table
    // width is fixed. If table width is percent, we set maxWidth to
    // unlimited.


    table->m_minWidth = 0;
    table->m_maxWidth = 0;
    int tableWidth = table->style()->width().type == Fixed ? table->style()->width().value - table->borderLeft() - table->borderRight() - table->cellSpacing() : 0;

    table->m_minWidth = calcWidthArray( tableWidth );
    int bs = table->bordersAndSpacing();
    table->m_minWidth += bs;

    table->m_minWidth = QMAX( table->m_minWidth, tableWidth );
    table->m_maxWidth = table->m_minWidth;
    if ( !tableWidth ) {
	bool haveNonFixed = false;
	for ( unsigned int i = 0; i < width.size(); i++ ) {
	    if ( width[i] == -1 ) {
		haveNonFixed = true;
		break;
	    }
	}
	if ( haveNonFixed )
	    table->m_maxWidth = 0x7fff;
    }
}

void FixedTableLayout::layout()
{
#ifdef DEBUG_LAYOUT
    qDebug("FixedTableLayout::layout:");
#endif

    int w = table->width() - table->borderLeft() - table->borderRight() - table->cellSpacing();
    // we know the table width by now.
    int rest = w - calcWidthArray( w );

    int nEffCols = table->numEffCols();


    if ( rest > 0 ) {
	// distribute equally over the columns
	int nCols = 0;
	int numVariable = 0;
	for ( int i = 0; i < nEffCols; i++ ) {
	    int span = table->spanOfEffCol( i );
	    nCols += span;
	    if ( width[i] == -1 )
		numVariable += span;
	}
#ifdef DEBUG_LAYOUT
	qDebug("distributing %d px over %d eff cols (nCols=%d, numVariable=%d)",  rest,  nEffCols, nCols, numVariable );
#endif

	for ( int i = 0; i < nEffCols; i++ ) {
	    if ( !numVariable ) {
		int span = table->spanOfEffCol( i );
		int add = rest*span/nCols;
		width[i] += add;
		rest -= add;
		nCols -= span;
	    } else if ( width[i] == -1 ) {
		int span = table->spanOfEffCol( i );
		int add = rest*span/numVariable;
		width[i] = add;
		rest -= add;
		numVariable -= span;
	    }
	}
    }

    for ( int i = 0; i < nEffCols; i++ )
	if ( width[i] <= 0 )
	    width[i] = 0; // IE gives min 1 px...

    int pos = 0;
    for ( int i = 0; i < nEffCols; i++ ) {
#ifdef DEBUG_LAYOUT
	qDebug("col %d: %d (width %d)", i, pos, width[i] );
#endif
	table->columnPos[i] = pos;
	pos += width[i];
    }
    table->columnPos[table->columnPos.size()-1] = pos;
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------


AutoTableLayout::AutoTableLayout( RenderTable* table )
    : TableLayout( table )
{
    percentagesDirty = true;
    effWidthDirty = true;
    total_percent = 0;
    hasPercent = false;
}

AutoTableLayout::~AutoTableLayout()
{
}

/* recalculates the full structure needed to do layouting and minmax calculations.
   This is usually calculated on the fly, but needs to be done fully when table cells change
   dynamically
*/
void AutoTableLayout::recalcColumn( int effCol )
{
    Layout &l = layoutStruct[effCol];

    RenderObject *child = table->firstChild();
    // first we iterate over all rows.

    RenderTableCell *fixedContributor = 0;
    RenderTableCell *maxContributor = 0;

    int spacing = table->cellSpacing();

    while ( child ) {
	if ( child->isTableSection() ) {
	    RenderTableSection *section = static_cast<RenderTableSection *>(child);
	    int numRows = section->numRows();
	    RenderTableCell *last = 0;
	    for ( int i = 0; i < numRows; i++ ) {
		RenderTableCell *cell = section->cellAt( i,  effCol );
		if ( cell && cell->colSpan() == 1 ) {
		    if ( !cell->minMaxKnown() )
			cell->calcMinMaxWidth();
		    if ( cell->minWidth() > l.minWidth )
			l.minWidth = cell->minWidth() + spacing;
		    if ( cell->maxWidth() > l.maxWidth ) {
			l.maxWidth = cell->maxWidth() + spacing;
			maxContributor = cell;
		    }

		    Length w = cell->style()->width();
		    switch( w.type ) {
		    case Fixed:
			// ignore width=0
			if ( w.value > 0 && (int)l.width.type != Percent ) {
			    if ( l.width.type == Fixed ) {
				// the 2nd part of the condition is Nav/IE Quirk, see below
				if ((l.width.value > w.value) ||
				    ((l.width.value == w.value) && (maxContributor == cell))) {
				    l.width = w;
				    fixedContributor = cell;
				}
			    } else {
				l.width = w;
				fixedContributor = cell;
			    }
			}
			break;
		    case Percent:
			if ( w.value > 0 ) {
			    hasPercent = true;
			    if ( l.width.type == Percent ) {
				if ( w.value > l.width.value )
				    l.width = w;
			    } else {
				l.width = w;
			    }
			}
			break;
		    case Relative:
			if ( w.type == Relative ) {
			    if ( w.value > l.width.value )
				l.width = w;
			} else if ( w.type == Variable ) {
			    l.width = w;
			}
			break;
		    default:
			break;
		    }
		} else {
		    if ( !effCol || section->cellAt( i, effCol-1 ) != cell )
			insertSpanCell( cell );
		    last = cell;
		}
	    }
	}
	child = child->nextSibling();
    }

    // Nav/IE quirk, see the condition above
    if ( l.width.type == Fixed ) {
	if ( table->style()->htmlHacks()
	     && (l.maxWidth > l.width.value) && (fixedContributor != maxContributor)) {
	    l.width = Length();
	    fixedContributor = 0;
	}
    }

    l.maxWidth = QMAX(l.maxWidth, l.minWidth);
#ifdef DEBUG_LAYOUT
    qDebug("col %d, final min=%d, max=%d, width=%d(%d)", effCol, l.minWidth, l.maxWidth, l.width.value,  l.width.type );
#endif

    // ### we need to add col elements aswell
}


void AutoTableLayout::fullRecalc()
{
    percentagesDirty = true;
    hasPercent = false;
    effWidthDirty = true;

    int nEffCols = table->numEffCols();
    layoutStruct.resize( nEffCols );
    layoutStruct.fill( Layout() );
    spanCells.fill( 0 );

    RenderObject *child = table->firstChild();
    Length grpWidth;
    int cCol = 0;
    while ( child ) {
	if ( child->isTableCol() ) {
	    RenderTableCol *col = static_cast<RenderTableCol *>(child);
	    int span = col->span();
	    if ( col->firstChild() ) {
		grpWidth = col->style()->width();
	    } else {
		Length w = col->style()->width();
		if ( w.isVariable() )
		    w = grpWidth;
		if ( (w.type == Fixed && w.value == 0) ||
		     (w.type == Percent && w.value == 0) )
		    w = Length();
		int cEffCol = table->colToEffCol( cCol );
#ifdef DEBUG_LAYOUT
		qDebug("    col element %d (eff=%d): Length=%d(%d), span=%d, effColSpan=%d",  cCol, cEffCol, w.value, w.type, span, table->spanOfEffCol(cEffCol ) );
#endif
		if ( (int)w.type != Variable && span == 1 && cEffCol < nEffCols ) {
		    if ( table->spanOfEffCol( cEffCol ) == 1 )
			layoutStruct[cEffCol].width = w;
		}
		cCol += span;
	    }
	} else {
	    break;
	}

	RenderObject *next = child->firstChild();
	if ( !next )
	    next = child->nextSibling();
	if ( !next && child->parent()->isTableCol() ) {
	    next = child->parent()->nextSibling();
	    grpWidth = Length();
	}
	child = next;
    }


    for ( int i = 0; i < nEffCols; i++ )
	recalcColumn( i );
}


void AutoTableLayout::calcMinMaxWidth()
{
#ifdef DEBUG_LAYOUT
    qDebug("AutoTableLayout::calcMinMaxWidth");
#endif
    fullRecalc();

    int spanMaxWidth = calcEffectiveWidth();

    int minWidth = 0;
    int maxWidth = 0;
    int maxPercent = 0;
    int maxNonPercent = 0;

    for ( unsigned int i = 0; i < layoutStruct.size(); i++ ) {
	minWidth += layoutStruct[i].effMinWidth;
	maxWidth += layoutStruct[i].effMaxWidth;
	if ( layoutStruct[i].effWidth.type == Percent ) {
	    int pw = (layoutStruct[i].effMaxWidth * 100 + 50) / layoutStruct[i].effWidth.value;
	    maxPercent = QMAX( pw,  maxPercent );
	} else {
	    maxNonPercent += layoutStruct[i].effMaxWidth;
	}
    }

    int totalpct = totalPercent();
    if ( totalpct < 100 ) {
	maxNonPercent = (maxNonPercent * 100 + 50) / (100-totalpct);
	maxWidth = QMAX( maxNonPercent,  maxWidth );
    }
    maxWidth = QMAX( maxWidth, maxPercent );
    maxWidth = QMAX( maxWidth, spanMaxWidth );

    int bs = table->bordersAndSpacing();
    minWidth += bs;
    maxWidth += bs;

    Length tw = table->style()->width();
    if ( tw.isFixed() ) {
	minWidth = QMAX( minWidth, tw.value );
	maxWidth = minWidth;
    }

    table->m_maxWidth = maxWidth;
    table->m_minWidth = minWidth;
#ifdef DEBUG_LAYOUT
    qDebug("    minWidth=%d, maxWidth=%d", table->m_minWidth, table->m_maxWidth );
#endif
}

/*
  This method takes care of colspans.
  effWidth is the same as width for cells without colspans. If we have colspans, they get modified.
 */
int AutoTableLayout::calcEffectiveWidth()
{
    int tMaxWidth = 0;

    unsigned int nEffCols = layoutStruct.size();
    int spacing = table->cellSpacing();
#ifdef DEBUG_LAYOUT
    qDebug("AutoTableLayout::calcEffectiveWidth for %d cols", nEffCols );
#endif
    for ( unsigned int i = 0; i < nEffCols; i++ ) {
	layoutStruct[i].effWidth = layoutStruct[i].width;
	layoutStruct[i].effMinWidth = layoutStruct[i].minWidth;
	layoutStruct[i].effMaxWidth = layoutStruct[i].maxWidth;
    }

    for ( unsigned int i = 0; i < spanCells.size(); i++ ) {
	RenderTableCell *cell = spanCells[i];
	if ( !cell )
	    break;
	int span = cell->colSpan();

	Length w = cell->style()->width();
	if ( !(w.type == Relative) && w.value == 0 )
	    w = Length(); // make it Variable

	int col = table->effColToCol( cell->col() );
	unsigned int lastCol = col;
	int cMinWidth = cell->minWidth();
	int cMaxWidth = cell->maxWidth();
	int totalPercent = 0;
	int minWidth = 0;
	int maxWidth = 0;
	bool allColsArePercent = true;
	bool allColsAreFixed = true;
	int fixedWidth = spacing;
	while ( lastCol < nEffCols && span > 0 ) {
	    switch( layoutStruct[lastCol].width.type ) {
	    case Percent:
		totalPercent += layoutStruct[lastCol].width.value;
		allColsAreFixed = false;
		break;
	    case Fixed:
		fixedWidth += layoutStruct[lastCol].width.value + spacing;
		allColsArePercent = false;
		layoutStruct[lastCol].effWidth = Length();
		break;
	    default:
		layoutStruct[lastCol].effWidth = Length();
		allColsArePercent = false;
		allColsAreFixed = false;
	    }
	    span -= table->spanOfEffCol( lastCol );
	    minWidth += layoutStruct[lastCol].effMinWidth;
	    maxWidth += layoutStruct[lastCol].effMaxWidth;
	    lastCol++;
	}
// 	qDebug("    colspan cell %p at effCol %d, span %d, type %d, value %d cmin=%d min=%d fixedwidth=%d", cell, col, span, w.type, w.value, cMinWidth, minWidth, fixedWidth );

	// adjust table max width if needed
	if ( w.type == Percent ) {
	    if ( totalPercent > w.value || allColsArePercent ) {
		// can't satify this condition, treat as variable
		w = Length();
	    } else {
		int spanMax = QMAX( maxWidth, cMaxWidth );
// 		qDebug("    adjusting tMaxWidth (%d): spanMax=%d, value=%d, totalPercent=%d", tMaxWidth, spanMax, w.value, totalPercent );
		tMaxWidth = QMAX( tMaxWidth, spanMax * 100 / w.value );

		// all non percent columns in the span get percent vlaues to sum up correctly.
		int percentMissing = w.value - totalPercent;
		int totalWidth = 0;
		for ( unsigned int pos = col; pos < lastCol; pos++ ) {
		    if ( !(layoutStruct[pos].width.type == Percent ) )
			totalWidth += layoutStruct[pos].effMaxWidth;
		}

		for ( unsigned int pos = col; pos < lastCol && totalWidth > 0; pos++ ) {
		    if ( !(layoutStruct[pos].width.type == Percent ) ) {
			int percent = percentMissing * layoutStruct[pos].effMaxWidth / totalWidth;
// 			qDebug("   col %d: setting percent value %d effMaxWidth=%d totalWidth=%d", pos, percent, layoutStruct[i].effMaxWidth, totalWidth );
			totalWidth -= layoutStruct[pos].effMaxWidth;
			percentMissing -= percent;
			if ( percent > 0 )
			    layoutStruct[pos].effWidth = Length( percent, Percent );
			else
			    layoutStruct[pos].effWidth = Length();
		    }
		}

	    }
	}

	if ( !(w.type == Percent ) ) {
	    // make sure minWidth and maxWidth of the spanning cell are honoured
	    if ( cMinWidth > minWidth ) {
		if ( allColsAreFixed ) {
// 		    qDebug("extending minWidth of cols %d-%d to %dpx currentMin=%d accroding to fixed sum %d", col, lastCol-1, cMinWidth, minWidth, fixedWidth );
		    for ( unsigned int pos = col; fixedWidth > 0 && pos < lastCol; pos++ ) {
			int w = QMAX( layoutStruct[pos].effMinWidth, cMinWidth * layoutStruct[pos].width.value / fixedWidth );
// 			qDebug("   col %d: min=%d, effMin=%d, new=%d", pos, layoutStruct[pos].effMinWidth, layoutStruct[pos].effMinWidth, w );
			fixedWidth -= layoutStruct[pos].width.value;
			cMinWidth -= w;
			layoutStruct[pos].effMinWidth = w;
		    }


		} else {
// 		    qDebug("extending minWidth of cols %d-%d to %dpx currentMin=%d", col, lastCol-1, cMinWidth, minWidth );
		    for ( unsigned int pos = col; minWidth > 0 && pos < lastCol; pos++ ) {
			int w = QMAX( layoutStruct[pos].effMinWidth, cMinWidth * layoutStruct[pos].effMinWidth / minWidth );
// 			qDebug("   col %d: min=%d, effMin=%d, new=%d", pos, layoutStruct[pos].effMinWidth, layoutStruct[pos].effMinWidth, w );
			minWidth -= layoutStruct[pos].effMinWidth;
			cMinWidth -= w;
			layoutStruct[pos].effMinWidth = w;
		    }
		}
	    }
	    if ( cMaxWidth > maxWidth ) {
// 		qDebug("extending maxWidth of cols %d-%d to %dpx", col, lastCol-1, cMaxWidth );
		for ( unsigned int pos = col; maxWidth > 0 && pos < lastCol; pos++ ) {
		    int w = QMAX( layoutStruct[pos].effMaxWidth, cMaxWidth * layoutStruct[pos].effMaxWidth / maxWidth );
// 		    qDebug("   col %d: max=%d, effMax=%d, new=%d", pos, layoutStruct[pos].effMaxWidth, layoutStruct[pos].effMaxWidth, w );
		    maxWidth -= layoutStruct[pos].effMaxWidth;
		    cMaxWidth -= w;
		    layoutStruct[pos].effMaxWidth = w;
		}
	    }
	}
    }
    effWidthDirty = false;

//     qDebug("calcEffectiveWidth: tMaxWidth=%d",  tMaxWidth );
    return tMaxWidth;
}

/* gets all cells that originate in a column and have a cellspan > 1
   Sorts them by increasing cellspan
*/
void AutoTableLayout::insertSpanCell( RenderTableCell *cell )
{
    if ( !cell || cell->colSpan() == 1 )
	return;

//     qDebug("inserting span cell %p with span %d", cell, cell->colSpan() );
    int size = spanCells.size();
    if ( !size || spanCells[size-1] != 0 ) {
	spanCells.resize( size + 10 );
	spanCells[size] = 0;
	size += 10;
    }

    // add them in sort. This is a slow algorithm, and a binary search or a fast sorting after collection would be better
    unsigned int pos = 0;
    int span = cell->colSpan();
    while ( pos < spanCells.size() && spanCells[pos] && span > spanCells[pos]->colSpan() )
	pos++;
    memmove( spanCells.data()+pos+1, spanCells.data()+pos, (size-pos-1)*sizeof( RenderTableCell * ) );
    spanCells[pos] = cell;
}


void AutoTableLayout::layout()
{
    // table layout based on the values collected in the layout structure.

    int tableWidth = table->width() - table->bordersAndSpacing();
    int available = tableWidth;
    int nEffCols = table->numEffCols();

    if ( nEffCols != (int)layoutStruct.size() ) {
	qWarning("WARNING: nEffCols is not equal to layoutstruct!" );
	fullRecalc();
	nEffCols = table->numEffCols();
    }
#ifdef DEBUG_LAYOUT
    qDebug("AutoTableLayout::layout()");
#endif

    if ( effWidthDirty )
	calcEffectiveWidth();

#ifdef DEBUG_LAYOUT
    qDebug("    tableWidth=%d,  nEffCols=%d", tableWidth,  nEffCols );
    for ( int i = 0; i < nEffCols; i++ ) {
	qDebug("    effcol %d is of type %d value %d, minWidth=%d, maxWidth=%d",
	       i, layoutStruct[i].width.type, layoutStruct[i].width.value,
	       layoutStruct[i].minWidth, layoutStruct[i].maxWidth );
	qDebug("        effective: type %d value %d, minWidth=%d, maxWidth=%d",
	       layoutStruct[i].effWidth.type, layoutStruct[i].effWidth.value,
	       layoutStruct[i].effMinWidth, layoutStruct[i].effMaxWidth );
    }
#endif

    bool havePercent = false;
    bool haveRelative = false;
    int totalRelative = 0;
    int numVariable = 0;
    int numFixed = 0;
    int totalVariable = 0;
    int totalFixed = 0;
    int totalPercent = 0;
    int allocVariable = 0;

    // fill up every cell with it's minWidth
    for ( int i = 0; i < nEffCols; i++ ) {
	int w = layoutStruct[i].effMinWidth;
	layoutStruct[i].calcWidth = w;
	available -= w;
	Length &width = layoutStruct[i].effWidth;
        switch( width.type) {
        case Percent:
	    havePercent = true;
	    totalPercent += width.value;
            break;
        case Relative:
	    haveRelative = true;
	    totalRelative += width.value;
            break;
        case Fixed:
            numFixed++;
            totalFixed += width.value;
            break;
        case Variable:
        case Static:
	    numVariable++;
	    totalVariable += layoutStruct[i].effMaxWidth;
	    allocVariable += w;
	}
    }

    // then allocate width to percent cols
    if ( available > 0 && havePercent ) {
	int totalAllocated = 0;
	for ( int i = 0; i < nEffCols; i++ ) {
	    Length &width = layoutStruct[i].effWidth;
	    if ( width.type == Percent ) {
		int w = QMAX( layoutStruct[i].minWidth, width.minWidth( tableWidth ) );
		totalAllocated += w;
#if 0
		if ( !htmlHacks )
		    // we need to add paddings to the width (at least mozilla does it)
		    w += cell->paddings
#endif
		available += layoutStruct[i].calcWidth - w;
		layoutStruct[i].calcWidth = w;
	    }
	}
	if ( totalPercent > 100 ) {
	    // remove overallocated space from the last columns
	    int excess = tableWidth*(totalPercent-100)/100;
	    for ( int i = nEffCols-1; i >= 0; i-- ) {
		if ( layoutStruct[i].effWidth.type == Percent ) {
		    int w = layoutStruct[i].calcWidth;
		    int reduction = QMIN( w,  excess );
		    // the lines below might look inconsistent, but that's the way it's handled in mozilla
		    excess -= reduction;
		    int newWidth = QMAX( w - reduction,  layoutStruct[i].effMinWidth );
		    available += w - newWidth;
		    layoutStruct[i].calcWidth = newWidth;
		    //qDebug("col %d: reducing to %d px (reduction=%d)", i, newWidth, reduction );
		}
	    }
	}
    }
#ifdef DEBUG_LAYOUT
    qDebug("percent satisfied: available is %d", available);
#endif

    // then allocate width to fixed cols
    if ( available > 0 ) {
	for ( int i = 0; i < nEffCols; i++ ) {
	    Length &width = layoutStruct[i].effWidth;
	    if ( width.type == Fixed && width.value > layoutStruct[i].calcWidth ) {
		int w = width.value;
		available += layoutStruct[i].calcWidth - w;
		layoutStruct[i].calcWidth = w;
            }
	}
    }
#ifdef DEBUG_LAYOUT
    qDebug("fixed satisfied: available is %d", available);
#endif

    // now satisfy relative
    if ( available > 0 ) {
	for ( int i = 0; i < nEffCols; i++ ) {
	    Length &width = layoutStruct[i].effWidth;
	    if ( width.type == Relative && width.value != 0 ) {
		// width=0* gets effMinWidth.
		int w = width.value*tableWidth/totalRelative;
		available += layoutStruct[i].calcWidth - w;
		layoutStruct[i].calcWidth = w;
	    }
	}
    }

    // now satisfy variable
    if ( available > 0 && numVariable ) {
	available += allocVariable; // this gets redistributed
// 	qDebug("redistributing %dpx to %d variable columns. totalVariable=%d",  available,  numVariable,  totalVariable );
	for ( int i = 0; i < nEffCols; i++ ) {
	    Length &width = layoutStruct[i].effWidth;
	    if ( width.type == Variable && totalVariable != 0 ) {
		int w = QMAX( layoutStruct[i].calcWidth, available * layoutStruct[i].effMaxWidth / totalVariable );
		available -= w;
		totalVariable -= layoutStruct[i].effMaxWidth;
		layoutStruct[i].calcWidth = w;
	    }
	}
    }
#ifdef DEBUG_LAYOUT
    qDebug("variable satisfied: available is %d",  available );
#endif

#if 0
    if ( available > 0 ) {
        qDebug("distributing %dpx equally to %d variable columns.",  available,  numVariable );
        // still have some width to spread
        for ( int i = 0; i < nEffCols; i++ ) {
            Length &width = layoutStruct[i].effWidth;
            if ( width.type == Variable ) {
                int w = available/numVariable;
                available -= w;
                numVariable--;
                layoutStruct[i].calcWidth += w;
            }
        }
    }
#endif

    // spread over fixed colums
    if ( available > 0 && numFixed) {
	// still have some width to spread, distribute to fixed columns
	for ( int i = 0; i < nEffCols; i++ ) {
            Length &width = layoutStruct[i].effWidth;
            if ( width.isFixed() ) {
                int w = available * width.value / totalFixed;
                available -= w;
                layoutStruct[i].calcWidth += w;
            }
	}
    }

    // spread over the rest
    if ( available > 0 ) {
        int total = nEffCols;
        // still have some width to spread
        for ( int i = 0; i < nEffCols; i++ ) {
            int w = available/total;
            available -= w;
            total--;
            layoutStruct[i].calcWidth += w;
        }
    }

    // if we have overallocated, reduce from the back
    if ( available < 0 ) {
	for ( int i = nEffCols-1; i >= 0; i-- ) {
	    int space = layoutStruct[i].calcWidth - layoutStruct[i].effMinWidth;
	    space = QMIN( space, -available );
	    layoutStruct[i].calcWidth -= space;
	    available += space;
	    if ( available >= 0 )
		break;
	}
    }

    //qDebug( "    final available=%d", available );

    int pos = 0;
    for ( int i = 0; i < nEffCols; i++ ) {
#ifdef DEBUG_LAYOUT
	qDebug("col %d: %d (width %d)", i, pos, layoutStruct[i].calcWidth );
#endif
	table->columnPos[i] = pos;
	pos += layoutStruct[i].calcWidth;
    }
    table->columnPos[table->columnPos.size()-1] = pos;

}


void AutoTableLayout::calcPercentages() const
{
    total_percent = 0;
    for ( unsigned int i = 0; i < layoutStruct.size(); i++ ) {
	if ( layoutStruct[i].width.type == Percent )
	    total_percent += layoutStruct[i].width.value;
    }
    percentagesDirty = false;
}
