// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _KJS_CSS_H_
#define _KJS_CSS_H_

#include <dom/dom_node.h>
#include <kjs/object.h>
#include <dom/css_value.h>
#include <dom/css_stylesheet.h>
#include <dom/css_rule.h>
#include "kjs_binding.h"

namespace KJS {

  class DOMCSSStyleDeclaration : public DOMObject {
  public:
    DOMCSSStyleDeclaration(ExecState *exec, DOM::CSSStyleDeclaration s);
    virtual ~DOMCSSStyleDeclaration();
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    virtual bool hasProperty(ExecState *exec, const UString &propertyName, bool recursive = true) const;
    virtual const ClassInfo *classInfo() const { return &info; }
    static const ClassInfo info;
    enum { CssText, Length, ParentRule,
           GetPropertyValue, GetPropertyCSSValue, RemoveProperty, GetPropertyPriority,
           SetProperty, Item };
    DOM::CSSStyleDeclaration toStyleDecl() const { return styleDecl; }
  protected:
    DOM::CSSStyleDeclaration styleDecl;
  };

  Value getDOMCSSStyleDeclaration(ExecState *exec, DOM::CSSStyleDeclaration n);

  class DOMStyleSheet : public DOMObject {
  public:
    // Build a DOMStyleSheet
    DOMStyleSheet(ExecState *, DOM::StyleSheet ss) : styleSheet(ss) { }
    // Constructor for inherited classes
    DOMStyleSheet(Object proto, DOM::StyleSheet ss) : DOMObject(proto), styleSheet(ss) { }
    virtual ~DOMStyleSheet();
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    Value getValue(ExecState *exec, int token) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    virtual bool toBoolean(ExecState *) const { return true; }
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Type, Disabled, OwnerNode, ParentStyleSheet, Href, Title, Media };
  protected:
    DOM::StyleSheet styleSheet;
  };

  Value getDOMStyleSheet(ExecState *exec, DOM::StyleSheet ss);

  class DOMStyleSheetList : public DOMObject {
  public:
    DOMStyleSheetList(ExecState *, DOM::StyleSheetList ssl) : styleSheetList(ssl) { }
    virtual ~DOMStyleSheetList();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    virtual bool toBoolean() const { return true; }
    static const ClassInfo info;
    DOM::StyleSheetList toStyleSheetList() const { return styleSheetList; }
    enum { Item, Length };
  private:
    DOM::StyleSheetList styleSheetList;
  };

  Value getDOMStyleSheetList(ExecState *exec, DOM::StyleSheetList ss);

  class DOMMediaList : public DOMObject {
  public:
    DOMMediaList(ExecState *, DOM::MediaList ml);
    virtual ~DOMMediaList();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    virtual const ClassInfo* classInfo() const { return &info; }
    virtual bool toBoolean() const { return true; }
    static const ClassInfo info;
    enum { MediaText, Length,
           Item, DeleteMedium, AppendMedium };
    DOM::MediaList toMediaList() const { return mediaList; }
  private:
    DOM::MediaList mediaList;
  };

  Value getDOMMediaList(ExecState *exec, DOM::MediaList ss);

  class DOMCSSStyleSheet : public DOMStyleSheet {
  public:
    DOMCSSStyleSheet(ExecState *exec, DOM::CSSStyleSheet ss);
    virtual ~DOMCSSStyleSheet();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { OwnerRule, CssRules,
           InsertRule, DeleteRule };
    DOM::CSSStyleSheet toCSSStyleSheet() const { return static_cast<DOM::CSSStyleSheet>(styleSheet); }
  };

  class DOMCSSRuleList : public DOMObject {
  public:
    DOMCSSRuleList(ExecState *, DOM::CSSRuleList rl) : cssRuleList(rl) { }
    virtual ~DOMCSSRuleList();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Item, Length };
    DOM::CSSRuleList toCSSRuleList() const { return cssRuleList; }
  protected:
    DOM::CSSRuleList cssRuleList;
  };

  Value getDOMCSSRuleList(ExecState *exec, DOM::CSSRuleList rl);

  class DOMCSSRule : public DOMObject {
  public:
    DOMCSSRule(ExecState *, DOM::CSSRule r) : cssRule(r) { }
    virtual ~DOMCSSRule();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Item, InsertRule, DeleteRule };
    DOM::CSSRule toCSSRule() const { return cssRule; }
  protected:
    DOM::CSSRule cssRule;
  };

  Value getDOMCSSRule(ExecState *exec, DOM::CSSRule r);

  /**
   * Convert an object to a CSSRule. Returns a null CSSRule if not possible.
   */
  DOM::CSSRule toCSSRule(const Value&);

  // Constructor for CSSRule - currently only used for some global values
  class CSSRuleConstructor : public DOMObject {
  public:
    CSSRuleConstructor(ExecState *) { }
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    Value getValue(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { UNKNOWN_RULE, STYLE_RULE, CHARSET_RULE, IMPORT_RULE, MEDIA_RULE, FONT_FACE_RULE, PAGE_RULE };
  };

  Value getCSSRuleConstructor(ExecState *exec);

  class DOMCSSValue : public DOMObject {
  public:
    DOMCSSValue(ExecState *, DOM::CSSValue v) : cssValue(v) { }
    virtual ~DOMCSSValue();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { CssText, CssValueType };
  protected:
    DOM::CSSValue cssValue;
  };

  Value getDOMCSSValue(ExecState *exec, DOM::CSSValue v);

  // Constructor for CSSValue - currently only used for some global values
  class CSSValueConstructor : public DOMObject {
  public:
    CSSValueConstructor(ExecState *) { }
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    Value getValue(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { CSS_VALUE_LIST, CSS_PRIMITIVE_VALUE, CSS_CUSTOM, CSS_INHERIT };
  };

  Value getCSSValueConstructor(ExecState *exec);

  class DOMCSSPrimitiveValue : public DOMCSSValue {
  public:
    DOMCSSPrimitiveValue(ExecState *exec, DOM::CSSPrimitiveValue v);
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  class DOMCSSPrimitiveValueFunc : public DOMFunction {
    friend class DOMNode;
  public:
    DOMCSSPrimitiveValueFunc(DOM::CSSPrimitiveValue v, int i)
        : DOMFunction(), val(v), id(i) { }
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List&args);
    enum { SetFloatValue, GetFloatValue, SetStringValue, GetStringValue,
           GetCounterValue, GetRectValue, GetRGBColorValue };
  private:
    DOM::CSSPrimitiveValue val;
    int id;
  };

  // Constructor for CSSPrimitiveValue - currently only used for some global values
  class CSSPrimitiveValueConstructor : public CSSValueConstructor {
  public:
    CSSPrimitiveValueConstructor(ExecState *exec) : CSSValueConstructor(exec) { }
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  Value getCSSPrimitiveValueConstructor(ExecState *exec);

  class DOMCSSValueList : public DOMCSSValue {
  public:
    DOMCSSValueList(ExecState *exec, DOM::CSSValueList v);
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  class DOMCSSValueListFunc : public DOMFunction {
  public:
    DOMCSSValueListFunc(DOM::CSSValueList vl, int i)
        : DOMFunction(), valueList(vl), id(i) { }
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List&args);
    enum { Item };
  private:
    DOM::CSSValueList valueList;
    int id;
  };

  class DOMRGBColor : public DOMObject {
  public:
    DOMRGBColor(DOM::RGBColor c) : rgbColor(c) { }
    ~DOMRGBColor();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  protected:
    DOM::RGBColor rgbColor;
  };

  Value getDOMRGBColor(ExecState *exec, DOM::RGBColor c);

  class DOMRect : public DOMObject {
  public:
    DOMRect(DOM::Rect r) : rect(r) { }
    ~DOMRect();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  protected:
    DOM::Rect rect;
  };

  Value getDOMRect(ExecState *exec, DOM::Rect r);

  class DOMCounter : public DOMObject {
  public:
    DOMCounter(DOM::Counter c) : counter(c) { }
    ~DOMCounter();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  protected:
    DOM::Counter counter;
  };

  Value getDOMCounter(ExecState *exec, DOM::Counter c);

}; // namespace

#endif
