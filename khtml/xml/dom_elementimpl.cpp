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

#include "rendering/render_object.h"
#include "misc/htmlhashes.h"
#include "css/css_valueimpl.h"
#include "css/css_stylesheetimpl.h"
#include "css/cssstyleselector.h"
#include "xml/dom_xmlimpl.h"

#include <qtextstream.h>
#include <kdebug.h>

using namespace DOM;
using namespace khtml;

void AttributeImpl::allocateImpl(ElementImpl* e) {
    _impl = new AttrImpl(e, this);
    _impl->ref();
}

AttrImpl::AttrImpl(ElementImpl* element, AttributeImpl* a)
    : NodeBaseImpl(element->docPtr()),
      m_element(element),
      m_attribute(a)
{
    assert(!m_attribute->_impl);
    m_attribute->_impl = this;
    m_attribute->ref();
}

AttrImpl::~AttrImpl()
{
    assert(m_attribute->_impl == this);
    m_attribute->_impl = 0;
    m_attribute->deref();
}

DOMString AttrImpl::nodeName() const
{
    return getDocument()->attrName(m_attribute->id());
}

unsigned short AttrImpl::nodeType() const
{
    return Node::ATTRIBUTE_NODE;
}

DOMString AttrImpl::prefix() const
{
    return m_attribute->prefix();
}

void AttrImpl::setPrefix(const DOMString &_prefix, int &exceptioncode )
{
    checkSetPrefix(_prefix, exceptioncode);
    if (exceptioncode)
        return;

    m_attribute->setPrefix(_prefix.implementation());
}

DOMString AttrImpl::nodeValue() const {
    return m_attribute->val();
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

    m_attribute->setValue(v.implementation());
}

void AttrImpl::setNodeValue( const DOMString &v, int &exceptioncode )
{
    exceptioncode = 0;
    // NO_MODIFICATION_ALLOWED_ERR: taken care of by setValue()
    setValue(v, exceptioncode);
}

NodeImpl *AttrImpl::cloneNode ( bool /*deep*/)
{
    return new AttrImpl(0,  new AttributeImpl(m_attribute->id(), m_attribute->val()));
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

ElementImpl::ElementImpl(DocumentPtr *doc)
    : NodeBaseImpl(doc)
{
    namedAttrMap = 0;
    m_styleDecls = 0;
    m_prefix = 0;
}

ElementImpl::~ElementImpl()
{
    if (m_render)
        detach();

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
    if (!namedAttrMap) return DOMString();
    AttributeImpl* a = namedAttrMap->getAttributeItem(id);
    if (a) return a->val();

    // then search in default attr in case it is not yet set
    NamedAttrMapImpl* dm = defaultMap();
    if(!dm) return DOMString();
    AttributeImpl* defattr = dm->getAttributeItem(id);
    if (!defattr) return DOMString();
    return defattr->val();
}

void ElementImpl::setAttribute(NodeImpl::Id id, DOMStringImpl* value, int &exceptioncode )
{
    // allocate attributemap if necessary
    AttributeImpl* old = attributes(false)->getAttributeItem(id);

    // NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly
    if (namedAttrMap->isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return;
    }

    if (old && !value)
        namedAttrMap->removeAttribute(id);
    else if (!old && value)
        namedAttrMap->addAttribute(new AttributeImpl(id, value));
    else if (old && value)
        old->setValue(value);
}

void ElementImpl::setAttributeMap( NamedAttrMapImpl* list )
{
    if(namedAttrMap)
        namedAttrMap->deref();

    namedAttrMap = list;

    if(namedAttrMap) {
        namedAttrMap->ref();
        namedAttrMap->element = this;
        unsigned int len = namedAttrMap->length();
        for(unsigned int i = 0; i < len; i++)
            parseAttribute(namedAttrMap->attrs[i]);
    }
}

NodeImpl *ElementImpl::cloneNode ( bool deep)
{
    // ### we loose the namespace here ... FIXME
    ElementImpl *clone = getDocument()->createElement(tagName());
    if (!clone) return 0;

    // clone attributes
    if(namedAttrMap)
        *(static_cast<NamedAttrMapImpl*>(clone->attributes())) = *namedAttrMap;

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

RenderObject *ElementImpl::createRenderer()
{
    return RenderObject::createObject(this);
}

void ElementImpl::attach()
{
    if (!m_render)
    {
#if SPEED_DEBUG < 2
        setStyle(getDocument()->styleSelector()->styleForElement(this));
#if SPEED_DEBUG < 1
        if(parentNode() && parentNode()->renderer()) {
            m_render = createRenderer();
            if(m_render) {
                m_render->setStyle(m_style);
                parentNode()->renderer()->addChild(m_render, nextRenderer());
            }
        }
#endif
#endif
    }

    NodeBaseImpl::attach();
}

void ElementImpl::detach()
{
    NodeBaseImpl::detach();

    if ( m_render )
        m_render->detach();

    m_render = 0;
}

void ElementImpl::recalcStyle( StyleChange change )
{
    if ( m_style ) {
	// ### should go away and be done in renderobject
	bool needsUpdate = false;
//  	qDebug("recalcStyle(%p: %s)", this, tagName().string().latin1());
	if ( change >= Inherit || changed() ) {
	    EDisplay oldDisplay = m_style ? m_style->display() : INLINE;

	    int dynamicState = StyleSelector::None;
	    if ( m_mouseInside )
		dynamicState |= StyleSelector::Hover;
	    if ( m_focused )
		dynamicState |= StyleSelector::Focus;
	    if ( m_active )
		dynamicState |= StyleSelector::Active;

	    RenderStyle *newStyle = getDocument()->styleSelector()->styleForElement(this, dynamicState);
	    StyleChange ch = diff( m_style, newStyle );
	    if ( ch != NoChange ) {
		setStyle( newStyle );

		if (oldDisplay != m_style->display()) {
		    // ### doesn't this already take care of changing the style
		    // for all children?
		    detach();
		    attach();
		    needsUpdate = true;
		}
		if( m_render && m_style ) {
//  		    qDebug("--> setting style on render element bgcolor=%s", m_style->backgroundColor().name().latin1());
		    m_render->setStyle(m_style);
		    needsUpdate = true;
		}
	    }
	    if ( change != Force )
		change = ch;
	}

	NodeImpl *n;
	for (n = _first; n; n = n->nextSibling()) {
// 	    qDebug("    (%p) calling recalcStyle on child %s, change=%d", this, n->isElementNode() ? ((ElementImpl *)n)->tagName().string().latin1() : n->isTextNode() ? "text" : "unknown", change );
	    if ( change >= Inherit || n->isTextNode() ||
		 n->hasChangedChild() || n->changed() )
		n->recalcStyle( change );
	}

	// ### should go away and be done by the renderobjects in
	// a more intelligent way
	if ( needsUpdate && m_render && !parentNode()->changed() ) {
	    RenderObject *r = m_render;
	    if ( !m_render->layouted() ) {
		if ( m_render->isInline() )
		    r = m_render->containingBlock();
		r->updateSize();
	    }
	    r->repaint();
	}
    }
    setChanged( false );
    setHasChangedChild( false );
}

void ElementImpl::setFocus(bool received)
{
    NodeBaseImpl::setFocus(received);
    recalcStyle( Inherit );
}

void ElementImpl::setActive(bool down)
{
    NodeBaseImpl::setActive(down);
    recalcStyle( Inherit );
}

khtml::FindSelectionResult ElementImpl::findSelectionNode( int _x, int _y, int _tx, int _ty, DOM::Node & node, int & offset )
{
    //kdDebug(6030) << "ElementImpl::findSelectionNode " << this << " _x=" << _x << " _y=" << _y
    //           << " _tx=" << _tx << " _ty=" << _ty << endl;

    // ######### Duplicated code from mouseEvent
    // TODO put the code above (getting _tx,_ty) in a common place and call it from here

    if (!m_render) return SelectionPointAfter;

    RenderObject *p = m_render->parent();
    while( p && p->isAnonymousBox() ) {
//      kdDebug( 6030 ) << "parent is anonymous!" << endl;
        // we need to add the offset of the anonymous box
        _tx += p->xPos();
        _ty += p->yPos();
        p = p->parent();
    }

    if(!m_render->isInline() || !m_render->firstChild() || m_render->isFloating() )
    {
        m_render->absolutePosition(_tx, _ty);
    }

    int off=0, lastOffset=0;
    DOM::Node nod;
    DOM::Node lastNode;
    NodeImpl *child = firstChild();
    while(child != 0)
    {
        khtml::FindSelectionResult pos = child->findSelectionNode(_x, _y, _tx, _ty, nod, off);
        //kdDebug(6030) << this << " child->findSelectionNode returned " << pos << endl;
        if ( pos == SelectionPointInside ) // perfect match
        {
            node = nod;
            offset = off;
            //kdDebug(6030) << "ElementImpl::findSelectionNode " << this << " match offset=" << offset << endl;
            return SelectionPointInside;
        } else if ( pos == SelectionPointBefore )
        {
            //x,y is before this element -> stop here
            if ( !lastNode.isNull() ) {
                node = lastNode;
                offset = lastOffset;
                //kdDebug(6030) << "ElementImpl::findSelectionNode " << this << " before this child -> returning offset=" << offset << endl;
                return SelectionPointInside;
            } else {
                //kdDebug(6030) << "ElementImpl::findSelectionNode " << this << " before us -> returning -2" << endl;
                return SelectionPointBefore;
            }
        }
        // SelectionPointAfter -> keep going
        if ( !nod.isNull() )
        {
            lastNode = nod;
            lastOffset = off;
        }
        child = child->nextSibling();
    }
    node = lastNode;
    offset = lastOffset;
    return SelectionPointAfter;
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

void ElementImpl::dispatchAttrRemovalEvent(AttributeImpl *attr)
{
    if (!getDocument()->hasListenerType(DocumentImpl::DOMATTRMODIFIED_LISTENER))
	return;
    int exceptioncode = 0;
//     dispatchEvent(new MutationEventImpl(EventImpl::DOMATTRMODIFIED_EVENT,true,false,attr,attr->value(),
// 		  attr->value(), getDocument()->attrName(attr->id()),MutationEvent::REMOVAL),exceptioncode);
}

void ElementImpl::dispatchAttrAdditionEvent(AttributeImpl *attr)
{
    if (!getDocument()->hasListenerType(DocumentImpl::DOMATTRMODIFIED_LISTENER))
	return;
    int exceptioncode = 0;
//     dispatchEvent(new MutationEventImpl(EventImpl::DOMATTRMODIFIED_EVENT,true,false,attr,attr->value(),
//                                         attr->value(),getDocument()->attrName(attr->id()),MutationEvent::ADDITION),exceptioncode);
}

void ElementImpl::dump(QTextStream *stream, QString ind) const
{
    if (namedAttrMap) {
        for (uint i = 0; i < namedAttrMap->length(); i++) {
            AttributeImpl *attr = namedAttrMap->attributeItem(i);
            *stream << " " << DOMString(getDocument()->attrName(attr->id())).string().ascii()
                    << "=\"" << DOMString(attr->value()).string().ascii() << "\"";
        }
    }

    NodeBaseImpl::dump(stream,ind);
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
    if(namedAttrMap)
        *(static_cast<NamedAttrMapImpl*>(clone->attributes())) = *namedAttrMap;

    if (deep)
        cloneChildNodes(clone);

    return clone;
}

// -------------------------------------------------------------------------

NamedAttrMapImpl::NamedAttrMapImpl(ElementImpl *e)
    : element(e)
{
    attrs = 0;
    len = 0;
}

NamedAttrMapImpl::~NamedAttrMapImpl()
{
    clearAttributes();
}


AttrImpl *NamedAttrMapImpl::getNamedItem ( NodeImpl::Id id ) const
{
    AttributeImpl* a = getAttributeItem(id);
    if (!a) return 0;

    if (!a->attrImpl())
        a->allocateImpl(element);

    return a->attrImpl();
}

Node NamedAttrMapImpl::setNamedItem ( NodeImpl* arg, int &exceptioncode )
{
    if (!element) {
        exceptioncode = DOMException::NOT_FOUND_ERR;
        return 0;
    }

    // NO_MODIFICATION_ALLOWED_ERR: Raised if this map is readonly.
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return 0;
    }

    // WRONG_DOCUMENT_ERR: Raised if arg was created from a different document than the one that created this map.
    if (arg->getDocument() != element->getDocument()) {
        exceptioncode = DOMException::WRONG_DOCUMENT_ERR;
        return 0;
    }

    // Not mentioned in spec: throw a HIERARCHY_REQUEST_ERROR if the user passes in a non-attribute node
    if (!arg->isAttributeNode()) {
        exceptioncode = DOMException::HIERARCHY_REQUEST_ERR;
        return 0;
    }
    AttrImpl *attr = static_cast<AttrImpl*>(arg);

    // INUSE_ATTRIBUTE_ERR: Raised if arg is an Attr that is already an attribute of another Element object.
    // The DOM user must explicitly clone Attr nodes to re-use them in other elements.
    if (attr->ownerElement()) {
        exceptioncode = DOMException::INUSE_ATTRIBUTE_ERR;
        return 0;
    }

    AttributeImpl* a = attr->attrImpl();
    assert(a); // ### can the impl be 0 ?

    Node r(attr);
    element->setAttribute(a->id(), a->val(), exceptioncode);
    return r;
}

// The DOM2 spec doesn't say that removeAttribute[NS] throws NOT_FOUND_ERR
// if the attribute is not found - David
Node NamedAttrMapImpl::removeNamedItem ( NodeImpl::Id id, int &exceptioncode )
{
    // ### should this really be raised when the attribute to remove isn't there at all?
    // NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly
    if (isReadOnly()) {
        exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
        return Node();
    }

    AttributeImpl* a = getAttributeItem(id);
    if (!a) return Node();

    if (!a->attrImpl())  a->allocateImpl(element);
    Node r(a->attrImpl());
    removeAttribute(id);
    return r;
}

AttrImpl *NamedAttrMapImpl::item ( unsigned long index ) const
{
    if (index >= len)
        return 0;

    if (!attrs[index]->attrImpl())
        attrs[index]->allocateImpl(element);

    return attrs[index]->attrImpl();
}

unsigned long NamedAttrMapImpl::length(  ) const
{
    return len;
}

AttributeImpl* NamedAttrMapImpl::getAttributeItem(NodeImpl::Id id) const
{
    for (unsigned long i = 0; i < len; ++i)
        if (attrs[i]->id() == id)
            return attrs[i];

    return 0;
}

NodeImpl::Id NamedAttrMapImpl::mapId(const DOMString& namespaceURI,
                                     const DOMString& localName, bool readonly)
{
    assert(element);
    if (!element) return NodeImpl::IdIllegal;
    return element->getDocument()->attrId(namespaceURI.implementation(),
                                            localName.implementation(), readonly);
}

void NamedAttrMapImpl::clearAttributes()
{
    if (attrs) {
        uint i;
        for (i = 0; i < len; i++) {
            if (attrs[i]->_impl)
                attrs[i]->_impl->m_element = 0;
            attrs[i]->deref();
        }
        delete [] attrs;
        attrs = 0;
    }
    len = 0;
}

void NamedAttrMapImpl::detachFromElement()
{
    // we allow a NamedAttrMapImpl w/o an element in case someone still has a reference
    // to if after the element gets deleted - but the map is now invalid
    element = 0;
    clearAttributes();
}

NamedAttrMapImpl& NamedAttrMapImpl::operator=(const NamedAttrMapImpl& other)
{
    // clone all attributes in the other map, but attach to our element
    if (!element) return *this;

    clearAttributes();
    len = other.len;
    attrs = new AttributeImpl* [len];

    // first initialize attrs vector, then call parseAttribute on it
    // this allows parseAttribute to use getAttribute
    for (uint i = 0; i < len; i++) {
        attrs[i] = new AttributeImpl(other.attrs[i]->id(), other.attrs[i]->val());
        attrs[i]->ref();
    }

    for(uint i = 0; i < len; i++)
        element->parseAttribute(attrs[i]);

    // ###
    element->setChanged(true);
    return *this;
}

void NamedAttrMapImpl::addAttribute(AttributeImpl *attr)
{
    // Add the attribute tot he list
    AttributeImpl **newAttrs = new AttributeImpl* [len+1];
    if (attrs) {
      for (uint i = 0; i < len; i++)
        newAttrs[i] = attrs[i];
      delete [] attrs;
    }
    attrs = newAttrs;
    attrs[len++] = attr;
    attr->ref();

    // Notify the element that the attribute has been added, and dispatch appropriate mutation events
    // Note that element may be null here if we are called from insertAttr() during parsing
    if (element) {
        element->parseAttribute(attr);
        element->setChanged(true);
        element->dispatchAttrAdditionEvent(attr);
        element->dispatchSubtreeModifiedEvent();
    }
}

void NamedAttrMapImpl::removeAttribute(NodeImpl::Id id)
{
    unsigned long index = len+1;
    for (unsigned long i = 0; i < len; ++i)
        if (attrs[i]->id() == id) {
            index = i;
            break;
        }

    if (index >= len) return;

    // Remove the attribute from the list
    AttributeImpl* ret = attrs[index];
    if (attrs[index]->_impl)
        attrs[index]->_impl->m_element = 0;
    if (len == 1) {
        delete [] attrs;
        attrs = 0;
        len = 0;
    }
    else {
        AttributeImpl **newAttrs = new AttributeImpl* [len-1];
        uint i;
        for (i = 0; i < uint(index); i++)
            newAttrs[i] = attrs[i];
        len--;
        for (; i < len; i++)
            newAttrs[i] = attrs[i+1];
        delete [] attrs;
        attrs = newAttrs;
    }

    // Notify the element that the attribute has been removed
    // dispatch appropriate mutation events
    if (ret->_value) {
        ret->_value->deref();
        ret->_value = 0;
        element->parseAttribute(ret);
    }
    element->dispatchAttrRemovalEvent(ret);
    element->dispatchSubtreeModifiedEvent();
}

