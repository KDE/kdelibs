/*
 * This file is part of the render object implementation for KHTML.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 1999-2003 Antti Koivisto (koivisto@kde.org)
 *           (C) 2002-2003 Dirk Mueller (mueller@kde.org)
 *           (C) 2003 Apple Computer, Inc.
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

#include <kglobal.h>

#include "rendering/render_arena.h"
#include "rendering/render_inline.h"
#include "rendering/render_block.h"
#include "xml/dom_docimpl.h"

using namespace khtml;

void RenderInline::setStyle(RenderStyle* _style)
{
    RenderFlow::setStyle(_style);
    setInline(true);

    // Ensure that all of the split inlines pick up the new style. We
    // only do this if we're an inline, since we don't want to propagate
    // a block's style to the other inlines.
    // e.g., <font>foo <h4>goo</h4> moo</font>.  The <font> inlines before
    // and after the block share the same style, but the block doesn't
    // need to pass its style on to anyone else.
    RenderFlow* currCont = continuation();
    while (currCont) {
        if (currCont->isInline()) {
            RenderFlow* nextCont = currCont->continuation();
            currCont->setContinuation(0);
            currCont->setStyle(style());
            currCont->setContinuation(nextCont);
        }
        currCont = currCont->continuation();
    }

    // Update pseudos for :before and :after now.
    updatePseudoChild(RenderStyle::BEFORE, firstChild());
    updatePseudoChild(RenderStyle::AFTER, lastChild());
}

bool RenderInline::isInlineContinuation() const
{
    return m_isContinuation;
}

void RenderInline::addChildToFlow(RenderObject* newChild, RenderObject* beforeChild)
{
    setLayouted( false );

    // Make sure we don't append things after :after-generated content if we have it.
    if (!beforeChild && lastChild() && lastChild()->style()->styleType() == RenderStyle::AFTER)
        beforeChild = lastChild();

    if (!newChild->isText() && newChild->style()->position() != STATIC)
        setOverhangingContents();

    if (!newChild->isInline() && !newChild->isFloatingOrPositioned() )
    {
        // We are placing a block inside an inline. We have to perform a split of this
        // inline into continuations.  This involves creating an anonymous block box to hold
        // |newChild|.  We then make that block box a continuation of this inline.  We take all of
        // the children after |beforeChild| and put them in a clone of this object.
        RenderStyle *newStyle = new RenderStyle();
        newStyle->inheritFrom(style());
        newStyle->setDisplay(BLOCK);

        RenderBlock *newBox = new (renderArena()) RenderBlock(element()->getDocument() /* anonymous box */);
        newBox->setStyle(newStyle);
        RenderFlow* oldContinuation = continuation();
        setContinuation(newBox);

        // Someone may have put a <p> inside a <q>, causing a split.  When this happens, the :after content
        // has to move into the inline continuation.  Call updatePseudoChild to ensure that our :after
        // content gets properly destroyed.
        bool isLastChild = (beforeChild == lastChild());
        updatePseudoChild(RenderStyle::AFTER, lastChild());
        if (isLastChild && beforeChild != lastChild())
            beforeChild = 0; // We destroyed the last child, so now we need to update our insertion
                             // point to be 0.  It's just a straight append now.

        splitFlow(beforeChild, newBox, newChild, oldContinuation);
        return;
    }

    RenderBox::addChild(newChild,beforeChild);

    newChild->setLayouted( false );
    newChild->setMinMaxKnown( false );
}

RenderInline* RenderInline::cloneInline(RenderFlow* src)
{
    RenderInline *o = new (src->renderArena()) RenderInline(src->element());
    o->m_isContinuation = true;
    o->setStyle(src->style());
    return o;
}

void RenderInline::splitInlines(RenderBlock* fromBlock, RenderBlock* toBlock,
                                RenderBlock* middleBlock,
                                RenderObject* beforeChild, RenderFlow* oldCont)
{
    // Create a clone of this inline.
    RenderInline* clone = cloneInline(this);
    clone->setContinuation(oldCont);

    // Now take all of the children from beforeChild to the end and remove
    // then from |this| and place them in the clone.
    RenderObject* o = beforeChild;
    while (o) {
        RenderObject* tmp = o;
        o = tmp->nextSibling();
        clone->appendChildNode(removeChildNode(tmp));
        tmp->setLayouted(false);
        tmp->setMinMaxKnown(false);
    }

    // Hook |clone| up as the continuation of the middle block.
    middleBlock->setContinuation(clone);

    // We have been reparented and are now under the fromBlock.  We need
    // to walk up our inline parent chain until we hit the containing block.
    // Once we hit the containing block we're done.
    RenderFlow* curr = static_cast<RenderFlow*>(parent());
    RenderFlow* currChild = this;
    while (curr && curr != fromBlock) {
        // Create a new clone.
        RenderInline* cloneChild = clone;
        clone = cloneInline(curr);

        // Insert our child clone as the first child.
        clone->appendChildNode(cloneChild);

        // Hook the clone up as a continuation of |curr|.
        RenderFlow* oldCont = curr->continuation();
        curr->setContinuation(clone);
        clone->setContinuation(oldCont);

        // Now we need to take all of the children starting from the first child
        // *after* currChild and append them all to the clone.
        o = currChild->nextSibling();
        while (o) {
            RenderObject* tmp = o;
            o = tmp->nextSibling();
            clone->appendChildNode(curr->removeChildNode(tmp));
            tmp->setLayouted(false);
            tmp->setMinMaxKnown(false);
        }

        // Keep walking up the chain.
        currChild = curr;
        curr = static_cast<RenderFlow*>(curr->parent());
    }

    // Now we are at the block level. We need to put the clone into the toBlock.
    toBlock->appendChildNode(clone);

    // Now take all the children after currChild and remove them from the fromBlock
    // and put them in the toBlock.
    o = currChild->nextSibling();
    while (o) {
        RenderObject* tmp = o;
        o = tmp->nextSibling();
        toBlock->appendChildNode(fromBlock->removeChildNode(tmp));
    }
}

void RenderInline::splitFlow(RenderObject* beforeChild, RenderBlock* newBlockBox,
                             RenderObject* newChild, RenderFlow* oldCont)
{
    RenderBlock* pre = 0;
    RenderBlock* block = containingBlock();
    bool madeNewBeforeBlock = false;
    if (block->isAnonymous() && block->style()->display() == BLOCK) {
        // We can reuse this block and make it the preBlock of the next continuation.
        pre = block;
        block = block->containingBlock();
    }
    else {
        // No anonymous block available for use.  Make one.
        pre = block->createAnonymousBlock();
        madeNewBeforeBlock = true;
    }

    RenderBlock* post = block->createAnonymousBlock();

    RenderObject* boxFirst = madeNewBeforeBlock ? block->firstChild() : pre->nextSibling();
    if (madeNewBeforeBlock)
        block->insertChildNode(pre, boxFirst);
    block->insertChildNode(newBlockBox, boxFirst);
    block->insertChildNode(post, boxFirst);
    block->setChildrenInline(false);

    if (madeNewBeforeBlock) {
        RenderObject* o = boxFirst;
        while (o)
        {
            RenderObject* no = o;
            o = no->nextSibling();
            pre->appendChildNode(block->removeChildNode(no));
            no->setLayouted(false);
            no->setMinMaxKnown(false);
        }
    }

    splitInlines(pre, post, newBlockBox, beforeChild, oldCont);

    // We already know the newBlockBox isn't going to contain inline kids, so avoid wasting
    // time in makeChildrenNonInline by just setting this explicitly up front.
    newBlockBox->setChildrenInline(false);

    // We don't just call addChild, since it would pass things off to the
    // continuation, so we call addChildToFlow explicitly instead.  We delayed
    // adding the newChild until now so that the |newBlockBox| would be fully
    // connected, thus allowing newChild access to a renderArena should it need
    // to wrap itself in additional boxes (e.g., table construction).
    newBlockBox->addChildToFlow(newChild, 0);

    // XXXdwh is any of this even necessary? I don't think it is.
    pre->close();
    pre->setPos(0, -500000);
    pre->setLayouted(false);
    newBlockBox->close();
    newBlockBox->setPos(0, -500000);
    newBlockBox->setLayouted(false);
    post->close();
    post->setPos(0, -500000);
    post->setLayouted(false);

    block->setLayouted(false);
    block->setMinMaxKnown(false);
}

void RenderInline::paint(PaintInfo& i,
                      int _tx, int _ty)
{
#ifdef DEBUG_LAYOUT
    //    kdDebug( 6040 ) << renderName() << "(RenderInline) " << this << " ::paintObject() w/h = (" << width() << "/" << height() << ")" << endl;
#endif

    if ( i.phase == PaintActionElementBackground )
        return;

    // let the children their backgrounds
    PaintAction oldphase = i.phase;
    if ( i.phase == PaintActionChildBackgrounds )
        i.phase = PaintActionChildBackground;

    paintLineBoxBackgroundBorder(i, _tx, _ty);

    for( RenderObject *child = firstChild(); child; child = child->nextSibling())
        if(!child->layer() && !child->isFloating())
            child->paint(i, _tx, _ty);

    paintLineBoxDecorations(i, _tx, _ty);

    i.phase = oldphase;
}

void RenderInline::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(RenderInline)::calcMinMaxWidth() this=" << this << endl;
#endif

    // Irrelevant, since some enclosing block will actually measure us and our children.
    m_minWidth = 0;
    m_maxWidth = 0;

    setMinMaxKnown();
}

short RenderInline::width() const
{
    // Return the width of the minimal left side and the maximal right side.
    short leftSide = 0;
    short rightSide = 0;
    for (InlineRunBox* curr = firstLineBox(); curr; curr = curr->nextLineBox()) {
        if (curr == firstLineBox() || curr->xPos() < leftSide)
            leftSide = curr->xPos();
        if (curr == firstLineBox() || curr->xPos() + curr->width() > rightSide)
            rightSide = curr->xPos() + curr->width();
    }

    return rightSide - leftSide;
}

int RenderInline::height() const
{
    int h = 0;
    if (firstLineBox())
        h = lastLineBox()->yPos() + lastLineBox()->height() - firstLineBox()->yPos();
    return h;
}

int RenderInline::offsetLeft() const
{
    int x = RenderFlow::offsetLeft();
    if (firstLineBox())
        x += firstLineBox()->xPos();
    return x;
}

int RenderInline::offsetTop() const
{
    int y = RenderFlow::offsetTop();
    if (firstLineBox())
        y += firstLineBox()->yPos();
    return y;
}

const char *RenderInline::renderName() const
{
    if (isRelPositioned())
        return "RenderInline (relative positioned)";
    if (isAnonymous())
        return "RenderInline (anonymous)";
    return "RenderInline";
}

bool RenderInline::nodeAtPoint(NodeInfo& info, int _x, int _y, int _tx, int _ty, HitTestAction hitTestAction, bool inside)
{
    if ( hitTestAction != HitTestSelfOnly ) {
        for (RenderObject* child = lastChild(); child; child = child->previousSibling())
            if (!child->layer() && !child->isFloating() && child->nodeAtPoint(info, _x, _y, _tx, _ty, hitTestAction))
                inside = true;
    }


    // Check our line boxes if we're still not inside.
    if (hitTestAction != HitTestChildrenOnly && !inside && style()->visibility() != HIDDEN) {
        // See if we're inside one of our line boxes.
        for (InlineRunBox* curr = firstLineBox(); curr; curr = curr->nextLineBox()) {
            if((_y >=_ty + curr->m_y) && (_y < _ty + curr->m_y + curr->m_height) &&
               (_x >= _tx + curr->m_x) && (_x <_tx + curr->m_x + curr->m_width) ) {
                inside = true;
                break;
            }
        }
    }

    if (inside && element()) {
        if (info.innerNode() && info.innerNode()->renderer() &&
            !info.innerNode()->renderer()->isInline()) {
            // Within the same layer, inlines are ALWAYS fully above blocks.  Change inner node.
            info.setInnerNode(element());

            // Clear everything else.
            info.setInnerNonSharedNode(0);
            info.setURLElement(0);
        }

        if (!info.innerNode())
            info.setInnerNode(element());

        if(!info.innerNonSharedNode())
            info.setInnerNonSharedNode(element());
    }

    return inside;
}

void RenderInline::caretPos(int offset, int flags, int &_x, int &_y, int &width, int &height)
{
    _x = -1;

    RenderBlock *cb = containingBlock();
    bool rtl = cb->style()->direction() == RTL;
    bool outsideEnd = flags & CFOutsideEnd;
    // I need to explain that: outsideEnd contains a meaningful value if
    // and only if flags & CFOutside is set. If it is not, then randomly
    // either the first or the last line box is returned.
    // This doesn't matter because the only case this can happen is on an
    // empty inline element, whose first and last line boxes are actually
    // the same.
    InlineFlowBox *line = !outsideEnd ^ rtl ? firstLineBox() : lastLineBox();

    if (!line) {		// umpf, handle "gracefully"
        RenderFlow::caretPos(offset, flags, _x, _y, width, height);
        return;
    }

    _x = line->xPos();
    width = 1;		// ### regard CFOverride

    // Place caret outside the border
    if (flags & CFOutside) {
        RenderStyle *s = element() && element()->parent()
			&& element()->parent()->renderer()
			? element()->parent()->renderer()->style()
			: style();
        const QFontMetrics &fm = s->fontMetrics();
        _y = line->yPos() + line->baseline() - fm.ascent();
        height = fm.height();

	if (!outsideEnd ^ rtl) {
	    _x -= line->marginBorderPaddingLeft();
	} else {
	    _x += line->width() + line->marginBorderPaddingRight();
	}

    } else {
        const QFontMetrics &fm = style()->fontMetrics();
        _y = line->yPos() + line->baseline() - fm.ascent();
        height = fm.height();
    }

    int absx, absy;
    if (cb && cb->absolutePosition(absx,absy)) {
        //kdDebug(6040) << "absx=" << absx << " absy=" << absy << endl;
        _x += absx;
        _y += absy;
    } else {
        // we don't know our absolute position, and there is no point returning
        // just a relative one
        _x = _y = -1;
    }
}

