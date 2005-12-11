/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
 * (C) 2000 Frederik Holljen (frederik.holljen@hig.no)
 * (C) 2001 Peter Kelly (pmk@post.com)
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
 */

#include "dom/dom_exception.h"
#include "xml/dom_docimpl.h"

using namespace DOM;

NodeIteratorImpl::NodeIteratorImpl(NodeImpl *_root, unsigned long _whatToShow,
				   NodeFilterImpl* _filter, bool _entityReferenceExpansion)
{
    m_root = _root;
    m_whatToShow = _whatToShow;
    m_filter = _filter;
    m_expandEntityReferences = _entityReferenceExpansion;

    m_referenceNode = _root;
    m_inFront = false;

    m_doc = m_root->getDocument();
    m_doc->attachNodeIterator(this);
    m_doc->ref();

    m_detached = false;
}

NodeIteratorImpl::~NodeIteratorImpl()
{
    m_doc->detachNodeIterator(this);
    m_doc->deref();
}

NodeImpl *NodeIteratorImpl::root()
{
    return m_root;
}

unsigned long NodeIteratorImpl::whatToShow()
{
    return m_whatToShow;
}

NodeFilterImpl* NodeIteratorImpl::filter()
{
    return m_filter.get();
}

bool NodeIteratorImpl::expandEntityReferences()
{
    return m_expandEntityReferences;
}

NodeImpl *NodeIteratorImpl::nextNode( int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    if (!m_referenceNode) {
	m_inFront = true;
	return 0;
    }

    if (!m_inFront) {
	m_inFront = true;
	if (isAccepted(m_referenceNode) == NodeFilter::FILTER_ACCEPT)
	    return m_referenceNode;
    }

    NodeImpl *_tempCurrent = getNextNode(m_referenceNode);
    while( _tempCurrent ) {
	m_referenceNode = _tempCurrent;
	if(isAccepted(_tempCurrent) == NodeFilter::FILTER_ACCEPT)
	    return m_referenceNode;
      _tempCurrent = getNextNode(_tempCurrent);
    }

    return 0;
}

NodeImpl *NodeIteratorImpl::getNextNode(NodeImpl *n)
{
  /*  1. my first child
   *  2. my next sibling
   *  3. my parents sibling, or their parents sibling (loop)
   *  4. not found
   */

  if( !n )
    return 0;

  if( n->hasChildNodes() )
    return n->firstChild();

  if( n->nextSibling() )
    return n->nextSibling();

  if( m_root == n)
     return 0;

  NodeImpl *parent = n->parentNode();
  while( parent )
    {
      n = parent->nextSibling();
      if( n )
        return n;

      if( m_root == parent )
        return 0;

      parent = parent->parentNode();
    }
  return 0;
}

NodeImpl *NodeIteratorImpl::previousNode( int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    if (!m_referenceNode) {
	m_inFront = false;
	return 0;
    }

    if (m_inFront) {
	m_inFront = false;
	if (isAccepted(m_referenceNode) == NodeFilter::FILTER_ACCEPT)
	    return m_referenceNode;
    }

    NodeImpl *_tempCurrent = getPreviousNode(m_referenceNode);
    while( _tempCurrent ) {
	m_referenceNode = _tempCurrent;
	if(isAccepted(_tempCurrent) == NodeFilter::FILTER_ACCEPT)
	    return m_referenceNode;
	_tempCurrent = getPreviousNode(_tempCurrent);
    }

    return 0;
}

NodeImpl *NodeIteratorImpl::getPreviousNode(NodeImpl *n)
{
/* 1. my previous sibling.lastchild
 * 2. my previous sibling
 * 3. my parent
 */
  NodeImpl *_tempCurrent;

  if( !n )
    return 0;

  _tempCurrent = n->previousSibling();
  if( _tempCurrent )
    {
      if( _tempCurrent->lastChild() )
        {
          while( _tempCurrent->lastChild() )
            _tempCurrent = _tempCurrent->lastChild();
          return _tempCurrent;
        }
      else
        return _tempCurrent;
    }


  if(n == m_root)
    return 0;

  return n->parentNode();


}

void NodeIteratorImpl::detach(int &/*exceptioncode*/)
{
    m_doc->detachNodeIterator(this);
    m_detached = true;
}


void NodeIteratorImpl::notifyBeforeNodeRemoval(NodeImpl *removed)
{
    // make sure the deleted node is with the root (but not the root itself)
    if (removed == m_root)
	return;

    NodeImpl *maybeRoot = removed->parentNode();
    while (maybeRoot && maybeRoot != m_root)
	maybeRoot = maybeRoot->parentNode();
    if (!maybeRoot)
	return;

    // did I get deleted, or one of my parents?
    NodeImpl *_tempDeleted = m_referenceNode;
    while( _tempDeleted && _tempDeleted != removed)
        _tempDeleted = _tempDeleted->parentNode();

    if( !_tempDeleted )  // someone that didn't consern me got deleted
        return;

    if( !m_inFront)
    {
        NodeImpl *_next = getNextNode(_tempDeleted);
        if( _next )
            m_referenceNode = _next;
        else
        {
	    // deleted node was at end of list
            m_inFront = true;
            m_referenceNode = getPreviousNode(_tempDeleted);
        }
    }
    else {
	NodeImpl *_prev = getPreviousNode(_tempDeleted);
	if ( _prev )
	    m_referenceNode = _prev;
	else
	{
	    // deleted node was at start of list
	    m_inFront = false;
	    m_referenceNode = getNextNode(_tempDeleted);
	}
    }

}

short NodeIteratorImpl::isAccepted(NodeImpl *n)
{
  // if XML is implemented we have to check expandEntityRerefences in this function
  if( ( ( 1 << ( n->nodeType()-1 ) ) & m_whatToShow) != 0 )
    {
        if(!m_filter.isNull())
            return m_filter->acceptNode(n);
        else
	    return NodeFilter::FILTER_ACCEPT;
    }
    return NodeFilter::FILTER_SKIP;
}

// --------------------------------------------------------------


NodeFilterImpl::NodeFilterImpl()
{
    m_customNodeFilter = 0;
}

NodeFilterImpl::~NodeFilterImpl()
{
    if (m_customNodeFilter)
	m_customNodeFilter->deref();
}

short NodeFilterImpl::acceptNode(const Node &n)
{
    if (m_customNodeFilter)
	return m_customNodeFilter->acceptNode(n);
    else
	return NodeFilter::FILTER_ACCEPT;
}

void NodeFilterImpl::setCustomNodeFilter(CustomNodeFilter *custom)
{
    m_customNodeFilter = custom;
    if (m_customNodeFilter)
	m_customNodeFilter->ref();
}

CustomNodeFilter *NodeFilterImpl::customNodeFilter()
{
    return m_customNodeFilter;
}

// --------------------------------------------------------------

TreeWalkerImpl::TreeWalkerImpl(NodeImpl *n, long _whatToShow, NodeFilterImpl *f,
                               bool entityReferenceExpansion)
{
  m_currentNode = n;
  m_rootNode = n;
  m_whatToShow = _whatToShow;
  m_filter = f;
  if ( m_filter )
      m_filter->ref();
  m_expandEntityReferences = entityReferenceExpansion;
  m_doc = m_rootNode->getDocument();
  m_doc->ref();
}

TreeWalkerImpl::~TreeWalkerImpl()
{
    m_doc->deref();
    if ( m_filter )
        m_filter->deref();
}

NodeImpl *TreeWalkerImpl::getRoot() const
{
    return m_rootNode;
}

unsigned long TreeWalkerImpl::getWhatToShow() const
{
    return m_whatToShow;
}

NodeFilterImpl *TreeWalkerImpl::getFilter() const
{
    return m_filter;
}

bool TreeWalkerImpl::getExpandEntityReferences() const
{
    return m_expandEntityReferences;
}

NodeImpl *TreeWalkerImpl::getCurrentNode() const
{
    return m_currentNode;
}

void TreeWalkerImpl::setWhatToShow(long _whatToShow)
{
    // do some testing wether this is an accepted value
    m_whatToShow = _whatToShow;
}

void TreeWalkerImpl::setFilter(NodeFilterImpl *_filter)
{
    m_filter->deref();
    m_filter = _filter;
    if ( m_filter )
        m_filter->ref();
}

void TreeWalkerImpl::setExpandEntityReferences(bool value)
{
    m_expandEntityReferences = value;
}

void TreeWalkerImpl::setCurrentNode( NodeImpl *n )
{
    if ( n )
    {
        //m_rootNode = n;
        m_currentNode = n;
    }
//     else
//         throw( DOMException::NOT_SUPPORTED_ERR );
}

NodeImpl *TreeWalkerImpl::parentNode(  )
{
    NodeImpl *n = getParentNode( m_currentNode );
    if ( n )
        m_currentNode = n;
    return n;
}


NodeImpl *TreeWalkerImpl::firstChild(  )
{
    NodeImpl *n = getFirstChild( m_currentNode );
    if ( n )
        m_currentNode = n;
    return n;
}


NodeImpl *TreeWalkerImpl::lastChild(  )
{
    NodeImpl *n = getLastChild(m_currentNode);
    if( n )
        m_currentNode = n;
    return n;
}

NodeImpl *TreeWalkerImpl::previousSibling(  )
{
    NodeImpl *n = getPreviousSibling( m_currentNode );
    if( n )
        m_currentNode = n;
    return n;
}

NodeImpl *TreeWalkerImpl::nextSibling(  )
{
    NodeImpl *n = getNextSibling( m_currentNode );
    if( n )
        m_currentNode = n;
    return n;
}

NodeImpl *TreeWalkerImpl::previousNode(  )
{
/* 1. my previous sibling.lastchild
 * 2. my previous sibling
 * 3. my parent
 */

    NodeImpl *n = getPreviousSibling( m_currentNode );
    if( !n )
    {
        n = getParentNode( m_currentNode );
        if( n )      //parent
        {
            m_currentNode = n;
            return m_currentNode;
        }
        else                  // parent failed.. no previous node
            return 0;
    }

    NodeImpl *child = getLastChild( n );
    if( child )     // previous siblings last child
    {
        m_currentNode = child;
        return m_currentNode;
    }
    else                      // previous sibling
    {
        m_currentNode = n;
        return m_currentNode;
    }
    return 0;            // should never get here!
}

NodeImpl *TreeWalkerImpl::nextNode(  )
{
/*  1. my first child
 *  2. my next sibling
 *  3. my parents sibling, or their parents sibling (loop)
 *  4. not found
 */

    NodeImpl *n = getFirstChild( m_currentNode );
    if( n ) // my first child
    {
        m_currentNode = n;
        return n;
    }

    n = getNextSibling( m_currentNode ); // my next sibling
    if( n )
    {
        m_currentNode = n;
        return m_currentNode;
    }
    NodeImpl *parent = getParentNode( m_currentNode );
    while( parent ) // parents sibling
    {
        n = getNextSibling( parent );
        if( n )
        {
          m_currentNode = n;
          return m_currentNode;
        }
        else
            parent = getParentNode( parent );
    }
    return 0;
}

short TreeWalkerImpl::isAccepted(NodeImpl *n)
{
    // if XML is implemented we have to check expandEntityRerefences in this function
    if( ( ( 1 << ( n->nodeType()-1 ) ) & m_whatToShow) != 0 )
    {
        if(m_filter)
            return m_filter->acceptNode(n);
        else
            return NodeFilter::FILTER_ACCEPT;
    }
    return NodeFilter::FILTER_SKIP;
}

NodeImpl *TreeWalkerImpl::getParentNode(NodeImpl *n)
{
    short _result = NodeFilter::FILTER_ACCEPT;

    if( n == m_rootNode /*|| !n*/ )
      return 0;

    NodeImpl *_tempCurrent = n->parentNode();

    if( !_tempCurrent )
        return 0;

    _result = isAccepted( _tempCurrent );
    if ( _result == NodeFilter::FILTER_ACCEPT )
        return _tempCurrent;       // match found

    return getParentNode( _tempCurrent );
}

NodeImpl *TreeWalkerImpl::getFirstChild(NodeImpl *n)
{
    short _result;

    if( !n || n->firstChild() )
        return 0;
    n = n->firstChild();

    _result = isAccepted( n );

    switch( _result )
    {
    case NodeFilter::FILTER_ACCEPT:
        return n;
        break;
    case NodeFilter::FILTER_SKIP:
        if( n->hasChildNodes() )
            return getFirstChild( n );
        else
            return getNextSibling( n );
        break;

    case NodeFilter::FILTER_REJECT:
        return getNextSibling( n );
        break;
    }
    return 0;      // should never get here!
}

NodeImpl *TreeWalkerImpl::getLastChild(NodeImpl *n)
{
    short _result;

    if( !n || n->lastChild() )
        return 0;
    n = n->lastChild();
    _result = isAccepted( n );

    switch( _result )
    {
    case NodeFilter::FILTER_ACCEPT:
        return n;
        break;

    case NodeFilter::FILTER_SKIP:
        if( n->hasChildNodes() )
            return getLastChild( n );
        else
            return getPreviousSibling( n );
        break;

    case NodeFilter::FILTER_REJECT:
        return getPreviousSibling( n );
        break;
    }
    return 0;
}

NodeImpl *TreeWalkerImpl::getPreviousSibling(NodeImpl *n)
{
    short _result;
    NodeImpl *_tempCurrent;

    if( !n )
        return 0;
    //first the cases if we have a previousSibling
    _tempCurrent = n->previousSibling();
    if( _tempCurrent )
    {
        _result = isAccepted( _tempCurrent );
        switch ( _result )
        {
        case NodeFilter::FILTER_ACCEPT:
            return _tempCurrent;
            break;

        case NodeFilter::FILTER_SKIP:
        {
            NodeImpl *nskip = getLastChild( _tempCurrent );
            if( nskip )
                return nskip;
            return getPreviousSibling( _tempCurrent );
            break;
        }

        case NodeFilter::FILTER_REJECT:
            return getPreviousSibling( _tempCurrent );
            break;
        }
    }
    // now the case if we don't have previous sibling
    else
    {
        _tempCurrent = _tempCurrent->parentNode();
        if( !_tempCurrent || _tempCurrent == m_rootNode)
            return 0;
        _result = isAccepted( _tempCurrent );
        if ( _result == NodeFilter::FILTER_SKIP )
            return getPreviousSibling( _tempCurrent );

        return 0;

    }
    return 0;  // should never get here!
}

NodeImpl *TreeWalkerImpl::getNextSibling(NodeImpl *n)
{
    NodeImpl *_tempCurrent = 0;
    short _result;

    if( !n )
        return 0;

    _tempCurrent = n->nextSibling();
    if( _tempCurrent )
    {
        _result = isAccepted( _tempCurrent );
        switch ( _result )
        {
        case NodeFilter::FILTER_ACCEPT:
            return _tempCurrent;
            break;

        case NodeFilter::FILTER_SKIP:
        {
            NodeImpl *nskip = getFirstChild( _tempCurrent );
            if( nskip )
                return nskip;
            return getNextSibling( _tempCurrent );
            break;
        }

        case NodeFilter::FILTER_REJECT:
            return getNextSibling( _tempCurrent );
            break;
        }
    }
    else
    {
        _tempCurrent = n->parentNode();
        if( !_tempCurrent || _tempCurrent == m_rootNode)
            return 0;
        _result = isAccepted( _tempCurrent );
        if( _result == NodeFilter::FILTER_SKIP )
            return getNextSibling( _tempCurrent );

        return 0;
    }
    return 0;
}

