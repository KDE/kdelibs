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

#include "dom_string.h"
#include "khtmlattrs.h"

#include "dom_elementimpl.h"
#include "dom_nodeimpl.h"
#include "dom_exception.h"
#include "dom_node.h"
#include "dom_textimpl.h"

using namespace DOM;

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
    return getAttrName(attr.id);
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

DOMString AttrImpl::nodeValue()
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
    attr = Attribute(name, value);
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
}

ElementImpl::~ElementImpl()
{
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

    // then search in default attr in case it is not yet set
    index = defaultMap()->find(id);
    if (index != -1) return defaultMap()->value(index);

    return 0;
}

void ElementImpl::setAttribute( const DOMString &name, const DOMString &value )
{
    checkReadOnly();
    // TODO: check for invalid characters in value -> throw exception
    Attribute a(name, value);
    attributeMap.add(a);

    parseAttribute(&a);
}

void ElementImpl::setAttribute( int id, const DOMString &value )
{
    checkReadOnly();
    // TODO: check for invalid characters in value -> throw exception
    Attribute a(id, value);
    attributeMap.add(a);

    parseAttribute(&a);
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
}

void ElementImpl::removeAttribute( const DOMString &name )
{
    checkReadOnly();
    attributeMap.remove(name);

    // ### defaultMap!!!
    Attribute a(name, "");
    parseAttribute(&a);
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

    Attribute a(newAttr->id(), newAttr->value());
    attributeMap.add(a);
    parseAttribute(&a);

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

    Attribute a(oldAttr->name(), "");
    parseAttribute(&a);

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

