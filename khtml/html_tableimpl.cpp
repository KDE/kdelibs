/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1997 Martin Jones (mjones@kde.org)
 *           (C) 1997 Torben Weis (weis@kde.org)
 *           (C) 1998 Waldo Bastian (bastian@kde.org)
 *           (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
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
//#define TABLE_DEBUG
//#define DEBUG_LAYOUT
//#define DEBUG_DRAW_BORDER

#include <qlist.h>
#include <qstack.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpalette.h>
#include <qdrawutil.h>

#include "dom_string.h"
#include "dom_nodeimpl.h"
#include "dom_exception.h"
#include "dom_textimpl.h"

#include "html_misc.h"
#include "html_element.h"
#include "html_table.h"
#include "html_tableimpl.h"
#include "html_documentimpl.h"
using namespace DOM;

#include "khtmlattrs.h"
#include "khtmlstyle.h"
#include "khtmltext.h"

#include <stdio.h>
#include <assert.h>

template class QArray<khtml::LengthType>;
template class QVector<DOM::HTMLTableElementImpl::ColInfoLine>;
template class QVector<DOM::HTMLTableElementImpl::ColInfo>;

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


inline int MAX(int a, int b)
{
    return a > b ? a : b;
}

inline int MIN(int a, int b)
{
    return a < b ? a : b;
}

HTMLTableElementImpl::HTMLTableElementImpl(DocumentImpl *doc)
  : HTMLBlockElementImpl(doc)
{
    tCaption = 0;
    _oldColElem = 0;
    head = 0;
    foot = 0;
    firstBody = 0;

    valign=VNone;
    halign=HNone;

    width = -1;
    availableWidth = 0;
    incremental = false;
    maxWidth = 0;

    spacing = 2;
    padding = 2;
    rules = None;
    frame = Void;
    border = 0;

    row = 0;
    col = 0;

    background = 0;

    maxColSpan = 0;
    totalColInfos = 0;

    colInfos.setAutoDelete(true);

    setBlocking();
    setParsing();

    _currentCol=0;

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

    totalCols = 0;   // this should be expanded to the maximum number of cols
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
    	{
	// these have to come before the table definition!
	if(head || foot || firstBody)
	    throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);
	HTMLBlockElementImpl::addChild(child);
	HTMLTableColElementImpl* colel = static_cast<HTMLTableColElementImpl *>(child);
	colel->setTable(this);
	if (_oldColElem)
	    _currentCol = _oldColElem->lastCol();
	_oldColElem = colel;
	colel->setStartCol(_currentCol);
	if (child->id() == ID_COL)
	    _currentCol++;
	else
	    _currentCol+=colel->span();
	addColInfo(colel);	
	incremental = true;
	}
	return child;
    case ID_THEAD:
	if(incremental && !columnPos[totalCols]);// calcColWidth();
	setTHead(static_cast<HTMLTableSectionElementImpl *>(child));
	break;
    case ID_TFOOT:
	if(incremental && !columnPos[totalCols]);// calcColWidth();
	setTFoot(static_cast<HTMLTableSectionElementImpl *>(child));
	break;
    case ID_TBODY:
	if(incremental && !columnPos[totalCols]);// calcColWidth();
	if(!firstBody)
	    firstBody = static_cast<HTMLTableSectionElementImpl *>(child);
    default:	
 	NodeBaseImpl::addChild(child);
	break;
    }
    child->setAvailableWidth(0);
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
#ifdef DEBUG_DRAW_BORDER
    	border=1;
#endif
	// wanted by HTML4 specs
	if(!border)
	    frame = Void, rules = None;
	else
	    frame = Box, rules = All;
	break;
    case ATTR_BGCOLOR:
	khtml::setNamedColor( bg, attr->value().string() );
	break;
    case ATTR_BACKGROUND:
        bgURL = attr->value();
	break;
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
	// right or left means floating table, not aligned cells
	if ( strcasecmp( attr->value(), "left" ) == 0 )
	    halign = Left;
	else if ( strcasecmp( attr->value(), "right" ) == 0 )
	    halign = Right;
	else if ( strcasecmp( attr->value(), "center" ) == 0 )
	    halign = HCenter;	
	break;
    }
    case ATTR_VALIGN:
	if ( strcasecmp( attr->value(), "top" ) == 0 )
	    valign = Top;
	else if ( strcasecmp( attr->value(), "middle" ) == 0 )
	    valign = VCenter;
	else if ( strcasecmp( attr->value(), "bottom" ) == 0 )
	    valign = Bottom;
	else if ( strcasecmp( attr->value(), "baseline" ) == 0 )
	    valign = Baseline;
	break;
    case ATTR_VSPACE:
	vspace = attr->val()->toLength();
	break;
    case ATTR_HSPACE:
	hspace = attr->val()->toLength();
	break;
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

    calcMinMaxWidth();    
    setAvailableWidth();
}

void HTMLTableElementImpl::addCell( HTMLTableCellElementImpl *cell )
{
    while ( col < totalCols && cells[row][col] != 0L )
	col++;
    setCells( row, col, cell );
    
    setMinMaxKnown(false);
    setLayouted(false);

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
#ifdef TABLE_DEBUG
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

    int mSpan = newCols;

    colInfos.resize(mSpan);

    percentTotals.resize(mSpan);
    memset( percentTotals.data() + totalCols , 0, num*sizeof(int));


    for ( unsigned int c =0 ; c < totalCols; c++ )
    {
    	colInfos[c]->resize(newCols);
    }
    for ( unsigned int c = totalCols; (int)c < newCols; c++ )
    {
    	colInfos.insert(c, new ColInfoLine(newCols-c+1));
    }

    totalCols = newCols;

}


void HTMLTableElementImpl::addColInfo(HTMLTableColElementImpl *colel)
{
	
    int _startCol = colel->col();
    int span = colel->span();
    int _minSize=0;
    int _maxSize=0;
    Length _width = colel->width();

    for (int n=0; n<span; ++n)
    {
#ifdef TABLE_DEBUG
	printf("COL\n");
	printf("    startCol=%d span=%d\n", _startCol, span);
	printf("    min=%d max=%d val=%d\n", _minSize, _maxSize,_width.value );    	
#endif
        addColInfo(_startCol+n, 1 , _minSize, _maxSize, _width ,0);
    }

}

void HTMLTableElementImpl::addColInfo(HTMLTableCellElementImpl *cell)
{
    int _startCol = cell->col();
    int _colSpan = cell->colSpan();
    int _minSize = cell->getMinWidth();
    int _maxSize = cell->getMaxWidth();
    Length _width = cell->predefWidth();
    addColInfo(_startCol, _colSpan, _minSize, _maxSize, _width ,cell);
}

void HTMLTableElementImpl::addColInfo(int _startCol, int _colSpan,
				      int _minSize, int _maxSize,
				      Length _width, HTMLTableCellElementImpl* _cell)
{
#ifdef TABLE_DEBUG
    printf("addColInfo():\n");
    printf("    startCol=%d span=%d\n", _startCol, _colSpan);
    printf("    min=%d max=%d\n", _minSize, _maxSize );
    printf("    totalCols=%d\n", totalCols );
#endif

    if (_startCol + _colSpan > (int) totalCols)
	addColumns(totalCols - _startCol + _colSpan);

    ColInfo* col = colInfos[_colSpan-1]->at(_startCol);
    if (!col)
    {
    	col = new ColInfo;
	col->span = _colSpan;
    	col->start = _startCol;
	col->minCell = _cell;
	col->maxCell = _cell;
	if (_colSpan>maxColSpan)
	    maxColSpan=_colSpan;
	colInfos[_colSpan-1]->insert(_startCol,col);
    }

    if (_minSize > col->min)
    {
    	col->min = _minSize;
	col->minCell = _cell;
    }
    if (_maxSize > col->max)
    {
    	col->max = _maxSize;
	col->maxCell = _cell;
    }
    if (_width.type > col->type)
    {
        col->type = _width.type;
    	col->value = _width.value;
	if (_width.type==Percent)
    	    percentTotals[_colSpan-1]+=_width.value;
    }
    if (_width.type == col->type)
    {
    	if (_width.value > col->value)
	{
	    if (_width.type==Percent)
	    	percentTotals[_colSpan-1]+=_width.value-col->value;
	    col->value = _width.value;
	}
    }
	
    setMinMaxKnown(false);
    
#ifdef TABLE_DEBUG
    printf("  end: min=%d max=%d act=%d\n", colMinWidth[_startCol],
	   colMaxWidth[_startCol], actColWidth[_startCol]);
#endif
}

void HTMLTableElementImpl::spreadSpanMinMax(int col, int span, int distmin,
    int distmax, LengthType type)
{
#ifdef TABLE_DEBUG
    printf("HTMLTableElementImpl::spreadSpanMinMax() %d\n",distmin);
#endif

    if (distmin<1 && distmax<1)
    	return;
    if (distmin<1)
    	distmin=0;
    if (distmax<1)
    	distmax=0;	

    bool hasUsableCols=false;
    int tmax=distmax;
    int tmin=distmin;
    int c;

    for (c=col; c < col+span ; ++c)
    {
    	if (colType[c]<=type)
	    hasUsableCols=true;
    }

    if (hasUsableCols)
    {
    	// spread span maxWidth evenly
	c=col;
	while(tmax)
	{
	    if (colType[c]<=type)
	    {
		colMaxWidth[c]+=distmax/span;
		tmax-=distmax/span;
		if (tmax<span)
		{
		    colMaxWidth[c]+=tmax;
		    colType[c]=type;
		    tmax=0;
		}
	    }
	    if (++c==col+span)
		c=col;
	}
	
    	// spread span minWidth 

    	LengthType tt = Variable;
	bool out=false;
	while (tt<=type && !out && tmin)
	{
	    tmin = distributeMinWidth(tmin,type,tt,col,span,true);
    	    switch (tt)
	    {
	    case Undefined:
    	    case Variable: tt=Relative; break;	
	    case Relative: tt=Percent; break;
	    case Percent: tt=Fixed; break;
	    case Fixed: out=true; break;
	    }
	}
	
	// force spread rest of the minWidth
	
    	tt = Variable;
    	out=false;
	while (tt<=type && !out && tmin)
	{
	    tmin = distributeMinWidth(tmin,type,tt,col,span,false);
    	    switch (tt)
	    {
	    case Undefined:
    	    case Variable: tt=Relative; break;	
	    case Relative: tt=Percent; break;
	    case Percent: tt=Fixed; break;
	    case Fixed: out=true; break;
	    }
	}    

	for (int c=col; c < col+span ; ++c)
	    colMaxWidth[c]=MAX(colMinWidth[c],colMaxWidth[c]);	    	
		
    }
}



void HTMLTableElementImpl::calcSingleColMinMax(int c, ColInfo* col)
{
#ifdef TABLE_DEBUG
    printf("HTMLTableElementImpl::calcSingleColMinMax()\n");
#endif

    int span=col->span;

    int oldmin=0;
    int oldmax=0;
    for (int o=c; o<c+span; ++o)
    {
	oldmin+=colMinWidth[o];
	oldmax+=colMaxWidth[o];
    }
    int smin = col->min;
    int	smax = col->max;

    if (col->type==Fixed)
    	smax = MAX(smin,col->value);
    	
    if (span==1)
    {
      //printf("col (s=1) c=%d,m=%d,x=%d\n",c,smin,smax);
        colMinWidth[c] = smin;
        colMaxWidth[c] = smax;
        colValue[c] = col->value;
        colType[c] = col->type;
    }
    else
    {	
	int spreadmin = smin-oldmin-(span-1)*spacing;
//	int spreadmax = smax-oldmax-(span-1)*spacing;
	//printf("spreading span %d,%d\n",spreadmin, spreadmax);
	spreadSpanMinMax
	    (c, span, spreadmin, 0 , col->type);
    }

}

void HTMLTableElementImpl::calcFinalColMax(int c, ColInfo* col)
{
#ifdef TABLE_DEBUG
    printf("HTMLTableElementImpl::calcPercentRelativeMax()\n");
#endif
    int span=col->span;

    int oldmax=0;
    int oldmin=0;
    for (int o=c; o<c+span; ++o)
    {
	oldmax+=colMaxWidth[o];
	oldmin+=colMinWidth[o];
    }

    int smin = col->min;
    int	smax = col->max;
    
    if (col->type==Fixed)
    {
    	smax = MAX(smin,col->value);    
    }
    else if (col->type == Percent)
    {
    	smax = width * col->value / MAX(100,totalPercent);
    }
    else if (col->type == Relative && totalRelative)
    {
    	smax= width * col->value / totalRelative;
    }

    smax = MAX(smax,oldmin);
    smax = MIN(smax,width);

    if (span==1)
    {
//       printf("col (s=1) c=%d,m=%d,x=%d\n",c,smin,smax);
       colMaxWidth[c] = smax;
       colType[c] = col->type;
    }
    else
    {	
	int spreadmax = smax-oldmax-(span-1)*spacing;
//	printf("spreading span %d,%d\n",spreadmin, spreadmax);
	spreadSpanMinMax
	    (c, span, 0, spreadmax, col->type);
    }

}



void HTMLTableElementImpl::calcColMinMax()
{
// Calculate minmimum and maximum widths for all
// columns.
// Calculate min and max width for the table.


#ifdef TABLE_DEBUG
    printf("HTMLTableElementImpl::calcColMinMax(), %d\n", minMaxKnown());
#endif

    // PHASE 1, prepare
    
//    if(minMaxKnown())
//    	return;

    colMinWidth.fill(0);
    colMaxWidth.fill(0);		


    // PHASE 2, calculate simple minimums and maximums

    for ( unsigned int s=0;  (int)s<maxColSpan ; ++s)
    {	
    	ColInfoLine* spanCols = colInfos[s];

    	for ( unsigned int c=0; c<totalCols-s; ++c)
    	{

    	    ColInfo* col;
    	    col = spanCols->at(c);		

	    if (!col || col->span==0)
		continue;
#ifdef TABLE_DEBUG
    	    printf(" s=%d c=%d min=%d value=%d\n",s,c,col->min,col->value);
#endif	    

    	    col->update();
	    calcSingleColMinMax(c, col);

	}	
    }


    // PHASE 3, calculate table width
    // NOT as simple as it sounds

    totalPercent=0;
    totalRelative=0;

    int maxFixed=0;
    int minPercent=0;
    int percentWidest=0;
    int percentWidestPercent=0;

    int minRel=0;
    int minVar=0;
    bool hasFixed=false;
    bool hasPercent=false;
    bool hasRel=false;
    bool hasVar=false;

    minWidth=border + border + spacing;
    maxWidth=border + border + spacing;

    for(int i = 0; i < (int)totalCols; i++)
    {
    	minWidth += colMinWidth[i] + spacing;
	maxWidth += colMaxWidth[i] + spacing;

    	switch(colType[i])
	{
	case Fixed:
	    maxFixed += colMaxWidth[i] + spacing;
	    hasFixed=true;
	    break;
	case Percent:
	    hasPercent=true;
	    totalPercent += colValue[i];
	    minPercent += colMinWidth[i] + spacing;	    
	    if (colValue[i]>0)
	    {
    		int pw =  colMaxWidth[i]*100/colValue[i];
		if (  pw > percentWidest)
		{
	    	    percentWidest = pw;
		    percentWidestPercent = colValue[i];
		}
	    }
	    break;
	case Relative:
	    hasRel=true;
	    totalRelative += colValue[i] ;
	    minRel += colMinWidth[i] + spacing;
	    break;
	case Undefined:
	case Variable:
	    hasVar=true;
	    minVar += colMinWidth[i] + spacing;
	}

    }
        
    
    if(predefinedWidth.type > Relative) // Percent or fixed table
    {
	width = predefinedWidth.minWidth(availableWidth);
	if(minWidth > width) width = minWidth;
/*	printf("1 width=%d minWidth=%d availableWidth=%d \n",
	    width,minWidth,availableWidth);
	if (width>1000) for(int i = 0; i < totalCols; i++)
    	{		
	    printf("DUMP col=%d type=%d max=%d min=%d value=%d\n",
	    	i,colType[i],colMaxWidth[i],colMinWidth[i],colValue[i]);
	}*/
	
    }
    else if (hasPercent && !hasFixed)
    {    	
//	printf("2 percentWidest=%d percentWidestPercent=%d \n",
//	    percentWidest, percentWidestPercent);
	width = percentWidest;
	width += (totalCols+1)*spacing+1;
	int tot = MIN(99,totalPercent);
	width = MAX(width, (minVar + minRel)*100/(100-tot) );
	width = MIN(width, availableWidth);
	totalPercent = MAX(totalPercent,100);
    }
    else if (hasPercent && hasFixed)
    {    	
    	totalPercent = MIN(99,totalPercent);
//	printf("3 maxFixed=%d  totalPercent=%d\n",maxFixed,totalPercent);
	width = (maxFixed + minVar + minRel) * 100 /
    	    (100 - totalPercent);
    	width = MIN (width, availableWidth);
    }
    else
    {
    	width = MIN(availableWidth,maxWidth);	
    }

    width = MAX (width, minWidth);
    	    
//    printf("TABLE width %d\n", width);



    // PHASE 4, calculate maximums for percent and relative columns

    for ( unsigned int s=0;  (int)s<maxColSpan ; ++s)
    {	
    	ColInfoLine* spanCols = colInfos[s];

    	for ( unsigned int c=0; c<totalCols-s; ++c)
    	{

    	    ColInfo* col;
    	    col = spanCols->at(c);

	    if (!col || col->span==0)
		continue;
	    if ((col->type==Fixed || col->type==Variable)
	    	&& col->span<2)
	    continue;

	    calcFinalColMax(c, col);

	}
	
    }


    // PHASE 5, set table min and max to final values

    if(predefinedWidth.type == Fixed)
    	minWidth = maxWidth = width;
    else
    {
        maxWidth=border + border + spacing;

    	for(int i = 0; i < (int)totalCols; i++)
	    maxWidth += colMaxWidth[i] + spacing;
    }
    
    setMinMaxKnown(true);

}

void HTMLTableElementImpl::calcColWidth(void)
{

#ifdef TABLE_DEBUG
    printf("START calcColWidth() this = %p\n", this);
    printf("---- %d ----\n", totalColInfos);
    printf("maxColSpan = %d\n", maxColSpan);
    printf("availableWidth = %d\n", availableWidth);
#endif

    if (totalCols==0)
    	return;

    /*
     * Calculate min and max width for every column,
     * and the width of the table
     */

    calcColMinMax();

    /*
     * Set actColWidth[] to column minimums, it will
     * grow from there.
     * Collect same statistics for future use.
     */

    int actWidth = border + border + spacing;

    int minFixed = 0;
    int minPercent = 0;
    int minRel = 0;
    int minVar = 0;

    int maxFixed = 0;
    int maxPercent = 0;
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
    	actColWidth[i] = colMinWidth[i];
	actWidth += actColWidth[i] + spacing;	
	
	switch(colType[i])
	{
	case Fixed:
	    minFixed += colMinWidth[i];
	    maxFixed += colMaxWidth[i];
	    numFixed++;
	    break;
	case Percent:
	    minPercent += colMinWidth[i];
	    maxPercent += colMaxWidth[i];
	    numPercent++;
	    break;
	case Relative:
	    minRel += colMinWidth[i];
	    maxRel += colMaxWidth[i];
	    numRel++;
	    break;
	case Undefined:
	case Variable:
	    minVar += colMinWidth[i];
	    maxVar += colMaxWidth[i];
	    numVar++;
	}
    	
    }

#ifdef TABLE_DEBUG
    for(int i = 1; i <= (int)totalCols; i++)
    {
	printf("Start->target %d: %d->%d\n", i, actColWidth[i-1],colMaxWidth[i-1]);
    }
#endif

    int tooAdd = width - actWidth;      // what we can add

#ifdef TABLE_DEBUG
    printf("tooAdd = width - actwidth: %d = %d - %d\n",
    	tooAdd, width, actWidth);
#endif

    /*
     * tooAddute the free width among the columns so that
     * they reach their max width.
     * Order: fixed->percent->relative->variable
     */    

    tooAdd = distributeWidth(tooAdd,Fixed,numFixed);
    tooAdd = distributeWidth(tooAdd,Percent,numPercent);
    tooAdd = distributeWidth(tooAdd,Relative,numRel);
    tooAdd = distributeWidth(tooAdd,Variable,numVar);

    /*
     * Some width still left? 
     * Reverse order, variable->relative->percent
     */

    tooAdd = distributeRest(tooAdd,Variable,maxVar);
    tooAdd = distributeRest(tooAdd,Relative,maxRel);
    tooAdd = distributeRest(tooAdd,Percent,maxPercent);

#ifdef TABLE_DEBUG
    printf("final tooAdd %d\n",tooAdd);
#endif

    /*
     * Calculate the placement of colums
     */

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

#ifdef TABLE_DEBUG
    if(width != columnPos[totalCols] + border) printf("========> table layout error!!! <===============================\n");
    printf("total width = %d\n", width);
#endif

    setBlocking(false);
}


int HTMLTableElementImpl::distributeWidth(int distrib, LengthType type, int typeCols )
{
    int olddis=0;
    int c=0;

    int tdis = distrib;
//    printf("DISTRIBUTING %d pixels to type %d cols\n", distrib, type);

    while(tdis>0)
    {
	if (colType[c]==type)
	{
	    int delta = MIN(distrib/typeCols,colMaxWidth[c]-actColWidth[c]);
	    delta = MIN(tdis,delta);
	    if (delta==0 && tdis && colMaxWidth[c]>actColWidth[c])
	    	delta=1;
	    actColWidth[c]+=delta;		
	    tdis-=delta;
	}
	if (++c == (int)totalCols)
	{
	    c=0;
	    if (olddis==tdis)
		break;
	    olddis=tdis;
	}
    }
    return tdis;
}


int HTMLTableElementImpl::distributeRest(int distrib, LengthType type, int divider )
{
    if (!divider)
    	return distrib;

//    printf("DISTRIBUTING rest, %d pixels to type %d cols\n", distrib, type);

    int olddis=0;
    int c=0;

    int tdis = distrib;

    while(tdis>0)
    {
	if (colType[c]==type)
	{
	    int delta = (colMaxWidth[c] * distrib) / divider;
	    delta=MIN(delta,tdis);
	    if (delta==0 && tdis)
	    	delta=1;
	    actColWidth[c] += delta;
	    tdis -= delta;
	}
	if (++c == (int)totalCols)
	{
	    c=0;
	    if (olddis==tdis)
		break;
	    olddis=tdis;
	}
    }
    return tdis;
}

int HTMLTableElementImpl::distributeMinWidth(int distrib, LengthType distType,
    	    LengthType toType, int start, int span, bool mlim )
{
//    printf("MINDIST, %d pixels of type %d"
//    	"to type %d cols sp=%d \n", distrib, distType, toType,span);

    int olddis=0;
    int c=start;

    int tdis = distrib;

    while(tdis>0)
    {
//    	printf ("ct=%d\n",colType[c]);
	if (colType[c]==toType)
	{
	    int delta = distrib/span;
	    if (mlim)
	    	delta = MIN(delta,colMaxWidth[c]-colMinWidth[c]);
	    delta = MIN(tdis,delta);
	    if (delta==0 && tdis && (!mlim || colMaxWidth[c]>colMinWidth[c]))
	    	delta=1;
	    colMinWidth[c]+=delta;
	    colType[c]=distType;
	    tdis-=delta;
	}
	if (++c==start+span)
	{
	    c=start;
	    if (olddis==tdis)
		break;
	    olddis=tdis;
	}
    }
  
    return tdis;
}



void HTMLTableElementImpl::calcRowHeights()
{
    unsigned int r, c;
    int indx;//, borderExtra = border ? 1 : 0;
    HTMLTableCellElementImpl *cell;

    rowHeights.resize( totalRows+1 );
    rowBaselines.resize( totalRows );
    rowHeights[0] = border + spacing + padding;

    for ( r = 0; r < totalRows; r++ )
    {
      //int oldheight = rowHeights[r+1] - rowHeights[r];
	rowHeights[r+1] = 0;
	
	int baseline=0;
	int bdesc=0;

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
		
	    // find out the baseline
	    if (cell->vAlign()==Baseline)
	    {
		NodeImpl* firstElem = cell->firstChild();	    	
		if (firstElem)
		{
		    int b=firstElem->getAscent();

		    if (b>baseline)
			baseline=b;
		    	    	
		    int td = rowHeights[ indx ] + cell->getHeight() - b;
		    if (td>bdesc)
			bdesc = td;
		}
	    }	    		
	}
	
	//do we have baseline aligned elements?
	if (baseline)
	{
	    // increase rowheight if baseline requires
	    int bRowPos = baseline + bdesc + 2*padding + spacing ;
    	    if (rowHeights[r+1]<bRowPos)
	    	rowHeights[r+1]=bRowPos;
		
	    rowBaselines[r]=baseline;
	}

	if ( rowHeights[r+1] < rowHeights[r] )
	    rowHeights[r+1] = rowHeights[r];
    }
}

//#include "qdatetime.h"

void HTMLTableElementImpl::layout(bool deep)
{
    if (layouted())
   	return;

    ascent = 0;
    descent = 0;

#ifdef DEBUG_LAYOUT
    printf("%s(Table)::layout(%d) width=%d, layouted=%d\n", nodeName().string().ascii(), deep, width, layouted());
#endif

//    QTime qt;
//    qt.start();

    if(blocking())
	calcColWidth();

//    printf("TIME calcColWidth() dt=%d\n",qt.elapsed());

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
    
//    printf("TIME cell layout() dt=%d\n",qt.elapsed());

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

	    VAlign va = cell->vAlign();
	    switch (va)
	    {
	    case Baseline: 
    		if (cell->firstChild()) 
    		    cell->setAscent( getBaseline(r) - cell->firstChild()->getYPos() );
		break;
	    case VNone:
	    case VCenter:
    		cell->setAscent((cellHeight-2*padding-cell->getDescent())/2);
		break;
	    case Bottom:
    		cell->setAscent((cellHeight-2*padding-cell->getDescent()));
		break;
	    case Top:
        	cell->setAscent(0);
    		break;
	    }
	    
	    cell->setPos( columnPos[indx] + padding,
			  rowHeights[rindx] + cell->getAscent());
	    cell->setRowHeight(cellHeight);
	}
    }
#endif
    descent += rowHeights[totalRows];
    if(frame & Below) descent += border;

    setLayouted();
    
//    printf("TIME layout() end dt=%d\n",qt.elapsed());    

}

void HTMLTableElementImpl::setAvailableWidth(int w)
{
#ifdef DEBUG_LAYOUT
    printf("%s(Table, this=0x%p)::setAvailableWidth(%d)\n", nodeName().string().ascii(), this, w);
#endif

    if (w == availableWidth)
    	return;

    setLayouted(false);	

    if(w != -1) availableWidth = w;

    calcColWidth();
	
    int indx;
    FOR_EACH_CELL( r, c, cell)
	{
	    if ( ( indx = c-cell->colSpan()+1) < 0 )
		indx = 0;
	    int w = columnPos[c+1] - columnPos[ indx ] - spacing - padding*2;
	
#ifdef TABLE_DEBUG
	    printf("0x%p: setting width %d/%d-%d (0x%p): %d \n", this, r, indx, c, cell, w);
#endif
	
	    cell->setAvailableWidth( w );
	}
    END_FOR_EACH
    
    setMinMaxKnown(false);
}

void HTMLTableElementImpl::print( QPainter *p, int _x, int _y,
				  int _w, int _h, int _tx, int _ty)
{
    _tx += x;
    _ty += y;

    if((_ty - ascent > _y + _h) || (_ty + ascent + descent < _y)) return;
    if((_tx > _x + _w) || (_tx + width < _x)) return;

    if(!layouted()) return;

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

    printObject(p, _x, _y, _w, _h, _tx, _ty);


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
	    rowHeights[totalRows] + border - padding, colorGrp, false, border );

        FOR_EACH_CELL(r, c, cell)
        {
	    if ( ( cindx = c-cell->colSpan()+1 ) < 0 )
	        cindx = 0;
	    if ( ( rindx = r-cell->rowSpan()+1 ) < 0 )
	        rindx = 0;

	    qDrawShadePanel(p,
	         _tx + columnPos[cindx],
	         _ty + rowHeights[rindx] + capOffset - padding,
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
	int off = spacing+padding;
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

    calcColMinMax();
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
    HTMLPositionedElementImpl::updateSize();
}

void HTMLTableElementImpl::attach(KHTMLWidget *)
{
    if(bgURL.length())
    {
	printf("TableCell: Requesting URL=%s\n", bgURL.string().ascii() );
	bgURL = document->requestImage(this, bgURL);
    }	
}

void HTMLTableElementImpl::detach()
{
    KHTMLCache::free(bgURL, this);
    NodeBaseImpl::detach();
}

void  HTMLTableElementImpl::setPixmap( QPixmap *p )
{
    background = p;
}

void  HTMLTableElementImpl::pixmapChanged( QPixmap *p )
{
    background = p;
}

// --------------------------------------------------------------------------

void HTMLTablePartElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_BGCOLOR:
	khtml::setNamedColor( bg, attr->value().string() );
	break;
    case ATTR_BACKGROUND:
        bgURL = attr->value();
	break;
    default:
	HTMLBlockElementImpl::parseAttribute(attr);
    }
}

void HTMLTablePartElementImpl::attach(KHTMLWidget *)
{
    if(bgURL.length())
    {
	printf("TableCell: Requesting URL=%s\n", bgURL.string().ascii() );
	bgURL = document->requestImage(this, bgURL);
    }	
}

void HTMLTablePartElementImpl::detach()
{
    KHTMLCache::free(bgURL, this);
    NodeBaseImpl::detach();
}

void  HTMLTablePartElementImpl::setPixmap( QPixmap *p )
{
    background = p;
}

void  HTMLTablePartElementImpl::pixmapChanged( QPixmap *p )
{
    background = p;
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
     printf("%s(TableSection)::layout(?) width=%d, layouted=%d\n", nodeName().string().ascii(), width, layouted());
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
  valign=VNone;
  halign=HNone;
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
    cell->setRowImpl(this);
    table->addCell(cell);

    return ret;
}

void HTMLTableRowElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_ALIGN:	
	if ( strcasecmp( attr->value(), "left" ) == 0 )
	    halign = Left;
	else if ( strcasecmp( attr->value(), "right" ) == 0 )
	    halign = Right;
	else if ( strcasecmp( attr->value(), "center" ) == 0 )
	    halign = HCenter;	
	break;
    case ATTR_VALIGN:
	if ( strcasecmp( attr->value(), "top" ) == 0 )
	    valign = Top;
	else if ( strcasecmp( attr->value(), "middle" ) == 0 )
	    valign = VCenter;
	else if ( strcasecmp( attr->value(), "bottom" ) == 0 )
	    valign = Bottom;
	else if ( strcasecmp( attr->value(), "baseline" ) == 0 )
	    valign = Baseline;
	break;	
    default:
	HTMLTablePartElementImpl::parseAttribute(attr);
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
  valign=VNone;
  halign=HNone;
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
	if(cSpan < 1) cSpan = 1; // fix for GNOME websites... ;-)
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
    case ATTR_ALIGN:
	if ( strcasecmp( attr->value(), "left" ) == 0 )
	    halign = Left;
	else if ( strcasecmp( attr->value(), "right" ) == 0 )
	    halign = Right;
	else if ( strcasecmp( attr->value(), "center" ) == 0 )
	    halign = HCenter;
	break;
    case ATTR_VALIGN:
	if ( strcasecmp( attr->value(), "top" ) == 0 )
	    valign = Top;
	else if ( strcasecmp( attr->value(), "middle" ) == 0 )
	    valign = VCenter;
	else if ( strcasecmp( attr->value(), "bottom" ) == 0 )
	    valign = Bottom;
	else if ( strcasecmp( attr->value(), "baseline" ) == 0 )
	    valign = Baseline;
	break;
    default:
	HTMLTablePartElementImpl::parseAttribute(attr);
    }
}

void HTMLTableCellElementImpl::calcMinMaxWidth()
{
#ifdef DEBUG_LAYOUT
    printf("%s(TableCell)::calcMinMaxWidth() known=%d\n", nodeName().string().ascii(), minMaxKnown());
#endif

    if(minMaxKnown()) return;
    
    int oldMin = minWidth;
    int oldMax = maxWidth;
    
    HTMLBlockElementImpl::calcMinMaxWidth();
    minWidth+=table->cellPadding()*2;
    maxWidth+=table->cellPadding()*2;

    if(nWrap && predefinedWidth.type!=Fixed) minWidth = maxWidth;
    
    if (minWidth!=oldMin || maxWidth!=oldMax)
        table->addColInfo(this);

}

#include "khtml.h"

void HTMLTableCellElementImpl::print(QPainter *p, int _x, int _y,
					int _w, int _h,
					int _tx, int _ty)
{
  //int padding = table->cellPadding();

    int _ascent = parentNode()->getAscent();

    _ty += y + _ascent;
    _tx += x;

    // check if we need to do anything at all...
    if((_ty - ascent > _y + _h) || (_ty + rowHeight < _y)) return;
    if((_tx > _x + _w) || (_tx + width < _x)) return;
    
    // background etc
    printObject(p, _x, _y, _w, _h, _tx, _ty-ascent);      
    
    // print children 

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

    QColor back = bg;
    if(!back.isValid())
	back = static_cast<HTMLTableRowElementImpl *>(parentNode())->bgColor();
    if(!back.isValid())
	back = table->bgColor();

    if ( back.isValid() )
    {
	QBrush brush( back );
	p->fillRect( _tx - padding , _ty - padding,
		     width+padding*2, rowHeight, brush );
    }


    QPixmap *pix = background;
    if(!pix || pix->isNull())
	pix = static_cast<HTMLTableRowElementImpl *>(parentNode())->bgPixmap();
    if(!pix || pix->isNull())
	pix = table->bgPixmap();

    if(pix && !pix->isNull())
    {
	p->drawTiledPixmap( _tx - padding , _ty - padding, width+padding*2, rowHeight,
			   *pix);
	return;
    }

}

void HTMLTableCellElementImpl::layout(bool deep)
{

    HTMLBlockElementImpl::layout(deep);
}


VAlign HTMLTableCellElementImpl::vAlign()
{
    VAlign va = valign;
    if (rowimpl && va==VNone)
    	va = rowimpl->vAlign();
    if (table && va==VNone)
    	va = table->vAlign();
    return va;
}

HAlign HTMLTableCellElementImpl::hAlign()
{
    HAlign ha = halign;
    if (rowimpl && ha==HNone)
    	ha = rowimpl->hAlign();
    return ha;
}

int HTMLTableCellElementImpl::getHeight()
{
    return MAX(predefinedHeight.minWidth(0),ascent+descent);
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
    return _id;
}


NodeImpl *HTMLTableColElementImpl::addChild(NodeImpl *child)
{
#ifdef DEBUG_LAYOUT
    printf("%s(Table)::addChild( %s )\n", nodeName().string().ascii(), child->nodeName().string().ascii());
#endif

    switch(child->id())
    {
    case ID_COL:
	// these have to come before the table definition!	
	HTMLElementImpl::addChild(child);
	HTMLTableColElementImpl* colel = static_cast<HTMLTableColElementImpl *>(child);
	colel->setStartCol(_currentCol);
//	printf("_currentCol=%d\n",_currentCol);
	table->addColInfo(colel);
	_currentCol++;
	return child;
    }
    return child;

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
    case ATTR_VALIGN:
	if ( strcasecmp( attr->value(), "top" ) == 0 )
	    valign = Top;
	else if ( strcasecmp( attr->value(), "middle" ) == 0 )
	    valign = VCenter;
	else if ( strcasecmp( attr->value(), "bottom" ) == 0 )
	    valign = Bottom;
	else if ( strcasecmp( attr->value(), "baseline" ) == 0 )
	    valign = Baseline;
	break;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }

}

Length HTMLTableColElementImpl::width()
{
    if (predefinedWidth.type == Undefined
	&& _parent && _parent->id()==ID_COLGROUP)
	return static_cast<HTMLTableColElementImpl*>(_parent)->width();
    else
	return predefinedWidth;	
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


