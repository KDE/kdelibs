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
#include <qstring.h>
#include <qrect.h>
using namespace DOM;


NamedNodeMap::NamedNodeMap()
{
    impl = 0;
}

NamedNodeMap::NamedNodeMap(const NamedNodeMap &other)
{
    impl = other.impl;
    if (impl) impl->ref();
}

NamedNodeMap::NamedNodeMap(NamedNodeMapImpl *i)
{
    impl = i;
    if (impl) impl->ref();
}

NamedNodeMap &NamedNodeMap::operator = (const NamedNodeMap &other)
{
    if(impl) impl->deref();
    impl = other.impl;
    if(impl) impl->ref();

    return *this;
}

NamedNodeMap::~NamedNodeMap()
{
    if(impl) impl->deref();
}

Node NamedNodeMap::getNamedItem( const DOMString &name )
{
    if (impl) return impl->getNamedItem(name);
    return 0;
}

Node NamedNodeMap::setNamedItem( const Node &arg )
{
    if (impl) return impl->setNamedItem(arg);
    return 0;
}

Node NamedNodeMap::removeNamedItem( const DOMString &name )
{
    if (impl) return impl->removeNamedItem(name);
    return 0;
}

Node NamedNodeMap::item( unsigned long index )
{
    if (impl) return impl->item(index);
    return 0;
}

unsigned long NamedNodeMap::length() const
{
    if (impl) return impl->length();
    return 0;
}

NamedNodeMapImpl *NamedNodeMap::handle() const
{
    return impl;
}

bool NamedNodeMap::isNull() const
{
    return (impl == 0);
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

bool Node::operator == (const Node &other)
{
    if(impl == other.impl)
        return true;
    else
        return false;
}

bool Node::operator != (const Node &other)
{
    if(impl != other.impl)
        return true;
    else
        return false;
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
    if (impl) return impl->attributes();
    return 0;
}

Document Node::ownerDocument() const
{
    if(impl) return impl->ownerDocument();
    // create an nonexistant Document (document == 0 should return true)
    return Document(false);
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

unsigned short Node::elementId() const
{
    if(!impl) return 0;
    return impl->id();
}

unsigned long Node::index() const
{
    Node _tempNode = previousSibling();
    unsigned long count=0;
    for( count=0; !_tempNode.isNull(); count++ )
        _tempNode = _tempNode.previousSibling();
    return count;
}

QString Node::toHTML()
{
    if(impl) return ( impl->toHTML() );
        return 0;
}

void Node::applyChanges()
{
    if(!impl) return;
    impl->applyChanges();
}

void Node::getCursor(int offset, int &_x, int &_y, int &height)
{
    if(!impl) return;
    impl->getCursor(offset, _x, _y, height);
}

QRect Node::getRect()
{
    if(!impl) return QRect();
    return impl->getRect();
}

bool Node::isNull() const
{
    return (impl == 0);
}

NodeImpl *Node::handle() const
{
    return impl;
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
    impl = const_cast<NodeListImpl *>(i);
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

Node NodeList::item( unsigned long index )
{
    if(!impl) return 0;
    return impl->item(index);
}

unsigned long NodeList::length() const
{
    if(!impl) return 0;
    return impl->length();
}

NodeListImpl *NodeList::handle() const
{
    return impl;
}

bool NodeList::isNull() const
{
    return (impl == 0);
}

