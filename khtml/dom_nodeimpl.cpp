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
#include "dtd.h"

#include "dom_nodeimpl.h"

#include <stdio.h>

using namespace DOM;


NodeImpl::NodeImpl(DocumentImpl *doc)
{
    document = doc;
    flags = 0;
}

NodeImpl::~NodeImpl()
{
}

DOMString NodeImpl::nodeValue() const
{
    return 0;
}

void NodeImpl::setNodeValue( const DOMString & )
{
    throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR);
}

const DOMString NodeImpl::nodeName() const
{
    return 0;
}

unsigned short NodeImpl::nodeType() const
{
    return 0;
}

NodeImpl *NodeImpl::parentNode() const
{
    return 0;
}

NodeListImpl *NodeImpl::childNodes()
{
    return 0;
}

NodeImpl *NodeImpl::firstChild() const
{
    return 0;
}

NodeImpl *NodeImpl::lastChild() const
{
    return 0;
}

NodeImpl *NodeImpl::previousSibling() const
{
    return 0;
}

NodeImpl *NodeImpl::nextSibling() const
{
    return 0;
}

void NodeImpl::setPreviousSibling(NodeImpl *)
{
}

void NodeImpl::setNextSibling(NodeImpl *)
{
}

NodeImpl *NodeImpl::insertBefore( NodeImpl *, NodeImpl * )
{
    throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);
}

NodeImpl *NodeImpl::replaceChild( NodeImpl *, NodeImpl * )
{
    throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);
}

NodeImpl *NodeImpl::removeChild( NodeImpl * )
{
    throw DOMException(DOMException::NOT_FOUND_ERR);
}

NodeImpl *NodeImpl::appendChild( NodeImpl * )
{
    throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);
}

bool NodeImpl::hasChildNodes(  )
{
    return false;
}

NodeImpl *NodeImpl::cloneNode( bool )
{
    // we have no childs, so we just clone this Node
    return new NodeImpl(document);
}

// helper functions not being part of the DOM
void NodeImpl::setParent(NodeImpl *)
{
}

void NodeImpl::setFirstChild(NodeImpl *)
{
}

void NodeImpl::setLastChild(NodeImpl *)
{
}

NodeImpl *NodeImpl::addChild(NodeImpl *)
{
    throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);
    return 0;
}

// ------------------------------------------------------------------------

NodeWParentImpl::NodeWParentImpl(DocumentImpl *doc) : NodeImpl(doc)
{
    _parent = 0;
    _previous = 0;
    _next = 0;
}

NodeWParentImpl::~NodeWParentImpl()
{
    // ### previous and next node may still reference this!!!
    // hope this fix is fine...
    if(_previous) _previous->setNextSibling(0);
    if(_next) _next->setPreviousSibling(0);
}

NodeImpl *NodeWParentImpl::parentNode() const
{
    return _parent;
}

NodeImpl *NodeWParentImpl::previousSibling() const
{
    return _previous;
}

NodeImpl *NodeWParentImpl::nextSibling() const
{
    return _next;
}

NodeImpl *NodeWParentImpl::cloneNode( bool )
{
    // we have no childs, so we just clone this Node
    // the parent is set to 0 anyway...
    return new NodeWParentImpl(document);
}

// not part of the DOM
void NodeWParentImpl::setParent(NodeImpl *n)
{
    _parent = n;
}

bool NodeWParentImpl::deleteMe()
{
    if(!_parent) return true;
    return false;
}

void NodeWParentImpl::setPreviousSibling(NodeImpl *n)
{
    if(!_parent)
	throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);

    _previous = n;
}

void NodeWParentImpl::setNextSibling(NodeImpl *n)
{
    if(!_parent)
	throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);

    _next = n;
}

void NodeWParentImpl::checkReadOnly()
{
}

//-------------------------------------------------------------------------

NodeBaseImpl::NodeBaseImpl(DocumentImpl *doc) : NodeWParentImpl(doc)
{
    _first = _last = 0;
}

NodeBaseImpl::~NodeBaseImpl()
{
    //printf("NodeBaseImpl destructor\n");
    // we have to tell all children, that the parent has died...
    NodeImpl *n;
    NodeImpl *next;

    for( n = _first; n != 0; n = next )
    {
	n->setParent(0);
	next = n->nextSibling();
	if(n->deleteMe()) delete n;
    }
}

NodeListImpl *NodeBaseImpl::childNodes()
{
    return new ChildNodeListImpl(this);
}

NodeImpl *NodeBaseImpl::firstChild() const
{
    return _first;
}

NodeImpl *NodeBaseImpl::lastChild() const
{
    return _last;
}

NodeImpl *NodeBaseImpl::insertBefore ( NodeImpl *newChild, NodeImpl *refChild )
{
    checkReadOnly();

    if(!refChild)
	return appendChild(newChild);

    // ### check for DocumentFragment

    checkSameDocument(newChild);
    checkIsChild(refChild);
    checkNoOwner(newChild);

    // seems ok, lets's insert it.

    NodeImpl *prev = refChild->previousSibling();

    if (prev)
      prev->setNextSibling(newChild);
    refChild->setPreviousSibling(newChild);
    newChild->setParent(this);
    newChild->setPreviousSibling(prev);
    newChild->setNextSibling(refChild);

    return newChild;
}

NodeImpl *NodeBaseImpl::replaceChild ( NodeImpl *newChild, NodeImpl *oldChild )
{
    checkReadOnly();
    checkSameDocument(newChild);
    checkIsChild(oldChild);
    checkNoOwner(newChild);

    // seems ok, lets's insert it.

    NodeImpl *prev = oldChild->previousSibling();
    NodeImpl *next = oldChild->nextSibling();

    if (prev)
      prev->setNextSibling(newChild);
    if (next)
      next->setPreviousSibling(newChild);
    oldChild->setPreviousSibling(0);
    oldChild->setNextSibling(0);
    oldChild->setParent(0);
    newChild->setParent(this);
    newChild->setPreviousSibling(prev);
    newChild->setNextSibling(next);

    return oldChild;
}

NodeImpl *NodeBaseImpl::removeChild ( NodeImpl *oldChild )
{
    checkReadOnly();
    checkIsChild(oldChild);

    NodeImpl *prev, *next;
    prev = oldChild->previousSibling();
    next = oldChild->nextSibling();

    next->setPreviousSibling(prev);
    prev->setNextSibling(next);

    oldChild->setParent(0);

    return oldChild;
}

NodeImpl *NodeBaseImpl::appendChild ( NodeImpl *newChild )
{
    checkReadOnly();
    checkSameDocument(newChild);
    checkNoOwner(newChild);

    newChild->setParent(this);

    if(_last)
    {
	newChild->setPreviousSibling(_last);
	_last->setNextSibling(newChild);
	_last = newChild;
    }
    else
    {
	_first = _last = newChild;
    }
    return newChild;
}

bool NodeBaseImpl::hasChildNodes (  )
{
    return _first != 0;
}

NodeImpl *NodeBaseImpl::cloneNode ( bool deep )
{
    NodeImpl *newImpl = new NodeBaseImpl(document);

    newImpl->setParent(0);
    newImpl->setFirstChild(0);
    newImpl->setLastChild(0);

    if(deep)
    {
	NodeImpl *n;
	for(n = firstChild(); n != lastChild(); n = n->nextSibling())
	{
	    newImpl->appendChild(n->cloneNode(deep));
	}
    }
    return newImpl;
}


// not part of the DOM
void NodeBaseImpl::setFirstChild(NodeImpl *child)
{
    _first = child;
}

void NodeBaseImpl::setLastChild(NodeImpl *child)
{
    _last = child;
}

// check for same source document:
void NodeBaseImpl::checkSameDocument( NodeImpl *newChild )
{
  if(newChild->ownerDocument() != document)
    throw DOMException(DOMException::WRONG_DOCUMENT_ERR);
}

// check for being (grand-..)father:
void NodeBaseImpl::checkNoOwner( NodeImpl *newChild )
{
  //check if newChild is parent of this...
  NodeImpl *n;
  for( n = this; n != (NodeImpl *)document; n = n->parentNode() )
    if(n == newChild)
      throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);

  if(newChild->parentNode() == this)
    removeChild(newChild);
}

// check for being child:
void NodeBaseImpl::checkIsChild( NodeImpl *oldChild )
{
    if(oldChild->parentNode() != this)
	throw DOMException(DOMException::NOT_FOUND_ERR);
}

NodeImpl *NodeBaseImpl::addChild(NodeImpl *newChild)
{
    // short check for consistency with DTD
    if(!checkChild(id(), newChild->id()))
    {
        //printf("AddChild failed! id=%d, child->id=%d\n", id(), newChild->id());
	throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);
    }

    newChild->setParsing();

    // just add it...
    newChild->setParent(this);

    if(_last)
    {
	newChild->setPreviousSibling(_last);
	_last->setNextSibling(newChild);
	_last = newChild;
    }
    else
    {
	_first = _last = newChild;
    }
    if(newChild->nodeType() == Node::ELEMENT_NODE)
	return newChild;
    return this;
}

void NodeBaseImpl::attach(KHTMLWidget *w)
{
    NodeImpl *child = _first;
    while(child != 0)
    {
	child->attach(w);
	child = child->nextSibling();
    }
}

void NodeBaseImpl::detach()
{
    NodeImpl *child = _first;
    while(child != 0)
    {
	child->detach();
	child = child->nextSibling();
    }
}

// ---------------------------------------------------------------------------

NodeImpl *NodeListImpl::item( unsigned long /*index*/ )
{
    // ###
    return 0;
}

unsigned long NodeListImpl::length() const
{
    // ###
    return 0;
}

ChildNodeListImpl::ChildNodeListImpl( NodeImpl *n )
{
    refNode = n;
    refNode->ref();
}

ChildNodeListImpl::~ChildNodeListImpl()
{
    refNode->deref();
}

unsigned long ChildNodeListImpl::length() const
{
    int len = 0;
    NodeImpl *n;
    for(n = refNode->firstChild(); n != 0; n = n->nextSibling())
	len++;

    return len;
}

NodeImpl *ChildNodeListImpl::item ( unsigned long index )
{
    unsigned int pos = 0;
    NodeImpl *n = refNode->firstChild();

    while( n != 0 && pos < index )
    {
	n = n->nextSibling();
	pos++;
    }

    return n;
}

