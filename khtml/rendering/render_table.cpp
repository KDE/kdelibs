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
 */

//#define TABLE_DEBUG
//#define TABLE_PRINT
//#define DEBUG_LAYOUT
//#define BOX_DEBUG
#include "rendering/render_table.h"
#include "rendering/table_layout.h"
#include "html/html_tableimpl.h"
#include "misc/htmltags.h"

#include <kglobal.h>

#include <qapplication.h>
#include <qstyle.h>

#include <kdebug.h>
#include <assert.h>

using namespace khtml;

RenderTable::RenderTable(DOM::NodeImpl* node)
    : RenderFlow(node)
{

    tCaption = 0;
    head = 0;
    foot = 0;
    firstBody = 0;

    m_maxWidth = 0;

    tableLayout = 0;

    rules = None;
    frame = Void;
    has_col_elems = false;
    needSectionRecalc = false;

    columnPos.resize( 2 );
    columnPos.fill( 0 );
    columns.resize( 1 );
    columns.fill( ColumnStruct() );

    columnPos[0] = spacing;
}

RenderTable::~RenderTable()
{
    delete tableLayout;
}

void RenderTable::setStyle(RenderStyle *_style)
{
    ETableLayout oldTableLayout = style() ? style()->tableLayout() : TAUTO;
    RenderFlow::setStyle(_style);

    // init RenderObject attributes
    setInline(style()->display()==INLINE_TABLE && !isPositioned());
    setReplaced(style()->display()==INLINE_TABLE);

    spacing = style()->borderSpacing();

    if ( !tableLayout || style()->tableLayout() != oldTableLayout ) {
	delete tableLayout;

	if (style()->tableLayout() == TFIXED ) {
	    tableLayout = new FixedTableLayout(this);
#ifdef DEBUG_LAYOUT
	    kdDebug( 6040 ) << "using fixed table layout" << endl;
#endif
	} else
	    tableLayout = new AutoTableLayout(this);
    }
}

void RenderTable::position(int x, int y, int, int, int, bool, bool, int)
{
    //for inline tables only
    m_x = x + marginLeft();
    m_y = y + marginTop();
}

void RenderTable::addChild(RenderObject *child, RenderObject *beforeChild)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(Table)::addChild( " << child->renderName() << ", " <<
                       (beforeChild ? beforeChild->renderName() : "0") << " )" << endl;
#endif
    RenderObject *o = child;

    switch(child->style()->display())
    {
    case TABLE_CAPTION:
        tCaption = static_cast<RenderTableCaption *>(child);
        break;
    case TABLE_COLUMN:
    case TABLE_COLUMN_GROUP:
	RenderContainer::addChild(child,beforeChild);
	has_col_elems = true;
        return;
    case TABLE_HEADER_GROUP:
	if ( !head )
	    head = static_cast<RenderTableSection *>(child);
        break;
    case TABLE_FOOTER_GROUP:
	if ( !foot )
	    foot = static_cast<RenderTableSection *>(child);
        break;
    case TABLE_ROW_GROUP:
        if(!firstBody)
            firstBody = static_cast<RenderTableSection *>(child);
        break;
    default:
        if ( !beforeChild && lastChild() &&
	     lastChild()->isTableSection() && lastChild()->isAnonymousBox() ) {
            o = lastChild();
        } else {
	    RenderObject *lastBox = beforeChild;
	    while ( lastBox && lastBox->parent()->isAnonymousBox() &&
		    !lastBox->isTableSection() && lastBox->style()->display() != TABLE_CAPTION )
		lastBox = lastBox->parent();
	    if ( lastBox && lastBox->isAnonymousBox() ) {
		lastBox->addChild( child, beforeChild );
		return;
	    } else {
		if ( beforeChild && !beforeChild->isTableSection() )
		    beforeChild = 0;
//   		kdDebug( 6040 ) << this <<" creating anonymous table section beforeChild="<< beforeChild << endl;
		o = new RenderTableSection(0 /* anonymous */);
		RenderStyle *newStyle = new RenderStyle();
		newStyle->inheritFrom(style());
		newStyle->setDisplay(TABLE_ROW_GROUP);
		o->setStyle(newStyle);
		o->setIsAnonymousBox(true);
		addChild(o, beforeChild);
	    }
        }
        o->addChild(child);
	child->setLayouted( false );
	child->setMinMaxKnown( false );
        return;
    }
    RenderContainer::addChild(child,beforeChild);
}



void RenderTable::calcWidth()
{
    if ( isPositioned() ) {
        calcAbsoluteHorizontal();
    }

    int borderWidth = borderLeft() + borderRight();
    RenderObject *cb = containingBlock();
    int availableWidth = cb->contentWidth() - borderWidth;


    LengthType widthType = style()->width().type;
    if(widthType > Relative) {
	// Percent or fixed table
        m_width = style()->width().minWidth( availableWidth );
        if(m_minWidth > m_width) m_width = m_minWidth;
	//kdDebug( 6040 ) << "1 width=" << m_width << " minWidth=" << m_minWidth << " availableWidth=" << availableWidth << " " << endl;
    } else {
        m_width = KMIN(short( availableWidth ),m_maxWidth);
    }

    // restrict width to what we really have in case we flow around floats
    if ( style()->flowAroundFloats() && cb->isFlow() )
	m_width = QMIN( static_cast<RenderFlow *>(cb)->lineWidth( m_y ) - borderWidth, m_width );

    m_width = KMAX (m_width, m_minWidth);

    m_marginRight=0;
    m_marginLeft=0;

    calcHorizontalMargins(style()->marginLeft(),style()->marginRight(),availableWidth);
}

void RenderTable::layout()
{
    KHTMLAssert( !layouted() );
    KHTMLAssert( minMaxKnown() );
    KHTMLAssert( !needSectionRecalc );

    //kdDebug( 6040 ) << renderName() << "(Table)"<< this << " ::layout0() width=" << width() << ", layouted=" << layouted() << endl;

    m_height = 0;

    //int oldWidth = m_width;
    calcWidth();

    // the optimisation below doesn't work since the internal table
    // layout could have changed.  we need to add a flag to the table
    // layout that tells us if something has changed in the min max
    // calculations to do it correctly.
//     if ( oldWidth != m_width || columns.size() + 1 != columnPos.size() )
    tableLayout->layout();

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(Table)::layout1() width=" << width() << ", marginLeft=" << marginLeft() << " marginRight=" << marginRight() << endl;
#endif

    setCellWidths();

    // layout child objects
    int calculatedHeight = 0;

    RenderObject *child = firstChild();
    while( child ) {
	if ( !child->layouted() )
	    child->layout();
	if ( child->isTableSection() ) {
	    static_cast<RenderTableSection *>(child)->calcRowHeight();
	    calculatedHeight += static_cast<RenderTableSection *>(child)->layoutRows( 0 );
	}
	child = child->nextSibling();
    }

    // ### collapse caption margin
    if(tCaption && tCaption->style()->captionSide() != CAPBOTTOM) {
        tCaption->setPos(tCaption->marginLeft(), m_height);
        m_height += tCaption->height() + tCaption->marginTop() + tCaption->marginBottom();
    }

    m_height += borderTop();

    // html tables with percent height are relative to view
    Length h = style()->height();
    int th=0;
    if (h.isFixed())
        th = h.value;
    else if (h.isPercent()) {
        Length ch = containingBlock()->style()->height();
        if (ch.isFixed())
            th = h.width(ch.value);
        else {
            // check we or not inside a table
            RenderObject* ro = parent();
            for (; ro && !ro->isTableCell(); ro=ro->parent());
            if (!ro)
            {
		// we need to substract the bodys margins
		// ### fixme: use exact values here.
                th = h.width(viewRect().height() - 20 );
                // not really, but this way the view height change
                // gets propagated correctly
                setOverhangingContents();
            }
        }
    }

    // layout rows
    if ( th > calculatedHeight ) {
	// we have to redistribute that height to get the constraint correctly
	// just force the first body to the height needed
	// ### FIXME This should take height constraints on all table sections into account and distribute
	// accordingly. For now this should be good enough
	firstBody->calcRowHeight();
	firstBody->layoutRows( th - calculatedHeight );
    }
    int bl = borderLeft();

    // position the table sections
    if ( head ) {
	head->setPos(bl, m_height);
	m_height += head->height();
    }
    RenderTableSection *body = firstBody;
    while ( body ) {
	body->setPos(bl, m_height);
	m_height += body->height();
	RenderObject *next = body->nextSibling();
	if ( next && next->isTableSection() && next != foot )
	    body = static_cast<RenderTableSection *>(next);
	else
	    body = 0;
    }
    if ( foot ) {
	foot->setPos(bl, m_height);
	m_height += foot->height();
    }


    m_height += borderBottom();

    if(tCaption && tCaption->style()->captionSide()==CAPBOTTOM) {
        tCaption->setPos(tCaption->marginLeft(), m_height);
        m_height += tCaption->height() + tCaption->marginTop() + tCaption->marginBottom();
    }

    //kdDebug(0) << "table height: " << m_height << endl;

    calcHeight();

    //kdDebug(0) << "table height: " << m_height << endl;

    // table can be containing block of positioned elements.
    // ### only pass true if width or height changed.
    layoutSpecialObjects( true );

    setLayouted();

}


void RenderTable::setCellWidths()
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(Table, this=0x" << this << ")::setCellWidths()" << endl;
#endif

    RenderObject *child = firstChild();
    while( child ) {
	if ( child->isTableSection() )
	    static_cast<RenderTableSection *>(child)->setCellWidths();
	child = child->nextSibling();
    }
}

void RenderTable::print( QPainter *p, int _x, int _y,
                                  int _w, int _h, int _tx, int _ty)
{

    if(!layouted()) return;

    _tx += xPos();
    _ty += yPos();

#ifdef TABLE_PRINT
    kdDebug( 6040 ) << "RenderTable::print() w/h = (" << width() << "/" << height() << ")" << endl;
#endif
    if (!overhangingContents() && !isRelPositioned() && !isPositioned())
    {
        if((_ty > _y + _h) || (_ty + height() < _y)) return;
        if((_tx > _x + _w) || (_tx + width() < _x)) return;
    }

#ifdef TABLE_PRINT
    kdDebug( 6040 ) << "RenderTable::print(2) " << _tx << "/" << _ty << " (" << _x << "/" << _y << ")" << endl;
#endif

    if(style()->visibility() == VISIBLE)
	printBoxDecorations(p, _x, _y, _w, _h, _tx, _ty);

    RenderObject *child = firstChild();
    while( child ) {
	if ( child->isTableSection() || child == tCaption )
	    child->print( p, _x, _y, _w, _h, _tx, _ty );
	child = child->nextSibling();
    }

    if ( specialObjects )
	printSpecialObjects( p, _x, _y, _w, _h, _tx, _ty);

#ifdef BOX_DEBUG
    outlineBox(p, _tx, _ty, "blue");
#endif
}

void RenderTable::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    if ( needSectionRecalc )
	recalcSections();

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(Table " << this << ")::calcMinMaxWidth()" <<  endl;
#endif

    tableLayout->calcMinMaxWidth();

    setMinMaxKnown();
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << " END: (Table " << this << ")::calcMinMaxWidth() min = " << m_minWidth << " max = " << m_maxWidth <<  endl;
#endif
}

void RenderTable::close()
{
//    kdDebug( 6040 ) << "RenderTable::close()" << endl;
    setLayouted(false);
    setMinMaxKnown(false);
}

int RenderTable::borderTopExtra()
{
    if (tCaption && tCaption->style()->captionSide()!=CAPBOTTOM)
        return -(tCaption->height() + tCaption->marginBottom() +  tCaption->marginTop());
    else
        return 0;

}

int RenderTable::borderBottomExtra()
{
    if (tCaption && tCaption->style()->captionSide()==CAPBOTTOM)
        return -(tCaption->height() + tCaption->marginBottom() +  tCaption->marginTop());
    else
        return 0;
}


void RenderTable::splitColumn( int pos, int firstSpan )
{
    // we need to add a new columnStruct
    int oldSize = columns.size();
    columns.resize( oldSize + 1 );
    int oldSpan = columns[pos].span;
    KHTMLAssert( oldSpan > firstSpan );
    columns[pos].span = firstSpan;
    memmove( columns.data()+pos+1, columns.data()+pos, (oldSize-pos)*sizeof(ColumnStruct) );
    columns[pos+1].span = oldSpan - firstSpan;

    // change width of all rows.
    RenderObject *child = firstChild();
    while ( child ) {
	if ( child->isTableSection() ) {
	    RenderTableSection *section = static_cast<RenderTableSection *>(child);
	    int size = section->grid.size();
	    int row = 0;
	    if ( section->cCol > pos )
		section->cCol++;
	    while ( row < size ) {
		section->grid[row].row->resize( oldSize+1 );
		RenderTableSection::Row &r = *section->grid[row].row;
		RenderTableCell *cell = r[pos];
		memmove( r.data()+pos+1, r.data()+pos, (oldSize-pos)*sizeof( RenderTableCell * ) );
		r[pos+1] = cell;
		row++;
	    }
	}
	child = child->nextSibling();
    }
    columnPos.resize( numEffCols()+1 );
    setMinMaxKnown( false );
    setLayouted( false );
}

void RenderTable::appendColumn( int span )
{
    // easy case.
    int pos = columns.size();
    int newSize = pos + 1;
    columns.resize( newSize );
    columns[pos].span = span;
    //qDebug("appending column at %d, span %d", pos,  span );

    // change width of all rows.
    RenderObject *child = firstChild();
    while ( child ) {
	if ( child->isTableSection() ) {
	    RenderTableSection *section = static_cast<RenderTableSection *>(child);
	    int size = section->grid.size();
	    int row = 0;
	    while ( row < size ) {
		section->grid[row].row->resize( newSize );
		section->cellAt( row, pos ) = 0;
		row++;
	    }

	}
	child = child->nextSibling();
    }
    columnPos.resize( numEffCols()+1 );
    setMinMaxKnown( false );
    setLayouted( false );
}

RenderTableCol *RenderTable::colElement( int col ) {
    if ( !has_col_elems )
	return 0;
    RenderObject *child = firstChild();
    int cCol = 0;
    while ( child ) {
	if ( child->isTableCol() ) {
	    RenderTableCol *colElem = static_cast<RenderTableCol *>(child);
	    int span = colElem->span();
	    if ( !colElem->firstChild() ) {
		if ( cCol >= col )
		    return colElem;
		cCol += span;
	    }

	    RenderObject *next = child->firstChild();
	    if ( !next )
		next = child->nextSibling();
	    if ( !next && child->parent()->isTableCol() )
		next = child->parent()->nextSibling();
	    child = next;
	} else
	    break;
    }
    return 0;
}

void RenderTable::setNeedSectionRecalc()
{
//     qDebug("setNeedSectionRecalc: %p",  this );
    needSectionRecalc = true;
}


void RenderTable::recalcSections()
{
//     qDebug("recalcSections, %p",  this);
    tCaption = 0;
    head = foot = firstBody = 0;
    has_col_elems = false;

    RenderObject *child = firstChild();
    // We need to get valid pointers to caption, head, foot and firstbody again
    while ( child ) {
	switch(child->style()->display()) {
	case TABLE_CAPTION:
	    if ( !tCaption)
		tCaption = static_cast<RenderTableCaption *>(child);
	    break;
	case TABLE_COLUMN:
	case TABLE_COLUMN_GROUP:
	    has_col_elems = true;
	    return;
	case TABLE_HEADER_GROUP: {
	    RenderTableSection *section = static_cast<RenderTableSection *>(child);
	    if ( !head )
		head = section;
	    if ( section->needCellRecalc )
		section->recalcCells();
	    break;
	}
	case TABLE_FOOTER_GROUP: {
	    RenderTableSection *section = static_cast<RenderTableSection *>(child);
	    if ( !foot )
		foot = section;
	    if ( section->needCellRecalc )
		section->recalcCells();
	    break;
	}
	case TABLE_ROW_GROUP: {
	    RenderTableSection *section = static_cast<RenderTableSection *>(child);
	    if ( !firstBody )
		firstBody = section;
	    if ( section->needCellRecalc )
		section->recalcCells();
	}
	default:
	    break;
	}
	child = child->nextSibling();
    }
    needSectionRecalc = false;
    setLayouted( false );
}

RenderObject* RenderTable::removeChildNode(RenderObject* child)
{
    setNeedSectionRecalc();
    return RenderContainer::removeChildNode( child );
}


#ifndef NDEBUG
void RenderTable::dump(QTextStream *stream, QString ind) const
{
    if (tCaption)
	*stream << " tCaption";
    if (head)
	*stream << " head";
    if (foot)
	*stream << " foot";

    *stream << endl << ind << "cspans:";
    for ( unsigned int i = 0; i < columns.size(); i++ )
	*stream << " " << columns[i].span;
    *stream << endl << ind;

    RenderFlow::dump(stream,ind);
}
#endif

// --------------------------------------------------------------------------

RenderTableSection::RenderTableSection(DOM::NodeImpl* node)
    : RenderBox(node)
{
    // init RenderObject attributes
    setInline(false);   // our object is not Inline
    cCol = 0;
    cRow = -1;
    needCellRecalc = false;
}

RenderTableSection::~RenderTableSection()
{
    // recalc cell info because RenderTable has unguarded pointers
    // stored that point to this RenderTableSection.
    if (table())
        table()->setNeedSectionRecalc();

    clearGrid();
}

void RenderTableSection::addChild(RenderObject *child, RenderObject *beforeChild)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(TableSection)::addChild( " << child->renderName()  << ", beforeChild=" <<
                       (beforeChild ? beforeChild->renderName() : "0") << " )" << endl;
#endif
    RenderObject *row = child;

    if ( !child->isTableRow() ) {

        if( !beforeChild )
            beforeChild = lastChild();

        if( beforeChild && beforeChild->isAnonymousBox() )
            row = beforeChild;
        else {
	    RenderObject *lastBox = beforeChild;
	    while ( lastBox && lastBox->parent()->isAnonymousBox() && !lastBox->isTableRow() )
		lastBox = lastBox->parent();
	    if ( lastBox && lastBox->isAnonymousBox() ) {
		lastBox->addChild( child, beforeChild );
		return;
	    } else {
		kdDebug( 6040 ) << "creating anonymous table row" << endl;
		row = new RenderTableRow(0 /* anonymous table */);
		RenderStyle *newStyle = new RenderStyle();
		newStyle->inheritFrom(style());
		newStyle->setDisplay(TABLE_ROW);
		row->setStyle(newStyle);
		row->setIsAnonymousBox(true);
		addChild(row, beforeChild);
	    }
        }
        row->addChild(child);
	child->setLayouted( false );
	child->setMinMaxKnown( false );
        return;
    }

    if (beforeChild)
	setNeedCellRecalc();

    cRow++;
    cCol = 0;

    ensureRows( cRow+1 );

    grid[cRow].height = child->style()->height();
    if ( grid[cRow].height.type == Relative )
	grid[cRow].height = Length();

    RenderContainer::addChild(child,beforeChild);
}

void RenderTableSection::ensureRows( int numRows )
{
    int nRows = grid.size();
    int nCols = table()->numEffCols();
    if ( numRows > nRows ) {
	grid.resize( numRows );
	for ( int r = nRows; r < numRows; r++ ) {
	    grid[r].row = new Row( nCols );
	    grid[r].row->fill( 0 );
	}
    }

}

void RenderTableSection::addCell( RenderTableCell *cell )
{
    int rSpan = cell->rowSpan();
    int cSpan = cell->colSpan();
    QMemArray<RenderTable::ColumnStruct> &columns = table()->columns;
    int nCols = columns.size();

    // ### mozilla still seems to do the old HTML way, even for strict DTD
    // (see the annotation on table cell layouting in the CSS specs and the testcase below:
    // <TABLE border>
    // <TR><TD>1 <TD rowspan="2">2 <TD>3 <TD>4
    // <TR><TD colspan="2">5
    // </TABLE>
#if 0
    // find empty space for the cell
    bool found = false;
    while ( !found ) {
	found = true;
	while ( cCol < nCols && cellAt( cRow, cCol ) )
	    cCol++;
	int pos = cCol;
	int span = 0;
	while ( pos < nCols && span < cSpan ) {
	    if ( cellAt( cRow, pos ) ) {
		found = false;
		cCol = pos;
		break;
	    }
	    span += columns[pos].span;
	    pos++;
	}
    }
#else
    while ( cCol < nCols && cellAt( cRow, cCol ) )
	cCol++;
#endif

//     qDebug("adding cell at %d/%d span=(%d/%d)",  cRow, cCol, rSpan, cSpan );

    if ( rSpan == 1 ) {
	// we ignore height settings on rowspan cells
	Length height = cell->style()->height();
	if ( height.value > 0 || (height.type == Relative && height.value >= 0) ) {
	    Length cRowHeight = grid[cRow].height;
	    switch( height.type ) {
	    case Percent:
		if ( !(cRowHeight.type == Percent) ||
		     ( cRowHeight.type == Percent && cRowHeight.value < height.value ) )
		    grid[cRow].height = height;
		     break;
	    case Fixed:
		if ( cRowHeight.type < Percent ||
		     ( cRowHeight.type == Fixed && cRowHeight.value < height.value ) )
		    grid[cRow].height = height;
		break;
	    case Relative:
#if 0
		// we treat this as variable. This is correct according to HTML4, as it only specifies length for the height.
		if ( cRowHeight.type == Variable ||
		     ( cRowHeight.type == Relative && cRowHeight.value < height.value ) )
		     grid[cRow].height = height;
		     break;
#endif
	    default:
		break;
	    }
	}
    }

    // make sure we have enough rows
    ensureRows( cRow + rSpan );

    int col = cCol;
    // tell the cell where it is
    while ( cSpan ) {
	int currentSpan;
	if ( cCol >= nCols ) {
	    table()->appendColumn( cSpan );
	    currentSpan = cSpan;
	} else {
	    if ( cSpan < columns[cCol].span )
		table()->splitColumn( cCol, cSpan );
	    currentSpan = columns[cCol].span;
	}
	int r = 0;
	while ( r < rSpan ) {
	    //qDebug("adding cell at %d, %d",  cRow + r, cCol );
	    cellAt( cRow + r, cCol ) = cell;
	    r++;
	}
	cCol++;
	cSpan -= currentSpan;
    }
    if ( cell ) {
	cell->setRow( cRow );
	cell->setCol( table()->effColToCol( col ) );
    }
}


void RenderTableSection::setCellWidths()
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(Table, this=0x" << this << ")::setCellWidths()" << endl;
#endif
    QMemArray<int> &columnPos = table()->columnPos;

    int rows = grid.size();
    for ( int i = 0; i < rows; i++ ) {
	Row &row = *grid[i].row;
	int cols = row.size();
	RenderTableCell *lastCell = 0;
	int lastCol = 0;
	for ( int j = 0; j < cols; j++ ) {
	    //qDebug("row(%d/%d) is %p, lastCell: %p",  i,  j,  row[j], lastCell );
	    if ( row[j] == lastCell )
		continue;
	    if ( lastCell ) {
		int w = columnPos[j]-columnPos[lastCol] - table()->cellSpacing();
#ifdef DEBUG_LAYOUT
		kdDebug( 6040 ) << "setting width of cell " << lastCell << " " << lastCell->row() << "/" << lastCell->col() << " to " << w << endl;
#endif
		int oldWidth = lastCell->width();
		if ( w != oldWidth ) {
		    lastCell->setLayouted(false);
		    lastCell->setWidth( w );
		}
	    }
	    lastCell = row[j];
	    lastCol = j;
	}
	if ( lastCell && lastCol < int( row.size() ) ) {
#ifdef DEBUG_LAYOUT
            kdDebug( 6040 ) << "lastcell: " << lastCell << " lastCol: " << lastCol << endl;
#endif
	    int w = columnPos[cols]-columnPos[lastCol] - table()->cellSpacing();
	    int oldWidth = lastCell->width();
#ifdef DEBUG_LAYOUT
	    kdDebug( 6040 ) << "setting width of cell " << lastCell->row() << "/" << lastCell->col() << " to " << w << endl;
#endif
	    if ( w != oldWidth ) {
		lastCell->setLayouted(false);
		lastCell->setWidth( w );
	    }
	}
    }
}


void RenderTableSection::calcRowHeight()
{
    int indx;
    RenderTableCell *cell;

    int totalRows = grid.size();
    int spacing = table()->cellSpacing();

    rowPos.resize( totalRows + 1 );
    rowPos[0] =  spacing + borderTop();

    for ( int r = 0; r < totalRows; r++ ) {
	rowPos[r+1] = 0;

	int baseline=0;
	int bdesc = 0;
// 	qDebug("height of row %d is %d/%d", r, grid[r].height.value, grid[r].height.type );
	int ch = grid[r].height.minWidth( 0 );
	int pos = rowPos[ r+1 ] + ch + table()->cellSpacing();

	if ( pos > rowPos[r+1] )
	    rowPos[r+1] = pos;

	Row *row = grid[r].row;
	int totalCols = row->size();
	int totalRows = grid.size();

	for ( int c = 0; c < totalCols; c++ ) {
	    if ( ( cell = cellAt(r, c) ) == 0 )
		continue;
	    if ( c < totalCols - 1 && cell == cellAt(r, c+1) )
		continue;
	    if ( r < totalRows - 1 && cellAt(r+1, c) == cell )
		continue;

	    if ( ( indx = r - cell->rowSpan() + 1 ) < 0 )
		indx = 0;

	    ch = cell->style()->height().width(0);
	    if ( cell->height() > ch)
		ch = cell->height();

	    pos = rowPos[ indx ] + ch + table()->cellSpacing();

	    if ( pos > rowPos[r+1] )
		rowPos[r+1] = pos;

	    // find out the baseline
	    EVerticalAlign va = cell->style()->verticalAlign();
	    if (va == BASELINE || va == TEXT_BOTTOM || va == TEXT_TOP
		|| va == SUPER || va == SUB)
	    {
		int b=cell->baselinePosition();

		if (b>baseline)
		    baseline=b;

		int td = rowPos[ indx ] + ch - b;
		if (td>bdesc)
		    bdesc = td;
	    }
	}

	//do we have baseline aligned elements?
	if (baseline) {
	    // increase rowheight if baseline requires
	    int bRowPos = baseline + bdesc  + table()->cellSpacing() ; // + 2*padding
	    if (rowPos[r+1]<bRowPos)
		rowPos[r+1]=bRowPos;

	    grid[r].baseLine = baseline;
	}

	if ( rowPos[r+1] < rowPos[r] )
	    rowPos[r+1] = rowPos[r];
//  	qDebug("rowpos(%d)=%d",  r, rowPos[r] );
    }
}

int RenderTableSection::layoutRows( int toAdd )
{
    int rHeight;
    int rindx;
    int totalRows = grid.size();
    int spacing = table()->cellSpacing();

    if (toAdd && totalRows && rowPos[totalRows]) {

	int totalHeight = rowPos[totalRows] + toAdd;
// 	qDebug("layoutRows: totalHeight = %d",  totalHeight );

        int dh = totalHeight-rowPos[totalRows];
	int totalPercent = 0;
	int numVariable = 0;
	for ( int r = 0; r < totalRows; r++ ) {
	    if ( grid[r].height.type == Variable )
		numVariable++;
	    else if ( grid[r].height.type == Percent )
		totalPercent += grid[r].height.value;
	}
	if ( totalPercent ) {
// 	    qDebug("distributing %d over percent rows totalPercent=%d", dh,  totalPercent );
	    // try to satisfy percent
	    int add = 0;
	    if ( totalPercent > 100 )
		totalPercent = 100;
	    for ( int r = 0; r < totalRows; r++ ) {
		if ( totalPercent > 0 && grid[r].height.type == Percent ) {
		    int toAdd = QMIN( dh, totalHeight * grid[r].height.value / 100 );
		    add += toAdd;
		    dh -= toAdd;
		    totalPercent -= grid[r].height.value;
		}
                rowPos[r+1] += add;
	    }
	}
	if ( numVariable ) {
	    // distribute over variable cols
// 	    qDebug("distributing %d over variable rows numVariable=%d", dh,  numVariable );
	    int add = 0;
	    for ( int r = 0; r < totalRows; r++ ) {
		if ( numVariable > 0 && grid[r].height.type == Variable ) {
		    int toAdd = dh/numVariable;
		    add += toAdd;
		    dh -= toAdd;
		}
                rowPos[r+1] += add;
	    }
	}
        if (dh>0) {
	    // if some left overs, distribute equally.
            int tot=rowPos[totalRows];
            int add=0;
            int prev=rowPos[0];
            for ( int r = 0; r < totalRows; r++ ) {
                //weight with the original height
                add+=dh*(rowPos[r+1]-prev)/tot;
                prev=rowPos[r+1];
                rowPos[r+1]+=add;
            }
        }
    }

    int leftOffset = borderLeft() + spacing;

    int nEffCols = table()->numEffCols();
    for ( int r = 0; r < totalRows; r++ )
    {
	Row *row = grid[r].row;
	int totalCols = row->size();
        for ( int c = 0; c < nEffCols; c++ )
        {
            RenderTableCell *cell = cellAt(r, c);
            if (!cell)
                continue;
            if ( c > 0 && cell == cellAt(r, c-1) )
                continue;
            if ( r < totalRows - 1 && cell == cellAt(r+1, c) )
                continue;

            if ( ( rindx = r-cell->rowSpan()+1 ) < 0 )
                rindx = 0;

#ifdef DEBUG_LAYOUT
            kdDebug( 6040 ) << "setting position " << r << "/" << c << ": "
			    << table()->columnPos[c] /*+ padding */ << "/" << rowPos[rindx] << " " << endl;
#endif
            rHeight = rowPos[r+1] - rowPos[rindx] - spacing;

            EVerticalAlign va = cell->style()->verticalAlign();
            int te=0;
            switch (va)
            {
            case SUB:
            case SUPER:
            case TEXT_TOP:
            case TEXT_BOTTOM:
            case BASELINE:
		te = getBaseline(r) - cell->baselinePosition() ;
                break;
            case TOP:
                te = 0;
                break;
            case MIDDLE:
                te = (rHeight - cell->height())/2;
                break;
            case BOTTOM:
                te = rHeight - cell->height();
                break;
            default:
                break;
            }
#ifdef DEBUG_LAYOUT
	    //            kdDebug( 6040 ) << "CELL " << cell << " te=" << te << ", be=" << rHeight - cell->height() - te << ", rHeight=" << rHeight << ", valign=" << va << endl;
#endif
            cell->setCellTopExtra( te );
            cell->setCellBottomExtra( rHeight - cell->height() - te);

            if (style()->direction()==RTL) {
                cell->setPos(
		    table()->columnPos[(int)totalCols] -
		    table()->columnPos[table()->colToEffCol(cell->col()+cell->colSpan())] +
		    leftOffset,
                    rowPos[rindx] );
            } else {
                cell->setPos( table()->columnPos[c] + leftOffset, rowPos[rindx] );
	    }
        }
    }

    m_height = rowPos[totalRows];
    return m_height;
}


void RenderTableSection::print( QPainter *p, int x, int y, int w, int h,
				int tx, int ty)
{
    unsigned int totalRows = grid.size();
    unsigned int totalCols = table()->columns.size();

    tx += m_x;
    ty += m_y;
    // check which rows and cols are visible and only print these
    // ### fixme: could use a binary search here
    unsigned int startrow = 0;
    unsigned int endrow = totalRows;
    for ( ; startrow < totalRows; startrow++ ) {
	if ( ty + rowPos[startrow+1] > y )
	    break;
    }
    for ( ; endrow > 0; endrow-- ) {
	if ( ty + rowPos[endrow-1] < y + h )
	    break;
    }
    unsigned int startcol = 0;
    unsigned int endcol = totalCols;
    if ( style()->direction() == LTR ) {
	for ( ; startcol < totalCols; startcol++ ) {
	    if ( tx + table()->columnPos[startcol+1] > x )
		break;
	}
	for ( ; endcol > 0; endcol-- ) {
	    if ( tx + table()->columnPos[endcol-1] < x + w )
		break;
	}
    }

    // draw the cells
    for ( unsigned int r = startrow; r < endrow; r++ ) {
        for ( unsigned int c = startcol; c < endcol; c++ ) {
            RenderTableCell *cell = cellAt(r, c);
            if (!cell)
                continue;
            if ( (c < endcol - 1) && (cell == cellAt(r, c+1) ) )
                continue;
	    if ( (r < endrow - 1) && (cellAt(r+1, c) == cell) )
                continue;
#ifdef TABLE_PRINT
	    kdDebug( 6040 ) << "printing cell " << r << "/" << c << endl;
#endif
	    cell->print( p, x, y, w, h, tx, ty);
	}
    }
}


void RenderTableSection::setNeedCellRecalc()
{
//     qDebug("setNeedCellRecalc: %p, table=%p",  this, table() );
    needCellRecalc = true;
    table()->setNeedSectionRecalc();
}

void RenderTableSection::recalcCells()
{
//     qDebug("recalcCells, %p",  this);
    cCol = cRow = 0;
    clearGrid();
    grid.resize( 0 );
    ensureRows( 1 );

    RenderObject *row = firstChild();
    while ( row ) {
	RenderObject *cell = row->firstChild();
	while ( cell ) {
	    if ( cell->isTableCell() )
		addCell( static_cast<RenderTableCell *>(cell) );
	    cell = cell->nextSibling();
	}
	row = row->nextSibling();
    }
    needCellRecalc = false;
    setLayouted( false );
}

void RenderTableSection::clearGrid()
{
    int rows = grid.size();
    while ( rows-- ) {
	delete grid[rows].row;
    }
}

RenderObject* RenderTableSection::removeChildNode(RenderObject* child)
{
    setNeedCellRecalc();
    return RenderContainer::removeChildNode( child );
}

#ifndef NDEBUG
void RenderTableSection::dump(QTextStream *stream, QString ind) const
{
    *stream << endl << ind << "grid=(" << grid.size() << "," << table()->numEffCols() << ")" << endl << ind;
    for ( unsigned int r = 0; r < grid.size(); r++ ) {
	for ( int c = 0; c < table()->numEffCols(); c++ ) {
	    if ( cellAt( r,  c ) )
		*stream << "(" << cellAt( r, c )->row() << "," << cellAt( r, c )->col() << ","
			<< cellAt(r, c)->rowSpan() << "," << cellAt(r, c)->colSpan() << ") ";
	    else
		*stream << cellAt( r, c ) << "null cell ";
	}
	*stream << endl << ind;
    }
    RenderContainer::dump(stream,ind);
}
#endif

// -------------------------------------------------------------------------

RenderTableRow::RenderTableRow(DOM::NodeImpl* node)
    : RenderContainer(node)
{
    // init RenderObject attributes
    setInline(false);   // our object is not Inline
}

RenderTableRow::~RenderTableRow()
{
    if (section())
        section()->setNeedCellRecalc();
}

void RenderTableRow::addChild(RenderObject *child, RenderObject *beforeChild)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(TableRow)::addChild( " << child->renderName() << " )"  << ", " <<
                       (beforeChild ? beforeChild->renderName() : "0") << " )" << endl;
#endif
    RenderTableCell *cell;

    if ( !child->isTableCell() ) {
        if ( !beforeChild )
            beforeChild = lastChild();
        RenderTableCell *cell;
        if( beforeChild && beforeChild->isAnonymousBox() && beforeChild->isTableCell() )
            cell = static_cast<RenderTableCell *>(beforeChild);
        else {
	    RenderObject *lastBox = beforeChild;
	    while ( lastBox && lastBox->parent()->isAnonymousBox() && !lastBox->isTableCell() )
		lastBox = lastBox->parent();
	    if ( lastBox && lastBox->isAnonymousBox() ) {
		lastBox->addChild( child, beforeChild );
		return;
	    } else {
//          kdDebug( 6040 ) << "creating anonymous table cell" << endl;
		cell = new RenderTableCell(0 /* anonymous object */);
		RenderStyle *newStyle = new RenderStyle();
		newStyle->inheritFrom(style());
		newStyle->setDisplay(TABLE_CELL);
		cell->setStyle(newStyle);
		cell->setIsAnonymousBox(true);
		addChild(cell, beforeChild);
	    }
        }
        cell->addChild(child);
	child->setLayouted( false );
	child->setMinMaxKnown( false );
        return;
    } else
        cell = static_cast<RenderTableCell *>(child);

    static_cast<RenderTableSection *>(parent())->addCell( cell );

    RenderContainer::addChild(cell,beforeChild);

    if ( (beforeChild || nextSibling()) && section() )
	section()->setNeedCellRecalc();
}

RenderObject* RenderTableRow::removeChildNode(RenderObject* child)
{
    if ( section() )
	section()->setNeedCellRecalc();
    return RenderContainer::removeChildNode( child );
}

#ifndef NDEBUG
void RenderTableRow::dump(QTextStream *stream, QString ind) const
{
    RenderContainer::dump(stream,ind);
}
#endif

void RenderTableRow::layout()
{
    KHTMLAssert( !layouted() );
    KHTMLAssert( minMaxKnown() );

    RenderObject *child = firstChild();
    while( child ) {
	if ( child->isTableCell() && !child->layouted() ) {
	    RenderTableCell *cell = static_cast<RenderTableCell *>(child);
	    cell->calcVerticalMargins();
	    cell->layout();
	    cell->setCellTopExtra(0);
	    cell->setCellBottomExtra(0);
	}
	child = child->nextSibling();
    }
    setLayouted();
}

// -------------------------------------------------------------------------

RenderTableCell::RenderTableCell(DOM::NodeImpl* _node)
  : RenderFlow(_node)
{
  _col = -1;
  _row = -1;
  updateFromElement();
  setSpecialObjects(true);
  _topExtra = 0;
  _bottomExtra = 0;
}

RenderTableCell::~RenderTableCell()
{
    if (parent() && section())
        section()->setNeedCellRecalc();
}

void RenderTableCell::updateFromElement()
{
  DOM::NodeImpl *node = element();
  if ( node && (node->id() == ID_TD || node->id() == ID_TH) ) {
      DOM::HTMLTableCellElementImpl *tc = static_cast<DOM::HTMLTableCellElementImpl *>(node);
      cSpan = tc->colSpan();
      rSpan = tc->rowSpan();
      nWrap = tc->noWrap();
  } else {
      cSpan = rSpan = 1;
      nWrap = false;
  }
}

void RenderTableCell::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(TableCell)::calcMinMaxWidth() known=" << minMaxKnown() << endl;
#endif

    RenderFlow::calcMinMaxWidth();

    if(nWrap && !(style()->width().type==Fixed))
        m_minWidth = m_maxWidth;

    setMinMaxKnown();
}

void RenderTableCell::calcWidth()
{
}

void RenderTableCell::setWidth( int width )
{
    if ( width != m_width ) {
	m_width = width;
	m_widthChanged = true;
    }
}

void RenderTableCell::close()
{
    RenderFlow::close();

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(RenderTableCell)::close() total height =" << m_height << endl;
#endif
}


void RenderTableCell::repaintRectangle(int x, int y, int w, int h, bool f)
{
    y += _topExtra;
    RenderFlow::repaintRectangle(x, y, w, h, f);
}

bool RenderTableCell::absolutePosition(int &xPos, int &yPos, bool f)
{
    bool ret = RenderFlow::absolutePosition(xPos, yPos, f);
    if (ret)
      yPos += _topExtra;
    return ret;
}

short RenderTableCell::baselinePosition( bool ) const
{
    RenderObject *o = firstChild();
    int offset = paddingTop() + borderTop();
    if ( !o ) return offset;
    while ( o->firstChild() ) {
	if ( !o->isInline() )
	    offset += o->paddingTop() + o->borderTop();
	o = o->firstChild();
    }
    offset += o->baselinePosition( true );
    return offset;
}


void RenderTableCell::setStyle( RenderStyle *style )
{
    RenderFlow::setStyle( style );
    setSpecialObjects(true);
}

#ifdef BOX_DEBUG
#include <qpainter.h>

static void outlineBox(QPainter *p, int _tx, int _ty, int w, int h)
{
    p->setPen(QPen(QColor("yellow"), 3, Qt::DotLine));
    p->setBrush( Qt::NoBrush );
    p->drawRect(_tx, _ty, w, h );
}
#endif

void RenderTableCell::print(QPainter *p, int _x, int _y,
                                       int _w, int _h, int _tx, int _ty)
{

#ifdef TABLE_PRINT
    kdDebug( 6040 ) << renderName() << "(RenderTableCell)::print() w/h = (" << width() << "/" << height() << ")" << endl;
#endif

    if (!layouted()) return;

    _tx += m_x;
    _ty += m_y + _topExtra;

    // check if we need to do anything at all...
    if(!overhangingContents() && ((_ty-_topExtra > _y + _h)
        || (_ty + m_height+_topExtra+_bottomExtra < _y))) return;

    printObject(p, _x, _y, _w, _h, _tx, _ty);

#ifdef BOX_DEBUG
    ::outlineBox( p, _tx, _ty - _topExtra, width(), height() + borderTopExtra() + borderBottomExtra());
#endif
}


void RenderTableCell::printBoxDecorations(QPainter *p,int, int _y,
                                       int, int _h, int _tx, int _ty)
{
    int w = width();
    int h = height() + borderTopExtra() + borderBottomExtra();
    _ty -= borderTopExtra();

    int my = KMAX(_ty,_y);
    int mh;
    if (_ty<_y)
        mh= KMAX(0,h-(_y-_ty));
    else
        mh = KMIN(_h,h);

    QColor c = style()->backgroundColor();
    if ( !c.isValid() && parent() ) // take from row
        c = parent()->style()->backgroundColor();
    if ( !c.isValid() && parent() && parent()->parent() ) // take from rowgroup
        c = parent()->parent()->style()->backgroundColor();
    if ( !c.isValid() ) {
	// see if we have a col or colgroup for this
	RenderTableCol *col = table()->colElement( _col );
	if ( col ) {
	    c = col->style()->backgroundColor();
	    if ( !c.isValid() ) {
		// try column group
		RenderStyle *style = col->parent()->style();
		if ( style->display() == TABLE_COLUMN_GROUP )
		    c = style->backgroundColor();
	    }
	}
    }

    // ### get offsets right in case the bgimage is inherited.
    CachedImage *bg = style()->backgroundImage();
    if ( !bg && parent() )
        bg = parent()->style()->backgroundImage();
    if ( !bg && parent() && parent()->parent() )
        bg = parent()->parent()->style()->backgroundImage();
    if ( !bg ) {
	// see if we have a col or colgroup for this
	RenderTableCol *col = table()->colElement( _col );
	if ( col ) {
	    bg = col->style()->backgroundImage();
	    if ( !bg ) {
		// try column group
		RenderStyle *style = col->parent()->style();
		if ( style->display() == TABLE_COLUMN_GROUP )
		    bg = style->backgroundImage();
	    }
	}
    }

    if ( bg || c.isValid() )
	printBackground(p, c, bg, my, mh, _tx, _ty, w, h);

    if(style()->hasBorder())
        printBorder(p, _tx, _ty, w, h, style());
}


#ifndef NDEBUG
void RenderTableCell::dump(QTextStream *stream, QString ind) const
{
    *stream << " row=" << _row;
    *stream << " col=" << _col;
    *stream << " rSpan=" << rSpan;
    *stream << " cSpan=" << cSpan;
//    *stream << " nWrap=" << nWrap;

    RenderFlow::dump(stream,ind);
}
#endif

// -------------------------------------------------------------------------

RenderTableCol::RenderTableCol(DOM::NodeImpl* node)
    : RenderContainer(node)
{
    // init RenderObject attributes
    setInline(true);   // our object is not Inline

    _span = 1;
    updateFromElement();
}

RenderTableCol::~RenderTableCol()
{
}

void RenderTableCol::updateFromElement()
{
  DOM::NodeImpl *node = element();
  if ( node && (node->id() == ID_COL || node->id() == ID_COLGROUP) ) {
      DOM::HTMLTableColElementImpl *tc = static_cast<DOM::HTMLTableColElementImpl *>(node);
      _span = tc->span();
  } else {
      if ( style()->display() == TABLE_COLUMN_GROUP )
	  _span = 0;
      else
	  _span = 1;
  }
}

void RenderTableCol::addChild(RenderObject *child, RenderObject *beforeChild)
{
#ifdef DEBUG_LAYOUT
    //kdDebug( 6040 ) << renderName() << "(Table)::addChild( " << child->renderName() << " )"  << ", " <<
    //                   (beforeChild ? beforeChild->renderName() : 0) << " )" << endl;
#endif

    if (child->style()->display() == TABLE_COLUMN)
        // these have to come before the table definition!
        RenderContainer::addChild(child,beforeChild);
}

#ifndef NDEBUG
void RenderTableCol::dump(QTextStream *stream, QString ind) const
{
    *stream << " _span=" << _span;
    RenderContainer::dump(stream,ind);
}
#endif

// -------------------------------------------------------------------------

RenderTableCaption::RenderTableCaption(DOM::NodeImpl* node)
  : RenderFlow(node)
{
}

RenderTableCaption::~RenderTableCaption()
{
}

#undef TABLE_DEBUG
#undef DEBUG_LAYOUT
#undef BOX_DEBUG
