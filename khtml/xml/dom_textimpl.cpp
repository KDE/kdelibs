/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 2001-2003 Dirk Mueller (mueller@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
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
 */

#include "dom/dom_exception.h"
#include "css/cssstyleselector.h"
#include "xml/dom2_eventsimpl.h"
#include "xml/dom_textimpl.h"
#include "xml/dom_docimpl.h"

#include "misc/htmlhashes.h"
#include "rendering/render_text.h"
#include "rendering/render_flow.h"

#include <kdebug.h>

using namespace DOM;
using namespace khtml;

static DOMString escapeHTML( const DOMString& in )
{
    //FIXME: this is rather slow
    DOMString s;
    for ( unsigned int i = 0; i < in.length(); ++i ) {
        switch( in[i].latin1() ) {
        case '&':
            s += "&amp;";
            break;
        case '<':
            s += "&lt;";
            break;
        case '>':
            s += "&gt;";
            break;
        default:
            s += DOMString( in[i] );
        }
    }

    return s;
}

CharacterDataImpl::CharacterDataImpl(DocumentPtr *doc, DOMStringImpl* _text)
    : NodeImpl(doc)
{
    str = _text ? _text : new DOMStringImpl(0, 0);
    str->ref();
}

CharacterDataImpl::~CharacterDataImpl()
{
    if(str) str->deref();
}

void CharacterDataImpl::setData( const DOMString &_data, int &exceptioncode )
{
    // NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return;
    }

    if(str == _data.impl) return; // ### fire DOMCharacterDataModified if modified?
    DOMStringImpl *oldStr = str;
    str = _data.impl;
    if(str) str->ref();
    if (m_render)
      (static_cast<RenderText*>(m_render))->setText(str);
    setChanged(true);

    dispatchModifiedEvent(oldStr);
    if(oldStr) oldStr->deref();
}

unsigned long CharacterDataImpl::length() const
{
    return str->l;
}

DOMString CharacterDataImpl::substringData( const unsigned long offset, const unsigned long count, int &exceptioncode )
{
    exceptioncode = 0;
    if ((long)count < 0)
	exceptioncode = DOMException::INDEX_SIZE_ERR;
    else
	checkCharDataOperation(offset, exceptioncode);
    if (exceptioncode)
        return DOMString();

    return str->substring(offset,count);
}

void CharacterDataImpl::appendData( const DOMString &arg, int &exceptioncode )
{
    exceptioncode = 0;

    // NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return;
    }

    DOMStringImpl *oldStr = str;
    str = str->copy();
    str->ref();
    str->append(arg.impl);
    if (m_render)
      (static_cast<RenderText*>(m_render))->setText(str);
    setChanged(true);

    dispatchModifiedEvent(oldStr);
    oldStr->deref();
}

void CharacterDataImpl::insertData( const unsigned long offset, const DOMString &arg, int &exceptioncode )
{
    exceptioncode = 0;
    checkCharDataOperation(offset, exceptioncode);
    if (exceptioncode)
        return;

    DOMStringImpl *oldStr = str;
    str = str->copy();
    str->ref();
    str->insert(arg.impl, offset);
    if (m_render)
      (static_cast<RenderText*>(m_render))->setText(str);
    setChanged(true);

    dispatchModifiedEvent(oldStr);
    oldStr->deref();
}

void CharacterDataImpl::deleteData( const unsigned long offset, const unsigned long count, int &exceptioncode )
{
    exceptioncode = 0;
    if ((long)count < 0)
	exceptioncode = DOMException::INDEX_SIZE_ERR;
    else
	checkCharDataOperation(offset, exceptioncode);
    if (exceptioncode)
        return;

    DOMStringImpl *oldStr = str;
    str = str->copy();
    str->ref();
    str->remove(offset,count);
    if (m_render)
      (static_cast<RenderText*>(m_render))->setText(str);
    setChanged(true);

    dispatchModifiedEvent(oldStr);
    oldStr->deref();
}

void CharacterDataImpl::replaceData( const unsigned long offset, const unsigned long count, const DOMString &arg, int &exceptioncode )
{
    exceptioncode = 0;
    if ((long)count < 0)
	exceptioncode = DOMException::INDEX_SIZE_ERR;
    else
	checkCharDataOperation(offset, exceptioncode);
    if (exceptioncode)
        return;

    unsigned long realCount;
    if (offset + count > str->l)
        realCount = str->l-offset;
    else
        realCount = count;

    DOMStringImpl *oldStr = str;
    str = str->copy();
    str->ref();
    str->remove(offset,realCount);
    str->insert(arg.impl, offset);
    if (m_render)
      (static_cast<RenderText*>(m_render))->setText(str);
    setChanged(true);

    dispatchModifiedEvent(oldStr);
    oldStr->deref();
}

DOMString CharacterDataImpl::nodeValue() const
{
    return str;
}

bool CharacterDataImpl::containsOnlyWhitespace() const
{
    return str->containsOnlyWhitespace();
}

void CharacterDataImpl::setNodeValue( const DOMString &_nodeValue, int &exceptioncode )
{
    // NO_MODIFICATION_ALLOWED_ERR: taken care of by setData()
    setData(_nodeValue, exceptioncode);
}

void CharacterDataImpl::dispatchModifiedEvent(DOMStringImpl *prevValue)
{
    if (parentNode())
        parentNode()->childrenChanged();
    if (!getDocument()->hasListenerType(DocumentImpl::DOMCHARACTERDATAMODIFIED_LISTENER))
        return;

    DOMStringImpl *newValue = str->copy();
    newValue->ref();
    int exceptioncode = 0;
    MutationEventImpl* const evt = new MutationEventImpl(EventImpl::DOMCHARACTERDATAMODIFIED_EVENT,true,false,0,prevValue,newValue,DOMString(),0);
    evt->ref();
    dispatchEvent(evt,exceptioncode);
    evt->deref();
    newValue->deref();
    dispatchSubtreeModifiedEvent();
}

void CharacterDataImpl::checkCharDataOperation( const unsigned long offset, int &exceptioncode )
{
    exceptioncode = 0;

    // INDEX_SIZE_ERR: Raised if the specified offset is negative or greater than the number of 16-bit
    // units in data.
    if (offset > str->l) {
        exceptioncode = DOMException::INDEX_SIZE_ERR;
        return;
    }

    // NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return;
    }
}

long CharacterDataImpl::minOffset() const
{
  RenderText *r = static_cast<RenderText *>(renderer());
  if (!r || !r->isText()) return 0;

  // take :first-letter into consideration
#ifdef __GNUC__
#warning FIXME
#endif
#if 0
  if (r->forcedMinOffset()) {
    RenderFlow *firstLetter = static_cast<RenderFlow *>(r->previousSibling());
    if (firstLetter && firstLetter->isFlow() && firstLetter->isFirstLetter()) {
      RenderText *letterText = static_cast<RenderText *>(firstLetter->firstChild());
      return letterText->minOffset();
    }
  }
#endif

  return r->minOffset();
}

long CharacterDataImpl::maxOffset() const
{
  RenderText *r = static_cast<RenderText *>(renderer());
  if (!r || !r->isText()) return (long)length();
  return r->maxOffset();
}

// ---------------------------------------------------------------------------

DOMString CommentImpl::nodeName() const
{
    return "#comment";
}

unsigned short CommentImpl::nodeType() const
{
    return Node::COMMENT_NODE;
}

NodeImpl *CommentImpl::cloneNode(bool /*deep*/)
{
    return getDocument()->createComment( str );
}

NodeImpl::Id CommentImpl::id() const
{
    return ID_COMMENT;
}

// DOM Section 1.1.1
bool CommentImpl::childTypeAllowed( unsigned short /*type*/ )
{
    return false;
}

DOMString CommentImpl::toString() const
{
    // FIXME: substitute entity references as needed!
    return DOMString("<!--") + escapeHTML( nodeValue() ) + "-->";
}

// ---------------------------------------------------------------------------

TextImpl *TextImpl::splitText( const unsigned long offset, int &exceptioncode )
{
    exceptioncode = 0;

    // INDEX_SIZE_ERR: Raised if the specified offset is negative or greater than
    // the number of 16-bit units in data.

    // ### we explicitly check for a negative long that has been cast to an unsigned long
    // ... this can happen if JS code passes in -1 - we need to catch this earlier! (in the
    // kjs bindings)
    if (offset > str->l || (long)offset < 0) {
        exceptioncode = DOMException::INDEX_SIZE_ERR;
        return 0;
    }

    // NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return 0;
    }

    DOMStringImpl *oldStr = str;
    TextImpl *newText = createNew(str->substring(offset,str->l-offset));
    str = str->copy();
    str->ref();
    str->remove(offset,str->l-offset);

    dispatchModifiedEvent(oldStr);
    oldStr->deref();

    if (parentNode())
        parentNode()->insertBefore(newText,nextSibling(), exceptioncode );
    if ( exceptioncode )
        return 0;

    if (m_render)
        (static_cast<RenderText*>(m_render))->setText(str);
    setChanged(true);
    return newText;
}

DOMString TextImpl::nodeName() const
{
  return "#text";
}

unsigned short TextImpl::nodeType() const
{
    return Node::TEXT_NODE;
}

NodeImpl *TextImpl::cloneNode(bool /*deep*/)
{
    return getDocument()->createTextNode(str);
}

void TextImpl::attach()
{
    assert(!m_render);
    assert(!attached());
    assert(parentNode() && parentNode()->isElementNode());

    ElementImpl* element = static_cast<ElementImpl*>(parentNode());
    if (element->renderer() && element->renderer()->childAllowed()) {
        khtml::RenderStyle* _style = element->renderer()->style();
        m_render = new (getDocument()->renderArena()) RenderText(this, str);
        m_render->setStyle(_style);
        parentNode()->renderer()->addChild(m_render, nextRenderer());
    }

    CharacterDataImpl::attach();
}

NodeImpl::Id TextImpl::id() const
{
    return ID_TEXT;
}

void TextImpl::recalcStyle( StyleChange change )
{
//      qDebug("textImpl::recalcStyle");
    if (change != NoChange && parentNode()) {
// 	qDebug("DomText::recalcStyle");
	if(m_render)
	    m_render->setStyle(parentNode()->renderer()->style());
    }
    if ( changed() && m_render && m_render->isText() )
	static_cast<RenderText*>(m_render)->setText(str);
    setChanged( false );
}

// DOM Section 1.1.1
bool TextImpl::childTypeAllowed( unsigned short /*type*/ )
{
    return false;
}

TextImpl *TextImpl::createNew(DOMStringImpl *_str)
{
    return new TextImpl(docPtr(),_str);
}

DOMString TextImpl::toString() const
{
    // FIXME: substitute entity references as needed!
    return escapeHTML( nodeValue() );
}

// ---------------------------------------------------------------------------

DOMString CDATASectionImpl::nodeName() const
{
  return "#cdata-section";
}

unsigned short CDATASectionImpl::nodeType() const
{
    return Node::CDATA_SECTION_NODE;
}

NodeImpl *CDATASectionImpl::cloneNode(bool /*deep*/)
{
    return getDocument()->createCDATASection(str);
}

// DOM Section 1.1.1
bool CDATASectionImpl::childTypeAllowed( unsigned short /*type*/ )
{
    return false;
}

TextImpl *CDATASectionImpl::createNew(DOMStringImpl *_str)
{
    return new CDATASectionImpl(docPtr(),_str);
}

DOMString CDATASectionImpl::toString() const
{
    // FIXME: substitute entity references as needed!
    return DOMString("<![CDATA[") + nodeValue() + "]]>";
}



