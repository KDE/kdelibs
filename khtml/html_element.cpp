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
#include "html_element.h"
#include "html_elementimpl.h"
using namespace DOM;

#include "khtmlattrs.h"

HTMLElement::HTMLElement() : Element()
{
}

HTMLElement::HTMLElement(const HTMLElement &other) : Element(other)
{
}

HTMLElement::HTMLElement(HTMLElementImpl *impl) : Element(impl)
{
}

HTMLElement &HTMLElement::operator = (const HTMLElement &other)
{
    Element::operator = (other);
    return *this;
}

HTMLElement::~HTMLElement()
{
}

DOMString HTMLElement::id() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_ID);
}

void HTMLElement::setId( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_ID, value);
}

DOMString HTMLElement::title() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_TITLE);
}

void HTMLElement::setTitle( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_TITLE, value);
}

DOMString HTMLElement::lang() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_LANG);
}

void HTMLElement::setLang( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_LANG, value);
}

DOMString HTMLElement::dir() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_DIR);
}

void HTMLElement::setDir( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_DIR, value);
}

DOMString HTMLElement::className() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_CLASS);
}

void HTMLElement::setClassName( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_CLASS, value);
}

