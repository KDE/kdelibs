/**
 * This file is part of the html renderer for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 1999-2003 Antti Koivisto (koivisto@kde.org)
 *           (C) 2002-2003 Dirk Mueller (mueller@kde.org)
 *           (C) 2002 Apple Computer, Inc.
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
// -------------------------------------------------------------------------
//#define DEBUG
//#define DEBUG_LAYOUT
//#define BOX_DEBUG
//#define FLOAT_DEBUG

#include <kdebug.h>
#include <assert.h>
#include <qpainter.h>
#include <kglobal.h>
#include <limits.h>

#include "rendering/render_flow.h"
#include "rendering/render_text.h"
#include "rendering/render_table.h"
#include "rendering/render_root.h"
#include "rendering/render_layer.h"

#include "xml/dom_nodeimpl.h"
#include "khtmlview.h"
using namespace DOM;
using namespace khtml;

#define TABLECELLMARGIN -0x4000


static inline int collapseMargins(int a, int b)
{
    if ( a == TABLECELLMARGIN || b == TABLECELLMARGIN ) return TABLECELLMARGIN;
    if(a >= 0 && b >= 0) return (a > b ? a : b );
    if(a > 0 && b < 0) return a + b;
    if(a < 0 && b > 0) return b + a;
    return ( a > b ? b : a);
}

RenderFlow::RenderFlow(DOM::NodeImpl* node)
    : RenderBox(node)
{
    m_childrenInline = true;
    m_pre = false;
    firstLine = false;
    m_clearStatus = CNONE;

    m_maxTopPosMargin = m_maxTopNegMargin = m_maxBottomPosMargin = m_maxBottomNegMargin = 0;
    m_topMarginQuirk = m_bottomMarginQuirk = false;

    floatingObjects = 0;
    m_overflowWidth = 0;
    m_overflowHeight = 0;

    m_continuation = 0;
}

void RenderFlow::setStyle(RenderStyle *_style)
{

//    kdDebug( 6040 ) << (void*)this<< " renderFlow(" << renderName() << ")::setstyle()" << endl;

    RenderBox::setStyle(_style);

    if(isPositioned())
        setInline(false);

    if (!isTableCell() &&
	(isPositioned() || isRelPositioned() || style()->overflow()==OHIDDEN) &&
	!m_layer)
        m_layer = new (renderArena()) RenderLayer(this);

    if(isFloating() || !style()->display() == INLINE)
        setInline(false);

    if (isInline() && !m_childrenInline)
        setInline(false);

    m_pre = (style()->whiteSpace() == PRE);
    // ### we could save this call when the change only affected
    // non inherited properties
    RenderObject*child = firstChild();
    RenderStyle* newStyle;
    if (!isInline() && child &&
        ( newStyle=style()->getPseudoStyle(RenderStyle::FIRST_LETTER) ) ) {
        child->setStyle(newStyle);
        child = child->nextSibling();
    }

    while(child != 0)
    {
        if(child->isAnonymousBox())
        {
            RenderStyle* newStyle = new RenderStyle();
            newStyle->inheritFrom(style());
            newStyle->setDisplay(BLOCK);
            child->setStyle(newStyle);
            child->setIsAnonymousBox(true);
        }
        child = child->nextSibling();
    }

    // Ensure that all of the split inlines pick up the new style. We
    // only do this if we're an inline, since we don't want to propagate
    // a block's style to the other inlines.
    // e.g., <font>foo <h4>goo</h4> moo</font>.  The <font> inlines before
    // and after the block share the same style, but the block doesn't
    // need to pass its style on to anyone else.
    if (isInline()) {
        RenderFlow* currCont = continuation();
        while (currCont) {
            if (currCont->isInline()) {
                RenderFlow* nextCont = currCont->continuation();
                currCont->setContinuation(0);
                currCont->setStyle(style());
                currCont->setContinuation(nextCont);
            }
            currCont = currCont->continuation();
        }
    }
}

RenderFlow::~RenderFlow()
{
    delete floatingObjects;
}

FindSelectionResult RenderFlow::checkSelectionPoint( int _x, int _y, int _tx, int _ty, DOM::NodeImpl*& node, int & offset )
{
    int lastOffset=0;
    int off = offset;
    DOM::NodeImpl* nod = node;
    DOM::NodeImpl* lastNode = 0;
    for (RenderObject *child = firstChild(); child; child=child->nextSibling()) {
        khtml::FindSelectionResult pos = child->checkSelectionPoint(_x, _y, _tx+xPos(), _ty+yPos(), nod, off);
        //kdDebug(6030) << this << " child->findSelectionNode returned result=" << pos << " nod=" << nod << " off=" << off << endl;
        switch(pos) {
        case SelectionPointBeforeInLine:
        case SelectionPointAfterInLine:
        case SelectionPointInside:
            //kdDebug(6030) << "RenderObject::checkSelectionPoint " << this << " returning SelectionPointInside offset=" << offset << endl;
            node = nod;
            offset = off;
            return SelectionPointInside;
        case SelectionPointBefore:
            //x,y is before this element -> stop here
            if ( lastNode ) {
                node = lastNode;
                offset = lastOffset;
                //kdDebug(6030) << "RenderObject::checkSelectionPoint " << this << " before this child "
                //              << node << "-> returning SelectionPointInside, offset=" << offset << endl;
                return SelectionPointInside;
            } else {
                node = nod;
                offset = off;
                //kdDebug(6030) << "RenderObject::checkSelectionPoint " << this << " before us -> returning SelectionPointBefore " << node << "/" << offset << endl;
                return SelectionPointBefore;
            }
            break;
        case SelectionPointAfter:
            //kdDebug(6030) << "RenderObject::checkSelectionPoint: selection after: " << nod << " offset: " << off << endl;
            lastNode = nod;
            lastOffset = off;
            // No "return" here, obviously. We must keep looking into the children.
            break;
        }
    }
    // If we are after the last child, return lastNode/lastOffset
    // But lastNode can be 0L if there is no child, for instance.
    if ( lastNode )
    {
        node = lastNode;
        offset = lastOffset;
    }
    //kdDebug(6030) << "fallback - SelectionPointAfter  node=" << node << " offset=" << offset << endl;
    return SelectionPointAfter;
}

void RenderFlow::paint(QPainter *p, int _x, int _y, int _w, int _h, int _tx, int _ty,
                                 RenderObject::PaintPhase paintPhase )
{

#ifdef DEBUG_LAYOUT
//     kdDebug( 6040 ) << renderName() << "(RenderFlow) " << this << " ::paint() x/y/w/h = ("  << xPos() << "/" << yPos() << "/" << width() << "/" << height()  << ")" << endl;
#endif

    if(!isInline()) {
        _tx += m_x;
        _ty += m_y;
    }

    // check if we need to do anything at all...
    if(!isInline() && !overhangingContents() && !isRelPositioned() && !isPositioned() ) {
        int h = m_height;
        if(floatingObjects && floatBottom() > h)
	    h = floatBottom();
        if((_ty > _y + _h) || (_ty + h < _y)) {
            //kdDebug( 6040 ) << "cut!" << endl;
            return;
        }
    }

    paintObject(p, _x, _y, _w, _h, _tx, _ty, paintPhase);
}

void RenderFlow::paintObject(QPainter *p, int _x, int _y, int _w, int _h,
			     int _tx, int _ty,  RenderObject::PaintPhase paintPhase)
{
    if(isRelPositioned())
        relativePositionOffset(_tx, _ty);

    // 1. paint background, borders etc
    if( paintPhase == BACKGROUND_PHASE &&
        shouldPaintBackgroundOrBorder() && !isInline() && style()->visibility() == VISIBLE )
        paintBoxDecorations(p, _x, _y, _w, _h, _tx, _ty);

    // 2. paint contents
    for ( RenderObject* child = firstChild(); child; child = child->nextSibling() )
        if(!child->layer() && !child->isFloating())
            child->paint(p, _x, _y, _w, _h, _tx, _ty, paintPhase);

    // 3. paint floats
    if (paintPhase == FLOAT_PHASE)
        paintFloats(p, _x, _y, _w, _h, _tx, _ty);

    if(paintPhase == BACKGROUND_PHASE &&
        !isInline() && !childrenInline() && style()->outlineWidth())
        paintOutline(p, _tx, _ty, width(), height(), style());

#ifdef BOX_DEBUG
    if ( style() && style()->visibility() == VISIBLE ) {
        if(isAnonymousBox())
            outlineBox(p, _tx, _ty, "green");
        if(isFloating())
	    outlineBox(p, _tx, _ty, "yellow");
        else
            outlineBox(p, _tx, _ty);
    }
#endif

}

void RenderFlow::paintFloats( QPainter *p, int _x, int _y, int _w, int _h, int _tx, int _ty)
{
    if (!floatingObjects)
        return;

    FloatingObject* r;
    QPtrListIterator<FloatingObject> it(*floatingObjects);
    for ( ; (r = it.current()); ++it) {
        // Only paint the object if our noPaint flag isn't set.
        if (!r->noPaint) {
            r->node->paint(p, _x, _y, _w, _h,
                           _tx + r->left - r->node->xPos() + r->node->marginLeft(),
                           _ty + r->startY - r->node->yPos() + r->node->marginTop(),
                           BACKGROUND_PHASE);
            r->node->paint(p, _x, _y, _w, _h,
                           _tx + r->left - r->node->xPos() + r->node->marginLeft(),
                           _ty + r->startY - r->node->yPos() + r->node->marginTop(),
                           FLOAT_PHASE);
            r->node->paint(p, _x, _y, _w, _h,
                           _tx + r->left - r->node->xPos() + r->node->marginLeft(),
                           _ty + r->startY - r->node->yPos() + r->node->marginTop(),
                           FOREGROUND_PHASE);
        }
#ifdef FLOAT_DEBUG
	p->save();
        p->setPen( QPen( Qt::magenta, 4) );
	p->setBrush( QPainter::NoBrush );
	qDebug("(%p): floating object at (%d/%d-%d/%d)", this, r->left, r->startY, r->width, r->endY - r->startY );
	p->drawRect( QRect( r->left+tx, r->startY+ty, r->width, r->endY - r->startY) );
	p->restore();
#endif
    }
}


void RenderFlow::layout()
{
//    kdDebug( 6040 ) << renderName() << " " << this << "::layout() start" << endl;
//     QTime t;
//     t.start();

    KHTMLAssert( !layouted() );
    KHTMLAssert( minMaxKnown() );
    KHTMLAssert(!isInline());

    int oldWidth = m_width;

    calcWidth();

    m_overflowWidth = m_width;

    bool relayoutChildren = false;
    if ( oldWidth != m_width )
	relayoutChildren = true;

    // need a small hack here, as tables are done a bit differently
    if ( isTableCell() ) //&& static_cast<RenderTableCell *>(this)->widthChanged() )
	relayoutChildren = true;

//     kdDebug( 6040 ) << floatingObjects << "," << oldWidth << ","
//                     << m_width << ","<< layouted() << "," << isAnonymousBox() << ","
//                     << overhangingContents()<< endl;

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(RenderFlow) " << this << " ::layout() width=" << m_width << ", layouted=" << layouted() << endl;
    if(containingBlock() == static_cast<RenderObject *>(this))
        kdDebug( 6040 ) << renderName() << ": containingBlock == this" << endl;
#endif

    // This is an incorrect optimization.  You cannot know at this
    // point whether or not a child will overhang in the horizontal
    // direction without laying out your children.  The following test
    // case illustrates this point, as it will fail with this code
    // commented back in.
    //
    // <html>
    // <body style="width:0px">
    // Hello world!
    // </body>
    // </html>
    //
    // In the real world, this affects (as of 7/24/2002) http://viamichelin.com/. -- dwh
    //
    /*   if(m_width<=0 && !isPositioned() && !overhangingContents()) {
        if(m_y < 0) m_y = 0;
        setLayouted();
        return;
    }
    */

    clearFloats();

    m_overflowHeight = m_height = 0;
    m_clearStatus = CNONE;

//    kdDebug( 6040 ) << "childrenInline()=" << childrenInline() << endl;
    if(childrenInline())
        layoutInlineChildren( relayoutChildren );
    else
        layoutBlockChildren( relayoutChildren );

    int oldHeight = m_height;
    calcHeight();
    if ( oldHeight != m_height ) {
	relayoutChildren = true;
        if ( m_overflowHeight > m_height )
            m_overflowHeight -= borderBottom() + paddingBottom();
        if ( m_overflowHeight < m_height )
            m_overflowHeight = m_height;
    }

    // table cells have never overflowing contents
    if ( isTableCell() && lastChild() ) {
        if ( lastChild()->hasOverhangingFloats() )
            m_height = lastChild()->yPos() + static_cast<RenderFlow*>(lastChild())->floatBottom();
        if ( m_overflowHeight > m_height )
            m_height = m_overflowHeight;

	m_height += borderBottom() + paddingBottom();
    }

    if( hasOverhangingFloats() && (isFloating() || isTableCell()) ) {
	m_height = floatBottom();
	m_height += borderBottom() + paddingBottom();
    }

    if (m_layer)
        layoutPositionedObjects( relayoutChildren );

    if ( style()->overflow() == OHIDDEN ) {
        // ok, we don't have any overflow..
        m_overflowHeight = m_height;
        m_overflowWidth = m_width;
    }
    else {
        m_overflowWidth = kMax(m_overflowWidth, m_width);
        m_overflowHeight = kMax(m_overflowHeight, m_height);
    }

    //kdDebug() << renderName() << " layout width=" << m_width << " height=" << m_height << endl;

    setLayouted();
}

void RenderFlow::layoutPositionedObjects( bool relayoutChildren )
{
    if (!m_layer)
        return;

    for (RenderLayer* l=m_layer->firstChild(); l; l=l->nextSibling()) {
        if (l->renderer()->isPositioned())
        {
            if (relayoutChildren ) {
                l->renderer()->setLayoutedLocal( false );
            }
            if ( !l->renderer()->layouted() )
                l->renderer()->layout();
        }
    }
}

void RenderFlow::layoutBlockChildren( bool relayoutChildren )
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << " layoutBlockChildren( " << this <<" ), relayoutChildren="<< relayoutChildren << endl;
#endif

    int xPos = 0;
    int toAdd = 0;

    m_height = 0;

    xPos += borderLeft() + paddingLeft();
    m_height += borderTop() + paddingTop();
    toAdd += borderBottom() + paddingBottom();
    m_overflowHeight = m_height;

    if( style()->direction() == RTL ) {
        xPos = marginLeft() + m_width - paddingRight() - borderRight();
    }

    RenderObject *child = firstChild();
    RenderFlow *prevFlow = 0;

    int prevMargin = 0;
    if(isTableCell() ) {
	prevMargin = TABLECELLMARGIN;
    } else if ( m_height == 0 ) {
	// the elements and childs margin collapse if there is no border and padding.
	prevMargin = marginTop();
	if ( parent() )
	    prevMargin = collapseMargins( prevMargin, parent()->marginTop() );
	if ( prevMargin != TABLECELLMARGIN )
	    m_height = -prevMargin;
    }
    //kdDebug() << "RenderFlow::layoutBlockChildren " << prevMargin << endl;

//     QTime t;
//     t.start();

    while( child != 0 )
    {

        // make sure we relayout children if we need it,
        // like inherited floats or percentage based widths.
        if ( relayoutChildren || floatBottom() > m_height ||
             ( ( child->isReplaced() || child->isFloating() ) &&
	       ( child->style()->width().isPercent() || child->style()->height().isPercent() ) )
	    )
	    child->setLayouted(false);

	if ( child->style()->flowAroundFloats() && !child->isFloating() &&
	     style()->width().isFixed() ) {
	    // flow around floats only flows around the ones on the left side (right side if direction==RTL)
	    int available = style()->direction() == LTR
			    ? m_width - leftOffset( m_height )
			    : rightOffset( m_height );
	    if ( child->minWidth() > available ) {
		// ## this might be a little to far.
		m_height = QMAX( m_height, style()->direction() == LTR ? leftBottom() : rightBottom() );
		prevMargin = 0;
	    }
	}

//         kdDebug( 6040 ) << "   " << child->renderName() << " loop " << child << ", " << child->isInline() << ", " << child->layouted() <<" float="<< child->isFloating() << " y=" << m_height << endl;
//         kdDebug( 6040 ) << t.elapsed() << endl;
        // ### might be some layouts are done two times... FIX that.

        if (child->isPositioned()) {
            child = child->nextSibling();
            continue;
        } else if ( child->isReplaced() ) {
            if ( !child->layouted() )
                child->layout();
        } else if ( child->isFloating() ) {
            if ( !child->layouted() )
                child->layout();
	    // margins of floats and other objects do not collapse. The hack below assures this.
	    if ( prevMargin != TABLECELLMARGIN )
		m_height += prevMargin;
	    insertFloatingObject( child );
	    positionNewFloats();
// 	    kdDebug() << "RenderFlow::layoutBlockChildren inserting float at "<< m_height <<" prevMargin="<<prevMargin << endl;
	    if ( prevMargin != TABLECELLMARGIN )
		m_height -= prevMargin;
	    child = child->nextSibling();
	    continue;
	}

        child->calcVerticalMargins();

        if(checkClear(child)) prevMargin = 0; // ### should only be 0
        // if oldHeight+prevMargin < newHeight

        int margin = child->marginTop();
        //kdDebug(0) << "margin = " << margin << " prevMargin = " << prevMargin << endl;
        margin = collapseMargins(margin, prevMargin);

	if ( margin != TABLECELLMARGIN )
	    m_height += margin;

        //kdDebug(0) << "margin = " << margin << " yPos = " << m_height << endl;

        if(prevFlow)
        {
            if (prevFlow->yPos()+prevFlow->floatBottom() > m_height)
                child->setLayouted(false);
            else
                prevFlow=0;
        }

	// #### ugly and hacky, as we calculate width twice, but works for now.
	// really need to fix this after 3.1
	int owidth = child->width();
	child->calcWidth();
        int chPos = xPos;

        if(style()->direction() == LTR) {
            // html blocks flow around floats
            if ( ( style()->htmlHacks() || child->isTable() ) && child->style()->flowAroundFloats() )
		chPos = leftOffset(m_height);
	    chPos += child->marginLeft();
        } else {
            if ( ( style()->htmlHacks() || child->isTable() ) && child->style()->flowAroundFloats() )
                chPos = rightOffset(m_height);
            chPos -= child->width() + child->marginRight();
        }
	child->setWidth( owidth );
        child->setPos(chPos, m_height);

	if ( !child->layouted() )
	    child->layout();

        m_height += child->height();
        if ( m_overflowHeight < m_height )
            m_overflowHeight = kMax(m_overflowHeight, m_height + child->overflowHeight() - child->height());

        prevMargin = child->marginBottom();

        if (child->isFlow())
            prevFlow = static_cast<RenderFlow*>(child);

	if ( child->hasOverhangingFloats() ) {
	    // need to add the float to our special objects
	    addOverHangingFloats( static_cast<RenderFlow *>(child), -child->xPos(), -child->yPos(), true );
	}

        m_overflowWidth = kMax(int(m_overflowWidth), child->overflowWidth() + child->xPos());

        child = child->nextSibling();
    }

    if( !isTableCell() && toAdd != 0 )
	m_height += prevMargin;
    if ( isPositioned() || isRelPositioned() )
	m_height = QMAX( m_height,  floatBottom() );

    m_height += toAdd;
    m_overflowHeight = kMax(m_overflowHeight, m_height);

    setLayouted();

    // kdDebug( 6040 ) << "layouted = " << layouted_ << endl;
}

bool RenderFlow::checkClear(RenderObject *child)
{
    //kdDebug( 6040 ) << "checkClear oldheight=" << m_height << endl;
    int bottom = 0;
    switch(child->style()->clear())
    {
    case CNONE:
        return false;
    case CLEFT:
        bottom = leftBottom();
        break;
    case CRIGHT:
        bottom = rightBottom();
        break;
    case CBOTH:
        bottom = floatBottom();
	break;
    }
    if(m_height < bottom)
	m_height = bottom;
    return true;
}

void RenderFlow::insertFloatingObject(RenderObject *o)
{
    KHTMLAssert(o->isFloating());

    // Create the list of floating objects if we don't aleady have one
    if (!floatingObjects) {
	floatingObjects = new QPtrList<FloatingObject>;
	floatingObjects->setAutoDelete(true);
    }
    else {
	// Don't insert the object again if it's already in the list
	QPtrListIterator<FloatingObject> it(*floatingObjects);
	FloatingObject *f;
	while ( (f = it.current()) ) {
	    if (f->node == o) return;
	    ++it;
	}
    }

    // Create the floating object entry & append it to the list

    FloatingObject *newObj = 0;

    // floating object
    if ( !o->layouted() )
	o->layout();

    newObj = new FloatingObject( (o->style()->floating() == FLEFT) ?
				FloatingObject::FloatLeft : FloatingObject::FloatRight );

    newObj->startY = -1;
    newObj->endY = -1;
    newObj->width = o->width() + o->marginLeft() + o->marginRight();
    newObj->node = o;

    floatingObjects->append(newObj);
}

void RenderFlow::removeFloatingObject(RenderObject *o)
{
    if (floatingObjects) {
        QPtrListIterator<FloatingObject> it(*floatingObjects);
	while (it.current()) {
	    if (it.current()->node == o)
		floatingObjects->removeRef(it.current());
	    ++it;
	}
    }
}

void RenderFlow::positionNewFloats()
{
    if(!floatingObjects) return;
    FloatingObject *f = floatingObjects->getLast();

    if(!f || f->startY != -1) return;
    FloatingObject *lastFloat;
    while(1) {
        lastFloat = floatingObjects->prev();
        if(!lastFloat || lastFloat->startY != -1) {
            floatingObjects->next();
            break;
        }
        f = lastFloat;
    }

    int y = m_height;

    // the float can not start above the y position of the last positioned float.
    if(lastFloat && lastFloat->startY > y)
        y = lastFloat->startY;

    while(f) {
        //skip elements copied from elsewhere and positioned elements
        if (f->node->containingBlock()!=this) {
            f = floatingObjects->next();
            continue;
        }

        RenderObject *o = f->node;
        int _height = o->height() + o->marginTop() + o->marginBottom();

        int ro = rightOffset(); // Constant part of right offset.
        int lo = leftOffset(); // Constat part of left offset.
        int fwidth = f->width; // The width we look for.
	//kdDebug( 6040 ) << " Object width: " << fwidth << " available width: " << ro - lo << endl;
        if (ro - lo < fwidth)
            fwidth = ro - lo; // Never look for more than what will be available.
        if (o->style()->floating() == FLEFT) {
	    if ( o->style()->clear() & CLEFT )
		y = QMAX( leftBottom(), y );
	    int heightRemainingLeft = 1;
	    int heightRemainingRight = 1;
            int fx = leftRelOffset(y,lo, &heightRemainingLeft);
            while (rightRelOffset(y,ro, &heightRemainingRight)-fx < fwidth) {
                y += QMIN( heightRemainingLeft, heightRemainingRight );
                fx = leftRelOffset(y,lo, &heightRemainingLeft);
            }
            if (fx<0) fx=0;
            f->left = fx;
            //kdDebug( 6040 ) << "positioning left aligned float at (" << fx + o->marginLeft()  << "/" << y + o->marginTop() << ") fx=" << fx << endl;
            o->setPos(fx + o->marginLeft(), y + o->marginTop());
        } else {
	    if ( o->style()->clear() & CRIGHT )
		y = QMAX( rightBottom(), y );
	    int heightRemainingLeft = 1;
	    int heightRemainingRight = 1;
            int fx = rightRelOffset(y,ro, &heightRemainingRight);
            while (fx - leftRelOffset(y,lo, &heightRemainingLeft) < fwidth) {
                y += QMIN(heightRemainingLeft, heightRemainingRight);
                fx = rightRelOffset(y,ro, &heightRemainingRight);
            }
            if (fx<f->width) fx=f->width;
            f->left = fx - f->width;
            //kdDebug( 6040 ) << "positioning right aligned float at (" << fx - o->marginRight() - o->width() << "/" << y + o->marginTop() << ")" << endl;
            o->setPos(fx - o->marginRight() - o->width(), y + o->marginTop());
        }
	f->startY = y;
        f->endY = f->startY + _height;


	//kdDebug( 6040 ) << "floatingObject x/y= (" << f->left << "/" << f->startY << "-" << f->width << "/" << f->endY - f->startY << ")" << endl;

        f = floatingObjects->next();
    }
}

void RenderFlow::newLine()
{
    positionNewFloats();
    // set y position
    int newY = 0;
    switch(m_clearStatus)
    {
    case CLEFT:
        newY = leftBottom();
        break;
    case CRIGHT:
        newY = rightBottom();
        break;
    case CBOTH:
        newY = floatBottom();
    default:
        break;
    }
    if(m_height < newY)
    {
//      kdDebug( 6040 ) << "adjusting y position" << endl;
        m_height = newY;
    }
    m_clearStatus = CNONE;
}


int
RenderFlow::leftOffset() const
{
    int left = 0;

    left += borderLeft() + paddingLeft();

    if ( firstLine && style()->direction() == LTR ) {
        int cw=0;
        if (style()->textIndent().isPercent())
            cw = containingBlock()->contentWidth();
        left += style()->textIndent().minWidth(cw);
    }

    return left;
}

int
RenderFlow::leftRelOffset(int y, int fixedOffset, int *heightRemaining ) const
{
    int left = fixedOffset;
    if(!floatingObjects)
	return left;

    if ( heightRemaining ) *heightRemaining = 1;
    FloatingObject* r;
    QPtrListIterator<FloatingObject> it(*floatingObjects);
    for ( ; (r = it.current()); ++it )
    {
	//kdDebug( 6040 ) <<(void *)this << " left: sy, ey, x, w " << r->startY << "," << r->endY << "," << r->left << "," << r->width << " " << endl;
        if (r->startY <= y && r->endY > y &&
            r->type == FloatingObject::FloatLeft &&
            r->left + r->width > left) {
	    left = r->left + r->width;
	    if ( heightRemaining ) *heightRemaining = r->endY - y;
	}
    }
    //kdDebug( 6040 ) << "leftOffset(" << y << ") = " << left << endl;
    return left;
}

int
RenderFlow::rightOffset() const
{
    int right = m_width;

    right -= borderRight() + paddingRight();

    if ( firstLine && style()->direction() == RTL ) {
        int cw=0;
        if (style()->textIndent().isPercent())
            cw = containingBlock()->contentWidth();
        right += style()->textIndent().minWidth(cw);
    }

    return right;
}

int
RenderFlow::rightRelOffset(int y, int fixedOffset, int *heightRemaining ) const
{
    int right = fixedOffset;

    if (!floatingObjects) return right;

    if (heightRemaining) *heightRemaining = 1;
    FloatingObject* r;
    QPtrListIterator<FloatingObject> it(*floatingObjects);
    for ( ; (r = it.current()); ++it )
    {
	//kdDebug( 6040 ) << "right: sy, ey, x, w " << r->startY << "," << r->endY << "," << r->left << "," << r->width << " " << endl;
        if (r->startY <= y && r->endY > y &&
            r->type == FloatingObject::FloatRight &&
            r->left < right) {
	    right = r->left;
	    if ( heightRemaining ) *heightRemaining = r->endY - y;
	}
    }
    //kdDebug( 6040 ) << "rightOffset(" << y << ") = " << right << endl;
    return right;
}

short
RenderFlow::lineWidth(int y) const
{
    //kdDebug( 6040 ) << "lineWidth(" << y << ")=" << rightOffset(y) - leftOffset(y) << endl;
    return rightOffset(y) - leftOffset(y);
}

int
RenderFlow::floatBottom() const
{
    if (!floatingObjects) return 0;
    int bottom=0;
    FloatingObject* r;
    QPtrListIterator<FloatingObject> it(*floatingObjects);
    for ( ; (r = it.current()); ++it )
        if (r->endY>bottom)
            bottom=r->endY;
    return bottom;
}

int
RenderFlow::lowestPosition() const
{
    int bottom = RenderBox::lowestPosition();
    //kdDebug(0) << renderName() << "("<<this<<") lowest = " << bottom << endl;
    int lp = 0;
    if ( !m_childrenInline ) {
        RenderObject *last = lastChild();
        while( last && (last->isPositioned() || last->isFloating()) )
            last = last->previousSibling();
        if( last )
            lp = last->yPos() + last->lowestPosition();
    }

    if(  lp > bottom )
        bottom = lp;

    //kdDebug(0) << "     bottom = " << bottom << endl;

    if (floatingObjects) {
        FloatingObject* r;
        QPtrListIterator<FloatingObject> it(*floatingObjects);
        for ( ; (r = it.current()); ++it ) {
            lp = r->startY + r->node->lowestPosition();
            //kdDebug(0) << r->node->renderName() << " lp = " << lp << "startY=" << r->startY << endl;
            if( lp > bottom)
                bottom = lp;
        }
    }

    if ( overhangingContents() ) {
        RenderObject *child = firstChild();
        while( child ) {
	    if ( child->overhangingContents() ) {
		int lp = child->yPos() + child->lowestPosition();
		if ( lp > bottom ) bottom = lp;
	    }
	    child = child->nextSibling();
	}
    }

    //kdDebug(0) << renderName() << "      bottom final = " << bottom << endl;
    return bottom;
}

int RenderFlow::rightmostPosition() const
{
    int right = RenderBox::rightmostPosition();

    RenderObject *c;
    for (c = firstChild(); c; c = c->nextSibling()) {
        if (!c->isPositioned() && !c->isFloating()) {
            int childRight = c->xPos() + c->rightmostPosition();
            if (childRight > right)
                right = childRight;
        }
    }

    if (floatingObjects) {
        FloatingObject* r;
        QPtrListIterator<FloatingObject> it(*floatingObjects);
        for ( ; (r = it.current()); ++it ) {
            int fRight = r->left + r->node->rightmostPosition();
            if (fRight > right)
                right = fRight;
        }
    }

    if ( overhangingContents() ) {
        RenderObject *child = firstChild();
        while( child ) {
	    if ( child->overhangingContents() ) {
		int r = child->xPos() + child->rightmostPosition();
		if ( r > right ) right = r;
	    }
	    child = child->nextSibling();
	}
    }

    return right;
}


int
RenderFlow::leftBottom()
{
    if (!floatingObjects) return 0;
    int bottom=0;
    FloatingObject* r;
    QPtrListIterator<FloatingObject> it(*floatingObjects);
    for ( ; (r = it.current()); ++it )
        if (r->endY>bottom && r->type == FloatingObject::FloatLeft)
            bottom=r->endY;

    return bottom;
}

int
RenderFlow::rightBottom()
{
    if (!floatingObjects) return 0;
    int bottom=0;
    FloatingObject* r;
    QPtrListIterator<FloatingObject> it(*floatingObjects);
    for ( ; (r = it.current()); ++it )
        if (r->endY>bottom && r->type == FloatingObject::FloatRight)
            bottom=r->endY;

    return bottom;
}

void
RenderFlow::clearFloats()
{
    //kdDebug( 6040 ) << this <<" clearFloats" << endl;

    if (floatingObjects)
        floatingObjects->clear();

    if (isFloating() || isPositioned()) return;

    RenderObject *prev = previousSibling();

    // find the element to copy the floats from
    // pass non-flows
    // pass fAF's unless they contain overhanging stuff
    bool parentHasFloats = false;
    while (prev) {
	if (!prev->isFlow() || prev->isFloating() ||
	    (prev->style()->flowAroundFloats() &&
	     (static_cast<RenderFlow *>(prev)->floatBottom()+prev->yPos() < m_y ))) {
	    if ( prev->isFloating() && parent()->isFlow() ) {
		parentHasFloats = true;
	    }
	    prev = prev->previousSibling();
	} else
	    break;
    }

    int offset = m_y;

    if ( parentHasFloats )
	addOverHangingFloats( static_cast<RenderFlow *>( parent() ),
			      parent()->borderLeft() + parent()->paddingLeft() , offset, false );

    if(prev ) {
        if(prev->isTableCell()) return;

        offset -= prev->yPos();
    } else {
        prev = parent();
	if(!prev) return;
    }
    //kdDebug() << "RenderFlow::clearFloats found previous "<< (void *)this << " prev=" << (void *)prev<< endl;

    // add overhanging floating objects from the previous RenderFlow
    if(!prev->isFlow()) return;
    RenderFlow * flow = static_cast<RenderFlow *>(prev);
    if(!flow->floatingObjects) return;
    if( ( style()->htmlHacks() || isTable() ) && style()->flowAroundFloats())
        return; //html tables and lists flow as blocks

    if(flow->floatBottom() > offset) {
	int xoff = flow == parent() ? xPos() : xPos() - flow->xPos();
	addOverHangingFloats( flow, xoff, offset );
    }
}

void RenderFlow::addOverHangingFloats( RenderFlow *flow, int xoff, int offset, bool child )
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << (void *)this << ": adding overhanging floats xoff=" << xoff << "  offset=" << offset << " child=" << child << endl;
#endif
    if ( !flow->floatingObjects )
        return;

    // we have overhanging floats
    if(!floatingObjects) {
	floatingObjects = new QPtrList<FloatingObject>;
	floatingObjects->setAutoDelete(true);
    }

    QPtrListIterator<FloatingObject> it(*flow->floatingObjects);
    FloatingObject *r;
    for ( ; (r = it.current()); ++it ) {
	if ( ( !child && r->endY > offset ) ||
	       ( child && flow->yPos() + r->endY > height() ) ) {

	    if ( child )
		r->noPaint = true;

	    FloatingObject* f = 0;
	    // don't insert it twice!
	    QPtrListIterator<FloatingObject> it(*floatingObjects);
	    while ( (f = it.current()) ) {
		if (f->node == r->node) break;
		++it;
	    }
	    if ( !f ) {
		FloatingObject *nf = new FloatingObject(r->type);
		nf->startY = r->startY - offset;
		nf->endY = r->endY - offset;
		nf->left = r->left - xoff;
		if ( !child ) {
		    nf->left -= marginLeft();
		    nf->noPaint = true;
		}
		nf->width = r->width;
		nf->node = r->node;
		floatingObjects->append(nf);
#ifdef DEBUG_LAYOUT
	kdDebug( 6040 ) << "addOverHangingFloats x/y= (" << nf->left << "/" << nf->startY << "-" << nf->width << "/" << nf->endY - nf->startY << ")" << endl;
#endif
	    }
	}
    }
}


static inline RenderObject *next(RenderObject *par, RenderObject *current)
{
    RenderObject *next = 0;
    while(current != 0)
    {
        //kdDebug( 6040 ) << "current = " << current << endl;
	if(!current->isFloating() && !current->isReplaced() && !current->isPositioned())
	    next = current->firstChild();
	if(!next) {
	    while(current && current != par) {
		next = current->nextSibling();
		if(next) break;
		current = current->parent();
	    }
	}

        if(!next) break;

        if(next->isText() || next->isBR() || next->isFloating() || next->isReplaced() || next->isPositioned())
            break;
        current = next;
    }
    return next;
}


void RenderFlow::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(RenderBox)::calcMinMaxWidth() this=" << this << endl;
#endif

    m_minWidth = 0;
    m_maxWidth = 0;

    if (isInline()) {
	setMinMaxKnown();
        return;
    }

    int cw = containingBlock()->contentWidth();
    // "style()->width().isVariable()" in the contition below gives more mozilla like behaviour, the current
    // condition is more IE like.
    bool tableCell = (isTableCell() && !style()->width().isFixed());

    // ## maybe we should replace the noWrap stuff in RenderTable by CSS.
    bool nowrap = style()->whiteSpace() == NOWRAP;

    // non breaking space
    const QChar nbsp = 0xa0;

    RenderObject *child = firstChild();
    RenderObject *prevchild = 0;

    if(childrenInline()) {
        int inlineMax=0;
        int currentMin=0;
        int inlineMin=0;
        bool noBreak=false;

        while(child != 0) {
            // positioned children don't affect the minmaxwidth
            if (child->isPositioned()) {
                child = next(this, child);
                continue;
            }

            if( !child->isBR() ) {
                RenderStyle* cstyle = child->style();
                int margins = 0;
		LengthType type = cstyle->marginLeft().type();
                if ( type != Variable )
                    margins += (type == Fixed ? cstyle->marginLeft().value() : child->marginLeft());
		type = cstyle->marginRight().type();
                if ( type != Variable )
                    margins += (type == Fixed ? cstyle->marginRight().value() : child->marginRight());
                int childMin = child->minWidth() + margins;
                int childMax = child->maxWidth() + margins;
// 		qDebug("child min=%d, max=%d, currentMin=%d, inlineMin=%d",  childMin,  childMax, currentMin, inlineMin );
                if (child->isText() && static_cast<RenderText *>(child)->stringLength() > 0) {

                    int ti = cstyle->textIndent().minWidth(cw);
                    childMin+=ti;
                    childMax+=ti;

                    RenderText* t = static_cast<RenderText *>(child);
// 		    qDebug("child is text, startMin=%d, endMin=%d", t->startMin(),  t->endMin() );
                    if (noBreak || t->text()[0] == nbsp) { //inline starts with nbsp
                        currentMin += t->startMin();
// 			qDebug("added startMin to currentMin, currentMin=%d", currentMin );
                    }
		    noBreak = false;
		    if ( t->hasBreakableChar() ) {
			inlineMin = QMAX( inlineMin, currentMin );
			currentMin = t->endMin();
		    }
                    if ( t->text()[t->stringLength()-1]==nbsp ) { //inline ends with nbsp
			noBreak = true;
                    }
// 		    qDebug("noBreak = %d, currentMin = %d", noBreak,  currentMin );
                } else if (noBreak || tableCell ||
		    (prevchild && prevchild->isFloating() && child->isFloating())) {
                    currentMin += childMin;
                    noBreak = false;
                }
		inlineMin = QMAX( inlineMin, childMin );
		inlineMax += childMax;
            }
            else
            {
		inlineMin = QMAX( currentMin, inlineMin );
                if(m_minWidth < inlineMin) m_minWidth = inlineMin;
                if(m_maxWidth < inlineMax) m_maxWidth = inlineMax;
                inlineMin = currentMin = inlineMax = 0;
            }
            prevchild = child;
            child = next(this, child);
        }
	inlineMin = QMAX( currentMin, inlineMin );
        if(m_minWidth < inlineMin) m_minWidth = inlineMin;
        if(m_maxWidth < inlineMax) m_maxWidth = inlineMax;
//          kdDebug( 6040 ) << "m_minWidth=" << m_minWidth
//  			<< " m_maxWidth=" << m_maxWidth << endl;
	if ( nowrap )
	    m_minWidth = m_maxWidth;
    }
    else
    {
        while(child != 0)
        {
            // positioned children don't affect the minmaxwidth
            if (child->isPositioned())
            {
                child = child->nextSibling();
                continue;
            }

            int margin=0;
            //  auto margins don't affect minwidth

            Length ml = child->style()->marginLeft();
            Length mr = child->style()->marginRight();

            if (style()->textAlign() == KONQ_CENTER)
            {
                if (ml.isFixed()) margin+=ml.value();
                if (mr.isFixed()) margin+=mr.value();
            }
            else
            {
                if (!ml.isVariable() && !mr.isVariable())
                {
                    if (!(child->style()->width().isVariable()))
                    {
                        if (child->style()->direction()==LTR)
                            margin = child->marginLeft();
                        else
                            margin = child->marginRight();
                    }
                    else
                        margin = child->marginLeft()+child->marginRight();

                }
                else if (!ml.isVariable())
                    margin = child->marginLeft();
                else if (!mr.isVariable())
                    margin = child->marginRight();
            }

            if (margin<0) margin=0;

            int w = child->minWidth() + margin;
            if(m_minWidth < w) m_minWidth = w;
	    // IE ignores tables for calculation of nowrap. Makes some sense.
	    if ( nowrap && !child->isTable() && m_maxWidth < w )
		m_maxWidth = w;
            w = child->maxWidth() + margin;
            if(m_maxWidth < w) m_maxWidth = w;
            child = child->nextSibling();
        }
    }
    if(m_maxWidth < m_minWidth) m_maxWidth = m_minWidth;

    if (style()->width().isFixed() && style()->width().value() > 0)
        m_maxWidth = KMAX(m_minWidth,short(style()->width().value()));

    int toAdd = 0;
    toAdd = borderLeft() + borderRight() + paddingLeft() + paddingRight();

    m_minWidth += toAdd;
    m_maxWidth += toAdd;

    setMinMaxKnown();

    //kdDebug( 6040 ) << "Text::calcMinMaxWidth(" << this << "): min = " << m_minWidth << " max = " << m_maxWidth << endl;
    // ### compare with min/max width set in style sheet...
}

void RenderFlow::close()
{
    if(lastChild() && lastChild()->isAnonymousBox()) {
        lastChild()->close();
    }

    RenderBox::close();
}

RenderFlow* RenderFlow::continuationBefore(RenderObject* beforeChild)
{
    if (beforeChild && beforeChild->parent() == this)
        return this;

    RenderFlow* curr = continuation();
    RenderFlow* nextToLast = this;
    RenderFlow* last = this;
    while (curr) {
        if (beforeChild && beforeChild->parent() == curr) {
            if (curr->firstChild() == beforeChild)
                return last;
            return curr;
        }

        nextToLast = last;
        last = curr;
        curr = curr->continuation();
    }

    if (!beforeChild && !last->firstChild())
        return nextToLast;
    return last;
}

static RenderFlow* cloneInline(RenderFlow* src)
{
    RenderFlow *o = new (src->renderArena()) RenderFlow(src->element());
    o->setStyle(src->style());
    return o;
}

void RenderFlow::splitInlines(RenderFlow* fromBlock, RenderFlow* toBlock,
                              RenderFlow* middleBlock,
                              RenderObject* beforeChild, RenderFlow* oldCont)
{
    // Create a clone of this inline.
    RenderFlow* clone = cloneInline(this);
    clone->setContinuation(oldCont);

    // Now take all of the children from beforeChild to the end and remove
    // then from |this| and place them in the clone.
    RenderObject* o = beforeChild;
    while (o) {
        RenderObject* tmp = o;
        o = tmp->nextSibling();
        clone->appendChildNode(removeChildNode(tmp));
        tmp->setLayouted(false);
        tmp->setMinMaxKnown(false);
    }

    // Hook |clone| up as the continuation of the middle block.
    middleBlock->setContinuation(clone);

    // We have been reparented and are now under the fromBlock.  We need
    // to walk up our inline parent chain until we hit the containing block.
    // Once we hit the containing block we're done.
    RenderFlow* curr = static_cast<RenderFlow*>(parent());
    RenderFlow* currChild = this;
    while (curr && curr != fromBlock) {
        // Create a new clone.
        RenderFlow* cloneChild = clone;
        clone = cloneInline(curr);

        // Insert our child clone as the first child.
        clone->appendChildNode(cloneChild);

        // Hook the clone up as a continuation of |curr|.
        RenderFlow* oldCont = curr->continuation();
        curr->setContinuation(clone);
        clone->setContinuation(oldCont);

        // Now we need to take all of the children starting from the first child
        // *after* currChild and append them all to the clone.
        o = currChild->nextSibling();
        while (o) {
            RenderObject* tmp = o;
            o = tmp->nextSibling();
            clone->appendChildNode(curr->removeChildNode(tmp));
            tmp->setLayouted(false);
            tmp->setMinMaxKnown(false);
        }

        // Keep walking up the chain.
        currChild = curr;
        curr = static_cast<RenderFlow*>(curr->parent());
    }

    // Now we are at the block level. We need to put the clone into the toBlock.
    toBlock->appendChildNode(clone);

    // Now take all the children after currChild and remove them from the fromBlock
    // and put them in the toBlock.
    o = currChild->nextSibling();
    while (o) {
        RenderObject* tmp = o;
        o = tmp->nextSibling();
        toBlock->appendChildNode(fromBlock->removeChildNode(tmp));
    }
}

void RenderFlow::splitFlow(RenderObject* beforeChild, RenderFlow* newBlockBox,
                           RenderObject* newChild, RenderFlow* oldCont)
{
    RenderObject* block = containingBlock();
    RenderFlow* pre = 0;
    RenderFlow* post = 0;

    RenderStyle* newStyle = new RenderStyle();
    newStyle->inheritFrom(block->style());
    newStyle->setDisplay(BLOCK);
    pre = new (renderArena()) RenderFlow(0 /* anonymous box */);
    pre->setStyle(newStyle);
    pre->setIsAnonymousBox(true);
    pre->setChildrenInline(true);

    newStyle = new RenderStyle();
    newStyle->inheritFrom(block->style());
    newStyle->setDisplay(BLOCK);
    post = new (renderArena()) RenderFlow(0 /* anonymous box */);
    post->setStyle(newStyle);
    post->setIsAnonymousBox(true);
    post->setChildrenInline(true);

    RenderObject* boxFirst = block->firstChild();
    block->insertChildNode(pre, boxFirst);
    block->insertChildNode(newBlockBox, boxFirst);
    block->insertChildNode(post, boxFirst);
    if ( block->isFlow() )
	static_cast<RenderFlow *>(block)->setChildrenInline(false);

    RenderObject* o = boxFirst;
    while (o)
    {
        RenderObject* no = o;
        o = no->nextSibling();
        pre->appendChildNode(block->removeChildNode(no));
        no->setLayouted(false);
        no->setMinMaxKnown(false);
    }

    splitInlines(pre, post, newBlockBox, beforeChild, oldCont);

    // We already know the newBlockBox isn't going to contain inline kids, so avoid wasting
    // time in makeChildrenNonInline by just setting this explicitly up front.
    newBlockBox->setChildrenInline(false);

    // We don't just call addChild, since it would pass things off to the
    // continuation, so we call addChildToFlow explicitly instead.  We delayed
    // adding the newChild until now so that the |newBlockBox| would be fully
    // connected, thus allowing newChild access to a renderArena should it need
    // to wrap itself in additional boxes (e.g., table construction).
    newBlockBox->addChildToFlow(newChild, 0);

    // XXXdwh is any of this even necessary? I don't think it is.
    pre->close();
    pre->setPos(0, -500000);
    pre->setLayouted(false);
    newBlockBox->close();
    newBlockBox->setPos(0, -500000);
    newBlockBox->setLayouted(false);
    post->close();
    post->setPos(0, -500000);
    post->setLayouted(false);

    block->setLayouted(false);
    block->setMinMaxKnown(false);
}

void RenderFlow::addChildWithContinuation(RenderObject* newChild, RenderObject* beforeChild)
{
    RenderFlow* flow = continuationBefore(beforeChild);
    RenderFlow* beforeChildParent = beforeChild ? static_cast<RenderFlow*>(beforeChild->parent()) :
                                    (flow->continuation() ? flow->continuation() : flow);

    if (newChild->isSpecial())
        return beforeChildParent->addChildToFlow(newChild, beforeChild);

    // A continuation always consists of two potential candidates: an inline or an anonymous
    // block box holding block children.
    bool childInline = newChild->isInline();
    bool bcpInline = beforeChildParent->isInline();
    bool flowInline = flow->isInline();

    if (flow == beforeChildParent)
        return flow->addChildToFlow(newChild, beforeChild);
    else {
        // The goal here is to match up if we can, so that we can coalesce and create the
        // minimal # of continuations needed for the inline.
        if (childInline == bcpInline)
            return beforeChildParent->addChildToFlow(newChild, beforeChild);
        else if (flowInline == childInline)
            return flow->addChildToFlow(newChild, 0); // Just treat like an append.
        else
            return beforeChildParent->addChildToFlow(newChild, beforeChild);
    }
}

void RenderFlow::addChild(RenderObject *newChild, RenderObject *beforeChild)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(RenderFlow)::addChild( " << newChild->renderName() <<
                       ", " << (beforeChild ? beforeChild->renderName() : "0") << " )" << endl;
    kdDebug( 6040 ) << "current height = " << m_height << endl;
#endif

    if (continuation())
        return addChildWithContinuation(newChild, beforeChild);
    return addChildToFlow(newChild, beforeChild);
}

void RenderFlow::addChildToFlow(RenderObject* newChild, RenderObject* beforeChild)
{
    setLayouted( false );

    bool madeBoxesNonInline = FALSE;

    RenderStyle* pseudoStyle=0;
    if ( !isInline() && ( !firstChild() || firstChild() == beforeChild )
	&& ( pseudoStyle=style()->getPseudoStyle(RenderStyle::FIRST_LETTER) ) )
    {

        if (newChild->isText() && !newChild->isBR()) {
	    RenderText* newTextChild = static_cast<RenderText*>(newChild);

	    //kdDebug( 6040 ) << "first letter" << endl;

	    RenderFlow* firstLetter = new (renderArena()) RenderFlow(0 /* anonymous box */);
	    pseudoStyle->setDisplay( INLINE );
	    firstLetter->setStyle(pseudoStyle);
            firstLetter->setIsAnonymousBox(true);

	    addChild(firstLetter);

	    DOMStringImpl* oldText = newTextChild->string();

	    if(oldText->l >= 1) {
		unsigned int length = 0;
		while ( length < oldText->l &&
			( (oldText->s+length)->isSpace() || (oldText->s+length)->isPunct() ) )
		    length++;
		length++;
		RenderText* letter = new (renderArena()) RenderText(0 /* anonymous object */, oldText->substring(0,length));
		RenderStyle* newStyle = new RenderStyle();
		newStyle->inheritFrom(pseudoStyle);
		letter->setStyle(newStyle);
                letter->setIsAnonymousBox(true);
		firstLetter->addChild(letter);
		newTextChild->setText(oldText->substring(length,oldText->l-length));
	    }
	    firstLetter->close();

	}
    }

    insertPseudoChild(RenderStyle::BEFORE, newChild, beforeChild);

    // If the requested beforeChild is not one of our children, then this is most likely because
    // there is an anonymous block box within this object that contains the beforeChild. So
    // just insert the child into the anonymous block box instead of here.
    if (beforeChild && beforeChild->parent() != this) {
        KHTMLAssert(beforeChild->parent());
        KHTMLAssert(beforeChild->parent()->isAnonymousBox());

        if (newChild->isInline()) {
            beforeChild->parent()->addChild(newChild,beforeChild);
	    newChild->setLayouted( false );
	    newChild->setMinMaxKnown( false );
            return;
        }
        else if (beforeChild->parent()->firstChild() != beforeChild)
            return beforeChild->parent()->addChild(newChild, beforeChild);
        else
            return addChildToFlow(newChild, beforeChild->parent());
    }

    // prevent non-layouted elements from getting painted by pushing them far above the top of the
    // page
    if (!newChild->isInline())
        newChild->setPos(newChild->xPos(), -500000);

    if (!newChild->isText())
    {
        if (newChild->style()->position() != STATIC)
            setOverhangingContents();
    }

    // RenderFlow has to either have all of its children inline, or
    // all of its children as blocks.  So, if our children are
    // currently inline and a block child has to be inserted, we move
    // all our inline children into anonymous block boxes
    if ( m_childrenInline && !newChild->isInline() && !newChild->isSpecial() )
    {
        if (isInline()) {
            // We are placing a block inside an inline. We have to
            // perform a split of this inline into continuations.
            // This involves creating an anonymous block box to hold
            // |newChild|.  We then make that block box a continuation
            // of this inline.  We take all of the children after
            // |beforeChild| and put them in a clone of this object.
            RenderStyle *newStyle = new RenderStyle();
            newStyle->inheritFrom(style());
            newStyle->setDisplay(BLOCK);

            RenderFlow *newBox = new (renderArena()) RenderFlow(0 /* anonymous box */);
            newBox->setStyle(newStyle);
            newBox->setIsAnonymousBox(true);
            RenderFlow* oldContinuation = continuation();
            setContinuation(newBox);
            splitFlow(beforeChild, newBox, newChild, oldContinuation);
            return;
        }
        else {
	    makeChildrenNonInline(beforeChild);
	    madeBoxesNonInline = true;
	}

        if (beforeChild) {
	    if ( beforeChild->parent() != this ) {
		beforeChild = beforeChild->parent();
		KHTMLAssert(beforeChild->isAnonymousBox());
		KHTMLAssert(beforeChild->parent() == this);
	    }
        }
    }
    else if (!m_childrenInline)
    {
        // If we're inserting an inline child but all of our children
        // are blocks, then we have to make sure it is put into an
        // anomyous block box. We try to use an existing anonymous box
        // if possible, otherwise a new one is created and inserted
        // into our list of children in the appropriate position.
        if(newChild->isInline()) {
            if (beforeChild) {
                if (beforeChild->previousSibling() && beforeChild->previousSibling()->isAnonymousBox()) {
                    beforeChild->previousSibling()->addChild(newChild);
		    newChild->setLayouted( false );
		    newChild->setMinMaxKnown( false );
                    return;
                }
            }
            else{
                if (m_last && m_last->isAnonymousBox()) {
                    m_last->addChild(newChild);
		    newChild->setLayouted( false );
		    newChild->setMinMaxKnown( false );
		    return;
                }
            }

            // no suitable existing anonymous box - create a new one
            RenderStyle *newStyle = new RenderStyle();
            newStyle->inheritFrom(style());
            newStyle->setDisplay(BLOCK);

            RenderFlow *newBox = new (renderArena()) RenderFlow(0 /* anonymous box */);
            newBox->setStyle(newStyle);
            newBox->setIsAnonymousBox(true);

            RenderBox::addChild(newBox,beforeChild);
            newBox->addChild(newChild);
            newBox->setPos(newBox->xPos(), -500000);

	    newChild->setLayouted( false );
	    newChild->setMinMaxKnown( false );
            return;
        }
        else {
            // We are adding another block child... if the current
            // last child is an anonymous box then it needs to be
            // closed.  ### get rid of the closing thing altogether
            // this will only work during initial parsing
            if (lastChild() && lastChild()->isAnonymousBox()) {
                lastChild()->close();
            }
        }
    }

    RenderBox::addChild(newChild,beforeChild);
    // ### care about aligned stuff

    newChild->setLayouted( false );
    newChild->setMinMaxKnown( false );
    insertPseudoChild(RenderStyle::AFTER, newChild, beforeChild);

    if ( madeBoxesNonInline )
	removeLeftoverAnonymousBoxes();
}

void RenderFlow::makeChildrenNonInline(RenderObject *box2Start)
{
    KHTMLAssert(!box2Start || box2Start->parent() == this);

    m_childrenInline = false;

    RenderObject *child = firstChild();
    RenderObject *next;
    RenderObject *boxFirst = 0;
    RenderObject *boxLast = 0;
    while (child) {
        next = child->nextSibling();

        if (child->isInline()) {
	    if ( !boxFirst )
		boxFirst = child;
            boxLast = child;
        }

        if ( boxFirst &&
	     ( !child->isInline() || !next || child == box2Start ) ) {
            // Create a new anonymous box containing all children starting from boxFirst
            // and up to (but not including) boxLast, and put it in place of the children
            RenderStyle *newStyle = new RenderStyle();
            newStyle->inheritFrom(style());
            newStyle->setDisplay(BLOCK);

            RenderFlow *box = new (renderArena()) RenderFlow(0 /* anonymous box */);
            box->setStyle(newStyle);
            box->setIsAnonymousBox(true);
            // ### the children have a wrong style!!!
            // They get exactly the style of this element, not of the anonymous box
            // might be important for bg colors!

            insertChildNode(box, boxFirst);
            RenderObject* o = boxFirst;
            while(o && o != boxLast)
            {
                RenderObject* no = o;
                o = no->nextSibling();
                box->appendChildNode(removeChildNode(no));
            }
            box->appendChildNode(removeChildNode(boxLast));
            box->close();
            box->setPos(box->xPos(), -500000);
            box->setLayouted(false);
            boxFirst = boxLast = next;
        }

        child = next;
    }

    if (isInline()) {
        setInline(false);
        if (parent()->isFlow()) {
            KHTMLAssert(parent()->childrenInline());
	    static_cast<RenderFlow *>(parent())->makeChildrenNonInline();
        }
    }

    setLayouted(false);
}

void RenderFlow::removeChild(RenderObject *oldChild)
{
    // If this child is a block, and if our previous and next siblings are
    // both anonymous blocks with inline content, then we can go ahead and
    // fold the inline content back together.
    RenderObject* prev = oldChild->previousSibling();
    RenderObject* next = oldChild->nextSibling();
    bool mergedBlocks = false;
    if (!isInline() && !oldChild->isInline() && !oldChild->continuation() &&
        prev && prev->isAnonymousBox() && prev->childrenInline() &&
        next && next->isAnonymousBox() && next->childrenInline()) {
        // Take all the children out of the |next| block and put them in
        // the |prev| block.
        RenderObject* o = next->firstChild();
        while (o) {
            RenderObject* no = o;
            o = no->nextSibling();
            prev->appendChildNode(next->removeChildNode(no));
            no->setLayouted(false);
            no->setMinMaxKnown(false);
        }
        prev->setLayouted(false);
        prev->setMinMaxKnown(false);

        // Nuke the now-empty block.
        next->detach(renderArena());

        mergedBlocks = true;
    }

    RenderBox::removeChild(oldChild);

    if (mergedBlocks && prev && !prev->previousSibling() && !prev->nextSibling()) {
        // The remerge has knocked us down to containing only a single anonymous
        // box.  We can go ahead and pull the content right back up into our
        // box.
        RenderObject* anonBlock = removeChildNode(prev);
        m_childrenInline = true;
        RenderObject* o = anonBlock->firstChild();
        while (o) {
            RenderObject* no = o;
            o = no->nextSibling();
            appendChildNode(anonBlock->removeChildNode(no));
            no->setLayouted(false);
            no->setMinMaxKnown(false);
        }
        setLayouted(false);
        setMinMaxKnown(false);
    }
}

bool RenderFlow::nodeAtPoint(NodeInfo& info, int _x, int _y, int _tx, int _ty)
{
    bool inBox = false;

    int stx=0;
    int sty=0;
    if (floatingObjects||layer())
    {
        stx = _tx + xPos();
        sty = _ty + yPos();
        if (isRelPositioned())
            static_cast<RenderBox*>(this)->relativePositionOffset(stx, sty);

        if (floatingObjects) {
            FloatingObject* o;
            QPtrListIterator<FloatingObject> it(*floatingObjects);
            for (it.toLast(); (o = it.current()); --it)
                if ( !o->noPaint )
                    inBox |= o->node->nodeAtPoint( info, _x, _y,
                                                   stx + o->left + o->node->marginLeft() - o->node->xPos(),
                                                   sty + o->startY + o->node->marginTop() - o->node->yPos() );
        }

        if (layer()) {
            // special case - special objects in root are relative to viewport
            if (isRoot()) {
                stx += static_cast<RenderRoot*>(this)->view()->contentsX();
                sty += static_cast<RenderRoot*>(this)->view()->contentsY();
            }
            for (RenderLayer* l=m_layer->firstChild(); l; l=l->nextSibling()) {
                if (l->renderer()->isPositioned())
                    inBox |= l->renderer()->nodeAtPoint(info, _x, _y, stx, sty);
            }
        }
    }

    inBox |= RenderBox::nodeAtPoint(info, _x, _y, _tx, _ty);
    return inBox;
}

#ifndef NDEBUG
void RenderFlow::printTree(int indent) const
{
    RenderBox::printTree(indent);

    if(floatingObjects)
    {
        QPtrListIterator<FloatingObject> it(*floatingObjects);
        FloatingObject *r;
        for ( ; (r = it.current()); ++it )
        {
            QString s;
            s.fill(' ', indent);
            kdDebug() << s << renderName() << ":  " <<
                (r->type == FloatingObject::FloatLeft ? "FloatLeft" : "FloatRight" )  <<
                "[" << r->node->renderName() << ": " << (void*)r->node << "] (" << r->startY << " - " << r->endY << ")" <<
                (r->noPaint ? " noPaint " : " ") << "left: " << r->left << " width: " << r->width <<
                endl;
        }
    }
}

void RenderFlow::dump(QTextStream *stream, QString ind) const
{
    if (m_childrenInline) { *stream << " childrenInline"; }
    if (m_pre) { *stream << " pre"; }
    if (firstLine) { *stream << " firstLine"; }

    if(floatingObjects && !floatingObjects->isEmpty())
    {
	*stream << " floating(";
        QPtrListIterator<FloatingObject> it(*floatingObjects);
        FloatingObject *r;
	bool first = true;
        for ( ; (r = it.current()); ++it )
        {
            if (!first)
		*stream << ",";
            *stream << r->node->renderName();
	    first = false;
        }
	*stream << ")";
    }

    // ### EClear m_clearStatus

    RenderBox::dump(stream,ind);
}
#endif

#undef DEBUG
#undef DEBUG_LAYOUT
#undef BOX_DEBUG
