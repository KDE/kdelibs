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
// --------------------------------------------------------------------------

#include "dom_string.h"

#include "html_base.h"
#include "html_baseimpl.h"
using namespace DOM;


HTMLBodyElement::HTMLBodyElement() : HTMLElement()
{
}

HTMLBodyElement::HTMLBodyElement(const HTMLBodyElement &other) : HTMLElement(other)
{
}

HTMLBodyElement::HTMLBodyElement(HTMLBodyElementImpl *impl) : HTMLElement(impl)
{
}

HTMLBodyElement &HTMLBodyElement::operator = (const HTMLBodyElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLBodyElement::~HTMLBodyElement()
{
}

DOMString HTMLBodyElement::aLink() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("aLink");
}

void HTMLBodyElement::setALink( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("aLink", value);
}

DOMString HTMLBodyElement::background() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("background");
}

void HTMLBodyElement::setBackground( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("background", value);
}

DOMString HTMLBodyElement::bgColor() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("bgColor");
}

void HTMLBodyElement::setBgColor( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("bgColor", value);
}

DOMString HTMLBodyElement::link() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("link");
}

void HTMLBodyElement::setLink( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("link", value);
}

DOMString HTMLBodyElement::text() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("text");
}

void HTMLBodyElement::setText( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("text", value);
}

DOMString HTMLBodyElement::vLink() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("vLink");
}

void HTMLBodyElement::setVLink( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("vLink", value);
}

// --------------------------------------------------------------------------

HTMLFrameElement::HTMLFrameElement() : HTMLElement()
{
}

HTMLFrameElement::HTMLFrameElement(const HTMLFrameElement &other) : HTMLElement(other)
{
}

HTMLFrameElement::HTMLFrameElement(HTMLFrameElementImpl *impl) : HTMLElement(impl)
{
}

HTMLFrameElement &HTMLFrameElement::operator = (const HTMLFrameElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLFrameElement::~HTMLFrameElement()
{
}

DOMString HTMLFrameElement::frameBorder() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("frameBorder");
}

void HTMLFrameElement::setFrameBorder( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("frameBorder", value);
}

DOMString HTMLFrameElement::longDesc() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("longDesc");
}

void HTMLFrameElement::setLongDesc( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("longDesc", value);
}

DOMString HTMLFrameElement::marginHeight() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("marginHeight");
}

void HTMLFrameElement::setMarginHeight( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("marginHeight", value);
}

DOMString HTMLFrameElement::marginWidth() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("marginWidth");
}

void HTMLFrameElement::setMarginWidth( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("marginWidth", value);
}

DOMString HTMLFrameElement::name() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("name");
}

void HTMLFrameElement::setName( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("name", value);
}

bool HTMLFrameElement::noResize() const
{
    if(!impl) return 0;
    return ((HTMLFrameElementImpl *)impl)->noResize();
}

void HTMLFrameElement::setNoResize( const bool &_noResize )
{

    if(impl)
        ((HTMLFrameElementImpl *)impl)->setNoResize( _noResize );
}

DOMString HTMLFrameElement::scrolling() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("scrolling");
}

void HTMLFrameElement::setScrolling( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("scrolling", value);
}

DOMString HTMLFrameElement::src() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("src");
}

void HTMLFrameElement::setSrc( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("src", value);
}

// --------------------------------------------------------------------------

HTMLFrameSetElement::HTMLFrameSetElement() : HTMLElement()
{
}

HTMLFrameSetElement::HTMLFrameSetElement(const HTMLFrameSetElement &other) : HTMLElement(other)
{
}

HTMLFrameSetElement::HTMLFrameSetElement(HTMLFrameSetElementImpl *impl) : HTMLElement(impl)
{
}

HTMLFrameSetElement &HTMLFrameSetElement::operator = (const HTMLFrameSetElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLFrameSetElement::~HTMLFrameSetElement()
{
}

DOMString HTMLFrameSetElement::cols() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("cols");
}

void HTMLFrameSetElement::setCols( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("cols", value);
}

DOMString HTMLFrameSetElement::rows() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("rows");
}

void HTMLFrameSetElement::setRows( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("rows", value);
}

// --------------------------------------------------------------------------

HTMLHeadElement::HTMLHeadElement() : HTMLElement()
{
}

HTMLHeadElement::HTMLHeadElement(const HTMLHeadElement &other) : HTMLElement(other)
{
}

HTMLHeadElement::HTMLHeadElement(HTMLHeadElementImpl *impl) : HTMLElement(impl)
{
}

HTMLHeadElement &HTMLHeadElement::operator = (const HTMLHeadElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLHeadElement::~HTMLHeadElement()
{
}

DOMString HTMLHeadElement::profile() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("profile");
}

void HTMLHeadElement::setProfile( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("profile", value);
}

// --------------------------------------------------------------------------

HTMLHtmlElement::HTMLHtmlElement() : HTMLElement()
{
}

HTMLHtmlElement::HTMLHtmlElement(const HTMLHtmlElement &other) : HTMLElement(other)
{
}

HTMLHtmlElement::HTMLHtmlElement(HTMLHtmlElementImpl *impl) : HTMLElement(impl)
{
}

HTMLHtmlElement &HTMLHtmlElement::operator = (const HTMLHtmlElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLHtmlElement::~HTMLHtmlElement()
{
}

DOMString HTMLHtmlElement::version() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("version");
}

void HTMLHtmlElement::setVersion( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("version", value);
}

