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
#include "dom2_traversal.h"
#include "Node.h"
using namespace DOM;


NodeIterator::NodeIterator()
{
}

NodeIterator::NodeIterator(const NodeIterator &other)
{
}

NodeIterator &NodeIterator::operator = (const NodeIterator &other)
{
    ::operator = (other);
    return *this;
}

NodeIterator::~NodeIterator()
{
}

long NodeIterator::whatToShow() const
{
}

NodeFilter NodeIterator::filter() const
{
}

bool NodeIterator::expandEntityReferences() const
{
}

Node NodeIterator::nextNode(  )
{
}

Node NodeIterator::previousNode(  )
{
}

// -----------------------------------------------------------

NodeFilter::NodeFilter()
{
}

NodeFilter::NodeFilter(const NodeFilter &other)
{
}

NodeFilter::NodeFilter(NodeFilterImpl *impl)
{
}

NodeFilter &NodeFilter::operator = (const NodeFilter &other)
{
    ::operator = (other);
    return *this;
}

NodeFilter::~NodeFilter()
{
}

short NodeFilter::acceptNode( const Node &n )
{
}

// --------------------------------------------------------------

TreeWalker::TreeWalker()
{
}

TreeWalker::TreeWalker(const TreeWalker &other)
{
}

TreeWalker &TreeWalker::operator = (const TreeWalker &other)
{
    ::operator = (other);
    return *this;
}

TreeWalker::~TreeWalker()
{
}

long TreeWalker::whatToShow() const
{
}

NodeFilter TreeWalker::filter() const
{
}

bool TreeWalker::expandEntityReferences() const
{
}

Node TreeWalker::currentNode() const
{
}

void TreeWalker::setCurrentNode( const Node &_currentNode )
{
}

Node TreeWalker::parentNode(  )
{
}

Node TreeWalker::firstChild(  )
{
}

Node TreeWalker::lastChild(  )
{
}

Node TreeWalker::previousSibling(  )
{
}

Node TreeWalker::nextSibling(  )
{
}

Node TreeWalker::previousNode(  )
{
}

Node TreeWalker::nextNode(  )
{
}

// -----------------------------------------------------------------------

DocumentTraversal::DocumentTraversal()
{
}

DocumentTraversal::DocumentTraversal(const DocumentTraversal &other)
{
}

DocumentTraversal &DocumentTraversal::operator = (const DocumentTraversal &other)
{
    ::operator = (other);
    return *this;
}

DocumentTraversal::~DocumentTraversal()
{
}

NodeIterator DocumentTraversal::createNodeIterator( const Node &root, long whatToShow, 
						    const NodeFilter &filter, 
						    bool entityReferenceExpansion )
{
}

TreeWalker DocumentTraversal::createTreeWalker( const Node &root, long whatToShow, 
						const NodeFilter &filter, 
						bool entityReferenceExpansion )
{
}



