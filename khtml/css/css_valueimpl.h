/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 2004, 2005, 2006 Apple Computer, Inc.
 *           (C) 2005 Allan Sandfeld Jensen (kde@carewolf.com)
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifndef _CSS_css_valueimpl_h_
#define _CSS_css_valueimpl_h_

#include "dom/css_value.h"
#include "dom/dom_string.h"
#include "css/css_base.h"
#include "misc/loader_client.h"
#include "misc/shared.h"

namespace khtml {
    class RenderStyle;
    class CachedImage;
}


namespace DOM {

class CSSRuleImpl;
class CSSValueImpl;
class NodeImpl;
class CounterImpl;
class PairImpl;


class CSSStyleDeclarationImpl : public StyleBaseImpl
{
public:
    CSSStyleDeclarationImpl(CSSRuleImpl *parentRule);
    CSSStyleDeclarationImpl(CSSRuleImpl *parentRule, QList<CSSProperty*> *lstValues);
    virtual ~CSSStyleDeclarationImpl();

    CSSStyleDeclarationImpl& operator=( const CSSStyleDeclarationImpl&);

    virtual unsigned long length() const;
    CSSRuleImpl *parentRule() const;
    virtual void removeProperty(int propertyID, DOM::DOMString* old = 0);
    virtual void removePropertiesInSet(const int* set, unsigned length);
    virtual bool setProperty ( int propertyId, const DOM::DOMString &value, bool important, int &ec);
    virtual bool setProperty ( int propertyId, const DOM::DOMString &value, bool important = false);
    virtual void setProperty ( int propertyId, int value, bool important = false);
    virtual void clear();
    // this treats integers as pixels!
    // needed for conversion of html attributes
    virtual void setLengthProperty(int id, const DOM::DOMString &value, bool important, bool multiLength = false);

    void setProperty(const DOMString &propertyName, const DOMString &value, const DOMString &priority);
    DOMString removeProperty(const DOMString &propertyName);

    // add a whole, unparsed property
    virtual void setProperty ( const DOMString &propertyString);
    virtual DOM::DOMString item ( unsigned long index ) const;

    DOM::DOMString cssText() const;
    void setCssText(DOM::DOMString str);

    virtual bool isStyleDeclaration() const { return true; }
    virtual bool parseString( const DOMString &string, bool = false );

    CSSValueImpl *getPropertyCSSValue(const DOMString &propertyName) const;
    DOMString getPropertyValue(const DOMString &propertyName) const ;
    DOMString getPropertyPriority(const DOMString &propertyName) const;

    virtual CSSValueImpl *getPropertyCSSValue( int propertyID ) const;
    virtual DOMString getPropertyValue( int propertyID ) const;
    virtual bool getPropertyPriority( int propertyID ) const;

    QList<CSSProperty*> *values() const { return m_lstValues; }
    void setNode(NodeImpl *_node) { m_node = _node; }

    void setChanged();

    void removeCSSHints();

protected:
    DOMString getShortHandValue( const int* properties, int number ) const;
    DOMString getLayeredShortHandValue(const int* properties, unsigned number) const;
    DOMString get4Values( const int* properties ) const;

    QList<CSSProperty*> *m_lstValues;
    NodeImpl *m_node;

private:
    // currently not needed - make sure its not used
    CSSStyleDeclarationImpl(const CSSStyleDeclarationImpl& o);
};

class CSSValueImpl : public StyleBaseImpl
{
public:
    CSSValueImpl() : StyleBaseImpl() {}

    virtual unsigned short cssValueType() const = 0;

    virtual DOM::DOMString cssText() const = 0;

    virtual bool isValue() const { return true; }
    virtual bool isFontValue() const { return false; }
    virtual bool isImplicitInitialValue() const { return false; }
};

class CSSInheritedValueImpl : public CSSValueImpl
{
public:
    CSSInheritedValueImpl() : CSSValueImpl() {}
    virtual ~CSSInheritedValueImpl() {}

    virtual unsigned short cssValueType() const;
    virtual DOM::DOMString cssText() const;
};

class CSSInitialValueImpl : public CSSValueImpl
{
public:
    CSSInitialValueImpl(bool implicit)
     :m_implicit(implicit)
    {}
    virtual unsigned short cssValueType() const;
    virtual DOM::DOMString cssText() const;

    virtual bool isImplicitInitialValue() const { return m_implicit; }
private:
    bool m_implicit;          
};

class CSSValueListImpl : public CSSValueImpl
{
public:
    CSSValueListImpl() : CSSValueImpl() {}

    virtual ~CSSValueListImpl();

    unsigned long length() const { return m_values.count(); }
    CSSValueImpl *item ( unsigned long index ) { return m_values.at(index); }

    virtual bool isValueList() const { return true; }

    virtual unsigned short cssValueType() const;

    void append(CSSValueImpl *val);
    virtual DOM::DOMString cssText() const;

protected:
    QList<CSSValueImpl*> m_values;
};


class Counter;
class RGBColor;
class Rect;

class CSSPrimitiveValueImpl : public CSSValueImpl
{
public:
    CSSPrimitiveValueImpl();
    CSSPrimitiveValueImpl(int ident);
    CSSPrimitiveValueImpl(double num, CSSPrimitiveValue::UnitTypes type);
    CSSPrimitiveValueImpl(const DOMString &str, CSSPrimitiveValue::UnitTypes type);
    CSSPrimitiveValueImpl(CounterImpl *c);
    CSSPrimitiveValueImpl( RectImpl *r);
    CSSPrimitiveValueImpl(QRgb color);
    CSSPrimitiveValueImpl(PairImpl *p);

    virtual ~CSSPrimitiveValueImpl();

    void cleanup();

    unsigned short primitiveType() const { return m_type; }

    /*
     * computes a length in pixels out of the given CSSValue. Need the RenderStyle to get
     * the fontinfo in case val is defined in em or ex.
     *
     * The metrics have to be a bit different for screen and printer output.
     * For screen output we assume 1 inch == 72 px, for printer we assume 300 dpi
     *
     * this is screen/printer dependent, so we probably need a config option for this,
     * and some tool to calibrate.
     */
    int computeLength( khtml::RenderStyle *style, int logicalDpiY);

    double computeLengthFloat( khtml::RenderStyle *style, int logicalDpiY);

    // Retrieves an explicit resolution from the CSSValue if it contains one.
    // This is specific to the CSS3 Media Queries module's resolution feature.
    int getDPIResolution() const;

    // use with care!!!
    void setPrimitiveType(unsigned short type) { m_type = type; }
    void setFloatValue ( unsigned short unitType, double floatValue, int &exceptioncode );
    double floatValue ( unsigned short unitType = CSSPrimitiveValue::CSS_UNKNOWN) const { (void)unitType; return m_value.num; }

    void setStringValue ( unsigned short stringType, const DOM::DOMString &stringValue, int &exceptioncode );
    DOM::DOMStringImpl *getStringValue () const {
	return ( ( m_type < CSSPrimitiveValue::CSS_STRING ||
		   m_type > CSSPrimitiveValue::CSS_ATTR ||
		   m_type == CSSPrimitiveValue::CSS_IDENT ) ? // fix IDENT
		 0 : m_value.string );
    }
    CounterImpl *getCounterValue () const {
        return ( m_type != CSSPrimitiveValue::CSS_COUNTER ? 0 : m_value.counter );
    }

    RectImpl *getRectValue () const {
	return ( m_type != CSSPrimitiveValue::CSS_RECT ? 0 : m_value.rect );
    }

    QRgb getRGBColorValue () const {
	return ( m_type != CSSPrimitiveValue::CSS_RGBCOLOR ? 0 : m_value.rgbcolor );
    }

    PairImpl* getPairValue() const {
        return (m_type != CSSPrimitiveValue::CSS_PAIR ? 0 : m_value.pair);
    }

    virtual bool isPrimitiveValue() const { return true; }
    virtual unsigned short cssValueType() const;

    int getIdent();

    virtual bool parseString( const DOMString &string, bool = false);
    virtual DOM::DOMString cssText() const;

    virtual bool isQuirkValue() const { return false; }

protected:
    int m_type;
    union {
	int ident;
	double num;
	DOM::DOMStringImpl *string;
	CounterImpl *counter;
	RectImpl *rect;
        QRgb rgbcolor;
        PairImpl* pair;
    } m_value;
};

// This value is used to handle quirky margins in reflow roots (body, td, and th) like WinIE.
// The basic idea is that a stylesheet can use the value __qem (for quirky em) instead of em
// in a stylesheet.  When the quirky value is used, if you're in quirks mode, the margin will
// collapse away inside a table cell.
class CSSQuirkPrimitiveValueImpl : public CSSPrimitiveValueImpl
{
public:
    CSSQuirkPrimitiveValueImpl(double num, CSSPrimitiveValue::UnitTypes type)
      :CSSPrimitiveValueImpl(num, type) {}

    virtual ~CSSQuirkPrimitiveValueImpl() {}

    virtual bool isQuirkValue() const { return true; }
};

class CounterImpl : public khtml::Shared<CounterImpl> {
public:
    CounterImpl() : m_listStyle(0) { }
    DOMString identifier() const { return m_identifier; }
    unsigned int listStyle() const { return m_listStyle; }
    DOMString separator() const { return m_separator; }

    DOMString m_identifier;
    unsigned int m_listStyle;
    DOMString m_separator;
};

class RectImpl : public khtml::Shared<RectImpl> {
public:
    RectImpl();
    ~RectImpl();

    CSSPrimitiveValueImpl *top() const { return m_top; }
    CSSPrimitiveValueImpl *right() const { return m_right; }
    CSSPrimitiveValueImpl *bottom() const { return m_bottom; }
    CSSPrimitiveValueImpl *left() const { return m_left; }

    void setTop( CSSPrimitiveValueImpl *top );
    void setRight( CSSPrimitiveValueImpl *right );
    void setBottom( CSSPrimitiveValueImpl *bottom );
    void setLeft( CSSPrimitiveValueImpl *left );
protected:
    CSSPrimitiveValueImpl *m_top;
    CSSPrimitiveValueImpl *m_right;
    CSSPrimitiveValueImpl *m_bottom;
    CSSPrimitiveValueImpl *m_left;
};

// A primitive value representing a pair.  This is useful for properties like border-radius, background-size/position,
// and border-spacing (all of which are space-separated sets of two values).  At the moment we are only using it for
// border-radius and background-size, but (FIXME) border-spacing and background-position could be converted over to use
// it (eliminating some extra -webkit- internal properties).
class PairImpl : public khtml::Shared<PairImpl> {
public:
    PairImpl() : m_first(0), m_second(0) { }
    PairImpl(CSSPrimitiveValueImpl* first, CSSPrimitiveValueImpl* second)
        : m_first(first), m_second(second) { if (first) first->ref(); if (second) second->ref(); }
    virtual ~PairImpl();

    CSSPrimitiveValueImpl* first() const { return m_first; }
    CSSPrimitiveValueImpl* second() const { return m_second; }

    void setFirst(CSSPrimitiveValueImpl* first);
    void setSecond(CSSPrimitiveValueImpl* second);

protected:
    CSSPrimitiveValueImpl* m_first;
    CSSPrimitiveValueImpl* m_second;
};


class CSSImageValueImpl : public CSSPrimitiveValueImpl, public khtml::CachedObjectClient
{
public:
    CSSImageValueImpl(const DOMString &url, const StyleBaseImpl *style);
    CSSImageValueImpl();
    virtual ~CSSImageValueImpl();

    khtml::CachedImage *image() { return m_image; }
protected:
    khtml::CachedImage *m_image;
};

class FontFamilyValueImpl : public CSSPrimitiveValueImpl
{
public:
    FontFamilyValueImpl( const QString &string);
    const QString &fontName() const { return parsedFontName; }
    int genericFamilyType() const { return _genericFamilyType; }
protected:
    QString parsedFontName;
private:
    int _genericFamilyType;
};

class FontValueImpl : public CSSValueImpl
{
public:
    FontValueImpl();
    virtual ~FontValueImpl();

    virtual unsigned short cssValueType() const { return CSSValue::CSS_CUSTOM; }

    virtual DOM::DOMString cssText() const;

    virtual bool isFontValue() const { return true; }

    CSSPrimitiveValueImpl *style;
    CSSPrimitiveValueImpl *variant;
    CSSPrimitiveValueImpl *weight;
    CSSPrimitiveValueImpl *size;
    CSSPrimitiveValueImpl *lineHeight;
    CSSValueListImpl *family;
};

// Used for quotes
class QuotesValueImpl : public CSSValueImpl
{
public:
    QuotesValueImpl();
//    virtual ~QuotesValueImpl();

    virtual unsigned short cssValueType() const { return CSSValue::CSS_CUSTOM; }
    virtual DOM::DOMString cssText() const;

    void addLevel(const QString& open, const QString& close);
    QString openQuote(int level) const;
    QString closeQuote(int level) const;

    unsigned int levels;
    QStringList data;
};

// Used for text-shadow and box-shadow
class ShadowValueImpl : public CSSValueImpl
{
public:
    ShadowValueImpl(CSSPrimitiveValueImpl* _x, CSSPrimitiveValueImpl* _y,
                    CSSPrimitiveValueImpl* _blur, CSSPrimitiveValueImpl* _color);
    virtual ~ShadowValueImpl();

    virtual unsigned short cssValueType() const { return CSSValue::CSS_CUSTOM; }

    virtual DOM::DOMString cssText() const;

    CSSPrimitiveValueImpl* x;
    CSSPrimitiveValueImpl* y;
    CSSPrimitiveValueImpl* blur;
    CSSPrimitiveValueImpl* color;
};

// Used for counter-reset and counter-increment
class CounterActImpl : public CSSValueImpl {
    public:
        CounterActImpl(const DOMString &c, short v) : m_counter(c), m_value(v) { }
        virtual ~CounterActImpl() {}

        virtual unsigned short cssValueType() const { return CSSValue::CSS_CUSTOM; }
        virtual DOM::DOMString cssText() const;

        const DOMString& counter() const { return m_counter; }
        short value() const { return m_value; }
        void setValue( const short v ) { m_value = v; }

        DOM::DOMString m_counter;
        short m_value;
};


// ------------------------------------------------------------------------------

// another helper class
class CSSProperty
{
public:
    CSSProperty()
    {
	m_id = -1;
	m_important = false;
        m_value = 0;
    }
    CSSProperty(const CSSProperty& o)
    {
        m_id = o.m_id;
        m_important = o.m_important;
        m_value = o.m_value;
        if (m_value) m_value->ref();
    }
    ~CSSProperty() {
	if(m_value) m_value->deref();
    }

    void setValue(CSSValueImpl *val) {
	if ( val != m_value ) {
	    if(m_value) m_value->deref();
	    m_value = val;
	    if(m_value) m_value->ref();
	}
    }

    int id() const { return m_id; }

    bool isImportant() const { return m_important; }

    CSSValueImpl *value() const { return m_value; }

    DOM::DOMString cssText() const;

    // make sure the following fits in 4 bytes.
    signed int  m_id   : 29;
    bool m_important   : 1;
protected:
    CSSValueImpl *m_value;
};


} // namespace

#endif
