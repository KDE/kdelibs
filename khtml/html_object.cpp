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
#include "html_object.h"

#include "dom_string.h"
#include "html_misc.h"
#include "html_miscimpl.h"
#include "html_objectimpl.h"
#include "html_element.h"
#include "html_elementimpl.h"

HTMLAppletElement::HTMLAppletElement() : HTMLElement()
{
}

HTMLAppletElement::HTMLAppletElement(const HTMLAppletElement &other)
    : HTMLElement(other)
{
}

HTMLAppletElement::HTMLAppletElement(HTMLAppletElementImpl *impl)
    : HTMLElement(impl)
{
}

HTMLAppletElement &HTMLAppletElement::operator = (const HTMLAppletElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLAppletElement::~HTMLAppletElement()
{
}

DOMString HTMLAppletElement::align() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("align");
}

void HTMLAppletElement::setAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("align", value);
}

DOMString HTMLAppletElement::alt() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("alt");
}

void HTMLAppletElement::setAlt( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("alt", value);
}

DOMString HTMLAppletElement::archive() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("archive");
}

void HTMLAppletElement::setArchive( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("archive", value);
}

DOMString HTMLAppletElement::code() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("code");
}

void HTMLAppletElement::setCode( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("code", value);
}

DOMString HTMLAppletElement::codeBase() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("codeBase");
}

void HTMLAppletElement::setCodeBase( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("codeBase", value);
}

DOMString HTMLAppletElement::height() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("height");
}

void HTMLAppletElement::setHeight( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("height", value);
}

DOMString HTMLAppletElement::hspace() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("hspace");
}

void HTMLAppletElement::setHspace( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("hspace", value);
}

DOMString HTMLAppletElement::name() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("name");
}

void HTMLAppletElement::setName( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("name", value);
}

DOMString HTMLAppletElement::object() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("object");
}

void HTMLAppletElement::setObject( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("object", value);
}

DOMString HTMLAppletElement::vspace() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("vspace");
}

void HTMLAppletElement::setVspace( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("vspace", value);
}

DOMString HTMLAppletElement::width() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("width");
}

void HTMLAppletElement::setWidth( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("width", value);
}

// --------------------------------------------------------------------------

HTMLObjectElement::HTMLObjectElement() : HTMLElement()
{
}

HTMLObjectElement::HTMLObjectElement(const HTMLObjectElement &other) : HTMLElement(other)
{
}

HTMLObjectElement::HTMLObjectElement(HTMLObjectElementImpl *impl) : HTMLElement(impl)
{
}

HTMLObjectElement &HTMLObjectElement::operator = (const HTMLObjectElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLObjectElement::~HTMLObjectElement()
{
}

HTMLFormElement HTMLObjectElement::form() const
{
    if(!impl) return 0;
    return ((HTMLObjectElementImpl *)impl)->form();
}

DOMString HTMLObjectElement::code() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("code");
}

void HTMLObjectElement::setCode( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("code", value);
}

DOMString HTMLObjectElement::align() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("align");
}

void HTMLObjectElement::setAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("align", value);
}

DOMString HTMLObjectElement::archive() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("archive");
}

void HTMLObjectElement::setArchive( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("archive", value);
}

DOMString HTMLObjectElement::border() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("border");
}

void HTMLObjectElement::setBorder( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("border", value);
}

DOMString HTMLObjectElement::codeBase() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("codeBase");
}

void HTMLObjectElement::setCodeBase( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("codeBase", value);
}

DOMString HTMLObjectElement::codeType() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("codeType");
}

void HTMLObjectElement::setCodeType( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("codeType", value);
}

DOMString HTMLObjectElement::data() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("data");
}

void HTMLObjectElement::setData( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("data", value);
}

bool HTMLObjectElement::declare() const
{
    if(!impl) return 0;
    return ((HTMLObjectElementImpl *)impl)->declare();
}

void HTMLObjectElement::setDeclare( const bool &_declare )
{

    if(impl)
        ((HTMLObjectElementImpl *)impl)->setDeclare( _declare );
}

DOMString HTMLObjectElement::height() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("height");
}

void HTMLObjectElement::setHeight( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("height", value);
}

DOMString HTMLObjectElement::hspace() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("hspace");
}

void HTMLObjectElement::setHspace( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("hspace", value);
}

DOMString HTMLObjectElement::name() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("name");
}

void HTMLObjectElement::setName( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("name", value);
}

DOMString HTMLObjectElement::standby() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("standby");
}

void HTMLObjectElement::setStandby( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("standby", value);
}

long HTMLObjectElement::tabIndex() const
{
    if(!impl) return 0;
    return ((HTMLObjectElementImpl *)impl)->tabIndex();
}

void HTMLObjectElement::setTabIndex( const long &_tabIndex )
{

    if(impl)
        ((HTMLObjectElementImpl *)impl)->setTabIndex( _tabIndex );
}

DOMString HTMLObjectElement::type() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("type");
}

void HTMLObjectElement::setType( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("type", value);
}

DOMString HTMLObjectElement::useMap() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("useMap");
}

void HTMLObjectElement::setUseMap( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("useMap", value);
}

DOMString HTMLObjectElement::vspace() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("vspace");
}

void HTMLObjectElement::setVspace( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("vspace", value);
}

DOMString HTMLObjectElement::width() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("width");
}

void HTMLObjectElement::setWidth( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("width", value);
}

// --------------------------------------------------------------------------

HTMLParamElement::HTMLParamElement() : HTMLElement()
{
}

HTMLParamElement::HTMLParamElement(const HTMLParamElement &other) : HTMLElement(other)
{
}

HTMLParamElement::HTMLParamElement(HTMLParamElementImpl *impl) : HTMLElement(impl)
{
}

HTMLParamElement &HTMLParamElement::operator = (const HTMLParamElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLParamElement::~HTMLParamElement()
{
}

DOMString HTMLParamElement::name() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("name");
}

void HTMLParamElement::setName( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("name", value);
}

DOMString HTMLParamElement::type() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("type");
}

void HTMLParamElement::setType( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("type", value);
}

DOMString HTMLParamElement::value() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("value");
}

void HTMLParamElement::setValue( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("value", value);
}

DOMString HTMLParamElement::valueType() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("valueType");
}

void HTMLParamElement::setValueType( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("valueType", value);
}

