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
#include "rendering/render_style.h"
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

AttrImpl::AttrImpl(DocumentImpl *doc, const DOMString &name) : NodeImpl(doc)
{
    _specified = false;
    _parent = 0;
    attr.setName(name);
}

AttrImpl::AttrImpl(const AttrImpl &other) : NodeImpl(other)
{
    attr = other.attr;
    _specified = other.specified();
    _parent = 0; // this is a clone
}

AttrImpl &AttrImpl::operator = (const AttrImpl &other)
{
    attr = other.attr;
    _specified = other.specified();
    document = other.ownerDocument();
    _parent = 0; // this is a clone...

    return *this;
}

AttrImpl::~AttrImpl()
{
    if(_parent) _parent->deref();
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
    return attr.name();
}

bool AttrImpl::specified() const
{
    return _specified;
}

DOMString AttrImpl::value() const
{
    return attr.value();
}

void AttrImpl::setValue( const DOMString &v )
{
    // according to the DOM docs, we should create an unparsed Text child
    // node here; we decided this was not necessary for HTML

    // TODO: parse value string, interprete entities
    attr.setValue(v);
    _specified = true;
}

DOMString AttrImpl::nodeValue() const
{
    return attr.value();
}

void AttrImpl::setNodeValue( const DOMString &v )
{
    setValue( v );
}

AttrImpl::AttrImpl(const DOMString &name, const DOMString &value,
		   DocumentImpl *doc, bool specified) : NodeImpl(doc)
{
    attr = khtml::Attribute(name, value);
    _parent = 0;
    _specified = specified;
}

NodeImpl *AttrImpl::parentNode() const
{
    return _parent;
}

void AttrImpl::setParent(NodeImpl *n)
{
    if(_parent) _parent->deref();
    _parent = n;
    if(_parent) _parent->ref();
}

// -------------------------------------------------------------------------

ElementImpl::ElementImpl(DocumentImpl *doc) : NodeBaseImpl(doc)
{
    m_style = 0;
}

ElementImpl::~ElementImpl()
{
    delete m_style;
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
    int index = attributeMap.find(name);
    if (index != -1) return attributeMap.value(index);

    if(!defaultMap()) return 0;
    // then search in default attr in case it is not yet set
    index = defaultMap()->find(name);
    if (index != -1) return defaultMap()->value(index);

    return 0;
}

DOMString ElementImpl::getAttribute( int id )
{
    // search in already set attributes first
    int index = attributeMap.find(id);
    if (index != -1) return attributeMap.value(index);

    if(!defaultMap()) return 0;
    // then search in default attr in case it is not yet set
    index = defaultMap()->find(id);
    if (index != -1) return defaultMap()->value(index);

    return 0;
}

AttrImpl *ElementImpl::getAttributeNode ( int index )
{
    return new AttrImpl(attributeMap.name(index) , attributeMap.value(index), document, true);
}

int ElementImpl::getAttributeCount()
{
    return attributeMap.length();
}

void ElementImpl::setAttribute( const DOMString &name, const DOMString &value )
{
    if (!value.implementation()) {
	removeAttribute(name);
	return;
    }

    checkReadOnly();
    // TODO: check for invalid characters in value -> throw exception
    khtml::Attribute a(name, value);
    attributeMap.add(a);

    parseAttribute(&a);
}

void ElementImpl::setAttribute( int id, const DOMString &value )
{
    checkReadOnly();
    // TODO: check for invalid characters in value -> throw exception
    khtml::Attribute a(id, value);
    if (!value.implementation()) {
	removeAttribute(a.name());
	return;
    }
    attributeMap.add(a);

    parseAttribute(&a);
    applyChanges();
}

void ElementImpl::setAttribute( AttributeList list )
{
    attributeMap = list;

    int i = 0;
    while((uint)i < attributeMap.length())
    {
	parseAttribute(attributeMap[i]);
	i++;
    }
    // used internally, during parsing. Don't call applyChages here!
    //applyChanges();
}

void ElementImpl::removeAttribute( const DOMString &name )
{
    checkReadOnly();
    attributeMap.remove(name);

    // ### defaultMap!!!
    khtml::Attribute a(name, "");
    parseAttribute(&a);
    applyChanges();
}

NodeImpl *ElementImpl::cloneNode ( bool deep )
{
    ElementImpl *newImpl = document->createElement(tagName());
    if (!newImpl)
      return 0;

    newImpl->setParent(0);
    newImpl->setFirstChild(0);
    newImpl->setLastChild(0);

    newImpl->attributeMap = attributeMap;
    unsigned int mapLength = newImpl->attributeMap.length();
    unsigned int i;
    for (i = 0; i < mapLength; i++)
	newImpl->parseAttribute(newImpl->attributeMap[i]);

    if(deep)
    {
	NodeImpl *n;
	for(n = firstChild(); n != lastChild(); n = n->nextSibling())
	{
	    newImpl->appendChild(n->cloneNode(deep));
	}
    }
    return newImpl;
}

// pay attention to memory leaks here!
AttrImpl *ElementImpl::getAttributeNode( const DOMString &name )
{
    // search in already set attributes first
    int index = attributeMap.find(name);
    if (index != -1)
	return new AttrImpl(name, attributeMap.value(index), document, true);

    // then search in default attr in case it is not yet set
    index = defaultMap()->find(name);
    if (index != -1)
	return new AttrImpl(name, defaultMap()->value(index), document, false);

    return 0;
}

AttrImpl *ElementImpl::setAttributeNode( AttrImpl *newAttr )
{
    checkReadOnly();
    checkSameDocument(newAttr);

    // #### Fix this...
    //if (newAttr->isInUse())
    //  if (newAttr->father!=this)
    //    throw DOMException(INUSE_ATTRIBUTE_ERR);

    khtml::Attribute a(newAttr->attrId(), newAttr->value());
    attributeMap.add(a);
    parseAttribute(&a);
    applyChanges();

    return newAttr;
}

AttrImpl *ElementImpl::removeAttributeNode( AttrImpl *oldAttr )
{
    checkReadOnly();
    int index = attributeMap.find(oldAttr->name());
    if (index == -1)
    {
	// _maybe_ we should also throw.. if default attr exists?
	index = defaultMap()->find(oldAttr->name());
	if (index == -1)
	    throw DOMException(DOMException::NOT_FOUND_ERR);
    }
    attributeMap.remove(index);

    khtml::Attribute a(oldAttr->name(), "");
    parseAttribute(&a);
    applyChanges();

    return oldAttr;
}

NodeListImpl *ElementImpl::getElementsByTagName( const DOMString &/*name*/ )
{
    // ### TODO: make NodeList able to handle NodeList with specific tag
    // i.e. with constructor which takes another argument (the tag)
    //return new NodeListImpl(this, name);
    return 0;
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

void ElementImpl::applyChanges(bool top)
{

    // ### find a better way to handle non-css attributes
    if(!m_render) return;
    bool faf = m_style->flowAroundFloats();
    delete m_style;
    m_style = document->styleSelector()->styleForElement(this);

    m_style->setFlowAroundFloats(faf);
    m_render->setStyle(m_style);


    // a style change can influence the children, so we just go
    // through them and trigger an appplyChanges there too
    NodeImpl *n = _first;
    while(n) {
	n->applyChanges(false);
	n = n->nextSibling();
    }

    // calc min and max widths starting from leafs
    // might belong to renderer, but this is simple to do here
    m_render->calcMinMaxWidth();

    if(top) {
	// force a relayout of this part of the document
	m_render->updateSize();
	// force a repaint of this part.
	// ### if updateSize() changes any size, it will already force a
	// repaint, so we might do double work here...
	m_render->repaint();
    }
}

const AttributeList ElementImpl::getAttributes()
{
    return attributeMap;
}

