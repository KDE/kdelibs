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

#include "html_form.h"
#include "html_formimpl.h"
#include "dom_string.h"
#include "html_miscimpl.h"
using namespace DOM;

HTMLButtonElement::HTMLButtonElement() : HTMLElement()
{
}

HTMLButtonElement::HTMLButtonElement(const HTMLButtonElement &other) : HTMLElement(other)
{
}

HTMLButtonElement::HTMLButtonElement(HTMLButtonElementImpl *impl) : HTMLElement(impl)
{
}

HTMLButtonElement &HTMLButtonElement::operator = (const HTMLButtonElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLButtonElement::~HTMLButtonElement()
{
}

HTMLFormElement HTMLButtonElement::form() const
{
    if(!impl) return 0;
    return ((HTMLButtonElementImpl *)impl)->form();
}

DOMString HTMLButtonElement::accessKey() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("accessKey");
}

void HTMLButtonElement::setAccessKey( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("accessKey", value);
}

bool HTMLButtonElement::disabled() const
{
    if(!impl) return 0;
    return ((HTMLButtonElementImpl *)impl)->disabled();
}

void HTMLButtonElement::setDisabled( const bool &_disabled )
{

    if(impl)
        ((HTMLButtonElementImpl *)impl)->setDisabled( _disabled );
}

DOMString HTMLButtonElement::name() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("name");
}

void HTMLButtonElement::setName( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("name", value);
}

long HTMLButtonElement::tabIndex() const
{
    if(!impl) return 0;
    return ((HTMLButtonElementImpl *)impl)->tabIndex();
}

void HTMLButtonElement::setTabIndex( const long &_tabIndex )
{

    if(impl)
        ((HTMLButtonElementImpl *)impl)->setTabIndex( _tabIndex );
}

DOMString HTMLButtonElement::type() const
{
    if(!impl) return 0;
    return ((HTMLButtonElementImpl *)impl)->type();
}

DOMString HTMLButtonElement::value() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("value");
}

void HTMLButtonElement::setValue( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("value", value);
}

// --------------------------------------------------------------------------

HTMLFieldSetElement::HTMLFieldSetElement() : HTMLElement()
{
}

HTMLFieldSetElement::HTMLFieldSetElement(const HTMLFieldSetElement &other) : HTMLElement(other)
{
}

HTMLFieldSetElement::HTMLFieldSetElement(HTMLFieldSetElementImpl *impl) : HTMLElement(impl)
{
}

HTMLFieldSetElement &HTMLFieldSetElement::operator = (const HTMLFieldSetElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLFieldSetElement::~HTMLFieldSetElement()
{
}

HTMLFormElement HTMLFieldSetElement::form() const
{
    if(!impl) return 0;
    return ((HTMLFieldSetElementImpl *)impl)->form();
}

// --------------------------------------------------------------------------

HTMLFormElement::HTMLFormElement() : HTMLElement()
{
}

HTMLFormElement::HTMLFormElement(const HTMLFormElement &other) : HTMLElement(other)
{
}

HTMLFormElement::HTMLFormElement(HTMLFormElementImpl *impl) : HTMLElement(impl)
{
}

HTMLFormElement &HTMLFormElement::operator = (const HTMLFormElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLFormElement::~HTMLFormElement()
{
}

HTMLCollection HTMLFormElement::elements() const
{
    if(!impl) return HTMLCollection();
    return HTMLCollection(impl, HTMLCollectionImpl::FORM_ELEMENTS);
}

long HTMLFormElement::length() const
{
    if(!impl) return 0;
    return ((HTMLFormElementImpl *)impl)->length();
}

DOMString HTMLFormElement::name() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("name");
}

void HTMLFormElement::setName( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("name", value);
}

DOMString HTMLFormElement::acceptCharset() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("acceptCharset");
}

void HTMLFormElement::setAcceptCharset( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("acceptCharset", value);
}

DOMString HTMLFormElement::action() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("action");
}

void HTMLFormElement::setAction( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("action", value);
}

DOMString HTMLFormElement::enctype() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("enctype");
}

void HTMLFormElement::setEnctype( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("enctype", value);
}

DOMString HTMLFormElement::method() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("method");
}

void HTMLFormElement::setMethod( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("method", value);
}

DOMString HTMLFormElement::target() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("target");
}

void HTMLFormElement::setTarget( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("target", value);
}

void HTMLFormElement::submit(  )
{
    if(impl)
        ((HTMLFormElementImpl *)impl)->submit(  );
}

void HTMLFormElement::reset(  )
{
    if(impl)
        ((HTMLFormElementImpl *)impl)->reset(  );
}

// --------------------------------------------------------------------------

HTMLInputElement::HTMLInputElement() : HTMLElement()
{
}

HTMLInputElement::HTMLInputElement(const HTMLInputElement &other) : HTMLElement(other)
{
}

HTMLInputElement::HTMLInputElement(HTMLInputElementImpl *impl) : HTMLElement(impl)
{
}

HTMLInputElement &HTMLInputElement::operator = (const HTMLInputElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLInputElement::~HTMLInputElement()
{
}

DOMString HTMLInputElement::defaultValue() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("defaultValue");
}

void HTMLInputElement::setDefaultValue( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("defaultValue", value);
}

bool HTMLInputElement::defaultChecked() const
{
    if(!impl) return 0;
    return ((HTMLInputElementImpl *)impl)->defaultChecked();
}

void HTMLInputElement::setDefaultChecked( const bool &_defaultChecked )
{

    if(impl)
        ((HTMLInputElementImpl *)impl)->setDefaultChecked( _defaultChecked );
}

HTMLFormElement HTMLInputElement::form() const
{
    if(!impl) return 0;
    return ((HTMLInputElementImpl *)impl)->form();
}

DOMString HTMLInputElement::accept() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("accept");
}

void HTMLInputElement::setAccept( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("accept", value);
}

DOMString HTMLInputElement::accessKey() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("accessKey");
}

void HTMLInputElement::setAccessKey( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("accessKey", value);
}

DOMString HTMLInputElement::align() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("align");
}

void HTMLInputElement::setAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("align", value);
}

DOMString HTMLInputElement::alt() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("alt");
}

void HTMLInputElement::setAlt( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("alt", value);
}

bool HTMLInputElement::checked() const
{
    if(!impl) return 0;
    return ((HTMLInputElementImpl *)impl)->checked();
}

void HTMLInputElement::setChecked( const bool &_checked )
{

    if(impl)
        ((HTMLInputElementImpl *)impl)->setChecked( _checked );
}

bool HTMLInputElement::disabled() const
{
    if(!impl) return 0;
    return ((HTMLInputElementImpl *)impl)->disabled();
}

void HTMLInputElement::setDisabled( const bool &_disabled )
{

    if(impl)
        ((HTMLInputElementImpl *)impl)->setDisabled( _disabled );
}

long HTMLInputElement::maxLength() const
{
    if(!impl) return 0;
    return ((HTMLInputElementImpl *)impl)->maxLength();
}

void HTMLInputElement::setMaxLength( const long &_maxLength )
{

    if(impl)
        ((HTMLInputElementImpl *)impl)->setMaxLength( _maxLength );
}

DOMString HTMLInputElement::name() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("name");
}

void HTMLInputElement::setName( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("name", value);
}

bool HTMLInputElement::readOnly() const
{
    if(!impl) return 0;
    return ((HTMLInputElementImpl *)impl)->readOnly();
}

void HTMLInputElement::setReadOnly( const bool &_readOnly )
{

    if(impl)
        ((HTMLInputElementImpl *)impl)->setReadOnly( _readOnly );
}

DOMString HTMLInputElement::size() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("size");
}

void HTMLInputElement::setSize( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("size", value);
}

DOMString HTMLInputElement::src() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("src");
}

void HTMLInputElement::setSrc( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("src", value);
}

long HTMLInputElement::tabIndex() const
{
    if(!impl) return 0;
    return ((HTMLInputElementImpl *)impl)->tabIndex();
}

void HTMLInputElement::setTabIndex( const long &_tabIndex )
{

    if(impl)
        ((HTMLInputElementImpl *)impl)->setTabIndex( _tabIndex );
}

DOMString HTMLInputElement::type() const
{
    if(!impl) return 0;
    return ((HTMLInputElementImpl *)impl)->type();
}

DOMString HTMLInputElement::useMap() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("useMap");
}

void HTMLInputElement::setUseMap( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("useMap", value);
}

DOMString HTMLInputElement::value() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("value");
}

void HTMLInputElement::setValue( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("value", value);
}

void HTMLInputElement::blur(  )
{
    if(impl)
        ((HTMLInputElementImpl *)impl)->blur(  );
}

void HTMLInputElement::focus(  )
{
    if(impl)
        ((HTMLInputElementImpl *)impl)->focus(  );
}

void HTMLInputElement::select(  )
{
    if(impl)
        ((HTMLInputElementImpl *)impl)->select(  );
}

void HTMLInputElement::click(  )
{
    if(impl)
        ((HTMLInputElementImpl *)impl)->click(  );
}

// --------------------------------------------------------------------------

HTMLLabelElement::HTMLLabelElement() : HTMLElement()
{
}

HTMLLabelElement::HTMLLabelElement(const HTMLLabelElement &other) : HTMLElement(other)
{
}

HTMLLabelElement::HTMLLabelElement(HTMLLabelElementImpl *impl) : HTMLElement(impl)
{
}

HTMLLabelElement &HTMLLabelElement::operator = (const HTMLLabelElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLLabelElement::~HTMLLabelElement()
{
}

HTMLFormElement HTMLLabelElement::form() const
{
    if(!impl) return 0;
    return ((HTMLLabelElementImpl *)impl)->form();
}

DOMString HTMLLabelElement::accessKey() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("accessKey");
}

void HTMLLabelElement::setAccessKey( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("accessKey", value);
}

DOMString HTMLLabelElement::htmlFor() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("htmlFor");
}

void HTMLLabelElement::setHtmlFor( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("htmlFor", value);
}

// --------------------------------------------------------------------------

HTMLLegendElement::HTMLLegendElement() : HTMLElement()
{
}

HTMLLegendElement::HTMLLegendElement(const HTMLLegendElement &other) : HTMLElement(other)
{
}

HTMLLegendElement::HTMLLegendElement(HTMLLegendElementImpl *impl) : HTMLElement(impl)
{
}

HTMLLegendElement &HTMLLegendElement::operator = (const HTMLLegendElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLLegendElement::~HTMLLegendElement()
{
}

HTMLFormElement HTMLLegendElement::form() const
{
    if(!impl) return 0;
    return ((HTMLLegendElementImpl *)impl)->form();
}

DOMString HTMLLegendElement::accessKey() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("accessKey");
}

void HTMLLegendElement::setAccessKey( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("accessKey", value);
}

DOMString HTMLLegendElement::align() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("align");
}

void HTMLLegendElement::setAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("align", value);
}

// --------------------------------------------------------------------------

HTMLOptGroupElement::HTMLOptGroupElement() : HTMLElement()
{
}

HTMLOptGroupElement::HTMLOptGroupElement(const HTMLOptGroupElement &other) : HTMLElement(other)
{
}

HTMLOptGroupElement::HTMLOptGroupElement(HTMLOptGroupElementImpl *impl) : HTMLElement(impl)
{
}

HTMLOptGroupElement &HTMLOptGroupElement::operator = (const HTMLOptGroupElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLOptGroupElement::~HTMLOptGroupElement()
{
}

bool HTMLOptGroupElement::disabled() const
{
    if(!impl) return 0;
    return ((HTMLOptGroupElementImpl *)impl)->disabled();
}

void HTMLOptGroupElement::setDisabled( const bool &_disabled )
{

    if(impl)
        ((HTMLOptGroupElementImpl *)impl)->setDisabled( _disabled );
}

DOMString HTMLOptGroupElement::label() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("label");
}

void HTMLOptGroupElement::setLabel( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("label", value);
}

// --------------------------------------------------------------------------

HTMLSelectElement::HTMLSelectElement() : HTMLElement()
{
}

HTMLSelectElement::HTMLSelectElement(const HTMLSelectElement &other) : HTMLElement(other)
{
}

HTMLSelectElement::HTMLSelectElement(HTMLSelectElementImpl *impl) : HTMLElement(impl)
{
}

HTMLSelectElement &HTMLSelectElement::operator = (const HTMLSelectElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLSelectElement::~HTMLSelectElement()
{
}

DOMString HTMLSelectElement::type() const
{
    if(!impl) return 0;
    return ((HTMLSelectElementImpl *)impl)->type();
}

long HTMLSelectElement::selectedIndex() const
{
    if(!impl) return 0;
    return ((HTMLSelectElementImpl *)impl)->selectedIndex();
}

void HTMLSelectElement::setSelectedIndex( const long &_selectedIndex )
{

    if(impl)
        ((HTMLSelectElementImpl *)impl)->setSelectedIndex( _selectedIndex );
}

DOMString HTMLSelectElement::value() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("value");
}

void HTMLSelectElement::setValue( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("value", value);
}

long HTMLSelectElement::length() const
{
    if(!impl) return 0;
    return ((HTMLSelectElementImpl *)impl)->length();
}

HTMLFormElement HTMLSelectElement::form() const
{
    if(!impl) return 0;
    return ((HTMLSelectElementImpl *)impl)->form();
}

HTMLCollection HTMLSelectElement::options() const
{
    if(!impl) return HTMLCollection();
    return HTMLCollection(impl, HTMLCollectionImpl::SELECT_OPTIONS);
}

bool HTMLSelectElement::disabled() const
{
    if(!impl) return 0;
    return ((HTMLSelectElementImpl *)impl)->disabled();
}

void HTMLSelectElement::setDisabled( const bool &_disabled )
{

    if(impl)
        ((HTMLSelectElementImpl *)impl)->setDisabled( _disabled );
}

bool HTMLSelectElement::multiple() const
{
    if(!impl) return 0;
    return ((HTMLSelectElementImpl *)impl)->multiple();
}

void HTMLSelectElement::setMultiple( const bool &_multiple )
{

    if(impl)
        ((HTMLSelectElementImpl *)impl)->setMultiple( _multiple );
}

DOMString HTMLSelectElement::name() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("name");
}

void HTMLSelectElement::setName( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("name", value);
}

long HTMLSelectElement::size() const
{
    if(!impl) return 0;
    return ((HTMLSelectElementImpl *)impl)->size();
}

void HTMLSelectElement::setSize( const long &_size )
{

    if(impl)
        ((HTMLSelectElementImpl *)impl)->setSize( _size );
}

long HTMLSelectElement::tabIndex() const
{
    if(!impl) return 0;
    return ((HTMLSelectElementImpl *)impl)->tabIndex();
}

void HTMLSelectElement::setTabIndex( const long &_tabIndex )
{

    if(impl)
        ((HTMLSelectElementImpl *)impl)->setTabIndex( _tabIndex );
}

void HTMLSelectElement::add( const HTMLElement &element, const HTMLElement &before )
{
    if(impl)
        ((HTMLSelectElementImpl *)impl)->add( element, before );
}

void HTMLSelectElement::remove( const long &index )
{
    if(impl)
        ((HTMLSelectElementImpl *)impl)->remove( index );
}

void HTMLSelectElement::blur(  )
{
    if(impl)
        ((HTMLSelectElementImpl *)impl)->blur(  );
}

void HTMLSelectElement::focus(  )
{
    if(impl)
        ((HTMLSelectElementImpl *)impl)->focus(  );
}

// --------------------------------------------------------------------------

HTMLTextAreaElement::HTMLTextAreaElement() : HTMLElement()
{
}

HTMLTextAreaElement::HTMLTextAreaElement(const HTMLTextAreaElement &other) : HTMLElement(other)
{
}

HTMLTextAreaElement::HTMLTextAreaElement(HTMLTextAreaElementImpl *impl) : HTMLElement(impl)
{
}

HTMLTextAreaElement &HTMLTextAreaElement::operator = (const HTMLTextAreaElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLTextAreaElement::~HTMLTextAreaElement()
{
}

DOMString HTMLTextAreaElement::defaultValue() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("defaultValue");
}

void HTMLTextAreaElement::setDefaultValue( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("defaultValue", value);
}

HTMLFormElement HTMLTextAreaElement::form() const
{
    if(!impl) return 0;
    return ((HTMLTextAreaElementImpl *)impl)->form();
}

DOMString HTMLTextAreaElement::accessKey() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("accessKey");
}

void HTMLTextAreaElement::setAccessKey( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("accessKey", value);
}

long HTMLTextAreaElement::cols() const
{
    if(!impl) return 0;
    return ((HTMLTextAreaElementImpl *)impl)->cols();
}

void HTMLTextAreaElement::setCols( const long &_cols )
{

    if(impl)
        ((HTMLTextAreaElementImpl *)impl)->setCols( _cols );
}

bool HTMLTextAreaElement::disabled() const
{
    if(!impl) return 0;
    return ((HTMLTextAreaElementImpl *)impl)->disabled();
}

void HTMLTextAreaElement::setDisabled( const bool &_disabled )
{

    if(impl)
        ((HTMLTextAreaElementImpl *)impl)->setDisabled( _disabled );
}

DOMString HTMLTextAreaElement::name() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("name");
}

void HTMLTextAreaElement::setName( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("name", value);
}

bool HTMLTextAreaElement::readOnly() const
{
    if(!impl) return 0;
    return ((HTMLTextAreaElementImpl *)impl)->readOnly();
}

void HTMLTextAreaElement::setReadOnly( const bool &_readOnly )
{

    if(impl)
        ((HTMLTextAreaElementImpl *)impl)->setReadOnly( _readOnly );
}

long HTMLTextAreaElement::rows() const
{
    if(!impl) return 0;
    return ((HTMLTextAreaElementImpl *)impl)->rows();
}

void HTMLTextAreaElement::setRows( const long &_rows )
{

    if(impl)
        ((HTMLTextAreaElementImpl *)impl)->setRows( _rows );
}

long HTMLTextAreaElement::tabIndex() const
{
    if(!impl) return 0;
    return ((HTMLTextAreaElementImpl *)impl)->tabIndex();
}

void HTMLTextAreaElement::setTabIndex( const long &_tabIndex )
{

    if(impl)
        ((HTMLTextAreaElementImpl *)impl)->setTabIndex( _tabIndex );
}

DOMString HTMLTextAreaElement::type() const
{
    if(!impl) return 0;
    return ((HTMLTextAreaElementImpl *)impl)->type();
}

DOMString HTMLTextAreaElement::value() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("value");
}

void HTMLTextAreaElement::setValue( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("value", value);
}

void HTMLTextAreaElement::blur(  )
{
    if(impl)
        ((HTMLTextAreaElementImpl *)impl)->blur(  );
}

void HTMLTextAreaElement::focus(  )
{
    if(impl)
        ((HTMLTextAreaElementImpl *)impl)->focus(  );
}

void HTMLTextAreaElement::select(  )
{
    if(impl)
        ((HTMLTextAreaElementImpl *)impl)->select(  );
}

// --------------------------------------------------------------------------

HTMLOptionElement::HTMLOptionElement() : HTMLElement()
{
}

HTMLOptionElement::HTMLOptionElement(const HTMLOptionElement &other) : HTMLElement(other)
{
}

HTMLOptionElement::HTMLOptionElement(HTMLOptionElementImpl *impl) : HTMLElement(impl)
{
}

HTMLOptionElement &HTMLOptionElement::operator = (const HTMLOptionElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLOptionElement::~HTMLOptionElement()
{
}

HTMLFormElement HTMLOptionElement::form() const
{
    if(!impl) return 0;
    return ((HTMLOptionElementImpl *)impl)->form();
}

bool HTMLOptionElement::defaultSelected() const
{
    if(!impl) return 0;
    return ((HTMLOptionElementImpl *)impl)->defaultSelected();
}

void HTMLOptionElement::setDefaultSelected( const bool &_defaultSelected )
{

    if(impl)
        ((HTMLOptionElementImpl *)impl)->setDefaultSelected( _defaultSelected );
}

DOMString HTMLOptionElement::text() const
{
    if(!impl) return 0;
    return ((HTMLOptionElementImpl *)impl)->text();
}

long HTMLOptionElement::index() const
{
    if(!impl) return 0;
    return ((HTMLOptionElementImpl *)impl)->index();
}

void HTMLOptionElement::setIndex( const long &_index )
{

    if(impl)
        ((HTMLOptionElementImpl *)impl)->setIndex( _index );
}

bool HTMLOptionElement::disabled() const
{
    if(!impl) return 0;
    return ((HTMLOptionElementImpl *)impl)->disabled();
}

void HTMLOptionElement::setDisabled( const bool &_disabled )
{

    if(impl)
        ((HTMLOptionElementImpl *)impl)->setDisabled( _disabled );
}

DOMString HTMLOptionElement::label() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("label");
}

void HTMLOptionElement::setLabel( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("label", value);
}

bool HTMLOptionElement::selected() const
{
    if(!impl) return 0;
    return ((HTMLOptionElementImpl *)impl)->selected();
}

DOMString HTMLOptionElement::value() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("value");
}

void HTMLOptionElement::setValue( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("value", value);
}

