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

#include "html_block.h"
#include "html_blockimpl.h"
using namespace DOM;


HTMLBlockquoteElement::HTMLBlockquoteElement() : HTMLElement()
{
}

HTMLBlockquoteElement::HTMLBlockquoteElement(const HTMLBlockquoteElement &other) : HTMLElement(other)
{
}

HTMLBlockquoteElement::HTMLBlockquoteElement(HTMLBlockquoteElementImpl *impl) : HTMLElement(impl)
{
}

HTMLBlockquoteElement &HTMLBlockquoteElement::operator = (const HTMLBlockquoteElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLBlockquoteElement::~HTMLBlockquoteElement()
{
}

DOMString HTMLBlockquoteElement::cite() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("cite");
}

void HTMLBlockquoteElement::setCite( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("cite", value);
}

// --------------------------------------------------------------------------

HTMLDivElement::HTMLDivElement() : HTMLElement()
{
}

HTMLDivElement::HTMLDivElement(const HTMLDivElement &other) : HTMLElement(other)
{
}

HTMLDivElement::HTMLDivElement(HTMLDivElementImpl *impl) : HTMLElement(impl)
{
}

HTMLDivElement &HTMLDivElement::operator = (const HTMLDivElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLDivElement::~HTMLDivElement()
{
}

DOMString HTMLDivElement::align() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("align");
}

void HTMLDivElement::setAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("align", value);
}

// --------------------------------------------------------------------------

HTMLHRElement::HTMLHRElement() : HTMLElement()
{
}

HTMLHRElement::HTMLHRElement(const HTMLHRElement &other) : HTMLElement(other)
{
}

HTMLHRElement::HTMLHRElement(HTMLHRElementImpl *impl) : HTMLElement(impl)
{
}

HTMLHRElement &HTMLHRElement::operator = (const HTMLHRElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLHRElement::~HTMLHRElement()
{
}

DOMString HTMLHRElement::align() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("align");
}

void HTMLHRElement::setAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("align", value);
}

bool HTMLHRElement::noShade() const
{
    if(!impl) return 0;
    return ((HTMLHRElementImpl *)impl)->noShade();
}

void HTMLHRElement::setNoShade( bool _noShade )
{

    if(impl)
        ((HTMLHRElementImpl *)impl)->setNoShade( _noShade );
}

DOMString HTMLHRElement::size() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("size");
}

void HTMLHRElement::setSize( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("size", value);
}

DOMString HTMLHRElement::width() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("width");
}

void HTMLHRElement::setWidth( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("width", value);
}

// --------------------------------------------------------------------------

HTMLHeadingElement::HTMLHeadingElement() : HTMLElement()
{
}

HTMLHeadingElement::HTMLHeadingElement(const HTMLHeadingElement &other) : HTMLElement(other)
{
}

HTMLHeadingElement::HTMLHeadingElement(HTMLHeadingElementImpl *impl) : HTMLElement(impl)
{
}

HTMLHeadingElement &HTMLHeadingElement::operator = (const HTMLHeadingElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLHeadingElement::~HTMLHeadingElement()
{
}

DOMString HTMLHeadingElement::align() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("align");
}

void HTMLHeadingElement::setAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("align", value);
}

// --------------------------------------------------------------------------

HTMLParagraphElement::HTMLParagraphElement() : HTMLElement()
{
}

HTMLParagraphElement::HTMLParagraphElement(const HTMLParagraphElement &other) : HTMLElement(other)
{
}

HTMLParagraphElement::HTMLParagraphElement(HTMLParagraphElementImpl *impl) : HTMLElement(impl)
{
}

HTMLParagraphElement &HTMLParagraphElement::operator = (const HTMLParagraphElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLParagraphElement::~HTMLParagraphElement()
{
}

DOMString HTMLParagraphElement::align() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("align");
}

void HTMLParagraphElement::setAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("align", value);
}

// --------------------------------------------------------------------------

HTMLPreElement::HTMLPreElement() : HTMLElement()
{
}

HTMLPreElement::HTMLPreElement(const HTMLPreElement &other) : HTMLElement(other)
{
}

HTMLPreElement::HTMLPreElement(HTMLPreElementImpl *impl) : HTMLElement(impl)
{
}

HTMLPreElement &HTMLPreElement::operator = (const HTMLPreElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLPreElement::~HTMLPreElement()
{
}

long HTMLPreElement::width() const
{
    if(!impl) return 0;
    return ((HTMLPreElementImpl *)impl)->width();
}

void HTMLPreElement::setWidth( long _width )
{

    if(impl)
        ((HTMLPreElementImpl *)impl)->setWidth( _width );
}

