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

#include "dom_node.h"
#include "dom_doc.h"
#include "dom_exception.h"
#include "dom_string.h"
#include "dom_nodeimpl.h"
#include "dom_elementimpl.h"
using namespace DOM;


NamedNodeMap::NamedNodeMap()
{
    map = 0;
}

NamedNodeMap::NamedNodeMap(const NamedNodeMap &other)
{
    map = other.map;
    if(map) map->ref();
}

NamedNodeMap::NamedNodeMap(NodeImpl *i)
{
    map = i;
    if(map) map->ref();
}

NamedNodeMap &NamedNodeMap::operator = (const NamedNodeMap &other)
{
    if(map) map->deref();
    map = other.map;
    if(map) map->ref();

    return *this;
}

NamedNodeMap::~NamedNodeMap()
{
    if(map) map->deref();
}

Node NamedNodeMap::getNamedItem( const DOMString &name )
{
    if(map->nodeType() != Node::ELEMENT_NODE)
	return 0;

    return (NodeImpl *)((ElementImpl *)map)->getAttributeNode( name );
}

Node NamedNodeMap::setNamedItem( const Node &arg )
{
    if(map->nodeType() != Node::ELEMENT_NODE)
	throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR);

    // ### check it's really an attribute

    return (NodeImpl *)((ElementImpl *)map)->setAttributeNode( (AttrImpl *)arg.impl );
}

Node NamedNodeMap::removeNamedItem( const DOMString &name )
{
    if(map->nodeType() != Node::ELEMENT_NODE)
	throw DOMException(DOMException::NOT_FOUND_ERR);

    AttrImpl *n = ((ElementImpl *)map)->getAttributeNode ( name );
    if(!n) throw DOMException(DOMException::NOT_FOUND_ERR);

    return (NodeImpl *)((ElementImpl *)map)->removeAttributeNode( n );
}

Node NamedNodeMap::item( unsigned long /*index*/ )
{
    // ####
    return 0;
}

unsigned long NamedNodeMap::length() const
{
    // ####
    return 0;
}

// ---------------------------------------------------------------------------

Node::Node()
{
    impl = 0;
}

Node::Node(const Node &other)
{
    impl = other.impl;
    if(impl) impl->ref();
}

Node::Node( NodeImpl *i )
{
    impl = i;
    if(impl) impl->ref();
}

Node &Node::operator = (const Node &other)
{
    if(impl == other.impl) return *this;
    if(impl) impl->deref();
    impl = other.impl;
    if(impl) impl->ref();
    return *this;
}

Node::~Node()
{
    if(impl) impl->deref();
}

DOMString Node::nodeName() const
{
    if(impl) return impl->nodeName();
    return 0;
}

DOMString Node::nodeValue() const
{
    if(impl) return impl->nodeValue();
    return 0;
}

void Node::setNodeValue( const DOMString &_str )
{
    if(impl) impl->setNodeValue( _str );
    else throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR);
}

unsigned short Node::nodeType() const
{
    if(impl) return impl->nodeType();
    return 0;
}

Node Node::parentNode() const
{
    if(impl) return impl->parentNode();
    return 0;
}

NodeList Node::childNodes() const
{
    if(impl) return impl->childNodes();
    return 0;
}

Node Node::firstChild() const
{
    if(impl) return impl->firstChild();
    return 0;
}

Node Node::lastChild() const
{
    if(impl) return impl->lastChild();
    return 0;
}

Node Node::previousSibling() const
{
    if(impl) return impl->previousSibling();
    return 0;
}

Node Node::nextSibling() const
{
    if(impl) return impl->nextSibling();
    return 0;
}

NamedNodeMap Node::attributes() const
{
    return NamedNodeMap(impl);
}

Document Node::ownerDocument() const
{
    if(impl) return impl->ownerDocument();
    // ### FIXME: this creates and returns a new Document, not 0
    return Document();
}

Node Node::insertBefore( const Node &newChild, const Node &refChild )
{
    if(impl) return impl->insertBefore( newChild.impl, refChild.impl );
    throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR);
}

Node Node::replaceChild( const Node &newChild, const Node &oldChild )
{
    if(impl) return impl->replaceChild( newChild.impl, oldChild.impl );
    throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR);
}

Node Node::removeChild( const Node &oldChild )
{
    if(impl) return impl->removeChild( oldChild.impl );
    throw DOMException(DOMException::NOT_FOUND_ERR);
}

Node Node::appendChild( const Node &newChild )
{
    if(impl) return impl->appendChild( newChild.impl );
    throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR);
}

bool Node::hasChildNodes(  )
{
    if(impl) return impl->hasChildNodes();
    return false;
}

Node Node::cloneNode( bool deep )
{
    if(impl) return impl->cloneNode( deep );
    return 0;
}

//-----------------------------------------------------------------------------

NodeList::NodeList()
{
    impl = 0;
}

NodeList::NodeList(const NodeList &other)
{
    impl = other.impl;
    if(impl) impl->ref();
}

NodeList::NodeList(const NodeListImpl *i)
{
    impl = (NodeListImpl *)i;
    if(impl) impl->ref();
}

NodeList &NodeList::operator = (const NodeList &other)
{
    if(impl) impl->deref();
    impl = other.impl;
    if(impl) impl->ref();
    return *this;
}

NodeList::~NodeList()
{
    if(impl) impl->deref();
}

Node NodeList::item( unsigned long /*index*/ )
{
    // ###
}

unsigned long NodeList::length() const
{
    // ###
    return 0;
}
