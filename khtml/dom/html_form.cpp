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
#include "dom_exception.h"
using namespace DOM;

#include "htmlhashes.h"

HTMLButtonElement::HTMLButtonElement() : HTMLElement()
{
}

HTMLButtonElement::HTMLButtonElement(const HTMLButtonElement &other) : HTMLElement(other)
{
}

HTMLButtonElement::HTMLButtonElement(HTMLButtonElementImpl *impl) : HTMLElement(impl)
{
}

HTMLButtonElement &HTMLButtonElement::operator = (const Node &other)
{
    if(other.elementId() != ID_BUTTON)
    {
	impl = 0;
	return *this;
    }
    Node::operator = (other);
    return *this;
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
    return ((ElementImpl *)impl)->getAttribute(ATTR_ACCESSKEY);
}

void HTMLButtonElement::setAccessKey( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_ACCESSKEY, value);
}

bool HTMLButtonElement::disabled() const
{
    if(!impl) return 0;
    return !((ElementImpl *)impl)->getAttribute(ATTR_DISABLED).isNull();
}

void HTMLButtonElement::setDisabled( bool _disabled )
{
    if (impl) ((ElementImpl *)impl)->setAttribute(ATTR_DISABLED, _disabled ? "" : 0);
}

DOMString HTMLButtonElement::name() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_NAME);
}

void HTMLButtonElement::setName( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_NAME, value);
}

long HTMLButtonElement::tabIndex() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_TABINDEX).toInt();
}

void HTMLButtonElement::setTabIndex( long _tabIndex )
{
    if (!impl) {
	DOMString value(QString::number(_tabIndex));
	((ElementImpl *)impl)->setAttribute(ATTR_TABINDEX, value);
    }
}

DOMString HTMLButtonElement::type() const
{
    if(!impl) return 0;
    return ((HTMLButtonElementImpl *)impl)->type();
}

DOMString HTMLButtonElement::value() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_VALUE);
}

void HTMLButtonElement::setValue( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_VALUE, value);
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

HTMLFieldSetElement &HTMLFieldSetElement::operator = (const Node &other)
{
    if(other.elementId() != ID_FIELDSET)
    {
	impl = 0;
	return *this;
    }
    Node::operator = (other);
    return *this;
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

HTMLFormElement &HTMLFormElement::operator = (const Node &other)
{
    if(other.elementId() != ID_FORM)
    {
	impl = 0;
	return *this;
    }
    Node::operator = (other);
    return *this;
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
    return ((ElementImpl *)impl)->getAttribute(ATTR_NAME);
}

void HTMLFormElement::setName( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_NAME, value);
}

DOMString HTMLFormElement::acceptCharset() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_ACCEPT_CHARSET);
}

void HTMLFormElement::setAcceptCharset( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_ACCEPT_CHARSET, value);
}

DOMString HTMLFormElement::action() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_ACTION);
}

void HTMLFormElement::setAction( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_ACTION, value);
}

DOMString HTMLFormElement::enctype() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_ENCTYPE);
}

void HTMLFormElement::setEnctype( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_ENCTYPE, value);
}

DOMString HTMLFormElement::method() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_METHOD);
}

void HTMLFormElement::setMethod( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_METHOD, value);
}

DOMString HTMLFormElement::target() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_TARGET);
}

void HTMLFormElement::setTarget( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_TARGET, value);
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

HTMLInputElement &HTMLInputElement::operator = (const Node &other)
{
    if(other.elementId() != ID_INPUT)
    {
	impl = 0;
	return *this;
    }
    Node::operator = (other);
    return *this;
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
    return ((ElementImpl *)impl)->getAttribute(ATTR_VALUE);
}

void HTMLInputElement::setDefaultValue( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_VALUE, value);
}

bool HTMLInputElement::defaultChecked() const
{
    if(!impl) return 0;
    return !((ElementImpl *)impl)->getAttribute(ATTR_CHECKED).isNull();
}

void HTMLInputElement::setDefaultChecked( bool _defaultChecked )
{
    if(impl)
    {
	((ElementImpl *)impl)->setAttribute(ATTR_CHECKED, _defaultChecked ? "" : 0);
    }
}

HTMLFormElement HTMLInputElement::form() const
{
    if(!impl) return 0;
    return ((HTMLInputElementImpl *)impl)->form();
}

DOMString HTMLInputElement::accept() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_ACCEPT);
}

void HTMLInputElement::setAccept( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_ACCEPT, value);
}

DOMString HTMLInputElement::accessKey() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_ACCESSKEY);
}

void HTMLInputElement::setAccessKey( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_ACCESSKEY, value);
}

DOMString HTMLInputElement::align() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_ALIGN);
}

void HTMLInputElement::setAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_ALIGN, value);
}

DOMString HTMLInputElement::alt() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_ALT);
}

void HTMLInputElement::setAlt( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_ALT, value);
}

bool HTMLInputElement::checked() const
{
    if(!impl) return 0;
    return ((HTMLInputElementImpl*)impl)->checked();
}

void HTMLInputElement::setChecked( bool _checked )
{
    if(impl)
	((HTMLInputElementImpl*)impl)->setChecked(_checked);
}

bool HTMLInputElement::disabled() const
{
    if(!impl) return 0;
    return !((ElementImpl *)impl)->getAttribute(ATTR_DISABLED).isNull();
}

void HTMLInputElement::setDisabled( bool _disabled )
{
    if(impl)
    {
	((ElementImpl *)impl)->setAttribute(ATTR_DISABLED, _disabled ? "" : 0);
    }
}

long HTMLInputElement::maxLength() const
{
    if(!impl) return 0;
    return ((HTMLInputElementImpl *)impl)->getAttribute(ATTR_MAXLENGTH).toInt();
}

void HTMLInputElement::setMaxLength( long _maxLength )
{
    if(impl) {
        DOMString value(QString::number(_maxLength));
        ((ElementImpl *)impl)->setAttribute(ATTR_MAXLENGTH,value);
    }
}

DOMString HTMLInputElement::name() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_NAME);
}

void HTMLInputElement::setName( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_NAME, value);
}

bool HTMLInputElement::readOnly() const
{
    if(!impl) return 0;
    return !((ElementImpl *)impl)->getAttribute(ATTR_READONLY).isNull();
}

void HTMLInputElement::setReadOnly( bool _readOnly )
{
    if(impl)
	((ElementImpl *)impl)->setAttribute(ATTR_READONLY, _readOnly ? "" : 0);
}

DOMString HTMLInputElement::size() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_SIZE);
}

void HTMLInputElement::setSize( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_SIZE, value);
}

DOMString HTMLInputElement::src() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_SRC);
}

void HTMLInputElement::setSrc( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_SRC, value);
}

long HTMLInputElement::tabIndex() const
{
    if(!impl) return 0;
    return ((HTMLInputElementImpl *)impl)->getAttribute(ATTR_TABINDEX).toInt();
}

void HTMLInputElement::setTabIndex( long _tabIndex )
{

    if(impl) {
	DOMString value(QString::number(_tabIndex));
        ((ElementImpl *)impl)->setAttribute(ATTR_TABINDEX,value);
    }
}

DOMString HTMLInputElement::type() const
{
    if(!impl) return 0;
    return ((HTMLInputElementImpl *)impl)->type();
}

DOMString HTMLInputElement::useMap() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_USEMAP);
}

void HTMLInputElement::setUseMap( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_USEMAP, value);
}

DOMString HTMLInputElement::value() const
{
    if(!impl) return 0;
    return ((HTMLInputElementImpl*)impl)->value();
}

void HTMLInputElement::setValue( const DOMString &value )
{
    if (impl)
	((HTMLInputElementImpl*)impl)->setValue(value);

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

HTMLLabelElement &HTMLLabelElement::operator = (const Node &other)
{
    if(other.elementId() != ID_LABEL)
    {
	impl = 0;
	return *this;
    }
    Node::operator = (other);
    return *this;
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
    return ((ElementImpl *)impl)->getAttribute(ATTR_ACCESSKEY);
}

void HTMLLabelElement::setAccessKey( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_ACCESSKEY, value);
}

DOMString HTMLLabelElement::htmlFor() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_FOR);
}

void HTMLLabelElement::setHtmlFor( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_FOR, value);
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

HTMLLegendElement &HTMLLegendElement::operator = (const Node &other)
{
    if(other.elementId() != ID_LEGEND)
    {
	impl = 0;
	return *this;
    }
    Node::operator = (other);
    return *this;
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
    return ((ElementImpl *)impl)->getAttribute(ATTR_ACCESSKEY);
}

void HTMLLegendElement::setAccessKey( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_ACCESSKEY, value);
}

DOMString HTMLLegendElement::align() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_ALIGN);
}

void HTMLLegendElement::setAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_ALIGN, value);
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

HTMLOptGroupElement &HTMLOptGroupElement::operator = (const Node &other)
{
    if(other.elementId() != ID_OPTGROUP)
    {
	impl = 0;
	return *this;
    }
    Node::operator = (other);
    return *this;
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
    return !((ElementImpl *)impl)->getAttribute(ATTR_DISABLED).isNull();
}

void HTMLOptGroupElement::setDisabled( bool _disabled )
{
    if(impl)
	((ElementImpl *)impl)->setAttribute(ATTR_DISABLED, _disabled ? "" : 0);
}

DOMString HTMLOptGroupElement::label() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_LABEL);
}

void HTMLOptGroupElement::setLabel( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_LABEL, value);
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

HTMLSelectElement &HTMLSelectElement::operator = (const Node &other)
{
    if(other.elementId() != ID_SELECT)
    {
	impl = 0;
	return *this;
    }
    Node::operator = (other);
    return *this;
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

void HTMLSelectElement::setSelectedIndex( long _selectedIndex )
{
    if(impl)
        ((HTMLSelectElementImpl *)impl)->setSelectedIndex(_selectedIndex);
}

DOMString HTMLSelectElement::value() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_VALUE);
}

void HTMLSelectElement::setValue( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_VALUE, value);
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
    return !((ElementImpl *)impl)->getAttribute(ATTR_DISABLED).isNull();
}

void HTMLSelectElement::setDisabled( bool _disabled )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_DISABLED, _disabled ? "" : 0);
}


bool HTMLSelectElement::multiple() const
{
    if(!impl) return 0;
    return !((ElementImpl *)impl)->getAttribute(ATTR_MULTIPLE).isNull();
}

void HTMLSelectElement::setMultiple( bool _multiple )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_MULTIPLE, _multiple ? "" : 0);
}

DOMString HTMLSelectElement::name() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_NAME);
}

void HTMLSelectElement::setName( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_NAME, value);
}

long HTMLSelectElement::size() const
{
    if(!impl) return 0;
    return ((HTMLSelectElementImpl *)impl)->getAttribute(ATTR_SIZE).toInt();
}

void HTMLSelectElement::setSize( long _size )
{

    if(impl) {
	DOMString value(QString::number(_size));
        ((ElementImpl *)impl)->setAttribute(ATTR_SIZE,value);
    }
}

long HTMLSelectElement::tabIndex() const
{
    if(!impl) return 0;
    return ((HTMLSelectElementImpl *)impl)->getAttribute(ATTR_TABINDEX).toInt();
}

void HTMLSelectElement::setTabIndex( long _tabIndex )
{

    if(impl) {
	DOMString value(QString::number(_tabIndex));
        ((ElementImpl *)impl)->setAttribute(ATTR_TABINDEX,value);
    }
}

void HTMLSelectElement::add( const HTMLElement &element, const HTMLElement &before )
{
    if(impl)
        ((HTMLSelectElementImpl *)impl)->add( element, before );
}

void HTMLSelectElement::remove( long index )
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

HTMLTextAreaElement &HTMLTextAreaElement::operator = (const Node &other)
{
    if(other.elementId() != ID_TEXTAREA)
    {
	impl = 0;
	return *this;
    }
    Node::operator = (other);
    return *this;
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
    return ((HTMLTextAreaElementImpl *)impl)->defaultValue();
}

void HTMLTextAreaElement::setDefaultValue( const DOMString &value )
{
    ((HTMLTextAreaElementImpl *)impl)->setDefaultValue(value);
}

HTMLFormElement HTMLTextAreaElement::form() const
{
    if(!impl) return 0;
    return ((HTMLTextAreaElementImpl *)impl)->form();
}

DOMString HTMLTextAreaElement::accessKey() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_ACCESSKEY);
}

void HTMLTextAreaElement::setAccessKey( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_ACCESSKEY, value);
}

long HTMLTextAreaElement::cols() const
{
    if(!impl) return 0;
    return ((HTMLTextAreaElementImpl *)impl)->getAttribute(ATTR_COLS).toInt();
}

void HTMLTextAreaElement::setCols( long _cols )
{

    if(impl) {
	DOMString value(QString::number(_cols));
        ((ElementImpl *)impl)->setAttribute(ATTR_COLS,value);
    }
}

bool HTMLTextAreaElement::disabled() const
{
    if(!impl) return 0;
    return !((ElementImpl *)impl)->getAttribute(ATTR_DISABLED).isNull();
}

void HTMLTextAreaElement::setDisabled( bool _disabled )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_DISABLED, _disabled ? "" : 0);
}

DOMString HTMLTextAreaElement::name() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_NAME);
}

void HTMLTextAreaElement::setName( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_NAME, value);
}

bool HTMLTextAreaElement::readOnly() const
{
    if(!impl) return 0;
    return !((ElementImpl *)impl)->getAttribute(ATTR_READONLY).isNull();
}

void HTMLTextAreaElement::setReadOnly( bool _readOnly )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_READONLY, _readOnly ? "" : 0);
}

long HTMLTextAreaElement::rows() const
{
    if(!impl) return 0;
    return ((HTMLTextAreaElementImpl *)impl)->getAttribute(ATTR_ROWS).toInt();
}

void HTMLTextAreaElement::setRows( long _rows )
{

    if(impl) {
	DOMString value(QString::number(_rows));
        ((ElementImpl *)impl)->setAttribute(ATTR_ROWS,value);
    }
}

long HTMLTextAreaElement::tabIndex() const
{
    if(!impl) return 0;
    return ((HTMLTextAreaElementImpl *)impl)->getAttribute(ATTR_TABINDEX).toInt();
}

void HTMLTextAreaElement::setTabIndex( long _tabIndex )
{

    if(impl) {
	DOMString value(QString::number(_tabIndex));
        ((ElementImpl *)impl)->setAttribute(ATTR_TABINDEX,value);
    }
}

DOMString HTMLTextAreaElement::type() const
{
    if(!impl) return 0;
    return ((HTMLTextAreaElementImpl *)impl)->type();
}

DOMString HTMLTextAreaElement::value() const
{
    if(!impl) return 0;
    return ((HTMLTextAreaElementImpl *)impl)->value();
}

void HTMLTextAreaElement::setValue( const DOMString &value )
{
    if(impl) ((HTMLTextAreaElementImpl *)impl)->setValue(value);
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

HTMLOptionElement &HTMLOptionElement::operator = (const Node &other)
{
    if(other.elementId() != ID_OPTION)
    {
	impl = 0;
	return *this;
    }
    Node::operator = (other);
    return *this;
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
    return !((ElementImpl *)impl)->getAttribute(ATTR_SELECTED).isNull();
}

void HTMLOptionElement::setDefaultSelected( bool _defaultSelected )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_SELECTED, _defaultSelected ? "" : 0);
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

void HTMLOptionElement::setIndex( long /*_index*/ )
{
    throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR);
}

bool HTMLOptionElement::disabled() const
{
    if(!impl) return 0;
    return !((ElementImpl *)impl)->getAttribute(ATTR_DISABLED).isNull();
}

void HTMLOptionElement::setDisabled( bool _disabled )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_DISABLED, _disabled ? "" : 0);
}

DOMString HTMLOptionElement::label() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_LABEL);
}

void HTMLOptionElement::setLabel( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_LABEL, value);
}

bool HTMLOptionElement::selected() const
{
    if(!impl) return 0;
    return ((HTMLOptionElementImpl *)impl)->selected();
}

void HTMLOptionElement::setSelected(bool _selected) {
    if(!impl) return;
    ((HTMLOptionElementImpl *)impl)->setSelected(_selected);
}

DOMString HTMLOptionElement::value() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute(ATTR_VALUE);
}

void HTMLOptionElement::setValue( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_VALUE, value);
}

// -----------------------------------------------------------------------------

HTMLIsIndexElement::HTMLIsIndexElement() : HTMLElement()
{
}

HTMLIsIndexElement::HTMLIsIndexElement(const HTMLIsIndexElement &other) : HTMLElement(other)
{
}

HTMLIsIndexElement::HTMLIsIndexElement(HTMLIsIndexElementImpl *impl) : HTMLElement(impl)
{
}

HTMLIsIndexElement &HTMLIsIndexElement::operator = (const Node &other)
{
    if(other.elementId() != ID_ISINDEX)
    {
	impl = 0;
	return *this;
    }
    Node::operator = (other);
    return *this;
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
    return ((ElementImpl *)impl)->getAttribute(ATTR_PROMPT);
}

void HTMLIsIndexElement::setPrompt( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute(ATTR_PROMPT, value);
}
