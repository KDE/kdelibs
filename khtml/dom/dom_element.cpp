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

#include "dom/dom_exception.h"
#include "xml/dom_docimpl.h"
#include "xml/dom_elementimpl.h"
#include "html/html_formimpl.h"

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
    NodeImpl* ohandle = other.handle();
    if ( impl != ohandle ) {
        if (!ohandle || !ohandle->isAttributeNode()) {
            if (impl) impl->deref();
            impl = 0;
        } else {
            Node::operator =(other);
        }
    }
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
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    return ((AttrImpl *)impl)->name();
}

bool Attr::specified() const
{
  if (impl) return ((AttrImpl *)impl)->specified();
  return 0;
}

Element Attr::ownerElement() const
{
  if (!impl) return 0;
  return static_cast<AttrImpl*>(impl)->ownerElement();
}

DOMString Attr::value() const
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    return impl->nodeValue();
}

void Attr::setValue( const DOMString &newValue )
{
  if (!impl)
    return;

  int exceptioncode = 0;
  ((AttrImpl *)impl)->setValue(newValue,exceptioncode);
  if (exceptioncode)
    throw DOMException(exceptioncode);
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
    NodeImpl* ohandle = other.handle();
    if ( impl != ohandle ) {
        if (!ohandle || !ohandle->isElementNode()) {
            if (impl) impl->deref();
            impl = 0;
	} else {
            Node::operator =(other);
	}
    }
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
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    return static_cast<ElementImpl*>(impl)->tagName();
}

DOMString Element::getAttribute( const DOMString &name )
{
    // ### getAttribute() and getAttributeNS() are supposed to return the empty string if the attribute
    // does not exist. However, there are a number of places around khtml that expect a null string
    // for nonexistent attributes. These need to be changed to use hasAttribute() instead.
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    if (!name.implementation()) throw DOMException(DOMException::NOT_FOUND_ERR);

    NodeImpl::Id id = impl->getDocument()->attrNames()->getId(name.implementation(),true);
    if (!id) return DOMString();

    ElementImpl* e = static_cast<ElementImpl*>(impl);
    return e->getAttribute(id);
}

void Element::setAttribute( const DOMString &name, const DOMString &value )
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    NodeImpl::Id id = impl->getDocument()->attrNames()->getId(name.implementation(), false /* allocate */);

    int exceptioncode = 0;
    static_cast<ElementImpl*>(impl)->setAttribute(id,0,0,value.implementation(),exceptioncode);
    if ( exceptioncode )
        throw DOMException( exceptioncode );
}

void Element::removeAttribute( const DOMString &name )
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    NodeImpl::Id id = impl->getDocument()->attrNames()->getId(name.implementation(), true);
    if (!id) return;

    int exceptioncode = 0;
    NamedNodeMapImpl *attributes = static_cast<ElementImpl*>(impl)->attributes(false);
    attributes->removeNamedItem(id, DOMString(), DOMString(), exceptioncode);
    // ignore exceptioncode, it's allowed to remove attributes that don't exist.
}

Attr Element::getAttributeNode( const DOMString &name )
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    if (!name.implementation()) throw DOMException(DOMException::NOT_FOUND_ERR);
    NodeImpl::Id id = impl->getDocument()->attrNames()->getId(name.implementation(),true);
    if (!id) return 0;

    ElementImpl* e = static_cast<ElementImpl*>(impl);
    if (!e->attributes()) return 0;

    return static_cast<AttrImpl*>(e->attributes()->getNamedItem(id,DOMString(),DOMString()));
}

Attr Element::setAttributeNode( const Attr &newAttr )
{
    return setAttributeNodeNS(newAttr);
}

Attr Element::removeAttributeNode( const Attr &oldAttr )
{
    if (!impl || oldAttr.isNull() || oldAttr.ownerElement().handle() != impl)
        throw DOMException(DOMException::NOT_FOUND_ERR);

    if (impl->isReadOnly())
        throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR);

    if (static_cast<ElementImpl*>(impl)->attributes(true))
        throw DOMException(DOMException::NOT_FOUND_ERR);

    NamedAttrMapImpl *attributes = static_cast<ElementImpl*>(impl)->attributes(false);
    return attributes->removeAttr(static_cast<AttrImpl*>(static_cast<AttrImpl*>(oldAttr.handle())));
}

NodeList Element::getElementsByTagName( const DOMString &name )
{
    if (!impl) return 0;
    NodeImpl::Id id = impl->getDocument()->elementNames()->getId(name.implementation(), false);
    return new TagNodeListImpl( impl, id );
}

NodeList Element::getElementsByTagNameNS( const DOMString &namespaceURI,
                                          const DOMString &localName )
{
    if (!impl) return 0;
    return new TagNodeListImpl( impl, namespaceURI, localName );
}

DOMString Element::getAttributeNS( const DOMString &namespaceURI,
                                   const DOMString &localName)
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    if (!localName.implementation()) throw DOMException(DOMException::NOT_FOUND_ERR);

    ElementImpl* e = static_cast<ElementImpl*>(impl);
    return e->getAttribute(0,namespaceURI.implementation(),localName.implementation());
}

void Element::setAttributeNS( const DOMString &namespaceURI,
                              const DOMString &qualifiedName,
                              const DOMString &value)
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);

    int exceptioncode = 0;
    static_cast<ElementImpl*>(impl)->setAttribute(0,namespaceURI.implementation(),qualifiedName.implementation(),
						  value.implementation(),exceptioncode);
    if ( exceptioncode )
        throw DOMException( exceptioncode );
}

void Element::removeAttributeNS( const DOMString &namespaceURI,
                                 const DOMString &localName )
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);

    int exceptioncode = 0;
    NamedNodeMapImpl *attributes = static_cast<ElementImpl*>(impl)->attributes(false);
    attributes->removeNamedItem(0, namespaceURI, localName, exceptioncode);
    if ( exceptioncode )
        throw DOMException( exceptioncode );
}

Attr Element::getAttributeNodeNS( const DOMString &namespaceURI,
                                  const DOMString &localName )
{
    if (!impl) throw DOMException(DOMException::NOT_FOUND_ERR);
    if (!localName.implementation()) throw DOMException(DOMException::NOT_FOUND_ERR);

    ElementImpl* e = static_cast<ElementImpl*>(impl);
    if (!e->attributes()) return 0;

    return static_cast<AttrImpl*>(e->attributes()->getNamedItem(0,namespaceURI,localName));
}

Attr Element::setAttributeNodeNS( const Attr &newAttr )
{
    if (!impl || newAttr.isNull())
        throw DOMException(DOMException::NOT_FOUND_ERR);
    if (impl->getDocument() != newAttr.handle()->getDocument())
        throw DOMException(DOMException::WRONG_DOCUMENT_ERR);
    if (!newAttr.ownerElement().isNull())
        throw DOMException(DOMException::INUSE_ATTRIBUTE_ERR);

    // ### respace ns/non-ns here
    int exceptioncode = 0;
    Attr r = static_cast<ElementImpl*>(impl)->attributes(false)->setNamedItem(newAttr.handle(), false, exceptioncode);
    if ( exceptioncode )
        throw DOMException( exceptioncode );
    return r;
}


bool Element::hasAttribute( const DOMString& name )
{
    if (!impl || !static_cast<ElementImpl*>(impl)->attributes()) return false; // ### throw ?
    NodeImpl::Id id = impl->getDocument()->attrNames()->getId(name.implementation(), true);
    if (!id) return false;

    if (!static_cast<ElementImpl*>(impl)->attributes(true /*readonly*/)) return false;
    return static_cast<ElementImpl*>(impl)->attributes(true)->getValue(id) != 0;
}

bool Element::hasAttributeNS( const DOMString &namespaceURI,
                              const DOMString &localName )
{
    if (!impl || !static_cast<ElementImpl*>(impl)->attributes()) return false; // ### throw ?
    if (!static_cast<ElementImpl*>(impl)->attributes(true /*readonly*/)) return false;
    return static_cast<ElementImpl*>(impl)->attributes(true)->getValue(0,namespaceURI.implementation(),
								       localName.implementation()) != 0;
}

bool Element::isHTMLElement() const
{
    if(!impl) return false;
    return ((ElementImpl *)impl)->isHTMLElement();
}

Element Element::form() const
{
    if (!impl || !impl->isGenericFormElement()) return 0;
    return static_cast<HTMLGenericFormElementImpl*>(impl)->form();
    ElementImpl* f = static_cast<HTMLGenericFormElementImpl*>( impl )->form();

    if( f && f->implicitNode() )
        return 0;
    return f;
}

CSSStyleDeclaration Element::style()
{
    if (impl) return ((ElementImpl *)impl)->styleRules();
    return 0;
}

bool Element::khtmlValidAttrName(const DOMString &/*name*/)
{
    // ###
    return true;
}

bool Element::khtmlValidPrefix(const DOMString &/*name*/)
{
    // ###
    return true;
}

bool Element::khtmlValidQualifiedName(const DOMString &/*name*/)
{
    // ###
    return true;
}

bool Element::khtmlMalformedQualifiedName(const DOMString &/*name*/)
{
    // ###
    return false;
}

bool Element::khtmlMalformedPrefix(const DOMString &/*name*/)
{
    // ###
    return false;
}
