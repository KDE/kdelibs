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
//#define BOX_DEBUG


#include "dom_string.h"

#include <qfontmetrics.h>
#include <qsortedlist.h>

#include "render_flow.h"
#include "render_text.h"

#include "htmlhashes.h"

#include "render_style.h"
#include "render_root.h"

#include <kdebug.h>
#include <assert.h>
#include <iostream.h>

using namespace DOM;
using namespace khtml;


static inline int MAX(int a, int b)
{
    return a > b ? a : b;
}

static inline int MIN(int a, int b)
{
    return a < b ? a : b;
}

static inline int collapseMargins(int a, int b)
{
    if(a >= 0 && b >= 0) return (a > b ? a : b );
    if(a > 0 && b < 0) return a + b;
    if(a < 0 && b > 0) return b + a;
    return ( a > b ? b : a);
}


RenderFlow::RenderFlow()
    : RenderBox()
{
    m_inline = true;
    m_childrenInline = true;
    m_haveAnonymous = false;

    specialObjects = 0;
}

void RenderFlow::setStyle(RenderStyle *style)
{

//    kdDebug( 6040 ) << (void*)this<< " renderFlow::setstyle()" << endl;

    RenderBox::setStyle(style);

    if(m_positioned || m_floating || !style->display() == INLINE)
	m_inline = false;
	
    if (m_inline == true && m_childrenInline==false)
    	m_inline = false;

    switch(m_style->textAlign())
    {
    case LEFT:
	setAlignment(AlignLeft);
	break;
    case RIGHT:
	setAlignment(AlignRight);
	break;
    case CENTER:
	setAlignment(AlignCenter);
	break;
    case JUSTIFY:
	setAlignment(AlignAuto);
    }

    setIgnoreNewline(false);
    if(m_style->whiteSpace() == PRE)
	setIgnoreLeadingSpaces(false);
    setVisualOrdering(m_style->visuallyOrdered());
    if(m_style->direction() == LTR)
	setBasicDirection(QChar::DirL);
    else
	setBasicDirection(QChar::DirR);
	
    if (haveAnonymousBox())
    {
	RenderObject *child = firstChild();
	while(child != 0)
	{
	    if(child->isAnonymousBox())
	    {
	    	if (child->style())
		    delete child->style();
		RenderStyle* newStyle = new RenderStyle(style);
		newStyle->setDisplay(BLOCK);
		child->setStyle(newStyle);
		child->setIsAnonymousBox(true);
	    }
	    child = child->nextSibling();
	}

    }
}

RenderFlow::~RenderFlow()
{
    if (specialObjects)
    	delete specialObjects;
    if(isAnonymousBox()) // usually the style object is deleted by the DOM tree, but
	// since anonymous boxes don't have a corresponding DOM element...
	delete m_style;
}

void RenderFlow::print(QPainter *p, int _x, int _y, int _w, int _h,
				 int _tx, int _ty)
{

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(RenderFlow) " << this << " ::print() x/y/w/h = ("  << xPos() << "/" << yPos() << "/" << width() << "/" << height()  << ")" << endl;
#endif

    if(!isInline())
    {
	_tx += m_x;
	_ty += m_y;
    }

    // check if we need to do anything at all...
    if(!isInline() && !containsPositioned() && !isRelPositioned() && !isPositioned() )
    {
	int h = m_height;
	if(specialObjects && floatBottom() > h) h = floatBottom();
	if((_ty > _y + _h) || (_ty + h < _y))
	{
//	    kdDebug( 6040 ) << "cut!" << endl;
	    return;
	}
    }

    printObject(p, _x, _y, _w, _h, _tx, _ty);
}

void RenderFlow::printObject(QPainter *p, int _x, int _y,
				       int _w, int _h, int _tx, int _ty)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(RenderFlow) " << this << " ::printObject() w/h = (" << width() << "/" << height() << ")" << endl;
#endif
    // add offset for relative positioning
    if(isRelPositioned())
	relativePositionOffset(_tx, _ty);

    // 1. print background, borders etc
    if(m_printSpecial && !isInline())
	printBoxDecorations(p, _x, _y, _w, _h, _tx, _ty);

    // 2. print floats and other non-flow objects
    if(specialObjects)
    {
	SpecialObject* r;	
	QListIterator<SpecialObject> it(*specialObjects);
	for ( ; (r = it.current()); ++it )
	{
    	    if (r->node->containingBlock()==this)
	    {
		RenderObject *o = r->node;	
		o->print(p, _x, _y, _w, _h, _tx , _ty);
	    }
	}
    }

    // 3. print contents
    RenderObject *child = firstChild();
    while(child != 0)
    {
	if(!child->isFloating() && !child->isPositioned())
	{
	    child->print(p, _x, _y, _w, _h, _tx, _ty);
	}
	child = child->nextSibling();
    }

#ifdef BOX_DEBUG
    outlineBox(p, _tx, _ty);
#endif

}

void RenderFlow::calcWidth()
{
    if (isPositioned())
    {
    	calcAbsoluteHorizontal();
    }
    else
    {
	Length w = m_style->width();
	if (w.type == Variable)
    	    m_width = containingBlockWidth() - marginLeft() - marginRight();
	else
	{
    	    m_width = w.width(containingBlockWidth());
	    m_width += paddingLeft() + paddingRight() + borderLeft() + borderRight();
	}
    }

    if(m_width < m_minWidth) m_width = m_minWidth;


#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << "RenderFlow::calcWidth(): m_width=" << m_width << " containingBlockWidth()=" << containingBlockWidth() << endl;
#endif
}


void RenderFlow::setPos( int xPos, int yPos )
{
    m_y = yPos;
    setXPos(xPos);
}

void RenderFlow::setXPos( int xPos )
{
    m_x = xPos;
}


void RenderFlow::layout( bool deep )
{
//    kdDebug( 6040 ) << renderName() << " " << this << "::layout() start" << endl;

    assert(!isInline());

    int oldWidth = m_width;
    calcWidth();
    // ### does not give correct results for fixed width paragraphs with
    // floats for some reason

    if (specialObjects==0)
    	if (oldWidth == m_width && layouted() && !isAnonymousBox()
	    && !containsPositioned() && !isPositioned()) return;
    else
    	if (nextSibling())
	    nextSibling()->setLayouted(false);
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(RenderFlow) " << this << " ::layout(" << deep << ") width=" << m_width << ", layouted=" << layouted() << endl;
    if(containingBlock() == static_cast<RenderObject *>(this))
    	kdDebug( 6040 ) << renderName() << ": containingBlock == this" << endl;
#endif

    if(m_width<=0) {
	if(m_y < 0) m_y = 0;
	setLayouted();
	return;
    }
    clearFloats();

    // Block elements usually just have descent.
    // ascent != 0 will give a separation.
    m_height = 0;
    m_clearStatus = CNONE;

//    kdDebug( 6040 ) << "childrenInline()=" << childrenInline() << endl;
    if(childrenInline())
	layoutInlineChildren();
    else
	layoutBlockChildren(deep);

    if (isPositioned())
    	calcAbsoluteVertical();
    else
    {
	Length h = style()->height();
	if (h.isFixed())
    	    m_height = MAX (h.value + borderTop() + paddingTop()
		+ borderBottom() + paddingBottom() , m_height);
	else if (h.isPercent())
	{
    	    Length ch = containingBlock()->style()->height();
	    if (ch.isFixed())
    		m_height = MAX (h.width(ch.value) + borderTop() + paddingTop()
	    	    + borderBottom() + paddingBottom(), m_height);
	}
    }

    if(floatBottom() > m_height)	
    {
	if(isFloating() || isTableCell())
	{
	    m_height = floatBottom();
	    m_height += borderBottom() + paddingBottom();
	}
	else if( m_next)	{
	    assert(!m_next->isInline());
	    m_next->setLayouted(false);
	    m_next->layout();
	}
    }

    if(specialObjects)
    {
	SpecialObject* r;	
	QListIterator<SpecialObject> it(*specialObjects);
	for ( ; (r = it.current()); ++it )
    	    if (r->type == SpecialObject::Positioned)
	    {
		r->node->layout(true);			
	    }
	specialObjects->sort();
    }


    setLayouted();
}

static int getIndent(RenderObject *child)
{
    int diff = child->containingBlockWidth() - child->width();
    if(diff <= 0) return 0;

    Length marginLeft = child->style()->marginLeft();
    Length marginRight = child->style()->marginRight();

    // ### hack to make <td align=> work. maybe it should be done with
    //	   css class selectors or something?
    if (child->style()->htmlHacks() && child->containingBlock()->isTableCell())
    {
    	if (child->containingBlock()->style()->textAlign()==RIGHT)
    	{
    	    marginLeft.type=Variable;
    	}
	else if (child->containingBlock()->style()->textAlign()==CENTER)
	{
	    marginLeft.type=Variable;
	    marginRight.type=Variable;
	}
    }
    if(marginLeft.type == Variable)
    {
	if(marginRight.type == Variable)
	    diff /= 2;
	//kdDebug( 6040 ) << "indenting margin by " << diff << endl;
	return diff;
    }
    else
	return child->marginLeft();
}


void RenderFlow::layoutBlockChildren(bool deep)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << "layoutBlockChildren" << endl;
#endif

    bool _layouted = true;
    int xPos = 0;
    int toAdd = 0;

    m_height = 0;

    if(m_style->hasBorder())
    {
	xPos += borderLeft();
	m_height += borderTop();
	toAdd += borderBottom();
    }
    if(m_style->hasPadding())
    {
	xPos += paddingLeft();
	m_height += paddingTop();
	toAdd += paddingBottom();
    }

    RenderObject *child = firstChild();
    int prevMargin = 0;
    if(isTableCell())
	prevMargin = -firstChild()->marginTop();
	
    while( child != 0 )
    {
//    	kdDebug( 6040 ) << "loop " << child << ", " << child->isInline() << ", " << child->layouted() << endl;

    	if (child->isPositioned())
	{
	    child->layout(true);
	    static_cast<RenderFlow*>(child->containingBlock())->insertPositioned(child);
	    child = child->nextSibling();
	    continue;
	}

	if(checkClear(child)) prevMargin = 0; // ### should only be 0
	// if oldHeight+prevMargin < newHeight
	int margin = child->marginTop();
	margin = collapseMargins(margin, prevMargin);
	
	m_height += margin;
	
	child->setYPos(m_height);

	if(deep) child->layout(deep);
	else if (!child->layouted())
	    _layouted = false;

    	// html blocks flow around floats	
    	if (style()->htmlHacks() && child->style()->flowAroundFloats() ) 	
	    child->setXPos(leftMargin(m_height) + getIndent(child));
	else
	    child->setXPos(xPos + getIndent(child));

	m_height += child->height();
	
	prevMargin = child->marginBottom();
	child = child->nextSibling();
    }

    if(!isTableCell()) m_height += prevMargin;
    m_height += toAdd;

    setLayouted(_layouted);

    // kdDebug( 6040 ) << "layouted = " << layouted_ << endl;
}

bool RenderFlow::checkClear(RenderObject *child)
{
    //kdDebug( 6040 ) << "checkClear oldheight=" << m_height << endl;
    RenderObject *o = child->previousSibling();
    while(o && !o->isFlow())
	o = o->previousSibling();
    if(!o) o = this;

    RenderFlow *prev = static_cast<RenderFlow *>(o);

    switch(child->style()->clear())
    {
    case CNONE:
	return false;
    case CLEFT:
    {
	int bottom = prev->leftBottom() + prev->yPos();
	if(m_height < bottom)
	    m_height = bottom; //###  + lastFloat()->marginBotton()?
	break;
    }
    case CRIGHT:
    {
	int bottom = prev->rightBottom() + prev->yPos();
	if(m_height < bottom)
	    m_height = bottom; //###  + lastFloat()->marginBotton()?
	break;
    }
    case CBOTH:
    {
	int bottom = prev->floatBottom() + prev->yPos();
	if(m_height < bottom)
	    m_height = bottom; //###  + lastFloat()->marginBotton()?
	break;
    }
    }
    //kdDebug( 6040 ) << "    newHeight = " << m_height << endl;
    return true;
}

void RenderFlow::layoutInlineChildren()
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << "layoutInlineChildren" << endl;
#endif
    int toAdd = 0;
    m_height = 0;

    if(m_style->hasBorder())
    {
	m_height = borderTop();
	toAdd = borderBottom();
    }
    if(m_style->hasPadding())
    {
	m_height += paddingTop();
	toAdd += paddingBottom();
    }
    if(m_first)
	m_height = reorder(0, m_height);
    m_height += toAdd;
}

void
RenderFlow::insertPositioned(RenderObject *o)
{
    if(!specialObjects) {
	specialObjects = new QSortedList<SpecialObject>;
	specialObjects->setAutoDelete(true);	
    }

    // don't insert it twice!
    QListIterator<SpecialObject> it(*specialObjects);
    SpecialObject* f;	
    while ( (f = it.current()) ) {
	if (f->node == o) return;
	++it;
    }

    if (!f) f = new SpecialObject;

    if (o->isRelPositioned())
    	f->type = SpecialObject::RelPositioned;
    else
        f->type = SpecialObject::Positioned;
    f->node = o;
    f->zindex = o->style()->zIndex();
    
    specialObjects->append(f);
}

void
RenderFlow::insertFloat(RenderObject *o)
{
    // a floating element
    if(!specialObjects) {
	specialObjects = new QSortedList<SpecialObject>;
	specialObjects->setAutoDelete(true);	
    }

    // don't insert it twice!
    QListIterator<SpecialObject> it(*specialObjects);
    SpecialObject* f;	
    while ( (f = it.current()) ) {
	if (f->node == o) return;
	++it;
    }


    if(!o->layouted())
    {
	o->layout(true);
    }

    if(!f) f = new SpecialObject;

    f->startY = -1;
    f->endY = -1;
    f->width = o->width() + o->marginLeft() + o->marginRight();
    if(o->style()->floating() == FLEFT)
	f->type = SpecialObject::FloatLeft;
    else
	f->type = SpecialObject::FloatRight;
    f->node = o;

    specialObjects->append(f);
//    kdDebug( 6040 ) << "inserting node " << o << " number of specialobject = " << //	   specialObjects->count() << endl;
	
    positionNewFloats();

    // html blocks flow around floats, to do this add floats to parent too
    if(style()->htmlHacks() && childrenInline() )
    {
    	RenderObject* obj = parent();
     	while ( obj && obj->childrenInline() ) obj=obj->parent();
    	if (obj && obj->isFlow() )
	{
	    RenderFlow* par = static_cast<RenderFlow*>(obj);
	
	    if (par->isFloating())
	    	return;

	    if(!par->specialObjects) {
		par->specialObjects = new QSortedList<SpecialObject>;
		par->specialObjects->setAutoDelete(true);	
	    }
	
	    QListIterator<SpecialObject> it(*par->specialObjects);
	    SpecialObject* tt;	
	    while ( (tt = it.current()) ) {
		if (tt->node == o) return;
		++it;
	    }	
	
	    SpecialObject* so = new SpecialObject(*f);
	    so->startY = so->startY + m_y;
	    so->endY = so->endY + m_y;
	    par->specialObjects->append(so);
	}
    }
}

void RenderFlow::positionNewFloats()
{
    if(!specialObjects) return;
    SpecialObject *f = specialObjects->getLast();
    if(!f || f->startY != -1) return;
    SpecialObject *lastFloat;
    while(1)
    {
	lastFloat = specialObjects->prev();
	if(!lastFloat || lastFloat->startY != -1) {
	    specialObjects->next();
	    break;
	}	
	f = lastFloat;
    }

    int y;
    if(m_childrenInline)
        y = currentY();
    else
        y = m_height;

    // the float can not start above the y position of the last positioned float.
    if(lastFloat && lastFloat->startY > y)
	y = lastFloat->startY;

    while(f)
    {
	RenderObject *o = f->node;
	int _height = o->height() + o->marginTop() + o->marginBottom();
	
	if (f->node->containingBlock()!=this)
	{
	    f = specialObjects->next();
	    continue;
	}

	if (o->style()->floating() == FLEFT)
	{
	    int fx = leftMargin(y);
	    if (contentWidth() >= f->width)
	    {
	    	while (rightMargin(y)-fx < f->width)
	    	{		
		    y++;
		    fx = leftMargin(y);
	    	}
	    }
	    f->left = fx;
//	    kdDebug( 6040 ) << "positioning left aligned float at (" << //		   fx + o->marginLeft()  << "/" << y + o->marginTop() << ")" << endl;
	    o->setXPos(fx + o->marginLeft());			
	    o->setYPos(y + o->marginTop());
	}
	else
	{
	    int fx = rightMargin(y);
	    if (contentWidth() >= f->width)
	    {
	    	while (fx - leftMargin(y) < f->width)
	    	{
		    y++;
		    fx = rightMargin(y);
	    	}
	    }
	    f->left = fx - f->width;
//	    kdDebug( 6040 ) << "positioning right aligned float at (" << //		   fx - o->marginRight() - o->width() << "/" << y + o->marginTop() << ")" << endl;
	    o->setXPos(fx - o->marginRight() - o->width());
	    o->setYPos(y + o->marginTop());
	}	
	f->startY = y;
	f->endY = f->startY + _height;


	
//	kdDebug( 6040 ) << "specialObject y= (" << f->startY << "-" << f->endY << ")" << endl;

	f = specialObjects->next();
    }
}

void RenderFlow::newLine()
{
    positionNewFloats();
    // set y position
    unsigned int newY = 0;
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
    if(currentY() < newY)
    {
//	kdDebug( 6040 ) << "adjusting y position" << endl;
	setCurrentY(newY);
    }
    m_clearStatus = CNONE;
}


short
RenderFlow::leftMargin(int y) const
{
    int left = 0;

    if(m_style->hasBorder())
	left = borderLeft();
    if(m_style->hasPadding())
	left += paddingLeft();

    if(!specialObjects) return left;

    SpecialObject* r;	
    QListIterator<SpecialObject> it(*specialObjects);
    for ( ; (r = it.current()); ++it )
    {
//    	kdDebug( 6040 ) << "left: sy, ey, x, w " << //	    r->startY << "," << r->endY << "," << r->left << "," << r->width << " " << endl;
	if (r->startY <= y && r->endY > y &&
	    r->type == SpecialObject::FloatLeft &&
	    r->left + r->width > left)
	    left = r->left + r->width;
    }
//    kdDebug( 6040 ) << "leftMargin(" << y << ") = " << left << endl;
    return left;
}

int
RenderFlow::rightMargin(int y) const
{
    int right = m_width;

    if(m_style->hasBorder())
	right -= borderRight();
    if(m_style->hasPadding())
	right -= paddingRight();

    if (!specialObjects) return right;

    SpecialObject* r;	
    QListIterator<SpecialObject> it(*specialObjects);
    for ( ; (r = it.current()); ++it )
    {
//    	kdDebug( 6040 ) << "right: sy, ey, x, w " << //	    r->startY << "," << r->endY << "," << r->left << "," << r->width << " " << endl;
	if (r->startY <= y && r->endY > y &&
	    r->type == SpecialObject::FloatRight &&
	    r->left < right)
	    right = r->left;
    }
//    kdDebug( 6040 ) << "rightMargin(" << y << ") = " << right << endl;
    return right;
}

unsigned short
RenderFlow::lineWidth(int y) const
{
//    kdDebug( 6040 ) << "lineWidth(" << y << ")=" << rightMargin(y) - leftMargin(y) << endl;
    return rightMargin(y) - leftMargin(y);

}

int
RenderFlow::floatBottom()
{
    if (!specialObjects) return 0;
    int bottom=0;
    SpecialObject* r;	
    QListIterator<SpecialObject> it(*specialObjects);
    for ( ; (r = it.current()); ++it )
	if (r->endY>bottom && r->type <= SpecialObject::FloatRight)
	    bottom=r->endY;
    return bottom;
}

int
RenderFlow::lowestPosition()
{
    if (!specialObjects) return m_height;
    int bottom=m_height;
    SpecialObject* r;	
    QListIterator<SpecialObject> it(*specialObjects);
    for ( ; (r = it.current()); ++it )
	if (r->endY>bottom)
	    bottom=r->endY;
    return bottom;
}

int
RenderFlow::leftBottom()
{
    if (!specialObjects) return 0;
    int bottom=0;
    SpecialObject* r;	
    QListIterator<SpecialObject> it(*specialObjects);
    for ( ; (r = it.current()); ++it )
	if (r->endY>bottom && r->type == SpecialObject::FloatLeft)
	    bottom=r->endY;

    return bottom;
}

int
RenderFlow::rightBottom()
{
    if (!specialObjects) return 0;
    int bottom=0;
    SpecialObject* r;	
    QListIterator<SpecialObject> it(*specialObjects);
    for ( ; (r = it.current()); ++it )
	if (r->endY>bottom && r->type == SpecialObject::FloatRight)
	    bottom=r->endY;

    return bottom;
}

void
RenderFlow::clearFloats()
{

//    kdDebug( 6040 ) << "clearFloats" << endl;
    if (specialObjects)
    {
	specialObjects->clear();
    }

    RenderObject *prev = m_previous;

    while (prev && prev->isTable())
	    prev = prev->previousSibling();

    int offset = 0;
    if(prev )
    {
	if(prev->isTableCell()) return;
	// ### FIXME
	//offset = m_previous->height() + collapseMargins(prev->marginBottom(), marginTop());
	offset = m_y - prev->yPos();
    }
    else
    {
	prev = m_parent;
	if(!prev) return;
	offset = m_y;
    }

    // add overhanging special objects from the previous RenderFlow
    if (isFloating()) return;
    if(!prev->isFlow()) return;
    RenderFlow * flow = static_cast<RenderFlow *>(prev);
    if(!flow->specialObjects) return;
    if(style()->htmlHacks() && style()->flowAroundFloats())
    	return; //html tables and lists flow as blocks

    if(flow->floatBottom() > offset)
    {
#ifdef DEBUG_LAYOUT
	kdDebug( 6040 ) << this << ": adding overhanging floats" << endl;
#endif
	
	// we have overhanging floats
	if(!specialObjects)
	{
	    specialObjects = new QSortedList<SpecialObject>;
	    specialObjects->setAutoDelete(true);	
	}
	
	QListIterator<SpecialObject> it(*flow->specialObjects);
	SpecialObject *r;
	for ( ; (r = it.current()); ++it )
	{
	    if (r->endY > offset && r->type <= SpecialObject::FloatRight)
	    {
		// we need to add the float here too
		SpecialObject *special = new SpecialObject;
		special->startY = r->startY - offset;
		special->endY = r->endY - offset;
		special->left = r->left; // ### the object might have different m,p&b
		special->width = r->width;
		special->node = r->node;
		special->type = r->type;
		specialObjects->append(special);
#ifdef DEBUG_LAYOUT
		kdDebug( 6040 ) << "    y: " << special->startY << "-" << special->endY << " left: " << special->left << " width: " << special->width << endl;
#endif
	    }
	}
    }
}


short RenderFlow::baselineOffset() const
{
    switch(vAlign())
    {
    case BASELINE:
    	{	
	int r = 0;
	if (firstChild())
	    r = firstChild()->yPos() + firstChild()->baselineOffset();
//kdDebug( 6040 ) << "aligned to baseline " << r << endl;
	return r;
	}	
    case SUB:
	// ###
    case SUPER:
	// ###
    case TOP:
	return 0;
    case TEXT_TOP:
	return QFontMetrics(m_style->font()).ascent();
    case MIDDLE:
	return contentHeight()/2;
    case BOTTOM:
	return contentHeight();
    case TEXT_BOTTOM:
	return contentHeight() - QFontMetrics(m_style->font()).descent();
    }
    return 0;
}


void RenderFlow::calcMinMaxWidth()
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(RenderBox)::calcMinMaxWidth() known=" << minMaxKnown() << endl;
#endif

    // non breaking space
    const QChar nbsp = 0xa0;

    if(minMaxKnown()) return;

    m_minWidth = 0;
    m_maxWidth = 0;

    RenderObject *child = firstChild();
    if(childrenInline())
    {
	int inlineMax=0;
	int inlineMin=0;
	bool noBreak=false;

	while(child != 0)
	{
	    if( !child->isBR() )
	    {
		// we have to take care about nbsp's, and places were
		// we can't break between two inline objects...
		// But for the moment, this will do...
	
		// mostly done -antti
	
		
		if (child->isText() && static_cast<RenderText *>(child)->length() > 0)
		{	
		    child->calcMinMaxWidth();
		    bool hasNbsp=false;
		    RenderText* t = static_cast<RenderText *>(child);
		    if (t->data()[0] == nbsp) //inline starts with nbsp
		    {
			inlineMin += child->minWidth();
			inlineMax += child->maxWidth();
			hasNbsp = true;
		    }
		    if (hasNbsp && t->data()[t->length()-1]==nbsp)
		    {   	    	    	//inline starts and ends with nbsp
			noBreak=true;
		    }
		    else if (t->data()[t->length()-1] == nbsp)
		    {   	    	    	//inline only ends with nbsp
			int w = child->minWidth();
			if(inlineMin < w) inlineMin = w;
			w = child->maxWidth();
			inlineMax += w;
			noBreak = true;
			hasNbsp = true;
		    }
		    if (hasNbsp)
		    {
			child = child->nextSibling();
			hasNbsp = false;		
			continue;
		    }
		}
		if (noBreak)
		{
		    inlineMin += child->minWidth();
		    inlineMax += child->maxWidth();
		    noBreak = false;
		}
		else	
		{	
		    int w = child->minWidth();
		    if(inlineMin < w) inlineMin = w;
		    w = child->maxWidth();
		    inlineMax += w;	
		
		}	
	    }
	    else
	    {
		if(m_minWidth < inlineMin) m_minWidth = inlineMin;
		if(m_maxWidth < inlineMax) m_maxWidth = inlineMax;
		inlineMin = inlineMax = 0;
	    }
	    child = child->nextSibling();
	}
	if(m_minWidth < inlineMin) m_minWidth = inlineMin;
	if(m_maxWidth < inlineMax) m_maxWidth = inlineMax;
    }
    else
    {
	while(child != 0)
	{
	    if(!child->minMaxKnown())
		child->calcMinMaxWidth();
	    int margin = child->marginLeft() + child->marginRight();
	    int w = child->minWidth() + margin;
	    if(m_minWidth < w) m_minWidth = w;
	    w = child->maxWidth() + margin;
	    if(m_maxWidth < w) m_maxWidth = w;
	    child = child->nextSibling();
	}
    }
    if(m_maxWidth < m_minWidth) m_maxWidth = m_minWidth;

    int toAdd = 0;
    if(m_style->hasBorder())
	toAdd = borderLeft() + borderRight();
    if(m_style->hasPadding())
	toAdd += paddingLeft() + paddingRight();

    m_minWidth += toAdd;
    m_maxWidth += toAdd;
}

void RenderFlow::close()
{
//    kdDebug( 6040 ) << (void*)this<< " renderFlow::close()" << endl;
    if(haveAnonymousBox())
    {
	m_last->close();
	//kdDebug( 6040 ) << "RenderFlow::close(): closing anonymous box" << endl;
	setHaveAnonymousBox(false);
    }
    if(!isInline() && m_childrenInline)
    {
	layout();
    }
    else
    {
	calcMinMaxWidth();
    }
    if(containingBlockWidth() < m_minWidth && m_parent)
    	containingBlock()->updateSize();
    else
    	containingBlock()->updateHeight();

    setParsing(false);

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(RenderFlow)::close() total height =" << m_height << endl;
#endif
}

void RenderFlow::addChild(RenderObject *newChild, RenderObject *beforeChild)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(RenderFlow)::addChild( " << newChild->renderName() <<
                       ", " << (beforeChild ? beforeChild->renderName() : "0") << " )" << endl;
    kdDebug( 6040 ) << "current height = " << m_height << endl;
#endif

    bool nonInlineInChild = false;

    if (beforeChild && beforeChild->parent() != this) {
	// perhaps beforeChild is inside an anonymous box that is our child
	if (!newChild->isInline() && !newChild->isFloating() && beforeChild->parent() &&
	    beforeChild->parent()->isAnonymousBox() && beforeChild->parent()->parent() == this)
	    nonInlineInChild = true;
	else {
	    beforeChild->parent()->addChild(newChild,beforeChild);
	    return;
	}
    }

    //to prevents non-layouted elements from getting printed
    if (!newChild->isInline() && !newChild->isFloating())
    {
    	newChild->setYPos(-500000);
    }

    if (!newChild->isText())
    {
	switch (newChild->style()->position())
	{
	    case RELATIVE:
	    case FIXED:	
    		setContainsPositioned(true);
		break;
	    case ABSOLUTE:
	    {
//		kdDebug( 6040 ) << "absolute found" << endl;
		setContainsPositioned(true);	
    		RenderObject::addChild(newChild,beforeChild);
		return;
	    }
	    default: ;

	}
    }


    if((m_childrenInline && !newChild->isInline() && !newChild->isFloating()) ||
       nonInlineInChild)
    {
	RenderObject *boxSource;
	if (nonInlineInChild)
	    boxSource = beforeChild->parent();
	else
	    boxSource = this;
	// put all inline children from boxSource in two anonymous block boxes -
	// one containing those before beforeChild, and one containing beforeChild and after
	if(boxSource->lastChild())
	{
//	    kdDebug( 6040 ) << "no inline child, moving previous inline children!" << endl;
	    RenderFlow *beforeBox = 0;
	
	    if(beforeChild != boxSource->firstChild()) {
		RenderStyle *newStyle = new RenderStyle(boxSource->style());
		newStyle->setDisplay(BLOCK);
	
		beforeBox = new RenderFlow();
		beforeBox->setStyle(newStyle);
		beforeBox->setIsAnonymousBox(true);
		// ### the children have a wrong style!!!
		// They get exactly the style of this element, not of the anonymous box
		// might be important for bg colors!
		beforeBox->setFirstChild(boxSource->firstChild());
		RenderObject *beforeBoxLast;
		if (beforeChild)
		    beforeBoxLast = beforeChild->previousSibling();
		else
		    beforeBoxLast = boxSource->lastChild();
		beforeBoxLast->setNextSibling(0);
		beforeBox->setLastChild(beforeBoxLast);
		RenderObject *o = beforeBox->firstChild();
		while(o) {
		    o->setParent(beforeBox);
		    o = o->nextSibling();
		}
		beforeBox->setParent(boxSource);
		boxSource->setFirstChild(beforeBox);
		boxSource->setLastChild(beforeBox);
		beforeBox->close();
		beforeBox->setYPos(-100000);
		beforeBox->setLayouted(false);
	    }
	    if (beforeChild) {
		RenderFlow *afterBox = new RenderFlow();
		afterBox = new RenderFlow();
		RenderStyle *newStyle = new RenderStyle(boxSource->style());
		newStyle->setDisplay(BLOCK);
		afterBox->setStyle(newStyle);
		afterBox->setIsAnonymousBox(true);
		// ### the children have a wrong style!!!
		// They get exactly the style of this element, not of the anonymous box
		// might be important for bg colors!
		beforeChild->setPreviousSibling(0);
		afterBox->setFirstChild(beforeChild);
		RenderObject *o = afterBox->firstChild();
		while(o)
		{
		    // a bit hacky, but should work
		    afterBox->setLastChild(o);
		    o->setParent(afterBox);
		    o = o->nextSibling();
		}
		afterBox->setParent(boxSource);
		boxSource->setLastChild(afterBox);
		if(beforeBox)
		  beforeBox->setNextSibling(afterBox);
		else
		  boxSource->setFirstChild(afterBox);
		afterBox->setPreviousSibling(beforeBox);
		afterBox->close();
		afterBox->setYPos(-100000);
		afterBox->setLayouted(false);
		beforeChild = afterBox;
	    }
	    if (nonInlineInChild) {
		boxSource->setLayouted(false);
		// boxSource will now contain up to two anonymous boxes - move
		// them into this in place of boxSource
		boxSource->firstChild()->setParent(this);
		boxSource->firstChild()->setPreviousSibling(boxSource->previousSibling());
		if (boxSource->previousSibling())
		    boxSource->previousSibling()->setNextSibling(boxSource->firstChild());
		
		boxSource->lastChild()->setParent(this);
		boxSource->lastChild()->setNextSibling(boxSource->nextSibling());
		if (boxSource->nextSibling())
		    boxSource->nextSibling()->setPreviousSibling(boxSource->lastChild());
		
		if (m_first == boxSource)
		    m_first = boxSource->firstChild();
		if (m_last == boxSource)
		    m_last = boxSource->lastChild();
		
		// make sure boxSource doesn't muck other objects up when deleted
		boxSource->setFirstChild(0);
		boxSource->setLastChild(0);
		boxSource->setPreviousSibling(0);
		boxSource->setNextSibling(0);
		delete boxSource;
		// ### what happens with boxSource's bg image if it had one?
	    }

	}
	m_childrenInline = false;
    }
    else if(!m_childrenInline)
    {
	if(newChild->isInline() || newChild->isFloating())
	{
	    // #### this won't work with beforeChild != 0 !!!!
	    if (beforeChild && beforeChild->previousSibling() && beforeChild->previousSibling()->isAnonymousBox()) {
		beforeChild->previousSibling()->addChild(newChild);
		setLayouted(false);
		return;
	    }
	
//	    kdDebug( 6040 ) << "adding inline child to anonymous box" << endl;
	    if(!haveAnonymousBox())
	    {
		//kdDebug( 6040 ) << "creating anonymous box" << endl;
		RenderStyle *newStyle = new RenderStyle(m_style);
		newStyle->setDisplay(BLOCK);
		RenderFlow *newBox = new RenderFlow();
		newBox->setStyle(newStyle);
		newBox->setIsAnonymousBox(true);
		RenderObject::addChild(newBox,beforeChild);
		newBox->addChild(newChild);
		newBox->setYPos(-100000);	
		setHaveAnonymousBox();
		return;
	    }
	    else
	    {
		//kdDebug( 6040 ) << "adding to last box" << endl;
		m_last->addChild(newChild); // ,beforeChild ???
		return;
	    }
	}
	else if(haveAnonymousBox())
	{
	    m_last->close();
	    m_last->layout();
	    setHaveAnonymousBox(false);
//	    kdDebug( 6040 ) << "closing anonymous box" << endl;
	}
    }
    else if(!newChild->isInline() && !newChild->isFloating())
    {
	m_childrenInline = false;
    }

    if(!newChild->isInline() && !newChild->isFloating())
    {
	newChild->setParent(this);
	if (style()->display() == INLINE)
	{
    	    m_inline=false; // inline can't contain blocks
	    if (parent() && parent()->isFlow())
		static_cast<RenderFlow*>(parent())->makeChildrenNonInline();
	}
    }

    setLayouted(false);
    RenderObject::addChild(newChild,beforeChild);
    // ### care about aligned stuff
}

void RenderFlow::makeChildrenNonInline()
{
// Put all inline children into anonymous block boxes
// ### should we call this all the way up to the top of the tree?

    m_childrenInline = false;

    RenderObject *child = m_first;
    RenderObject *next;
    RenderObject *boxFirst = m_first;
    RenderObject *boxLast = m_first;
    while (child) {
	next = child->nextSibling();

	if (child->isInline() || child->isFloating()) {
	    boxLast = child;
	}

	if ((!child->isInline() && !child->isFloating() && boxFirst != child) ||
            (!next && (boxFirst->isInline() || boxFirst->isFloating()))) {
	
	    // make anon box of those before child
	    RenderStyle *newStyle = new RenderStyle(style());
	    newStyle->setDisplay(BLOCK);

	    RenderFlow *box = new RenderFlow();
	    box->setStyle(newStyle);
	    box->setIsAnonymousBox(true);
	    // ### the children have a wrong style!!!
	    // They get exactly the style of this element, not of the anonymous box
	    // might be important for bg colors!
	    box->setPreviousSibling(boxFirst->previousSibling());
	    if (boxFirst->previousSibling())
		boxFirst->previousSibling()->setNextSibling(box);
	    boxFirst->setPreviousSibling(0);

	    box->setNextSibling(boxLast->nextSibling());
	    if (boxLast->nextSibling())
		boxLast->nextSibling()->setPreviousSibling(box);
	    boxLast->setNextSibling(0);

	    if (m_first == boxFirst)
		m_first = box;
	    if (m_last == boxLast)
		m_last = box;

	    box->setFirstChild(boxFirst);
	    box->setLastChild(boxLast);

	    RenderObject *o = box->firstChild();
	    while(o) {
		o->setParent(box);
		o = o->nextSibling();
	    }
	    box->setParent(this);

	    box->close();
	    box->setYPos(-100000);
	    box->setLayouted(false);
	}
	
	if (!child->isInline() && !child->isFloating())
	    boxFirst = boxLast = next;
	
	child = next;
    }
    setLayouted(false);
}

BiDiObject *RenderFlow::first()
{
    if(!m_first) return 0;
    RenderObject *o = m_first;

    if(o->isText()) static_cast<RenderText *>(o)->deleteSlaves();
    if(!o->isText() && !o->isReplaced() && !o->isFloating() && !o->isPositioned())
	o = static_cast<RenderObject *>(next(o));

    return o;
}

BiDiObject *RenderFlow::next(BiDiObject *c)
{
    if(!c) return 0;
    RenderObject *current = static_cast<RenderObject *>(c);

    while(current != 0)
    {
	//kdDebug( 6040 ) << "current = " << current << endl;
	RenderObject *next = nextObject(current);

	if(!next) return 0;

	if(next->isText())
	{
	    static_cast<RenderText *>(next)->deleteSlaves();
	    return next;
	}
	else if(next->isFloating() || next->isReplaced() || next->isPositioned())
	{
	    return next;
	}
	current = next;
    }
    return 0;
}

RenderObject *RenderFlow::nextObject(RenderObject *current)
{
    RenderObject *next = 0;
    if(!current->isFloating() && !current->isReplaced() && !current->isPositioned())
	next = current->firstChild();
    if(next) return next;

    while(current && current != static_cast<RenderObject *>(this))
    {
	next = current->nextSibling();
	if(next) return next;
	current = current->parent();
    }
    return 0;
}

void RenderFlow::specialHandler(BiDiObject *special)
{
    RenderObject *o = static_cast<RenderObject *>(special);
//    kdDebug( 6040 ) << "specialHandler" << endl;
    if(o->isFloating())
    {
	o->layout(true);
	insertFloat(o);
    }
    else if(o->isPositioned())
    {
	static_cast<RenderFlow*>(o->containingBlock())->insertPositioned(o);
    }	
    else if(o->isReplaced())
	o->layout(true);

	
    if( !o->isPositioned() && !o->isFloating() &&
    	(!o->isInline() || o->isBR()) )
    {
	//check the clear status
	EClear clear = o->style()->clear();
	if(clear != CNONE)
	{
	    //kdDebug( 6040 ) << "setting clear to " << clear << endl;
	    m_clearStatus = (EClear) (m_clearStatus | clear);
	}	
    }
}

void RenderFlow::absolutePosition(int &xPos, int &yPos)
{
    if(m_parent)
    {
	m_parent->absolutePosition(xPos, yPos);
	if(!isInline() && xPos != -1)
	    xPos += m_x, yPos += m_y;
    }
    else
	xPos = yPos = -1;
}

