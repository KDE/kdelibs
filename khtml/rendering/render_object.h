/*
 * This file is part of the html renderer for KDE.
 *
 * Copyright (C) 2000-2003 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000-2003 Dirk Mueller (mueller@kde.org)
 *           (C) 2002-2003 Apple Computer, Inc.
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
#include <qrect.h>
#include <assert.h>
#include <qvaluelist.h>

#include <kdebug.h>
#include <kglobal.h>

#include "xml/dom_docimpl.h"
#include "misc/khtmllayout.h"
#include "misc/loader_client.h"
#include "misc/helper.h"
#include "rendering/render_style.h"

class QPainter;
class QTextStream;
class CSSStyle;
class KHTMLView;

#ifndef NDEBUG
#define KHTMLAssert( x ) if( !(x) ) { \
    const RenderObject *o = this; while( o->parent() ) o = o->parent(); \
    o->printTree(); \
    qDebug(" this object = %p, %s", (void*) this, kdBacktrace().latin1() ); \
    assert( x ); \
}
#else
#define KHTMLAssert( x )
#endif

/*
 *	The painting of a layer occurs in three distinct phases.  Each phase involves
 *	a recursive descent into the layer's render objects. The first phase is the background phase.
 *	The backgrounds and borders of all blocks are painted.  Inlines are not painted at all.
 *	Floats must paint above block backgrounds but entirely below inline content that can overlap them.
 *	In the foreground phase, all inlines are fully painted.  Inline replaced elements will get all
 *	three phases invoked on them during this phase.
 */

typedef enum {
    PaintActionElementBackground = 0,
    PaintActionChildBackground,
    PaintActionChildBackgrounds,
    PaintActionFloat,
    PaintActionForeground,
    PaintActionOutline,
    PaintActionSelection,
    PaintActionCollapsedTableBorders
} PaintAction;

typedef enum {
    HitTestAll = 0,
    HitTestSelfOnly = 1,
    HitTestChildrenOnly = 2
} HitTestAction;

namespace DOM {
    class HTMLAreaElementImpl;
    class DOMString;
    class NodeImpl;
    class DocumentImpl;
    class ElementImpl;
    class EventImpl;
}

namespace khtml {
    class RenderFlow;
    class RenderStyle;
    class RenderTable;
    class CachedObject;
    class RenderObject;
    class RenderCanvas;
    class RenderText;
    class RenderFrameSet;
    class RenderArena;
    class RenderLayer;
    class RenderBlock;
    class InlineBox;
    class InlineFlowBox;

/**
 * Base Class for all rendering tree objects.
 */
class RenderObject : public CachedObjectClient
{
    RenderObject(const RenderObject&);
    RenderObject& operator=(const RenderObject&);
public:

    RenderObject(DOM::NodeImpl* node);
    virtual ~RenderObject();

    RenderObject *parent() const { return m_parent; }

    RenderObject *previousSibling() const { return m_previous; }
    RenderObject *nextSibling() const { return m_next; }

    virtual RenderObject *firstChild() const { return 0; }
    virtual RenderObject *lastChild() const { return 0; }

    virtual bool childAllowed() const { return false; }

    virtual RenderLayer* layer() const { return 0; }
    RenderLayer* enclosingLayer() const;
    void addLayers(RenderLayer* parentLayer, RenderLayer* beforeChild=0);
    void removeLayers(RenderLayer* parentLayer);
    void moveLayers(RenderLayer* oldParent, RenderLayer* newParent);
    RenderLayer* findNextLayer(RenderLayer* parentLayer, RenderObject* startPoint,
                               bool checkParent=true);
    virtual void positionChildLayers() { }
    virtual bool requiresLayer() const {
        return isRoot() || (!isTableCell() && (isPositioned() || isRelPositioned()));
    }

    // ### rename to overflowClipRect and clipRect
    virtual QRect getOverflowClipRect(int /*tx*/, int /*ty*/)
	{ return QRect(0,0,0,0); }
    virtual QRect getClipRect(int /*tx*/, int /*ty*/) { return QRect(0,0,0,0); }
    bool hasClip() const { return isPositioned() &&  style()->hasClip(); }
    bool hasOverflowClip() const { return style()->hidesOverflow(); }

    virtual int getBaselineOfFirstLineBox() { return -1; } // Tables and blocks implement this.
    virtual InlineFlowBox* getFirstLineBox() { return 0; } // Tables and blocks implement this.

    // Whether or not a positioned element requires normal flow x/y to be computed
    // to determine its position.
    bool hasStaticX() const;
    bool hasStaticY() const;

    // Linear tree traversal
    RenderObject *objectBelow() const;
    RenderObject *objectAbove() const;

    // RenderObject tree manipulation
    //////////////////////////////////////////
    virtual void addChild(RenderObject *newChild, RenderObject *beforeChild = 0);
    void removeChild(RenderObject *oldChild);

    // raw tree manipulation
    virtual RenderObject* removeChildNode(RenderObject* child);
    virtual void appendChildNode(RenderObject* child);
    virtual void insertChildNode(RenderObject* child, RenderObject* before);
    //////////////////////////////////////////

    //////////////////////////////////////////
    // Helper functions. Dangerous to use!
    void setPreviousSibling(RenderObject *previous) { m_previous = previous; }
    void setNextSibling(RenderObject *next) { m_next = next; }
    void setParent(RenderObject *parent) { m_parent = parent; }
    //////////////////////////////////////////

public:
    virtual const char *renderName() const { return "RenderObject"; }
#ifdef ENABLE_DUMP
    QString information() const;
    virtual void printTree(int indent=0) const;
    virtual void dump(QTextStream &stream, const QString &ind = QString::null) const;
#endif

    static RenderObject *createObject(DOM::NodeImpl* node, RenderStyle* style);

    // Overloaded new operator.  Derived classes must override operator new
    // in order to allocate out of the RenderArena.
    void* operator new(size_t sz, RenderArena* renderArena) throw();

    // Overridden to prevent the normal delete from being called.
    void operator delete(void* ptr, size_t sz);

private:
    // The normal operator new is disallowed on all render objects.
    void* operator new(size_t sz);

public:
    RenderArena* renderArena() const;
    virtual RenderFlow* continuation() const { return 0; }
    virtual bool isInlineContinuation() const { return false; }


    bool isRoot() const;
    // some helper functions...
    virtual bool isRenderBlock() const { return false; }
    virtual bool isRenderInline() const { return false; }
    virtual bool isInlineFlow() const { return false; }
    virtual bool isBlockFlow() const { return false; }
    virtual bool isInlineBlockOrInlineTable() const { return false; }
    virtual bool childrenInline() const { return false; }
    virtual bool isBox() const { return false; }
    virtual bool isRenderReplaced() const { return false; }

    virtual bool isListItem() const { return false; }
    virtual bool isListMarker() const { return false; }
    virtual bool isCanvas() const { return false; }
    virtual bool isBR() const { return false; }
    virtual bool isTableCell() const { return false; }
    virtual bool isTableRow() const { return false; }
    virtual bool isTableSection() const { return false; }
    virtual bool isTableCol() const { return false; }
    virtual bool isTable() const { return false; }
    virtual bool isWidget() const { return false; }
    virtual bool isBody() const { return false; }
    virtual bool isFormElement() const { return false; }
    virtual bool isFrameSet() const { return false; }
    virtual bool isApplet() const { return false; }

    bool isHTMLMarquee() const;

    bool isAnonymous() const { return m_isAnonymous; }
    void setIsAnonymous(bool b) { m_isAnonymous = b; }

    bool isFloating() const { return m_floating; }
    bool isPositioned() const { return m_positioned; }
    bool isRelPositioned() const { return m_relPositioned; }
    bool isText() const { return m_isText; }
    bool isInline() const { return m_inline; }
    bool isCompact() const { return style()->display() == COMPACT; } // compact
    bool mouseInside() const;
    bool isReplaced() const { return m_replaced; }
    bool isReplacedBlock() const { return isInline() && isReplaced() && isRenderBlock(); }
    bool shouldPaintBackgroundOrBorder() const { return m_paintBackground; }
    bool needsLayout() const   { return m_needsLayout || m_normalChildNeedsLayout || m_posChildNeedsLayout; }
    bool selfNeedsLayout() const { return m_needsLayout; }
    bool posChildNeedsLayout() const { return m_posChildNeedsLayout; }
    bool normalChildNeedsLayout() const { return m_normalChildNeedsLayout; }
    bool minMaxKnown() const{ return m_minMaxKnown; }
    bool overhangingContents() const { return m_overhangingContents; }
    bool hasFirstLine() const { return m_hasFirstLine; }
    bool isSelectionBorder() const { return m_isSelectionBorder; }
    bool recalcMinMax() const { return m_recalcMinMax; }

    RenderCanvas* canvas() const;
    // don't even think about making this method virtual!
    DOM::DocumentImpl* document() const;
    DOM::NodeImpl* element() const { return isAnonymous() ? 0L : m_node; }
    DOM::NodeImpl* node() const { return m_node; }

   /**
     * returns the object containing this one. can be different from parent for
     * positioned elements
     */
    RenderObject *container() const;

    void setOverhangingContents(bool p=true);
    void markContainingBlocksForLayout();
    void setNeedsLayout(bool b, bool markParents = true);
    void setChildNeedsLayout(bool b, bool markParents = true);
    void setMinMaxKnown(bool b=true) {
	m_minMaxKnown = b;
	if ( !b ) {
	    RenderObject *o = this;
	    RenderObject *root = this;
	    while( o ) { // ### && !o->m_recalcMinMax ) {
		o->m_recalcMinMax = true;
		root = o;
		o = o->m_parent;
	    }
	}
    }
    void setNeedsLayoutAndMinMaxRecalc() {
        setMinMaxKnown(false);
        setNeedsLayout(true);
    }
    void setPositioned(bool b=true)  { m_positioned = b;  }
    void setRelPositioned(bool b=true) { m_relPositioned = b; }
    void setFloating(bool b=true) { m_floating = b; }
    void setInline(bool b=true) { m_inline = b; }
    void setMouseInside(bool b=true) { m_mouseInside = b; }
    void setShouldPaintBackgroundOrBorder(bool b=true) { m_paintBackground = b; }
    void setRenderText() { m_isText = true; }
    void setReplaced(bool b=true) { m_replaced = b; }
    void setIsSelectionBorder(bool b=true) { m_isSelectionBorder = b; }

    void scheduleRelayout(RenderObject *clippedObj = 0);

    virtual InlineBox* createInlineBox(bool makePlaceHolderBox, bool isRootLineBox);

    virtual short lineHeight( bool firstLine ) const;
    virtual short verticalPositionHint( bool firstLine ) const;
    virtual short baselinePosition( bool firstLine ) const;
    short getVerticalPosition( bool firstLine, RenderObject* ref=0 ) const;

    /*
     * Print the object and its children, clipped by (x|y|w|h).
     * (tx|ty) is the calculated position of the parent
     */
    struct PaintInfo {
       PaintInfo(QPainter* _p, const QRect& _r, PaintAction _phase)
           : p(_p), r(_r), phase(_phase) {}
       QPainter* p;
       QRect     r;
       PaintAction phase;
    };
    virtual void paint( PaintInfo& i, int tx, int ty);

    void paintBorder(QPainter *p, int _tx, int _ty, int w, int h, const RenderStyle* style, bool begin=true, bool end=true);
    void paintOutline(QPainter *p, int _tx, int _ty, int w, int h, const RenderStyle* style);

    virtual void paintBoxDecorations(PaintInfo&, int /*_tx*/, int /*_ty*/) {}

    virtual void paintBackgroundExtended(QPainter* /*p*/, const QColor& /*c*/, CachedImage* /*bg*/,
                                         int /*clipy*/, int /*cliph*/, int /*_tx*/, int /*_ty*/,
                                         int /*w*/, int /*height*/, int /*bleft*/, int /*bright*/ ) {}


    /*
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
     * assumes calcMinMaxWidth has already been called for all children.
     */
    virtual void calcMinMaxWidth() { }

    /*
     * Does the min max width recalculations after changes.
     */
    void recalcMinMaxWidths();

    /*
     * Calculates the actual width of the object (only for non inline
     * objects)
     */
    virtual void calcWidth() {}

    /*
     * This function should cause the Element to calculate its
     * width and height and the layout of its content
     *
     * when the Element calls setNeedsLayout(false), layout() is no
     * longer called during relayouts, as long as there is no
     * style sheet change. When that occurs, m_needsLayout will be
     * set to true and the Element receives layout() calls
     * again.
     */
    virtual void layout() = 0;

    /* This function performs a layout only if one is needed. */
    void layoutIfNeeded() { if (needsLayout()) layout(); }

    // used for element state updates that can not be fixed with a
    // repaint and do not need a relayout
    virtual void updateFromElement() {}

    // The corresponding closing element has been parsed. ### remove me
    virtual void close() { }

    virtual int availableHeight() const { return 0; }

    // Whether or not the element shrinks to its max width (rather than filling the width
    // of a containing block).  HTML4 buttons, legends, and floating/compact elements do this.
    bool sizesToMaxWidth() const;

    // does a query on the rendertree and finds the innernode
    // and overURL for the given position
    // if readonly == false, it will recalc hover styles accordingly
   class NodeInfo
    {
        friend class RenderImage;
        friend class RenderFlow;
        friend class RenderInline;
        friend class RenderText;
        friend class RenderObject;
        friend class RenderFrameSet;
	friend class RenderLayer;
        friend class DOM::HTMLAreaElementImpl;
    public:
        NodeInfo(bool readonly, bool active)
            : m_innerNode(0), m_innerNonSharedNode(0), m_innerURLElement(0), m_readonly(readonly), m_active(active)
            { }

        DOM::NodeImpl* innerNode() const { return m_innerNode; }
        DOM::NodeImpl* innerNonSharedNode() const { return m_innerNonSharedNode; }
        DOM::NodeImpl* URLElement() const { return m_innerURLElement; }
        bool readonly() const { return m_readonly; }
        bool active() const { return m_active; }

    private:
        void setInnerNode(DOM::NodeImpl* n) { m_innerNode = n; }
        void setInnerNonSharedNode(DOM::NodeImpl* n) { m_innerNonSharedNode = n; }
        void setURLElement(DOM::NodeImpl* n) { m_innerURLElement = n; }

        DOM::NodeImpl* m_innerNode;
        DOM::NodeImpl* m_innerNonSharedNode;
        DOM::NodeImpl* m_innerURLElement;
        bool m_readonly;
        bool m_active;
    };

    /** contains stateful information for a checkSelectionPoint call
     */
    struct SelPointState {
        /** last node that was before the current position */
        DOM::NodeImpl *m_lastNode;
	/** offset of last node */
	long m_lastOffset;
	/** true when the last node had the result SelectionAfterInLine */
	bool m_afterInLine;

	SelPointState() : m_lastNode(0), m_lastOffset(0), m_afterInLine(false)
	{}
    };

    virtual FindSelectionResult checkSelectionPoint( int _x, int _y, int _tx, int _ty,
                                                     DOM::NodeImpl*&, int & offset,
						     SelPointState & );
    virtual bool nodeAtPoint(NodeInfo& info, int x, int y, int tx, int ty, HitTestAction, bool inside = false);

    // set the style of the object.
    virtual void setStyle(RenderStyle *style);

    // returns the containing block level element for this element.
    RenderBlock *containingBlock() const;

    // return just the width of the containing block
    virtual short containingBlockWidth() const;
    // return just the height of the containing block
    virtual int containingBlockHeight() const;

    // size of the content area (box size minus padding/border)
    virtual short contentWidth() const { return 0; }
    virtual int contentHeight() const { return 0; }

    // intrinsic extend of replaced elements. undefined otherwise
    virtual short intrinsicWidth() const { return 0; }
    virtual int intrinsicHeight() const { return 0; }

    // relative to parent node
    virtual void setPos( int /*xPos*/, int /*yPos*/ ) { }
    virtual void setWidth( int /*width*/ ) { }
    virtual void setHeight( int /*height*/ ) { }

    virtual int xPos() const { return 0; }
    virtual int yPos() const { return 0; }

    /** Leftmost coordinate of this inline element relative to containing
     * block. Always zero for non-inline elements.
     */
    virtual int inlineXPos() const { return 0; }
    /** Topmost coordinate of this inline element relative to containing
     * block. Always zero for non-inline elements.
     */
    virtual int inlineYPos() const { return 0; }

    // calculate client position of box
    virtual bool absolutePosition(int &/*xPos*/, int &/*yPos*/, bool fixed = false);

    // width and height are without margins but include paddings and borders
    virtual short width() const { return 0; }
    virtual int height() const { return 0; }

    // The height of a block when you include overflow spillage out of
    // the bottom of the block (e.g., a <div style="height:25px"> that
    // has a 100px tall image inside it would have an overflow height
    // of borderTop() + paddingTop() + 100px.
    virtual int overflowHeight() const { return height(); }
    virtual int overflowWidth() const { return width(); }

    /**
     * Returns the height that is effectively considered when contemplating the
     * object as a whole -- usually the overflow height, or the height if clipped.
     */
    int effectiveHeight() const { return hasOverflowClip() ? height() : overflowHeight(); }
    /**
     * Returns the width that is effectively considered when contemplating the
     * object as a whole -- usually the overflow width, or the width if clipped.
     */
    int effectiveWidth() const { return hasOverflowClip() ? width() : overflowWidth(); }

    // IE extensions, heavily used in ECMA
    virtual short offsetWidth() const { return width(); }
    virtual int offsetHeight() const { return height(); }
    virtual int offsetLeft() const;
    virtual int offsetTop() const;
    virtual RenderObject* offsetParent() const;
    short clientWidth() const;
    int clientHeight() const;
    short scrollWidth() const;
    int scrollHeight() const;

    virtual bool isSelfCollapsingBlock() const { return false; }
    short collapsedMarginTop() const { return maxTopMargin(true)-maxTopMargin(false);  }
    short collapsedMarginBottom() const { return maxBottomMargin(true)-maxBottomMargin(false); }

    virtual bool isTopMarginQuirk() const { return false; }
    virtual bool isBottomMarginQuirk() const { return false; }
    virtual short maxTopMargin(bool positive) const
    { return positive ? kMax( int( marginTop() ), 0 ) : - kMin( int( marginTop() ), 0 ); }
    virtual short maxBottomMargin(bool positive) const
    { return positive ? kMax( int( marginBottom() ), 0 ) : - kMin( int( marginBottom() ), 0 ); }

    virtual short marginTop() const { return 0; }
    virtual short marginBottom() const { return 0; }
    virtual short marginLeft() const { return 0; }
    virtual short marginRight() const { return 0; }

    virtual int paddingTop() const;
    virtual int paddingBottom() const;
    virtual int paddingLeft() const;
    virtual int paddingRight() const;

    virtual int borderTop() const { return style()->borderTopWidth(); }
    virtual int borderBottom() const { return style()->borderBottomWidth(); }
    virtual int borderLeft() const { return style()->borderLeftWidth(); }
    virtual int borderRight() const { return style()->borderRightWidth(); }

    virtual short minWidth() const { return 0; }
    virtual short maxWidth() const { return 0; }

    RenderStyle* style() const { return m_style; }
    RenderStyle* style( bool firstLine ) const {
	RenderStyle *s = m_style;
	if( firstLine && hasFirstLine() ) {
	    RenderStyle *pseudoStyle  = style()->getPseudoStyle(RenderStyle::FIRST_LINE);
	    if ( pseudoStyle )
		s = pseudoStyle;
	}
	return s;
    }

    void getTextDecorationColors(int decorations, QColor& underline, QColor& overline,
                                 QColor& linethrough, bool quirksMode=false);

    enum BorderSide {
        BSTop, BSBottom, BSLeft, BSRight
    };
    void drawBorder(QPainter *p, int x1, int y1, int x2, int y2, BorderSide s,
                    QColor c, const QColor& textcolor, EBorderStyle style,
                    int adjbw1, int adjbw2, bool invalidisInvert = false);

    // Used by collapsed border tables.
    virtual void collectBorders(QValueList<CollapsedBorderValue>& borderStyles);

    // force a complete repaint
    virtual void repaint(bool immediate = false) { if(m_parent) m_parent->repaint(immediate); }
    virtual void repaintRectangle(int x, int y, int w, int h, bool immediate = false, bool f=false);

    virtual unsigned int length() const { return 1; }

    virtual bool isHidden() const { return isFloating() || isPositioned(); }

    // Special objects are objects that are neither really inline nor blocklevel
    bool isFloatingOrPositioned() const { return (isFloating() || isPositioned()); };
    virtual bool hasOverhangingFloats() const { return false; }
    virtual bool hasFloats() const { return false; }
    virtual bool containsFloat(RenderObject* /*o*/) const { return false; }
    virtual void markAllDescendantsWithFloatsForLayout(RenderObject* /*floatToRemove*/ = 0) {}

    bool usesLineWidth() const;

    // positioning of inline children (bidi)
    virtual void position(InlineBox*, int, int, bool) {}
//    virtual void position(int, int, int, int, int, bool, bool, int) {}

    // Applied as a "slop" to dirty rect checks during the outline painting phase's dirty-rect checks.
    int maximalOutlineSize(PaintAction p) const;

    enum SelectionState {
        SelectionNone,
        SelectionStart,
        SelectionInside,
        SelectionEnd,
        SelectionBoth
    };

    virtual SelectionState selectionState() const { return SelectionNone;}
    virtual void setSelectionState(SelectionState) {}

    /**
     * Flags which influence the appearence and position
     * @param CFOverride input overrides existing character, caret should be
     *		cover the whole character
     * @param CFOutside coordinates are to be interpreted outside of the
     *		render object
     * @param CFOutsideEnd coordinates are to be interpreted at the outside
     *		end of the render object (only valid if CFOutside is also set)
     */
    enum CaretFlags { CFOverride = 0x01, CFOutside = 0x02, CFOutsideEnd = 0x04 };

    /**
     * Returns the content coordinates of the caret within this render object.
     * @param offset zero-based offset determining position within the render object.
     * @param flags combination of enum CaretFlags
     * @param _x returns the left coordinate
     * @param _y returns the top coordinate
     * @param width returns the caret's width
     * @param height returns the caret's height
     */
    virtual void caretPos(int offset, int flags, int &_x, int &_y, int &width, int &height);

    // returns the lowest position of the lowest object in that particular object.
    // This 'height' is relative to the topleft of the margin box of the object.
    // Implemented in RenderFlow.
    virtual int lowestPosition(bool /*includeOverflowInterior*/=true, bool /*includeSelf*/=true) const { return 0; }
    virtual int rightmostPosition(bool /*includeOverflowInterior*/=true, bool /*includeSelf*/=true) const { return 0; }
    virtual int leftmostPosition(bool /*includeOverflowInterior*/=true, bool /*includeSelf*/=true) const { return 0; }

    // recursively invalidate current layout
    // unused: void invalidateLayout();

    virtual void calcVerticalMargins() {}
    void removeFromObjectLists();

    virtual void deleteInlineBoxes(RenderArena* arena=0) {(void)arena;}
    virtual void detach( );

    const QFont &font(bool firstLine) const {
	return style( firstLine )->font();
    }

    const QFontMetrics &fontMetrics(bool firstLine) const {
	return style( firstLine )->fontMetrics();
    }

    /** returns the lowest possible value the caret offset may have to
     * still point to a valid position.
     *
     * Returns 0 by default.
     */
    virtual long minOffset() const { return 0; }
    /** returns the highest possible value the caret offset may have to
     * still point to a valid position.
     *
     * Returns 0 by default, as generic elements are considered to have no
     * width.
     */
    virtual long maxOffset() const { return 0; }

    virtual void setPixmap(const QPixmap &, const QRect&, CachedImage *);

protected:
    virtual void selectionStartEnd(int& spos, int& epos);

    virtual QRect viewRect() const;
    void remove();
    void invalidateVerticalPositions();


    virtual void removeLeftoverAnonymousBoxes();

    void arenaDelete(RenderArena *arena);

private:
    RenderStyle* m_style;
    DOM::NodeImpl* m_node;
    RenderObject *m_parent;
    RenderObject *m_previous;
    RenderObject *m_next;

    short m_verticalPosition;

    bool m_needsLayout               : 1;
    bool m_normalChildNeedsLayout    : 1;
    bool m_posChildNeedsLayout       : 1;
    bool m_minMaxKnown               : 1;
    bool m_floating                  : 1;

    bool m_positioned                : 1;
    bool m_overhangingContents       : 1;
    bool m_relPositioned             : 1;
    bool m_paintBackground           : 1; // if the box has something to paint in the
                                          // background painting phase (background, border, etc)

    bool m_isAnonymous               : 1;
    bool m_recalcMinMax 	     : 1;
    bool m_isText                    : 1;
    bool m_inline                    : 1;

    bool m_replaced                  : 1;
    bool m_mouseInside               : 1;
    bool m_hasFirstLine              : 1;
    bool m_isSelectionBorder         : 1;
    // note: do not add unnecessary bitflags, we have 32 bit already!


    void arenaDelete(RenderArena *arena, void *objectBase);

    friend class RenderLayer;
    friend class RenderListItem;
    friend class RenderContainer;
    friend class RenderCanvas;
};


enum VerticalPositionHint {
    PositionTop = -0x4000,
    PositionBottom = 0x4000,
    PositionUndefined = 0x3fff
};

} //namespace
#endif
