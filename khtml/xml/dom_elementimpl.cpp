/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
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
#include "dom_elementimpl.h"

#include "dom_exception.h"
#include "dom_node.h"
#include "dom_textimpl.h"
#include "dom_docimpl.h"

#include "css/cssstyleselector.h"
#include "rendering/render_object.h"
#include "misc/htmlhashes.h"

using namespace DOM;
using namespace khtml;

/*
 * NOTE:
 * According to the DOM docs, an Attr stores the value directly in it's parsed
 * form, but for values containing entities, creates a subtree with nodes
 * containing the unexpanded form (for XML). On read, the value is always
 * returned entity-free, so we decided for HTML we could only store a
 * parsed DOMString and have no child-nodes.
 */

AttrImpl::AttrImpl() : NodeImpl(0)
{
    _specified = false;
    _element = 0;

    _name = 0;
    _value = 0;
    attrId = 0;
}


AttrImpl::AttrImpl(DocumentImpl *doc, const DOMString &name) : NodeImpl(doc)
{
    _specified = false;
    _element = 0;

    attrId = 0;
    _name = 0;
    setName(name);
    _value = 0;
}

AttrImpl::AttrImpl(DocumentImpl *doc, int id) : NodeImpl(doc) // ### change parameter id to _id
{
    _specified = false;
    _element = 0;

    _name = 0;
    _value = 0;
    attrId = id;
}

AttrImpl::AttrImpl(const AttrImpl &other) : NodeImpl(other.ownerDocument())
{
    _specified = other.specified();
    _element = other._element;
    _name = other._name;
    if (_name) _name->ref();
    _value = other._value;
    if (_value) _value->ref();
    attrId = other.attrId;
}

AttrImpl &AttrImpl::operator = (const AttrImpl &other)
{
    _specified = other.specified();
    document = other.ownerDocument();
    _element = other._element;

    if (_name) _name->deref();
    _name = other._name;
    if (_name) _name->ref();
    if (_value) _value->deref();
    _value = other._value;
    if (_value) _value->ref();
    attrId = other.attrId;

    return *this;
}

AttrImpl::~AttrImpl()
{
    if(_name) _name->deref();
    if(_value) _value->deref();
}

const DOMString AttrImpl::nodeName() const
{
    return name();
}

unsigned short AttrImpl::nodeType() const
{
    return Node::ATTRIBUTE_NODE;
}


DOMString AttrImpl::name() const
{
    if(attrId)
	return getAttrName(attrId);
    else if (_name)
	return _name;
    else
	return DOMString();
}

void AttrImpl::setName(const DOMString &n)
{
    if(_name) _name->deref();
    _name = n.implementation();
    attrId = khtml::getAttrID(QConstString(_name->s, _name->l).string().lower().ascii(), _name->l);
    if (attrId)
	_name = 0;
    else
	_name->ref();
}

DOMString AttrImpl::value() const {
    return _value;
}

void AttrImpl::setValue( const DOMString &v )
{
    // according to the DOM docs, we should create an unparsed Text child
    // node here; we decided this was not necessary for HTML

    // TODO: parse value string, interprete entities

    if (_element)
	_element->checkReadOnly();

    if (_value) _value->deref();
    _value = v.implementation();
    if (_value) _value->ref();
    _specified = true;

    if (_element) {
	_element->parseAttribute(this);
	_element->setChanged(true);
    }
}

void AttrImpl::setNodeValue( const DOMString &v )
{
    setValue(v);
}

// ### is this still needed?
AttrImpl::AttrImpl(const DOMString &name, const DOMString &value,
		   DocumentImpl *doc, bool specified) : NodeImpl(doc)
{
    _element = 0;
    _specified = specified;

    attrId = 0;
    _name = 0;
    setName(name);
    _value = value.implementation();
    if (_value) _value->ref();

}

AttrImpl::AttrImpl(const khtml::Attribute *attr, DocumentImpl *doc, ElementImpl *element) : NodeImpl(doc)
{
    _name = attr->n;
    if (_name) _name->ref();
    _value = attr->v;
    if (_value) _value->ref();
    attrId = attr->id;
    _element = element;
    _specified = 1;

}

AttrImpl::AttrImpl(const DOMString &name, const DOMString &value, DocumentImpl *doc) : NodeImpl(doc)
{
    attrId = 0;
    _name = 0;
    setName(name);
    _value = value.implementation();
    if (_value) _value->ref();
    _element = 0;
    _specified = 1;
}

AttrImpl::AttrImpl(int _id, const DOMString &value, DocumentImpl *doc) : NodeImpl(doc)
{
  attrId = _id;
  _name = 0;
  _value = value.implementation();
  if (_value) _value->ref();
  _element = 0;
  _specified = false;
}

NodeImpl *AttrImpl::parentNode() const
{
    return 0;
}

NodeImpl *AttrImpl::previousSibling() const
{
    return 0;
}

NodeImpl *AttrImpl::nextSibling() const
{
    return 0;
}

NodeImpl *AttrImpl::cloneNode ( bool /*deep*/ )
{
    AttrImpl *newImpl = new AttrImpl(*this);
    newImpl->_element = 0; // can't have two attributes with the same name/id attached to an element
    return newImpl;
}

bool AttrImpl::deleteMe()
{
    if(!_element && _ref <= 0) return true;
    return false;
}

// -------------------------------------------------------------------------

ElementImpl::ElementImpl(DocumentImpl *doc) : NodeBaseImpl(doc)
{
    m_style = 0;
    namedAttrMap = new NamedAttrMapImpl(this);
    namedAttrMap->ref();
}

ElementImpl::~ElementImpl()
{
    delete m_style;
    namedAttrMap->detachFromElement();
    namedAttrMap->deref();
}

bool ElementImpl::isInline()
{
    if(!m_style) return false;
    return (m_style->display() == khtml::INLINE);
}

unsigned short ElementImpl::nodeType() const
{
    return Node::ELEMENT_NODE;
}

DOMString ElementImpl::tagName() const
{
    return nodeName();
}

DOMString ElementImpl::getAttribute( const DOMString &name )
{
  // search in already set attributes first
    AttrImpl *attr = static_cast<AttrImpl*>(namedAttrMap->getNamedItem(name));
    if (attr) return attr->value();

    if(!defaultMap()) return 0;
    // then search in default attr in case it is not yet set
    int index = defaultMap()->find(name);
    if (index != -1) return defaultMap()->value(index);

    return 0;
}

DOMString ElementImpl::getAttribute( int id )
{
    // search in already set attributes first
    AttrImpl *attr = static_cast<AttrImpl*>(namedAttrMap->getIdItem(id));
    if (attr) return attr->value();

    if(!defaultMap()) return 0;
    // then search in default attr in case it is not yet set
    int index = defaultMap()->find(id);
    if (index != -1) return defaultMap()->value(index);

    return 0;
}

AttrImpl *ElementImpl::getAttributeNode ( int index )
{
    return namedAttrMap->getIdItem(index);
}

int ElementImpl::getAttributeCount()
{
    return namedAttrMap->length();
}

void ElementImpl::setAttribute( const DOMString &name, const DOMString &value )
{
    // TODO: check for invalid characters in value -> throw exception
    AttrImpl *oldAttr;
    if (value.isNull())
	oldAttr = static_cast<AttrImpl*>(namedAttrMap->removeNamedItem(name));
    else
	oldAttr = static_cast<AttrImpl*>(namedAttrMap->setNamedItem(new AttrImpl(name,value,document)));
    if (oldAttr && oldAttr->deleteMe())
	delete oldAttr;
}

void ElementImpl::setAttribute( int id, const DOMString &value )
{
    AttrImpl *oldAttr;
    if (value.isNull())
	oldAttr = namedAttrMap->removeIdItem(id);
    else
	oldAttr = namedAttrMap->setIdItem(new AttrImpl(id,value,document));
    if (oldAttr && oldAttr->deleteMe())
	delete oldAttr;
}

void ElementImpl::setAttribute( AttributeList list )
{
    namedAttrMap->fromAttributeList(list);
}

void ElementImpl::removeAttribute( const DOMString &name )
{
    AttrImpl *oldAttr = static_cast<AttrImpl*>(namedAttrMap->removeNamedItem(name));
    if (oldAttr && oldAttr->deleteMe())
	delete oldAttr;
}

NodeImpl *ElementImpl::cloneNode ( bool deep )
{
    ElementImpl *newImpl = document->createElement(tagName());
    if (!newImpl)
      return 0;

    newImpl->setParent(0);
    newImpl->setFirstChild(0);
    newImpl->setLastChild(0);

    newImpl->namedAttrMap->fromNamedAttrMapImpl(namedAttrMap);

    if(deep)
    {
	NodeImpl *n;
	for(n = firstChild(); n; n = n->nextSibling())
	{
	    newImpl->appendChild(n->cloneNode(deep));
	}
    }
    return newImpl;
}

NamedNodeMapImpl *ElementImpl::attributes() const
{
    return namedAttrMap;
}

AttrImpl *ElementImpl::getAttributeNode( const DOMString &name )
{
    // ### do we return attribute node if it is in the default map but not specified?
    return static_cast<AttrImpl*>(namedAttrMap->getNamedItem(name));

}

AttrImpl *ElementImpl::setAttributeNode( AttrImpl *newAttr )
{
    if (!newAttr)
	throw DOMException(DOMException::NOT_FOUND_ERR);

    if (newAttr->attrId)
	return static_cast<AttrImpl*>(namedAttrMap->setIdItem(newAttr));
    else
	return static_cast<AttrImpl*>(namedAttrMap->setNamedItem(newAttr));
}

AttrImpl *ElementImpl::removeAttributeNode( AttrImpl *oldAttr )
{
    if (!oldAttr)
	throw DOMException(DOMException::NOT_FOUND_ERR);
    checkReadOnly();

    // ### should we replace with default in map? currently default attrs don't exist in map
    return namedAttrMap->removeAttr(oldAttr);
}

NodeListImpl *ElementImpl::getElementsByTagName( const DOMString &name )
{
    return new TagNodeListImpl( this, name );
}

void ElementImpl::normalize()
{
    NodeImpl *child = _first;
    while(1)
    {
	NodeImpl *nextChild = child->nextSibling();
	if(!nextChild) return;
	if ( (child->nodeType() == Node::TEXT_NODE)
	     && (nextChild->nodeType() == Node::TEXT_NODE))
	{
	    (static_cast<TextImpl *>(child))->appendData( (static_cast<TextImpl *>(nextChild))->data() );
	    removeChild(nextChild);
	}
	else
	{	
	    child = nextChild;
	    if(child->isElementNode())
	    {
		(static_cast<ElementImpl *>(child))->normalize();
	    }
	}
    }
}

AttributeList *ElementImpl::defaultMap() const
{
    return 0;
}

void ElementImpl::attach(KHTMLView *w)
{
    m_style = document->styleSelector()->styleForElement(this);
    khtml::RenderObject *r = _parent->renderer();
    if(r)
    {
	m_render = khtml::RenderObject::createObject(this);
	if(m_render)
	{
	    r->addChild(m_render, _next ? _next->renderer() : 0);
	    m_render->ref();
	}
    }

    NodeBaseImpl::attach(w);
}

void ElementImpl::detach()
{
    if(m_render) m_render->deref();
    m_render = 0;
    NodeBaseImpl::detach();
}

void ElementImpl::applyChanges(bool top, bool force)
{
    // ### find a better way to handle non-css attributes
    if(!m_render) return;

    if (force || changed())
	recalcStyle();

    // a style change can influence the children, so we just go
    // through them and trigger an appplyChanges there too
    NodeImpl *n = _first;
    while(n) {
	n->applyChanges(false,force || changed());
	n = n->nextSibling();
    }

    // calc min and max widths starting from leafs
    // might belong to renderer, but this is simple to do here
    if (force || changed())
	m_render->calcMinMaxWidth();

    if(top) {
	// force a relayout of this part of the document
	m_render->updateSize();
	// force a repaint of this part.
	// ### if updateSize() changes any size, it will already force a
	// repaint, so we might do double work here...
	m_render->repaint();
    }
    setChanged(false);
}

void ElementImpl::recalcStyle()
{
    if(!m_render) return;
    bool faf = m_style->flowAroundFloats();
    delete m_style;
    m_style = document->styleSelector()->styleForElement(this);

    m_style->setFlowAroundFloats(faf);
    m_render->setStyle(m_style);

    NodeImpl *n;
    for (n = _first; n; n = n->nextSibling())
	n->recalcStyle();
}

// -------------------------------------------------------------------------

NamedAttrMapImpl::NamedAttrMapImpl(ElementImpl *e) : element(e)
{
    attrs = 0;
    len = 0;
}

NamedAttrMapImpl::~NamedAttrMapImpl()
{
    clearAttrs();
}

void NamedAttrMapImpl::fromAttributeList(const AttributeList list)
{
    if (!element)
	throw DOMException(DOMException::NOT_FOUND_ERR);
    element->checkReadOnly();
	
    uint i;
    clearAttrs(); // should be empty, but just in case...

    // now import the list
    len = list.length();
    attrs = new AttrImpl* [len];

    for (i = 0; i < len; i++) {
	attrs[i] = new AttrImpl(list[i],element->ownerDocument(),element);
	element->parseAttribute(attrs[i]);
    }
    // used only during parsing - we don't call applyChanges() here

}

void NamedAttrMapImpl::fromNamedAttrMapImpl(const NamedAttrMapImpl *other)
{
    // clone all attributes in the other map, but attach to our element
    if (!element)
	throw DOMException(DOMException::NOT_FOUND_ERR);
	
    clearAttrs();
    len = other->len;
    attrs = new AttrImpl* [len];
    uint i;
    for (i = 0; i < len; i++) {
	attrs[i] = static_cast<AttrImpl*>(other->attrs[i]->cloneNode(false));
	attrs[i]->_element = element;
	element->parseAttribute(attrs[i]);
    }
    element->setChanged(true);
}

unsigned long NamedAttrMapImpl::length() const {
    if (!element)
	throw DOMException(DOMException::NOT_FOUND_ERR);

    return len;
}

NodeImpl *NamedAttrMapImpl::getNamedItem ( const DOMString &name ) const
{
    if (!element)
	throw DOMException(DOMException::NOT_FOUND_ERR);

    uint i;
    for (i = 0; i < len; i++) {
	if (!strcasecmp(attrs[i]->name(),name))
	    return attrs[i];
    }

    return 0;
}

AttrImpl *NamedAttrMapImpl::getIdItem ( int id ) const
{
    if (!element)
	throw DOMException(DOMException::NOT_FOUND_ERR);

    uint i;
    for (i = 0; i < len; i++) {
	if (attrs[i]->attrId == id)
	    return attrs[i];
    }

    return 0;
}


NodeImpl *NamedAttrMapImpl::setNamedItem ( const Node &arg )
{
    // ### check for invalid chars in name ?
    // ### check same document

    if (!element)
	throw DOMException(DOMException::NOT_FOUND_ERR);
    element->checkReadOnly();

    if (arg.nodeType() != Node::ATTRIBUTE_NODE)
	throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);

    AttrImpl *attr = static_cast<AttrImpl*>(arg.handle());

    if (attr->_element)
	throw DOMException(DOMException::INUSE_ATTRIBUTE_ERR);

    uint i;
    for (i = 0; i < len; i++) {
	if (!strcasecmp(attrs[i]->name(),attr->name())) {
	    // attribute with this id already in list
	    AttrImpl *oldAttr = attrs[i];
	    attrs[i] = attr;
	    attr->_element = element;
	    oldAttr->_element = 0;
	    element->parseAttribute(attr);
	    element->setChanged(true);
	    return oldAttr; // ### check this gets deleted if ref = 0 and it's not assigned to anything
	}
    }

    // attribute with this id not yet in list
    AttrImpl **newAttrs = new AttrImpl* [len+1];
    if (attrs) {
      for (i = 0; i < len; i++)
        newAttrs[i] = attrs[i];
      delete attrs;
    }
    attrs = newAttrs;
    attrs[len] = attr;
    len++;
    attr->_element = element;
    element->parseAttribute(attr);
    element->setChanged(true);
    return 0;
}

AttrImpl *NamedAttrMapImpl::setIdItem ( AttrImpl *attr )
{
    if (!element)
	throw DOMException(DOMException::NOT_FOUND_ERR);
    element->checkReadOnly();

    if (attr->_element)
	throw DOMException(DOMException::INUSE_ATTRIBUTE_ERR);

    uint i;
    for (i = 0; i < len; i++) {
	if (attrs[i]->attrId == attr->attrId) {
	    // attribute with this id already in list
	    AttrImpl *oldAttr = attrs[i];
	    attrs[i] = attr;
	    attr->_element = element;
	    oldAttr->_element = 0;
	    element->parseAttribute(attr);
	    element->setChanged(true);
	    return oldAttr; // ### check this gets deleted if ref = 0 and it's not assigned to anything
	}
    }

    // attribute with this id not yet in list
    AttrImpl **newAttrs = new AttrImpl* [len+1];
    if (attrs) {
      for (i = 0; i < len; i++)
        newAttrs[i] = attrs[i];
      delete attrs;
    }
    attrs = newAttrs;
    attrs[len] = attr;
    len++;
    attr->_element = element;
    element->parseAttribute(attr);
    element->setChanged(true);
    return 0;
}

NodeImpl *NamedAttrMapImpl::removeNamedItem ( const DOMString &name )
{
    if (!element)
	throw DOMException(DOMException::NOT_FOUND_ERR);
    element->checkReadOnly();

    if (!attrs)
	return 0;

    uint i;
    int found = -1;
    for (i = 0; i < len && found < 0; i++) {
	if (!strcasecmp(attrs[i]->name(),name))
	    found = i;
    }
    if (found < 0)
	return 0;

    AttrImpl *ret = attrs[found];
    ret->_element = 0;
    if (len == 1) {
	delete attrs;
	attrs = 0;
	len = 0;
	AttrImpl a(name,"",element->ownerDocument());
	element->parseAttribute(&a);
	element->setChanged(true);
	return ret;
    }

    AttrImpl **newAttrs = new AttrImpl* [len-1];
    for (i = 0; i < uint(found); i++)
        newAttrs[i] = attrs[i];
    len--;
    for (; i < len; i++)
	newAttrs[i] = attrs[i+1];
    delete attrs;
    attrs = newAttrs;
	
    AttrImpl a(name,"",element->ownerDocument());
    element->parseAttribute(&a);
    element->setChanged(true);

    return ret;
}

AttrImpl *NamedAttrMapImpl::removeIdItem ( int id )
{
    if (!element)
	throw DOMException(DOMException::NOT_FOUND_ERR);
    element->checkReadOnly();

    if (!attrs)
	return 0;

    uint i;
    int found = -1;
    for (i = 0; i < len && found < 0; i++) {
	if (attrs[i]->attrId == id)
	    found = i;
    }
    if (found < 0)
	return 0;

    AttrImpl *ret = attrs[found];
    ret->_element = 0;
    if (len == 1) {
	delete attrs;
	attrs = 0;
	len = 0;
	AttrImpl a(id,"",element->ownerDocument());
	element->parseAttribute(&a);
	element->setChanged(true);
	return ret;
    }

    AttrImpl **newAttrs = new AttrImpl* [len-1];
    for (i = 0; i < uint(found); i++)
        newAttrs[i] = attrs[i];
    len--;
    for (; i < len; i++)
	newAttrs[i] = attrs[i+1];
    delete attrs;
    attrs = newAttrs;
	
    AttrImpl a(id,"",element->ownerDocument());
    element->parseAttribute(&a);
    element->setChanged(true);

    return ret;
}

NodeImpl *NamedAttrMapImpl::item ( unsigned long index ) const
{
    if (!element)
	throw DOMException(DOMException::NOT_FOUND_ERR);

    if (index >= len)
	return 0;
    else
	return attrs[index];
}

void NamedAttrMapImpl::clearAttrs()
{
    if (attrs) {
	uint i;
	for (i = 0; i < len; i++) {
	    attrs[i]->_element = 0;
	    if (attrs[i]->deleteMe())
		delete attrs[i];
	}
	delete attrs;
	attrs = 0;
    }
    len = 0;
}

AttrImpl *NamedAttrMapImpl::removeAttr( AttrImpl *oldAttr )
{
    if (!element)
	throw DOMException(DOMException::NOT_FOUND_ERR);

    uint i;
    for (i = 0; i < len; i++) {
	if (attrs[i] == oldAttr) {
	    AttrImpl *ret = attrs[i];
	
	    if (len == 1) {
		delete attrs;
		attrs = 0;
		len = 0;
	    }
	    else {
		AttrImpl **newAttrs = new AttrImpl* [len-1];
		uint ni;
		for (ni = 0; ni < i; ni++)
		    newAttrs[ni] = attrs[ni];
		len--;
		for (; ni < len; ni++)
		    newAttrs[ni] = attrs[ni+1];
		delete attrs;
		attrs = newAttrs;
	    }
	    	
	    ret->_element = 0;
	    AttrImpl a = oldAttr->attrId ? AttrImpl(oldAttr->attrId,"",element->ownerDocument()) :
	                               AttrImpl(oldAttr->name(),"",element->ownerDocument());
	    element->parseAttribute(&a);
	    element->setChanged(true);
	    return ret;
	}
    }
    throw DOMException(DOMException::NOT_FOUND_ERR);
    return 0;
	
}

void NamedAttrMapImpl::detachFromElement()
{
    if (!element)
	throw DOMException(DOMException::NOT_FOUND_ERR);

    // we allow a NamedAttrMapImpl w/o an element in case someone still has a reference
    // to if after the element gets deleted - but the map is now invalid
    element = 0;
}



