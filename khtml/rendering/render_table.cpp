/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1997 Martin Jones (mjones@kde.org)
 *           (C) 1997 Torben Weis (weis@kde.org)
 *           (C) 1998 Waldo Bastian (bastian@kde.org)
 *           (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2003 Apple Computer, Inc.
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
#include "misc/htmlattrs.h"
#include "rendering/render_line.h"
#include "xml/dom_docimpl.h"

#include <kglobal.h>

#include <qapplication.h>
#include <qstyle.h>

#include <kdebug.h>
#include <assert.h>

using namespace khtml;
using namespace DOM;

RenderTable::RenderTable(DOM::NodeImpl* node)
    : RenderBlock(node)
{

    tCaption = 0;
    head = foot = firstBody = 0;
    tableLayout = 0;

    rules = None;
    frame = Void;
    has_col_elems = false;
    hspacing = vspacing = 0;
    padding = 0;
    needSectionRecalc = false;
    padding = 0;

    columnPos.resize( 2 );
    columnPos.fill( 0 );
    columns.resize( 1 );
    columns.fill( ColumnStruct() );

    columnPos[0] = 0;
}

RenderTable::~RenderTable()
{
    delete tableLayout;
}

void RenderTable::setStyle(RenderStyle *_style)
{
    ETableLayout oldTableLayout = style() ? style()->tableLayout() : TAUTO;
    if ( _style->display() == INLINE ) _style->setDisplay( INLINE_TABLE );
    if ( _style->display() != INLINE_TABLE ) _style->setDisplay(TABLE);
    RenderBlock::setStyle(_style);

    // init RenderObject attributes
    setInline(style()->display()==INLINE_TABLE && !isPositioned());
    setReplaced(style()->display()==INLINE_TABLE);

    hspacing = style()->borderHorizontalSpacing();
    vspacing = style()->borderVerticalSpacing();
    columnPos[0] = hspacing;

    if ( !tableLayout || style()->tableLayout() != oldTableLayout ) {
	delete tableLayout;

        // According to the CSS2 spec, you only use fixed table layout if an
        // explicit width is specified on the table.  Auto width implies auto table layout.
	if (style()->tableLayout() == TFIXED && !style()->width().isVariable()) {
	    tableLayout = new FixedTableLayout(this);
#ifdef DEBUG_LAYOUT
	    kdDebug( 6040 ) << "using fixed table layout" << endl;
#endif
	} else
	    tableLayout = new AutoTableLayout(this);
    }
}

short RenderTable::lineHeight(bool b) const
{
    // Inline tables are replaced elements. Otherwise, just pass off to
    // the base class.
    if (isReplaced())
        return height()+marginTop()+marginBottom();
    return RenderBlock::lineHeight(b);
}

short RenderTable::baselinePosition(bool b) const
{
    // Inline tables are replaced elements. Otherwise, just pass off to
    // the base class.
    if (isReplaced())
        return height()+marginTop()+marginBottom();
    return RenderBlock::baselinePosition(b);
}


void RenderTable::addChild(RenderObject *child, RenderObject *beforeChild)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(Table)::addChild( " << child->renderName() << ", " <<
                       (beforeChild ? beforeChild->renderName() : "0") << " )" << endl;
#endif
    RenderObject *o = child;

    if (child->element() && child->element()->id() == ID_FORM) {
        RenderContainer::addChild(child,beforeChild);
        return;
    }

    switch(child->style()->display())
    {
    case TABLE_CAPTION:
        tCaption = static_cast<RenderBlock *>(child);
        break;
    case TABLE_COLUMN:
    case TABLE_COLUMN_GROUP:
	has_col_elems = true;
        break;
    case TABLE_HEADER_GROUP:
	if ( !head )
	    head = static_cast<RenderTableSection *>(child);
	else if ( !firstBody )
            firstBody = static_cast<RenderTableSection *>(child);
        break;
    case TABLE_FOOTER_GROUP:
	if ( !foot ) {
	    foot = static_cast<RenderTableSection *>(child);
	    break;
	}
	// fall through
    case TABLE_ROW_GROUP:
        if(!firstBody)
            firstBody = static_cast<RenderTableSection *>(child);
        break;
    default:
        if ( !beforeChild && lastChild() &&
	     lastChild()->isTableSection() && lastChild()->isAnonymous() ) {
            o = lastChild();
        } else {
	    RenderObject *lastBox = beforeChild;
	    while ( lastBox && lastBox->parent()->isAnonymous() &&
		    !lastBox->isTableSection() && lastBox->style()->display() != TABLE_CAPTION )
		lastBox = lastBox->parent();
	    if ( lastBox && lastBox->isAnonymous() ) {
		lastBox->addChild( child, beforeChild );
		return;
	    } else {
		if ( beforeChild && !beforeChild->isTableSection() )
		    beforeChild = 0;
  		//kdDebug( 6040 ) << this <<" creating anonymous table section beforeChild="<< beforeChild << endl;
		o = new (renderArena()) RenderTableSection(document() /* anonymous */);
		RenderStyle *newStyle = new RenderStyle();
		newStyle->inheritFrom(style());
                newStyle->setDisplay(TABLE_ROW_GROUP);
		o->setStyle(newStyle);
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

    RenderBlock *cb = containingBlock();
    int availableWidth = cb->contentWidth();

    LengthType widthType = style()->width().type();
    if(widthType > Relative && style()->width().value() > 0) {
	// Percent or fixed table
        m_width = style()->width().minWidth( availableWidth );
        if(m_minWidth > m_width) m_width = m_minWidth;
	//kdDebug( 6040 ) << "1 width=" << m_width << " minWidth=" << m_minWidth << " availableWidth=" << availableWidth << " " << endl;
    } else {
        m_width = KMIN(short( availableWidth ),m_maxWidth);
    }

    // restrict width to what we really have in case we flow around floats
    // EXCEPT percent tables, which are still calculated as above
    if ( style()->flowAroundFloats() && widthType!=Percent ) {
	availableWidth = cb->lineWidth( m_y );
	m_width = KMIN( short( availableWidth ), m_width );
    }

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
    initMaxMarginValues();

    //int oldWidth = m_width;
    calcWidth();
    m_overflowWidth = m_width;

    // the optimization below doesn't work since the internal table
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
	if ( !child->layouted() && !(child->element() && child->element()->id() == ID_FORM) )
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
        th = h.value();
    else if (h.isPercent()) {
        RenderObject* c = containingBlock();
        for ( ;
            !c->isCanvas() && !c->isBody() && !c->isTableCell() &&
		  !c->isPositioned() && c->isFloating();
             c = c->containingBlock()) {
            Length ch = c->style()->height();
            if (ch.isFixed()) {
                th = h.width(ch.value());
                break;
            }
        }

        if (!c->isTableCell()) {
            Length ch = c->style()->height();
	    if (ch.isFixed())
		th = h.width(ch.value());
	    else {
		// we need to substract out the margins of this block. -dwh
		th = h.width(viewRect().height() - c->marginBottom() - c->marginTop());
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
        if (firstBody) {
            firstBody->calcRowHeight();
            firstBody->layoutRows( th - calculatedHeight );
        }
    }
    int bl = borderLeft();

    // position the table sections
    if ( head ) {
	head->setPos(bl, m_height);
	m_height += head->height();
    }
    RenderObject *body = firstBody;
    while ( body ) {
	if ( body != head && body != foot && body->isTableSection() ) {
	    body->setPos(bl, m_height);
	    m_height += body->height();
	}
	body = body->nextSibling();
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
    layoutPositionedObjects( true );

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

void RenderTable::paint( PaintInfo& pI, int _tx, int _ty)
{

    if(!layouted()) return;

    _tx += xPos();
    _ty += yPos();

#ifdef TABLE_PRINT
    kdDebug( 6040 ) << "RenderTable::paint() w/h = (" << width() << "/" << height() << ")" << endl;
#endif
    if (!overhangingContents() && !isRelPositioned() && !isPositioned())
    {
        if((_ty > pI.r.y() + pI.r.height()) || (_ty + height() < pI.r.y())) return;
        if((_tx > pI.r.x() + pI.r.width()) || (_tx + width() < pI.r.x())) return;
    }

#ifdef TABLE_PRINT
    kdDebug( 6040 ) << "RenderTable::paint(2) " << _tx << "/" << _ty << " (" << _y << "/" << _h << ")" << endl;
#endif

    if(( pI.phase == PaintActionElementBackground || pI.phase == PaintActionChildBackground ) &&
       style()->visibility() == VISIBLE)
	paintBoxDecorations(pI, _tx, _ty);

    if ( pI.phase == PaintActionElementBackground )
        return;

    PaintAction oldphase = pI.phase;
    if ( pI.phase == PaintActionChildBackgrounds )
        pI.phase = PaintActionChildBackground;

    for( RenderObject *child = firstChild(); child; child = child->nextSibling())
	if ( child->isTableSection() || child == tCaption )
	    child->paint( pI, _tx, _ty );

    pI.phase = oldphase;
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

    if (tCaption && tCaption->minWidth() > m_minWidth)
        m_minWidth = tCaption->minWidth();

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
//     qDebug("splitColumn( %d,%d ), oldSize=%d, oldSpan=%d", pos, firstSpan, oldSize, oldSpan );
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
		memmove( r.data()+pos+1, r.data()+pos, (oldSize-pos)*sizeof( RenderTableCell * ) );
// 		qDebug("moving from %d to %d, num=%d", pos, pos+1, (oldSize-pos-1) );
		r[pos+1] = r[pos] ? (RenderTableCell *)-1 : 0;
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
//     qDebug("appendColumn( %d ), size=%d", span, pos );
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
		cCol += span;
		if ( cCol > col )
		    return colElem;
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

void RenderTable::recalcSections()
{
    tCaption = 0;
    head = foot = firstBody = 0;
    has_col_elems = false;

    RenderObject *child = firstChild();
    // We need to get valid pointers to caption, head, foot and firstbody again
    while ( child ) {
	switch(child->style()->display()) {
	case TABLE_CAPTION:
	    if ( !tCaption) {
		tCaption = static_cast<RenderBlock*>(child);
                tCaption->setLayouted(false);
            }
	    break;
	case TABLE_COLUMN:
	case TABLE_COLUMN_GROUP:
	    has_col_elems = true;
	    break;
	case TABLE_HEADER_GROUP: {
	    RenderTableSection *section = static_cast<RenderTableSection *>(child);
	    if ( !head )
		head = section;
	    else if ( !firstBody )
		firstBody = section;
	    if ( section->needCellRecalc )
		section->recalcCells();
	    break;
	}
	case TABLE_FOOTER_GROUP: {
	    RenderTableSection *section = static_cast<RenderTableSection *>(child);
	    if ( !foot )
		foot = section;
	    else if ( !firstBody )
		firstBody = section;
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


#ifdef ENABLE_DUMP
void RenderTable::dump(QTextStream &stream, const QString &ind) const
{
    RenderBlock::dump(stream, ind);

    if (tCaption)
	stream << " tCaption";
    if (head)
	stream << " head";
    if (foot)
	stream << " foot";

    stream << " [cspans:";
    for ( unsigned int i = 0; i < columns.size(); i++ )
	stream << " " << columns[i].span;
    stream << "]";
}

#endif

FindSelectionResult RenderTable::checkSelectionPoint( int _x, int _y, int _tx, int _ty, DOM::NodeImpl*& node, int & offset, SelPointState &state )
{
    int off = offset;
    DOM::NodeImpl* nod = node;

    FindSelectionResult pos;
    TableSectionIterator it(this);
    for (; *it; ++it) {
        pos = (*it)->checkSelectionPoint(_x, _y, _tx + m_x, _ty + m_y, nod, off, state);
        switch(pos) {
        case SelectionPointBeforeInLine:
        case SelectionPointInside:
            //kdDebug(6030) << "RenderTable::checkSelectionPoint " << this << " returning SelectionPointInside offset=" << offset << endl;
            node = nod;
            offset = off;
            return SelectionPointInside;
        case SelectionPointBefore:
            //x,y is before this element -> stop here
            if ( state.m_lastNode ) {
                node = state.m_lastNode;
                offset = state.m_lastOffset;
                //kdDebug(6030) << "RenderTable::checkSelectionPoint " << this << " before this child "
                //              << node << "-> returning SelectionPointInside, offset=" << offset << endl;
                return SelectionPointInside;
            } else {
                node = nod;
                offset = off;
                //kdDebug(6030) << "RenderTable::checkSelectionPoint " << this << " before us -> returning SelectionPointBefore " << node << "/" << offset << endl;
                return SelectionPointBefore;
            }
            break;
        case SelectionPointAfter:
	    if (state.m_afterInLine) break;
	    // fall through
        case SelectionPointAfterInLine:
	    if (pos == SelectionPointAfterInLine) state.m_afterInLine = true;
            //kdDebug(6030) << "RenderTable::checkSelectionPoint: selection after: " << nod << " offset: " << off << " afterInLine: " << state.m_afterInLine << endl;
            state.m_lastNode = nod;
            state.m_lastOffset = off;
            // No "return" here, obviously. We must keep looking into the children.
            break;
        }
    }
    // If we are after the last child, return lastNode/lastOffset
    // But lastNode can be 0L if there is no child, for instance.
    if ( state.m_lastNode )
    {
        node = state.m_lastNode;
        offset = state.m_lastOffset;
    }
    // Fallback
    return SelectionPointAfter;
}

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
    clearGrid();
}

void RenderTableSection::detach()
{
    // recalc cell info because RenderTable has unguarded pointers
    // stored that point to this RenderTableSection.
    if (table())
        table()->setNeedSectionRecalc();

    RenderBox::detach();
}

void RenderTableSection::setStyle(RenderStyle* _style)
{
    // we don't allow changing this one
    if (style())
        _style->setDisplay(style()->display());
    else if (_style->display() != TABLE_FOOTER_GROUP && _style->display() != TABLE_HEADER_GROUP)
        _style->setDisplay(TABLE_ROW_GROUP);

    RenderBox::setStyle(_style);
}

void RenderTableSection::addChild(RenderObject *child, RenderObject *beforeChild)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(TableSection)::addChild( " << child->renderName()  << ", beforeChild=" <<
                       (beforeChild ? beforeChild->renderName() : "0") << " )" << endl;
#endif
    RenderObject *row = child;

    if (child->element() && child->element()->id() == ID_FORM) {
        RenderContainer::addChild(child,beforeChild);
        return;
    }

    if ( !child->isTableRow() ) {

        if( !beforeChild )
            beforeChild = lastChild();

        if( beforeChild && beforeChild->isAnonymous() )
            row = beforeChild;
        else {
	    RenderObject *lastBox = beforeChild;
	    while ( lastBox && lastBox->parent()->isAnonymous() && !lastBox->isTableRow() )
		lastBox = lastBox->parent();
	    if ( lastBox && lastBox->isAnonymous() ) {
		lastBox->addChild( child, beforeChild );
		return;
	    } else {
		//kdDebug( 6040 ) << "creating anonymous table row" << endl;
		row = new (renderArena()) RenderTableRow(document() /* anonymous table */);
		RenderStyle *newStyle = new RenderStyle();
		newStyle->inheritFrom(style());
		newStyle->setDisplay( TABLE_ROW );
		row->setStyle(newStyle);
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

    if (!beforeChild) {
        grid[cRow].height = child->style()->height();
        if ( grid[cRow].height.isRelative() )
            grid[cRow].height = Length();
    }


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
	    grid[r].baseLine = 0;
	    grid[r].height = Length();
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

//       qDebug("adding cell at %d/%d span=(%d/%d)",  cRow, cCol, rSpan, cSpan );

    if ( rSpan == 1 ) {
	// we ignore height settings on rowspan cells
	Length height = cell->style()->height();
	if ( height.value() > 0 || (height.isRelative() && height.value() >= 0) ) {
	    Length cRowHeight = grid[cRow].height;
	    switch( height.type() ) {
	    case Percent:
		if ( !cRowHeight.isPercent() ||
		     (cRowHeight.isPercent() && cRowHeight.value() < height.value() ) )
		    grid[cRow].height = height;
		     break;
	    case Fixed:
		if ( cRowHeight.type() < Percent ||
		     ( cRowHeight.isFixed() && cRowHeight.value() < height.value() ) )
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
    RenderTableCell *set = cell;
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
	    if ( !cellAt( cRow + r, cCol ) ) {
// 		qDebug("    adding cell at %d, %d",  cRow + r, cCol );
		cellAt( cRow + r, cCol ) = set;
	    }
	    r++;
	}
	cCol++;
	cSpan -= currentSpan;
	set = (RenderTableCell *)-1;
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
	for ( int j = 0; j < cols; j++ ) {
	    RenderTableCell *cell = row[j];
// 	    qDebug("cell[%d,%d] = %p", i, j, cell );
	    if ( !cell || cell == (RenderTableCell *)-1 )
		continue;
	    int endCol = j;
	    int cspan = cell->colSpan();
	    while ( cspan && endCol < cols ) {
		cspan -= table()->columns[endCol].span;
		endCol++;
	    }
	    int w = columnPos[endCol] - columnPos[j] - table()->borderHSpacing();
#ifdef DEBUG_LAYOUT
	    kdDebug( 6040 ) << "setting width of cell " << cell << " " << cell->row() << "/" << cell->col() << " to " << w << " colspan=" << cell->colSpan() << " start=" << j << " end=" << endCol << endl;
#endif
	    int oldWidth = cell->width();
	    if ( w != oldWidth ) {
		cell->setLayouted(false);
		cell->setWidth( w );
	    }
	}
    }
}


void RenderTableSection::calcRowHeight()
{
    int indx;
    RenderTableCell *cell;

    int totalRows = grid.size();
    int vspacing = table()->borderVSpacing();

    rowPos.resize( totalRows + 1 );
    rowPos[0] =  vspacing + borderTop();

    for ( int r = 0; r < totalRows; r++ ) {
	rowPos[r+1] = 0;

	int baseline=0;
	int bdesc = 0;
// 	qDebug("height of row %d is %d/%d", r, grid[r].height.value, grid[r].height.type );
	int ch = grid[r].height.minWidth( 0 );
	int pos = rowPos[ r+1 ] + ch + vspacing;

	if ( pos > rowPos[r+1] )
	    rowPos[r+1] = pos;

	Row *row = grid[r].row;
	int totalCols = row->size();
	int totalRows = grid.size();

	for ( int c = 0; c < totalCols; c++ ) {
	    cell = cellAt(r, c);
	    if ( !cell || cell == (RenderTableCell *)-1 )
		continue;
	    if ( r < totalRows - 1 && cellAt(r+1, c) == cell )
		continue;

	    if ( ( indx = r - cell->rowSpan() + 1 ) < 0 )
		indx = 0;

	    ch = cell->style()->height().width(0);
	    if ( cell->height() > ch)
		ch = cell->height();

	    pos = rowPos[ indx ] + ch + vspacing;

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
	    int bRowPos = baseline + bdesc  + vspacing ; // + 2*padding
	    if (rowPos[r+1]<bRowPos)
		rowPos[r+1]=bRowPos;

	    grid[r].baseLine = baseline;
	}

	if ( rowPos[r+1] < rowPos[r] )
	    rowPos[r+1] = rowPos[r];
//   	qDebug("rowpos(%d)=%d",  r, rowPos[r] );
    }
}

int RenderTableSection::layoutRows( int toAdd )
{
    int rHeight;
    int rindx;
    int totalRows = grid.size();
    int hspacing = table()->borderHSpacing();
    int vspacing = table()->borderVSpacing();

    if (toAdd && totalRows && (rowPos[totalRows] || !nextSibling())) {

	int totalHeight = rowPos[totalRows] + toAdd;
// 	qDebug("layoutRows: totalHeight = %d",  totalHeight );

        int dh = totalHeight-rowPos[totalRows];
	int totalPercent = 0;
	int numVariable = 0;
	for ( int r = 0; r < totalRows; r++ ) {
	    if ( grid[r].height.isVariable() )
		numVariable++;
	    else if ( grid[r].height.isPercent() )
		totalPercent += grid[r].height.value();
	}
	if ( totalPercent ) {
// 	    qDebug("distributing %d over percent rows totalPercent=%d", dh,  totalPercent );
	    // try to satisfy percent
	    int add = 0;
	    if ( totalPercent > 100 )
		totalPercent = 100;
	    int rh = rowPos[1]-rowPos[0];
	    for ( int r = 0; r < totalRows; r++ ) {
		if ( totalPercent > 0 && grid[r].height.isPercent() ) {
		    int toAdd = kMin( dh, (totalHeight * grid[r].height.value() / 100)-rh );
		    // If toAdd is negative, then we don't want to shrink the row (this bug
                    // affected Outlook Web Access).
                    toAdd = kMax(0, toAdd);
		    add += toAdd;
		    dh -= toAdd;
		    totalPercent -= grid[r].height.value();
// 		    qDebug( "adding %d to row %d", toAdd, r );
		}
		if ( r < totalRows-1 )
		    rh = rowPos[r+2] - rowPos[r+1];
                rowPos[r+1] += add;
	    }
	}
	if ( numVariable ) {
	    // distribute over variable cols
// 	    qDebug("distributing %d over variable rows numVariable=%d", dh,  numVariable );
	    int add = 0;
	    for ( int r = 0; r < totalRows; r++ ) {
		if ( numVariable > 0 && grid[r].height.isVariable() ) {
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

    int leftOffset = borderLeft() + hspacing;

    int nEffCols = table()->numEffCols();
    for ( int r = 0; r < totalRows; r++ )
    {
	Row *row = grid[r].row;
	int totalCols = row->size();
        for ( int c = 0; c < nEffCols; c++ )
        {
            RenderTableCell *cell = cellAt(r, c);
            if (!cell || cell == (RenderTableCell *)-1 )
                continue;
            if ( r < totalRows - 1 && cell == cellAt(r+1, c) )
		continue;

            if ( ( rindx = r-cell->rowSpan()+1 ) < 0 )
                rindx = 0;

            rHeight = rowPos[r+1] - rowPos[rindx] - vspacing;

            // Force percent height children to lay themselves out again.
            // This will cause, e.g., textareas to grow to
            // fill the area.  FIXME: <div>s and blocks still don't
            // work right.  We'll need to have an efficient way of
            // invalidating all percent height objects in a render subtree.
            // For now, we just handle immediate children. -dwh
            bool cellChildrenFlex = false;
            RenderObject* o = cell->firstChild();
            while (o) {
                if (o->style()->height().isPercent()) {
                    o->setLayouted(false);
                    cellChildrenFlex = true;
                }
                o = o->nextSibling();
            }
            if (cellChildrenFlex) {
                cell->setCellPercentageHeight(rHeight);
                if (!cell->layouted()) cell->layout();

                // Alignment within a cell is based off the calculated
                // height, which becomes irrelevant once the cell has
                // been resized based off its percentage. -dwh
                cell->setCellTopExtra(0);
                cell->setCellBottomExtra(0);
            } else {
#ifdef DEBUG_LAYOUT
		kdDebug( 6040 ) << "setting position " << r << "/" << c << ": "
				<< table()->columnPos[c] /*+ padding */ << "/" << rowPos[rindx] << " height=" << rHeight<< endl;
#endif

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
	    }
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


void RenderTableSection::paint( PaintInfo& pI, int tx, int ty )
{
    unsigned int totalRows = grid.size();
    unsigned int totalCols = table()->columns.size();

    tx += m_x;
    ty += m_y;

    int x = pI.r.x(), y = pI.r.y(), w = pI.r.width(), h = pI.r.height();
    // check which rows and cols are visible and only paint these
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
    if ( startcol < endcol ) {
	// draw the cells
	for ( unsigned int r = startrow; r < endrow; r++ ) {
	    unsigned int c = startcol;
	    // since a cell can be -1 (indicating a colspan) we might have to search backwards to include it
	    while ( c && cellAt( r, c ) == (RenderTableCell *)-1 )
		c--;
	    for ( ; c < endcol; c++ ) {
		RenderTableCell *cell = cellAt(r, c);
		if (!cell || cell == (RenderTableCell *)-1 )
		    continue;
		if ( (r < endrow - 1) && (cellAt(r+1, c) == cell) )
		    continue;

#ifdef TABLE_PRINT
		kdDebug( 6040 ) << "painting cell " << r << "/" << c << endl;
#endif
		cell->paint( pI, tx, ty);
	    }
	}
    }
}

void RenderTableSection::recalcCells()
{
    cCol = 0;
    cRow = -1;
    clearGrid();
    grid.resize( 0 );

    RenderObject *row = firstChild();
    while ( row ) {
	cRow++;
	cCol = 0;
	ensureRows( cRow+1 );
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

#ifdef ENABLE_DUMP
void RenderTableSection::dump(QTextStream &stream, const QString &ind) const
{
    RenderContainer::dump(stream,ind);

    stream << " grid=(" << grid.size() << "," << table()->numEffCols() << ")";
    for ( unsigned int r = 0; r < grid.size(); r++ ) {
	for ( int c = 0; c < table()->numEffCols(); c++ ) {
	    if ( cellAt( r,  c ) && cellAt( r, c ) != (RenderTableCell *)-1 )
		stream << " (" << cellAt( r, c )->row() << "," << cellAt( r, c )->col() << ","
                       << cellAt(r, c)->rowSpan() << "," << cellAt(r, c)->colSpan() << ") ";
	    else
		stream << " null cell";
	}
    }
}
#endif

static RenderTableCell *seekCell(RenderTableSection *section, int row, int col)
{
    if (row < 0 || col < 0 || row >= section->numRows()) return 0;
    // since a cell can be -1 (indicating a colspan) we might have to search backwards to include it
    while ( col && section->cellAt( row, col ) == (RenderTableCell *)-1 )
	col--;

    return section->cellAt(row, col);
}

/** Looks for the first element suitable for text selection, beginning from
 * the last.
 * @param base search is restricted within this node. This node must have
 *	a renderer.
 * @return the element or @p base if no suitable element found.
 */
static NodeImpl *findLastSelectableNode(NodeImpl *base)
{
  NodeImpl *last = base;
  // Look for last text/cdata node that has a renderer,
  // or last childless replaced element
  while ( last && !(last->renderer()
  	&& ((last->nodeType() == Node::TEXT_NODE || last->nodeType() == Node::CDATA_SECTION_NODE)
		|| (last->renderer()->isReplaced() && !last->renderer()->lastChild()))))
  {
    NodeImpl *next = last->lastChild();
    if ( !next ) next = last->previousSibling();
    while ( last != base && !next )
    {
      last = last->parentNode();
      if ( last != base )
        next = last->previousSibling();
    }
    last = next;
  }

  return last ? last : base;
}

FindSelectionResult RenderTableSection::checkSelectionPoint( int _x, int _y, int _tx, int _ty, DOM::NodeImpl*& node, int & offset, SelPointState &state )
{
    // Table sections need extra treatment for selections. The rows are scanned
    // from top to bottom, and within each row, only the cell that matches
    // the given position best is descended into.

    unsigned int totalRows = grid.size();
    unsigned int totalCols = table()->columns.size();

//    absolutePosition(_tx, _ty, false);

    _tx += m_x;
    _ty += m_y;

//    bool save_last = false;	// true to save last investigated cell

    if (!layouted() || _y < _ty) return SelectionPointBefore;
//    else if (_y >= _ty + height()) save_last = true;

    // bluntly taken from paint (LS)
    // check which rows and cols are visible and only paint these
    // ### fixme: could use a binary search here
    int row_idx = (int)totalRows - 1;
    if ( row_idx > 0 ) {
        for ( ; row_idx >= 0; row_idx-- ) {
            if ( _ty + rowPos[row_idx] < _y )
                break;
        }
        if (row_idx < 0) row_idx = 0;
    }
    int col_idx;
    if ( style()->direction() == LTR ) {
	for ( col_idx = (int)totalCols - 1; col_idx >= 0; col_idx-- ) {
	    if ( _tx + table()->columnPos[col_idx] < _x )
		break;
	}
	if (col_idx < 0) col_idx = 0;
    } else {
	for ( col_idx = 0; col_idx < (int)totalCols; col_idx++ ) {
	    if ( _tx + table()->columnPos[col_idx] > _x )
		break;
	}
	if (col_idx >= (int)totalCols) col_idx = (int)totalCols + 1;
    }

    FindSelectionResult pos = SelectionPointBefore;

    RenderTableCell *cell = seekCell(this, row_idx, col_idx);
    // ### dunno why cell can be 0, maybe due to weird spans? (LS)
    if (cell) {
        SelPointState localState;
        pos = cell->checkSelectionPoint(_x, _y, _tx, _ty, node, offset, localState);
    }

    if (pos != SelectionPointBefore) return pos;

    // store last column of last line
    row_idx--;
    col_idx = totalCols - 1;
    cell = seekCell(this, row_idx, col_idx);

    // end of section? take previous section
    RenderTableSection *sec = this;
    if (!cell) {
        sec = *--TableSectionIterator(sec);
        if (!sec) return pos;

	cell = seekCell(sec, sec->grid.size() - 1, col_idx);
	if (!cell) return pos;
    }

    // take last child of previous cell, and store this one as last node
    NodeImpl *element = cell->element();
    if (!element) return SelectionPointBefore;

    element = findLastSelectableNode(element);

    state.m_lastNode = element;
    state.m_lastOffset = element->maxOffset();
    return SelectionPointBefore;
}

// -------------------------------------------------------------------------

RenderTableRow::RenderTableRow(DOM::NodeImpl* node)
    : RenderContainer(node)
{
    // init RenderObject attributes
    setInline(false);   // our object is not Inline
}

void RenderTableRow::detach()
{
    RenderTableSection *s = section();
    if (s)
        s->setNeedCellRecalc();

    RenderContainer::detach();
}

void RenderTableRow::setStyle(RenderStyle* style)
{
    style->setDisplay(TABLE_ROW);
    RenderContainer::setStyle(style);
}

void RenderTableRow::addChild(RenderObject *child, RenderObject *beforeChild)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(TableRow)::addChild( " << child->renderName() << " )"  << ", " <<
                       (beforeChild ? beforeChild->renderName() : "0") << " )" << endl;
#endif
    if (child->element() && child->element()->id() == ID_FORM) {
        RenderContainer::addChild(child,beforeChild);
        return;
    }

    RenderTableCell *cell;

    if ( !child->isTableCell() ) {
	RenderObject *last = beforeChild;
        if ( !last )
            last = lastChild();
        RenderTableCell *cell = 0;
        if( last && last->isAnonymous() && last->isTableCell() )
            cell = static_cast<RenderTableCell *>(last);
        else {
	    cell = new (renderArena()) RenderTableCell(document() /* anonymous object */);
	    RenderStyle *newStyle = new RenderStyle();
	    newStyle->inheritFrom(style());
	    newStyle->setDisplay( TABLE_CELL );
	    cell->setStyle(newStyle);
	    addChild(cell, beforeChild);
        }
        cell->addChild(child);
	child->setLayouted( false );
	child->setMinMaxKnown( false );
        return;
    } else
        cell = static_cast<RenderTableCell *>(child);

    static_cast<RenderTableSection *>(parent())->addCell( cell );

    RenderContainer::addChild(cell,beforeChild);

    if ( ( beforeChild || nextSibling()) && section() )
	section()->setNeedCellRecalc();
}

RenderObject* RenderTableRow::removeChildNode(RenderObject* child)
{
// RenderTableCell detach should do it
//     if ( section() )
// 	section()->setNeedCellRecalc();
    return RenderContainer::removeChildNode( child );
}

#ifdef ENABLE_DUMP
void RenderTableRow::dump(QTextStream &stream, const QString &ind) const
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
	if ( child->isTableCell() ) {
            RenderTableCell *cell = static_cast<RenderTableCell *>(child);
            if ( cell->cellPercentageHeight() ) {
                cell->setCellPercentageHeight( 0 );
                if ( cell->layouted() )
                    cell->setLayouted( false );
            }
            if ( !child->layouted() ) {
                cell->calcVerticalMargins();
                cell->layout();
                cell->setCellTopExtra(0);
                cell->setCellBottomExtra(0);
            }
	}
	child = child->nextSibling();
    }
    setLayouted();
}

// -------------------------------------------------------------------------

RenderTableCell::RenderTableCell(DOM::NodeImpl* _node)
  : RenderBlock(_node)
{
  _col = -1;
  _row = -1;
  updateFromElement();
  setShouldPaintBackgroundOrBorder(true);
  _topExtra = 0;
  _bottomExtra = 0;
  m_percentageHeight = 0;
}

void RenderTableCell::detach()
{
    if (parent() && section())
        section()->setNeedCellRecalc();

    RenderBlock::detach();
}

void RenderTableCell::updateFromElement()
{
  DOM::NodeImpl *node = element();
  if ( node && (node->id() == ID_TD || node->id() == ID_TH) ) {
      DOM::HTMLTableCellElementImpl *tc = static_cast<DOM::HTMLTableCellElementImpl *>(node);
      cSpan = tc->colSpan();
      rSpan = tc->rowSpan();
  } else {
      cSpan = rSpan = 1;
  }
}

void RenderTableCell::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(TableCell)::calcMinMaxWidth() known=" << minMaxKnown() << endl;
#endif

    RenderBlock::calcMinMaxWidth();
    if (element() && style()->whiteSpace() == NORMAL) {
        // See if nowrap was set.
        DOMString nowrap = static_cast<ElementImpl*>(element())->getAttribute(ATTR_NOWRAP);
        if (!nowrap.isNull() && style()->width().isFixed() &&
            m_minWidth < style()->width().value() )
            // Nowrap is set, but we didn't actually use it because of the
            // fixed width set on the cell.  Even so, it is a WinIE/Moz trait
            // to make the minwidth of the cell into the fixed width.  They do this
            // even in strict mode, so do not make this a quirk.  Affected the top
            // of hiptop.com.
            m_minWidth = style()->width().value();
    }

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

void RenderTableCell::layout()
{
    layoutBlock( m_widthChanged );
    m_widthChanged = false;
}

void RenderTableCell::close()
{
    RenderBlock::close();

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(RenderTableCell)::close() total height =" << m_height << endl;
#endif
}


void RenderTableCell::repaintRectangle(int x, int y, int w, int h, bool immediate, bool f)
{
    y += _topExtra;
    RenderBlock::repaintRectangle(x, y, w, h, immediate, f);
}

bool RenderTableCell::absolutePosition(int &xPos, int &yPos, bool f)
{
    bool ret = RenderBlock::absolutePosition(xPos, yPos, f);
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
    style->setDisplay(TABLE_CELL);
    RenderBlock::setStyle( style );
    setShouldPaintBackgroundOrBorder(true);

    if (style->whiteSpace() == KHTML_NOWRAP) {
      // Figure out if we are really nowrapping or if we should just
      // use normal instead.  If the width of the cell is fixed, then
      // we don't actually use NOWRAP.
      if (style->width().isFixed())
	style->setWhiteSpace(NORMAL);
      else
	style->setWhiteSpace(NOWRAP);
    }
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

void RenderTableCell::paint(PaintInfo& pI, int _tx, int _ty)
{

#ifdef TABLE_PRINT
    kdDebug( 6040 ) << renderName() << "(layouted: " << layouted() << ")::paint() w/h = (" << width() << "/" << height() << ")" << " _y/_h=" << _y << "/" << _h << endl;
#endif

    if (!layouted()) return;

    _tx += m_x;
    _ty += m_y + _topExtra;

    // check if we need to do anything at all...
    if(!overhangingContents() && ((_ty-_topExtra > pI.r.y() + pI.r.height())
        || (_ty + m_height + _bottomExtra < pI.r.y()))) return;

    paintObject(pI, _tx, _ty);

#ifdef BOX_DEBUG
    ::outlineBox( p, _tx, _ty - _topExtra, width(), height() + borderTopExtra() + borderBottomExtra());
#endif
}


void RenderTableCell::paintBoxDecorations(PaintInfo& pI, int _tx, int _ty)
{
    int w = width();
    int h = height() + borderTopExtra() + borderBottomExtra();
    _ty -= borderTopExtra();

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

    int my = kMax(_ty,pI.r.y());
    int end = kMin( pI.r.y() + pI.r.height(),  _ty + h );
    int mh = end - my;

    if ( bg || c.isValid() )
	paintBackground(pI.p, c, bg, my, mh, _tx, _ty, w, h);

    if(style()->hasBorder())
        paintBorder(pI.p, _tx, _ty, w, h, style());
}


#ifdef ENABLE_DUMP
void RenderTableCell::dump(QTextStream &stream, const QString &ind) const
{
    RenderFlow::dump(stream,ind);
    stream << " row=" << _row;
    stream << " col=" << _col;
    stream << " rSpan=" << rSpan;
    stream << " cSpan=" << cSpan;
//    *stream << " nWrap=" << nWrap;
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

void RenderTableCol::updateFromElement()
{
  DOM::NodeImpl *node = element();
  if ( node && (node->id() == ID_COL || node->id() == ID_COLGROUP) ) {
      DOM::HTMLTableColElementImpl *tc = static_cast<DOM::HTMLTableColElementImpl *>(node);
      _span = tc->span();
  } else
      _span = ! ( style() && style()->display() == TABLE_COLUMN_GROUP );
}

void RenderTableCol::addChild(RenderObject *child, RenderObject *beforeChild)
{
#ifdef DEBUG_LAYOUT
    //kdDebug( 6040 ) << renderName() << "(Table)::addChild( " << child->renderName() << " )"  << ", " <<
    //                   (beforeChild ? beforeChild->renderName() : 0) << " )" << endl;
#endif

    KHTMLAssert(child->style()->display() == TABLE_COLUMN);

    // these have to come before the table definition!
    RenderContainer::addChild(child,beforeChild);
}

#ifdef ENABLE_DUMP
void RenderTableCol::dump(QTextStream &stream, const QString &ind) const
{
    RenderContainer::dump(stream,ind);
    stream << " _span=" << _span;
}
#endif

// -------------------------------------------------------------------------

TableSectionIterator::TableSectionIterator(RenderTable *table, bool fromEnd)
{
  if (fromEnd) {
    sec = table->foot;
    if (sec) return;

    sec = static_cast<RenderTableSection *>(table->lastChild());
    while (sec && (!sec->isTableSection()
    		|| sec == table->head || sec == table->foot))
      sec = static_cast<RenderTableSection *>(sec->previousSibling());
    if (sec) return;

    sec = table->head;
  } else {
    sec = table->head;
    if (sec) return;

    sec = static_cast<RenderTableSection *>(table->firstChild());
    while (sec && (!sec->isTableSection()
    		|| sec == table->head || sec == table->foot))
      sec = static_cast<RenderTableSection *>(sec->nextSibling());
    if (sec) return;

    sec = table->foot;
  }/*end if*/

}

TableSectionIterator &TableSectionIterator::operator ++()
{
  RenderTable *table = sec->table();
  if (sec == table->head) {

    sec = static_cast<RenderTableSection *>(table->firstChild());
    while (sec && (!sec->isTableSection()
    		|| sec == table->head || sec == table->foot))
      sec = static_cast<RenderTableSection *>(sec->nextSibling());
    if (sec) return *this;

  } else if (sec == table->foot) {
    sec = 0;
    return *this;

  } else {

    do {
      sec = static_cast<RenderTableSection *>(sec->nextSibling());
    } while (sec && (!sec->isTableSection() || sec == table->head || sec == table->foot));
    if (sec) return *this;

  }/*end if*/

  sec = table->foot;
  return *this;
}

TableSectionIterator &TableSectionIterator::operator --()
{
  RenderTable *table = sec->table();
  if (sec == table->foot) {

    sec = static_cast<RenderTableSection *>(table->lastChild());
    while (sec && (!sec->isTableSection()
    		|| sec == table->head || sec == table->foot))
      sec = static_cast<RenderTableSection *>(sec->previousSibling());
    if (sec) return *this;

  } else if (sec == table->head) {
    sec = 0;
    return *this;

  } else {

    do {
      sec = static_cast<RenderTableSection *>(sec->previousSibling());
    } while (sec && (!sec->isTableSection() || sec == table->head || sec == table->foot));
    if (sec) return *this;

  }/*end if*/

  sec = table->foot;
  return *this;
}

#undef TABLE_DEBUG
#undef DEBUG_LAYOUT
#undef BOX_DEBUG
