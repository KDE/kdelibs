/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
 * (C) 2000 Gunnstein Lye (gunnstein@netcom.no)
 * (C) 2000 Frederik Holljen (frederik.holljen@hig.no)
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
#include "dom2_range.h"
#include "dom2_traversal.h"
#include "dom_node.h"
#include "dom_doc.h"
#include "dom_string.h"
#include "dom_text.h"
#include "dom_exception.h"
#include "dom_docimpl.h"
#include "dom2_rangeimpl.h"
#include <qstring.h>
#include <stdio.h>               // for printf

using namespace DOM;


Range::Range()
{
    impl = 0;
}

Range::Range(const Document rootContainer)
{
    impl = new RangeImpl(rootContainer);
    impl->ref();
}

Range::Range(const Range &other)
{
    impl = other.impl;
    if (impl) impl->ref();
}

Range::Range(const Node startContainer, const long startOffset, const Node endContainer, const long endOffset)
{
    impl = new RangeImpl(startContainer,startOffset,endContainer,endOffset);
    impl->ref();
}

Range::Range(RangeImpl *i)
{
    impl = i;
    if (impl) impl->ref();
}

Range &Range::operator = (const Range &other)
{
    if (impl) impl->deref();
    impl = other.impl;
    if (impl) impl->ref();

    return *this;
}

Range::~Range()
{
    if (impl) impl->deref();
}

Node Range::startContainer() const
{
    if (impl) impl->getStartContainer();
    return 0;
}

long Range::startOffset() const
{
    if (impl) return impl->getStartOffset();
    return 0;
}

Node Range::endContainer() const
{
    if (impl) return impl->getEndContainer();
    return 0;
}

long Range::endOffset() const
{
    if (impl) return impl->getEndOffset();
    return 0;
}

bool Range::collapsed() const
{
    if (impl) return impl->getCollapsed();
    return false;
}

Node Range::commonAncestorContainer()
{
    if (impl) return impl->getCommonAncestorContainer();
    return 0;
}

void Range::setStart( const Node &refNode, long offset )
{
    if (impl) impl->setStart(refNode,offset);
}

void Range::setEnd( const Node &refNode, long offset )
{
    if (impl) impl->setEnd(refNode,offset);
}

void Range::setStartBefore( const Node &refNode )
{
    if (impl) impl->setStartBefore(refNode);
}

void Range::setStartAfter( const Node &refNode )
{
    if (impl) impl->setStartAfter(refNode);
}

void Range::setEndBefore( const Node &refNode )
{
    if (impl) impl->setEndBefore(refNode);
}

void Range::setEndAfter( const Node &refNode )
{
    if (impl) impl->setEndAfter(refNode);
}

void Range::collapse( bool toStart )
{
    if (impl) impl->collapse(toStart);
}

void Range::selectNode( const Node &refNode )
{
    if (impl) impl->selectNode(refNode);
}

void Range::selectNodeContents( const Node &refNode )
{
    if (impl) impl->selectNodeContents(refNode);
}

short Range::compareBoundaryPoints( CompareHow how, const Range &sourceRange )
{
    if (impl) return impl->compareBoundaryPoints(how,sourceRange);
    return 0;
}

bool Range::boundaryPointsValid(  )
{
    if (impl) return impl->boundaryPointsValid();
    return false;
}

void Range::deleteContents(  )
{
    if (impl) impl->deleteContents();
}

DocumentFragment Range::extractContents(  )
{
    if (impl) return impl->extractContents();
    return 0;
}

DocumentFragment Range::cloneContents(  )
{
    if (impl) return impl->cloneContents();
    return 0;
}

void Range::insertNode( const Node &newNode )
{
    if (impl) impl->insertNode(newNode);
}

void Range::surroundContents( const Node &newParent )
{
    if (impl) impl->surroundContents( newParent );
}

Range Range::cloneRange(  )
{
    if (impl) return impl->cloneRange();
    return 0;
}

DOMString Range::toString(  )
{
    if (impl) return impl->toString();
    return 0;
}

DOMString Range::toHTML(  )
{
    if (impl) return impl->toHTML();
    return 0;
}

void Range::detach(  )
{
    if (impl) impl->detach(); // ### get rid of impl instead? (thereby becoming null)
}

bool Range::isDetached() const
{
    if (impl) return isDetached();
    return false;
}

RangeImpl *Range::handle() const
{
    return impl;
}

bool Range::isNull() const
{
    return (impl == 0);
}

// ---------------------------------------------------------------
/*
DocumentRange::DocumentRange()
    : Document()
{
}

DocumentRange::DocumentRange(const DocumentRange &other)
    : Document(other)
{
}

DocumentRange &DocumentRange::operator = (const DocumentRange &other)
{
    Document::operator = (other);
    return *this;
}

DocumentRange::~DocumentRange()
{
}

//Range DocumentRange::createRange(  )
//{
//}

*/

