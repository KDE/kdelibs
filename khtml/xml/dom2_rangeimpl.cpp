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

#include "dom/dom2_traversal.h"
#include "dom/dom_node.h"
#include "dom/dom_doc.h"
#include "dom/dom_string.h"
#include "dom/dom_text.h"
#include "dom/dom_exception.h"
#include "dom_docimpl.h"
#include "dom2_rangeimpl.h"
#include "dom2_traversalimpl.h"
#include <qstring.h>
#include <stdio.h>               // for printf

using namespace DOM;


RangeImpl::RangeImpl()
{
    startContainer = 0;
    endContainer = 0;
    startOffset = 0;
    endOffset = 0;
    commonAncestorContainer = 0;
    collapsed = true;
    detached = false;
}

RangeImpl::RangeImpl(DocumentImpl */*rootContainer*/)
{
    // ###
}

RangeImpl::RangeImpl(const Document rootContainer)
{
    ownerDocument = rootContainer;
    startContainer = (Node)rootContainer;
    endContainer = (Node)rootContainer;
    startOffset = 0;
    endOffset = 0;
    commonAncestorContainer = (Node)rootContainer;
    collapsed = true;
    detached = false;
}

RangeImpl::RangeImpl(const RangeImpl &other) : DomShared(other)
{
    ownerDocument = other.ownerDocument;
    startContainer = other.startContainer;
    startOffset = other.startOffset;
    endContainer = other.endContainer;
    endOffset = other.endOffset;
    commonAncestorContainer = other.commonAncestorContainer;
    collapsed = other.collapsed;
    detached = false;
}

RangeImpl::RangeImpl(const Node sc, const long so, const Node ec, const long eo)
{
    startContainer = sc;
    startOffset = so;
    endContainer = ec;
    endOffset = eo;
    commonAncestorContainer = getCommonAncestorContainer();
    if( startContainer == endContainer && startOffset == endOffset )  collapsed = true;
    else  collapsed = false;
    detached = false;
}

RangeImpl &RangeImpl::operator = (const RangeImpl &other)
{
    ownerDocument = other.ownerDocument;
    startContainer = other.startContainer;
    startOffset = other.startOffset;
    endContainer = other.endContainer;
    endOffset = other.endOffset;
    commonAncestorContainer = other.commonAncestorContainer;
    collapsed = other.collapsed;
    detached = false;
    return *this;
}

RangeImpl::~RangeImpl()
{
}

Node RangeImpl::getStartContainer() const
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);
    else
        return startContainer;
}

long RangeImpl::getStartOffset() const
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);
    else
        return startOffset;
}

Node RangeImpl::getEndContainer() const
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);
    else
        return endContainer;
}

long RangeImpl::getEndOffset() const
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);
    else
        return endOffset;
}

Node RangeImpl::getCommonAncestorContainer() /*const*/
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);

    Node parentStart = startContainer;
    Node parentEnd = endContainer;

    while( !parentStart.isNull() && (parentStart != parentEnd) )
    {
        while( !parentEnd.isNull() && (parentStart != parentEnd) )
            parentEnd = parentEnd.parentNode();

        if(parentStart == parentEnd)  break;
        parentStart = parentStart.parentNode();
        parentEnd = endContainer;
    }

    if(parentStart == parentEnd)
        commonAncestorContainer = parentStart;
    else
    {
        return Node();
    }

    return commonAncestorContainer;
}

bool RangeImpl::getCollapsed() const
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);
    else
        return collapsed;
}

void RangeImpl::setStart( const Node &refNode, long offset )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw DOMException( DOMException::NOT_FOUND_ERR );

    while( !_tempNode.isNull() )
    {
        if( _tempNode.nodeType() == Node::ATTRIBUTE_NODE ||
            _tempNode.nodeType() == Node::ENTITY_NODE ||
            _tempNode.nodeType() == Node::NOTATION_NODE ||
            _tempNode.nodeType() == Node::DOCUMENT_TYPE_NODE )
            throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

        _tempNode = _tempNode.parentNode();
    }

    if( offset < 0 )
        throw DOMException( DOMException::INDEX_SIZE_ERR );

    if( refNode.nodeType() != Node::TEXT_NODE )
    {
	if( (unsigned)offset > refNode.childNodes().length() )
            throw DOMException( DOMException::INDEX_SIZE_ERR );
    }
    else
    {
	Text t;
	t = refNode;
        if( t.isNull() || (unsigned)offset > t.length() )
            throw DOMException( DOMException::INDEX_SIZE_ERR );
    }

    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    startContainer = refNode;
    startOffset = offset;

    if( endContainer != 0 )
    {
        if( commonAncestorContainer != 0 )
        {
            Node oldCommonAncestorContainer = commonAncestorContainer;
            if( oldCommonAncestorContainer != getCommonAncestorContainer() )
                collapse( true );
            if( !boundaryPointsValid() )
                collapse( true );
        }
        else  getCommonAncestorContainer();
    }
}

void RangeImpl::setEnd( const Node &refNode, long offset )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw DOMException( DOMException::NOT_FOUND_ERR );

    while( !_tempNode.isNull() )
    {
        if( _tempNode.nodeType() == Node::ATTRIBUTE_NODE ||
            _tempNode.nodeType() == Node::ENTITY_NODE ||
            _tempNode.nodeType() == Node::NOTATION_NODE ||
            _tempNode.nodeType() == Node::DOCUMENT_TYPE_NODE )
            throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

        _tempNode = _tempNode.parentNode();
    }

    if( offset < 0 )
        throw DOMException( DOMException::INDEX_SIZE_ERR );

    if( refNode.nodeType() != Node::TEXT_NODE )
    {
	if( (unsigned)offset > refNode.childNodes().length() )
            throw DOMException( DOMException::INDEX_SIZE_ERR );
    }
    else
    {
	Text t;
	t = refNode;
        if( t.isNull() || (unsigned)offset > t.length() )
            throw DOMException( DOMException::INDEX_SIZE_ERR );
    }

    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    endContainer = refNode;
    endOffset = offset;

    if( startContainer != 0 )
    {
        if( commonAncestorContainer != 0 )
        {
            Node oldCommonAncestorContainer = commonAncestorContainer;
            if( oldCommonAncestorContainer != getCommonAncestorContainer() )
                collapse( true );
            if( !boundaryPointsValid() )
                collapse( true );
        }
        else  getCommonAncestorContainer();
    }
}

void RangeImpl::setStartBefore( const Node &refNode )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw DOMException( DOMException::NOT_FOUND_ERR );

    _tempNode = refNode.parentNode();
    while( !_tempNode.isNull() )
    {
        if( _tempNode.nodeType() == Node::ATTRIBUTE_NODE ||
            _tempNode.nodeType() == Node::ENTITY_NODE ||
            _tempNode.nodeType() == Node::NOTATION_NODE ||
            _tempNode.nodeType() == Node::DOCUMENT_TYPE_NODE )
            throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

        _tempNode = _tempNode.parentNode();
    }

    if( refNode.nodeType() == Node::DOCUMENT_NODE ||
        refNode.nodeType() == Node::DOCUMENT_FRAGMENT_NODE ||
        refNode.nodeType() == Node::ATTRIBUTE_NODE ||
        refNode.nodeType() == Node::ENTITY_NODE ||
        refNode.nodeType() == Node::NOTATION_NODE )
        throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    try
    {
        setStart( refNode.parentNode(), refNode.index() );
    }
    catch( RangeException r )
    {
        if( r.code == RangeException::INVALID_NODE_TYPE_ERR )
            fprintf( stderr, "Exception: Invalid Node type\n" );
        return;
    }
    catch( DOMException d )
    {
        if( d.code == DOMException::NOT_FOUND_ERR )
            fprintf( stderr, "Exception: Null Nodes not accepted\n" );
        if( d.code == DOMException::INDEX_SIZE_ERR )
            fprintf( stderr, "Exception: offset has wrong size\n" );
        if( d.code == DOMException::INVALID_STATE_ERR )
            fprintf( stderr, "Exception: detach() has been invoked\n" );
        return;
    }
}

void RangeImpl::setStartAfter( const Node &refNode )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw DOMException( DOMException::NOT_FOUND_ERR );

    _tempNode = refNode.parentNode();
    while( !_tempNode.isNull() )
    {
        if( _tempNode.nodeType() == Node::ATTRIBUTE_NODE ||
            _tempNode.nodeType() == Node::ENTITY_NODE ||
            _tempNode.nodeType() == Node::NOTATION_NODE ||
            _tempNode.nodeType() == Node::DOCUMENT_TYPE_NODE )
            throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

        _tempNode = _tempNode.parentNode();
    }

    if( refNode.nodeType() == Node::DOCUMENT_NODE ||
        refNode.nodeType() == Node::DOCUMENT_FRAGMENT_NODE ||
        refNode.nodeType() == Node::ATTRIBUTE_NODE ||
        refNode.nodeType() == Node::ENTITY_NODE ||
        refNode.nodeType() == Node::NOTATION_NODE )
        throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    try
    {
        setStart( refNode.parentNode(), refNode.index()+1 );
    }
    catch( RangeException r )
    {
        if( r.code == RangeException::INVALID_NODE_TYPE_ERR )
            fprintf( stderr, "Exception: Invalid Node type\n" );
        return;
    }
    catch( DOMException d )
    {
        if( d.code == DOMException::NOT_FOUND_ERR )
            fprintf( stderr, "Exception: Null Nodes not accepted\n" );
        if( d.code == DOMException::INDEX_SIZE_ERR )
            fprintf( stderr, "Exception: offset has wrong size\n" );
        if( d.code == DOMException::INVALID_STATE_ERR )
            fprintf( stderr, "Exception: detach() has been invoked\n" );
        return;
    }
}

void RangeImpl::setEndBefore( const Node &refNode )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw DOMException( DOMException::NOT_FOUND_ERR );

    _tempNode = refNode.parentNode();
    while( !_tempNode.isNull() )
    {
        if( _tempNode.nodeType() == Node::ATTRIBUTE_NODE ||
            _tempNode.nodeType() == Node::ENTITY_NODE ||
            _tempNode.nodeType() == Node::NOTATION_NODE ||
            _tempNode.nodeType() == Node::DOCUMENT_TYPE_NODE )
            throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

        _tempNode = _tempNode.parentNode();
    }

    if( refNode.nodeType() == Node::DOCUMENT_NODE ||
        refNode.nodeType() == Node::DOCUMENT_FRAGMENT_NODE ||
        refNode.nodeType() == Node::ATTRIBUTE_NODE ||
        refNode.nodeType() == Node::ENTITY_NODE ||
        refNode.nodeType() == Node::NOTATION_NODE )
        throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    try
    {
        setEnd( refNode.parentNode(), refNode.index() );
    }
    catch( RangeException r )
    {
        if( r.code == RangeException::INVALID_NODE_TYPE_ERR )
            fprintf( stderr, "Exception: Invalid Node type\n" );
        return;
    }
    catch( DOMException d )
    {
        if( d.code == DOMException::NOT_FOUND_ERR )
            fprintf( stderr, "Exception: Null Nodes not accepted\n" );
        if( d.code == DOMException::INDEX_SIZE_ERR )
            fprintf( stderr, "Exception: offset has wrong size\n" );
        if( d.code == DOMException::INVALID_STATE_ERR )
            fprintf( stderr, "Exception: detach() has been invoked\n" );
        return;
    }
}

void RangeImpl::setEndAfter( const Node &refNode )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw DOMException( DOMException::NOT_FOUND_ERR );

    _tempNode = refNode.parentNode();
    while( !_tempNode.isNull() )
    {
        if( _tempNode.nodeType() == Node::ATTRIBUTE_NODE ||
            _tempNode.nodeType() == Node::ENTITY_NODE ||
            _tempNode.nodeType() == Node::NOTATION_NODE ||
            _tempNode.nodeType() == Node::DOCUMENT_TYPE_NODE )
            throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

        _tempNode = _tempNode.parentNode();
    }

    if( refNode.nodeType() == Node::DOCUMENT_NODE ||
        refNode.nodeType() == Node::DOCUMENT_FRAGMENT_NODE ||
        refNode.nodeType() == Node::ATTRIBUTE_NODE ||
        refNode.nodeType() == Node::ENTITY_NODE ||
        refNode.nodeType() == Node::NOTATION_NODE )
        throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    try
    {
        setEnd( refNode.parentNode(), refNode.index()+1 );
    }
    catch( RangeException r )
    {
        if( r.code == RangeException::INVALID_NODE_TYPE_ERR )
            fprintf( stderr, "Exception: Invalid Node type\n" );
        return;
    }
    catch( DOMException d )
    {
        if( d.code == DOMException::NOT_FOUND_ERR )
            fprintf( stderr, "Exception: Null Nodes not accepted\n" );
        if( d.code == DOMException::INDEX_SIZE_ERR )
            fprintf( stderr, "Exception: offset has wrong size\n" );
        if( d.code == DOMException::INVALID_STATE_ERR )
            fprintf( stderr, "Exception: detach() has been invoked\n" );
        return;
    }
}

void RangeImpl::collapse( bool toStart )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    if( toStart )   // collapse to start
    {
        endContainer = startContainer;
        endOffset = startOffset;
        collapsed = true;
        commonAncestorContainer = startContainer;
    }
    else            // collapse to end
    {
        startContainer = endContainer;
        startOffset = endOffset;
        collapsed = true;
        commonAncestorContainer = endContainer;
    }
}

void RangeImpl::selectNode( const Node &refNode )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw DOMException( DOMException::NOT_FOUND_ERR );

    _tempNode = refNode.parentNode();
    while( !_tempNode.isNull() )
    {
        if( _tempNode.nodeType() == Node::ATTRIBUTE_NODE ||
            _tempNode.nodeType() == Node::ENTITY_NODE ||
            _tempNode.nodeType() == Node::NOTATION_NODE ||
            _tempNode.nodeType() == Node::DOCUMENT_TYPE_NODE )
            throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

        _tempNode = _tempNode.parentNode();
    }

    if( refNode.nodeType() == Node::DOCUMENT_NODE ||
        refNode.nodeType() == Node::DOCUMENT_FRAGMENT_NODE ||
        refNode.nodeType() == Node::ATTRIBUTE_NODE ||
        refNode.nodeType() == Node::ENTITY_NODE ||
        refNode.nodeType() == Node::NOTATION_NODE )
        throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    try
    {
        setStartBefore( refNode );
        setEndAfter( refNode );
    }
    catch( RangeException r )
    {
        if( r.code == RangeException::INVALID_NODE_TYPE_ERR )
            fprintf( stderr, "Exception: Invalid Node type\n" );
        return;
    }
    catch( DOMException d )
    {
        if( d.code == DOMException::NOT_FOUND_ERR )
            fprintf( stderr, "Exception: Null Nodes not accepted\n" );
        if( d.code == DOMException::INVALID_STATE_ERR )
            fprintf( stderr, "Exception: detach() has been invoked\n" );
        return;
    }
}

void RangeImpl::selectNodeContents( const Node &refNode )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw DOMException( DOMException::NOT_FOUND_ERR );

    while( !_tempNode.isNull() )
    {
        if( _tempNode.nodeType() == Node::ATTRIBUTE_NODE ||
            _tempNode.nodeType() == Node::ENTITY_NODE ||
            _tempNode.nodeType() == Node::NOTATION_NODE ||
            _tempNode.nodeType() == Node::DOCUMENT_TYPE_NODE )
            throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

        _tempNode = _tempNode.parentNode();
    }

    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    try
    {
        setStartBefore( refNode.firstChild() );
        setEndAfter( refNode.lastChild() );
    }
    catch( RangeException r )
    {
        if( r.code == RangeException::INVALID_NODE_TYPE_ERR )
            fprintf( stderr, "Exception: Invalid Node type\n" );
        return;
    }
    catch( DOMException d )
    {
        if( d.code == DOMException::NOT_FOUND_ERR )
            fprintf( stderr, "Exception: Null Nodes not accepted\n" );
        if( d.code == DOMException::INVALID_STATE_ERR )
            fprintf( stderr, "Exception: detach() has been invoked\n" );
        return;
    }
}

short RangeImpl::compareBoundaryPoints( Range::CompareHow how, const Range &sourceRange )
{
    if( commonAncestorContainer.ownerDocument() != sourceRange.handle()->commonAncestorContainer.ownerDocument() )
        throw DOMException( DOMException::WRONG_DOCUMENT_ERR );

    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    switch(how)
    {
    case Range::START_TO_START:
        return compareBoundaryPoints( getStartContainer(), getStartOffset(),
                                      sourceRange.startContainer(), sourceRange.startOffset() );
        break;
    case Range::START_TO_END:
        return compareBoundaryPoints( getStartContainer(), getStartOffset(),
                                      sourceRange.endContainer(), sourceRange.endOffset() );
        break;
    case Range::END_TO_END:
        return compareBoundaryPoints( getEndContainer(), getEndOffset(),
                                      sourceRange.endContainer(), sourceRange.endOffset() );
        break;
    case Range::END_TO_START:
        return compareBoundaryPoints( getEndContainer(), getEndOffset(),
                                      sourceRange.startContainer(), sourceRange.startOffset() );
        break;
    default:
        printf( "Function compareBoundaryPoints: Invalid CompareHow\n" );
        return 2;     // undocumented - should throw an exception here
    }
}

short RangeImpl::compareBoundaryPoints( Node containerA, long offsetA, Node containerB, long offsetB )
{
    if( offsetA < 0 || offsetB < 0 )
    {
        printf( "Function compareBoundaryPoints: No negative offsets allowed\n" );
        return 2;     // undocumented - should throw an exception here
    }

    if( containerA == containerB )
    {
        if( offsetA == offsetB )  return 0;    // A is equal to B
        if( offsetA < offsetB )  return -1;    // A is before B
        else  return 1;                        // A is after B
    }

    Node n = containerA;
    while( n != 0 )
    {
        if( n == containerB)  return -1;       // A is before B

        Node next;
        if( n == containerA )  next = n.childNodes().item( offsetA );
        else  next = n.firstChild();
        if( next == 0 )  next = n.nextSibling();
        while( n != 0 && next == 0 )
        {
            n = n.parentNode();
            next = n.nextSibling();
        }
        n = next;
    }
    return 1;                                  // A is after B
}

bool RangeImpl::boundaryPointsValid(  )
{
    short valid =  compareBoundaryPoints( getStartContainer(), getStartOffset(),
                                          getEndContainer(), getEndOffset() );
    if( valid == 1 )  return false;
    else  return true;

}

void RangeImpl::deleteContents(  )
{
    Node cmnRoot = getCommonAncestorContainer();
    printf("CommonAC: %s \n", cmnRoot.nodeName().string().ascii());
//    printf("end: %d, start: %d", startOffset, endOffset);

    if(startContainer == endContainer)
    {
        if(startOffset == endOffset)            // we have a collapsed range
        {printf("collapsed\n");return;}

        // TODO: we need to delete the text Node if a whole text is selected!!
        if( startContainer.nodeType() == Node::TEXT_NODE )
        {
            startContainer.nodeValue().remove(startOffset, endOffset);
            startContainer.applyChanges();
        }
        else
        {
            printf("same but not a text node\n");
            Node _tempParent = startContainer;
            Node _tempCurrent = startContainer.firstChild();
            unsigned int i;

            for(i=0; i < startOffset; i++)    // get the node given by the offset
                _tempCurrent = _tempCurrent.nextSibling();

            /* now delete all nodes between the offsets */
            unsigned int range = endOffset - startOffset;
            Node _nextCurrent = _tempCurrent;                  // to keep track of which node to take next

            for(i=0; i<range && !_tempCurrent.isNull(); i++)
            {
                if(_tempParent == _tempCurrent.parentNode() )
                    printf("like\n");
                _nextCurrent = _tempCurrent.nextSibling();
                printf("just before remove\n");
                _tempParent.removeChild(_tempCurrent);
                printf("just after remove\n");
                _tempCurrent = _nextCurrent;
            }
        _tempParent.applyChanges();
        }
        return;
    }// END COMMON CONTAINER CASE!!

    printf("end common case\n");
    Node _nextCurrent;
    Node _tempCurrent;

    // cleanup left side
    Node _leftParent = startContainer;
    if( startContainer.nodeType() == Node::TEXT_NODE )
    {
        printf("left side text\n");
        (void)startContainer.nodeValue().split(startOffset); // what about complete removals?
    }
    else
    {

        _tempCurrent = startContainer.firstChild();
        unsigned int i;

        for(i=0; i < startOffset; i++)    // get the node given by the offset
            _tempCurrent = _tempCurrent.nextSibling();

        _nextCurrent = _tempCurrent;                  // to keep track of which node to take next

        while( !_tempCurrent.isNull() )
        {
            _nextCurrent = _tempCurrent.nextSibling();
            _leftParent.removeChild(_tempCurrent);
            _tempCurrent = _nextCurrent;
        }
    }
    _tempCurrent = _leftParent;
    _leftParent = _leftParent.parentNode();
    while( _leftParent != cmnRoot )
    {
        while( !_tempCurrent.isNull() )
        {
            _nextCurrent = _tempCurrent.nextSibling();
            _leftParent.removeChild(_tempCurrent);
            _tempCurrent = _nextCurrent;
        }
        _tempCurrent = _leftParent;
        _leftParent = _leftParent.parentNode();
    }


    // cleanup right side
    Node _rightParent = endContainer;
    if( endContainer.nodeType() == Node::TEXT_NODE )
    {
        endContainer.nodeValue().remove(0, endOffset); // what about complete removals?
    }
    else
    {

        Node _tempCurrent = endContainer.firstChild();
        unsigned int i;

        for(i=0; i < endOffset; i++)    // get the node given by the offset
            _tempCurrent = _tempCurrent.nextSibling();

        Node _nextCurrent = _tempCurrent;                  // to keep track of which node to take next

        while( !_tempCurrent.isNull() )
        {
            _nextCurrent = _tempCurrent.previousSibling();
            _leftParent.removeChild(_tempCurrent);
            _tempCurrent = _nextCurrent;
        }
    }
    _tempCurrent = _rightParent;
    _rightParent = _rightParent.parentNode();
    while( _rightParent != cmnRoot )
    {
        while( !_tempCurrent.isNull() )
        {
            _nextCurrent = _tempCurrent.previousSibling();
            _rightParent.removeChild(_tempCurrent);
            _tempCurrent = _nextCurrent;
        }
        _tempCurrent = _rightParent;
        _rightParent = _rightParent.parentNode();
    }

    // cleanup middle
    _leftParent = _leftParent.nextSibling();
    while( _leftParent != _rightParent )
    {
        cmnRoot.removeChild(_leftParent);
        _leftParent = _leftParent.nextSibling();
    }


    // FIXME! this allways collapses to the front (see DOM specs)
    //collapse(true);
    return;

}

DocumentFragment RangeImpl::extractContents(  )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    return masterTraverse( true );
}

DocumentFragment RangeImpl::cloneContents(  )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    return masterTraverse( false );
}

void RangeImpl::insertNode( const Node &newNode )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    if( newNode.nodeType() == Node::ATTRIBUTE_NODE ||
        newNode.nodeType() == Node::ENTITY_NODE ||
        newNode.nodeType() == Node::NOTATION_NODE ||
        newNode.nodeType() == Node::DOCUMENT_NODE ||
        newNode.nodeType() == Node::DOCUMENT_FRAGMENT_NODE)
        throw RangeException( RangeException::INVALID_NODE_TYPE_ERR);

    if( newNode.ownerDocument() != startContainer.ownerDocument() )
        throw DOMException( DOMException::WRONG_DOCUMENT_ERR );

    if( startContainer.nodeType() == Node::TEXT_NODE )
    {
        Text newText;
        Node newParent = newNode.parentNode();
        Text textNode = static_cast<Text>(startContainer);
        newText = textNode.splitText(startOffset);
        newParent.insertBefore( newNode, newText );        
    }
    else
        startContainer.insertBefore( newNode, startContainer.childNodes().item( startOffset ) );
}

void RangeImpl::surroundContents( const Node &newParent )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    if( newParent.isNull() )
        return;

    if( newParent.ownerDocument() != startContainer.ownerDocument() )
        throw DOMException( DOMException::WRONG_DOCUMENT_ERR );
    
    if( newParent.nodeType() == Node::ATTRIBUTE_NODE ||
        newParent.nodeType() == Node::ENTITY_NODE ||
        newParent.nodeType() == Node::NOTATION_NODE ||
        newParent.nodeType() == Node::DOCUMENT_TYPE_NODE ||
        newParent.nodeType() == Node::DOCUMENT_NODE ||
        newParent.nodeType() == Node::DOCUMENT_FRAGMENT_NODE)
        throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

    // revisit: if you set a range without optimizing it (trimming) the following exception might be
    // thrown incorrectly
    Node realStart = (startContainer.nodeType() == Node::TEXT_NODE)? startContainer.parentNode() : startContainer;
    Node realEnd = (endContainer.nodeType() == Node::TEXT_NODE)? endContainer.parentNode() : endContainer;
    if( realStart != realEnd )
        throw RangeException( RangeException::BAD_BOUNDARYPOINTS_ERR );

    DocumentFragment fragment = extractContents();
    insertNode( newParent );
    // BIC: to avoid this const_cast newParent shouldn't be const
    (const_cast<Node>(newParent)).appendChild( fragment );
    selectNode( newParent );
    
    
}

Range RangeImpl::cloneRange(  )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    return Range( this );
}

DOMString RangeImpl::toString(  )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    NodeIteratorImpl iterator( getStartContainer().childNodes().item( getStartOffset() ) );
    DOMString _string;
    Node _node = iterator.nextNode();

    while( !_node.isNull() )
    {
        printf( "\nNodetype: %s\n", _node.nodeName().string().ascii() );
        if( _node.nodeType() == Node::TEXT_NODE )
        {
            QString str = _node.nodeValue().string();
            if( _node == getStartContainer() && _node == getEndContainer() )
                _string = str.mid( getStartOffset(), getEndOffset() - getStartOffset() );
            else if( _node == getStartContainer() )
                _string = str.mid( getStartOffset() );
            else if( _node == getEndContainer() )
                _string += str.left( getStartOffset() );
            else
                _string += str;
        }
        else if( _node.nodeName() == "BR" )  _string += "\n";
        else if( _node.nodeName() == "P" || _node.nodeName() == "TD" )  _string += "\n\n";
        else  _string += " ";

        _node = iterator.nextNode();
    }
    return _string;
}

DOMString RangeImpl::toHTML(  )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    // this is just to avoid compiler warnings
    DOMString d;
    return d;
}

void RangeImpl::detach(  )
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);
    else
        detached = true;
}

bool RangeImpl::isDetached() const
{
    return detached;
}

DocumentFragment RangeImpl::masterTraverse(bool contentExtract)
{
    /* function description easy case, startContainer == endContainer
     * If we have a text node simply clone/extract the contents between
     * start & end and put them into the fragment
     * If we don't have a text node, find the offset and copy/clone the content
     * between the two offsets
     * We end with returning the fragment of course
     */
    Node _clone;
    DocumentFragment _endFragment( ownerDocument.createDocumentFragment() );

    if(startContainer == endContainer)
    {
        if(startOffset == endOffset)            // we have a collapsed range
            return DocumentFragment();

        // TODO: we need to delete the text Node if a whole text is selected!!
        if( startContainer.nodeType() == Node::TEXT_NODE )    // we have a text node.. special :)
        {
            _clone = startContainer.cloneNode(false);
            _clone.nodeValue().remove(0, startOffset);  // we need to get the SUBSTRING
            _clone.nodeValue().remove(endOffset, _clone.nodeValue().length() - endOffset);
            if(contentExtract)
            {
                // full trim :)
                startContainer.nodeValue().remove(startOffset, endOffset - startOffset);
            }
            _endFragment.appendChild(_clone);
        }
        else  // we have the same container class but we are not a text node
        {
            Node _tempCurrent = startContainer.firstChild();
            unsigned int i;

            for(i=0; i < startOffset; i++)    // get the node given by the offset
                _tempCurrent = _tempCurrent.nextSibling();

            /* now copy (or move) all the nodes in the range into the document fragment */
            unsigned int range = endOffset - startOffset;
            Node _nextCurrent = _tempCurrent;                  // to keep track of which node to take next
            for(i=0; i<range && !_tempCurrent.isNull(); i++)   // check of isNull in case of strange errors
            {
                _nextCurrent = _tempCurrent.nextSibling();

                if(contentExtract)
                {
                    _endFragment.appendChild(_tempCurrent);
                }
                else
                {
                    _clone = _tempCurrent.cloneNode(true);
                    _endFragment.appendChild(_clone);
                }

                _tempCurrent = _nextCurrent;
            }
        }
        return _endFragment;
    }// END COMMON CONTAINER HERE!!!


    /* Ok here we go for the harder part, first a general desription:
     * First we copy all the border nodes (the have to be copied as long
     * as they are partially selected) from the startContainer to the CmnAContainer. Then we do
     * the same for the endContainer. After this we add all fully selected
     * nodes that are between these two!
     */

    Node _cmnRoot = getCommonAncestorContainer();
    Node _tempCurrent = startContainer;
    Node _tempPartial;
    // we still have Node _clone!!

    // Special case text is first:
    if( _tempCurrent.nodeType() == Node::TEXT_NODE )
    {
        _clone = _tempCurrent.cloneNode(false);
        _clone.nodeValue().remove(0, startOffset);
        if(contentExtract)
        {
            startContainer.nodeValue().split(startOffset);
        }
    }
    else
    {
        _tempCurrent = _tempCurrent.firstChild();
        unsigned int i;
        for(i=0; i < startOffset; i++)
            _tempCurrent = _tempCurrent.nextSibling();

        if(contentExtract)
            _clone = _tempCurrent.cloneNode(true);
        else
            _clone = _tempCurrent; 
    }

    Node _tempParent;                       // we use this to traverse upwords trough the tree
    Node _cloneParent;                      // this one is used to copy the current parent
    Node _fragmentRoot;                     // this is eventually becomming the root of the DocumentFragment


    while( _tempCurrent != _cmnRoot )    // traversing from the Container, all the way up to the commonAncestor
    {                                    // all these node must be cloned as they are partially selected
        _tempParent = _tempCurrent.parentNode();

        if(_tempParent == _cmnRoot)
        {
            _cloneParent = _endFragment;
            _fragmentRoot = _tempCurrent;
        }
        else
        {
            _cloneParent = _tempParent.cloneNode(false);
            if( _tempPartial.isNull() && _tempParent != _cmnRoot )
            {
                _tempPartial = _tempParent;
                // TODO: this means we should collapse after I think... :))
            }
        }

        // we must not forget to grab with us the rest of this nodes siblings
        Node _nextCurrent;

        _tempCurrent = _tempCurrent.nextSibling();
        _cloneParent.appendChild( _clone );
        while( !_tempCurrent.isNull() )
        {
            _nextCurrent = _tempCurrent.nextSibling();
            if( !_tempCurrent.isNull() && _tempParent != _cmnRoot) // the isNull() part should be unessesary
            {
                if(contentExtract)
                {
                    _cloneParent.appendChild(_tempCurrent);
                }
                else
                {
                    _clone = _tempCurrent.cloneNode(true);
                    _cloneParent.appendChild(_clone);
                }
            }
            _tempCurrent = _nextCurrent;
        }
        _tempCurrent = _tempParent;
        _clone = _cloneParent;
    }

    //****** we should now be FINISHED with startContainer **********
    _tempCurrent = endContainer;
    Node _tempEnd;
    // we still have Node _clone!!

    // Special case text is first:
    if( _tempCurrent.nodeType() == Node::TEXT_NODE )
    {
        _clone = _tempCurrent.cloneNode(false);
        _clone.nodeValue().split(endOffset); 
        if(contentExtract)
        {
            endContainer.nodeValue().remove(endOffset, endContainer.nodeValue().length() - endOffset );
        }
    }
    else
    {
        if(endOffset == 0)
            _tempCurrent = endContainer;
        else
        {
            _tempCurrent = _tempCurrent.firstChild();
            unsigned int i;
            for(i=0; i< endOffset; i++)
                _tempCurrent = _tempCurrent.nextSibling();
        }
        if(contentExtract)
            _clone = _tempCurrent;
        else
            _clone = _tempCurrent.cloneNode(true);
    }




    while( _tempCurrent != _cmnRoot )    // traversing from the Container, all the way up to the commonAncestor
    {                                  //  all these node must be cloned as they are partially selected
        _tempParent = _tempCurrent.parentNode();

        if(_tempParent == _cmnRoot)
        {
            _cloneParent = _endFragment;
            _fragmentRoot = _tempCurrent;
        }
        else
        {
            _cloneParent = _tempParent.cloneNode(false);
            if( _tempPartial.isNull() && _tempParent != _cmnRoot )
            {
                _tempPartial = _tempParent;
                // TODO: this means we should collapse before I think... :))
            }
        }

        // we must not forget to grab with us the rest of this nodes siblings
        Node _nextCurrent;
        Node _stopNode = _tempCurrent;
        _tempCurrent = _tempParent.firstChild();


        _cloneParent.appendChild(_clone);

        while( _tempCurrent != _stopNode && !_tempCurrent.isNull() )
        {
            _nextCurrent = _tempCurrent.nextSibling();
            if( !_tempCurrent.isNull() && _tempParent != _cmnRoot) // the isNull() part should be unessesary
            {
                if(contentExtract)
                {
                    _cloneParent.appendChild(_tempCurrent);
                }
                else
                {
                    _clone = _tempCurrent.cloneNode(true);
                    _cloneParent.appendChild(_clone);
                }
            }
            _tempCurrent = _nextCurrent;
        }
        _tempCurrent = _tempParent;
        _clone = _cloneParent;
    }
    // To end the balade we grab with us any nodes that are between the two topmost parents under
    // the commonRoot

    Node _clonePrevious = _endFragment.lastChild();
    _tempCurrent = _tempEnd.previousSibling();
    Node _nextCurrent;

    while( (_nextCurrent != _fragmentRoot) && (!_tempCurrent.isNull()) )
    {
        _nextCurrent = _tempCurrent.previousSibling();

        if(contentExtract)
            _clone = _tempCurrent.cloneNode(true);
        else
            _clone = _tempCurrent;

        _endFragment.insertBefore(_clone, _clonePrevious);

        _tempCurrent = _nextCurrent;
        _clonePrevious = _tempCurrent;
    }
    // WHAT ABOUT THE COLLAPSES??
    return _endFragment;
}


