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
    m_marginTop = 0;
    m_marginBottom = 0;
    m_marginLeft = 0;
    m_marginRight = 0;
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
    //int xOff = 0;
    //int yOff = 0;
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
#if 0
    if (style()->htmlHacks() && parent())
    {
    	if (parent()->isRoot())
	    parent()->repaint();
	if (parent()->parent()->isRoot())
	    parent()->parent()->repaint();
    }
#endif
    repaint();	//repaint bg when it gets loaded
}

void RenderBox::printBoxDecorations(QPainter *p,int, int _y,
				       int, int _h, int _tx, int _ty)
{
    //kdDebug( 6040 ) << renderName() << "::printDecorations()" << endl;

    QColor c = m_style->backgroundColor();

    int w = width();
    int h = height() + borderTopExtra() + borderBottomExtra();	
    _ty -= borderTopExtra();

    int my = MAX(_ty,_y);
    int mh;
    if (_ty<_y)
    	mh=MAX(0,h-(_y-_ty));
    else
    	mh = MIN(_h,h);

    if(c.isValid()) {
	//kdDebug( 6040 ) << "printing bgcolor" << endl;
	p->fillRect(_tx, my, w, mh, c);
    }
    if(m_bgImage) {
	//kdDebug( 6040 ) << "printing bgimage at " << _tx << "/" << _ty << endl;
	// ### might need to add some correct offsets
	// ### use paddingX/Y
	
	int sx = 0;
	int sy = 0;
	
	if( !m_style->backgroundAttachment() ) {
	    //kdDebug(0) << "fixed background" << endl;
	    QRect r = viewRect();
	    sx = _tx - r.x();
	    sy = _ty - r.y();
	
	} 	

	switch(m_style->backgroundRepeat()) {
	case NO_REPEAT:
	    if(m_bgImage->pixmap().width() < w)
		h = m_bgImage->pixmap().width();
	case REPEAT_X:
	    if(m_bgImage->pixmap().height() < h)
		h = m_bgImage->pixmap().height();
	    break;
	case REPEAT_Y:
	    if(m_bgImage->pixmap().width() < h)
		h = m_bgImage->pixmap().width();
	    break;
	case REPEAT:
	    break;
	}
	p->drawTiledPixmap(_tx, _ty, w, h,m_bgImage->pixmap(), sx, sy);
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
    calcWidth();
    calcHeight();
    calcMinMaxWidth();
    if(containingBlockWidth() < m_minWidth && m_parent)
    	containingBlock()->updateSize();
    else if(!isInline() || isReplaced())
    {
	layout();
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

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(RenderBox) " << this << " ::updateSize()" << endl;
#endif


    int oldMin = m_minWidth;
    int oldMax = m_maxWidth;
    setMinMaxKnown(false);
    calcMinMaxWidth();

    if ((isInline() || isFloating()) && parent())
    {
        if (!isInline())
            setLayouted(false);
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

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(RenderBox) " << this << " ::updateHeight()" << endl;
#endif

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
	layout();

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
    switch(m_style->verticalAlign())
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
    switch(m_style->verticalAlign())
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
    {
    	if (!m_style->top().isPercent()
	    	|| containingBlock()->style()->height().isFixed())
	    ty += m_style->top().width(containingBlockHeight());
    }
    else if(!m_style->bottom().isVariable())
    {
    	if (!m_style->bottom().isPercent()
	    	|| containingBlock()->style()->height().isFixed())
	    ty -= m_style->bottom().width(containingBlockHeight());
    }
}

void RenderBox::calcWidth()
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << "RenderBox("<<renderName()<<")::calcWidth()" << endl;
#endif
    if (isPositioned())
    {
    	calcAbsoluteHorizontal();
    }
    else
    {

        Length w = m_style->width();
        if (isReplaced() && w.type==Variable)
            w = Length(intrinsicWidth(), Fixed);

    	Length ml = m_style->marginLeft();
	Length mr = m_style->marginRight();

	int cw = containingBlockWidth();
	
	m_marginLeft = 0;
	m_marginRight = 0;
	
	if (w.type == Variable)
	{
//	    kdDebug( 6040 ) << "variable" << endl;
	    m_marginLeft = ml.minWidth(cw);
	    m_marginRight = mr.minWidth(cw);
    	    m_width = cw - m_marginLeft - m_marginRight;
//	    kdDebug( 6040 ) <<  m_width <<"," <<
//	    	m_marginLeft <<"," <<  m_marginRight << endl;
		
	    if(m_width < m_minWidth) m_width = m_minWidth;
	}
	else
	{	
//	    kdDebug( 6040 ) << "non-variable " << w.type << ","<< w.value << endl;
    	    m_width = w.width(cw);
	    m_width += paddingLeft() + paddingRight() + borderLeft() + borderRight();
	
	    if(m_width < m_minWidth) m_width = m_minWidth;
	
            calcHorizontalMargins(ml,mr,cw);

	}
	if (cw != m_width + m_marginLeft + m_marginRight && !isFloating())
	{
    	    if (style()->direction()==LTR)
		m_marginRight = cw - m_width - m_marginLeft;
	    else
		m_marginLeft = cw - m_width - m_marginRight;

	}
    }


#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << "RenderBox::calcWidth(): m_width=" << m_width << " containingBlockWidth()=" << containingBlockWidth() << endl;
    kdDebug( 6040 ) << "m_marginLeft=" << m_marginLeft << " m_marginRight=" << m_marginRight << endl;
#endif
}

void RenderBox::calcHorizontalMargins(const Length& ml, const Length& mr, int cw)
{
    if (isFloating())
    {
 	m_marginLeft = ml.minWidth(cw);
	m_marginRight = mr.minWidth(cw);
    }
    else
    {
        if (ml.type == Variable && mr.type == Variable )
        {
	    m_marginRight = (cw - m_width)/2;		
	    m_marginLeft = cw - m_width - m_marginRight;
        }
        else if (mr.type == Variable)
        {
	    m_marginLeft = ml.width(cw);
	    m_marginRight = cw - m_width - m_marginLeft;
        }
        else if (ml.type == Variable)
        {	    	
	    m_marginRight = mr.width(cw);		
	    m_marginLeft = cw - m_width - m_marginRight;
        }
        else
        {
	    m_marginLeft = ml.minWidth(cw);
	    m_marginRight = mr.minWidth(cw);
        }
    }
}

void RenderBox::calcHeight()
{

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << "RenderBox::calcHeight()" << endl;
#endif

    //cell height is managed by table
    if (isTableCell())
        return;               
    
    if (isPositioned())
    	calcAbsoluteVertical();
    else
    {

        Length h = m_style->height();

        if (isReplaced() && h.type == Variable)
            h = Length(intrinsicHeight(), Fixed);

	Length tm = style()->marginTop();
	Length bm = style()->marginBottom();
	Length ch = containingBlock()->style()->height();
	
	if (!tm.isPercent() || ch.isFixed())
	    m_marginTop = tm.minWidth(ch.value);		
	else
	    m_marginTop = 0;
	
	if (!bm.isPercent() || ch.isFixed())
	    m_marginBottom = bm.minWidth(ch.value);		
	else
	    m_marginBottom = 0;

        // for tables, calculate margins only
        if (isTable())
            return;      
                
	if (h.isFixed())
    	    m_height = MAX (h.value + borderTop() + paddingTop()
		+ borderBottom() + paddingBottom() , m_height);
	else if (h.isPercent())
	{    	
	    if (ch.isFixed())
    		m_height = MAX (h.width(ch.value) + borderTop() + paddingTop()
	    	    + borderBottom() + paddingBottom(), m_height);
	}
    }
}


void RenderBox::calcAbsoluteHorizontal()
{
    const int AUTO = -666666;
    int l,r,w,ml,mr,cw;

    int pab = borderLeft()+ borderRight()+ paddingLeft()+ paddingRight();

    l=r=ml=mr=w=AUTO;
    cw = containingBlockWidth()
    	+containingBlock()->paddingLeft() +containingBlock()->paddingRight();

    if(!m_style->left().isVariable())
	l = m_style->left().width(cw);
    if(!m_style->right().isVariable())
	r = m_style->right().width(cw);		
    if(!m_style->width().isVariable())
	w = m_style->width().width(cw);
    else if (isReplaced())
        w = intrinsicWidth();
    if(!m_style->marginLeft().isVariable())
	ml = m_style->marginLeft().width(cw);		
    if(!m_style->marginRight().isVariable())
	mr = m_style->marginRight().width(cw);


    // css2 spec 10.3.7 & 10.3.8
    // 1
    RenderObject* o=parent();
    if (style()->direction()==LTR && l==AUTO )
    {
	if (m_next) l = m_next->xPos();
	else if (m_previous) l = m_previous->xPos()+m_previous->contentWidth();
	else l=0;
	while (o && o!=containingBlock()) { l+=o->xPos(); o=o->parent(); }
    }

    // 2
    else if (style()->direction()==RTL && r==AUTO )
    {
    	if (m_previous) r = (m_previous->xPos() + m_previous->contentWidth());
	else if (m_next) r = m_next->xPos();
	else r=0;
        r += cw -
            (o->width()-o->borderLeft()-o->borderRight()-o->paddingLeft()-o->paddingRight());
	while (o && o!=containingBlock()) { r-=o->xPos(); o=o->parent(); }
    }
    
//    printf("h12: w=%d, l=%d, r=%d, ml=%d, mr=%d\n",w,l,r,ml,mr);

    // 3
    if (w==AUTO)
    {
    	if (l==AUTO) l=0;
	if (r==AUTO) r=0;
    };

//    printf("h3: w=%d, l=%d, r=%d, ml=%d, mr=%d\n",w,l,r,ml,mr);

    // 4
    if (w==AUTO || l==AUTO || r==AUTO)
    {
        if (ml==AUTO) ml=0;
        if (mr==AUTO) mr=0;
    }

//    printf("h4: w=%d, l=%d, r=%d, ml=%d, mr=%d\n",w,l,r,ml,mr);
    
    // 5
    if (ml==AUTO && mr==AUTO)
    {
        int ot = w + r + l + pab;
        ml = (cw - ot)/2;
        mr = cw - ot - ml;
    }

//    printf("h5: w=%d, l=%d, r=%d, ml=%d, mr=%d\n",w,l,r,ml,mr);

    // 6
    if (w==AUTO)
        w = cw - ( r + l + ml + mr + pab);
    if (l==AUTO)
        l = cw - ( r + w + ml + mr + pab);
    if (r==AUTO)
        r = cw - ( l + w + ml + mr + pab);
    if (ml==AUTO)
        ml = cw - ( r + l + w + mr + pab);
    if (mr==AUTO)
        mr = cw - ( r + l + w + ml + pab);

//    printf("h6: w=%d, l=%d, r=%d, ml=%d, mr=%d\n",w,l,r,ml,mr);
    
    // 7 
    if (cw != l + r + w + ml + mr + pab)
    {
        if (m_style->left().isVariable())
            l = cw - ( r + w + ml + mr + pab);
        else if (m_style->right().isVariable())    
            r = cw - ( l + w + ml + mr + pab);
        else if (style()->direction()==LTR)
            r = cw - ( l + w + ml + mr + pab);
        else 
            l = cw - ( r + w + ml + mr + pab);
    }

    m_width = w + pab;
    m_marginLeft = ml+l;
    m_marginRight = mr+r;
    m_x = l + ml + containingBlock()->borderLeft();
	
//    printf("h: w=%d, l=%d, r=%d, ml=%d, mr=%d\n",w,l,r,ml,mr);
}


void RenderBox::calcAbsoluteVertical()
{
    const int AUTO = -666666;
    int t,b,h,mt,mb,ch;

    t=b=h=mt=mb=AUTO;

    int pab = borderTop()+borderBottom()+paddingTop()+paddingBottom();

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
    {
	h = m_style->height().width(ch);
        // use real height if higher
        if (h < m_height - pab)
            h = m_height - pab;
    }
    else if (isReplaced())
        h = intrinsicHeight();
    
    if(!m_style->marginTop().isVariable())
	mt = m_style->marginTop().width(ch);		
    if(!m_style->marginBottom().isVariable())
	mb = m_style->marginBottom().width(ch);


    // css2 spec 10.6.4 & 10.6.5
    // 1
    RenderObject* o = parent();
    if (t==AUTO && (h==AUTO || b==AUTO))
    {
	if (m_next) t = m_next->yPos();
	else if (m_previous) t = m_previous->yPos()+m_previous->height();
	else t=0;	
	while (o && o!=containingBlock()) { t+=o->yPos(); o=o->parent(); }
    }

    // 2
    if (b==AUTO && h==AUTO)
    	b=0;

    // 3
    if (b==AUTO || h==AUTO || t==AUTO)
    {
    	if (mt==AUTO) mt=0;
        if (mb==AUTO) mb=0;
    }

    // 4
    if (mt==AUTO && mb==AUTO)
    {
        int ot = h + t + b + pab;
        mt = (ch - ot)/2;
        mb = ch - ot - mt;
    }

    // 5
    if (h==AUTO)
    	h = ch - ( t+b+mt+mb+pab);
    if (t==AUTO)
    	t = ch - ( h+b+mt+mb+pab);
    if (b==AUTO)
    	b = ch - ( h+t+mt+mb+pab);
    if (mt==AUTO)
    	mt = ch - ( h+t+b+mb+pab);
    if (mb==AUTO)
    	mb = ch - ( h+t+b+mt+pab);

    if (m_height<h+pab)
    	m_height = h+pab;
    
    // add the top and bottom distance to the value of margin.
    // in principle this is incorrect, but no one (except
    // the document height code, where it is useful) should 
    // care about it anyway
    m_marginTop = mt+t;
    m_marginBottom = mb+b;    
    m_y = t + mt + containingBlock()->borderTop();
	
//    printf("v: h=%d, t=%d, b=%d, mt=%d, mb=%d, m_y=%d\n",h,t,b,mt,mb,m_y);
    	
}


int RenderBox::lowestPosition() const
{
    return m_height + marginBottom();
}
