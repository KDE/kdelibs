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
#include <qlist.h>

#include "render_flow.h"
#include "render_text.h"

#include "htmlhashes.h"

#include "render_style.h"
#include "render_root.h"

#include <stdio.h>
#include <assert.h>

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
    if(a > 0 && b > 0) return (a > b ? a : b );
    if(a > 0 && b < 0) return a - b;
    if(a < 0 && b > 0) return b - a;
    return ( a > b ? -b : -a);
}


RenderFlow::RenderFlow(RenderStyle* style)
    : RenderBox(style)
{
    m_inline = true;
    m_childrenInline = true;
    m_haveAnonymous = false;

    specialObjects = 0;

    if(m_positioned || m_floating || !style->display() == INLINE)
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

    m_isAnonymous = false;

}

RenderFlow::~RenderFlow()
{
    if (specialObjects)
    	delete specialObjects;
}

void RenderFlow::print(QPainter *p, int _x, int _y, int _w, int _h,
				 int _tx, int _ty)
{

    if(!isInline())
    {
	_tx += m_x;
	_ty += m_y;
    }

    // check if we need to do anything at all...
    if(!isInline())
    {
	int h = m_height;
	if(specialObjects && floatBottom() > h) h = floatBottom();
	if((_ty > _y + _h) || (_ty + h < _y)) return;
    }

    printObject(p, _x, _y, _w, _h, _tx, _ty);
}

void RenderFlow::printObject(QPainter *p, int _x, int _y,
				       int _w, int _h, int _tx, int _ty)
{
#ifdef DEBUG_LAYOUT
    printf("%s(RenderFlow)::printObject() w/h = (%d/%d)\n", renderName(), width(), height());
#endif    


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
    	    if (!r->noPaint)
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
	if(!child->isFloating())
	{
	    printf("%d,%d",child->xPos(),child->yPos());
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
    Length w = m_style->width();
    m_width = containingBlockWidth() - marginLeft() - marginRight();
    //assert(containingBlock() != this);
    m_width = w.width(m_width);
    if(m_width < m_minWidth) m_width = m_minWidth;

#ifdef DEBUG_LAYOUT
    printf("RenderFlow::calcWidth(): m_width=%d containingBlockWidth()=%d\n", m_width, containingBlockWidth());
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
    assert(!isInline());

    int oldWidth = m_width;
    calcWidth();
    // ### does not give correct results for fixed width paragraphs with
    // floats for some reason
    if(oldWidth == m_width && layouted()) return;

#ifdef DEBUG_LAYOUT
    printf("%s(RenderFlow)::layout(%d) width=%d, layouted=%d\n", renderName(), deep, m_width, layouted());
    if(containingBlock() == static_cast<RenderObject *>(this))
    	printf("%s: containingBlock == this\n", renderName());
#endif

    if(m_width<=0) return;
    clearFloats();

    // Block elements usually just have descent.
    // ascent != 0 will give a separation.
    m_height = 0;
    m_clearStatus = CNONE;

//    printf("childrenInline()=%d\n",childrenInline());
    if(childrenInline())
	layoutInlineChildren();
    else
	layoutBlockChildren(deep);

    if(floatBottom() > m_height)	
    {
	if(isFloating())
	    m_height = floatBottom();
	else if(isTableCell())
	    m_height = floatBottom();
	else if( m_next)
	{
	    assert(!m_next->isInline());
	    m_next->setLayouted(false);
	    m_next->layout();
	}
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
/*    if (child->style()->htmlHacks() && child->containingBlock()->isTableCell())
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
    }*/
    if(marginLeft.type == Variable)
    {
	if(marginRight.type == Variable)
	    diff /= 2;
	//printf("indenting margin by %d\n", diff);
	return diff;
    }
    else
	return child->marginLeft();
}


void RenderFlow::layoutBlockChildren(bool deep)
{
#ifdef DEBUG_LAYOUT
    printf("layoutBlockChildren\n");
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
    while( child != 0 )
    {
//    	printf("loop %x, %d, %d\n",child, child->isInline(),child->layouted());
	if(checkClear(child)) prevMargin = 0; // ### should only be 0
	// if oldHeight+prevMargin < newHeight
	int margin = child->marginTop();
	margin = collapseMargins(margin, prevMargin);
	m_height += margin;

    	// html blocks flow around floats	
    	if (style()->htmlHacks() && child->style()->flowAroundFloats() ) 	
	    child->setPos(leftMargin(m_height) + getIndent(child), m_height);
	else
	    child->setPos(xPos + getIndent(child), m_height);

	if(deep) child->layout(deep);
	else if (!child->layouted())
	    _layouted = false;

	m_height += child->height();
	prevMargin = child->marginBottom();
	child = child->nextSibling();
    }

    m_height += prevMargin + toAdd;

    setLayouted(_layouted);

    // printf("layouted = %d\n", layouted_);
}

bool RenderFlow::checkClear(RenderObject *child)
{
    //printf("checkClear oldheight=%d\n", m_height);
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
    //printf("    newHeight = %d\n", m_height);
    return true;
}

void RenderFlow::layoutInlineChildren()
{
#ifdef DEBUG_LAYOUT
    printf("layoutInlineChildren\n");
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
RenderFlow::insertFloat(RenderObject *o)
{
    // a floating element
    if(!specialObjects) {
	specialObjects = new QList<SpecialObject>;
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

    f->noPaint=false;
    f->startY = -1;
    f->endY = -1;
    f->width = o->width() + o->marginLeft() + o->marginRight();
    if(o->style()->floating() == FLEFT)
	f->type = SpecialObject::FloatLeft;
    else
	f->type = SpecialObject::FloatRight;
    f->node = o;

    specialObjects->append(f);
//    printf("inserting node %p number of specialobject = %d\n", o,
//	   specialObjects->count());
	
    positionNewFloats();

    // html blocks flow around floats, to do this add floats to parent too
    if(style()->htmlHacks() && childrenInline())
    {
    	RenderObject* obj = parent();
     	while ( obj && obj->childrenInline() ) obj=obj->parent();
    	if (obj && obj->isFlow() && f->noPaint == false)
	{
	    RenderFlow* par = static_cast<RenderFlow*>(obj);

	    if(!par->specialObjects) {
		par->specialObjects = new QList<SpecialObject>;
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
	    so->noPaint = true;
	    par->specialObjects->append(so);
	}
    }
}

void RenderFlow::positionNewFloats()
{
    if(!specialObjects) return;
    SpecialObject *f = specialObjects->getLast();
    if(!f || f->startY != -1) return;
    while(1)
    {
	SpecialObject *aFloat = specialObjects->prev();
	if(!aFloat || aFloat->startY != -1) {
	    specialObjects->next();
	    break;
	}	
	f = aFloat;
    }

    int y;
    if(m_childrenInline)
        y = currentY();
    else
        y = m_height;

    while(f)
    {
	RenderObject *o = f->node;
	int _height = o->height() + o->marginTop() + o->marginBottom();
	
	if (f->noPaint)
	{
	    f = specialObjects->next();
	    continue;
	}

	if (o->style()->floating() == FLEFT)
	{
	    int fx = leftMargin(y);
	    if (rightMargin(y)-fx < f->width)
	    {
		fx=borderLeft();
		y=leftBottom()+1;
	    }
	    f->left = fx;
//	    printf("positioning left aligned float at (%d/%d)\n",	
//		   fx + o->marginLeft() , y + o->marginTop());	
	    o->setXPos(fx + o->marginLeft());			
	    o->setYPos(y + o->marginTop());
	}
	else
	{
	    int fx = rightMargin(y);
	    if (fx - leftMargin(y) < f->width)
	    {
		fx=m_width-borderRight();
		y=leftBottom()+1;
	    }
	    f->left = fx - f->width;
//	    printf("positioning right aligned float at (%d/%d)\n",
//		   fx - o->marginRight() - o->width(), y + o->marginTop());
	    o->setXPos(fx - o->marginRight() - o->width());
	    o->setYPos(y + o->marginTop());
	}	
	f->startY = y;
	f->endY = f->startY + _height;


	
//	printf("specialObject y= (%d-%d)\n",f->startY,f->endY);

	f = specialObjects->next();
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
    if(currentY() < newY)
    {
//	printf("adjusting y position\n");
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
//    	printf("left: sy, ey, x, w %d,%d,%d,%d \n",
//	    r->startY, r->endY, r->left, r->width);
	if (r->startY <= y && r->endY > y &&
	    r->type == SpecialObject::FloatLeft &&
	    r->left + r->width > left)
	    left = r->left + r->width;
    }
//    printf("leftMargin(%d) = %d\n", y, left);
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
//    	printf("right: sy, ey, x, w %d,%d,%d,%d \n",
//	    r->startY, r->endY, r->left, r->width);
	if (r->startY <= y && r->endY > y &&
	    r->type == SpecialObject::FloatRight &&
	    r->left < right)
	    right = r->left;
    }
//    printf("rightMargin(%d) = %d\n", y, right);
    return right;
}

unsigned short
RenderFlow::lineWidth(int y) const
{
//    printf("lineWidth(%d)=%d\n",y,rightMargin(y) - leftMargin(y));
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

//    printf("clearFloats\n");
    if (specialObjects)
    {
	specialObjects->clear();
    }

    RenderObject *prev = m_previous;
    int offset = 0;
    if(prev)
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

    if(!prev->isFlow()) return;
    RenderFlow * flow = static_cast<RenderFlow *>(prev);
    if(!flow->specialObjects) return;
    if(style()->htmlHacks() && style()->flowAroundFloats())
    	return; //html tables and lists flow as blocks
    if(flow->floatBottom() > offset)
    {
#ifdef DEBUG_LAYOUT
	printf("adding overhanging floats\n");
#endif
	
	// we have overhanging floats
	if(!specialObjects)
	{
	    specialObjects = new QList<SpecialObject>;
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
		special->noPaint = true; // previous paragraph paints it
		specialObjects->append(special);
#ifdef DEBUG_LAYOUT
		printf("    y: %d-%d left: %d width: %d\n", special->startY, special->endY, special->left, special->width);
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
//printf("aligned to baseline %d\n", r);
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
    printf("%s(RenderBox)::calcMinMaxWidth() known=%d\n", renderName(), minMaxKnown());
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
	
		
		if (child->isText())
		{	
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
    //printf("renderFlow::close()\n");
    setParsing(false);
    if(haveAnonymousBox())
    {
	m_last->close();
	//printf("RenderFlow::close(): closing anonymous box\n");
	setHaveAnonymousBox(false);
    }
    if(!isInline() && m_childrenInline)
    {
	layout();
    }
    else
    {
	//if(m_last)
	//    m_height += m_last->height() + m_last->marginBottom();
	if(m_style->hasBorder())
	    m_height += borderBottom();
	if(m_style->hasPadding())
	    m_height += paddingBottom();
	calcMinMaxWidth();
    }
//    if(containingBlockWidth() < m_minWidth && m_parent)
    if (!isAnonymousBox())
    	containingBlock()->updateSize();

#ifdef DEBUG_LAYOUT
    printf("%s(RenderFlow)::close() total height =%d\n", renderName(), m_height);
#endif
}

void RenderFlow::addChild(RenderObject *newChild)
{
#ifdef DEBUG_LAYOUT
    printf("%s(RenderFlow)::addChild( %s )\n", renderName(), newChild->renderName());
    printf("current height = %d\n", m_height);
#endif

    if(m_height == 0)
    {
	// to position the first element, we need the top
        if(m_style->hasBorder())
	    m_height += borderTop();
	if(m_style->hasPadding())
	    m_height += paddingTop();
    }

    int xPos = 0;
    if(m_style->hasBorder())
	xPos += borderLeft();
    if(m_style->hasPadding())
	xPos += paddingLeft();

    int margin = 0;

    if(m_last && !m_last->isInline() && !m_last->isFloating())
    {
    	if (!layouted())
	    m_height += m_last->height();
	margin = m_last->marginBottom();
	printf("last's margin = %d, last's height = %d\n", margin, m_last->height());
    }

    if(m_childrenInline && !newChild->isInline() && !newChild->isFloating())
    {
	// put all inline children we have up to now in a anonymous block box
	if(m_last)
	{
	    printf("no inline child, moving previous inline children!\n");
	    RenderStyle *newStyle = new RenderStyle(m_style);
	    newStyle->setDisplay(BLOCK);
	    RenderFlow *newBox = new RenderFlow(newStyle);
	    newBox->setIsAnonymousBox(true);
	    // ### the children have a wrong style!!!
	    // They get exactly the style of this element, not of the anonymous box
	    // might be important for bg colors!
	    newBox->setFirstChild(m_first);
	    newBox->setLastChild(m_last);
	    RenderObject *o = newBox->firstChild();
	    while(o)
	    {
		o->setParent(newBox);
		o = o->nextSibling();
	    }
	    newBox->setPos(xPos, m_height);
	    newBox->setParent(this);
	    m_first = m_last = newBox;
	    newBox->close();	    
	    newBox->layout();
	    printf("%d,%d,%d,%d,%d\n",newBox->paddingLeft(),newBox->marginLeft()
	    	,newBox->firstChild()->xPos(),newBox->xPos(),newBox->yPos());	    
	    m_height += newBox->height();
	}
	m_childrenInline = false;
    }
    else if(!m_childrenInline)
    {
	if(newChild->isInline() || newChild->isFloating())
	{
	    printf("adding inline child to anonymous box\n");
	    if(!haveAnonymousBox())
	    {
		printf("creating anonymous box\n");
		RenderStyle *newStyle = new RenderStyle(m_style);
		newStyle->setDisplay(BLOCK);
		RenderFlow *newBox = new RenderFlow(newStyle);
		newBox->setIsAnonymousBox(true);
		newBox->setPos(xPos, m_height);
		RenderObject::addChild(newBox);
		newBox->calcWidth();
		newBox->addChild(newChild);
		setHaveAnonymousBox();
		return;
	    }
	    else
	    {
		m_last->addChild(newChild);
		return;
	    }
	}
	else if(haveAnonymousBox())
	{
	    m_last->close();
	    m_last->layout();
	    m_height += m_last->height();
	    setHaveAnonymousBox(false);
	    printf("closing anonymous box\n");
	}
    }
    else if(!newChild->isInline() && !newChild->isFloating())
	m_childrenInline = false;

    if(!newChild->isInline() && !newChild->isFloating())
    {
	newChild->setParent(this);
	//printf("new child's margin = %d\n", newChild->marginTop());
	// ### FIXME
	//margin = collapseMargins(margin, newChild->marginTop());
	//### see comment in layoutBlockChildren
	if(checkClear(newChild)) margin = 0;
	margin = MAX(margin, newChild->marginTop());
	//printf("margin = %d\n", margin);
	m_height += margin;
	printf("positioning new block child at (%d/%d)\n", xPos, m_height);
	newChild->calcWidth();
	newChild->setPos(xPos + getIndent(newChild), m_height);
    }

    if(newChild->isFloating())
    {
    	insertFloat(newChild);
    }

    setLayouted(false);
    RenderObject::addChild(newChild);
    // ### care about aligned stuff
}

BiDiObject *RenderFlow::first()
{
    if(!m_first) return 0;
    RenderObject *o = m_first;

    if(o->isText()) static_cast<RenderText *>(o)->deleteSlaves();
    if(!o->isText() && !o->isReplaced() && !o->isFloating())
	o = static_cast<RenderObject *>(next(o));

    return o;
}

BiDiObject *RenderFlow::next(BiDiObject *c)
{
    if(!c) return 0;
    RenderObject *current = static_cast<RenderObject *>(c);

    while(current != 0)
    {
	//printf("current = %p\n", current);
	RenderObject *next = nextObject(current);

	if(!next) return 0;

	if(next->isText())
	{
	    static_cast<RenderText *>(next)->deleteSlaves();
	    return next;
	}
	else if(next->isFloating() || next->isReplaced())
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
    if(!current->isFloating() && !current->isReplaced())
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

    if(o->isFloating())
    {
	o->layout(true);
	insertFloat(o);
    }
    else if(o->isReplaced())
	o->layout(true);
    if( !o->isFloating() && (!o->isInline() || o->isBR()) )
    {
	//check the clear status
	EClear clear = o->style()->clear();
	if(clear != CNONE)
	{
	    //printf("setting clear to %d\n", clear);
	    m_clearStatus = (EClear) (m_clearStatus | clear);
	}	
    }
}
