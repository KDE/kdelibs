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

#include <stdio.h>
#include <assert.h>

#include "misc/loader.h"

using namespace DOM;
using namespace khtml;


RenderBox::RenderBox(RenderStyle* style)
    : RenderObject(style)
{
    m_minWidth = -1;
    m_maxWidth = -1;
    m_x = m_y = m_width = m_height = 0;

    CachedImage *i = style->backgroundImage();
    if(i)
    {
	i->ref(this);
    }

    switch(style->position())
    {
    case ABSOLUTE:
    case FIXED:
	m_positioned = true;
	break;
    default:
	if(style->isFloating())
	{
	    //printf("floating element\n");
	    m_floating = true;
	}
    }
}

RenderBox::~RenderBox()
{
    //printf("Element destructor: this=%s\n", nodeName().string().ascii());
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
    //printf("RenderBox::contentWidth(1) = %d\n", m_width);
    if(m_style->hasBorder())
	w -= borderLeft() + borderRight();
    if(m_style->hasPadding())
	w -= paddingLeft() + paddingRight();

    //printf("RenderBox::contentWidth(2) = %d\n", w);
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

void RenderBox::setPixmap(const QPixmap &)
{
    repaint();	//repaint bg when it gets loaded
}

void RenderBox::printBoxDecorations(QPainter *p,int _x, int _y,
				       int _w, int _h, int _tx, int _ty)
{
    //printf("renderBox::printDecorations()\n");

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
    CachedImage *i = m_style->backgroundImage();
    if(i)
    {
	// ### might need to add some correct offsets
	// ### use paddingX/Y
	p->drawTiledPixmap(_tx + borderLeft(), _ty + borderTop(), w, h, i->pixmap());
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

void RenderBox::outlineBox(QPainter *p, int _tx, int _ty)
{
    p->setPen(QPen(QColor("#ff0000"), 1, Qt::DotLine));
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
//    printf("positionelement::updateSize()\n");
    int oldMin = m_minWidth;
    setLayouted(false);
    calcMinMaxWidth();
    if(m_minWidth > containingBlockWidth() || m_minWidth != oldMin)
    {
    	//printf("parent id=%d\n",_parent->id());
	if(containingBlock() != this) containingBlock()->updateSize();
    }
    else if(!isInline() || isReplaced())
    {
	int oldHeight = m_height;
   	layout(true);	
	if(m_height != oldHeight)
	{
	    if(containingBlock() != this) containingBlock()->updateSize();
	} else {
	    repaint();
	}
    }
}

void RenderBox::position(int x, int y, int, int, int width, bool)
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
	//printf("aligned to baseline\n");
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
	printf("aligned to baseline\n");
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
    //printf("repaint!\n");
    repaintRectangle(0, 0, m_width, m_height);
}

void RenderBox::repaintRectangle(int x, int y, int w, int h)
{
    x += m_x;
    y += m_y;
    //printf("renderBox::repaintRectangle (%d/%d) (%d/%d)\n", x, y, w, h);
    if(m_parent) m_parent->repaintRectangle(x, y, w, h);
}

void RenderBox::repaintObject(RenderObject *o, int x, int y)
{
    x += m_x;
    y += m_y;
    if(m_parent) m_parent->repaintObject(o, x, y);
}
