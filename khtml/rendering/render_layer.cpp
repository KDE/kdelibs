/*
 * Copyright (C) 2003 Apple Computer, Inc.
 *               2003 Lars Knoll <knoll@kde.org>
 *
 * Portions are Copyright (C) 1998 Netscape Communications Corporation.
 *
 * Other contributors:
 *   Robert O'Callahan <roc+@cs.cmu.edu>
 *   David Baron <dbaron@fas.harvard.edu>
 *   Christian Biesinger <cbiesinger@web.de>
 *   Randall Jesup <rjesup@wgate.com>
 *   Roland Mainz <roland.mainz@informatik.med.uni-giessen.de>
 *   Josh Soref <timeless@mac.com>
 *   Boris Zbarsky <bzbarsky@mit.edu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Alternatively, the contents of this file may be used under the terms
 * of either the Mozilla Public License Version 1.1, found at
 * http://www.mozilla.org/MPL/ (the "MPL") or the GNU General Public
 * License Version 2.0, found at http://www.fsf.org/copyleft/gpl.html
 * (the "GPL"), in which case the provisions of the MPL or the GPL are
 * applicable instead of those above.  If you wish to allow use of your
 * version of this file only under the terms of one of those two
 * licenses (the MPL or the GPL) and not to allow others to use your
 * version of this file under the LGPL, indicate your decision by
 * deletingthe provisions above and replace them with the notice and
 * other provisions required by the MPL or the GPL, as the case may be.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under any of the LGPL, the MPL or the GPL.
 */

#include <kdebug.h>
#include <assert.h>
#include "khtmlview.h"
#include "rendering/render_layer.h"
#include "rendering/render_block.h"
#include "rendering/render_replaced.h"
#include "rendering/render_arena.h"
#include "xml/dom_docimpl.h"

#include <qscrollbar.h>
#include <qstyle.h>
#include <qvaluevector.h>
#include <qtl.h>

using namespace DOM;
using namespace khtml;

#ifndef NDEBUG
static bool inRenderLayerDetach;
#endif

void RenderScrollMediator::slotValueChanged()
{
    m_layer->updateScrollPositionFromScrollbars();
}

RenderLayer::RenderLayer(RenderObject* object)
    : m_object( object ),
      m_parent( 0 ),
      m_previous( 0 ),
      m_next( 0 ),
      m_first( 0 ),
      m_last( 0 ),
      m_hBar( 0 ),
      m_vBar( 0 ),
      m_scrollMediator( 0 ),
      m_zOrderList( 0 ),
      m_y( 0 ),
      m_x( 0 ),
      m_scrollY( 0 ),
      m_scrollX( 0 ),
      m_scrollWidth( 0 ),
      m_scrollHeight( 0 ),
      m_zOrderListDirty( true )
{
}

RenderLayer::~RenderLayer()
{
    // Child layers will be deleted by their corresponding render objects, so
    // our destructor doesn't have to do anything.
    m_parent = m_previous = m_next = m_first = m_last = 0;
    delete m_hBar;
    delete m_vBar;
    delete m_scrollMediator;
    delete m_zOrderList;
}

void RenderLayer::updateLayerPosition()
{
    // The canvas is sized to the docWidth/Height over in
    // RenderCanvas::layout, so we don't need to ever update our layer
    // position here.
    if (renderer()->isCanvas())
        return;

    int x = m_object->xPos();
    int y = m_object->yPos();

    if (!m_object->isPositioned()) {
        // We must adjust our position by walking up the render tree
        // looking for the nearest enclosing object with a layer.
        RenderObject* curr = m_object->parent();
        while (curr && !curr->layer()) {
            x += curr->xPos();
            y += curr->yPos();
            curr = curr->parent();
        }
    }

    if (m_object->isRelPositioned())
        static_cast<RenderBox*>(m_object)->relativePositionOffset(x, y);

    // Subtract our parent's scroll offset.
    if (parent())
        parent()->subtractScrollOffset(x, y);

    setPos(x,y);
}

short RenderLayer::width() const
{
    int w = m_object->width();
    if (!m_object->isCanvas() && !m_object->style()->hidesOverflow())
        w = QMAX(m_object->overflowWidth(), w);
    return w;
}

int RenderLayer::height() const
{
    int h = m_object->height();
    if (!m_object->isCanvas() && !m_object->style()->hidesOverflow())
        h = QMAX(m_object->overflowHeight(), h);
    return h;
}

RenderLayer *RenderLayer::stackingContext() const
{
    RenderLayer* curr = parent();
    for ( ; curr && !curr->m_object->isCanvas() && !curr->m_object->isRoot() &&
         curr->m_object->style()->hasAutoZIndex();
         curr = curr->parent())
        ;

    return curr;
}

RenderLayer* RenderLayer::enclosingPositionedAncestor() const
{
    RenderLayer* curr = parent();
    for ( ; curr && !curr->m_object->isCanvas() && !curr->m_object->isRoot() &&
         !curr->m_object->isPositioned() && !curr->m_object->isRelPositioned();
         curr = curr->parent())
        ;

    return curr;
}

void* RenderLayer::operator new(size_t sz, RenderArena* renderArena) throw()
{
    return renderArena->allocate(sz);
}

void RenderLayer::operator delete(void* ptr, size_t sz)
{
    assert(inRenderLayerDetach);

    // Stash size where detach can find it.
    *(size_t *)ptr = sz;
}

void RenderLayer::detach(RenderArena* renderArena)
{
#ifndef NDEBUG
    inRenderLayerDetach = true;
#endif
    delete this;
#ifndef NDEBUG
    inRenderLayerDetach = false;
#endif

    // Recover the size left there for us by operator delete and free the memory.
    renderArena->free(*(size_t *)this, this);
}

void RenderLayer::addChild(RenderLayer *child, RenderLayer* beforeChild)
{
    RenderLayer* prevSibling = beforeChild ? beforeChild->previousSibling() : lastChild();
    if (prevSibling) {
        child->setPreviousSibling(prevSibling);
        prevSibling->setNextSibling(child);
    }
    else
        setFirstChild(child);

    if (beforeChild) {
        beforeChild->setPreviousSibling(child);
        child->setNextSibling(beforeChild);
    }
    else
        setLastChild(child);

    child->setParent(this);

    child->stackingContext()->setLayerDirty();
}

RenderLayer* RenderLayer::removeChild(RenderLayer* oldChild)
{
    // remove the child
    if (oldChild->previousSibling())
        oldChild->previousSibling()->setNextSibling(oldChild->nextSibling());
    if (oldChild->nextSibling())
        oldChild->nextSibling()->setPreviousSibling(oldChild->previousSibling());

    if (m_first == oldChild)
        m_first = oldChild->nextSibling();
    if (m_last == oldChild)
        m_last = oldChild->previousSibling();

    RenderLayer* stackingContext = oldChild->stackingContext();
    if ( stackingContext )
        stackingContext->setLayerDirty();

    oldChild->setPreviousSibling(0);
    oldChild->setNextSibling(0);
    oldChild->setParent(0);

    return oldChild;
}

void RenderLayer::removeOnlyThisLayer()
{
    if (!m_parent)
        return;

    // Remove us from the parent.
    RenderLayer* parent = m_parent;
    RenderLayer* nextSib = nextSibling();
    parent->removeChild(this);

    // Now walk our kids and reattach them to our parent.
    RenderLayer* current = m_first;
    while (current) {
        RenderLayer* next = current->nextSibling();
        removeChild(current);
        parent->addChild(current, nextSib);
        current = next;
    }

    detach(renderer()->renderArena());
}

void RenderLayer::insertOnlyThisLayer()
{
    if (!m_parent && renderer()->parent()) {
        // We need to connect ourselves when our renderer() has a parent.
        // Find our enclosingLayer and add ourselves.
        RenderLayer* parentLayer = renderer()->parent()->enclosingLayer();
        if (parentLayer)
            parentLayer->addChild(this,
                                  renderer()->parent()->findNextLayer(parentLayer, renderer()));
    }

    // Remove all descendant layers from the hierarchy and add them to the new position.
    for (RenderObject* curr = renderer()->firstChild(); curr; curr = curr->nextSibling())
        curr->moveLayers(m_parent, this);
}

void RenderLayer::convertToLayerCoords(const RenderLayer* ancestorLayer, int& x, int& y) const
{
    if (ancestorLayer == this)
        return;

    if (m_object->style()->position() == FIXED) {
        // Add in the offset of the view.  We can obtain this by calling
        // absolutePosition() on the RenderCanvas.
        int xOff, yOff;
        m_object->absolutePosition(xOff, yOff, true);
        x += xOff;
        y += yOff;
        return;
    }

    RenderLayer* parentLayer;
    if (m_object->style()->position() == ABSOLUTE)
        parentLayer = enclosingPositionedAncestor();
    else
        parentLayer = parent();

    if (!parentLayer) return;

    parentLayer->convertToLayerCoords(ancestorLayer, x, y);

    x += xPos();
    y += yPos();
}

void RenderLayer::scrollOffset(int& x, int& y)
{
    x += scrollXOffset();
    y += scrollYOffset();
}

void RenderLayer::subtractScrollOffset(int& x, int& y)
{
    x -= scrollXOffset();
    y -= scrollYOffset();
}

void RenderLayer::scrollToOffset(int x, int y, bool updateScrollbars)
{
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    int maxX = m_scrollWidth - m_object->clientWidth();
    int maxY = m_scrollHeight - m_object->clientHeight();
    if (x > maxX) x = maxX;
    if (y > maxY) y = maxY;

    // FIXME: Eventually, we will want to perform a blit.  For now never
    // blit, since the check for blitting is going to be very
    // complicated (since it will involve testing whether our layer
    // is either occluded by another layer or clipped by an enclosing
    // layer or contains fixed backgrounds, etc.).
    m_scrollX = x;
    m_scrollY = y;

    // FIXME: Fire the onscroll DOM event.

    // Just schedule a full repaint of our object.
    m_object->repaint();

    if (updateScrollbars) {
        if (m_hBar)
            m_hBar->setValue(m_scrollX);
        if (m_vBar)
            m_vBar->setValue(m_scrollY);
    }
}

void RenderLayer::updateScrollPositionFromScrollbars()
{
    bool needUpdate = false;
    int newX = m_scrollX;
    int newY = m_scrollY;

    if (m_hBar) {
        newX = m_hBar->value();
        if (newX != m_scrollX)
           needUpdate = true;
    }

    if (m_vBar) {
        newY = m_vBar->value();
        if (newY != m_scrollY)
           needUpdate = true;
    }

    if (needUpdate)
        scrollToOffset(newX, newY, false);
}

void RenderLayer::showScrollbar(Qt::Orientation o, bool show)
{
    QScrollBar *sb = (o == Qt::Horizontal) ? m_hBar : m_vBar;

    if (show && !sb) {
        QScrollView* scrollView = m_object->element()->getDocument()->view();
        sb = new QScrollBar(o, scrollView);
        scrollView->addChild(sb, 0, -50000);
	sb->setBackgroundMode(QWidget::NoBackground);
	sb->show();
        if (!m_scrollMediator)
            m_scrollMediator = new RenderScrollMediator(this);
        m_scrollMediator->connect(sb, SIGNAL(valueChanged(int)), SLOT(slotValueChanged()));
    }
    else if (!show && sb) {
        delete sb;
        sb = 0;
    }

    if (o == Qt::Horizontal)
	m_hBar = sb;
    else
	m_vBar = sb;
}

void
RenderLayer::moveScrollbarsAside()
{
    QScrollView* scrollView = m_object->element()->getDocument()->view();
    if (m_hBar)
        scrollView->addChild(m_hBar, 0, -50000);
    if (m_vBar)
        scrollView->addChild(m_vBar, 0, -50000);
}

void RenderLayer::layout()
{
    updateLayerPosition();
    checkScrollbarsAfterLayout();
    if (!renderer()->style()->hasAutoZIndex() || renderer()->isCanvas())
	updateLayerInformation();
}

void RenderLayer::positionScrollbars(int tx,  int ty)
{
    int bl = m_object->borderLeft();
    int bt = m_object->borderTop();
    int w = width() - bl - m_object->borderRight();
    int h = height() - bt - m_object->borderBottom();

    if (w <= 0 || h <= 0 || (!m_vBar && !m_hBar))
	return;

    QScrollView* scrollView = m_object->element()->getDocument()->view();

    tx += bl;
    ty += bt;

    QScrollBar *b = m_hBar;
    if (!m_hBar)
	b = m_vBar;
    int sw = b->style().pixelMetric(QStyle::PM_ScrollBarExtent);

    if (m_vBar) {
	vBarRect = QRect(tx + w - sw + 1, ty, sw, h - (m_hBar ? sw : 0) + 1);
        m_vBar->resize(vBarRect.width(), vBarRect.height());
        scrollView->addChild(m_vBar, vBarRect.x(), vBarRect.y());
    }

    if (m_hBar) {
	hBarRect = QRect(tx, ty + h - sw + 1, w - (m_vBar ? sw : 0) + 1, sw);
        m_hBar->resize(hBarRect.width(), hBarRect.height());
        scrollView->addChild(m_hBar, hBarRect.x(), hBarRect.y());
    }
}

#define LINE_STEP   10
#define PAGE_KEEP   40

void RenderLayer::checkScrollbarsAfterLayout()
{
    int rightPos = m_object->rightmostPosition();
    int bottomPos = m_object->lowestPosition();

    int clientWidth = m_object->clientWidth();
    int clientHeight = m_object->clientHeight();
    m_scrollWidth = clientWidth;
    m_scrollHeight = clientHeight;

    if (rightPos - m_object->borderLeft() > m_scrollWidth)
        m_scrollWidth = rightPos - m_object->borderLeft();
    if (bottomPos - m_object->borderTop() > m_scrollHeight)
        m_scrollHeight = bottomPos - m_object->borderTop();

    bool needHorizontalBar = rightPos > width();
    bool needVerticalBar = bottomPos > height();

    bool haveHorizontalBar = m_hBar;
    bool haveVerticalBar = m_vBar;

    // overflow:scroll should just enable/disable.
    if (m_object->style()->overflow() == OSCROLL) {
        m_hBar->setEnabled(needHorizontalBar);
        m_vBar->setEnabled(needVerticalBar);
    }

    // overflow:auto may need to lay out again if scrollbars got added/removed.
    bool scrollbarsChanged = (m_object->style()->overflow() == OAUTO) &&
        (haveHorizontalBar != needHorizontalBar || haveVerticalBar != needVerticalBar);
    if (scrollbarsChanged) {
        showScrollbar(Qt::Horizontal, needHorizontalBar);
        showScrollbar(Qt::Vertical, needVerticalBar);

        m_object->setLayouted(false);
#if 0
	// ##############
	if (m_object->isRenderBlock())
            static_cast<RenderBlock*>(m_object)->layoutBlock(true);
        else
            m_object->layout();

#endif
    }

    // Set up the range (and page step/line step).
    if (m_hBar) {
        int pageStep = (clientWidth-PAGE_KEEP);
        if (pageStep < 0) pageStep = clientWidth;
        m_hBar->setSteps(LINE_STEP, pageStep);
#ifdef APPLE_CHANGES
        m_hBar->setKnobProportion(clientWidth, m_scrollWidth);
#else
        m_hBar->setRange(0, m_scrollWidth-clientWidth);
#endif
    }
    if (m_vBar) {
        int pageStep = (clientHeight-PAGE_KEEP);
        if (pageStep < 0) pageStep = clientHeight;
        m_vBar->setSteps(LINE_STEP, pageStep);
#ifdef APPLE_CHANGES
        m_vBar->setKnobProportion(clientHeight, m_scrollHeight);
#else
        m_vBar->setRange(0, m_scrollHeight-clientHeight);
#endif
    }

}

void RenderLayer::paintScrollbars(QPainter* p, int x, int y, int w, int h)
{
#ifdef APPLE_CHANGES
    if (m_hBar)
        m_hBar->paint(p, QRect(x, y, w, h));
    if (m_vBar)
        m_vBar->paint(p, QRect(x, y, w, h));
#else
    if (m_hBar)
	RenderWidget::paintWidget(p, m_hBar, x, y, w, h,
			    hBarRect.x(), hBarRect.y());
    if (m_vBar)
	RenderWidget::paintWidget(p, m_vBar, x, y, w, h,
			    vBarRect.x(), vBarRect.y());
#endif
}


static void setClip(QPainter *p, const QRect &clip)
{
    QRect cr = p->xForm(clip);

    QRegion creg(cr);
    QRegion old = p->clipRegion();
    if (!old.isNull())
	creg = old.intersect(creg);

    p->save();
    p->setClipRegion(creg);
}

void RenderLayer::paint(QPainter *p, int x, int y, int w, int h,
		   int tx, int ty, bool selectionOnly)
{
    tx += xPos();
    ty += yPos();

    if ( m_zOrderListDirty )
        updateLayerInformation();

    if(!m_zOrderList
//        || (ty > y + h) || (ty + height() < y)
//        || (tx > x + h) || (tx + width() < x)
        )
 	return;


    // do this before we paint the background
    positionScrollbars(tx, ty);
    /*
      This is from the latest CSS 2.1 draft:

      Each stacking context consists of the following stacking levels (from
      back to front):
      1.     the background and borders of the element forming the stacking
      context.
      2.     the stacking contexts of descendants with negative stack levels.
      3.     a stacking level containing in-flow non-inline-level descendants.
      4.     a stacking level for floats and their contents.
      5.     a stacking level for in-flow inline-level descendants.
      6.     a stacking level for positioned descendants with 'z-index:
      auto', and any descendant stacking contexts with 'z-index: 0'.
      7.     the stacking contexts of descendants with positive stack levels.
    */

    // Walk the list and paint each layer, adding in the appropriate offset.

    bool clip = renderer()->style()->overflow() != OVISIBLE;
    QScrollBar *bar = m_hBar;
    if (!m_hBar)
	bar = m_vBar;
    int cw = width();
    int ch = height();
    int sw = bar ? bar->style().pixelMetric(QStyle::PM_ScrollBarExtent) - 1 : 0;
    int bl = m_object->borderLeft();
    int br = m_object->borderRight();
    int bt = m_object->borderTop();
    int bb = m_object->borderBottom();
    bool clip2 = bar && ((bl != 0) | (br != 0) | (bt != 0) | (bb != 0));

    // paint our background
    if (!selectionOnly) {
	if (clip)
	    setClip(p, QRect(tx, ty, cw, ch));
	renderer()->paint(p, x, y, w, h,
			  tx - renderer()->xPos(), ty - renderer()->yPos(),
			  PaintActionBackground);
	if (clip2)
            p->restore();
    }
    if (clip2)
	setClip(p, QRect(tx + bl, ty + bt,
			 w - bl - br - (m_vBar ? sw : 0) , h - bt - bb - (m_hBar ? sw : 0)));

    uint count = m_zOrderList->count();
    for (uint i = 0; i < count; i++) {
        const PositionedLayer &pLayer = m_zOrderList->at(i);
	RenderLayer *l = pLayer.layer;
	RenderObject *r = l->renderer();
	int xOff = 0, yOff = 0;
	l->convertToLayerCoords(this, xOff, yOff);

	bool lclip = r->style()->overflow() != OVISIBLE;
	cw = l->width();
	ch = l->height();
	bar = l->m_hBar;
	if (!m_hBar)
	    bar = l->m_vBar;
	sw = bar ? bar->style().pixelMetric(QStyle::PM_ScrollBarExtent) -1 : 0;
	bl = r->borderLeft();
	br = r->borderRight();
	bt = r->borderTop();
	bb = r->borderBottom();
	bool clip2 = bar && ((bl != 0) | (br != 0) | (bt != 0) | (bb != 0));

	l->positionScrollbars(tx + xOff, ty + yOff);
	if (l != this && !l->hasAutoZIndex()) {
	    // layer has it's own stacking context, just call paint on the layer.
	    l->paint(p, x, y, w, h, tx + xOff - l->xPos(), ty + yOff - l->yPos(), selectionOnly);
	} else {
	    if (selectionOnly) {
		r->paint(p, x, y, w, h,
			 tx + xOff - r->xPos(), ty + yOff - r->yPos(), PaintActionSelection);
	    } else {
		if (l != this) {
		    if (lclip)
			setClip(p, QRect(tx + xOff, ty + yOff, cw, ch));
		    r->paint(p, x, y, w, h,
			     tx + xOff - r->xPos(), ty + yOff - r->yPos(), PaintActionBackground);
		    if (clip2)
                        p->restore();
		}
		if (clip2)
		    setClip(p, QRect(tx + xOff + bl, ty + yOff + bt,
				     cw - bl - bt - (l->m_vBar ? sw : 0) ,
				     ch - bt - bb - (l->m_hBar ? sw : 0)));
		r->paint(p, x, y, w, h,
			 tx + xOff - r->xPos(), ty + yOff - r->yPos(), PaintActionFloat);
		r->paint(p, x, y, w, h,
			 tx + xOff - r->xPos(), ty + yOff - r->yPos(), PaintActionForeground);
		if (lclip)
                    p->restore();

		l->paintScrollbars(p, x, y, w, h);
	    }
	}
    }


    if (clip)
        p->restore();

    paintScrollbars(p, x, y, w, h);
}

void RenderLayer::clearOtherLayersHoverActiveState()
{
    if (!m_parent)
        return;

    for (RenderLayer* curr = m_parent->firstChild(); curr; curr = curr->nextSibling()) {
        if (curr == this)
            continue;
        curr->clearHoverAndActiveState(curr->renderer());
    }

    m_parent->clearOtherLayersHoverActiveState();
}

void RenderLayer::clearHoverAndActiveState(RenderObject* obj)
{
    if (!obj->mouseInside())
        return;

    obj->setMouseInside(false);
    if (obj->element()) {
        obj->element()->setActive(false);
        if (obj->style()->affectedByHoverRules() || obj->style()->affectedByActiveRules())
            obj->element()->setChanged(true);
    }

    for (RenderObject* child = obj->firstChild(); child; child = child->nextSibling())
        if (child->mouseInside())
            clearHoverAndActiveState(child);
}

bool
RenderLayer::nodeAtPoint(RenderObject::NodeInfo& info, int x, int y, int tx, int ty)
{
    tx += xPos();
    ty += yPos();

    if (!m_zOrderList)
	return renderer()->nodeAtPoint(info, x, y, tx - renderer()->xPos(), ty - renderer()->yPos());

    bool inside = false;
    RenderLayer *insideLayer = 0;


//     qDebug("%p nodeAtPoint: numlayers=%d x=%d y=%d, tx=%d, ty=%d, xPos=%d, yPos=%d", this->renderer(), m_zOrderList->count(), x, y, tx, ty, xPos(), yPos());
    for (int i = m_zOrderList->count()-1; i >= 0; i--) {
        const PositionedLayer &pLayer = m_zOrderList->at(i);
	RenderLayer *l = pLayer.layer;
	RenderObject *r = l->renderer();
	int xOff = 0, yOff = 0;
        if (!(l && r)) {
            continue;
        }
	l->convertToLayerCoords(this, xOff, yOff);

// 	qDebug("   testing %p x=%d y=%d, w=%d, h=%d", r, tx+xOff, ty+yOff, l->width(), l->height());
	if (l != this)
	    inside = l->nodeAtPoint(info, x, y, tx + xOff - l->xPos(), ty + yOff - l->yPos());
	else
	    inside = r->nodeAtPoint(info, x, y, tx + xOff - r->xPos(), ty + yOff - r->yPos());
        if (inside) {
            insideLayer = l;
            break;
        }
    }

//     if (insideLayer)
// 	qDebug("      insideLayer: %p     %d %d %d %d", insideLayer->renderer(), insideLayer->xPos(), insideLayer->yPos(), insideLayer->width(), insideLayer->height());
//     else
// 	qDebug("      no layer found");
    if (insideLayer) {
        // Clear out the other layers' hover/active state
        insideLayer->clearOtherLayersHoverActiveState();

        // Now clear out our descendant layers
        for (RenderLayer* child = insideLayer->firstChild();
             child; child = child->nextSibling())
            child->clearHoverAndActiveState(child->renderer());
    }

    return inside;
}

void RenderLayer::setLayerDirty()
{
    if ( m_zOrderList )
        m_zOrderList->clear();

    m_zOrderListDirty = true;
}

void RenderLayer::updateLayerInformation()
{
    assert(!renderer()->style()->hasAutoZIndex() || renderer()->isCanvas());

//    if ( renderer()->isCanvas() || !m_zOrderListDirty )
//         return;

    if (!m_zOrderList)
	m_zOrderList = new QValueVector<PositionedLayer>();
    else
	m_zOrderList->clear();

    int idx = 0;
    collectLayers(m_zOrderList, 0, 0, idx);
    qHeapSort(*m_zOrderList);

    m_zOrderListDirty = false;
}


void RenderLayer::collectLayers(QValueVector<PositionedLayer> *l, int tx, int ty, int &idx)
{
    tx += xPos();
    ty += yPos();


    PositionedLayer p = { this, idx++ };
    l->append(p);

    if (l == m_zOrderList || renderer()->style()->hasAutoZIndex()) {
	// collect from all children
        for (RenderLayer* child = firstChild();
             child; child = child->nextSibling())
	    child->collectLayers(l, tx, ty, idx);
    }
}


#include "render_layer.moc"
