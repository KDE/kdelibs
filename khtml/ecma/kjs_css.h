/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000 Harri Porten (porten@kde.org)
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
#include "kjs_binding.h"

namespace KJS {

  class DOMCSSStyleDeclaration : public DOMObject {
  public:
    DOMCSSStyleDeclaration(DOM::CSSStyleDeclaration s) : styleDecl(s) { }
    ~DOMCSSStyleDeclaration();
    virtual KJSO tryGet(const UString &p) const;
    virtual void tryPut(const UString &p, const KJSO& v);
    virtual bool hasProperty(const UString &p, bool recursive = true) const;
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  protected:
    DOM::CSSStyleDeclaration styleDecl;
  };

  class DOMCSSStyleDeclarationFunc : public DOMFunction {
    friend class DOMNode;
  public:
    DOMCSSStyleDeclarationFunc(DOM::CSSStyleDeclaration s, int i) : styleDecl(s), id(i) { }
    Completion tryExecute(const List &);
    enum { GetPropertyValue, GetPropertyCSSValue, RemoveProperty, GetPropertyPriority,
           SetProperty, Item };
  private:
    DOM::CSSStyleDeclaration styleDecl;
    int id;
  };

  KJSO getDOMCSSStyleDeclaration(DOM::CSSStyleDeclaration n);

  class DOMStyleSheet : public DOMObject {
  public:
    DOMStyleSheet(DOM::StyleSheet ss) : styleSheet(ss) { }
    ~DOMStyleSheet();
    virtual KJSO tryGet(const UString &p) const;
    virtual void tryPut(const UString &p, const KJSO& v);
    virtual const TypeInfo* typeInfo() const { return &info; }
    virtual Boolean toBoolean() const { return Boolean(true); }
    static const TypeInfo info;
  private:
    DOM::StyleSheet styleSheet;
  };

  KJSO getDOMStyleSheet(DOM::StyleSheet ss);

  class DOMStyleSheetList : public DOMObject {
  public:
    DOMStyleSheetList(DOM::StyleSheetList ssl) : styleSheetList(ssl) { }
    ~DOMStyleSheetList();
    virtual KJSO tryGet(const UString &p) const;
    // no put - all read-only
    virtual const TypeInfo* typeInfo() const { return &info; }
    virtual Boolean toBoolean() const { return Boolean(true); }
    static const TypeInfo info;
  private:
    DOM::StyleSheetList styleSheetList;
  };

  KJSO getDOMStyleSheetList(DOM::StyleSheetList ss);

  class DOMStyleSheetListFunc : public DOMFunction {
    friend class DOMStyleSheetList;
  public:
    DOMStyleSheetListFunc(DOM::StyleSheetList ssl, int i) : styleSheetList(ssl), id(i) { }
    Completion tryExecute(const List &);
    enum { Item };
  private:
    DOM::StyleSheetList styleSheetList;
    int id;
  };

  class DOMMediaList : public DOMObject {
  public:
    DOMMediaList(DOM::MediaList ml) : mediaList(ml) { }
    ~DOMMediaList();
    virtual KJSO tryGet(const UString &p) const;
    virtual void tryPut(const UString &p, const KJSO& v);
    virtual const TypeInfo* typeInfo() const { return &info; }
    virtual Boolean toBoolean() const { return Boolean(true); }
    static const TypeInfo info;
  private:
    DOM::MediaList mediaList;
  };

  KJSO getDOMMediaList(DOM::MediaList ss);

  class DOMMediaListFunc : public DOMFunction {
    friend class DOMMediaList;
  public:
    DOMMediaListFunc(DOM::MediaList ml, int i) : mediaList(ml), id(i) { }
    Completion tryExecute(const List &);
    enum { Item, DeleteMedium, AppendMedium };
  private:
    DOM::MediaList mediaList;
    int id;
  };










}; // namespace

#endif
