/**
 * This file is part of the html renderer for KDE.
 *
 * Copyright (C) 2001-2003 Lars Knoll (knoll@kde.org)
 *           (C) 2001 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000-2003 Dirk Mueller (mueller@kde.org)
 *           (C) 2002 Apple Computer, Inc.
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

//#define DEBUG_LAYOUT

#include "rendering/render_container.h"
#include "rendering/render_table.h"
#include "rendering/render_text.h"
#include "rendering/render_image.h"
#include "rendering/render_canvas.h"
#include "xml/dom_docimpl.h"

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
    kdDebug( 6040 ) << this << ": " <<  renderName() << "(RenderObject)::addChild( " << newChild << ": " <<
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

            //kdDebug( 6040 ) << "adding section" << endl;
            if ( !isTable() )
                needsTable = true;
            break;
        case TABLE_ROW:
            //kdDebug( 6040 ) << "adding row" << endl;
            if ( !isTableSection() )
                needsTable = true;
            break;
        case TABLE_CELL:
            //kdDebug( 6040 ) << "adding cell" << endl;
            if ( !isTableRow() )
                needsTable = true;
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
	    //kdDebug( 6040 ) << "creating anonymous table, before=" << beforeChild << endl;
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
    newChild->setLayouted( false );
    newChild->setMinMaxKnown( false );
}

RenderObject* RenderContainer::removeChildNode(RenderObject* oldChild)
{
    KHTMLAssert(oldChild->parent() == this);

    if ( document()->renderer() ) {

        oldChild->setLayouted( false );
        oldChild->setMinMaxKnown( false );
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

    setLayouted( false );
    setMinMaxKnown( false );

    return oldChild;
}

void RenderContainer::updatePseudoChild(RenderStyle::PseudoId type, RenderObject* child)
{
    // In CSS2, before/after pseudo-content cannot nest.  Check this first.
    if (style()->styleType() == RenderStyle::BEFORE || style()->styleType() == RenderStyle::AFTER)
        return;


    RenderStyle* pseudo = style()->getPseudoStyle(type);

    // Whether or not we currently have generated content attached.
    bool oldContentPresent = child && (child->style()->styleType() == type);

    // Whether or not we now want generated content.
    bool newContentWanted = pseudo && pseudo->display() != NONE;

    // For <q><p/></q>, if this object is the inline continuation of the <q>, we only want to generate
    // :after content and not :before content.
    if (type == RenderStyle::BEFORE && isInlineContinuation())
        newContentWanted = false;

    // Similarly, if we're the beginning of a <q>, and there's an inline continuation for our object,
    // then we don't generate the :after content.
    if (type == RenderStyle::AFTER && isRenderInline() && continuation())
        newContentWanted = false;

    // If we don't want generated content any longer, or if we have generated content, but it's no longer
    // identical to the new content data we want to build render objects for, then we nuke all
    // of the old generated content.
    if (!newContentWanted ||
        (oldContentPresent && !child->style()->contentDataEquivalent(pseudo))) {
        if (child && child->style()->styleType() == type) {
            // The child needs to be removed.
            oldContentPresent = false;
            removeChild(child);
            child = (type == RenderStyle::BEFORE) ? firstChild() : lastChild();
        }
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

    if (oldContentPresent) {
        if (child && child->style()->styleType() == type) {
            // We have generated content present still.  We want to walk this content and update our
            // style information with the new pseudo style.
            child->setStyle(pseudo);

            // Note that if we ever support additional types of generated content (which should be way off
            // in the future), this code will need to be patched.
            for (RenderObject* genChild = child->firstChild(); genChild; genChild = genChild->nextSibling()) {
                if (genChild->style()->styleType() == RenderStyle::FIRST_LETTER) {
                    // We can't properly update, so remove the child and bail
                    oldContentPresent = false;
                    removeChild(child);
                    child = (type == RenderStyle::BEFORE) ? firstChild() : lastChild();
                    break;
                } else if (genChild->isText()) {
                    // Generated text content is a child whose style also needs to be set to the pseudo
                    // style.
                    genChild->setStyle(pseudo);
                } else {
                    // Images get an empty style that inherits from the pseudo.
                    RenderStyle* style = new RenderStyle();
                    style->inheritFrom(pseudo);
                    genChild->setStyle(style);
                }
            }
        }
        if (oldContentPresent)
            return; // We've updated the generated content. That's all we needed to do.
    }

    RenderObject* insertBefore = (type == RenderStyle::BEFORE) ? child : 0;

    // Generated content consists of a single container that houses multiple children (specified
    // by the content property).  This pseudo container gets the pseudo style set on it.
    RenderObject* pseudoContainer = 0;

    // Now walk our list of generated content and create render objects for every type
    // we encounter.
    for (ContentData* contentData = pseudo->contentData();
         contentData; contentData = contentData->_nextContent) {
        if (!pseudoContainer)
            pseudoContainer = RenderFlow::createFlow(document(), pseudo, renderArena()); /* anonymous box */

        if (contentData->_contentType == CONTENT_TEXT)
        {
            RenderText* t = new (renderArena()) RenderText( document() /*anonymous object */, contentData->contentText());
            t->setStyle(pseudo);
            pseudoContainer->addChild(t);
        }
        else if (contentData->_contentType == CONTENT_OBJECT)
        {
            RenderImage* img = new (renderArena()) RenderImage(document()); /* Anonymous object */
            RenderStyle* style = new RenderStyle();
            style->inheritFrom(pseudo);
            img->setStyle(style);
            img->setContentObject(contentData->contentObject());
            pseudoContainer->addChild(img);
        }
    }

    if (pseudoContainer) {
        // Add the pseudo after we've installed all our content, so that addChild will be able to find the text
        // inside the inline for e.g., first-letter styling.
        addChild(pseudoContainer, insertBefore);
        pseudoContainer->close();
    }
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

    // Keep our layer hierarchy updated.
    RenderLayer* layer = enclosingLayer();
    newChild->addLayers(layer, findNextLayer(layer, newChild));

    newChild->setLayouted( false );
    newChild->setMinMaxKnown( false );
}

void RenderContainer::insertChildNode(RenderObject* child, RenderObject* beforeChild)
{
    if(!beforeChild) {
        appendChildNode(child);
        return;
    }

    KHTMLAssert(!child->parent());
    while ( beforeChild->parent() != this && beforeChild->parent()->isAnonymous() )
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
    child->addLayers(layer, findNextLayer(layer, child));

    child->setLayouted( false );
    child->setMinMaxKnown( false );
}


void RenderContainer::layout()
{
    KHTMLAssert( !layouted() );
    KHTMLAssert( minMaxKnown() );

    RenderObject *child = firstChild();
    while( child ) {
	if( !child->layouted() )
	    child->layout();
	child = child->nextSibling();
    }
    setLayouted();
}

void RenderContainer::removeLeftoverAnonymousBoxes()
{
    // we have to go over all child nodes and remove anonymous boxes, that do _not_
    // have inline children to keep the tree flat
    RenderObject *child = firstChild();
    while( child ) {
	RenderObject *next = child->nextSibling();

	if ( child->isRenderBlock() && child->isAnonymous() && !child->continuation() &&
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
