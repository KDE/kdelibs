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
#include "css_valueimpl.h"
#include "css_value.h"
#include "css_ruleimpl.h"
#include "css_stylesheetimpl.h"
#include "css_parser.h"
#include "dom_exception.h"
#include "dom_string.h"

using namespace DOM;

CSSStyleDeclarationImpl::CSSStyleDeclarationImpl(CSSRuleImpl *parent)
    : StyleBaseImpl(parent)
{
    m_lstValues = 0;
}

CSSStyleDeclarationImpl::CSSStyleDeclarationImpl(CSSRuleImpl *parent, QList<CSSProperty> *lstValues)
    : StyleBaseImpl(parent)
{
    m_lstValues = lstValues;
}

CSSStyleDeclarationImpl::~CSSStyleDeclarationImpl()
{
    // ####
}

DOMString CSSStyleDeclarationImpl::getPropertyValue( const DOMString &propertyName )
{
    // ###
}

CSSValueImpl *CSSStyleDeclarationImpl::getPropertyCSSValue( const DOMString &propertyName )
{
    // ###
}

DOMString CSSStyleDeclarationImpl::removeProperty( const DOMString &propertyName )
{
    // ###
}

DOMString CSSStyleDeclarationImpl::getPropertyPriority( const DOMString &propertyName )
{
    // ###
}

void CSSStyleDeclarationImpl::setProperty( const DOMString &propertyName, const DOMString &value, const DOMString &priority )
{
    // ###
}

unsigned long CSSStyleDeclarationImpl::length() const
{
    if(!m_lstValues) return 0;
    return m_lstValues->count();
}

DOMString CSSStyleDeclarationImpl::item( unsigned long index )
{
    // ###
}

CSSRuleImpl *CSSStyleDeclarationImpl::parentRule() const
{
    if( !m_parent ) return 0;
    if( m_parent->isRule() ) return static_cast<CSSRuleImpl *>(m_parent);
    return 0;
}

DOM::DOMString CSSStyleDeclarationImpl::cssText() const
{
    // ###
}

void CSSStyleDeclarationImpl::setCssText(DOM::DOMString str)
{
    // ###
}

bool CSSStyleDeclarationImpl::parseString( const DOMString &string )
{
    // ###
}


// --------------------------------------------------------------------------------------

CSSValueImpl::CSSValueImpl()
    : StyleBaseImpl()
{
}

CSSValueImpl::~CSSValueImpl()
{
}

DOM::DOMString CSSValueImpl::cssText() const
{
    // ###
}

void CSSValueImpl::setCssText(DOM::DOMString str)
{
    // ###
}

// ----------------------------------------------------------------------------------------

CSSValueListImpl::CSSValueListImpl()
    : CSSValueImpl()
{
}

CSSValueListImpl::~CSSValueListImpl()
{
}

unsigned long CSSValueListImpl::length() const
{
}

CSSValueImpl *CSSValueListImpl::item( unsigned long index )
{
}

unsigned short CSSValueListImpl::valueType() const
{
    return CSSValue::CSS_VALUE_LIST;
}

// -------------------------------------------------------------------------------------

CSSPrimitiveValueImpl::CSSPrimitiveValueImpl()
    : CSSValueImpl()
{
    m_ident = 0;
    m_type = 0;
}

CSSPrimitiveValueImpl::CSSPrimitiveValueImpl(int ident)
    : CSSValueImpl()
{
    m_ident = ident;
    m_type = CSSPrimitiveValue::CSS_IDENT;
}

CSSPrimitiveValueImpl::~CSSPrimitiveValueImpl()
{
}

unsigned short CSSPrimitiveValueImpl::primitiveType() const
{
}

void CSSPrimitiveValueImpl::setFloatValue( unsigned short unitType, float floatValue )
{
}

float CSSPrimitiveValueImpl::getFloatValue( unsigned short unitType )
{
}

void CSSPrimitiveValueImpl::setStringValue( unsigned short stringType, const DOMString &stringValue )
{
}

DOMString CSSPrimitiveValueImpl::getStringValue(  )
{
}

Counter CSSPrimitiveValueImpl::getCounterValue(  )
{
}

Rect CSSPrimitiveValueImpl::getRectValue(  )
{
}

RGBColor CSSPrimitiveValueImpl::getRGBColorValue(  )
{
}

unsigned short CSSPrimitiveValueImpl::valueType() const
{
    return CSSValue::CSS_PRIMITIVE_VALUE;
}

bool CSSPrimitiveValueImpl::parseString( const DOMString &string )
{
}

