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
#ifndef _CSS_css_valueimpl_h_
#define _CSS_css_valueimpl_h_

#include <css_value.h>
//#include <css_stylesheetimpl.h>
#include <dom_string.h>
#include "css_parser.h"

#include <qintdict.h>

namespace DOM {

class CSSRuleImpl;
class CSSValueImpl;


class CSSStyleDeclarationImpl : public StyleBaseImpl
{
public:
    CSSStyleDeclarationImpl(CSSRuleImpl *parentRule);
    CSSStyleDeclarationImpl(CSSRuleImpl *parentRule, QList<CSSProperty> *lstValues);

    virtual ~CSSStyleDeclarationImpl();

    unsigned long length() const;
    CSSRuleImpl *parentRule() const;
    DOM::DOMString getPropertyValue ( const DOM::DOMString &propertyName );
    CSSValueImpl *getPropertyCSSValue ( const DOM::DOMString &propertyName );
    DOM::DOMString removeProperty ( const DOM::DOMString &propertyName );
    DOM::DOMString getPropertyPriority ( const DOM::DOMString &propertyName );
    void setProperty ( const DOM::DOMString &propertyName, const DOM::DOMString &value, const DOM::DOMString &priority );
    DOM::DOMString item ( unsigned long index );

    DOM::DOMString cssText() const;
    void setCssText(DOM::DOMString str);

    virtual bool isStyleDeclaration() { return true; }

    virtual bool parseString( const DOMString &string );
protected:
    QList<CSSProperty> *m_lstValues;
};

class CSSValueImpl : public StyleBaseImpl
{
public:
    CSSValueImpl();

    virtual ~CSSValueImpl();

    virtual unsigned short valueType() const = 0;

    DOM::DOMString cssText() const;
    void setCssText(DOM::DOMString str);

    virtual bool isValue() { return true; }
};

class CSSInheritedValueImpl : public CSSValueImpl
{
public:
    CSSInheritedValueImpl() : CSSValueImpl() {}
    virtual ~CSSInheritedValueImpl() {}

    virtual unsigned short valueType() const { return CSSValue::CSS_INHERIT; }
};


class CSSValueListImpl : public CSSValueImpl
{
public:
    CSSValueListImpl();

    virtual ~CSSValueListImpl();

    unsigned long length() const;
    CSSValueImpl *item ( unsigned long index );

    virtual bool isValueList() { return true; }

    virtual unsigned short valueType() const;
};


class Counter;
class RGBColor;
class Rect;

class CSSPrimitiveValueImpl : public CSSValueImpl
{
public:
    CSSPrimitiveValueImpl();
    CSSPrimitiveValueImpl(int ident);
    CSSPrimitiveValueImpl(int num, CSSValue::UnitTypes type);
    CSSPrimitiveValueImpl(const DOMString &str, CSSValue::UnitTypes type);
    CSSPrimitiveValueImpl(const Counter &c);
    CSSPrimitiveValueImpl(const Rect &r);
    CSSPrimitiveValueImpl(const RGBColor &rgb);
    
    virtual ~CSSPrimitiveValueImpl();

    unsigned short primitiveType() const;
    void setFloatValue ( unsigned short unitType, float floatValue );
    float getFloatValue ( unsigned short unitType );
    void setStringValue ( unsigned short stringType, const DOM::DOMString &stringValue );
    DOM::DOMStringImpl *getStringValue (  );
    Counter *getCounterValue (  );
    Rect *getRectValue (  );
    RGBColor *getRGBColorValue (  );

    virtual bool isPrimitiveValue() { return true; }
    virtual unsigned short valueType() const;

    int getIdent();

    virtual bool parseString( const DOMString &string );

protected:
    int m_type;
    union {
	int ident;
	float num;
	DOM::DOMStringImpl *string;
	Counter *counter;
	Rect *rect;
	RGBColor *rgbcolor;
    } m_value;
};
// ------------------------------------------------------------------------------

}; // namespace

#endif
