/**
 * This file is part of the html renderer for KDE.
 *
 * Copyright (C) 2001-2003 Lars Knoll (knoll@kde.org)
 *           (C) 2001 Antti Koivisto (koivisto@kde.org)
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

//#define DEBUG_LAYOUT

#include "rendering/render_container.h"
#include "rendering/render_table.h"
#include "rendering/render_text.h"
#include "rendering/render_image.h"
#include "rendering/render_canvas.h"
#include "rendering/render_generated.h"
#include "rendering/render_inline.h"
#include "xml/dom_docimpl.h"
#include "css/css_valueimpl.h"

#include <kdebug.h>
#include <assert.h>

using namespace khtml;

RenderContainer::RenderContainer(DOM::NodeImpl* node)
    : RenderObject(node)
{
    m_first = 0;
    m_last = 0;
}

void RenderContainer::detach()
{
    if (continuation())
        continuation()->detach();

    RenderObject* next;
    for(RenderObject* n = m_first; n; n = next ) {
	n->removeFromObjectLists();
        n->setParent(0);
        next = n->nextSibling();
        n->detach();
    }
    m_first = 0;
    m_last = 0;

    RenderObject::detach();
}

void RenderContainer::addChild(RenderObject *newChild, RenderObject *beforeChild)
{
#ifdef DEBUG_LAYOUT
    kDebug( 6040 ) << this << ": " <<  renderName() << "(RenderObject)::addChild( " << newChild << ": " <<
        newChild->renderName() << ", " << (beforeChild ? beforeChild->renderName() : "0") << " )" << endl;
#endif

    bool needsTable = false;

    if(!newChild->isText() && !newChild->isReplaced()) {
        switch(newChild->style()->display()) {
        case INLINE:
        case BLOCK:
        case LIST_ITEM:
        case RUN_IN:
	case COMPACT:
        case INLINE_BLOCK:
        case TABLE:
        case INLINE_TABLE:
            break;
        case TABLE_COLUMN:
            if ( isTableCol() )
                break;
            // nobreak
        case TABLE_COLUMN_GROUP:
        case TABLE_CAPTION:
        case TABLE_ROW_GROUP:
        case TABLE_HEADER_GROUP:
        case TABLE_FOOTER_GROUP:

            //kDebug( 6040 ) << "adding section" << endl;
            if ( !isTable() )
                needsTable = true;
            break;
        case TABLE_ROW:
            //kDebug( 6040 ) << "adding row" << endl;
            if ( !isTableSection() )
                needsTable = true;
            break;
        case TABLE_CELL:
            //kDebug( 6040 ) << "adding cell" << endl;
            if ( !isTableRow() )
                needsTable = true;
            // I'm not 100% sure this is the best way to fix this, but without this
            // change we recurse infinitely when trying to render the CSS2 test page:
            // http://www.bath.ac.uk/%7Epy8ieh/internet/eviltests/htmlbodyheadrendering2.html.
            if ( isTableCell() && !firstChild() && !newChild->isTableCell() )
                needsTable = false;

            break;
        case NONE:
            // RenderHtml and some others can have display:none
            // KHTMLAssert(false);
            break;
        }
    }

    if ( needsTable ) {
        RenderTable *table;
	RenderObject *last = beforeChild ? beforeChild->previousSibling() : lastChild();
        if ( last && last->isTable() && last->isAnonymous() ) {
            table = static_cast<RenderTable *>(last);
        } else {
	    //kDebug( 6040 ) << "creating anonymous table, before=" << beforeChild << endl;
            table = new (renderArena()) RenderTable(document() /* is anonymous */);
            RenderStyle *newStyle = new RenderStyle();
            newStyle->inheritFrom(style());
	    newStyle->setDisplay( TABLE );
	    newStyle->setFlowAroundFloats( true );
	    table->setParent( this ); // so it finds the arena
            table->setStyle(newStyle);
	    table->setParent( 0 );
            addChild(table, beforeChild);
        }
        table->addChild(newChild);
    } else {
	// just add it...
	insertChildNode(newChild, beforeChild);
    }
}

RenderObject* RenderContainer::removeChildNode(RenderObject* oldChild)
{
    KHTMLAssert(oldChild->parent() == this);

    // So that we'll get the appropriate dirty bit set (either that a normal flow child got yanked or
    // that a positioned child got yanked).  We also repaint, so that the area exposed when the child
    // disappears gets repainted properly.
    if ( document()->renderer() ) {
        oldChild->setNeedsLayoutAndMinMaxRecalc();
        oldChild->repaint();

        // Keep our layer hierarchy updated.
        oldChild->removeLayers(enclosingLayer());

        // if oldChild is the start or end of the selection, then clear
        // the selection to avoid problems of invalid pointers

        // ### This is not the "proper" solution... ideally the selection
        // ### should be maintained based on DOM Nodes and a Range, which
        // ### gets adjusted appropriately when nodes are deleted/inserted
        // ### near etc. But this at least prevents crashes caused when
        // ### the start or end of the selection is deleted and then
        // ### accessed when the user next selects something.

        if (oldChild->isSelectionBorder()) {
            RenderObject *root = oldChild;
            while (root && root->parent())
                root = root->parent();
            if (root->isCanvas()) {
                static_cast<RenderCanvas*>(root)->clearSelection();
            }
        }
    }

    // remove the child
    if (oldChild->previousSibling())
        oldChild->previousSibling()->setNextSibling(oldChild->nextSibling());
    if (oldChild->nextSibling())
        oldChild->nextSibling()->setPreviousSibling(oldChild->previousSibling());

    if (m_first == oldChild)
        m_first = oldChild->nextSibling();
    if (m_last == oldChild)
        m_last = oldChild->previousSibling();

    oldChild->setPreviousSibling(0);
    oldChild->setNextSibling(0);
    oldChild->setParent(0);

    return oldChild;
}

void RenderContainer::setStyle(RenderStyle* _style)
{
    RenderObject::setStyle(_style);

    // If we are a pseudo-container we need to restyle the children
    if (style()->styleType() == RenderStyle::BEFORE
     || style()->styleType() == RenderStyle::AFTER)
    {
        // ### we could save this call when the change only affected
        // non inherited properties
        RenderStyle *pseudoStyle = new RenderStyle();
        pseudoStyle->inheritFrom(style());
        pseudoStyle->ref();
        RenderObject *child = firstChild();
        while (child != 0)
        {
            child->setStyle(pseudoStyle);
            child = child->nextSibling();
        }
        pseudoStyle->deref();
    }
}

void RenderContainer::updatePseudoChildren()
{
    // In CSS2, before/after pseudo-content cannot nest.  Check this first.
    // Remove when CSS 3 Generated Content becomes Candidate Recommendation
    if (style()->styleType() == RenderStyle::BEFORE
     || style()->styleType() == RenderStyle::AFTER)
        return;

    updatePseudoChild(RenderStyle::BEFORE, firstChild());
    updatePseudoChild(RenderStyle::AFTER, lastChild());
    // updatePseudoChild(RenderStyle::MARKER, marker());
}

void RenderContainer::updatePseudoChild(RenderStyle::PseudoId type, RenderObject* child)
{
    RenderStyle* pseudo = style()->getPseudoStyle(type);

    // Whether or not we currently have generated content attached.
    bool oldContentPresent = child && (child->style()->styleType() == type);

    // Whether or not we now want generated content.
    bool newContentWanted = pseudo && pseudo->display() != NONE;

    // For <q><p/></q>, if this object is the inline continuation of the <q>,
    // we only want to generate ::after content and not ::before content.
    if (type == RenderStyle::BEFORE && isInlineContinuation())
        newContentWanted = false;

    // Similarly, if we're the beginning of a <q>, and there's an inline continuation for
    // our object, then we don't generate the :after content.
    if (type == RenderStyle::AFTER && isRenderInline() && continuation())
        newContentWanted = false;

    // No generated content
    if (!oldContentPresent && !newContentWanted)
        return;

    // Whether or not we want the same old content.
    bool sameOldContent = oldContentPresent && newContentWanted
                       && (child->style()->contentDataEquivalent(pseudo));

    // No change in content, update style
    if( sameOldContent ) {
        child->setStyle(pseudo);
        return;
    }

    // If we don't want generated content any longer, or if we have generated content,
    // but it's no longer identical to the new content data we want to build
    // render objects for, then we nuke all of the old generated content.
    if (oldContentPresent && (!newContentWanted || !sameOldContent))
    {
        // The child needs to be removed.
        oldContentPresent = false;
        removeChild(child);
        child = (type == RenderStyle::BEFORE) ? firstChild() : lastChild();
    }

    // If we have no pseudo-style or if the pseudo's display type is NONE, then we
    // have no generated content and can now return.
    if (!newContentWanted)
        return;

    if (isInlineFlow() && pseudo->display() != INLINE)
        // According to the CSS2 spec (the end of section 12.1), the only allowed
        // display values for the pseudo style are NONE and INLINE.  Since we already
        // determined that the pseudo is not display NONE, any display other than
        // inline should be mutated to INLINE.
        pseudo->setDisplay(INLINE);

    RenderObject* insertBefore = (type == RenderStyle::BEFORE) ? child : 0;

    // Generated content consists of a single container that houses multiple children (specified
    // by the content property).  This pseudo container gets the pseudo style set on it.
    RenderContainer* pseudoContainer = 0;
    pseudoContainer = RenderFlow::createFlow(element(), pseudo, renderArena());
    pseudoContainer->setIsAnonymous( true );
    pseudoContainer->createCSSContent();

    // Only add the container if it had content
    if (pseudoContainer->firstChild()) {
        // Add the pseudo after we've installed all our content, so that addChild will be able
        // to find the text inside the inline for e.g., first-letter styling.
        addChild(pseudoContainer, insertBefore);
        pseudoContainer->close();
    }
}

void RenderContainer::createCSSContent()
{
    RenderStyle* pseudo = style();
    RenderStyle* style = new RenderStyle();
    style->ref();
    style->inheritFrom(pseudo);

    // Now walk our list of generated content and create render objects for every type
    // we encounter.
    for (ContentData* contentData = pseudo->contentData();
         contentData; contentData = contentData->_nextContent)
    {
        if (contentData->_contentType == CONTENT_TEXT)
        {
            RenderText* t = new (renderArena()) RenderText( node(), 0);
            t->setIsAnonymous( true );
            t->setStyle(style);
            t->setText(contentData->contentText());
            addChild(t);
        }
        else if (contentData->_contentType == CONTENT_OBJECT)
        {
            RenderImage* img = new (renderArena()) RenderImage(node());
            img->setIsAnonymous( true );
            img->setStyle(style);
            img->setContentObject(contentData->contentObject());
            addChild(img);
        }
        else if (contentData->_contentType == CONTENT_COUNTER)
        {
            RenderCounter* t = new (renderArena()) RenderCounter( node(), contentData->contentCounter() );
            t->setIsAnonymous( true );
            t->setStyle(style);
            addChild(t);
        }
        else if (contentData->_contentType == CONTENT_QUOTE)
        {
            RenderQuote* t = new (renderArena()) RenderQuote( node(), contentData->contentQuote() );
            t->setIsAnonymous( true );
            t->setStyle(style);
            addChild(t);
        }
    }
    style->deref();
}

void RenderContainer::appendChildNode(RenderObject* newChild)
{
    KHTMLAssert(newChild->parent() == 0);

    newChild->setParent(this);
    RenderObject* lChild = lastChild();

    if(lChild)
    {
        newChild->setPreviousSibling(lChild);
        lChild->setNextSibling(newChild);
    }
    else
        setFirstChild(newChild);

    setLastChild(newChild);

    // Keep our layer hierarchy updated.  Optimize for the common case where we don't have any children
    // and don't have a layer attached to ourselves.
    if (newChild->firstChild() || newChild->layer()) {
        RenderLayer* layer = enclosingLayer();
        newChild->addLayers(layer, newChild);
    }

    newChild->setNeedsLayoutAndMinMaxRecalc(); // Goes up the containing block hierarchy.
    if (!normalChildNeedsLayout())
        setChildNeedsLayout(true); // We may supply the static position for an absolute positioned child.
}

void RenderContainer::insertChildNode(RenderObject* child, RenderObject* beforeChild)
{
    if(!beforeChild) {
        appendChildNode(child);
        return;
    }

    KHTMLAssert(!child->parent());
    while ( beforeChild->parent() != this && beforeChild->parent()->isAnonymousBlock() )
	beforeChild = beforeChild->parent();
    KHTMLAssert(beforeChild->parent() == this);

    if(beforeChild == firstChild())
        setFirstChild(child);

    RenderObject* prev = beforeChild->previousSibling();
    child->setNextSibling(beforeChild);
    beforeChild->setPreviousSibling(child);
    if(prev) prev->setNextSibling(child);
    child->setPreviousSibling(prev);
    child->setParent(this);

    // Keep our layer hierarchy updated.
    RenderLayer* layer = enclosingLayer();
    child->addLayers(layer, child);

    child->setNeedsLayoutAndMinMaxRecalc();
    if (!normalChildNeedsLayout())
        setChildNeedsLayout(true); // We may supply the static position for an absolute positioned child.
}


void RenderContainer::layout()
{
    KHTMLAssert( needsLayout() );
    KHTMLAssert( minMaxKnown() );
    const bool pagedMode = canvas()->pagedMode();
    RenderObject *child = firstChild();
    while( child ) {
        if (pagedMode) child->setNeedsLayout(true);
        child->layoutIfNeeded();
        if (child->containsPageBreak()) setContainsPageBreak(true);
        if (child->needsPageClear()) setNeedsPageClear(true);
        child = child->nextSibling();
    }
    setNeedsLayout(false);
}

void RenderContainer::removeLeftoverAnonymousBoxes()
{
    // we have to go over all child nodes and remove anonymous boxes, that do _not_
    // have inline children to keep the tree flat
    RenderObject *child = firstChild();
    while( child ) {
	RenderObject *next = child->nextSibling();

	if ( child->isRenderBlock() && child->isAnonymousBlock() && !child->continuation() &&
             !child->childrenInline() && !child->isTableCell() ) {
	    RenderObject *firstAnChild = child->firstChild();
	    RenderObject *lastAnChild = child->lastChild();
	    if ( firstAnChild ) {
		RenderObject *o = firstAnChild;
		while( o ) {
		    o->setParent( this );
		    o = o->nextSibling();
		}
		firstAnChild->setPreviousSibling( child->previousSibling() );
		lastAnChild->setNextSibling( child->nextSibling() );
		if ( child->previousSibling() )
		    child->previousSibling()->setNextSibling( firstAnChild );
		if ( child->nextSibling() )
		    child->nextSibling()->setPreviousSibling( lastAnChild );
	    } else {
		if ( child->previousSibling() )
		    child->previousSibling()->setNextSibling( child->nextSibling() );
		if ( child->nextSibling() )
		    child->nextSibling()->setPreviousSibling( child->previousSibling() );

	    }
	    if ( child == firstChild() )
		m_first = firstAnChild;
	    if ( child == lastChild() )
		m_last = lastAnChild;
	    child->setParent( 0 );
	    child->setPreviousSibling( 0 );
	    child->setNextSibling( 0 );
	    if ( !child->isText() ) {
		RenderContainer *c = static_cast<RenderContainer *>(child);
		c->m_first = 0;
		c->m_next = 0;
	    }
	    child->detach();
	}
	child = next;
    }
    if ( parent() )
	parent()->removeLeftoverAnonymousBoxes();
}

#undef DEBUG_LAYOUT
