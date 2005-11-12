/*
 * This file is part of the render object implementation for KHTML.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 1999-2003 Antti Koivisto (koivisto@kde.org)
 *           (C) 2002-2003 Dirk Mueller (mueller@kde.org)
 *           (C) 2003 Apple Computer, Inc.
 *           (C) 2004 Germain Garand (germain@ebooksfrance.org)
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
 *
 */

//#define DEBUG
//#define DEBUG_LAYOUT
//#define BOX_DEBUG
//#define FLOAT_DEBUG
//#define PAGE_DEBUG

#include <kdebug.h>
#include "rendering/render_text.h"
#include "rendering/render_table.h"
#include "rendering/render_canvas.h"
#include "rendering/render_layer.h"
#include "rendering/render_block.h"

#include "xml/dom_nodeimpl.h"
#include "xml/dom_docimpl.h"
#include "html/html_formimpl.h"
#include "misc/htmltags.h"

#include "khtmlview.h"

using namespace DOM;

namespace khtml {

// -------------------------------------------------------------------------------------------------------

// Our MarginInfo state used when laying out block children.
RenderBlock::MarginInfo::MarginInfo(RenderBlock* block, int top, int bottom)
{
    // Whether or not we can collapse our own margins with our children.  We don't do this
    // if we had any border/padding (obviously), if we're the root or HTML elements, or if
    // we're positioned, floating, a table cell.
    m_canCollapseWithChildren = !block->isCanvas() && !block->isRoot() && !block->isPositioned() &&
        !block->isFloating() && !block->isTableCell() && !block->hasOverflowClip() && !block->isInlineBlockOrInlineTable();

    m_canCollapseTopWithChildren = m_canCollapseWithChildren && (top == 0) /*&& block->style()->marginTopCollapse() != MSEPARATE */;

    // If any height other than auto is specified in CSS, then we don't collapse our bottom
    // margins with our children's margins.  To do otherwise would be to risk odd visual
    // effects when the children overflow out of the parent block and yet still collapse
    // with it.  We also don't collapse if we have any bottom border/padding.
    m_canCollapseBottomWithChildren = m_canCollapseWithChildren && (bottom == 0) &&
        (block->style()->height().isVariable() && block->style()->height().value() == 0) /*&& block->style()->marginBottomCollapse() != MSEPARATE*/;

    m_quirkContainer = block->isTableCell() || block->isBody() /*|| block->style()->marginTopCollapse() == MDISCARD ||
        block->style()->marginBottomCollapse() == MDISCARD*/;

    m_atTopOfBlock = true;
    m_atBottomOfBlock = false;

    m_posMargin = m_canCollapseTopWithChildren ? block->maxTopMargin(true) : 0;
    m_negMargin = m_canCollapseTopWithChildren ? block->maxTopMargin(false) : 0;

    m_selfCollapsingBlockClearedFloat = false;

    m_topQuirk = m_bottomQuirk = m_determinedTopQuirk = false;
}

// -------------------------------------------------------------------------------------------------------

RenderBlock::RenderBlock(DOM::NodeImpl* node)
    : RenderFlow(node)
{
    m_childrenInline = true;
    m_floatingObjects = 0;
    m_positionedObjects = 0;
    m_pre = false;
    m_firstLine = false;
    m_avoidPageBreak = false;
    m_clearStatus = CNONE;
    m_maxTopPosMargin = m_maxTopNegMargin = m_maxBottomPosMargin = m_maxBottomNegMargin = 0;
    m_topMarginQuirk = m_bottomMarginQuirk = false;
    m_overflowHeight = 0;
    m_overflowWidth = 0;
}

RenderBlock::~RenderBlock()
{
    delete m_floatingObjects;
    delete m_positionedObjects;
}

void RenderBlock::setStyle(RenderStyle* _style)
{
    setReplaced(_style->isDisplayReplacedType());

    RenderFlow::setStyle(_style);

    m_pre = ( _style->whiteSpace() == PRE );

    // ### we could save this call when the change only affected
    // non inherited properties
    RenderObject *child = firstChild();
    while (child != 0)
    {
        if (child->isAnonymousBlock())
        {
            RenderStyle* newStyle = new RenderStyle();
            newStyle->inheritFrom(style());
            newStyle->setDisplay(BLOCK);
            child->setStyle(newStyle);
        }
        child = child->nextSibling();
    }

    // Update pseudos for :before and :after now.
    updatePseudoChildren();

    // handled by close() during parsing
    if (!document()->parsing())
        updateFirstLetter();
}

void RenderBlock::updateFirstLetter()
{
    // FIXME: We need to destroy the first-letter object if it is no longer the first child.  Need to find
    // an efficient way to check for that situation though before implementing anything.

    RenderStyle * pseudoStyle;
    if ( isTable() || !(pseudoStyle = style()->getPseudoStyle(RenderStyle::FIRST_LETTER)) )
        return;

    // Drill into inlines looking for our first text child.
    RenderObject* currChild = firstChild();
    while (currChild && currChild->needsLayout() && !currChild->isReplaced() && !currChild->isText())
        currChild = currChild->firstChild();

   if (currChild && currChild->isText() && !currChild->isBR()) {

        bool update = (currChild->parent()->style()->styleType() == RenderStyle::FIRST_LETTER);
        RenderObject* firstLetterContainer = update ? currChild->parent()->parent() : currChild->parent();
        RenderText* textObj = static_cast<RenderText*>(currChild);

        // Force inline display (except for floating first-letters)
        pseudoStyle->setDisplay( pseudoStyle->isFloating() ? BLOCK : INLINE);
        pseudoStyle->setPosition( STATIC ); // CSS2 says first-letter can't be positioned.

        if (update) {
            firstLetterContainer->firstChild()->setStyle( pseudoStyle );
            RenderStyle* newStyle = new RenderStyle();
            newStyle->inheritFrom( pseudoStyle );
            currChild->setStyle( newStyle );
            return;
        }

        RenderObject* firstLetter = RenderFlow::createFlow(element(), pseudoStyle, renderArena() );
        firstLetter->setIsAnonymous( true );
        firstLetterContainer->addChild(firstLetter, firstLetterContainer->firstChild());

        // The original string is going to be either a generated content string or a DOM node's
        // string.  We want the original string before it got transformed in case first-letter has
        // no text-transform or a different text-transform applied to it.
        DOMStringImpl* oldText = textObj->originalString();
        if (!oldText)
            oldText = textObj->string();

        if(oldText->l >= 1) {
            oldText->ref();
            unsigned int length = 0;
            while ( length < oldText->l &&
                    ( (oldText->s+length)->isSpace() || (oldText->s+length)->isPunct() ) )
                length++;
            if (!( (oldText->s+length)->isSpace() || (oldText->s+length)->isPunct() ))
                length++;
            RenderTextFragment* remainingText =
                new (renderArena()) RenderTextFragment(textObj->node(), oldText, length, oldText->l-length);
            remainingText->setIsAnonymous( textObj->isAnonymous() );
            remainingText->setStyle(textObj->style());
            if (remainingText->element())
                remainingText->element()->setRenderer(remainingText);

            RenderObject* nextObj = textObj->nextSibling();
            firstLetterContainer->removeChild(textObj);
            firstLetterContainer->addChild(remainingText, nextObj);

            RenderTextFragment* letter =
                new (renderArena()) RenderTextFragment(remainingText->node(), oldText, 0, length);
            letter->setIsAnonymous( remainingText->isAnonymous() );
            RenderStyle* newStyle = new RenderStyle();
            newStyle->inheritFrom(pseudoStyle);
            letter->setStyle(newStyle);
            firstLetter->addChild(letter);
            oldText->deref();
        }
        firstLetter->close();
    }
}

void RenderBlock::addChildToFlow(RenderObject* newChild, RenderObject* beforeChild)
{
    // Make sure we don't append things after :after-generated content if we have it.
    if ( !beforeChild && lastChild() && lastChild()->style()->styleType() == RenderStyle::AFTER )
        beforeChild = lastChild();

    bool madeBoxesNonInline = false;

    // If the requested beforeChild is not one of our children, then this is most likely because
    // there is an anonymous block box within this object that contains the beforeChild. So
    // just insert the child into the anonymous block box instead of here.
    if (beforeChild && beforeChild->parent() != this) {

        KHTMLAssert(beforeChild->parent());
        KHTMLAssert(beforeChild->parent()->isAnonymousBlock());

        if (newChild->isInline()) {
            beforeChild->parent()->addChild(newChild,beforeChild);
            return;
        }
        else if (beforeChild->parent()->firstChild() != beforeChild)
            return beforeChild->parent()->addChild(newChild, beforeChild);
        else
            return addChildToFlow(newChild, beforeChild->parent());
    }

    // prevent elements that haven't received a layout yet from getting painted by pushing
    // them far above the top of the page
    if (!newChild->isInline())
        newChild->setPos(newChild->xPos(), -500000);

    if (!newChild->isText() && newChild->style()->position() != STATIC)
        setOverhangingContents();

    // A block has to either have all of its children inline, or all of its children as blocks.
    // So, if our children are currently inline and a block child has to be inserted, we move all our
    // inline children into anonymous block boxes
    if ( m_childrenInline && !newChild->isInline() && !newChild->isFloatingOrPositioned() )
    {
        // This is a block with inline content. Wrap the inline content in anonymous blocks.
        makeChildrenNonInline(beforeChild);
        madeBoxesNonInline = true;

        if (beforeChild && beforeChild->parent() != this) {
            beforeChild = beforeChild->parent();
            KHTMLAssert(beforeChild->isAnonymousBlock());
            KHTMLAssert(beforeChild->parent() == this);
        }
    }
    else if (!m_childrenInline && !newChild->isFloatingOrPositioned())
    {
        // If we're inserting an inline child but all of our children are blocks, then we have to make sure
        // it is put into an anomyous block box. We try to use an existing anonymous box if possible, otherwise
        // a new one is created and inserted into our list of children in the appropriate position.
        if (newChild->isInline()) {
            if (beforeChild) {
                if ( beforeChild->previousSibling() && beforeChild->previousSibling()->isAnonymousBlock() ) {
                    beforeChild->previousSibling()->addChild(newChild);
                    return;
                }
            }
            else {
                if ( m_last && m_last->isAnonymousBlock() ) {
                    m_last->addChild(newChild);
                    return;
                }
            }

            // no suitable existing anonymous box - create a new one
            RenderBlock* newBox = createAnonymousBlock();
            RenderBox::addChild(newBox,beforeChild);
            newBox->addChild(newChild);
            newBox->setPos(newBox->xPos(), -500000);
            return;
        }
        else {
            // We are adding another block child... if the current last child is an anonymous box
            // then it needs to be closed.
            // ### get rid of the closing thing altogether this will only work during initial parsing
            if (lastChild() && lastChild()->isAnonymous()) {
                lastChild()->close();
            }
        }
    }

    RenderBox::addChild(newChild,beforeChild);
    // ### care about aligned stuff

    if ( madeBoxesNonInline )
        removeLeftoverAnonymousBoxes();
}

static void getInlineRun(RenderObject* start, RenderObject* stop,
                         RenderObject*& inlineRunStart,
                         RenderObject*& inlineRunEnd)
{
    // Beginning at |start| we find the largest contiguous run of inlines that
    // we can.  We denote the run with start and end points, |inlineRunStart|
    // and |inlineRunEnd|.  Note that these two values may be the same if
    // we encounter only one inline.
    //
    // We skip any non-inlines we encounter as long as we haven't found any
    // inlines yet.
    //
    // |stop| indicates a non-inclusive stop point.  Regardless of whether |stop|
    // is inline or not, we will not include it.  It's as though we encountered
    // a non-inline.
    inlineRunStart = inlineRunEnd = 0;

    // Start by skipping as many non-inlines as we can.
    RenderObject * curr = start;
    while (curr && !curr->isInline())
        curr = curr->nextSibling();

    if (!curr)
        return; // No more inline children to be found.

    inlineRunStart = inlineRunEnd = curr;

    curr = curr->nextSibling();
    while (curr && curr->isInline() && (curr != stop)) {
        inlineRunEnd = curr;
        curr = curr->nextSibling();
    }
}

void RenderBlock::makeChildrenNonInline(RenderObject *insertionPoint)
{
    // makeChildrenNonInline takes a block whose children are *all* inline and it
    // makes sure that inline children are coalesced under anonymous
    // blocks.  If |insertionPoint| is defined, then it represents the insertion point for
    // the new block child that is causing us to have to wrap all the inlines.  This
    // means that we cannot coalesce inlines before |insertionPoint| with inlines following
    // |insertionPoint|, because the new child is going to be inserted in between the inlines,
    // splitting them.
    KHTMLAssert(isReplacedBlock() || !isInline());
    KHTMLAssert(!insertionPoint || insertionPoint->parent() == this);

    m_childrenInline = false;

    RenderObject *child = firstChild();

    while (child) {
        RenderObject *inlineRunStart, *inlineRunEnd;
        getInlineRun(child, insertionPoint, inlineRunStart, inlineRunEnd);

        if (!inlineRunStart)
            break;

        child = inlineRunEnd->nextSibling();

        RenderBlock* box = createAnonymousBlock();
        insertChildNode(box, inlineRunStart);
        RenderObject* o = inlineRunStart;
        while(o != inlineRunEnd)
        {
            RenderObject* no = o;
            o = no->nextSibling();
            box->appendChildNode(removeChildNode(no));
        }
        box->appendChildNode(removeChildNode(inlineRunEnd));
        box->close();
        box->setPos(box->xPos(), -500000);
    }
}

void RenderBlock::makePageBreakAvoidBlocks()
{
    KHTMLAssert(!childrenInline());
    KHTMLAssert(canvas()->pagedMode());

    RenderObject *breakAfter = firstChild();
    RenderObject *breakBefore = breakAfter ? breakAfter->nextSibling() : 0;

    RenderBlock* pageRun = 0;

    // ### Should follow margin-collapsing rules, skipping self-collapsing blocks
    // and exporting page-breaks from first/last child when collapsing with parent margin.
    while (breakAfter) {
        if (breakAfter->isRenderBlock() && !breakAfter->childrenInline())
            static_cast<RenderBlock*>(breakAfter)->makePageBreakAvoidBlocks();
        EPageBreak pbafter = breakAfter->style()->pageBreakAfter();
        EPageBreak pbbefore = breakBefore ? breakBefore->style()->pageBreakBefore() : PBALWAYS;
        if ((pbafter == PBAVOID && pbbefore == PBAVOID) ||
            (pbafter == PBAVOID && pbbefore == PBAUTO) ||
            (pbafter == PBAUTO && pbbefore == PBAVOID))
        {
            if (!pageRun) {
                pageRun = createAnonymousBlock();
                pageRun->m_avoidPageBreak = true;
                pageRun->setChildrenInline(false);
            }
            pageRun->appendChildNode(removeChildNode(breakAfter));
        } else
        {
            if (pageRun) {
                pageRun->appendChildNode(removeChildNode(breakAfter));
                pageRun->close();
                insertChildNode(pageRun, breakBefore);
                pageRun = 0;
            }
        }
        breakAfter = breakBefore;
        breakBefore = breakBefore ? breakBefore->nextSibling() : 0;
    }

    // recurse into positioned block children as well.
    if (m_positionedObjects) {
        Q3PtrListIterator<RenderObject> it(*m_positionedObjects);
        for ( ; it.current(); ++it ) {
            if (it.current()->isRenderBlock() && !it.current()->childrenInline()) {
                static_cast<RenderBlock*>(it.current())->makePageBreakAvoidBlocks();
            }
        }
    }

    // recurse into floating block children.
    if (m_floatingObjects) {
        Q3PtrListIterator<FloatingObject> it(*m_floatingObjects);
        for ( ; it.current(); ++it ) {
            if (it.current()->node->isRenderBlock() && !it.current()->node->childrenInline()) {
                static_cast<RenderBlock*>(it.current()->node)->makePageBreakAvoidBlocks();
            }
        }
    }
}


void RenderBlock::removeChild(RenderObject *oldChild)
{
    // If this child is a block, and if our previous and next siblings are
    // both anonymous blocks with inline content, then we can go ahead and
    // fold the inline content back together.
    RenderObject* prev = oldChild->previousSibling();
    RenderObject* next = oldChild->nextSibling();
    bool mergedBlocks = false;
    if (document()->renderer() && !isInline() && !oldChild->isInline() && !oldChild->continuation() &&
        prev && prev->isAnonymousBlock() && prev->childrenInline() &&
        next && next->isAnonymousBlock() && next->childrenInline()) {
        // Take all the children out of the |next| block and put them in
        // the |prev| block.
        RenderObject* o = next->firstChild();
        while (o) {
            RenderObject* no = o;
            o = no->nextSibling();
            prev->appendChildNode(next->removeChildNode(no));
            no->setNeedsLayoutAndMinMaxRecalc();
        }
        prev->setNeedsLayoutAndMinMaxRecalc();

        // Nuke the now-empty block.
        next->detach();

        mergedBlocks = true;
    }

    RenderFlow::removeChild(oldChild);

    if (mergedBlocks && prev && !prev->previousSibling() && !prev->nextSibling()) {
        // The remerge has knocked us down to containing only a single anonymous
        // box.  We can go ahead and pull the content right back up into our
        // box.
        RenderObject* anonBlock = removeChildNode(prev);
        m_childrenInline = true;
        RenderObject* o = anonBlock->firstChild();
        while (o) {
            RenderObject* no = o;
            o = no->nextSibling();
            appendChildNode(anonBlock->removeChildNode(no));
            no->setNeedsLayoutAndMinMaxRecalc();
        }

        // Nuke the now-empty block.
        anonBlock->detach();
    }
}

bool RenderBlock::isSelfCollapsingBlock() const
{
    // We are not self-collapsing if we
    // (a) have a non-zero height according to layout (an optimization to avoid wasting time)
    // (b) are a table,
    // (c) have border/padding,
    // (d) have a min-height
    if (m_height > 0 ||
        isTable() || (borderBottom() + paddingBottom() + borderTop() + paddingTop()) != 0 ||
        style()->minHeight().value() > 0)
        return false;

    bool hasAutoHeight = style()->height().isVariable();
    if (style()->height().isPercent() && !style()->htmlHacks()) {
        hasAutoHeight = true;
        for (RenderBlock* cb = containingBlock(); !cb->isCanvas(); cb = cb->containingBlock()) {
            if (cb->style()->height().isFixed() || cb->isTableCell())
                hasAutoHeight = false;
        }
    }

    // If the height is 0 or auto, then whether or not we are a self-collapsing block depends
    // on whether we have content that is all self-collapsing or not.
    if (hasAutoHeight || ((style()->height().isFixed() || style()->height().isPercent()) && style()->height().value() == 0)) {
        // If the block has inline children, see if we generated any line boxes.  If we have any
        // line boxes, then we can't be self-collapsing, since we have content.
        if (childrenInline())
            return !firstLineBox();

        // Whether or not we collapse is dependent on whether all our normal flow children
        // are also self-collapsing.
        for (RenderObject* child = firstChild(); child; child = child->nextSibling()) {
            if (child->isFloatingOrPositioned())
                continue;
            if (!child->isSelfCollapsingBlock())
                return false;
        }
        return true;
    }
    return false;
}

void RenderBlock::layout()
{
    // Table cells call layoutBlock directly, so don't add any logic here.  Put code into
    // layoutBlock().
    layoutBlock(false);
}

void RenderBlock::layoutBlock(bool relayoutChildren)
{
    if (isInline() && !isReplacedBlock()) {
        setNeedsLayout(false);
        return;
    }
    //    kdDebug( 6040 ) << renderName() << " " << this << "::layoutBlock() start" << endl;
    //     QTime t;
    //     t.start();
    KHTMLAssert( needsLayout() );
    KHTMLAssert( minMaxKnown() );

    if (canvas()->pagedMode()) relayoutChildren = true;

    if (!relayoutChildren && posChildNeedsLayout() && !normalChildNeedsLayout() && !selfNeedsLayout()) {
        // All we have to is lay out our positioned objects.
        layoutPositionedObjects(relayoutChildren);
        if (hasOverflowClip())
            m_layer->checkScrollbarsAfterLayout();
        setNeedsLayout(false);
        return;
    }

    if (markedForRepaint()) {
        repaintDuringLayout();
        setMarkedForRepaint(false);
    }

    int oldWidth = m_width;

    calcWidth();
    m_overflowWidth = m_width;

    if ( oldWidth != m_width )
        relayoutChildren = true;

    //     kdDebug( 6040 ) << floatingObjects << "," << oldWidth << ","
    //                     << m_width << ","<< needsLayout() << "," << isAnonymousBox() << ","
    //                     << overhangingContents() << "," << isPositioned() << endl;

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(RenderBlock) " << this << " ::layout() width=" << m_width << ", needsLayout=" << needsLayout() << endl;
    if(containingBlock() == static_cast<RenderObject *>(this))
        kdDebug( 6040 ) << renderName() << ": containingBlock == this" << endl;
#endif

    clearFloats();

    m_height = 0;
    m_overflowHeight = 0;
    m_clearStatus = CNONE;

    // We use four values, maxTopPos, maxPosNeg, maxBottomPos, and maxBottomNeg, to track
    // our current maximal positive and negative margins.  These values are used when we
    // are collapsed with adjacent blocks, so for example, if you have block A and B
    // collapsing together, then you'd take the maximal positive margin from both A and B
    // and subtract it from the maximal negative margin from both A and B to get the
    // true collapsed margin.  This algorithm is recursive, so when we finish layout()
    // our block knows its current maximal positive/negative values.
    //
    // Start out by setting our margin values to our current margins.  Table cells have
    // no margins, so we don't fill in the values for table cells.
    if (!isTableCell()) {
        initMaxMarginValues();

        m_topMarginQuirk = style()->marginTop().isQuirk();
        m_bottomMarginQuirk = style()->marginBottom().isQuirk();

        if (element() && element()->id() == ID_FORM && static_cast<HTMLFormElementImpl*>(element())->isMalformed())
            // See if this form is malformed (i.e., unclosed). If so, don't give the form
            // a bottom margin.
            m_maxBottomPosMargin = m_maxBottomNegMargin = 0;
    }

    if (style()->scrollsOverflow() && m_layer) {
        // For overflow:scroll blocks, ensure we have both scrollbars in place always.
        if (style()->overflow() == OSCROLL) {
            m_layer->showScrollbar( Qt::Horizontal, true );
            m_layer->showScrollbar( Qt::Vertical, true );
        }

        // Move the scrollbars aside during layout.  The layer will move them back when it
        // does painting or event handling.
        m_layer->moveScrollbarsAside();
    }

    // A quirk that has become an unfortunate standard.  Positioned elements, floating elements
    // and table cells don't ever collapse their margins with either themselves or their
    // children.
    bool canCollapseOwnMargins = !isPositioned() && !isFloating() && !isTableCell();

    setContainsPageBreak(false);

    if (childrenInline())
        layoutInlineChildren( relayoutChildren );
    else
        layoutBlockChildren( relayoutChildren );

    // Expand our intrinsic height to encompass floats.
    int toAdd = borderBottom() + paddingBottom();
    if (m_layer && style()->scrollsOverflow() && style()->height().isVariable())
        toAdd += m_layer->horizontalScrollbarHeight();
    if ( hasOverhangingFloats() && (isFloatingOrPositioned() || flowAroundFloats()) )
        m_overflowHeight = m_height = floatBottom() + toAdd;

    int oldHeight = m_height;
    calcHeight();
    if (oldHeight != m_height) {
        relayoutChildren = true;

        // If the block got expanded in size, then increase our overflowheight to match.
        if (m_overflowHeight > m_height)
            m_overflowHeight -= (borderBottom()+paddingBottom());
        if (m_overflowHeight < m_height)
            m_overflowHeight = m_height;
    }

    if (isTableCell()) {
        // Table cells need to grow to accommodate both overhanging floats and
        // blocks that have overflowed content.
        // Check for an overhanging float first.
        // FIXME: This needs to look at the last flow, not the last child.
        if (lastChild() && lastChild()->hasOverhangingFloats() && !lastChild()->style()->hidesOverflow()) {
            KHTMLAssert(lastChild()->isRenderBlock());
            m_height = lastChild()->yPos() + static_cast<RenderBlock*>(lastChild())->floatBottom();
            m_height += borderBottom() + paddingBottom();
        }

        if (m_overflowHeight > m_height && !style()->hidesOverflow())
            m_height = m_overflowHeight + borderBottom() + paddingBottom();
    }

    if( hasOverhangingFloats() && ((isFloating() && style()->height().isVariable()) || isTableCell())) {
        m_height = floatBottom();
        m_height += borderBottom() + paddingBottom();
    }

    if (canvas()->pagedMode()) {
#ifdef PAGE_DEBUG
        kdDebug(6040) << renderName() << " Page Bottom: " << pageTopAfter(0) << endl;
        kdDebug(6040) << renderName() << " Bottom: " << m_height << endl;
#endif
        bool needsPageBreak = false;
        int xpage = crossesPageBreak(0, m_height);
        if (xpage) {
            needsPageBreak = true;
#ifdef PAGE_DEBUG
            kdDebug( 6040 ) << renderName() << " crosses to page " << xpage << endl;
#endif
        }
        if (needsPageBreak && !containsPageBreak()) {
            setNeedsPageClear(true);
#ifdef PAGE_DEBUG
            kdDebug( 6040 ) << renderName() << " marked for page-clear" << endl;
#endif
        }
    }

    layoutPositionedObjects( relayoutChildren );

    // Always ensure our overflow width/height are at least as large as our width/height.
    m_overflowWidth = qMax(m_overflowWidth, (int)m_width);
    m_overflowHeight = qMax(m_overflowHeight, m_height);

    if (canCollapseOwnMargins && m_height == 0) {
        // We are a block with no border and padding and a computed height
        // of 0.  The CSS spec states that zero-height blocks collapse their margins
        // together.
        // When blocks are self-collapsing, we just use the top margin values and set the
        // bottom margin max values to 0.  This way we don't factor in the values
        // twice when we collapse with our previous vertically adjacent and
        // following vertically adjacent blocks.
        if (m_maxBottomPosMargin > m_maxTopPosMargin)
            m_maxTopPosMargin = m_maxBottomPosMargin;
        if (m_maxBottomNegMargin > m_maxTopNegMargin)
            m_maxTopNegMargin = m_maxBottomNegMargin;
        m_maxBottomNegMargin = m_maxBottomPosMargin = 0;
    }

    // Update our scrollbars if we're overflow:auto/scroll now that we know if
    // we overflow or not.
    if (style()->hidesOverflow() && m_layer)
        m_layer->checkScrollbarsAfterLayout();

    setNeedsLayout(false);
}

void RenderBlock::adjustPositionedBlock(RenderObject* child, const MarginInfo& marginInfo)
{
    if (child->isBox() && child->hasStaticX()) {
        if (style()->direction() == LTR)
            static_cast<RenderBox*>(child)->setStaticX(borderLeft() + paddingLeft());
        else
            static_cast<RenderBox*>(child)->setStaticX(borderRight() + paddingRight());
    }

    if (child->isBox() && child->hasStaticY()) {
        int marginOffset = 0;
        if (!marginInfo.canCollapseWithTop()) {
            int collapsedTopPos = marginInfo.posMargin();
            int collapsedTopNeg = marginInfo.negMargin();
            bool posMargin = child->marginTop() >= 0;
            if (posMargin && child->marginTop() > collapsedTopPos)
                collapsedTopPos = child->marginTop();
            else if (!posMargin && child->marginTop() > collapsedTopNeg)
                collapsedTopNeg = child->marginTop();
            marginOffset += (collapsedTopPos - collapsedTopNeg) - child->marginTop();
        }

        static_cast<RenderBox*>(child)->setStaticY(m_height + marginOffset);
    }
}

void RenderBlock::adjustFloatingBlock(const MarginInfo& marginInfo)
{
    // The float should be positioned taking into account the bottom margin
    // of the previous flow.  We add that margin into the height, get the
    // float positioned properly, and then subtract the margin out of the
    // height again.  In the case of self-collapsing blocks, we always just
    // use the top margins, since the self-collapsing block collapsed its
    // own bottom margin into its top margin.
    //
    // Note also that the previous flow may collapse its margin into the top of
    // our block.  If this is the case, then we do not add the margin in to our
    // height when computing the position of the float.   This condition can be tested
    // for by simply calling canCollapseWithTop.  See
    // http://www.hixie.ch/tests/adhoc/css/box/block/margin-collapse/046.html for
    // an example of this scenario.
    int marginOffset = marginInfo.canCollapseWithTop() ? 0 : marginInfo.margin();
    m_height += marginOffset;
    positionNewFloats();
    m_height -= marginOffset;
}

RenderObject* RenderBlock::handleSpecialChild(RenderObject* child, const MarginInfo& marginInfo, CompactInfo& compactInfo, bool& handled)
{
    // Handle positioned children first.
    RenderObject* next = handlePositionedChild(child, marginInfo, handled);
    if (handled) return next;

    // Handle floating children next.
    next = handleFloatingChild(child, marginInfo, handled);
    if (handled) return next;

    // See if we have a compact element.  If we do, then try to tuck the compact element into the margin space of the next block.
    next = handleCompactChild(child, compactInfo, marginInfo, handled);
    if (handled) return next;

    // Finally, see if we have a run-in element.
    return handleRunInChild(child, handled);
}

RenderObject* RenderBlock::handlePositionedChild(RenderObject* child, const MarginInfo& marginInfo, bool& handled)
{
    if (child->isPositioned()) {
        handled = true;
        child->containingBlock()->insertPositionedObject(child);
        adjustPositionedBlock(child, marginInfo);
        return child->nextSibling();
    }
    return 0;
}

RenderObject* RenderBlock::handleFloatingChild(RenderObject* child, const MarginInfo& marginInfo, bool& handled)
{
    if (child->isFloating()) {
        handled = true;
        insertFloatingObject(child);
        adjustFloatingBlock(marginInfo);
        return child->nextSibling();
    }
    return 0;
}

static inline bool isAnonymousWhitespace( RenderObject* o ) {
    if (!o->isAnonymous())
        return false;
    RenderObject *fc = o->firstChild();
    return fc && fc == o->lastChild() && fc->isText() && static_cast<RenderText *>(fc)->stringLength() == 1 &&
           static_cast<RenderText *>(fc)->text()[0].unicode() == ' ';
}

RenderObject* RenderBlock::handleCompactChild(RenderObject* child, CompactInfo& compactInfo, const MarginInfo& marginInfo, bool& handled)
{
    // FIXME: We only deal with one compact at a time.  It is unclear what should be
    // done if multiple contiguous compacts are encountered.  For now we assume that
    // compact A followed by another compact B should simply be treated as block A.
    if (child->isCompact() && !compactInfo.compact() && (child->childrenInline() || child->isReplaced())) {
        // Get the next non-positioned/non-floating RenderBlock.
        RenderObject* next = child->nextSibling();
        RenderObject* curr = next;
        while (curr && (curr->isFloatingOrPositioned() || isAnonymousWhitespace(curr)) )
            curr = curr->nextSibling();
        if (curr && curr->isRenderBlock() && !curr->isCompact() && !curr->isRunIn()) {
            curr->calcWidth(); // So that horizontal margins are correct.
            // Need to compute margins for the child as though it is a block.
            child->style()->setDisplay(BLOCK);
            child->calcWidth();
            child->style()->setDisplay(COMPACT);

            int childMargins = child->marginLeft() + child->marginRight();
            int margin = style()->direction() == LTR ? curr->marginLeft() : curr->marginRight();
            if (margin >= (childMargins + child->maxWidth())) {
                // The compact will fit in the margin.
                handled = true;
                compactInfo.set(child, curr);
                child->layoutIfNeeded();
                int off = marginInfo.margin();
                m_height += off + curr->marginTop() < child->marginTop() ?
                            child->marginTop() - curr->marginTop() -off: 0;

                child->setPos(0,0); // This position will be updated to reflect the compact's
                                    // desired position and the line box for the compact will
                                    // pick that position up.
                return next;
            }
        }
    }
    return 0;
}

void RenderBlock::insertCompactIfNeeded(RenderObject* child, CompactInfo& compactInfo)
{
    if (compactInfo.matches(child)) {
        // We have a compact child to squeeze in.
        RenderObject* compactChild = compactInfo.compact();
        int compactXPos = borderLeft() + paddingLeft() + compactChild->marginLeft();
        if (style()->direction() == RTL) {
            compactChild->calcWidth(); // have to do this because of the capped maxwidth
            compactXPos = width() - borderRight() - paddingRight() -
                compactChild->width() - compactChild->marginRight();
        }

        int compactYPos = child->yPos() + child->borderTop() + child->paddingTop()
                            - compactChild->paddingTop() - compactChild->borderTop();
        int adj = 0;
        KHTMLAssert(child->isRenderBlock());
        InlineRunBox *b = static_cast<RenderBlock*>(child)->firstLineBox();
        InlineRunBox *c = static_cast<RenderBlock*>(compactChild)->firstLineBox();
        if (b && c) {
            // adjust our vertical position
            int vpos = compactChild->getVerticalPosition( true, child );
            if (vpos == PositionBottom)
                adj = b->height() > c->height() ? (b->height() + b->yPos() - c->height() - c->yPos()) : 0;
            else if (vpos == PositionTop)
                adj = b->yPos() - c->yPos();
            else
                adj = vpos;
            compactYPos += adj;
        }
        Length newLineHeight( qMax(compactChild->lineHeight(true)+adj, (int)child->lineHeight(true)), khtml::Fixed);
        child->style()->setLineHeight( newLineHeight );
        child->setNeedsLayout( true, false );
        child->layout();
        compactChild->setPos(compactXPos, compactYPos); // Set the x position.
        compactInfo.clear();
    }
}

RenderObject* RenderBlock::handleRunInChild(RenderObject* child, bool& handled)
{
    // See if we have a run-in element with inline children.  If the
    // children aren't inline, then just treat the run-in as a normal
    // block.
    if (child->isRunIn() && (child->childrenInline() || child->isReplaced())) {
        // Get the next non-positioned/non-floating RenderBlock.
        RenderObject* curr = child->nextSibling();
        while (curr && (curr->isFloatingOrPositioned() || isAnonymousWhitespace(curr)) )
            curr = curr->nextSibling();
        if (curr && (curr->isRenderBlock() && curr->childrenInline() && !curr->isCompact() && !curr->isRunIn())) {
            // The block acts like an inline, so just null out its
            // position.
            handled = true;
            child->setInline(true);
            child->setPos(0,0);

            // Remove the child.
            RenderObject* next = child->nextSibling();
            removeChildNode(child);

            // Now insert the child under |curr|.
            curr->insertChildNode(child, curr->firstChild());
            return next;
        }
    }
    return 0;
}

void RenderBlock::collapseMargins(RenderObject* child, MarginInfo& marginInfo, int yPosEstimate)
{
    // Get our max pos and neg top margins.
    int posTop = child->maxTopMargin(true);
    int negTop = child->maxTopMargin(false);

    // For self-collapsing blocks, collapse our bottom margins into our
    // top to get new posTop and negTop values.
    if (child->isSelfCollapsingBlock()) {
        posTop = qMax(posTop, (int)child->maxBottomMargin(true));
        negTop = qMax(negTop, (int)child->maxBottomMargin(false));
    }

    // See if the top margin is quirky. We only care if this child has
    // margins that will collapse with us.
    bool topQuirk = child->isTopMarginQuirk() /*|| style()->marginTopCollapse() == MDISCARD*/;

    if (marginInfo.canCollapseWithTop()) {
        // This child is collapsing with the top of the
        // block.  If it has larger margin values, then we need to update
        // our own maximal values.
        if (!style()->htmlHacks() || !marginInfo.quirkContainer() || !topQuirk) {
            m_maxTopPosMargin = qMax(posTop, (int)m_maxTopPosMargin);
            m_maxTopNegMargin = qMax(negTop, (int)m_maxTopNegMargin);
        }

        // The minute any of the margins involved isn't a quirk, don't
        // collapse it away, even if the margin is smaller (www.webreference.com
        // has an example of this, a <dt> with 0.8em author-specified inside
        // a <dl> inside a <td>.
        if (!marginInfo.determinedTopQuirk() && !topQuirk && (posTop-negTop)) {
            m_topMarginQuirk = false;
            marginInfo.setDeterminedTopQuirk(true);
        }

        if (!marginInfo.determinedTopQuirk() && topQuirk && marginTop() == 0)
            // We have no top margin and our top child has a quirky margin.
            // We will pick up this quirky margin and pass it through.
            // This deals with the <td><div><p> case.
            // Don't do this for a block that split two inlines though.  You do
            // still apply margins in this case.
            m_topMarginQuirk = true;
    }

    if (marginInfo.quirkContainer() && marginInfo.atTopOfBlock() && (posTop - negTop))
        marginInfo.setTopQuirk(topQuirk);

    int ypos = m_height;
    if (child->isSelfCollapsingBlock()) {
        // This child has no height.  We need to compute our
        // position before we collapse the child's margins together,
        // so that we can get an accurate position for the zero-height block.
        int collapsedTopPos = qMax(marginInfo.posMargin(), (int)child->maxTopMargin(true));
        int collapsedTopNeg = qMax(marginInfo.negMargin(), (int)child->maxTopMargin(false));
        marginInfo.setMargin(collapsedTopPos, collapsedTopNeg);

        // Now collapse the child's margins together, which means examining our
        // bottom margin values as well.
        marginInfo.setPosMarginIfLarger(child->maxBottomMargin(true));
        marginInfo.setNegMarginIfLarger(child->maxBottomMargin(false));

        if (!marginInfo.canCollapseWithTop())
            // We need to make sure that the position of the self-collapsing block
            // is correct, since it could have overflowing content
            // that needs to be positioned correctly (e.g., a block that
            // had a specified height of 0 but that actually had subcontent).
            ypos = m_height + collapsedTopPos - collapsedTopNeg;
    }
    else {
#ifdef APPLE_CHANGES
        if (child->style()->marginTopCollapse() == MSEPARATE) {
            m_height += marginInfo.margin() + child->marginTop();
            ypos = m_height;
        }
        else
#endif
        if (!marginInfo.atTopOfBlock() ||
            (!marginInfo.canCollapseTopWithChildren()
             && (!style()->htmlHacks() || !marginInfo.quirkContainer() || !marginInfo.topQuirk()))) {
            // We're collapsing with a previous sibling's margins and not
            // with the top of the block.
            m_height += qMax(marginInfo.posMargin(), posTop) - qMax(marginInfo.negMargin(), negTop);
            ypos = m_height;
        }

        marginInfo.setPosMargin(child->maxBottomMargin(true));
        marginInfo.setNegMargin(child->maxBottomMargin(false));

        if (marginInfo.margin())
            marginInfo.setBottomQuirk(child->isBottomMarginQuirk() /*|| style()->marginBottomCollapse() == MDISCARD*/);

        marginInfo.setSelfCollapsingBlockClearedFloat(false);
    }

    child->setPos(child->xPos(), ypos);
    if (ypos != yPosEstimate) {
        if (child->style()->width().isPercent() && child->usesLineWidth())
            // The child's width is a percentage of the line width.
            // When the child shifts to clear an item, its width can
            // change (because it has more available line width).
            // So go ahead and mark the item as dirty.
            child->setChildNeedsLayout(true);

        if (!child->flowAroundFloats() || child->hasFloats())
            child->markAllDescendantsWithFloatsForLayout();

        // Our guess was wrong. Make the child lay itself out again.
        child->layoutIfNeeded();
    }
}

void RenderBlock::clearFloatsIfNeeded(RenderObject* child, MarginInfo& marginInfo, int oldTopPosMargin, int oldTopNegMargin)
{
    int heightIncrease = getClearDelta(child);
    if (heightIncrease) {
        // The child needs to be lowered.  Move the child so that it just clears the float.
        child->setPos(child->xPos(), child->yPos() + heightIncrease);

        // Increase our height by the amount we had to clear.
        if (!child->isSelfCollapsingBlock())
            m_height += heightIncrease;
        else {
            // For self-collapsing blocks that clear, they may end up collapsing
            // into the bottom of the parent block.  We simulate this behavior by
            // setting our positive margin value to compensate for the clear.
            marginInfo.setPosMargin(qMax(0, child->yPos() - m_height));
            marginInfo.setNegMargin(0);
            marginInfo.setSelfCollapsingBlockClearedFloat(true);
        }

        if (marginInfo.canCollapseWithTop()) {
            // We can no longer collapse with the top of the block since a clear
            // occurred.  The empty blocks collapse into the cleared block.
            // FIXME: This isn't quite correct.  Need clarification for what to do
            // if the height the cleared block is offset by is smaller than the
            // margins involved.
            m_maxTopPosMargin = oldTopPosMargin;
            m_maxTopNegMargin = oldTopNegMargin;
            marginInfo.setAtTopOfBlock(false);
        }

        // If our value of clear caused us to be repositioned vertically to be
        // underneath a float, we might have to do another layout to take into account
        // the extra space we now have available.
        if (!child->style()->width().isFixed()  && child->usesLineWidth())
            // The child's width is a percentage of the line width.
            // When the child shifts to clear an item, its width can
            // change (because it has more available line width).
            // So go ahead and mark the item as dirty.
            child->setChildNeedsLayout(true);
        if (child->hasFloats())
            child->markAllDescendantsWithFloatsForLayout();
        child->layoutIfNeeded();
    }
}

bool RenderBlock::canClear(RenderObject *child, PageBreakLevel level)
{
    KHTMLAssert(child->parent() && child->parent() == this);
    KHTMLAssert(canvas()->pagedMode());

    // Positioned elements cannot be moved. Only normal flow and floating.
    if (child->isPositioned() || child->isRelPositioned()) return false;

    switch(level) {
        case PageBreakNormal:
            // check page-break-inside: avoid
            if (!style()->pageBreakInside())
                // we cannot, but can our parent?
                if(!parent()->canClear(this, level)) return false;
        case PageBreakHarder:
            // check page-break-after/before: avoid
            if (m_avoidPageBreak)
                // we cannot, but can our parent?
                if(!parent()->canClear(this, level)) return false;
        case PageBreakForced:
            // child is larger than page-height and is forced to break
            if(child->height() > canvas()->pageHeight()) return false;
            return true;
    }
    assert(false);
    return false;
}

void RenderBlock::clearPageBreak(RenderObject* child, int pageBottom)
{
    KHTMLAssert(child->parent() && child->parent() == this);
    KHTMLAssert(canvas()->pagedMode());

    if (child->yPos() >= pageBottom) return;

    int heightIncrease = 0;

    heightIncrease = pageBottom - child->yPos();

    // ### should never happen, canClear should have been called to detect it.
    if (child->height() > canvas()->pageHeight()) {
        kdDebug(6040) << "### child is too large to clear: " << child->height() << " > " << canvas()->pageHeight() << endl;
        return;
    }

    // The child needs to be lowered.  Move the child so that it just clears the break.
    child->setPos(child->xPos(), pageBottom);

#ifdef PAGE_DEBUG
    kdDebug(6040) << "Cleared block " << heightIncrease << "px" << endl;
#endif

    // Increase our height by the amount we had to clear.
    m_height += heightIncrease;

    // We might have to do another layout to take into account
    // the extra space we now have available.
    if (!child->style()->width().isFixed()  && child->usesLineWidth())
        // The child's width is a percentage of the line width.
        // When the child shifts to clear a page-break, its width can
        // change (because it has more available line width).
        // So go ahead and mark the item as dirty.
        child->setChildNeedsLayout(true);
    if (child->hasFloats())
        child->markAllDescendantsWithFloatsForLayout();
    if (child->containsPageBreak())
        child->setNeedsLayout(true);
    child->layoutIfNeeded();

    child->setAfterPageBreak(true);
}

int RenderBlock::estimateVerticalPosition(RenderObject* child, const MarginInfo& marginInfo)
{
    // FIXME: We need to eliminate the estimation of vertical position, because
    // when it's wrong we sometimes trigger a pathological relayout if there are
    // intruding floats.
    int yPosEstimate = m_height;
    if (!marginInfo.canCollapseWithTop()) {
        int childMarginTop = child->selfNeedsLayout() ? child->marginTop() : child->collapsedMarginTop();
        yPosEstimate += qMax(marginInfo.margin(), childMarginTop);
    }
    return yPosEstimate;
}

void RenderBlock::determineHorizontalPosition(RenderObject* child)
{
    if (style()->direction() == LTR) {
        int xPos = borderLeft() + paddingLeft();

        // Add in our left margin.
        int chPos = xPos + child->marginLeft();

        // Some objects (e.g., tables, horizontal rules, overflow:auto blocks) avoid floats.  They need
        // to shift over as necessary to dodge any floats that might get in the way.
        if (child->flowAroundFloats()) {
            int leftOff = leftOffset(m_height);
            if (style()->textAlign() != KHTML_CENTER && !child->style()->marginLeft().isVariable()) {
                if (child->marginLeft() < 0)
                    leftOff += child->marginLeft();
                chPos = qMax(chPos, leftOff); // Let the float sit in the child's margin if it can fit.
            }
            else if (leftOff != xPos) {
                // The object is shifting right. The object might be centered, so we need to
                // recalculate our horizontal margins. Note that the containing block content
                // width computation will take into account the delta between |leftOff| and |xPos|
                // so that we can just pass the content width in directly to the |calcHorizontalMargins|
                // function.
                static_cast<RenderBox*>(child)->calcHorizontalMargins(child->style()->marginLeft(), child->style()->marginRight(), lineWidth(child->yPos()));
                chPos = leftOff + child->marginLeft();
            }
        }

        child->setPos(chPos, child->yPos());
    } else {
        int xPos = m_width - borderRight() - paddingRight();
        if (m_layer && style()->scrollsOverflow())
            xPos -= m_layer->verticalScrollbarWidth();
        int chPos = xPos - (child->width() + child->marginRight());
        if (child->flowAroundFloats()) {
            int rightOff = rightOffset(m_height);
            if (style()->textAlign() != KHTML_CENTER && !child->style()->marginRight().isVariable()) {
                if (child->marginRight() < 0)
                    rightOff -= child->marginRight();
                chPos = qMin(chPos, rightOff - child->width()); // Let the float sit in the child's margin if it can fit.
            } else if (rightOff != xPos) {
                // The object is shifting left. The object might be centered, so we need to
                // recalculate our horizontal margins. Note that the containing block content
                // width computation will take into account the delta between |rightOff| and |xPos|
                // so that we can just pass the content width in directly to the |calcHorizontalMargins|
                // function.
                static_cast<RenderBox*>(child)->calcHorizontalMargins(child->style()->marginLeft(), child->style()->marginRight(), lineWidth(child->yPos()));
                chPos = rightOff - child->marginRight() - child->width();
            }
        }
        child->setPos(chPos, child->yPos());
    }
}

void RenderBlock::setCollapsedBottomMargin(const MarginInfo& marginInfo)
{
    if (marginInfo.canCollapseWithBottom() && !marginInfo.canCollapseWithTop()) {
        // Update our max pos/neg bottom margins, since we collapsed our bottom margins
        // with our children.
        m_maxBottomPosMargin = qMax((int)m_maxBottomPosMargin, marginInfo.posMargin());
        m_maxBottomNegMargin = qMax((int)m_maxBottomNegMargin, marginInfo.negMargin());

        if (!marginInfo.bottomQuirk())
            m_bottomMarginQuirk = false;

        if (marginInfo.bottomQuirk() && marginBottom() == 0)
            // We have no bottom margin and our last child has a quirky margin.
            // We will pick up this quirky margin and pass it through.
            // This deals with the <td><div><p> case.
            m_bottomMarginQuirk = true;
    }
}

void RenderBlock::handleBottomOfBlock(int top, int bottom, MarginInfo& marginInfo)
{
    // If our last flow was a self-collapsing block that cleared a float, then we don't
    // collapse it with the bottom of the block.
    if (!marginInfo.selfCollapsingBlockClearedFloat())
        marginInfo.setAtBottomOfBlock(true);

    // If we can't collapse with children then go ahead and add in the bottom margin.
    if (!marginInfo.canCollapseWithBottom() && !marginInfo.canCollapseWithTop()
        && (!style()->htmlHacks() || !marginInfo.quirkContainer() || !marginInfo.bottomQuirk()))
        m_height += marginInfo.margin();

    // Now add in our bottom border/padding.
    m_height += bottom;

    // Negative margins can cause our height to shrink below our minimal height (border/padding).
    // If this happens, ensure that the computed height is increased to the minimal height.
    m_height = qMax(m_height, top + bottom);

    // Always make sure our overflow height is at least our height.
    m_overflowHeight = qMax(m_height, m_overflowHeight);

    // Update our bottom collapsed margin info.
    setCollapsedBottomMargin(marginInfo);
}

void RenderBlock::layoutBlockChildren( bool relayoutChildren )
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << " layoutBlockChildren( " << this <<" ), relayoutChildren="<< relayoutChildren << endl;
#endif

    int top = borderTop() + paddingTop();
    int bottom = borderBottom() + paddingBottom();
    if (m_layer && style()->scrollsOverflow())
        bottom += m_layer->horizontalScrollbarHeight();

    m_height = m_overflowHeight = top;

    // The margin struct caches all our current margin collapsing state.
    // The compact struct caches state when we encounter compacts.
    MarginInfo marginInfo(this, top, bottom);
    CompactInfo compactInfo;

    // Fieldsets need to find their legend and position it inside the border of the object.
    // The legend then gets skipped during normal layout.
    RenderObject* legend = layoutLegend(relayoutChildren);

    PageBreakInfo pageBreakInfo(pageTopAfter(0));

    RenderObject* child = firstChild();
    while( child != 0 )
    {
        if (legend == child) {
            child = child->nextSibling();
            continue; // Skip the legend, since it has already been positioned up in the fieldset's border.
        }

        int oldTopPosMargin = m_maxTopPosMargin;
        int oldTopNegMargin = m_maxTopNegMargin;

        // make sure we relayout children if we need it.
        if (relayoutChildren ||
            (child->isReplaced() && (child->style()->width().isPercent() || child->style()->height().isPercent())) ||
            (child->isRenderBlock() && child->style()->height().isPercent()))
            child->setChildNeedsLayout(true);

        // Handle the four types of special elements first.  These include positioned content, floating content, compacts and
        // run-ins.  When we encounter these four types of objects, we don't actually lay them out as normal flow blocks.
        bool handled = false;
        RenderObject* next = handleSpecialChild(child, marginInfo, compactInfo, handled);
        if (handled) { child = next; continue; }

        // The child is a normal flow object.  Compute its vertical margins now.
        child->calcVerticalMargins();

#ifdef APPLE_CHANGES /* margin-*-collapse not merged yet */
        // Do not allow a collapse if the margin top collapse style is set to SEPARATE.
        if (child->style()->marginTopCollapse() == MSEPARATE) {
            marginInfo.setAtTopOfBlock(false);
            marginInfo.clearMargin();
        }
#endif

        // Try to guess our correct y position.  In most cases this guess will
        // be correct.  Only if we're wrong (when we compute the real y position)
        // will we have to potentially relayout.
        int yPosEstimate = estimateVerticalPosition(child, marginInfo);

        // If an element might be affected by the presence of floats, then always mark it for
        // layout.
        if ( !child->flowAroundFloats() || child->usesLineWidth() ) {
            int fb = floatBottom();
            if (fb > m_height || fb > yPosEstimate)
                child->setChildNeedsLayout(true);
        }

        // Go ahead and position the child as though it didn't collapse with the top.
        child->setPos(child->xPos(), yPosEstimate);
        child->layoutIfNeeded();

        // Now determine the correct ypos based on examination of collapsing margin
        // values.
        collapseMargins(child, marginInfo, yPosEstimate);

        // Now check for clear.
        clearFloatsIfNeeded(child, marginInfo, oldTopPosMargin, oldTopNegMargin);

        // We are no longer at the top of the block if we encounter a non-empty child.
        // This has to be done after checking for clear, so that margins can be reset if a clear occurred.
        if (marginInfo.atTopOfBlock() && !child->isSelfCollapsingBlock())
            marginInfo.setAtTopOfBlock(false);

        // Now place the child in the correct horizontal position
        determineHorizontalPosition(child);

        // Update our height now that the child has been placed in the correct position.
        m_height += child->height();

#ifdef APPLE_CHANGES
        if (child->style()->marginBottomCollapse() == MSEPARATE) {
            m_height += child->marginBottom();
            marginInfo.clearMargin();
        }
#endif

        // Check for page-breaks
        if (canvas()->pagedMode())
            clearChildOfPageBreaks(child, pageBreakInfo, marginInfo);

        if (child->hasOverhangingFloats() && !child->flowAroundFloats()) {
            // need to add the child's floats to our floating objects list, but not in the case where
            // overflow is auto/scroll
            addOverHangingFloats( static_cast<RenderBlock *>(child), -child->xPos(), -child->yPos(), true );
        }

        // See if this child has made our overflow need to grow.
        // ### --dwh Work with left overflow as well as right overflow.
        int overflowDelta = - child->height() ;
        if ( child->isBlockFlow () && !child->isTable() && child->style()->hidesOverflow() )
            overflowDelta += child->height();
        else
            overflowDelta += child->overflowHeight();

        // See if this child has made our overflow need to grow.
        int rightChildPos = child->xPos() + qMax(child->effectiveWidth(), (int)child->width());
        if (child->isRelPositioned()) {
            // CSS 2.1-9.4.3 - allow access to relatively positioned content
            // ### left overflow support
            int xoff = 0, yoff = 0;
            static_cast<RenderBox*>(child)->relativePositionOffset(xoff, yoff);
            if (xoff>0)
               rightChildPos += xoff;
            if (yoff>0)
               overflowDelta += yoff;
        }

        m_overflowHeight = qMax(m_height + overflowDelta, m_overflowHeight);
        m_overflowWidth = qMax(rightChildPos, m_overflowWidth);

        // Insert our compact into the block margin if we have one.
        insertCompactIfNeeded(child, compactInfo);

        child = child->nextSibling();
    }

    // The last child had forced page-break-after
    if (pageBreakInfo.forcePageBreak())
        m_height = pageBreakInfo.pageBottom();

    // Now do the handling of the bottom of the block, adding in our bottom border/padding and
    // determining the correct collapsed bottom margin information.
    handleBottomOfBlock(top, bottom, marginInfo);

    setNeedsLayout(false);
}

void RenderBlock::clearChildOfPageBreaks(RenderObject *child, PageBreakInfo &pageBreakInfo, MarginInfo &marginInfo)
{
    int childTop = child->yPos();
    int childBottom = child->yPos()+child->height();
#ifdef PAGE_DEBUG
    kdDebug(6040) << renderName() << " ChildTop: " << childTop << " ChildBottom: " << childBottom << endl;
#endif

    bool forcePageBreak = pageBreakInfo.forcePageBreak() || child->style()->pageBreakBefore() == PBALWAYS;
#ifdef PAGE_DEBUG
    if (forcePageBreak)
        kdDebug(6040) << renderName() << "Forced break required" << endl;
#endif

    int xpage = crossesPageBreak(childTop, childBottom);
    if (xpage || forcePageBreak)
    {
        if (!forcePageBreak && child->containsPageBreak() && !child->needsPageClear()) {
#ifdef PAGE_DEBUG
            kdDebug(6040) << renderName() << " Child contains page-break to page " << xpage << endl;
#endif
            // ### Actually this assumes floating children are breaking/clearing
            // nicely as well.
            setContainsPageBreak(true);
        }
        else {
            bool doBreak = true;
            // don't break before the first child or when page-break-inside is avoid
            if (!forcePageBreak && (!style()->pageBreakInside() || m_avoidPageBreak || child == firstChild())) {
                if (parent()->canClear(this, (m_avoidPageBreak) ? PageBreakHarder : PageBreakNormal )) {
#ifdef PAGE_DEBUG
                    kdDebug(6040) << renderName() << "Avoid page-break inside" << endl;
#endif
                    child->setNeedsPageClear(false);
                    setNeedsPageClear(true);
                    doBreak = false;
                }
#ifdef PAGE_DEBUG
                else
                    kdDebug(6040) << renderName() << "Ignoring page-break avoid" << endl;
#endif
            }
            if (doBreak) {
#ifdef PAGE_DEBUG
                kdDebug(6040) << renderName() << " Clearing child of page-break" << endl;
                kdDebug(6040) << renderName() << " child top of page " << xpage << endl;
#endif
                clearPageBreak(child, pageBreakInfo.pageBottom());
                child->setNeedsPageClear(false);
                setContainsPageBreak(true);
            }
        }
        pageBreakInfo.setPageBottom(pageBreakInfo.pageBottom() + canvas()->pageHeight());
    }
    else
    if (child->yPos() >= pageBreakInfo.pageBottom()) {
        bool doBreak = true;
#ifdef PAGE_DEBUG
        kdDebug(6040) << "Page-break between children" << endl;
#endif
        if (!style()->pageBreakInside() || m_avoidPageBreak) {
            if(parent()->canClear(this, (m_avoidPageBreak) ? PageBreakHarder : PageBreakNormal )) {
#ifdef PAGE_DEBUG
                kdDebug(6040) << "Avoid page-break inside" << endl;
#endif
                child->setNeedsPageClear(false);
                setNeedsPageClear(true);
                doBreak = false;
            }
#ifdef PAGE_DEBUG
            else
                kdDebug(6040) << "Ignoring page-break avoid" << endl;
#endif
        }
        if (doBreak) {
            // Break between children
            setContainsPageBreak(true);
            // ### Should collapse top-margin with page-margin
        }
        pageBreakInfo.setPageBottom(pageBreakInfo.pageBottom() + canvas()->pageHeight());
    }

    // Do we need a forced page-break before next child?
    pageBreakInfo.setForcePageBreak(false);
    if (child->style()->pageBreakAfter() == PBALWAYS)
        pageBreakInfo.setForcePageBreak(true);
}

void RenderBlock::layoutPositionedObjects(bool relayoutChildren)
{
    if (m_positionedObjects) {
        //kdDebug( 6040 ) << renderName() << " " << this << "::layoutPositionedObjects() start" << endl;
        RenderObject* r;
        Q3PtrListIterator<RenderObject> it(*m_positionedObjects);
        bool adjOverflow = !(style()->position() == FIXED) && style()->hidesOverflow();
        for ( ; (r = it.current()); ++it ) {
            //kdDebug(6040) << "   have a positioned object" << endl;
            if (r->markedForRepaint()) {
                r->repaintDuringLayout();
                r->setMarkedForRepaint(false);
            }
            if ( relayoutChildren || (r->hasStaticY() && r->parent() != this && r->parent()->isBlockFlow()) )
                r->setChildNeedsLayout(true);
            r->layoutIfNeeded();
            if (adjOverflow && r->style()->position() == ABSOLUTE) {
                if (r->xPos() + r->effectiveWidth() > m_overflowWidth)
                    m_overflowWidth = r->xPos() + r->effectiveWidth();
                if (r->yPos() + r->effectiveHeight() > m_overflowHeight)
                    m_overflowHeight = r->yPos() + r->effectiveHeight();
            }
        }
    }
}

void RenderBlock::paint(PaintInfo& pI, int _tx, int _ty)
{
    _tx += m_x;
    _ty += m_y;

    // check if we need to do anything at all...
    if (!isRoot() && !isInlineFlow() && !overhangingContents() && !isRelPositioned() && !isPositioned() )
    {
        int h = m_overflowHeight;
        int yPos = _ty;
        if (m_floatingObjects && floatBottom() > h)
            h = floatBottom();

        // Sanity check the first line
        // to see if it extended a little above our box. Overflow out the bottom is already handled via
        // overflowHeight(), so we don't need to check that.
        if (m_firstLineBox && m_firstLineBox->topOverflow() < 0)
            yPos += m_firstLineBox->topOverflow();

        int os = 2*maximalOutlineSize(pI.phase);
        if( (yPos > pI.r.bottom() + os) || (_ty + h <= pI.r.y() - os))
            return;
    }

    paintObject(pI, _tx, _ty);
}

void RenderBlock::paintObject(PaintInfo& pI, int _tx, int _ty, bool shouldPaintOutline)
{
#ifdef DEBUG_LAYOUT
   //kdDebug( 6040 ) << renderName() << "(RenderBlock) " << this << " ::paintObject() w/h = (" << width() << "/" << height() << ")" << endl;
#endif

    // If we're a repositioned run-in, don't paint background/borders.
    bool inlineFlow = isInlineFlow();

    // 1. paint background, borders etc
    if (!inlineFlow &&
        (pI.phase == PaintActionElementBackground || pI.phase == PaintActionChildBackground ) &&
         shouldPaintBackgroundOrBorder() && style()->visibility() == VISIBLE)
        paintBoxDecorations(pI, _tx, _ty);

    if ( pI.phase == PaintActionElementBackground )
        return;
    if ( pI.phase == PaintActionChildBackgrounds )
        pI.phase = PaintActionChildBackground;

    paintLineBoxBackgroundBorder(pI, _tx, _ty);

    // 2. paint contents
    int scrolledX = _tx;
    int scrolledY = _ty;
    int _y = pI.r.y();
    int _h = pI.r.height();
    if (style()->hidesOverflow() && m_layer)
        m_layer->subtractScrollOffset(scrolledX, scrolledY);

    for(RenderObject *child = firstChild(); child; child = child->nextSibling()) {
        if(!child->layer() && !child->isFloating())
            child->paint(pI, scrolledX, scrolledY);
    }

    paintLineBoxDecorations(pI, scrolledX, scrolledY);

    // 3. paint floats.
    if (!inlineFlow && (pI.phase == PaintActionFloat || pI.phase == PaintActionSelection))
        paintFloats(pI, scrolledX, scrolledY, pI.phase == PaintActionSelection);

    // 4. paint outline.
    if (shouldPaintOutline && !inlineFlow && pI.phase == PaintActionOutline &&
        style()->outlineWidth() && style()->visibility() == VISIBLE)
        paintOutline(pI.p, _tx, _ty, width(), height(), style());

#ifdef BOX_DEBUG
    if ( style() && style()->visibility() == VISIBLE ) {
        if(isAnonymous())
            outlineBox(pI.p, _tx, _ty, "green");
        if(isFloating())
            outlineBox(pI.p, _tx, _ty, "yellow");
        else
            outlineBox(pI.p, _tx, _ty);
    }
#endif
}

void RenderBlock::paintFloats(PaintInfo& pI, int _tx, int _ty, bool paintSelection)
{
    if (!m_floatingObjects)
        return;

    FloatingObject* r;
    Q3PtrListIterator<FloatingObject> it(*m_floatingObjects);
    for ( ; (r = it.current()); ++it) {
        // Only paint the object if our noPaint flag isn't set.
        if (r->node->isFloating() && !r->noPaint && !r->node->layer()) {
            PaintAction oldphase = pI.phase;
            if (paintSelection) {
                pI.phase = PaintActionSelection;
                r->node->paint(pI, _tx + r->left - r->node->xPos() + r->node->marginLeft(),
                               _ty + r->startY - r->node->yPos() + r->node->marginTop());
            }
            else {
                pI.phase = PaintActionElementBackground;
                r->node->paint(pI,
                               _tx + r->left - r->node->xPos() + r->node->marginLeft(),
                               _ty + r->startY - r->node->yPos() + r->node->marginTop());
                pI.phase = PaintActionChildBackgrounds;
                r->node->paint(pI,
                               _tx + r->left - r->node->xPos() + r->node->marginLeft(),
                               _ty + r->startY - r->node->yPos() + r->node->marginTop());
                pI.phase = PaintActionFloat;
                r->node->paint(pI,
                               _tx + r->left - r->node->xPos() + r->node->marginLeft(),
                               _ty + r->startY - r->node->yPos() + r->node->marginTop());
                pI.phase = PaintActionForeground;
                r->node->paint(pI,
                               _tx + r->left - r->node->xPos() + r->node->marginLeft(),
                               _ty + r->startY - r->node->yPos() + r->node->marginTop());
                pI.phase = PaintActionOutline;
                r->node->paint(pI,
                               _tx + r->left - r->node->xPos() + r->node->marginLeft(),
                               _ty + r->startY - r->node->yPos() + r->node->marginTop());
            }
            pI.phase = oldphase;
        }
    }
}

void RenderBlock::insertPositionedObject(RenderObject *o)
{
    // Create the list of special objects if we don't aleady have one
    if (!m_positionedObjects) {
        m_positionedObjects = new Q3PtrList<RenderObject>;
        m_positionedObjects->setAutoDelete(false);
    }
    else {
        // Don't insert the object again if it's already in the list
        Q3PtrListIterator<RenderObject> it(*m_positionedObjects);
        RenderObject* f;
        while ( (f = it.current()) ) {
            if (f == o) return;
            ++it;
        }
    }

    // Create the special object entry & append it to the list
    setOverhangingContents();
    m_positionedObjects->append(o);
}

void RenderBlock::removePositionedObject(RenderObject *o)
{
    if (m_positionedObjects) {
        Q3PtrListIterator<RenderObject> it(*m_positionedObjects);
        while (it.current()) {
            if (it.current() == o)
                m_positionedObjects->removeRef(it.current());
            ++it;
        }
    }
}

void RenderBlock::insertFloatingObject(RenderObject *o)
{
    // Create the list of special objects if we don't aleady have one
    if (!m_floatingObjects) {
        m_floatingObjects = new Q3PtrList<FloatingObject>;
        m_floatingObjects->setAutoDelete(true);
    }
    else {
        // Don't insert the object again if it's already in the list
        Q3PtrListIterator<FloatingObject> it(*m_floatingObjects);
        FloatingObject* f;
        while ( (f = it.current()) ) {
            if (f->node == o) return;
            ++it;
        }
    }

    // Create the special object entry & append it to the list

    FloatingObject *newObj;
    if (o->isFloating()) {
        // floating object
        o->layoutIfNeeded();

        if(o->style()->floating() == FLEFT)
            newObj = new FloatingObject(FloatingObject::FloatLeft);
        else
            newObj = new FloatingObject(FloatingObject::FloatRight);

        newObj->startY = -500000;
        newObj->endY = -500000;
        newObj->width = o->width() + o->marginLeft() + o->marginRight();
    }
    else {
        // We should never get here, as insertFloatingObject() should only ever be called with floating
        // objects.
        KHTMLAssert(false);
        newObj = 0; // keep gcc's uninitialized variable warnings happy
    }

    newObj->node = o;

    m_floatingObjects->append(newObj);
}

void RenderBlock::removeFloatingObject(RenderObject *o)
{
    if (m_floatingObjects) {
        Q3PtrListIterator<FloatingObject> it(*m_floatingObjects);
        while (it.current()) {
            if (it.current()->node == o)
                m_floatingObjects->removeRef(it.current());
            ++it;
        }
    }
}

void RenderBlock::positionNewFloats()
{
    if(!m_floatingObjects) return;
    FloatingObject *f = m_floatingObjects->getLast();
    if(!f || f->startY != -500000) return;
    FloatingObject *lastFloat;
    while(1)
    {
        lastFloat = m_floatingObjects->prev();
        if (!lastFloat || lastFloat->startY != -500000) {
            m_floatingObjects->next();
            break;
        }
        f = lastFloat;
    }

    int y = m_height;


    // the float can not start above the y position of the last positioned float.
    if(lastFloat && lastFloat->startY > y)
        y = lastFloat->startY;

    while(f)
    {
        //skip elements copied from elsewhere and positioned elements
        if (f->node->containingBlock()!=this)
        {
            f = m_floatingObjects->next();
            continue;
        }

        RenderObject *o = f->node;
        int _height = o->height() + o->marginTop() + o->marginBottom();

        // floats avoid page-breaks
        if(canvas()->pagedMode())
        {
            int top = y;
            int bottom = y + o->height();
            if (crossesPageBreak(top, bottom) && o->height() < canvas()->pageHeight() ) {
                int newY = pageTopAfter(top);
#ifdef PAGE_DEBUG
                kdDebug(6040) << renderName() << " clearing float " << newY - y << "px" << endl;
#endif
                y = newY;
            }
        }

        int ro = rightOffset(); // Constant part of right offset.
        int lo = leftOffset(); // Constant part of left offset.
        int fwidth = f->width; // The width we look for.
                               //kdDebug( 6040 ) << " Object width: " << fwidth << " available width: " << ro - lo << endl;
        if (ro - lo < fwidth)
            fwidth = ro - lo; // Never look for more than what will be available.

        if ( o->style()->clear() & CLEFT )
            y = qMax( leftBottom(), y );
        if ( o->style()->clear() & CRIGHT )
            y = qMax( rightBottom(), y );

        if (o->style()->floating() == FLEFT)
        {
            int heightRemainingLeft = 1;
            int heightRemainingRight = 1;
            int fx = leftRelOffset(y,lo, false, &heightRemainingLeft);
            while (rightRelOffset(y,ro, false, &heightRemainingRight)-fx < fwidth)
            {
                y += qMin( heightRemainingLeft, heightRemainingRight );
                fx = leftRelOffset(y,lo, false, &heightRemainingLeft);
            }
            if (fx<0) fx=0;
            f->left = fx;
            //kdDebug( 6040 ) << "positioning left aligned float at (" << fx + o->marginLeft()  << "/" << y + o->marginTop() << ") fx=" << fx << endl;
            o->setPos(fx + o->marginLeft(), y + o->marginTop());
        }
        else
        {
            int heightRemainingLeft = 1;
            int heightRemainingRight = 1;
            int fx = rightRelOffset(y,ro, false, &heightRemainingRight);
            while (fx - leftRelOffset(y,lo, false, &heightRemainingLeft) < fwidth)
            {
                y += qMin(heightRemainingLeft, heightRemainingRight);
                fx = rightRelOffset(y,ro, false, &heightRemainingRight);
            }
            if (fx<f->width) fx=f->width;
            f->left = fx - f->width;
            //kdDebug( 6040 ) << "positioning right aligned float at (" << fx - o->marginRight() - o->width() << "/" << y + o->marginTop() << ")" << endl;
            o->setPos(fx - o->marginRight() - o->width(), y + o->marginTop());
        }

        if ( m_layer && style()->hidesOverflow() && (o->xPos()+o->width() > m_overflowWidth) )
            m_overflowWidth = o->xPos()+o->width();
                 
        f->startY = y;
        f->endY = f->startY + _height;


        //kdDebug( 6040 ) << "floatingObject x/y= (" << f->left << "/" << f->startY << "-" << f->width << "/" << f->endY - f->startY << ")" << endl;

        f = m_floatingObjects->next();
    }
}

void RenderBlock::newLine()
{
    positionNewFloats();
    // set y position
    int newY = 0;
    switch(m_clearStatus)
    {
        case CLEFT:
            newY = leftBottom();
            break;
        case CRIGHT:
            newY = rightBottom();
            break;
        case CBOTH:
            newY = floatBottom();
        default:
            break;
    }
    if(m_height < newY)
    {
        //      kdDebug( 6040 ) << "adjusting y position" << endl;
        m_height = newY;
    }
    m_clearStatus = CNONE;
}

int
RenderBlock::leftOffset() const
{
    return borderLeft()+paddingLeft();
}

int
RenderBlock::leftRelOffset(int y, int fixedOffset, bool applyTextIndent, int *heightRemaining ) const
{
    int left = fixedOffset;
    if (m_floatingObjects) {
        if ( heightRemaining ) *heightRemaining = 1;
        FloatingObject* r;
        Q3PtrListIterator<FloatingObject> it(*m_floatingObjects);
        for ( ; (r = it.current()); ++it )
        {
            //kdDebug( 6040 ) <<(void *)this << " left: sy, ey, x, w " << r->startY << "," << r->endY << "," << r->left << "," << r->width << " " << endl;
            if (r->startY <= y && r->endY > y &&
                r->type == FloatingObject::FloatLeft &&
                r->left + r->width > left) {
                left = r->left + r->width;
                if ( heightRemaining ) *heightRemaining = r->endY - y;
            }
        }
    }

    if (applyTextIndent && m_firstLine && style()->direction() == LTR ) {
        int cw=0;
        if (style()->textIndent().isPercent())
            cw = containingBlock()->contentWidth();
        left += style()->textIndent().minWidth(cw);
    }

    //kdDebug( 6040 ) << "leftOffset(" << y << ") = " << left << endl;
    return left;
}

int
RenderBlock::rightOffset() const
{
    int right = m_width - borderRight() - paddingRight();
    if (style()->scrollsOverflow() && m_layer)
        right -= m_layer->verticalScrollbarWidth();
    return right;
}

int
RenderBlock::rightRelOffset(int y, int fixedOffset, bool applyTextIndent, int *heightRemaining ) const
{
    int right = fixedOffset;

    if (m_floatingObjects) {
        if (heightRemaining) *heightRemaining = 1;
        FloatingObject* r;
        Q3PtrListIterator<FloatingObject> it(*m_floatingObjects);
        for ( ; (r = it.current()); ++it )
        {
            //kdDebug( 6040 ) << "right: sy, ey, x, w " << r->startY << "," << r->endY << "," << r->left << "," << r->width << " " << endl;
            if (r->startY <= y && r->endY > y &&
                r->type == FloatingObject::FloatRight &&
                r->left < right) {
                right = r->left;
                if ( heightRemaining ) *heightRemaining = r->endY - y;
            }
        }
    }

    if (applyTextIndent &&  m_firstLine && style()->direction() == RTL ) {
        int cw=0;
        if (style()->textIndent().isPercent())
            cw = containingBlock()->contentWidth();
        right -= style()->textIndent().minWidth(cw);
    }

    //kdDebug( 6040 ) << "rightOffset(" << y << ") = " << right << endl;
    return right;
}

unsigned short
RenderBlock::lineWidth(int y) const
{
    //kdDebug( 6040 ) << "lineWidth(" << y << ")=" << rightOffset(y) - leftOffset(y) << endl;
    int result = rightOffset(y) - leftOffset(y);
    return (result < 0) ? 0 : result;
}

int
RenderBlock::nearestFloatBottom(int height) const
{
    if (!m_floatingObjects) return 0;
    int bottom = 0;
    FloatingObject* r;
    Q3PtrListIterator<FloatingObject> it(*m_floatingObjects);
    for ( ; (r = it.current()); ++it )
        if (r->endY>height && (r->endY<bottom || bottom==0))
            bottom=r->endY;
    return qMax(bottom, height);
}

int RenderBlock::floatBottom() const
{
    if (!m_floatingObjects) return 0;
    int bottom=0;
    FloatingObject* r;
    Q3PtrListIterator<FloatingObject> it(*m_floatingObjects);
    for ( ; (r = it.current()); ++it )
        if (r->endY>bottom)
            bottom=r->endY;
    return bottom;
}

int RenderBlock::lowestPosition(bool includeOverflowInterior, bool includeSelf) const
{
    int bottom = RenderFlow::lowestPosition(includeOverflowInterior, includeSelf);
    if (!includeOverflowInterior && style()->hidesOverflow())
        return bottom;
    if (includeSelf && m_overflowHeight > bottom)
        bottom = m_overflowHeight;

    if (m_floatingObjects) {
        FloatingObject* r;
        Q3PtrListIterator<FloatingObject> it(*m_floatingObjects);
        for ( ; (r = it.current()); ++it ) {
            if (!r->noPaint) {
                int lp = r->startY + r->node->marginTop() + r->node->lowestPosition(false);
                bottom = qMax(bottom, lp);
            }
        }
    }
    bottom = qMax(bottom, lowestAbsolutePosition());

    if (!includeSelf && lastLineBox()) {
        int lp = lastLineBox()->yPos() + lastLineBox()->height();
        bottom = qMax(bottom, lp);
    }

    return bottom;
}

int RenderBlock::lowestAbsolutePosition() const
{
    if (!m_positionedObjects)
        return 0;
        
    // Fixed positioned objects do not scroll and thus should not constitute
    // part of the lowest position.
    int bottom = 0;
    RenderObject* r;
    Q3PtrListIterator<RenderObject> it(*m_positionedObjects);
    for ( ; (r = it.current()); ++it ) {
        if (r->style()->position() == FIXED)
            continue;
        int lp = r->yPos() + r->lowestPosition(false);
        bottom = qMax(bottom, lp);
    }
    return bottom;
}

int RenderBlock::rightmostPosition(bool includeOverflowInterior, bool includeSelf) const
{
    int right = RenderFlow::rightmostPosition(includeOverflowInterior, includeSelf);
    if (!includeOverflowInterior && style()->hidesOverflow())
        return right;
    if (includeSelf && m_overflowWidth > right)
        right = m_overflowWidth;

    if (m_floatingObjects) {
        FloatingObject* r;
        Q3PtrListIterator<FloatingObject> it(*m_floatingObjects);
        for ( ; (r = it.current()); ++it ) {
            if (!r->noPaint) {
                int rp = r->left + r->node->marginLeft() + r->node->rightmostPosition(false);
           	right = qMax(right, rp);
            }
        }
    }
    right = qMax(right, rightmostAbsolutePosition());

    if (!includeSelf && firstLineBox()) {
        for (InlineRunBox* currBox = firstLineBox(); currBox; currBox = currBox->nextLineBox()) {
            int rp = currBox->xPos() + currBox->width();
            right = qMax(right, rp);
        }
    }

    return right;
}

int RenderBlock::rightmostAbsolutePosition() const
{
    if (!m_positionedObjects)
        return 0;
    int right = 0;
    RenderObject* r;
    Q3PtrListIterator<RenderObject> it(*m_positionedObjects);
    for ( ; (r = it.current()); ++it ) {
        if (r->style()->position() == FIXED)
            continue;
        int rp = r->xPos() + r->rightmostPosition(false);
        right = qMax(right, rp);
    }
    return right;
}

int RenderBlock::leftmostPosition(bool includeOverflowInterior, bool includeSelf) const
{
    int left = RenderFlow::leftmostPosition(includeOverflowInterior, includeSelf);
    if (!includeOverflowInterior && style()->hidesOverflow())
        return left;

    // FIXME: Check left overflow when we eventually support it.

    if (m_floatingObjects) {
        FloatingObject* r;
        Q3PtrListIterator<FloatingObject> it(*m_floatingObjects);
        for ( ; (r = it.current()); ++it ) {
            if (!r->noPaint) {
                int lp = r->left + r->node->marginLeft() + r->node->leftmostPosition(false);
                left = qMin(left, lp);
            }
        }
    }
    left = qMin(left, leftmostAbsolutePosition());

    if (!includeSelf && firstLineBox()) {
        for (InlineRunBox* currBox = firstLineBox(); currBox; currBox = currBox->nextLineBox())
            left = qMin(left, (int)currBox->xPos());
    }

    return left;
}

int RenderBlock::leftmostAbsolutePosition() const
{
    if (!m_positionedObjects)
        return 0;
    int  left = 0;
    RenderObject* r;
    Q3PtrListIterator<RenderObject> it(*m_positionedObjects);
    for ( ; (r = it.current()); ++it ) {
        if (r->style()->position() == FIXED)
            continue;                         
        int lp = r->xPos() + r->leftmostPosition(false);
        left = qMin(left, lp);
    }
    return left;
}

int
RenderBlock::leftBottom()
{
    if (!m_floatingObjects) return 0;
    int bottom=0;
    FloatingObject* r;
    Q3PtrListIterator<FloatingObject> it(*m_floatingObjects);
    for ( ; (r = it.current()); ++it )
        if (r->endY>bottom && r->type == FloatingObject::FloatLeft)
            bottom=r->endY;

    return bottom;
}

int
RenderBlock::rightBottom()
{
    if (!m_floatingObjects) return 0;
    int bottom=0;
    FloatingObject* r;
    Q3PtrListIterator<FloatingObject> it(*m_floatingObjects);
    for ( ; (r = it.current()); ++it )
        if (r->endY>bottom && r->type == FloatingObject::FloatRight)
            bottom=r->endY;

    return bottom;
}

void
RenderBlock::clearFloats()
{
    if (m_floatingObjects)
        m_floatingObjects->clear();

    // we are done if the element defines a new block formatting context
    if (flowAroundFloats() || isRoot() || isCanvas() || isFloatingOrPositioned() || isTableCell()) return;

    RenderObject *prev = previousSibling();

    // find the element to copy the floats from
    // pass non-flows
    // pass fAF's
    bool parentHasFloats = false;
    while (prev) {
        if (!prev->isRenderBlock() || prev->isFloatingOrPositioned() || prev->flowAroundFloats()) {
            if ( prev->isFloating() && parent()->isRenderBlock() ) {
                parentHasFloats = true;
            }
            prev = prev->previousSibling();
        } else
            break;
    }

    int offset = m_y;
    if (parentHasFloats)
        addOverHangingFloats( static_cast<RenderBlock *>( parent() ),
                              parent()->borderLeft() + parent()->paddingLeft(), offset, false );

    int xoffset = 0;
    if (prev) {
        if(prev->isTableCell()) return;
        offset -= prev->yPos();
    } else {
        prev = parent();
        if(!prev) return;
        xoffset += prev->borderLeft() + prev->paddingLeft();
    }
    //kdDebug() << "RenderBlock::clearFloats found previous "<< (void *)this << " prev=" << (void *)prev<< endl;

    // add overhanging special objects from the previous RenderBlock
    if(!prev->isRenderBlock()) return;
    RenderBlock * flow = static_cast<RenderBlock *>(prev);
    if(!flow->m_floatingObjects) return;
    if(flow->floatBottom() > offset)
        addOverHangingFloats( flow, xoffset, offset, false );
}

void RenderBlock::addOverHangingFloats( RenderBlock *flow, int xoff, int offset, bool child )
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << (void *)this << ": adding overhanging floats xoff=" << xoff << "  offset=" << offset << " child=" << child << endl;
#endif

    // Prevent floats from being added to the canvas by the root element, e.g., <html>.
    if ( !flow->m_floatingObjects || (child && flow->isRoot()) )
        return;

    Q3PtrListIterator<FloatingObject> it(*flow->m_floatingObjects);
    FloatingObject *r;
    for ( ; (r = it.current()); ++it ) {
        if ( ( !child && r->endY > offset ) ||
             ( child && flow->yPos() + r->endY > height() ) ) {
            if (child && !r->crossedLayer) {
                if (flow->enclosingLayer() == enclosingLayer()) {
                  // Set noPaint to true only if we didn't cross layers.
                  r->noPaint = true;
                } else {
                  r->crossedLayer = true;
                }
            }

            FloatingObject* f = 0;
            // don't insert it twice!
            if (m_floatingObjects) {
                Q3PtrListIterator<FloatingObject> it(*m_floatingObjects);
                while ( (f = it.current()) ) {
                    if (f->node == r->node) break;
                    ++it;
                }
            }
            if ( !f ) {
                FloatingObject *floatingObj = new FloatingObject(r->type);
                floatingObj->startY = r->startY - offset;
                floatingObj->endY = r->endY - offset;
                floatingObj->left = r->left - xoff;
                // Applying the child's margin makes no sense in the case where the child was passed in.
                // since his own margin was added already through the subtraction of the |xoff| variable
                // above.  |xoff| will equal -flow->marginLeft() in this case, so it's already been taken
                // into account.  Only apply this code if |child| is false, since otherwise the left margin
                // will get applied twice. -dwh
                if (!child && flow != parent())
                    floatingObj->left += flow->marginLeft();
                if ( !child ) {
                    floatingObj->left -= marginLeft();
                    floatingObj->noPaint = true;
                }
                else {
                    floatingObj->noPaint = (r->crossedLayer || !r->noPaint);
                    floatingObj->crossedLayer = r->crossedLayer;
                }

                floatingObj->width = r->width;
                floatingObj->node = r->node;
                if (!m_floatingObjects) {
                    m_floatingObjects = new Q3PtrList<FloatingObject>;
                    m_floatingObjects->setAutoDelete(true);
                 }
                m_floatingObjects->append(floatingObj);
#ifdef DEBUG_LAYOUT
                kdDebug( 6040 ) << "addOverHangingFloats x/y= (" << floatingObj->left << "/" << floatingObj->startY << "-" << floatingObj->width << "/" << floatingObj->endY - floatingObj->startY << ")" << endl;
#endif
            }
        }
    }
}

bool RenderBlock::containsFloat(RenderObject* o) const
{
    if (m_floatingObjects) {
        Q3PtrListIterator<FloatingObject> it(*m_floatingObjects);
        while (it.current()) {
            if (it.current()->node == o)
                return true;
            ++it;
        }
    }
    return false;
}

void RenderBlock::markAllDescendantsWithFloatsForLayout(RenderObject* floatToRemove)
{
    setNeedsLayout(true);

    if (floatToRemove)
        removeFloatingObject(floatToRemove);

    // Iterate over our children and mark them as needed.
    if (!childrenInline()) {
        for (RenderObject* child = firstChild(); child; child = child->nextSibling()) {
            if (isBlockFlow() && !child->isFloatingOrPositioned() &&
                (floatToRemove ? child->containsFloat(floatToRemove) : child->hasFloats()))
                child->markAllDescendantsWithFloatsForLayout(floatToRemove);
        }
    }
}

int RenderBlock::getClearDelta(RenderObject *child)
{
    //kdDebug( 6040 ) << "getClearDelta on child " << child << " oldheight=" << m_height << endl;
    bool clearSet = child->style()->clear() != CNONE;
    int bottom = 0;
    switch(child->style()->clear())
    {
        case CNONE:
            break;
        case CLEFT:
            bottom = leftBottom();
            break;
        case CRIGHT:
            bottom = rightBottom();
            break;
        case CBOTH:
            bottom = floatBottom();
            break;
    }

    // We also clear floats if we are too big to sit on the same line as a float, and happen to flow around floats.
    // FIXME: Note that the remaining space checks aren't quite accurate, since you should be able to clear only some floats (the minimum # needed
    // to fit) and not all (we should be using nearestFloatBottom and looping).
    int result = clearSet ? qMax(0, bottom - child->yPos()) : 0;
    if (!result && child->flowAroundFloats() && !style()->width().isVariable()) {
        if ((child->style()->width().isPercent() && child->width() > lineWidth(child->yPos())) ||
            (child->style()->width().isFixed() && child->minWidth() > lineWidth(child->yPos()) && 
              child->minWidth() <= contentWidth()))
            result = qMax(0, floatBottom() - child->yPos());
    }
    return result;
}

bool RenderBlock::isPointInScrollbar(int _x, int _y, int _tx, int _ty)
{
    if (!style()->scrollsOverflow() || !m_layer)
        return false;

    if (m_layer->verticalScrollbarWidth()) {
        QRect vertRect(_tx + width() - borderRight() - m_layer->verticalScrollbarWidth(),
                       _ty + borderTop(),
                       m_layer->verticalScrollbarWidth(),
                       height()-borderTop()-borderBottom());
        if (vertRect.contains(_x, _y)) {
#ifdef APPLE_CHANGES
            RenderLayer::gScrollBar = m_layer->verticalScrollbar();
#endif
            return true;
        }
    }

    if (m_layer->horizontalScrollbarHeight()) {
        QRect horizRect(_tx + borderLeft(),
                        _ty + height() - borderBottom() - m_layer->horizontalScrollbarHeight(),
                        width()-borderLeft()-borderRight(),
                        m_layer->horizontalScrollbarHeight());
        if (horizRect.contains(_x, _y)) {
#ifdef APPLE_CHANGES
            RenderLayer::gScrollBar = m_layer->horizontalScrollbar();
#endif
            return true;
        }
    }

    return false;
}

bool RenderBlock::nodeAtPoint(NodeInfo& info, int _x, int _y, int _tx, int _ty, HitTestAction hitTestAction, bool inBox)
{
    bool inScrollbar = isPointInScrollbar(_x, _y, _tx+xPos(), _ty+yPos());
    if (inScrollbar && hitTestAction != HitTestChildrenOnly)
        inBox = true;

    if (hitTestAction != HitTestSelfOnly && m_floatingObjects && !inScrollbar) {
        int stx = _tx + xPos();
        int sty = _ty + yPos();
        if (style()->hidesOverflow() && m_layer)
            m_layer->subtractScrollOffset(stx, sty);
        FloatingObject* o;
        Q3PtrListIterator<FloatingObject> it(*m_floatingObjects);
        for (it.toLast(); (o = it.current()); --it)
            if (!o->noPaint && !o->node->layer())
                inBox |= o->node->nodeAtPoint(info, _x, _y,
                                              stx+o->left + o->node->marginLeft() - o->node->xPos(),
                                              sty+o->startY + o->node->marginTop() - o->node->yPos(), HitTestAll ) ;
    }

    inBox |= RenderFlow::nodeAtPoint(info, _x, _y, _tx, _ty, hitTestAction, inBox);
    return inBox;
}

void RenderBlock::calcMinMaxWidth()
{
    KHTMLAssert( !minMaxKnown() );

#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(RenderBlock)::calcMinMaxWidth() this=" << this << endl;
#endif

    m_minWidth = 0;
    m_maxWidth = 0;

    bool preOrNowrap = style()->whiteSpace() != NORMAL;
    if (childrenInline())
        calcInlineMinMaxWidth();
    else
        calcBlockMinMaxWidth();

    if(m_maxWidth < m_minWidth) m_maxWidth = m_minWidth;

    if (preOrNowrap && childrenInline()) {
         m_minWidth = m_maxWidth;

        // A horizontal marquee with inline children has no minimum width.
        if (style()->overflow() == OMARQUEE && m_layer && m_layer->marquee() &&
            m_layer->marquee()->isHorizontal() && !m_layer->marquee()->isUnfurlMarquee())
            m_minWidth = 0;
    }

     if (style()->width().isFixed() && style()->width().value() > 0) {
        if (isTableCell())
            m_maxWidth = qMax(m_minWidth,short(style()->width().value()));
        else
            m_minWidth = m_maxWidth = short(style()->width().value());
    }

    if (style()->minWidth().isFixed() && style()->minWidth().value() > 0) {
        m_maxWidth = qMax(m_maxWidth, int(style()->minWidth().value()));
        m_minWidth = qMax(m_minWidth, short(style()->minWidth().value()));
    }

    if (style()->maxWidth().isFixed() && style()->maxWidth().value() != UNDEFINED) {
        m_maxWidth = qMin(m_maxWidth, int(style()->maxWidth().value()));
        m_minWidth = qMin(m_minWidth, short(style()->maxWidth().value()));
    }

    int toAdd = 0;
    toAdd = borderLeft() + borderRight() + paddingLeft() + paddingRight();

    m_minWidth += toAdd;
    m_maxWidth += toAdd;

    setMinMaxKnown();

    //kdDebug( 6040 ) << "Text::calcMinMaxWidth(" << this << "): min = " << m_minWidth << " max = " << m_maxWidth << endl;
    // ### compare with min/max width set in style sheet...
}

struct InlineMinMaxIterator
{
/* InlineMinMaxIterator is a class that will iterate over all render objects that contribute to
   inline min/max width calculations.  Note the following about the way it walks:
   (1) Positioned content is skipped (since it does not contribute to min/max width of a block)
   (2) We do not drill into the children of floats or replaced elements, since you can't break
       in the middle of such an element.
   (3) Inline flows (e.g., <a>, <span>, <i>) are walked twice, since each side can have
       distinct borders/margin/padding that contribute to the min/max width.
*/
    RenderObject* parent;
    RenderObject* current;
    bool endOfInline;

    InlineMinMaxIterator(RenderObject* p, RenderObject* o, bool end = false)
        :parent(p), current(o), endOfInline(end) {}

    RenderObject* next();
};

RenderObject* InlineMinMaxIterator::next()
{
    RenderObject* result = 0;
    bool oldEndOfInline = endOfInline;
    endOfInline = false;
    while (current != 0 || (current == parent))
    {
        //kdDebug( 6040 ) << "current = " << current << endl;
        if (!oldEndOfInline &&
            (current == parent ||
             (!current->isFloating() && !current->isReplaced() && !current->isPositioned())))
            result = current->firstChild();
        if (!result) {
            // We hit the end of our inline. (It was empty, e.g., <span></span>.)
            if (!oldEndOfInline && current->isInlineFlow()) {
                result = current;
                endOfInline = true;
                break;
            }

            while (current && current != parent) {
                result = current->nextSibling();
                if (result) break;
                current = current->parent();
                if (current && current != parent && current->isInlineFlow()) {
                    result = current;
                    endOfInline = true;
                    break;
                }
            }
        }

        if (!result) break;

        if (!result->isPositioned() && (result->isText() || result->isBR() ||
            result->isFloating() || result->isReplaced() ||
            result->isInlineFlow()))
            break;

        current = result;
        result = 0;
    }

    // Update our position.
    current = result;
    return current;
}

// bidi.cpp defines the following functions too.
// Maybe these should not be static, after all...

#ifndef KDE_USE_FINAL

static int getBPMWidth(int childValue, Length cssUnit)
{
    if (!cssUnit.isVariable())
        return (cssUnit.isFixed() ? cssUnit.value() : childValue);
    return 0;
}

static int getBorderPaddingMargin(RenderObject* child, bool endOfInline)
{
    RenderStyle* cstyle = child->style();
    int result = 0;
    bool leftSide = (cstyle->direction() == LTR) ? !endOfInline : endOfInline;
    result += getBPMWidth((leftSide ? child->marginLeft() : child->marginRight()),
                          (leftSide ? cstyle->marginLeft() :
                                      cstyle->marginRight()));
    result += getBPMWidth((leftSide ? child->paddingLeft() : child->paddingRight()),
                          (leftSide ? cstyle->paddingLeft() :
                                      cstyle->paddingRight()));
    result += leftSide ? child->borderLeft() : child->borderRight();
    return result;
}
#endif

static void stripTrailingSpace(bool pre,
                               int& inlineMax, int& inlineMin,
                               RenderObject* trailingSpaceChild)
{
    if (!pre && trailingSpaceChild && trailingSpaceChild->isText()) {
        // Collapse away the trailing space at the end of a block.
        RenderText* t = static_cast<RenderText *>(trailingSpaceChild);
        const Font *f = t->htmlFont( false );
        QChar space[1]; space[0] = ' ';
        int spaceWidth = f->width(space, 1, 0);
        inlineMax -= spaceWidth;
        if (inlineMin > inlineMax)
            inlineMin = inlineMax;
    }
}

void RenderBlock::calcInlineMinMaxWidth()
{
    int inlineMax=0;
    int inlineMin=0;

    int cw = containingBlock()->contentWidth();

    // If we are at the start of a line, we want to ignore all white-space.
    // Also strip spaces if we previously had text that ended in a trailing space.
    bool stripFrontSpaces = true;
    
    bool isTcQuirk = isTableCell() && style()->htmlHacks() && style()->width().isVariable();
    
    RenderObject* trailingSpaceChild = 0;

    bool normal, oldnormal;
    normal = oldnormal = style()->whiteSpace() == NORMAL;

    InlineMinMaxIterator childIterator(this, this);
    bool addedTextIndent = false; // Only gets added in once.
    RenderObject* prevFloat = 0;
    while (RenderObject* child = childIterator.next())
    {
        normal = child->style()->whiteSpace() == NORMAL;

        if( !child->isBR() )
        {
            // Step One: determine whether or not we need to go ahead and
            // terminate our current line.  Each discrete chunk can become
            // the new min-width, if it is the widest chunk seen so far, and
            // it can also become the max-width.

            // Children fall into three categories:
            // (1) An inline flow object.  These objects always have a min/max of 0,
            // and are included in the iteration solely so that their margins can
            // be added in.
            //
            // (2) An inline non-text non-flow object, e.g., an inline replaced element.
            // These objects can always be on a line by themselves, so in this situation
            // we need to go ahead and break the current line, and then add in our own
            // margins and min/max width on its own line, and then terminate the line.
            //
            // (3) A text object.  Text runs can have breakable characters at the start,
            // the middle or the end.  They may also lose whitespace off the front if
            // we're already ignoring whitespace.  In order to compute accurate min-width
            // information, we need three pieces of information.
            // (a) the min-width of the first non-breakable run.  Should be 0 if the text string
            // starts with whitespace.
            // (b) the min-width of the last non-breakable run. Should be 0 if the text string
            // ends with whitespace.
            // (c) the min/max width of the string (trimmed for whitespace).
            //
            // If the text string starts with whitespace, then we need to go ahead and
            // terminate our current line (unless we're already in a whitespace stripping
            // mode.
            //
            // If the text string has a breakable character in the middle, but didn't start
            // with whitespace, then we add the width of the first non-breakable run and
            // then end the current line.  We then need to use the intermediate min/max width
            // values (if any of them are larger than our current min/max).  We then look at
            // the width of the last non-breakable run and use that to start a new line
            // (unless we end in whitespace).
            RenderStyle* cstyle = child->style();
            short childMin = 0;
            short childMax = 0;

            if (!child->isText()) {
                // Case (1) and (2).  Inline replaced and inline flow elements.
                if (child->isInlineFlow()) {
                    // Add in padding/border/margin from the appropriate side of
                    // the element.
                    int bpm = getBorderPaddingMargin(child, childIterator.endOfInline);
                    childMin += bpm;
                    childMax += bpm;

                    inlineMin += childMin;
                    inlineMax += childMax;
                }
                else {
                    // Inline replaced elements add in their margins to their min/max values.
                    int margins = 0;
                    LengthType type = cstyle->marginLeft().type();
                    if ( type != Variable )
                        margins += (type == Fixed ? cstyle->marginLeft().value() : child->marginLeft());
                    type = cstyle->marginRight().type();
                    if ( type != Variable )
                        margins += (type == Fixed ? cstyle->marginRight().value() : child->marginRight());
                    childMin += margins;
                    childMax += margins;
                }
            }

            if (!child->isRenderInline() && !child->isText()) {
                
                bool qBreak = isTcQuirk && !child->isFloatingOrPositioned();
                // Case (2). Inline replaced elements and floats.
                // Go ahead and terminate the current line as far as
                // minwidth is concerned.
                childMin += child->minWidth();
                childMax += child->maxWidth();

                if (!qBreak && (normal || oldnormal)) {
                    if(m_minWidth < inlineMin) m_minWidth = inlineMin;
                    inlineMin = 0;
                }

                // Check our "clear" setting.  If we're supposed to clear the previous float, then
                // go ahead and terminate maxwidth as well.
                if (child->isFloating()) {
                    if (prevFloat &&
                        ((prevFloat->style()->floating() == FLEFT && (child->style()->clear() & CLEFT)) ||
                         (prevFloat->style()->floating() == FRIGHT && (child->style()->clear() & CRIGHT)))) {
                        m_maxWidth = qMax(inlineMax, (int)m_maxWidth);
                        inlineMax = 0;
                    }
                    prevFloat = child;
                }

                // Add in text-indent.  This is added in only once.
                int ti = 0;
                if ( !addedTextIndent ) {
                    addedTextIndent = true;
                    ti = style()->textIndent().minWidth( cw );
                    childMin+=ti;
                    childMax+=ti;
                }

                // Add our width to the max.
                inlineMax += childMax;

                if (!normal||qBreak)
                    inlineMin += childMin;
                else {
                    // Now check our line.
                    inlineMin = childMin;
                    if(m_minWidth < inlineMin) m_minWidth = inlineMin;

                    // Now start a new line.
                    inlineMin = 0;
                }

                // We are no longer stripping whitespace at the start of
                // a line.
                if (!child->isFloating()) {
                    stripFrontSpaces = false;
                   trailingSpaceChild = 0;
                }
            }
            else if (child->isText())
            {
                // Case (3). Text.
                RenderText* t = static_cast<RenderText *>(child);

                // Determine if we have a breakable character.  Pass in
                // whether or not we should ignore any spaces at the front
                // of the string.  If those are going to be stripped out,
                // then they shouldn't be considered in the breakable char
                // check.
                bool hasBreakableChar, hasBreak;
                short beginMin, endMin;
                bool beginWS, endWS;
                short beginMax, endMax;
                t->trimmedMinMaxWidth(beginMin, beginWS, endMin, endWS, hasBreakableChar,
                                      hasBreak, beginMax, endMax,
                                      childMin, childMax, stripFrontSpaces);

                // This text object is insignificant and will not be rendered.  Just
                // continue.
                if (!hasBreak && childMax == 0) continue;

                if (stripFrontSpaces)
                    trailingSpaceChild = child;
                else
                    trailingSpaceChild = 0;

                // Add in text-indent.  This is added in only once.
                int ti = 0;
                if (!addedTextIndent) {
                    addedTextIndent = true;
                    ti = style()->textIndent().minWidth(cw);
                    childMin+=ti; beginMin += ti;
                    childMax+=ti; beginMax += ti;
                }

                // If we have no breakable characters at all,
                // then this is the easy case. We add ourselves to the current
                // min and max and continue.
                if (!hasBreakableChar) {
                    inlineMin += childMin;
                }
                else {
                    // We have a breakable character.  Now we need to know if
                    // we start and end with whitespace.
                    if (beginWS) {
                        // Go ahead and end the current line.
                        if(m_minWidth < inlineMin) m_minWidth = inlineMin;
                    }
                    else {
                        inlineMin += beginMin;
                        if(m_minWidth < inlineMin) m_minWidth = inlineMin;
                        childMin -= ti;
                    }

                    inlineMin = childMin;

                    if (endWS) {
                        // We end in whitespace, which means we can go ahead
                        // and end our current line.
                        if(m_minWidth < inlineMin) m_minWidth = inlineMin;
                        inlineMin = 0;
                    }
                    else {
                        if(m_minWidth < inlineMin) m_minWidth = inlineMin;
                        inlineMin = endMin;
                    }
                }

                if (hasBreak) {
                    inlineMax += beginMax;
                    if (m_maxWidth < inlineMax) m_maxWidth = inlineMax;
                    if (m_maxWidth < childMax) m_maxWidth = childMax;
                    inlineMax = endMax;
                }
                else
                    inlineMax += childMax;
            }
        }
        else
        {
            if(m_minWidth < inlineMin) m_minWidth = inlineMin;
            if(m_maxWidth < inlineMax) m_maxWidth = inlineMax;
            inlineMin = inlineMax = 0;
            stripFrontSpaces = true;
            trailingSpaceChild = 0;
        }

        oldnormal = normal;
    }

    stripTrailingSpace(m_pre, inlineMax, inlineMin, trailingSpaceChild);

    if(m_minWidth < inlineMin) m_minWidth = inlineMin;
    if(m_maxWidth < inlineMax) m_maxWidth = inlineMax;
    //         kdDebug( 6040 ) << "m_minWidth=" << m_minWidth
    // 			<< " m_maxWidth=" << m_maxWidth << endl;
}

// Use a very large value (in effect infinite).
#define BLOCK_MAX_WIDTH 15000

void RenderBlock::calcBlockMinMaxWidth()
{
    bool nowrap = style()->whiteSpace() == NOWRAP;

    RenderObject *child = firstChild();
    RenderObject* prevFloat = 0;
    int floatWidths = 0;
    while(child != 0)
    {
        // positioned children don't affect the minmaxwidth
        if (child->isPositioned()) {
            child = child->nextSibling();
            continue;
        }

        if (prevFloat && (!child->isFloating() ||
                          (prevFloat->style()->floating() == FLEFT && (child->style()->clear() & CLEFT)) ||
                          (prevFloat->style()->floating() == FRIGHT && (child->style()->clear() & CRIGHT)))) {
            m_maxWidth = qMax(floatWidths, m_maxWidth);
            floatWidths = 0;
        }

        Length ml = child->style()->marginLeft();
        Length mr = child->style()->marginRight();

        // Call calcWidth on the child to ensure that our margins are
        // up to date.  This method can be called before the child has actually
        // calculated its margins (which are computed inside calcWidth).
        if (ml.isPercent() || mr.isPercent())
            calcWidth();

        // A margin basically has three types: fixed, percentage, and auto (variable).
        // Auto margins simply become 0 when computing min/max width.
        // Fixed margins can be added in as is.
        // Percentage margins are computed as a percentage of the width we calculated in
        // the calcWidth call above.  In this case we use the actual cached margin values on
        // the RenderObject itself.
        int margin = 0;
        if (ml.isFixed())
            margin += ml.value();
        else if (ml.isPercent())
            margin += child->marginLeft();

        if (mr.isFixed())
            margin += mr.value();
        else if (mr.isPercent())
            margin += child->marginRight();

        if (margin < 0) margin = 0;

        int w = child->minWidth() + margin;
        if(m_minWidth < w) m_minWidth = w;
        // IE ignores tables for calculation of nowrap. Makes some sense.
        if ( nowrap && !child->isTable() && m_maxWidth < w )
            m_maxWidth = w;

        w = child->maxWidth() + margin;

        if(m_maxWidth < w) m_maxWidth = w;

        if (child->isFloating())
            floatWidths += w;
        else if (m_maxWidth < w)
            m_maxWidth = w;

        // A very specific WinIE quirk.
        // Example:
        /*
           <div style="position:absolute; width:100px; top:50px;">
              <div style="position:absolute;left:0px;top:50px;height:50px;background-color:green">
                <table style="width:100%"><tr><td></table>
              </div>
           </div>
        */
        // In the above example, the inner absolute positioned block should have a computed width
        // of 100px because of the table.
        // We can achieve this effect by making the maxwidth of blocks that contain tables
        // with percentage widths be infinite (as long as they are not inside a table cell).
        if (style()->htmlHacks() && child->style()->width().isPercent() &&
            !isTableCell() && child->isTable() && m_maxWidth < BLOCK_MAX_WIDTH) {
            RenderBlock* cb = containingBlock();
            while (!cb->isCanvas() && !cb->isTableCell())
                cb = cb->containingBlock();
            if (!cb->isTableCell())
                m_maxWidth = BLOCK_MAX_WIDTH;
        }
        if (child->isFloating())
            prevFloat = child;
        child = child->nextSibling();
    }
    m_maxWidth = qMax(floatWidths, m_maxWidth);
}

void RenderBlock::close()
{
    if (lastChild() && lastChild()->isAnonymousBlock())
        lastChild()->close();
    updateFirstLetter();
    RenderFlow::close();
}

int RenderBlock::getBaselineOfFirstLineBox()
{
    if (m_firstLineBox)
        return m_firstLineBox->yPos() + m_firstLineBox->baseline();

    if (isInline())
        return -1; // We're inline and had no line box, so we have no baseline we can return.

    for (RenderObject* curr = firstChild(); curr; curr = curr->nextSibling()) {
        int result = curr->getBaselineOfFirstLineBox();
        if (result != -1)
            return curr->yPos() + result; // Translate to our coordinate space.
    }

    return -1;
}

InlineFlowBox* RenderBlock::getFirstLineBox()
{
    if (m_firstLineBox)
        return m_firstLineBox;

    if (isInline())
        return 0; // We're inline and had no line box, so we have no baseline we can return.

    for (RenderObject* curr = firstChild(); curr; curr = curr->nextSibling()) {
        InlineFlowBox* result = curr->getFirstLineBox();
        if (result)
            return result;
    }

    return 0;
}

bool RenderBlock::inRootBlockContext() const
{
    if (isTableCell() || isFloatingOrPositioned() || style()->hidesOverflow())
        return false;

    if (isRoot() || isCanvas())
        return true;

    return containingBlock()->inRootBlockContext();
}

const char *RenderBlock::renderName() const
{
    if (isFloating())
        return "RenderBlock (floating)";
    if (isPositioned())
        return "RenderBlock (positioned)";
    if (isAnonymousBlock() && m_avoidPageBreak)
        return "RenderBlock (avoidPageBreak)";
    if (isAnonymousBlock())
        return "RenderBlock (anonymous)";
    else if (isAnonymous())
        return "RenderBlock (generated)";
    if (isRelPositioned())
        return "RenderBlock (relative positioned)";
    if (style() && style()->display() == COMPACT)
        return "RenderBlock (compact)";
    if (style() && style()->display() == RUN_IN)
        return "RenderBlock (run-in)";
    return "RenderBlock";
}

#ifdef ENABLE_DUMP
void RenderBlock::printTree(int indent) const
{
    RenderFlow::printTree(indent);

    if (m_floatingObjects)
    {
        Q3PtrListIterator<FloatingObject> it(*m_floatingObjects);
        FloatingObject *r;
        for ( ; (r = it.current()); ++it )
        {
            QString s;
            s.fill(' ', indent);
            kdDebug() << s << renderName() << ":  " <<
                (r->type == FloatingObject::FloatLeft ? "FloatLeft" : "FloatRight" )  <<
                "[" << r->node->renderName() << ": " << (void*)r->node << "] (" << r->startY << " - " << r->endY << ")" << "width: " << r->width <<
                endl;
        }
    }
}

void RenderBlock::dump(QTextStream &stream, const QString &ind) const
{
    RenderFlow::dump(stream,ind);

    if (m_childrenInline) { stream << QLatin1String(" childrenInline"); }
    if (m_pre) { stream << (" pre"); }
    if (m_firstLine) { stream << QLatin1String(" firstLine"); }

    if (m_floatingObjects && !m_floatingObjects->isEmpty())
    {
        stream << QLatin1String(" special(");
        Q3PtrListIterator<FloatingObject> it(*m_floatingObjects);
        FloatingObject *r;
        bool first = true;
        for ( ; (r = it.current()); ++it )
        {
            if (!first)
                stream << QLatin1Char(',');
            stream << r->node->renderName();
            first = false;
        }
        stream << QLatin1Char(')');
    }

    // ### EClear m_clearStatus
}
#endif

#undef DEBUG
#undef DEBUG_LAYOUT
#undef BOX_DEBUG

} // namespace khtml

