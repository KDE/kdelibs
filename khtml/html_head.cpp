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
#include "html_form.h"
#include "html_head.h"
#include "html_headimpl.h"
using namespace DOM;


HTMLBaseElement::HTMLBaseElement() : HTMLElement()
{
}

HTMLBaseElement::HTMLBaseElement(const HTMLBaseElement &other) : HTMLElement(other)
{
}

HTMLBaseElement::HTMLBaseElement(HTMLBaseElementImpl *impl) : HTMLElement(impl)
{
}

HTMLBaseElement &HTMLBaseElement::operator = (const HTMLBaseElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLBaseElement::~HTMLBaseElement()
{
}

DOMString HTMLBaseElement::href() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("href");
}

void HTMLBaseElement::setHref( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("href", value);
}

DOMString HTMLBaseElement::target() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("target");
}

void HTMLBaseElement::setTarget( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("target", value);
}

// --------------------------------------------------------------------------

HTMLIsIndexElement::HTMLIsIndexElement() : HTMLElement()
{
}

HTMLIsIndexElement::HTMLIsIndexElement(const HTMLIsIndexElement &other) : HTMLElement(other)
{
}

HTMLIsIndexElement::HTMLIsIndexElement(HTMLIsIndexElementImpl *impl) : HTMLElement(impl)
{
}

HTMLIsIndexElement &HTMLIsIndexElement::operator = (const HTMLIsIndexElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLIsIndexElement::~HTMLIsIndexElement()
{
}

HTMLFormElement HTMLIsIndexElement::form() const
{
    if(!impl) return 0;
    return ((HTMLIsIndexElementImpl *)impl)->form();
}

DOMString HTMLIsIndexElement::prompt() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("prompt");
}

void HTMLIsIndexElement::setPrompt( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("prompt", value);
}

// --------------------------------------------------------------------------

HTMLLinkElement::HTMLLinkElement() : HTMLElement()
{
}

HTMLLinkElement::HTMLLinkElement(const HTMLLinkElement &other) : HTMLElement(other)
{
}

HTMLLinkElement::HTMLLinkElement(HTMLLinkElementImpl *impl) : HTMLElement(impl)
{
}

HTMLLinkElement &HTMLLinkElement::operator = (const HTMLLinkElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLLinkElement::~HTMLLinkElement()
{
}

bool HTMLLinkElement::disabled() const
{
    if(!impl) return 0;
    return ((HTMLLinkElementImpl *)impl)->disabled();
}

void HTMLLinkElement::setDisabled( const bool &_disabled )
{

    if(impl)
        ((HTMLLinkElementImpl *)impl)->setDisabled( _disabled );
}

DOMString HTMLLinkElement::charset() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("charset");
}

void HTMLLinkElement::setCharset( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("charset", value);
}

DOMString HTMLLinkElement::href() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("href");
}

void HTMLLinkElement::setHref( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("href", value);
}

DOMString HTMLLinkElement::hreflang() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("hreflang");
}

void HTMLLinkElement::setHreflang( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("hreflang", value);
}

DOMString HTMLLinkElement::media() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("media");
}

void HTMLLinkElement::setMedia( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("media", value);
}

DOMString HTMLLinkElement::rel() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("rel");
}

void HTMLLinkElement::setRel( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("rel", value);
}

DOMString HTMLLinkElement::rev() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("rev");
}

void HTMLLinkElement::setRev( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("rev", value);
}

DOMString HTMLLinkElement::target() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("target");
}

void HTMLLinkElement::setTarget( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("target", value);
}

DOMString HTMLLinkElement::type() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("type");
}

void HTMLLinkElement::setType( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("type", value);
}

// --------------------------------------------------------------------------

HTMLMetaElement::HTMLMetaElement() : HTMLElement()
{
}

HTMLMetaElement::HTMLMetaElement(const HTMLMetaElement &other) : HTMLElement(other)
{
}

HTMLMetaElement::HTMLMetaElement(HTMLMetaElementImpl *impl) : HTMLElement(impl)
{
}

HTMLMetaElement &HTMLMetaElement::operator = (const HTMLMetaElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLMetaElement::~HTMLMetaElement()
{
}

DOMString HTMLMetaElement::content() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("content");
}

void HTMLMetaElement::setContent( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("content", value);
}

DOMString HTMLMetaElement::httpEquiv() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("httpEquiv");
}

void HTMLMetaElement::setHttpEquiv( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("httpEquiv", value);
}

DOMString HTMLMetaElement::name() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("name");
}

void HTMLMetaElement::setName( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("name", value);
}

DOMString HTMLMetaElement::scheme() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("scheme");
}

void HTMLMetaElement::setScheme( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("scheme", value);
}

// --------------------------------------------------------------------------

HTMLScriptElement::HTMLScriptElement() : HTMLElement()
{
}

HTMLScriptElement::HTMLScriptElement(const HTMLScriptElement &other) : HTMLElement(other)
{
}

HTMLScriptElement::HTMLScriptElement(HTMLScriptElementImpl *impl) : HTMLElement(impl)
{
}

HTMLScriptElement &HTMLScriptElement::operator = (const HTMLScriptElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLScriptElement::~HTMLScriptElement()
{
}

DOMString HTMLScriptElement::text() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("text");
}

void HTMLScriptElement::setText( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("text", value);
}

DOMString HTMLScriptElement::htmlFor() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("htmlFor");
}

void HTMLScriptElement::setHtmlFor( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("htmlFor", value);
}

DOMString HTMLScriptElement::event() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("event");
}

void HTMLScriptElement::setEvent( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("event", value);
}

DOMString HTMLScriptElement::charset() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("charset");
}

void HTMLScriptElement::setCharset( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("charset", value);
}

bool HTMLScriptElement::defer() const
{
    if(!impl) return 0;
    return ((HTMLScriptElementImpl *)impl)->defer();
}

void HTMLScriptElement::setDefer( const bool &_defer )
{

    if(impl)
        ((HTMLScriptElementImpl *)impl)->setDefer( _defer );
}

DOMString HTMLScriptElement::src() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("src");
}

void HTMLScriptElement::setSrc( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("src", value);
}

DOMString HTMLScriptElement::type() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("type");
}

void HTMLScriptElement::setType( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("type", value);
}

// --------------------------------------------------------------------------

HTMLStyleElement::HTMLStyleElement() : HTMLElement()
{
}

HTMLStyleElement::HTMLStyleElement(const HTMLStyleElement &other) : HTMLElement(other)
{
}

HTMLStyleElement::HTMLStyleElement(HTMLStyleElementImpl *impl) : HTMLElement(impl)
{
}

HTMLStyleElement &HTMLStyleElement::operator = (const HTMLStyleElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLStyleElement::~HTMLStyleElement()
{
}

bool HTMLStyleElement::disabled() const
{
    if(!impl) return 0;
    return ((HTMLStyleElementImpl *)impl)->disabled();
}

void HTMLStyleElement::setDisabled( const bool &_disabled )
{

    if(impl)
        ((HTMLStyleElementImpl *)impl)->setDisabled( _disabled );
}

DOMString HTMLStyleElement::media() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("media");
}

void HTMLStyleElement::setMedia( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("media", value);
}

DOMString HTMLStyleElement::type() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("type");
}

void HTMLStyleElement::setType( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("type", value);
}

// --------------------------------------------------------------------------

HTMLTitleElement::HTMLTitleElement() : HTMLElement()
{
}

HTMLTitleElement::HTMLTitleElement(const HTMLTitleElement &other) : HTMLElement(other)
{
}

HTMLTitleElement::HTMLTitleElement(HTMLTitleElementImpl *impl) : HTMLElement(impl)
{
}

HTMLTitleElement &HTMLTitleElement::operator = (const HTMLTitleElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLTitleElement::~HTMLTitleElement()
{
}

DOMString HTMLTitleElement::text() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("text");
}

void HTMLTitleElement::setText( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("text", value);
}

