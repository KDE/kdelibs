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
    DOMCSSStyleDeclaration(DOM::CSSStyleDeclaration s) : styleDecl(s) { }
    virtual ~DOMCSSStyleDeclaration();
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    virtual bool hasProperty(ExecState *exec, const UString &propertyName, bool recursive = true) const;
    virtual const ClassInfo *classInfo() const { return &info; }
    static const ClassInfo info;
  protected:
    DOM::CSSStyleDeclaration styleDecl;
  };

  class DOMCSSStyleDeclarationFunc : public DOMFunction {
    friend class DOMNode;
  public:
    DOMCSSStyleDeclarationFunc(DOM::CSSStyleDeclaration s, int i)
        : DOMFunction(), styleDecl(s), id(i) { }
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List&args);
    enum { GetPropertyValue, GetPropertyCSSValue, RemoveProperty, GetPropertyPriority,
           SetProperty, Item };
  private:
    DOM::CSSStyleDeclaration styleDecl;
    int id;
  };

  Value getDOMCSSStyleDeclaration(DOM::CSSStyleDeclaration n);

  class DOMStyleSheet : public DOMObject {
  public:
    DOMStyleSheet(DOM::StyleSheet ss) : styleSheet(ss) { }
    virtual ~DOMStyleSheet();
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    virtual bool toBoolean(ExecState *) const { return true; }
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  protected:
    DOM::StyleSheet styleSheet;
  };

  Value getDOMStyleSheet(DOM::StyleSheet ss);

  class DOMStyleSheetList : public DOMObject {
  public:
    DOMStyleSheetList(DOM::StyleSheetList ssl) : styleSheetList(ssl) { }
    virtual ~DOMStyleSheetList();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    virtual bool toBoolean() const { return true; }
    static const ClassInfo info;
  private:
    DOM::StyleSheetList styleSheetList;
  };

  Value getDOMStyleSheetList(DOM::StyleSheetList ss);

  class DOMStyleSheetListFunc : public DOMFunction {
    friend class DOMStyleSheetList;
  public:
    DOMStyleSheetListFunc(DOM::StyleSheetList ssl, int i)
        : styleSheetList(ssl), id(i) { }
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List&args);
    enum { Item };
  private:
    DOM::StyleSheetList styleSheetList;
    int id;
  };

  class DOMMediaList : public DOMObject {
  public:
    DOMMediaList(DOM::MediaList ml) : mediaList(ml) { }
    virtual ~DOMMediaList();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    virtual const ClassInfo* classInfo() const { return &info; }
    virtual bool toBoolean() const { return true; }
    static const ClassInfo info;
  private:
    DOM::MediaList mediaList;
  };

  Value getDOMMediaList(DOM::MediaList ss);

  class DOMMediaListFunc : public DOMFunction {
    friend class DOMMediaList;
  public:
    DOMMediaListFunc(DOM::MediaList ml, int i)
        : mediaList(ml), id(i) { }
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List&args);
    enum { Item, DeleteMedium, AppendMedium };
  private:
    DOM::MediaList mediaList;
    int id;
  };

  class DOMCSSStyleSheet : public DOMStyleSheet {
  public:
    DOMCSSStyleSheet(DOM::CSSStyleSheet ss) : DOMStyleSheet(ss) { }
    virtual ~DOMCSSStyleSheet();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  class DOMCSSStyleSheetFunc : public DOMFunction {
    friend class DOMNode;
  public:
    DOMCSSStyleSheetFunc(DOM::CSSStyleSheet ss, int i)
        : DOMFunction(), styleSheet(ss), id(i) { }
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List&args);
    enum { InsertRule, DeleteRule };
  private:
    DOM::CSSStyleSheet styleSheet;
    int id;
  };

  class DOMCSSRuleList : public DOMObject {
  public:
    DOMCSSRuleList(DOM::CSSRuleList rl) : cssRuleList(rl) { }
    virtual ~DOMCSSRuleList();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  protected:
    DOM::CSSRuleList cssRuleList;
  };

  class DOMCSSRuleListFunc : public DOMFunction {
  public:
    DOMCSSRuleListFunc(DOM::CSSRuleList rl, int i)
        : DOMFunction(), cssRuleList(rl), id(i) { }
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List&args);
    enum { Item };
  private:
    DOM::CSSRuleList cssRuleList;
    int id;
  };

  Value getDOMCSSRuleList(DOM::CSSRuleList rl);

  class DOMCSSRule : public DOMObject {
  public:
    DOMCSSRule(DOM::CSSRule r) : cssRule(r) { }
    virtual ~DOMCSSRule();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    DOM::CSSRule toCSSRule() const { return cssRule; }
  protected:
    DOM::CSSRule cssRule;
  };

  class DOMCSSRuleFunc : public DOMFunction {
  public:
    DOMCSSRuleFunc(DOM::CSSRule r, int i)
        : DOMFunction(), cssRule(r), id(i) { }
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List&args);
    enum { Item, InsertRule, DeleteRule };
  private:
    DOM::CSSRule cssRule;
    int id;
  };

  Value getDOMCSSRule(DOM::CSSRule r);

  /**
   * Convert an object to a CSSRule. Returns a null CSSRule if not possible.
   */
  DOM::CSSRule toCSSRule(const Value&);

  // Constructor for CSSRule - currently only used for some global values
  class CSSRuleConstructor : public DOMObject {
  public:
    CSSRuleConstructor(ExecState *) { }
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  Value getCSSRuleConstructor(ExecState *exec);

  class DOMCSSValue : public DOMObject {
  public:
    DOMCSSValue(DOM::CSSValue v) : cssValue(v) { }
    virtual ~DOMCSSValue();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  protected:
    DOM::CSSValue cssValue;
  };

  Value getDOMCSSValue(DOM::CSSValue v);

  // Prototype object CSSValue
  class CSSValuePrototype : public DOMObject {
  public:
    CSSValuePrototype() { }
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  Value getCSSValuePrototype(ExecState *exec);

  class DOMCSSPrimitiveValue : public DOMCSSValue {
  public:
    DOMCSSPrimitiveValue(DOM::CSSPrimitiveValue v) : DOMCSSValue(v) { }
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

  // Prototype object CSSPrimitiveValue
  class CSSPrimitiveValuePrototype : public CSSValuePrototype {
  public:
    CSSPrimitiveValuePrototype() { } // ### classname
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  Value getCSSPrimitiveValuePrototype(ExecState *exec);

  class DOMCSSValueList : public DOMCSSValue {
  public:
    DOMCSSValueList(DOM::CSSValueList v) : DOMCSSValue(v) { } //## classname missing
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

  Value getDOMRGBColor(DOM::RGBColor c);

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

  Value getDOMRect(DOM::Rect r);

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

  Value getDOMCounter(DOM::Counter c);

}; // namespace

#endif
