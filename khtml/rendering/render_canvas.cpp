/**
 * This file is part of the HTML widget for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 2003 Apple Computer, Inc.
 *           (C) 2005 Allan Sandfeld Jensen (kde@carewolf.com)
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#include "rendering/render_canvas.h"
#include "rendering/render_layer.h"
#include "xml/dom_docimpl.h"

#include "khtmlview.h"
#include "khtml_part.h"
#include <kdebug.h>
#include <kglobal.h>

using namespace khtml;

//#define BOX_DEBUG
//#define SPEED_DEBUG

RenderCanvas::RenderCanvas(DOM::NodeImpl* node, KHTMLView *view)
    : RenderBlock(node)
{
    // init RenderObject attributes
    setInline(false);
    setIsAnonymous(false);

    m_view = view;
    // try to contrain the width to the views width

    m_minWidth = 0;
    m_height = 0;

    m_width = m_minWidth;
    m_maxWidth = m_minWidth;

    m_rootWidth = m_rootHeight = 0;
    m_viewportWidth = m_viewportHeight = 0;

    setPositioned(true); // to 0,0 :)

    m_staticMode = false;
    m_pagedMode = false;
    m_printImages = true;

    m_pageTop = 0;
    m_pageBottom = 0;

    m_page = 0;

    m_maximalOutlineSize = 0;

    m_selectionStart = 0;
    m_selectionEnd = 0;
    m_selectionStartPos = -1;
    m_selectionEndPos = -1;

    // Create a new root layer for our layer hierarchy.
    m_layer = new (node->getDocument()->renderArena()) RenderLayer(this);
}

RenderCanvas::~RenderCanvas()
{
    delete m_page;
}

void RenderCanvas::setStyle(RenderStyle* style)
{
    /*
    if (m_pagedMode)
        style->setOverflow(OHIDDEN); */
    RenderBlock::setStyle(style);
}

void RenderCanvas::calcHeight()
{
    if (m_pagedMode || !m_view)
        m_height = m_rootHeight;
    else
        m_height = m_view->visibleHeight();
}

void RenderCanvas::calcWidth()
{
    // the width gets set by KHTMLView::print when printing to a printer.
    if(m_pagedMode || !m_view)
    {
        m_width = m_rootWidth;
        return;
    }

    m_width = m_view ?
                m_view->frameWidth() + paddingLeft() + paddingRight() + borderLeft() + borderRight()
                : m_minWidth;

    if (style()->marginLeft().isFixed())
        m_marginLeft = style()->marginLeft().value();
    else
        m_marginLeft = 0;

    if (style()->marginRight().isFixed())
        m_marginRight = style()->marginRight().value();
    else
        m_marginRight = 0;
}

void RenderCanvas::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

    RenderBlock::calcMinMaxWidth();

    m_maxWidth = m_minWidth;

    setMinMaxKnown();
}

//#define SPEED_DEBUG

void RenderCanvas::layout()
{
    if (m_pagedMode) {
       m_minWidth = m_width;
//        m_maxWidth = m_width;
    }

    m_needsFullRepaint =  markedForRepaint() || !view() || view()->needsFullRepaint() || m_pagedMode;

    setChildNeedsLayout(true);
    setMinMaxKnown(false);
    for(RenderObject* c = firstChild(); c; c = c->nextSibling())
        c->setChildNeedsLayout(true);

#ifdef SPEED_DEBUG
    QTime qt;
    qt.start();
#endif
    if ( recalcMinMax() )
	recalcMinMaxWidths();

#ifdef SPEED_DEBUG
    kdDebug() << "RenderCanvas::calcMinMax time used=" << qt.elapsed() << endl;
    qt.start();
#endif

#ifdef SPEED_DEBUG
    kdDebug() << "RenderCanvas::layout time used=" << qt.elapsed() << endl;
    qt.start();
#endif

    if (m_pagedMode || !m_view) {
        m_width = m_rootWidth;
        m_height = m_rootHeight;
    }
    else
    {
        m_viewportWidth = m_width = m_view->visibleWidth();
        m_viewportHeight = m_height = m_view->visibleHeight();
    }

    RenderBlock::layout();

    int docW = docWidth();
    int docH = docHeight();

    if (!m_pagedMode) {
        bool vss = m_view->verticalScrollBar()->isShown();
        bool hss = m_view->horizontalScrollBar()->isShown();
        QSize s = m_view->viewportSize(docW, docH);

        // content size, with scrollbar hysteresis
        int hDocH = docH;
        int hDocW = docW;

        // if we are about to show a scrollbar, and the document is sized to the viewport w or h,
        // then reserve the scrollbar space so that it doesn't trigger the _other_ scrollbar

        if (!vss && m_width - m_view->verticalScrollBar()->sizeHint().width() == s.width() &&
            docW <= m_width)
            hDocW = kMin( docW, s.width() );

        if (!hss && m_height - m_view->horizontalScrollBar()->sizeHint().height() == s.height() &&
            docH <= m_height)
            hDocH = kMin( docH, s.height() );

        // likewise, if a scrollbar is shown, and we have a cunning plan to turn it off,
        // think again if we are falling downright in the hysteresis zone

        if (vss && s.width() > docW && docW > m_view->visibleWidth())
            hDocW = s.width()+1;

        if (hss && s.height() > docH && docH > m_view->visibleHeight())
            hDocH = s.height()+1;

        m_view->resizeContents(hDocW, hDocH);

        setWidth( m_viewportWidth = s.width() );
        setHeight( m_viewportHeight = s.height() );
    }

    // ### we could maybe do the call below better and only pass true if the docsize changed.
    layoutPositionedObjects( true );

#ifdef SPEED_DEBUG
    kdDebug() << "RenderCanvas::end time used=" << qt.elapsed() << endl;
#endif

//     kdDebug(6040) << "RenderCanvas::resize layer to " << kMax( docW,int( m_width ) ) << "x"  << kMax( docH,m_height ) << endl;

    layer()->resize( kMax( docW,int( m_width ) ), kMax( docH,m_height ) );
    layer()->updateLayerPositions( layer(), needsFullRepaint(), true );

    scheduleDeferredRepaints();
    setNeedsLayout(false);
}

bool RenderCanvas::needsFullRepaint() const
{
    return m_needsFullRepaint || m_pagedMode;
}

void RenderCanvas::repaintViewRectangle(int x, int y, int w, int h)
{
  KHTMLAssert( view() );
  view()->scheduleRepaint( x, y, w, h );
}

bool RenderCanvas::absolutePosition(int &xPos, int &yPos, bool f)
{
    if ( f && m_pagedMode) {
        xPos = 0;
        yPos = m_pageTop;
    }
    else
    if ( f && m_view) {
	xPos = m_view->contentsX();
	yPos = m_view->contentsY();
    }
    else {
        xPos = yPos = 0;
    }
    return true;
}

void RenderCanvas::paint(PaintInfo& paintInfo, int _tx, int _ty)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << this << " ::paintObject() w/h = (" << width() << "/" << height() << ")" << endl;
#endif

    // 1. paint background, borders etc
    if(paintInfo.phase == PaintActionElementBackground) {
        paintBoxDecorations(paintInfo, _tx, _ty);
        return;
    }

    // 2. paint contents
    for( RenderObject *child = firstChild(); child; child=child->nextSibling())
        if(!child->layer() && !child->isFloating())
            child->paint(paintInfo, _tx, _ty);

    // 3. paint floats.
    if (paintInfo.phase == PaintActionFloat)
        paintFloats(paintInfo, _tx, _ty);

#ifdef BOX_DEBUG
    if (m_view)
    {
        _tx += m_view->contentsX();
        _ty += m_view->contentsY();
    }

    outlineBox(p, _tx, _ty);
#endif

}

void RenderCanvas::paintBoxDecorations(PaintInfo& paintInfo, int /*_tx*/, int /*_ty*/)
{
    if ((firstChild() && firstChild()->style()->visibility() == VISIBLE) || !view())
        return;

    paintInfo.p->fillRect(paintInfo.r, view()->palette().active().color(QColorGroup::Base));
}

void RenderCanvas::repaintRectangle(int x, int y, int w, int h, bool immediate, bool f)
{
    if (m_staticMode) return;
//    kdDebug( 6040 ) << "updating views contents (" << x << "/" << y << ") (" << w << "/" << h << ")" << endl;

    if (f && m_pagedMode) {
        y += m_pageTop;
    } else
    if ( f && m_view ) {
        x += m_view->contentsX();
        y += m_view->contentsY();
    }

    QRect vr = viewRect();
    QRect ur(x, y, w, h);

    if (m_view && ur.intersects(vr)) {

        if (immediate)
	// ### KWQ's updateContents has an additional parameter "now".
	// It's not clear what the difference between updateContents(...,true)
	// and repaintContents(...) is. As Qt doesn't have this, I'm leaving it out. (LS)
            m_view->updateContents(ur/*, true*/);
        else
            m_view->scheduleRepaint(x, y, w, h);
    }
}

void RenderCanvas::deferredRepaint( RenderObject* o )
{
    m_dirtyChildren.append( o );
}

void RenderCanvas::scheduleDeferredRepaints()
{
    if (!needsFullRepaint()) {
        Q3ValueList<RenderObject*>::const_iterator it;
        for ( it = m_dirtyChildren.begin(); it != m_dirtyChildren.end(); ++it )
            (*it)->repaint();
    }
    //kdDebug(6040) << "scheduled deferred repaints: " << m_dirtyChildren.count() << " needed full repaint: " << needsFullRepaint() << endl;
    m_dirtyChildren.clear();
}

void RenderCanvas::repaint(bool immediate)
{
    if (m_view && !m_staticMode) {
        if (immediate) {
            //m_view->resizeContents(docWidth(), docHeight());
            m_view->unscheduleRepaint();
            if (needsLayout()) {
                m_view->scheduleRelayout();
                return;
            }
	    // ### same as in repaintRectangle
            m_view->updateContents(m_view->contentsX(), m_view->contentsY(),
                                   m_view->visibleWidth(), m_view->visibleHeight()/*, true*/);
        }
        else
            m_view->scheduleRepaint(m_view->contentsX(), m_view->contentsY(),
                                    m_view->visibleWidth(), m_view->visibleHeight());
    }
}

static QRect enclosingPositionedRect (RenderObject *n)
{
    RenderObject *enclosingParent =  n->containingBlock();
    QRect rect(0,0,0,0);
    if (enclosingParent) {
        int ox, oy;
        enclosingParent->absolutePosition(ox, oy);
        rect.setX(ox);
        rect.setY(oy);
        rect.setWidth (enclosingParent->effectiveWidth());
        rect.setHeight (enclosingParent->effectiveHeight());
    }
    return rect;
}

QRect RenderCanvas::selectionRect() const
{
    RenderObject *r = m_selectionStart;
    if (!r)
        return QRect();

    QRect selectionRect = enclosingPositionedRect(r);

    while (r && r != m_selectionEnd)
    {
        RenderObject* n;
        if ( !(n = r->firstChild()) ){
            if ( !(n = r->nextSibling()) )
            {
                n = r->parent();
                while (n && !n->nextSibling())
                    n = n->parent();
                if (n)
                    n = n->nextSibling();
            }
        }
        r = n;
        if (r) {
            selectionRect = selectionRect.unite(enclosingPositionedRect(r));
        }
    }

    return selectionRect;
}

void RenderCanvas::setSelection(RenderObject *s, int sp, RenderObject *e, int ep)
{
    // Check we got valid renderobjects. www.msnbc.com and clicking
    // around, to find the case where this happened.
    if ( !s || !e )
    {
        kdWarning(6040) << "RenderCanvas::setSelection() called with start=" << s << " end=" << e << endl;
        return;
    }
//     kdDebug( 6040 ) << "RenderCanvas::setSelection(" << s << "," << sp << "," << e << "," << ep << ")" << endl;

    bool changedSelectionBorder = ( s != m_selectionStart || e != m_selectionEnd );

    // Cut out early if the selection hasn't changed.
    if ( !changedSelectionBorder && m_selectionStartPos == sp && m_selectionEndPos == ep )
        return;

    // Record the old selected objects.  Will be used later
    // to delta against the selected objects.

    RenderObject *oldStart = m_selectionStart;
    int oldStartPos = m_selectionStartPos;
    RenderObject *oldEnd = m_selectionEnd;
    int oldEndPos = m_selectionEndPos;
    Q3PtrList<RenderObject> oldSelectedInside;
    Q3PtrList<RenderObject> newSelectedInside;
    RenderObject *os = oldStart;

    while (os && os != oldEnd)
    {
        RenderObject* no;
        if ( !(no = os->firstChild()) ){
            if ( !(no = os->nextSibling()) )
            {
                no = os->parent();
                while (no && !no->nextSibling())
                    no = no->parent();
                if (no)
                    no = no->nextSibling();
            }
        }
        if (os->selectionState() == SelectionInside && !oldSelectedInside.containsRef(os))
            oldSelectedInside.append(os);

        os = no;
    }
    if (changedSelectionBorder)
        clearSelection(false);

    while (s->firstChild())
        s = s->firstChild();
    while (e->lastChild())
        e = e->lastChild();

#if 0
    bool changedSelectionBorder = ( s != m_selectionStart || e != m_selectionEnd );

    if ( !changedSelectionBorder && m_selectionStartPos == sp && m_selectionEndPos = ep )
        return;
#endif

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

#if 0
    kdDebug( 6040 ) << "old selection (" << oldStart << "," << oldStartPos << "," << oldEnd << "," << oldEndPos << ")" << endl;
    kdDebug( 6040 ) << "new selection (" << s << "," << sp << "," << e << "," << ep << ")" << endl;
#endif

    // update selection status of all objects between m_selectionStart and m_selectionEnd
    RenderObject* o = s;

    while (o && o!=e)
    {
        o->setSelectionState(SelectionInside);
//      kdDebug( 6040 ) << "setting selected " << o << ", " << o->isText() << endl;
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
        if (o->selectionState() == SelectionInside && !newSelectedInside.containsRef(o))
            newSelectedInside.append(o);

        o=no;
    }
    s->setSelectionState(SelectionStart);
    e->setSelectionState(SelectionEnd);
    if(s == e) s->setSelectionState(SelectionBoth);

    if (!m_view)
        return;

    newSelectedInside.removeRef(s);
    newSelectedInside.removeRef(e);

    QRect updateRect;

    // Don't use repaint() because it will cause all rects to
    // be united (see khtmlview::scheduleRepaint()).  Instead
    // just draw damage rects for objects that have a change
    // in selection state.
    // ### for Qt, updateContents will unite them, too. This has to be
    // circumvented somehow (LS)

    // Are any of the old fully selected objects not in the new selection?
    // If so we have to draw them.
    // Could be faster by building list of non-intersecting rectangles rather
    // than unioning rectangles.
    Q3PtrListIterator<RenderObject> oldIterator(oldSelectedInside);
    bool firstRect = true;
    for (; oldIterator.current(); ++oldIterator){
        if (!newSelectedInside.containsRef(oldIterator.current())){
            if (firstRect){
                updateRect = enclosingPositionedRect(oldIterator.current());
                firstRect = false;
            }
            else
                updateRect = updateRect.unite(enclosingPositionedRect(oldIterator.current()));
        }
    }
    if (!firstRect){
        m_view->updateContents( updateRect );
    }

    // Are any of the new fully selected objects not in the previous selection?
    // If so we have to draw them.
    // Could be faster by building list of non-intersecting rectangles rather
    // than unioning rectangles.
    Q3PtrListIterator<RenderObject> newIterator(newSelectedInside);
    firstRect = true;
    for (; newIterator.current(); ++newIterator){
        if (!oldSelectedInside.containsRef(newIterator.current())){
            if (firstRect){
                updateRect = enclosingPositionedRect(newIterator.current());
                firstRect = false;
            }
            else
                updateRect = updateRect.unite(enclosingPositionedRect(newIterator.current()));
        }
    }
    if (!firstRect) {
        m_view->updateContents( updateRect );
    }

    // Is the new starting object different, or did the position in the starting
    // element change?  If so we have to draw it.
    if (oldStart != m_selectionStart ||
        (oldStart == oldEnd && (oldStartPos != m_selectionStartPos || oldEndPos != m_selectionEndPos)) ||
        (oldStart == m_selectionStart && oldStartPos != m_selectionStartPos)){
        m_view->updateContents( enclosingPositionedRect(m_selectionStart) );
    }

    // Draw the old selection start object if it's different than the new selection
    // start object.
    if (oldStart && oldStart != m_selectionStart){
        m_view->updateContents( enclosingPositionedRect(oldStart) );
    }

    // Does the selection span objects and is the new end object different, or did the position
    // in the end element change?  If so we have to draw it.
    if (/*(oldStart != oldEnd || !oldEnd) &&*/
        (oldEnd != m_selectionEnd ||
        (oldEnd == m_selectionEnd && oldEndPos != m_selectionEndPos))){
        m_view->updateContents( enclosingPositionedRect(m_selectionEnd) );
    }

    // Draw the old selection end object if it's different than the new selection
    // end object.
    if (oldEnd && oldEnd != m_selectionEnd){
        m_view->updateContents( enclosingPositionedRect(oldEnd) );
    }
}

void RenderCanvas::clearSelection(bool doRepaint)
{
    // update selection status of all objects between m_selectionStart and m_selectionEnd
    RenderObject* o = m_selectionStart;
    while (o && o!=m_selectionEnd)
    {
        if (o->selectionState()!=SelectionNone)
            if (doRepaint)
                o->repaint();
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
        if (doRepaint)
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

void RenderCanvas::selectionStartEnd(int& spos, int& epos)
{
    spos = m_selectionStartPos;
    epos = m_selectionEndPos;
}

QRect RenderCanvas::viewRect() const
{
    if (m_pagedMode)
        if (m_pageTop == m_pageBottom) {
            kdDebug(6040) << "viewRect: " << QRect(0, m_pageTop, m_width, m_height) << endl;
            return QRect(0, m_pageTop, m_width, m_height);
        }
        else {
            kdDebug(6040) << "viewRect: " << QRect(0, m_pageTop, m_width, m_pageBottom - m_pageTop) << endl;
            return QRect(0, m_pageTop, m_width, m_pageBottom - m_pageTop);
        }
    else if (m_view)
        return QRect(m_view->contentsX(),
            m_view->contentsY(),
            m_view->visibleWidth(),
            m_view->visibleHeight());
    else
        return QRect(0,0,m_rootWidth,m_rootHeight);
}

int RenderCanvas::docHeight() const
{
    int h;
    if (m_pagedMode || !m_view)
        h = m_height;
    else
        h = 0;

    RenderObject *fc = firstChild();
    if(fc) {
        int dh = fc->overflowHeight() + fc->marginTop() + fc->marginBottom();
        int lowestPos = firstChild()->lowestPosition(false);
// kdDebug(6040) << "h " << h << " lowestPos " << lowestPos << " dh " << dh << " fc->rh " << fc->effectiveHeight() << " fc->height() " << fc->height() << endl;
        if( lowestPos > dh )
            dh = lowestPos;
        if( dh > h )
            h = dh;
    }

    RenderLayer *layer = m_layer;
    h = kMax( h, layer->yPos() + layer->height() );
// kdDebug(6040) << "h " << h << " layer(" << layer->renderer()->renderName() << "@" << layer->renderer() << ")->height " << layer->height() << " lp " << (layer->yPos() + layer->height()) << " height() " << layer->renderer()->height() << " rh " << layer->renderer()->effectiveHeight() << endl;
    return h;
}

int RenderCanvas::docWidth() const
{
    int w;
    if (m_pagedMode || !m_view)
        w = m_width;
    else
        w = 0;

    RenderObject *fc = firstChild();
    if(fc) {
        int dw = fc->effectiveWidth() + fc->marginLeft() + fc->marginRight();
        int rightmostPos = fc->rightmostPosition(false);
// kdDebug(6040) << "w " << w << " rightmostPos " << rightmostPos << " dw " << dw << " fc->rw " << fc->effectiveWidth() << " fc->width() " << fc->width() << endl;
        if( rightmostPos > dw )
            dw = rightmostPos;
        if( dw > w )
            w = dw;
    }

    RenderLayer *layer = m_layer;
    w = kMax( w, layer->xPos() + layer->width() );
// kdDebug(6040) << "w " << w << " layer(" << layer->renderer()->renderName() << ")->width " << layer->width() << " rm " << (layer->xPos() + layer->width()) << " width() " << layer->renderer()->width() << " rw " << layer->renderer()->effectiveWidth() << endl;
    return w;
}

RenderPage* RenderCanvas::page() {
    if (!m_page) m_page = new RenderPage(this);
    return m_page;
}
