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

#ifndef _DOM2_RangeImpl_h_
#define _DOM2_RangeImpl_h_

#include "dom2_range.h"

namespace DOM {

class RangeImpl : public DomShared
{
    friend class DocumentImpl;
public:
    RangeImpl(DocumentImpl *rootContainer);
    RangeImpl(NodeImpl *sc, const long so, NodeImpl *ec, const long eo);

    ~RangeImpl();

    // ### remove the get from these methods (i.e. getStartContainer() -> startContainer())
    NodeImpl *startContainer(int &exceptioncode) const;
    long startOffset(int &exceptioncode) const;
    NodeImpl *endContainer(int &exceptioncode) const;
    long endOffset(int &exceptioncode) const;
    bool collapsed(int &exceptioncode) const;

    NodeImpl *commonAncestorContainer(int &exceptioncode);
    void setStart ( NodeImpl *refNode, long offset, int &exceptioncode );
    void setEnd ( NodeImpl *refNode, long offset, int &exceptioncode );
    void collapse ( bool toStart, int &exceptioncode );
    short compareBoundaryPoints ( Range::CompareHow how, const RangeImpl *sourceRange, int &exceptioncode );
    short compareBoundaryPoints ( NodeImpl *containerA, long offsetA, NodeImpl *containerB, long offsetB );
    bool boundaryPointsValid (  );
    void deleteContents ( int &exceptioncode );
    DocumentFragmentImpl *extractContents ( int &exceptioncode );
    DocumentFragmentImpl *cloneContents ( int &exceptioncode );
    void insertNode( const NodeImpl *newNode, int &exceptioncode );
    DOMString toString ( int &exceptioncode );
    DOMString toHTML (  );
    void detach ( int &exceptioncode );
    bool isDetached() const;
    DocumentFragmentImpl *masterTraverse(bool contentExtract, int &exceptioncode);
    RangeImpl *cloneRange(int &exceptioncode);

    void setStartAfter( const NodeImpl *refNode, int &exceptioncode );
    void setEndBefore( const NodeImpl *refNode, int &exceptioncode );
    void setEndAfter( const NodeImpl *refNode, int &exceptioncode );
    void selectNode( const NodeImpl *refNode, int &exceptioncode );
    void selectNodeContents( const NodeImpl *refNode, int &exceptioncode );
    void surroundContents( const NodeImpl *newParent, int &exceptioncode );
    void setStartBefore( const NodeImpl *refNode, int &exceptioncode );

protected:
    DocumentImpl *m_ownerDocument;
    NodeImpl *m_startContainer;
    unsigned long m_startOffset;
    NodeImpl *m_endContainer;
    unsigned long m_endOffset;
    bool m_detached;

private:
    void checkNode( const NodeImpl *n, int &exceptioncode ) const;
    void checkNodeWOffset( const NodeImpl *n, int offset, int &exceptioncode) const;
    void checkNodeBA( const NodeImpl *n, int &exceptioncode ) const;
    void checkCommon(int &exceptioncode) const;
};

}; // namespace

#endif

