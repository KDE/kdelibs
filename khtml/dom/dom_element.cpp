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
#include "dom_element.h"
#include "dom_elementimpl.h"
#include "dom_node.h"
#include "dom_exception.h"
using namespace DOM;

Attr::Attr() : Node()
{
}

Attr::Attr(const Attr &other) : Node(other)
{
}

Attr::Attr( AttrImpl *_impl )
{
    impl= _impl;
    if (impl) impl->ref();
}

Attr &Attr::operator = (const Node &other)
{
    if(other.nodeType() != ATTRIBUTE_NODE)
    {
	impl = 0;
	return *this;
    }
    Node::operator =(other);
    return *this;
}

Attr &Attr::operator = (const Attr &other)
{
    Node::operator =(other);
    return *this;
}

Attr::~Attr()
{
}

DOMString Attr::name() const
{
  if (impl) return ((AttrImpl *)impl)->name();
  return DOMString();
}

bool Attr::specified() const
{
  if (impl) return ((AttrImpl *)impl)->specified();
  return 0;
}

Element Attr::ownerElement() const
{
  if (impl) return ((AttrImpl *)impl)->ownerElement();
  return Element();
}

DOMString Attr::value() const
{
  if (impl) return ((AttrImpl *)impl)->value();
  return DOMString();
}

void Attr::setValue( const DOMString &newValue )
{
  if (impl) ((AttrImpl *)impl)->setValue(newValue);
}

// ---------------------------------------------------------------------------

Element::Element() : Node()
{
}

Element::Element(const Element &other) : Node(other)
{
}

Element::Element(ElementImpl *impl) : Node(impl)
{
}

Element &Element::operator = (const Node &other)
{
    if(other.nodeType() != ELEMENT_NODE)
    {
	impl = 0;
	return *this;
    }
    Node::operator =(other);
    return *this;
}

Element &Element::operator = (const Element &other)
{
    Node::operator =(other);
    return *this;
}

Element::~Element()
{
}

DOMString Element::tagName() const
{
    if (impl) return ((ElementImpl *)impl)->tagName();
    return DOMString();
}

DOMString Element::getAttribute( const DOMString &name )
{
    if (!impl)
        return DOMString(); // ### throw exception

    int exceptioncode = 0;
    DOMString r = ((ElementImpl *)impl)->getAttribute(name, exceptioncode);
    if ( exceptioncode )
        throw DOMException( exceptioncode );
    return r;
}

void Element::setAttribute( const DOMString &name, const DOMString &value )
{
    if (!impl)
        return;

    int exceptioncode = 0;
    ((ElementImpl *)impl)->setAttribute(name, value, exceptioncode);
    if ( exceptioncode )
        throw DOMException( exceptioncode );
}

void Element::removeAttribute( const DOMString &name )
{
    if (!impl)
        return; // ### throw exception

    int exceptioncode = 0;
    ((ElementImpl *)impl)->removeAttribute(name, exceptioncode);
    if ( exceptioncode )
        throw DOMException( exceptioncode );
}

Attr Element::getAttributeNode( const DOMString &name )
{
    if (!impl)
        return 0;

    int exceptioncode = 0;
    AttrImpl *r = ((ElementImpl *)impl)->getAttributeNode(name, exceptioncode);
    if ( exceptioncode )
        throw DOMException( exceptioncode );
    return r;
}

Attr Element::setAttributeNode( const Attr &newAttr )
{
    int exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
    Attr r = 0;
    if (impl)
        r = ((ElementImpl *)impl)->setAttributeNode((AttrImpl *)newAttr.impl, exceptioncode);
    if ( exceptioncode )
        throw DOMException( exceptioncode );
    return r;
}

Attr Element::removeAttributeNode( const Attr &oldAttr )
{
    int exceptioncode = DOMException::NO_MODIFICATION_ALLOWED_ERR;
    Attr r = 0;
    if (impl)
        r = ((ElementImpl *)impl)->removeAttributeNode((AttrImpl *)oldAttr.impl, exceptioncode);
    if ( exceptioncode )
        throw DOMException( exceptioncode );
    return r;
}

NodeList Element::getElementsByTagName( const DOMString &name )
{
    if (!impl)
        return 0; // ### throw exception

    int exceptioncode = 0;
    NodeListImpl *r = ((ElementImpl *)impl)->getElementsByTagName(name, exceptioncode);
    if ( exceptioncode )
        throw DOMException( exceptioncode );
    return r;
}

DOMString Element::getAttributeNS( const DOMString &namespaceURI,
                                   const DOMString &localName)
{
    if (!impl)
	throw DOMException(DOMException::NOT_FOUND_ERR);

    int exceptioncode = 0;
    DOMString r = ((ElementImpl *)impl)->getAttributeNS(namespaceURI,localName,exceptioncode);
    if ( exceptioncode )
        throw DOMException( exceptioncode );
    return r;
}

void Element::setAttributeNS( const DOMString &namespaceURI,
                              const DOMString &qualifiedName, 
                              const DOMString &value)
{
    if (!impl)
	throw DOMException(DOMException::NOT_FOUND_ERR);

    int exceptioncode = 0;
    ((ElementImpl *)impl)->setAttributeNS(namespaceURI,qualifiedName,value,exceptioncode);
    if ( exceptioncode )
        throw DOMException( exceptioncode );
}

void Element::removeAttributeNS( const DOMString &namespaceURI,
                                 const DOMString &localName )
{
    if (!impl)
	throw DOMException(DOMException::NOT_FOUND_ERR);

    int exceptioncode = 0;
    ((ElementImpl *)impl)->removeAttributeNS(namespaceURI,localName,exceptioncode);
    if ( exceptioncode )
        throw DOMException( exceptioncode );
}

Attr Element::getAttributeNodeNS( const DOMString &namespaceURI,
                                  const DOMString &localName )
{
    if (!impl)
	throw DOMException(DOMException::NOT_FOUND_ERR);

  int exceptioncode = 0;
  AttrImpl *r = ((ElementImpl *)impl)->getAttributeNodeNS(namespaceURI,localName,exceptioncode);
  if ( exceptioncode )
      throw DOMException( exceptioncode );
  return r;
}

Attr Element::setAttributeNodeNS( const Attr &newAttr )
{
    if (!impl)
	throw DOMException(DOMException::NOT_FOUND_ERR);

    int exceptioncode = 0;
    AttrImpl *r = ((ElementImpl *)impl)->setAttributeNodeNS((AttrImpl*)newAttr.handle(),exceptioncode);
    if ( exceptioncode )
        throw DOMException( exceptioncode );
    return r;
}

NodeList Element::getElementsByTagNameNS( const DOMString &namespaceURI,
                                          const DOMString &localName )
{
    if (!impl)
	throw DOMException(DOMException::NOT_FOUND_ERR);

    int exceptioncode = 0;
    NodeListImpl *r = ((ElementImpl *)impl)->getElementsByTagNameNS(namespaceURI,localName,exceptioncode);
    if ( exceptioncode )
        throw DOMException( exceptioncode );
    return r;
}

bool Element::hasAttribute( const DOMString& name )
{
    if (!impl)
        return false; // ### throw exception

    int exceptioncode = 0;
    bool r = ((ElementImpl *)impl)->hasAttribute(name, exceptioncode);
    if ( exceptioncode )
        throw DOMException( exceptioncode );
    return r;
}

bool Element::hasAttributeNS( const DOMString &namespaceURI,
                              const DOMString &localName )
{
    if (!impl)
	throw DOMException(DOMException::NOT_FOUND_ERR);

    int exceptioncode = 0;
    bool r = ((ElementImpl *)impl)->hasAttributeNS(namespaceURI,localName,exceptioncode);
    if ( exceptioncode )
        throw DOMException( exceptioncode );
    return r;
}

void Element::normalize()
{
    if (!impl)
	throw DOMException(DOMException::NOT_FOUND_ERR);

    int exceptioncode = 0;
    ((ElementImpl *)impl)->normalize(exceptioncode);
    if (exceptioncode)
	throw DOMException(exceptioncode);
}

bool Element::isHTMLElement() const
{
    if(!impl) return false;
    return ((ElementImpl *)impl)->isHTMLElement();
}

CSSStyleDeclaration Element::style()
{
    if (impl) return ((ElementImpl *)impl)->styleRules();
    return 0;
}


