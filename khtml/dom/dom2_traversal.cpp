/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
 * (C) 2000 Frederik Holljen (frederik.holljen@hig.no)
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

#include "dom2_traversal.h"
#include "dom_node.h"
#include "dom_exception.h"
using namespace DOM;


NodeIterator::NodeIterator()
{
  filter = 0;
}

NodeIterator::NodeIterator(const NodeIterator &other)
{
  referenceNode = other.referenceNode;
  rootNode = other.rootNode;
  whatToShow = other.whatToShow;
  filter = other.filter;
  inFront = other.inFront;
  expandEntityReferences = other.expandEntityReferences;
}

NodeIterator::NodeIterator(Node n, NodeFilter *f)
{
  if( !n.isNull() )
    {
      rootNode = n;
      referenceNode = n;
      whatToShow = 0x0000FFFF;
      filter = f;
    }
}

NodeIterator::NodeIterator(Node n, long _whatToShow , NodeFilter *f)
{
    filter = f;
    whatToShow = _whatToShow;
    referenceNode = n;
    rootNode = n;
}

NodeIterator &NodeIterator::operator = (const NodeIterator &other)
{
  referenceNode = other.referenceNode;
  rootNode = other.rootNode;
  whatToShow = other.whatToShow;
  filter = other.filter;
  inFront = other.inFront;
  expandEntityReferences = other.expandEntityReferences;
  return *this;
}

NodeIterator::~NodeIterator()
{
  if(filter)
    {
    delete filter;
    filter = 0;
    }
}

void NodeIterator::setWhatToShow(long _whatToShow)
{
  whatToShow = _whatToShow;
}

void NodeIterator::moveReferenceNode(Node n)
{
  if( !n.isNull() )
    {
      // cheching if they are in the same subtree??
      referenceNode = n;
      inFront = true;
    }
}

void NodeIterator::setReferenceNode(Node n)
{
  if( !n.isNull() )
    {
      rootNode = n;
      referenceNode = n;
      inFront = true;
    }
}

void NodeIterator::setFilter(NodeFilter *_filter)
{
  if(_filter != 0)
    {
      if( filter != 0 )
        delete filter;

      filter = _filter;
    }
}

void NodeIterator::setExpandEntityReferences(bool value)
{
  expandEntityReferences = value;
}

Node NodeIterator::nextNode(  )
{
  short _result;
  Node _tempCurrent = getNextNode(referenceNode);
  while( !_tempCurrent.isNull() )
    {
      _result =  isAccepted(_tempCurrent);
      
      if(_result == NodeFilter::FILTER_ACCEPT)
        {
          referenceNode = _tempCurrent;
          return referenceNode;
        }
      _tempCurrent = getNextNode(_tempCurrent);
    }
  
  return Node();
}

Node NodeIterator::getNextNode(Node n)
{
  /*  1. my first child
   *  2. my next sibling
   *  3. my parents sibling, or their parents sibling (loop)
   *  4. not found
   */

  if( n.isNull() )
    return n;

  inFront = true;
  
  if( n.hasChildNodes() )
    return n.firstChild();

  if( !n.nextSibling().isNull() )
    return n.nextSibling();
  
  if( rootNode == n)
     return Node();
  
  Node parent = n.parentNode();
  while( !parent.isNull() )
    {
      n = parent.nextSibling();
      if( !n.isNull() )
        return n;

      if( rootNode == parent )
        return Node();

      parent = parent.parentNode();
    }
  return Node();      
}

Node NodeIterator::previousNode(  )
{
  short _result;
  Node _tempCurrent = getPreviousNode(referenceNode);
  while( !_tempCurrent.isNull() )
    {
      _result = isAccepted(_tempCurrent);
      if(_result == NodeFilter::FILTER_ACCEPT)
        {
          referenceNode = _tempCurrent;
          return referenceNode;
        }
      _tempCurrent = getPreviousNode(_tempCurrent);
    }
  
  return Node();
}

Node NodeIterator::getPreviousNode(Node n)
{
/* 1. my previous sibling.lastchild
 * 2. my previous sibling
 * 3. my parent
 */
  Node _tempCurrent;

  if( n.isNull() )
    return Node();

    inFront = false;
  
  _tempCurrent = n.previousSibling();
  if( !_tempCurrent.isNull() )
    {
      if( _tempCurrent.hasChildNodes() )
        {
          while( _tempCurrent.hasChildNodes() )
            _tempCurrent = _tempCurrent.lastChild();
          return _tempCurrent;
        }
      else
        return _tempCurrent;
    }
    

  if(n == rootNode)
    return Node();

  return n.parentNode();
  
}
void NodeIterator::deleteNode(Node n)
{
    if( n.isNull() )
        return;  // someone tried to delete a null node :)

    Node _tempDeleted = referenceNode;
    while( !_tempDeleted.isNull() && _tempDeleted != n) // did I get deleted, or one of my parents?
        _tempDeleted = _tempDeleted.parentNode();

    if( _tempDeleted.isNull() )  // someone that did consern me got deleted
        return;

    if( !inFront)
    {
        Node _next = getNextNode(_tempDeleted);
        if( !_next.isNull() )
            referenceNode = _next;
        else
        {
            inFront = false;
            referenceNode = getPreviousNode(_tempDeleted);
            return;
        }
    }
    referenceNode = getPreviousNode(_tempDeleted);
}

short NodeIterator::isAccepted(Node n)
{
  // if XML is implemented we have to check expandEntityRerefences in this function
  if( ( ( 1 << n.nodeType()-1) & whatToShow) != 0 ) 
    {
        if(filter)
            return filter->acceptNode(n);
        else
            return NodeFilter::FILTER_ACCEPT;
    }
    return NodeFilter::FILTER_SKIP;
}

// -----------------------------------------------------------

NodeFilter::NodeFilter()
{
}

NodeFilter::NodeFilter(const NodeFilter &/*other*/)
{
}

NodeFilter &NodeFilter::operator = (const NodeFilter &/*other*/)
{
    return *this;
}

NodeFilter::~NodeFilter()
{
}

short NodeFilter::acceptNode( const Node &/*n*/ )
{
    return NodeFilter::FILTER_ACCEPT;
}

// --------------------------------------------------------------

TreeWalker::TreeWalker()
{
    filter = 0;
    whatToShow = 0x0000FFFF;
    expandEntityReferences = true;
}

TreeWalker::TreeWalker(const TreeWalker &other)
{
    expandEntityReferences = other.expandEntityReferences;
    filter = other.filter;
    whatToShow = other.whatToShow;
    currentNode = other.currentNode;
    rootNode = other.rootNode;
}

TreeWalker::TreeWalker(Node n, NodeFilter *f)
{
  currentNode = n;
  rootNode = n;
  whatToShow = 0x0000FFFF;
  filter = f;
}

TreeWalker::TreeWalker(Node n, long _whatToShow, NodeFilter *f)
{
  currentNode = n;
  rootNode = n;
  whatToShow = _whatToShow;
  filter = f;
}

TreeWalker &TreeWalker::operator = (const TreeWalker &other)
{
  expandEntityReferences = other.expandEntityReferences;
  filter = other.filter;
  whatToShow = other.whatToShow;
  currentNode = other.currentNode;
  return *this;
}

TreeWalker::~TreeWalker()
{
    if(filter)
      {
        delete filter;
        filter = 0;
      }
}

void TreeWalker::setWhatToShow(long _whatToShow)
{
  // do some testing wether this is an accepted value
  whatToShow = _whatToShow;
}

void TreeWalker::setFilter(NodeFilter *_filter)
{
  if(_filter)
    filter = _filter;
}

void TreeWalker::setExpandEntityReferences(bool value)
{
  expandEntityReferences = value;
}

Node TreeWalker::getCurrentNode()
{
    return currentNode;;
}


void TreeWalker::setCurrentNode(/* const*/ Node n )
{
    if( n.isNull() )
        throw( DOMException::NOT_SUPPORTED_ERR );
    else
      {
        rootNode = n;
        currentNode = n;
      }
}

Node TreeWalker::parentNode(  )
{
    Node n = getParentNode(currentNode);
    if( !n.isNull() )
        currentNode = n;
    return n;
}


Node TreeWalker::firstChild(  )
{
    Node n = getFirstChild(currentNode);
    if( !n.isNull() )
        currentNode = n;
    return n;
}

 
Node TreeWalker::lastChild(  )
{
    Node n = getLastChild(currentNode);
    if( !n.isNull() )
        currentNode = n;
    return n;
}

Node TreeWalker::previousSibling(  )
{
    Node n = getPreviousSibling(currentNode);
    if( !n.isNull() )
        currentNode = n;
    return n;
}

Node TreeWalker::nextSibling(  )
{
    Node n = getNextSibling(currentNode);
    if( !n.isNull() )
        currentNode = n;
    return n;
}

Node TreeWalker::previousNode(  )
{
/* 1. my previous sibling.lastchild
 * 2. my previous sibling
 * 3. my parent
 */
    
    Node n = getPreviousSibling(currentNode);
    if( n.isNull() )
    {
        n = getParentNode(currentNode);
        if( !n.isNull() )      //parent
        {
            currentNode = n;
            return currentNode;
        }
        else                  // parent failed.. no previous node
            return Node();
    }

    Node child = getLastChild(n);
    if( !child.isNull() )     // previous siblings last child
    {
        currentNode = child;
        return currentNode;
    }
    else                      // previous sibling
    {
        currentNode = n;
        return currentNode;
    }
    return Node();            // should never get here!
}

Node TreeWalker::nextNode(  )  
{
/*  1. my first child
 *  2. my next sibling
 *  3. my parents sibling, or their parents sibling (loop)
 *  4. not found
 */

    Node n = getFirstChild(currentNode);
    if( !n.isNull()  ) // my first child
    {
        currentNode = n;
        return n;
    }
    
    n = getNextSibling(currentNode); // my next sibling
    if( !n.isNull() )
    {
        currentNode = n;
        return currentNode;
    }
    Node parent = getParentNode(currentNode);
    while( !parent.isNull() ) // parents sibling
    {
        n = getNextSibling(parent);
        if( !n.isNull() )
        {
          currentNode = n;
          return currentNode;
        }
        else
            parent = getParentNode(parent);
    }
    return Node();
}

short TreeWalker::isAccepted(Node n)
{
    // if XML is implemented we have to check expandEntityRerefences in this function
  if( ( ( 1 << n.nodeType()-1 ) & whatToShow) != 0 )    
    {
      if(filter)
        return filter->acceptNode(n);
      else
        return NodeFilter::FILTER_ACCEPT;
    }
  return NodeFilter::FILTER_SKIP;
}

Node TreeWalker::getParentNode(Node n)
{
     short _result = NodeFilter::FILTER_ACCEPT;

    if( n == rootNode /*|| n.isNull()*/ )
      return Node();
    
    Node _tempCurrent = n.parentNode();

    if( _tempCurrent.isNull() )
      return Node();
    
    _result = isAccepted(_tempCurrent );
    if(_result == NodeFilter::FILTER_ACCEPT)
      return _tempCurrent;       // match found
    
    return getParentNode(_tempCurrent);
}

Node TreeWalker::getFirstChild(Node n)
{
    short _result;

    if( n.isNull() || n.firstChild().isNull() )
        return Node();
    n = n.firstChild();

    _result = isAccepted(n);
        
    switch(_result)
    {
         case NodeFilter::FILTER_ACCEPT:
           return n;
           break;
        case NodeFilter::FILTER_SKIP:
          if( n.hasChildNodes() )
                return getFirstChild(n);
            else
                return getNextSibling(n);
            break;

        case NodeFilter::FILTER_REJECT:
            return getNextSibling(n);
            break;
    }
    return Node();      // should never get here!
}

Node TreeWalker::getLastChild(Node n)
{
    short _result;

    if( n.isNull() || n.lastChild().isNull() )
        return Node();
    n = n.lastChild();
    _result = isAccepted(n);
    switch(_result)
    {
        case NodeFilter::FILTER_ACCEPT:
            return n;
            break;

        case NodeFilter::FILTER_SKIP:
            if( n.hasChildNodes() )
                return getLastChild(n);
            else
                return getPreviousSibling(n);
            break;

        case NodeFilter::FILTER_REJECT:
            return getPreviousSibling(n);
            break;
    }
    return Node();
}

Node TreeWalker::getPreviousSibling(Node n)
{
    short _result;
    Node _tempCurrent;

    if( n.isNull() )
        return Node();
    //first the cases if we have a previousSibling
    _tempCurrent = n.previousSibling();
    if( !_tempCurrent.isNull() )
    {
        _result = isAccepted(_tempCurrent);
        switch(_result)
        {
            case NodeFilter::FILTER_ACCEPT:
                return _tempCurrent;
                break;

            case NodeFilter::FILTER_SKIP:
            {
                Node nskip = getLastChild(_tempCurrent);
                if( !nskip.isNull() )
                    return nskip;
                return getPreviousSibling(_tempCurrent);
                break;
            }

            case NodeFilter::FILTER_REJECT:
                return getPreviousSibling(_tempCurrent);
                break;
        }
    }
    // now the case if we don't have previous sibling
    else
    {
        _tempCurrent = _tempCurrent.parentNode();
        if(_tempCurrent.isNull() || _tempCurrent == rootNode)
            return Node();
        _result = isAccepted(_tempCurrent);
        if(_result == NodeFilter::FILTER_SKIP)
            return getPreviousSibling(_tempCurrent);
        
        return Node();    
    
    }
    return Node();  // should never get here!
}

Node TreeWalker::getNextSibling(Node n)
{
 short _result;
    Node _tempCurrent;

    if( n.isNull() || _tempCurrent == rootNode)
        return Node();
    
    _tempCurrent = n.nextSibling();
    if( !_tempCurrent.isNull() )
    {
        _result = isAccepted(_tempCurrent);
        switch(_result)
        {
            case NodeFilter::FILTER_ACCEPT:
                return _tempCurrent;
                break;

            case NodeFilter::FILTER_SKIP:
            {
                Node nskip = getFirstChild(_tempCurrent);
                if( !nskip.isNull() )
                    return nskip;
                return getNextSibling(_tempCurrent);
                break;
            }

            case NodeFilter::FILTER_REJECT:
                return getNextSibling(_tempCurrent);
                break;
        }
    }
    else
    {
        _tempCurrent = _tempCurrent.parentNode();
        if(_tempCurrent.isNull() || _tempCurrent == rootNode)
            return Node();
        _result = isAccepted(_tempCurrent);
        if(_result == NodeFilter::FILTER_SKIP)
            return getNextSibling(_tempCurrent);
        
        return Node();
    }
    return Node();
}
// -----------------------------------------------------------------------

DocumentTraversal::DocumentTraversal()
{
}

DocumentTraversal::DocumentTraversal(const DocumentTraversal &/*other*/)
{
}

DocumentTraversal &DocumentTraversal::operator = (const DocumentTraversal &other)
{
    DocumentTraversal::operator = (other);
    return *this;
}

DocumentTraversal::~DocumentTraversal()
{
}

NodeIterator DocumentTraversal::createNodeIterator( const Node &/*root*/, long /*whatToShow*/, 
						    const NodeFilter &/*filter*/, 
						    bool /*entityReferenceExpansion*/ )
{
    return NodeIterator();
}

TreeWalker DocumentTraversal::createTreeWalker( const Node &/*root*/, long /*whatToShow*/, 
						const NodeFilter &/*filter*/, 
						bool /*entityReferenceExpansion*/ )
{
    return TreeWalker();
}



