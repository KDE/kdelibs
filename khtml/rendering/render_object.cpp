/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
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

#include "render_object.h"

#include "dom_nodeimpl.h"
#include "render_box.h"
#include "render_flow.h"
#include "render_style.h"
#include "render_table.h"
#include "render_list.h"
#include "render_style.h"
#include "render_root.h"
#include "render_hr.h"
#include "misc/loader.h"

#include <kdebug.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qcolor.h>
#include <qpen.h>
#include <qsize.h>
#include <qtextstream.h>

#include <assert.h>
using namespace DOM;
using namespace khtml;

RenderObject *RenderObject::createObject(DOM::NodeImpl *node)
{
    RenderStyle *style = node->style();
    RenderObject *o = 0;
    switch(style->display())
    {
    case INLINE:
    case BLOCK:
        o = new RenderFlow();
        break;
    case LIST_ITEM:
        o = new RenderListItem();
        break;
    case RUN_IN:
    case COMPACT:
    case MARKER:
        break;
    case KONQ_RULER:
        o = new RenderHR();
        break;
    case TABLE:
    case INLINE_TABLE:
        // ### set inline/block right
        //kdDebug( 6040 ) << "creating RenderTable" << endl;
        o = new RenderTable();
        break;
    case TABLE_ROW_GROUP:
    case TABLE_HEADER_GROUP:
    case TABLE_FOOTER_GROUP:
        o = new RenderTableSection();
        break;
    case TABLE_ROW:
        o = new RenderTableRow();
        break;
    case TABLE_COLUMN_GROUP:
    case TABLE_COLUMN:
        o = new RenderTableCol();
        break;
    case TABLE_CELL:
        o = new RenderTableCell();
        break;
    case TABLE_CAPTION:
        o = new RenderTableCaption();
        break;
    case NONE:
        return 0;
    }
    if(o) o->setStyle(style);
    return o;
}


RenderObject::RenderObject()
{
    m_style = 0;

    m_layouted = false;
    m_parsing = false;
    m_minMaxKnown = false;

    m_parent = 0;
    m_previous = 0;
    m_next = 0;

    m_floating = false;
    m_positioned = false;
    m_relPositioned = false;
    m_printSpecial = false;
    m_containsPositioned = false;
    m_isAnonymous = false;
    m_isText = false;
    m_inline = true;
    m_replaced = false;
    m_visible = true;
    m_containsWidget = false;
    m_containsOverhangingFloats = false;
    m_hasFirstLine = false;
    m_verticalPosition = PositionUndefined;
}

RenderObject::~RenderObject()
{
    if(m_style->backgroundImage())
        m_style->backgroundImage()->deref(this);

    if (m_style)
	m_style->deref();
}


void RenderObject::addChild(RenderObject* , RenderObject *)
{
    assert(0);
}

RenderObject* RenderObject::removeChildNode(RenderObject* )
{
    assert(0);
    return 0;
}

void RenderObject::removeChild(RenderObject* )
{
    assert(0);
}

void RenderObject::appendChildNode(RenderObject*)
{
    assert(0);
}

void RenderObject::insertChildNode(RenderObject*, RenderObject*)
{
    assert(0);
}

RenderObject *RenderObject::containingBlock() const
{
    if(isTableCell()) {
        return static_cast<const RenderTableCell *>(this)->table();
    }

    RenderObject *o = parent();
    if(m_style->position() == FIXED) {
        while ( o && !o->isRoot() )
            o = o->parent();
    }
    else if(m_style->position() == ABSOLUTE) {
        while (o && o->style()->position() == STATIC && !o->isHtml() && !o->isRoot())
            o = o->parent();
    } else {
        while(o && o->isInline())
            o = o->parent();
    }
    // this is just to make sure we return a valid element.
    // the case below should never happen...
    if(!o) {
        if(!isRoot()) {
            //assert ( false );
            kdDebug( 6040 ) << renderName() << "(RenderObject): No containingBlock!" << endl;
        }
        return const_cast<RenderObject *>(this);
    } else
        return o;
}

short RenderObject::containingBlockWidth() const
{
    // ###
    return containingBlock()->contentWidth();
}

int RenderObject::containingBlockHeight() const
{
    // ###
    return containingBlock()->contentHeight();
}

void RenderObject::drawBorder(QPainter *p, int x1, int y1, int x2, int y2,
                              BorderSide s, QColor c, const QColor& textcolor, EBorderStyle style,
                              bool sb1, bool sb2, int adjbw1, int adjbw2, bool invalidisInvert)
{
    int width = (s==BSTop||s==BSBottom?y2-y1:x2-x1);

    if(style == DOUBLE && width < 3)
        style = SOLID;

    if(!c.isValid()) {
        if(invalidisInvert)
        {
            p->setRasterOp(Qt::XorROP);
            c = Qt::white;
        }
        else {
            if(style == INSET || style == OUTSET || style == RIDGE || style ==
	    GROOVE)
                c.setRgb(238, 238, 238);
            else
                c = textcolor;
        }
    }

    switch(style)
    {
    case BNONE:
    case BHIDDEN:
        // should not happen
        if(invalidisInvert && p->rasterOp() == Qt::XorROP)
            p->setRasterOp(Qt::CopyROP);

        return;
    case DOTTED:
        p->setPen(QPen(c, width == 1 ? 0 : width, Qt::DotLine));
        /* nobreak; */
    case DASHED:
        if(style == DASHED)
            p->setPen(QPen(c, width == 1 ? 0 : width, Qt::DashLine));
        {
            int half = width/2;

            switch(s)
            {
            case BSTop:
            case BSBottom:
                p->drawLine(x1+half, (y1+y2)/2, x2-half, (y1+y2)/2);
            case BSLeft:
            case BSRight:
                p->drawLine((x1+x2-1)/2, y1+half, (x1+x2-1)/2, y2-half);
            }

        }
        break;

    case DOUBLE:
    {
        p->setPen(Qt::NoPen);
        p->setBrush(c);
	// ### fix off-by-one-errors
        int third = (width+1)/3;
        switch(s)
        {
        case BSTop:
            p->drawRect(x1+QMAX(-adjbw1,0), y1      , x2-x1-QMAX(-adjbw1,0)-QMAX(-adjbw2,0), third);
            p->drawRect(x1+QMAX( adjbw1,0), y2-third, x2-x1-QMAX( adjbw1,0)-QMAX( adjbw2,0), third);
            break;
        case BSBottom:
            p->drawRect(x1+QMAX( adjbw1,0)+1, y1      , x2-x1-QMAX( adjbw1,0)-QMAX( adjbw2,0)-1, third);
            p->drawRect(x1+QMAX(-adjbw1,0)+1, y2-third, x2-x1-QMAX(-adjbw1,0)-QMAX(-adjbw2,0)-1, third);
            break;
        case BSLeft:
            p->drawRect(x1      , y1+QMAX(-adjbw1,0)+1, third, y2-y1-QMAX(-adjbw1,0)-QMAX(-adjbw2,0)-1);
            p->drawRect(x2-third, y1+QMAX( adjbw1,0)+1, third, y2-y1-QMAX( adjbw1,0)-QMAX( adjbw2,0)-1);
            break;
        case BSRight:
            p->drawRect(x1      , y1+QMAX( adjbw1,0)+1, third, y2-y1-QMAX( adjbw1,0)-QMAX( adjbw2,0)-1);
            p->drawRect(x2-third, y1+QMAX(-adjbw1,0)+1, third, y2-y1-QMAX(-adjbw1,0)-QMAX(-adjbw2,0)-1);
            break;
        }

        break;
    }
    case RIDGE:
    case GROOVE:
    {
        EBorderStyle s1;
        EBorderStyle s2;
        if (style==GROOVE)
        {
            s1 = INSET;
            s2 = OUTSET;
        }
        else
        {
            s1 = OUTSET;
            s2 = INSET;
        }

	int adjbw1bighalf;
	int adjbw2bighalf;
	if (adjbw1>0) adjbw1bighalf=adjbw1+1;
	else adjbw1bighalf=adjbw1-1;
	adjbw1bighalf/=2;

	if (adjbw2>0) adjbw2bighalf=adjbw2+1;
	else adjbw2bighalf=adjbw2-1;
	adjbw2bighalf/=2;

        switch (s)
	{
	case BSTop:
	    drawBorder(p, x1+QMAX(-adjbw1  ,0)/2,  y1        , x2-QMAX(-adjbw2,0)/2, (y1+y2+1)/2, s, c, textcolor, s1, true, true, adjbw1bighalf, adjbw2bighalf);
	    drawBorder(p, x1+QMAX( adjbw1+1,0)/2, (y1+y2+1)/2, x2-QMAX( adjbw2+1,0)/2,  y2        , s, c, textcolor, s2, true, true, adjbw1/2, adjbw2/2);
	    break;
	case BSLeft:
            drawBorder(p,  x1        , y1+QMAX(-adjbw1  ,0)/2, (x1+x2+1)/2, y2-QMAX(-adjbw2,0)/2, s, c, textcolor, s1, true, true, adjbw1bighalf, adjbw2bighalf);
	    drawBorder(p, (x1+x2+1)/2, y1+QMAX( adjbw1+1,0)/2,  x2        , y2-QMAX( adjbw2+1,0)/2, s, c, textcolor, s2, true, true, adjbw1/2, adjbw2/2);
	    break;
	case BSBottom:
	    drawBorder(p, x1+QMAX( adjbw1  ,0)/2,  y1        , x2-QMAX( adjbw2,0)/2, (y1+y2+1)/2, s, c, textcolor, s2, true, true, adjbw1bighalf, adjbw2bighalf);
	    drawBorder(p, x1+QMAX(-adjbw1+1,0)/2, (y1+y2+1)/2, x2-QMAX(-adjbw2+1,0)/2,  y2        , s, c, textcolor, s1, true, true, adjbw1/2, adjbw2/2);
	    break;
	case BSRight:
            drawBorder(p,  x1        , y1+QMAX( adjbw1  ,0)/2, (x1+x2+1)/2, y2-QMAX( adjbw2,0)/2, s, c, textcolor, s2, true, true, adjbw1bighalf, adjbw2bighalf);
	    drawBorder(p, (x1+x2+1)/2, y1+QMAX(-adjbw1+1,0)/2,  x2        , y2-QMAX(-adjbw2+1,0)/2, s, c, textcolor, s1, true, true, adjbw1/2, adjbw2/2);
	    break;
	}
        break;
    }
    case INSET:
        if(s == BSTop || s == BSLeft)
            c = c.dark();

        /* nobreak; */
    case OUTSET:
        if(style == OUTSET && (s == BSBottom || s == BSRight))
            c = c.dark();

        if(s == BSTop || s == BSLeft)
            sb2 = true;
        else
            sb1 = true;
        /* nobreak; */
    case SOLID:
        QPointArray quad(4);
        p->setPen(Qt::NoPen);
        p->setBrush(c);
        switch(s) {
        case BSTop:
            quad.setPoints(4,
			   x1+QMAX(-adjbw1,0), y1,
                           x1+QMAX( adjbw1,0), y2,
                           x2-QMAX( adjbw2,0), y2,
                           x2-QMAX(-adjbw2,0), y1);
            break;
        case BSBottom:
            quad.setPoints(4,
			   x1+QMAX( adjbw1,0), y1,
                           x1+QMAX(-adjbw1,0), y2,
                           x2-QMAX(-adjbw2,0), y2,
                           x2-QMAX( adjbw2,0), y1);
            break;
        case BSLeft:
	  quad.setPoints(4,
			 x1, y1+QMAX(-adjbw1,0),
               		 x1, y2-QMAX(-adjbw2,0),
			 x2, y2-QMAX( adjbw2,0),
			 x2, y1+QMAX( adjbw1,0));
            break;
        case BSRight:
	  quad.setPoints(4,
			 x1, y1+QMAX( adjbw1,0),
               		 x1, y2-QMAX( adjbw2,0),
			 x2, y2-QMAX(-adjbw2,0),
			 x2, y1+QMAX(-adjbw1,0));
            break;
        }
	p->drawPolygon(quad);
        break;
    }

    if(invalidisInvert && p->rasterOp() == Qt::XorROP)
        p->setRasterOp(Qt::CopyROP);
}

void RenderObject::printBorder(QPainter *p, int _tx, int _ty, int w, int h, const RenderStyle* style, bool begin, bool end)
{
    const QColor& tc = style->borderTopColor();
    const QColor& lc = style->borderLeftColor();
    const QColor& rc = style->borderRightColor();
    const QColor& bc = style->borderBottomColor();
    bool render_t = style->borderTopStyle() != BNONE && style->borderTopStyle() != BHIDDEN;
    bool render_l = style->borderLeftStyle() != BNONE && style->borderLeftStyle() != BHIDDEN && begin;
    bool render_r = style->borderRightStyle() != BNONE && style->borderRightStyle() != BHIDDEN && end;
    bool render_b = style->borderBottomStyle() != BNONE && style->borderBottomStyle() != BHIDDEN;

    if(render_r)
        drawBorder(p, _tx + w - style->borderRightWidth(), _ty, _tx + w, _ty + h, BSRight, rc, style->color(),
                   style->borderRightStyle(), render_t && tc != rc, render_b && bc != rc,
                   style->borderTopWidth(), style->borderBottomWidth());

    if(render_b)
        drawBorder(p, _tx, _ty + h - style->borderBottomWidth(), _tx + w, _ty + h, BSBottom, bc, style->color(),
                   style->borderBottomStyle(), render_l && lc != bc, render_r && rc != bc,
                   begin ? style->borderLeftWidth() : 0, end ? style->borderRightWidth() : 0);

    if(render_l)
        drawBorder(p, _tx, _ty, _tx + style->borderLeftWidth(), _ty + h, BSLeft, lc, style->color(),
                   style->borderLeftStyle(), render_t && tc != lc, render_b && bc != lc,
                   style->borderTopWidth(), style->borderBottomWidth());

    if(render_t)
        drawBorder(p, _tx, _ty, _tx + w, _ty +  style->borderTopWidth(), BSTop, tc, style->color(),
                   style->borderTopStyle(), render_l && lc != tc, render_r && rc != tc,
                   begin ? style->borderLeftWidth() : 0, end ? style->borderRightWidth() : 0);
}

void RenderObject::printOutline(QPainter *p, int _tx, int _ty, int w, int h, const RenderStyle* style)
{
    int ow = style->outlineWidth();
    if(!ow) return;

    const QColor& oc = style->outlineColor();
    EBorderStyle os = style->outlineStyle();

    drawBorder(p, _tx-ow, _ty-ow, _tx, _ty+h+ow, BSLeft,
	       QColor(oc), style->color(),
               os, false, false, ow, ow, true);

    drawBorder(p, _tx-ow, _ty-ow, _tx+w+ow, _ty, BSTop,
	       QColor(oc), style->color(),
	       os, false, false, ow, ow, true);

    drawBorder(p, _tx+w, _ty-ow, _tx+w+ow, _ty+h+ow, BSRight,
	       QColor(oc), style->color(),
	       os, false, false, ow, ow, true);

    drawBorder(p, _tx-ow, _ty+h, _tx+w+ow, _ty+h+ow, BSBottom,
	       QColor(oc), style->color(),
	       os, false, false, ow, ow, true);

}

void RenderObject::print( QPainter *p, int x, int y, int w, int h, int tx, int ty)
{
    printObject(p, x, y, w, h, tx, ty);
}

void RenderObject::repaintRectangle(int x, int y, int w, int h, bool f)
{
    if(parent()) parent()->repaintRectangle(x, y, w, h, f);
}

void RenderObject::printTree(int indent) const
{
    QString ind;
    ind.fill(' ', indent);
    int childcount = 0;
    for(RenderObject* c = firstChild(); c; c = c->nextSibling())
        childcount++;

    kdDebug()    << ind << renderName()
                 << (childcount ?
                     (QString::fromLatin1("[") + QString::number(childcount) + QString::fromLatin1("]"))
                     : QString::null)
                 << "(" << (style() ? style()->refCount() : 0) << ")"
                 << ": " << (void*)this
                 << " il=" << (int)isInline() << " ci=" << (int) childrenInline()
                 << " fl=" << (int)isFloating() << " rp=" << (int)isReplaced()
                 << " an=" << (int)isAnonymousBox()
                 << " ps=" << (int)isPositioned()
                 << " cp=" << (int)containsPositioned()
                 << " lt=" << (int)layouted()
                 << " cw=" << (int)containsWidget()
                 << " pa=" << (int)parsing()
                 << " (" << xPos() << "," << yPos() << "," << width() << "," << height() << ")" << endl;
    RenderObject *child = firstChild();
    while( child != 0 )
    {
        child->printTree(indent+2);
        child = child->nextSibling();
    }
}

void RenderObject::dump(QTextStream *stream, QString ind) const
{
    *stream << ind << "m_verticalPosition = " << m_verticalPosition << endl;
    *stream << ind << "m_layouted = " << m_layouted << endl;
    *stream << ind << "m_parsing = " << m_parsing << endl;
    *stream << ind << "m_minMaxKnown = " << m_minMaxKnown << endl;
    *stream << ind << "m_floating = " << m_floating << endl;
    *stream << ind << "m_positioned = " << m_positioned << endl;
    *stream << ind << "m_containsPositioned = " << m_containsPositioned << endl;
    *stream << ind << "m_relPositioned = " << m_relPositioned << endl;
    *stream << ind << "m_printSpecial = " << m_printSpecial << endl;
    *stream << ind << "m_isAnonymous = " << m_isAnonymous << endl;
    *stream << ind << "m_visible = " << m_visible << endl;
    *stream << ind << "m_isText = " << m_isText << endl;
    *stream << ind << "m_inline = " << m_inline << endl;
    *stream << ind << "m_replaced " << m_replaced << endl;
    *stream << ind << "m_containsWidget = " << m_containsWidget << endl;
    *stream << ind << "m_containsOverhangingFloats = " << m_containsOverhangingFloats << endl;
    *stream << ind << "m_hasFirstLine = " << m_hasFirstLine << endl;
    *stream << endl;

    RenderObject *child = firstChild();
    while( child != 0 )
    {
	*stream << ind << "    *** " << child->renderName() << " *** " << endl;
        child->dump(stream,ind+"    ");
        child = child->nextSibling();
    }
}

void RenderObject::selectionStartEnd(int& spos, int& epos)
{
    if (parent())
        parent()->selectionStartEnd(spos, epos);
}

void RenderObject::updateSize()
{
    containingBlock()->updateSize();
}

void RenderObject::setStyle(RenderStyle *style)
{
    if (m_style == style)
	return;

    // reset style flags
    m_floating = false;
    m_positioned = false;
    m_relPositioned = false;
    m_printSpecial = false;
    // no support for changing the display type dynamically... object must be
    // detached and re-attached as a different type
    //m_inline = true;
    m_visible = true;

    RenderStyle *oldStyle = m_style;
    m_style = style;

    CachedImage* ob = 0;
    CachedImage* nb = 0;

    if (m_style)
    {
	m_style->ref();
        nb = m_style->backgroundImage();
    }
    if (oldStyle)
    {
        ob = oldStyle->backgroundImage();
	oldStyle->deref();
    }

    if( ob != nb ) {
	if(ob) ob->deref(this);
	if(nb) nb->ref(this);
    }

    if( m_style->backgroundColor().isValid() || m_style->hasBorder() || nb )
        setSpecialObjects(true);
    else
        setSpecialObjects(false);

    if( m_style->visiblity() == HIDDEN || m_style->visiblity() == COLLAPSE )
	m_visible = false;

    m_hasFirstLine = (style->getPseudoStyle(RenderStyle::FIRST_LINE) != 0);

    setMinMaxKnown(false);
    setLayouted(false);
}

void RenderObject::setContainsPositioned(bool p)
{
    if (p)
    {
        m_containsPositioned = true;
        if (containingBlock()!=this)
            containingBlock()->setContainsPositioned(true);
    }
    else
    {
        RenderObject *n;
        bool c=false;

        for( n = firstChild(); n != 0; n = n->nextSibling() )
        {
            if (n->isPositioned() || n->containsPositioned())
                c=true;
        }

        if (c)
            return;
        else
        {
            m_containsPositioned = false;
            if (containingBlock()!=this)
                containingBlock()->setContainsPositioned(false);
        }
    }
}

QRect RenderObject::viewRect() const
{
    return containingBlock()->viewRect();
}

bool RenderObject::absolutePosition(int &xPos, int &yPos, bool f)
{
    if(parent())
        return parent()->absolutePosition(xPos, yPos, f);
    else
    {
        xPos = yPos = 0;
        return false;
    }
}

void RenderObject::cursorPos(int /*offset*/, int &_x, int &_y, int &height)
{
    _x = _y = height = -1;
}

int RenderObject::paddingTop() const
{
    int cw=0;
    if (style()->paddingTop().isPercent())
        cw = containingBlock()->contentWidth();
    return m_style->paddingTop().minWidth(cw);
}

int RenderObject::paddingBottom() const
{
    int cw=0;
    if (style()->paddingBottom().isPercent())
        cw = containingBlock()->contentWidth();
    return m_style->paddingBottom().minWidth(cw);
}

int RenderObject::paddingLeft() const
{
    int cw=0;
    if (style()->paddingLeft().isPercent())
        cw = containingBlock()->contentWidth();
    return m_style->paddingLeft().minWidth(cw);
}

int RenderObject::paddingRight() const
{
    int cw=0;
    if (style()->paddingRight().isPercent())
        cw = containingBlock()->contentWidth();
    return m_style->paddingRight().minWidth(cw);
}

RenderRoot* RenderObject::root() const
{
    RenderObject* o = const_cast<RenderObject*>( this );
    while ( o->parent() ) o = o->parent();

    assert( o->isRoot() );
    return static_cast<RenderRoot*>( o );
}

RenderObject *RenderObject::container() const
{
    EPosition pos = m_style->position();
    RenderObject *o = 0;
    if( pos == FIXED )
	o = root();
    else if ( pos == ABSOLUTE )
	o = containingBlock();
    else
	o = parent();
    return o;
}

void RenderObject::invalidateLayout()
{
    setLayouted(false);
    if (m_parent && m_parent->layouted())
        m_parent->invalidateLayout();
}

void RenderObject::removeFromSpecialObjects()
{
    if (isPositioned() || isFloating()) {
	RenderObject *p;
	for (p = parent(); p; p = p->parent()) {
	    if (p->isFlow())
		static_cast<RenderFlow*>(p)->removeSpecialObject(this);
	}
    }
}

void RenderObject::detach()
{
    remove();
    // by default no refcounting
    delete this;
}

bool RenderObject::containsPoint(int _x, int _y, int _tx, int _ty)
{
    return ((_y >= _ty) && (_y < _ty + height()) &&
	    (_x >= _tx) && (_x < _tx + width()));
}

short RenderObject::verticalPositionHint( bool firstLine ) const
{
    short vpos = m_verticalPosition;
    if ( m_verticalPosition == PositionUndefined || firstLine ) {
	vpos = getVerticalPosition( firstLine );
	if ( !firstLine )
	    const_cast<RenderObject *>(this)->m_verticalPosition = vpos;
    }
    return vpos;

}

short RenderObject::getVerticalPosition( bool firstLine ) const
{
    // vertical align for table cells has a different meaning
    int vpos = 0;
    if ( !isTableCell() ) {
	EVerticalAlign va = style()->verticalAlign();
	if ( va == TOP ) {
	    vpos = PositionTop;
	} else if ( va == BOTTOM ) {
	    vpos = PositionBottom;
	} else if ( va == LENGTH ) {
	    vpos = -style()->verticalAlignLength().width( lineHeight( firstLine ) );
	} else if ( parent() && parent()->childrenInline() ) {
	    vpos = parent()->verticalPositionHint( firstLine );
	    // don't allow elements nested inside text-top to have a different valignment.
	    if ( va == BASELINE || vpos == PositionTop || vpos == PositionBottom )
		return vpos;

	    QFont f = parent()->font( firstLine );
	    if ( va == SUB )
		vpos += f.pixelSize()/5 + 1;
	    else if ( va == SUPER )
		vpos -= f.pixelSize()/3 + 1;
	    else if ( va == TEXT_TOP ) {
		vpos += -QFontMetrics(f).ascent() + baselinePosition( firstLine );
	    } else if ( va == MIDDLE ) {
		QRect b = QFontMetrics(f).boundingRect('x');
		vpos += -b.height()/2 - lineHeight( firstLine )/2 + baselinePosition( firstLine );
	    } else if ( va == TEXT_BOTTOM ) {
		vpos += QFontMetrics(f).descent();
		if ( !isReplaced() )
		    vpos -= QFontMetrics(font(firstLine)).descent();
	    } else if ( va == BASELINE_MIDDLE )
		vpos += - lineHeight( firstLine )/2 + baselinePosition( firstLine );
	}
    }
    return vpos;
}

int RenderObject::lineHeight( bool firstLine ) const
{
    // is this method ever called?
    //assert( 0 );

    // ### optimise and don't ignore :first-line
    return style()->lineHeight().width( QFontMetrics( font( firstLine ) ).height() );
}

short RenderObject::baselinePosition( bool firstLine ) const
{
    return isInline() ? QFontMetrics(font(firstLine)).ascent() : contentHeight();
}

QFont RenderObject::font(bool firstLine) const
{
    if( firstLine && hasFirstLine() ) {
	RenderStyle *pseudoStyle  = style()->getPseudoStyle(RenderStyle::FIRST_LINE);
	if ( pseudoStyle )
	    return pseudoStyle->font();
    }
    return style()->font();
}

void RenderObject::invalidateVerticalPositions()
{
    m_verticalPosition = PositionUndefined;
    RenderObject *child = firstChild();
    while( child ) {
	child->invalidateVerticalPositions();
	child = child->nextSibling();
    }
}

