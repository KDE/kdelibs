/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Peter Kelly (pmk@post.com)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
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
 */

//#define EVENT_DEBUG
#include "dom/dom_exception.h"
#include "dom/dom_node.h"
#include "xml/dom_textimpl.h"
#include "xml/dom_docimpl.h"
#include "xml/dom2_eventsimpl.h"
#include "xml/dom_elementimpl.h"

#include "html/dtd.h"
#include "html/htmlparser.h"

#include "rendering/render_root.h"
#include "misc/htmlhashes.h"
#include "css/css_valueimpl.h"
#include "css/css_stylesheetimpl.h"
#include "css/cssstyleselector.h"
#include "xml/dom_xmlimpl.h"

#include <qtextstream.h>
#include <kdebug.h>
#include <stdlib.h>

// ### support default attributes
// ### dispatch mutation events
// ### check for INVALID_CHARACTER_ERR where appropriate

using namespace DOM;
using namespace khtml;

namespace DOM {
    DOMString extractPrefix(DOMStringImpl *qualifiedName);
    DOMString extractLocalName(DOMStringImpl *qualifiedName);
};

DOMString DOM::extractPrefix(DOMStringImpl *qualifiedName)
{
    DOMString prefix;
    int colonpos = -1;
    for (uint i = 0; i < qualifiedName->l && colonpos < 0; ++i)
        if (qualifiedName->s[i] == ':')
            colonpos = i;

    if (colonpos >= 0) {
        prefix = qualifiedName->copy();
        prefix.implementation()->truncate(colonpos);
    }
    return prefix;
}

DOMString DOM::extractLocalName(DOMStringImpl *qualifiedName)
{
    DOMString localName;
    int colonpos = -1;
    for (uint i = 0; i < qualifiedName->l && colonpos < 0; ++i)
        if (qualifiedName->s[i] == ':')
            colonpos = i;

    if (colonpos >= 0) {
        localName = qualifiedName->copy();
        localName.implementation()->remove(0,colonpos+1);
    }
    else {
        localName = qualifiedName;
    }
    return localName;
}

AttrImpl::AttrImpl(ElementImpl* element, DocumentPtr* docPtr, NodeImpl::Id attrId,
		   DOMStringImpl *_value)
    : NodeBaseImpl(docPtr),
      m_element(element)
{
    m_attrId = attrId;
    m_value = _value;
    m_value->ref();

    m_prefix = 0;
    m_namespaceURI = 0;
    m_localName = 0;
    m_specified = true; // we don't yet support default attributes
}

AttrImpl::AttrImpl(ElementImpl* element, DocumentPtr* docPtr, DOMStringImpl *_namespaceURI,
		   DOMStringImpl *_qualifiedName, DOMStringImpl *_value)
    : NodeBaseImpl(docPtr),
      m_element(element)
{
    m_attrId = docPtr->document()->attrNames()->getId(_qualifiedName,false);
    m_value = _value;
    m_value->ref();
    m_namespaceURI = _namespaceURI;
    if (m_namespaceURI)
	m_namespaceURI->ref();

    DOMString prefix = extractPrefix(_qualifiedName);
    m_prefix = prefix.implementation();
    if (m_prefix)
	m_prefix->ref();
    DOMString localName = extractLocalName(_qualifiedName);
    m_localName = localName.implementation();
    if (m_localName)
	m_localName->ref();
    m_specified = true; // we don't yet support default attributes
}

AttrImpl::~AttrImpl()
{
    m_value->deref();
    if (m_prefix)
	m_prefix->deref();
    if (m_namespaceURI)
	m_namespaceURI->deref();
    if (m_localName)
	m_localName->deref();
}

DOMString AttrImpl::nodeName() const
{
    return getDocument()->attrNames()->getName(m_attrId);
}

unsigned short AttrImpl::nodeType() const
{
    return Node::ATTRIBUTE_NODE;
}

DOMString AttrImpl::prefix() const
{
    return m_prefix;
}

void AttrImpl::setPrefix(const DOMString &_prefix, int &exceptioncode )
{
    checkSetPrefix(_prefix, exceptioncode);
    if (exceptioncode)
        return;

    if (m_prefix == _prefix.implementation())
	return;

    if (m_prefix)
	m_prefix->deref();
    m_prefix = _prefix.implementation();
    if (m_prefix)
	m_prefix->ref();
}

DOMString AttrImpl::namespaceURI() const
{
    return m_namespaceURI;
}

DOMString AttrImpl::localName() const
{
    return m_localName;
}

DOMString AttrImpl::nodeValue() const
{
    return m_value;
}

DOMString AttrImpl::name() const
{
    return getDocument()->attrNames()->getName(m_attrId);
}

void AttrImpl::setValue( const DOMString &v, int &exceptioncode )
{
    exceptioncode = 0;

    // ### according to the DOM docs, we should create an unparsed Text child
    // node here
    // do not interprete entities in the string, its literal!

    // NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return;
    }

    // ### what to do on 0 ?
    if (v.isNull()) {
        exceptioncode = DOMException::DOMSTRING_SIZE_ERR;
        return;
    }

    if (m_value == v.implementation())
	return;

    m_value->deref();
    m_value = v.implementation();
    m_value->ref();

    if (m_element)
        m_element->parseAttribute(m_attrId,m_value);
}

void AttrImpl::setNodeValue( const DOMString &v, int &exceptioncode )
{
    exceptioncode = 0;
    // NO_MODIFICATION_ALLOWED_ERR: taken care of by setValue()
    setValue(v, exceptioncode);
}

NodeImpl *AttrImpl::cloneNode ( bool /*deep*/)
{
    if (m_localName) // namespace support
	return new AttrImpl(0,docPtr(),m_namespaceURI,
			    getDocument()->attrNames()->getName(m_attrId).implementation(),m_value);
    else
	return new AttrImpl(0,docPtr(),m_attrId,m_value);
}

// DOM Section 1.1.1
bool AttrImpl::childAllowed( NodeImpl *newChild )
{
    if(!newChild)
        return false;

    return childTypeAllowed(newChild->nodeType());
}

bool AttrImpl::childTypeAllowed( unsigned short type )
{
    switch (type) {
        case Node::TEXT_NODE:
        case Node::ENTITY_REFERENCE_NODE:
            return true;
            break;
        default:
            return false;
    }
}

// -------------------------------------------------------------------------

void AttrImpl::setElement(ElementImpl *element)
{
    m_element = element;
}

void AttributeImpl::setValue(DOMStringImpl *value, ElementImpl *element)
{
    assert(value);
    if (m_attrId) {
	if (m_data.value == value)
	    return;
	m_data.value->deref();
	m_data.value = value;
	m_data.value->ref();
	if (element)
	    element->parseAttribute(this);
    }
    else {
	int exceptioncode = 0;
	m_data.attr->setValue(value,exceptioncode);
	// AttrImpl::setValue() calls parseAttribute()
    }
}

AttrImpl *AttributeImpl::createAttr(ElementImpl *element, DocumentPtr *docPtr)
{
    if (m_attrId) {
	AttrImpl *attr = new AttrImpl(element,docPtr,m_attrId,m_data.value);
	m_data.value->deref();
	m_data.attr = attr;
	m_data.attr->ref();
	m_attrId = 0;
    }

    return m_data.attr;
}

void AttributeImpl::free()
{
    if (m_attrId) {
	m_data.value->deref();
    }
    else {
	m_data.attr->deref();
	m_data.attr->setElement(0);
    }
}

// -------------------------------------------------------------------------

ElementImpl::ElementImpl(DocumentPtr *doc)
    : NodeBaseImpl(doc)
{
    namedAttrMap = 0;
    m_styleDecls = 0;
    m_prefix = 0;
    m_namespaceURI = 0;
}

ElementImpl::~ElementImpl()
{
    if(namedAttrMap) {
        namedAttrMap->detachFromElement();
        namedAttrMap->deref();
    }

    if (m_styleDecls) {
        m_styleDecls->setNode(0);
        m_styleDecls->setParent(0);
        m_styleDecls->deref();
    }

    if (m_prefix)
        m_prefix->deref();
}

unsigned short ElementImpl::nodeType() const
{
    return Node::ELEMENT_NODE;
}

DOMString ElementImpl::getAttribute( NodeImpl::Id id, DOMStringImpl *namespaceURI,
				     DOMStringImpl *qualifiedName ) const
{
    if (!namedAttrMap)
	return DOMString();

    DOMStringImpl *value = namedAttrMap->getValue(id,namespaceURI,qualifiedName);
    if (value)
	return value;

    // then search in default attr in case it is not yet set
    NamedAttrMapImpl* dm = defaultMap();
    value = dm ? dm->getValue(id,namespaceURI,qualifiedName) : 0;
    if (value)
	return value;

    return DOMString();
}

void ElementImpl::setAttribute( NodeImpl::Id id, DOMStringImpl *namespaceURI, DOMStringImpl *qualifiedName,
				DOMStringImpl* value, int &exceptioncode )
{
    // NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return;
    }

    attributes()->setValue(id,namespaceURI,qualifiedName,value);
}

void ElementImpl::setAttribute(NodeImpl::Id id, const DOMString &value)
{
    int exceptioncode = 0;
    setAttribute(id,0,0,value.implementation(),exceptioncode);
}

void ElementImpl::setAttributeMap( NamedAttrMapImpl* list )
{
    if (namedAttrMap) {
	namedAttrMap->detachFromElement();
        namedAttrMap->deref();
    }

    namedAttrMap = list;

    if (namedAttrMap) {
        namedAttrMap->ref();
        assert(namedAttrMap->m_element == 0);
        namedAttrMap->setElement(this);
        unsigned long len = namedAttrMap->length();
        for (unsigned long i = 0; i < len; i++)
            parseAttribute(&namedAttrMap->m_attrs[i]);
    }
}

NodeImpl *ElementImpl::cloneNode(bool deep)
{
    // ### we loose the namespace here ... FIXME
    ElementImpl *clone = getDocument()->createElement(tagName());
    if (!clone) return 0;

    // clone attributes
    if (namedAttrMap)
	clone->attributes()->copyAttributes(namedAttrMap);

    // clone individual style rules
    if (m_styleDecls)
        *(clone->styleRules()) = *m_styleDecls;

    if (deep)
        cloneChildNodes(clone);

    return clone;
}

DOMString ElementImpl::nodeName() const
{
    return tagName();
}

DOMString ElementImpl::prefix() const
{
    return m_prefix;
}

void ElementImpl::setPrefix( const DOMString &_prefix, int &exceptioncode )
{
    checkSetPrefix(_prefix, exceptioncode);
    if (exceptioncode)
        return;

    if (m_prefix == _prefix.implementation())
	return;

    if (m_prefix)
        m_prefix->deref();
    m_prefix = _prefix.implementation();
    if (m_prefix)
        m_prefix->ref();
}

DOMString ElementImpl::namespaceURI() const
{
    return m_namespaceURI;
}

void ElementImpl::setNamespaceURI(const DOMString &_namespaceURI)
{
    if (m_namespaceURI == _namespaceURI.implementation())
	return;

    if (m_namespaceURI)
	m_namespaceURI->deref();
    m_namespaceURI = _namespaceURI.implementation();
    if (m_namespaceURI)
	m_namespaceURI->ref();
}

void ElementImpl::createAttributeMap() const
{
    namedAttrMap = new NamedAttrMapImpl(const_cast<ElementImpl*>(this));
    namedAttrMap->ref();
}

NamedAttrMapImpl* ElementImpl::defaultMap() const
{
    return 0;
}

void ElementImpl::attach()
{
    assert(!attached());
    assert(!m_render);
    assert(parentNode());

#if SPEED_DEBUG < 1
    if (parentNode()->renderer()) {
        RenderStyle* _style = getDocument()->styleSelector()->styleForElement(this);
        _style->ref();
        m_render = RenderObject::createObject(this, _style);
        if(m_render)
            parentNode()->renderer()->addChild(m_render, nextRenderer());
        _style->deref();
    }
#endif

    NodeBaseImpl::attach();
}

void ElementImpl::recalcStyle( StyleChange change )
{
    // ### should go away and be done in renderobject
    RenderStyle* _style = m_render ? m_render->style() : 0;
#if 0
    const char* debug;
    switch(change) {
    case NoChange: debug = "NoChange";
        break;
    case NoInherit: debug= "NoInherit";
        break;
    case Inherit: debug = "Inherit";
        break;
    case Force: debug = "Force";
        break;
    }
    qDebug("recalcStyle(%d: %s, changed: %d)[%p: %s]", change, debug, changed(), this, tagName().string().latin1());
#endif
    if ( change >= Inherit || changed() ) {
        EDisplay oldDisplay = _style ? _style->display() : NONE;
        EPosition oldPosition = _style ? _style->position() : STATIC;

        RenderStyle *newStyle = getDocument()->styleSelector()->styleForElement(this);
        newStyle->ref();
        StyleChange ch = diff( _style, newStyle );
        if ( ch != NoChange ) {
            if (oldDisplay != newStyle->display()
                || oldPosition != newStyle->position()) {
                if (attached()) detach();
                // ### uuhm, suboptimal. style gets calculated again
                attach();
		// attach recalulates the style for all children. No need to do it twice.
		setChanged( false );
		setHasChangedChild( false );
		newStyle->deref();
		return;
            }
            if( m_render && newStyle ) {
                //qDebug("--> setting style on render element bgcolor=%s", newStyle->backgroundColor().name().latin1());
                m_render->setStyle(newStyle);
            }
        }
        newStyle->deref();

        if ( change != Force )
            change = ch;
    }

    NodeImpl *n;
    for (n = _first; n; n = n->nextSibling()) {
        if ( change >= Inherit || n->isTextNode() ||
             n->hasChangedChild() || n->changed() ) {
	    //qDebug("    (%p) calling recalcStyle on child %p/%s, change=%d", this, n, n->isElementNode() ? ((ElementImpl *)n)->tagName().string().latin1() : n->isTextNode() ? "text" : "unknown", change );
            n->recalcStyle( change );
        }
    }

    setChanged( false );
    setHasChangedChild( false );
}

bool ElementImpl::isSelectable() const
{
    return false;
}

// DOM Section 1.1.1
bool ElementImpl::childAllowed( NodeImpl *newChild )
{
    if (!childTypeAllowed(newChild->nodeType()))
        return false;

    // ### check xml element allowedness according to DTD

    // If either this node or the other node is an XML element node, allow regardless (we don't do DTD checks for XML
    // yet)
    if (isXMLElementNode() || newChild->isXMLElementNode())
	return true;
    else
	return checkChild(id(), newChild->id());
}

bool ElementImpl::childTypeAllowed( unsigned short type )
{
    switch (type) {
        case Node::ELEMENT_NODE:
        case Node::TEXT_NODE:
        case Node::COMMENT_NODE:
        case Node::PROCESSING_INSTRUCTION_NODE:
        case Node::CDATA_SECTION_NODE:
        case Node::ENTITY_REFERENCE_NODE:
            return true;
            break;
        default:
            return false;
    }
}

void ElementImpl::createDecl( )
{
    m_styleDecls = new CSSStyleDeclarationImpl(0);
    m_styleDecls->ref();
    m_styleDecls->setParent(getDocument()->elementSheet());
    m_styleDecls->setNode(this);
    m_styleDecls->setStrictParsing( getDocument()->parseMode() == DocumentImpl::Strict );
}

void ElementImpl::dispatchAttrRemovalEvent(NodeImpl::Id /*id*/, DOMStringImpl */*value*/)
{
    // ### enable this stuff again
    if (!getDocument()->hasListenerType(DocumentImpl::DOMATTRMODIFIED_LISTENER))
	return;
    //int exceptioncode = 0;
    //dispatchEvent(new MutationEventImpl(EventImpl::DOMATTRMODIFIED_EVENT,true,false,attr,attr->value(),
    //attr->value(), getDocument()->attrName(attr->id()),MutationEvent::REMOVAL),exceptioncode);
}

void ElementImpl::dispatchAttrAdditionEvent(NodeImpl::Id /*id*/, DOMStringImpl */*value*/)
{
    // ### enable this stuff again
    if (!getDocument()->hasListenerType(DocumentImpl::DOMATTRMODIFIED_LISTENER))
	return;
   //int exceptioncode = 0;
   //dispatchEvent(new MutationEventImpl(EventImpl::DOMATTRMODIFIED_EVENT,true,false,attr,attr->value(),
   //attr->value(),getDocument()->attrName(attr->id()),MutationEvent::ADDITION),exceptioncode);
}

// -------------------------------------------------------------------------

XMLElementImpl::XMLElementImpl(DocumentPtr *doc, DOMStringImpl *_tagName)
    : ElementImpl(doc)
{
    // Called from createElement(). In this case localName, prefix, and namespaceURI all need to be null.
    m_tagName = _tagName;
    m_tagName->ref();
    m_localName = 0;
}

XMLElementImpl::XMLElementImpl(DocumentPtr *doc, DOMStringImpl *_qualifiedName, DOMStringImpl *_namespaceURI)
    : ElementImpl(doc)
{
    // Called from createElementNS()
    m_tagName = _qualifiedName;
    m_tagName->ref();

    DOMString prefix = extractPrefix(_qualifiedName);
    m_prefix = prefix.implementation();
    if (m_prefix)
	m_prefix->ref();
    DOMString localName = extractLocalName(_qualifiedName);
    m_localName = localName.implementation();
    if (m_localName)
	m_localName->ref();

    m_namespaceURI = _namespaceURI;
    if (m_namespaceURI)
	m_namespaceURI->ref();
}

XMLElementImpl::~XMLElementImpl()
{
}

DOMString XMLElementImpl::tagName() const
{
    return m_tagName;
}

DOMString XMLElementImpl::localName() const
{
    return m_localName;
}

NodeImpl *XMLElementImpl::cloneNode ( bool deep )
{
    XMLElementImpl *clone;
    if (!m_localName)
	clone = new XMLElementImpl(docPtr(),m_tagName);
    else
	clone = new XMLElementImpl(docPtr(),m_tagName,m_namespaceURI);

    // clone attributes
    if (namedAttrMap)
	clone->attributes()->copyAttributes(namedAttrMap);

    // clone individual style rules
    if (m_styleDecls)
        *(clone->styleRules()) = *m_styleDecls;

    if (deep)
        cloneChildNodes(clone);

    return clone;
}

// -------------------------------------------------------------------------

NamedAttrMapImpl::NamedAttrMapImpl(ElementImpl *element)
  : m_element(element),
    m_attrs(0),
    m_attrCount(0)
{
}

NamedAttrMapImpl::~NamedAttrMapImpl()
{
    for (unsigned long i = 0; i < m_attrCount; i++)
	m_attrs[i].free();
    free(m_attrs);
}

NodeImpl *NamedAttrMapImpl::getNamedItem ( NodeImpl::Id id, const DOMString &namespaceURI,
					   const DOMString &localName ) const
{
    if (!m_element)
	return 0;

    for (unsigned long i = 0; i < m_attrCount; i++) {
	// For getNamedItem(), just compare the id (nodeName). For getNamedItemNS(), we need to compare
	// the namespace and local name, as there may be multiple attributes with the same nodeName.
	if ((id != 0 && m_attrs[i].id() == id) ||
	    (id == 0 && m_attrs[i].namespaceURI() == namespaceURI && m_attrs[i].localName() == localName))
	    return m_attrs[i].createAttr(m_element,m_element->docPtr());
    }

    return 0;
}

Node NamedAttrMapImpl::removeNamedItem ( NodeImpl::Id id, const DOMString &namespaceURI,
					 const DOMString &localName, int &exceptioncode )
{
    if (!m_element) {
        exceptioncode = DOMException::NOT_FOUND_ERR;
        return 0;
    }

    // NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return 0;
    }

    for (unsigned long i = 0; i < m_attrCount; i++) {
	// For removeNamedItem(), just compare the id (nodeName). For removeNamedItemNS(), we need to compare
	// the namespace and local name, as there may be multiple attributes with the same nodeName.
	if ((id != 0 && m_attrs[i].id() == id) ||
	    (id == 0 && m_attrs[i].namespaceURI() == namespaceURI && m_attrs[i].localName() == localName)) {
	    id = m_attrs[i].id();
	    Node removed(m_attrs[i].createAttr(m_element,m_element->docPtr()));
	    m_attrs[i].free();
	    memmove(m_attrs+i,m_attrs+i+1,(m_attrCount-i-1)*sizeof(AttributeImpl));
	    m_attrCount--;
	    m_attrs = (AttributeImpl*)realloc(m_attrs,m_attrCount*sizeof(AttributeImpl));
	    m_element->parseAttribute(id,0);
	    return removed;
	}
    }

    // NOT_FOUND_ERR: Raised if there is no node with the specified namespaceURI
    // and localName in this map.
    exceptioncode = DOMException::NOT_FOUND_ERR;
    return 0;
}

Node NamedAttrMapImpl::setNamedItem ( NodeImpl* arg, bool ns, int &exceptioncode )
{
    if (!m_element) {
        exceptioncode = DOMException::NOT_FOUND_ERR;
        return 0;
    }

    // NO_MODIFICATION_ALLOWED_ERR: Raised if this map is readonly.
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return 0;
    }

    // WRONG_DOCUMENT_ERR: Raised if arg was created from a different document than the one that created this map.
    if (arg->getDocument() != m_element->getDocument()) {
        exceptioncode = DOMException::WRONG_DOCUMENT_ERR;
        return 0;
    }

    // HIERARCHY_REQUEST_ERR: Raised if an attempt is made to add a node doesn't belong in this NamedNodeMap
    if (!arg->isAttributeNode()) {
        exceptioncode = DOMException::HIERARCHY_REQUEST_ERR;
        return 0;
    }
    AttrImpl *attr = static_cast<AttrImpl*>(arg);

    // INUSE_ATTRIBUTE_ERR: Raised if arg is an Attr that is already an attribute of another Element object.
    // The DOM user must explicitly clone Attr nodes to re-use them in other elements.
    if (attr->ownerElement() && attr->ownerElement() != m_element) {
        exceptioncode = DOMException::INUSE_ATTRIBUTE_ERR;
        return 0;
    }

    if (attr->ownerElement() == m_element) {
	// Already have this attribute. Since we're not "replacing" it, return null.
	return 0;
    }

    for (unsigned long i = 0; i < m_attrCount; i++) {
	// For setNamedItem(), just compare the id (nodeName). For setNamedItemNS(), we need to compare
	// the namespace and local name, as there may be multiple attributes with the same nodeName.
	if ((!ns && m_attrs[i].id() == attr->attrId()) ||
	    (ns && m_attrs[i].namespaceURI() == attr->namespaceURI() &&
	     m_attrs[i].localName() == attr->localName())) {
	    // Attribute exists; replace it
	    Node replaced = m_attrs[i].createAttr(m_element,m_element->docPtr());
	    m_attrs[i].free();
	    m_attrs[i].m_attrId = 0;
	    m_attrs[i].m_data.attr = attr;
	    m_attrs[i].m_data.attr->ref();
	    attr->setElement(m_element);
	    m_element->parseAttribute(&m_attrs[i]);
	    // ### dispatch mutation events
	    return replaced;
	}
    }

    // No existing attribute; add to list
    m_attrCount++;
    m_attrs = (AttributeImpl*)realloc(m_attrs,m_attrCount*sizeof(AttributeImpl));
    m_attrs[m_attrCount-1].m_attrId = 0;
    m_attrs[m_attrCount-1].m_data.attr = attr;
    m_attrs[m_attrCount-1].m_data.attr->ref();
    attr->setElement(m_element);
    m_element->parseAttribute(&m_attrs[m_attrCount-1]);
    // ### dispatch mutation events

    return 0;
}

NodeImpl *NamedAttrMapImpl::item ( unsigned long index ) const
{
    if (!m_element)
	return 0;

    if (index >= m_attrCount)
	return 0;
    else
	return m_attrs[index].createAttr(m_element,m_element->docPtr());
}

unsigned long NamedAttrMapImpl::length(  ) const
{
    if (!m_element)
	return 0;

    return m_attrCount;
}

NodeImpl::Id NamedAttrMapImpl::idAt(unsigned long index) const
{
    assert(index <= m_attrCount);
    return m_attrs[index].id();
}

DOMStringImpl *NamedAttrMapImpl::valueAt(unsigned long index) const
{
    assert(index <= m_attrCount);
    return m_attrs[index].val();
}

DOMStringImpl *NamedAttrMapImpl::getValue(NodeImpl::Id id, DOMStringImpl *namespaceURI,
					  DOMStringImpl *localName) const
{
    DOMString nsURI(namespaceURI);
    DOMString lName(localName);
    for (unsigned long i = 0; i < m_attrCount; i++)
	if ((id != 0 && m_attrs[i].id() == id) ||
	    (id == 0 && m_attrs[i].namespaceURI() == nsURI && m_attrs[i].localName() == lName))
	    return m_attrs[i].val();

    return 0;
}

void NamedAttrMapImpl::setValue(NodeImpl::Id id, DOMStringImpl *namespaceURI,
				DOMStringImpl *qualifiedName, DOMStringImpl *value)
{
    // Passing in a null value here causes the attribute to be removed. This is a khtml extension
    // (the spec does not specify what to do in this situation).
    int exceptioncode = 0;
    if (!value) {
	DOMString localName = (id ? DOMString() : extractLocalName(qualifiedName));
	removeNamedItem(id,namespaceURI,localName,exceptioncode);
	return;
    }

    DOMString prefix;
    DOMString localName;
    if (id == 0) {
	prefix = extractPrefix(qualifiedName);
	localName = extractLocalName(qualifiedName);
    }

    // Check for an existing attribute. If called from setAttribute(), we only compare the ids (i.e. nodeName)
    // If called from setAttributeNS(), we need to compare both the namespace URI and the local name (there
    // may be multiple attributes with the same nodeName).
    DOMString nsURI(namespaceURI);
    for (unsigned long i = 0; i < m_attrCount; i++) {
	if ((id != 0 && m_attrs[i].id() == id) ||
	    (id == 0 && m_attrs[i].namespaceURI() == nsURI && m_attrs[i].localName() == localName)) {

	    if (id == 0)
		m_attrs[i].attr()->setPrefix(prefix,exceptioncode);

	    m_attrs[i].setValue(value,m_element);
	    // ### dispatch mutation events
	    return;
	}
    }

    // No existing matching attribute; add a new one
    m_attrCount++;
    m_attrs = (AttributeImpl*)realloc(m_attrs,m_attrCount*sizeof(AttributeImpl));
    m_attrs[m_attrCount-1].m_attrId = id;
    if (id) {
	// Called from setAttribute()... we only have a name
	m_attrs[m_attrCount-1].m_data.value = value;
	m_attrs[m_attrCount-1].m_data.value->ref();
    }
    else {
	// Called from setAttributeNS()... need to create a full AttrImpl here to store the
	// namespaceURI, prefix and localName
	m_attrs[m_attrCount-1].m_data.attr = new AttrImpl(m_element,m_element->docPtr(),
							  namespaceURI,
							  qualifiedName,
							  value);
	m_attrs[m_attrCount-1].m_data.attr->ref();
    }
    if (m_element)
	m_element->parseAttribute(&m_attrs[m_attrCount-1]);
    // ### dispatch mutation events
}

Attr NamedAttrMapImpl::removeAttr(AttrImpl *attr)
{
    for (unsigned long i = 0; i < m_attrCount; i++) {
	if (m_attrs[i].attr() == attr) {
	    NodeImpl::Id id = m_attrs[i].id();
	    Node removed(m_attrs[i].createAttr(m_element,m_element->docPtr()));
	    m_attrs[i].free();
	    memmove(m_attrs+i,m_attrs+i+1,(m_attrCount-i-1)*sizeof(AttributeImpl));
	    m_attrCount--;
	    m_attrs = (AttributeImpl*)realloc(m_attrs,m_attrCount*sizeof(AttributeImpl));
	    m_element->parseAttribute(id,0);
	    // ### dispatch mutation events
	    return removed;
	}
    }

    return 0;
}

NodeImpl::Id NamedAttrMapImpl::mapId(const DOMString& name, bool readonly)
{
    if (!m_element)
	return 0;

    return m_element->getDocument()->attrNames()->getId(name.implementation(), readonly);
}

void NamedAttrMapImpl::copyAttributes(NamedAttrMapImpl *other)
{
    assert(m_element);
    unsigned long i;
    for (i = 0; i < m_attrCount; i++)
	m_attrs[i].free();
    m_attrCount = other->m_attrCount;
    m_attrs = (AttributeImpl*)realloc(m_attrs,m_attrCount*sizeof(AttributeImpl));
    for (i = 0; i < m_attrCount; i++) {
	m_attrs[i].m_attrId = other->m_attrs[i].m_attrId;
	if (m_attrs[i].m_attrId) {
	    m_attrs[i].m_data.value = other->m_attrs[i].m_data.value;
	    m_attrs[i].m_data.value->ref();
	}
	else {
	    m_attrs[i].m_data.attr = static_cast<AttrImpl*>(other->m_attrs[i].m_data.attr->cloneNode(true));
	    m_attrs[i].m_data.attr->ref();
	    m_attrs[i].m_data.attr->setElement(m_element);
	}
    }
}

void NamedAttrMapImpl::setElement(ElementImpl *element)
{
    assert(!m_element);
    m_element = element;

    for (unsigned long i = 0; i < m_attrCount; i++)
	if (m_attrs[i].attr())
	    m_attrs[i].attr()->setElement(element);
}

void NamedAttrMapImpl::detachFromElement()
{
    // This makes the map invalid; nothing can really be done with it now since it's not
    // associated with an element. But we have to keep it around in memory in case there
    // are still references to it.
    m_element = 0;
    for (unsigned long i = 0; i < m_attrCount; i++)
	m_attrs[i].free();
    free(m_attrs);
    m_attrs = 0;
    m_attrCount = 0;
}
