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
#ifndef _DOM_DocumentImpl_h_
#define _DOM_DocumentImpl_h_

#include "dom_nodeimpl.h"
#include "dom_node.h"
#include "dom_string.h"
class HTMLImageRequester;

namespace DOM {

class DocumentImpl;
class ElementImpl;
class DocumentFragmentImpl;
class TextImpl;
class AttrImpl;
class NodeListImpl;

class DocumentImpl : public NodeBaseImpl
{
public:
    DocumentImpl();
    ~DocumentImpl();

    virtual const DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual bool isDocumentNode() { return true; }

    //DocumentType doctype() const;

    //DOMImplementation implementation() const;

    ElementImpl *documentElement() const;

    ElementImpl *createElement ( const DOMString &tagName );

    DocumentFragmentImpl *createDocumentFragment ();

    TextImpl *createTextNode ( const DOMString &data );

    //Comment createComment ( const DOMString &data );

    //CDATASection createCDATASection ( const DOMString &data );

    //ProcessingInstruction createProcessingInstruction ( const DOMString &target, const DOMString &data );

    AttrImpl *createAttribute ( const DOMString &name );

    //EntityReference createEntityReference ( const DOMString &name );

    NodeListImpl *getElementsByTagName ( const DOMString &tagname );

    // for KHTML
    virtual DOMString requestImage(HTMLImageRequester *, DOMString ) {}
};

class DocumentFragmentImpl : public NodeImpl
{
public:
    DocumentFragmentImpl(DocumentImpl *doc);
    DocumentFragmentImpl(const DocumentFragmentImpl &other);

    virtual const DOMString nodeName() const;
    virtual unsigned short nodeType() const;
};

}; //namespace
#endif
