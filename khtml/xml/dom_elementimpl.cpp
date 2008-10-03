/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Peter Kelly (pmk@post.com)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
  *          (C) 2004, 2005, 2006, 2007 Apple Inc. All rights reserved.
 *           (C) 2005, 2008 Maksim Orlovich (maksim@kde.org)  
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

//#define EVENT_DEBUG

#include "dom_elementimpl.h"

#include <dom/dom_exception.h>
#include <dom/dom_node.h>
#include <dom/html_image.h>
#include "dom_textimpl.h"
#include "dom_docimpl.h"
#include "dom2_eventsimpl.h"
#include "dom_restyler.h"
#include "dom_xmlimpl.h"

#include <html/dtd.h>
#include <html/htmlparser.h>
#include <html/html_imageimpl.h>

#include <rendering/render_canvas.h>
#include <misc/htmlhashes.h>
#include <css/css_valueimpl.h>
#include <css/css_stylesheetimpl.h>
#include <css/cssstyleselector.h>
#include <css/cssvalues.h>
#include <css/cssproperties.h>
#include <khtml_part.h>

#include <QtCore/QTextIStream>
#include <QTextDocument>
#include <kdebug.h>
#include <stdlib.h>

#include <wtf/HashMap.h>

// ### support default attributes
// ### dispatch mutation events
// ### check for INVALID_CHARACTER_ERR where appropriate

using namespace khtml;

namespace DOM {

AttrImpl::AttrImpl(ElementImpl* element, DocumentImpl* docPtr, NodeImpl::Id attrId,
		   DOMStringImpl *value, DOMStringImpl *prefix)
    : NodeBaseImpl(docPtr),
      m_attrId(attrId)
{
    m_value = value;
    m_value->ref();

    m_prefix = prefix;
    if (m_prefix)
	m_prefix->ref();

    // When creating the text node initially, we want element = 0,
    // so we don't attempt to update the getElementById cache or
    // call parseAttribute, etc. This is because we're normally lazily,
    // from previous attributes, so there is nothing really changing
    m_element = 0; 
    createTextChild();
    m_element = element;
}

AttrImpl::~AttrImpl()
{
    m_value->deref();
    if (m_prefix)
	m_prefix->deref();
}

DOMString AttrImpl::nodeName() const
{
    return name();
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
    if (m_htmlCompat)
        return DOMString();
    return document()->getName(NamespaceId, m_attrId >> 16);
}

DOMString AttrImpl::localName() const
{
    if (m_htmlCompat)
       return DOMString();
    return document()->getName(AttributeId, m_attrId);
}

DOMString AttrImpl::nodeValue() const
{
    return m_value;
}

DOMString AttrImpl::name() const
{
    DOMString n = document()->getName(AttributeId, m_attrId);

    // compat mode always return attribute names in lowercase.
    // that's not formally in the specification, but common
    // practice - a w3c erratum to DOM L2 is pending.
    if (m_htmlCompat)
        n = n.lower();

    if (m_prefix && m_prefix->l)
        return DOMString(m_prefix) + ":" + n;

    return n;
}

void AttrImpl::createTextChild()
{
    // add a text node containing the attribute value
    if (m_value->length() > 0) {
	TextImpl* textNode = ownerDocument()->createTextNode(m_value);
	int exceptioncode;

	// We want to use addChild and not appendChild here to avoid triggering
	// mutation events. childrenChanged() will still be called.
	addChild(textNode);
    }
}

void AttrImpl::childrenChanged()
{
    NodeBaseImpl::childrenChanged();

    // update value
    DOMStringImpl* oldVal = m_value;
    m_value = new DOMStringImpl(0, 0);
    m_value->ref();
    for (NodeImpl* n = firstChild(); n; n = n->nextSibling()) {
	DOMStringImpl* data = static_cast<const TextImpl*>(n)->string();
	m_value->append(data);
    }

    if (m_element) {
	if (m_attrId == ATTR_ID)
	    m_element->updateId(oldVal, m_value);
	m_element->parseAttribute  (this);
	m_element->attributeChanged(m_attrId);
    }

    oldVal->deref();
}

void AttrImpl::setValue( const DOMString &v, int &exceptioncode )
{
    exceptioncode = 0;

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

    int e = 0;
    removeChildren();
    appendChild(ownerDocument()->createTextNode(v.implementation()), e);
}

void AttrImpl::rewriteValue( const DOMString& newValue )
{
    int ec;

    // We want to avoid any notifications, so temporarily set m_element to 0
    ElementImpl* saveElement = m_element;
    m_element = 0;
    setValue(newValue, ec);
    m_element = saveElement;
}

void AttrImpl::setNodeValue( const DOMString &v, int &exceptioncode )
{
    exceptioncode = 0;
    // NO_MODIFICATION_ALLOWED_ERR: taken care of by setValue()
    setValue(v, exceptioncode);
}

WTF::PassRefPtr<NodeImpl> AttrImpl::cloneNode ( bool /*deep*/)
{
     AttrImpl* attr = new AttrImpl(0, docPtr(), m_attrId, m_value, m_prefix);
     attr->setHTMLCompat(m_htmlCompat);
     return attr;
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

DOMString AttrImpl::toString() const
{
    DOMString result;

    result += nodeName();

    // FIXME: substitute entities for any instances of " or ' --
    // maybe easier to just use text value and ignore existing
    // entity refs?

    if ( !nodeValue().isEmpty() ) {
        //remove the else once the AttributeImpl changes are merged
        result += "=\"";
        result += nodeValue();
        result += "\"";
    }

    return result;
}

void AttrImpl::setElement(ElementImpl *element)
{
    m_element = element;
}

// -------------------------------------------------------------------------

void AttributeImpl::setValue(DOMStringImpl *value, ElementImpl *element)
{
    assert(value);
    if (m_attrId) {
	if (m_data.value == value)
	    return;

	if (element && m_attrId == ATTR_ID)
	   element->updateId(m_data.value, value);

	m_data.value->deref();
	m_data.value = value;
	m_data.value->ref();

	if (element) {
	    element->parseAttribute(this);
	    element->attributeChanged(m_attrId);
        }
    }
    else {
	int exceptioncode = 0;
	m_data.attr->setValue(value,exceptioncode);
	// AttrImpl::setValue() calls parseAttribute()
    }
}

void AttributeImpl::rewriteValue( const DOMString& newValue )
{
    if (m_attrId) {
	DOMStringImpl* value = newValue.implementation();
	if (m_data.value == value)
	    return;

	m_data.value->deref();
	m_data.value = value;
	m_data.value->ref();
    }
    else {
	m_data.attr->rewriteValue(newValue);
    }
}

AttrImpl *AttributeImpl::createAttr(ElementImpl *element, DocumentImpl *docPtr)
{
    if (m_attrId) {
	AttrImpl *attr = new AttrImpl(element,docPtr,m_attrId,m_data.value);
        if (!attr) return 0;
        attr->setHTMLCompat( docPtr->htmlMode() != DocumentImpl::XHtml );
	m_data.value->deref();
	m_data.attr = attr;
	m_data.attr->ref();
	m_attrId = 0; /* "has implementation" flag */
    }

    return m_data.attr;
}

void AttributeImpl::free()
{
    if (m_attrId) {
	m_data.value->deref();
    }
    else {
	m_data.attr->setElement(0);
	m_data.attr->deref();
    }
}

// -------------------------------------------------------------------------

class ElementRareDataImpl {
public:
    ElementRareDataImpl();
    void resetComputedStyle();
    RenderStyle* m_computedStyle;
};

typedef WTF::HashMap<const ElementImpl*, ElementRareDataImpl*> ElementRareDataMap;

static ElementRareDataMap& rareDataMap()
{
    static ElementRareDataMap* dataMap = new ElementRareDataMap;
    return *dataMap;
}

static ElementRareDataImpl* rareDataFromMap(const ElementImpl* element)
{
    return rareDataMap().get(element);
}

inline ElementRareDataImpl::ElementRareDataImpl()
    : m_computedStyle(0)
{}

void ElementRareDataImpl::resetComputedStyle()
{
    if (!m_computedStyle)
        return;
    m_computedStyle->deref();
    m_computedStyle = 0;
}

// -------------------------------------------------------------------------

ElementImpl::ElementImpl(DocumentImpl *doc)
    : NodeBaseImpl(doc)
{
    namedAttrMap = 0;
    m_style.inlineDecls = 0;
    m_prefix = 0;
}

ElementImpl::~ElementImpl()
{
    if(namedAttrMap) {
        namedAttrMap->detachFromElement();
        namedAttrMap->deref();
    }

    if (m_style.inlineDecls) {
        if (CSSStyleDeclarationImpl * ild = inlineStyleDecls()) {
            // remove inline declarations
            ild->setNode(0);
            ild->setParent(0);
            ild->deref();
        }
        if (CSSStyleDeclarationImpl * ncd = nonCSSStyleDecls()) {
            // remove presentational declarations
            ncd->setNode(0);
            ncd->setParent(0);
            ncd->deref();
            delete m_style.combinedDecls;
        }
    }

    if (m_prefix)
        m_prefix->deref();

    if (!m_elementHasRareData) {
        ASSERT(!rareDataMap().contains(this));
    } else {
        ElementRareDataMap& dataMap = rareDataMap();
        ElementRareDataMap::iterator it = dataMap.find(this);
        ASSERT(it != dataMap.end());
        delete it->second;
        dataMap.remove(it);
    }
}

ElementRareDataImpl* ElementImpl::rareData()
{
    return m_elementHasRareData ? rareDataFromMap(this) : 0;
}

const ElementRareDataImpl* ElementImpl::rareData() const
{
    return m_elementHasRareData ? rareDataFromMap(this) : 0;
}

ElementRareDataImpl* ElementImpl::createRareData()
{
    if (m_elementHasRareData)
        return rareDataMap().get(this);
    ASSERT(!rareDataMap().contains(this));
    ElementRareDataImpl* data = new ElementRareDataImpl();
    rareDataMap().set(this, data);
    m_elementHasRareData = true;
    return data;
}

void ElementImpl::removeAttribute( NodeImpl::Id id, int &exceptioncode )
{
    if (namedAttrMap) {
        namedAttrMap->removeNamedItem(id, false, 0, exceptioncode);
        if (exceptioncode == DOMException::NOT_FOUND_ERR) {
            exceptioncode = 0;
        }
    }
}

unsigned short ElementImpl::nodeType() const
{
    return Node::ELEMENT_NODE;
}

DOMStringImpl* ElementImpl::getAttributeImpl( NodeImpl::Id id, bool nsAware, DOMStringImpl* qName) const
{
    if (!namedAttrMap)
	return 0;

    DOMStringImpl *value = namedAttrMap->getValue(id, nsAware, qName);
    if (value)
	return value;

    // then search in default attr in case it is not yet set
    NamedAttrMapImpl* dm = defaultMap();
    value = dm ? dm->getValue(id, nsAware, qName) : 0;
    if (value)
	return value;

    return 0;
}

DOMString ElementImpl::getAttribute( NodeImpl::Id id, bool nsAware, const DOMString& qName) const
{
    return DOMString(getAttributeImpl(id, nsAware, qName.implementation()));
}

void ElementImpl::setAttribute(NodeImpl::Id id, const DOMString &value, const DOMString& qName, int &exceptioncode)
{
    // NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return;
    }
    attributes()->setValue(id, value.implementation(), (qName.isEmpty() ? 0: qName.implementation()));
}

void ElementImpl::setAttributeNS( const DOMString &namespaceURI, const DOMString &qualifiedName,
				const DOMString &value, int &exceptioncode )
{
    // NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return;
    }
    int colonPos;
    if (!DOM::checkQualifiedName(qualifiedName, namespaceURI, &colonPos,
                                 false/*nameCanBeNull*/, false/*nameCanBeEmpty*/,
                                 &exceptioncode))
        return;
    DOMString prefix, localName;
    splitPrefixLocalName(qualifiedName.implementation(), prefix, localName, colonPos);
    NodeImpl::Id id = document()->getId(AttributeId, namespaceURI.implementation(),
                            prefix.implementation(), localName.implementation(), false, true /*lookupHTML*/);
    attributes()->setValue(id, value.implementation(), 0, prefix.implementation(),
                           true /*nsAware*/, !namespaceURI.isNull() /*hasNS*/);
}

void ElementImpl::setAttribute(NodeImpl::Id id, const DOMString &value)
{
    int exceptioncode = 0;
    setAttribute(id,value,DOMString(),exceptioncode);
}

void ElementImpl::setAttributeMap( NamedAttrMapImpl* list )
{
    // If setting the whole map changes the id attribute, we need to
    // call updateId.
    DOMStringImpl *oldId = namedAttrMap ? namedAttrMap->getValue(ATTR_ID) : 0;
    DOMStringImpl *newId = list ? list->getValue(ATTR_ID) : 0;

    if (oldId || newId) {
       updateId(oldId, newId);
    }

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
        for (unsigned long i = 0; i < len; i++) {
            parseAttribute(&namedAttrMap->m_attrs[i]);
            attributeChanged(namedAttrMap->m_attrs[i].id());
        }
    }
}

WTF::PassRefPtr<NodeImpl> ElementImpl::cloneNode(bool deep)
{
    WTF::RefPtr<ElementImpl> clone; // Make sure to guard...
    if ( !localName().isNull() )
        clone = document()->createElementNS( namespaceURI(), nodeName() );
    else
        clone = document()->createElement( nodeName() );
    if (!clone) return 0;
    finishCloneNode( clone.get(), deep );
    return clone;
}

void ElementImpl::finishCloneNode( ElementImpl* clone, bool deep )
{
    // clone attributes
    if (namedAttrMap)
	clone->attributes()->copyAttributes(namedAttrMap);

    // clone individual style rules
    if (m_style.inlineDecls) {
        if (m_hasCombinedStyle) {
            if (!clone->m_hasCombinedStyle)
                clone->createNonCSSDecl();
            if (m_style.combinedDecls->inlineDecls)
                *(clone->getInlineStyleDecls()) = *m_style.combinedDecls->inlineDecls;
            *clone->m_style.combinedDecls->nonCSSDecls = *m_style.combinedDecls->nonCSSDecls;
        } else {
            *(clone->getInlineStyleDecls()) = *m_style.inlineDecls;
        }
    }

    // ### fold above style cloning into this function?
    clone->copyNonAttributeProperties(this);

    if (deep)
        cloneChildNodes(clone);
}

bool ElementImpl::hasAttributes() const
{
    return namedAttrMap && namedAttrMap->length() > 0;
}

bool ElementImpl::hasAttribute( const DOMString& name ) const
{
    NodeImpl::Id id = document()->getId(NodeImpl::AttributeId, name.implementation(), true, true);
    if (!id) return false;
    if (!namedAttrMap) return false;
    return namedAttrMap->getValue(id, false, name.implementation()) != 0;
}

bool ElementImpl::hasAttributeNS( const DOMString &namespaceURI,
                                  const DOMString &localName ) const
{
    NodeImpl::Id id = document()->getId(NodeImpl::AttributeId,namespaceURI.implementation(),
						 0/*prefix*/, localName.implementation(), true, true);
    if (!id) return false;
    if (!namedAttrMap) return false;
    return namedAttrMap->getValue(id, true) != 0;
}

DOMString ElementImpl::getAttribute( const DOMString &name )
{
    NodeImpl::Id id = document()->getId(NodeImpl::AttributeId,name.implementation(),true,true);
    if (!id) return DOMString();

    return getAttribute(id, false, name);
}

void ElementImpl::setAttribute( const DOMString &name, const DOMString &value, int& exceptioncode )
{
    NodeImpl::Id id = document()->getId(NodeImpl::AttributeId, name.implementation(), false /* allocate */,
                                                 true, &exceptioncode);

    setAttribute(id, value, name, exceptioncode);
}

void ElementImpl::removeAttribute( const DOMString &name, int& exceptioncode )
{
    NodeImpl::Id id = document()->getId(NodeImpl::AttributeId, name.implementation(), true, true);
    if (!id) return;

    attributes(false)->removeNamedItem(id, false, name.implementation(), exceptioncode);
    // it's allowed to remove attributes that don't exist.
    if ( exceptioncode == DOMException::NOT_FOUND_ERR )
        exceptioncode = 0;
}

AttrImpl* ElementImpl::getAttributeNode( const DOMString &name )
{
    NodeImpl::Id id = document()->getId(NodeImpl::AttributeId, name.implementation(), true, true);
    if (!id) return 0;
    if (!namedAttrMap) return 0;

    return static_cast<AttrImpl*>(attributes()->getNamedItem(id, false, name.implementation()));
}

Attr ElementImpl::setAttributeNode( AttrImpl* newAttr, int& exceptioncode )
{
    if (!newAttr) {
      exceptioncode = DOMException::NOT_FOUND_ERR;
      return 0;
    }
    Attr r = attributes(false)->setNamedItem(newAttr, false,
                         newAttr->nodeName().implementation(), exceptioncode);
    if ( !exceptioncode )
      newAttr->setOwnerElement( this );
    return r;
}

Attr ElementImpl::removeAttributeNode( AttrImpl* oldAttr, int& exceptioncode )
{
  if (!oldAttr || oldAttr->ownerElement() != this) {
    exceptioncode = DOMException::NOT_FOUND_ERR;
    return 0;
  }

  if (isReadOnly()) {
    exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
    return 0;
  }

  if (!namedAttrMap) {
    exceptioncode = DOMException::NOT_FOUND_ERR;
    return 0;
  }

  return attributes(false)->removeAttr(oldAttr);
}


DOMString ElementImpl::getAttributeNS( const DOMString &namespaceURI,
                                       const DOMString &localName,
                                       int& exceptioncode)
{
    if (!localName.implementation()) {
      exceptioncode = DOMException::NOT_FOUND_ERR;
      return DOMString();
    }

    NodeImpl::Id id = document()->getId(NodeImpl::AttributeId, namespaceURI.implementation(), 0/*prefix*/, localName.implementation(), true, true);
    return getAttribute(id, true);
}

void ElementImpl::removeAttributeNS( const DOMString &namespaceURI,
                                 const DOMString &localName,
                                 int& exceptioncode)
{
    NodeImpl::Id id = document()->getId(NodeImpl::AttributeId, namespaceURI.implementation(), 0/*prefix*/, localName.implementation(), false, true);
    attributes(false)->removeNamedItem(id, true, 0, exceptioncode);
}

AttrImpl* ElementImpl::getAttributeNodeNS( const DOMString &namespaceURI,
                                  const DOMString &localName,
                                  int& exceptioncode )
{
    if (!localName.implementation()) {
      exceptioncode = DOMException::NOT_FOUND_ERR;
      return 0;
    }

    NodeImpl::Id id = document()->getId(NodeImpl::AttributeId, namespaceURI.implementation(),
						0/*prefix*/, localName.implementation(), true, true);
    if (!attributes(true)) return 0;

    return static_cast<AttrImpl*>(attributes()->getNamedItem(id, true));
}

Attr ElementImpl::setAttributeNodeNS( AttrImpl* newAttr, int& exceptioncode )
{
    if (!newAttr) {
        exceptioncode = DOMException::NOT_FOUND_ERR;
        return 0;
    }
    // WRONG_DOCUMENT_ERR and INUSE_ATTRIBUTE_ERR are already tested & thrown by setNamedItem
    Attr r = attributes(false)->setNamedItem(newAttr, true, 0, exceptioncode);
    if ( !exceptioncode )
        newAttr->setOwnerElement( this );
    return r;
}


DOMString ElementImpl::nodeName() const
{
    return tagName();
}

DOMString ElementImpl::namespaceURI() const
{
   if (m_htmlCompat)
        return DOMString();
   return document()->getName(NamespaceId, id() >> 16);
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

void ElementImpl::defaultEventHandler(EventImpl *e)
{
    if (m_render && m_render->scrollsOverflow()) {
        switch( e->id() ) {
          case EventImpl::KEYDOWN_EVENT:
          case EventImpl::KEYUP_EVENT:
          case EventImpl::KEYPRESS_EVENT:
            if (!focused() || e->target() != this)
              break;
          // fall through
          case EventImpl::KHTML_MOUSEWHEEL_EVENT:
            if (m_render->handleEvent(*e))
                e->setDefaultHandled();
          default:
            break;
        }
    }
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

RenderStyle *ElementImpl::styleForRenderer(RenderObject * /*parentRenderer*/)
{
    return document()->styleSelector()->styleForElement(this);
}

RenderObject *ElementImpl::createRenderer(RenderArena *arena, RenderStyle *style)
{
    if (document()->documentElement() == this && style->display() == NONE) {
        // Ignore display: none on root elements.  Force a display of block in that case.
        RenderBlock* result = new (arena) RenderBlock(this);
        if (result) result->setStyle(style);
        return result;
    }
    return RenderObject::createObject(this, style);
}

void ElementImpl::attach()
{
    assert(!attached());
    assert(!m_render);
    assert(parentNode());

#if SPEED_DEBUG < 1
    createRendererIfNeeded();
#endif

    NodeBaseImpl::attach();
}

void ElementImpl::close()
{
    NodeImpl::close();

    // Trigger all the addChild changes as one large dynamic appendChildren change
    if (attached())
        backwardsStructureChanged();
}

void ElementImpl::detach()
{
    document()->dynamicDomRestyler().resetDependencies(this);

    if (ElementRareDataImpl* rd = rareData())
        rd->resetComputedStyle();

    NodeBaseImpl::detach();
}

void ElementImpl::structureChanged()
{
    NodeBaseImpl::structureChanged();

    if (!document()->renderer())
        return; // the document is about to be destroyed

    document()->dynamicDomRestyler().restyleDepedent(this, StructuralDependency);
    // In theory BackwardsStructurualDependencies are indifferent to prepend,
    // but it's too rare to optimize.
    document()->dynamicDomRestyler().restyleDepedent(this, BackwardsStructuralDependency);
}

void ElementImpl::backwardsStructureChanged()
{
    NodeBaseImpl::backwardsStructureChanged();

    if (!document()->renderer())
        return; // the document is about to be destroyed

    // Most selectors are not affected by append. Fire the few that are.
    document()->dynamicDomRestyler().restyleDepedent(this, BackwardsStructuralDependency);
}

void ElementImpl::attributeChanged(NodeImpl::Id id)
{
    if (!document()->renderer())
        return; // the document is about to be destroyed

#if 0 // one-one dependencies for attributes disabled
    document()->dynamicDomRestyler().restyleDepedent(this, AttributeDependency);
#endif
    if (document()->dynamicDomRestyler().checkDependency(id, PersonalDependency))
        setChanged(true);
    if (document()->dynamicDomRestyler().checkDependency(id, AncestorDependency))
        setChangedAscendentAttribute(true);
    if (document()->dynamicDomRestyler().checkDependency(id, PredecessorDependency) && parent())
        // Any element that dependt on a predecessors attribute, also depend structurally on parent
        parent()->structureChanged();
}

void ElementImpl::recalcStyle( StyleChange change )
{
    // ### should go away and be done in renderobject
    RenderStyle* _style = m_render ? m_render->style() : 0;
    bool hasParentRenderer = parent() ? parent()->attached() : false;

    if ((change > NoChange || changed())) {
        if (ElementRareDataImpl* rd = rareData())
            rd->resetComputedStyle();
    }

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
    qDebug("recalcStyle(%d: %s, changed: %d)[%p: %s]", change, debug, changed(), this, tagName().string().toLatin1().constData());
#endif
    if ( hasParentRenderer && (change >= Inherit || changed() || (change==NoInherit && affectedByNoInherit())) ) {
        RenderStyle *newStyle = document()->styleSelector()->styleForElement(this);
        newStyle->ref();
        StyleChange ch = diff( _style, newStyle );
        if (ch == Detach) {
            if (attached()) detach();
            // ### Suboptimal. Style gets calculated again.
            attach();
            // attach recalulates the style for all children. No need to do it twice.
            setChanged( false );
            setHasChangedChild( false );
            newStyle->deref();
            return;
        }
        else if (ch != NoChange) {
            if( m_render ) {
                m_render->setStyle(newStyle);
            }
        }
        newStyle->deref();

        if ( change != Force)
            change = ch;
    }
    // If a changed attribute has ancestor dependencies, restyle all children
    if (changedAscendentAttribute()) {
        change = Force;
        setChangedAscendentAttribute(false);
    }

    NodeImpl *n;
    for (n = _first; n; n = n->nextSibling()) {
        if ( change >= Inherit || n->hasChangedChild() || n->changed() ||
             ( change == NoInherit && n->affectedByNoInherit() )
           ) {
	    //qDebug("    (%p) calling recalcStyle on child %p/%s, change=%d", this, n, n->isElementNode() ? ((ElementImpl *)n)->tagName().string().toLatin1().constData() : n->isTextNode() ? "text" : "unknown", change );
            n->recalcStyle( change );
        }
    }

    setChanged( false );
    setHasChangedChild( false );
}

bool ElementImpl::isFocusable() const
{
    if (m_render && m_render->scrollsOverflow())
        return true;

    // Only make editable elements selectable if its parent element
    // is not editable. FIXME: this is not 100% right as non-editable elements
    // within editable elements are focusable too.
    return isContentEditable() && !(parentNode() && parentNode()->isContentEditable());
}

bool ElementImpl::isContentEditable() const
{
    if (document()->part() && document()->part()->isEditable())
        return true;

    // document()->updateRendering();

    if (!renderer()) {
        if (parentNode())
            return parentNode()->isContentEditable();
        else
            return false;
    }

    return renderer()->style()->userInput() == UI_ENABLED;
}

void ElementImpl::setContentEditable(bool enabled) {
    // FIXME: the approach is flawed, better use an enum instead of bool
    int value;
    if (enabled)
        value = CSS_VAL_ENABLED;
    else {
        // Intelligently use "none" or "disabled", depending on the type of
        // element
        // FIXME: intelligence not impl'd yet
        value = CSS_VAL_NONE;

        // FIXME: reset caret if it is in this node or a child
    }/*end if*/
    // FIXME: use addCSSProperty when I get permission to move it here
//    kDebug(6000) << "CSS_PROP__KHTML_USER_INPUT: "<< value << endl;
    getInlineStyleDecls()->setProperty(CSS_PROP__KHTML_USER_INPUT, value, false);
    setChanged();
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
	return checkChild(id(), newChild->id(), document()->inStrictMode());
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

void ElementImpl::scrollIntoView(bool /*alignToTop*/)
{
    // ###
    kWarning() << "non-standard scrollIntoView() not implemented";
}

void ElementImpl::createNonCSSDecl()
{
    assert(!m_hasCombinedStyle);
    CSSStyleDeclarationImpl *ild = m_style.inlineDecls;
    m_style.combinedDecls = new CombinedStyleDecl;
    m_style.combinedDecls->inlineDecls = ild;
    CSSStyleDeclarationImpl *ncd = new CSSStyleDeclarationImpl(0);
    m_style.combinedDecls->nonCSSDecls = ncd;
    ncd->ref();
    ncd->setParent(document()->elementSheet());
    ncd->setNode(this);
    ncd->setStrictParsing( false );
    m_hasCombinedStyle = true;
}

CSSStyleDeclarationImpl *ElementImpl::getInlineStyleDecls()
{
    if (!inlineStyleDecls()) createInlineDecl();
        return inlineStyleDecls();
}

void ElementImpl::createInlineDecl( )
{
    assert( !m_style.inlineDecls || (m_hasCombinedStyle && !m_style.combinedDecls->inlineDecls) );

    CSSStyleDeclarationImpl *dcl = new CSSStyleDeclarationImpl(0);
    dcl->ref();
    dcl->setParent(document()->elementSheet());
    dcl->setNode(this);
    dcl->setStrictParsing( !document()->inCompatMode() );
    if (m_hasCombinedStyle)
        m_style.combinedDecls->inlineDecls = dcl;
    else
        m_style.inlineDecls = dcl;
}

void ElementImpl::dispatchAttrRemovalEvent(NodeImpl::Id /*id*/, DOMStringImpl * /*value*/)
{
    // ### enable this stuff again
    if (!document()->hasListenerType(DocumentImpl::DOMATTRMODIFIED_LISTENER))
	return;
    //int exceptioncode = 0;
    //dispatchEvent(new MutationEventImpl(EventImpl::DOMATTRMODIFIED_EVENT,true,false,attr,attr->value(),
    //attr->value(), document()->attrName(attr->id()),MutationEvent::REMOVAL),exceptioncode);
}

void ElementImpl::dispatchAttrAdditionEvent(NodeImpl::Id /*id*/, DOMStringImpl * /*value*/)
{
    // ### enable this stuff again
    if (!document()->hasListenerType(DocumentImpl::DOMATTRMODIFIED_LISTENER))
	return;
   //int exceptioncode = 0;
   //dispatchEvent(new MutationEventImpl(EventImpl::DOMATTRMODIFIED_EVENT,true,false,attr,attr->value(),
   //attr->value(),document()->attrName(attr->id()),MutationEvent::ADDITION),exceptioncode);
}

void ElementImpl::updateId(DOMStringImpl* oldId, DOMStringImpl* newId)
{
    if (!inDocument())
        return;

    if (oldId && oldId->l)
        removeId(DOMString(oldId).string());

    if (newId && newId->l)
        addId(DOMString(newId).string());
}

void ElementImpl::removeId(const QString& id)
{
  document()->getElementByIdCache().remove(id, this);
}

void ElementImpl::addId(const QString& id)
{
  document()->getElementByIdCache().add(id, this);
}

void ElementImpl::insertedIntoDocument()
{
    // need to do superclass processing first so inDocument() is true
    // by the time we reach updateId
    NodeBaseImpl::insertedIntoDocument();

    if (hasID()) {
        DOMString id = getAttribute(ATTR_ID);
        updateId(0, id.implementation());
    }
}

void ElementImpl::removedFromDocument()
{
    if (hasID()) {
        DOMString id = getAttribute(ATTR_ID);
        updateId(id.implementation(), 0);
    }

    NodeBaseImpl::removedFromDocument();
}

DOMString ElementImpl::openTagStartToString(bool expandurls) const
{
    DOMString result = DOMString("<") + tagName();

    NamedAttrMapImpl *attrMap = attributes(true);

    if (attrMap) {
	unsigned long numAttrs = attrMap->length();
	for (unsigned long i = 0; i < numAttrs; i++) {
	    result += " ";

	    AttributeImpl *attribute = attrMap->attrAt(i);
	    AttrImpl *attr = attribute->attr();

	    if (attr) {
		result += attr->toString();
	    } else {
		result += document()->getName( NodeImpl::AttributeId, attribute->id());
		if (!attribute->value().isNull()) {
		    result += "=\"";
		    // FIXME: substitute entities for any instances of " or '
		    // Expand out all urls, i.e. the src and href attributes
		    if(expandurls && ( attribute->id() == ATTR_SRC || attribute->id() == ATTR_HREF))
			if(document()) {
                            //We need to sanitize the urls - strip out the passwords.
			    //FIXME:   are src=  and href=  the only places that might have a password and need to be sanitized?
			    KUrl safeURL(document()->completeURL(attribute->value().string()));
			    safeURL.setPass(QString());
			    result += Qt::escape(safeURL.prettyUrl());
			}
		        else {
		 	    kWarning() << "document() returned false";
			    result += attribute->value();
			}
		    else
		        result += attribute->value();
		    result += "\"";
		}
	    }
	}
    }

    return result;
}
DOMString ElementImpl::selectionToString(NodeImpl *selectionStart, NodeImpl *selectionEnd, int startOffset, int endOffset, bool &found) const
{
    DOMString result = openTagStartToString();

    if (hasChildNodes()) {
	result += ">";

	for (NodeImpl *child = firstChild(); child != NULL; child = child->nextSibling()) {
	    result += child->selectionToString(selectionStart, selectionEnd, startOffset, endOffset, found); // this might set found to true
	    if(child == selectionEnd)
	        found = true;
	    if(found) break;
	}

	result += "</";
	result += tagName();
	result += ">";
    } else {
	result += " />";
    }

    return result;
}

DOMString ElementImpl::toString() const
{
    QString result = openTagStartToString().string(); //Accumulate in QString, since DOMString can't append well.

    if (hasChildNodes()) {
	result += ">";

	for (NodeImpl *child = firstChild(); child != NULL; child = child->nextSibling()) {
	    DOMString kid = child->toString();
	    result += QString::fromRawData(kid.unicode(), kid.length());
	}

	result += "</";
	result += tagName().string();
	result += ">";
    } else if (result.length() == 1) {
	// ensure we do not get results like < /> can happen when serialize document
        result = "";
    } else {
	result += " />";
    }

    return result;
}

RenderStyle* ElementImpl::computedStyle()
{
    if (m_render && m_render->style())
	return m_render->style();

    if (!attached())
        // FIXME: Try to do better than this. Ensure that styleForElement() works for elements that are not in the
        // document tree and figure out when to destroy the computed style for such elements.
        return 0;

    ElementRareDataImpl* rd = createRareData();
    if (!rd->m_computedStyle) {
        rd->m_computedStyle = document()->styleSelector()->styleForElement(this, parent() ? parent()->computedStyle() : 0);
        rd->m_computedStyle->ref();
    }
    return rd->m_computedStyle;
}

void ElementImpl::blur()
{
    if(document()->focusNode() == this)
        document()->setFocusNode(0);
}

void ElementImpl::focus()
{
    document()->setFocusNode(this);
}

// -------------------------------------------------------------------------

XMLElementImpl::XMLElementImpl(DocumentImpl *doc, NodeImpl::Id id)
    : ElementImpl(doc)
{
    // Called from createElement(). In this case localName, prefix, and namespaceURI all need to be null.
    m_id = id;
}

XMLElementImpl::XMLElementImpl(DocumentImpl *doc, NodeImpl::Id id, DOMStringImpl *_prefix)
    : ElementImpl(doc)
{
    // Called from createElementNS()
    m_id = id;

    m_prefix = _prefix;
    if (m_prefix)
	m_prefix->ref();
}

XMLElementImpl::~XMLElementImpl()
{
}

DOMString XMLElementImpl::localName() const
{
    if ( m_htmlCompat )
       return DOMString(); // was created with non-namespace-aware createElement()
    return document()->getName(ElementId, m_id);
}

DOMString XMLElementImpl::tagName() const
{
    DOMString tn = document()->getName(ElementId, id());
    if (m_htmlCompat)
        tn = tn.upper();

    if (m_prefix)
        return DOMString(m_prefix) + ":" + tn;

    return tn;
}

WTF::PassRefPtr<NodeImpl> XMLElementImpl::cloneNode ( bool deep )
{
    WTF::RefPtr<ElementImpl> clone = new XMLElementImpl(docPtr(), id(), m_prefix);
    finishCloneNode( clone.get(), deep );
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

NodeImpl *NamedAttrMapImpl::getNamedItem ( NodeImpl::Id id, bool nsAware, DOMStringImpl* qName ) const
{
    if (!m_element)
	return 0;
    unsigned int mask = nsAware ? ~0L : NodeImpl_IdLocalMask;
    id = (id & mask);

    for (unsigned long i = 0; i < m_attrCount; i++) {
	if ((m_attrs[i].id() & mask) == id) {
            // if we are called with a qualified name, filter out NS-aware elements with non-matching name.
            if (qName && (namespacePart(m_attrs[i].id()) != defaultNamespace) &&
                strcasecmp(m_attrs[i].name(), DOMString(qName)))
                continue;
	    return m_attrs[i].createAttr(m_element,m_element->docPtr());
        }
    }

    return 0;
}

Node NamedAttrMapImpl::removeNamedItem ( NodeImpl::Id id, bool nsAware, DOMStringImpl* qName, int &exceptioncode )
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
    unsigned int mask = nsAware ? ~0L : NodeImpl_IdLocalMask;
    id = (id & mask);

    for (unsigned long i = 0; i < m_attrCount; i++) {
	if ((m_attrs[i].id() & mask) == id) {
            // if we are called with a qualified name, filter out NS-aware elements with non-matching name.
            if (qName && (namespacePart(m_attrs[i].id()) != defaultNamespace) &&
                strcasecmp(m_attrs[i].name(), DOMString(qName)))
                continue;
	    id = m_attrs[i].id();
	    if (id == ATTR_ID)
	       m_element->updateId(m_attrs[i].val(), 0);
	    Node removed(m_attrs[i].createAttr(m_element,m_element->docPtr()));
	    m_attrs[i].free(); // Also sets the remove'd ownerElement to 0
	    memmove(m_attrs+i,m_attrs+i+1,(m_attrCount-i-1)*sizeof(AttributeImpl));
	    m_attrCount--;
	    m_attrs = (AttributeImpl*)realloc(m_attrs,m_attrCount*sizeof(AttributeImpl));
	    m_element->parseNullAttribute(id);
	    m_element->attributeChanged(id);
	    return removed;
	}
    }

    // NOT_FOUND_ERR: Raised if there is no node with the specified namespaceURI
    // and localName in this map.
    exceptioncode = DOMException::NOT_FOUND_ERR;
    return 0;
}

Node NamedAttrMapImpl::setNamedItem ( NodeImpl* arg, bool nsAware, DOMStringImpl* qName, int &exceptioncode )
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
    if (arg->document() != m_element->document()) {
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
	// Already have this attribute.
	// DOMTS core-1 test "hc_elementreplaceattributewithself" says we should return it.
	return attr;
    }
    unsigned int mask = nsAware ? ~0L : NodeImpl_IdLocalMask;
    NodeImpl::Id id = (attr->id() & mask);

    for (unsigned long i = 0; i < m_attrCount; i++) {
	if ((m_attrs[i].id() & mask) == id) {
            // if we are called with a qualified name, filter out NS-aware elements with non-matching name.
            if (qName && (namespacePart(m_attrs[i].id()) != defaultNamespace) &&
                strcasecmp(m_attrs[i].name(), DOMString(qName)))
                continue;
	    // Attribute exists; replace it
	    if (id == ATTR_ID)
	       m_element->updateId(m_attrs[i].val(), attr->val());

	    Node replaced = m_attrs[i].createAttr(m_element,m_element->docPtr());
	    m_attrs[i].free();
	    m_attrs[i].m_attrId = 0; /* "has implementation" flag */
	    m_attrs[i].m_data.attr = attr;
	    m_attrs[i].m_data.attr->ref();
	    attr->setElement(m_element);
	    m_element->parseAttribute(&m_attrs[i]);
	    m_element->attributeChanged(m_attrs[i].id());
	    // ### dispatch mutation events
	    return replaced;
	}
    }

    // No existing attribute; add to list
    m_attrCount++;
    m_attrs = (AttributeImpl*)realloc(m_attrs,m_attrCount*sizeof(AttributeImpl));
    m_attrs[m_attrCount-1].m_attrId = 0; /* "has implementation" flag */
    m_attrs[m_attrCount-1].m_data.attr = attr;
    m_attrs[m_attrCount-1].m_data.attr->ref();
    attr->setElement(m_element);
    if (id == ATTR_ID)
        m_element->updateId(0, attr->val());
    m_element->parseAttribute(&m_attrs[m_attrCount-1]);
    m_element->attributeChanged(m_attrs[m_attrCount-1].id());
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

DOMStringImpl *NamedAttrMapImpl::getValue(NodeImpl::Id id, bool nsAware, DOMStringImpl* qName) const
{
    unsigned int mask = nsAware ? ~0L : NodeImpl_IdLocalMask;
    id = (id & mask);
    for (unsigned long i = 0; i < m_attrCount; i++)
        if ((m_attrs[i].id() & mask) == id) {
            // if we are called with a qualified name, filter out NS-aware elements with non-matching name.
            if (qName && (namespacePart(m_attrs[i].id()) != defaultNamespace) &&
                strcasecmp(m_attrs[i].name(), qName))
                continue;
            return m_attrs[i].val();
        }
    return 0;
}

void NamedAttrMapImpl::setValue(NodeImpl::Id id, DOMStringImpl *value, DOMStringImpl* qName,
                                DOMStringImpl *prefix, bool nsAware, bool hasNS)
{
    assert( !(qName && nsAware) );
    if (!id) return;
    // Passing in a null value here causes the attribute to be removed. This is a khtml extension
    // (the spec does not specify what to do in this situation).
    int exceptioncode = 0;
    if (!value) {
	removeNamedItem(id, nsAware, qName, exceptioncode);
	return;
    }
    unsigned int mask = nsAware ? ~0L : NodeImpl_IdLocalMask;
    NodeImpl::Id mid = (id & mask);

    // Check for an existing attribute.
    for (unsigned long i = 0; i < m_attrCount; i++) {
	if ((m_attrs[i].id() & mask) == mid) {
            // if we are called with a qualified name, filter out NS-aware elements with non-matching name.
            if (qName && (m_attrs[i].id() & NodeImpl_IdNSMask) &&
                strcasecmp(m_attrs[i].name(), DOMString(qName)))
                continue;
	    if (prefix)
		m_attrs[i].attr()->setPrefix(prefix,exceptioncode);
	    m_attrs[i].setValue(value,m_element);
	    // ### dispatch mutation events
	    return;
	}
    }

    // No existing matching attribute; add a new one
    m_attrCount++;
    m_attrs = (AttributeImpl*)realloc(m_attrs,m_attrCount*sizeof(AttributeImpl));
    if (!nsAware) {
	// Called from setAttribute()... we only have a name
        m_attrs[m_attrCount-1].m_attrId = id;
        m_attrs[m_attrCount-1].m_data.value = value;
	m_attrs[m_attrCount-1].m_data.value->ref();
    }
    else {
	// Called from setAttributeNS()... need to create a full AttrImpl here
        if(!m_element)
            return;
	m_attrs[m_attrCount-1].m_data.attr = new AttrImpl(m_element,m_element->docPtr(),
							  id,
							  value,
                                                          prefix);
	m_attrs[m_attrCount-1].m_attrId = 0; /* "has implementation" flag */
	m_attrs[m_attrCount-1].m_data.attr->ref();
        m_attrs[m_attrCount-1].m_data.attr->setHTMLCompat( !hasNS &&
                                                 m_element->document()->htmlMode() != DocumentImpl::XHtml );
    }
    if (m_element) {
	if (id == ATTR_ID)
	    m_element->updateId(0, value);
	m_element->parseAttribute(&m_attrs[m_attrCount-1]);
	m_element->attributeChanged(m_attrs[m_attrCount-1].id());
    }
    // ### dispatch mutation events
}

Attr NamedAttrMapImpl::removeAttr(AttrImpl *attr)
{
    for (unsigned long i = 0; i < m_attrCount; i++) {
	if (m_attrs[i].attr() == attr) {
	    NodeImpl::Id id = m_attrs[i].id();
	    if (id == ATTR_ID)
	        m_element->updateId(attr->val(), 0);
	    Node removed(m_attrs[i].createAttr(m_element,m_element->docPtr()));
	    m_attrs[i].free();
	    memmove(m_attrs+i,m_attrs+i+1,(m_attrCount-i-1)*sizeof(AttributeImpl));
	    m_attrCount--;
	    m_attrs = (AttributeImpl*)realloc(m_attrs,m_attrCount*sizeof(AttributeImpl));
	    m_element->parseNullAttribute(id);
	    m_element->attributeChanged(id);
	    // ### dispatch mutation events
	    return removed;
	}
    }

    return 0;
}

NodeImpl::Id NamedAttrMapImpl::mapId(DOMStringImpl* namespaceURI,
				     DOMStringImpl* localName, bool readonly)
{
    if (!m_element)
	return 0;

    return m_element->document()->getId(NodeImpl::AttributeId, namespaceURI, 0, localName, readonly,
                                           true /*lookupHTML*/);
}

void NamedAttrMapImpl::copyAttributes(NamedAttrMapImpl *other)
{
    assert(m_element);
    unsigned long i;
    for (i = 0; i < m_attrCount; i++) {
        if (m_attrs[i].id() == ATTR_ID)
            m_element->updateId(m_attrs[i].val(), 0);
	m_attrs[i].free();
    }
    m_attrCount = other->m_attrCount;
    m_attrs = (AttributeImpl*)realloc(m_attrs,m_attrCount*sizeof(AttributeImpl));
    for (i = 0; i < m_attrCount; i++) {
	m_attrs[i].m_attrId = other->m_attrs[i].m_attrId;
	if (m_attrs[i].m_attrId) {
	    m_attrs[i].m_data.value = other->m_attrs[i].m_data.value;
	    m_attrs[i].m_data.value->ref();
	}
	else {
	    WTF::RefPtr<NodeImpl> clonedAttr = other->m_attrs[i].m_data.attr->cloneNode(true);
	    m_attrs[i].m_data.attr = static_cast<AttrImpl*>(clonedAttr.get());
	    m_attrs[i].m_data.attr->ref();
	    m_attrs[i].m_data.attr->setElement(m_element);
	}
	if (m_attrs[i].id() == ATTR_ID)
	   m_element->updateId(0, m_attrs[i].val());
	m_element->parseAttribute(&m_attrs[i]);
	m_element->attributeChanged(m_attrs[i].id());
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

}