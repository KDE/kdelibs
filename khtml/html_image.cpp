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
#include "html_image.h"
#include "html_imageimpl.h"
#include "html_misc.h"
#include "html_miscimpl.h"
using namespace DOM;


HTMLAreaElement::HTMLAreaElement() : HTMLElement()
{
}

HTMLAreaElement::HTMLAreaElement(const HTMLAreaElement &other) : HTMLElement(other)
{
}

HTMLAreaElement::HTMLAreaElement(HTMLAreaElementImpl *impl) : HTMLElement(impl)
{
}

HTMLAreaElement &HTMLAreaElement::operator = (const HTMLAreaElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLAreaElement::~HTMLAreaElement()
{
}

DOMString HTMLAreaElement::accessKey() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("accessKey");
}

void HTMLAreaElement::setAccessKey( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("accessKey", value);
}

DOMString HTMLAreaElement::alt() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("alt");
}

void HTMLAreaElement::setAlt( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("alt", value);
}

DOMString HTMLAreaElement::coords() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("coords");
}

void HTMLAreaElement::setCoords( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("coords", value);
}

DOMString HTMLAreaElement::href() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("href");
}

void HTMLAreaElement::setHref( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("href", value);
}

bool HTMLAreaElement::noHref() const
{
    if(!impl) return 0;
    return ((HTMLAreaElementImpl *)impl)->noHref();
}

void HTMLAreaElement::setNoHref( const bool &_noHref )
{

    if(impl)
        ((HTMLAreaElementImpl *)impl)->setNoHref( _noHref );
}

DOMString HTMLAreaElement::shape() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("shape");
}

void HTMLAreaElement::setShape( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("shape", value);
}

long HTMLAreaElement::tabIndex() const
{
    if(!impl) return 0;
    return ((HTMLAreaElementImpl *)impl)->tabIndex();
}

void HTMLAreaElement::setTabIndex( const long &_tabIndex )
{

    if(impl)
        ((HTMLAreaElementImpl *)impl)->setTabIndex( _tabIndex );
}

DOMString HTMLAreaElement::target() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("target");
}

void HTMLAreaElement::setTarget( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("target", value);
}

// --------------------------------------------------------------------------

HTMLImageElement::HTMLImageElement() : HTMLElement()
{
}

HTMLImageElement::HTMLImageElement(const HTMLImageElement &other) : HTMLElement(other)
{
}

HTMLImageElement::HTMLImageElement(HTMLImageElementImpl *impl) : HTMLElement(impl)
{
}

HTMLImageElement &HTMLImageElement::operator = (const HTMLImageElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLImageElement::~HTMLImageElement()
{
}

DOMString HTMLImageElement::lowSrc() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("lowSrc");
}

void HTMLImageElement::setLowSrc( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("lowSrc", value);
}

DOMString HTMLImageElement::name() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("name");
}

void HTMLImageElement::setName( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("name", value);
}

DOMString HTMLImageElement::align() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("align");
}

void HTMLImageElement::setAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("align", value);
}

DOMString HTMLImageElement::alt() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("alt");
}

void HTMLImageElement::setAlt( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("alt", value);
}

DOMString HTMLImageElement::border() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("border");
}

void HTMLImageElement::setBorder( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("border", value);
}

DOMString HTMLImageElement::height() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("height");
}

void HTMLImageElement::setHeight( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("height", value);
}

DOMString HTMLImageElement::hspace() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("hspace");
}

void HTMLImageElement::setHspace( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("hspace", value);
}

bool HTMLImageElement::isMap() const
{
    if(!impl) return 0;
    return ((HTMLImageElementImpl *)impl)->isMap();
}

void HTMLImageElement::setIsMap( const bool &_isMap )
{

    if(impl)
        ((HTMLImageElementImpl *)impl)->setIsMap( _isMap );
}

DOMString HTMLImageElement::longDesc() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("longDesc");
}

void HTMLImageElement::setLongDesc( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("longDesc", value);
}

DOMString HTMLImageElement::src() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("src");
}

void HTMLImageElement::setSrc( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("src", value);
}

DOMString HTMLImageElement::useMap() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("useMap");
}

void HTMLImageElement::setUseMap( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("useMap", value);
}

DOMString HTMLImageElement::vspace() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("vspace");
}

void HTMLImageElement::setVspace( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("vspace", value);
}

DOMString HTMLImageElement::width() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("width");
}

void HTMLImageElement::setWidth( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("width", value);
}

// --------------------------------------------------------------------------

HTMLMapElement::HTMLMapElement() : HTMLElement()
{
}

HTMLMapElement::HTMLMapElement(const HTMLMapElement &other) : HTMLElement(other)
{
}

HTMLMapElement::HTMLMapElement(HTMLMapElementImpl *impl) : HTMLElement(impl)
{
}

HTMLMapElement &HTMLMapElement::operator = (const HTMLMapElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLMapElement::~HTMLMapElement()
{
}

HTMLCollection HTMLMapElement::areas() const
{
    if(!impl) return HTMLCollection();
    return HTMLCollection(impl, HTMLCollectionImpl::MAP_AREAS);
}

DOMString HTMLMapElement::name() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("name");
}

void HTMLMapElement::setName( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("name", value);
}

