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
 */
#include "dom_nodeimpl.h"

#include "dom_node.h"
#include "dom_element.h"
#include "dom_exception.h"
#include "dom_string.h"

#include "dom_elementimpl.h"
#include "dom_docimpl.h"

#include <kdebug.h>

#include "rendering/render_object.h"
#include <qstring.h>
#include <qrect.h>
#include <iostream.h>

#define QT_ALLOC_QCHAR_VEC( N ) (QChar*) new char[ 2*( N ) ]
#define QT_DELETE_QCHAR_VEC( P ) delete[] ((char*)( P ))

using namespace DOM;
using namespace khtml;

const QChar NodeImpl::LESSTHAN = '<';
const QChar NodeImpl::MORETHAN = '>';
const QChar NodeImpl::SLASH = '/';
const QChar NodeImpl::SPACE = ' ';
const QChar NodeImpl::EQUALS = '=';
const QChar NodeImpl::QUOTE = '"';

NodeImpl::NodeImpl(DocumentImpl *doc)
{
  document = doc;
  flags = 0;
  m_render = 0;
}

NodeImpl::~NodeImpl()
{
  if(m_render) m_render->deref();
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
  cerr << "NodeImpl::childNodes()\n";
  return new ChildNodeListImpl(this);
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

NamedNodeMapImpl *NodeImpl::attributes() const
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

QString NodeImpl::toHTML()
{
    long offset = 0;
    const int stdInc = 10000;
    long currentLength = stdInc;
    QChar *htmlText = QT_ALLOC_QCHAR_VEC(stdInc);

    recursive( htmlText, currentLength, offset, stdInc );
    QString finishedHtmlText( htmlText, offset );
    return finishedHtmlText;
}

void NodeImpl::recursive( QChar *&htmlText, long &currentLength, long &offset, int stdInc )
{
    DOMString me;

    // Copy who I am into the htmlText string
    if ( nodeType() == Node::TEXT_NODE )
    {
        me = nodeValue();
        int i = me.length();
        while( (currentLength - offset) <= i*2+4)
            increaseStringLength( htmlText, currentLength, offset, stdInc);

        memcpy(htmlText+offset, me.stringPtr(), i*2);
        offset += i;
    }
    else
    {   // If I am an element, not a text
        me = nodeName();
        int i = me.length();
        while( (currentLength - offset) <= i*2+4)
            increaseStringLength( htmlText, currentLength, offset, stdInc);
        memcpy(htmlText+offset, &LESSTHAN, 2);              // prints <
        memcpy(htmlText+offset+1, me.stringPtr(), i*2);     // prints tagname

        // print attributes
        if( nodeType() == Node::ELEMENT_NODE )
        {
            int lattrs = 0;
            ElementImpl *el = (ElementImpl*)this;
            AttrImpl *attr;
            NamedNodeMapImpl *attrs = static_cast<NamedNodeMapImpl*>(el->attributes());
            unsigned long lmap = attrs->length();
            for( uint j=0; j<lmap; j++ )
            {
                attr = static_cast<AttrImpl*>(attrs->item(i));
                unsigned long lname = attr->name().length();
                unsigned long lvalue = attr->value().length();
                while( (currentLength - offset) <= (signed)(i*2+lattrs+lname+lvalue+4) )
                    increaseStringLength( htmlText, currentLength, offset, stdInc);
                memcpy(htmlText+offset+i+lattrs+1, &SPACE, 2);                 // prints a space
                memcpy(htmlText+offset+i+lattrs+2, attr->name().stringPtr(), lname*2);
                memcpy(htmlText+offset+i+lattrs+lname+2, &EQUALS, 2);          // prints =
                memcpy(htmlText+offset+i+lattrs+lname+3, &QUOTE, 2);           // prints "
                memcpy(htmlText+offset+i+lattrs+lname+4, attr->value().stringPtr(), lvalue*2);
                memcpy(htmlText+offset+i+lattrs+lname+lvalue+4, &QUOTE, 2);    // prints "
                lattrs += lname + lvalue + 4;
            }
            offset += lattrs;
        }

        // print ending bracket of start tag
        if( firstChild() == 0 )     // if element has no endtag
        {
            memcpy(htmlText+offset+i+1, &SPACE, 2);      // prints a space
            memcpy(htmlText+offset+i+2, &SLASH, 2);      // prints /
            memcpy(htmlText+offset+i+3, &MORETHAN, 2);   // prints >
            offset += i+4;
        }
        else                        // if element has endtag
        {
            memcpy(htmlText+offset+i+1, &MORETHAN, 2);     // prints >
            offset += i+2;
        }
    }

    if( firstChild() != 0 )
    {
        // print firstChild
        firstChild()->recursive( htmlText, currentLength, offset, stdInc);

        // Print my ending tag
        if ( nodeType() != Node::TEXT_NODE )
        {
            me = nodeName();
            int i = me.length();
            while( (currentLength - offset) <= i*2+3)
                increaseStringLength( htmlText, currentLength, offset, stdInc);
            memcpy(htmlText+offset, &LESSTHAN, 2);             // prints <
            memcpy(htmlText+offset+1, &SLASH, 2);              // prints /
            memcpy(htmlText+offset+2, me.stringPtr(), i*2);    // prints tagname
            memcpy(htmlText+offset+i+2, &MORETHAN, 2);         // prints >
            offset += i+3;
        }
    }
    // print next sibling
    if( nextSibling() )
        nextSibling()->recursive( htmlText, currentLength, offset, stdInc);
}

bool NodeImpl::increaseStringLength( QChar *&htmlText, long &currentLength, long offset, int stdInc)
{
    currentLength += stdInc;
    QChar *htmlTextTmp = QT_ALLOC_QCHAR_VEC( currentLength );
    memcpy( htmlTextTmp, htmlText, offset*sizeof(QChar) );
    QT_DELETE_QCHAR_VEC( htmlText );
    htmlText = htmlTextTmp;
    return true;       // should return false if not enough memory
}

void NodeImpl::applyChanges(bool, bool)
{
    setChanged(false);
}

void NodeImpl::getCursor(int offset, int &_x, int &_y, int &height)
{
    if(m_render) m_render->cursorPos(offset, _x, _y, height);
    else _x = _y = height = -1;
}

QRect NodeImpl::getRect()
{
    int _x, _y;
    if(m_render)
        {
            m_render->absolutePosition(_x, _y);
            return QRect( _x, _y, m_render->width(), m_render->height() );
        }
    return QRect();
}

void NodeImpl::setKeyboardFocus(ActivationState b)
{
  if (m_render)
    {
      m_render->setKeyboardFocus(b);
      m_render->repaintContainingBlock();
    }
}

//--------------------------------------------------------------------

NodeWParentImpl::NodeWParentImpl(DocumentImpl *doc) : NodeImpl(doc)
{
    _parent = 0;
    _previous = 0;
    _next = 0;
}

NodeWParentImpl::~NodeWParentImpl()
{
    // previous and next node may still reference this!!!
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
    if(!_parent && _ref <= 0) return true;
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
    //kdDebug( 6020 ) << "NodeBaseImpl destructor" << endl;
    // we have to tell all children, that the parent has died...
    NodeImpl *n;
    NodeImpl *next;

    for( n = _first; n != 0; n = next )
    {
	next = n->nextSibling();
        n->setPreviousSibling(0);
        n->setNextSibling(0);
	n->setParent(0);
	if(n->deleteMe()) delete n;
    }
}

NodeListImpl *NodeBaseImpl::childNodes() //
{
    return NodeImpl::childNodes();
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
    if (!newChild || (newChild->nodeType() == Node::DOCUMENT_FRAGMENT_NODE && !newChild->firstChild()))
	throw DOMException(DOMException::NOT_FOUND_ERR);

    if(!refChild)
	return appendChild(newChild);

    checkSameDocument(newChild);
    checkNoOwner(newChild);
    checkIsChild(refChild);

    bool isFragment = newChild->nodeType() == Node::DOCUMENT_FRAGMENT_NODE;
    NodeImpl *nextChild;
    NodeImpl *child = isFragment ? newChild->firstChild() : newChild;

    NodeImpl *prev = refChild->previousSibling();
    while (child) {
	nextChild = isFragment ? child->nextSibling() : 0;

	checkNoOwner(child);
	if(!childAllowed(child))
	    throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);
		
	// if already in the tree, remove it first!
	NodeImpl *newParent = child->parentNode();
	if(newParent)
	    newParent->removeChild( child );

	// seems ok, lets's insert it.
	if (prev)
	    prev->setNextSibling(child);
	else
	    _first = child;
	refChild->setPreviousSibling(child);
	child->setParent(this);
	child->setPreviousSibling(prev);
	child->setNextSibling(refChild);
	if (attached())
	    child->attach(document->view());
	
	prev = child;
	child = nextChild;
    }

    // ### set style in case it's attached
    setChanged(true);

    return newChild;
}

NodeImpl *NodeBaseImpl::replaceChild ( NodeImpl *newChild, NodeImpl *oldChild )
{
    checkReadOnly();
    if (!newChild || (newChild->nodeType() == Node::DOCUMENT_FRAGMENT_NODE && !newChild->firstChild()))
	throw DOMException(DOMException::NOT_FOUND_ERR);
    checkSameDocument(newChild);
    checkIsChild(oldChild);
    checkNoOwner(newChild);

    bool isFragment = newChild->nodeType() == Node::DOCUMENT_FRAGMENT_NODE;
    NodeImpl *nextChild;
    NodeImpl *child = isFragment ? newChild->firstChild() : newChild;

    // make sure we will be able to insert the first node before we go removing the old one
    checkNoOwner(isFragment ? newChild->firstChild() : newChild);
    if(!childAllowed(isFragment ? newChild->firstChild() : newChild))
	throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);

    NodeImpl *prev = oldChild->previousSibling();
    NodeImpl *next = oldChild->nextSibling();
    oldChild->setPreviousSibling(0);
    oldChild->setNextSibling(0);
    oldChild->setParent(0);
    if (m_render)
	m_render->removeChild(oldChild->renderer());

    while (child) {
	nextChild = isFragment ? child->nextSibling() : 0;

	checkNoOwner(child);
	if(!childAllowed(child))
	    throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);
	
	// if already in the tree, remove it first!
	NodeImpl *newParent = child->parentNode();
	if(newParent)
	    newParent->removeChild( child );

	// seems ok, lets's insert it.
	if (prev) prev->setNextSibling(child);
	if (next) next->setPreviousSibling(child);
	if(!prev) _first = child;
	if(!next) _last = child;

	child->setParent(this);
	child->setPreviousSibling(prev);
	child->setNextSibling(next);
	if (attached())
	    child->attach(document->view());
	prev = child;
	child = nextChild;
    }

    // ### set style in case it's attached
    setChanged(true);

    return oldChild;
}

NodeImpl *NodeBaseImpl::removeChild ( NodeImpl *oldChild )
{
    checkReadOnly();
    checkIsChild(oldChild);

    NodeImpl *prev, *next;
    prev = oldChild->previousSibling();
    next = oldChild->nextSibling();

    if(next) next->setPreviousSibling(prev);
    if(prev) prev->setNextSibling(next);
    if(_first == oldChild) _first = next;
    if(_last == oldChild) _last = prev;

    oldChild->setPreviousSibling(0);
    oldChild->setNextSibling(0);
    oldChild->setParent(0);
    if (m_render)
	m_render->removeChild(oldChild->renderer());

    setChanged(true);

    return oldChild;
}

NodeImpl *NodeBaseImpl::appendChild ( NodeImpl *newChild )
{
//    kdDebug(6010) << "NodeBaseImpl::appendChild( " << newChild << " );" <<endl;
    checkReadOnly();
    if (!newChild || (newChild->nodeType() == Node::DOCUMENT_FRAGMENT_NODE && !newChild->firstChild()))
	throw DOMException(DOMException::NOT_FOUND_ERR);
    checkSameDocument(newChild);
    checkNoOwner(newChild);

    bool isFragment = newChild->nodeType() == Node::DOCUMENT_FRAGMENT_NODE;
    NodeImpl *nextChild;
    NodeImpl *child = isFragment ? newChild->firstChild() : newChild;

    while (child) {
	nextChild = isFragment ? child->nextSibling() : 0;

	checkNoOwner(child);
	if(!childAllowed(child))
	    throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);
	
	// if already in the tree, remove it first!
	NodeImpl *newParent = child->parentNode();
	if(newParent)
	    newParent->removeChild( child );
		
	// lets append it
	child->setParent(this);

	if(_last)
	{
	    child->setPreviousSibling(_last);
	    _last->setNextSibling(child);
	    _last = child;
	}
	else
	{
	    _first = _last = child;
	}
	if (attached())
		child->attach(document->view());
		
	child = nextChild;
    }

    setChanged(true);
    // ### set style in case it's attached
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
    DocumentImpl *ownerDocThis = static_cast<DocumentImpl*>(nodeType() == Node::DOCUMENT_NODE ? this : ownerDocument());
    DocumentImpl *ownerDocNew = static_cast<DocumentImpl*>(newChild->nodeType() == Node::DOCUMENT_NODE ? newChild : newChild->ownerDocument());
    if(ownerDocThis != ownerDocNew) {
	kdDebug(6010)<< "not same document, newChild = " << newChild << "document = " << document << endl;
	throw DOMException(DOMException::WRONG_DOCUMENT_ERR);
    }
}

// check for being (grand-..)father:
void NodeBaseImpl::checkNoOwner( NodeImpl *newChild )
{
  //check if newChild is parent of this...
  NodeImpl *n;
  for( n = this; n != (NodeImpl *)document && n!= 0; n = n->parentNode() )
    if(n == newChild)
      throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);

  if(newChild->parentNode() == this)
    removeChild(newChild);
}

// check for being child:
void NodeBaseImpl::checkIsChild( NodeImpl *oldChild )
{
    if(!oldChild || oldChild->parentNode() != this)
	throw DOMException(DOMException::NOT_FOUND_ERR);
}

bool NodeBaseImpl::childAllowed( NodeImpl *newChild )
{
    return checkChild(id(), newChild->id());
}

NodeImpl *NodeBaseImpl::addChild(NodeImpl *newChild)
{
    // do not add applyChanges here! This function is only used during parsing

    // short check for consistency with DTD
    if(!childAllowed(newChild))
    {
        //kdDebug( 6020 ) << "AddChild failed! id=" << id() << ", child->id=" << newChild->id() << endl;
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

void NodeBaseImpl::attach(KHTMLView *w)
{
    NodeImpl *child = _first;
    while(child != 0)
    {
	child->attach(w);
	child = child->nextSibling();
    }
    NodeWParentImpl::attach(w);
}

void NodeBaseImpl::detach()
{
    NodeImpl *child = _first;
    while(child != 0)
    {
	child->detach();
	child = child->nextSibling();
    }
    NodeWParentImpl::detach();
}

// ---------------------------------------------------------------------------

NodeImpl *NodeListImpl::item( unsigned long /*index*/ ) const
{
    return 0;
}

unsigned long NodeListImpl::length() const
{
    return 0;
}

unsigned long NodeListImpl::recursiveLength(NodeImpl *start) const
{
    unsigned long len = 0;

    for(NodeImpl *n = start->firstChild(); n != 0; n = n->nextSibling()) {
	if ( n->nodeType() == Node::ELEMENT_NODE ) {
	    if (nodeMatches(n))
		len++;
	    len+= recursiveLength(n);
	}
    }

    return len;
}

NodeImpl *NodeListImpl::recursiveItem ( NodeImpl *start, unsigned long &offset ) const
{
    for(NodeImpl *n = start->firstChild(); n != 0; n = n->nextSibling()) {
	if ( n->nodeType() == Node::ELEMENT_NODE ) {
	    if (nodeMatches(n))
		if (!offset--)
		    return n;

	    NodeImpl *depthSearch= recursiveItem(n, offset);
	    if (depthSearch)
		return depthSearch;
	}
    }

    return 0; // no matching node in this subtree
}

bool NodeListImpl::nodeMatches( NodeImpl */*testNode*/ ) const
{
  // ###
    return false;
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
    unsigned long len = 0;
    NodeImpl *n;
    for(n = refNode->firstChild(); n != 0; n = n->nextSibling())
	len++;

    return len;
}

NodeImpl *ChildNodeListImpl::item ( unsigned long index ) const
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



TagNodeListImpl::TagNodeListImpl(NodeImpl *n, const DOMString &t )
  : tagName(t)
{
    refNode = n;
    refNode->ref();
    allElements = (t == "*");
}

TagNodeListImpl::~TagNodeListImpl()
{
    refNode->deref();
}

unsigned long TagNodeListImpl::length() const
{
    return recursiveLength( refNode );
}

NodeImpl *TagNodeListImpl::item ( unsigned long index ) const
{
    return recursiveItem( refNode, index );
}

bool TagNodeListImpl::nodeMatches( NodeImpl *testNode ) const
{
    return ((allElements && testNode->nodeType() == Node::ELEMENT_NODE) ||
            !strcasecmp(testNode->nodeName(),tagName));
}



NameNodeListImpl::NameNodeListImpl(DocumentImpl *doc, const DOMString &t )
  : nodeName(t)
{
    refDoc= doc;
    refDoc->ref();
}

NameNodeListImpl::~NameNodeListImpl()
{
    refDoc->deref();
}

unsigned long NameNodeListImpl::length() const
{
    return recursiveLength( refDoc->documentElement() );
}

NodeImpl *NameNodeListImpl::item ( unsigned long index ) const
{
    return recursiveItem( refDoc->documentElement(), index );
}

bool NameNodeListImpl::nodeMatches( NodeImpl *testNode ) const
{
    // assert(testNode->nodeType() == Node::ELEMENT_NODE)
    // this function is only called by NodeListImpl::recursiveXXX if
    // the node is an element
    return static_cast<ElementImpl *>(testNode) ->
      getAttribute("name") == nodeName;
}

// ---------------------------------------------------------------------------

NamedNodeMapImpl::NamedNodeMapImpl()
{
}

NamedNodeMapImpl::~NamedNodeMapImpl()
{
}

unsigned long NamedNodeMapImpl::length() const
{
  // ###
  return 0;
}

NodeImpl *NamedNodeMapImpl::getNamedItem ( const DOMString &/*name*/ ) const
{
  // ###
  return 0;
}

NodeImpl *NamedNodeMapImpl::setNamedItem ( const Node &/*arg*/ )
{
  // ###
  return 0;
}

NodeImpl *NamedNodeMapImpl::removeNamedItem ( const DOMString &/*name*/ )
{
  // ###
  return 0;
}


NodeImpl *NamedNodeMapImpl::item ( unsigned long /*index*/ ) const
{
  // ###
  return 0;
}


