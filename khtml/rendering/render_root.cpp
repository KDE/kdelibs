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
#include <stdio.h>
using namespace khtml;

RenderRoot::RenderRoot(RenderStyle *style, KHTMLView *view)
    : RenderFlow(style)
{
    m_view = view;
    // try to contrain the width to the views width
    m_minWidth = view->frameWidth();
    m_width = m_minWidth;
    m_maxWidth = m_minWidth;

    selectionStart = 0;
    selectionEnd = 0;
    selectionStartPos = -1;
    selectionEndPos = -1;
    setParsing();
}


void RenderRoot::calcWidth()
{
    m_width = m_view->frameWidth();
    if(m_width < m_minWidth) m_width = m_minWidth;
    if(m_maxWidth != m_minWidth) m_maxWidth = m_minWidth;
}


void RenderRoot::layout(bool deep)
{
    if (deep)
    	RenderFlow::layout(true);        

    // resize so that body height >= viewport height
    if (style()->htmlHacks())
    {
    	RenderObject* child = firstChild();
	if (!deep)
	{
	    setLayouted(false);
	    child->setLayouted(false);
	    if (child->firstChild())
	    	child->firstChild()->setLayouted(false);
	    layout(true);
	}
	
        if (m_height < m_view->visibleHeight())
    	    m_height=m_view->visibleHeight();
	
	int h = m_height;
	if (child)
	{
    	    h -= child->marginTop()+child->marginBottom();
    	    child->setHeight(h);
    	    child = child->firstChild();
    	    if (child)	
    		child->setHeight(h - child->marginTop()+child->marginBottom());
	}
    }

}

QScrollView *RenderRoot::view()
{
    return m_view;
}

void RenderRoot::absolutePosition(int &xPos, int &yPos)
{
    xPos = yPos = 0;
}


void RenderRoot::repaintRectangle(int x, int y, int w, int h)
{
    x += m_x;
    y += m_y;
//    printf("updating views contents (%d/%d) (%d/%d)\n", x,y,w,h);
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
    calcMinMaxWidth();

    updateHeight();
}


void RenderRoot::updateHeight()
{
//    printf("%s(RenderRoot)::updateSize()\n", renderName());
    //int oldMin = m_minWidth;
    setLayouted(false);

    if (parsing())
    {
	if (!updateTimer.isNull() && updateTimer.elapsed()<300)
	{
	    return;
	}
	else
	    updateTimer.start();	
    }


    int oldHeight = m_height;
    layout(true);	
    if(m_height != oldHeight)
    {
//    	printf("resizing %d,%d\n",m_width,m_height);
    	m_view->resizeContents(m_width,m_height);
    	m_view->repaintContents(0,0,m_width,m_height);	//sync repaint!
    }
}

void RenderRoot::close()
{
    setParsing(false);
    updateSize();
    repaint();
//    printTree();
//    setSelection(this,-1,this,-1);
}

void RenderRoot::setSelection(RenderObject *s, int sp, RenderObject *e, int ep)
{

    //printf("RenderRoot::setSelection(%x,%d,%x,%d)\n", s,sp,e,ep);

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
//	printf("setting selected %x, %d\n",o, o->isText());
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
