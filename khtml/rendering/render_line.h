/*
 * This file is part of the line box implementation for KDE.
 *
 * Copyright (C) 2003 Apple Computer, Inc.
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
 *
 */
#ifndef RENDER_LINE_H
#define RENDER_LINE_H

namespace khtml {

class InlineFlowBox;
class RootInlineBox;

// InlineBox represents a rectangle that occurs on a line.  It corresponds to
// some RenderObject (i.e., it represents a portion of that RenderObject).
class InlineBox
{
public:
    InlineBox(RenderObject* obj)
    :m_object(obj), m_x(0), m_width(0), m_y(0), m_height(0), m_baseline(0),
     m_firstLine(false), m_constructed(false)
    {
        m_next = 0;
        m_prev = 0;
        m_parent = 0;
    }

    virtual ~InlineBox() {}

    void detach(RenderArena* renderArena);

    // Overloaded new operator.
    void* operator new(size_t sz, RenderArena* renderArena) throw();

    // Overridden to prevent the normal delete from being called.
    void operator delete(void* ptr, size_t sz);

private:
    // The normal operator new is disallowed.
    void* operator new(size_t sz) throw();

public:
    virtual bool isInlineBox() const { return false; }
    virtual bool isInlineFlowBox() const { return false; }
    virtual bool isContainer() const { return false; }
    virtual bool isInlineTextBox() const { return false; }
    virtual bool isRootInlineBox() const { return false; }

    bool isConstructed() const { return m_constructed; }
    virtual void setConstructed() {
        m_constructed = true;
        if (m_next)
            m_next->setConstructed();
    }

    void setFirstLineStyleBit(bool f) { m_firstLine = f; }

    InlineBox* nextOnLine() { return m_next; }
    InlineBox* prevOnLine() { return m_prev; }
    RenderObject* object() const { return m_object; }

    InlineFlowBox* parent() { return m_parent; }
    void setParent(InlineFlowBox* par) { m_parent = par; }

    RootInlineBox* root();

    void setWidth(short w) { m_width = w; }
    short width() const { return m_width; }

    void setXPos(short x) { m_x = x; }
    short xPos() const { return m_x; }

    void setYPos(int y) { m_y = y; }
    int yPos() const { return m_y; }

    void setHeight(int h) { m_height = h; }
    int height() const { return m_height; }

    void setBaseline(int b) { m_baseline = b; }
    int baseline() const { return m_baseline; }

    virtual bool hasTextChildren() const { return true; }

    virtual int topOverflow() const { return yPos(); }
    virtual int bottomOverflow() const { return yPos()+height(); }

    virtual long minOffset() const { return 0; }
    virtual long maxOffset() const { return 0; }

public: // FIXME: Would like to make this protected, but methods are accessing these
        // members over in the part.
    RenderObject* m_object;

    short m_x;
    short m_width;
    int m_y;
    int m_height;
    int m_baseline;

    bool m_firstLine : 1;
    bool m_constructed : 1;

    InlineBox* m_next; // The next element on the same line as us.
    InlineBox* m_prev; // The previous element on the same line as us.

    InlineFlowBox* m_parent; // The box that contains us.
};

class InlineRunBox : public InlineBox
{
public:
    InlineRunBox(RenderObject* obj)
    :InlineBox(obj)
    {
        m_prevLine = 0;
        m_nextLine = 0;
    }

    InlineRunBox* prevLineBox() { return m_prevLine; }
    InlineRunBox* nextLineBox() { return m_nextLine; }
    void setNextLineBox(InlineRunBox* n) { m_nextLine = n; }
    void setPreviousLineBox(InlineRunBox* p) { m_prevLine = p; }

    virtual void paintBackgroundAndBorder(RenderObject::PaintInfo&, int /*_tx*/, int /*_ty*/, int /*xOffsetOnLine*/) {}
    virtual void paintDecorations(RenderObject::PaintInfo&, int /*_tx*/, int /*_ty*/) {}

protected:
    InlineRunBox* m_prevLine;  // The previous box that also uses our RenderObject
    InlineRunBox* m_nextLine;  // The next box that also uses our RenderObject
};

class InlineFlowBox : public InlineRunBox
{
public:
    InlineFlowBox(RenderObject* obj)
    :InlineRunBox(obj)
    {
        m_firstChild = 0;
        m_lastChild = 0;
        m_includeLeftEdge = m_includeRightEdge = false;
        m_hasTextChildren = false;
        m_afterPageBreak = false;
    }

    virtual bool isInlineFlowBox() const { return true; }

    InlineBox* firstChild() const  { return m_firstChild; }
    InlineBox* lastChild() const { return m_lastChild; }

    virtual void setConstructed() {
        InlineBox::setConstructed();
        if (m_firstChild)
            m_firstChild->setConstructed();
    }
    void addToLine(InlineBox* child) {
        if (!m_firstChild)
            m_firstChild = m_lastChild = child;
        else {
            m_lastChild->m_next = child;
            child->m_prev = m_lastChild;
            m_lastChild = child;
        }
        child->setFirstLineStyleBit(m_firstLine);
        child->setParent(this);
        if (child->isInlineTextBox())
            m_hasTextChildren = true;
    }
    void removeFromLine(InlineBox* child);
    virtual void paintBackgroundAndBorder(RenderObject::PaintInfo&, int _tx, int _ty, int xOffsetOnLine);
    void paintBackgrounds(QPainter* p, const QColor& c, const BackgroundLayer* bgLayer,
                          int my, int mh, int _tx, int _ty, int w, int h, int xoff);
    void paintBackground(QPainter* p, const QColor& c, const BackgroundLayer* bgLayer,
                         int my, int mh, int _tx, int _ty, int w, int h, int xoff);
    virtual void paintDecorations(RenderObject::PaintInfo&, int _tx, int _ty);

    int marginBorderPaddingLeft() const;
    int marginBorderPaddingRight() const;
    int marginLeft() const;
    int marginRight( )const;
    int borderLeft() const { if (includeLeftEdge()) return object()->borderLeft(); return 0; }
    int borderRight() const { if (includeRightEdge()) return object()->borderRight(); return 0; }
    int paddingLeft() const { if (includeLeftEdge()) return object()->paddingLeft(); return 0; }
    int paddingRight() const { if (includeRightEdge()) return object()->paddingRight(); return 0; }

    bool includeLeftEdge() const { return m_includeLeftEdge; }
    bool includeRightEdge() const { return m_includeRightEdge; }
    void setEdges(bool includeLeft, bool includeRight) {
        m_includeLeftEdge = includeLeft;
        m_includeRightEdge = includeRight;
    }
    virtual bool hasTextChildren() const { return m_hasTextChildren; }

    // Helper functions used during line construction and placement.
    void determineSpacingForFlowBoxes(bool lastLine, RenderObject* endObject);
    int getFlowSpacingWidth() const;
    bool nextOnLineExists();
    bool prevOnLineExists();
    bool onEndChain(RenderObject* endObject);
    int placeBoxesHorizontally(int x);
    void verticallyAlignBoxes(int& heightOfBlock);
    void computeLogicalBoxHeights(int& maxPositionTop, int& maxPositionBottom,
                                  int& maxAscent, int& maxDescent, bool strictMode);
    void adjustMaxAscentAndDescent(int& maxAscent, int& maxDescent,
                                   int maxPositionTop, int maxPositionBottom);
    void placeBoxesVertically(int y, int maxHeight, int maxAscent, bool strictMode,
                              int& topPosition, int& bottomPosition);
    void shrinkBoxesWithNoTextChildren(int topPosition, int bottomPosition);

    virtual void setOverflowPositions(int /*top*/, int /*bottom*/) {}

    void setAfterPageBreak(bool b = true) { m_afterPageBreak = b; }
    bool afterPageBreak() const { return m_afterPageBreak; }

protected:
    InlineBox* m_firstChild;
    InlineBox* m_lastChild;
    bool m_includeLeftEdge : 1;
    bool m_includeRightEdge : 1;
    bool m_hasTextChildren : 1;
    bool m_afterPageBreak : 1;
};

class RootInlineBox : public InlineFlowBox
{
public:
    RootInlineBox(RenderObject* obj)
    :InlineFlowBox(obj)
    {
        m_topOverflow = m_bottomOverflow = 0;
    }

    virtual bool isRootInlineBox() const { return true; }
    virtual int topOverflow() const { return m_topOverflow; }
    virtual int bottomOverflow() const { return m_bottomOverflow; }
    virtual void setOverflowPositions(int top, int bottom) { m_topOverflow = top; m_bottomOverflow = bottom; }

protected:
    int m_topOverflow;
    int m_bottomOverflow;
};

} //namespace

#endif
