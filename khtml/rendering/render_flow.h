/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
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
#ifndef RENDER_FLOW_H
#define RENDER_FLOW_H

#include <qptrlist.h>

#include "render_box.h"
#include "bidi.h"

namespace khtml {

/**
 * all geometry managing stuff is only in the block elements.
 *
 * Inline elements don't layout themselves, but the whole paragraph
 * gets layouted by the surrounding block element. This is, because
 * one needs to know the whole paragraph to calculate bidirectional
 * behaviour of text, so putting the layouting routines in the inline
 * elements is impossible.
 */
class RenderFlow : public RenderBox
{

public:
    RenderFlow(DOM::NodeImpl* node);

    virtual ~RenderFlow();

    virtual const char *renderName() const { return "RenderFlow"; }

    virtual void setStyle(RenderStyle *style);

    virtual bool isFlow() const { return true; }
    virtual bool childrenInline() const { return m_childrenInline; }
    void setChildrenInline(bool b) { m_childrenInline = b; }
    virtual bool isRendered() const { return true; }

    virtual RenderFlow* continuation() const { return m_continuation; }
    void setContinuation(RenderFlow* c) { m_continuation = c; }
    RenderFlow* continuationBefore(RenderObject* beforeChild);

    void splitInlines(RenderFlow* fromBlock, RenderFlow* toBlock, RenderFlow* middleBlock,
                      RenderObject* beforeChild, RenderFlow* oldCont);
    void splitFlow(RenderObject* beforeChild, RenderFlow* newBlockBox,
                   RenderObject* newChild, RenderFlow* oldCont);
    void addChildWithContinuation(RenderObject* newChild, RenderObject* beforeChild);
    void addChildToFlow(RenderObject* newChild, RenderObject* beforeChild);
    void removeChild(RenderObject *oldChild);

    void makeChildrenNonInline(RenderObject *box2Start = 0);

    // overrides RenderObject

    virtual void paint( QPainter *, int x, int y, int w, int h,
                        int tx, int ty, PaintAction paintPhase);
    virtual void paintObject( QPainter *, int x, int y, int w, int h,
                        int tx, int ty, PaintAction paintPhase);
    void paintFloats( QPainter *, int x, int y, int w, int h,
                        int tx, int ty);

    virtual bool requiresLayer() {
	return (isPositioned() || isRelPositioned() ||
		style()->overflow()==OHIDDEN) && !isTableCell();
    }

    virtual void layout( );

    virtual void close();

    virtual void addChild(RenderObject *newChild, RenderObject *beforeChild = 0);

    virtual short lineWidth(int y) const;

    virtual int lowestPosition() const;
    virtual int rightmostPosition() const;

    int rightOffset() const;
    int rightRelOffset(int y, int fixedOffset, int *heightRemaining = 0) const;
    int rightOffset(int y) const { return rightRelOffset(y, rightOffset()); }

    int leftOffset() const;
    int leftRelOffset(int y, int fixedOffset, int *heightRemaining = 0) const;
    int leftOffset(int y) const { return leftRelOffset(y, leftOffset()); }

#ifndef NDEBUG
    virtual void printTree(int indent=0) const;
    virtual void dump(QTextStream *stream, QString ind = "") const;
#endif

    virtual FindSelectionResult checkSelectionPoint( int _x, int _y, int _tx, int _ty,
                                                     DOM::NodeImpl*&, int & offset );

    virtual bool nodeAtPoint(NodeInfo& info, int x, int y, int tx, int ty);

protected:

    virtual void newLine();

    void layoutBlockChildren( bool relayoutChildren );
    void layoutInlineChildren( bool relayoutChildren );
    void layoutPositionedObjects( bool relayoutChildren );

public:
    int floatBottom() const;
    inline int leftBottom();
    inline int rightBottom();
    bool checkClear(RenderObject *child);

    void insertFloatingObject(RenderObject *o);
    void removeFloatingObject(RenderObject *o);
    // called from lineWidth, to position the floats added in the last line.
    void positionNewFloats();
    void clearFloats();
    virtual void calcMinMaxWidth();

    virtual bool containsFloating() { return floatingObjects!=0; }
    virtual bool hasOverhangingFloats() { return floatBottom() > m_height; }

    void addOverHangingFloats( RenderFlow *flow, int xoffset, int yoffset, bool child = false );

    // implementation of the following functions is in bidi.cpp
    void bidiReorderLine(const BidiIterator &start, const BidiIterator &end);
    BidiIterator findNextLineBreak(BidiIterator &start);

    // The height (and width) of a block when you include overflow
    // spillage out of the bottom of the block (e.g., a <div
    // style="height:25px"> that has a 100px tall image inside it
    // would have an overflow height of borderTop() + paddingTop() +
    // 100px.
    virtual int overflowHeight() const { return m_overflowHeight; }
    virtual int overflowWidth() const { return m_overflowWidth; }

protected:

    struct FloatingObject {
        enum Type {
            FloatLeft,
            FloatRight
	};

        FloatingObject(Type _type) {
	    node = 0;
	    startY = 0;
	    endY = 0;
	    type = _type;
	    left = 0;
	    width = 0;
            noPaint = false;

        }
        RenderObject* node;
        int startY;
        int endY;
        short left;
        short width;
        Type type : 1; // left or right aligned
        bool noPaint : 1;
    };

    QPtrList<FloatingObject>* floatingObjects;

private:
    bool m_childrenInline    : 1;
    bool m_pre               : 1;
    bool firstLine           : 1; // used in inline layouting
    EClear m_clearStatus     : 2; // used during layuting of paragraphs
    bool m_topMarginQuirk    : 1;
    bool m_bottomMarginQuirk : 1;
    uint unused : 9;

    short m_maxTopPosMargin;
    short m_maxTopNegMargin;
    short m_maxBottomPosMargin;
    short m_maxBottomNegMargin;

    // width/height of overflowing contents
    short m_overflowWidth;
    int m_overflowHeight;

    // An inline can be split with blocks occurring in between the inline content.
    // When this occurs we need a pointer to our next object.  We can basically be
    // split into a sequence of inlines and blocks.  The continuation will either be
    // an anonymous block (that houses other blocks) or it will be an inline flow.
    RenderFlow* m_continuation;
};


}; //namespace

#endif
