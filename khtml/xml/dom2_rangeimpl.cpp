/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
 * (C) 2000 Gunnstein Lye (gunnstein@netcom.no)
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
#include "dom_textimpl.h"
#include "dom_xmlimpl.h"
#include <qstring.h>

using namespace DOM;


RangeImpl::RangeImpl(DocumentImpl *rootContainer)
{
    m_ownerDocument = rootContainer;
    m_startContainer = rootContainer;
    m_endContainer = rootContainer;
    m_startOffset = 0;
    m_endOffset = 0;
    m_detached = false;
}

RangeImpl::RangeImpl(NodeImpl *sc, const long so, NodeImpl *ec, const long eo)
{
    m_startContainer = sc;
    m_startOffset = so;
    m_endContainer = ec;
    m_endOffset = eo;
    m_detached = false;
}

RangeImpl::~RangeImpl()
{
}

NodeImpl *RangeImpl::startContainer(int &exceptioncode) const
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    return m_startContainer;
}

long RangeImpl::startOffset(int &exceptioncode) const
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    return m_startOffset;
}

NodeImpl *RangeImpl::endContainer(int &exceptioncode) const
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    return m_endContainer;
}

long RangeImpl::endOffset(int &exceptioncode) const
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    return m_endOffset;
}

NodeImpl *RangeImpl::commonAncestorContainer(int &exceptioncode)
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    NodeImpl *parentStart = m_startContainer;
    NodeImpl *parentEnd = m_endContainer;

    while( parentStart && (parentStart != parentEnd) )
    {
        while( parentEnd && (parentStart != parentEnd) )
            parentEnd = parentEnd->parentNode();

        if(parentStart == parentEnd)  break;
        parentStart = parentStart->parentNode();
        parentEnd = m_endContainer;
    }

    if (!parentStart || !parentEnd || parentStart != parentEnd) {
	exceptioncode = DOMException::WRONG_DOCUMENT_ERR;
        return 0;
    }
    else
	return parentStart;
}

bool RangeImpl::collapsed(int &exceptioncode) const
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    return (m_startContainer == m_endContainer && m_startOffset == m_endOffset);
}

void RangeImpl::setStart( NodeImpl *refNode, long offset, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    checkNodeWOffset( refNode, offset, exceptioncode );
    if (exceptioncode)
	return;

    m_startContainer = refNode;
    m_startOffset = offset;

    // ### check if different root container

    if (compareBoundaryPoints(m_startContainer,m_startOffset,m_endContainer,m_endOffset) > 0)
	collapse(true,exceptioncode);
}

void RangeImpl::setEnd( NodeImpl *refNode, long offset, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    checkNodeWOffset( refNode, offset, exceptioncode );
    if (exceptioncode)
	return;

    m_endContainer = refNode;
    m_endOffset = offset;

    // ### check if different root container

    if (compareBoundaryPoints(m_startContainer,m_startOffset,m_endContainer,m_endOffset) > 0)
 	collapse(false,exceptioncode);
}

void RangeImpl::collapse( bool toStart, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    if( toStart )   // collapse to start
    {
        m_endContainer = m_startContainer;
        m_endOffset = m_startOffset;
    }
    else            // collapse to end
    {
        m_startContainer = m_endContainer;
        m_startOffset = m_endOffset;
    }
}

short RangeImpl::compareBoundaryPoints( Range::CompareHow how, RangeImpl *sourceRange, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    if (!sourceRange) {
	exceptioncode = DOMException::NOT_FOUND_ERR;
	return 0;
    }

    NodeImpl *thisCont = commonAncestorContainer(exceptioncode);
    NodeImpl *sourceCont = sourceRange->commonAncestorContainer(exceptioncode);
    if (exceptioncode)
	return 0;

    if (thisCont->ownerDocument() != sourceCont->ownerDocument()) { // ### what about if in separate DocumentFragments?
        exceptioncode = DOMException::WRONG_DOCUMENT_ERR;
        return 0;
    }

    switch(how)
    {
    case Range::START_TO_START:
        return compareBoundaryPoints( m_startContainer, m_startOffset,
                                      sourceRange->startContainer(exceptioncode), sourceRange->startOffset(exceptioncode) );
        break;
    case Range::START_TO_END:
        return compareBoundaryPoints( m_startContainer, m_startOffset,
                                      sourceRange->endContainer(exceptioncode), sourceRange->endOffset(exceptioncode) );
        break;
    case Range::END_TO_END:
        return compareBoundaryPoints( m_endContainer, m_endOffset,
                                      sourceRange->endContainer(exceptioncode), sourceRange->endOffset(exceptioncode) );
        break;
    case Range::END_TO_START:
        return compareBoundaryPoints( m_endContainer, m_endOffset,
                                      sourceRange->startContainer(exceptioncode), sourceRange->startOffset(exceptioncode) );
        break;
    default:
//        printf( "Function compareBoundaryPoints: Invalid CompareHow\n" );
        return 2;     // ### undocumented - should throw an exception here
    }
}

short RangeImpl::compareBoundaryPoints( NodeImpl *containerA, long offsetA, NodeImpl *containerB, long offsetB )
{
    if( containerA == containerB )
    {
        if( offsetA == offsetB )  return 0;    // A is equal to B
        if( offsetA < offsetB )  return -1;    // A is before B
        else  return 1;                        // A is after B
    }

    NodeImpl *n;
    if (offsetA >= (long)containerA->childNodes()->length()) {
	// find the next node
	if (containerA->lastChild())
	    n = containerA->lastChild();
	else
	    n = containerA;

	while (n && !n->nextSibling())
	    n = n->parentNode();
	if (n)
	    n = n->nextSibling();
    }
    else
	n = containerA->childNodes()->item(offsetA);
	
    while (n) {
	// traverse forwards and see if we find B
        if( n == containerB)  return -1;       // A is before B

        if (n->firstChild())
	    n = n->firstChild();
	else if (n->nextSibling())
	    n = n->nextSibling();
	else {
	    while (n && !n->nextSibling())
		n = n->parentNode();
	    if (n)
		n = n->nextSibling();
	}
    }

    return 1;                                  // A is after B
}

bool RangeImpl::boundaryPointsValid(  )
{
    short valid =  compareBoundaryPoints( m_startContainer, m_startOffset,
                                          m_endContainer, m_endOffset );
    if( valid == 1 )  return false;
    else  return true;

}

void RangeImpl::deleteContents( int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    if (collapsed(exceptioncode))
	return;
    if (exceptioncode)
	return;
	
    NodeImpl *cmnRoot = commonAncestorContainer(exceptioncode);
//    printf("CommonAC: %s \n", cmnRoot->nodeName().string().ascii());
//    printf("end: %d, start: %d", m_startOffset, m_endOffset);


    if(m_startContainer == m_endContainer)
    {
        // ### we need to delete the text Node if a whole text is selected!!
        if( m_startContainer->nodeType() == Node::TEXT_NODE )
        {
            m_startContainer->nodeValue().remove(m_startOffset, m_endOffset);
            m_startContainer->applyChanges(); // ### setChanged(true)
        }
        else
        {
//            printf("same but not a text node\n");
            NodeImpl *_tempParent = m_startContainer;
            NodeImpl *_tempCurrent = m_startContainer->firstChild();
            unsigned int i;

            for(i=0; i < m_startOffset; i++)    // get the node given by the offset
                _tempCurrent = _tempCurrent->nextSibling();

            /* now delete all nodes between the offsets */
            unsigned int range = m_endOffset - m_startOffset;
            NodeImpl *_nextCurrent = _tempCurrent;                  // to keep track of which node to take next

            for(i=0; i<range && _tempCurrent; i++)
            {
//                if(_tempParent == _tempCurrent->parentNode() )
//                    printf("like\n");
                _nextCurrent = _tempCurrent->nextSibling();
//                printf("just before remove\n");
                _tempParent->removeChild(_tempCurrent,exceptioncode);
                if (exceptioncode)
		    return;
//                printf("just after remove\n");
                _tempCurrent = _nextCurrent;
            }
        _tempParent->applyChanges(); // ### setChanged(true)
        }
        return;
    }// END COMMON CONTAINER CASE!!

//    printf("end common case\n");
    NodeImpl *_nextCurrent;
    NodeImpl *_tempCurrent;

    // cleanup left side
    NodeImpl *_leftParent = m_startContainer;
    if( m_startContainer->nodeType() == Node::TEXT_NODE )
    {
//        printf("left side text\n");
        (void)m_startContainer->nodeValue().split(m_startOffset); // what about complete removals?
    }
    else
    {

        _tempCurrent = m_startContainer->firstChild();
        unsigned int i;

        for(i=0; i < m_startOffset; i++)    // get the node given by the offset
            _tempCurrent = _tempCurrent->nextSibling();

        _nextCurrent = _tempCurrent;                  // to keep track of which node to take next

        while( _tempCurrent )
        {
            _nextCurrent = _tempCurrent->nextSibling();
            _leftParent->removeChild(_tempCurrent,exceptioncode);
            if (exceptioncode)
		return;
            _tempCurrent = _nextCurrent;
        }
    }
    _tempCurrent = _leftParent;
    _leftParent = _leftParent->parentNode();
    while( _leftParent != cmnRoot )
    {
        while( _tempCurrent )
        {
            _nextCurrent = _tempCurrent->nextSibling();
            _leftParent->removeChild(_tempCurrent,exceptioncode);
            if (exceptioncode)
		return;
            _tempCurrent = _nextCurrent;
        }
        _tempCurrent = _leftParent;
        _leftParent = _leftParent->parentNode();
    }


    // cleanup right side
    NodeImpl *_rightParent = m_endContainer;
    if( m_endContainer->nodeType() == Node::TEXT_NODE )
    {
        m_endContainer->nodeValue().remove(0, m_endOffset); // what about complete removals?
    }
    else
    {

        NodeImpl *_tempCurrent = m_endContainer->firstChild();
        unsigned int i;

        for(i=0; i < m_endOffset; i++)    // get the node given by the offset
            _tempCurrent = _tempCurrent->nextSibling();

        NodeImpl *_nextCurrent = _tempCurrent;                  // to keep track of which node to take next

        while( _tempCurrent )
        {
            _nextCurrent = _tempCurrent->previousSibling();
            _leftParent->removeChild(_tempCurrent,exceptioncode);
            if (exceptioncode)
		return;
            _tempCurrent = _nextCurrent;
        }
    }
    _tempCurrent = _rightParent;
    _rightParent = _rightParent->parentNode();
    while( _rightParent != cmnRoot )
    {
        while( _tempCurrent )
        {
            _nextCurrent = _tempCurrent->previousSibling();
            _rightParent->removeChild(_tempCurrent,exceptioncode);
            if (exceptioncode)
		return;
            _tempCurrent = _nextCurrent;
        }
        _tempCurrent = _rightParent;
        _rightParent = _rightParent->parentNode();
    }

    // cleanup middle
    _leftParent = _leftParent->nextSibling();
    while( _leftParent != _rightParent )
    {
        cmnRoot->removeChild(_leftParent,exceptioncode);
	if (exceptioncode)
	    return;
        _leftParent = _leftParent->nextSibling();
    }


    // FIXME! this allways collapses to the front (see DOM specs)
    //collapse(true);
    return;

}

DocumentFragmentImpl *RangeImpl::extractContents( int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    return masterTraverse( true, exceptioncode );
}

DocumentFragmentImpl *RangeImpl::cloneContents( int &exceptioncode  )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    return masterTraverse( false, exceptioncode );
}

void RangeImpl::insertNode( NodeImpl *newNode, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    if( newNode->nodeType() == Node::ATTRIBUTE_NODE ||
        newNode->nodeType() == Node::ENTITY_NODE ||
        newNode->nodeType() == Node::NOTATION_NODE ||
        newNode->nodeType() == Node::DOCUMENT_NODE ||
        newNode->nodeType() == Node::DOCUMENT_FRAGMENT_NODE) {
        exceptioncode = RangeException::INVALID_NODE_TYPE_ERR + RangeException::_EXCEPTION_OFFSET;
        return;
    }

    if( newNode->ownerDocument() != m_startContainer->ownerDocument() ) {
        exceptioncode = DOMException::WRONG_DOCUMENT_ERR;
        return;
    }

    if( m_startContainer->nodeType() == Node::TEXT_NODE )
    {
        TextImpl *newText;
        NodeImpl *newParent = newNode->parentNode();
        TextImpl *textNode = static_cast<TextImpl*>(m_startContainer);
        newText = textNode->splitText(m_startOffset,exceptioncode);
        if (exceptioncode)
	    return;
        newParent->insertBefore( newNode, newText, exceptioncode );
    }
    else {
        m_startContainer->insertBefore( newNode, m_startContainer->childNodes()->item( m_startOffset ), exceptioncode );
    }
}

DOMString RangeImpl::toString( int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    // ###
/*    NodeIteratorImpl iterator( m_startContainer.childNodes().item( m_startOffset ) );
    DOMString _string;
    Node _node = iterator.nextNode();

    while( !_node.isNull() )
    {
        printf( "\nNodetype: %s\n", _node.nodeName().string().ascii() );
        if( _node.nodeType() == Node::TEXT_NODE )
        {
            QString str = _node.nodeValue().string();
            if( _node == m_startContainer && _node == m_endContainer )
                _string = str.mid( m_startOffset, m_endOffset - m_startOffset );
            else if( _node == m_startContainer )
                _string = str.mid( m_startOffset );
            else if( _node == m_endContainer )
                _string += str.left( m_startOffset );
            else
                _string += str;
        }
        else if( _node.nodeName() == "BR" )  _string += "\n";
        else if( _node.nodeName() == "P" || _node.nodeName() == "TD" )  _string += "\n\n";
        else  _string += " ";

        _node = iterator.nextNode();
    }
    return _string;*/
    return DOMString("");
}

DOMString RangeImpl::toHTML(  )
{
    // ### implement me!!!!

    // this is just to avoid compiler warnings
    DOMString d;
    return d;
}

void RangeImpl::detach( int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    m_detached = true;
}

bool RangeImpl::isDetached() const
{
    return m_detached;
}

DocumentFragmentImpl *RangeImpl::masterTraverse(bool contentExtract, int &exceptioncode)
{
    /* function description easy case, m_startContainer == m_endContainer
     * If we have a text node simply clone/extract the contents between
     * start & end and put them into the fragment
     * If we don't have a text node, find the offset and copy/clone the content
     * between the two offsets
     * We end with returning the fragment of course
     */
    NodeImpl *_clone;
    DocumentFragmentImpl *_endFragment = m_ownerDocument->createDocumentFragment(); // ### check this gets deleted where necessary

    if(m_startContainer == m_endContainer)
    {
        if(m_startOffset == m_endOffset)            // we have a collapsed range
            return 0;

        // TODO: we need to delete the text Node if a whole text is selected!!
        if( m_startContainer->nodeType() == Node::TEXT_NODE )    // we have a text node.. special :)
        {
            _clone = m_startContainer->cloneNode(false,exceptioncode);
            if (exceptioncode)
		return 0;
            _clone->nodeValue().remove(0, m_startOffset);  // we need to get the SUBSTRING
            _clone->nodeValue().remove(m_endOffset, _clone->nodeValue().length() - m_endOffset);
            if(contentExtract)
            {
                // full trim :)
                m_startContainer->nodeValue().remove(m_startOffset, m_endOffset - m_startOffset);
            }
            _endFragment->appendChild(_clone,exceptioncode);
            if (exceptioncode)
		return 0;
        }
        else  // we have the same container class but we are not a text node
        {
            NodeImpl *_tempCurrent = m_startContainer->firstChild();
            unsigned int i;

            for(i=0; i < m_startOffset; i++)    // get the node given by the offset
                _tempCurrent = _tempCurrent->nextSibling();

            /* now copy (or move) all the nodes in the range into the document fragment */
            unsigned int range = m_endOffset - m_startOffset;
            NodeImpl *_nextCurrent = _tempCurrent;                  // to keep track of which node to take next
            for(i=0; i<range && _tempCurrent; i++)   // check of isNull in case of strange errors
            {
                _nextCurrent = _tempCurrent->nextSibling();

                if(contentExtract)
                {
                    _endFragment->appendChild(_tempCurrent,exceptioncode);
                    if (exceptioncode)
			return 0;
                }
                else
                {
                    _clone = _tempCurrent->cloneNode(true,exceptioncode);
                    if (exceptioncode)
			return 0;
                    _endFragment->appendChild(_clone,exceptioncode);
                    if (exceptioncode)
			return 0;
                }

                _tempCurrent = _nextCurrent;
            }
        }
        return _endFragment;
    }// END COMMON CONTAINER HERE!!!


    /* Ok here we go for the harder part, first a general desription:
     * First we copy all the border nodes (the have to be copied as long
     * as they are partially selected) from the m_startContainer to the CmnAContainer. Then we do
     * the same for the m_endContainer. After this we add all fully selected
     * nodes that are between these two!
     */

    NodeImpl *_cmnRoot = commonAncestorContainer(exceptioncode);
    NodeImpl *_tempCurrent = m_startContainer;
    NodeImpl *_tempPartial = 0;
    // we still have Node _clone!!

    // Special case text is first:
    if( _tempCurrent->nodeType() == Node::TEXT_NODE )
    {
        _clone = _tempCurrent->cloneNode(false,exceptioncode);
        if (exceptioncode)
	    return 0;
        _clone->nodeValue().remove(0, m_startOffset);
        if(contentExtract)
        {
            m_startContainer->nodeValue().split(m_startOffset);
        }
    }
    else
    {
        _tempCurrent = _tempCurrent->firstChild();
        unsigned int i;
        for(i=0; i < m_startOffset; i++)
            _tempCurrent = _tempCurrent->nextSibling();

        if(contentExtract) {
            _clone = _tempCurrent->cloneNode(true,exceptioncode);
            if (exceptioncode)
		return 0;
        }
        else
            _clone = _tempCurrent;
    }

    NodeImpl *_tempParent;                       // we use this to traverse upwords trough the tree
    NodeImpl *_cloneParent;                      // this one is used to copy the current parent
    NodeImpl *_fragmentRoot = 0;                 // this is eventually becomming the root of the DocumentFragment


    while( _tempCurrent != _cmnRoot )    // traversing from the Container, all the way up to the commonAncestor
    {                                    // all these node must be cloned as they are partially selected
        _tempParent = _tempCurrent->parentNode();

        if(_tempParent == _cmnRoot)
        {
            _cloneParent = _endFragment;
            _fragmentRoot = _tempCurrent;
        }
        else
        {
            _cloneParent = _tempParent->cloneNode(false,exceptioncode);
            if (exceptioncode)
		return 0;
            if( !_tempPartial && _tempParent != _cmnRoot )
            {
                _tempPartial = _tempParent;
                // TODO: this means we should collapse after I think... :))
            }
        }

        // we must not forget to grab with us the rest of this nodes siblings
        NodeImpl *_nextCurrent;

        _tempCurrent = _tempCurrent->nextSibling();
        _cloneParent->appendChild( _clone, exceptioncode );
        if (exceptioncode)
	    return 0;
        while( _tempCurrent )
        {
            _nextCurrent = _tempCurrent->nextSibling();
            if( _tempCurrent && _tempParent != _cmnRoot) // the isNull() part should be unessesary
            {
                if(contentExtract)
                {
                    _cloneParent->appendChild(_tempCurrent,exceptioncode);
                    if (exceptioncode)
			return 0;
                }
                else
                {
                    _clone = _tempCurrent->cloneNode(true,exceptioncode);
                    if (exceptioncode)
			return 0;
                    _cloneParent->appendChild(_clone,exceptioncode);
                    if (exceptioncode)
			return 0;
                }
            }
            _tempCurrent = _nextCurrent;
        }
        _tempCurrent = _tempParent;
        _clone = _cloneParent;
    }

    //****** we should now be FINISHED with m_startContainer **********
    _tempCurrent = m_endContainer;
    NodeImpl *_tempEnd = 0;
    // we still have Node _clone!!

    // Special case text is first:
    if( _tempCurrent->nodeType() == Node::TEXT_NODE )
    {
        _clone = _tempCurrent->cloneNode(false,exceptioncode);
        if (exceptioncode)
	    return 0;
        _clone->nodeValue().split(m_endOffset);
        if(contentExtract)
        {
            m_endContainer->nodeValue().remove(m_endOffset, m_endContainer->nodeValue().length() - m_endOffset );
        }
    }
    else
    {
        if(m_endOffset == 0)
            _tempCurrent = m_endContainer;
        else
        {
            _tempCurrent = _tempCurrent->firstChild();
            unsigned int i;
            for(i=0; i< m_endOffset; i++)
                _tempCurrent = _tempCurrent->nextSibling();
        }
        if(contentExtract)
            _clone = _tempCurrent;
        else {
            _clone = _tempCurrent->cloneNode(true,exceptioncode);
            if (exceptioncode)
		return 0;
        }
    }




    while( _tempCurrent != _cmnRoot )    // traversing from the Container, all the way up to the commonAncestor
    {                                  //  all these node must be cloned as they are partially selected
        _tempParent = _tempCurrent->parentNode();

        if(_tempParent == _cmnRoot)
        {
            _cloneParent = _endFragment;
            _fragmentRoot = _tempCurrent;
        }
        else
        {
            _cloneParent = _tempParent->cloneNode(false,exceptioncode);
            if (exceptioncode)
		return 0;
            if( !_tempPartial && _tempParent != _cmnRoot )
            {
                _tempPartial = _tempParent;
                // ### TODO: this means we should collapse before I think... :))
            }
        }

        // we must not forget to grab with us the rest of this nodes siblings
        NodeImpl *_nextCurrent;
        NodeImpl *_stopNode = _tempCurrent;
        _tempCurrent = _tempParent->firstChild();


        _cloneParent->appendChild(_clone,exceptioncode);
        if (exceptioncode)
	    return 0;

        while( _tempCurrent != _stopNode && _tempCurrent )
        {
            _nextCurrent = _tempCurrent->nextSibling();
            if( _tempCurrent && _tempParent != _cmnRoot) // the isNull() part should be unessesary
            {
                if(contentExtract)
                {
                    _cloneParent->appendChild(_tempCurrent,exceptioncode);
                    if (exceptioncode)
			return 0;
                }
                else
                {
                    _clone = _tempCurrent->cloneNode(true,exceptioncode);
                    if (exceptioncode)
			return 0;
                    _cloneParent->appendChild(_clone,exceptioncode);
                    if (exceptioncode)
			return 0;
                }
            }
            _tempCurrent = _nextCurrent;
        }
        _tempCurrent = _tempParent;
        _clone = _cloneParent;
    }
    // To end the balade we grab with us any nodes that are between the two topmost parents under
    // the commonRoot

    NodeImpl *_clonePrevious = _endFragment->lastChild();
    _tempCurrent = _tempEnd->previousSibling(); // ### _tempEnd is always 0
    NodeImpl *_nextCurrent = 0;

    while( (_nextCurrent != _fragmentRoot) && _tempCurrent )
    {
        _nextCurrent = _tempCurrent->previousSibling();

        if(contentExtract) {
            _clone = _tempCurrent->cloneNode(true,exceptioncode);
            if (exceptioncode)
		return 0;
        }
        else
            _clone = _tempCurrent;

        _endFragment->insertBefore(_clone, _clonePrevious, exceptioncode);
        if (exceptioncode)
	    return 0;

        _tempCurrent = _nextCurrent;
        _clonePrevious = _tempCurrent;
    }
    // WHAT ABOUT THE COLLAPSES??
    return _endFragment;
}


void RangeImpl::checkCommon(int &exceptioncode) const
{
    if( m_detached )
        exceptioncode = DOMException::INVALID_STATE_ERR;
}

void RangeImpl::checkNode( const NodeImpl *n, int &exceptioncode ) const
{
    checkCommon(exceptioncode);
    if (exceptioncode)
	return;

    if( !n ) {
        exceptioncode = DOMException::NOT_FOUND_ERR;
        return;
    }

    const NodeImpl *_tempNode = n;
    while( _tempNode )
    {
        if( _tempNode->nodeType() == Node::ATTRIBUTE_NODE ||
            _tempNode->nodeType() == Node::ENTITY_NODE ||
            _tempNode->nodeType() == Node::NOTATION_NODE ||
            _tempNode->nodeType() == Node::DOCUMENT_TYPE_NODE ) {
            exceptioncode = RangeException::INVALID_NODE_TYPE_ERR + RangeException::_EXCEPTION_OFFSET;
            return;
        }

        _tempNode = _tempNode->parentNode();
    }
}

void RangeImpl::checkNodeWOffset( NodeImpl *n, int offset, int &exceptioncode) const
{
    checkNode( n, exceptioncode );
    if (exceptioncode)
	return;

    if( offset < 0 ) {
        exceptioncode = DOMException::INDEX_SIZE_ERR;
    }

    switch (n->nodeType()) {
	case Node::TEXT_NODE:
	case Node::COMMENT_NODE:
	case Node::CDATA_SECTION_NODE:
	    if ( (unsigned long)offset > static_cast<CharacterDataImpl*>(n)->length() )
		exceptioncode = DOMException::INDEX_SIZE_ERR;
	    break;
	case Node::PROCESSING_INSTRUCTION_NODE:
	    // ### are we supposed to check with just data or the whole contents?
	    if ( (unsigned long)offset > static_cast<ProcessingInstructionImpl*>(n)->data().length() )
		exceptioncode = DOMException::INDEX_SIZE_ERR;
	    break;
	default:
	    if ( (unsigned long)offset > n->childNodes()->length() )
		exceptioncode = DOMException::INDEX_SIZE_ERR;
	    break;
    }
}

void RangeImpl::checkNodeBA( const NodeImpl *n, int &exceptioncode ) const
{
    checkNode( n, exceptioncode );
    if (exceptioncode)
	return;

    if( n->nodeType() == Node::DOCUMENT_NODE ||
        n->nodeType() == Node::DOCUMENT_FRAGMENT_NODE ||
        n->nodeType() == Node::ATTRIBUTE_NODE ||
        n->nodeType() == Node::ENTITY_NODE ||
        n->nodeType() == Node::NOTATION_NODE )
        exceptioncode = RangeException::INVALID_NODE_TYPE_ERR  + RangeException::_EXCEPTION_OFFSET;

}

RangeImpl *RangeImpl::cloneRange(int &exceptioncode)
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return 0;
    }

    // ###
    return 0;
}

void RangeImpl::setStartAfter( const NodeImpl *refNode, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    checkNodeBA( refNode, exceptioncode );
    if (exceptioncode)
	return;

    setStart( refNode->parentNode(), refNode->index()+1, exceptioncode );
}

void RangeImpl::setEndBefore( const NodeImpl *refNode, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    checkNodeBA( refNode, exceptioncode );
    if (exceptioncode)
	return;

    setEnd( refNode->parentNode(), refNode->index(), exceptioncode );
}

void RangeImpl::setEndAfter( const NodeImpl *refNode, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    checkNodeBA( refNode, exceptioncode );
    if (exceptioncode)
	return;

    setEnd( refNode->parentNode(), refNode->index()+1, exceptioncode );

}

void RangeImpl::selectNode( const NodeImpl *refNode, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    checkNodeBA( refNode, exceptioncode );
    if (exceptioncode)
	return;

    setStartBefore( refNode, exceptioncode );
    if (exceptioncode)
	return;
    setEndAfter( refNode, exceptioncode );
}

void RangeImpl::selectNodeContents( const NodeImpl *refNode, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    checkNode( refNode, exceptioncode );
    if (exceptioncode)
	return;

    setStartBefore( refNode->firstChild(), exceptioncode );
    if (exceptioncode)
	return;
    setEndAfter( refNode->lastChild(), exceptioncode );
}

void RangeImpl::surroundContents( NodeImpl *newParent, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    if( !newParent )
        return; // ### are we supposed to throw an exception here?

    NodeImpl *start = m_startContainer;
    if( newParent->ownerDocument() != start->ownerDocument() ) {
        exceptioncode = DOMException::WRONG_DOCUMENT_ERR;
        return;
    }

    if( newParent->nodeType() == Node::ATTRIBUTE_NODE ||
        newParent->nodeType() == Node::ENTITY_NODE ||
        newParent->nodeType() == Node::NOTATION_NODE ||
        newParent->nodeType() == Node::DOCUMENT_TYPE_NODE ||
        newParent->nodeType() == Node::DOCUMENT_NODE ||
        newParent->nodeType() == Node::DOCUMENT_FRAGMENT_NODE) {
        exceptioncode = RangeException::INVALID_NODE_TYPE_ERR + RangeException::_EXCEPTION_OFFSET;
        return;
    }

    // ### revisit: if you set a range without optimizing it (trimming) the following exception might be
    // thrown incorrectly
    NodeImpl *realStart = (start->nodeType() == Node::TEXT_NODE)? start->parentNode() : start;
    NodeImpl *end = m_endContainer;
    NodeImpl *realEnd = (end->nodeType() == Node::TEXT_NODE)? end->parentNode() : end;
    if( realStart != realEnd ) {
        exceptioncode = RangeException::BAD_BOUNDARYPOINTS_ERR + RangeException::_EXCEPTION_OFFSET;
        return;
    }

    DocumentFragmentImpl *fragment = extractContents(exceptioncode);
    if (exceptioncode)
	return;
    insertNode( newParent, exceptioncode );
    if (exceptioncode)
	return;
    // BIC: to avoid this const_cast newParent shouldn't be const
    //(const_cast<Node>(newParent)).appendChild( fragment );
    ((NodeImpl*)(newParent))->appendChild( fragment, exceptioncode );
    if (exceptioncode)
	return;
    selectNode( newParent, exceptioncode );
}

void RangeImpl::setStartBefore( const NodeImpl *refNode, int &exceptioncode )
{
    if (m_detached) {
	exceptioncode = DOMException::INVALID_STATE_ERR;
	return;
    }

    checkNodeBA( refNode, exceptioncode );
    if (exceptioncode)
	return;

    setStart( refNode->parentNode(), refNode->index(), exceptioncode );
}

