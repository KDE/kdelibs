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

#include "html_list.h"
#include "html_listimpl.h"
using namespace DOM;


HTMLDListElement::HTMLDListElement() : HTMLElement()
{
}

HTMLDListElement::HTMLDListElement(const HTMLDListElement &other) : HTMLElement(other)
{
}

HTMLDListElement::HTMLDListElement(HTMLDListElementImpl *impl) : HTMLElement(impl)
{
}

HTMLDListElement &HTMLDListElement::operator = (const HTMLDListElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLDListElement::~HTMLDListElement()
{
}

bool HTMLDListElement::compact() const
{
    if(!impl) return 0;
    return ((HTMLDListElementImpl *)impl)->compact();
}

void HTMLDListElement::setCompact( const bool &_compact )
{

    if(impl)
        ((HTMLDListElementImpl *)impl)->setCompact( _compact );
}

// --------------------------------------------------------------------------

HTMLDirectoryElement::HTMLDirectoryElement() : HTMLElement()
{
}

HTMLDirectoryElement::HTMLDirectoryElement(const HTMLDirectoryElement &other) : HTMLElement(other)
{
}

HTMLDirectoryElement::HTMLDirectoryElement(HTMLDirectoryElementImpl *impl) : HTMLElement(impl)
{
}

HTMLDirectoryElement &HTMLDirectoryElement::operator = (const HTMLDirectoryElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLDirectoryElement::~HTMLDirectoryElement()
{
}

bool HTMLDirectoryElement::compact() const
{
    if(!impl) return 0;
    return ((HTMLDirectoryElementImpl *)impl)->compact();
}

void HTMLDirectoryElement::setCompact( const bool &_compact )
{

    if(impl)
        ((HTMLDirectoryElementImpl *)impl)->setCompact( _compact );
}

// --------------------------------------------------------------------------

HTMLLIElement::HTMLLIElement() : HTMLElement()
{
}

HTMLLIElement::HTMLLIElement(const HTMLLIElement &other) : HTMLElement(other)
{
}

HTMLLIElement::HTMLLIElement(HTMLLIElementImpl *impl) : HTMLElement(impl)
{
}

HTMLLIElement &HTMLLIElement::operator = (const HTMLLIElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLLIElement::~HTMLLIElement()
{
}

DOMString HTMLLIElement::type() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("type");
}

void HTMLLIElement::setType( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("type", value);
}

long HTMLLIElement::value() const
{
    if(!impl) return 0;
    return ((HTMLLIElementImpl *)impl)->value();
}

void HTMLLIElement::setValue( const long &_value )
{

    if(impl)
        ((HTMLLIElementImpl *)impl)->setValue( _value );
}

// --------------------------------------------------------------------------

HTMLMenuElement::HTMLMenuElement() : HTMLElement()
{
}

HTMLMenuElement::HTMLMenuElement(const HTMLMenuElement &other) : HTMLElement(other)
{
}

HTMLMenuElement::HTMLMenuElement(HTMLMenuElementImpl *impl) : HTMLElement(impl)
{
}

HTMLMenuElement &HTMLMenuElement::operator = (const HTMLMenuElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLMenuElement::~HTMLMenuElement()
{
}

bool HTMLMenuElement::compact() const
{
    if(!impl) return 0;
    return ((HTMLMenuElementImpl *)impl)->compact();
}

void HTMLMenuElement::setCompact( const bool &_compact )
{

    if(impl)
        ((HTMLMenuElementImpl *)impl)->setCompact( _compact );
}

// --------------------------------------------------------------------------

HTMLOListElement::HTMLOListElement() : HTMLElement()
{
}

HTMLOListElement::HTMLOListElement(const HTMLOListElement &other) : HTMLElement(other)
{
}

HTMLOListElement::HTMLOListElement(HTMLOListElementImpl *impl) : HTMLElement(impl)
{
}

HTMLOListElement &HTMLOListElement::operator = (const HTMLOListElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLOListElement::~HTMLOListElement()
{
}

bool HTMLOListElement::compact() const
{
    if(!impl) return 0;
    return ((HTMLOListElementImpl *)impl)->compact();
}

void HTMLOListElement::setCompact( const bool &_compact )
{

    if(impl)
        ((HTMLOListElementImpl *)impl)->setCompact( _compact );
}

long HTMLOListElement::start() const
{
    if(!impl) return 0;
    return ((HTMLOListElementImpl *)impl)->start();
}

void HTMLOListElement::setStart( const long &_start )
{

    if(impl)
        ((HTMLOListElementImpl *)impl)->setStart( _start );
}

DOMString HTMLOListElement::type() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("type");
}

void HTMLOListElement::setType( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("type", value);
}

// --------------------------------------------------------------------------

HTMLUListElement::HTMLUListElement() : HTMLElement()
{
}

HTMLUListElement::HTMLUListElement(const HTMLUListElement &other) : HTMLElement(other)
{
}

HTMLUListElement::HTMLUListElement(HTMLUListElementImpl *impl) : HTMLElement(impl)
{
}

HTMLUListElement &HTMLUListElement::operator = (const HTMLUListElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLUListElement::~HTMLUListElement()
{
}

bool HTMLUListElement::compact() const
{
    if(!impl) return 0;
    return ((HTMLUListElementImpl *)impl)->compact();
}

void HTMLUListElement::setCompact( const bool &_compact )
{

    if(impl)
        ((HTMLUListElementImpl *)impl)->setCompact( _compact );
}

DOMString HTMLUListElement::type() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("type");
}

void HTMLUListElement::setType( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("type", value);
}

