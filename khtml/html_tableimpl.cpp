/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1997 Martin Jones (mjones@kde.org)
 *           (C) 1997 Torben Weis (weis@kde.org)
 *           (C) 1998 Waldo Bastian (bastian@kde.org)
 *           (C) 1999 Lars Knoll (knoll@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
// -------------------------------------------------------------------------

// ### FIXME: get rid of setStyle calls...
// ### cellpadding and spacing should be converted to Length
#undef TABLE_DEBUG
#define DEBUG_LAYOUT

#include <qlist.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qdrawutil.h>

#include "dom_string.h"
#include "dom_nodeimpl.h"
#include "dom_exception.h"

#include "html_misc.h"
#include "html_element.h"
#include "html_table.h"
#include "html_tableimpl.h"
#include "html_documentimpl.h"
using namespace DOM;

#include "khtmlattrs.h"
#include "khtmlstyle.h"

#include <stdio.h>
#include <assert.h>

#define FOR_EACH_CELL(r,c,cell) \
    for ( unsigned int r = 0; r < totalRows; r++ )                    \
    {                                                                 \
        for ( unsigned int c = 0; c < totalCols; c++ )                \
        {                                                             \
            HTMLTableCellElementImpl *cell = cells[r][c];             \
            if (!cell)                                                \
                continue;                                             \
	    if ( (c < totalCols - 1) && (cell == cells[r][c+1]) )     \
	        continue;                                             \
	    if ( (r < totalRows - 1) && (cells[r+1][c] == cell) )     \
	        continue;                                             

#define END_FOR_EACH } }


HTMLTableElementImpl::HTMLTableElementImpl(DocumentImpl *doc)
  : HTMLBlockElementImpl(doc)
{
    tCaption = 0;
    cols = 0;
    head = 0;
    foot = 0;
    firstBody = 0;

    width = -1;
    availableWidth = 0;
    incremental = false;
    maxWidth = 0;

    spacing = 2;
    padding = 6;
    rules = None;
    frame = Void;
    border = 0;

    row = 0;
    col = 0;

    maxColSpan = 1;
    totalColInfos = 0;

    setBlocking();
    setParsing();
    
    columnPos.resize( 2 );
    colMaxWidth.resize( 1 );
    colMinWidth.resize( 1 );
    colValue.resize(1);
    colType.resize(1);
    actColWidth.resize(1);
    columnPos.fill( 0 );
    colMaxWidth.fill( 0 );
    colMinWidth.fill( 0 );
    colValue.fill(0);
    colType.fill(Variable);
    actColWidth.fill(0);

    columnPos[0] = border + spacing;

    totalCols = 1;   // this should be expanded to the maximum number of cols
                     // by the first row parsed
    totalRows = 1;
    allocRows = 5;   // allocate five rows initially

    cells = new HTMLTableCellElementImpl ** [allocRows];

    for ( unsigned int r = 0; r < allocRows; r++ )
    {
	cells[r] = new HTMLTableCellElementImpl * [totalCols];
	memset( cells[r], 0, totalCols * sizeof( HTMLTableCellElementImpl * ));
    }
}

HTMLTableElementImpl::~HTMLTableElementImpl()
{
    for ( unsigned int r = 0; r < allocRows; r++ )
	delete [] cells[r];
    delete [] cells;
}

const DOMString HTMLTableElementImpl::nodeName() const
{
    return "TABLE";
}

ushort HTMLTableElementImpl::id() const
{
    return ID_TABLE;
}

void HTMLTableElementImpl::setCaption( HTMLTableCaptionElementImpl *c )
{
    if(tCaption) 
	replaceChild ( c, tCaption );
    else
	insertBefore( c, firstChild() );
    tCaption = c;
    c->setTable(this);
}

void HTMLTableElementImpl::setTHead( HTMLTableSectionElementImpl *s )
{
    if(head) 
	replaceChild ( s, head );
    else if( foot )
	insertBefore( s, foot );
    else if( firstBody )
	insertBefore( s, firstBody );
    else 
	appendChild( s );
    head = s;
    s->setTable(this);
}

void HTMLTableElementImpl::setTFoot( HTMLTableSectionElementImpl *s )
{
    if(foot) 
	replaceChild ( s, foot );
    else if( firstBody )
	insertBefore( s, firstBody );
    else 
	appendChild( s );
    foot = s;
    s->setTable(this);
}

HTMLElementImpl *HTMLTableElementImpl::createTHead(  )
{
    if(!head)
    {
	head = new HTMLTableSectionElementImpl(document, ID_THEAD);
	head->setTable(this);
	if(foot)
	    insertBefore( head, foot );
	if(firstBody)
	    insertBefore( head, firstBody);
	else
	    appendChild(head);
    }
    return head;
}

void HTMLTableElementImpl::deleteTHead(  )
{
    if(head) removeChild(head);
    head = 0;
}

HTMLElementImpl *HTMLTableElementImpl::createTFoot(  )
{
    if(!foot)
    {
	foot = new HTMLTableSectionElementImpl(document, ID_TFOOT);
	foot->setTable(this);
	if(firstBody)
	    insertBefore( foot, firstBody );
	else
	    appendChild(foot);
    }
    return foot;
}

void HTMLTableElementImpl::deleteTFoot(  )
{
    if(foot) removeChild(foot);
    foot = 0;
}

HTMLElementImpl *HTMLTableElementImpl::createCaption(  )
{
    if(!tCaption)
    {
	tCaption = new HTMLTableCaptionElementImpl(document);
	tCaption->setTable(this);
	insertBefore( tCaption, firstChild() );
    }
    return tCaption;
}

void HTMLTableElementImpl::deleteCaption(  )
{
    if(tCaption) removeChild(tCaption);
    tCaption = 0;
}

HTMLElementImpl *HTMLTableElementImpl::insertRow( long /*index*/ )
{
    // ###
    return 0;
}

void HTMLTableElementImpl::deleteRow( long /*index*/ )
{
    // ###
}

NodeImpl *HTMLTableElementImpl::addChild(NodeImpl *child)
{
#ifdef DEBUG_LAYOUT
    printf("%s(Table)::addChild( %s )\n", nodeName().string().ascii(), child->nodeName().string().ascii());
#endif

    switch(child->id())
    {
    case ID_CAPTION:
	setCaption(static_cast<HTMLTableCaptionElementImpl *>(child));
	break;
    case ID_COL:
    case ID_COLGROUP:
	// these have to come before the table definition!
	if(head || foot || firstBody) 
	    throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);
	HTMLBlockElementImpl::addChild(child);
	if(!cols) cols = static_cast<HTMLTableColElementImpl *>(child);
	incremental = true;
	return child;
    case ID_THEAD:
	if(incremental && !columnPos[totalCols]) calcColWidth();
	setTHead(static_cast<HTMLTableSectionElementImpl *>(child));
	break;
    case ID_TFOOT:
	if(incremental && !columnPos[totalCols]) calcColWidth();
	setTFoot(static_cast<HTMLTableSectionElementImpl *>(child));
	break;
    case ID_TBODY:
	if(incremental && !columnPos[totalCols]) calcColWidth();
	if(!firstBody) 
	    firstBody = static_cast<HTMLTableSectionElementImpl *>(child);
    default:
	child->setAvailableWidth(0);
 	NodeBaseImpl::addChild(child);
	break;
    }
	
    static_cast<HTMLTablePartElementImpl *>(child)->setTable(this);
    return child;
}

void HTMLTableElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_WIDTH:
	predefinedWidth = attr->val()->toLength();
	width = predefinedWidth.minWidth(availableWidth);
    	break;
    case ATTR_BORDER:
	if(attr->val()->l == 0)
	    border = 1;
	else
	    border = attr->val()->toInt();
	// wanted by HTML4 specs
	if(!border)
	    frame = Void, rules = None;
	else
	    frame = Box, rules = All;
	break;
    case ATTR_BGCOLOR:
	setNamedColor( bg, attr->value().string() );
    case ATTR_FRAME:
	if ( strcasecmp( attr->value(), "void" ) == 0 )
	    frame = Void;
	else if ( strcasecmp( attr->value(), "border" ) == 0 )
	    frame = Box;
	else if ( strcasecmp( attr->value(), "box" ) == 0 )
	    frame = Box;
	else if ( strcasecmp( attr->value(), "hsides" ) == 0 )
	    frame = Hsides;
	else if ( strcasecmp( attr->value(), "vsides" ) == 0 )
	    frame = Vsides;
	else if ( strcasecmp( attr->value(), "above" ) == 0 )
	    frame = Above;
	else if ( strcasecmp( attr->value(), "below" ) == 0 )
	    frame = Below;
	else if ( strcasecmp( attr->value(), "lhs" ) == 0 )
	    frame = Lhs;
	else if ( strcasecmp( attr->value(), "rhs" ) == 0 )
	    frame = Rhs;
	break;
    case ATTR_RULES:
	if ( strcasecmp( attr->value(), "none" ) == 0 )
	    rules = None;
	else if ( strcasecmp( attr->value(), "groups" ) == 0 )
	    rules = Groups;
	else if ( strcasecmp( attr->value(), "rows" ) == 0 )
	    rules = Rows;
	else if ( strcasecmp( attr->value(), "cols" ) == 0 )
	    rules = Cols;
	else if ( strcasecmp( attr->value(), "all" ) == 0 )
	    rules = All;
    case ATTR_CELLSPACING:
	spacing = attr->val()->toInt();
	break;
    case ATTR_CELLPADDING:
	padding = attr->val()->toInt();
	break;
    case ATTR_COLS:
    {
	int c;
	c = attr->val()->toInt();
	addColumns(c-totalCols);
	break;
    case ATTR_ALIGN:
	// vertical alignment with respect to the current baseline of the text
	// right or left means floating images
	if ( strcasecmp( attr->value(), "left" ) == 0 )
	    halign = Left;
	else if ( strcasecmp( attr->value(), "right" ) == 0 )
	    halign = Right;
	else if ( strcasecmp( attr->value(), "center" ) == 0 )
	    halign = HCenter;	    
	break;
    }
    default:
	HTMLBlockElementImpl::parseAttribute(attr);
    }
}

void HTMLTableElementImpl::startRow()
{
    while ( col < totalCols && cells[row][col] != 0 )
	col++;
    if ( col )
	row++;
    col = 0;
    if(row > totalRows) totalRows = row;
}

void HTMLTableElementImpl::addCell( HTMLTableCellElementImpl *cell )
{
    while ( col < totalCols && cells[row][col] != 0L )
	col++;
    setCells( row, col, cell );

    col++;
}

void HTMLTableElementImpl::setCells( unsigned int r, unsigned int c, 
				     HTMLTableCellElementImpl *cell )
{
#ifdef TABLE_DEBUG
    printf("span = %ld/%ld\n", cell->rowSpan(), cell->colSpan());
#endif
    cell->setRow(r);
    cell->setCol(c);

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

void HTMLTableElementImpl::addRows( int num )
{
    HTMLTableCellElementImpl ***newRows = 
	new HTMLTableCellElementImpl ** [allocRows + num];
    memcpy( newRows, cells, allocRows * sizeof(HTMLTableCellElementImpl **) );
    delete [] cells;
    cells = newRows;

    for ( unsigned int r = allocRows; r < allocRows + num; r++ )
    {
	cells[r] = new HTMLTableCellElementImpl * [totalCols];
	memset( cells[r], 0, totalCols * sizeof( HTMLTableCellElementImpl * ));
    }

    allocRows += num;
}

void HTMLTableElementImpl::addColumns( int num )
{
#ifdef Table_DEBUG
    printf("addColumns() totalCols=%d new=%d\n", totalCols, num);
#endif
    HTMLTableCellElementImpl **newCells;

    int newCols = totalCols + num;
    // resize the col structs to the number of columns
    columnPos.resize(newCols+1);
    memset( columnPos.data() + totalCols + 1, 0, num*sizeof(int));
    colMaxWidth.resize(newCols);
    memset( colMaxWidth.data() + totalCols , 0, num*sizeof(int));
    colMinWidth.resize(newCols);
    memset( colMinWidth.data() + totalCols , 0, num*sizeof(int));
    colValue.resize(newCols);
    memset( colValue.data() + totalCols , 0, num*sizeof(int));
    colType.resize(newCols);
    memset( colType.data() + totalCols , 0, num*sizeof(LengthType));
    actColWidth.resize(newCols);
    memset( actColWidth.data() + totalCols , 0, num*sizeof(LengthType));
 
    for ( unsigned int r = 0; r < allocRows; r++ )
    {
	newCells = new HTMLTableCellElementImpl * [newCols];
	memcpy( newCells, cells[r], 
		totalCols * sizeof( HTMLTableCellElementImpl * ) );
	memset( newCells + totalCols, 0, 
		num * sizeof( HTMLTableCellElementImpl * ) );
	delete [] cells[r];
	cells[r] = newCells;
    }

    totalCols = newCols;

}

void HTMLTableElementImpl::addColInfo(HTMLTableCellElementImpl *cell)
{
    int _startCol = cell->col();
    int _colSpan = cell->colSpan();
    int _minSize = cell->getMinWidth() + padding;
    int _maxSize = cell->getMaxWidth() + padding;
    Length _width = cell->predefWidth();
    addColInfo(_startCol, _colSpan, _minSize, _maxSize, _width);
}

void HTMLTableElementImpl::addColInfo(int _startCol, int _colSpan, 
				      int _minSize, int _maxSize, 
				      Length _width)
{
#ifdef TABLE_DEBUG
    printf("addColInfo():\n");
    printf("    startCol=%d span=%d\n", _startCol, _colSpan);
    printf("    min=%d max=%d\n", _minSize, _maxSize );
#endif

    if (_startCol + _colSpan > (int) totalCols)
	addColumns(totalCols - _startCol - _colSpan);

    if(_colSpan == 1)
    {
	// add the stuff directly
	if (_width.type > colType[_startCol])
	{
	    colValue[_startCol] = _width.value;
	    colType[_startCol] = _width.type;
	    if(_width.type == Percent && _width.value > 100)
		colValue[_startCol] = 100;
	}
	if(colMaxWidth[_startCol] < _maxSize)
	    colMaxWidth[_startCol] = _maxSize;
	if(colMinWidth[_startCol] < _minSize)
	    colMinWidth[_startCol] = _minSize;
    }
    else
    {
	int fixedCount = 0;
	int percentCount = 0;
	int relCount = 0;
	int varCount = 0;
	int currMinSize = 0;
	int currMaxSize = 0;
	// Update minimum sizes
	for (int k = _colSpan; k; k--)
	{
	    int pos = _startCol + k - 1;
#ifdef TABLE_DEBUG
	    printf("looking at col %d, totalCols = %d\n", pos, totalCols);
#endif
	    currMinSize += colMinWidth[pos];
	    currMaxSize += colMaxWidth[pos];
	    switch( colType[pos] )
	    {
	    case Fixed:
		fixedCount++;
		break;
	    case Percent:
		percentCount++;
		break;
	    case Relative:
		relCount++;
		break;
	    default:
		varCount++;
		break;
	    } 
	} 
#ifdef TABLE_DEBUG
	printf("Col = %d span = %d, fix = %d, perc = %d, rel = %d, var = %d, min = %d, currMin = %d\n",
	       _startCol, _colSpan, fixedCount, percentCount, relCount, 
	       varCount, _minSize, currMinSize );
#endif 
	if (currMinSize < _minSize)
	{
	    int tooAdd = _minSize - currMinSize;
	    // lets try to add the minwidth to the minWidth of the fixed 
	    // columns. This is possible as long as the minWidth is smaller
	    // than the fixed width...
	    if(fixedCount || percentCount)
	    {
#ifdef TABLE_DEBUG
		printf("Spreading %d across nonVar cols\n", tooAdd);
#endif
		tooAdd = addColsMinWidthNonVar(_startCol, _colSpan, tooAdd, 
					    fixedCount + percentCount);
	    }
	    if ( tooAdd )
	    {
		if ( (fixedCount == _colSpan) || 
		     (percentCount == _colSpan) ||
		     (varCount == _colSpan) ||
		     (relCount == _colSpan) )
		{
#ifdef TABLE_DEBUG
		    printf("Spreading %d pixels equally\n", tooAdd);
#endif
		    // Spread extra width across all columns equally
		    addColsMinWidthEqual(_startCol, _colSpan, tooAdd);
		}
		else
		{
#ifdef TABLE_DEBUG
		    printf("Spreading %d pixels\n", tooAdd);
#endif
		    if (varCount > 0)
		    {
			// Spread extra width across all variable columns 
			addColsMinWidthVar(_startCol, _colSpan, tooAdd, 
					   varCount);
		    }
		    else
		    {
			// Spread extra width across all columns equally
			addColsMinWidthEqual(_startCol, _colSpan, tooAdd);
		    }
		}
	    }
	}
	if (currMaxSize < _maxSize)
	{
	    int tooAdd = _maxSize - currMaxSize;
	    if ( (varCount == 0) || 
		 (varCount == _colSpan) )
	    {
		// Spread extra width across all columns equally
		addColsMaxWidthEqual(_startCol, _colSpan, tooAdd);                        
	    }
	    else
	    {
		// Spread extra width across all Variable columns 
		addColsMaxWidthVar(_startCol, _colSpan, tooAdd, varCount);
	    }
	}
    }
#ifdef TABLE_DEBUG
    printf("  end: min=%d max=%d act=%d\n", colMinWidth[_startCol], 
	   colMaxWidth[_startCol], actColWidth[_startCol]);
#endif    
}

void HTMLTableElementImpl::addColMinWidth(int k, int delta)
{     
#ifdef TABLE_DEBUG              	    
printf("Adding %d pixels to col %d: %d --> %d\n",
     delta, k, columnPos[k], columnPos[k]+delta);
#endif

    colMinWidth[k] += delta;

    // Make sure colMaxWidth > columnPos
    if (colMinWidth[k] > colMaxWidth[k])
 	colMaxWidth[k] = colMinWidth[k];
}

void HTMLTableElementImpl::addColsMinWidthEqual(int kol, int span, int tooAdd)
{
    // Spread extra width across all columns equally
    for (int k = span; k; k--)
    {
        int delta = tooAdd / k;
        addColMinWidth(kol+k-1, delta);
        tooAdd -= delta;
    }
}

void HTMLTableElementImpl::addColsMinWidthVar(int kol, int span, int tooAdd, 
					      int varCount)
{
    assert( varCount > 0);
    // Spread extra width across all variable columns 
    for (int k = span; k; k--)
    {
        if ( colType[kol + k - 1 ] == Variable )
        {
            int delta = tooAdd / varCount;
            addColMinWidth(kol+k-1, delta);
            varCount--;
            tooAdd -= delta;
        }
    }
}

int HTMLTableElementImpl::addColsMinWidthNonVar(int kol, int span, 
					     int tooAdd, int count)
{
    assert( count > 0);
    // Spread extra width across all non-fixed columns 
    int remaining = 0;

    for (int k = span; k; k--)
    {
	int pos = kol + k - 1;
        if ( colType[pos] == Fixed && colMinWidth[pos] < colValue[pos])
	    remaining += colValue[pos] - colMinWidth[pos];
        if ( colType[pos] == Percent )
	{
	    int space = colMinWidth[pos] < colValue[pos]*availableWidth/100;
	    if(colMinWidth[pos] < space)
		remaining += colValue[pos] - space;
	}
    }
    int adding = remaining;
    if(adding > tooAdd) adding = tooAdd;
    for (int k = span; k; k--)
    {
	int pos = kol + k - 1;
	int space = 0;
        switch( colType[pos] )
        {
	case Fixed:
	    space = colValue[pos] - colMinWidth[pos];
	    break;
	case Percent:
	    space = colMinWidth[pos] < colValue[pos]*availableWidth/100;
	    break;
	default:
	    break;
	}
	if(space > 0)
	{
            int delta = 0;
	    if(remaining)
		delta = adding * (colMinWidth[pos] < colValue[pos])
		    / remaining;
            addColMinWidth(kol+k-1, delta);
            adding -= delta;
            remaining -= space;
        }
    }
    return tooAdd - adding;
}

void HTMLTableElementImpl::addColsMinWidthNonFix(int kol, int span, 
						 int tooAdd, int nonFixedCount)
{
    assert( nonFixedCount > 0);
    // Spread extra width across all non-fixed columns 
    for (int k = span; k; k--)
    {
        if ( colType[kol + k - 1] != Fixed )
        {
            int delta = tooAdd / nonFixedCount;
            addColMinWidth(kol+k-1, delta);
            nonFixedCount--;
            tooAdd -= delta;
        }
    }
}

void HTMLTableElementImpl::addColsMaxWidthEqual(int kol, int span, int tooAdd)
{
    // Spread extra width across all columns equally
    for (int k = span; k; k--)
    {
        int delta = tooAdd / k;
        colMaxWidth[kol + k - 1] += delta;
        tooAdd -= delta;
    }
}

void HTMLTableElementImpl::addColsMaxWidthVar(int kol, int span, int tooAdd, 
					      int varCount)
{
    assert( varCount > 0);
    // Spread extra width across all non-fixed columns 
    for (int k = span; k; k--)
    {
        if ( colType[kol + k - 1] == Variable )
        {
            int delta = tooAdd / varCount;
            varCount--;
            colMaxWidth[kol + k - 1] += delta;
            tooAdd -= delta;
        }
    }
}

void HTMLTableElementImpl::calcColWidth()
{
    printf("calcColWidth: doing incremental layout!\n");

    width = predefinedWidth.minWidth(availableWidth);

    NodeImpl *col = cols;

    // gather all width...
    unsigned int column = 0;
    while(col && (col->id() == ID_COL || col->id() == ID_COLGROUP))
    {
	HTMLTableColElementImpl *c = static_cast<HTMLTableColElementImpl *>(col);
	Length w = c->width();
	NodeImpl *child = c->firstChild();
	if(!child)
	{
	    unsigned int span = c->span();
	    if(column + span > totalCols)
		addColumns(column + span - totalCols);
	    while(span)
	    {
		colValue[column] = w.value;
		colType[column] = w.type;
		column++;
		span--;
	    }
	}
	else
	{
	    while(child)
	    {
		c = static_cast<HTMLTableColElementImpl *>(child);
		Length childWidth = c->width();
		if(childWidth.isUndefined())
		    childWidth = w;
		int span = c->span();
		if(column + span > totalCols)
		    addColumns(column + span - totalCols);
		while(span)
		{
		    colValue[column] = w.value;
		    colType[column] = w.type;
		    column++;
		    span--;
		}
		child = child->nextSibling();
	    }
	}
	col = col->nextSibling();
    }
    printf("found %d columns\n", totalCols);

    int remaining = width;
    if(frame & Lhs) remaining -= border;
    if(frame & Rhs) remaining -= border;
    remaining -= (totalCols+1)*spacing;

    unsigned int i;
    int numVar = 0;
    int numRel = 0;
    int numFix = 0;
    for(i=0; i<totalCols; i++)
    {
	switch(colType[i])
	{
	case Variable:
	    actColWidth[i] = 0;
	    numVar++;
	    break;
	case Percent:
	    actColWidth[i] = colValue[i]*availableWidth/100;
	    numFix++;
	    break;
	case Fixed:
	    actColWidth[i] = colValue[i];
	    numFix++;
	    break;
	case Relative:
	    // ###
	    actColWidth[i] = 0;
	    numRel++;
	    break;
	}
	remaining -= actColWidth[i];
    }

    if(numVar + numRel > 1)
    {
	incremental = false;
	printf("no incremental layout possible!\n");
	return;
    }

    // can't format, if we don't have the total width...
    if(!width && (numVar | numRel) ) 
    {
	incremental = false;
	printf("no incremental layout possible!\n");
	return;
    }

    // do wee need to enlarge things?
    if(remaining < 0) width -= remaining, remaining = 0;

    if(numVar || numRel)
    {
	if(!remaining < padding) 
	{
	    incremental = false;
	    printf("no incremental layout possible!\n");
	    return;
	}
	bool found = false;
	for(i=0; !found && i<totalCols; i++)
	{
	    switch(colType[i])
	    {
	    case Variable:
	    case Relative:
		actColWidth[i] = remaining;
		found = true;
	    case Percent:
	    case Fixed:
		break;
	    }
	}
    }

    columnPos.resize( totalCols + 1 );
    int pos = spacing;
    if(frame & Lhs) pos += border;
    for(i=0; i<=totalCols; i++)
    {
	printf("setting colPos: col=%d width=%d pos=%d\n", i, actColWidth[i], pos);
	columnPos[i] = pos;
	pos += actColWidth[i] + spacing;
    }

    setBlocking(false);
}


// layouting function used for non incremental tables
// (autolayout algorithm)                        
void HTMLTableElementImpl::calcColWidthII(void)
{
// Even if it's almost impossible to satisfy every possible
// combination of width given, one should still try.
// 
// 1. calculate min and max width for every column
    // --> this step is already done during adding (see addColInfo())
// 2. calc min and max width for the table
// 3. set table width to max(minWidth, min(available, maxWidth))
// 4. set columns to minimum width.
    // --> done in addColInfo()
// 5. set percentage columns
    // --> done in addColInfo()
// 6. spread width across relative columns
// 7. spread remaining widths across variable columns
#ifdef TABLE_DEBUG
    printf("START calcColWidthII() this = %p\n", this);
    printf("---- %d ----\n", totalColInfos);
    printf("maxColSpan = %d\n", maxColSpan);
#endif

    // 2. calc min and max width for the table
    minWidth = border + border + spacing;
    int actWidth = border + border + spacing;
    maxWidth = border + border + spacing;
    int totalWidthPercent = 0;
    int totalFixed = 0; 
    int totalPercent = 0;
    int totalRel = 0;
    int totalVar = 0;
    int minFixed = 0;
    int minPercent = 0;
    int minRel = 0;
    int minVar = 0;
    int maxRel = 0;
    int maxVar = 0;
    int numFixed = 0;
    int numPercent = 0;
    int numRel = 0;
    int numVar = 0;
    actColWidth.fill(0);

    unsigned int i;
    for(i = 0; i < totalCols; i++)
    {
	switch(colType[i])
	{
	case Fixed:
	    actColWidth[i] = colValue[i];
	    maxWidth += actColWidth[i] + spacing;
	    minWidth += actColWidth[i] + spacing;
	    // we use actColWidth here, might be bigger than colValue!
	    totalFixed += actColWidth[i];
	    minFixed += colMinWidth[i];
	    numFixed++;
	    break;
	case Percent:
	    // if the table is not fixed width, we use the available space
	    if(!width)
		actColWidth[i] = colValue[i]*availableWidth/100;
	    else
		actColWidth[i] = colValue[i]*width/100;
	    maxWidth += actColWidth[i] + spacing;
	    minWidth += colMinWidth[i] + spacing;
	    totalPercent += colValue[i];
	    totalWidthPercent += actColWidth[i];
	    minPercent += colMinWidth[i];
	    numPercent++;
	    break;
	case Relative:
	    maxWidth += colMaxWidth[i] + spacing;
	    minWidth += colMinWidth[i] + spacing;
	    totalRel += colValue[i];
	    minRel += colMinWidth[i];
	    maxRel += colMaxWidth[i];
	    numRel++;
	    break;
	case Variable:
	    actColWidth[i] = colMinWidth[i];
	    maxWidth += colMaxWidth[i] + spacing;
	    minWidth += colMinWidth[i] + spacing;
	    totalVar += colMinWidth[i];
	    minVar += colMinWidth[i];
	    maxVar += colMaxWidth[i];
	    numVar++;
	}
	if(actColWidth[i] < colMinWidth[i])
	    actColWidth[i] = colMinWidth[i];
	actWidth += actColWidth[i] + spacing;
    }
#ifdef TABLE_DEBUG
    for(int i = 1; i <= (int)totalCols; i++)
    {
	printf("Actual width col %d: %d\n", i, actColWidth[i-1]);
    }
#endif

    // 3. set table width to max(minWidth, min(available, maxWidth))
    //    or (if we have a predefined width): max(minWidth, predefinedWidth)
    int tableWidth;

    if(predefinedWidth.type > Relative) // Percent or fixed
    {
	tableWidth = predefinedWidth.minWidth(availableWidth);
	if(minWidth > tableWidth) tableWidth = minWidth;
    }
    else // width of table not declared
    {
	tableWidth = availableWidth < maxWidth ? 
	    availableWidth : maxWidth;
    }
#ifdef TABLE_DEBUG
    printf("table width set to %d\n", tableWidth);
    printf("table min/max %d/%d\n", minWidth, maxWidth);
#endif

// 6. spread width across relative columns
// ### implement!!!

// 7. spread remaining widths across variable columns

    int tooAdd = tableWidth - actWidth;      // what we can add
    int pref = maxRel + maxVar - minRel - minVar; // what we would like to add
#ifdef TABLE_DEBUG
    printf("adding another %d pixels! pref=%d\n", tooAdd, pref);
#endif
    bool wide = (tooAdd > pref);
    for(i = 0; i < totalCols; i++)
    {
	switch(colType[i])
	{
	case Fixed:
	case Percent:
	    break;
	case Relative:
	    // ###
	case Variable:
	    int cPref = (colMaxWidth[i] - colMinWidth[i]);
#ifdef TABLE_DEBUG
	    printf("col %d prefWidth=%d space=%d\n", i, colMaxWidth[i],
		colMaxWidth[i]-colMinWidth[i]);
#endif
	    if(wide)
	    {
		actColWidth[i] += cPref;
		tooAdd -= cPref;
		totalVar += cPref;
	    }
	    else if(pref)
	    {
		int delta =  cPref * tooAdd / pref;
		actColWidth[i] += delta;
		tooAdd -= delta;
		pref -= cPref;
		totalVar += delta;
	    }
	}
    }
    int num = numRel + numVar;

    if(tooAdd > 0)
    {
#ifdef TABLE_DEBUG
	printf("spreading %d pixels equally across variable cols!\n", tooAdd);
#endif
	for(i = 0; i < totalCols; i++)
	{
	    switch(colType[i])
	    {
	    case Fixed:
	    case Percent:
		break;
	    case Relative:
		// ###
	    case Variable:
		int delta =  tooAdd/num;
		actColWidth[i] += delta;
		num--;
		tooAdd -= delta;
	    }
	}
    }
    else if(tooAdd < 0)
    {
	tooAdd = -tooAdd;
	printf("table to wide, reducing...\n");
	if(numVar > 0 && totalVar - minVar > 0)
	{
	    printf("reducing variable cols...\n");
	    float percent = 1;
	    int space = totalVar - minVar;
	    if(space > tooAdd) percent = (float)tooAdd/((float)space);
	    for(i = 0; i < totalCols; i++)
	    {
		switch(colType[i])
		{
		case Fixed:
		case Percent:
		    break;
		case Relative:
		    // ###
		case Variable:
		{
		    int delta = (int)(actColWidth[i]-colMinWidth[i])*percent;
		    actColWidth[i] -= delta;
		    num--;
		    tooAdd -= delta;
		}
		}
	    }
	}
	if(tooAdd > 0 && numPercent && totalWidthPercent > minPercent)
	{
	    printf("reducing percent cols...\n");
	    float percent = 1;
	    int space = totalWidthPercent - minPercent;
	    printf("space available = %d\n", space);
	    if(space > tooAdd) percent = (float)tooAdd/((float)space);
	    printf("percent = %d\n", percent);
	    for(i = 0; i < totalCols; i++)
	    {
		switch(colType[i])
		{
		case Fixed:
		    break;
		case Percent:
		{
		    int delta =  (int)(actColWidth[i]-colMinWidth[i])*percent;
		    printf("col %d: reducing by %d\n", i+1, delta);
		    actColWidth[i] -= delta;
		    num--;
		    tooAdd -= delta;
		    break;
		}
		case Relative:
		    // ###
		case Variable:
		    break;
		}
	    }
	}
    }
    columnPos.fill(0);
    columnPos[0] = border + spacing;
    for(i = 1; i <= totalCols; i++)
    {
    	columnPos[i] += columnPos[i-1] + actColWidth[i-1] + spacing;
#ifdef TABLE_DEBUG
	printf("Actual width col %d: %d pos = %d\n", i, 
	       actColWidth[i-1], columnPos[i-1]);
#endif
    }

    width = columnPos[totalCols] + border;
    if(width != tableWidth) printf("========> table layout error!!! <===============================\n");

#ifdef TABLE_DEBUG
    printf("total width = %d\n", width);
#endif

    setBlocking(false);
}


void HTMLTableElementImpl::calcRowHeights()
{
    unsigned int r, c;
    int indx;//, borderExtra = border ? 1 : 0;
    HTMLTableCellElementImpl *cell;
    int pad = padding/2;

    rowHeights.resize( totalRows+1 );
    rowHeights[0] = border + spacing + pad;

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
		padding + spacing;// + borderExtra;

	    if ( rowPos > rowHeights[r+1] )
		rowHeights[r+1] = rowPos;
	}

	if ( rowHeights[r+1] < rowHeights[r] )
	    rowHeights[r+1] = rowHeights[r];
    }
}

void HTMLTableElementImpl::layout(bool deep)
{
    ascent = 0;
    descent = 0;
    int pad = padding/2;

#ifdef DEBUG_LAYOUT
    printf("%s(Table)::layout(%d) width=%d, layouted=%d\n", nodeName().string().ascii(), deep, width, layouted());
#endif

    if(blocking()) 
	calcColWidthII();

    if(tCaption) 
    {
	tCaption->setYPos(descent);
	if(deep)
	    tCaption->layout(deep);
	descent += tCaption->getDescent();
    }
    if(frame & Above) descent += border;
#if 0
    if(head) 
    {
	head->setYPos(descent);
	head->layout();
	descent += head->getDescent();
    }
    descent += spacing;
    NodeImpl *child = firstBody;
    while( child != 0 )
    {
	child->setYPos(descent);
	child->layout();
	descent += child->getDescent();
	child = child->nextSibling();
    }
    descent += spacing;
    if(foot) 
    {
	foot->setYPos(descent);
 	foot->layout();
	descent += foot->getDescent();
    }
#else

    if(deep)
    {
	FOR_EACH_CELL( r, c, cell)
	    {
		cell->layout(deep);
	    }
	END_FOR_EACH
    }    

    // We have the cell sizes now, so calculate the vertical positions
    calcRowHeights();

    // set cell positions
    int indx;
    for ( unsigned int r = 0; r < totalRows; r++ )
    {
	int cellHeight;

	if ( tCaption )// && capAlign == HTMLClue::Top )
	    descent += tCaption->getHeight();

	for ( unsigned int c = 0; c < totalCols; c++ )
	{
            HTMLTableCellElementImpl *cell = cells[r][c];
            if (!cell)
                continue; 
	    if ( c < totalCols - 1 && cell == cells[r][c+1] )
		continue;
	    if ( r < totalRows - 1 && cell == cells[r+1][c] )
		continue;

	    if ( ( indx = c-cell->colSpan()+1 ) < 0 )
		indx = 0;

	    int rindx;
	    if ( ( rindx = r-cell->rowSpan()+1 ) < 0 )
		rindx = 0;

	    //printf("setting position %d/%d-%d: %d/%d \n", r, indx, c, 
	    //columnPos[indx] + padding, rowHeights[rindx]);
	    cellHeight = rowHeights[r+1] - rowHeights[rindx] -
		spacing;
	    cell->setPos( columnPos[indx] + pad,
			  rowHeights[rindx] );
	    cell->setRowHeight(cellHeight);
	}
    }
#endif
    descent += rowHeights[totalRows];
    if(frame & Below) descent += border;

    setLayouted();
}

void HTMLTableElementImpl::setAvailableWidth(int w)
{
#ifdef DEBUG_LAYOUT
    printf("%s(Table, this=0x%p)::setAvailableWidth(%d)\n", nodeName().string().ascii(), this, w);
#endif

    if (w != availableWidth)
    	setLayouted(false);

    if(w != -1) availableWidth = w;

    if(blocking() && parsing())
    {
	FOR_EACH_CELL( r, c, cell)
	    {
		cell->setAvailableWidth( 0 );
	    }
	END_FOR_EACH
	return;
    }
    
    calcColWidthII();
	
    int indx;
    FOR_EACH_CELL( r, c, cell)
	{
	    if ( ( indx = c-cell->colSpan()+1) < 0 )
		indx = 0;
	    int w = columnPos[c+1] - columnPos[ indx ] - spacing - 
		padding;
#ifdef TABLE_DEBUG
	    printf("0x%p: setting width %d/%d-%d (0x%p): %d \n", this, r, indx, c, cell, w);
#endif
	    
	    cell->setAvailableWidth( w );
	}
    END_FOR_EACH
}

void HTMLTableElementImpl::print( QPainter *p, int _x, int _y, 
				  int _w, int _h, int _tx, int _ty)
{
    _tx += x;
    _ty += y;

    if((_ty > _y + _h) || (_ty + descent < _y)) return;
    if(!layouted()) return;

    printObject(p, _x, _y, _w, _h, _tx, _ty);

    if ( tCaption )
    {
	tCaption->print( p, _x, _y, _w, _h, _tx, _ty );
    }

    // draw the cells
    FOR_EACH_CELL(r, c, cell)
    {
	//printf("printing cell %d/%d\n", r, c);
        cell->print( p, _x, _y, _w, _h, _tx, _ty);
    }
    END_FOR_EACH


}

void HTMLTableElementImpl::printObject( QPainter *p, int, int, 
					int, int, int _tx, int _ty)
{
#ifdef DEBUG_LAYOUT
    printf("%s(Table)::printObject()\n", nodeName().string().ascii());
#endif

    // ### don't call children here...

    int cindx, rindx;

    // draw the border - needs work to print to printer
    if ( border )
    {
	int capOffset = 0;
	if ( tCaption )//### && capAlign == HTMLClue::Top )
	    capOffset = tCaption->getHeight();
	QColorGroup colorGrp( Qt::black, Qt::lightGray, Qt::white, 
			      Qt::darkGray, Qt::gray, Qt::black, Qt::white );
	qDrawShadePanel( p, _tx, _ty + capOffset, width,
	    rowHeights[totalRows] + border - padding/2, colorGrp, false, border );

        FOR_EACH_CELL(r, c, cell)
        {
	    if ( ( cindx = c-cell->colSpan()+1 ) < 0 )
	        cindx = 0;
	    if ( ( rindx = r-cell->rowSpan()+1 ) < 0 )
	        rindx = 0;

	    qDrawShadePanel(p,
	         _tx + columnPos[cindx],
	         _ty + rowHeights[rindx] + capOffset - padding/2,
	         columnPos[c+1] - columnPos[cindx] - spacing,
	         rowHeights[r+1] - rowHeights[rindx] - spacing,
	         colorGrp, TRUE, 1 );
	}
        END_FOR_EACH
    }
}

#if 0
void HTMLTableElementImpl::print(QPainter *p, int _x, int _y, int _w, int _h, 
				  int _tx, int _ty)
{
    _tx += x;
    _ty += y;

    if((_ty > _y + _h) || (_ty + descent < _y)) return;

    int capOffset = 0;
    if ( tCaption ) //### && capAlign == HTMLClue::Top )
	capOffset = tCaption->getHeight();
    QColorGroup colorGrp( Qt::black, Qt::lightGray, Qt::white, 
			  Qt::darkGray, Qt::gray, Qt::black, Qt::white );

    if ( frame == Box )
    {
	qDrawShadePanel( p, _tx, _ty + capOffset, width,
	    descent - capOffset, colorGrp, false, border );
    }
    else if( frame != Void )
    {
	if( frame & Above )
	{
	    qDrawShadePanel( p, _tx, _ty + capOffset, width,
			     border, colorGrp, false, border/2 );
	}
	if( frame & Below )
	{
	    qDrawShadePanel( p, _tx, _ty + descent - border, width,
			     border, colorGrp, false, border/2 );
	}
	if( frame & Lhs )
	{
	    qDrawShadePanel( p, _tx, _ty + capOffset, border,
			     descent-capOffset, colorGrp, false, border/2 );
	}
	if( frame & Rhs )
	{
	    qDrawShadePanel( p, _tx + width - border, _ty + capOffset, border,
			     descent-capOffset, colorGrp, false, border/2 );
	}
    }
    
    if(tCaption) tCaption->print(p, _x, _y, _w, _h, _tx, _ty);

    // positions needed for rules...
    int yPos = _ty + capOffset;
    int xPos = _tx;
    int w = width;
    if(border & Above) yPos += border;
    if(border &Lhs) xPos += border, w -= border;
    if(border &Rhs) w -= border;
    if(head)
    {
	head->print(p, _x, _y, _w, _h, _tx, _ty);
	yPos += head->getHeight();
	if(rules & RGroups) qDrawShadePanel( p, xPos, yPos, w, spacing, 
					     colorGrp, false, spacing/2 );
    }
    NodeImpl *n = firstBody;
    while(n)
    {
	n->print(p, _x, _y, _w, _h, _tx, _ty);
	yPos += n->getHeight() + spacing;
	n = n->nextSibling();
	if(n && (rules & RGroups)) qDrawShadePanel(p, xPos, yPos, w, spacing, 
						   colorGrp, false, spacing/2);
    }
    if(foot) 
    {
	if(rules & RGroups) qDrawShadePanel( p, xPos, yPos, w, spacing, 
					    colorGrp, false, spacing/2 );
	foot->print(p, _x, _y, _w, _h, _tx, _ty);
    }

    // ### rules for colgroups (rules=CGroups) missing
    // ### rules have to be moved to cells in case we have colspan != 1
    if(rules & Cols)
    {
	int off = spacing+padding/2;
	int i;
	for(i = 1; i< totalCols; i++)
	{
	    int pos = colPos[i] - off;
	    qDrawShadePanel( p, _tx + pos, _ty + capOffset + border, 
			     spacing, descent - capOffset - 2*border, 
			     colorGrp, false, spacing/2 );
	}
    }
}
#endif

void HTMLTableElementImpl::calcMinMaxWidth()
{
#ifdef DEBUG_LAYOUT
    printf("%s(Table)::calcMinMaxWidth() known=%d\n", nodeName().string().ascii(), minMaxKnown());
#endif
    minWidth = border + border + spacing;
    maxWidth = border + border + spacing;

    for(int i = 0; i < totalCols; i++)
    {
	maxWidth += actColWidth[i] + spacing;
	minWidth += colMinWidth[i] + spacing;
    }

    if(!availableWidth || minMaxKnown()) return;

//    if(availableWidth && minWidth > availableWidth)
//	if(_parent) _parent->updateSize();

}

void HTMLTableElementImpl::close()
{
    setParsing(false);
    calcMinMaxWidth();
    if(!availableWidth) return;
    if(availableWidth < minWidth)
	_parent->updateSize();
    setAvailableWidth(); // update child width
    layout(true);
    if(layouted()) 
	static_cast<HTMLDocumentImpl *>(document)->print(this, true);
}
 
void HTMLTableElementImpl::updateSize()
{
    calcMinMaxWidth(); 
    if (incremental)
    	calcColWidth();
    else
    	calcColWidthII();   
    setLayouted(false);
    if(_parent) _parent->updateSize(); 
}

// -------------------------------------------------------------------------

HTMLTableSectionElementImpl::HTMLTableSectionElementImpl(DocumentImpl *doc, 
							 ushort tagid) 
    : HTMLTablePartElementImpl(doc)
{
    _id = tagid;
}

HTMLTableSectionElementImpl::~HTMLTableSectionElementImpl()
{
    nrows = 0;
}

const DOMString HTMLTableSectionElementImpl::nodeName() const
{
    return getTagName(_id);
}

ushort HTMLTableSectionElementImpl::id() const
{
    return _id;
}


// these functions are rather slow, since we need to get the row at
// the index... but they aren't used during usual HTML parsing anyway
HTMLElementImpl *HTMLTableSectionElementImpl::insertRow( long index )
{
    nrows++;

    HTMLTableRowElementImpl *r = new HTMLTableRowElementImpl(document);
    r->setTable(this->table);
    if(index < 1)
    {
	insertBefore(r, firstChild());
	return r;
    }

    NodeListImpl *children = childNodes();
    if(!children || (int)children->length() <= index)
	appendChild(r);
    else
	insertBefore(r, children->item(index));
    if(children) delete children;
    return r;
}

void HTMLTableSectionElementImpl::deleteRow( long index )
{
    if(index < 0) return;
    NodeListImpl *children = childNodes();
    if(children && (int)children->length() > index)
    {
	nrows--;
	removeChild(children->item(index));
    }
    if(children) delete children;
}

#if 0
void HTMLTableSectionElementImpl::print(QPainter *p, int _x, int _y, 
					int _w, int _h, 
					int _tx, int _ty)
{
    _tx += x;
    _ty += y;

    if((_ty > _y + _h) || (_ty + descent < _y)) return;

    QColorGroup colorGrp( Qt::black, Qt::lightGray, Qt::white, 
			  Qt::darkGray, Qt::gray, Qt::black, Qt::white );
    int spacing = table->cellSpacing();
    HTMLTableElementImpl::Rules rules = table->getRules();

    int yPos = _ty - spacing;
    NodeImpl *n = firstChild();
    while(n)
    {
	n->print(p, _x, _y, _w, _h, _tx, _ty);
	yPos += n->getHeight() + spacing;
	n = n->nextSibling();
	if(n && (rules & HTMLTableElementImpl::Rows)) 
	    qDrawShadePanel(p, _tx, yPos, width, spacing, 
			    colorGrp, false, spacing/2);
    }
}
#endif

void HTMLTableSectionElementImpl::layout(bool)
{
#if 0
    int spacing = table->cellSpacing();
    ascent = 0;
    descent = 0;

    NodeImpl *child = firstChild();
    while( child != 0 )
    {
	child->setYPos(descent);
	child->layout();
	descent += child->getDescent() + spacing;
	child = child->nextSibling();
    }
    descent -= spacing;
#endif
#ifdef DEBUG_LAYOUT
     printf("%s(TableSection)::layout(???) width=%d, layouted=%d\n", nodeName().string().ascii(), width, layouted());
#endif

    setLayouted();
}

NodeImpl *HTMLTableSectionElementImpl::addChild(NodeImpl *child)
{
#ifdef DEBUG_LAYOUT
    printf("%s(TableSection)::addChild( %s )\n", nodeName().string().ascii(), child->nodeName().string().ascii());
#endif

    table->startRow();
    return HTMLTablePartElementImpl::addChild(child);
}

// -------------------------------------------------------------------------

HTMLTableRowElementImpl::HTMLTableRowElementImpl(DocumentImpl *doc)
  : HTMLTablePartElementImpl(doc)
{
  rIndex = -1;
}

HTMLTableRowElementImpl::~HTMLTableRowElementImpl()
{
}

const DOMString HTMLTableRowElementImpl::nodeName() const
{
    return "TR";
}

ushort HTMLTableRowElementImpl::id() const
{
    return ID_TR;
}

long HTMLTableRowElementImpl::rowIndex() const
{
    // ###
    return 0;
}

void HTMLTableRowElementImpl::setRowIndex( long  )
{
    // ###
}

void HTMLTableRowElementImpl::setCells( const HTMLCollection & )
{
}

HTMLElementImpl *HTMLTableRowElementImpl::insertCell( long index )
{
    HTMLTableCellElementImpl *c = new HTMLTableCellElementImpl(document, ID_TD);
    c->setTable(this->table);

    if(index < 1)
    {
	insertBefore(c, firstChild());
	return c;
    }

    NodeListImpl *children = childNodes();
    if(!children || (int)children->length() <= index)
	appendChild(c);
    else
	insertBefore(c, children->item(index));
    if(children) delete children;
    return c;
}

void HTMLTableRowElementImpl::deleteCell( long index )
{
    if(index < 0) return;
    NodeListImpl *children = childNodes();
    if(children && (int)children->length() > index)
	removeChild(children->item(index));
    if(children) delete children;
}

NodeImpl *HTMLTableRowElementImpl::addChild(NodeImpl *child)
{
#ifdef DEBUG_LAYOUT
    printf("%s(TableRow)::addChild( %s )\n", nodeName().string().ascii(), child->nodeName().string().ascii());
#endif
 
    NodeImpl *ret = HTMLBlockElementImpl::addChild(child);

    HTMLTableCellElementImpl *cell =
	static_cast<HTMLTableCellElementImpl *>(child);
    cell->setTable(this->table);
    table->addCell(cell);

    return ret;
}

void HTMLTableRowElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_BGCOLOR:
	setNamedColor( bg, attr->value().string() );
	break;
    default:
	HTMLBlockElementImpl::parseAttribute(attr);
    }
}

// -------------------------------------------------------------------------

HTMLTableCellElementImpl::HTMLTableCellElementImpl(DocumentImpl *doc, int tag) 
  : HTMLTablePartElementImpl(doc)
{
  _col = -1;
  _row = -1;
  cSpan = rSpan = 1;
  nWrap = false;
  _id = tag;
  rowHeight = 0;
}

HTMLTableCellElementImpl::~HTMLTableCellElementImpl()
{
}

const DOMString HTMLTableCellElementImpl::nodeName() const
{
    return getTagName(_id);
}

void HTMLTableCellElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_ROWSPAN:
	rSpan = attr->val()->toInt();
	break;
    case ATTR_COLSPAN:
	cSpan = attr->val()->toInt();
	break;
    case ATTR_NOWRAP:
	nWrap = true;
	break;
    case ATTR_WIDTH:
	predefinedWidth = attr->val()->toLength();
	break;
    case ATTR_HEIGHT:
	predefinedHeight = attr->val()->toLength();
	break;
    case ATTR_BGCOLOR:
	setNamedColor( bg, attr->value().string() );
	break;
    case ATTR_ALIGN:
	if ( strcasecmp( attr->value(), "left" ) == 0 )
	    halign = Left;
	else if ( strcasecmp( attr->value(), "right" ) == 0 )
	    halign = Right;
	else if ( strcasecmp( attr->value(), "center" ) == 0 )
	    halign = HCenter;
	break;
    default:
	HTMLBlockElementImpl::parseAttribute(attr);
    }
}

void HTMLTableCellElementImpl::calcMinMaxWidth()
{
#ifdef DEBUG_LAYOUT
    printf("%s(TableCell)::calcMinMaxWidth() known=%d\n", nodeName().string().ascii(), minMaxKnown());
#endif

    if(minMaxKnown()) return;

    HTMLBlockElementImpl::calcMinMaxWidth();
    if(nWrap) minWidth = maxWidth;
    table->addColInfo(this);
    printf("cell: calcminmaxwidth %d, %d\n",width,minWidth);    
//    if(availableWidth && minWidth > availableWidth)
//	if(_parent) _parent->updateSize();

}

void HTMLTableCellElementImpl::print(QPainter *p, int _x, int _y, 
					int _w, int _h, 
					int _tx, int _ty)
{
    int padding = table->cellPadding();

    int _ascent = parentNode()->getAscent();

    _ty += y + _ascent;
    int pad = padding/2;

    // check if we need to do anything at all...
    if((_ty - pad > _y + _h) || (_ty + rowHeight - pad < _y)) return;
    // add relative position of the element
    _tx += x;

    printObject(p, _x, _y, _w, _h, _tx, _ty);

    NodeImpl *child = firstChild();
    while(child != 0) 
    {
	child->print(p, _x, _y, _w, _h, _tx, _ty);
	child = child->nextSibling();
    }

}

void HTMLTableCellElementImpl::printObject(QPainter *p, int, int,
					   int, int, int _tx, int _ty)
{
#ifdef DEBUG_LAYOUT
    printf("%s(TableCell)::printObject()\n", nodeName().string().ascii());
    printf("    width = %d\n", width);
#endif

    int padding = table->cellPadding();
    int pad = padding/2;

    QColor back = bg;
    if(!back.isValid())
	back = static_cast<HTMLTableRowElementImpl *>(parentNode())->bgColor();
    if(!back.isValid())
	back = table->bgColor();

    if ( back.isValid() )
    {
	QBrush brush( back );
	p->fillRect( _tx - pad , _ty - pad, 
		     width + padding, rowHeight, brush );
    }
}

// -------------------------------------------------------------------------

HTMLTableColElementImpl::HTMLTableColElementImpl(DocumentImpl *doc, ushort i)
    : HTMLElementImpl(doc)
{
    _id = i;
    _span = 1;
}

HTMLTableColElementImpl::~HTMLTableColElementImpl()
{
}

const DOMString HTMLTableColElementImpl::nodeName() const
{
    return getTagName(_id);
}

ushort HTMLTableColElementImpl::id() const
{
    return ID_COL;
}

void HTMLTableColElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_SPAN:
	_span = attr->val()->toInt();
	break;
    case ATTR_WIDTH:
	predefinedWidth = attr->val()->toLength();
	break;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

// -------------------------------------------------------------------------

HTMLTableCaptionElementImpl::HTMLTableCaptionElementImpl(DocumentImpl *doc) 
  : HTMLTablePartElementImpl(doc)
{
}

HTMLTableCaptionElementImpl::~HTMLTableCaptionElementImpl()
{
}

const DOMString HTMLTableCaptionElementImpl::nodeName() const
{
    return "CAPTION";
}

ushort HTMLTableCaptionElementImpl::id() const
{
    return ID_CAPTION;
}


