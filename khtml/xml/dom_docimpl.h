/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
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

#include <qlist.h>
#include <qstringlist.h>

class KHTMLView;

namespace khtml {
    class CSSStyleSelector;
}

namespace DOM {

    class ElementImpl;
    class DocumentFragmentImpl;
    class TextImpl;
    class CommentImpl;
    class AttrImpl;
    class NodeListImpl;
    class StyleSheetListImpl;
    class RangeImpl;
    class NodeIteratorImpl;
    class TreeWalkerImpl;
    class NodeFilterImpl;

class DOMImplementationImpl : public DomShared
{
public:
    DOMImplementationImpl();
    ~DOMImplementationImpl();

    bool hasFeature ( const DOMString &feature, const DOMString &version );
};


/**
 * @internal
 */
class DocumentImpl : public NodeBaseImpl
{
public:
    DocumentImpl();
    DocumentImpl(KHTMLView *v);
    ~DocumentImpl();

    virtual const DOMString nodeName() const;
    virtual unsigned short nodeType() const;
    virtual bool isDocumentNode() { return true; }

    virtual bool isHTMLDocument() { return false; }

    virtual void setStyle(khtml::RenderStyle *style) { m_style = style; }
    virtual khtml::RenderStyle *style() { return m_style; }

    virtual void applyChanges(bool top=true, bool force=true);

    //DocumentType doctype() const;

    //DOMImplementation implementation() const;

    ElementImpl *documentElement() const;

    ElementImpl *createElement ( const DOMString &tagName );

    DocumentFragmentImpl *createDocumentFragment ();

    TextImpl *createTextNode ( const DOMString &data );

    CommentImpl *createComment ( const DOMString &data );

    //CDATASection createCDATASection ( const DOMString &data );

    //ProcessingInstruction createProcessingInstruction ( const DOMString &target, const DOMString &data );

    AttrImpl *createAttribute ( const DOMString &name );

    //EntityReference createEntityReference ( const DOMString &name );

    NodeListImpl *getElementsByTagName ( const DOMString &tagname );

    virtual StyleSheetListImpl *styleSheets();

    khtml::CSSStyleSelector *styleSelector() { return m_styleSelector; }
    virtual void createSelector();

    // Used to maintain list of all elements in the document
    // that want to save and restore state.
    // Returns the state the element should restored to.
    QString registerElement(ElementImpl *);

    // Used to maintain list of all forms in document
    void removeElement(ElementImpl *);

    // Query all registered elements for their state
    QStringList state();

    // Set the state the document should restore to
    void setRestoreState( const QStringList &s) { m_state = s; }

    KHTMLView *view() const { return m_view; }

    RangeImpl *createRange();

    NodeIteratorImpl *createNodeIterator(NodeImpl *root, unsigned long whatToShow,
                                    NodeFilterImpl *filter, bool entityReferenceExpansion);

// ###    TreeWalker createTreeWalker(Node root, unsigned long whatToShow, NodeFilter filter,
//                                bool entityReferenceExpansion);

protected:
    khtml::CSSStyleSelector *m_styleSelector;
    KHTMLView *m_view;
    QList<ElementImpl> m_registeredElements;
    QStringList m_state;

    khtml::RenderStyle *m_style;
};

class DocumentFragmentImpl : public NodeBaseImpl
{
public:
    DocumentFragmentImpl(DocumentImpl *doc);
    DocumentFragmentImpl(const DocumentFragmentImpl &other);

    virtual const DOMString nodeName() const;
    virtual unsigned short nodeType() const;

protected:
    virtual bool childAllowed( NodeImpl *newChild );
};

}; //namespace
#endif
