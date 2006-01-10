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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "dom/dom_doc.h"
#include "dom/dom_exception.h"
#include "dom/dom2_events.h"
#include "xml/dom_docimpl.h"
#include "xml/dom_elementimpl.h"
#include "xml/dom2_eventsimpl.h"

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
    if ( impl != other.impl ) {
        if(impl) impl->deref();
        impl = other.impl;
        if(impl) impl->ref();
    }
    return *this;
}

NamedNodeMap::~NamedNodeMap()
{
    if(impl) impl->deref();
}

Node NamedNodeMap::getNamedItem( const DOMString &name ) const
{
    if (!impl) return 0;
    return impl->getNamedItem(name);
}

Node NamedNodeMap::setNamedItem( const Node &arg )
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    
    int exceptioncode = 0;
    Node r = impl->setNamedItem(arg, exceptioncode);
    if (exceptioncode)
        throw DOMException(exceptioncode);
    return r;
}

Node NamedNodeMap::removeNamedItem( const DOMString &name )
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    int exceptioncode = 0;
    Node r = impl->removeNamedItem(name, exceptioncode);
    if (exceptioncode)
        throw DOMException(exceptioncode);
    return r;
}

Node NamedNodeMap::item( unsigned long index ) const
{
    if (!impl) return 0;
    return impl->item(index);
}

Node NamedNodeMap::getNamedItemNS( const DOMString &namespaceURI, const DOMString &localName ) const
{
    if (!impl) return 0;
    return impl->getNamedItemNS(namespaceURI, localName);
}

Node NamedNodeMap::setNamedItemNS( const Node &arg )
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    int exceptioncode = 0;
    Node r = impl->setNamedItemNS(arg, exceptioncode);
    if (exceptioncode)
        throw DOMException(exceptioncode);
    return r;
}

Node NamedNodeMap::removeNamedItemNS( const DOMString &namespaceURI, const DOMString &localName )
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    int exceptioncode = 0;
    Node r = impl->removeNamedItemNS(namespaceURI, localName, exceptioncode);
    if (exceptioncode)
        throw DOMException(exceptioncode);
    return r;
}

unsigned long NamedNodeMap::length() const
{
    if (!impl) return 0;
    return impl->length();
}

// ---------------------------------------------------------------------------

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
    if(impl != other.impl) {
        if(impl) impl->deref();
        impl = other.impl;
        if(impl) impl->ref();
    }
    return *this;
}

bool Node::operator == (const Node &other) const
{
    return (impl == other.impl);
}

bool Node::operator != (const Node &other) const
{
    return !(impl == other.impl);
}

Node::~Node()
{
    if(impl) impl->deref();
}

DOMString Node::nodeName() const
{
    if(impl) return impl->nodeName();
    return DOMString();
}

DOMString Node::nodeValue() const
{
    // ### should throw exception on plain node ?
    if(impl) return impl->nodeValue();
    return DOMString();
}

void Node::setNodeValue( const DOMString &_str )
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);

    int exceptioncode = 0;
    if(impl) impl->setNodeValue( _str,exceptioncode );
    if (exceptioncode)
	throw DOMException(exceptioncode);
}

unsigned short Node::nodeType() const
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    return impl->nodeType();
}

Node Node::parentNode() const
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    return impl->parentNode();
}

NodeList Node::childNodes() const
{
    if (!impl) return 0;
    return impl->childNodes();
}

Node Node::firstChild() const
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    return impl->firstChild();
}

Node Node::lastChild() const
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    return impl->lastChild();
}

Node Node::previousSibling() const
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    return impl->previousSibling();
}

Node Node::nextSibling() const
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    return impl->nextSibling();
}

NamedNodeMap Node::attributes() const
{
    if (!impl || !impl->isElementNode()) return 0;
    return static_cast<ElementImpl*>(impl)->attributes();
}

Document Node::ownerDocument() const
{
    if (!impl || !impl->ownerDocument())
        return Document(false);
    return impl->ownerDocument();
}

Node Node::insertBefore( const Node &newChild, const Node &refChild )
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    int exceptioncode = 0;
    NodeImpl *r = impl->insertBefore( newChild.impl, refChild.impl, exceptioncode );
    if (exceptioncode)
	throw DOMException(exceptioncode);
    if (!newChild.impl->closed()) newChild.impl->close();
    return r;
}

Node Node::replaceChild( const Node &newChild, const Node &oldChild )
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    int exceptioncode = 0;
    NodeImpl *r = impl->replaceChild( newChild.impl, oldChild.impl, exceptioncode );
    if (exceptioncode)
	throw DOMException(exceptioncode);
    if (!newChild.impl->closed()) newChild.impl->close();
    return r;
}

Node Node::removeChild( const Node &oldChild )
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    int exceptioncode = 0;
    NodeImpl *r = impl->removeChild( oldChild.impl, exceptioncode );
    if (exceptioncode)
	throw DOMException(exceptioncode);
    return r;
}

Node Node::appendChild( const Node &newChild )
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    int exceptioncode = 0;
    NodeImpl *r = impl->appendChild( newChild.impl, exceptioncode );
    if (exceptioncode)
	throw DOMException(exceptioncode);
    if (!newChild.impl->closed()) newChild.impl->close();
    return r;
}

bool Node::hasAttributes()
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    return impl->hasAttributes();
}

bool Node::hasChildNodes(  )
{
    if (!impl) return false;
    return impl->hasChildNodes();
}

Node Node::cloneNode( bool deep )
{
    if (!impl) return 0;
    return impl->cloneNode( deep  );
}

void Node::normalize (  )
{
    if (!impl) return;
    impl->normalize();
}

bool Node::isSupported( const DOMString &feature,
                        const DOMString &version ) const
{
    return NodeImpl::isSupported(feature, version);
}

DOMString Node::namespaceURI(  ) const
{
    if (!impl) return DOMString();
    return impl->namespaceURI();
}

DOMString Node::prefix(  ) const
{
    if (!impl) return DOMString();
    return impl->prefix();
}

void Node::setPrefix(const DOMString &prefix )
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    int exceptioncode = 0;
    impl->setPrefix(prefix,exceptioncode);
    if (exceptioncode)
        throw DOMException(exceptioncode);
}

DOMString Node::localName(  ) const
{
    if (!impl) return DOMString();
    return impl->localName();
}

void Node::addEventListener(const DOMString &type,
			  EventListener *listener,
			  const bool useCapture)
{
    if (!impl) return;
    if (listener)
        impl->addEventListener(EventImpl::typeToId(type),listener,useCapture);
}

void Node::removeEventListener(const DOMString &type,
			     EventListener *listener,
			     bool useCapture)
{
    if (!impl) return;
    impl->removeEventListener(EventImpl::typeToId(type),listener,useCapture);
}

bool Node::dispatchEvent(const Event &evt)
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    if (!evt.handle())
        throw DOMException(DOMException::NOT_FOUND_ERR);

    int exceptioncode = 0;
    impl->dispatchEvent(evt.handle(),exceptioncode);
    if (exceptioncode)
	throw DOMException(exceptioncode);
    return !evt.handle()->defaultPrevented();
}


unsigned int Node::elementId() const
{
    if (!impl) return 0;
    return impl->id();
}

unsigned long Node::index() const
{
    if (!impl) return 0;
    return impl->nodeIndex();
}

QString Node::toHTML()
{
    if (!impl) return QString();
    return impl->toString().string();
}

void Node::applyChanges()
{
    if (!impl) return;
    impl->recalcStyle( NodeImpl::Inherit );
}

void Node::getCursor(int offset, int &_x, int &_y, int &height)
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    int dummy;
    impl->getCaret(offset, false, _x, _y, dummy, height);
}

QRect Node::getRect()
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    return impl->getRect();
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
    if ( impl != other.impl ) {
        if(impl) impl->deref();
        impl = other.impl;
        if(impl) impl->ref();
    }
    return *this;
}

NodeList::~NodeList()
{
    if(impl) impl->deref();
}

Node NodeList::item( unsigned long index ) const
{
    if (!impl) return 0;
    return impl->item(index);
}

unsigned long NodeList::length() const
{
    if (!impl) return 0;
    return impl->length();
}
