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
#include "dom_nodeimpl.h"

#include "dom_node.h"
#include "dom_exception.h"
#include "dom_string.h"

#include "dom_elementimpl.h"
#include "dom_docimpl.h"

#include <stdio.h>

#include "rendering/render_object.h"
#include <qstring.h>

using namespace DOM;
using namespace khtml;

const QChar NodeImpl::LT= '<';
const QChar NodeImpl::MT = '>';
const QChar NodeImpl::SLASH = '/';
const QChar NodeImpl::SPACE = ' ';

NodeImpl::NodeImpl(DocumentImpl *doc)
{
    document = doc;
    flags = 0;
    m_style = 0;
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

bool NodeImpl::isInline()
{
    if(!m_style) return false;
    return (m_style->display() == INLINE);
}

DOMString NodeImpl::toHTML(DOMString _string)
{
    _string = innerHTML(_string);
    return _string;
}

DOMString NodeImpl::innerHTML(DOMString _string)
{
    NodeImpl *_current;
    for(_current = firstChild(); _current != 0; _current = _current->nextSibling() )
    {
        _string = _current->toHTML(_string);
    }
    return _string;
}

QString NodeImpl::toHTML()
{
    long offset = 0;
    int stdInc = 5000;
    long currentLength = stdInc;
    QChar *htmlText = QT_ALLOC_QCHAR_VEC(stdInc);

    recursive( htmlText, currentLength, offset, stdInc );
    QString finishedHtmlText( htmlText, offset );
    return finishedHtmlText;
}

void NodeImpl::recursive( QChar *htmlText, long &currentLength, long &offset, int stdInc )
{
//printf("\nOFFSET: %d\n\n", offset);
//printf("\nSIZE: %d\n\n", sizeof htmlText);
//printf("recursive 1: %s\n", nodeName().string().ascii() );
    DOMString me;

    // Copy who I am into the htmlStext string
    if ( nodeType() == Node::TEXT_NODE )
    {
//printf("recursive 2: %s\n", nodeName().string().ascii() );
        me = nodeValue();
        int i = me.length();
        while( (currentLength - offset) <= i*2+4){
//            printf("\ni: %d\n", i);
            increaseStringLength( htmlText, currentLength, offset, stdInc);
        }
        
        memcpy(htmlText+offset, me.stringPtr(), i*2);
        offset += i;
    }
    else
    {
//printf("recursive 3: %s\n", nodeName().string().ascii() );
        me = nodeName();
        int i = me.length();
        while( (currentLength - offset) <= i*2+4)
        {
//            printf("\ni: %d\n", i);
            increaseStringLength( htmlText, currentLength, offset, stdInc);
        }
        memcpy(htmlText+offset, &LT, 2);             // prints "<"
        memcpy(htmlText+offset+1, me.stringPtr(), i*2);     // prints tagname
      
        // insert attributes here
        /*                  if( nodeType() == Node::ELEMENT_NODE )
                            {
                            NamedNodeMap nnm = n.attributes();
                            Attr _attr;
                            unsigned long lmap = nnm.length();
                            //                printf( "Heffa: %s %d\n", n.nodeName().string().ascii(), lmap );
                            for( unsigned int j=0; j<lmap; j++ )
                            {
                            _attr = nnm.item(j);
                            //                    printf( "Attr: %s\n", _attr.name().string().ascii() );
                            unsigned long lname = _attr.name().length();
                            unsigned long lvalue = _attr.value().length();
                            int len = lname + lvalue + 4;
                            htmlText[offset+i+1] = SPACE;
                            memcpy(htmlText+offset+2, _attr.name().stringPtr(), lname*2);
                            htmlText[offset+i+2] = '=';
                            htmlText[offset+i+3] = '"';
                            memcpy(htmlText+offset+3, _attr.value().stringPtr(), lvalue*2);
                            htmlText[offset+i+4] = '"';
                            i += len;
                            }
                            }*/
        // end attr stuff
            
        if( firstChild() == 0 )     // if element has no endtag
        {
//printf("recursive 4: %s\n", nodeName().string().ascii() );
            memcpy(htmlText+offset+i+1, &SPACE, 2);     // prints " "
            memcpy(htmlText+offset+i+2, &SLASH, 2);     // prints "/"
            memcpy(htmlText+offset+i+3, &MT, 2);     // prints ">"
            offset += i+4;
        }
        else                  // if element has endtag
        {
//printf("recursive 5: %s\n", nodeName().string().ascii() );
            memcpy(htmlText+offset+i+1, &MT, 2);     // prints ">"
            offset += i+2;
        }
    }
//printf("recursive 5b: %s\n", nodeName().string().ascii() );
    
    if( firstChild() != 0 )
    {
//printf("recursive 6: %s\n", nodeName().string().ascii() );
        // print firstChild
        firstChild()->recursive( htmlText, currentLength, offset, stdInc);
        
        // Print my ending tag
        if ( nodeType() != Node::TEXT_NODE )
        {
            me = nodeName();
            int i = me.length();
            while( (currentLength - offset) <= i*2+3)
                increaseStringLength( htmlText, currentLength, offset, stdInc);
            memcpy(htmlText+offset, &LT, 2);                   // prints "<"
            memcpy(htmlText+offset+1, &SLASH, 2);              // prints "/"
            memcpy(htmlText+offset+2, me.stringPtr(), i*2);    // prints tagname
            memcpy(htmlText+offset+i+2, &MT, 2);               // prints ">"
            offset += i+3;
        } 
    }
    // print next sibling
    if( nextSibling() )
        nextSibling()->recursive( htmlText, currentLength, offset, stdInc);
//printf("recursive 8: %s\n", nodeName().string().ascii() );
//printf("\nOFFSET2: %d\n\n", offset);
}

int NodeImpl::increaseStringLength( QChar *htmlText, long &currentLength, long offset, int stdInc)
{
//printf("Hei!\n");
    currentLength += stdInc;
//printf("1\n");
    QChar *htmlTextTmp = QT_ALLOC_QCHAR_VEC(currentLength);
//printf("2\n");
    memcpy(htmlTextTmp, htmlText, offset);
//printf("3\n");
    QT_DELETE_QCHAR_VEC(htmlText);
//printf("4\n");
    htmlText = htmlTextTmp;
//printf("Ha det!\n");
}

void NodeImpl::applyChanges()
{
    m_render->styleChanged();
}

void NodeImpl::getCursor(int offset, int &_x, int &_y, int &height)
{
    m_render->cursorPos(offset, _x, _y, height);
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

NodeListImpl *NodeBaseImpl::childNodes() //
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
    else
	_first = newChild;
    refChild->setPreviousSibling(newChild);
    newChild->setParent(this);
    newChild->setPreviousSibling(prev);
    newChild->setNextSibling(refChild);

    // ### set style in case it's attached

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

    if (prev) prev->setNextSibling(newChild);
    if (next) next->setPreviousSibling(newChild);
    if(_first == oldChild) _first = newChild;
    if(_last == oldChild) _last = newChild;

    oldChild->setPreviousSibling(0);
    oldChild->setNextSibling(0);
    oldChild->setParent(0);

    newChild->setParent(this);
    newChild->setPreviousSibling(prev);
    newChild->setNextSibling(next);

    // ### set style in case it's attached

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
        printf("AddChild failed! id=%d, child->id=%d\n", id(), newChild->id());
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



TagNodeListImpl::TagNodeListImpl(DocumentImpl *doc, const DOMString &t )
  : tagName(t)
{
    refDoc= doc;
    refDoc->ref();
}

TagNodeListImpl::~TagNodeListImpl()
{
    refDoc->deref();
}

unsigned long TagNodeListImpl::length() const
{
    return recursiveLength( refDoc->documentElement() );
}

NodeImpl *TagNodeListImpl::item ( unsigned long index ) const
{
    return recursiveItem( refDoc->documentElement(), index );
}

bool TagNodeListImpl::nodeMatches( NodeImpl *testNode ) const
{
    return testNode->nodeName()==tagName;
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
