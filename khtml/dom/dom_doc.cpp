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

#include "dom/dom_exception.h"
#include "dom/dom_xml.h"
#include "dom/dom2_range.h"
#include "dom/dom2_events.h"
#include "dom/dom2_views.h"
#include "dom/dom2_traversal.h"
#include "dom/html_document.h"
#include "html/html_documentimpl.h"

#include "xml/dom_docimpl.h"
#include "xml/dom_elementimpl.h"

#include <kdebug.h>

using namespace DOM;

DOMImplementation::DOMImplementation()
{
    impl = 0;
}

DOMImplementation::DOMImplementation(const DOMImplementation &other)
{
    impl = other.impl;
    if (impl) impl->ref();
}

DOMImplementation::DOMImplementation(DOMImplementationImpl *i)
{
    impl = i;
    if (impl) impl->ref();
}

DOMImplementation &DOMImplementation::operator = (const DOMImplementation &other)
{
    if ( impl != other.impl ) {
        if (impl) impl->deref();
        impl = other.impl;
        if (impl) impl->ref();
    }
    return *this;
}

DOMImplementation::~DOMImplementation()
{
    if (impl) impl->deref();
}

bool DOMImplementation::hasFeature( const DOMString &feature, const DOMString &version )
{
    if (!impl)
	return false; // ### enable throw DOMException(DOMException::NOT_FOUND_ERR);

    return impl->hasFeature(feature,version);
}

DocumentType DOMImplementation::createDocumentType ( const DOMString &qualifiedName,
                                                     const DOMString &publicId,
                                                     const DOMString &systemId )
{
    if (!impl)
	throw DOMException(DOMException::NOT_FOUND_ERR);

    int exceptioncode = 0;
    DocumentTypeImpl *r = impl->createDocumentType(qualifiedName, publicId, systemId, exceptioncode);
    if ( exceptioncode )
        throw DOMException( exceptioncode );
    return r;
}

Document DOMImplementation::createDocument ( const DOMString &namespaceURI,
                                             const DOMString &qualifiedName,
                                             const DocumentType &doctype )
{
    if (!impl)
	throw DOMException(DOMException::NOT_FOUND_ERR);

    int exceptioncode = 0;
    DocumentImpl *r = impl->createDocument(namespaceURI, qualifiedName, doctype, exceptioncode );
    if ( exceptioncode )
        throw DOMException( exceptioncode );
    return r;
}

HTMLDocument DOMImplementation::createHTMLDocument( const DOMString& title )
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);

    HTMLDocumentImpl* r = impl->createHTMLDocument( 0 /* ### create a view otherwise it doesn't work */);

    r->open();

    r->write(QString::fromLatin1("<HTML><HEAD><TITLE>") + title.string() +
             QString::fromLatin1("</TITLE></HEAD>"));

    return r;
}

DOMImplementation DOMImplementation::getInterface(const DOMString &feature) const
{
    if (!impl)
        throw DOMException(DOMException::NOT_FOUND_ERR);

    return impl->getInterface(feature);
}

CSSStyleSheet DOMImplementation::createCSSStyleSheet(const DOMString &title, const DOMString &media)
{
    if (!impl)
        throw DOMException(DOMException::NOT_FOUND_ERR);

    int exceptioncode = 0;
    CSSStyleSheetImpl *r = impl->createCSSStyleSheet(title.implementation(), media.implementation(),
                                                     exceptioncode);
    if ( exceptioncode )
        throw DOMException( exceptioncode );
    return r;
}

DOMImplementationImpl *DOMImplementation::handle() const
{
    return impl;
}

bool DOMImplementation::isNull() const
{
    return (impl == 0);
}

// ----------------------------------------------------------------------------

Document::Document()
    : Node()
{
    // we always want an implementation
    impl = DOMImplementationImpl::instance()->createDocument();
    impl->ref();
}

Document::Document(bool create)
    : Node()
{
    if(create)
    {
	impl = DOMImplementationImpl::instance()->createDocument();
	impl->ref();
    }
    else
	impl = 0;
//    kdDebug(6090) << "Document::Document(bool)" << endl;
}

Document::Document(const Document &other) : Node(other)
{
//    kdDebug(6090) << "Document::Document(Document &)" << endl;
}

Document::Document(DocumentImpl *i) : Node(i)
{
//    kdDebug(6090) << "Document::Document(DocumentImpl)" << endl;
}

Document &Document::operator = (const Node &other)
{
    NodeImpl* ohandle = other.handle();
    if ( impl != ohandle ) {
        if (!ohandle || ohandle->nodeType() != DOCUMENT_NODE) {
	    if ( impl ) impl->deref();
            impl = 0;
	} else {
            Node::operator =(other);
	}
    }
    return *this;
}

Document &Document::operator = (const Document &other)
{
    Node::operator =(other);
    return *this;
}

Document::~Document()
{
//    kdDebug(6090) << "Document::~Document\n" << endl;
}

DocumentType Document::doctype() const
{
    if (impl) return ((DocumentImpl *)impl)->doctype();
    return 0;
}

DOMImplementation Document::implementation() const
{
    if (impl) return ((DocumentImpl *)impl)->implementation();
    return 0;
}

Element Document::documentElement() const
{
    if (impl) return ((DocumentImpl *)impl)->documentElement();
    return 0;
}

Element Document::createElement( const DOMString &tagName )
{
    if (impl) return ((DocumentImpl *)impl)->createElement(tagName);
    return 0;
}

Element Document::createElementNS( const DOMString &namespaceURI, const DOMString &qualifiedName )
{
    if (impl) return ((DocumentImpl *)impl)->createElementNS(namespaceURI,qualifiedName);
    return 0;
}

DocumentFragment Document::createDocumentFragment(  )
{
    if (impl) return ((DocumentImpl *)impl)->createDocumentFragment();
    return 0;
}

Text Document::createTextNode( const DOMString &data )
{
    if (impl) return ((DocumentImpl *)impl)->createTextNode( data.implementation() );
    return 0;
}

Comment Document::createComment( const DOMString &data )
{
    if (impl) return ((DocumentImpl *)impl)->createComment( data.implementation() );
    return 0;
}

CDATASection Document::createCDATASection( const DOMString &data )
{
    // ### DOM1 spec says raise exception if html documents - what about XHTML documents?
    if (impl) return ((DocumentImpl *)impl)->createCDATASection( data.implementation() );
    return 0;
}

ProcessingInstruction Document::createProcessingInstruction( const DOMString &target, const DOMString &data )
{
    if (impl) return ((DocumentImpl *)impl)->createProcessingInstruction( target, data.implementation() );
    return 0;
}

Attr Document::createAttribute( const DOMString &name )
{
    return createAttributeNS(DOMString(), name);
}

Attr Document::createAttributeNS( const DOMString &namespaceURI, const DOMString &qualifiedName )
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    if (qualifiedName.isNull()) throw DOMException(DOMException::NAMESPACE_ERR);

    DOMString localName(qualifiedName.copy());
    DOMString prefix;
    int colonpos;
    if ((colonpos = qualifiedName.find(':')) >= 0) {
        prefix = qualifiedName.copy();
        prefix.truncate(colonpos);
        localName.remove(0, colonpos+1);
    }

    // ### check correctness of parameters

    NodeImpl::Id id = static_cast<DocumentImpl*>(impl)->attrId(namespaceURI.implementation(), localName.implementation(), false /* allocate */);
    Attr r = static_cast<DocumentImpl*>(impl)->createAttribute(id);
    int exceptioncode = 0;
    if (r.handle() && prefix.implementation())
        r.handle()->setPrefix(prefix.implementation(), exceptioncode);
    if (exceptioncode)
        throw DOMException(exceptioncode);
    return r;
}

EntityReference Document::createEntityReference( const DOMString &name )
{
    if (impl) return ((DocumentImpl *)impl)->createEntityReference( name );
    return 0;
}

Element Document::getElementById( const DOMString &elementId ) const
{
    if(impl) return ((DocumentImpl *)impl)->getElementById( elementId );
    return 0;
}

NodeList Document::getElementsByTagName( const DOMString &tagName )
{
    if (!impl) return 0;
    return static_cast<DocumentImpl*>(impl)->
        getElementsByTagNameNS(0, tagName.implementation());
}

NodeList Document::getElementsByTagNameNS( const DOMString &namespaceURI, const DOMString &localName )
{
    if (!impl) return 0;
    return static_cast<DocumentImpl*>(impl)->
        getElementsByTagNameNS(namespaceURI.implementation(), localName.implementation());
}

Node Document::importNode( const Node & importedNode, bool deep )
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    int exceptioncode = 0;
    NodeImpl *r = static_cast<DocumentImpl*>(impl)->importNode(importedNode.handle(), deep, exceptioncode);
    if (exceptioncode)
	throw DOMException(exceptioncode);
    return r;
}

bool Document::isHTMLDocument() const
{
    if (impl) return ((DocumentImpl *)impl)->isHTMLDocument();
    return 0;
}

Range Document::createRange()
{
    if (impl) return ((DocumentImpl *)impl)->createRange();
    return 0;
}

NodeIterator Document::createNodeIterator(Node root, unsigned long whatToShow,
                                    NodeFilter filter, bool entityReferenceExpansion)
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    int exceptioncode = 0;
    NodeIteratorImpl *r = static_cast<DocumentImpl*>(impl)->createNodeIterator(root.handle(),
			  whatToShow,filter,entityReferenceExpansion,exceptioncode);
    if (exceptioncode)
	throw DOMException(exceptioncode);
    return r;
}

TreeWalker Document::createTreeWalker(Node root, unsigned long whatToShow, NodeFilter filter,
                                bool entityReferenceExpansion)
{
    if (impl) return ((DocumentImpl *)impl)->createTreeWalker(root,whatToShow,filter,entityReferenceExpansion);
    return 0;
}

Event Document::createEvent(const DOMString &eventType)
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    int exceptioncode = 0;
    EventImpl *r = ((DocumentImpl *)impl)->createEvent(eventType,exceptioncode);
    if (exceptioncode)
	throw DOMException(exceptioncode);
    return r;
}

AbstractView Document::defaultView() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return ((DocumentImpl *)impl)->defaultView();
}

StyleSheetList Document::styleSheets() const
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    return ((DocumentImpl *)impl)->styleSheets();
}


KHTMLView *Document::view() const
{
    if (!impl) return 0;

    return static_cast<DocumentImpl*>(impl)->view();
}

DOMString Document::completeURL(const DOMString& url)
{
    if ( !impl ) return url;
    return static_cast<DocumentImpl*>( impl )->completeURL( url.string() );
}

CSSStyleDeclaration Document::getOverrideStyle(const Element &elt, const DOMString &pseudoElt)
{
    if (!impl)
	throw DOMException(DOMException::INVALID_STATE_ERR);

    int exceptioncode = 0;
    CSSStyleDeclarationImpl *r = ((DocumentImpl *)impl)->getOverrideStyle(static_cast<ElementImpl*>(elt.handle()),pseudoElt.implementation());
    if (exceptioncode)
	throw DOMException(exceptioncode);
    return r;
}

// ----------------------------------------------------------------------------

DocumentFragment::DocumentFragment() : Node()
{
}

DocumentFragment::DocumentFragment(const DocumentFragment &other) : Node(other)
{
}

DocumentFragment &DocumentFragment::operator = (const Node &other)
{
    NodeImpl* ohandle = other.handle();
    if ( impl != ohandle ) {
        if (!ohandle || ohandle->nodeType() != DOCUMENT_FRAGMENT_NODE) {
            if ( impl ) impl->deref();
            impl = 0;
        } else {
            Node::operator =(other);
        }
    }
    return *this;
}

DocumentFragment &DocumentFragment::operator = (const DocumentFragment &other)
{
    Node::operator =(other);
    return *this;
}

DocumentFragment::~DocumentFragment()
{
}

DocumentFragment::DocumentFragment(DocumentFragmentImpl *i) : Node(i)
{
}

// ----------------------------------------------------------------------------

DocumentType::DocumentType()
    : Node()
{
}

DocumentType::DocumentType(const DocumentType &other)
    : Node(other)
{
}

DocumentType::DocumentType(DocumentTypeImpl *impl) : Node(impl)
{
}

DocumentType &DocumentType::operator = (const Node &other)
{
    NodeImpl* ohandle = other.handle();
    if ( impl != ohandle ) {
        if (!ohandle || ohandle->nodeType() != DOCUMENT_TYPE_NODE) {
	    if ( impl ) impl->deref();
            impl = 0;
	} else {
            Node::operator =(other);
	}
    }
    return *this;
}

DocumentType &DocumentType::operator = (const DocumentType &other)
{
    Node::operator =(other);
    return *this;
}

DocumentType::~DocumentType()
{
}

DOMString DocumentType::name() const
{
    if (!impl)
	return DOMString(); // ### enable throw DOMException(DOMException::NOT_FOUND_ERR);

    return static_cast<DocumentTypeImpl*>(impl)->name();
}

NamedNodeMap DocumentType::entities() const
{
    if (!impl)
	return 0; // ### enable throw DOMException(DOMException::NOT_FOUND_ERR);

    return static_cast<DocumentTypeImpl*>(impl)->entities();
}

NamedNodeMap DocumentType::notations() const
{
    if (!impl)
	return 0; // ### enable throw DOMException(DOMException::NOT_FOUND_ERR);

    return static_cast<DocumentTypeImpl*>(impl)->notations();
}

DOMString DocumentType::publicId() const
{
    if (!impl)
	throw DOMException(DOMException::NOT_FOUND_ERR);

    return static_cast<DocumentTypeImpl*>(impl)->publicId();
}

DOMString DocumentType::systemId() const
{
    if (!impl)
	throw DOMException(DOMException::NOT_FOUND_ERR);

    return static_cast<DocumentTypeImpl*>(impl)->systemId();
}

DOMString DocumentType::internalSubset() const
{
    if (!impl)
	throw DOMException(DOMException::NOT_FOUND_ERR);

    return static_cast<DocumentTypeImpl*>(impl)->internalSubset();
}

