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

#include "kjs_css.h"

#include <qptrdict.h>

#include <css/cssparser.h>
#include "kjs_dom.h"

using namespace KJS;
#include <kdebug.h>

QPtrDict<DOMCSSStyleDeclaration> domCSSStyleDeclarations;
QPtrDict<DOMStyleSheet> styleSheets;
QPtrDict<DOMStyleSheetList> styleSheetLists;
QPtrDict<DOMMediaList> mediaLists;
QPtrDict<DOMCSSRuleList> cssRuleLists;
QPtrDict<DOMCSSRule> cssRules;
QPtrDict<DOMCSSValue> cssValues;
QPtrDict<DOMRect> rects;
QPtrDict<DOMCounter> counters;

static QString jsNameToProp( const UString &p )
{
    QString prop = p.qstring();
    int i = prop.length();
    while( --i ) {
	char c = prop[i].latin1();
	if ( c < 'A' || c > 'Z' )
	    continue;
	prop.insert( i, '-' );
    }

    return prop.lower();
}

DOMCSSStyleDeclaration::~DOMCSSStyleDeclaration()
{
  domCSSStyleDeclarations.remove(styleDecl.handle());
}

const ClassInfo DOMCSSStyleDeclaration::info = { "CSSStyleDeclaration", 0, 0, 0 };


bool DOMCSSStyleDeclaration::hasProperty(ExecState *exec, const UString &p,
					 bool recursive) const
{
  if (p == "cssText" ||
      p == "getPropertyValue" ||
      p == "getPropertyCSSValue" ||
      p == "removeProperty" ||
      p == "getPropertyPriority" ||
      p == "setProperty" ||
      p == "length" ||
      p == "item")
      return true;

  DOM::DOMString cssprop = jsNameToProp(p);
  if (DOM::getPropertyID(cssprop.string().ascii(), cssprop.length()))
      return true;

  return (recursive && ObjectImp::hasProperty(exec, p, true));
}

Value DOMCSSStyleDeclaration::tryGet(ExecState *exec, const UString &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMCSSStyleDeclaration::tryGet " << p.qstring() << endl;
#endif
  if (p == "cssText" )
    return getString(styleDecl.cssText());
  else if (p == "getPropertyValue")
    return new DOMCSSStyleDeclarationFunc(styleDecl,DOMCSSStyleDeclarationFunc::GetPropertyValue);
  else if (p == "getPropertyCSSValue")
    return new DOMCSSStyleDeclarationFunc(styleDecl,DOMCSSStyleDeclarationFunc::GetPropertyCSSValue);
  else if (p == "removeProperty")
    return new DOMCSSStyleDeclarationFunc(styleDecl,DOMCSSStyleDeclarationFunc::RemoveProperty);
  else if (p == "getPropertyPriority")
    return new DOMCSSStyleDeclarationFunc(styleDecl,DOMCSSStyleDeclarationFunc::GetPropertyPriority);
  else if (p == "setProperty")
    return new DOMCSSStyleDeclarationFunc(styleDecl,DOMCSSStyleDeclarationFunc::SetProperty);
  else if (p == "length" )
    return Number(styleDecl.length());
  else if (p == "item")
    return new DOMCSSStyleDeclarationFunc(styleDecl,DOMCSSStyleDeclarationFunc::Item);
  else if (p == "parentRule" )
    return Undefined(); // ###
  else {
    bool ok;
    long unsigned int u = p.toULong(&ok);
    if (ok)
      return getString(DOM::CSSStyleDeclaration(styleDecl).item(u));

#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMCSSStyleDeclaration: converting to css property name: " << jsNameToProp(p) << endl;
#endif
    DOM::CSSStyleDeclaration styleDecl2 = styleDecl;
    DOM::DOMString v = styleDecl2.getPropertyValue(DOM::DOMString(jsNameToProp(p)));
    if (!v.isNull())
	return getString(v);
  }
  return DOMObject::tryGet(exec, p);
}


void DOMCSSStyleDeclaration::tryPut(ExecState *exec, const UString &propertyName, const Value& value, int )
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMCSSStyleDeclaration::tryPut " << propertyName.qstring() << endl;
#endif
  if (propertyName == "cssText") {
    styleDecl.setCssText(value.toString(exec).value().string());
  }
  else {
    QString prop = jsNameToProp(propertyName);
    QString propvalue = value.toString(exec).value().qstring();

    if(prop.left(4) == "css-")
      prop = prop.mid(4);

    if(prop.startsWith( "pixel-") || prop.startsWith( "pos-" ) ) {
      prop = prop.mid(prop.find( '-' )+1);
      propvalue += "px";
    }
#ifdef KJS_VERBOSE
    kdDebug(6070) << "DOMCSSStyleDeclaration: prop=" << prop << " propvalue=" << propvalue << endl;
#endif
    styleDecl.removeProperty(prop);
    if(!propvalue.isEmpty())
      styleDecl.setProperty(prop,DOM::DOMString(propvalue),""); // ### is "" ok for priority?
  }
}

Value DOMCSSStyleDeclarationFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  Value result;
  String str = args[0].toString(exec);
  DOM::DOMString s = str.value().string();

  switch (id) {
    case GetPropertyValue:
      result = getString(styleDecl.getPropertyValue(s));
      break;
    case GetPropertyCSSValue:
      result = Undefined(); // ###
      break;
    case RemoveProperty:
      result = getString(styleDecl.removeProperty(s));
      break;
    case GetPropertyPriority:
      result = getString(styleDecl.getPropertyPriority(s));
      break;
    case SetProperty:
      styleDecl.setProperty(args[0].toString(exec).value().string(),
                            args[1].toString(exec).value().string(),
                            args[2].toString(exec).value().string());
      result = Undefined();
      break;
    case Item:
      result = getString(styleDecl.item(args[0].toNumber(exec).intValue()));
      break;
    default:
      result = Undefined();
  }

  return result;
}

Value KJS::getDOMCSSStyleDeclaration(DOM::CSSStyleDeclaration s)
{
  DOMCSSStyleDeclaration *ret;
  if (s.isNull())
    return Null();
  else if ((ret = domCSSStyleDeclarations[s.handle()]))
    return ret;
  else {
    ret = new DOMCSSStyleDeclaration(s);
    domCSSStyleDeclarations.insert(s.handle(),ret);
    return ret;
  }
}

// -------------------------------------------------------------------------

const ClassInfo DOMStyleSheet::info = { "StyleSheet", 0, 0, 0 };

DOMStyleSheet::~DOMStyleSheet()
{
  styleSheets.remove(styleSheet.handle());
}

Value DOMStyleSheet::tryGet(ExecState *exec, const UString &p) const
{
  Value result;

  if (p == "type")
    return getString(styleSheet.type());
  else if (p == "disabled")
    return Boolean(styleSheet.disabled());
  else if (p == "ownerNode")
    return getDOMNode(exec,styleSheet.ownerNode());
  else if (p == "parentStyleSheet")
    return getDOMStyleSheet(styleSheet.parentStyleSheet());
  else if (p == "href")
    return getString(styleSheet.href());
  else if (p == "title")
    return getString(styleSheet.title());
//  else if ( p == "media") ###
//    return getDOMMediaList(styleSheet.media());
  return DOMObject::tryGet(exec, p);
}

void DOMStyleSheet::tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr)
{
  if (propertyName == "disabled") {
    styleSheet.setDisabled(value.toBoolean(exec).value());
  }
  else
    DOMObject::tryPut(exec, propertyName, value, attr);
}

Value KJS::getDOMStyleSheet(DOM::StyleSheet ss)
{
  DOMStyleSheet *ret;
  if (ss.isNull())
    return Null();
  else if ((ret = styleSheets[ss.handle()]))
    return ret;
  else {
    if (ss.isCSSStyleSheet()) {
      DOM::CSSStyleSheet cs;
      cs = ss;
      ret = new DOMCSSStyleSheet(cs);
    }
    else
      ret = new DOMStyleSheet(ss);
    styleSheets.insert(ss.handle(),ret);
    return ret;
  }
}

// -------------------------------------------------------------------------

const ClassInfo DOMStyleSheetList::info = { "StyleSheetList", 0, 0, 0 };

DOMStyleSheetList::~DOMStyleSheetList()
{
  styleSheetLists.remove(styleSheetList.handle());
}

Value DOMStyleSheetList::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "length")
    return Number(styleSheetList.length());
  else if (p == "item")
    return new DOMStyleSheetListFunc(styleSheetList,DOMStyleSheetListFunc::Item);

  bool ok;
  long unsigned int u = p.toULong(&ok);
  if (ok)
    return getDOMStyleSheet(DOM::StyleSheetList(styleSheetList).item(u));

  return DOMObject::tryGet(exec, p);
}

Value KJS::getDOMStyleSheetList(DOM::StyleSheetList ssl)
{
  DOMStyleSheetList *ret;
  if (ssl.isNull())
    return Null();
  else if ((ret = styleSheetLists[ssl.handle()]))
    return ret;
  else {
    ret = new DOMStyleSheetList(ssl);
    styleSheetLists.insert(ssl.handle(),ret);
    return ret;
  }
}

Value DOMStyleSheetListFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  Value result;

  if (id == Item)
    result = getDOMStyleSheet(styleSheetList.item(args[0].toNumber(exec).intValue()));
  return result;
}

// -------------------------------------------------------------------------

const ClassInfo DOMMediaList::info = { "MediaList", 0, 0, 0 };

DOMMediaList::~DOMMediaList()
{
  mediaLists.remove(mediaList.handle());
}

Value DOMMediaList::tryGet(ExecState *exec, const UString &p) const
{
  DOM::MediaList list = DOM::MediaList(mediaList.handle());
//  DOM::MediaListImpl *handle = mediaList.handle();
  if (p == "mediaText")
    return getString(list.mediaText());
  else if (p == "length")
    return Number(list.length());
  else if (p == "item")
    return new DOMMediaListFunc(list,DOMMediaListFunc::Item);
  else if (p == "deleteMedium")
    return new DOMMediaListFunc(list,DOMMediaListFunc::DeleteMedium);
  else if (p == "appendMedium")
    return new DOMMediaListFunc(list,DOMMediaListFunc::AppendMedium);

  bool ok;
  long unsigned int u = p.toULong(&ok);
  if (ok)
    return getString(list.item(u));

  return DOMObject::tryGet(exec, p);
}

void DOMMediaList::tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr)
{
  if (propertyName == "mediaText")
    mediaList.setMediaText(value.toString(exec).value().string());
  else
    DOMObject::tryPut(exec, propertyName, value, attr);
}

Value KJS::getDOMMediaList(DOM::MediaList ml)
{
  DOMMediaList *ret;
  if (ml.isNull())
    return Null();
  else if ((ret = mediaLists[ml.handle()]))
    return ret;
  else {
    ret = new DOMMediaList(ml);
    mediaLists.insert(ml.handle(),ret);
    return ret;
  }
}

Value DOMMediaListFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  Value result;

  switch (id) {
    case Item:
      result = getString(mediaList.item(args[0].toNumber(exec).intValue()));
      break;
    case DeleteMedium:
      mediaList.deleteMedium(args[0].toString(exec).value().string());
      result = Undefined();
      break;
    case AppendMedium:
      mediaList.appendMedium(args[0].toString(exec).value().string());
      result = Undefined();
      break;
    default:
      break;
  }

  return result;
}

// -------------------------------------------------------------------------

const ClassInfo DOMCSSStyleSheet::info = { "CSSStyleSheet", 0, 0, 0 };

DOMCSSStyleSheet::~DOMCSSStyleSheet()
{
}

Value DOMCSSStyleSheet::tryGet(ExecState *exec, const UString &p) const
{
  Value result;

  DOM::CSSStyleSheet cssStyleSheet = static_cast<DOM::CSSStyleSheet>(styleSheet);

  if (p == "ownerRule")
    return getDOMCSSRule(cssStyleSheet.ownerRule());
  else if (p == "cssRules")
    return getDOMCSSRuleList(cssStyleSheet.cssRules());
  else if (p == "insertRule")
    return new DOMCSSStyleSheetFunc(cssStyleSheet,DOMCSSStyleSheetFunc::InsertRule);
  else if (p == "deleteRule")
    return new DOMCSSStyleSheetFunc(cssStyleSheet,DOMCSSStyleSheetFunc::DeleteRule);

  return DOMStyleSheet::tryGet(exec,p);
}

Value DOMCSSStyleSheetFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  Value result;
  String str = args[0].toString(exec);
  DOM::DOMString s = str.value().string();

  switch (id) {
    case InsertRule:
      result = Number(styleSheet.insertRule(args[0].toString(exec).value().string(),(long unsigned int)args[1].toNumber(exec).intValue()));
      break;
    case DeleteRule:
      styleSheet.deleteRule(args[0].toNumber(exec).intValue());
      break;
    default:
      result = Undefined();
  }

  return result;
}

// -------------------------------------------------------------------------

const ClassInfo DOMCSSRuleList::info = { "CSSRuleList", 0, 0, 0 };

DOMCSSRuleList::~DOMCSSRuleList()
{
  cssRuleLists.remove(cssRuleList.handle());
}

Value DOMCSSRuleList::tryGet(ExecState *exec, const UString &p) const
{
  Value result;

  if (p == "length")
    return Number(cssRuleList.length());
  else if (p == "item")
    return new DOMCSSRuleListFunc(cssRuleList,DOMCSSRuleListFunc::Item);

  bool ok;
  long unsigned int u = p.toULong(&ok);
  if (ok)
    return getDOMCSSRule(DOM::CSSRuleList(cssRuleList).item(u));

  return DOMObject::tryGet(exec,p);
}

Value DOMCSSRuleListFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  Value result;

  switch (id) {
    case Item:
      result = getDOMCSSRule(cssRuleList.item(args[0].toNumber(exec).intValue()));
      break;
    default:
      result = Undefined();
  }

  return result;
}

Value KJS::getDOMCSSRuleList(DOM::CSSRuleList rl)
{
  DOMCSSRuleList *ret;
  if (rl.isNull())
    return Null();
  else if ((ret = cssRuleLists[rl.handle()]))
    return ret;
  else {
    ret = new DOMCSSRuleList(rl);
    cssRuleLists.insert(rl.handle(),ret);
    return ret;
  }
}

// -------------------------------------------------------------------------

const ClassInfo DOMCSSRule::info = { "CSSRule", 0, 0, 0 };

DOMCSSRule::~DOMCSSRule()
{
  cssRules.remove(cssRule.handle());
}

Value DOMCSSRule::tryGet(ExecState *exec, const UString &p) const
{
  Value result;

  switch (cssRule.type()) {
    case DOM::CSSRule::STYLE_RULE: {
        DOM::CSSStyleRule rule = static_cast<DOM::CSSStyleRule>(cssRule);
        if (p == "selectorText") return getString(rule.selectorText());
        if (p == "style") return getDOMCSSStyleDeclaration(rule.style());
        break;
      }
    case DOM::CSSRule::MEDIA_RULE: {
        DOM::CSSMediaRule rule = static_cast<DOM::CSSMediaRule>(cssRule);
        if (p == "media") return getDOMMediaList(rule.media());
        if (p == "cssRules") return getDOMCSSRuleList(rule.cssRules());
        if (p == "insertRule") return new DOMCSSRuleFunc(rule,DOMCSSRuleFunc::InsertRule);
        if (p == "deleteRule") return new DOMCSSRuleFunc(rule,DOMCSSRuleFunc::DeleteRule);
        break;
      }
    case DOM::CSSRule::FONT_FACE_RULE: {
        DOM::CSSFontFaceRule rule = static_cast<DOM::CSSFontFaceRule>(cssRule);
        if (p == "style") return getDOMCSSStyleDeclaration(rule.style());
        break;
      }
    case DOM::CSSRule::PAGE_RULE: {
        DOM::CSSPageRule rule = static_cast<DOM::CSSPageRule>(cssRule);
        if (p == "selectorText") return getString(rule.selectorText());
        if (p == "style") return getDOMCSSStyleDeclaration(rule.style());
        break;
      }
    case DOM::CSSRule::IMPORT_RULE: {
        DOM::CSSImportRule rule = static_cast<DOM::CSSImportRule>(cssRule);
        if (p == "href") return getString(rule.href());
        if (p == "media") return getDOMMediaList(rule.media());
        if (p == "styleSheet") return getDOMStyleSheet(rule.styleSheet());
        break;
      }
    case DOM::CSSRule::CHARSET_RULE: {
        DOM::CSSCharsetRule rule = static_cast<DOM::CSSCharsetRule>(cssRule);
        if (p == "encoding") return getString(rule.encoding());
        break;
      }
    case DOM::CSSRule::UNKNOWN_RULE:
      break;
  }

  if (p == "type")
    return Number(cssRule.type());
  else if (p == "cssText")
    return getString(cssRule.cssText());
  else if (p == "parentStyleSheet")
    return getDOMStyleSheet(cssRule.parentStyleSheet());
  else if (p == "parentRule")
    return getDOMCSSRule(cssRule.parentRule());

  return DOMObject::tryGet(exec,p);
};

void DOMCSSRule::tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr)
{

  switch (cssRule.type()) {
    case DOM::CSSRule::STYLE_RULE: {
        DOM::CSSStyleRule rule = static_cast<DOM::CSSStyleRule>(cssRule);
        if (propertyName == "selectorText") {
          rule.setSelectorText(value.toString(exec).value().string());
          return;
        }
        break;
      }
    case DOM::CSSRule::PAGE_RULE: {
        DOM::CSSPageRule rule = static_cast<DOM::CSSPageRule>(cssRule);
        if (propertyName == "selectorText") {
          rule.setSelectorText(value.toString(exec).value().string());
          return;
        }
        break;
      }
    case DOM::CSSRule::CHARSET_RULE: {
        DOM::CSSCharsetRule rule = static_cast<DOM::CSSCharsetRule>(cssRule);
        if (propertyName == "encoding") {
          rule.setEncoding(value.toString(exec).value().string());
          return;
        }
        break;
      }
    default:
      break;
  }

  DOMObject::tryPut(exec,propertyName,value,attr);

}

Value DOMCSSRuleFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  Value result = Undefined();

  if (cssRule.type() == DOM::CSSRule::MEDIA_RULE) {
    DOM::CSSMediaRule rule = static_cast<DOM::CSSMediaRule>(cssRule);
    if (id == InsertRule)
      result = Number(rule.insertRule(args[0].toString(exec).value().string(),args[1].toNumber(exec).intValue()));
    else if (id == DeleteRule)
      rule.deleteRule(args[0].toNumber(exec).intValue());
  }

  return result;
}

Value KJS::getDOMCSSRule(DOM::CSSRule r)
{
  DOMCSSRule *ret;
  if (r.isNull())
    return Null();
  else if ((ret = cssRules[r.handle()]))
    return ret;
  else {
    ret = new DOMCSSRule(r);
    cssRules.insert(r.handle(),ret);
    return ret;
  }
}

// -------------------------------------------------------------------------


DOM::CSSRule KJS::toCSSRule(const Value& val)
{
  Object obj = Object::dynamicCast(val);
  if (obj.isNull() || !obj.inherits(&DOMCSSRule::info))
    return DOM::CSSRule();

  const DOMCSSRule *dobj = static_cast<const DOMCSSRule*>(obj.imp());
  return dobj->toCSSRule();
}

// -------------------------------------------------------------------------

const ClassInfo CSSRulePrototype::info = { "CSSRulePrototype", 0, 0, 0 };

Value CSSRulePrototype::tryGet(ExecState *exec, const UString &p) const
{

// also prototype of CSSRule objects?
  if (p == "UNKNOWN_RULE")
    return Number(DOM::CSSRule::UNKNOWN_RULE);
  else if (p == "STYLE_RULE")
    return Number(DOM::CSSRule::STYLE_RULE);
  else if (p == "CHARSET_RULE")
    return Number(DOM::CSSRule::CHARSET_RULE);
  else if (p == "IMPORT_RULE")
    return Number(DOM::CSSRule::IMPORT_RULE);
  else if (p == "MEDIA_RULE")
    return Number(DOM::CSSRule::MEDIA_RULE);
  else if (p == "FONT_FACE_RULE")
    return Number(DOM::CSSRule::FONT_FACE_RULE);
  else if (p == "PAGE_RULE")
    return Number(DOM::CSSRule::PAGE_RULE);

  return DOMObject::tryGet(exec,p);
}

Value KJS::getCSSRulePrototype(ExecState *exec)
{
  Value proto = exec->interpreter()->globalObject().get(exec, "[[cssRule.prototype]]");
  if (!proto.isNull())
    return proto;
  else
  {
    Object cssRuleProto( new CSSRulePrototype );
    exec->interpreter()->globalObject().put(exec, "[[cssRule.prototype]]", cssRuleProto);
    return cssRuleProto;
  }
}

// -------------------------------------------------------------------------

const ClassInfo DOMCSSValue::info = { "CSSValue", 0, 0, 0 };

DOMCSSValue::~DOMCSSValue()
{
  cssValues.remove(cssValue.handle());
}

Value DOMCSSValue::tryGet(ExecState *exec, const UString &p) const
{
  Value result;

  if (p == "cssText")
    return getString(cssValue.cssText());
  else if (p == "cssValueType");
    return Number(cssValue.cssValueType());

  return DOMObject::tryGet(exec,p);
}

void DOMCSSValue::tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr)
{
  if (propertyName == "cssText")
    cssValue.setCssText(value.toString(exec).value().string());
  else
    DOMObject::tryPut(exec, propertyName, value, attr);
}

Value KJS::getDOMCSSValue(DOM::CSSValue v)
{
  DOMCSSValue *ret;
  if (v.isNull())
    return Null();
  else if ((ret = cssValues[v.handle()]))
    return ret;
  else {
    if (v.isCSSValueList())
      ret = new DOMCSSValueList(v);
    else if (v.isCSSPrimitiveValue())
      ret = new DOMCSSPrimitiveValue(v);
    else
      ret = new DOMCSSValue(v);
    cssValues.insert(v.handle(),ret);
    return ret;
  }
}

// -------------------------------------------------------------------------

const ClassInfo CSSValuePrototype::info = { "CSSValuePrototype", 0, 0, 0 };

Value CSSValuePrototype::tryGet(ExecState *exec, const UString &p) const
{
// also prototype of CSSValue objects?
  if (p == "CSS_INHERIT")
    return Number(DOM::CSSValue::CSS_INHERIT);
  else if (p == "CSS_PRIMITIVE_VALUE")
    return Number(DOM::CSSValue::CSS_PRIMITIVE_VALUE);
  else if (p == "CSS_VALUE_LIST")
    return Number(DOM::CSSValue::CSS_VALUE_LIST);
  else if (p == "CSS_CUSTOM")
    return Number(DOM::CSSValue::CSS_CUSTOM);

  return DOMObject::tryGet(exec,p);
}

Value KJS::getCSSValuePrototype(ExecState *exec)
{
  Value proto = exec->interpreter()->globalObject().get(exec, "[[cssValue.prototype]]");
  if (!proto.isNull())
    return proto;
  else
  {
    Object cssValueProto( new CSSRulePrototype );
    exec->interpreter()->globalObject().put(exec, "[[cssValue.prototype]]", cssValueProto);
    return cssValueProto;
  }
}

// -------------------------------------------------------------------------

const ClassInfo DOMCSSPrimitiveValue::info = { "CSSPrimitiveValue", 0, 0, 0 };

Value DOMCSSPrimitiveValue::tryGet(ExecState *exec, const UString &p) const
{
  Value result;
  DOM::CSSPrimitiveValue val = static_cast<DOM::CSSPrimitiveValue>(cssValue);

  if (p == "primitiveType")
    return Number(val.primitiveType());
  if (p == "setFloatValue")
    return new DOMCSSPrimitiveValueFunc(val,DOMCSSPrimitiveValueFunc::SetFloatValue);
  if (p == "getFloatValue")
    return new DOMCSSPrimitiveValueFunc(val,DOMCSSPrimitiveValueFunc::GetFloatValue);
  if (p == "setStringValue")
    return new DOMCSSPrimitiveValueFunc(val,DOMCSSPrimitiveValueFunc::SetStringValue);
  if (p == "getStringValue")
    return new DOMCSSPrimitiveValueFunc(val,DOMCSSPrimitiveValueFunc::GetStringValue);
  if (p == "getCounterValue")
    return new DOMCSSPrimitiveValueFunc(val,DOMCSSPrimitiveValueFunc::GetCounterValue);
  if (p == "getRectValue")
    return new DOMCSSPrimitiveValueFunc(val,DOMCSSPrimitiveValueFunc::GetRectValue);
  if (p == "getRGBColorValue")
    return new DOMCSSPrimitiveValueFunc(val,DOMCSSPrimitiveValueFunc::GetRGBColorValue);

  return DOMObject::tryGet(exec,p);
}

Value DOMCSSPrimitiveValueFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  Value result;

  switch (id) {
    case SetFloatValue:
      val.setFloatValue(args[0].toNumber(exec).intValue(),args[1].toNumber(exec).value());
      result = Undefined();
      break;
    case GetFloatValue:
      result = Number(val.getFloatValue(args[0].toNumber(exec).intValue()));
      break;
    case SetStringValue:
      val.setStringValue(args[0].toNumber(exec).intValue(),args[1].toString(exec).value().string());
      result = Undefined();
      break;
    case GetStringValue:
      result = getString(val.getStringValue());
      break;
    case GetCounterValue:
      result = getDOMCounter(val.getCounterValue());
      break;
    case GetRectValue:
      result = getDOMRect(val.getRectValue());
      break;
    case GetRGBColorValue:
      result = getDOMRGBColor(val.getRGBColorValue());
      break;
    default:
      result = Undefined();
  }

  return result;
}

// -------------------------------------------------------------------------

const ClassInfo CSSPrimitiveValuePrototype::info = { "CSSPrimitiveValuePrototype", 0, 0, 0 };

Value CSSPrimitiveValuePrototype::tryGet(ExecState *exec, const UString &p) const
{
// also prototype of CSSPrimitiveValue objects?
  if (p == "CSS_UNKNOWN")    return Number(DOM::CSSPrimitiveValue::CSS_UNKNOWN);
  if (p == "CSS_NUMBER")     return Number(DOM::CSSPrimitiveValue::CSS_NUMBER);
  if (p == "CSS_PERCENTAGE") return Number(DOM::CSSPrimitiveValue::CSS_PERCENTAGE);
  if (p == "CSS_EMS")        return Number(DOM::CSSPrimitiveValue::CSS_EMS);
  if (p == "CSS_EXS")        return Number(DOM::CSSPrimitiveValue::CSS_EXS);
  if (p == "CSS_PX")         return Number(DOM::CSSPrimitiveValue::CSS_PX);
  if (p == "CSS_CM")         return Number(DOM::CSSPrimitiveValue::CSS_CM);
  if (p == "CSS_MM")         return Number(DOM::CSSPrimitiveValue::CSS_MM);
  if (p == "CSS_IN")         return Number(DOM::CSSPrimitiveValue::CSS_IN);
  if (p == "CSS_PT")         return Number(DOM::CSSPrimitiveValue::CSS_PT);
  if (p == "CSS_PC")         return Number(DOM::CSSPrimitiveValue::CSS_PC);
  if (p == "CSS_DEG")        return Number(DOM::CSSPrimitiveValue::CSS_DEG);
  if (p == "CSS_RAD")        return Number(DOM::CSSPrimitiveValue::CSS_RAD);
  if (p == "CSS_GRAD")       return Number(DOM::CSSPrimitiveValue::CSS_GRAD);
  if (p == "CSS_MS")         return Number(DOM::CSSPrimitiveValue::CSS_MS);
  if (p == "CSS_S")          return Number(DOM::CSSPrimitiveValue::CSS_S);
  if (p == "CSS_HZ")         return Number(DOM::CSSPrimitiveValue::CSS_HZ);
  if (p == "CSS_KHZ")        return Number(DOM::CSSPrimitiveValue::CSS_KHZ);
  if (p == "CSS_DIMENSION")  return Number(DOM::CSSPrimitiveValue::CSS_DIMENSION);
  if (p == "CSS_STRING")     return Number(DOM::CSSPrimitiveValue::CSS_STRING);
  if (p == "CSS_URI")        return Number(DOM::CSSPrimitiveValue::CSS_URI);
  if (p == "CSS_IDENT")      return Number(DOM::CSSPrimitiveValue::CSS_IDENT);
  if (p == "CSS_ATTR")       return Number(DOM::CSSPrimitiveValue::CSS_ATTR);
  if (p == "CSS_COUNTER")    return Number(DOM::CSSPrimitiveValue::CSS_COUNTER);
  if (p == "CSS_RECT")       return Number(DOM::CSSPrimitiveValue::CSS_RECT);
  if (p == "CSS_RGBCOLOR")   return Number(DOM::CSSPrimitiveValue::CSS_RGBCOLOR);
  return CSSValuePrototype::tryGet(exec,p);
}

Value KJS::getCSSPrimitiveValuePrototype(ExecState *exec)
{
    Value proto = exec->interpreter()->globalObject().get(exec, "[[cssPrimitiveValue.prototype]]");
    if (!proto.isNull())
        return proto;
    else
    {
        Object cssPrimitiveValueProto( new CSSRulePrototype );
        exec->interpreter()->globalObject().put(exec, "[[cssPrimitiveValue.prototype]]", cssPrimitiveValueProto);
        return cssPrimitiveValueProto;
    }
}

// -------------------------------------------------------------------------

const ClassInfo DOMCSSValueList::info = { "CSSValueList", 0, 0, 0 };

Value DOMCSSValueList::tryGet(ExecState *exec, const UString &p) const
{
  Value result;
  DOM::CSSValueList valueList = static_cast<DOM::CSSValueList>(cssValue);

  if (p == "length")
    return Number(valueList.length());
  else if (p == "item")
    return new DOMCSSValueListFunc(valueList,DOMCSSValueListFunc::Item);

  bool ok;
  long unsigned int u = p.toULong(&ok);
  if (ok)
    return getDOMCSSValue(valueList.item(u));

  return DOMCSSValue::tryGet(exec,p);
}

Value DOMCSSValueListFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  Value result;

  switch (id) {
    case Item:
      result = getDOMCSSValue(valueList.item(args[0].toNumber(exec).intValue()));
      break;
    default:
      result = Undefined();
      break;
  }

  return result;
}

// -------------------------------------------------------------------------

const ClassInfo DOMRGBColor::info = { "RGBColor", 0, 0, 0 };

DOMRGBColor::~DOMRGBColor()
{
  //rgbColors.remove(rgbColor.handle());
}

Value DOMRGBColor::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "red")
    return getDOMCSSValue(rgbColor.red());
  if (p == "green")
    return getDOMCSSValue(rgbColor.green());
  if (p == "blue")
    return getDOMCSSValue(rgbColor.blue());

  return DOMObject::tryGet(exec,p);
}

Value KJS::getDOMRGBColor(DOM::RGBColor c)
{
  // ### implement equals for RGBColor since they're not refcounted objects
  return new DOMRGBColor(c);
}

// -------------------------------------------------------------------------

const ClassInfo DOMRect::info = { "Rect", 0, 0, 0 };

DOMRect::~DOMRect()
{
  rects.remove(rect.handle());
}

Value DOMRect::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "top")
    return getDOMCSSValue(rect.top());
  if (p == "right")
    return getDOMCSSValue(rect.right());
  if (p == "bottom")
    return getDOMCSSValue(rect.bottom());
  if (p == "left")
    return getDOMCSSValue(rect.left());

  return DOMObject::tryGet(exec,p);
}

Value KJS::getDOMRect(DOM::Rect r)
{
  DOMRect *ret;
  if (r.isNull())
    return Null();
  else if ((ret = rects[r.handle()]))
    return ret;
  else {
    ret = new DOMRect(r);
    rects.insert(r.handle(),ret);
    return ret;
  }
}

// -------------------------------------------------------------------------

const ClassInfo DOMCounter::info = { "Counter", 0, 0, 0 };

DOMCounter::~DOMCounter()
{
  counters.remove(counter.handle());
}

Value DOMCounter::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "identifier")
    return getString(counter.identifier());
  if (p == "listStyle")
    return getString(counter.listStyle());
  if (p == "separator")
    return getString(counter.separator());

  return DOMObject::tryGet(exec,p);
}

Value KJS::getDOMCounter(DOM::Counter c)
{
  DOMCounter *ret;
  if (c.isNull())
    return Null();
  else if ((ret = counters[c.handle()]))
    return ret;
  else {
    ret = new DOMCounter(c);
    counters.insert(c.handle(),ret);
    return ret;
  }
}




