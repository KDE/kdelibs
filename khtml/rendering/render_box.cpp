/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
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
 */
// -------------------------------------------------------------------------
//#define DEBUG_LAYOUT
//#define CLIP_DEBUG


#include <qpainter.h>

#include "misc/loader.h"
#include "rendering/render_replaced.h"
#include "rendering/render_canvas.h"
#include "rendering/render_table.h"
#include "rendering/render_inline.h"
#include "rendering/render_block.h"
#include "render_layer.h"
#include "misc/htmlhashes.h"
#include "xml/dom_nodeimpl.h"
#include "xml/dom_docimpl.h"
#include "render_line.h"

#include <khtmlview.h>
#include <kdebug.h>
#include <kglobal.h>
#include <assert.h>


using namespace DOM;
using namespace khtml;

#define TABLECELLMARGIN -0x4000

RenderBox::RenderBox(DOM::NodeImpl* node)
    : RenderContainer(node)
{
    m_minWidth = -1;
    m_maxWidth = -1;
    m_width = m_height = 0;
    m_x = 0;
    m_y = 0;
    m_marginTop = 0;
    m_marginBottom = 0;
    m_marginLeft = 0;
    m_marginRight = 0;
    m_staticX = 0;
    m_staticY = 0;

    m_placeHolderBox = 0;
    m_layer = 0;
}

RenderBlock* RenderBox::createAnonymousBlock()
{
    RenderStyle *newStyle = new RenderStyle();
    newStyle->inheritFrom(style());
    newStyle->setDisplay(BLOCK);

    RenderBlock *newBox = new (renderArena()) RenderBlock(document() /* anonymous*/);
    newBox->setStyle(newStyle);
    return newBox;
}

void RenderBox::restructureParentFlow() {
    if (!parent() || parent()->childrenInline() == isInline())
        return;
    // We have gone from not affecting the inline status of the parent flow to suddenly
    // having an impact.  See if there is a mismatch between the parent flow's
    // childrenInline() state and our state.
    if (!isInline()) {
        if (parent()->isRenderInline()) {
            // We have to split the parent flow.
            RenderInline* parentInline = static_cast<RenderInline*>(parent());
            RenderBlock* newBox = parentInline->createAnonymousBlock();
            
            RenderFlow* oldContinuation = parent()->continuation();
            parentInline->setContinuation(newBox);

            RenderObject* beforeChild = nextSibling();
            parent()->removeChildNode(this);
            parentInline->splitFlow(beforeChild, newBox, this, oldContinuation);
        }
        else if (parent()->isRenderBlock())
            static_cast<RenderBlock*>(parent())->makeChildrenNonInline();
    }
    else {
        // An anonymous block must be made to wrap this inline.
        RenderBlock* box = createAnonymousBlock();
        parent()->insertChildNode(box, this);
        box->appendChildNode(parent()->removeChildNode(this));
    }
}
 
void RenderBox::setStyle(RenderStyle *_style)
{
    bool affectsParent = style() && isFloatingOrPositioned() &&
         (!_style->isFloating() && _style->position() != ABSOLUTE && _style->position() != FIXED) &&
         parent() && (parent()->isBlockFlow() || parent()->isInlineFlow());
    
    RenderObject::setStyle(_style);

    // The root always paints its background/border.
    if (isRoot())
        setShouldPaintBackgroundOrBorder(true);

    switch(_style->display())
    {
    case INLINE:
    case INLINE_BLOCK:
    case INLINE_TABLE:
        setInline(true);
        break;
    case RUN_IN:
        if (isInline() && parent() && parent()->childrenInline()) 
            break;
    default:
        setInline(false);
    }

    switch(_style->position())
    {
    case ABSOLUTE:
    case FIXED:
        setPositioned(true);
        break;
    default:
        setPositioned(false);
        if( !isTableCell() && _style->isFloating() )
            setFloating(true);

        if( _style->position() == RELATIVE )
            setRelPositioned(true);
    }

    if (requiresLayer()) {
        if (!m_layer) {
            m_layer = new (renderArena()) RenderLayer(this);
            m_layer->insertOnlyThisLayer();
        }
    }
    else if (m_layer && !isRoot() && !isCanvas()) {
        m_layer->removeOnlyThisLayer();
        m_layer = 0;
    }

    if (m_layer) {
        // Make sure our z-index values are only applied if we're positioned or
        // relpositioned.
        if (!isPositioned() && !isRelPositioned()) {
            // Set the auto z-index flag.
            if (isRoot())
                style()->setZIndex(0);
            else
                style()->setHasAutoZIndex();
        }
        m_layer->styleChanged();
    }
    // ### outlineSize() and outlineOffset() not merged yet
    if (style()->outlineWidth() > 0 && style()->outlineWidth() > maximalOutlineSize(PaintActionOutline))
        static_cast<RenderCanvas*>(document()->renderer())->setMaximalOutlineSize(style()->outlineWidth());
    if (affectsParent)
        restructureParentFlow();
}

RenderBox::~RenderBox()
{
    //kdDebug( 6040 ) << "Element destructor: this=" << nodeName().string() << endl;
}

void RenderBox::detach()
{
    RenderLayer* layer = m_layer;
    RenderArena* arena = renderArena();

    RenderContainer::detach();

    if (layer)
        layer->detach(arena);
}

InlineBox* RenderBox::createInlineBox(bool /*makePlaceHolderBox*/, bool /*isRootLineBox*/)
{
    return (m_placeHolderBox = new (renderArena()) InlineBox(this));
}

void RenderBox::deleteInlineBoxes(RenderArena* arena)
{
    if (m_placeHolderBox) {
        m_placeHolderBox->detach( arena ? arena : renderArena() );
        m_placeHolderBox = 0;
    }
}

short RenderBox::contentWidth() const
{
    short w = m_width - style()->borderLeftWidth() - style()->borderRightWidth();
    w -= paddingLeft() + paddingRight();

    if (m_layer && style()->scrollsOverflow())
        w -= m_layer->verticalScrollbarWidth();

    //kdDebug( 6040 ) << "RenderBox::contentWidth(2) = " << w << endl;
    return w;
}

int RenderBox::contentHeight() const
{
    int h = m_height - style()->borderTopWidth() - style()->borderBottomWidth();
    h -= paddingTop() + paddingBottom();

    if (m_layer && style()->scrollsOverflow())
        h -= m_layer->horizontalScrollbarHeight();

    return h;
}

void RenderBox::setPos( int xPos, int yPos )
{
    m_x = xPos; m_y = yPos;
}

short RenderBox::width() const
{
    return m_width;
}

int RenderBox::height() const
{
    return m_height;
}

void RenderBox::setWidth( int width )
{
    m_width = width;
}

void RenderBox::setHeight( int height )
{
    m_height = height;
}

int RenderBox::calcBoxHeight(int h) const
{
    if (style()->boxSizing() == CONTENT_BOX)
        h += borderTop() + borderBottom() + paddingTop() + paddingBottom();

    return h;
}

int RenderBox::calcBoxWidth(int w) const
{
    if (style()->boxSizing() == CONTENT_BOX)
        w += borderLeft() + borderRight() + paddingLeft() + paddingRight();

    return w;
}

// --------------------- painting stuff -------------------------------

void RenderBox::paint(PaintInfo& i, int _tx, int _ty)
{
    _tx += m_x;
    _ty += m_y;

    if (style()->hidesOverflow() && m_layer)
        m_layer->subtractScrollOffset(_tx, _ty);

    // default implementation. Just pass things through to the children
    for(RenderObject* child = firstChild(); child; child = child->nextSibling())
        child->paint(i, _tx, _ty);
}

void RenderBox::paintRootBoxDecorations(PaintInfo& paintInfo, int _tx, int _ty)
{
    //kdDebug( 6040 ) << renderName() << "::paintRootBoxDecorations()" << _tx << "/" << _ty << endl;
    QColor c = style()->backgroundColor();
    CachedImage *bg = style()->backgroundImage();

    if (!c.isValid() && !bg && firstChild()) {
        if (!c.isValid())
            c = firstChild()->style()->backgroundColor();
        if (!bg)
            bg = firstChild()->style()->backgroundImage();
    }

    if( !c.isValid() && canvas()->view())
        c = canvas()->view()->palette().active().color(QColorGroup::Base);

    int w = width();
    int h = height();

    //    kdDebug(0) << "width = " << w <<endl;

    int rw, rh;
    if (canvas()->view()) {
        rw = canvas()->view()->contentsWidth();
        rh = canvas()->view()->contentsHeight();
    } else {
        rw = canvas()->docWidth();
        rh = canvas()->docHeight();
    }

    //    kdDebug(0) << "rw = " << rw <<endl;

    int bx = _tx - marginLeft();
    int by = _ty - marginTop();
    int bw = QMAX(w + marginLeft() + marginRight() + borderLeft() + borderRight(), rw);
    int bh = QMAX(h + marginTop() + marginBottom() + borderTop() + borderBottom(), rh);

    // CSS2 14.2:
    // " The background of the box generated by the root element covers the entire canvas."
    // hence, paint the background even in the margin areas (unlike for every other element!)
    // I just love these little inconsistencies .. :-( (Dirk)
    int my = kMax(by, paintInfo.r.y());

    paintBackground(paintInfo.p, c, bg, my, paintInfo.r.height(), bx, by, bw, bh);

    if(style()->hasBorder())
        paintBorder( paintInfo.p, _tx, _ty, w, h, style() );
}

void RenderBox::paintBoxDecorations(PaintInfo& paintInfo, int _tx, int _ty)
{
    //kdDebug( 6040 ) << renderName() << "::paintDecorations()" << endl;

    if(isRoot())
        return paintRootBoxDecorations(paintInfo, _tx, _ty);

    int w = width();
    int h = height() + borderTopExtra() + borderBottomExtra();
    _ty -= borderTopExtra();

    int my = kMax(_ty,paintInfo.r.y());
    int end = kMin( paintInfo.r.y() + paintInfo.r.height(), _ty + h );
    int mh = end - my;

    paintBackground(paintInfo.p, style()->backgroundColor(), style()->backgroundImage(), my, mh, _tx, _ty, w, h);

    if(style()->hasBorder()) {
        paintBorder(paintInfo.p, _tx, _ty, w, h, style());
    }
}

void RenderBox::paintBackground(QPainter *p, const QColor &c, CachedImage *bg, int clipy, int cliph, int _tx, int _ty, int w, int height)
{
    paintBackgroundExtended(p, c, bg, clipy, cliph, _tx, _ty, w, height,
                            borderLeft(), borderRight());
}

void RenderBox::paintBackgroundExtended(QPainter *p, const QColor &c, CachedImage *bg, int clipy, int cliph,
                                        int _tx, int _ty, int w, int h,
                                        int bleft, int bright)
{
    if ( cliph < 0 )
	return;

    if(c.isValid())
        p->fillRect(_tx, clipy, w, cliph, c);

    // no progressive loading of the background image
    if(bg && bg->pixmap_size() == bg->valid_rect().size() && !bg->isTransparent() && !bg->isErrorImage()) {
        //kdDebug( 6040 ) << "painting bgimage at " << _tx << "/" << _ty << endl;
        // ### might need to add some correct offsets
        // ### use paddingX/Y

        //hacky stuff
        RenderStyle* sptr = style();
        if ( isRoot() && firstChild() && !style()->backgroundImage() )
            sptr = firstChild()->style();

        int sx = 0;
        int sy = 0;
        int cw,ch;
        int cx,cy;
        int vpab = bleft + bright;
        int hpab = borderTop() + borderBottom();

        // CSS2 chapter 14.2.1

	int pixw = bg->pixmap_size().width();
	int pixh = bg->pixmap_size().height();
        if (sptr->backgroundAttachment())
        {
            //scroll
            int pw = w - vpab;
            int ph = h - hpab;
            EBackgroundRepeat bgr = sptr->backgroundRepeat();
            if( (bgr == NO_REPEAT || bgr == REPEAT_Y) && pw > pixw ) {
                cw = pixw;
                int xp = sptr->backgroundXPosition().minWidth(pw-pixw);
                if ( xp >= 0 )
                    cx = _tx + xp;
                else {
                    cx = _tx;
                    if (pixw == 0)
                        sx = 0;
                    else {
                        sx = -xp;
                        cw += xp;
                    }
                }
                cx += bleft;
            } else {
                cw = w;
                cx = _tx;
                if (pixw == 0)
                    sx = 0;
                else {
                    sx =  pixw - ((sptr->backgroundXPosition().minWidth(pw-pixw)) % pixw );
                    sx -= bleft % pixw;
                }
            }

            if( (bgr == NO_REPEAT || bgr == REPEAT_X) && ph > pixh ) {
                ch = pixh;
                int yp = sptr->backgroundYPosition().minWidth(ph-pixh);
                if ( yp >= 0 )
                    cy = _ty + yp;
                else {
                    cy = _ty;
                    if (pixh == 0) {
                        sy = 0;
                    } else {
                        sy = -yp;
                        ch += yp;
                    }
                }

                cy += borderTop();
            } else {
                ch = h;
                cy = _ty;
                if (pixh == 0) {
                    sy = 0;
                } else {
                    sy = pixh - ((sptr->backgroundYPosition().minWidth(ph-pixh)) % pixh );
                    sy -= borderTop() % pixh;
                }
            }

	    if (layer())
		layer()->scrollOffset(sx, sy);
        }
        else
        {
            //fixed
            QRect vr = viewRect();
            int pw = vr.width();
            int ph = vr.height();

            EBackgroundRepeat bgr = sptr->backgroundRepeat();
            if( (bgr == NO_REPEAT || bgr == REPEAT_Y) && pw > pixw ) {
                cw = pixw;
                cx = vr.x() + sptr->backgroundXPosition().minWidth(pw-pixw);
            } else {
                cw = pw;
                cx = vr.x();
                sx =  pixw ? pixw - ((sptr->backgroundXPosition().minWidth(pw-pixw)) % pixw ) : 0;
            }

            if( (bgr == NO_REPEAT || bgr == REPEAT_X) && ph > pixh ) {
                ch = pixh;
                cy = vr.y() + sptr->backgroundYPosition().minWidth(ph-pixh);
            } else {
                ch = ph;
                cy = vr.y();
                sy = pixh ? pixh - ((sptr->backgroundYPosition().minWidth(ph-pixh)) % pixh ) : 0;
            }

            QRect fix(cx,cy,cw,ch);
            QRect ele(_tx+bleft,_ty+borderTop(),w-vpab,h-hpab);
            QRect b = fix.intersect(ele);

            //kdDebug() <<" ele is " << ele << " b is " << b << " fix is " << fix << endl;
            sx+=b.x()-cx;
            sy+=b.y()-cy;
            cx=b.x();cy=b.y();cw=b.width();ch=b.height();
        }


        //kdDebug() << "cy="<<cy<< " ch="<<ch << " clipy=" << clipy << " cliph=" << cliph << " sx="<<sx << " sy="<<sy << endl;
	int diff = clipy - cy;
	if ( diff > 0 ) {
	    cy += diff;
	    sy += diff;
	    sy %= pixh;
	    ch -= diff;
	}
	ch = kMin( ch, clipy + cliph - cy );
 	//kdDebug() << "clip="<<cx << " cy="<<cy<< " cw="<<cw << " ch="<<ch << " sx="<<sx << " sy="<<sy << endl;

        if (cw>0 && ch>0)
            p->drawTiledPixmap(cx, cy, cw, ch, bg->tiled_pixmap(c), sx, sy);
//            p->drawTiledPixmap(cx, cy, cw, ch, bg->pixmap(), sx, sy);

    }
}

void RenderBox::outlineBox(QPainter *p, int _tx, int _ty, const char *color)
{
    p->setPen(QPen(QColor(color), 1, Qt::DotLine));
    p->setBrush( Qt::NoBrush );
    p->drawRect(_tx, _ty, m_width, m_height);
}

QRect RenderBox::getOverflowClipRect(int tx, int ty)
{
    // XXX When overflow-clip (CSS3) is implemented, we'll obtain the property
    // here.
    int bl=borderLeft(),bt=borderTop(),bb=borderBottom(),br=borderRight();
    int clipx = tx+bl;
    int clipy = ty+bt;
    int clipw = m_width-bl-br;
    int cliph = m_height-bt-bb;

    // Substract out scrollbars if we have them.
    if (m_layer) {
        clipw -= m_layer->verticalScrollbarWidth();
        cliph -= m_layer->horizontalScrollbarHeight();
    }

    return QRect(clipx,clipy,clipw,cliph);
}

QRect RenderBox::getClipRect(int tx, int ty)
{
    int bl=borderLeft(),bt=borderTop(),bb=borderBottom(),br=borderRight();
    // ### what about apddings?
    int clipw = m_width-bl-br;
    int cliph = m_height-bt-bb;

    bool rtl = (style()->direction() == RTL);

    int clipleft = 0;
    int clipright = clipw;
    int cliptop = 0;
    int clipbottom = cliph;

    if ( style()->hasClip() && style()->position() == ABSOLUTE ) {
	// the only case we use the clip property according to CSS 2.1
	if (!style()->clipLeft().isVariable()) {
	    int c = style()->clipLeft().width(clipw);
	    if ( rtl )
		clipleft = clipw - c;
	    else
		clipleft = c;
	}
	if (!style()->clipRight().isVariable()) {
	    int w = style()->clipRight().width(clipw);
	    if ( rtl ) {
		clipright = clipw - w;
	    } else {
		clipright = w;
	    }
	}
	if (!style()->clipTop().isVariable())
	    cliptop = style()->clipTop().width(cliph);
	if (!style()->clipBottom().isVariable())
	    clipbottom = style()->clipBottom().width(cliph);
    }
    int clipx = tx + clipleft;
    int clipy = ty + cliptop;
    clipw = clipright-clipleft;
    cliph = clipbottom-cliptop;

    //kdDebug( 6040 ) << "setting clip("<<clipx<<","<<clipy<<","<<clipw<<","<<cliph<<")"<<endl;

    return QRect(clipx,clipy,clipw,cliph);
}

void RenderBox::close()
{
    setNeedsLayoutAndMinMaxRecalc();
}

short RenderBox::containingBlockWidth() const
{
    if (isRoot() && canvas()->view())
        return canvas()->view()->visibleWidth();

    RenderBlock* cb = containingBlock();
    if (usesLineWidth())
        return cb->lineWidth(m_y);
    else
        return cb->contentWidth();
}

bool RenderBox::absolutePosition(int &xPos, int &yPos, bool f)
{
    if ( style()->position() == FIXED )
	f = true;
    RenderObject *o = container();
    if( o && o->absolutePosition(xPos, yPos, f))
    {
        if ( o->style()->hidesOverflow() && o->layer() )
            o->layer()->subtractScrollOffset( xPos, yPos );

        if(!isInline() || isReplaced())
            xPos += m_x, yPos += m_y;

        if(isRelPositioned())
            relativePositionOffset(xPos, yPos);
        return true;
    }
    else
    {
        xPos = yPos = 0;
        return false;
    }
}

void RenderBox::position(InlineBox* box, int /*from*/, int /*len*/, bool /*reverse*/)
{
    if (isPositioned()) {
        // Cache the x position only if we were an INLINE type originally.
        bool wasInline = style()->originalDisplay() == INLINE ||
                         style()->originalDisplay() == INLINE_TABLE;
        
        if (wasInline && hasStaticX()) {
            // The value is cached in the xPos of the box.  We only need this value if
            // our object was inline originally, since otherwise it would have ended up underneath
            // the inlines.
            m_staticX = box->xPos();
        }
        else if (!wasInline && hasStaticY()) {
            // Our object was a block originally, so we make our normal flow position be
            // just below the line box (as though all the inlines that came before us got
            // wrapped in an anonymous block, which is what would have happened had we been
            // in flow).  This value was cached in the yPos() of the box.
            m_staticY = box->yPos();
        }
    }
    else if (isReplaced())
        setPos( box->xPos(), box->yPos() );
}

void RenderBox::repaint(bool immediate)
{
    int ow = style() ? style()->outlineWidth() : 0;
    if( isInline() && !isReplaced() )
    {
	RenderObject* p = parent();
	Q_ASSERT(p);
	while( p->isInline() && !p->isReplaced() )
	    p = p->parent();
        p->repaintRectangle( -ow, -ow, p->effectiveWidth()+ow*2, p->effectiveHeight()+ow*2, immediate);
    }
    else
    {
        repaintRectangle( -ow, -ow, effectiveWidth()+ow*2, effectiveHeight()+ow*2, immediate);
    }
}

void RenderBox::repaintRectangle(int x, int y, int w, int h, bool immediate, bool f)
{
    x += m_x;
    y += m_y;

    // Apply the relative position offset when invalidating a
    // rectangle.  The layer is translated, but the render box isn't,
    // so we need to do this to get the right dirty rect.
    if (isRelPositioned())
        relativePositionOffset(x,y);

    if (style()->position()==FIXED) f=true;


    // kdDebug( 6040 ) << "RenderBox(" <<this << ", " << renderName() << ")::repaintRectangle (" << x << "/" << y << ") (" << w << "/" << h << ")" << endl;
    RenderObject *o = container();
    if( o ) o->repaintRectangle(x, y, w, h, immediate, f);
}

void RenderBox::relativePositionOffset(int &tx, int &ty) const
{
    if(!style()->left().isVariable())
        tx += style()->left().width(containingBlockWidth());
    else if(!style()->right().isVariable())
        tx -= style()->right().width(containingBlockWidth());
    if(!style()->top().isVariable())
    {
        if (!style()->top().isPercent()
                || containingBlock()->style()->height().isFixed())
            ty += style()->top().width(containingBlockHeight());
    }
    else if(!style()->bottom().isVariable())
    {
        if (!style()->bottom().isPercent()
                || containingBlock()->style()->height().isFixed())
            ty -= style()->bottom().width(containingBlockHeight());
    }
}

void RenderBox::calcWidth()
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << "RenderBox("<<renderName()<<")::calcWidth()" << endl;
#endif
    if (isPositioned())
    {
        calcAbsoluteHorizontal();
    }
    else
    {
        bool treatAsReplaced = isReplaced() && !isInlineBlockOrInlineTable();
        Length w;
        if (treatAsReplaced)
            w = Length( calcReplacedWidth(), Fixed );
        else
            w = style()->width();

        Length ml = style()->marginLeft();
        Length mr = style()->marginRight();

	int cw = containingBlockWidth();
        if (cw<0) cw = 0;

        m_marginLeft = 0;
        m_marginRight = 0;

        if (isInline() && !isInlineBlockOrInlineTable())
        {
            // just calculate margins
            m_marginLeft = ml.minWidth(cw);
            m_marginRight = mr.minWidth(cw);
            if (treatAsReplaced)
            {
                m_width = calcBoxWidth(w.width(cw));
                m_width = KMAX(m_width, m_minWidth);
            }

            return;
        }
        else
        {
            LengthType widthType, minWidthType, maxWidthType;
            if (treatAsReplaced) {
                m_width = calcBoxWidth(w.width(cw));
                widthType = w.type();
            } else {
                m_width = calcWidthUsing(Width, cw, widthType);
                int minW = calcWidthUsing(MinWidth, cw, minWidthType);
                int maxW = style()->maxWidth().value() == UNDEFINED ?
                             m_width : calcWidthUsing(MaxWidth, cw, maxWidthType);

                if (m_width > maxW) {
                    m_width = maxW;
                    widthType = maxWidthType;
                }
                if (m_width < minW) {
                    m_width = minW;
                    widthType = minWidthType;
                }
            }

            if (widthType == Variable) {
    //          kdDebug( 6040 ) << "variable" << endl;
                m_marginLeft = ml.minWidth(cw);
                m_marginRight = mr.minWidth(cw);
            }
            else
            {
//              kdDebug( 6040 ) << "non-variable " << w.type << ","<< w.value << endl;
                calcHorizontalMargins(ml,mr,cw);
            }
        }

        if (cw && cw != m_width + m_marginLeft + m_marginRight && !isFloating() && !isInline())
        {
            if (containingBlock()->style()->direction()==LTR)
                m_marginRight = cw - m_width - m_marginLeft;
            else
                m_marginLeft = cw - m_width - m_marginRight;
        }
    }

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << "RenderBox::calcWidth(): m_width=" << m_width << " containingBlockWidth()=" << containingBlockWidth() << endl;
    kdDebug( 6040 ) << "m_marginLeft=" << m_marginLeft << " m_marginRight=" << m_marginRight << endl;
#endif
}

int RenderBox::calcWidthUsing(WidthType widthType, int cw, LengthType& lengthType)
{
    int width = m_width;
    Length w;
    if (widthType == Width)
        w = style()->width();
    else if (widthType == MinWidth)
        w = style()->minWidth();
    else
        w = style()->maxWidth();

    lengthType = w.type();

    if (lengthType == Variable) {
        int marginLeft = style()->marginLeft().minWidth(cw);
        int marginRight = style()->marginRight().minWidth(cw);
        if (cw) width = cw - marginLeft - marginRight;

        // size to max width?
        if (sizesToMaxWidth()) {
            width = KMAX(width, (int)m_minWidth);
            width = KMIN(width, (int)m_maxWidth);
        }
    }
    else
    {
        width = calcBoxWidth(w.width(cw));
    }

    return width;
}

void RenderBox::calcHorizontalMargins(const Length& ml, const Length& mr, int cw)
{
    if (isFloating() || isInline()) // Inline blocks/tables and floats don't have their margins increased.
    {
        m_marginLeft = ml.minWidth(cw);
        m_marginRight = mr.minWidth(cw);
    }
    else
    {
        if ( (ml.isVariable() && mr.isVariable()) ||
             (!ml.isVariable() && !mr.isVariable() &&
                containingBlock()->style()->textAlign() == KHTML_CENTER) )
        {
            m_marginLeft = (cw - m_width)/2;
            if (m_marginLeft<0) m_marginLeft=0;
            m_marginRight = cw - m_width - m_marginLeft;
        }
        else if (mr.isVariable() ||
                 (!ml.isVariable() && containingBlock()->style()->direction() == RTL &&
                  containingBlock()->style()->textAlign() == KHTML_LEFT))
        {
            m_marginLeft = ml.width(cw);
            m_marginRight = cw - m_width - m_marginLeft;
        }
        else if (ml.isVariable() ||
                 (!mr.isVariable() && containingBlock()->style()->direction() == LTR &&
                  containingBlock()->style()->textAlign() == KHTML_RIGHT))
        {
            m_marginRight = mr.width(cw);
            m_marginLeft = cw - m_width - m_marginRight;
        }
        else
        {
            m_marginLeft = ml.minWidth(cw);
            m_marginRight = mr.minWidth(cw);
        }
    }
}

void RenderBox::calcHeight()
{

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << "RenderBox::calcHeight()" << endl;
#endif

    //cell height is managed by table, inline elements do not have a height property.
    if ( isTableCell() || (isInline() && !isReplaced()) )
        return;

    if (isPositioned())
        calcAbsoluteVertical();
    else
    {
        calcVerticalMargins();

        // For tables, calculate margins only
        if (isTable())
            return;

        Length h;
        bool treatAsReplaced = isReplaced() && !isInlineBlockOrInlineTable();
        bool checkMinMaxHeight = false;

        if ( treatAsReplaced )
            h = Length( calcReplacedHeight(), Fixed );
        else {
            h = style()->height();
            checkMinMaxHeight = true;
        }

        int height;
        if (checkMinMaxHeight) {
            height = calcHeightUsing(style()->height());
            int minH = calcHeightUsing(style()->minHeight());
            int maxH = style()->maxHeight().value() == UNDEFINED ? height : calcHeightUsing(style()->maxHeight());
            height = kMin(maxH, height);
            height = kMax(minH, height);
        }
        else {
            // The only times we don't check min/max height are when a fixed length has
            // been given as an override.  Just use that.
            height = calcBoxHeight(h.value());
        }

        if (height<m_height && !overhangingContents() && style()->overflow()==OVISIBLE)
            setOverhangingContents();

        m_height = height;
    }

    // Unfurling marquees override with the furled height.
    if (style()->overflow() == OMARQUEE && m_layer && m_layer->marquee() &&
        m_layer->marquee()->isUnfurlMarquee() && !m_layer->marquee()->isHorizontal()) {
        m_layer->marquee()->setEnd(m_height);
        m_height = kMin(m_height, m_layer->marquee()->unfurlPos());
    }

}

int RenderBox::calcHeightUsing(const Length& h)
{
    if (!h.isVariable()) {
        int height = -1;
        if (h.isFixed())
            height = h.value();
        else if (h.isPercent())
            height = calcPercentageHeight(h);
        if (height != -1) {
            height = calcBoxHeight(height);
            return height;
        }
    }
    return m_height;
}

int RenderBox::calcPercentageHeight(const Length& height)
{
    int result = -1;
    RenderBlock* cb = containingBlock();
    // Table cells violate what the CSS spec says to do with heights.  Basically we
    // don't care if the cell specified a height or not.  We just always make ourselves
    // be a percentage of the cell's current content height.
    if (cb->isTableCell()) {
        result = static_cast<RenderTableCell*>(cb)->cellPercentageHeight();
        if (result == 0)
            return -1;

        // It is necessary to use the border-box to match WinIE's broken
        // box model.  This is essential for sizing inside
        // table cells using percentage heights.
        if (style()->boxSizing() != BORDER_BOX) {
            result -= (borderTop() + paddingTop() + borderBottom() + paddingBottom());
            result = kMax(0, result);
        }
    }

    // Otherwise we only use our percentage height if our containing block had a specified
    // height.
    else if (cb->style()->height().isFixed())
        result = cb->style()->height().value();
    else if (cb->style()->height().isPercent())
        // We need to recur and compute the percentage height for our containing block.
        result = cb->calcPercentageHeight(cb->style()->height());
    else if (cb->isCanvas() || ( cb->isBody() && style()->htmlHacks() && 
                                 cb->style()->height().isVariable() && !cb->isFloatingOrPositioned())) {
        // Don't allow this to affect the block' m_height member variable, since this
        // can get called while the block is still laying out its kids.
        int oldHeight = cb->height();
        cb->calcHeight();
        result = cb->contentHeight();
        cb->setHeight(oldHeight);
        if (cb->isBody()) {
            // In quirk mode, percentages of body apply at least to the canvas's visible height (minus pbm)
            int margins = cb->collapsedMarginTop() + cb->collapsedMarginBottom();
            int visHeight = canvas()->view()->visibleHeight();
            RenderObject* p = cb->parent();
            result = kMax(result, visHeight - 
                        (margins + p->marginTop() + p->marginBottom() + 
                         p->borderTop() + p->borderBottom() +
                         p->paddingTop() + p->paddingBottom()));
        }
    }
    if (result != -1) {
        result = height.width(result);
    }
    return result;
}

short RenderBox::calcReplacedWidth() const
{
    int width = calcReplacedWidthUsing(Width);
    int minW = calcReplacedWidthUsing(MinWidth);
    int maxW = style()->maxWidth().value() == UNDEFINED ? width : calcReplacedWidthUsing(MaxWidth);

    if (width > maxW)
        width = maxW;

    if (width < minW)
        width = minW;

    return width;
}

int RenderBox::calcReplacedWidthUsing(WidthType widthType) const
{
    Length w;
    if (widthType == Width)
        w = style()->width();
    else if (widthType == MinWidth)
        w = style()->minWidth();
    else
        w = style()->maxWidth();

    switch (w.type()) {
    case Fixed:
        return w.value();
    case Percent:
    {
        const int cw = containingBlockWidth();
        if (cw > 0) {
            int result = w.minWidth(cw);
            return result;
        }
    }
    // fall through
    default:
        return intrinsicWidth();
    }
}

int RenderBox::calcReplacedHeight() const
{
    int height = calcReplacedHeightUsing(Height);
    int minH = calcReplacedHeightUsing(MinHeight);
    int maxH = style()->maxHeight().value() == UNDEFINED ? height : calcReplacedHeightUsing(MaxHeight);

    if (height > maxH)
        height = maxH;

    if (height < minH)
        height = minH;

    return height;
}

int RenderBox::calcReplacedHeightUsing(HeightType heightType) const
{
    Length h;
    if (heightType == Height)
        h = style()->height();
    else if (heightType == MinHeight)
        h = style()->minHeight();
    else
        h = style()->maxHeight();
    switch( h.type() ) {
    case Percent:
        return availableHeightUsing(h);
    case Fixed:
        return h.value();
    default:
        return intrinsicHeight();
    };
}

int RenderBox::availableHeight() const
{
    return availableHeightUsing(style()->height());
}

int RenderBox::availableHeightUsing(const Length& h) const
{
    if (h.isFixed())
        return h.value();

    if (isCanvas())
        return static_cast<const RenderCanvas*>(this)->viewportHeight();

    // We need to stop here, since we don't want to increase the height of the table
    // artificially.  We're going to rely on this cell getting expanded to some new
    // height, and then when we lay out again we'll use the calculation below.
    if (isTableCell() && (h.isVariable() || h.isPercent())) {
        const RenderTableCell* tableCell = static_cast<const RenderTableCell*>(this);
        return tableCell->cellPercentageHeight() -
	    (borderTop()+borderBottom()+paddingTop()+paddingBottom());
    }

    if (h.isPercent())
       return h.width(containingBlock()->availableHeight());

    return containingBlock()->availableHeight();
}

void RenderBox::calcVerticalMargins()
{
    if( isTableCell() ) {
	// table margins are basically infinite
	m_marginTop = TABLECELLMARGIN;
	m_marginBottom = TABLECELLMARGIN;
	return;
    }

    Length tm = style()->marginTop();
    Length bm = style()->marginBottom();

    // margins are calculated with respect to the _width_ of
    // the containing block (8.3)
    int cw = containingBlock()->contentWidth();

    m_marginTop = tm.minWidth(cw);
    m_marginBottom = bm.minWidth(cw);
}

void RenderBox::setStaticX(short staticX)
{
    m_staticX = staticX;
}

void RenderBox::setStaticY(int staticY)
{
    m_staticY = staticY;
}

void RenderBox::calcAbsoluteHorizontal()
{
    const int AUTO = -666666;
    int l,r,w,ml,mr,cw;

    RenderObject* cb = container();
    int pab = borderLeft()+ borderRight()+ paddingLeft()+ paddingRight();

    l=r=ml=mr=w=AUTO;
    cw = containingBlockWidth() + cb->paddingLeft() + cb->paddingRight();

    if(!style()->left().isVariable())
        l = style()->left().width(cw);
    if(!style()->right().isVariable())
        r = style()->right().width(cw);
    if(!style()->width().isVariable())
        w = style()->width().width(cw);
    else if (isReplaced())
        w = intrinsicWidth();
    if(!style()->marginLeft().isVariable())
        ml = style()->marginLeft().width(cw);
    if(!style()->marginRight().isVariable())
        mr = style()->marginRight().width(cw);


    //qDebug("h1: w=%d, l=%d, r=%d, ml=%d, mr=%d",w,l,r,ml,mr);

    int static_distance=0;
    if ((parent()->style()->direction()==LTR && (l==AUTO && r==AUTO ))
            || style()->left().isStatic())
    {
        // calc hypothetical location in the normal flow
        // used for 1) left=static-position
        //          2) left, right, width are all auto -> calc top -> 3.
        //          3) precalc for case 2 below

        // all positioned elements are blocks, so that
        // would be at the left edge

        static_distance = m_staticX - cb->borderLeft(); // Should already have been set through layout of the parent().
        for (RenderObject* po = parent(); po && po != cb; po = po->parent())
            static_distance += po->xPos();

        if (l==AUTO || style()->left().isStatic())
            l = static_distance;
    }

    else if ((parent()->style()->direction()==RTL && (l==AUTO && r==AUTO ))
            || style()->right().isStatic())
    {
        static_distance = m_staticX - cb->borderLeft(); // Should already have been set through layout of the parent().
        for (RenderObject* po = parent(); po && po != cb; po = po->parent())
            static_distance += po->xPos();

        if (r==AUTO || style()->right().isStatic())
            r = static_distance;
    }


    if (l!=AUTO && w!=AUTO && r!=AUTO)
    {
        // left, width, right all given, play with margins
        int ot = l + w + r + pab;

        if (ml==AUTO && mr==AUTO)
        {
            // both margins auto, solve for equality
            ml = (cw - ot)/2;
            mr = cw - ot - ml;
        }
        else if (ml==AUTO)
            // solve for left margin
            ml = cw - ot - mr;
        else if (mr==AUTO)
            // solve for right margin
            mr = cw - ot - ml;
        else
        {
            // overconstrained, solve according to dir
            if (style()->direction()==LTR)
                r = cw - ( l + w + ml + mr + pab);
            else
                l = cw - ( r + w + ml + mr + pab);
        }
    }
    else
    {
        // one or two of (left, width, right) missing, solve

        // auto margins are ignored
        if (ml==AUTO) ml = 0;
        if (mr==AUTO) mr = 0;

        //1. solve left & width.
        if (l==AUTO && w==AUTO && r!=AUTO) {
            w = kMin( int ( m_maxWidth ), kMax(cw - ( r + ml + mr + pab), int ( m_minWidth ) ));
            l = cw - ( r + w + ml + mr + pab);
        }
        else
        //2. solve left & right. use static positioning.
        if (l==AUTO && w!=AUTO && r==AUTO) {
            if (style()->direction()==RTL) {
                r = static_distance;
                l = cw - ( r + w + ml + mr + pab);
            }
            else {
                l = static_distance;
                r = cw - ( l + w + ml + mr + pab);
            }
        }
        else
        //3. solve width & right.
        if (l!=AUTO && w==AUTO && r==AUTO) {
            w = kMin(int ( m_maxWidth - pab ), kMax( int ( m_minWidth - pab ), cw - ( l + ml + mr + pab)));
            r = cw - ( l + w + ml + mr + pab);
        }
        else

        //4. solve left
        if (l==AUTO && w!=AUTO && r!=AUTO)
            l = cw - ( r + w + ml + mr + pab);
        else

        //5. solve width
        if (l!=AUTO && w==AUTO && r!=AUTO)
            w = cw - ( r + l + ml + mr + pab);
        else

        //6. solve right
        if (l!=AUTO && w!=AUTO && r==AUTO)
            r = cw - ( l + w + ml + mr + pab);
    }

    m_width = w + pab;
    m_marginLeft = ml;
    m_marginRight = mr;
    short old_m_x = m_x;
    m_x = l + ml + cb->borderLeft();
    if ( old_m_x != m_x && m_layer)
        m_layer->updateLayerPosition();

    //qDebug("h: w=%d, l=%d, r=%d, ml=%d, mr=%d",w,l,r,ml,mr);
}


void RenderBox::calcAbsoluteVertical()
{
    // css2 spec 10.6.4 & 10.6.5

    // based on
    // http://www.w3.org/Style/css2-updates/REC-CSS2-19980512-errata
    // (actually updated 2000-10-24)
    // that introduces static-position value for top, left & right

    const int AUTO = -666666;
    int t,b,h,mt,mb,ch;

    t=b=h=mt=mb=AUTO;

    int pab = borderTop()+borderBottom()+paddingTop()+paddingBottom();

    RenderObject* cb = container();

    Length hl = cb->style()->height();
    if (hl.isFixed())
        ch = hl.value() + cb->paddingTop() + cb->paddingBottom();
    else if (cb->isCanvas() || cb->isRoot())
        ch = cb->availableHeight();
    else
        ch = cb->height() - cb->borderTop() - cb->borderBottom();


    if(!style()->top().isVariable())
        t = style()->top().width(ch);
    if(!style()->bottom().isVariable())
        b = style()->bottom().width(ch);
    // for tables "auto" means shrink-to-fit
    if ( isTable() && style()->height().isVariable() )
        h = m_height - pab;
    else if(!style()->height().isVariable())
    {
        h = style()->height().width(ch);

        if (m_height-pab > h) {
            if ( isRenderBlock() ) {
              static_cast<RenderBlock*>( this )->setOverflowHeight( m_height + pab - ( paddingBottom() + borderBottom() ) );
            }
            m_height = h + pab;
        }
    }
    else if (isReplaced())
        h = intrinsicHeight();

    if(!style()->marginTop().isVariable())
        mt = style()->marginTop().width(ch);
    if(!style()->marginBottom().isVariable())
        mb = style()->marginBottom().width(ch);

    int static_top=0;
    if ((t==AUTO && b==AUTO ) || style()->top().isStatic())
    {
        // calc hypothetical location in the normal flow
        // used for 1) top=static-position
        //          2) top, bottom, height are all auto -> calc top -> 3.
        //          3) precalc for case 2 below
        static_top = m_staticY -cb->borderTop(); // Should already have been set through layout of the parent().
        RenderObject* po = parent();
        for (; po && po != cb; po = po->parent())
            static_top += po->yPos();

        if (h==AUTO || style()->top().isStatic())
            t = static_top;
    }

    if (t!=AUTO && h!=AUTO && b!=AUTO)
    {
        // top, height, bottom all given, play with margins
        int ot = h + t + b + pab;

        if (mt==AUTO && mb==AUTO)
        {
            // both margins auto, solve for equality
            mt = (ch - ot)/2;
            mb = ch - ot - mt;
        }
        else if (mt==AUTO)
            // solve for top margin
            mt = ch - ot - mb;
        else if (mb==AUTO)
            // solve for bottom margin
            mb = ch - ot - mt;
        else
            // overconstrained, solve for bottom
            b = ch - ( h+t+mt+mb+pab);
    }
    else
    {
        // one or two of (top, height, bottom) missing, solve

        // auto margins are ignored
        if (mt==AUTO) mt = 0;
        if (mb==AUTO) mb = 0;

        //1. solve top & height. use content height.
        if (t==AUTO && h==AUTO && b!=AUTO)
        {
            h = m_height-pab;
            t = ch - ( h+b+mt+mb+pab);
        }
        else

        //2. solve top & bottom. use static positioning.
        if (t==AUTO && h!=AUTO && b==AUTO)
        {
            t = static_top;
            b = ch - ( h+t+mt+mb+pab);
        }
        else

        //3. solve height & bottom. use content height.
        if (t!=AUTO && h==AUTO && b==AUTO)
        {
            h = m_height-pab;
            b = ch - ( h+t+mt+mb+pab);
        }
        else

        //4. solve top
        if (t==AUTO && h!=AUTO && b!=AUTO)
            t = ch - ( h+b+mt+mb+pab);
        else

        //5. solve height
        if (t!=AUTO && h==AUTO && b!=AUTO)
            h = ch - ( t+b+mt+mb+pab);
        else

        //6. solve bottom
        if (t!=AUTO && h!=AUTO && b==AUTO)
            b = ch - ( h+t+mt+mb+pab);
    }

    if (m_height<h+pab) //content must still fit
        m_height = h+pab;

    if (style()->hidesOverflow() && m_height > h+pab)
        m_height = h+pab;

    m_marginTop = mt;
    m_marginBottom = mb;
    m_y = t + mt + cb->borderTop();

    //qDebug("v: m_height = %d, h=%d, t=%d, b=%d, mt=%d, mb=%d, m_y=%d",m_height,h,t,b,mt,mb,m_y);
}


int RenderBox::lowestPosition(bool includeOverflowInterior, bool includeSelf) const
{
    return includeSelf ? m_height : 0;
}

int RenderBox::rightmostPosition(bool includeOverflowInterior, bool includeSelf) const
{
    return includeSelf ? m_width : 0;
}

int RenderBox::leftmostPosition(bool includeOverflowInterior, bool includeSelf) const
{
    return includeSelf ? 0 : m_width;
}

void RenderBox::caretPos(int /*offset*/, int flags, int &_x, int &_y, int &width, int &height)
{
#if 0
    _x = -1;

    // propagate it downwards to its children, someone will feel responsible
    RenderObject *child = firstChild();
//    if (child) kdDebug(6040) << "delegating caretPos to " << child->renderName() << endl;
    if (child) child->caretPos(offset, override, _x, _y, width, height);

    // if not, use the extents of this box. offset 0 means left, offset 1 means
    // right
    if (_x == -1) {
        //kdDebug(6040) << "no delegation" << endl;
        _x = xPos() + (offset == 0 ? 0 : m_width);
	_y = yPos();
	height = m_height;
	width = override && offset == 0 ? m_width : 1;

	// If height of box is smaller than font height, use the latter one,
	// otherwise the caret might become invisible.
	// FIXME: ignoring :first-line, missing good reason to take care of
	int fontHeight = style()->fontMetrics().height();
	if (fontHeight > height)
	  height = fontHeight;

        int absx, absy;

        RenderObject *cb = containingBlock();

        if (cb && cb != this && cb->absolutePosition(absx,absy)) {
            //kdDebug(6040) << "absx=" << absx << " absy=" << absy << endl;
            _x += absx;
            _y += absy;
        } else {
            // we don't know our absolute position, and there is no point returning
            // just a relative one
            _x = _y = -1;
        }
    }
#endif

    _x = xPos();
    _y = yPos();
//     kdDebug(6040) << "_x " << _x << " _y " << _y << endl;
    width = 1;		// no override is indicated in boxes

    RenderBlock *cb = containingBlock();

    // Place caret outside the border
    if (flags & CFOutside) {

        RenderStyle *s = element() && element()->parent()
			&& element()->parent()->renderer()
			? element()->parent()->renderer()->style()
			: cb->style();

        const QFontMetrics &fm = s->fontMetrics();
        height = fm.height();

	bool rtl = s->direction() == RTL;
	bool outsideEnd = flags & CFOutsideEnd;

	if (outsideEnd) {
	    _x += this->width();
	} else {
	    _x--;
	}

	int hl = fm.leading() / 2;
	if (!isReplaced() || style()->display() == BLOCK) {
	    if (!outsideEnd ^ rtl)
	        _y -= hl;
	    else
	        _y += kMax(this->height() - fm.ascent() - hl, 0);
	} else {
	    _y += baselinePosition(false) - fm.ascent() - hl;
	}

    // Place caret inside the element
    } else {
        const QFontMetrics &fm = style()->fontMetrics();
        height = fm.height();

        RenderStyle *s = style();

        _x += borderLeft() + paddingLeft();
        _y += borderTop() + paddingTop();

        // ### regard direction
	switch (s->textAlign()) {
	case LEFT:
	case KHTML_LEFT:
	case TAAUTO:	// ### find out what this does
	case JUSTIFY:
	    break;
	case CENTER:
	case KHTML_CENTER:
	    _x += contentWidth() / 2;
	    break;
	case KHTML_RIGHT:
	case RIGHT:
	    _x += contentWidth();
	    break;
	}
    }

    int absx, absy;
    if (cb && cb != this && cb->absolutePosition(absx,absy)) {
//         kdDebug(6040) << "absx=" << absx << " absy=" << absy << endl;
        _x += absx;
        _y += absy;
    } else {
        // we don't know our absolute position, and there is no point returning
        // just a relative one
        _x = _y = -1;
    }
}

#undef DEBUG_LAYOUT
