/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
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
//#define DEBUG
//#define DEBUG_LAYOUT
//#define PAR_DEBUG

#include "dom_string.h"

#include <qpainter.h>
#include <qfontmetrics.h>
#include <qstack.h>

#include "dom_node.h"
#include "dom_textimpl.h"
#include "dom_stringimpl.h"
#include "dom_exception.h"

#include "htmlhashes.h"
#include "khtmlview.h"

#include "render_box.h"
#include "render_style.h"
#include "render_object.h"
#include "render_text.h"

#include "render_root.h"

#include <kdebug.h>
#include <assert.h>

#include "misc/loader.h"

using namespace DOM;
using namespace khtml;


RenderBox::RenderBox()
    : RenderObject()
{
    m_minWidth = -1;
    m_maxWidth = -1;
    m_width = m_height = 0;
    m_x = 0;
    m_y = 0;
}

void RenderBox::setStyle(RenderStyle *style)
{
    RenderObject::setStyle(style);

    switch(style->position())
    {
    case ABSOLUTE:
    case FIXED:
	m_positioned = true;
	break;
    default:
	if(style->isFloating()) {
	    m_floating = true;
	} else {
	    if(style->position() == RELATIVE)
		m_relPositioned = true;
	}
    }
}

RenderBox::~RenderBox()
{
    //kdDebug( 6040 ) << "Element destructor: this=" << nodeName().string() << endl;
}

QSize RenderBox::contentSize() const
{
    int w = m_width;
    int h = m_height;
    if(m_style->hasBorder())
    {
	w -= borderLeft() + borderRight();
	h -= borderTop() + borderBottom();
    }
    if(m_style->hasPadding())
    {
	w -= paddingLeft() + paddingRight();
	h -= paddingTop() + paddingBottom();
    }

    return QSize(w, h);
}

short RenderBox::contentWidth() const
{
    short w = m_width;
    //kdDebug( 6040 ) << "RenderBox::contentWidth(1) = " << m_width << endl;
    if(m_style->hasBorder())
	w -= borderLeft() + borderRight();
    if(m_style->hasPadding())
	w -= paddingLeft() + paddingRight();

    //kdDebug( 6040 ) << "RenderBox::contentWidth(2) = " << w << endl;
    return w;
}

int RenderBox::contentHeight() const
{
    int h = m_height;
    if(m_style->hasBorder())
	h -= borderTop() + borderBottom();
    if(m_style->hasPadding())
	h -= paddingTop() + paddingBottom();

    return h;
}

QSize RenderBox::contentOffset() const
{
    // ###
    int xOff = 0;
    int yOff = 0;
    return QSize(0, 0);
}

QSize RenderBox::paddingSize() const
{
    return QSize(0, 0);
}

QSize RenderBox::size() const
{
    return QSize(0, 0);
}

short RenderBox::width() const
{
    return m_width;
}

int RenderBox::height() const
{
    return m_height;
}


// --------------------- printing stuff -------------------------------

void RenderBox::print(QPainter *p, int _x, int _y, int _w, int _h,
				  int _tx, int _ty)
{
    _tx += m_x;
    _ty += m_y;

    // default implementation. Just pass things through to the children
    RenderObject *child = firstChild();
    while(child != 0)
    {
	child->print(p, _x, _y, _w, _h, _tx, _ty);
	child = child->nextSibling();
    }
}

void RenderBox::setPixmap(const QPixmap &, CachedObject *, bool *)
{
    if (style()->htmlHacks() && parent())
    {
    	if (parent()->isRoot())
	    parent()->repaint();
	if (parent()->parent()->isRoot())
	    parent()->parent()->repaint();
    }
    repaint();	//repaint bg when it gets loaded
}

void RenderBox::printBoxDecorations(QPainter *p,int, int _y,
				       int, int _h, int _tx, int _ty)
{
    //kdDebug( 6040 ) << "renderBox::printDecorations()" << endl;

    QColor c = m_style->backgroundColor();

    int w = width();
    int h = height() + cellTopExtra() + cellBottomExtra();	
    _ty -= cellTopExtra();

    int my = MAX(_ty,_y);
    int mh;
    if (_ty<_y)
    	mh=MAX(0,h-(_y-_ty));
    else
    	mh = MIN(_h,h);

    if(c.isValid())
	p->fillRect(_tx, my, w, mh, c);
    if(m_bgImage)
    {
	// ### might need to add some correct offsets
	// ### use paddingX/Y
        switch(m_style->backgroundRepeat())
        {
        case REPEAT_X:
        //p->drawPixmap(_tx, _ty, m_bgImage->pixmap());
            p->drawTiledPixmap(_tx, _ty, w, m_bgImage->pixmap().height(),m_bgImage->pixmap());
            break;
        case REPEAT_Y:
            p->drawTiledPixmap(_tx, _ty, m_bgImage->pixmap().width(), h,m_bgImage->pixmap());
            break;
        case NO_REPEAT:
            p->drawPixmap(_tx, _ty, m_bgImage->pixmap());
            break;
        case REPEAT:
        default:
            p->drawTiledPixmap(_tx + borderLeft(), _ty + borderTop(), w, h,
m_bgImage->tiled_pixmap());
            return;
        }
    }

    if(m_style->hasBorder())
    {
	if(m_style->borderTopStyle() != BNONE)
	{
	    c = m_style->borderTopColor();
	    if(!c.isValid()) c = m_style->color();
	    drawBorder(p, _tx, _ty, _tx + w, _ty, m_style->borderTopWidth(),
		       BSTop, c, m_style->borderTopStyle());
	}
	if(m_style->borderBottomStyle() != BNONE)
	{
	    c = m_style->borderBottomColor();
	    if(!c.isValid()) c = m_style->color();
	    drawBorder(p, _tx, _ty + h, _tx + w, _ty + h, m_style->borderBottomWidth(),
		       BSBottom, c, m_style->borderBottomStyle());
	}
	if(m_style->borderLeftStyle() != BNONE)
	{
	    c = m_style->borderLeftColor();
	    if(!c.isValid()) c = m_style->color();
	    drawBorder(p, _tx, _ty, _tx, _ty + h, m_style->borderLeftWidth(),
		       BSLeft, c, m_style->borderLeftStyle());
	}
	if(m_style->borderRightStyle() != BNONE)
	{
	    c = m_style->borderRightColor();
	    if(!c.isValid()) c = m_style->color();
	    drawBorder(p, _tx + w, _ty, _tx + w, _ty + h, m_style->borderRightWidth(),
		       BSRight, c, m_style->borderRightStyle());
	}
    }
}

void RenderBox::outlineBox(QPainter *p, int _tx, int _ty, const char *color)
{
    p->setPen(QPen(QColor(color), 1, Qt::DotLine));
    p->setBrush( Qt::NoBrush );
    p->drawRect(_tx, _ty, m_width, m_height);
}


void RenderBox::close()
{
    setParsing(false);
    calcMinMaxWidth();
    if(containingBlockWidth() < m_minWidth && m_parent)
    	containingBlock()->updateSize();
    else if(!isInline() || isReplaced())
    {
	layout(true);
    }
}

short RenderBox::containingBlockWidth() const
{
    if (style()->htmlHacks() && style()->flowAroundFloats() && containingBlock()->isFlow() ) 	
    	return static_cast<RenderFlow*>(containingBlock())->lineWidth(m_y);
    else
    	return containingBlock()->contentWidth();
}

void RenderBox::absolutePosition(int &xPos, int &yPos)
{
    if(m_parent)
    {
	m_parent->absolutePosition(xPos, yPos);
	if(xPos != -1) xPos += m_x, yPos += m_y;
    }
    else
	xPos = yPos = -1;
}

void RenderBox::updateSize()
{
//    kdDebug( 6040 ) << renderName() << "(RenderBox) " << this << " ::updateSize()" << endl;

    int oldMin = m_minWidth;
    int oldMax = m_maxWidth;
    setMinMaxKnown(false);
    calcMinMaxWidth();

    if (isInline() && parent() && parent()->isInline())
    {
    	parent()->updateSize();
	return;
    }

    if(m_minWidth > containingBlockWidth() || m_minWidth != oldMin ||
    	m_maxWidth != oldMax || isReplaced())
    {    	
	setLayouted(false);
	if(containingBlock() != this) containingBlock()->updateSize();
    }
    else
    	updateHeight();
}

void RenderBox::updateHeight()
{
//    kdDebug( 6040 ) << renderName() << "(RenderBox) " << this << " ::updateHeight()" << endl;

    if (parsing())
    {
    	setLayouted(false);
    	containingBlock()->updateHeight();		
	return;
    }

    if(!isInline() || isReplaced())
    {
	int oldHeight = m_height;
	setLayouted(false);
	layout(true);

	if(m_height != oldHeight) {
	    if(containingBlock() != this) containingBlock()->updateHeight();
	} else {
	    repaint();
	}
    }

}

void RenderBox::position(int x, int y, int, int, int, bool)
{
    m_x = x;
    m_y = y;
    // ### paddings
    //m_width = width;
}

short RenderBox::verticalPositionHint() const
{
    switch(vAlign())
    {
    case BASELINE:
	//kdDebug( 6040 ) << "aligned to baseline" << endl;
	return contentHeight();
    case SUB:
	// ###
    case SUPER:
	// ###
    case TOP:
	return PositionTop;
    case TEXT_TOP:
	return QFontMetrics(m_style->font()).ascent();
    case MIDDLE:
	return contentHeight()/2;
    case BOTTOM:
	return PositionBottom;
    case TEXT_BOTTOM:
	return QFontMetrics(m_style->font()).descent();
    }
    return 0;
}


short RenderBox::baselineOffset() const
{
    switch(vAlign())
    {
    case BASELINE:
//	kdDebug( 6040 ) << "aligned to baseline" << endl;
	return m_height;
    case SUB:
	// ###
    case SUPER:
	// ###
    case TOP:
	return -1000;
    case TEXT_TOP:
	return QFontMetrics(m_style->font()).ascent();
    case MIDDLE:
	return -QFontMetrics(m_style->font()).width('x')/2;
    case BOTTOM:
	return 1000;
    case TEXT_BOTTOM:
	return QFontMetrics(m_style->font()).descent();
    }
    return 0;
}

int RenderBox::bidiHeight() const
{
    return contentHeight();
}


void RenderBox::repaint()
{
    //kdDebug( 6040 ) << "repaint!" << endl;
    repaintRectangle(0, 0, m_width, m_height);
}

void RenderBox::repaintRectangle(int x, int y, int w, int h)
{
    x += m_x;
    y += m_y;
    // kdDebug( 6040 ) << "RenderBox(" << renderName() << ")::repaintRectangle (" << x << "/" << y << ") (" << w << "/" << h << ")" << endl;
    if(m_parent) m_parent->repaintRectangle(x, y, w, h);
}

void RenderBox::repaintObject(RenderObject *o, int x, int y)
{
    x += m_x;
    y += m_y;
    if(m_parent) m_parent->repaintObject(o, x, y);
}

void RenderBox::relativePositionOffset(int &tx, int &ty)
{
    if(!m_style->left().isVariable())
	tx += m_style->left().width(containingBlockWidth());
    else if(!m_style->right().isVariable())
	tx -= m_style->right().width(containingBlockWidth());
    if(!m_style->top().isVariable())
	ty += m_style->top().width(containingBlockHeight());
    else if(!m_style->bottom().isVariable())
	ty -= m_style->bottom().width(containingBlockHeight());
}

void RenderBox::calcAbsoluteHorizontal()
{
    const int AUTO = -666666;
    int l,r,w, cw;

    l=r=w=AUTO;
    cw = containingBlockWidth()
    	+containingBlock()->paddingLeft() +containingBlock()->paddingRight();

    if(!m_style->left().isVariable())
	l = m_style->left().width(cw);
    if(!m_style->right().isVariable())
	r = m_style->right().width(cw);		
    if(!m_style->width().isVariable())
	w = m_style->width().width(cw);

    RenderObject* o=parent();
    if (style()->direction()==LTR && l==AUTO)
    {
	if (m_next) l = m_next->xPos();
	else if (m_previous) l = m_previous->xPos()+m_previous->contentWidth();
	else l=0;
	while (o && o!=containingBlock()) { l+=o->xPos(); o=o->parent(); }
    }
    else if (r==AUTO)
    {
    	if (m_previous) r = cw - (m_previous->xPos() + m_previous->contentWidth());
	else if (m_next) r = cw - m_next->xPos();
	else r=cw;
	while (o && o!=containingBlock()) { r+=o->xPos(); o=o->parent(); }	
    }

    if (w==AUTO)
    {
    	if (l==AUTO) l=0;
	if (r==AUTO) r=0;
	w = cw - ( r+l+marginLeft()+marginRight()
	    + borderLeft()+ borderRight()+ paddingLeft()+ paddingRight());
    };

    m_width = w + borderLeft()+ borderRight()+ paddingLeft()+ paddingRight();
	
    if (l==AUTO)
    	l=0;

    m_x = l + marginLeft() +
    	containingBlock()->paddingLeft() + containingBlock()->borderLeft();
	
//    printf("h: %d, %d, %d\n",l,w,r);
}

void RenderBox::calcAbsoluteVertical()
{
    const int AUTO = -666666;
    int t,b,h, ch;

    t=b=h=AUTO;

    Length hl = containingBlock()->style()->height();
    if (hl.isFixed())
    	ch = hl.value + containingBlock()->paddingTop()
	     + containingBlock()->paddingBottom();
    else
    	ch = containingBlock()->height();

    if(!m_style->top().isVariable())
	t = m_style->top().width(ch);
    if(!m_style->bottom().isVariable())
	b = m_style->bottom().width(ch);		
    if(!m_style->height().isVariable())
	h = m_style->height().width(ch);

/*    if (t==AUTO && b!=AUTO && h!=AUTO)
    {
    	t = ch - b -
	    (h +borderBottom()+paddingTop()+paddingBottom());
    }*/

    RenderObject* o = parent();
    if (t==AUTO)
    {
	if (m_next) t = m_next->yPos();
	else if (m_previous) t = m_previous->yPos()+m_previous->height();
	else t=0;	
	while (o && o!=containingBlock()) { t+=o->yPos(); o=o->parent(); }
    }

    if (b==AUTO && h==AUTO)
    	b=0;

    if (h==AUTO)
    	h = ch - ( t+b+marginTop()+marginBottom()+
	    borderTop()+borderBottom()+paddingTop()+paddingBottom());	

    h += borderTop()+borderBottom()+paddingTop()+paddingBottom();

    if (m_height<h)
    	m_height = h;

//    printf("v: %d, %d, %d\n",t,h,b);

    m_y = t + marginTop() +
    	containingBlock()->paddingTop() + containingBlock()->borderTop();
    	
}
