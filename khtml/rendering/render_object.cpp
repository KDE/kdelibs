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

#include <kdebug.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qcolor.h>
#include <qpen.h>
#include <qsize.h>

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
    hasKeyboardFocus=DOM::ActivationOff;

    m_layouted = false;
    m_parsing = false;
    m_minMaxKnown = false;

    m_parent = 0;
    m_previous = 0;
    m_next = 0;
    m_first = 0;
    m_last = 0;

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

    m_bgImage = 0;
}

RenderObject::~RenderObject()
{
    if(m_bgImage) m_bgImage->deref(this);

    if (m_parent)
        //have parent, take care of the tree integrity
        m_parent->removeChild(this);

    //if not, it is mass a deletion, just kill everyone
    RenderObject *n;
    RenderObject *next;
    for( n = m_first; n != 0; n = next )
    {
        n->setParent(0); //zero the parent
        next = n->nextSibling();
        delete n;
    }
}


void RenderObject::addChild(RenderObject *newChild, RenderObject *beforeChild)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(RenderObject)::addChild( " << newChild->renderName() << ", "
                       (beforeChild ? beforeChild->renderName() : "0") << " )" << endl;
#endif

    newChild->setParsing();

    bool needsTable = false;

    if(!newChild->isText()) {
        switch(newChild->style()->display()) {
        case INLINE:
        case BLOCK:
        case LIST_ITEM:
        case RUN_IN:
        case COMPACT:
        case MARKER:
        case TABLE:
        case INLINE_TABLE:
            break;
        case TABLE_COLUMN_GROUP:
        case TABLE_COLUMN:
        case TABLE_CAPTION:
        case TABLE_ROW_GROUP:
        case TABLE_HEADER_GROUP:
        case TABLE_FOOTER_GROUP:
            //kdDebug( 6040 ) << "adding section" << endl;
            if ( !isTable() )
                needsTable = true;
            break;
        case TABLE_ROW:
            //kdDebug( 6040 ) << "adding row" << endl;
            if ( !isTableSection() )
                needsTable = true;
            break;
        case TABLE_CELL:
            //kdDebug( 6040 ) << "adding cell" << endl;
            if ( !isTableRow() )
                needsTable = true;
            break;
        case NONE:
            kdDebug( 6000 ) << "error in RenderObject::addChild()!!!!" << endl;
	    break;
        }
    }

    if ( needsTable ) {
        RenderTable *table;
        if( !beforeChild )
            beforeChild = lastChild();
        if( beforeChild && beforeChild->isAnonymousBox() && beforeChild->isTable() )
            table = static_cast<RenderTable *>(beforeChild);
        else {
//          kdDebug( 6040 ) << "creating anonymous table" << endl;
            table = new RenderTable();
            RenderStyle *newStyle = new RenderStyle(m_style);
            newStyle->setDisplay(TABLE);
            table->setStyle(newStyle);
            table->setIsAnonymousBox(true);
            addChild(table, beforeChild);
        }
        table->addChild(newChild);
        return;
    }

    // just add it...
    newChild->setParent(this);

    if (!beforeChild) {
        if(m_last)
        {
            newChild->setPreviousSibling(m_last);
            m_last->setNextSibling(newChild);
            m_last = newChild;
        }
        else
        {
            m_first = m_last = newChild;
        }
    }
    else {
        if (beforeChild == m_first)
            m_first = newChild;
        RenderObject *newPrev = beforeChild->previousSibling();
        newChild->setNextSibling(beforeChild);
        beforeChild->setPreviousSibling(newChild);

        if(newPrev) newPrev->setNextSibling(newChild);
        newChild->setPreviousSibling(newPrev);

    }
    newChild->calcWidth();
}

void RenderObject::removeChild(RenderObject *oldChild)
{
    //kdDebug() << "RenderObject::removeChild" << endl;
    if (oldChild->previousSibling())
        oldChild->previousSibling()->setNextSibling(oldChild->nextSibling());
    if (oldChild->nextSibling())
        oldChild->nextSibling()->setPreviousSibling(oldChild->previousSibling());

    if (m_first == oldChild)
        m_first = oldChild->nextSibling();
    if (m_last == oldChild)
        m_last = oldChild->previousSibling();

    oldChild->setPreviousSibling(0);
    oldChild->setNextSibling(0);
    oldChild->setParent(0);

    setLayouted(false);
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
        while (o && o->style()->position() == STATIC && !o->isHtml())
            o = o->parent();
    } else {
        while(o && o->style()->display() == INLINE)
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

QSize RenderObject::containingBlockSize() const
{
    RenderObject *o = containingBlock();

    if(m_style->position() == ABSOLUTE)
    {
        if(o->isInline())
        {
            // ### fixme
        }
        else
            return o->paddingSize();
    }

    return o->contentSize();
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


QSize RenderObject::contentSize() const
{
    return QSize(0, 0);
}

QSize RenderObject::contentOffset() const
{
    return QSize(0, 0);
}

QSize RenderObject::paddingSize() const
{
    return QSize(0, 0);
}

QSize RenderObject::size() const
{
    return QSize(0, 0);
}

void RenderObject::drawBorder(QPainter *p, int x1, int y1, int x2, int y2, int width,
                              BorderSide s, QColor c, EBorderStyle style, bool sb1, bool sb2)
{
    if(style == DOUBLE && width < 3)
        style = SOLID;

    int half = width/2;
    switch(style)
    {
    case BNONE:
    case BHIDDEN:
        // should not happen
        return;
    case DOTTED:
        p->setPen(QPen(c, width == 1 ? 0 : width, Qt::DotLine));
        /* nobreak; */
    case DASHED:
        if(style == DASHED)
            p->setPen(QPen(c, width == 1 ? 0 : width, Qt::DashLine));

        switch(s)
        {
        case BSTop:
            y1 += half; y2 += half;   break;
        case BSBottom:
            y1 -= half; y2 -= half;   break;
        case BSLeft:
            x1 += half; x2 += half;
            y1 += width; y2 -= width; break;
        case BSRight:
            x1 -= half; x2 -= half;
            y2 -= width; y1 += width; break;
        }

        p->drawLine(x1, y1, x2, y2);
        break;

    case DOUBLE:
    {
        p->setPen(Qt::NoPen);
        p->setBrush(c);

        int w = width/3;
        switch(s)
        {
        case BSTop:
            p->drawRect(x1, y1, x2-x1, w);
            p->drawRect(x1+(sb1 ? 0 : width), y1+width-w, x2-x1-(sb1 ? 0 : width)-(sb2 ? 0 : width), width-2*w);
            break;
        case BSBottom:
            p->drawRect(x1, y1-w, x2-x1, w);
            p->drawRect(x1+(sb1 ? 0 : width-w), y1-width, x2-x1-(sb1 ? 0 : width-w)-(sb2 ? 0 : width-w), width-2*w);
            break;
        case BSLeft:
            p->drawRect(x1, y1, w, y2-y1);
            p->drawRect(x1+width-w, y1+width-w, width-2*w, y2-y1-2*width+2*w);
            break;
        case BSRight:
            p->drawRect(x1-w, y1, w, y2-y1);
            p->drawRect(x1-width, y1+width-w, width-2*w, y2-y1-2*width+2*w);
            break;
        }

        break;
    }
    case GROOVE:
        // could be more efficient. but maybe current code is already faster than
        // drawing two small rectangles?
        // disadvantage is that current edges doesn't look right because of reverse
        // drawing order
        drawBorder(p, x1, y1, x2, y2, width, s, c, INSET, sb1, sb2);
        drawBorder(p, x1, y1, x2, y2, half, s, c, OUTSET, sb1, sb2);
        break;
    case RIDGE:
        drawBorder(p, x1, y1, x2, y2, width, s, c, OUTSET, sb1, sb2);
        drawBorder(p, x1, y1, x2, y2, half, s, c, INSET, sb1, sb2);
        break;
    case INSET:
        if(style == INSET) {
            if(s == BSTop || s == BSLeft) {
                c = c.dark();
                sb2 = true;
            }
            else
                sb1 = true;
        }
        /* nobreak; */
    case OUTSET:
        if(style == OUTSET) {
            if(s == BSBottom || s == BSRight)
                c = c.dark();
            sb2 = (s == BSBottom || s == BSRight);
            sb1 = !(s == BSBottom || s == BSRight);
        }
        /* nobreak; */
    case SOLID:
        // ###: make this shitty code faster (Dirk)
        // use convex polygon drawing (40% faster)
        // only draw expensive edges if its actually visible (colors / visibility different, see sb1 / sb2)
        QPointArray tri(3);
        p->setPen(Qt::NoPen);
        p->setBrush(c);
        switch(s) {
        case BSTop:
            if(width) {
//                if(sb1) {
                    tri.setPoints(3, x1, y1, x1+width, y1, x1+width, y1+width);
                    p->drawPolygon(tri);
//                }
//                if(sb2) {
                    tri.setPoints(3, x2-width, y2, x2, y2, x2-width, y2+width);
                    p->drawPolygon(tri);
//                }
            }
            p->drawRect(x1+(sb1 ? width : 0), y1, x2-x1-(sb1 ? width : 0)-(sb2 ? width : 0), width);
            break;
        case BSBottom:
            if(width)
            {
//                if(sb1)
//                {
                    tri.setPoints(3, x1, y1, x1+width, y1, x1+width, y1-width);
                    p->drawPolygon(tri);
//                }
//                 if(sb2)
//                 {
                    tri.setPoints(3, x2-width, y2-width, x2-width, y2, x2, y2);
                    p->drawPolygon(tri);
//                }
            }
            p->drawRect(x1+width, y1-width, x2-x1-2*width, width);
            break;
        break;
        case BSLeft:
            if(width)
            {
//                 if(sb1)
//                 {
                    tri.setPoints(3, x1, y1, x1, y1+width, x1+width, y1+width);
                    p->drawPolygon(tri);
//                 }
//                 if(sb2)
//                 {
                    tri.setPoints(3, x2, y2-width, x2, y2, x2+width, y2-width);
                    p->drawPolygon(tri);
//                }
            }
            p->drawRect(x1, y1+width, width, y2-y1-2*width);
            break;
        case BSRight:
            if(width)
            {
//                 if(sb1)
//                 {
                    tri.setPoints(3, x1, y1, x1, y1+width, x1-width, y1+width);
                    p->drawPolygon(tri);
//                 }
//                 if(sb2)
//                 {
                    tri.setPoints(3, x2, y2, x2, y2-width, x2-width, y2-width);
                    p->drawPolygon(tri);
//                 }
            }
            p->drawRect(x1-width, y1+width, width, y2-y1-2*width);
            break;
        }
        break;
    }
}

void RenderObject::printBorder(QPainter *p, int _tx, int _ty, int w, int h, const RenderStyle* style, bool begin, bool end)
{
    int bottom = _ty + h;
    int right  = _tx + w;
    const QColor& tc = style->borderTopColor().isValid() ? style->borderTopColor() : style->color();
    const QColor& lc = style->borderLeftColor().isValid() ? style->borderLeftColor() : style->color();
    const QColor& rc = style->borderRightColor().isValid() ? style->borderRightColor() : style->color();
    const QColor& bc = style->borderBottomColor().isValid() ? style->borderBottomColor() : style->color();
    bool render_t = style->borderTopStyle() != BNONE && style->borderTopStyle() != BHIDDEN;
    bool render_l = style->borderLeftStyle() != BNONE && style->borderLeftStyle() != BHIDDEN && begin;
    bool render_r = style->borderRightStyle() != BNONE && style->borderRightStyle() != BHIDDEN && end;
    bool render_b = style->borderBottomStyle() != BNONE && style->borderBottomStyle() != BHIDDEN;

    if(render_r)
        drawBorder(p, right, _ty, right, bottom, style->borderRightWidth(), BSRight, rc,
                   style->borderRightStyle(), render_t && tc != rc, render_b && bc != rc);

    if(render_b)
        drawBorder(p, _tx, _ty, _tx, bottom, style->borderLeftWidth(), BSLeft, bc,
                   style->borderLeftStyle(), render_l && lc != bc, render_r && rc != bc);

    if(render_l)
        drawBorder(p, _tx, bottom, right, bottom, style->borderBottomWidth(), BSBottom, lc,
                   style->borderBottomStyle(), render_t && tc != lc, render_b && bc != lc);

    if(render_t)
        drawBorder(p, _tx, _ty, right, _ty, style->borderTopWidth(), BSTop, tc,
                   style->borderTopStyle(), render_l && lc != tc, render_r && rc != tc);
}

void RenderObject::print( QPainter *p, int x, int y, int w, int h, int tx, int ty)
{
    printObject(p, x, y, w, h, tx, ty);
}

void RenderObject::repaintRectangle(int x, int y, int w, int h)
{
    if(m_parent) m_parent->repaintRectangle(x, y, w, h);
}

void RenderObject::repaintObject(RenderObject *o, int x, int y)
{
    if(m_parent) m_parent->repaintObject(o, x, y);
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
                 << ": " << (void*)this
                 << " il=" << (int)isInline() << " ci=" << (int) childrenInline()
                 << " fl=" << (int)isFloating() << " rp=" << (int)isReplaced()
                 << " an=" << (int)isAnonymousBox()
                 << " ps=" << (int)isPositioned()
                 << " cp=" << (int)containsPositioned()
                 << " laytd=" << (int)layouted()
                 << " (" << xPos() << "," << yPos() << "," << width() << "," << height() << ")" << endl;
    RenderObject *child = firstChild();
    while( child != 0 )
    {
        child->printTree(indent+2);
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
    // reset style flags
    m_floating = false;
    m_positioned = false;
    m_relPositioned = false;
    m_printSpecial = false;
    // ### no support for changing the display type dynamically...
    //m_inline = true;
    m_visible = true;

    // deletion of styles is handled by the DOM elements
    m_style = style;

    if( m_bgImage != m_style->backgroundImage() ) {
	if(m_bgImage) m_bgImage->deref(this);
	m_bgImage = m_style->backgroundImage();
	if(m_bgImage) m_bgImage->ref(this);
    }

    if( m_style->backgroundColor().isValid() || m_style->hasBorder() || m_bgImage )
        m_printSpecial = true;
    else
	m_printSpecial = false;

    if( m_style->visiblity() == HIDDEN || m_style->visiblity() == COLLAPSE )
	m_visible = false;

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

        for( n = m_first; n != 0; n = n->nextSibling() )
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

void RenderObject::setKeyboardFocus(DOM::ActivationState b)
{
  RenderObject *actChild = firstChild();
  //  printTree(0);
  while(actChild) {
      actChild->setKeyboardFocus(b);
      actChild=actChild->nextSibling();
  }
  hasKeyboardFocus=b;
}

QRect RenderObject::viewRect() const
{
    return containingBlock()->viewRect();
}

void RenderObject::absolutePosition(int &xPos, int &yPos, bool f)
{
    if(m_parent)
        m_parent->absolutePosition(xPos, yPos, f);
    else
        xPos = yPos = -1;
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

RenderObject *RenderObject::container() const
{
    EPosition pos = m_style->position();
    RenderObject *o = 0;
    if( pos == FIXED )
	o = root();
    else if ( pos == ABSOLUTE )
	o = containingBlock();
    else
	o = m_parent;
    return o;
}

RenderObject *RenderObject::root() const
{
    RenderObject *o = m_parent;
    while( o && !o->isRoot() )
	o = o->parent();
    return o;
}
