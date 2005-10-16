/**
 * This file is part of the html renderer for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000-2003 Dirk Mueller (mueller@kde.org)
 *           (C) 2002-2004 Apple Computer, Inc.
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

#include "rendering/render_object.h"
#include "rendering/render_table.h"
#include "rendering/render_list.h"
#include "rendering/render_canvas.h"
#include "rendering/render_block.h"
#include "rendering/render_arena.h"
#include "rendering/render_layer.h"
#include "rendering/render_line.h"
#include "rendering/render_inline.h"
#include "rendering/render_text.h"
#include "rendering/render_replaced.h"
#include "rendering/render_generated.h"
#include "rendering/counter_tree.h"

#include "xml/dom_elementimpl.h"
#include "xml/dom_docimpl.h"
#include "dom/dom_doc.h"
#include "misc/htmlhashes.h"
#include "misc/loader.h"

#include <kdebug.h>
#include <kglobal.h>
#include <qpainter.h>
#include "khtmlview.h"
#include <khtml_part.h>

#include <assert.h>
using namespace DOM;
using namespace khtml;

#define RED_LUMINOSITY        30
#define GREEN_LUMINOSITY      59
#define BLUE_LUMINOSITY       11
#define INTENSITY_FACTOR      25
#define LIGHT_FACTOR          0
#define LUMINOSITY_FACTOR     75

#define MAX_COLOR             255
#define COLOR_DARK_THRESHOLD  51
#define COLOR_LIGHT_THRESHOLD 204

#define COLOR_LITE_BS_FACTOR 45
#define COLOR_LITE_TS_FACTOR 70

#define COLOR_DARK_BS_FACTOR 30
#define COLOR_DARK_TS_FACTOR 50

#define LIGHT_GRAY qRgb(192, 192, 192)
#define DARK_GRAY  qRgb(96, 96, 96)

#ifndef NDEBUG
static void *baseOfRenderObjectBeingDeleted;
#endif

void* RenderObject::operator new(size_t sz, RenderArena* renderArena) throw()
{
    return renderArena->allocate(sz);
}

void RenderObject::operator delete(void* ptr, size_t sz)
{
    assert(baseOfRenderObjectBeingDeleted == ptr);

    // Stash size where detach can find it.
    *(size_t *)ptr = sz;
}

RenderObject *RenderObject::createObject(DOM::NodeImpl* node,  RenderStyle* style)
{
    RenderObject *o = 0;
    khtml::RenderArena* arena = node->getDocument()->renderArena();
    switch(style->display())
    {
    case NONE:
        break;
    case INLINE:
    case INLINE_BLOCK:
    case BLOCK:
        // In compat mode, if <td> has a display of block, build a table cell instead.
        // This corrects erroneous HTML.  A better fix would be to implement full-blown
        // CSS2 anonymous table render object construction, but until then, this will have
        // to suffice. -dwh
        if (style->display() == BLOCK && node->id() == ID_TD && style->htmlHacks())
            o = new (arena) RenderTableCell(node);
        // In quirks mode if <table> has a display of block, make a table. If it has
        // a display of inline, make an inline-table.
        else if (node->id() == ID_TABLE && style->htmlHacks())
            o = new (arena) RenderTable(node);
        else if (style->display() == INLINE)
            o = new (arena) RenderInline(node);
        else
            o = new (arena) RenderBlock(node);
        break;
    case TABLE_CAPTION:
        o = new (arena) RenderBlock(node);
        break;
    case LIST_ITEM:
        o = new (arena) RenderListItem(node);
        break;
    case RUN_IN:
    case COMPACT:
        o = new (arena) RenderBlock(node);
        break;
    case TABLE:
    case INLINE_TABLE:
        style->setFlowAroundFloats(true);
        o = new (arena) RenderTable(node);
        break;
    case TABLE_ROW_GROUP:
    case TABLE_HEADER_GROUP:
    case TABLE_FOOTER_GROUP:
        o = new (arena) RenderTableSection(node);
        break;
    case TABLE_ROW:
        o = new (arena) RenderTableRow(node);
        break;
    case TABLE_COLUMN_GROUP:
    case TABLE_COLUMN:
        o = new (arena) RenderTableCol(node);
        break;
    case TABLE_CELL:
        o = new (arena) RenderTableCell(node);
        break;
    }
    return o;
}


RenderObject::RenderObject(DOM::NodeImpl* node)
    : CachedObjectClient(),
      m_style( 0 ),
      m_node( node ),
      m_parent( 0 ),
      m_previous( 0 ),
      m_next( 0 ),
      m_verticalPosition( PositionUndefined ),
      m_needsLayout( false ),
      m_normalChildNeedsLayout( false ),
      m_markedForRepaint( false ),
      m_posChildNeedsLayout( false ),
      m_minMaxKnown( false ),
      m_floating( false ),

      m_positioned( false ),
      m_overhangingContents( false ),
      m_relPositioned( false ),
      m_paintBackground( false ),

      m_isAnonymous( node->isDocumentNode() ),
      m_recalcMinMax( false ),
      m_isText( false ),
      m_inline( true ),

      m_replaced( false ),
      m_mouseInside( false ),
      m_hasFirstLine( false ),
      m_isSelectionBorder( false ),
      m_isRoot( false ),
      m_afterPageBreak( false ),
      m_needsPageClear( false ),
      m_containsPageBreak( false )
{
  assert( node );
  if (node->getDocument()->documentElement() == node) setIsRoot(true);
}

RenderObject::~RenderObject()
{
    if(m_style->backgroundImage())
        m_style->backgroundImage()->deref(this);

    if (m_style)
        m_style->deref();
}



RenderObject* RenderObject::objectBelow() const
{
    RenderObject* obj = firstChild();
    if ( !obj ) {
        obj = nextSibling();
        if ( !obj )
        {
            obj = parent();
            while (obj && !obj->nextSibling())
                obj = obj->parent();
            if (obj)
                obj = obj->nextSibling();
        }
    }
    return obj;
}

RenderObject* RenderObject::objectAbove() const
{
    RenderObject* obj = previousSibling();
    if ( !obj )
        return parent();

    RenderObject* last = obj->lastChild();
    while ( last )
    {
        obj = last;
        last = last->lastChild();
    }
    return obj;
}
/*
bool RenderObject::isRoot() const
{
    return !isAnonymous() &&
        element()->getDocument()->documentElement() == element();
}*/

bool RenderObject::isHR() const
{
    return element() && element()->id() == ID_HR;
}

bool RenderObject::isHTMLMarquee() const
{
    return element() && element()->renderer() == this && element()->id() == ID_MARQUEE;
}

void RenderObject::addChild(RenderObject* , RenderObject *)
{
    KHTMLAssert(0);
}

RenderObject* RenderObject::removeChildNode(RenderObject* )
{
    KHTMLAssert(0);
    return 0;
}

void RenderObject::removeChild(RenderObject *o )
{
    setNeedsLayout(true);
    removeChildNode( o );
}

void RenderObject::appendChildNode(RenderObject*)
{
    KHTMLAssert(0);
}

void RenderObject::insertChildNode(RenderObject*, RenderObject*)
{
    KHTMLAssert(0);
}

static void addLayers(RenderObject* obj, RenderLayer* parentLayer, RenderObject*& newObject,
                      RenderLayer*& beforeChild)
{
    if (obj->layer()) {
        if (!beforeChild && newObject) {
            // We need to figure out the layer that follows newObject.  We only do
            // this the first time we find a child layer, and then we update the
            // pointer values for newObject and beforeChild used by everyone else.
            beforeChild = newObject->parent()->findNextLayer(parentLayer, newObject);
            newObject = 0;
        }
        parentLayer->addChild(obj->layer(), beforeChild);
        return;
    }

    for (RenderObject* curr = obj->firstChild(); curr; curr = curr->nextSibling())
        addLayers(curr, parentLayer, newObject, beforeChild);
}

void RenderObject::addLayers(RenderLayer* parentLayer, RenderObject* newObject)
{
    if (!parentLayer)
        return;

    RenderObject* object = newObject;
    RenderLayer* beforeChild = 0;
    ::addLayers(this, parentLayer, object, beforeChild);
}

void RenderObject::removeLayers(RenderLayer* parentLayer)
{
    if (!parentLayer)
        return;

    if (layer()) {
        parentLayer->removeChild(layer());
        return;
    }

    for (RenderObject* curr = firstChild(); curr; curr = curr->nextSibling())
        curr->removeLayers(parentLayer);
}

void RenderObject::moveLayers(RenderLayer* oldParent, RenderLayer* newParent)
{
    if (!newParent)
        return;

    if (layer()) {
        if (oldParent)
            oldParent->removeChild(layer());
        newParent->addChild(layer());
        return;
    }

    for (RenderObject* curr = firstChild(); curr; curr = curr->nextSibling())
        curr->moveLayers(oldParent, newParent);
}

RenderLayer* RenderObject::findNextLayer(RenderLayer* parentLayer, RenderObject* startPoint,
                                         bool checkParent)
{
    // Error check the parent layer passed in.  If it's null, we can't find anything.
    if (!parentLayer)
        return 0;

    // Step 1: Descend into our siblings trying to find the next layer.  If we do find
    // a layer, and if its parent layer matches our desired parent layer, then we have
    // a match.
    for (RenderObject* curr = startPoint ? startPoint->nextSibling() : firstChild();
         curr; curr = curr->nextSibling()) {
        RenderLayer* nextLayer = curr->findNextLayer(parentLayer, 0, false);
        if (nextLayer) {
            if (nextLayer->parent() == parentLayer)
                return nextLayer;
            return 0;
        }
    }

    // Step 2: If our layer is the desired parent layer, then we're finished.  We didn't
    // find anything.
    RenderLayer* ourLayer = layer();
    if (parentLayer == ourLayer)
        return 0;

    // Step 3: If we have a layer, then return that layer.  It will be checked against
    // the desired parent layer in the for loop above.
    if (ourLayer)
        return ourLayer;

    // Step 4: If |checkParent| is set, climb up to our parent and check its siblings that
    // follow us to see if we can locate a layer.
    if (checkParent && parent())
        return parent()->findNextLayer(parentLayer, this, true);

    return 0;
}

RenderLayer* RenderObject::enclosingLayer() const
{
    const RenderObject* curr = this;
    while (curr) {
        RenderLayer *layer = curr->layer();
        if (layer)
            return layer;
        curr = curr->parent();
    }
    return 0;
}

int RenderObject::offsetLeft() const
{
    if ( isPositioned() )
        return xPos();

    if ( isBody() && style()->htmlHacks() )
        return 0;

    int x = xPos();
    if (isRelPositioned()) {
        int y = 0;
        static_cast<const RenderBox*>(this)->relativePositionOffset(x, y);
    }

    RenderObject* offsetPar = offsetParent();
    for( RenderObject* curr = parent();
         curr && curr != offsetPar;
         curr = curr->parent() )
        x += curr->xPos();

    if ( offsetPar && offsetPar->isBody() && style()->htmlHacks() )
        x += offsetPar->xPos();

    return x;
}

int RenderObject::offsetTop() const
{
    if ( isPositioned() )
        return yPos();

    if ( isBody() && style()->htmlHacks() )
        return 0;

    int y = yPos();
    if (isRelPositioned()) {
        int x = 0;
        static_cast<const RenderBox*>(this)->relativePositionOffset(x, y);
    }
    RenderObject* offsetPar = offsetParent();
    for( RenderObject* curr = parent();
         curr && curr != offsetPar;
         curr = curr->parent() )
        y += curr->yPos();

    if ( offsetPar && offsetPar->isBody() && style()->htmlHacks() )
        y += offsetPar->yPos();

    return y;
}

RenderObject* RenderObject::offsetParent() const
{
    bool skipTables = isPositioned() || isRelPositioned();
    bool strict = !style()->htmlHacks();
    RenderObject* curr = parent();
    while (curr && !curr->isPositioned() && !curr->isRelPositioned() && 
                   !(strict && skipTables ? curr->isRoot() : curr->isBody())) {
        if (!skipTables && (curr->isTableCell() || curr->isTable()))
            break;
        curr = curr->parent();
    }
    return curr;
}

// IE extensions.
// clientWidth and clientHeight represent the interior of an object
short RenderObject::clientWidth() const
{
    return width() - borderLeft() - borderRight() -
        (layer() ? layer()->verticalScrollbarWidth() : 0);
}

int RenderObject::clientHeight() const
{
    return height() - borderTop() - borderBottom() -
      (layer() ? layer()->horizontalScrollbarHeight() : 0);
}

// scrollWidth/scrollHeight is the size including the overflow area
short RenderObject::scrollWidth() const
{
    return (style()->hidesOverflow() && layer()) ? layer()->scrollWidth() : overflowWidth();
}

int RenderObject::scrollHeight() const
{
    return (style()->hidesOverflow() && layer()) ? layer()->scrollHeight() : overflowHeight();
}

bool RenderObject::hasStaticX() const
{
    return (style()->left().isVariable() && style()->right().isVariable()) ||
            style()->left().isStatic() ||
            style()->right().isStatic();
}

bool RenderObject::hasStaticY() const
{
    return (style()->top().isVariable() && style()->bottom().isVariable()) || style()->top().isStatic();
}

void RenderObject::setPixmap(const QPixmap&, const QRect& /*r*/, CachedImage* image)
{
    //repaint bg when it finished loading
    if(image && parent() && style() && style()->backgroundImage() == image
       && image->valid_rect().size() == image->pixmap_size() ) {
        isBody() ? canvas()->repaint() : repaint();
    }
}

void RenderObject::setNeedsLayout(bool b, bool markParents)
{
    bool alreadyNeededLayout = m_needsLayout;
    m_needsLayout = b;
    if (b) {
        if (!alreadyNeededLayout && markParents) {
            dirtyFormattingContext( false );
            markContainingBlocksForLayout();
        }
    }
    else {
        m_posChildNeedsLayout = false;
        m_normalChildNeedsLayout = false;
    }
}

void RenderObject::setChildNeedsLayout(bool b, bool markParents)
{
    bool alreadyNeededLayout = m_normalChildNeedsLayout;
    m_normalChildNeedsLayout = b;
    if (b) {
        if (!alreadyNeededLayout && markParents)
            markContainingBlocksForLayout();
    }
    else {
        m_posChildNeedsLayout = false;
        m_normalChildNeedsLayout = false;
    }
}

void RenderObject::markContainingBlocksForLayout()
{
    RenderObject *o = container();
    RenderObject *last = this;

    while (o) {
        if (!last->isText() && (last->style()->position() == FIXED || last->style()->position() == ABSOLUTE)) {
            if (o->m_posChildNeedsLayout)
                return;
            o->m_posChildNeedsLayout = true;
        }
        else {
            if (o->m_normalChildNeedsLayout)
                return;
            o->m_normalChildNeedsLayout = true;
        }

        last = o;
        o = o->container();
    }

    last->scheduleRelayout();
}

RenderBlock *RenderObject::containingBlock() const
{
    if(isTableCell())
        return static_cast<RenderBlock*>( parent()->parent()->parent() );
    if (isCanvas())
        return const_cast<RenderBlock*>( static_cast<const RenderBlock*>(this) );

    RenderObject *o = parent();
    if(m_style->position() == FIXED) {
        while ( o && !o->isCanvas() )
            o = o->parent();
    }
    else if(m_style->position() == ABSOLUTE) {
        while (o &&
               ( o->style()->position() == STATIC || ( o->isInline() && !o->isReplaced() ) ) && !o->isCanvas())
            o = o->parent();
    } else {
        while(o && ( ( o->isInline() && !o->isReplaced() ) || o->isTableRow() || o->isTableSection() ||
                       o->isTableCol() || o->isFrameSet() ) )
            o = o->parent();
    }
    // this is just to make sure we return a valid element.
    // the case below should never happen...
    if(!o || !o->isRenderBlock()) {
        if(!isCanvas()) {
#ifndef NDEBUG
            kdDebug( 6040 ) << this << ": " << renderName() << "(RenderObject): No containingBlock!" << endl;
            kdDebug( 6040 ) << kdBacktrace() << endl;
            const RenderObject* p = this;
            while (p->parent()) p = p->parent();
            p->printTree();
#endif
        }
        return 0L;
    }

    return static_cast<RenderBlock*>( o );
}

short RenderObject::containingBlockWidth() const
{
    // ###
    return containingBlock()->contentWidth();
}

int RenderObject::containingBlockHeight() const
{
    // ###
    return containingBlock()->contentHeight();
}

bool RenderObject::sizesToMaxWidth() const
{
    // Marquees in WinIE are like a mixture of blocks and inline-blocks.  They size as though they're blocks,
    // but they allow text to sit on the same line as the marquee.
    if (isFloating() || isCompact() ||
        (isInlineBlockOrInlineTable() && !isHTMLMarquee()) ||
        (element() && (element()->id() == ID_BUTTON || element()->id() == ID_LEGEND)))
        return true;

    // Children of a horizontal marquee do not fill the container by default.
    // FIXME: Need to deal with MAUTO value properly.  It could be vertical.
    if (parent()->style()->overflow() == OMARQUEE) {
        EMarqueeDirection dir = parent()->style()->marqueeDirection();
        if (dir == MAUTO || dir == MFORWARD || dir == MBACKWARD || dir == MLEFT || dir == MRIGHT)
            return true;
    }

#ifdef APPLE_CHANGES	// ### what the heck is a flexbox?
    // Flexible horizontal boxes lay out children at their maxwidths.  Also vertical boxes
    // that don't stretch their kids lay out their children at their maxwidths.
    if (parent()->isFlexibleBox() &&
        (parent()->style()->boxOrient() == HORIZONTAL || parent()->style()->boxAlign() != BSTRETCH))
        return true;
#endif

    return false;
}

// from Mozilla's nsCSSColorUtils.cpp
static int brightness(int red, int green, int blue)
{

  int intensity = (red + green + blue) / 3;

  int luminosity =
    ((RED_LUMINOSITY * red) / 100) +
    ((GREEN_LUMINOSITY * green) / 100) +
    ((BLUE_LUMINOSITY * blue) / 100);

  return ((intensity * INTENSITY_FACTOR) +
          (luminosity * LUMINOSITY_FACTOR)) / 100;
}

static void calc3DColor(QColor &color, bool darken)
{
  int rb = color.red();
  int gb = color.green();
  int bb = color.blue();

  int brightness_ = brightness(rb,gb,bb);

  int f0, f1;
  if (brightness_ < COLOR_DARK_THRESHOLD) {
    f0 = COLOR_DARK_BS_FACTOR;
    f1 = COLOR_DARK_TS_FACTOR;
  } else if (brightness_ > COLOR_LIGHT_THRESHOLD) {
    f0 = COLOR_LITE_BS_FACTOR;
    f1 = COLOR_LITE_TS_FACTOR;
  } else {
    f0 = COLOR_DARK_BS_FACTOR +
      (brightness_ *
       (COLOR_LITE_BS_FACTOR - COLOR_DARK_BS_FACTOR) / MAX_COLOR);
    f1 = COLOR_DARK_TS_FACTOR +
      (brightness_ *
       (COLOR_LITE_TS_FACTOR - COLOR_DARK_TS_FACTOR) / MAX_COLOR);
  }

  if (darken) {
    int r = rb - (f0 * rb / 100);
    int g = gb - (f0 * gb / 100);
    int b = bb - (f0 * bb / 100);
    if ((r == rb) && (g == gb) && (b == bb))
      color = (color == Qt::black) ? DARK_GRAY : Qt::black;
    else
      color.setRgb(r, g, b);
  } else {
    int r = kMin(rb + (f1 * (MAX_COLOR - rb) / 100), 255);
    int g = kMin(gb + (f1 * (MAX_COLOR - gb) / 100), 255);
    int b = kMin(bb + (f1 * (MAX_COLOR - bb) / 100), 255);
    if ((r == rb) && (g == gb) && (b == bb))
      color = (color == Qt::white) ? LIGHT_GRAY : Qt::white;
    else
      color.setRgb(r, g, b);
  }
}

void RenderObject::drawBorder(QPainter *p, int x1, int y1, int x2, int y2,
                              BorderSide s, QColor c, const QColor& textcolor, EBorderStyle style,
                              int adjbw1, int adjbw2, bool invalidisInvert)
{
    int width = (s==BSTop||s==BSBottom?y2-y1:x2-x1);

    if(style == DOUBLE && width < 3)
        style = SOLID;

    if(!c.isValid()) {
        if(invalidisInvert)
        {
            p->setRasterOp(Qt::XorROP);
            c = Qt::white;
        }
        else {
            if(style == INSET || style == OUTSET || style == RIDGE || style ==
            GROOVE)
                c = Qt::white;
            else
                c = textcolor;
        }
    }

    switch(style)
    {
    case BNATIVE:
    case BNONE:
    case BHIDDEN:
        // should not happen
        if(invalidisInvert && p->rasterOp() == Qt::XorROP)
            p->setRasterOp(Qt::CopyROP);

        return;
    case DOTTED:
        if ( width == 1 ) {
            // workaround Qt brokenness
            p->setPen(QPen(c, width, Qt::SolidLine));
            switch(s) {
            case BSBottom:
            case BSTop:
                for ( ; x1 < x2; x1 += 2 )
                    p->drawPoint( x1, y1 );
                break;
            case BSRight:
            case BSLeft:
                for ( ; y1 < y2; y1 += 2 )
                    p->drawPoint( x1, y1 );
            }
            break;
        }

        p->setPen(QPen(c, width, Qt::DotLine));
        /* nobreak; */
    case DASHED:
        if(style == DASHED)
            p->setPen(QPen(c, width == 1 ? 0 : width, width == 1 ? Qt::DotLine : Qt::DashLine));

        if (width > 0)
            switch(s) {
            case BSBottom:
            case BSTop:
                p->drawLine(x1, (y1+y2)/2, x2, (y1+y2)/2);
                break;
            case BSRight:
            case BSLeft:
                p->drawLine((x1+x2)/2, y1, (x1+x2)/2, y2);
                break;
            }

        break;
    case DOUBLE:
    {
        int third = (width+1)/3;

        if (adjbw1 == 0 && adjbw2 == 0)
        {
            p->setPen(Qt::NoPen);
            p->setBrush(c);
            switch(s)
            {
            case BSTop:
            case BSBottom:
                p->drawRect(x1, y1      , x2-x1, third);
                p->drawRect(x1, y2-third, x2-x1, third);
                break;
            case BSLeft:
                p->drawRect(x1      , y1+1, third, y2-y1-1);
                p->drawRect(x2-third, y1+1, third, y2-y1-1);
                break;
            case BSRight:
                p->drawRect(x1      , y1+1, third, y2-y1-1);
                p->drawRect(x2-third, y1+1, third, y2-y1-1);
                break;
            }
        }
        else
        {
            int adjbw1bigthird;
            if (adjbw1>0) adjbw1bigthird = adjbw1+1;
            else adjbw1bigthird = adjbw1 - 1;
            adjbw1bigthird /= 3;

            int adjbw2bigthird;
            if (adjbw2>0) adjbw2bigthird = adjbw2 + 1;
            else adjbw2bigthird = adjbw2 - 1;
            adjbw2bigthird /= 3;

          switch(s)
            {
            case BSTop:
              drawBorder(p, x1+kMax((-adjbw1*2+1)/3,0), y1        , x2-kMax((-adjbw2*2+1)/3,0), y1 + third, s, c, textcolor, SOLID, adjbw1bigthird, adjbw2bigthird);
              drawBorder(p, x1+kMax(( adjbw1*2+1)/3,0), y2 - third, x2-kMax(( adjbw2*2+1)/3,0), y2        , s, c, textcolor, SOLID, adjbw1bigthird, adjbw2bigthird);
              break;
            case BSLeft:
              drawBorder(p, x1        , y1+kMax((-adjbw1*2+1)/3,0), x1+third, y2-kMax((-adjbw2*2+1)/3,0), s, c, textcolor, SOLID, adjbw1bigthird, adjbw2bigthird);
              drawBorder(p, x2 - third, y1+kMax(( adjbw1*2+1)/3,0), x2      , y2-kMax(( adjbw2*2+1)/3,0), s, c, textcolor, SOLID, adjbw1bigthird, adjbw2bigthird);
              break;
            case BSBottom:
              drawBorder(p, x1+kMax(( adjbw1*2+1)/3,0), y1      , x2-kMax(( adjbw2*2+1)/3,0), y1+third, s, c, textcolor, SOLID, adjbw1bigthird, adjbw2bigthird);
              drawBorder(p, x1+kMax((-adjbw1*2+1)/3,0), y2-third, x2-kMax((-adjbw2*2+1)/3,0), y2      , s, c, textcolor, SOLID, adjbw1bigthird, adjbw2bigthird);
              break;
            case BSRight:
            drawBorder(p, x1      , y1+kMax(( adjbw1*2+1)/3,0), x1+third, y2-kMax(( adjbw2*2+1)/3,0), s, c, textcolor, SOLID, adjbw1bigthird, adjbw2bigthird);
            drawBorder(p, x2-third, y1+kMax((-adjbw1*2+1)/3,0), x2      , y2-kMax((-adjbw2*2+1)/3,0), s, c, textcolor, SOLID, adjbw1bigthird, adjbw2bigthird);
              break;
            default:
              break;
            }
        }
        break;
    }
    case RIDGE:
    case GROOVE:
    {
        EBorderStyle s1;
        EBorderStyle s2;
        if (style==GROOVE)
        {
            s1 = INSET;
            s2 = OUTSET;
        }
        else
        {
            s1 = OUTSET;
            s2 = INSET;
        }

        int adjbw1bighalf;
        int adjbw2bighalf;
        if (adjbw1>0) adjbw1bighalf=adjbw1+1;
        else adjbw1bighalf=adjbw1-1;
        adjbw1bighalf/=2;

        if (adjbw2>0) adjbw2bighalf=adjbw2+1;
        else adjbw2bighalf=adjbw2-1;
        adjbw2bighalf/=2;

        switch (s)
        {
        case BSTop:
            drawBorder(p, x1+kMax(-adjbw1  ,0)/2,  y1        , x2-kMax(-adjbw2,0)/2, (y1+y2+1)/2, s, c, textcolor, s1, adjbw1bighalf, adjbw2bighalf);
            drawBorder(p, x1+kMax( adjbw1+1,0)/2, (y1+y2+1)/2, x2-kMax( adjbw2+1,0)/2,  y2        , s, c, textcolor, s2, adjbw1/2, adjbw2/2);
            break;
        case BSLeft:
            drawBorder(p,  x1        , y1+kMax(-adjbw1  ,0)/2, (x1+x2+1)/2, y2-kMax(-adjbw2,0)/2, s, c, textcolor, s1, adjbw1bighalf, adjbw2bighalf);
            drawBorder(p, (x1+x2+1)/2, y1+kMax( adjbw1+1,0)/2,  x2        , y2-kMax( adjbw2+1,0)/2, s, c, textcolor, s2, adjbw1/2, adjbw2/2);
            break;
        case BSBottom:
            drawBorder(p, x1+kMax( adjbw1  ,0)/2,  y1        , x2-kMax( adjbw2,0)/2, (y1+y2+1)/2, s, c, textcolor, s2,  adjbw1bighalf, adjbw2bighalf);
            drawBorder(p, x1+kMax(-adjbw1+1,0)/2, (y1+y2+1)/2, x2-kMax(-adjbw2+1,0)/2,  y2        , s, c, textcolor, s1, adjbw1/2, adjbw2/2);
            break;
        case BSRight:
            drawBorder(p,  x1        , y1+kMax( adjbw1  ,0)/2, (x1+x2+1)/2, y2-kMax( adjbw2,0)/2, s, c, textcolor, s2, adjbw1bighalf, adjbw2bighalf);
            drawBorder(p, (x1+x2+1)/2, y1+kMax(-adjbw1+1,0)/2,  x2        , y2-kMax(-adjbw2+1,0)/2, s, c, textcolor, s1, adjbw1/2, adjbw2/2);
            break;
        }
        break;
    }
    case INSET:
    case OUTSET:
        calc3DColor(c, (style == OUTSET && (s == BSBottom || s == BSRight)) ||
             (style == INSET && ( s == BSTop || s == BSLeft ) ) );
        /* nobreak; */
    case SOLID:
        p->setPen(Qt::NoPen);
        p->setBrush(c);
        Q_ASSERT(x2>=x1);
        Q_ASSERT(y2>=y1);
        if (adjbw1==0 && adjbw2 == 0) {
            p->drawRect(x1,y1,x2-x1,y2-y1);
            return;
        }
        QPointArray quad(4);
        switch(s) {
        case BSTop:
            quad.setPoints(4,
                           x1+kMax(-adjbw1,0), y1,
                           x1+kMax( adjbw1,0), y2,
                           x2-kMax( adjbw2,0), y2,
                           x2-kMax(-adjbw2,0), y1);
            break;
        case BSBottom:
            quad.setPoints(4,
                           x1+kMax( adjbw1,0), y1,
                           x1+kMax(-adjbw1,0), y2,
                           x2-kMax(-adjbw2,0), y2,
                           x2-kMax( adjbw2,0), y1);
            break;
        case BSLeft:
          quad.setPoints(4,
                         x1, y1+kMax(-adjbw1,0),
                                x1, y2-kMax(-adjbw2,0),
                         x2, y2-kMax( adjbw2,0),
                         x2, y1+kMax( adjbw1,0));
            break;
        case BSRight:
          quad.setPoints(4,
                         x1, y1+kMax( adjbw1,0),
                                x1, y2-kMax( adjbw2,0),
                         x2, y2-kMax(-adjbw2,0),
                         x2, y1+kMax(-adjbw1,0));
            break;
        }
        p->drawConvexPolygon(quad);
        break;
    }

    if(invalidisInvert && p->rasterOp() == Qt::XorROP)
        p->setRasterOp(Qt::CopyROP);
}

void RenderObject::paintBorder(QPainter *p, int _tx, int _ty, int w, int h, const RenderStyle* style, bool begin, bool end)
{
    const QColor& tc = style->borderTopColor();
    const QColor& bc = style->borderBottomColor();
    const QColor& lc = style->borderLeftColor();
    const QColor& rc = style->borderRightColor();

    bool tt = style->borderTopIsTransparent();
    bool bt = style->borderBottomIsTransparent();
    bool rt = style->borderRightIsTransparent();
    bool lt = style->borderLeftIsTransparent();

    EBorderStyle ts = style->borderTopStyle();
    EBorderStyle bs = style->borderBottomStyle();
    EBorderStyle ls = style->borderLeftStyle();
    EBorderStyle rs = style->borderRightStyle();

    bool render_t = ts > BHIDDEN && !tt;
    bool render_l = ls > BHIDDEN && begin && !lt;
    bool render_r = rs > BHIDDEN && end && !rt;
    bool render_b = bs > BHIDDEN && !bt;

    if(render_t) {
        bool ignore_left =
            (tc == lc) && (tt == lt) &&
            (ts >= OUTSET) &&
            (ls == DOTTED || ls == DASHED || ls == SOLID || ls == OUTSET);

        bool ignore_right =
            (tc == rc) && (tt == rt) &&
            (ts >= OUTSET) &&
            (rs == DOTTED || rs == DASHED || rs == SOLID || rs == INSET);

        drawBorder(p, _tx, _ty, _tx + w, _ty +  style->borderTopWidth(), BSTop, tc, style->color(), ts,
                   ignore_left?0:style->borderLeftWidth(),
                   ignore_right?0:style->borderRightWidth());
    }

    if(render_b) {
        bool ignore_left =
            (bc == lc) && (bt == lt) &&
            (bs >= OUTSET) &&
            (ls == DOTTED || ls == DASHED || ls == SOLID || ls == OUTSET);

        bool ignore_right =
            (bc == rc) && (bt == rt) &&
            (bs >= OUTSET) &&
            (rs == DOTTED || rs == DASHED || rs == SOLID || rs == INSET);

        drawBorder(p, _tx, _ty + h - style->borderBottomWidth(), _tx + w, _ty + h, BSBottom, bc, style->color(), bs,
                   ignore_left?0:style->borderLeftWidth(),
                   ignore_right?0:style->borderRightWidth());
    }

    if(render_l)
    {
	bool ignore_top =
	  (tc == lc) && (tt == lt) &&
	  (ls >= OUTSET) &&
	  (ts == DOTTED || ts == DASHED || ts == SOLID || ts == OUTSET);

	bool ignore_bottom =
	  (bc == lc) && (bt == lt) &&
	  (ls >= OUTSET) &&
	  (bs == DOTTED || bs == DASHED || bs == SOLID || bs == INSET);

        drawBorder(p, _tx, _ty, _tx + style->borderLeftWidth(), _ty + h, BSLeft, lc, style->color(), ls,
                   ignore_top?0:style->borderTopWidth(),
                   ignore_bottom?0:style->borderBottomWidth());
    }

    if(render_r)
    {
	bool ignore_top =
	  (tc == rc) && (tt == rt) &&
	  (rs >= DOTTED || rs == INSET) &&
	  (ts == DOTTED || ts == DASHED || ts == SOLID || ts == OUTSET);

	bool ignore_bottom =
	  (bc == rc) && (bt == rt) &&
	  (rs >= DOTTED || rs == INSET) &&
	  (bs == DOTTED || bs == DASHED || bs == SOLID || bs == INSET);

        drawBorder(p, _tx + w - style->borderRightWidth(), _ty, _tx + w, _ty + h, BSRight, rc, style->color(), rs,
                   ignore_top?0:style->borderTopWidth(),
                   ignore_bottom?0:style->borderBottomWidth());
    }
}

void RenderObject::paintOutline(QPainter *p, int _tx, int _ty, int w, int h, const RenderStyle* style)
{
    int ow = style->outlineWidth();
    if(!ow) return;

    const QColor& oc = style->outlineColor();
    EBorderStyle os = style->outlineStyle();
    // ### outline-offset should be implemented in renderInline before reactivated here
    int offset = 0; // style->outlineOffset();

#ifdef APPLE_CHANGES
    if (style->outlineStyleIsAuto()) {
        p->initFocusRing(ow, offset, oc);
        addFocusRingRects(p, _tx, _ty);
        p->drawFocusRing();
        p->clearFocusRing();
        return;
    }
#endif

    _tx -= offset;
    _ty -= offset;
    w += 2*offset;
    h += 2*offset;

    drawBorder(p, _tx-ow, _ty-ow, _tx, _ty+h+ow, BSLeft,
               QColor(oc), style->color(),
               os, ow, ow, true);

    drawBorder(p, _tx-ow, _ty-ow, _tx+w+ow, _ty, BSTop,
               QColor(oc), style->color(),
               os, ow, ow, true);

    drawBorder(p, _tx+w, _ty-ow, _tx+w+ow, _ty+h+ow, BSRight,
               QColor(oc), style->color(),
               os, ow, ow, true);

    drawBorder(p, _tx-ow, _ty+h, _tx+w+ow, _ty+h+ow, BSBottom,
               QColor(oc), style->color(),
               os, ow, ow, true);

}

void RenderObject::paint( PaintInfo&, int /*tx*/, int /*ty*/)
{
}

void RenderObject::repaintRectangle(int x, int y, int w, int h, bool immediate, bool f)
{
    if(parent()) parent()->repaintRectangle(x, y, w, h, immediate, f);
}

#ifdef ENABLE_DUMP

QString RenderObject::information() const
{
    QString str;
    QTextStream ts( &str, IO_WriteOnly );
    ts << renderName()
        << "(" << (style() ? style()->refCount() : 0) << ")"
       << ": " << (void*)this << "  ";
    if (isInline()) ts << "il ";
    if (childrenInline()) ts << "ci ";
    if (isFloating()) ts << "fl ";
    if (isAnonymous()) ts << "an ";
    if (isRelPositioned()) ts << "rp ";
    if (isPositioned()) ts << "ps ";
    if (isReplaced()) ts << "rp ";
    if (overhangingContents()) ts << "oc ";
    if (needsLayout()) ts << "nl ";
    if (minMaxKnown()) ts << "mmk ";
    if (m_recalcMinMax) ts << "rmm ";
    if (mouseInside()) ts << "mi ";
    if (style() && style()->zIndex()) ts << "zI: " << style()->zIndex();
    if (style() && style()->hasAutoZIndex()) ts << "zI: auto ";
    if (element()) {
        if (element()->active()) ts << "act ";
        if (element()->hasAnchor()) ts << "anchor ";
        if (element()->focused()) ts << "focus ";
        ts << " <" <<  getTagName(element()->id()) << ">";

    } else if (isPseudoAnonymous() && style() && style()->styleType() != RenderStyle::NOPSEUDO) {
        ts << " <" <<  getTagName(node()->id());
        QString pseudo;
        switch (style()->styleType()) {
          case RenderStyle::FIRST_LETTER:
            pseudo = ":first-letter"; break;
          case RenderStyle::BEFORE:
            pseudo = ":before"; break;
          case RenderStyle::AFTER:
            pseudo = ":after"; break;
          default:
            pseudo = ":pseudo-element";
        }
        ts << pseudo;
        ts << ">";
    }
    ts << " (" << xPos() << "," << yPos() << "," << width() << "," << height() << ")"
       << " [" << minWidth() << "-" << maxWidth() << "]"
       << " { mT: " << marginTop() << " qT: " << isTopMarginQuirk()
       << " mB: " << marginBottom() << " qB: " << isBottomMarginQuirk()
       << "}"
        << (isTableCell() ?
            ( QString::fromLatin1(" [r=") +
              QString::number( static_cast<const RenderTableCell *>(this)->row() ) +
              QString::fromLatin1(" c=") +
              QString::number( static_cast<const RenderTableCell *>(this)->col() ) +
              QString::fromLatin1(" rs=") +
              QString::number( static_cast<const RenderTableCell *>(this)->rowSpan() ) +
              QString::fromLatin1(" cs=") +
              QString::number( static_cast<const RenderTableCell *>(this)->colSpan() ) +
              QString::fromLatin1("]") ) : QString::null );
    if ( layer() )
        ts << " layer=" << layer();
    if ( continuation() )
        ts << " continuation=" << continuation();
    if (isText())
        ts << " \"" << QConstString(static_cast<const RenderText *>(this)->text(), kMin(static_cast<const RenderText *>(this)->length(), 10u)).string() << "\"";
    return str;
}

void RenderObject::printTree(int indent) const
{
    QString ind;
    ind.fill(' ', indent);

    kdDebug() << ind << information() << endl;

    RenderObject *child = firstChild();
    while( child != 0 )
    {
        child->printTree(indent+2);
        child = child->nextSibling();
    }
}

static QTextStream &operator<<(QTextStream &ts, const QRect &r)
{
    return ts << "at (" << r.x() << "," << r.y() << ") size " << r.width() << "x" << r.height();
}

void RenderObject::dump(QTextStream &ts, const QString &ind) const
{
    if ( !layer() )
        ts << endl;

    ts << ind << renderName();

    if (style() && style()->zIndex()) {
        ts << " zI: " << style()->zIndex();
    }

    if (element()) {
        QString tagName(getTagName(element()->id()));
        if (!tagName.isEmpty()) {
            ts << " {" << tagName << "}";
        }
    } else if (isPseudoAnonymous() && style() && style()->styleType() != RenderStyle::NOPSEUDO) {
        QString pseudo;
        QString tagName(getTagName(node()->id()));
        switch (style()->styleType()) {
          case RenderStyle::FIRST_LETTER:
            pseudo = ":first-letter"; break;
          case RenderStyle::BEFORE:
            pseudo = ":before"; break;
          case RenderStyle::AFTER:
            pseudo = ":after"; break;
          default:
            pseudo = ":pseudo-element";
        }
        ts << " {" << tagName << pseudo << "}";
    }

    QRect r(xPos(), yPos(), width(), height());
    ts << " " << r;

    if ( parent() )
        ts << style()->createDiff( *parent()->style() );

    if (isAnonymous()) { ts << " anonymousBox"; }
    if (isFloating()) { ts << " floating"; }
    if (isPositioned()) { ts << " positioned"; }
    if (isRelPositioned()) { ts << " relPositioned"; }
    if (isText()) { ts << " text"; }
    if (isInline()) { ts << " inline"; }
    if (isReplaced()) { ts << " replaced"; }
    if (shouldPaintBackgroundOrBorder()) { ts << " paintBackground"; }
    if (needsLayout()) { ts << " needsLayout"; }
    if (minMaxKnown()) { ts << " minMaxKnown"; }
    if (overhangingContents()) { ts << " overhangingContents"; }
    if (hasFirstLine()) { ts << " hasFirstLine"; }
    if (afterPageBreak()) { ts << " afterPageBreak"; }
}
#endif

void RenderObject::selectionStartEnd(int& spos, int& epos)
{
    if (parent())
        parent()->selectionStartEnd(spos, epos);
}

void RenderObject::setStyle(RenderStyle *style)
{
    if (m_style == style)
        return;

    RenderStyle::Diff d = m_style ? m_style->diff( style ) : RenderStyle::Layout;

    //qDebug("m_style: %p new style, diff=%d", m_style,  d);

    if ( d == RenderStyle::Visible && m_parent && m_style &&
         m_style->outlineWidth() > style->outlineWidth() )
        repaint();

    if ( m_style &&
         ( ( isFloating() && m_style->floating() != style->floating() ) ||
           ( isPositioned() && m_style->position() != style->position() &&
             style->position() != ABSOLUTE && style->position() != FIXED ) ) )
        removeFromObjectLists();

    // reset style flags
    m_floating = false;
    m_positioned = false;
    m_relPositioned = false;
    m_paintBackground = false;

    if ( style->position() == STATIC ) {
        if ( isRoot() )
            style->setZIndex( 0 );
        else
            style->setHasAutoZIndex();
    }

    if ( layer() && style && m_style ) {
        if ( ( m_style->hasAutoZIndex() != style->hasAutoZIndex() ||
               m_style->zIndex() != style->zIndex() ||
               m_style->visibility() != style->visibility() ) ) {
            layer()->stackingContext()->dirtyZOrderLists();
            layer()->dirtyZOrderLists();
        }
    }

    RenderStyle *oldStyle = m_style;
    m_style = style;

    updateBackgroundImages(oldStyle);

    if (m_style)
        m_style->ref();

    if (oldStyle)
        oldStyle->deref();

    setShouldPaintBackgroundOrBorder(m_style->hasBorder() || m_style->hasBackground());

    m_hasFirstLine = (style->getPseudoStyle(RenderStyle::FIRST_LINE) != 0);
    if (m_parent) {
        if ( d >= RenderStyle::Position ) {
            if (!isText() && d == RenderStyle::CbLayout) {
                dirtyFormattingContext( true );
            }
            setNeedsLayoutAndMinMaxRecalc();
        } else if (!isText() && d == RenderStyle::Visible) {
            if (layer())
                layer()->repaint();
            else
                repaint();
        }
    }
}

void RenderObject::dirtyFormattingContext( bool checkContainer )
{
    if (m_markedForRepaint && !checkContainer)
        return;
    m_markedForRepaint = true;
    if (layer() && (style()->position() == FIXED || style()->position() == ABSOLUTE))
        return;
    if (m_parent && (checkContainer || style()->width().isVariable() || style()->height().isVariable() ||
                     !(isFloating() || flowAroundFloats() || isTableCell())))
        m_parent->dirtyFormattingContext(false);
}

void RenderObject::repaintDuringLayout()
{
    if (canvas()->needsFullRepaint() || isText())
        return;
    if (layer()) {
        layer()->repaint( true );
    } else {
       repaint();
       canvas()->deferredRepaint( this );
    }
}

void RenderObject::setOverhangingContents(bool p)
{
    if (m_overhangingContents == p)
        return;

    RenderBlock *cb = containingBlock();
    if (p)
    {
        m_overhangingContents = true;
        KHTMLAssert( cb != this || isCanvas());
        if (cb && cb != this)
            cb->setOverhangingContents();
    }
    else
    {
        RenderObject *n;
        bool c=false;

        for( n = firstChild(); n != 0; n = n->nextSibling() )
        {
            if (n->isPositioned() || n->overhangingContents())
                c=true;
        }

        if (c)
            return;
        else
        {
            m_overhangingContents = false;
            KHTMLAssert( cb != this );
            if (cb && cb != this)
                cb->setOverhangingContents(false);
        }
    }
}

void RenderObject::updateBackgroundImages(RenderStyle* oldStyle)
{
    // FIXME: This will be slow when a large number of images is used.  Fix by using a dict.
    const BackgroundLayer* oldLayers = oldStyle ? oldStyle->backgroundLayers() : 0;
    const BackgroundLayer* newLayers = m_style ? m_style->backgroundLayers() : 0;
    for (const BackgroundLayer* currOld = oldLayers; currOld; currOld = currOld->next()) {
        if (currOld->backgroundImage() && (!newLayers || !newLayers->containsImage(currOld->backgroundImage())))
            currOld->backgroundImage()->deref(this);
    }
    for (const BackgroundLayer* currNew = newLayers; currNew; currNew = currNew->next()) {
        if (currNew->backgroundImage() && (!oldLayers || !oldLayers->containsImage(currNew->backgroundImage())))
            currNew->backgroundImage()->ref(this);
    }
}

QRect RenderObject::viewRect() const
{
    return containingBlock()->viewRect();
}

bool RenderObject::absolutePosition(int &xPos, int &yPos, bool f)
{
    RenderObject* p = parent();
    if(p) {
        parent()->absolutePosition(xPos, yPos, f);
        if ( p->style()->hidesOverflow() && p->layer() )
            p->layer()->subtractScrollOffset( xPos, yPos );
        return true;
    }
    else
    {
        xPos = yPos = 0;
        return false;
    }
}

void RenderObject::caretPos(int /*offset*/, int /*flags*/, int &_x, int &_y, int &width, int &height)
{
    _x = _y = height = -1;
    width = 1;        // the caret has a default width of one pixel. If you want
                    // to check for validity, only test the x-coordinate for >= 0.
}

int RenderObject::paddingTop() const
{
    int w = 0;
    Length padding = m_style->paddingTop();
    if (padding.isPercent())
        w = containingBlock()->contentWidth();
    w = padding.minWidth(w);
    if ( isTableCell() && padding.isVariable() )
        w = static_cast<const RenderTableCell *>(this)->table()->cellPadding();
    return w;
}

int RenderObject::paddingBottom() const
{
    int w = 0;
    Length padding = style()->paddingBottom();
    if (padding.isPercent())
        w = containingBlock()->contentWidth();
    w = padding.minWidth(w);
    if ( isTableCell() && padding.isVariable() )
        w = static_cast<const RenderTableCell *>(this)->table()->cellPadding();
    return w;
}

int RenderObject::paddingLeft() const
{
    int w = 0;
    Length padding = style()->paddingLeft();
    if (padding.isPercent())
        w = containingBlock()->contentWidth();
    w = padding.minWidth(w);
    if ( isTableCell() && padding.isVariable() )
        w = static_cast<const RenderTableCell *>(this)->table()->cellPadding();
    return w;
}

int RenderObject::paddingRight() const
{
    int w = 0;
    Length padding = style()->paddingRight();
    if (padding.isPercent())
        w = containingBlock()->contentWidth();
    w = padding.minWidth(w);
    if ( isTableCell() && padding.isVariable() )
        w = static_cast<const RenderTableCell *>(this)->table()->cellPadding();
    return w;
}

RenderObject *RenderObject::container() const
{
    // This method is extremely similar to containingBlock(), but with a few notable
    // exceptions.
    // (1) It can be used on orphaned subtrees, i.e., it can be called safely even when
    // the object is not part of the primary document subtree yet.
    // (2) For normal flow elements, it just returns the parent.
    // (3) For absolute positioned elements, it will return a relative positioned inline.
    // containingBlock() simply skips relpositioned inlines and lets an enclosing block handle
    // the layout of the positioned object.  This does mean that calcAbsoluteHorizontal and
    // calcAbsoluteVertical have to use container().
    EPosition pos = m_style->position();
    RenderObject *o = 0;
    if( pos == FIXED ) {
        // container() can be called on an object that is not in the
        // tree yet.  We don't call canvas() since it will assert if it
        // can't get back to the canvas.  Instead we just walk as high up
        // as we can.  If we're in the tree, we'll get the root.  If we
        // aren't we'll get the root of our little subtree (most likely
        // we'll just return 0).
        o = parent();
        while ( o && o->parent() ) o = o->parent();
    }
    else if ( pos == ABSOLUTE ) {
        // Same goes here.  We technically just want our containing block, but
        // we may not have one if we're part of an uninstalled subtree.  We'll
        // climb as high as we can though.
        o = parent();
        while (o && o->style()->position() == STATIC && !o->isCanvas())
            o = o->parent();
    }
    else
        o = parent();
    return o;
}

DOM::DocumentImpl* RenderObject::document() const
{
    return m_node->getDocument();
}

void RenderObject::remove()
{
    removeFromObjectLists();

    if ( parent() )
        //have parent, take care of the tree integrity
        parent()->removeChild(this);
}

void RenderObject::removeFromObjectLists()
{
    // in destruction mode, don't care.
    if ( !document()->renderer() ) return;

    if (isFloating()) {
        RenderBlock* outermostBlock = containingBlock();
        for (RenderBlock* p = outermostBlock; p && !p->isCanvas() && p->containsFloat(this);) {
            outermostBlock = p;
            if (p->isFloatingOrPositioned())
                break;
            p = p->containingBlock();
        }

        if (outermostBlock)
            outermostBlock->markAllDescendantsWithFloatsForLayout(this);
    }

    if (isPositioned()) {
        RenderObject *p;
        for (p = parent(); p; p = p->parent()) {
            if (p->isRenderBlock())
                static_cast<RenderBlock*>(p)->removePositionedObject(this);
        }
    }
}

RenderArena* RenderObject::renderArena() const
{
    return m_node->getDocument()->renderArena();
}

void RenderObject::detach()
{
    detachCounters();

    deleteInlineBoxes();
    remove();

    // by default no refcounting
    arenaDelete(renderArena(), this);
}

void RenderObject::arenaDelete(RenderArena *arena, void *base)
{
#ifndef NDEBUG
    void *savedBase = baseOfRenderObjectBeingDeleted;
    baseOfRenderObjectBeingDeleted = base;
#endif
    delete this;
#ifndef NDEBUG
    baseOfRenderObjectBeingDeleted = savedBase;
#endif

    // Recover the size left there for us by operator delete and free the memory.
    arena->free(*(size_t *)base, base);
}

void RenderObject::arenaDelete(RenderArena *arena)
{
    // static_cast unfortunately doesn't work, since we multiple inherit
    // in eg. RenderWidget.
    arenaDelete(arena, dynamic_cast<void *>(this));
}

FindSelectionResult RenderObject::checkSelectionPoint( int _x, int _y, int _tx, int _ty, DOM::NodeImpl*& node, int & offset, SelPointState &state )
{
#if 0
    NodeInfo info(true, false);
    if ( nodeAtPoint( info, _x, _y, _tx, _ty ) && info.innerNode() )
    {
        RenderObject* r = info.innerNode()->renderer();
        if ( r ) {
            if ( r == this ) {
                node = info.innerNode();
                offset = 0; // we have no text...
                return SelectionPointInside;
            }
            else
                return r->checkSelectionPoint( _x, _y, _tx, _ty, node, offset, state );
        }
    }
    //kdDebug(6030) << "nodeAtPoint Failed. Fallback - hmm, SelectionPointAfter" << endl;
    node = 0;
    offset = 0;
    return SelectionPointAfter;
#endif
    int off = offset;
    DOM::NodeImpl* nod = node;

    for (RenderObject *child = firstChild(); child; child=child->nextSibling()) {
        // ignore empty text boxes, they produce totally bogus information
        // for caret navigation (LS)
        if (child->isText() && !static_cast<RenderText *>(child)->inlineTextBoxCount())
            continue;

//        kdDebug(6040) << "iterating " << (child ? child->renderName() : "") << "@" << child << (child->isText() ? " contains: \"" + QConstString(static_cast<RenderText *>(child)->text(), kMin(static_cast<RenderText *>(child)->length(), 10u)).string() + "\"" : QString::null) << endl;
//        kdDebug(6040) << "---------- checkSelectionPoint recursive -----------" << endl;
        khtml::FindSelectionResult pos = child->checkSelectionPoint(_x, _y, _tx+xPos(), _ty+yPos(), nod, off, state);
//        kdDebug(6040) << "-------- end checkSelectionPoint recursive ---------" << endl;
//        kdDebug(6030) << this << " child->findSelectionNode returned result=" << pos << " nod=" << nod << " off=" << off << endl;
        switch(pos) {
        case SelectionPointBeforeInLine:
        case SelectionPointInside:
            //kdDebug(6030) << "RenderObject::checkSelectionPoint " << this << " returning SelectionPointInside offset=" << offset << endl;
            node = nod;
            offset = off;
            return SelectionPointInside;
        case SelectionPointBefore:
            //x,y is before this element -> stop here
            if ( state.m_lastNode ) {
                node = state.m_lastNode;
                offset = state.m_lastOffset;
                //kdDebug(6030) << "RenderObject::checkSelectionPoint " << this << " before this child "
                //              << node << "-> returning SelectionPointInside, offset=" << offset << endl;
                return SelectionPointInside;
            } else {
                node = nod;
                offset = off;
                //kdDebug(6030) << "RenderObject::checkSelectionPoint " << this << " before us -> returning SelectionPointBefore " << node << "/" << offset << endl;
                return SelectionPointBefore;
            }
            break;
        case SelectionPointAfter:
            if (state.m_afterInLine) break;
            // fall through
        case SelectionPointAfterInLine:
            if (pos == SelectionPointAfterInLine) state.m_afterInLine = true;
            //kdDebug(6030) << "RenderObject::checkSelectionPoint: selection after: " << nod << " offset: " << off << " afterInLine: " << state.m_afterInLine << endl;
            state.m_lastNode = nod;
            state.m_lastOffset = off;
            // No "return" here, obviously. We must keep looking into the children.
            break;
        }
    }
    // If we are after the last child, return lastNode/lastOffset
    // But lastNode can be 0L if there is no child, for instance.
    if ( state.m_lastNode )
    {
        node = state.m_lastNode;
        offset = state.m_lastOffset;
    }
    //kdDebug(6030) << "fallback - SelectionPointAfter  node=" << node << " offset=" << offset << endl;
    return SelectionPointAfter;
}

bool RenderObject::mouseInside() const
{
    if (!m_mouseInside && continuation())
        return continuation()->mouseInside();
    return m_mouseInside;
}

bool RenderObject::nodeAtPoint(NodeInfo& info, int _x, int _y, int _tx, int _ty, HitTestAction hitTestAction, bool inside)
{
    int tx = _tx + xPos();
    int ty = _ty + yPos();

    inside |= ( style()->visibility() != HIDDEN &&
                (_y >= ty) && (_y < ty + height()) && (_x >= tx) && (_x < tx + width())) || isRoot() || isBody();
    bool inOverflowRect = inside;
    if ( !inOverflowRect ) {
        QRect overflowRect( tx, ty, overflowWidth(), overflowHeight() );
        inOverflowRect = overflowRect.contains( _x, _y );
    }

    // ### table should have its own, more performant method
    if (hitTestAction != HitTestSelfOnly &&
        (( !isRenderBlock() ||
           !static_cast<RenderBlock*>( this )->isPointInScrollbar( _x, _y, _tx, _ty )) &&
        (overhangingContents() || inOverflowRect || isInline() || isRoot() || isCanvas() ||
        isTableRow() || isTableSection() || inside || mouseInside() ))) {
        if ( hitTestAction == HitTestChildrenOnly )
            inside = false;
        if ( style()->hidesOverflow() && layer() )
            layer()->subtractScrollOffset(tx, ty);
        for (RenderObject* child = lastChild(); child; child = child->previousSibling())
            if (!child->layer() && child->nodeAtPoint(info, _x, _y, tx, ty, HitTestAll))
                inside = true;
    }

    if (inside) {
        if (!info.innerNode() && !isInline() && continuation()) {
            // We are in the margins of block elements that are part of a continuation.  In
            // this case we're actually still inside the enclosing inline element that was
            // split.  Go ahead and set our inner node accordingly.
            info.setInnerNode(continuation()->element());
            if (!info.innerNonSharedNode())
                info.setInnerNonSharedNode(continuation()->element());
        }

        if (info.innerNode() && info.innerNode()->renderer() &&
            !info.innerNode()->renderer()->isInline() && element() && isInline()) {
            // Within the same layer, inlines are ALWAYS fully above blocks.  Change inner node.
            info.setInnerNode(element());

            // Clear everything else.
            info.setInnerNonSharedNode(0);
            info.setURLElement(0);
        }

        if (!info.innerNode() && element())
            info.setInnerNode(element());

        if(!info.innerNonSharedNode() && element())
            info.setInnerNonSharedNode(element());

    }

    return inside;
}

short RenderObject::verticalPositionHint( bool firstLine ) const
{
    short vpos = m_verticalPosition;
    if ( m_verticalPosition == PositionUndefined || firstLine ) {
        vpos = getVerticalPosition( firstLine );
        if ( !firstLine )
            const_cast<RenderObject *>(this)->m_verticalPosition = vpos;
    }
    return vpos;

}

short RenderObject::getVerticalPosition( bool firstLine, RenderObject* ref ) const
{
    // vertical align for table cells has a different meaning
    int vpos = 0;
    if ( !isTableCell() && isInline() ) {
        EVerticalAlign va = style()->verticalAlign();
        if ( va == TOP ) {
            vpos = PositionTop;
        } else if ( va == BOTTOM ) {
            vpos = PositionBottom;
        } else {
            if (!ref) ref = parent();
            bool checkParent = ref->isInline() && !ref->isReplacedBlock() &&
                               !( ref->style()->verticalAlign() == TOP || ref->style()->verticalAlign() == BOTTOM );
            vpos = checkParent ? ref->verticalPositionHint( firstLine ) : 0;
            // don't allow elements nested inside text-top to have a different valignment.
            if ( va == BASELINE )
                return vpos;
            else if ( va == LENGTH )
                return vpos - style()->verticalAlignLength().width( lineHeight( firstLine ) );

            const QFont &f = ref->font( firstLine );
            int fontheight = ref->lineHeight( firstLine );
            int fontsize = f.pixelSize();
            int halfleading = ( fontheight - fontsize ) / 2;

            if ( va == SUB )
                vpos += fontsize/5 + 1;
            else if ( va == SUPER )
                vpos -= fontsize/3 + 1;
            else if ( va == TEXT_TOP ) {
//                 qDebug( "got TEXT_TOP vertical pos hint" );
//                 qDebug( "parent:" );
//                 qDebug( "CSSLH: %d, CSS_FS: %d, basepos: %d", fontheight, fontsize, ref->baselinePosition( firstLine ) );
//                 qDebug( "this:" );
//                 qDebug( "CSSLH: %d, CSS_FS: %d, basepos: %d", lineHeight( firstLine ), style()->font().pixelSize(), baselinePosition( firstLine ) );
                vpos += ( baselinePosition( firstLine ) - ref->baselinePosition( firstLine ) +
                        halfleading );
            } else if ( va == MIDDLE ) {
                QRect b = QFontMetrics(f).boundingRect('x');
                vpos += -b.height()/2 - lineHeight( firstLine )/2 + baselinePosition( firstLine );
            } else if ( va == TEXT_BOTTOM ) {
                vpos += QFontMetrics(f).descent();
                if ( !isReplaced() )
                    vpos -= fontMetrics(firstLine).descent();
            } else if ( va == BASELINE_MIDDLE )
                vpos += - lineHeight( firstLine )/2 + baselinePosition( firstLine );
        }
    }
    return vpos;
}

short RenderObject::lineHeight( bool firstLine ) const
{
    // Inline blocks are replaced elements. Otherwise, just pass off to
    // the base class.  If we're being queried as though we're the root line
    // box, then the fact that we're an inline-block is irrelevant, and we behave
    // just like a block.

    if (isReplaced() && (!isInlineBlockOrInlineTable() || !needsLayout()))
        return height()+marginTop()+marginBottom();

    Length lh;
    if( firstLine && hasFirstLine() ) {
        RenderStyle *pseudoStyle  = style()->getPseudoStyle(RenderStyle::FIRST_LINE);
        if ( pseudoStyle )
            lh = pseudoStyle->lineHeight();
    }
    else
        lh = style()->lineHeight();

    // its "unset", choose nice default
    if ( lh.value() < 0 )
        return style()->fontMetrics().lineSpacing();

    if ( lh.isPercent() )
        return lh.minWidth( style()->font().pixelSize() );

    // its fixed
    return lh.value();
}

short RenderObject::baselinePosition( bool firstLine ) const
{
    // Inline blocks are replaced elements. Otherwise, just pass off to
    // the base class.  If we're being queried as though we're the root line
    // box, then the fact that we're an inline-block is irrelevant, and we behave
    // just like a block.

    if (isReplaced() && (!isInlineBlockOrInlineTable() || !needsLayout()))
        return height()+marginTop()+marginBottom();

    const QFontMetrics &fm = fontMetrics( firstLine );
    return fm.ascent() + ( lineHeight( firstLine) - fm.height() ) / 2;
}

void RenderObject::invalidateVerticalPositions()
{
    m_verticalPosition = PositionUndefined;
    RenderObject *child = firstChild();
    while( child ) {
        child->invalidateVerticalPositions();
        child = child->nextSibling();
    }
}

void RenderObject::recalcMinMaxWidths()
{
    KHTMLAssert( m_recalcMinMax );

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << " recalcMinMaxWidths() this=" << this <<endl;
#endif

    RenderObject *child = firstChild();
    int cmin=0;
    int cmax=0;

    while( child ) {
        bool test = false;
        if ( ( m_minMaxKnown && child->m_recalcMinMax ) || !child->m_minMaxKnown ) {
            cmin = child->minWidth();
            cmax = child->maxWidth();
            test = true;
        }
        if ( child->m_recalcMinMax )
            child->recalcMinMaxWidths();
        if ( !child->m_minMaxKnown )
            child->calcMinMaxWidth();
        if ( m_minMaxKnown && test && (cmin != child->minWidth() || cmax != child->maxWidth()) )
            m_minMaxKnown = false;
        child = child->nextSibling();
    }

    // we need to recalculate, if the contains inline children, as the change could have
    // happened somewhere deep inside the child tree
    if ( ( !isInline() || isReplacedBlock() ) && childrenInline() )
        m_minMaxKnown = false;

    if ( !m_minMaxKnown )
        calcMinMaxWidth();
    m_recalcMinMax = false;
}

void RenderObject::scheduleRelayout(RenderObject *clippedObj)
{
    if (!isCanvas()) return;
    KHTMLView *view = static_cast<RenderCanvas *>(this)->view();
    if ( view )
        view->scheduleRelayout(clippedObj);
}


void RenderObject::removeLeftoverAnonymousBoxes()
{
}

InlineBox* RenderObject::createInlineBox(bool /*makePlaceHolderBox*/, bool /*isRootLineBox*/)
{
    KHTMLAssert(false);
    return 0;
}

void RenderObject::getTextDecorationColors(int decorations, QColor& underline, QColor& overline,
                                           QColor& linethrough, bool quirksMode)
{
    RenderObject* curr = this;
    do {
        RenderStyle *st = curr->style();
        int currDecs = st->textDecoration();
        if (currDecs) {
            if (currDecs & UNDERLINE) {
                decorations &= ~UNDERLINE;
                underline = st->color();
            }
            if (currDecs & OVERLINE) {
                decorations &= ~OVERLINE;
                overline = st->color();
            }
            if (currDecs & LINE_THROUGH) {
                decorations &= ~LINE_THROUGH;
                linethrough = st->color();
            }
        }
        curr = curr->parent();
        if (curr && curr->isRenderBlock() && curr->continuation())
            curr = curr->continuation();
    } while (curr && decorations && (!quirksMode || !curr->element() ||
                                     (curr->element()->id() != ID_A && curr->element()->id() != ID_FONT)));

    // If we bailed out, use the element we bailed out at (typically a <font> or <a> element).
    if (decorations && curr) {
        RenderStyle *st = curr->style();
        if (decorations & UNDERLINE)
            underline = st->color();
        if (decorations & OVERLINE)
            overline = st->color();
        if (decorations & LINE_THROUGH)
            linethrough = st->color();
    }
}

int RenderObject::maximalOutlineSize(PaintAction p) const
{
    if (p != PaintActionOutline)
        return 0;
    return static_cast<RenderCanvas*>(document()->renderer())->maximalOutlineSize();
}

void RenderObject::collectBorders(QValueList<CollapsedBorderValue>& borderStyles)
{
    for (RenderObject* curr = firstChild(); curr; curr = curr->nextSibling())
        curr->collectBorders(borderStyles);
}

bool RenderObject::flowAroundFloats() const
{
    return isReplaced() || hasOverflowClip() || style()->flowAroundFloats();
}

bool RenderObject::usesLineWidth() const
{
    // 1. All auto-width objects that avoid floats should always use lineWidth
    // 2. For objects with a specified width, we match WinIE's behavior:
    // (a) tables use contentWidth
    // (b) <hr>s use lineWidth
    // (c) all other objects use lineWidth in quirks mode and contentWidth in strict mode.
    return (flowAroundFloats() && (style()->width().isVariable() || isHR() || (style()->htmlHacks() && !isTable())));
}

bool RenderObject::hasCounter(const QString& counter) const
{
    if (style() && (!isText() || isCounter())) {
        if (lookupCounter(counter)) return true;
        if (style()->hasCounterReset(counter)) {
            return true;
        }
        else if (style()->hasCounterIncrement(counter)) {
            return true;
        }
    }
    if (counter == "list-item") {
        if (isListItem()) return true;
        if (element() && (
                element()->id() == ID_OL ||
                element()->id() == ID_UL ||
                element()->id() == ID_MENU ||
                element()->id() == ID_DIR))
            return true;
    } else
    if (counter == "-khtml-quotes" && isQuote()) {
        return (static_cast<const RenderQuote*>(this)->quoteCount() != 0);
    }
    return false;
}

CounterNode* RenderObject::getCounter(const QString& counter, bool view, bool counters)
{
//     kdDebug( 6040 ) << renderName() << " getCounter(" << counter << ")" << endl;

    if (!style()) return 0;

    if (isText() && !isCounter()) return 0;

    CounterNode *i = lookupCounter(counter);
    if (i) return i;
    int val = 0;

    if (style()->hasCounterReset(counter) || isRoot()) {
        i = new CounterReset(this);
        val = style()->counterReset(counter);
//         kdDebug( 6040 ) << renderName() << " counter-reset: " << counter << " " << val << endl;
    }
    else
    if (style()->hasCounterIncrement(counter)) {
        i = new CounterNode(this);
        val = style()->counterIncrement(counter);
//         kdDebug( 6040 ) << renderName() << " counter-increment: " << counter << " " << val << endl;
    }
    else if (counter == "list-item") {
        if (isListItem()) {
            if (element() && element()->id() == ID_LI) {
                DOMString v = static_cast<ElementImpl*>(element())->getAttribute(ATTR_VALUE);
                if ( !v.isEmpty() ) {
                    i = new CounterReset(this);
                    val = v.toInt();
//                     kdDebug( 6040 ) << renderName() << " counter-reset: " << counter << " " << val << endl;
                }
            }
            if (!i) {
                i = new CounterNode(this);
                val = 1;
//                 kdDebug( 6040 ) << renderName() << " counter-increment: " << counter << " " << val << endl;
            }
        }
        else
        if (element() && element()->id() == ID_OL) {
            i = new CounterReset(this);
            DOMString v = static_cast<ElementImpl*>(element())->getAttribute(ATTR_START);
            if ( !v.isEmpty() )
                val = v.toInt()-1;
            else
                val = 0;
//             kdDebug( 6040 ) << renderName() << " counter-reset: " << counter << " " << val << endl;
        }
        else
        if (element() &&
            (element()->id() == ID_UL ||
             element()->id() == ID_MENU||
             element()->id() == ID_DIR))
        {
            i = new CounterReset(this);
            val = 0;
//             kdDebug( 6040 ) << renderName() << " counter-reset: " << counter << " " << val << endl;
        }
    }
    else if (counter == "-khtml-quotes" && isQuote()) {
        i = new CounterNode(this);
        val = static_cast<RenderQuote*>(this)->quoteCount();
    }

    if (!i) {
        i = new CounterNode(this);
        val = 0;
//         kdDebug( 6040 ) << renderName() << " counter-increment: " << counter << " " << val << endl;
    }
    i->setValue(val);
    if (view) i->setIsVisual();
    if (counters) i->setHasCounters();

    insertCounter(counter, i);

    if (!isRoot()) {
        CounterNode *last=0, *current=0;
        RenderObject *n = previousSibling();
        while(n) {
            if (n->hasCounter(counter)) {
                current = n->getCounter(counter);
                break;
            }
            else
                n = n->previousSibling();
        }
        last = current;

        CounterNode *sibling = current;
        // counter-reset on same render-level is our counter-parent
        if (last) {
            // Found render-sibling, now search for later counter-siblings among its render-children
            n = n->lastChild();
            while (n) {
                if (n->hasCounter(counter)) {
                    current = n->getCounter(counter);
                    if (last->parent() == current->parent() || sibling == current->parent()) {
                        last = current;
                        // If the current counter is not the last, search deeper
                        if (current->nextSibling()) {
                            n = n->lastChild();
                            continue;
                        }
                        else
                            break;
                    }
                }
                n = n->previousSibling();
            }
            if (sibling->isReset())
                if (last != sibling)
                    sibling->insertAfter(i, last);
                else
                    sibling->insertAfter(i, 0);
            else
                last->parent()->insertAfter(i, last);
        }
        else {
            // Nothing found among siblings, let our parent search
            last = parent()->getCounter(counter, false);
            if (last->isReset())
                last->insertAfter(i, 0);
            else
                last->parent()->insertAfter(i, last);
        }
    }

    return i;
}

CounterNode* RenderObject::lookupCounter(const QString& counter) const
{
    QDict<khtml::CounterNode>* counters = document()->counters(this);
    if (counters)
        return counters->find(counter);
    else
        return 0;
}

void RenderObject::detachCounters()
{
    QDict<khtml::CounterNode>* counters = document()->counters(this);
    if (!counters) return;

    QDictIterator<khtml::CounterNode> i(*counters);

    while (i.current()) {
        (*i)->remove();
        delete (*i);
        ++i;
    }
    document()->removeCounters(this);
}

void RenderObject::insertCounter(const QString& counter, CounterNode* val)
{
    QDict<khtml::CounterNode>* counters = document()->counters(this);

    if (!counters) {
        counters = new QDict<khtml::CounterNode>(11);
        document()->setCounters(this, counters);
    }

    counters->insert(counter, val);
}


#undef RED_LUMINOSITY
#undef GREEN_LUMINOSITY
#undef BLUE_LUMINOSITY
#undef INTENSITY_FACTOR
#undef LIGHT_FACTOR
#undef LUMINOSITY_FACTOR

#undef MAX_COLOR
#undef COLOR_DARK_THRESHOLD
#undef COLOR_LIGHT_THRESHOLD

#undef COLOR_LITE_BS_FACTOR
#undef COLOR_LITE_TS_FACTOR

#undef COLOR_DARK_BS_FACTOR
#undef COLOR_DARK_TS_FACTOR

#undef LIGHT_GRAY
#undef DARK_GRAY

