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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

//#define EVENT_DEBUG
#include "dom/dom_exception.h"
#include "dom/dom_node.h"
#include "dom/html_image.h"
#include "xml/dom_textimpl.h"
#include "xml/dom_docimpl.h"
#include "xml/dom2_eventsimpl.h"
#include "xml/dom_elementimpl.h"

#include "html/dtd.h"
#include "html/htmlparser.h"
#include "html/html_imageimpl.h"

#include "rendering/render_canvas.h"
#include "misc/htmlhashes.h"
#include "css/css_valueimpl.h"
#include "css/css_stylesheetimpl.h"
#include "css/cssstyleselector.h"
#include "css/cssvalues.h"
#include "css/cssproperties.h"
#include "xml/dom_xmlimpl.h"

#include <qtextstream.h>
#include <kdebug.h>
#include <stdlib.h>

// ### support default attributes
// ### dispatch mutation events
// ### check for INVALID_CHARACTER_ERR where appropriate

using namespace DOM;
using namespace khtml;

AttrImpl::AttrImpl(ElementImpl* element, DocumentPtr* docPtr, NodeImpl::Id attrId,
		   DOMStringImpl *value, DOMStringImpl *prefix)
    : NodeBaseImpl(docPtr),
      m_element(element),
      m_attrId(attrId)
{
    m_value = value;
    m_value->ref();

    m_prefix = prefix;
    if (m_prefix)
	m_prefix->ref();
    m_specified = true; // we don't yet support default attributes
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
    return getDocument()->getName(NamespaceId, m_attrId >> 16);
}

DOMString AttrImpl::localName() const
{
    if (m_htmlCompat)
       return DOMString();
    return getDocument()->getName(AttributeId, m_attrId);
}

DOMString AttrImpl::nodeValue() const
{
    return m_value;
}

DOMString AttrImpl::name() const
{
    DOMString n = getDocument()->getName(AttributeId, m_attrId);

    // compat mode always return attribute names in lowercase.
    // that's not formally in the specification, but common
    // practice - a w3c erratum to DOM L2 is pending.
    if (m_htmlCompat)
        n = n.lower();

    if (m_prefix && m_prefix->l)
        return DOMString(m_prefix) + ":" + n;

    return n;
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

    if (m_element && m_attrId == ATTR_ID)
        m_element->updateId(m_value, v.implementation());

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

    if ( firstChild() ) {
	result += "=\"";

	for (NodeImpl *child = firstChild(); child != NULL; child = child->nextSibling()) {
	    result += child->toString();
	}

	result += "\"";
    } else if ( !nodeValue().isEmpty() ){
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
        if (!attr) return 0;
        attr->setHTMLCompat( docPtr->document()->htmlMode() != DocumentImpl::XHtml );
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

ElementImpl::ElementImpl(DocumentPtr *doc)
    : NodeBaseImpl(doc)
{
    namedAttrMap = 0;
    m_styleDecls = 0;
    m_prefix = 0;
    m_restyleLate = false;
    m_restyleSelfLate = false;
    m_restyleChildrenLate = false;
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

DOMString ElementImpl::getAttribute( NodeImpl::Id id, bool nsAware, const DOMString& qName) const
{
    if (!namedAttrMap)
	return DOMString();

    DOMStringImpl *value = namedAttrMap->getValue(id, nsAware, (qName.isEmpty() ? 0: qName.implementation()));
    if (value)
	return value;

    // then search in default attr in case it is not yet set
    NamedAttrMapImpl* dm = defaultMap();
    value = dm ? dm->getValue(id, nsAware, (qName.isEmpty() ? 0: qName.implementation())) : 0;
    if (value)
	return value;

    return DOMString();
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
    NodeImpl::Id id = getDocument()->getId(AttributeId, namespaceURI.implementation(),
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
        for (unsigned long i = 0; i < len; i++)
            parseAttribute(&namedAttrMap->m_attrs[i]);
    }
}

NodeImpl *ElementImpl::cloneNode(bool deep)
{
    ElementImpl *clone;
    if ( !localName().isNull() )
        clone = getDocument()->createElementNS( namespaceURI(), nodeName() );
    else
        clone = getDocument()->createElement( nodeName() );
    if (!clone) return 0;
    finishCloneNode( clone, deep );
    return clone;
}

void ElementImpl::finishCloneNode( ElementImpl* clone, bool deep )
{
    // clone attributes
    if (namedAttrMap)
	clone->attributes()->copyAttributes(namedAttrMap);

    // clone individual style rules
    if (m_styleDecls)
        *(clone->styleRules()) = *m_styleDecls;

    if (deep)
        cloneChildNodes(clone);
}

bool ElementImpl::hasAttributes() const
{
    return namedAttrMap && namedAttrMap->length() > 0;
}

bool ElementImpl::hasAttribute( const DOMString& name )
{
    NodeImpl::Id id = getDocument()->getId(NodeImpl::AttributeId, name.implementation(), true, true);
    if (!id) return false;
    if (!namedAttrMap) return false;
    return namedAttrMap->getValue(id, false, name.implementation()) != 0;
}

bool ElementImpl::hasAttributeNS( const DOMString &namespaceURI,
                              const DOMString &localName )
{
    NodeImpl::Id id = getDocument()->getId(NodeImpl::AttributeId,namespaceURI.implementation(),
						 0/*prefix*/, localName.implementation(), true, true);
    if (!id) return false;
    if (!namedAttrMap) return false;
    return namedAttrMap->getValue(id, true) != 0;
}

DOMString ElementImpl::getAttribute( const DOMString &name )
{
    NodeImpl::Id id = getDocument()->getId(NodeImpl::AttributeId,name.implementation(),true,true);
    if (!id) return DOMString();

    return getAttribute(id, false, name);
}

void ElementImpl::setAttribute( const DOMString &name, const DOMString &value, int& exceptioncode )
{
    NodeImpl::Id id = getDocument()->getId(NodeImpl::AttributeId, name.implementation(), false /* allocate */,
                                                 true, &exceptioncode);

    setAttribute(id, value, name, exceptioncode);
}

void ElementImpl::removeAttribute( const DOMString &name, int& exceptioncode )
{
    NodeImpl::Id id = getDocument()->getId(NodeImpl::AttributeId, name.implementation(), true, true);
    if (!id) return; 

    attributes(false)->removeNamedItem(id, false, name.implementation(), exceptioncode);
}

AttrImpl* ElementImpl::getAttributeNode( const DOMString &name )
{
    NodeImpl::Id id = getDocument()->getId(NodeImpl::AttributeId, name.implementation(), true, true);
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
    
    NodeImpl::Id id = getDocument()->getId(NodeImpl::AttributeId, namespaceURI.implementation(), 0/*prefix*/, localName.implementation(), true, true);
    return getAttribute(id, true);
}

void ElementImpl::removeAttributeNS( const DOMString &namespaceURI,
                                 const DOMString &localName,
                                 int& exceptioncode)
{
    NodeImpl::Id id = getDocument()->getId(NodeImpl::AttributeId, namespaceURI.implementation(), 0/*prefix*/, localName.implementation(), false, true);
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
    
    NodeImpl::Id id = getDocument()->getId(NodeImpl::AttributeId, namespaceURI.implementation(),
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
   return getDocument()->getName(NamespaceId, id() >> 16);
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
    return getDocument()->styleSelector()->styleForElement(this);
}

RenderObject *ElementImpl::createRenderer(RenderArena *arena, RenderStyle *style)
{
    if (getDocument()->documentElement() == this && style->display() == NONE) {
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

    if (!getDocument()->renderer())
        return; // the document is about to be destroyed
    
    if (m_restyleChildrenLate) {
        NodeImpl *e = firstChild();
        while(e) {
            if (e->isElementNode()) {
                if (static_cast<ElementImpl*>(e)->restyleLate()) {
                    static_cast<ElementImpl*>(e)->recalcStyle(Force);
                    static_cast<ElementImpl*>(e)->setRestyleLate(false);
                }
            }
            e = e->nextSibling();
        }
        m_restyleChildrenLate = false;
    }
    if (m_restyleSelfLate) {
        recalcStyle(Force);
        m_restyleSelfLate = false;
    }
}

void ElementImpl::recalcStyle( StyleChange change )
{
    // ### should go away and be done in renderobject
    RenderStyle* _style = m_render ? m_render->style() : 0;
    bool hasParentRenderer = parent() ? parent()->attached() : false;

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
    if ( hasParentRenderer && (change >= Inherit || changed()) ) {
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

       if ( change != Force) {
            if (getDocument()->usesDescendantRules())
                change = Force;
            else
                change = ch;
        }
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

bool ElementImpl::isFocusable() const
{
    // Only make editable elements selectable if its parent element
    // is not editable. FIXME: this is not 100% right as non-editable elements
    // within editable elements are focusable too.
    return contentEditable() && (!parentNode() || !parentNode()->contentEditable());
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
	return checkChild(id(), newChild->id(), !getDocument()->inCompatMode());
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
    m_styleDecls->setStrictParsing( !getDocument()->inCompatMode() );
}

void ElementImpl::dispatchAttrRemovalEvent(NodeImpl::Id /*id*/, DOMStringImpl * /*value*/)
{
    // ### enable this stuff again
    if (!getDocument()->hasListenerType(DocumentImpl::DOMATTRMODIFIED_LISTENER))
	return;
    //int exceptioncode = 0;
    //dispatchEvent(new MutationEventImpl(EventImpl::DOMATTRMODIFIED_EVENT,true,false,attr,attr->value(),
    //attr->value(), getDocument()->attrName(attr->id()),MutationEvent::REMOVAL),exceptioncode);
}

void ElementImpl::dispatchAttrAdditionEvent(NodeImpl::Id /*id*/, DOMStringImpl * /*value*/)
{
    // ### enable this stuff again
    if (!getDocument()->hasListenerType(DocumentImpl::DOMATTRMODIFIED_LISTENER))
	return;
   //int exceptioncode = 0;
   //dispatchEvent(new MutationEventImpl(EventImpl::DOMATTRMODIFIED_EVENT,true,false,attr,attr->value(),
   //attr->value(),getDocument()->attrName(attr->id()),MutationEvent::ADDITION),exceptioncode);
}

void ElementImpl::updateId(DOMStringImpl* oldId, DOMStringImpl* newId)
{
    if (!inDocument())
        return;

    DocumentImpl* doc = getDocument();
    if (oldId && oldId->l)
        removeId(DOMString(oldId).string());

    if (newId && newId->l)
        addId(DOMString(newId).string());
}

void ElementImpl::removeId(const QString& id)
{
  getDocument()->getElementByIdCache().remove(id, this);
}

void ElementImpl::addId(const QString& id)
{
  getDocument()->getElementByIdCache().add(id, this);
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
		result += getDocument()->getName( NodeImpl::AttributeId, attribute->id());
		if (!attribute->value().isNull()) {
		    result += "=\"";
		    // FIXME: substitute entities for any instances of " or '
		    // Expand out all urls, i.e. the src and href attributes
		    if(expandurls && ( attribute->id() == ATTR_SRC || attribute->id() == ATTR_HREF))
			if(getDocument()) {
                            //We need to sanitize the urls - strip out the passwords.
			    //FIXME:   are src=  and href=  the only places that might have a password and need to be sanitized?
                            KURL safeURL(getDocument()->completeURL(attribute->value().string()));
                            safeURL.setPass(QString::null);
			    result += safeURL.htmlURL();
			}
		        else {
		 	    kdWarning() << "getDocument() returned false";
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
    DOMString result = openTagStartToString();

    if (hasChildNodes()) {
	result += ">";

	for (NodeImpl *child = firstChild(); child != NULL; child = child->nextSibling()) {
	    result += child->toString();
	}

	result += "</";
	result += tagName();
	result += ">";
    } else {
	result += " />";
    }

    return result;
}

bool ElementImpl::contentEditable() const {
#if 0
    DOM::CSSPrimitiveValueImpl *val = static_cast<DOM::CSSPrimitiveValueImpl *>
    		(const_cast<ElementImpl *>(this)->styleRules()
		->getPropertyCSSValue(CSS_PROP__KONQ_USER_INPUT));
//    kdDebug() << "val" << val << endl;
    return val ? val->getIdent() == CSS_VAL_ENABLED : false;
#endif
    return NodeImpl::contentEditable();
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
//    kdDebug(6000) << "CSS_PROP__KHTML_USER_INPUT: "<< value << endl;
    styleRules()->setProperty(CSS_PROP__KHTML_USER_INPUT, value, false, true);
    setChanged();

}

// -------------------------------------------------------------------------

XMLElementImpl::XMLElementImpl(DocumentPtr *doc, NodeImpl::Id id)
    : ElementImpl(doc)
{
    // Called from createElement(). In this case localName, prefix, and namespaceURI all need to be null.
    m_id = id;
}

XMLElementImpl::XMLElementImpl(DocumentPtr *doc, NodeImpl::Id id, DOMStringImpl *_prefix)
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
    return getDocument()->getName(ElementId, m_id);
}

DOMString XMLElementImpl::tagName() const
{
    DOMString tn = getDocument()->getName(ElementId, id());
    if (m_htmlCompat)
        tn = tn.upper();

    if (m_prefix)
        return DOMString(m_prefix) + ":" + tn;

    return tn;
}

NodeImpl *XMLElementImpl::cloneNode ( bool deep )
{
    XMLElementImpl *clone = new XMLElementImpl(docPtr(), id(), m_prefix);
    finishCloneNode( clone, deep );
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
            if (qName && (m_attrs[i].id() & NodeImpl_IdNSMask) &&
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
            if (qName && (m_attrs[i].id() & NodeImpl_IdNSMask) &&
                strcasecmp(m_attrs[i].name(), DOMString(qName)))
                continue;
	    id = m_attrs[i].id();
	    if (id == ATTR_ID)
	       m_element->updateId(m_attrs[i].val(), 0);
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
	// Already have this attribute.
	// DOMTS core-1 test "hc_elementreplaceattributewithself" says we should return it.
	return attr;
    }
    unsigned int mask = nsAware ? ~0L : NodeImpl_IdLocalMask;
    NodeImpl::Id id = (attr->id() & mask);

    for (unsigned long i = 0; i < m_attrCount; i++) {
	if ((m_attrs[i].id() & mask) == id) {
            // if we are called with a qualified name, filter out NS-aware elements with non-matching name.
            if (qName && (m_attrs[i].id() & NodeImpl_IdNSMask) &&
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
            if (qName && (m_attrs[i].id() & NodeImpl_IdNSMask) &&
                strcasecmp(m_attrs[i].name(), DOMString(qName)))
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
                                                 m_element->getDocument()->htmlMode() != DocumentImpl::XHtml );
    }
    if (m_element) {
	if (id == ATTR_ID)
	    m_element->updateId(0, value);
	m_element->parseAttribute(&m_attrs[m_attrCount-1]);
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
	    m_element->parseAttribute(id,0);
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

    return m_element->getDocument()->getId(NodeImpl::AttributeId, namespaceURI, 0, localName, readonly,
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
	    m_attrs[i].m_data.attr = static_cast<AttrImpl*>(other->m_attrs[i].m_data.attr->cloneNode(true));
	    m_attrs[i].m_data.attr->ref();
	    m_attrs[i].m_data.attr->setElement(m_element);
	}
	if (m_attrs[i].id() == ATTR_ID)
	   m_element->updateId(0, m_attrs[i].val());
	m_element->parseAttribute(&m_attrs[i]);
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
