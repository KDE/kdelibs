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
#ifndef render_object_h
#define render_object_h

#include <qcolor.h>
#include <qsize.h>
#include <qrect.h>

#include "xml/dom_nodeimpl.h"
#include "misc/khtmllayout.h"
#include "misc/loader_client.h"

#include "render_style.h"

class QPainter;
class CSSStyle;
class KHTMLView;

namespace DOM {
    class DOMString;
    class NodeImpl;
};

namespace khtml {

    class RenderStyle;
    class RenderTable;
    class CachedObject;

class RenderObject : public CachedObjectClient
{
public:

    RenderObject();
    virtual ~RenderObject();

    static RenderObject *createObject(DOM::NodeImpl *node);

    DOM::ActivationState hasKeyboardFocus;

    virtual const char *renderName() const { return "RenderObject"; }
    virtual void printTree(int indent=0) const;

    void setParent(RenderObject *parent) { m_parent = parent; }
    RenderObject *parent() const { return m_parent; }

    RenderObject *previousSibling() const { return m_previous; }
    RenderObject *nextSibling() const { return m_next; }
    void setPreviousSibling(RenderObject *previous) { m_previous = previous; }
    void setNextSibling(RenderObject *next) { m_next = next; }

    RenderObject *firstChild() const { return m_first; }
    RenderObject *lastChild() const { return m_last; }
    void setFirstChild(RenderObject *first) { m_first = first; }
    void setLastChild(RenderObject *last) { m_last = last; }

    virtual void addChild(RenderObject *newChild, RenderObject *beforeChild = 0);
    virtual void removeChild(RenderObject *oldChild);

    // some helper functions...
    /**
     * is an Element that should be floated in the textstream
     */
    virtual bool childrenInline() const { return false; }
    virtual bool isRendered() const { return false; }
    virtual bool isFlow() const { return false; }

    virtual bool isListItem() const { return false; }
    virtual bool isRoot() const { return false; }
    virtual bool isBR() const { return false; }
    virtual bool isHtml() const { return false; }
    virtual bool isTableCell() const { return false; }
    virtual bool isTableRow() const { return false; }
    virtual bool isTableSection() const { return false; }
    virtual bool isTable() const { return false; }

    bool isAnonymousBox() const { return m_isAnonymous; }
    void setIsAnonymousBox(bool b) { m_isAnonymous = b; }

    bool isFloating() const { return m_floating; }
    bool isPositioned() const { return m_positioned; } // absolute or fixed positioning
    bool isRelPositioned() const { return m_relPositioned; } // relative positioning
    bool isText() const  { return m_isText; }   // inherits RenderText
    bool isInline() const { return m_inline; }  // inline object
    bool isReplaced() const { return m_replaced; } // a "replaced" element (see CSS)

    bool layouted() const   { return m_layouted; }
    bool parsing() const    { return m_parsing;     }
    bool minMaxKnown() const{ return m_minMaxKnown; }
    bool containsPositioned() const { return m_containsPositioned; }
        
        // absolute relative or fixed positioning

    void setContainsPositioned(bool p);

    void setLayouted(bool b=true) { m_layouted = b; }
    void setParsing(bool b=true) { m_parsing = b; }
    void setMinMaxKnown(bool b=true) { m_minMaxKnown = b; }

    virtual short baselineOffset() const { return 0; }
    virtual short verticalPositionHint() const { return 0; }

    /**
     * Print the object and it's children, but only if it fits in the
     * rectangle given by x,y,w,h. (tx|ty) is parents position.
     */
    virtual void print( QPainter *p, int x, int y,
                        int w, int h, int tx, int ty)
    { printObject(p, x, y, w, h, tx, ty); }

    /**
     * assumes (_tx/_ty) point to the upper left corner of the object
     * prints only this object without calling print for the children.
     */
    virtual void printObject( QPainter */*p*/, int /*x*/, int /*y*/,
                        int /*w*/, int /*h*/, int /*tx*/, int /*ty*/) {}


    /**
     * This function calculates the minimum & maximum width that the object
     * can be set to.
     *
     * when the Element calls setMinMaxKnown(true), calcMinMaxWidth() will
     * be no longer called.
     *
     * when a element has a fixed size, m_minWidth and m_maxWidth should be
     * set to the same value. This has the special meaning that m_width,
     * contains the actual value.
     *
     * ### assumes calcMinMaxWidth has already been called for all children.
     */
    virtual void calcMinMaxWidth() { }

    /**
     * Calculates the actual width of the object (only for non inline
     * objects)
     */
    virtual void calcWidth() {}

    /**
     * This function should cause the Element to calculate its
     * width and height and the layout of it's content
     *
     * if deep is true, the Element should also layout all it's
     * direct child items.
     *
     * when the Element calls setLayouted(true), layout() is no
     * longer called during relayouts, as long as there is no
     * style sheet change. When that occurs, isLayouted will be
     * set to false and the Element receives layout() calls
     * again.
     */
    virtual void layout() = 0;

    /**
     * this function get's called, if a child changed it's geometry
     * (because an image got loaded or some changes in the DOM...)
     */
    virtual void updateSize();

    /**
     * this function get's called, if a child changed it's height
     * (because an image got loaded or some changes in the DOM...)
     */
    virtual void updateHeight() {}

    /**
     * This function gets called, when the parser leaves the element
     *
     */
    virtual void close() { setParsing(false); }

    /**
     * set the style of the object. This _has_ to be called after
     * the objects constructor to set the correct style. Also used for
     * dhtml to change the objects current style.
     *
     * If changing the style dynamically, you might need to call
     * updateSize() after applying the style change to force a
     * relayout/repaint
     */
    virtual void setStyle(RenderStyle *style);


    /**
     * returns the containing block level element for this element.
     * needed to compute margins and paddings
     *
     * objects with positioning set to absolute and fixed have to be added
     * to this objects rendering list
     *
     * function must not be called before the Element has been added
     * to the Renderingtree.
     */
    RenderObject *containingBlock() const;

    /** return the size of the containing block.
     * Needed for layout
     * calculations, see CSS2 specs, 10.1
     */
    virtual QSize containingBlockSize() const;

    /**
     * returns the width of the block the current Element is in. useful
     * for relative width/height calculations.
     *
     * must not be called before the Element has been added to the
     * Renderingtree.
     */
    virtual short containingBlockWidth() const;
    virtual int containingBlockHeight() const;

    // the size of the content area
    virtual QSize contentSize() const;
    virtual short contentWidth() const { return 0; }
    virtual int contentHeight() const { return 0; }

    // Intrinsic size of replaced element
    virtual short intrinsicWidth() const { return 0; }
    virtual int intrinsicHeight() const { return 0; }


    // the offset of the contents relative to the box borders (basically border+padding)
    virtual QSize contentOffset() const;
    // the size of the content + padding
    virtual QSize paddingSize() const;
    // the size of the box (including padding and border)
    virtual QSize size() const;

    virtual void setPos( int /*xPos*/, int /*yPos*/ ) { }
    virtual void setXPos( int /*xPos*/ ) { }
    virtual void setYPos( int /*yPos*/ ) { }

    virtual void setSize( int /*width*/, int /*height*/ ) { }
    virtual void setWidth( int /*width*/ ) { }
    virtual void setHeight( int /*height*/ ) { }

    /**
     * Get X-Position of this object relative to its parent
     */
    virtual int xPos() const { return 0; }

    /**
     * Get Y-Position of this object relative to its parent
     */
    virtual int yPos() const { return 0; }

    virtual void absolutePosition(int &/*xPos*/, int &/*yPos*/);

    // width and height are without margins but include paddings and borders
    virtual short width() const { return 0; }
    virtual int height() const { return 0; }

    // these are just for convinience
    virtual short marginTop() const { return 0; }
    virtual short marginBottom() const { return 0; }
    virtual short marginLeft() const { return 0; }
    virtual short marginRight() const { return 0; }

    int paddingTop() const;
    int paddingBottom() const;
    int paddingLeft() const;
    int paddingRight() const;

    int borderTop() const { return m_style->borderTopWidth(); }
    int borderBottom() const { return m_style->borderBottomWidth(); }
    int borderLeft() const { return m_style->borderLeftWidth(); }
    int borderRight() const { return m_style->borderRightWidth(); }

    virtual short minWidth() const { return 0; }
    virtual short maxWidth() const { return 0; }

    virtual RenderStyle* style() const { return m_style; }

    enum BorderSide {
        BSTop, BSBottom, BSLeft, BSRight
    };
    void drawBorder(QPainter *p, int x1, int y1, int x2, int y2, int width, BorderSide s,
                    const QColor &c, EBorderStyle style);

    virtual void setTable(RenderTable*) {};

    /*
     * force a full repaint of the rendering tree
     */
    virtual void repaint() { if(m_parent) m_parent->repaint(); }
    virtual void repaintRectangle(int x, int y, int w, int h);
    virtual void repaintObject(RenderObject *o, int x, int y);

    virtual unsigned int length() const { return 0; }

    virtual bool isHidden() const { return isFloating() || isPositioned(); }
    /*
     * Special objects are objects that should be floated
     * but draw themselves (i.e. have content)
     */
    bool isSpecial() const;
    virtual bool containsSpecial() { return false; }
    virtual bool hasOverhangingFloats() { return false; }
    
    virtual int bidiHeight() const { return 0; }
    virtual void position(int, int, int, int, int, bool) {}

    enum SelectionState {
        SelectionNone,
        SelectionStart,
        SelectionInside,
        SelectionEnd,
        SelectionBoth
    };

    virtual SelectionState selectionState() const { return SelectionNone;}
    virtual void setSelectionState(SelectionState) {}

    virtual void cursorPos(int /*offset*/, int &/*_x*/, int &/*_y*/, int &/*height*/);

    virtual void setKeyboardFocus(DOM::ActivationState b=DOM::ActivationPassive);// { hasKeyboardFocus=b; };

    virtual int lowestPosition() const {return 0;}

    CachedImage *backgroundImage() const { return m_bgImage; }

protected:
    virtual void selectionStartEnd(int& spos, int& epos);

    virtual void printBoxDecorations(QPainter* /*p*/, int /*_x*/, int /*_y*/,
                                     int /*_w*/, int /*_h*/, int /*_tx*/, int /*_ty*/) {}

    virtual QRect viewRect() const;

    RenderStyle *m_style;
    RenderObject *m_parent;
    RenderObject *m_previous;
    RenderObject *m_next;
    RenderObject *m_first;
    RenderObject *m_last;

    CachedImage *m_bgImage;

    bool m_layouted       : 1;
    bool m_parsing        : 1;
    bool m_minMaxKnown    : 1;
    bool m_floating       : 1;

    bool m_positioned     : 1;
    bool m_containsPositioned     : 1;
    bool m_relPositioned  : 1;
    bool m_printSpecial   : 1; // if the box has something special to print (background, border, etc)
    bool m_isAnonymous    : 1;
    bool m_visible        : 1;
    bool m_isText         : 1;
    bool m_inline         : 1;
    bool m_replaced       : 1;
};


    enum VerticalPositionHint {
        PositionTop = -1,
        PositionBottom = -2
    };



}; //namespace
#endif
