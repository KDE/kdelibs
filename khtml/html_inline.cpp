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
#include "html_inline.h"
#include "html_inlineimpl.h"
using namespace DOM;


HTMLAnchorElement::HTMLAnchorElement() : HTMLElement()
{
}

HTMLAnchorElement::HTMLAnchorElement(const HTMLAnchorElement &other) : HTMLElement(other)
{
}

HTMLAnchorElement::HTMLAnchorElement(HTMLAnchorElementImpl *impl) : HTMLElement(impl)
{
}

HTMLAnchorElement &HTMLAnchorElement::operator = (const HTMLAnchorElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLAnchorElement::~HTMLAnchorElement()
{
}

DOMString HTMLAnchorElement::accessKey() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("accessKey");
}

void HTMLAnchorElement::setAccessKey( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("accessKey", value);
}

DOMString HTMLAnchorElement::charset() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("charset");
}

void HTMLAnchorElement::setCharset( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("charset", value);
}

DOMString HTMLAnchorElement::coords() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("coords");
}

void HTMLAnchorElement::setCoords( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("coords", value);
}

DOMString HTMLAnchorElement::href() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("href");
}

void HTMLAnchorElement::setHref( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("href", value);
}

DOMString HTMLAnchorElement::hreflang() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("hreflang");
}

void HTMLAnchorElement::setHreflang( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("hreflang", value);
}

DOMString HTMLAnchorElement::name() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("name");
}

void HTMLAnchorElement::setName( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("name", value);
}

DOMString HTMLAnchorElement::rel() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("rel");
}

void HTMLAnchorElement::setRel( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("rel", value);
}

DOMString HTMLAnchorElement::rev() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("rev");
}

void HTMLAnchorElement::setRev( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("rev", value);
}

DOMString HTMLAnchorElement::shape() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("shape");
}

void HTMLAnchorElement::setShape( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("shape", value);
}

long HTMLAnchorElement::tabIndex() const
{
    if(!impl) return 0;
    return ((HTMLAnchorElementImpl *)impl)->tabIndex();
}

void HTMLAnchorElement::setTabIndex( const long &_tabIndex )
{

    if(impl)
        ((HTMLAnchorElementImpl *)impl)->setTabIndex( _tabIndex );
}

DOMString HTMLAnchorElement::target() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("target");
}

void HTMLAnchorElement::setTarget( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("target", value);
}

DOMString HTMLAnchorElement::type() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("type");
}

void HTMLAnchorElement::setType( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("type", value);
}

void HTMLAnchorElement::blur(  )
{
    if(impl)
        ((HTMLAnchorElementImpl *)impl)->blur(  );
}

void HTMLAnchorElement::focus(  )
{
    if(impl)
        ((HTMLAnchorElementImpl *)impl)->focus(  );
}

// --------------------------------------------------------------------------

HTMLBRElement::HTMLBRElement() : HTMLElement()
{
}

HTMLBRElement::HTMLBRElement(const HTMLBRElement &other) : HTMLElement(other)
{
}

HTMLBRElement::HTMLBRElement(HTMLBRElementImpl *impl) : HTMLElement(impl)
{
}

HTMLBRElement &HTMLBRElement::operator = (const HTMLBRElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLBRElement::~HTMLBRElement()
{
}

DOMString HTMLBRElement::clear() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("clear");
}

void HTMLBRElement::setClear( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("clear", value);
}

// --------------------------------------------------------------------------

HTMLFontElement::HTMLFontElement() : HTMLElement()
{
}

HTMLFontElement::HTMLFontElement(const HTMLFontElement &other) : HTMLElement(other)
{
}

HTMLFontElement::HTMLFontElement(HTMLFontElementImpl *impl) : HTMLElement(impl)
{
}

HTMLFontElement &HTMLFontElement::operator = (const HTMLFontElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLFontElement::~HTMLFontElement()
{
}

DOMString HTMLFontElement::color() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("color");
}

void HTMLFontElement::setColor( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("color", value);
}

DOMString HTMLFontElement::face() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("face");
}

void HTMLFontElement::setFace( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("face", value);
}

DOMString HTMLFontElement::size() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("size");
}

void HTMLFontElement::setSize( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("size", value);
}

// --------------------------------------------------------------------------

HTMLIFrameElement::HTMLIFrameElement() : HTMLElement()
{
}

HTMLIFrameElement::HTMLIFrameElement(const HTMLIFrameElement &other) : HTMLElement(other)
{
}

HTMLIFrameElement::HTMLIFrameElement(HTMLIFrameElementImpl *impl) : HTMLElement(impl)
{
}

HTMLIFrameElement &HTMLIFrameElement::operator = (const HTMLIFrameElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLIFrameElement::~HTMLIFrameElement()
{
}

DOMString HTMLIFrameElement::align() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("align");
}

void HTMLIFrameElement::setAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("align", value);
}

DOMString HTMLIFrameElement::frameBorder() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("frameBorder");
}

void HTMLIFrameElement::setFrameBorder( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("frameBorder", value);
}

DOMString HTMLIFrameElement::height() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("height");
}

void HTMLIFrameElement::setHeight( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("height", value);
}

DOMString HTMLIFrameElement::longDesc() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("longDesc");
}

void HTMLIFrameElement::setLongDesc( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("longDesc", value);
}

DOMString HTMLIFrameElement::marginHeight() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("marginHeight");
}

void HTMLIFrameElement::setMarginHeight( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("marginHeight", value);
}

DOMString HTMLIFrameElement::marginWidth() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("marginWidth");
}

void HTMLIFrameElement::setMarginWidth( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("marginWidth", value);
}

DOMString HTMLIFrameElement::name() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("name");
}

void HTMLIFrameElement::setName( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("name", value);
}

DOMString HTMLIFrameElement::scrolling() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("scrolling");
}

void HTMLIFrameElement::setScrolling( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("scrolling", value);
}

DOMString HTMLIFrameElement::src() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("src");
}

void HTMLIFrameElement::setSrc( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("src", value);
}

DOMString HTMLIFrameElement::width() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("width");
}

void HTMLIFrameElement::setWidth( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("width", value);
}

// --------------------------------------------------------------------------

HTMLModElement::HTMLModElement() : HTMLElement()
{
}

HTMLModElement::HTMLModElement(const HTMLModElement &other) : HTMLElement(other)
{
}

HTMLModElement::HTMLModElement(HTMLModElementImpl *impl) : HTMLElement(impl)
{
}

HTMLModElement &HTMLModElement::operator = (const HTMLModElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLModElement::~HTMLModElement()
{
}

DOMString HTMLModElement::cite() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("cite");
}

void HTMLModElement::setCite( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("cite", value);
}

DOMString HTMLModElement::dateTime() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("dateTime");
}

void HTMLModElement::setDateTime( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("dateTime", value);
}

// --------------------------------------------------------------------------

HTMLQuoteElement::HTMLQuoteElement() : HTMLElement()
{
}

HTMLQuoteElement::HTMLQuoteElement(const HTMLQuoteElement &other) : HTMLElement(other)
{
}

HTMLQuoteElement::HTMLQuoteElement(HTMLQuoteElementImpl *impl) : HTMLElement(impl)
{
}

HTMLQuoteElement &HTMLQuoteElement::operator = (const HTMLQuoteElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLQuoteElement::~HTMLQuoteElement()
{
}

DOMString HTMLQuoteElement::cite() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("cite");
}

void HTMLQuoteElement::setCite( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("cite", value);
}

