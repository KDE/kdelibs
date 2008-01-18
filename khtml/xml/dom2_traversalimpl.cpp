/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
 * (C) 2000 Frederik Holljen (frederik.holljen@hig.no)
 * (C) 2001 Peter Kelly (pmk@post.com)
 * (C) 2008 Maksim Orlovich (maksim@kde.org)
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

/**
 Robustness notes: we protect the object we hold on to, to
 prevent crashes. To ensure termination in JS used, we propagate exceptions,
 and rely on the JS CPU guard to set one to force us to bail out.

 ### TODO/CHECKONE
*/

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

  if( m_root == n)
     return 0;

  if( n->nextSibling() )
    return n->nextSibling();

  NodeImpl *parent = n->parentNode();
  while( parent )
    {
      if( m_root == parent )
           return 0;

      n = parent->nextSibling();
      if( n )
        return n;

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

  if( !n || m_root == n )
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
    return m_rootNode.get();
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
    return m_currentNode.get();
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
    NodePtr n = getParentNode( m_currentNode );
    if ( n )
        m_currentNode = n;
    return n.get();
}


NodeImpl *TreeWalkerImpl::firstChild(  )
{
    NodePtr n = getFirstChild( m_currentNode );
    if ( n )
        m_currentNode = n;
    return n.get();
}


NodeImpl *TreeWalkerImpl::lastChild(  )
{
    NodePtr n = getLastChild(m_currentNode);
    if( n )
        m_currentNode = n;
    return n.get();
}

NodeImpl *TreeWalkerImpl::previousSibling(  )
{
    NodePtr n = getPreviousSibling( m_currentNode );
    if( n )
        m_currentNode = n;
    return n.get();
}

NodeImpl *TreeWalkerImpl::nextSibling(  )
{
    NodePtr n = getNextSibling( m_currentNode );
    if( n )
        m_currentNode = n;
    return n.get();
}

NodeImpl *TreeWalkerImpl::nextNode(  )
{
    NodePtr n = getNextNode();
    if( n )
        m_currentNode = n;
    return n.get();
}

NodeImpl *TreeWalkerImpl::previousNode(  )
{
    NodePtr n = getPreviousNode();
    if( n )
        m_currentNode = n;
    return n.get();
}

TreeWalkerImpl::NodePtr TreeWalkerImpl::getPreviousNode(  )
{
/* 1. last node in my previous sibling's subtree
 * 2. my previous sibling (special case of the above)
 * 3. my parent
 */

    NodePtr n = getPreviousSibling(m_currentNode);
    if (n) {
        // Find the last kid in the tree's preorder traversal, if any,
        // by following the lastChild links.
        NodePtr desc = getLastChild(n);
        while (desc) {
            n    = desc;
            desc = getLastChild(desc);
        }
        return n;
    }

    return getParentNode(n);
}

TreeWalkerImpl::NodePtr TreeWalkerImpl::getNextNode()
{
/*  1. my first child
 *  2. my next sibling
 *  3. my parents sibling, or their parents sibling (loop)
 *  4. not found
 */

    NodePtr n = getFirstChild(m_currentNode);
    if (n) // my first child
        return n;

    n = getNextSibling(m_currentNode); // my next sibling
    if (n)
        return n;

    NodePtr parent = getParentNode(m_currentNode);
    while (parent) // parent's sibling
    {
        n = getNextSibling(parent);
        if (n)
          return n;
        parent = getParentNode(parent);
    }
    return 0;
}

short TreeWalkerImpl::isAccepted(TreeWalkerImpl::NodePtr n)
{
    // if XML is implemented we have to check expandEntityRerefences in this function
    if( ( ( 1 << ( n->nodeType()-1 ) ) & m_whatToShow) != 0 )
    {
        if(m_filter)
            return m_filter->acceptNode(n.get());
        else
            return NodeFilter::FILTER_ACCEPT;
    }
    return NodeFilter::FILTER_SKIP;
}

/**
 This is quite a bit more complicated than it would at first seem, due to the following note:
 "Note that the structure of a TreeWalker's filtered view of a document may differ significantly from that of the document itself. For example, a TreeWalker with only SHOW_TEXT specified in its whatToShow parameter would present all the Text nodes as if they were siblings of each other yet had no parent."

 My read of this is that e.g. that when a node is  FILTER_SKIP'd, its children are handled
 as the children of its parent. -M.O.
*/

TreeWalkerImpl::NodePtr TreeWalkerImpl::getParentNode(TreeWalkerImpl::NodePtr n)
{
    NodePtr cursor = n;

    // Walk up, to find the first visible node != n, until we run out of
    // document or into the root (which we don't have to be inside of!)
    while (cursor && cursor != m_rootNode) {
        if (cursor != n && isAccepted(cursor) == NodeFilter::FILTER_ACCEPT)
            return cursor;
        else
            cursor = cursor->parentNode();
    }

    return 0;
}

TreeWalkerImpl::NodePtr TreeWalkerImpl::getFirstChild(TreeWalkerImpl::NodePtr n)
{
    NodePtr cursor;
    for (cursor = n->firstChild(); cursor; cursor = cursor->nextSibling()) {
        switch (isAccepted(cursor)) {
        case NodeFilter::FILTER_ACCEPT:
            return cursor;
        case NodeFilter::FILTER_SKIP: {
            // We ignore this candidate proper, but perhaps it has a kid?
            NodePtr cand = getFirstChild(cursor);
            if (cand)
                return cand;
            break;
        }
        case NodeFilter::FILTER_REJECT:
            // It effectively doesn't exist..
            break;
        }
        // ### this is unclear: what should happen if we "pass through" the root??
    }

    // Nothing found..    
    return 0;
}

TreeWalkerImpl::NodePtr TreeWalkerImpl::getLastChild(TreeWalkerImpl::NodePtr n)
{
    TreeWalkerImpl::NodePtr cursor;
    for (cursor = n->lastChild(); cursor; cursor = cursor->previousSibling()) {
        switch (isAccepted(cursor)) {
        case NodeFilter::FILTER_ACCEPT:
            return cursor;
        case NodeFilter::FILTER_SKIP: {
            // We ignore this candidate proper, but perhaps it has a kid?
            TreeWalkerImpl::NodePtr cand = getLastChild(cursor);
            if (cand)
                return cand;
            break;
        }
        case NodeFilter::FILTER_REJECT:
            // It effectively doesn't exist..
            break;
        }
        // ### this is unclear: what should happen if we "pass through" the root??
    }

    // Nothing found..
    return 0;
}

TreeWalkerImpl::NodePtr TreeWalkerImpl::getNextSibling(TreeWalkerImpl::NodePtr n)
{
    // If we're the root node, clearly we don't have any siblings.
    if (n == m_rootNode)
        return 0;

    // What would can our siblings be? Well, they can be our actual siblings,
    // or if those are 'skip' their first kid. We may also have to walk up
    // through any skipped nodes. (The behavior of going through rejected nodes is unspecified,
    // we'll keep backwards compat and stop).
    NodePtr cursor;
    for (cursor = n->nextSibling(); cursor; cursor = cursor->nextSibling()) {
        switch (isAccepted(cursor)) {
        case NodeFilter::FILTER_ACCEPT:
            return cursor;
        case NodeFilter::FILTER_SKIP: {
            NodePtr cand = getFirstChild(cursor);
            if (cand)
                return cand;
            break;
        }
        case NodeFilter::FILTER_REJECT:
            break;
        }
    }

    // Now, we have scanned through all of our parent's kids. Our siblings may also be
    // above if we could have skipped the parent, and are not captured by it.
    NodePtr parent = n->parentNode();
    if (!parent || parent == m_rootNode)
        return 0;

    /* "In particular: If the currentNode becomes part of a subtree that would otherwise have
        been Rejected by the filter, that entire subtree may be added as transient members of the
        logical view. You will be able to navigate within that subtree (subject to all the usual
        filtering) until you move upward past the Rejected ancestor. The behavior is as if the Rejected
        node had only been Skipped (since we somehow wound up inside its subtree) until we leave it;
        thereafter, standard filtering applies.*/
    if (isAccepted(parent) == NodeFilter::FILTER_ACCEPT)
        return 0;

    return getNextSibling(parent);
}

TreeWalkerImpl::NodePtr TreeWalkerImpl::getPreviousSibling(TreeWalkerImpl::NodePtr n)
{
    // See the above for all the comments :-)
    if (n == m_rootNode)
        return 0;

    NodePtr cursor;
    for (cursor = n->previousSibling(); cursor; cursor = cursor->previousSibling()) {
        switch (isAccepted(cursor)) {
        case NodeFilter::FILTER_ACCEPT:
            return cursor;
        case NodeFilter::FILTER_SKIP: {
            NodePtr cand = getLastChild(cursor);
            if (cand)
                return cand;
            break;
        }
        case NodeFilter::FILTER_REJECT:
            break;
        }
    }

    NodePtr parent = n->parentNode();
    if (!parent || parent == m_rootNode)
        return 0;

    if (isAccepted(parent) == NodeFilter::FILTER_ACCEPT)
        return 0;

    return getPreviousSibling(parent);
}

