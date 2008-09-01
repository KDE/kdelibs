/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 *           (C) 2002-2006 Apple Computer, Inc.
 *           (C) 2006 Allan Sandfeld Jensen (kde@carewolf.com)
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "dom_docimpl.h"

#include <dom/dom_exception.h>

#include "dom_textimpl.h"
#include "dom_xmlimpl.h"
#include "dom2_rangeimpl.h"
#include "dom2_eventsimpl.h"
#include "xml_tokenizer.h"
#include <html/htmltokenizer.h>
#include "dom_restyler.h"

#include <css/csshelper.h>
#include <css/cssstyleselector.h>
#include <css/css_stylesheetimpl.h>
#include <misc/htmlhashes.h>
#include <misc/helper.h>
#include <misc/seed.h>
#include <misc/loader.h>
#include <ecma/kjs_proxy.h>
#include <ecma/kjs_binding.h>

#include <QtCore/QStack>
//Added by qt3to4:
#include <QTimerEvent>
#include <QtCore/QList>
#include <kdebug.h>
#include <klocale.h>

#include <rendering/counter_tree.h>
#include <rendering/render_canvas.h>
#include <rendering/render_replaced.h>
#include <rendering/render_arena.h>
#include <rendering/render_layer.h>
#include <rendering/render_frames.h>
#include <rendering/render_image.h>

#include <khtmlview.h>
#include <khtml_part.h>
#include <kauthorized.h>
#include <kglobalsettings.h>
#include <kstringhandler.h>
#include <kdatetime.h>
#include <khtml_settings.h>
#include <khtmlpart_p.h>

#include <html/html_baseimpl.h>
#include <html/html_blockimpl.h>
#include <html/html_canvasimpl.h>
#include <html/html_documentimpl.h>
#include <html/html_formimpl.h>
#include <html/html_headimpl.h>
#include <html/html_imageimpl.h>
#include <html/html_listimpl.h>
#include <html/html_miscimpl.h>
#include <html/html_tableimpl.h>
#include <html/html_objectimpl.h>
#include <html/HTMLAudioElement.h>
#include <html/HTMLVideoElement.h>
#include <html/HTMLSourceElement.h>
#include <editing/htmlediting.h>
#include <editing/jsediting.h>

#include <kio/job.h>

#include <stdlib.h>
#include <limits.h>

template class QStack<DOM::NodeImpl*>;

using namespace DOM;
using namespace khtml;

// ------------------------------------------------------------------------

DOMImplementationImpl *DOMImplementationImpl::m_instance = 0;

DOMImplementationImpl::DOMImplementationImpl()
{
}

DOMImplementationImpl::~DOMImplementationImpl()
{
}

bool DOMImplementationImpl::hasFeature ( const DOMString &feature, const DOMString &version )
{
    // ### update when we (fully) support the relevant features
    QString lower = feature.string().toLower();
    if ((lower == "html" || lower == "xml") &&
        (version.isEmpty() || version == "1.0" || version == "2.0"))
        return true;

    // ## Do we support Core Level 3 ?
    if ((lower == "core" ) &&
        (version.isEmpty() || version == "2.0"))
        return true;

    if ((lower == "traversal") &&
        (version.isEmpty() || version == "2.0"))
        return true;

    if ((lower == "css") &&
        (version.isEmpty() || version == "2.0"))
        return true;

    if ((lower == "events" || lower == "uievents" ||
         lower == "mouseevents" || lower == "mutationevents" ||
         lower == "htmlevents" || lower == "textevents" ) &&
        (version.isEmpty() || version == "2.0" || version == "3.0"))
        return true;
    return false;
}

DocumentTypeImpl *DOMImplementationImpl::createDocumentType( const DOMString &qualifiedName, const DOMString &publicId,
                                                             const DOMString &systemId, int &exceptioncode )
{
    // Not mentioned in spec: throw NAMESPACE_ERR if no qualifiedName supplied
    if (qualifiedName.isNull()) {
        exceptioncode = DOMException::NAMESPACE_ERR;
        return 0;
    }

    // INVALID_CHARACTER_ERR: Raised if the specified qualified name contains an illegal character.
    if (!Element::khtmlValidQualifiedName(qualifiedName)) {
        exceptioncode = DOMException::INVALID_CHARACTER_ERR;
        return 0;
    }

    // NAMESPACE_ERR: Raised if the qualifiedName is malformed.
    // Added special case for the empty string, which seems to be a common pre-DOM2 misuse
    if (!qualifiedName.isEmpty() && Element::khtmlMalformedQualifiedName(qualifiedName)) {
        exceptioncode = DOMException::NAMESPACE_ERR;
        return 0;
    }

    return new DocumentTypeImpl(this,0,qualifiedName,publicId,systemId);
}

DOMImplementationImpl* DOMImplementationImpl::getInterface(const DOMString& /*feature*/) const
{
    // ###
    return 0;
}

DocumentImpl *DOMImplementationImpl::createDocument( const DOMString &namespaceURI, const DOMString &qualifiedName,
                                                     DocumentTypeImpl* dtype,
                                                     KHTMLView* v,
                                                     int &exceptioncode )
{
    exceptioncode = 0;

    if (!checkQualifiedName(qualifiedName, namespaceURI, 0, true/*nameCanBeNull*/,
                            true /*nameCanBeEmpty, see #61650*/, &exceptioncode) )
        return 0;

    // WRONG_DOCUMENT_ERR: Raised if doctype has already been used with a different document or was
    // created from a different implementation.
    if (dtype && (dtype->document() || dtype->implementation() != this)) {
        exceptioncode = DOMException::WRONG_DOCUMENT_ERR;
        return 0;
    }

    // ### view can be 0 which can cause problems
    DocumentImpl* doc;
    if (namespaceURI == XHTML_NAMESPACE)
        doc = new HTMLDocumentImpl(this, v);
    else
        doc = new DocumentImpl(this, v);

    if (dtype) {
        doc->setDocType(dtype);
    }

    // the document must be created empty if all parameters are null
    // (or empty for qName/nsURI as a tolerance) - see DOM 3 Core.
    if (dtype || !qualifiedName.isEmpty() || !namespaceURI.isEmpty()) {
        ElementImpl *element = doc->createElementNS(namespaceURI,qualifiedName);
        doc->appendChild(element,exceptioncode);
        if (exceptioncode) {
            delete element;
            delete doc;
            return 0;
        }
    }
    return doc;
}

CSSStyleSheetImpl *DOMImplementationImpl::createCSSStyleSheet(DOMStringImpl* /*title*/, DOMStringImpl *media,
                                                              int &/*exceptioncode*/)
{
    // ### TODO : title should be set, and media could have wrong syntax, in which case we should
	// generate an exception.
	CSSStyleSheetImpl *parent = 0L;
	CSSStyleSheetImpl *sheet = new CSSStyleSheetImpl(parent, DOMString());
	sheet->setMedia(new MediaListImpl(sheet, media, true /*fallbackToDescriptor*/));
	return sheet;
}

DocumentImpl *DOMImplementationImpl::createDocument( KHTMLView *v )
{
    DocumentImpl* doc = new DocumentImpl(this, v);

    return doc;
}

HTMLDocumentImpl *DOMImplementationImpl::createHTMLDocument( KHTMLView *v )
{
    HTMLDocumentImpl* doc = new HTMLDocumentImpl(this, v);

    return doc;
}

HTMLDocumentImpl* DOMImplementationImpl::createHTMLDocument( const DOMString& title )
{
    HTMLDocumentImpl* r = createHTMLDocument( 0 /* ### create a view otherwise it doesn't work */);

    r->open();

    r->write(QLatin1String("<HTML><HEAD><TITLE>") + title.string() +
             QLatin1String("</TITLE></HEAD>"));

    return r;
}

DOMImplementationImpl *DOMImplementationImpl::instance()
{
    if (!m_instance) {
        m_instance = new DOMImplementationImpl();
        m_instance->ref();
    }

    return m_instance;
}

// ------------------------------------------------------------------------

ElementMappingCache::ElementMappingCache():m_dict()
{
}

ElementMappingCache::~ElementMappingCache()
{
    qDeleteAll( m_dict );
}

void ElementMappingCache::add(const QString& id, ElementImpl* nd)
{
    if (id.isEmpty()) return;

    ItemInfo* info = m_dict.value(id);
    if (info)
    {
        info->ref++;
        info->nd = 0; //Now ambigous
    }
    else
    {
        ItemInfo* info = new ItemInfo();
        info->ref = 1;
        info->nd  = nd;
        m_dict.insert(id, info);
    }
}

void ElementMappingCache::set(const QString& id, ElementImpl* nd)
{
    if (id.isEmpty()) return;

    assert(m_dict.contains(id));
    ItemInfo* info = m_dict.value(id);
    info->nd = nd;
}

void ElementMappingCache::remove(const QString& id, ElementImpl* nd)
{
    if (id.isEmpty()) return;

    assert(m_dict.contains(id));
    ItemInfo* info = m_dict.value(id);
    info->ref--;
    if (info->ref == 0)
    {
        m_dict.take(id);
        delete info;
    }
    else
    {
        if (info->nd == nd)
            info->nd = 0;
    }
}

bool ElementMappingCache::contains(const QString& id)
{
    if (id.isEmpty()) return false;
    return m_dict.contains(id);
}

ElementMappingCache::ItemInfo* ElementMappingCache::get(const QString& id)
{
    if (id.isEmpty()) return 0;
    return m_dict.value(id);
}

typedef QList<DocumentImpl*> ChangedDocuments ;
K_GLOBAL_STATIC(ChangedDocuments, s_changedDocuments)

// KHTMLView might be 0
DocumentImpl::DocumentImpl(DOMImplementationImpl *_implementation, KHTMLView *v)
    : NodeBaseImpl( 0 ), m_domtree_version(0), m_counterDict(),
      m_imageLoadEventTimer(0)
{
    m_document.resetSkippingRef(this); //Make document return us..
    m_selfOnlyRefCount = 0;

    m_paintDevice = 0;
    //m_decoderMibEnum = 0;
    m_textColor = Qt::black;

    m_view = v;
    m_renderArena.reset();

    KHTMLGlobal::registerDocumentImpl(this);

    if ( v ) {
        m_docLoader = new DocLoader(v->part(), this );
        setPaintDevice( m_view );
    }
    else
        m_docLoader = new DocLoader( 0, this );

    visuallyOrdered = false;
    m_bParsing = false;
    m_docChanged = false;
    m_elemSheet = 0;
    m_tokenizer = 0;
    m_doctype = 0;
    m_implementation = _implementation;
    m_implementation->ref();
    pMode = Strict;
    hMode = XHtml;
    m_textColor = "#000000";
    m_attrMap = new IdNameMapping(ATTR_LAST_ATTR+1);
    m_elementMap = new IdNameMapping(ID_LAST_TAG+1);
    m_namespaceMap = new IdNameMapping(1);
    QString xhtml(XHTML_NAMESPACE);
    m_namespaceMap->names.insert(emptyNamespace, new DOMStringImpl(""));
    m_namespaceMap->names.insert(xhtmlNamespace, new DOMStringImpl(xhtml.unicode(), xhtml.length()));
    m_namespaceMap->names[emptyNamespace]->ref();
    m_namespaceMap->names[xhtmlNamespace]->ref();
    m_namespaceMap->count+=2;
    m_focusNode = 0;
    m_hoverNode = 0;
    m_activeNode = 0;
    m_defaultView = new AbstractViewImpl(this);
    m_defaultView->ref();
    m_listenerTypes = 0;
    m_styleSheets = new StyleSheetListImpl;
    m_styleSheets->ref();
    m_addedStyleSheets = 0;
    m_inDocument = true;
    m_styleSelectorDirty = false;
    m_styleSelector = 0;

    m_inStyleRecalc = false;
    m_pendingStylesheets = 0;
    m_ignorePendingStylesheets = false;
    m_async = true;
    m_hadLoadError = false;
    m_docLoading = false;
    m_inSyncLoad = 0;
    m_loadingXMLDoc = 0;
    m_documentElement = 0;
    m_cssTarget = 0;
    m_jsEditor = 0;
    m_dynamicDomRestyler = new khtml::DynamicDomRestyler();
    m_stateRestorePos = 0;
}

void DocumentImpl::removedLastRef()
{
    if (m_selfOnlyRefCount) {
        /* In this case, the only references to us are from children,
           so we have a cycle. We'll try to break it by disconnecting the
           children from us; this sucks/is wrong, but it's pretty much
           the best we can do without tracing.

           Of course, if dumping the children causes the refcount from them to
           drop to 0 we can get killed right here, so better hold
           a temporary reference, too
        */
        DocPtr<DocumentImpl> guard(this);

        // we must make sure not to be retaining any of our children through
        // these extra pointers or we will create a reference cycle
        if (m_doctype) {
            m_doctype->deref();
            m_doctype = 0;
        }

        if (m_cssTarget) {
            m_cssTarget->deref();
            m_cssTarget = 0;
        }

        if (m_focusNode) {
            m_focusNode->deref();
            m_focusNode = 0;
        }

        if (m_hoverNode) {
            m_hoverNode->deref();
            m_hoverNode = 0;
        }

        if (m_activeNode) {
            m_activeNode->deref();
            m_activeNode = 0;
        }

        if (m_documentElement) {
            m_documentElement->deref();
            m_documentElement = 0;
        }

        removeChildren();

        delete m_tokenizer;
        m_tokenizer = 0;
    } else {
        delete this;
    }
}

DocumentImpl::~DocumentImpl()
{
    //Important: if you need to remove stuff here,
    //you may also have to fix removedLastRef() above - M.O.
    assert( !m_render );

    QHashIterator<long,NodeListImpl::Cache*> it(m_nodeListCache);
    while (it.hasNext())
        it.next().value()->deref();

    if (m_loadingXMLDoc)
	m_loadingXMLDoc->deref(this);
    if (s_changedDocuments && m_docChanged)
        s_changedDocuments->removeAll(this);
    delete m_tokenizer;
    m_document.resetSkippingRef(0);
    delete m_styleSelector;
    delete m_docLoader;
    if (m_elemSheet )  m_elemSheet->deref();
    if (m_doctype)
        m_doctype->deref();
    m_implementation->deref();
    delete m_elementMap;
    delete m_attrMap;
    delete m_namespaceMap;
    delete m_dynamicDomRestyler;
    delete m_jsEditor;
    m_defaultView->deref();
    m_styleSheets->deref();
    if (m_addedStyleSheets)
        m_addedStyleSheets->deref();
    if (m_cssTarget)
        m_cssTarget->deref();
    if (m_focusNode)
        m_focusNode->deref();
    if ( m_hoverNode )
        m_hoverNode->deref();
    if (m_activeNode)
        m_activeNode->deref();
    if (m_documentElement)
        m_documentElement->deref();
    qDeleteAll(m_counterDict);

    m_renderArena.reset();

    KHTMLGlobal::deregisterDocumentImpl(this);
}


DocumentTypeImpl *DocumentImpl::doctype() const
{
    return m_doctype;
}

void DocumentImpl::setDocType(DocumentTypeImpl* dt)
{
    assert(m_doctype == 0 && dt != 0);
    m_doctype = dt;
    m_doctype->ref();
    m_doctype->setDocument(this);
}

DOMImplementationImpl *DocumentImpl::implementation() const
{
    return m_implementation;
}

void DocumentImpl::childrenChanged()
{
    // invalidate the document element we have cached in case it was replaced
    if (m_documentElement)
        m_documentElement->deref();
    m_documentElement = 0;
}

ElementImpl *DocumentImpl::documentElement() const
{
    if (!m_documentElement) {
        NodeImpl* n = firstChild();
        while (n && n->nodeType() != Node::ELEMENT_NODE)
            n = n->nextSibling();
        m_documentElement = static_cast<ElementImpl*>(n);
        if (m_documentElement)
            m_documentElement->ref();
    }
    return m_documentElement;
}

ElementImpl *DocumentImpl::createElement( const DOMString &name, int* pExceptioncode )
{
    Id id = getId( NodeImpl::ElementId, name.implementation(),
                   false /* allocate */, false /*HTMLDocumentImpl::createElement looked for HTML elements already*/,
                   pExceptioncode);
    if ( pExceptioncode && *pExceptioncode )
        return 0;

    XMLElementImpl* e = new XMLElementImpl( document(), id );
    e->setHTMLCompat( htmlMode() != XHtml ); // Not a real HTML element, but inside an html-compat doc all tags are uppercase.
    return e;
}

AttrImpl *DocumentImpl::createAttribute( const DOMString &tagName, int* pExceptioncode )
{
    Id id = getId( NodeImpl::AttributeId, tagName.implementation(),
                  false /* allocate */, isHTMLDocument(), pExceptioncode);
    if ( pExceptioncode && *pExceptioncode )
        return 0;
    AttrImpl* attr = new AttrImpl( 0, document(), id, DOMString("").implementation());
    attr->setHTMLCompat( htmlMode() != XHtml );
    return attr;
}

DocumentFragmentImpl *DocumentImpl::createDocumentFragment(  )
{
    return new DocumentFragmentImpl( docPtr() );
}

CommentImpl *DocumentImpl::createComment ( DOMStringImpl* data )
{
    return new CommentImpl( docPtr(), data );
}

CDATASectionImpl *DocumentImpl::createCDATASection ( DOMStringImpl* data )
{
    return new CDATASectionImpl( docPtr(), data );
}

ProcessingInstructionImpl *DocumentImpl::createProcessingInstruction ( const DOMString &target, DOMStringImpl* data )
{
    return new ProcessingInstructionImpl( docPtr(),target,data);
}

EntityReferenceImpl *DocumentImpl::createEntityReference ( const DOMString &name )
{
    return new EntityReferenceImpl(docPtr(), name.implementation());
}

EditingTextImpl *DocumentImpl::createEditingTextNode(const DOMString &text)
{
    return new EditingTextImpl(docPtr(), text);
}

NodeImpl *DocumentImpl::importNode(NodeImpl *importedNode, bool deep, int &exceptioncode)
{
    NodeImpl *result = 0;

    // Not mentioned in spec: throw NOT_FOUND_ERR if evt is null
    if (!importedNode) {
        exceptioncode = DOMException::NOT_FOUND_ERR;
        return 0;
    }

    if(importedNode->nodeType() == Node::ELEMENT_NODE)
    {
        // Why not use cloneNode?
	ElementImpl *otherElem = static_cast<ElementImpl*>(importedNode);
	NamedAttrMapImpl *otherMap = static_cast<ElementImpl *>(importedNode)->attributes(true);

	ElementImpl *tempElementImpl;
	if (!importedNode->localName().isNull())
	    tempElementImpl = createElementNS(otherElem->namespaceURI(),otherElem->nodeName());
	else
	    tempElementImpl = createElement(otherElem->nodeName());
	result = tempElementImpl;


	if(otherMap) {
	    for(unsigned long i = 0; i < otherMap->length(); i++)
	    {
		AttrImpl *otherAttr = otherMap->attrAt(i)->createAttr(otherElem,otherElem->docPtr());

		if (!otherAttr->localName().isNull()) {
		    // attr was created via createElementNS()
		    tempElementImpl->setAttributeNS(otherAttr->namespaceURI(),
						    otherAttr->name(),
						    otherAttr->nodeValue(),
						  exceptioncode);
		}
		else {
		    // attr was created via createElement()
		    tempElementImpl->setAttribute(otherAttr->id(),
                                                  otherAttr->nodeValue(),
                                                  otherAttr->name(),
						  exceptioncode);
		}

		if(exceptioncode != 0)
		    break; // ### properly cleanup here
	    }
	}
    }
    else if(importedNode->nodeType() == Node::TEXT_NODE)
    {
	result = createTextNode(static_cast<TextImpl*>(importedNode)->string());
	deep = false;
    }
    else if(importedNode->nodeType() == Node::CDATA_SECTION_NODE)
    {
	result = createCDATASection(static_cast<CDATASectionImpl*>(importedNode)->string());
	deep = false;
    }
    else if(importedNode->nodeType() == Node::ENTITY_REFERENCE_NODE)
	result = createEntityReference(importedNode->nodeName());
    else if(importedNode->nodeType() == Node::PROCESSING_INSTRUCTION_NODE)
    {
	result = createProcessingInstruction(importedNode->nodeName(), importedNode->nodeValue().implementation());
	deep = false;
    }
    else if(importedNode->nodeType() == Node::COMMENT_NODE)
    {
	result = createComment(static_cast<CommentImpl*>(importedNode)->string());
	deep = false;
    }
    else if (importedNode->nodeType() == Node::DOCUMENT_FRAGMENT_NODE)
	result = createDocumentFragment();
    else
	exceptioncode = DOMException::NOT_SUPPORTED_ERR;

    //### FIXME: This should handle Attributes, and a few other things

    if(deep && result)
    {
	for(Node n = importedNode->firstChild(); !n.isNull(); n = n.nextSibling())
	    result->appendChild(importNode(n.handle(), true, exceptioncode), exceptioncode);
    }

    return result;
}

ElementImpl *DocumentImpl::createElementNS( const DOMString &_namespaceURI, const DOMString &_qualifiedName, int* pExceptioncode )
{
    ElementImpl *e = 0;
    int colonPos = -2;
    // check NAMESPACE_ERR/INVALID_CHARACTER_ERR
    if (pExceptioncode && !checkQualifiedName(_qualifiedName, _namespaceURI, &colonPos,
                                              false/*nameCanBeNull*/, false/*nameCanBeEmpty*/,
                                              pExceptioncode))
        return 0;
    DOMString prefix, localName;
    splitPrefixLocalName(_qualifiedName.implementation(), prefix, localName, colonPos);

    if ((isHTMLDocument() && _namespaceURI.isNull()) ||
        (strcasecmp(_namespaceURI, XHTML_NAMESPACE) == 0 && localName == localName.lower())) {
        e = createHTMLElement(localName);
        if (e) {
            int _exceptioncode = 0;
            if (!prefix.isNull())
                e->setPrefix(prefix, _exceptioncode);
            if ( _exceptioncode ) {
                 if ( pExceptioncode ) *pExceptioncode = _exceptioncode;
                 delete e;
                 return 0;
            }
            e->setHTMLCompat( _namespaceURI.isNull() && htmlMode() != XHtml );
        }
    }
    if (!e) {
        Id id = getId(NodeImpl::ElementId, _namespaceURI.implementation(), prefix.implementation(),
                      localName.implementation(), false, false /*HTML already looked up*/);
        e = new XMLElementImpl( document(), id, prefix.implementation() );
    }

    return e;
}

AttrImpl *DocumentImpl::createAttributeNS( const DOMString &_namespaceURI,
                                           const DOMString &_qualifiedName, int* pExceptioncode)
{
    int colonPos = -2;
    // check NAMESPACE_ERR/INVALID_CHARACTER_ERR
    if (pExceptioncode && !checkQualifiedName(_qualifiedName, _namespaceURI, &colonPos,
                                              false/*nameCanBeNull*/, false/*nameCanBeEmpty*/,
                                              pExceptioncode))
        return 0;
    DOMString prefix, localName;
    splitPrefixLocalName(_qualifiedName.implementation(), prefix, localName, colonPos);
    Id id = getId(NodeImpl::AttributeId, _namespaceURI.implementation(), prefix.implementation(),
                  localName.implementation(), false, true /*lookupHTML*/);
    AttrImpl* attr = new AttrImpl(0, document(), id, DOMString("").implementation(),
                         prefix.implementation());
    attr->setHTMLCompat( _namespaceURI.isNull() && htmlMode() != XHtml );
    return attr;
}

ElementImpl *DocumentImpl::getElementById( const DOMString &elementId ) const
{
    QString stringKey = elementId.string();

    ElementMappingCache::ItemInfo* info = m_getElementByIdCache.get(stringKey);

    if (!info)
        return 0;

    //See if cache has an unambiguous answer.
    if (info->nd)
        return info->nd;

    //Now we actually have to walk.
    QStack<NodeImpl*> nodeStack;
    NodeImpl *current = _first;

    while(1)
    {
        if(!current)
        {
            if(nodeStack.isEmpty()) break;
            current = nodeStack.pop();
            current = current->nextSibling();
        }
        else
        {
            if(current->isElementNode())
            {
                ElementImpl *e = static_cast<ElementImpl *>(current);
                if(e->getAttribute(ATTR_ID) == elementId) {
                    info->nd = e;
                    return e;
                }
            }

            NodeImpl *child = current->firstChild();
            if(child)
            {
                nodeStack.push(current);
                current = child;
            }
            else
            {
                current = current->nextSibling();
            }
        }
    }

    assert(0); //If there is no item with such an ID, we should never get here

    //kDebug() << "WARNING: *DocumentImpl::getElementById not found " << elementId.string();

    return 0;
}

void DocumentImpl::setTitle(const DOMString& _title)
{
    if (_title == m_title && !m_title.isNull()) return;

    m_title = _title;

    QString titleStr = m_title.string();
    for (int i = 0; i < titleStr.length(); ++i)
        if (titleStr[i] < ' ')
            titleStr[i] = ' ';
    titleStr = titleStr.simplified();
    if ( view() && !view()->part()->parentPart() ) {
	if (titleStr.isNull() || titleStr.isEmpty()) {
	    // empty title... set window caption as the URL
	    KUrl url = m_url;
	    url.setRef(QString());
	    url.setQuery(QString());
	    titleStr = url.prettyUrl();
	}

	emit view()->part()->setWindowCaption( KStringHandler::csqueeze( titleStr, 128 ) );
    }
}

DOMString DocumentImpl::nodeName() const
{
    return "#document";
}

unsigned short DocumentImpl::nodeType() const
{
    return Node::DOCUMENT_NODE;
}

ElementImpl *DocumentImpl::createHTMLElement( const DOMString &name )
{
    uint id = khtml::getTagID( name.string().toLower().toLatin1().constData(), name.string().length() );

    ElementImpl *n = 0;
    switch(id)
    {
    case ID_HTML:
        n = new HTMLHtmlElementImpl(docPtr());
        break;
    case ID_HEAD:
        n = new HTMLHeadElementImpl(docPtr());
        break;
    case ID_BODY:
        n = new HTMLBodyElementImpl(docPtr());
        break;

// head elements
    case ID_BASE:
        n = new HTMLBaseElementImpl(docPtr());
        break;
    case ID_LINK:
        n = new HTMLLinkElementImpl(docPtr());
        break;
    case ID_META:
        n = new HTMLMetaElementImpl(docPtr());
        break;
    case ID_STYLE:
        n = new HTMLStyleElementImpl(docPtr());
        break;
    case ID_TITLE:
        n = new HTMLTitleElementImpl(docPtr());
        break;

// frames
    case ID_FRAME:
        n = new HTMLFrameElementImpl(docPtr());
        break;
    case ID_FRAMESET:
        n = new HTMLFrameSetElementImpl(docPtr());
        break;
    case ID_IFRAME:
        n = new HTMLIFrameElementImpl(docPtr());
        break;

// form elements
// ### FIXME: we need a way to set form dependency after we have made the form elements
    case ID_FORM:
            n = new HTMLFormElementImpl(docPtr(), false);
        break;
    case ID_BUTTON:
            n = new HTMLButtonElementImpl(docPtr());
        break;
    case ID_FIELDSET:
            n = new HTMLFieldSetElementImpl(docPtr());
        break;
    case ID_INPUT:
            n = new HTMLInputElementImpl(docPtr());
        break;
    case ID_ISINDEX:
            n = new HTMLIsIndexElementImpl(docPtr());
        break;
    case ID_LABEL:
            n = new HTMLLabelElementImpl(docPtr());
        break;
    case ID_LEGEND:
            n = new HTMLLegendElementImpl(docPtr());
        break;
    case ID_OPTGROUP:
            n = new HTMLOptGroupElementImpl(docPtr());
        break;
    case ID_OPTION:
            n = new HTMLOptionElementImpl(docPtr());
        break;
    case ID_SELECT:
            n = new HTMLSelectElementImpl(docPtr());
        break;
    case ID_TEXTAREA:
            n = new HTMLTextAreaElementImpl(docPtr());
        break;

// lists
    case ID_DL:
        n = new HTMLDListElementImpl(docPtr());
        break;
    case ID_DD:
        n = new HTMLGenericElementImpl(docPtr(), id);
        break;
    case ID_DT:
        n = new HTMLGenericElementImpl(docPtr(), id);
        break;
    case ID_UL:
        n = new HTMLUListElementImpl(docPtr());
        break;
    case ID_OL:
        n = new HTMLOListElementImpl(docPtr());
        break;
    case ID_DIR:
        n = new HTMLDirectoryElementImpl(docPtr());
        break;
    case ID_MENU:
        n = new HTMLMenuElementImpl(docPtr());
        break;
    case ID_LI:
        n = new HTMLLIElementImpl(docPtr());
        break;

// formatting elements (block)
    case ID_DIV:
    case ID_P:
        n = new HTMLDivElementImpl( docPtr(), id );
        break;
    case ID_BLOCKQUOTE:
    case ID_H1:
    case ID_H2:
    case ID_H3:
    case ID_H4:
    case ID_H5:
    case ID_H6:
        n = new HTMLGenericElementImpl(docPtr(), id);
        break;
    case ID_HR:
        n = new HTMLHRElementImpl(docPtr());
        break;
    case ID_PLAINTEXT:
    case ID_XMP:
    case ID_PRE:
        n = new HTMLPreElementImpl(docPtr(), id);
        break;

// font stuff
    case ID_BASEFONT:
        n = new HTMLBaseFontElementImpl(docPtr());
        break;
    case ID_FONT:
        n = new HTMLFontElementImpl(docPtr());
        break;

// ins/del
    case ID_DEL:
    case ID_INS:
        n = new HTMLGenericElementImpl(docPtr(), id);
        break;

// anchor
    case ID_A:
        n = new HTMLAnchorElementImpl(docPtr());
        break;

// images
    case ID_IMG:
        n = new HTMLImageElementImpl(docPtr());
        break;
    case ID_CANVAS:
        n = new HTMLCanvasElementImpl(docPtr());
        break;
    case ID_MAP:
        n = new HTMLMapElementImpl(docPtr());
        /*n = map;*/
        break;
    case ID_AREA:
        n = new HTMLAreaElementImpl(docPtr());
        break;

// objects, applets and scripts
    case ID_APPLET:
        n = new HTMLAppletElementImpl(docPtr());
        break;
    case ID_OBJECT:
        n = new HTMLObjectElementImpl(docPtr());
        break;
    case ID_EMBED:
        n = new HTMLEmbedElementImpl(docPtr());
        break;
    case ID_PARAM:
        n = new HTMLParamElementImpl(docPtr());
        break;
    case ID_SCRIPT:
        n = new HTMLScriptElementImpl(docPtr());
        break;

// media
    case ID_AUDIO:
        n = new HTMLAudioElement(docPtr());
        break;
    case ID_VIDEO:
        n = new HTMLVideoElement(docPtr());
        break;
    case ID_SOURCE:
        n = new HTMLSourceElement(docPtr());
        break;

// tables
    case ID_TABLE:
        n = new HTMLTableElementImpl(docPtr());
        break;
    case ID_CAPTION:
        n = new HTMLTableCaptionElementImpl(docPtr());
        break;
    case ID_COLGROUP:
    case ID_COL:
        n = new HTMLTableColElementImpl(docPtr(), id);
        break;
    case ID_TR:
        n = new HTMLTableRowElementImpl(docPtr());
        break;
    case ID_TD:
    case ID_TH:
        n = new HTMLTableCellElementImpl(docPtr(), id);
        break;
    case ID_THEAD:
    case ID_TBODY:
    case ID_TFOOT:
        n = new HTMLTableSectionElementImpl(docPtr(), id, false);
        break;

// inline elements
    case ID_BR:
        n = new HTMLBRElementImpl(docPtr());
        break;
    case ID_WBR:
        n = new HTMLWBRElementImpl(docPtr());
        break;
    case ID_Q:
        n = new HTMLGenericElementImpl(docPtr(), id);
        break;

// elements with no special representation in the DOM

// block:
    case ID_ADDRESS:
    case ID_CENTER:
        n = new HTMLGenericElementImpl(docPtr(), id);
        break;
// inline
        // %fontstyle
    case ID_TT:
    case ID_U:
    case ID_B:
    case ID_I:
    case ID_S:
    case ID_STRIKE:
    case ID_BIG:
    case ID_SMALL:

        // %phrase
    case ID_EM:
    case ID_STRONG:
    case ID_DFN:
    case ID_CODE:
    case ID_SAMP:
    case ID_KBD:
    case ID_VAR:
    case ID_CITE:
    case ID_ABBR:
    case ID_ACRONYM:

        // %special
    case ID_SUB:
    case ID_SUP:
    case ID_SPAN:
    case ID_NOBR:
    case ID_BDO:
    case ID_NOFRAMES:
    case ID_NOSCRIPT:
    case ID_NOEMBED:
    case ID_NOLAYER:
        n = new HTMLGenericElementImpl(docPtr(), id);
        break;

    case ID_MARQUEE:
        n = new HTMLMarqueeElementImpl(docPtr());
        break;
// text
    case ID_TEXT:
        kDebug( 6020 ) << "Use document->createTextNode()";
        break;

    default:
        break;
    }
    return n;
}

void DocumentImpl::attemptRestoreState(NodeImpl* n)
{
    if (!n->isElementNode())
        return;

    ElementImpl* el = static_cast<ElementImpl*>(n);

    if (m_stateRestorePos >= m_state.size())
        return;

    // Grab the state and element info..
    QString idStr = m_state[m_stateRestorePos];
    QString nmStr = m_state[m_stateRestorePos + 1];
    QString tpStr = m_state[m_stateRestorePos + 2];
    QString stStr = m_state[m_stateRestorePos + 3];

    // Make sure it matches!
    if (idStr.toUInt() != el->id())
        return;
    if (nmStr != el->getAttribute(ATTR_NAME).string())
        return;
    if (tpStr != el->getAttribute(ATTR_TYPE).string())
        return;

    m_stateRestorePos += 4;
    if (!stStr.isNull())
        el->restoreState(stStr);
}

QStringList DocumentImpl::docState()
{
    QStringList s;
    for (QListIterator<NodeImpl*> it(m_maintainsState); it.hasNext();) {
        NodeImpl* n = it.next();
        if (!n->isElementNode())
            continue;

        ElementImpl* el = static_cast<ElementImpl*>(n);
        // Encode the element ID, as well as the name and type attributes
        s.append(QString::number(el->id()));
        s.append(el->getAttribute(ATTR_NAME).string());
        s.append(el->getAttribute(ATTR_TYPE).string());
        s.append(el->state());
    }

    return s;
}

bool DocumentImpl::unsubmittedFormChanges()
{
    for (QListIterator<NodeImpl*> it(m_maintainsState); it.hasNext();)
        if (it.next()->state().endsWith('M'))
            return true;

    return false;
}

RangeImpl *DocumentImpl::createRange()
{
    return new RangeImpl( docPtr() );
}

NodeIteratorImpl *DocumentImpl::createNodeIterator(NodeImpl *root, unsigned long whatToShow,
                                                   NodeFilterImpl* filter, bool entityReferenceExpansion,
                                                   int &exceptioncode)
{
    if (!root) {
        exceptioncode = DOMException::NOT_SUPPORTED_ERR;
        return 0;
    }

    return new NodeIteratorImpl(root,whatToShow,filter,entityReferenceExpansion);
}

TreeWalkerImpl *DocumentImpl::createTreeWalker(NodeImpl *root, unsigned long whatToShow, NodeFilterImpl *filter,
                                bool entityReferenceExpansion, int &exceptioncode)
{
    if (!root) {
        exceptioncode = DOMException::NOT_SUPPORTED_ERR;
        return 0;
    }

    return new TreeWalkerImpl( root, whatToShow, filter, entityReferenceExpansion );
}

void DocumentImpl::setDocumentChanged(bool b)
{
    if (b && !m_docChanged)
        s_changedDocuments->append(this);
    else if (!b && m_docChanged)
        s_changedDocuments->removeAll(this);
    m_docChanged = b;
}

void DocumentImpl::recalcStyle( StyleChange change )
{
//     qDebug("recalcStyle(%p)", this);
//     QTime qt;
//     qt.start();
    if (m_inStyleRecalc)
        return; // Guard against re-entrancy. -dwh

    m_inStyleRecalc = true;

    if( !m_render ) goto bail_out;

    if ( change == Force ) {
        RenderStyle* oldStyle = m_render->style();
        if ( oldStyle ) oldStyle->ref();
        RenderStyle* _style = new RenderStyle();
        _style->setDisplay(BLOCK);
        _style->setVisuallyOrdered( visuallyOrdered );
        // ### make the font stuff _really_ work!!!!

	khtml::FontDef fontDef;
	QFont f = KGlobalSettings::generalFont();
	fontDef.family = f.family();
	fontDef.italic = f.italic();
	fontDef.weight = f.weight();
        if (m_view) {
            const KHTMLSettings *settings = m_view->part()->settings();
	    QString stdfont = settings->stdFontName();
	    if ( !stdfont.isEmpty() )
		fontDef.family = stdfont;

            fontDef.size = m_styleSelector->fontSizes()[3];
        }

        //kDebug() << "DocumentImpl::attach: setting to charset " << settings->charset();
        _style->setFontDef(fontDef);
	_style->htmlFont().update( 0 );
        if ( inCompatMode() )
            _style->setHtmlHacks(true); // enable html specific rendering tricks

        StyleChange ch = diff( _style, oldStyle );
        if(m_render && ch != NoChange)
            m_render->setStyle(_style);
	else
	    delete _style;

        if (oldStyle)
            oldStyle->deref();
    }

    NodeImpl *n;
    for (n = _first; n; n = n->nextSibling())
        if ( change>= Inherit || n->hasChangedChild() || n->changed() )
            n->recalcStyle( change );
    //kDebug( 6020 ) << "TIME: recalcStyle() dt=" << qt.elapsed();

    if (changed() && m_view)
	m_view->layout();

bail_out:
    setChanged( false );
    setHasChangedChild( false );
    setDocumentChanged( false );

    m_inStyleRecalc = false;
}

void DocumentImpl::updateRendering()
{
    if (!hasChangedChild()) return;

//     QTime time;
//     time.start();
//     kDebug() << "UPDATERENDERING: ";

    StyleChange change = NoChange;
#if 0
    if ( m_styleSelectorDirty ) {
	recalcStyleSelector();
	change = Force;
    }
#endif
    recalcStyle( change );

//    kDebug() << "UPDATERENDERING time used="<<time.elapsed();
}

void DocumentImpl::updateDocumentsRendering()
{
    if (!s_changedDocuments)
        return;

    while ( !s_changedDocuments->isEmpty() ) {
        DocumentImpl* it = s_changedDocuments->takeFirst();
        if (it->isDocumentChanged())
            it->updateRendering();
    }
}

void DocumentImpl::updateLayout()
{
    if (ElementImpl* oe = ownerElement())
        oe->document()->updateLayout();

    bool oldIgnore = m_ignorePendingStylesheets;

    if (!haveStylesheetsLoaded()) {
        m_ignorePendingStylesheets = true;
        updateStyleSelector();
    }

    updateRendering();

    // Only do a layout if changes have occurred that make it necessary.
    if (m_view && renderer() && renderer()->needsLayout())
	m_view->layout();

    m_ignorePendingStylesheets = oldIgnore;
}

void DocumentImpl::attach()
{
    assert(!attached());

    if ( m_view )
        setPaintDevice( m_view );

    if (!m_renderArena)
	m_renderArena.reset(new RenderArena());

    // Create the rendering tree
    assert(!m_styleSelector);
    m_styleSelector = new CSSStyleSelector( this, m_usersheet, m_styleSheets, m_url,
                                            !inCompatMode() );
    m_render = new (m_renderArena.get()) RenderCanvas(this, m_view);
    m_styleSelector->computeFontSizes(m_paintDevice->logicalDpiY(), m_view ? m_view->part()->fontScaleFactor() : 100);
    recalcStyle( Force );

    RenderObject* render = m_render;
    m_render = 0;

    NodeBaseImpl::attach();
    m_render = render;
}

void DocumentImpl::detach()
{
    RenderObject* render = m_render;

    // indicate destruction mode,  i.e. attached() but m_render == 0
    m_render = 0;

    delete m_tokenizer;
    m_tokenizer = 0;

    // Empty out these lists as a performance optimization
    m_imageLoadEventDispatchSoonList.clear();
    m_imageLoadEventDispatchingList.clear();
    NodeBaseImpl::detach();

    if ( render )
        render->detach();

    m_view = 0;

    m_renderArena.reset();
}

void DocumentImpl::setVisuallyOrdered()
{
    visuallyOrdered = true;
    if (m_render)
        m_render->style()->setVisuallyOrdered(true);
}

void DocumentImpl::setSelection(NodeImpl* s, int sp, NodeImpl* e, int ep)
{
    if ( m_render )
        static_cast<RenderCanvas*>(m_render)->setSelection(s->renderer(),sp,e->renderer(),ep);
}

void DocumentImpl::clearSelection()
{
    if ( m_render )
        static_cast<RenderCanvas*>(m_render)->clearSelection();
}

void DocumentImpl::updateSelection()
{
    if (!m_render)
        return;

    RenderCanvas *canvas = static_cast<RenderCanvas*>(m_render);
    Selection s = part()->caret();
    if (s.isEmpty() || s.state() == Selection::CARET) {
        canvas->clearSelection();
    }
    else {
        RenderObject *startRenderer = s.start().node() ? s.start().node()->renderer() : 0;
        RenderObject *endRenderer = s.end().node() ? s.end().node()->renderer() : 0;
        static_cast<RenderCanvas*>(m_render)->setSelection(startRenderer, s.start().offset(), endRenderer, s.end().offset());
    }
}

khtml::Tokenizer *DocumentImpl::createTokenizer()
{
    return new khtml::XMLTokenizer(docPtr(),m_view);
}

int DocumentImpl::logicalDpiY()
{
    return m_paintDevice->logicalDpiY();
}

void DocumentImpl::open( bool clearEventListeners )
{
    if (parsing()) return;

    if (m_tokenizer)
        close();

    delete m_tokenizer;
    m_tokenizer = 0;

    KHTMLView* view = m_view;
    bool was_attached = attached();
    if ( was_attached )
        detach();

    removeChildren();
    childrenChanged(); // Reset m_documentElement
    delete m_styleSelector;
    m_styleSelector = 0;
    m_view = view;
    if ( was_attached )
        attach();

    if (clearEventListeners)
        m_windowEventListeners.clear();

    m_tokenizer = createTokenizer();
    //m_decoderMibEnum = 0;
    connect(m_tokenizer,SIGNAL(finishedParsing()),this,SIGNAL(finishedParsing()));
    m_tokenizer->begin();
}

HTMLElementImpl* DocumentImpl::body() const
{
    NodeImpl *de = documentElement();
    if (!de)
        return 0;

    // try to prefer a FRAMESET element over BODY
    NodeImpl* body = 0;
    for (NodeImpl* i = de->firstChild(); i; i = i->nextSibling()) {
        if (i->id() == ID_FRAMESET)
            return static_cast<HTMLElementImpl*>(i);

        if (i->id() == ID_BODY)
            body = i;
    }
    return static_cast<HTMLElementImpl *>(body);
}

void DocumentImpl::close(  )
{
    if (parsing() || !m_tokenizer) return;

    if ( m_render )
        m_render->close();

    // on an explicit document.close(), the tokenizer might still be waiting on scripts,
    // and in that case we don't want to destroy it because that will prevent the
    // scripts from getting processed.
    if (m_tokenizer && !m_tokenizer->isWaitingForScripts() && !m_tokenizer->isExecutingScript()) {
        delete m_tokenizer;
        m_tokenizer = 0;
    }

    if (m_view)
        m_view->part()->checkEmitLoadEvent();
}

void DocumentImpl::write( const DOMString &text )
{
    write(text.string());
}

void DocumentImpl::write( const QString &text )
{
    if (!m_tokenizer) {
        open();
        if (m_view)
            m_view->part()->resetFromScript();
        m_tokenizer->setAutoClose();
        write(QLatin1String("<html>"));
    }
    m_tokenizer->write(text, false);
}

void DocumentImpl::writeln( const DOMString &text )
{
    write(text);
    write(DOMString("\n"));
}

void DocumentImpl::finishParsing (  )
{
    if(m_tokenizer)
        m_tokenizer->finish();
}

void DocumentImpl::setUserStyleSheet( const QString& sheet )
{
    if ( m_usersheet != sheet ) {
        m_usersheet = sheet;
        updateStyleSelector();
    }
}

CSSStyleSheetImpl* DocumentImpl::elementSheet()
{
    if (!m_elemSheet) {
        m_elemSheet = new CSSStyleSheetImpl(this, baseURL().url() );
        m_elemSheet->ref();
    }
    return m_elemSheet;
}

void DocumentImpl::determineParseMode()
{
    // For XML documents, use strict parse mode
    pMode = Strict;
    hMode = XHtml;
    kDebug(6020) << " using strict parseMode";
}

NodeImpl *DocumentImpl::nextFocusNode(NodeImpl *fromNode)
{
    short fromTabIndex;

    if (!fromNode) {
	// No starting node supplied; begin with the top of the document
	NodeImpl *n;

	int lowestTabIndex = SHRT_MAX + 1;
	for (n = this; n != 0; n = n->traverseNextNode()) {
	    if (n->isTabFocusable()) {
		if ((n->tabIndex() > 0) && (n->tabIndex() < lowestTabIndex))
		    lowestTabIndex = n->tabIndex();
	    }
	}

	if (lowestTabIndex == SHRT_MAX + 1)
	    lowestTabIndex = 0;

	// Go to the first node in the document that has the desired tab index
	for (n = this; n != 0; n = n->traverseNextNode()) {
	    if (n->isTabFocusable() && (n->tabIndex() == lowestTabIndex))
		return n;
	}

	return 0;
    }
    else {
	fromTabIndex = fromNode->tabIndex();
    }

    if (fromTabIndex == 0) {
	// Just need to find the next selectable node after fromNode (in document order) that doesn't have a tab index
	NodeImpl *n = fromNode->traverseNextNode();
	while (n && !(n->isTabFocusable() && n->tabIndex() == 0))
	    n = n->traverseNextNode();
	return n;
    }
    else {
	// Find the lowest tab index out of all the nodes except fromNode, that is greater than or equal to fromNode's
	// tab index. For nodes with the same tab index as fromNode, we are only interested in those that come after
	// fromNode in document order.
	// If we don't find a suitable tab index, the next focus node will be one with a tab index of 0.
	int lowestSuitableTabIndex = SHRT_MAX + 1;
	NodeImpl *n;

	bool reachedFromNode = false;
	for (n = this; n != 0; n = n->traverseNextNode()) {
	    if (n->isTabFocusable() &&
		((reachedFromNode && (n->tabIndex() >= fromTabIndex)) ||
		 (!reachedFromNode && (n->tabIndex() > fromTabIndex))) &&
		(n->tabIndex() < lowestSuitableTabIndex) &&
		(n != fromNode)) {

		// We found a selectable node with a tab index at least as high as fromNode's. Keep searching though,
		// as there may be another node which has a lower tab index but is still suitable for use.
		lowestSuitableTabIndex = n->tabIndex();
	    }

	    if (n == fromNode)
		reachedFromNode = true;
	}

	if (lowestSuitableTabIndex == SHRT_MAX + 1) {
	    // No next node with a tab index -> just take first node with tab index of 0
	    NodeImpl *n = this;
	    while (n && !(n->isTabFocusable() && n->tabIndex() == 0))
		n = n->traverseNextNode();
	    return n;
	}

	// Search forwards from fromNode
	for (n = fromNode->traverseNextNode(); n != 0; n = n->traverseNextNode()) {
	    if (n->isTabFocusable() && (n->tabIndex() == lowestSuitableTabIndex))
		return n;
	}

	// The next node isn't after fromNode, start from the beginning of the document
	for (n = this; n != fromNode; n = n->traverseNextNode()) {
	    if (n->isTabFocusable() && (n->tabIndex() == lowestSuitableTabIndex))
		return n;
	}

	assert(false); // should never get here
	return 0;
    }
}

NodeImpl *DocumentImpl::previousFocusNode(NodeImpl *fromNode)
{
    NodeImpl *lastNode = this;
    while (lastNode->lastChild())
	lastNode = lastNode->lastChild();

    if (!fromNode) {
	// No starting node supplied; begin with the very last node in the document
	NodeImpl *n;

	int highestTabIndex = 0;
	for (n = lastNode; n != 0; n = n->traversePreviousNode()) {
	    if (n->isTabFocusable()) {
		if (n->tabIndex() == 0)
		    return n;
		else if (n->tabIndex() > highestTabIndex)
		    highestTabIndex = n->tabIndex();
	    }
	}

	// No node with a tab index of 0; just go to the last node with the highest tab index
	for (n = lastNode; n != 0; n = n->traversePreviousNode()) {
	    if (n->isTabFocusable() && (n->tabIndex() == highestTabIndex))
		return n;
	}

	return 0;
    }
    else {
	short fromTabIndex = fromNode->tabIndex();

	if (fromTabIndex == 0) {
	    // Find the previous selectable node before fromNode (in document order) that doesn't have a tab index
	    NodeImpl *n = fromNode->traversePreviousNode();
	    while (n && !(n->isTabFocusable() && n->tabIndex() == 0))
		n = n->traversePreviousNode();
	    if (n)
		return n;

	    // No previous nodes with a 0 tab index, go to the last node in the document that has the highest tab index
	    int highestTabIndex = 0;
	    for (n = this; n != 0; n = n->traverseNextNode()) {
		if (n->isTabFocusable() && (n->tabIndex() > highestTabIndex))
		    highestTabIndex = n->tabIndex();
	    }

	    if (highestTabIndex == 0)
		return 0;

	    for (n = lastNode; n != 0; n = n->traversePreviousNode()) {
		if (n->isTabFocusable() && (n->tabIndex() == highestTabIndex))
		    return n;
	    }

	    assert(false); // should never get here
	    return 0;
	}
	else {
	    // Find the lowest tab index out of all the nodes except fromNode, that is less than or equal to fromNode's
	    // tab index. For nodes with the same tab index as fromNode, we are only interested in those before
	    // fromNode.
	    // If we don't find a suitable tab index, then there will be no previous focus node.
	    short highestSuitableTabIndex = 0;
	    NodeImpl *n;

	    bool reachedFromNode = false;
	    for (n = this; n != 0; n = n->traverseNextNode()) {
		if (n->isTabFocusable() &&
		    ((!reachedFromNode && (n->tabIndex() <= fromTabIndex)) ||
		     (reachedFromNode && (n->tabIndex() < fromTabIndex)))  &&
		    (n->tabIndex() > highestSuitableTabIndex) &&
		    (n != fromNode)) {

		    // We found a selectable node with a tab index no higher than fromNode's. Keep searching though, as
		    // there may be another node which has a higher tab index but is still suitable for use.
		    highestSuitableTabIndex = n->tabIndex();
		}

		if (n == fromNode)
		    reachedFromNode = true;
	    }

	    if (highestSuitableTabIndex == 0) {
		// No previous node with a tab index. Since the order specified by HTML is nodes with tab index > 0
		// first, this means that there is no previous node.
		return 0;
	    }

	    // Search backwards from fromNode
	    for (n = fromNode->traversePreviousNode(); n != 0; n = n->traversePreviousNode()) {
		if (n->isTabFocusable() && (n->tabIndex() == highestSuitableTabIndex))
		    return n;
	    }
	    // The previous node isn't before fromNode, start from the end of the document
	    for (n = lastNode; n != fromNode; n = n->traversePreviousNode()) {
		if (n->isTabFocusable() && (n->tabIndex() == highestSuitableTabIndex))
		    return n;
	    }

	    assert(false); // should never get here
	    return 0;
	}
    }
}

ElementImpl* DocumentImpl::findAccessKeyElement(QChar c)
{
    c = c.toUpper();
    for( NodeImpl* n = this;
         n != NULL;
         n = n->traverseNextNode()) {
        if( n->isElementNode()) {
            ElementImpl* en = static_cast< ElementImpl* >( n );
            DOMString s = en->getAttribute( ATTR_ACCESSKEY );
            if( s.length() == 1
                && s[ 0 ].toUpper() == c )
                return en;
        }
    }
    return NULL;
}

int DocumentImpl::nodeAbsIndex(NodeImpl *node)
{
    assert(node->document() == this);

    int absIndex = 0;
    for (NodeImpl *n = node; n && n != this; n = n->traversePreviousNode())
	absIndex++;
    return absIndex;
}

NodeImpl *DocumentImpl::nodeWithAbsIndex(int absIndex)
{
    NodeImpl *n = this;
    for (int i = 0; n && (i < absIndex); i++) {
	n = n->traverseNextNode();
    }
    return n;
}

void DocumentImpl::processHttpEquiv(const DOMString &equiv, const DOMString &content)
{
    assert(!equiv.isNull() && !content.isNull());

    KHTMLView *v = document()->view();

    if(strcasecmp(equiv, "refresh") == 0 && v && v->part()->metaRefreshEnabled())
    {
        // get delay and url
        QString str = content.string().trimmed();
        int pos = str.indexOf(QRegExp("[;,]"));
        if ( pos == -1 )
            pos = str.indexOf(QRegExp("[ \t]"));

        bool ok = false;
	int delay = qMax( 0, content.implementation()->toInt(&ok) );
        if ( !ok && str.length() && str[0] == '.' )
            ok = true;

        if (pos == -1) // There can be no url (David)
        {
            if(ok)
                v->part()->scheduleRedirection(delay, v->part()->url().url() );
        } else {
            pos++;
            while(pos < (int)str.length() && str[pos].isSpace()) pos++;
            str = str.mid(pos);
            if(str.indexOf("url", 0, Qt::CaseInsensitive ) == 0)  str = str.mid(3);
            str = str.trimmed();
            if ( str.length() && str[0] == '=' ) str = str.mid( 1 ).trimmed();
            while(str.length() &&
                  (str[str.length()-1] == ';' || str[str.length()-1] == ','))
                str.resize(str.length()-1);
            str = parseURL( DOMString(str) ).string();
            QString newURL = document()->completeURL( str );
            if ( ok )
                v->part()->scheduleRedirection(delay, document()->completeURL( str ),  delay < 2 || newURL == URL().url());
        }
    }
    else if(strcasecmp(equiv, "expires") == 0)
    {
        bool relative = false;
        QString str = content.string().trimmed();
        time_t expire_date = KDateTime::fromString(str, KDateTime::RFCDate).toTime_t();
        if (!expire_date)
        {
            expire_date = str.toULong();
            relative = true;
        }
        if (!expire_date)
            expire_date = 1; // expire now
        if (m_docLoader)
            m_docLoader->setExpireDate(expire_date, relative);
    }
    else if(v && (strcasecmp(equiv, "pragma") == 0 || strcasecmp(equiv, "cache-control") == 0))
    {
        QString str = content.string().toLower().trimmed();
        KUrl url = v->part()->url();
        if ((str == "no-cache") && url.protocol().startsWith(QLatin1String("http")))
        {
           KIO::http_update_cache(url, true, 0);
        }
    }
    else if( (strcasecmp(equiv, "set-cookie") == 0))
    {
        // ### make setCookie work on XML documents too; e.g. in case of <html:meta .....>
        HTMLDocumentImpl *d = static_cast<HTMLDocumentImpl *>(this);
        d->setCookie(content);
    }
    else if (strcasecmp(equiv, "default-style") == 0) {
        // HTML 4.0 14.3.2
        // http://www.hixie.ch/tests/evil/css/import/main/preferred.html
        m_preferredStylesheetSet = content;
        updateStyleSelector();
    }
    else if (strcasecmp(equiv, "content-language") == 0) {
        m_contentLanguage = content.string();
    }
}

bool DocumentImpl::prepareMouseEvent( bool readonly, int _x, int _y, MouseEvent *ev )
{
    if ( m_render ) {
        assert(m_render->isCanvas());
        RenderObject::NodeInfo renderInfo(readonly, ev->type == MousePress);
        bool isInside = m_render->layer()->nodeAtPoint(renderInfo, _x, _y);
        ev->innerNode = renderInfo.innerNode();
	ev->innerNonSharedNode = renderInfo.innerNonSharedNode();

        if (renderInfo.URLElement()) {
            assert(renderInfo.URLElement()->isElementNode());
            //qDebug("urlnode: %s  (%d)", getTagName(renderInfo.URLElement()->id()).string().toLatin1().constData(), renderInfo.URLElement()->id());

            ElementImpl* e =  static_cast<ElementImpl*>(renderInfo.URLElement());
            DOMString href = khtml::parseURL(e->getAttribute(ATTR_HREF));
            DOMString target = e->getAttribute(ATTR_TARGET);

            if (!target.isNull() && !href.isNull()) {
                ev->target = target;
                ev->url = href;
            }
            else
                ev->url = href;
        }

        if (!readonly)
            updateRendering();

        return isInside;
    }


    return false;
}


// DOM Section 1.1.1
bool DocumentImpl::childTypeAllowed( unsigned short type )
{
    switch (type) {
        case Node::ATTRIBUTE_NODE:
        case Node::CDATA_SECTION_NODE:
        case Node::DOCUMENT_FRAGMENT_NODE:
        case Node::DOCUMENT_NODE:
        case Node::ENTITY_NODE:
        case Node::ENTITY_REFERENCE_NODE:
        case Node::NOTATION_NODE:
        case Node::TEXT_NODE:
//        case Node::XPATH_NAMESPACE_NODE:
            return false;
        case Node::COMMENT_NODE:
        case Node::PROCESSING_INSTRUCTION_NODE:
            return true;
        case Node::DOCUMENT_TYPE_NODE:
        case Node::ELEMENT_NODE:
            // Documents may contain no more than one of each of these.
            // (One Element and one DocumentType.)
            for (NodeImpl* c = firstChild(); c; c = c->nextSibling())
                if (c->nodeType() == type)
                    return false;
            return true;
    }
    return false;
}

WTF::PassRefPtr<NodeImpl> DocumentImpl::cloneNode ( bool deep )
{
#if 0
    NodeImpl *dtn = m_doctype->cloneNode(deep);
    DocumentTypeImpl *dt = static_cast<DocumentTypeImpl*>(dtn);
#endif

    int exceptioncode;
    WTF::RefPtr<NodeImpl> clone = m_implementation->createDocument("",
							   "",
                                                           0, 0,
                                                           exceptioncode);
    assert( exceptioncode == 0 );

    // ### attributes, styles, ...

    if (deep)
	cloneChildNodes(clone.get());

    return clone;
}

typedef const char* (*NameLookupFunction)(unsigned short id);
typedef int (*IdLookupFunction)(const char *tagStr, int len);

NodeImpl::Id DocumentImpl::getId( NodeImpl::IdType _type, DOMStringImpl* _nsURI, DOMStringImpl *_prefix,
                                  DOMStringImpl *_name, bool readonly, bool /*lookupHTML*/, int *pExceptioncode)
{
    /*kDebug() << "DocumentImpl::getId( type: " << _type << ", uri: " << DOMString(_nsURI).string()
              << ", prefix: " << DOMString(_prefix).string() << ", name: " << DOMString(_name).string()
              << ", readonly: " << readonly
              << ", lookupHTML: " << lookupHTML
              << ", exceptions: " << (pExceptioncode ? "yes" : "no")
              << " )" << endl;*/

    if(!_name) return 0;
    IdNameMapping *map;
    IdLookupFunction lookup;

    switch (_type) {
    case NodeImpl::ElementId:
        map = m_elementMap;
        lookup = getTagID;
        break;
    case NodeImpl::AttributeId:
        map = m_attrMap;
        lookup = getAttrID;
        break;
    case NodeImpl::NamespaceId:
        if( strcasecmp(_name, XHTML_NAMESPACE) == 0)
            return xhtmlNamespace;
        if( _name->l == 0)
            return emptyNamespace;
        // defaultNamespace handled by "if (!_name) return 0"
        map = m_namespaceMap;
        lookup= 0;
        break;
    default:
        return 0;
    }
    // Names and attributes with ""
    if (_name->l == 0) return 0;

    NodeImpl::Id id, nsid;
    id = nsid = 0;
    const QString n = QString::fromRawData(_name->s, _name->l);
    bool cs = true; // case sensitive
    if (_type != NodeImpl::NamespaceId) {
        if (_nsURI)
            nsid = getId( NodeImpl::NamespaceId, 0, 0, _nsURI, false, false, 0 );

        // for attributes empty and default namespaces are the same
        if (_type == NodeImpl::AttributeId && nsid == emptyNamespace)
            nsid = defaultNamespace;

        // Each document maintains a mapping of tag name -> id for every tag name encountered
        // in the document.
        cs = (htmlMode() == XHtml) || (_nsURI && _type != NodeImpl::AttributeId);

        // First see if it's a HTML element name
        // xhtml is lower case - case sensitive, easy to implement
        if ( cs && (id = lookup(n.toAscii().constData(), _name->l)) ) {
            map->addAlias(_prefix, _name, cs, id);
            return makeId(nsid, id);
        }
        // compatibility: upper case - case insensitive
        if ( !cs && (id = lookup(n.toLower().toAscii().constData(), _name->l )) ) {
            map->addAlias(_prefix, _name, cs, id);
            return makeId(nsid, id);
        }
    }

    // Look in the names array for the name
    // compatibility mode has to lookup upper case
    QString name = cs ? n : n.toUpper();

    if (!_nsURI) {
        id = (NodeImpl::Id)(long)map->ids.value( name );
        if (!id && _type != NodeImpl::NamespaceId) {
            id = (NodeImpl::Id)(long)map->ids.value( "aliases: " + name );
	}
    } else {
        id = (NodeImpl::Id)(long)map->ids.value( name );
        if (!readonly && id && _prefix && _prefix->l) {
            // we were called in registration mode... check if the alias exists
            const QString px = QString::fromRawData( _prefix->s, _prefix->l );
            QString qn("aliases: " + (cs ? px : px.toUpper()) + ":" + name);
            if (!map->ids.contains( qn )) {
                map->ids.insert( qn, (void*)id );
            }
        }
    }

    if (id) return makeId(nsid, id);

    // unknown
    if (readonly) return 0;

    if ( pExceptioncode && _type != NodeImpl::NamespaceId && !Element::khtmlValidQualifiedName(_name)) {
        *pExceptioncode = DOMException::INVALID_CHARACTER_ERR;
        return 0;
    }

    // Name not found, so let's add it
    NodeImpl::Id cid = map->count++ + map->idStart;
    map->names.insert( cid, _name );
    _name->ref();

    map->ids.insert( name, (void*)cid );

    // and register an alias if needed for DOM1 methods compatibility
    map->addAlias(_prefix, _name, cs, cid);

    return makeId(nsid, cid);
 }

NodeImpl::Id DocumentImpl::getId( NodeImpl::IdType _type, DOMStringImpl *_nodeName, bool readonly, bool lookupHTML, int *pExceptioncode)
{
     return getId(_type, 0, 0, _nodeName, readonly, lookupHTML, pExceptioncode);
}

DOMString DocumentImpl::getName( NodeImpl::IdType _type, NodeImpl::Id _id ) const
{
    IdNameMapping *map;
    NameLookupFunction lookup;
    bool hasNS = (namespacePart(_id) != defaultNamespace);
    switch (_type) {
    case NodeImpl::ElementId:
        map = m_elementMap;
        lookup = getTagName;
        break;
    case NodeImpl::AttributeId:
        map = m_attrMap;
        lookup = getAttrName;
        break;
    case NodeImpl::NamespaceId:
        if( _id == xhtmlNamespace )
            return XHTML_NAMESPACE;
        else
        if( _id == emptyNamespace )
            return DOMString("");
        else
        if ( _id == defaultNamespace )
            return DOMString();
        map = m_namespaceMap;
        lookup = 0;
        break;
    default:
        return DOMString();;
    }
    _id = localNamePart(_id) ;
    if (_id >= map->idStart) {
        return map->names[_id];
    }
    else if (lookup) {
        // ### put them in a cache
        if (hasNS)
            return DOMString(lookup(_id)).lower();
        else
            return lookup(_id);
    } else
        return DOMString();
}

// This method is called whenever a top-level stylesheet has finished loading.
void DocumentImpl::styleSheetLoaded()
{
  // Make sure we knew this sheet was pending, and that our count isn't out of sync.
  assert(m_pendingStylesheets > 0);

  m_pendingStylesheets--;
  updateStyleSelector();
}

void DocumentImpl::addPendingSheet()
{
     m_pendingStylesheets++;
}

DOMString DocumentImpl::selectedStylesheetSet() const
{
    if (!view()) return DOMString();

    return view()->part()->d->m_sheetUsed;
}

void DocumentImpl::setSelectedStylesheetSet(const DOMString& s)
{
    // this code is evil
    if (view() && view()->part()->d->m_sheetUsed != s.string()) {
        view()->part()->d->m_sheetUsed = s.string();
        updateStyleSelector();
    }
}

void DocumentImpl::addStyleSheet(StyleSheetImpl *sheet, int *exceptioncode)
{
    int excode = 0;

    if (!m_addedStyleSheets) {
        m_addedStyleSheets = new StyleSheetListImpl;
        m_addedStyleSheets->ref();
    }

    m_addedStyleSheets->add(sheet);
    if (sheet->isCSSStyleSheet()) updateStyleSelector();

    if (exceptioncode) *exceptioncode = excode;
}

void DocumentImpl::removeStyleSheet(StyleSheetImpl *sheet, int *exceptioncode)
{
    int excode = 0;
    bool removed = false;
    bool is_css = sheet->isCSSStyleSheet();

    if (m_addedStyleSheets) {
        bool in_main_list = !sheet->hasOneRef();
        removed = m_addedStyleSheets->styleSheets.removeAll(sheet);
        sheet->deref();

        if (m_addedStyleSheets->styleSheets.count() == 0) {
            bool reset = m_addedStyleSheets->hasOneRef();
            m_addedStyleSheets->deref();
            if (reset) m_addedStyleSheets = 0;
        }

        // remove from main list, too
        if (in_main_list) m_styleSheets->remove(sheet);
    }

    if (removed) {
        if (is_css) updateStyleSelector();
    } else
        excode = DOMException::NOT_FOUND_ERR;

    if (exceptioncode) *exceptioncode = excode;
}

void DocumentImpl::updateStyleSelector(bool shallow)
{
//    kDebug() << "PENDING " << m_pendingStylesheets;

    // Don't bother updating, since we haven't loaded all our style info yet.
    if (m_pendingStylesheets > 0)
        return;

    if (shallow)
        rebuildStyleSelector();
    else
        recalcStyleSelector();
    recalcStyle(Force);
#if 0

    m_styleSelectorDirty = true;
#endif
    if ( renderer() )
        renderer()->setNeedsLayoutAndMinMaxRecalc();
}

bool DocumentImpl::readyForLayout() const
{
    return renderer() && haveStylesheetsLoaded() && (!isHTMLDocument() || (body() && body()->renderer()));
}

void DocumentImpl::recalcStyleSelector()
{
    if ( !m_render || !attached() ) return;

    assert(m_pendingStylesheets==0);

    QList<StyleSheetImpl*> oldStyleSheets = m_styleSheets->styleSheets;
    m_styleSheets->styleSheets.clear();
    QString sheetUsed = view() ? view()->part()->d->m_sheetUsed.replace("&&", "&") : QString();
    bool autoselect = sheetUsed.isEmpty();
    if (autoselect && !m_preferredStylesheetSet.isEmpty())
        sheetUsed = m_preferredStylesheetSet.string();
    NodeImpl *n;
    for (int i=0 ; i<2 ; i++) {
        m_availableSheets.clear();
        m_availableSheets << i18n("Basic Page Style");
        bool canResetSheet = false;

        for (n = this; n; n = n->traverseNextNode()) {
            StyleSheetImpl *sheet = 0;

            if (n->nodeType() == Node::PROCESSING_INSTRUCTION_NODE)
            {
                // Processing instruction (XML documents only)
                ProcessingInstructionImpl* pi = static_cast<ProcessingInstructionImpl*>(n);
                sheet = pi->sheet();
                if (!sheet && !pi->localHref().isEmpty())
                {
                    // Processing instruction with reference to an element in this document - e.g.
                    // <?xml-stylesheet href="#mystyle">, with the element
                    // <foo id="mystyle">heading { color: red; }</foo> at some location in
                    // the document
                    ElementImpl* elem = getElementById(pi->localHref());
                    if (elem) {
                        DOMString sheetText("");
                        NodeImpl *c;
                        for (c = elem->firstChild(); c; c = c->nextSibling()) {
                            if (c->nodeType() == Node::TEXT_NODE || c->nodeType() == Node::CDATA_SECTION_NODE)
                                sheetText += c->nodeValue();
                        }

                        CSSStyleSheetImpl *cssSheet = new CSSStyleSheetImpl(this);
                        cssSheet->parseString(sheetText);
                        pi->setStyleSheet(cssSheet);
                        sheet = cssSheet;
                    }
                }

            }
            else if (n->isHTMLElement() && ( n->id() == ID_LINK || n->id() == ID_STYLE) ) {
                QString title;
                if ( n->id() == ID_LINK ) {
                    HTMLLinkElementImpl* l = static_cast<HTMLLinkElementImpl*>(n);
                    if (l->isCSSStyleSheet()) {
                        sheet = l->sheet();

                        if (sheet || l->isLoading() || l->isAlternate() )
                            title = l->getAttribute(ATTR_TITLE).string();

			if ((autoselect || title != sheetUsed) && l->isDisabled()) {
                            sheet = 0;
                        } else if (!title.isEmpty() && !l->isAlternate() && sheetUsed.isEmpty()) {
                            sheetUsed = title;
                            l->setDisabled(false);
                        }
                    }
                }
                else {
                    // <STYLE> element
                    HTMLStyleElementImpl* s = static_cast<HTMLStyleElementImpl*>(n);
                    if (!s->isLoading()) {
                        sheet = s->sheet();
                        if (sheet) title = s->getAttribute(ATTR_TITLE).string();
                    }
                    if (!title.isEmpty() && sheetUsed.isEmpty())
                        sheetUsed = title;
                }

                if ( !title.isEmpty() ) {
                    if ( title != sheetUsed )
                        sheet = 0; // don't use it

                    title = title.replace('&',  "&&");

                    if ( !m_availableSheets.contains( title ) )
                        m_availableSheets.append( title );
                }
            }
            else if (n->isHTMLElement() && n->id() == ID_BODY) {
                // <BODY> element (doesn't contain styles as such but vlink="..." and friends
                // are treated as style declarations)
                sheet = static_cast<HTMLBodyElementImpl*>(n)->sheet();
            }

            if (sheet) {
                sheet->ref();
                m_styleSheets->styleSheets.append(sheet);
            }

            // For HTML documents, stylesheets are not allowed within/after the <BODY> tag. So we
            // can stop searching here.
            if (isHTMLDocument() && n->id() == ID_BODY) {
                canResetSheet = !canResetSheet;
                break;
            }
        }

        // we're done if we don't select an alternative sheet
        // or we found the sheet we selected
        if (sheetUsed.isEmpty() ||
            (!canResetSheet && tokenizer()) ||
            m_availableSheets.contains(sheetUsed)) {
            break;
        }

        // the alternative sheet we used doesn't exist anymore
        // so try from scratch again
        if (view())
            view()->part()->d->m_sheetUsed.clear();
        if (!m_preferredStylesheetSet.isEmpty() && !(sheetUsed == m_preferredStylesheetSet))
            sheetUsed = m_preferredStylesheetSet.string();
        else
            sheetUsed.clear();
        autoselect = true;
    }

    // Include programmatically added style sheets
    if (m_addedStyleSheets) {
        foreach (StyleSheetImpl* sh, m_addedStyleSheets->styleSheets) {
            if (sh->isCSSStyleSheet() && !sh->disabled())
                m_styleSheets->add(sh);
        }
    }

    // De-reference all the stylesheets in the old list
    foreach ( StyleSheetImpl* sh, oldStyleSheets)
        sh->deref();

    rebuildStyleSelector();
}

void DocumentImpl::rebuildStyleSelector()
{
    // Create a new style selector
    delete m_styleSelector;
    QString usersheet = m_usersheet;
    if ( m_view && m_view->mediaType() == "print" )
	usersheet += m_printSheet;
    m_styleSelector = new CSSStyleSelector( this, usersheet, m_styleSheets, m_url,
                                            !inCompatMode() );

    m_styleSelectorDirty = false;
}

void DocumentImpl::setBaseURL(const KUrl& _baseURL)
{
    m_baseURL = _baseURL;
    if (m_elemSheet)
        m_elemSheet->setHref( baseURL().url() );
}

void DocumentImpl::setHoverNode(NodeImpl *newHoverNode)
{
    NodeImpl* oldHoverNode = m_hoverNode;
    if (newHoverNode ) newHoverNode->ref();
    m_hoverNode = newHoverNode;
    if ( oldHoverNode ) oldHoverNode->deref();
}

void DocumentImpl::setActiveNode(NodeImpl* newActiveNode)
{
    NodeImpl* oldActiveNode = m_activeNode;
    if (newActiveNode ) newActiveNode->ref();
    m_activeNode = newActiveNode;
    if ( oldActiveNode ) oldActiveNode->deref();
}

void DocumentImpl::quietResetFocus()
{
    assert(m_focusNode != this);
    if (m_focusNode) {
        if (m_focusNode->active())
            setActiveNode(0);

        m_focusNode->setFocus(false);
        m_focusNode->deref();
    }
    m_focusNode = 0;

    //We're blurring. Better clear the Qt focus/give it to the view...
    if (view())
        view()->setFocus();
}

void DocumentImpl::setFocusNode(NodeImpl *newFocusNode)
{
    // don't process focus changes while detaching
    if( !m_render ) return;

    // See if the new node is really focusable. It might not be
    // if focus() was called explicitly.
    if (newFocusNode && !newFocusNode->isFocusable())
        return;

    // Make sure newFocusNode is actually in this document
    if (newFocusNode && (newFocusNode->document() != this))
        return;

    if (m_focusNode != newFocusNode) {
        NodeImpl *oldFocusNode = m_focusNode;
        // Set focus on the new node
        m_focusNode = newFocusNode;
        // Remove focus from the existing focus node (if any)
        if (oldFocusNode) {
            if (oldFocusNode->active())
                oldFocusNode->setActive(false);

            oldFocusNode->setFocus(false);

            oldFocusNode->dispatchHTMLEvent(EventImpl::BLUR_EVENT,false,false);
            oldFocusNode->dispatchUIEvent(EventImpl::DOMFOCUSOUT_EVENT);

            if ((oldFocusNode == this) && oldFocusNode->hasOneRef()) {
                oldFocusNode->deref(); // deletes this
                return;
            }
	    else {
                oldFocusNode->deref();
            }
        }

        if (m_focusNode) {
            m_focusNode->ref();
            m_focusNode->dispatchHTMLEvent(EventImpl::FOCUS_EVENT,false,false);
            if (m_focusNode != newFocusNode) return;
            m_focusNode->dispatchUIEvent(EventImpl::DOMFOCUSIN_EVENT);
            if (m_focusNode != newFocusNode) return;
            m_focusNode->setFocus();
            if (m_focusNode != newFocusNode) return;

            // eww, I suck. set the qt focus correctly
            // ### find a better place in the code for this
            if (view()) {
                if (!m_focusNode->renderer() || !m_focusNode->renderer()->isWidget())
                    view()->setFocus();
                else if (static_cast<RenderWidget*>(m_focusNode->renderer())->widget())
                {
                    if (view()->isVisible())
                        static_cast<RenderWidget*>(m_focusNode->renderer())->widget()->setFocus();
                }
            }
        } else {
            //We're blurring. Better clear the Qt focus/give it to the view...
            if (view())
                view()->setFocus();
        }

        updateRendering();
    }
}

void DocumentImpl::setCSSTarget(NodeImpl* n)
{
    if (n == m_cssTarget)
        return;

    if (m_cssTarget) {
        m_cssTarget->setChanged();
        m_cssTarget->deref();
    }
    m_cssTarget = n;
    if (n) {
        n->setChanged();
        n->ref();
    }
}

void DocumentImpl::attachNodeIterator(NodeIteratorImpl *ni)
{
    m_nodeIterators.append(ni);
}

void DocumentImpl::detachNodeIterator(NodeIteratorImpl *ni)
{
    int i = m_nodeIterators.indexOf(ni);
    if (i != -1)
        m_nodeIterators.removeAt(i);
}

void DocumentImpl::notifyBeforeNodeRemoval(NodeImpl *n)
{
    QListIterator<NodeIteratorImpl*> it(m_nodeIterators);
    while (it.hasNext())
        it.next()->notifyBeforeNodeRemoval(n);
}

bool DocumentImpl::isURLAllowed(const QString& url) const
{
    KHTMLPart *thisPart = part();

    KUrl newURL(completeURL(url));
    newURL.setRef(QString());

    if (KHTMLGlobal::defaultHTMLSettings()->isAdFiltered( newURL.url() ))
        return false;

    // Prohibit non-file URLs if we are asked to.
    if (!thisPart || thisPart->onlyLocalReferences() && newURL.protocol() != "file" && newURL.protocol() != "data")
        return false;

    // do we allow this suburl ?
    if (newURL.protocol() != "javascript" && !KAuthorized::authorizeUrlAction("redirect", thisPart->url(), newURL))
        return false;

    // We allow one level of self-reference because some sites depend on that.
    // But we don't allow more than one.
    bool foundSelfReference = false;
    for (KHTMLPart *part = thisPart; part; part = part->parentPart()) {
        KUrl partURL = part->url();
        partURL.setRef(QString());
        if (partURL == newURL) {
            if (foundSelfReference)
                return false;
            foundSelfReference = true;
        }
    }

    return true;
}

void DocumentImpl::setDesignMode(bool b)
{
    if (part())
         part()->setEditable(b);
}

bool DocumentImpl::designMode() const
{
    return part() ? part()->isEditable() : false;
}

EventImpl *DocumentImpl::createEvent(const DOMString &eventType, int &exceptioncode)
{
    if (eventType == "UIEvents" || eventType == "UIEvent")
        return new UIEventImpl();
    else if (eventType == "MouseEvents" || eventType == "MouseEvent")
        return new MouseEventImpl();
    else if (eventType == "TextEvent")
        return new TextEventImpl();
    else if (eventType == "KeyboardEvent")
        return new KeyboardEventImpl();
    else if (eventType == "MutationEvents" || eventType == "MutationEvent")
        return new MutationEventImpl();
    else if (eventType == "HTMLEvents" || eventType == "Events" ||
             eventType == "HTMLEvent"  || eventType == "Event")
        return new EventImpl();
    else {
        exceptioncode = DOMException::NOT_SUPPORTED_ERR;
        return 0;
    }
}

CSSStyleDeclarationImpl *DocumentImpl::getOverrideStyle(ElementImpl* /*elt*/, DOMStringImpl* /*pseudoElt*/)
{
    return 0; // ###
}

void DocumentImpl::abort()
{
    if (m_inSyncLoad) {
        assert(m_inSyncLoad->isRunning());
        m_inSyncLoad->exit();
    }

    if (m_loadingXMLDoc)
	m_loadingXMLDoc->deref(this);
    m_loadingXMLDoc = 0;
}

void DocumentImpl::load(const DOMString &uri)
{
    if (m_inSyncLoad) {
        assert(m_inSyncLoad->isRunning());
        m_inSyncLoad->exit();
    }

    m_hadLoadError = false;
    if (m_loadingXMLDoc)
	m_loadingXMLDoc->deref(this);

    // Use the document loader to retrieve the XML file. We use CachedCSSStyleSheet because
    // this is an easy way to retrieve an arbitrary text file... it is not specific to
    // stylesheets.

    // ### Note: By loading the XML document this way we do not get the proper decoding
    // of the data retrieved from the server based on the character set, as happens with
    // HTML files. Need to look into a way of using the decoder in CachedCSSStyleSheet.
    m_docLoading = true;
    m_loadingXMLDoc = m_docLoader->requestStyleSheet(uri.string(),QString(),"text/xml");

    if (!m_loadingXMLDoc) {
	m_docLoading = false;
	return;
    }

    m_loadingXMLDoc->ref(this);

    if (!m_async && m_docLoading) {
        assert(!m_inSyncLoad);
	m_inSyncLoad = new QEventLoop();
	m_inSyncLoad->exec();
	// returning from event loop:
	assert(!m_inSyncLoad->isRunning());
	delete m_inSyncLoad;
	m_inSyncLoad = 0;
    }
}

void DocumentImpl::loadXML(const DOMString &source)
{
    open(false);
    write(source);
    finishParsing();
    close();
    dispatchHTMLEvent(EventImpl::LOAD_EVENT,false,false);
}

void DocumentImpl::setStyleSheet(const DOM::DOMString &url, const DOM::DOMString &sheet, const DOM::DOMString &/*charset*/, const DOM::DOMString &mimetype)
{
    if (!m_hadLoadError) {
	m_url = url.string();
	loadXML(khtml::isAcceptableCSSMimetype(mimetype) ? sheet : "");
    }

    m_docLoading = false;
    if (m_inSyncLoad) {
        assert(m_inSyncLoad->isRunning());
        m_inSyncLoad->exit();
    }

    assert(m_loadingXMLDoc != 0);
    m_loadingXMLDoc->deref(this);
    m_loadingXMLDoc = 0;
}

void DocumentImpl::error(int err, const QString &text)
{
    m_docLoading = false;
    if (m_inSyncLoad) {
        assert(m_inSyncLoad->isRunning());
        m_inSyncLoad->exit();
    }

    m_hadLoadError = true;

    int exceptioncode = 0;
    EventImpl *evt = new EventImpl(EventImpl::ERROR_EVENT,false,false);
    if (err != 0)
      evt->setMessage(KIO::buildErrorString(err,text));
    else
      evt->setMessage(text);
    evt->ref();
    dispatchEvent(evt,exceptioncode,true);
    evt->deref();

    assert(m_loadingXMLDoc != 0);
    m_loadingXMLDoc->deref(this);
    m_loadingXMLDoc = 0;
}

void DocumentImpl::defaultEventHandler(EventImpl *evt)
{
    // if any html event listeners are registered on the window, then dispatch them here
    if (m_windowEventListeners.listeners && !evt->propagationStopped()) {

        QList<RegisteredEventListener>::iterator it;

        //Grab a copy in case of clear
        QList<RegisteredEventListener> listeners = *m_windowEventListeners.listeners;
        Event ev(evt);
        for (it = listeners.begin(); it != listeners.end(); ++it) {
            //Check to make sure it didn't get removed. KDE4: use Java-style iterators
            if (!m_windowEventListeners.stillContainsListener(*it))
                continue;

            if ((*it).eventName == evt->name()) {
                // currentTarget must be 0 in khtml for kjs_events to set "this" correctly.
                // (this is how we identify events dispatched to the window, like window.onmousedown)
                // ## currentTarget is unimplemented in IE, and is "window" in Mozilla (how? not a DOM node)
                evt->setCurrentTarget(0);
                (*it).listener->handleEvent(ev);
	    }
        }
    }
    if ( evt->id() == EventImpl::KHTML_CONTENTLOADED_EVENT && !evt->propagationStopped() && !evt->defaultPrevented() )
        contentLoaded();
}

void DocumentImpl::setHTMLWindowEventListener(EventName id, EventListener *listener)
{
    m_windowEventListeners.setHTMLEventListener(id, listener);
}

void DocumentImpl::setHTMLWindowEventListener(unsigned id, EventListener *listener)
{
    m_windowEventListeners.setHTMLEventListener(EventName::fromId(id), listener);
}

EventListener *DocumentImpl::getHTMLWindowEventListener(EventName id)
{
    return m_windowEventListeners.getHTMLEventListener(id);
}

EventListener *DocumentImpl::getHTMLWindowEventListener(unsigned id)
{
    return m_windowEventListeners.getHTMLEventListener(EventName::fromId(id));
}

void DocumentImpl::addWindowEventListener(EventName id, EventListener *listener, const bool useCapture)
{
    m_windowEventListeners.addEventListener(id, listener, useCapture);
}

void DocumentImpl::removeWindowEventListener(EventName id, EventListener *listener, bool useCapture)
{
    m_windowEventListeners.removeEventListener(id, listener, useCapture);
}

bool DocumentImpl::hasWindowEventListener(EventName id)
{
    return m_windowEventListeners.hasEventListener(id);
}

EventListener *DocumentImpl::createHTMLEventListener(const QString& code, const QString& name, NodeImpl* node)
{
    return part() ? part()->createHTMLEventListener(code, name, node) : 0;
}

void DocumentImpl::dispatchImageLoadEventSoon(HTMLImageElementImpl *image)
{
    m_imageLoadEventDispatchSoonList.append(image);
    if (!m_imageLoadEventTimer) {
        m_imageLoadEventTimer = startTimer(0);
    }
}

void DocumentImpl::removeImage(HTMLImageElementImpl *image)
{
    // Remove instances of this image from both lists.
    m_imageLoadEventDispatchSoonList.removeAll(image);
    m_imageLoadEventDispatchingList.removeAll(image);
    if (m_imageLoadEventDispatchSoonList.isEmpty() && m_imageLoadEventTimer) {
        killTimer(m_imageLoadEventTimer);
        m_imageLoadEventTimer = 0;
    }
}

void DocumentImpl::dispatchImageLoadEventsNow()
{
    // need to avoid re-entering this function; if new dispatches are
    // scheduled before the parent finishes processing the list, they
    // will set a timer and eventually be processed
    if (!m_imageLoadEventDispatchingList.isEmpty()) {
        return;
    }

    if (m_imageLoadEventTimer) {
        killTimer(m_imageLoadEventTimer);
        m_imageLoadEventTimer = 0;
    }

    m_imageLoadEventDispatchingList = m_imageLoadEventDispatchSoonList;
    m_imageLoadEventDispatchSoonList.clear();
    for (QLinkedListIterator<HTMLImageElementImpl*> it(m_imageLoadEventDispatchingList); it.hasNext(); )
        it.next()->dispatchLoadEvent();
    m_imageLoadEventDispatchingList.clear();
}

void DocumentImpl::timerEvent(QTimerEvent *)
{
    dispatchImageLoadEventsNow();
}

/*void DocumentImpl::setDecoderCodec(const QTextCodec *codec)
{
    m_decoderMibEnum = codec->mibEnum();
}*/

ElementImpl *DocumentImpl::ownerElement() const
{
    KHTMLPart *childPart = part();
    if (!childPart)
        return 0;
    ChildFrame *childFrame = childPart->d->m_frame;
    if (!childFrame)
        return 0;
    ElementImpl *el = childFrame->m_partContainerElement;
    return el;
}

DOMString DocumentImpl::domain() const
{
    if ( m_domain.isEmpty() ) // not set yet (we set it on demand to save time and space)
        m_domain = URL().host(); // Initially set to the host
    return m_domain;
}

void DocumentImpl::setDomain(const DOMString &newDomain)
{
    if ( m_domain.isEmpty() ) // not set yet (we set it on demand to save time and space)
        m_domain = URL().host().toLower(); // Initially set to the host

    if ( m_domain.isEmpty() /*&& part() && part()->openedByJS()*/ )
        m_domain = newDomain.lower();

    // Both NS and IE specify that changing the domain is only allowed when
    // the new domain is a suffix of the old domain.
    int oldLength = m_domain.length();
    int newLength = newDomain.length();
    if ( newLength < oldLength ) // e.g. newDomain=kde.org (7) and m_domain=www.kde.org (11)
    {
        DOMString test = m_domain.copy();
        DOMString reference = newDomain.lower();
        if ( test[oldLength - newLength - 1] == '.' ) // Check that it's a subdomain, not e.g. "de.org"
        {
            test.remove( 0, oldLength - newLength ); // now test is "kde.org" from m_domain
            if ( test == reference )                 // and we check that it's the same thing as newDomain
                m_domain = reference;
        }
    }
}

DOMString DocumentImpl::toString() const
{
    DOMString result;

    for (NodeImpl *child = firstChild(); child != NULL; child = child->nextSibling()) {
        result += child->toString();
    }

    return result;
}

void DOM::DocumentImpl::setRestoreState( const QStringList &s)
{
    m_state = s;
    m_stateRestorePos = 0;
}

KHTMLView* DOM::DocumentImpl::view() const
{
    return m_view;
}

KHTMLPart* DOM::DocumentImpl::part() const
{
    // ### TODO: make this independent from a KHTMLView one day.
    return view() ? view()->part() : 0;
}

NodeListImpl::Cache* DOM::DocumentImpl::acquireCachedNodeListInfo(
       NodeListImpl::CacheFactory* factory, NodeImpl* base, int type)
{
    //### might want to flush the dict when the version number
    //changes
    NodeListImpl::CacheKey key(base, type);

    //Check to see if we have this sort of item cached.
    NodeListImpl::Cache* cached =
        (type == NodeListImpl::UNCACHEABLE) ? 0 : m_nodeListCache.value(key.hash());

    if (cached) {
        if (cached->key == key) {
            cached->ref(); //Add the nodelist's reference
            return cached;
        } else {
            //Conflict. Drop our reference to the old item.
            cached->deref();
        }
    }

    //Nothing to reuse, make a new item.
    NodeListImpl::Cache* newInfo = factory();
    newInfo->key = key;
    newInfo->clear(this);
    newInfo->ref(); //Add the nodelist's reference

    if (type != NodeListImpl::UNCACHEABLE) {
        newInfo->ref(); //Add the cache's reference
        m_nodeListCache.insert(key.hash(), newInfo);
    }

    return newInfo;
}

void DOM::DocumentImpl::releaseCachedNodeListInfo(NodeListImpl::Cache* entry)
{
    entry->deref();
}

// ----------------------------------------------------------------------------
// Support for Javascript execCommand, and related methods

JSEditor *DocumentImpl::jsEditor()
{
    if (!m_jsEditor)
        m_jsEditor = new JSEditor(this);
    return m_jsEditor;
}

bool DocumentImpl::execCommand(const DOMString &command, bool userInterface, const DOMString &value)
{
    return jsEditor()->execCommand(jsEditor()->commandImp(command), userInterface, value);
}

bool DocumentImpl::queryCommandEnabled(const DOMString &command)
{
    return jsEditor()->queryCommandEnabled(jsEditor()->commandImp(command));
}

bool DocumentImpl::queryCommandIndeterm(const DOMString &command)
{
    return jsEditor()->queryCommandIndeterm(jsEditor()->commandImp(command));
}

bool DocumentImpl::queryCommandState(const DOMString &command)
{
    return jsEditor()->queryCommandState(jsEditor()->commandImp(command));
}

bool DocumentImpl::queryCommandSupported(const DOMString &command)
{
    return jsEditor()->queryCommandSupported(jsEditor()->commandImp(command));
}

DOMString DocumentImpl::queryCommandValue(const DOMString &command)
{
    return jsEditor()->queryCommandValue(jsEditor()->commandImp(command));
}

// ----------------------------------------------------------------------------

DocumentFragmentImpl::DocumentFragmentImpl(DocumentImpl *doc) : NodeBaseImpl(doc)
{
}

DocumentFragmentImpl::DocumentFragmentImpl(const DocumentFragmentImpl &other)
    : NodeBaseImpl(other)
{
}

DOMString DocumentFragmentImpl::nodeName() const
{
  return "#document-fragment";
}

unsigned short DocumentFragmentImpl::nodeType() const
{
    return Node::DOCUMENT_FRAGMENT_NODE;
}

// DOM Section 1.1.1
bool DocumentFragmentImpl::childTypeAllowed( unsigned short type )
{
    switch (type) {
        case Node::ELEMENT_NODE:
        case Node::PROCESSING_INSTRUCTION_NODE:
        case Node::COMMENT_NODE:
        case Node::TEXT_NODE:
        case Node::CDATA_SECTION_NODE:
        case Node::ENTITY_REFERENCE_NODE:
            return true;
            break;
        default:
            return false;
    }
}

DOMString DocumentFragmentImpl::toString() const
{
    DOMString result;

    for (NodeImpl *child = firstChild(); child != NULL; child = child->nextSibling()) {
        if (child->nodeType() == Node::COMMENT_NODE || child->nodeType() == Node::PROCESSING_INSTRUCTION_NODE)
            continue;
	result += child->toString();
    }

    return result;
}

WTF::PassRefPtr<NodeImpl> DocumentFragmentImpl::cloneNode ( bool deep )
{
    WTF::RefPtr<DocumentFragmentImpl> clone = new DocumentFragmentImpl( docPtr() );
    if (deep)
        cloneChildNodes(clone.get());
    return clone;
}


// ----------------------------------------------------------------------------

DocumentTypeImpl::DocumentTypeImpl(DOMImplementationImpl *implementation, DocumentImpl *doc,
                                   const DOMString &qualifiedName, const DOMString &publicId,
                                   const DOMString &systemId)
    : NodeImpl(doc), m_implementation(implementation),
      m_qualifiedName(qualifiedName), m_publicId(publicId), m_systemId(systemId)
{
    m_implementation->ref();

    m_entities = 0;
    m_notations = 0;

    // if doc is 0, it is not attached to a document and / or
    // therefore does not provide entities or notations. (DOM Level 3)
}

DocumentTypeImpl::~DocumentTypeImpl()
{
    m_implementation->deref();
    if (m_entities)
        m_entities->deref();
    if (m_notations)
        m_notations->deref();
}

DOMString DocumentTypeImpl::toString() const
{
    DOMString result = "<!DOCTYPE ";
    result += m_qualifiedName;
    if (!m_publicId.isEmpty()) {
	result += " PUBLIC \"";
	result += m_publicId;
	result += "\" \"";
	result += m_systemId;
	result += "\"";
    } else if (!m_systemId.isEmpty()) {
	result += " SYSTEM \"";
	result += m_systemId;
	result += "\"";
    }

    if (!m_subset.isEmpty()) {
	result += " [";
	result += m_subset;
	result += "]";
    }

    result += ">";

    return result;
}

DOMString DocumentTypeImpl::nodeName() const
{
    return name();
}

unsigned short DocumentTypeImpl::nodeType() const
{
    return Node::DOCUMENT_TYPE_NODE;
}

// DOM Section 1.1.1
bool DocumentTypeImpl::childTypeAllowed( unsigned short /*type*/ )
{
    return false;
}

WTF::PassRefPtr<NodeImpl> DocumentTypeImpl::cloneNode ( bool /*deep*/ )
{
    DocumentTypeImpl *clone = new DocumentTypeImpl(implementation(),
						   0,
						   name(), publicId(),
						   systemId());
    // ### copy entities etc.
    return clone;
}

NamedNodeMapImpl * DocumentTypeImpl::entities() const
{
    if ( !m_entities ) {
        m_entities = new GenericRONamedNodeMapImpl( docPtr() );
        m_entities->ref();
    }
    return m_entities;
}

NamedNodeMapImpl * DocumentTypeImpl::notations() const
{
    if ( !m_notations ) {
        m_notations = new GenericRONamedNodeMapImpl( docPtr() );
        m_notations->ref();
    }
    return m_notations;
}

#include "dom_docimpl.moc"
