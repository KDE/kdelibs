/*
 * Copyright (C) 2003 Apple Computer, Inc.
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

#ifndef render_layer_h
#define render_layer_h

#include <qcolor.h>
#include <qrect.h>
#include <assert.h>

#include "render_object.h"
#include "render_style.h"

class QScrollBar;
template <class T> class QValueVector;

namespace khtml {
    class RenderStyle;
    class RenderTable;
    class CachedObject;
    class RenderCanvas;
    class RenderText;
    class RenderFrameSet;
    class RenderObject;
    class RenderScrollMediator;

class RenderScrollMediator: public QObject
{
    Q_OBJECT
public:
    RenderScrollMediator(RenderLayer* layer)
    :m_layer(layer) {}

public slots:
    void slotValueChanged();

private:
    RenderLayer* m_layer;
};

class RenderLayer
{
public:
    RenderLayer(RenderObject* object);
    ~RenderLayer();

    RenderObject* renderer() const { return m_object; }
    RenderLayer *parent() const { return m_parent; }
    RenderLayer *previousSibling() const { return m_previous; }
    RenderLayer *nextSibling() const { return m_next; }

    RenderLayer *firstChild() const { return m_first; }
    RenderLayer *lastChild() const { return m_last; }

    void addChild(RenderLayer *newChild, RenderLayer* beforeChild = 0);
    RenderLayer* removeChild(RenderLayer *oldChild);

    void removeOnlyThisLayer();
    void insertOnlyThisLayer();

    RenderLayer* root() {
        RenderLayer* curr = this;
        while (curr->parent()) curr = curr->parent();
        return curr;
    }

    int xPos() const { return m_x; }
    int yPos() const { return m_y; }
    short width() const;
    int height() const;
    short scrollWidth() const { return m_scrollWidth; }
    int scrollHeight() const { return m_scrollHeight; }

    void setPos( int xPos, int yPos ) {
        m_x = xPos;
        m_y = yPos;
    }

    // Scrolling methods for layers that can scroll their overflow.
    void scrollOffset(int& x, int& y);
    void subtractScrollOffset(int& x, int& y);
    short scrollXOffset() { return m_scrollX; }
    int scrollYOffset() { return m_scrollY; }
    void scrollToOffset(int x, int y, bool updateScrollbars = true);
    void scrollToXOffset(int x) { scrollToOffset(x, m_scrollY); }
    void scrollToYOffset(int y) { scrollToOffset(m_scrollX, y); }
    void setHasHorizontalScrollbar(bool hasScrollbar);
    void setHasVerticalScrollbar(bool hasScrollbar);
    QScrollBar* horizontalScrollbar() { return m_hBar; }
    QScrollBar* verticalScrollbar() { return m_vBar; }
    int verticalScrollbarWidth();
    int horizontalScrollbarHeight();
    void moveScrollbarsAside();
    void positionScrollbars(int tx, int ty);
#ifdef APPLE_CHANGES
    void paintScrollbars(QPainter* p, int x, int y, int w, int h);
#endif
    void checkScrollbarsAfterLayout();
    void slotValueChanged(int);
    void updateScrollPositionFromScrollbars();

    void layout();
    void updateLayerPosition();

    // Gets the nearest enclosing positioned ancestor layer (also includes
    // the <html> layer and the root layer).
    RenderLayer* enclosingPositionedAncestor();

    void convertToLayerCoords(RenderLayer* ancestorLayer, int& x, int& y);

    bool hasAutoZIndex() { return renderer()->style()->hasAutoZIndex(); }
    int zIndex() { return renderer()->style()->zIndex(); }

    // The two main functions that use the layer system.  The paint method
    // paints the layers that intersect the damage rect from back to
    // front.  The nodeAtPoint method looks for mouse events by walking
    // layers that intersect the point from front to back.
    void paint(QPainter *p, int x, int y, int w, int h, int tx, int ty, bool selectionOnly=false);
    bool nodeAtPoint(RenderObject::NodeInfo& info, int x, int y, int tx, int ty);

    void clearOtherLayersHoverActiveState();
    void clearHoverAndActiveState(RenderObject* obj);

    void detach(RenderArena* renderArena);

     // Overloaded new operator.  Derived classes must override operator new
    // in order to allocate out of the RenderArena.
    void* operator new(size_t sz, RenderArena* renderArena) throw();

    // Overridden to prevent the normal delete from being called.
    void operator delete(void* ptr, size_t sz);

    void updateLayerInformation();

private:
    // The normal operator new is disallowed on all render objects.
    void* operator new(size_t sz) throw();

private:
    void setNextSibling(RenderLayer* next) { m_next = next; }
    void setPreviousSibling(RenderLayer* prev) { m_previous = prev; }
    void setParent(RenderLayer* parent) { m_parent = parent; }
    void setFirstChild(RenderLayer* first) { m_first = first; }
    void setLastChild(RenderLayer* last) { m_last = last; }

protected:
    RenderObject* m_object;

    RenderLayer *m_parent;
    RenderLayer *m_previous;
    RenderLayer *m_next;

    RenderLayer *m_first;
    RenderLayer *m_last;

    // Our (x,y) coordinates are in our parent layer's coordinate space.
    int m_y;
    short m_x;

    // Our scroll offsets if the view is scrolled.
    short m_scrollX;
    int m_scrollY;

    // The width/height of our scrolled area.
    short m_scrollWidth;
    short m_scrollHeight;

    // For layers with overflow, we have a pair of scrollbars.
    QScrollBar* m_hBar;
    QScrollBar* m_vBar;
    RenderScrollMediator* m_scrollMediator;
    struct PositionedLayer {
	RenderLayer *layer;
	// these are relative to the layer holding the stacking context
	int idx; // used by sorting to preserve doc order
	bool operator == (const PositionedLayer &o) const { return layer == o.layer; }
	bool operator < (const PositionedLayer &o) const {
	    if (layer == o.layer)
		return false;
	    if (layer->renderer()->style()->zIndex() != o.layer->renderer()->style()->zIndex())
		return layer->renderer()->style()->zIndex() < o.layer->renderer()->style()->zIndex();
	    return idx < o.idx;
	}
    };
    QValueVector<PositionedLayer> *zOrderList;
    void collectLayers(QValueVector<PositionedLayer> *, int tx, int ty, int &idx);
};

}; // namespace
#endif
