/**
 * This file is part of the HTML widget for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
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


#include "rendering/render_root.h"
#include "rendering/render_layer.h"
#include "xml/dom_docimpl.h"

#include "khtmlview.h"
#include <kdebug.h>

using namespace khtml;

//#define BOX_DEBUG
//#define SPEED_DEBUG

RenderRoot::RenderRoot(DOM::NodeImpl* node, KHTMLView *view)
    : RenderFlow(node)
{
    // init RenderObject attributes
    setInline(false);

    m_view = view;
    // try to contrain the width to the views width

    m_minWidth = 0;
    m_height = 0;

    m_width = m_minWidth;
    m_maxWidth = m_minWidth;

    m_rootWidth = m_rootHeight = 0;
    m_viewportWidth = m_viewportHeight = 0;

    setPositioned(true); // to 0,0 :)

    m_printingMode = false;
    m_paintImages = true;

    m_selectionStart = 0;
    m_selectionEnd = 0;
    m_selectionStartPos = -1;
    m_selectionEndPos = -1;

    // Create a new root layer for our layer hierarchy.
    m_layer = new (node->getDocument()->renderArena()) RenderLayer(this);
}

RenderRoot::~RenderRoot()
{
}

void RenderRoot::calcWidth()
{
    RenderBox::calcWidth();
    return;

    // exception: m_width is already known and set in layout()

    if (style()->marginLeft().isFixed())
        m_marginLeft = style()->marginLeft().value();
    else
        m_marginLeft = 0;

    if (style()->marginRight().isFixed())
        m_marginRight = style()->marginRight().value();
    else
        m_marginRight = 0;
}

void RenderRoot::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    RenderFlow::calcMinMaxWidth();

    m_maxWidth = m_minWidth;

    setMinMaxKnown();
}

//#define SPEED_DEBUG

void RenderRoot::layout()
{
    if (m_printingMode)
       m_minWidth = m_width;

    if(firstChild())
        firstChild()->setLayouted(false);

#ifdef SPEED_DEBUG
    QTime qt;
    qt.start();
#endif
    if ( recalcMinMax() )
	recalcMinMaxWidths();
#ifdef SPEED_DEBUG
    kdDebug() << "RenderRoot::calcMinMax time used=" << qt.elapsed() << endl;
    qt.start();
#endif

#ifdef SPEED_DEBUG
    kdDebug() << "RenderRoot::layout time used=" << qt.elapsed() << endl;
    qt.start();
#endif
    if (!m_printingMode) {
        QSize s = m_view->viewportSize(m_view->contentsWidth(),
                                       m_view->contentsHeight());
        m_viewportWidth = m_width = s.width();
        m_viewportHeight = m_height = s.height();
    }
    else {
        m_width = m_rootWidth;
        m_height = m_rootHeight;
    }

    RenderFlow::layout();

    if (!m_printingMode) {
        m_view->resizeContents(docWidth(), docHeight());
        QSize s = m_view->viewportSize(m_view->contentsWidth(),
                                       m_view->contentsHeight());
        setWidth( m_viewportWidth = s.width() );
        setHeight(  m_viewportHeight = s.height() );
    }

    // ### we could maybe do the call below better and only pass true if the docsize changed.
    layoutPositionedObjects( true );

#ifdef SPEED_DEBUG
    kdDebug() << "RenderRoot::end time used=" << qt.elapsed() << endl;
#endif

    layer()->setHeight(m_height);
    layer()->setWidth(m_width);

    setLayouted();
}

bool RenderRoot::absolutePosition(int &xPos, int &yPos, bool f)
{
    if ( f && m_view) {
	xPos = m_view->contentsX();
	yPos = m_view->contentsY();
    }
    else {
        xPos = yPos = 0;
    }
    return true;
}

void RenderRoot::paint(QPainter *p, int _x, int _y, int _w, int _h, int _tx, int _ty,
		       PaintAction paintPhase)
{
    paintObject(p, _x, _y, _w, _h, _tx, _ty, paintPhase);
}

void RenderRoot::paintObject(QPainter *p, int _x, int _y, int _w, int _h,
			     int _tx, int _ty,  PaintAction paintPhase)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(RenderFlow) " << this << " ::paintObject() w/h = (" << width() << "/" << height() << ")" << endl;
#endif
    // add offset for relative positioning
    if(isRelPositioned())
        relativePositionOffset(_tx, _ty);

    // 1. paint background, borders etc
    if(paintPhase == PaintActionBackground && shouldPaintBackgroundOrBorder() && !isInline())
        paintBoxDecorations(p, _x, _y, _w, _h, _tx, _ty);

    // 2. paint contents
    RenderObject *child = firstChild();
    while(child != 0) {
        if(!child->layer() && !child->isFloating()) {
            child->paint(p, _x, _y, _w, _h, _tx, _ty, paintPhase);
        }
        child = child->nextSibling();
    }

#ifdef BOX_DEBUG
    if (m_view)
    {
        _tx += m_view->contentsX();
        _ty += m_view->contentsY();
    }

    outlineBox(p, _tx, _ty);
#endif

}


void RenderRoot::repaintRectangle(int x, int y, int w, int h, bool f)
{
    if (m_printingMode) return;
//    kdDebug( 6040 ) << "updating views contents (" << x << "/" << y << ") (" << w << "/" << h << ")" << endl;

    if ( f && m_view ) {
        x += m_view->contentsX();
        y += m_view->contentsY();
    }

    QRect vr = viewRect();
    QRect ur(x, y, w, h);

    if (ur.intersects(vr))
        if (m_view) m_view->scheduleRepaint(x, y, w, h);
}

void RenderRoot::repaint()
{
    if (m_view && !m_printingMode)
        m_view->scheduleRepaint(m_view->contentsX(), m_view->contentsY(),
                                m_view->visibleWidth(), m_view->visibleHeight());
}

void RenderRoot::close()
{
    setLayouted( false );
    if (m_view) {
        m_view->layout();
	repaint();
    }
    //printTree();
}

void RenderRoot::setSelection(RenderObject *s, int sp, RenderObject *e, int ep)
{
    // Check we got valid renderobjects. www.msnbc.com and clicking
    // around, to find the case where this happened.
    if ( !s || !e )
    {
        kdWarning(6040) << "RenderRoot::setSelection() called with start=" << s << " end=" << e << endl;
        return;
    }
//     kdDebug( 6040 ) << "RenderRoot::setSelection(" << s << "," << sp << "," << e << "," << ep << ")" << endl;

    while (s->firstChild())
        s = s->firstChild();
    while (e->lastChild())
        e = e->lastChild();

    bool changedSelectionBorder = ( s != m_selectionStart || e != m_selectionEnd );

    if ( !changedSelectionBorder && m_selectionStartPos == sp && m_selectionEndPos == ep )
        return;

    if ( changedSelectionBorder )
        clearSelection();
    else if( m_selectionStart )
        m_selectionStart->repaint();

    // set selection start
    if (m_selectionStart)
        m_selectionStart->setIsSelectionBorder(false);
    m_selectionStart = s;
    if (m_selectionStart)
        m_selectionStart->setIsSelectionBorder(true);
    m_selectionStartPos = sp;

    // set selection end
    if (m_selectionEnd)
        m_selectionEnd->setIsSelectionBorder(false);
    m_selectionEnd = e;
    if (m_selectionEnd)
        m_selectionEnd->setIsSelectionBorder(true);
    m_selectionEndPos = ep;

    // update selection status of all objects between m_selectionStart and m_selectionEnd
    if (  s && changedSelectionBorder ) {
        for( RenderObject* o = s; o != e; ) {
            o->setSelectionState(SelectionInside);
            o->repaint();
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
        e->setSelectionState(s == e ? SelectionBoth : SelectionEnd);
    }
}


void RenderRoot::clearSelection()
{
    // update selection status of all objects between m_selectionStart and m_selectionEnd
    RenderObject* o = m_selectionStart;
    while (o && o!=m_selectionEnd)
    {
        o->setSelectionState(SelectionNone);
        o->repaint();
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
    if (m_selectionEnd) {
        m_selectionEnd->setSelectionState(SelectionNone);
	m_selectionEnd->repaint();
    }

    // set selection start & end to 0
    if (m_selectionStart)
        m_selectionStart->setIsSelectionBorder(false);
    m_selectionStart = 0;
    m_selectionStartPos = -1;

    if (m_selectionEnd)
        m_selectionEnd->setIsSelectionBorder(false);
    m_selectionEnd = 0;
    m_selectionEndPos = -1;
}

void RenderRoot::selectionStartEnd(int& spos, int& epos)
{
    spos = m_selectionStartPos;
    epos = m_selectionEndPos;
}

QRect RenderRoot::viewRect() const
{
    if (m_printingMode)
        return QRect(0,0, m_width, m_height);
    else if (m_view)
        return QRect(m_view->contentsX(),
            m_view->contentsY(),
            m_view->visibleWidth(),
            m_view->visibleHeight());
    else return QRect(0,0,m_rootWidth,m_rootHeight);
}

int RenderRoot::docHeight() const
{
    int h;
    if (m_printingMode || !m_view)
        h = m_height;
    else
        h = 0;
#if 1
    RenderObject *fc = firstChild();
    if(fc) {
        int dh = fc->overflowHeight() + fc->marginTop() + fc->marginBottom();
        int lowestPos = firstChild()->lowestPosition();
        if( lowestPos > dh )
            dh = lowestPos;
        if( dh > h )
            h = dh;
    }
#endif

    RenderLayer *layer = m_layer;
    int y = 0;
    while ( layer ) {
	h = QMAX( h, layer->yPos() + layer->height() );
	h = QMAX( h, layer->xPos() + layer->renderer()->overflowHeight() );
	if ( layer->firstChild() ) {
	    y += layer->yPos();
	    layer = layer->firstChild();
	} else if ( layer->nextSibling() )
	    layer = layer->nextSibling();
	else {
	    while ( layer ) {
		layer = layer->parent();
		y -= layer->yPos();
		if ( layer && layer->nextSibling() ) {
		    layer = layer->nextSibling();
		    break;
		}
	    }
	}
    }
    return h;
}

int RenderRoot::docWidth() const
{
    int w;
    if (m_printingMode || !m_view)
        w = m_width;
    else
        w = 0;

#if 1
    RenderObject *fc = firstChild();
    if(fc) {
        int dw = fc->overflowWidth() + fc->marginLeft() + fc->marginRight();
        int rightmostPos = fc->rightmostPosition();
        if( rightmostPos > dw )
            dw = rightmostPos;
        if( dw > w )
            w = dw;
    }
#endif

    RenderLayer *layer = m_layer;
    int x = 0;
    while ( layer ) {
	w = QMAX( w, layer->xPos() + layer->width() );
	w = QMAX( w, layer->xPos() + layer->renderer()->overflowWidth() );
	if ( layer->firstChild() ) {
	    x += layer->xPos();
	    layer = layer->firstChild();
	} else if ( layer->nextSibling() )
	    layer = layer->nextSibling();
	else {
	    while ( layer ) {
		layer = layer->parent();
		x -= layer->xPos();
		if ( layer && layer->nextSibling() ) {
		    layer = layer->nextSibling();
		    break;
		}
	    }
	}
    }
    return w;
}
