// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _KJS_CSS_H_
#define _KJS_CSS_H_

#include <dom/dom_node.h>
#include <dom/dom_doc.h>
#include <kjs/object.h>
#include <dom/css_value.h>
#include <dom/css_stylesheet.h>
#include <dom/css_rule.h>
#include "kjs_binding.h"

namespace KJS {

  class DOMCSSStyleDeclaration : public DOMObject {
  public:
    DOMCSSStyleDeclaration(ExecState *exec, const DOM::CSSStyleDeclaration& s);
    virtual ~DOMCSSStyleDeclaration();
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    virtual bool hasProperty(ExecState *exec, const UString &propertyName) const;
    virtual const ClassInfo *classInfo() const { return &info; }
    static const ClassInfo info;
    enum { CssText, Length, ParentRule,
           GetPropertyValue, GetPropertyCSSValue, RemoveProperty, GetPropertyPriority,
           SetProperty, Item };
    DOM::CSSStyleDeclaration toStyleDecl() const { return styleDecl; }
  protected:
    DOM::CSSStyleDeclaration styleDecl;
  };

  Value getDOMCSSStyleDeclaration(ExecState *exec, const DOM::CSSStyleDeclaration& n);

  class DOMStyleSheet : public DOMObject {
  public:
    // Build a DOMStyleSheet
    DOMStyleSheet(ExecState *, const DOM::StyleSheet& ss);
    // Constructor for inherited classes
    DOMStyleSheet(const Object& proto, const DOM::StyleSheet& ss) : DOMObject(proto), styleSheet(ss) { }
    virtual ~DOMStyleSheet();
    virtual Value tryGet(ExecState *exec, const UString &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    virtual bool toBoolean(ExecState *) const { return true; }
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Type, Disabled, OwnerNode, ParentStyleSheet, Href, Title, Media };
  protected:
    DOM::StyleSheet styleSheet;
  };

  Value getDOMStyleSheet(ExecState *exec, const DOM::StyleSheet& ss);

  class DOMStyleSheetList : public DOMObject {
  public:
    DOMStyleSheetList(ExecState *, const DOM::StyleSheetList& ssl, const DOM::Document& doc);
    virtual ~DOMStyleSheetList();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    virtual Value call(ExecState *exec, Object &thisObj, const List &args);
    Value tryCall(ExecState *exec, Object &thisObj, const List &args);
    virtual bool implementsCall() const { return true; }
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    virtual bool toBoolean(ExecState* ) const { return true; }
    static const ClassInfo info;
    DOM::StyleSheetList toStyleSheetList() const { return styleSheetList; }
    enum { Item, Length };
  private:
    DOM::StyleSheetList styleSheetList;
    DOM::Document m_doc;
  };

  // The document is only used for get-stylesheet-by-name (make optional if necessary)
  Value getDOMStyleSheetList(ExecState *exec, const DOM::StyleSheetList& ss, const DOM::Document& doc);

  class DOMMediaList : public DOMObject {
  public:
    DOMMediaList(ExecState *, const DOM::MediaList& ml);
    virtual ~DOMMediaList();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    virtual const ClassInfo* classInfo() const { return &info; }
    virtual bool toBoolean(ExecState* ) const { return true; }
    static const ClassInfo info;
    enum { MediaText, Length,
           Item, DeleteMedium, AppendMedium };
    DOM::MediaList toMediaList() const { return mediaList; }
  private:
    DOM::MediaList mediaList;
  };

  Value getDOMMediaList(ExecState *exec, const DOM::MediaList& ss);

  class DOMCSSStyleSheet : public DOMStyleSheet {
  public:
    DOMCSSStyleSheet(ExecState *exec, const DOM::CSSStyleSheet& ss);
    virtual ~DOMCSSStyleSheet();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { OwnerRule, CssRules, Rules,
           InsertRule, DeleteRule, AddRule, RemoveRule };
    DOM::CSSStyleSheet toCSSStyleSheet() const { return static_cast<DOM::CSSStyleSheet>(styleSheet); }
  };

  class DOMCSSRuleList : public DOMObject {
  public:
    DOMCSSRuleList(ExecState *, const DOM::CSSRuleList& rl);
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

  Value getDOMCSSRuleList(ExecState *exec, const DOM::CSSRuleList& rl);

  class DOMCSSRule : public DOMObject {
  public:
    DOMCSSRule(ExecState *, const DOM::CSSRule& r);
    virtual ~DOMCSSRule();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    void putValueProperty(ExecState *exec, int token, const Value& value, int attr);
    virtual const ClassInfo* classInfo() const;
    static const ClassInfo info;
    static const ClassInfo style_info, media_info, fontface_info, page_info, import_info, charset_info;
    enum { ParentStyleSheet, Type, CssText, ParentRule,
           Style_SelectorText, Style_Style,
           Media_Media, Media_InsertRule, Media_DeleteRule, Media_CssRules,
           FontFace_Style, Page_SelectorText, Page_Style,
           Import_Href, Import_Media, Import_StyleSheet, Charset_Encoding };
    DOM::CSSRule toCSSRule() const { return cssRule; }
  protected:
    DOM::CSSRule cssRule;
  };

  Value getDOMCSSRule(ExecState *exec, const DOM::CSSRule& r);

  /**
   * Convert an object to a CSSRule. Returns a null CSSRule if not possible.
   */
  DOM::CSSRule toCSSRule(const Value&);

  // Constructor for CSSRule - currently only used for some global values
  class CSSRuleConstructor : public DOMObject {
  public:
    CSSRuleConstructor(ExecState *);
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { UNKNOWN_RULE, STYLE_RULE, CHARSET_RULE, IMPORT_RULE, MEDIA_RULE, FONT_FACE_RULE, PAGE_RULE };
  };

  Value getCSSRuleConstructor(ExecState *exec);

  class DOMCSSValue : public DOMObject {
  public:
    DOMCSSValue(ExecState *, const DOM::CSSValue& v);
    DOMCSSValue(const Object& proto, const DOM::CSSValue& v) : DOMObject(proto), cssValue(v) { }
    virtual ~DOMCSSValue();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    virtual void tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr = None);
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { CssText, CssValueType };
  protected:
    DOM::CSSValue cssValue;
  };

  Value getDOMCSSValue(ExecState *exec, const DOM::CSSValue& v);

  // Constructor for CSSValue - currently only used for some global values
  class CSSValueConstructor : public DOMObject {
  public:
    CSSValueConstructor(ExecState *exec);
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { CSS_VALUE_LIST, CSS_PRIMITIVE_VALUE, CSS_CUSTOM, CSS_INHERIT };
  };

  Value getCSSValueConstructor(ExecState *exec);

  class DOMCSSPrimitiveValue : public DOMCSSValue {
  public:
    DOMCSSPrimitiveValue(ExecState *exec, const DOM::CSSPrimitiveValue& v);
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    DOM::CSSPrimitiveValue toCSSPrimitiveValue() const { return static_cast<DOM::CSSPrimitiveValue>(cssValue); }
    enum { PrimitiveType, SetFloatValue, GetFloatValue, SetStringValue, GetStringValue,
           GetCounterValue, GetRectValue, GetRGBColorValue };
  };

  // Constructor for CSSPrimitiveValue - currently only used for some global values
  class CSSPrimitiveValueConstructor : public CSSValueConstructor {
  public:
    CSSPrimitiveValueConstructor(ExecState *exec) : CSSValueConstructor(exec) { }
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  Value getCSSPrimitiveValueConstructor(ExecState *exec);

  class DOMCSSValueList : public DOMCSSValue {
  public:
    DOMCSSValueList(ExecState *exec, const DOM::CSSValueList& v);
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Item, Length };
    DOM::CSSValueList toValueList() const { return static_cast<DOM::CSSValueList>(cssValue); }
  };

  class DOMRGBColor : public DOMObject {
  public:
    DOMRGBColor(ExecState*, const DOM::RGBColor& c);
    ~DOMRGBColor();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Red, Green, Blue };
  protected:
    DOM::RGBColor rgbColor;
  };

  Value getDOMRGBColor(ExecState *exec, const DOM::RGBColor& c);

  class DOMRect : public DOMObject {
  public:
    DOMRect(ExecState *, const DOM::Rect& r);
    ~DOMRect();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Top, Right, Bottom, Left };
  protected:
    DOM::Rect rect;
  };

  Value getDOMRect(ExecState *exec, const DOM::Rect& r);

  class DOMCounter : public DOMObject {
  public:
    DOMCounter(ExecState *, const DOM::Counter& c);
    ~DOMCounter();
    virtual Value tryGet(ExecState *exec,const UString &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;
    // no put - all read-only
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { _Identifier, ListStyle, Separator };
  protected:
    DOM::Counter counter;
  };

  Value getDOMCounter(ExecState *exec, const DOM::Counter& c);

}; // namespace

#endif
