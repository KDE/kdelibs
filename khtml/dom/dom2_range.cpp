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
#include "dom_node.h"
#include "dom_doc.h"
#include "dom_string.h"
#include "dom_text.h"
#include "dom_exception.h"
#include "dom_docimpl.h"
#include <qstring.h>
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

Node Range::getCommonAncestorContainer() /*const*/
{
    if( isDetached() )
        throw DOMException(DOMException::INVALID_STATE_ERR);

    // this function must be somewhat slow.. if this is a problem,
    // maybe we should come up with something better :)
    Node parentStart;
    Node parentEnd;

    for(parentStart = startContainer;
        (!parentStart.isNull() ) && (parentStart != parentEnd);
        parentStart = parentStart.parentNode() )
      for(parentEnd = endContainer;
          (!parentEnd.isNull() ) && (parentStart != parentEnd);
          parentEnd = parentEnd.parentNode() );

    if(parentStart == parentEnd)
      commonAncestorContainer = parentStart;
    else
     {
       printf("uh? No common ancestor container?");
       return Node();
     }
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

    if( offset < 0 )
        throw DOMException( DOMException::INDEX_SIZE_ERR );

    if( !refNode.nodeType() == Node::TEXT_NODE )
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
    if( commonAncestorContainer.ownerDocument() != sourceRange.commonAncestorContainer.ownerDocument() )
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

DocumentFragment Range::masterTraverse(bool contentExtract)
{
    /* function description easy case, startContainer == endContainer
     * If we have a text node simply clone/extract the contents between
     * start & end and put them into the fragment
     * If we don't have a text node, find the offset and copy/clone the content
     * between the two offsets
     * We end with returning the fragment ofcourse
     */
  Node _clone;
  DocumentFragment _endFragment;

  if(startContainer == endContainer)
    {
      if(startOffset == endOffset)            // we have a collapsed range
        return DocumentFragment();

      // TODO: we need to delete the text Node if a whole text is selected!!
      if( startContainer.nodeType() == Node::TEXT_NODE )    // we have a text node.. special :)
        {
          _clone = startContainer.cloneNode(false);
          // TODO: clone.nodeValue().string().remove(startOffset, endOffset-startOffset);  // we need to get the SUBSTRING
          // use QString.mid() and QString.remove()
          if(contentExtract)
            {
            //TODO:  startContainer.nodeValue().string().remove(startOffset, endOffset-startOffset);
            // remove what wasn't in the substring
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
          for(i=0; i<range && !_tempCurrent.isNull(); i++)  // check of isNull in case of strange errors
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
      // TODO: clone.nodeValue().string().remove(startOffset, endOffset-startOffset);  // we need to get the SUBSTRING
      // use QString.mid() and QString.remove()
      if(contentExtract)
        {
          //TODO:  startContainer.nodeValue().string().remove(startOffset, endOffset-startOffset);
          // remove what wasn't in the substring
        }
    }
  else // container node was not a text node
    {
      _tempCurrent = _tempCurrent.firstChild();
      unsigned int i;
      for(i=0; i < startOffset; i++)
        _tempCurrent = _tempCurrent.nextSibling();

      if(contentExtract)
        _clone = _tempCurrent.cloneNode(true);
      else
        _clone = _tempCurrent;   // is this enough? Don't we have to delete the node from the original tree??

    }

  Node _tempParent;                       // we use this to traverse upwords trough the tree
  Node _cloneParent;                      // this one is used to copy the current parent
  Node _fragmentRoot;                     // this is eventually becomming the root of the DocumentFragment


  while( _tempCurrent != _cmnRoot )    // traversing from the Container, all the way up to the commonAncestor
    {                                  // we are in luck, all these node must be cloned because they are partially selected
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
      _cloneParent.appendChild( _tempCurrent );
      while( !_tempCurrent.isNull() )
        {
          _nextCurrent = _tempCurrent.nextSibling();
          if( !_tempCurrent.isNull() && _tempParent != _cmnRoot) // the isNull() part should be unessesary
            {
              if(contentExtract)
                {
                  _cloneParent.appendChild(_tempCurrent);         // delete from old tree?
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

  //****** we should now FINISHED with startContainer **********
  _tempCurrent = endContainer;
  Node _tempEnd;
  // we still have Node _clone!!

  // Special case text is first:
  if( _tempCurrent.nodeType() == Node::TEXT_NODE )
    {
      _clone = _tempCurrent.cloneNode(false);
      // TODO: clone.nodeValue().string().remove(startOffset, endOffset-startOffset);  // we need to get the SUBSTRING
      // use QString.mid() and QString.remove()
      if(contentExtract)
        {
          //TODO:  startContainer.nodeValue().string().remove(startOffset, endOffset-startOffset);
          // remove what wasn't in the substring
        }
    }
  else // container node was not a text node
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
    {                                  // we are in luck, all these node must be cloned because they are partially selected
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
                  _cloneParent.appendChild(_tempCurrent);         // delete from old tree?
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
  // now we should copy all the shit in between!!

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

