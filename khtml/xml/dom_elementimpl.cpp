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

using namespace DOM;
using namespace khtml;

// ### support prefixes on Attrs

AttrImpl::AttrImpl(ElementImpl* element, DocumentPtr* docPtr, NodeImpl::Id attrId,
		   DOMStringImpl *value, DOMStringImpl *prefix)
    : NodeBaseImpl(docPtr),
      m_element(element),
      m_attrId(attrId),
      m_value(value),
      m_prefix(prefix)
{
    assert(m_value);
    m_value->ref();
    if (m_prefix)
	m_prefix->ref();
}

AttrImpl::~AttrImpl()
{
    m_value->deref();
    if (m_prefix)
	m_prefix->deref();
}

DOMString AttrImpl::nodeName() const
{
    return getDocument()->attrName(m_attrId);
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

    if (m_prefix)
	m_prefix->deref();
    m_prefix = _prefix.implementation();
    if (m_prefix)
	m_prefix->ref();
}

DOMString AttrImpl::nodeValue() const
{
    return m_value;
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

    m_value->deref();
    m_value = v.implementation();
    m_value->deref();

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
    return new AttrImpl(0,docPtr(),m_attrId,m_value,m_prefix);
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
	m_data.value->deref();
	m_data.value = value;
	m_data.value->ref();
	if (element)
	    element->parseAttribute(this);
    }
    else {
	int exceptioncode;
	m_data.attr->setValue(value,exceptioncode);
	// AttrImpl::setValue() calls parseAttribute()
    }
}

AttrImpl *AttributeImpl::createAttr(ElementImpl *element, DocumentPtr *docPtr)
{
    if (m_attrId) {
	AttrImpl *attr = new AttrImpl(element,docPtr,m_attrId,m_data.value,0);
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

void ElementImpl::removeAttribute( NodeImpl::Id id, int &exceptioncode )
{
    // NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return;
    }

    if (namedAttrMap)
        namedAttrMap->removeNamedItem(id, exceptioncode);
}

void ElementImpl::setAttribute(NodeImpl::Id id, const DOMString &value)
{
    int exceptioncode = 0;
    setAttribute(id,value.implementation(),exceptioncode);
}

unsigned short ElementImpl::nodeType() const
{
    return Node::ELEMENT_NODE;
}

DOMString ElementImpl::getAttribute(NodeImpl::Id id) const
{
    if (!namedAttrMap)
	return DOMString();

    DOMStringImpl *value = namedAttrMap->getValue(id);
    if (value)
	return value;

    // then search in default attr in case it is not yet set
    NamedAttrMapImpl* dm = defaultMap();
    value = dm ? dm->getValue(id) : 0;
    if (value)
	return value;

    return DOMString();
}

void ElementImpl::setAttribute(NodeImpl::Id id, DOMStringImpl* value, int &exceptioncode )
{
    // NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return;
    }

    attributes()->setValue(id,value);
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

DOMString ElementImpl::tagName() const
{
    DOMString tn = getDocument()->tagName(id());

    if (m_prefix)
        return DOMString(m_prefix) + ":" + tn;

    return tn;
}

void ElementImpl::setPrefix( const DOMString &_prefix, int &exceptioncode )
{
    checkSetPrefix(_prefix, exceptioncode);
    if (exceptioncode)
        return;

    if (m_prefix)
        m_prefix->deref();
    m_prefix = _prefix.implementation();
    if (m_prefix)
        m_prefix->ref();
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

        int dynamicState = StyleSelector::None;
        if ( m_render && m_render->mouseInside() )
            dynamicState |= StyleSelector::Hover;
        if ( m_focused )
            dynamicState |= StyleSelector::Focus;
        if ( m_active )
            dynamicState |= StyleSelector::Active;

        RenderStyle *newStyle = getDocument()->styleSelector()->styleForElement(this, dynamicState);
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
    m_id = doc->document()->tagId(0 /* no namespace */, _tagName,  false /* allocate */);
}

XMLElementImpl::XMLElementImpl(DocumentPtr *doc, DOMStringImpl *_qualifiedName, DOMStringImpl *_namespaceURI)
    : ElementImpl(doc)
{
    int colonpos = -1;
    for (uint i = 0; i < _qualifiedName->l; ++i)
        if (_qualifiedName->s[i] == ':') {
            colonpos = i;
            break;
        }

    if (colonpos >= 0) {
        // we have a prefix
        DOMStringImpl* localName = _qualifiedName->copy();
        localName->ref();
        localName->remove(0,colonpos+1);
        m_id = doc->document()->tagId(_namespaceURI, localName, false /* allocate */);
        localName->deref();
        m_prefix = _qualifiedName->copy();
        m_prefix->ref();
        m_prefix->truncate(colonpos);
    }
    else {
        // no prefix
        m_id = doc->document()->tagId(_namespaceURI, _qualifiedName, false /* allocate */);
        m_prefix = 0;
    }
}

XMLElementImpl::~XMLElementImpl()
{
}

DOMString XMLElementImpl::localName() const
{
    return getDocument()->tagName(m_id);
}


NodeImpl *XMLElementImpl::cloneNode ( bool deep )
{
    // ### we loose namespace here FIXME
    // should pass id around
    XMLElementImpl *clone = new XMLElementImpl(docPtr(), getDocument()->tagName(m_id).implementation());
    clone->m_id = m_id;

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

NodeImpl *NamedAttrMapImpl::getNamedItem ( NodeImpl::Id id ) const
{
    // ### add useNamespace parameter
    if (!m_element)
	return 0;

    for (unsigned long i = 0; i < m_attrCount; i++)
	if (m_attrs[i].id() == id)
	    return m_attrs[i].createAttr(m_element,m_element->docPtr());

    return 0;
}

Node NamedAttrMapImpl::removeNamedItem ( NodeImpl::Id id, int &exceptioncode )
{
    // ### add useNamespace parameter
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
	if (m_attrs[i].id() == id) {
	    Node removed(m_attrs[i].createAttr(m_element,m_element->docPtr()));
	    m_attrs[i].free();
	    memmove(m_attrs+i,m_attrs+i+1,m_attrCount-i-1*sizeof(AttributeImpl));
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

Node NamedAttrMapImpl::setNamedItem ( NodeImpl* arg, int &exceptioncode )
{
    // ### add useNamespace parameter
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
	// Already have this attribute
	return attr;
    }

    for (unsigned long i = 0; i < m_attrCount; i++) {
	if (m_attrs[i].id() == attr->attrId()) {
	    // Attribute exists; replace it
	    Node replaced = m_attrs[i].createAttr(m_element,m_element->docPtr());
	    m_attrs[i].free();
	    m_attrs[i].m_attrId = attr->attrId();
	    m_attrs[i].m_data.attr = attr;
	    m_attrs[i].m_data.attr->ref();
	    m_element->parseAttribute(&m_attrs[i]);
	    attr->setElement(m_element);
	    // ### dispatch mutation events
	    return replaced;
	}
    }

    // No existing attribute; add to list
    m_attrCount++;
    m_attrs = (AttributeImpl*)realloc(m_attrs,m_attrCount*sizeof(AttributeImpl));
    m_attrs[m_attrCount-1].m_attrId = attr->attrId();
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

DOMStringImpl *NamedAttrMapImpl::getValue(NodeImpl::Id id) const
{
    for (unsigned long i = 0; i < m_attrCount; i++)
	if (m_attrs[i].id() == id)
	    return m_attrs[i].val();

    return 0;
}

void NamedAttrMapImpl::setValue(NodeImpl::Id id, DOMStringImpl *value)
{
    for (unsigned long i = 0; i < m_attrCount; i++) {
	if (m_attrs[i].id() == id) {
	    if (value) {
		m_attrs[i].setValue(value,m_element);
	    }
	    else {
		m_attrs[i].free();
		memmove(m_attrs+i,m_attrs+i+1,(m_attrCount-i-1)*sizeof(AttributeImpl));
		m_attrCount--;
		m_attrs = (AttributeImpl*)realloc(m_attrs,m_attrCount*sizeof(AttributeImpl));
		if (m_element)
		    m_element->parseAttribute(id,0);
	    }
	    // ### dispatch mutation events
	    return;
	}
    }

    if (!value)
	return;

    // No existing attribute; add to list
    m_attrCount++;
    m_attrs = (AttributeImpl*)realloc(m_attrs,m_attrCount*sizeof(AttributeImpl));
    m_attrs[m_attrCount-1].m_attrId = id;
    m_attrs[m_attrCount-1].m_data.value = value;
    m_attrs[m_attrCount-1].m_data.value->ref();
    if (m_element)
	m_element->parseAttribute(&m_attrs[m_attrCount-1]);
    // ### dispatch mutation events
}

NodeImpl::Id NamedAttrMapImpl::mapId(const DOMString& namespaceURI,  const DOMString& localName,  bool readonly)
{
    if (!m_element)
	return 0;

    return m_element->getDocument()->attrId(namespaceURI.implementation(),
					    localName.implementation(), readonly);
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
