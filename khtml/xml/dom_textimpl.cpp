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
#include "dom_textimpl.h"

#include "dom_stringimpl.h"

#include "dom/dom_node.h"
#include "misc/htmlhashes.h"
#include "rendering/render_text.h"

using namespace DOM;
using namespace khtml;


CharacterDataImpl::CharacterDataImpl(DocumentImpl *doc) : NodeWParentImpl(doc)
{
    str = 0;
}

CharacterDataImpl::CharacterDataImpl(DocumentImpl *doc, const DOMString &_text)
    : NodeWParentImpl(doc)
{
    str = _text.impl;
    str->ref();
}

CharacterDataImpl::~CharacterDataImpl()
{
    if(str) str->deref();
}

DOMString CharacterDataImpl::data() const
{
    return str;
}

void CharacterDataImpl::setData( const DOMString &newStr )
{
    if(str == newStr.impl) return;
    if(str) str->deref();
    str = newStr.impl;
    if(str) str->ref();
    applyChanges();
}

unsigned long CharacterDataImpl::length() const
{
    return str->l;
}

DOMString CharacterDataImpl::substringData( const unsigned long /*offset*/, const unsigned long /*count*/ )
{
    // ###
    return 0;
}

void CharacterDataImpl::appendData( const DOMString &arg )
{
    str->append(arg.impl);
    applyChanges();
}

void CharacterDataImpl::insertData( const unsigned long offset, const DOMString &arg )
{
    str->insert(arg.impl, offset);
    applyChanges();
}

void CharacterDataImpl::deleteData( const unsigned long /*offset*/, const unsigned long /*count */)
{
    // ###
}

void CharacterDataImpl::replaceData( const unsigned long /*offset*/, const unsigned long /*count*/, const DOMString &/*arg*/ )
{
    // ###
}

// ---------------------------------------------------------------------------

TextImpl::TextImpl(DocumentImpl *doc, const DOMString &_text)
    : CharacterDataImpl(doc, _text)
{
    m_style = 0;
}

TextImpl::TextImpl(DocumentImpl *doc)
    : CharacterDataImpl(doc)
{
    m_style = 0;
}

TextImpl::~TextImpl()
{
    // we don't delete m_style, since it's just a pointer to the parents
    // style object
}

TextImpl *TextImpl::splitText( const unsigned long /*offset*/ )
{
    // ####
    return 0;
}

const DOMString TextImpl::nodeName() const
{
  return "#text";
}
DOMString TextImpl::nodeValue() const
{
    return str;
}


unsigned short TextImpl::nodeType() const
{
    return Node::TEXT_NODE;
}

void TextImpl::attach(KHTMLView *)
{
    m_style = parentNode()->style();
    RenderObject *r = _parent->renderer();
    if(r)
    {
	m_render = new RenderText(str);
	m_render->setStyle(m_style);
	r->addChild(m_render, _next ? _next->renderer() : 0);
	m_render->ref();
    }
}


void TextImpl::applyChanges(bool)
{
    m_style = parentNode()->style();
    if(m_render) m_render->setStyle(m_style);
}

bool TextImpl::mouseEvent( int _x, int _y, int, MouseEventType,
			   int _tx, int _ty, DOMString &,
                           NodeImpl *&innerNode, long &offset)
{
    //kdDebug( 6020 ) << "Text::mouseEvent" << endl;

    if(!m_render) return false;

    int off = 0;
    if( static_cast<RenderText *>(m_render)->checkPoint(_x, _y, _tx, _ty, off) )
    {
	offset = off;
	innerNode = this;
	return true;
    }
    return false;
}

ushort TextImpl::id() const
{
    return ID_TEXT;
}

