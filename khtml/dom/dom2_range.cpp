/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
 * (C) 2000 Gunnstein Lye (gunnstein@netcom.no)
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
#include "dom_node.h"
#include "dom_doc.h"
#include "dom_string.h"
#include "dom_exception.h"
#include "dom_docimpl.h"
#include <stdio.h>               // for printf

//using namespace CSS;       // leff
using namespace DOM;


Range::Range()
{
    detached = false;
}

Range::Range(const Document rootContainer)
{
    startContainer = (Node)rootContainer;
    endContainer = (Node)rootContainer;
    startOffset = 0;
    endOffset = 0;
    commonAncestorContainer = (Node)rootContainer;
    collapsed = true;
    detached = false;
}

Range::Range(const Range &other)
{
    startContainer = other.startContainer;
    startOffset = other.startOffset;
    endContainer = other.endContainer;
    endOffset = other.endOffset;
    commonAncestorContainer = other.commonAncestorContainer;
    collapsed = other.collapsed;
    detached = false;
}

Range &Range::operator = (const Range &other)
{
    Range::operator = (other);
    return *this;
}

Range::~Range()
{
}

Node Range::getStartContainer() const
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);
    else
        return startContainer;
}

long Range::getStartOffset() const
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);
    else
        return startOffset;
}

Node Range::getEndContainer() const
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);
    else
        return endContainer;
}

long Range::getEndOffset() const
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);
    else
        return endOffset;
}

Node Range::getCommonAncestorContainer() const
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);
    else
        return commonAncestorContainer;
}

bool Range::isCollapsed() const
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);
    else
        return collapsed;
}

bool Range::isDetached() const
{
    return detached;
}

void Range::setStart( const Node &refNode, long offset )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw RangeException( RangeException::NULL_NODE_ERR );
    
    while( !_tempNode.isNull() )
    {
        if( _tempNode.nodeType() == Node::ATTRIBUTE_NODE ||
            _tempNode.nodeType() == Node::ENTITY_NODE ||
            _tempNode.nodeType() == Node::NOTATION_NODE ||
            _tempNode.nodeType() == Node::DOCUMENT_TYPE_NODE )
            throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );
        
        _tempNode = _tempNode.parentNode();
    }
    
    if( offset < 0  ||  (unsigned)offset > refNode.childNodes().length() )
        throw DOMException( DOMException::INDEX_SIZE_ERR );
    
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    startContainer = refNode;
    startOffset = offset;

    _tempNode = refNode.parentNode();
    Node rootContainer = getCommonAncestorContainer(); 
    while( !_tempNode.isNull() )
    {
        if( _tempNode == rootContainer )
            break;
        _tempNode = _tempNode.parentNode();
    }
    if( _tempNode != rootContainer )
        collapse( true );

    if( !boundaryPointsValid() )
        collapse( true );
}

void Range::setEnd( const Node &refNode, long offset )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw RangeException( RangeException::NULL_NODE_ERR );
    
    while( !_tempNode.isNull() )
    {
        if( _tempNode.nodeType() == Node::ATTRIBUTE_NODE ||
            _tempNode.nodeType() == Node::ENTITY_NODE ||
            _tempNode.nodeType() == Node::NOTATION_NODE ||
            _tempNode.nodeType() == Node::DOCUMENT_TYPE_NODE )
            throw RangeException( RangeException::INVALID_NODE_TYPE_ERR );

        _tempNode = _tempNode.parentNode();
    }

    if( offset < 0  ||  (unsigned)offset > refNode.childNodes().length() )
        throw DOMException( DOMException::INDEX_SIZE_ERR );

    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );
    
    endContainer = refNode;
    endOffset = offset;

    _tempNode = refNode.parentNode();
    Node rootContainer = getCommonAncestorContainer(); 
    while( !_tempNode.isNull() )
    {
        if( _tempNode == rootContainer )
            break;
        _tempNode = _tempNode.parentNode();
    }
    if( _tempNode != rootContainer )
        collapse( false );

    if( !boundaryPointsValid() )
        collapse( false );
}

void Range::setStartBefore( const Node &refNode )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw RangeException( RangeException::NULL_NODE_ERR );
    
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
        if( r.code == RangeException::NULL_NODE_ERR )
            fprintf( stderr, "Exception: Null Nodes not accepted\n" );
        if( r.code == RangeException::INVALID_NODE_TYPE_ERR )
            fprintf( stderr, "Exception: Invalid Node type\n" );
        return;
    }
    catch( DOMException d )
    {
        if( d.code == DOMException::INDEX_SIZE_ERR )
            fprintf( stderr, "Exception: offset has wrong size\n" );
        if( d.code == DOMException::INVALID_STATE_ERR )
            fprintf( stderr, "Exception: detach() has been invoked\n" );
        return;
    }
}

void Range::setStartAfter( const Node &refNode )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw RangeException( RangeException::NULL_NODE_ERR );
    
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
        if( r.code == RangeException::NULL_NODE_ERR )
            fprintf( stderr, "Exception: Null Nodes not accepted\n" );
        if( r.code == RangeException::INVALID_NODE_TYPE_ERR )
            fprintf( stderr, "Exception: Invalid Node type\n" );
        return;
    }
    catch( DOMException d )
    {
        if( d.code == DOMException::INDEX_SIZE_ERR )
            fprintf( stderr, "Exception: offset has wrong size\n" );
        if( d.code == DOMException::INVALID_STATE_ERR )
            fprintf( stderr, "Exception: detach() has been invoked\n" );
        return;
    }
}

void Range::setEndBefore( const Node &refNode )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw RangeException( RangeException::NULL_NODE_ERR );
    
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
        if( r.code == RangeException::NULL_NODE_ERR )
            fprintf( stderr, "Exception: Null Nodes not accepted\n" );
        if( r.code == RangeException::INVALID_NODE_TYPE_ERR )
            fprintf( stderr, "Exception: Invalid Node type\n" );
        return;
    }
    catch( DOMException d )
    {
        if( d.code == DOMException::INDEX_SIZE_ERR )
            fprintf( stderr, "Exception: offset has wrong size\n" );
        if( d.code == DOMException::INVALID_STATE_ERR )
            fprintf( stderr, "Exception: detach() has been invoked\n" );
        return;
    }
}

void Range::setEndAfter( const Node &refNode )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw RangeException( RangeException::NULL_NODE_ERR );
    
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
        if( r.code == RangeException::NULL_NODE_ERR )
            fprintf( stderr, "Exception: Null Nodes not accepted\n" );
        if( r.code == RangeException::INVALID_NODE_TYPE_ERR )
            fprintf( stderr, "Exception: Invalid Node type\n" );
        return;
    }
    catch( DOMException d )
    {
        if( d.code == DOMException::INDEX_SIZE_ERR )
            fprintf( stderr, "Exception: offset has wrong size\n" );
        if( d.code == DOMException::INVALID_STATE_ERR )
            fprintf( stderr, "Exception: detach() has been invoked\n" );
        return;
    }
}

void Range::collapse( bool toStart )
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

void Range::selectNode( const Node &refNode )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw RangeException( RangeException::NULL_NODE_ERR );
    
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
        if( r.code == RangeException::NULL_NODE_ERR )
            fprintf( stderr, "Exception: Null Nodes not accepted\n" );
        if( r.code == RangeException::INVALID_NODE_TYPE_ERR )
            fprintf( stderr, "Exception: Invalid Node type\n" );
        return;
    }
    catch( DOMException d )
    {
        if( d.code == DOMException::INVALID_STATE_ERR )
            fprintf( stderr, "Exception: detach() has been invoked\n" );
        return;
    }
}

void Range::selectNodeContents( const Node &refNode )
{
    Node _tempNode = refNode;
    if( _tempNode.isNull() )
        throw RangeException( RangeException::NULL_NODE_ERR );
    
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
        if( r.code == RangeException::NULL_NODE_ERR )
            fprintf( stderr, "Exception: Null Nodes not accepted\n" );
        if( r.code == RangeException::INVALID_NODE_TYPE_ERR )
            fprintf( stderr, "Exception: Invalid Node type\n" );
        return;
    }
    catch( DOMException d )
    {
        if( d.code == DOMException::INVALID_STATE_ERR )
            fprintf( stderr, "Exception: detach() has been invoked\n" );
        return;
    }
}

short Range::compareBoundaryPoints( CompareHow how, const Range &sourceRange )
{
    if( &commonAncestorContainer.ownerDocument != &sourceRange.commonAncestorContainer.ownerDocument )
        throw DOMException( DOMException::WRONG_DOCUMENT_ERR );
    
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    switch(how)
    {
    case StartToStart:
        return compareBoundaryPoints( getStartContainer(), getStartOffset(),
                                      sourceRange.getStartContainer(), sourceRange.getStartOffset() );
        break;
    case StartToEnd:
        return compareBoundaryPoints( getStartContainer(), getStartOffset(),
                                      sourceRange.getEndContainer(), sourceRange.getEndOffset() );
        break;
    case EndToEnd:
        return compareBoundaryPoints( getEndContainer(), getEndOffset(),
                                      sourceRange.getEndContainer(), sourceRange.getEndOffset() );
        break;
    case EndToStart:
        return compareBoundaryPoints( getEndContainer(), getEndOffset(),
                                      sourceRange.getStartContainer(), sourceRange.getStartOffset() );
        break;
    default:
        printf( "Function compareBoundaryPoints: Invalid CompareHow\n" );
        return 2;     // undocumented - should throw an exception here
    }
}

short Range::compareBoundaryPoints( Node containerA, long offsetA, Node containerB, long offsetB )
{
    if( offsetA < 0 || offsetB < 0 )
    {
        printf( "Function compareBoundaryPoints: No negative offsets allowed\n" );
        return 2;     // undocumented - should throw an exception here
    }
    
    if( containerA == containerB )        // Case 1 in DOM2 pt. 8.5
    {
        if( offsetA < offsetB )  return -1;
        else if( offsetA == offsetB )  return 0;
        else if( offsetA > offsetB )  return 1;
    }
    else
    {
        Node _tempNode = containerB;      // Case 2 in DOM2 pt. 8.5
        while( !_tempNode.isNull() )
        {
            if( _tempNode.parentNode() == containerA )
            {
                if( _tempNode.parentNode().index() < (unsigned)offsetA )  return -1;
                else  return 1;
            }
            _tempNode = _tempNode.parentNode();
        }
        
        _tempNode = containerA;           // Case 3 in DOM2 pt. 8.5
        while( !_tempNode.isNull() )
        {
            if( _tempNode.parentNode() == containerB )
            {
                if( _tempNode.parentNode().index() > (unsigned)offsetB )  return -1;
                else  return 1;
            }
            _tempNode = _tempNode.parentNode();
        }

        // Case 4 in DOM2 pt. 8.5 (use a treewalker here)
        }
    return 0;
}

bool Range::boundaryPointsValid(  )
{
    short valid =  compareBoundaryPoints( getStartContainer(), getStartOffset(),
                                          getEndContainer(), getEndOffset() );
    if( valid == -1 )  return false;
    else  return true;
}

void Range::deleteContents(  )
{
/*    if( isDetached() )
      throw DOMException( DOMException::INVALID_STATE_ERR );
      
      try
      {
      }
      catch( DOMException d )
      {
      if( d.code == DOMException::NO_MODIFICATION_ALLOWED_ERR )
      fprintf( stderr, "Exception: Node is readonly\n" );
      if( d.code == DOMException::NOT_FOUND_ERR )
      fprintf( stderr, "Exception: Node not found\n" );
      }*/
}

DocumentFragment Range::extractContents(  )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    // this is just to avoid compiler warnings
    DocumentFragment d;
    return d;
}

DocumentFragment Range::cloneContents(  )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    // this is just to avoid compiler warnings
    DocumentFragment d;
    return d;
}

void Range::insertNode( const Node &/*newNode*/ )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );
}

void Range::surroundContents( const Node &/*newParent*/ )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );
}

Range Range::cloneRange(  )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    // this is just to avoid compiler warnings
    Range r;
    return r;
}

DOMString Range::toString(  )
{
    if( isDetached() )
        throw DOMException( DOMException::INVALID_STATE_ERR );

    // this is just to avoid compiler warnings
    DOMString d;
    return d;
}

void Range::detach(  )
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);
    else
        detached = true;
}

// ---------------------------------------------------------------

DocumentRange::DocumentRange()
{
}

DocumentRange::DocumentRange(const DocumentRange &other)
{
    impl = other.impl;
}

DocumentRange &DocumentRange::operator = (const DocumentRange &other)
{
    DocumentRange::operator = (other);
    return *this;
}

DocumentRange::~DocumentRange()
{
}

//Range DocumentRange::createRange(  )
//{
//}

