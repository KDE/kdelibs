/**
 * This file is part of the HTML widget for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
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
#include "render_root.h"

#include "khtmlview.h"
#include <kdebug.h>
using namespace khtml;

RenderRoot::RenderRoot(KHTMLView *view)
    : RenderFlow()
{
    m_view = view;
    // try to contrain the width to the views width
    m_minWidth = view->visibleWidth();
    m_width = m_minWidth;
    m_maxWidth = m_minWidth;
    m_height = view->visibleHeight();

    m_positioned=true; // to 0,0 :)
    printingMode = false;

    selectionStart = 0;
    selectionEnd = 0;
    selectionStartPos = -1;
    selectionEndPos = -1;
    setParsing();
}

RenderRoot::~RenderRoot()
{
    kdDebug( 6090 ) << "renderRoot desctructor called" << endl;
}

void RenderRoot::calcWidth()
{

    // the width gets set by KHTMLView::print when printing to a printer.
    if(printingMode) return;

    m_width = m_view->frameWidth() + paddingLeft() + paddingRight() + borderLeft() + borderRight();

    if(m_width < m_minWidth) m_width = m_minWidth;

    if (style()->marginLeft().type==Fixed)
        m_marginLeft = style()->marginLeft().value;
    else
        m_marginLeft = 0;

    if (style()->marginRight().type==Fixed)
        m_marginRight = style()->marginRight().value;
    else
        m_marginRight = 0;
}

void RenderRoot::calcMinMaxWidth()
{
    RenderFlow::calcMinMaxWidth();
    if(m_maxWidth != m_minWidth) m_maxWidth = m_minWidth;
}

void RenderRoot::layout()
{
    //kdDebug(6040) << "RenderRoot::layout()" << endl;

    calcMinMaxWidth();
    calcWidth();

    // this fixes frameset resizing
    if(firstChild()) {
    	firstChild()->setLayouted(false);
    }
    RenderFlow::layout();

    m_height = m_view->visibleHeight();
    m_width = m_view->visibleWidth();
    //kdDebug(0) << "visibleHeight = " << contentsHeight << endl;

    layoutSpecialObjects();

    //kdDebug(0) << "root: height = " << m_height << endl;
}

QScrollView *RenderRoot::view()
{
    return m_view;
}

void RenderRoot::absolutePosition(int &xPos, int &yPos)
{
    xPos = yPos = 0;
}

void RenderRoot::print(QPainter *p, int _x, int _y, int _w, int _h, int _tx, int _ty)
{
    printObject(p, _x, _y, _w, _h, _tx, _ty);
}

void RenderRoot::printObject(QPainter *p, int _x, int _y,
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

    // 2. print contents
    RenderObject *child = firstChild();
    while(child != 0) {
	if(!child->isFloating() && !child->isPositioned()) {
	    child->print(p, _x, _y, _w, _h, _tx, _ty);
	}
	child = child->nextSibling();
    }

    // 3. print floats and other non-flow objects.
    // we have to do that after the contents otherwise they would get obscured by background settings.
    // it is anyway undefined if regular text is above fixed objects or the other way round.
    _tx += m_view->contentsX();
    _ty += m_view->contentsY();
    
    if(specialObjects)
    {
	SpecialObject* r;	
	QListIterator<SpecialObject> it(*specialObjects);
	for ( ; (r = it.current()); ++it )
	{
    	    if (r->node->containingBlock()==this)
	    {
		RenderObject *o = r->node;	
		//kdDebug(0) << "printing positioned at " << _tx + o->xPos() << "/" << _ty + o->yPos()<< endl;
		o->print(p, _x, _y, _w, _h, _tx , _ty);
	    }
	}
    }

#ifdef BOX_DEBUG
    outlineBox(p, _tx, _ty);
#endif

}


void RenderRoot::repaintRectangle(int x, int y, int w, int h)
{
    x += m_x;
    y += m_y;
    //kdDebug( 6040 ) << "updating views contents (" << x << "/" << y << ") (" << w << "/" << h << ")" << endl;
    if (m_view) m_view->updateContents(x, y, w, h);
}

void RenderRoot::repaint()
{
    if (m_view) m_view->updateContents(0, 0, m_width, m_height);
}

void RenderRoot::repaintObject(RenderObject *o, int x, int y)
{
    x += m_x;
    y += m_y;

    if (m_view) m_view->paintElement(o, x, y);
}

void RenderRoot::updateSize()
{
    //kdDebug( 6040 ) << renderName() << "(RenderRoot)::updateSize()" << endl;
    setMinMaxKnown(false);
    calcMinMaxWidth();
    if(m_width < m_minWidth) m_width = m_minWidth;

    updateHeight();
}


void RenderRoot::updateHeight()
{
    //kdDebug( 6040 ) << renderName() << "(RenderRoot)::updateHeight() timer=" << updateTimer.elapsed() << endl;
    //int oldMin = m_minWidth;
    setLayouted(false);

    if (parsing())
    {
	if (!updateTimer.isNull() && updateTimer.elapsed()<1000) {
	    return;
	} else
	    updateTimer.start();	
    }

    int oldHeight = docHeight();

    m_view->layout(true);

    //kdDebug(0) << "root layout, time used=" << updateTimer.elapsed() << endl;

    if(parsing())
	updateTimer.start();

    if(docHeight() != oldHeight || docHeight() < m_view->visibleHeight())
    {
//    	kdDebug( 6040 ) << "resizing " << m_width << "," << m_height << endl;
    	m_view->resizeContents(docWidth(), docHeight());    	
    }
    m_view->repaintContents(0,0,1000000,1000000);	//sync repaint!
}

void RenderRoot::close()
{
    setParsing(false);
    updateSize();
    m_view->layout(true);	
    m_view->repaintContents(0,0,1000000,1000000);
//    printTree();
}

void RenderRoot::setSelection(RenderObject *s, int sp, RenderObject *e, int ep)
{

    //kdDebug( 6040 ) << "RenderRoot::setSelection(" << s << "," << sp << "," << e << "," << ep << ")" << endl;

    clearSelection();

    while (s->firstChild())
    	s = s->firstChild();
    while (e->lastChild())
    	e = e->lastChild();

    selectionStart = s;
    selectionEnd = e;
    selectionStartPos = sp;
    selectionEndPos = ep;
	
    RenderObject* o = s;
    while (o && o!=e)
    {
    	if (o->selectionState()!=SelectionInside)
	    o->repaint();
    	o->setSelectionState(SelectionInside);	
//	kdDebug( 6040 ) << "setting selected " << o << ", " << o->isText() << endl;
    	RenderObject* no;
    	if ( !(no = o->firstChild()) )
    	    if ( !(no = o->nextSibling()) )
	    {
	    	no = o->parent();
		while (no && !no->nextSibling())
		    no = no->parent();
		if (no)
		    no = no->nextSibling();
	    }
	o=no;    	
    }
    s->setSelectionState(SelectionStart);
    e->setSelectionState(SelectionEnd);
    if(s == e) s->setSelectionState(SelectionBoth);
    e->repaint();

}


void RenderRoot::clearSelection()
{	
    RenderObject* o = selectionStart;
    while (o && o!=selectionEnd)
    {
    	if (o->selectionState()!=SelectionNone)
	    o->repaint();
    	o->setSelectionState(SelectionNone);	
    	RenderObject* no;
    	if ( !(no = o->firstChild()) )
    	    if ( !(no = o->nextSibling()) )
	    {
	    	no = o->parent();
		while (no && !no->nextSibling())
		    no = no->parent();
		if (no)
		    no = no->nextSibling();
	    }
	o=no;    	
    }
    if (selectionEnd)
    {
    	selectionEnd->setSelectionState(SelectionNone);
    	selectionEnd->repaint();
    }

}

void RenderRoot::selectionStartEnd(int& spos, int& epos)
{
    spos = selectionStartPos;
    epos = selectionEndPos;
}

QRect RenderRoot::viewRect() const
{
    if (m_view)
    	return QRect(m_view->contentsX(),
	    m_view->contentsY(),
	    m_view->visibleWidth(),
	    m_view->visibleHeight());
    else return QRect();
}

int RenderRoot::docHeight() const
{
    int h = m_view->visibleHeight();
    if(m_first) {
	int dh = m_first->height() + m_first->marginTop() + m_first->marginBottom();
	if( m_first->lowestPosition() > dh )
	    dh = m_first->lowestPosition();
	if( dh > h )
	    h = dh;
    }
    return h;
}

int RenderRoot::docWidth() const
{
    int w = m_view->visibleWidth();
    if(m_first) {
	int dw = m_first->width() + m_first->marginLeft() + m_first->marginRight();
	if( dw > w )
	    w = dw;
    }
    return w;
}
