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
#include "css_rule.h"
#include "css_value.h"
#include "css_valueimpl.h"
#include "dom_exception.h"
#include "dom_string.h"
using namespace DOM;

CSSStyleDeclaration::CSSStyleDeclaration()
{
    impl = 0;
}

CSSStyleDeclaration::CSSStyleDeclaration(const CSSStyleDeclaration &other)
{
    impl = other.impl;
    if(impl) impl->ref();
}

CSSStyleDeclaration::CSSStyleDeclaration(CSSStyleDeclarationImpl *i)
{
    impl = i;
    if(impl) impl->ref();
}

CSSStyleDeclaration &CSSStyleDeclaration::operator = (const CSSStyleDeclaration &other)
{
    if(impl) impl->deref();
    impl = other.impl;
    if(impl) impl->ref();
    return *this;
}

CSSStyleDeclaration::~CSSStyleDeclaration()
{
    if(impl) impl->deref();
}

DOMString CSSStyleDeclaration::cssText() const
{
    if(!impl) return 0;
    //    return ((ElementImpl *)impl)->getAttribute("cssText");
}

void CSSStyleDeclaration::setCssText( const DOMString &value )
{
    //if(impl) ((ElementImpl *)impl)->setAttribute("cssText", value);
}

DOMString CSSStyleDeclaration::getPropertyValue( const DOMString &propertyName )
{
    if(!impl) return 0;
    return ((CSSStyleDeclarationImpl *)impl)->getPropertyValue( propertyName );
}

CSSValue CSSStyleDeclaration::getPropertyCSSValue( const DOMString &propertyName )
{
    //    if(!impl) return 0;
    //return ((CSSStyleDeclarationImpl *)impl)->getPropertyCSSValue( propertyName );
}

DOMString CSSStyleDeclaration::removeProperty( const DOMString &propertyName )
{
    //if(!impl) return 0;
    //return ((CSSStyleDeclarationImpl *)impl)->removeProperty( propertyName );
}

DOMString CSSStyleDeclaration::getPropertyPriority( const DOMString &propertyName )
{
    //if(!impl) return 0;
    //return ((CSSStyleDeclarationImpl *)impl)->getPropertyPriority( propertyName );
}

void CSSStyleDeclaration::setProperty( const DOMString &propertyName, const DOMString &value, const DOMString &priority )
{
    //if(impl)
    //   ((CSSStyleDeclarationImpl *)impl)->setProperty( propertyName, value, priority );
}

unsigned long CSSStyleDeclaration::length() const
{
    //if(!impl) return 0;
    //return ((CSSStyleDeclarationImpl *)impl)->length();
}

DOMString CSSStyleDeclaration::item( unsigned long index )
{
    //if(!impl) return 0;
    //return ((CSSStyleDeclarationImpl *)impl)->item( index );
}

CSSRule CSSStyleDeclaration::parentRule() const
{
    //if(!impl) return 0;
    //return ((CSSStyleDeclarationImpl *)impl)->parentRule();
}



CSSValue::CSSValue()
{
    impl = 0;
}

CSSValue::CSSValue(const CSSValue &other)
{
    impl = other.impl;
    if(impl) impl->ref();
}

CSSValue::CSSValue(CSSValueImpl *i)
{
    impl = i;
    if(impl) impl->ref();
}

CSSValue &CSSValue::operator = (const CSSValue &other)
{
    if(impl) impl->deref();
    impl = other.impl;
    if(impl) impl->ref();
    return *this;
}

CSSValue::~CSSValue()
{
    if(impl) impl->deref();
}

DOMString CSSValue::cssText() const
{
    if(!impl) return 0;
    //return ((ElementImpl *)impl)->getAttribute("cssText");
}

void CSSValue::setCssText( const DOMString &value )
{
    //if(impl) ((ElementImpl *)impl)->setAttribute("cssText", value);
}

unsigned short CSSValue::valueType() const
{
    if(!impl) return 0;
    return ((CSSValueImpl *)impl)->valueType();
}



CSSValueList::CSSValueList() : CSSValue()
{
}

CSSValueList::CSSValueList(const CSSValueList &other) : CSSValue(other)
{
}

CSSValueList::CSSValueList(CSSValueListImpl *impl) : CSSValue(impl)
{
}

CSSValueList &CSSValueList::operator = (const CSSValueList &other)
{
    //CSSValue::operator = (other);
    //return *this;
}

CSSValueList::~CSSValueList()
{
}

unsigned long CSSValueList::length() const
{
    if(!impl) return 0;
    return ((CSSValueListImpl *)impl)->length();
}

CSSValue CSSValueList::item( unsigned long index )
{
    //if(!impl) return 0;
    //return ((CSSValueListImpl *)impl)->item( index );
}



Counter::Counter()
{
}

Counter::Counter(const Counter &other)
{
}

Counter &Counter::operator = (const Counter &other)
{
    //::operator = (other);
    //return *this;
}

Counter::~Counter()
{
}

DOMString Counter::identifier() const
{
}

void Counter::setIdentifier( const DOMString &value )
{
}

DOMString Counter::listStyle() const
{
}

void Counter::setListStyle( const DOMString &value )
{
}

DOMString Counter::separator() const
{
}

void Counter::setSeparator( const DOMString &value )
{
}



RGBColor::RGBColor()
{
}

RGBColor::RGBColor(const RGBColor &other)
{
}

RGBColor &RGBColor::operator = (const RGBColor &other)
{
}

RGBColor::~RGBColor()
{
}

CSSValue RGBColor::red() const
{
}

void RGBColor::setRed( const CSSValue &_red )
{
}

CSSValue RGBColor::green() const
{
}

void RGBColor::setGreen( const CSSValue &_green )
{
}

CSSValue RGBColor::blue() const
{
}

void RGBColor::setBlue( const CSSValue &_blue )
{
}



Rect::Rect()
{
}

Rect::Rect(const Rect &other)
{
}

Rect &Rect::operator = (const Rect &other)
{
}

Rect::~Rect()
{
}

CSSValue Rect::top() const
{
}

void Rect::setTop( const CSSValue &_top )
{
}

CSSValue Rect::right() const
{
}

void Rect::setRight( const CSSValue &_right )
{
}

CSSValue Rect::bottom() const
{
}

void Rect::setBottom( const CSSValue &_bottom )
{
}

CSSValue Rect::left() const
{
}

void Rect::setLeft( const CSSValue &_left )
{
}



CSSPrimitiveValue::CSSPrimitiveValue() : CSSValue()
{
}

CSSPrimitiveValue::CSSPrimitiveValue(const CSSPrimitiveValue &other) : CSSValue(other)
{
}

CSSPrimitiveValue::CSSPrimitiveValue(CSSPrimitiveValueImpl *impl) : CSSValue(impl)
{
}

CSSPrimitiveValue &CSSPrimitiveValue::operator = (const CSSPrimitiveValue &other)
{
    //CSSValue::operator = (other);
    //return *this;
}

CSSPrimitiveValue::~CSSPrimitiveValue()
{
}

unsigned short CSSPrimitiveValue::primitiveType() const
{
    if(!impl) return 0;
    return ((CSSPrimitiveValueImpl *)impl)->primitiveType();
}

void CSSPrimitiveValue::setFloatValue( unsigned short unitType, float floatValue )
{
    //if(impl)
    //    ((CSSPrimitiveValueImpl *)impl)->setFloatValue( unitType, floatValue );
}

float CSSPrimitiveValue::getFloatValue( unsigned short unitType )
{
    if(!impl) return 0;
    return ((CSSPrimitiveValueImpl *)impl)->getFloatValue( unitType );
}

void CSSPrimitiveValue::setStringValue( unsigned short stringType, const DOMString &stringValue )
{
    if(impl)
        ((CSSPrimitiveValueImpl *)impl)->setStringValue( stringType, stringValue );
}

DOMString CSSPrimitiveValue::getStringValue(  )
{
    //if(!impl) return 0;
    //return ((CSSPrimitiveValueImpl *)impl)->getStringValue(  );
}

Counter CSSPrimitiveValue::getCounterValue(  )
{
    //if(!impl) return 0;
    //return ((CSSPrimitiveValueImpl *)impl)->getCounterValue(  );
}

Rect CSSPrimitiveValue::getRectValue(  )
{
    //if(!impl) return 0;
    //return ((CSSPrimitiveValueImpl *)impl)->getRectValue(  );
}

RGBColor CSSPrimitiveValue::getRGBColorValue(  )
{
    //if(!impl) return 0;
    //return ((CSSPrimitiveValueImpl *)impl)->getRGBColorValue(  );
}



