/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
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

#include "dom_docimpl.h"
#include "dom_elementimpl.h"
#include "dom_textimpl.h"
#include "dom_exception.h"
using namespace DOM;


DocumentImpl::DocumentImpl() : NodeBaseImpl(0)
{
}

DocumentImpl::~DocumentImpl()
{
}

const DOMString DocumentImpl::nodeName() const
{
  return "#document";
}

unsigned short DocumentImpl::nodeType() const
{
    return Node::DOCUMENT_NODE;
}

ElementImpl *DocumentImpl::documentElement() const
{
    if(!firstChild() || firstChild()->nodeType() != Node::ELEMENT_NODE)
       return 0;
    return (ElementImpl *)firstChild();
}

ElementImpl *DocumentImpl::createElement( const DOMString & )
{
    // ### this will never get called for HTML
    throw DOMException(DOMException::NOT_SUPPORTED_ERR);
}

DocumentFragmentImpl *DocumentImpl::createDocumentFragment(  )
{
    return new DocumentFragmentImpl(this);
}

TextImpl *DocumentImpl::createTextNode( const DOMString &data )
{
    return new TextImpl(this, data);
}

AttrImpl *DocumentImpl::createAttribute( const DOMString &name )
{
    return new AttrImpl(this, name);
}

NodeListImpl *DocumentImpl::getElementsByTagName( const DOMString &tagname )
{
    // #### implement
}

// ----------------------------------------------------------------------------

DocumentFragmentImpl::DocumentFragmentImpl(DocumentImpl *doc) : NodeImpl(doc)
{
}

DocumentFragmentImpl::DocumentFragmentImpl(const DocumentFragmentImpl &other)
    : NodeImpl(other)
{
}

const DOMString DocumentFragmentImpl::nodeName() const
{
  return "#document-fragment";
}

unsigned short DocumentFragmentImpl::nodeType() const
{
    return Node::DOCUMENT_FRAGMENT_NODE;
}

