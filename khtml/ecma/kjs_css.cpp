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
#include "kjs_css.lut.h"

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

/*
@begin DOMCSSStyleDeclarationProtoTable 7
  getPropertyValue	DOMCSSStyleDeclaration::GetPropertyValue	DontDelete|Function 1
  getPropertyCSSValue	DOMCSSStyleDeclaration::GetPropertyCSSValue	DontDelete|Function 1
  removeProperty	DOMCSSStyleDeclaration::RemoveProperty		DontDelete|Function 1
  getPropertyPriority	DOMCSSStyleDeclaration::GetPropertyPriority	DontDelete|Function 1
  setProperty		DOMCSSStyleDeclaration::SetProperty		DontDelete|Function 3
  item			DOMCSSStyleDeclaration::Item			DontDelete|Function 1
@end
@begin DOMCSSStyleDeclarationTable 3
  cssText		DOMCSSStyleDeclaration::CssText		DontDelete
  length		DOMCSSStyleDeclaration::Length		DontDelete|ReadOnly
  parentRule		DOMCSSStyleDeclaration::ParentRule	DontDelete|ReadOnly
@end
*/
DEFINE_PROTOTYPE("DOMCSSStyleDeclaration", DOMCSSStyleDeclarationProto)
IMPLEMENT_PROTOFUNC(DOMCSSStyleDeclarationProtoFunc)
IMPLEMENT_PROTOTYPE(DOMCSSStyleDeclarationProto, DOMCSSStyleDeclarationProtoFunc)

const ClassInfo DOMCSSStyleDeclaration::info = { "CSSStyleDeclaration", 0, &DOMCSSStyleDeclarationTable, 0 };

DOMCSSStyleDeclaration::DOMCSSStyleDeclaration(ExecState *exec, DOM::CSSStyleDeclaration s)
  : DOMObject(DOMCSSStyleDeclarationProto::self(exec)), styleDecl(s)
{ }

DOMCSSStyleDeclaration::~DOMCSSStyleDeclaration()
{
  domCSSStyleDeclarations.remove(styleDecl.handle());
}

bool DOMCSSStyleDeclaration::hasProperty(ExecState *exec, const UString &p,
					 bool recursive) const
{
  DOM::DOMString cssprop = jsNameToProp(p);
  if (DOM::getPropertyID(cssprop.string().ascii(), cssprop.length()))
      return true;

  return ObjectImp::hasProperty(exec, p, recursive);
}

Value DOMCSSStyleDeclaration::tryGet(ExecState *exec, const UString &propertyName) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMCSSStyleDeclaration::tryGet " << propertyName.qstring() << endl;
#endif
  const HashEntry* entry = Lookup::findEntry(&DOMCSSStyleDeclarationTable, propertyName);
  if (entry)
    switch (entry->value) {
    case CssText:
      return getString(styleDecl.cssText());
    case Length:
      return Number(styleDecl.length());
    case ParentRule:
      return Undefined(); // ###
    default:
      break;
    }

  // Look in the prototype (for functions) before assuming it's a name
  Object proto = Object::dynamicCast(prototype());
  if (!proto.isNull() && proto.hasProperty(exec,propertyName))
    return proto.get(exec,propertyName);

  bool ok;
  long unsigned int u = propertyName.toULong(&ok);
  if (ok)
    return getString(DOM::CSSStyleDeclaration(styleDecl).item(u));

#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMCSSStyleDeclaration: converting to css property name: " << jsNameToProp(propertyName) << endl;
#endif
  DOM::CSSStyleDeclaration styleDecl2 = styleDecl;
  DOM::DOMString p = jsNameToProp(propertyName);
  DOM::DOMString v = styleDecl2.getPropertyValue(p);
  if (!v.isNull())
    return getString(v);

  // see if we know this css property, return empty then
  QCString prop = p.string().latin1();
  if (DOM::getPropertyID(prop.data(), prop.length()))
      return getString(DOM::DOMString(""));

  return DOMObject::tryGet(exec, propertyName);
}


void DOMCSSStyleDeclaration::tryPut(ExecState *exec, const UString &propertyName, const Value& value, int )
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMCSSStyleDeclaration::tryPut " << propertyName.qstring() << endl;
#endif
  if (propertyName == "cssText") {
    styleDecl.setCssText(value.toString(exec).string());
  }
  else {
    QString prop = jsNameToProp(propertyName);
    QString propvalue = value.toString(exec).qstring();

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

Value DOMCSSStyleDeclarationProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  DOM::CSSStyleDeclaration styleDecl = static_cast<DOMCSSStyleDeclaration *>(thisObj.imp())->toStyleDecl();
  Value result;
  String str = args[0].toString(exec);
  DOM::DOMString s = str.value().string();

  switch (id) {
    case DOMCSSStyleDeclaration::GetPropertyValue:
      result = getString(styleDecl.getPropertyValue(s));
      break;
    case DOMCSSStyleDeclaration::GetPropertyCSSValue:
      result = Undefined(); // ###
      break;
    case DOMCSSStyleDeclaration::RemoveProperty:
      result = getString(styleDecl.removeProperty(s));
      break;
    case DOMCSSStyleDeclaration::GetPropertyPriority:
      result = getString(styleDecl.getPropertyPriority(s));
      break;
    case DOMCSSStyleDeclaration::SetProperty:
      styleDecl.setProperty(args[0].toString(exec).string(),
                            args[1].toString(exec).string(),
                            args[2].toString(exec).string());
      result = Undefined();
      break;
    case DOMCSSStyleDeclaration::Item:
      result = getString(styleDecl.item(args[0].toInteger(exec)));
      break;
    default:
      result = Undefined();
  }

  return result;
}

Value KJS::getDOMCSSStyleDeclaration(ExecState *exec, DOM::CSSStyleDeclaration s)
{
  DOMCSSStyleDeclaration *ret;
  if (s.isNull())
    return Null();
  else if ((ret = domCSSStyleDeclarations[s.handle()]))
    return ret;
  else {
    ret = new DOMCSSStyleDeclaration(exec, s);
    domCSSStyleDeclarations.insert(s.handle(),ret);
    return ret;
  }
}

// -------------------------------------------------------------------------

const ClassInfo DOMStyleSheet::info = { "StyleSheet", 0, &DOMStyleSheetTable, 0 };
/*
@begin DOMStyleSheetTable 7
  type		DOMStyleSheet::Type		DontDelete|ReadOnly
  disabled	DOMStyleSheet::Disabled		DontDelete
  ownerNode	DOMStyleSheet::OwnerNode	DontDelete|ReadOnly
  parentStyleSheet DOMStyleSheet::ParentStyleSheet	DontDelete|ReadOnly
  href		DOMStyleSheet::Href		DontDelete|ReadOnly
  title		DOMStyleSheet::Title		DontDelete|ReadOnly
  media		DOMStyleSheet::Media		DontDelete|ReadOnly
@end
*/

DOMStyleSheet::~DOMStyleSheet()
{
  styleSheets.remove(styleSheet.handle());
}

Value DOMStyleSheet::tryGet(ExecState *exec, const UString &propertyName) const
{
  return DOMObjectLookupGetValue<DOMStyleSheet,DOMObject>(exec,propertyName,&DOMStyleSheetTable,this);
}

Value DOMStyleSheet::getValue(ExecState *exec, int token) const
{
  switch (token) {
  case Type:
    return getString(styleSheet.type());
  case Disabled:
    return Boolean(styleSheet.disabled());
  case OwnerNode:
    return getDOMNode(exec,styleSheet.ownerNode());
  case ParentStyleSheet:
    return getDOMStyleSheet(exec,styleSheet.parentStyleSheet());
  case Href:
    return getString(styleSheet.href());
  case Title:
    return getString(styleSheet.title());
  case Media:
    return getDOMMediaList(exec, styleSheet.media());
  }
  return Value();
}

void DOMStyleSheet::tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr)
{
  if (propertyName == "disabled") {
    styleSheet.setDisabled(value.toBoolean(exec));
  }
  else
    DOMObject::tryPut(exec, propertyName, value, attr);
}

Value KJS::getDOMStyleSheet(ExecState *exec, DOM::StyleSheet ss)
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
      ret = new DOMCSSStyleSheet(exec,cs);
    }
    else
      ret = new DOMStyleSheet(exec,ss);
    styleSheets.insert(ss.handle(),ret);
    return ret;
  }
}

// -------------------------------------------------------------------------

const ClassInfo DOMStyleSheetList::info = { "StyleSheetList", 0, &DOMStyleSheetListTable, 0 };

/*
@begin DOMStyleSheetListTable 2
  length	DOMStyleSheetList::Length	DontDelete|ReadOnly
  item		DOMStyleSheetList::Item		DontDelete|Function 1
@end
*/
IMPLEMENT_PROTOFUNC(DOMStyleSheetListFunc) // not really a proto, but doesn't matter

DOMStyleSheetList::~DOMStyleSheetList()
{
  styleSheetLists.remove(styleSheetList.handle());
}

Value DOMStyleSheetList::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "length")
    return Number(styleSheetList.length());
  else if (p == "item")
    return lookupOrCreateFunction<DOMStyleSheetListFunc>(exec,p,this,DOMStyleSheetList::Item,1,DontDelete|Function);

  bool ok;
  long unsigned int u = p.toULong(&ok);
  if (ok)
    return getDOMStyleSheet(exec, DOM::StyleSheetList(styleSheetList).item(u));

  return DOMObject::tryGet(exec, p);
}

Value KJS::getDOMStyleSheetList(ExecState *exec, DOM::StyleSheetList ssl)
{
  DOMStyleSheetList *ret;
  if (ssl.isNull())
    return Null();
  else if ((ret = styleSheetLists[ssl.handle()]))
    return ret;
  else {
    ret = new DOMStyleSheetList(exec, ssl);
    styleSheetLists.insert(ssl.handle(),ret);
    return ret;
  }
}

Value DOMStyleSheetListFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  Value result;
  DOM::StyleSheetList styleSheetList = static_cast<DOMStyleSheetList *>(thisObj.imp())->toStyleSheetList();
  if (id == DOMStyleSheetList::Item)
    result = getDOMStyleSheet(exec, styleSheetList.item(args[0].toInteger(exec)));
  return result;
}

// -------------------------------------------------------------------------

const ClassInfo DOMMediaList::info = { "MediaList", 0, &DOMMediaListTable, 0 };

/*
@begin DOMMediaListTable 2
  mediaText	DOMMediaList::MediaText		DontDelete|ReadOnly
  length	DOMMediaList::Length		DontDelete|ReadOnly
@end
@begin DOMMediaListProtoTable 3
  item		DOMMediaList::Item		DontDelete|Function 1
  deleteMedium	DOMMediaList::DeleteMedium	DontDelete|Function 1
  appendMedium	DOMMediaList::AppendMedium	DontDelete|Function 1
@end
*/
DEFINE_PROTOTYPE("DOMMediaList", DOMMediaListProto)
IMPLEMENT_PROTOFUNC(DOMMediaListProtoFunc)
IMPLEMENT_PROTOTYPE(DOMMediaListProto, DOMMediaListProtoFunc)

DOMMediaList::DOMMediaList(ExecState *exec, DOM::MediaList ml)
  : DOMObject(DOMMediaListProto::self(exec)), mediaList(ml) { }

DOMMediaList::~DOMMediaList()
{
  mediaLists.remove(mediaList.handle());
}

Value DOMMediaList::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "mediaText")
    return getString(mediaList.mediaText());
  else if (p == "length")
    return Number(mediaList.length());

  bool ok;
  long unsigned int u = p.toULong(&ok);
  if (ok)
    return getString(mediaList.item(u));

  return DOMObject::tryGet(exec, p);
}

void DOMMediaList::tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr)
{
  if (propertyName == "mediaText")
    mediaList.setMediaText(value.toString(exec).string());
  else
    DOMObject::tryPut(exec, propertyName, value, attr);
}

Value KJS::getDOMMediaList(ExecState *exec, DOM::MediaList ml)
{
  DOMMediaList *ret;
  if (ml.isNull())
    return Null();
  else if ((ret = mediaLists[ml.handle()]))
    return ret;
  else {
    ret = new DOMMediaList(exec, ml);
    mediaLists.insert(ml.handle(),ret);
    return ret;
  }
}

Value KJS::DOMMediaListProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  DOM::MediaList mediaList = static_cast<DOMMediaList *>(thisObj.imp())->toMediaList();
  switch (id) {
    case DOMMediaList::Item:
      return getString(mediaList.item(args[0].toInteger(exec)));
    case DOMMediaList::DeleteMedium:
      mediaList.deleteMedium(args[0].toString(exec).string());
      return Undefined();
    case DOMMediaList::AppendMedium:
      mediaList.appendMedium(args[0].toString(exec).string());
      return Undefined();
    default:
      return Undefined();
  }
}

// -------------------------------------------------------------------------

const ClassInfo DOMCSSStyleSheet::info = { "CSSStyleSheet", 0, &DOMCSSStyleSheetTable, 0 };

/*
@begin DOMCSSStyleSheetTable 2
  ownerRule	DOMCSSStyleSheet::OwnerRule	DontDelete|ReadOnly
  cssRules	DOMCSSStyleSheet::CssRules	DontDelete|ReadOnly
@end
@begin DOMCSSStyleSheetProtoTable 2
  insertRule	DOMCSSStyleSheet::InsertRule	DontDelete|Function 2
  deleteRule	DOMCSSStyleSheet::DeleteRule	DontDelete|Function 1
@end
*/
DEFINE_PROTOTYPE("DOMCSSStyleSheet",DOMCSSStyleSheetProto)
IMPLEMENT_PROTOFUNC(DOMCSSStyleSheetProtoFunc)
IMPLEMENT_PROTOTYPE(DOMCSSStyleSheetProto,DOMCSSStyleSheetProtoFunc) // warning, use _WITH_PARENT if DOMStyleSheet gets a proto

DOMCSSStyleSheet::DOMCSSStyleSheet(ExecState *exec, DOM::CSSStyleSheet ss)
  : DOMStyleSheet(DOMCSSStyleSheetProto::self(exec),ss) { }

DOMCSSStyleSheet::~DOMCSSStyleSheet()
{
}

Value DOMCSSStyleSheet::tryGet(ExecState *exec, const UString &p) const
{
  DOM::CSSStyleSheet cssStyleSheet = static_cast<DOM::CSSStyleSheet>(styleSheet);
  if (p == "ownerRule")
    return getDOMCSSRule(exec,cssStyleSheet.ownerRule());
  else if (p == "cssRules")
    return getDOMCSSRuleList(exec,cssStyleSheet.cssRules());
  return DOMStyleSheet::tryGet(exec,p);
}

Value DOMCSSStyleSheetProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  DOM::CSSStyleSheet styleSheet = static_cast<DOMCSSStyleSheet *>(thisObj.imp())->toCSSStyleSheet();
  Value result;
  UString str = args[0].toString(exec);
  DOM::DOMString s = str.string();

  switch (id) {
    case DOMCSSStyleSheet::InsertRule:
      result = Number(styleSheet.insertRule(args[0].toString(exec).string(),(long unsigned int)args[1].toInteger(exec)));
      break;
    case DOMCSSStyleSheet::DeleteRule:
      styleSheet.deleteRule(args[0].toInteger(exec));
      break;
    default:
      result = Undefined();
  }
  return result;
}

// -------------------------------------------------------------------------

const ClassInfo DOMCSSRuleList::info = { "CSSRuleList", 0, &DOMCSSRuleListTable, 0 };
/*
@begin DOMCSSRuleListTable 3
  length		DOMCSSRuleList::Length		DontDelete|ReadOnly
  item			DOMCSSRuleList::Item		DontDelete|Function 1
@end
*/
IMPLEMENT_PROTOFUNC(DOMCSSRuleListFunc) // not really a proto, but doesn't matter

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
    return lookupOrCreateFunction<DOMCSSRuleListFunc>(exec,p,this,DOMCSSRuleList::Item,1,DontDelete|Function);

  bool ok;
  long unsigned int u = p.toULong(&ok);
  if (ok)
    return getDOMCSSRule(exec,DOM::CSSRuleList(cssRuleList).item(u));

  return DOMObject::tryGet(exec,p);
}

Value DOMCSSRuleListFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  DOM::CSSRuleList cssRuleList = static_cast<DOMCSSRuleList *>(thisObj.imp())->toCSSRuleList();
  switch (id) {
    case DOMCSSRuleList::Item:
      return getDOMCSSRule(exec,cssRuleList.item(args[0].toInteger(exec)));
    default:
      return Undefined();
  }
}

Value KJS::getDOMCSSRuleList(ExecState *exec, DOM::CSSRuleList rl)
{
  DOMCSSRuleList *ret;
  if (rl.isNull())
    return Null();
  else if ((ret = cssRuleLists[rl.handle()]))
    return ret;
  else {
    ret = new DOMCSSRuleList(exec, rl);
    cssRuleLists.insert(rl.handle(),ret);
    return ret;
  }
}

// -------------------------------------------------------------------------

const ClassInfo DOMCSSRule::info = { "CSSRule", 0, 0, 0 };

IMPLEMENT_PROTOFUNC(DOMCSSRuleFunc) // Not a proto, but doesn't matter

DOMCSSRule::~DOMCSSRule()
{
  cssRules.remove(cssRule.handle());
}

// ### like HTMLElement, this class needs to be split up if we want to use a static hashtable
Value DOMCSSRule::tryGet(ExecState *exec, const UString &p) const
{
  Value result;

  switch (cssRule.type()) {
    case DOM::CSSRule::STYLE_RULE: {
        DOM::CSSStyleRule rule = static_cast<DOM::CSSStyleRule>(cssRule);
        if (p == "selectorText") return getString(rule.selectorText());
        if (p == "style") return getDOMCSSStyleDeclaration(exec,rule.style());
        break;
      }
    case DOM::CSSRule::MEDIA_RULE: {
        DOM::CSSMediaRule rule = static_cast<DOM::CSSMediaRule>(cssRule);
        if (p == "media") return getDOMMediaList(exec,rule.media());
        if (p == "cssRules") return getDOMCSSRuleList(exec,rule.cssRules());
        if (p == "insertRule") return lookupOrCreateFunction<DOMCSSRuleFunc>(exec,p,this,DOMCSSRule::InsertRule,2,DontDelete|Function);
        if (p == "deleteRule") return lookupOrCreateFunction<DOMCSSRuleFunc>(exec,p,this,DOMCSSRule::DeleteRule,1,DontDelete|Function);
        break;
      }
    case DOM::CSSRule::FONT_FACE_RULE: {
        DOM::CSSFontFaceRule rule = static_cast<DOM::CSSFontFaceRule>(cssRule);
        if (p == "style") return getDOMCSSStyleDeclaration(exec,rule.style());
        break;
      }
    case DOM::CSSRule::PAGE_RULE: {
        DOM::CSSPageRule rule = static_cast<DOM::CSSPageRule>(cssRule);
        if (p == "selectorText") return getString(rule.selectorText());
        if (p == "style") return getDOMCSSStyleDeclaration(exec,rule.style());
        break;
      }
    case DOM::CSSRule::IMPORT_RULE: {
        DOM::CSSImportRule rule = static_cast<DOM::CSSImportRule>(cssRule);
        if (p == "href") return getString(rule.href());
        if (p == "media") return getDOMMediaList(exec,rule.media());
        if (p == "styleSheet") return getDOMStyleSheet(exec,rule.styleSheet());
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
    return getDOMStyleSheet(exec,cssRule.parentStyleSheet());
  else if (p == "parentRule")
    return getDOMCSSRule(exec,cssRule.parentRule());

  return DOMObject::tryGet(exec,p);
};

void DOMCSSRule::tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr)
{

  switch (cssRule.type()) {
    case DOM::CSSRule::STYLE_RULE: {
        DOM::CSSStyleRule rule = static_cast<DOM::CSSStyleRule>(cssRule);
        if (propertyName == "selectorText") {
          rule.setSelectorText(value.toString(exec).string());
          return;
        }
        break;
      }
    case DOM::CSSRule::PAGE_RULE: {
        DOM::CSSPageRule rule = static_cast<DOM::CSSPageRule>(cssRule);
        if (propertyName == "selectorText") {
          rule.setSelectorText(value.toString(exec).string());
          return;
        }
        break;
      }
    case DOM::CSSRule::CHARSET_RULE: {
        DOM::CSSCharsetRule rule = static_cast<DOM::CSSCharsetRule>(cssRule);
        if (propertyName == "encoding") {
          rule.setEncoding(value.toString(exec).string());
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
  DOM::CSSRule cssRule = static_cast<DOMCSSRule *>(thisObj.imp())->toCSSRule();
  Value result = Undefined();

  if (cssRule.type() == DOM::CSSRule::MEDIA_RULE) {
    DOM::CSSMediaRule rule = static_cast<DOM::CSSMediaRule>(cssRule);
    if (id == DOMCSSRule::InsertRule)
      result = Number(rule.insertRule(args[0].toString(exec).string(),args[1].toInteger(exec)));
    else if (id == DOMCSSRule::DeleteRule)
      rule.deleteRule(args[0].toInteger(exec));
  }

  return result;
}

Value KJS::getDOMCSSRule(ExecState *exec, DOM::CSSRule r)
{
  DOMCSSRule *ret;
  if (r.isNull())
    return Null();
  else if ((ret = cssRules[r.handle()]))
    return ret;
  else {
    ret = new DOMCSSRule(exec, r);
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

const ClassInfo CSSRuleConstructor::info = { "CSSRuleConstructor", 0, &CSSRuleConstructorTable, 0 };
/*
@begin CSSRuleConstructorTable 7
  UNKNOWN_RULE	CSSRuleConstructor::UNKNOWN_RULE	DontDelete|ReadOnly
  STYLE_RULE	CSSRuleConstructor::STYLE_RULE		DontDelete|ReadOnly
  CHARSET_RULE	CSSRuleConstructor::CHARSET_RULE	DontDelete|ReadOnly
  IMPORT_RULE	CSSRuleConstructor::IMPORT_RULE		DontDelete|ReadOnly
  MEDIA_RULE	CSSRuleConstructor::MEDIA_RULE		DontDelete|ReadOnly
  FONT_FACE_RULE CSSRuleConstructor::FONT_FACE_RULE	DontDelete|ReadOnly
  PAGE_RULE	CSSRuleConstructor::PAGE_RULE		DontDelete|ReadOnly
@end
*/

Value CSSRuleConstructor::tryGet(ExecState *exec, const UString &p) const
{
  return DOMObjectLookupGetValue<CSSRuleConstructor,DOMObject>(exec,p,&CSSRuleConstructorTable,this);
}

Value CSSRuleConstructor::getValue(ExecState *, int token) const
{
  switch (token) {
  case UNKNOWN_RULE:
    return Number(DOM::CSSRule::UNKNOWN_RULE);
  case STYLE_RULE:
    return Number(DOM::CSSRule::STYLE_RULE);
  case CHARSET_RULE:
    return Number(DOM::CSSRule::CHARSET_RULE);
  case IMPORT_RULE:
    return Number(DOM::CSSRule::IMPORT_RULE);
  case MEDIA_RULE:
    return Number(DOM::CSSRule::MEDIA_RULE);
  case FONT_FACE_RULE:
    return Number(DOM::CSSRule::FONT_FACE_RULE);
  case PAGE_RULE:
    return Number(DOM::CSSRule::PAGE_RULE);
  }
  return Value();
}

Value KJS::getCSSRuleConstructor(ExecState *exec)
{
  return cacheGlobalObject<CSSRuleConstructor>( exec, "[[cssRule.constructor]]" );
}

// -------------------------------------------------------------------------

const ClassInfo DOMCSSValue::info = { "CSSValue", 0, &DOMCSSValueTable, 0 };

/*
@begin DOMCSSValueTable 2
  cssText	DOMCSSValue::CssText		DontDelete|ReadOnly
  cssValueType	DOMCSSValue::CssValueType	DontDelete|ReadOnly
@end
*/
DOMCSSValue::~DOMCSSValue()
{
  cssValues.remove(cssValue.handle());
}

Value DOMCSSValue::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "cssText")
    return getString(cssValue.cssText());
  else if (p == "cssValueType");
    return Number(cssValue.cssValueType());
  return DOMObject::tryGet(exec,p);
}

void DOMCSSValue::tryPut(ExecState *exec, const UString &propertyName, const Value& value, int attr)
{
  if (propertyName == "cssText")
    cssValue.setCssText(value.toString(exec).string());
  else
    DOMObject::tryPut(exec, propertyName, value, attr);
}

Value KJS::getDOMCSSValue(ExecState *exec, DOM::CSSValue v)
{
  DOMCSSValue *ret;
  if (v.isNull())
    return Null();
  else if ((ret = cssValues[v.handle()]))
    return ret;
  else {
    if (v.isCSSValueList())
      ret = new DOMCSSValueList(exec,v);
    else if (v.isCSSPrimitiveValue())
      ret = new DOMCSSPrimitiveValue(exec,v);
    else
      ret = new DOMCSSValue(exec,v);
    cssValues.insert(v.handle(),ret);
    return ret;
  }
}

// -------------------------------------------------------------------------

const ClassInfo CSSValueConstructor::info = { "CSSValueConstructor", 0, &CSSValueConstructorTable, 0 };
/*
@begin CSSValueConstructorTable 5
  CSS_INHERIT		CSSValueConstructor::CSS_INHERIT		DontDelete|ReadOnly
  CSS_PRIMITIVE_VALUE	CSSValueConstructor::CSS_PRIMITIVE_VALUE	DontDelete|ReadOnly
  CSS_VALUE_LIST	CSSValueConstructor::CSS_VALUE_LIST		DontDelete|ReadOnly
  CSS_CUSTOM		CSSValueConstructor::CSS_CUSTOM			DontDelete|ReadOnly
@end
*/
Value CSSValueConstructor::tryGet(ExecState *exec, const UString &p) const
{
  return DOMObjectLookupGetValue<CSSValueConstructor,DOMObject>(exec,p,&CSSValueConstructorTable,this);
}

Value CSSValueConstructor::getValue(ExecState *, int token) const
{
  switch (token) {
  case CSS_INHERIT:
    return Number(DOM::CSSValue::CSS_INHERIT);
  case CSS_PRIMITIVE_VALUE:
    return Number(DOM::CSSValue::CSS_PRIMITIVE_VALUE);
  case CSS_VALUE_LIST:
    return Number(DOM::CSSValue::CSS_VALUE_LIST);
  case CSS_CUSTOM:
    return Number(DOM::CSSValue::CSS_CUSTOM);
  }
  return Value();
}

Value KJS::getCSSValueConstructor(ExecState *exec)
{
  return cacheGlobalObject<CSSValueConstructor>( exec, "[[cssValue.constructor]]" );
}

// -------------------------------------------------------------------------

const ClassInfo DOMCSSPrimitiveValue::info = { "CSSPrimitiveValue", 0, &DOMCSSPrimitiveValueTable, 0 };
/*
@begin DOMCSSPrimitiveValueTable 1
  primitiveType		DOMCSSPrimitiveValue::PrimitiveType	DontDelete|ReadOnly
@end
@begin DOMCSSPrimitiveValueProtoTable 3
  setFloatValue		DOMCSSPrimitiveValue::SetFloatValue	DontDelete|Function 2
  getFloatValue		DOMCSSPrimitiveValue::GetFloatValue	DontDelete|Function 1
  setStringValue	DOMCSSPrimitiveValue::SetStringValue	DontDelete|Function 2
  getStringValue	DOMCSSPrimitiveValue::GetStringValue	DontDelete|Function 0
  getCounterValue	DOMCSSPrimitiveValue::GetCounterValue	DontDelete|Function 0
  getRectValue		DOMCSSPrimitiveValue::GetRectValue	DontDelete|Function 0
  getRGBColorValue	DOMCSSPrimitiveValue::GetRGBColorValue	DontDelete|Function 0
@end
*/
DEFINE_PROTOTYPE("DOMCSSPrimitiveValue",DOMCSSPrimitiveValueProto)
IMPLEMENT_PROTOFUNC(DOMCSSPrimitiveValueProtoFunc)
IMPLEMENT_PROTOTYPE(DOMCSSPrimitiveValueProto,DOMCSSPrimitiveValueProtoFunc)

DOMCSSPrimitiveValue::DOMCSSPrimitiveValue(ExecState *exec, DOM::CSSPrimitiveValue v)
  : DOMCSSValue(DOMCSSPrimitiveValueProto::self(exec), v) { }

Value DOMCSSPrimitiveValue::tryGet(ExecState *exec, const UString &p) const
{
  if (p=="primitiveType")
    return Number(static_cast<DOM::CSSPrimitiveValue>(cssValue).primitiveType());
  return DOMObject::tryGet(exec,p);
}

Value DOMCSSPrimitiveValueProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  DOM::CSSPrimitiveValue val = static_cast<DOMCSSPrimitiveValue *>(thisObj.imp())->toCSSPrimitiveValue();
  switch (id) {
    case DOMCSSPrimitiveValue::SetFloatValue:
      val.setFloatValue(args[0].toInteger(exec),args[1].toNumber(exec));
      return Undefined();
    case DOMCSSPrimitiveValue::GetFloatValue:
      return Number(val.getFloatValue(args[0].toInteger(exec)));
    case DOMCSSPrimitiveValue::SetStringValue:
      val.setStringValue(args[0].toInteger(exec),args[1].toString(exec).string());
      return Undefined();
    case DOMCSSPrimitiveValue::GetStringValue:
      return getString(val.getStringValue());
    case DOMCSSPrimitiveValue::GetCounterValue:
      return getDOMCounter(exec,val.getCounterValue());
    case DOMCSSPrimitiveValue::GetRectValue:
      return getDOMRect(exec,val.getRectValue());
    case DOMCSSPrimitiveValue::GetRGBColorValue:
      return getDOMRGBColor(exec,val.getRGBColorValue());
    default:
      return Undefined();
  }
}

// -------------------------------------------------------------------------

const ClassInfo CSSPrimitiveValueConstructor::info = { "CSSPrimitiveValueConstructor", 0, &CSSPrimitiveValueConstructorTable, 0 };

/*
@begin CSSPrimitiveValueConstructorTable 27
  CSS_UNKNOWN   	DOM::CSSPrimitiveValue::CSS_UNKNOWN	DontDelete|ReadOnly
  CSS_NUMBER    	DOM::CSSPrimitiveValue::CSS_NUMBER	DontDelete|ReadOnly
  CSS_PERCENTAGE	DOM::CSSPrimitiveValue::CSS_PERCENTAGE	DontDelete|ReadOnly
  CSS_EMS       	DOM::CSSPrimitiveValue::CSS_EMS		DontDelete|ReadOnly
  CSS_EXS       	DOM::CSSPrimitiveValue::CSS_EXS		DontDelete|ReadOnly
  CSS_PX        	DOM::CSSPrimitiveValue::CSS_PX		DontDelete|ReadOnly
  CSS_CM        	DOM::CSSPrimitiveValue::CSS_CM		DontDelete|ReadOnly
  CSS_MM        	DOM::CSSPrimitiveValue::CSS_MM		DontDelete|ReadOnly
  CSS_IN        	DOM::CSSPrimitiveValue::CSS_IN		DontDelete|ReadOnly
  CSS_PT        	DOM::CSSPrimitiveValue::CSS_PT		DontDelete|ReadOnly
  CSS_PC        	DOM::CSSPrimitiveValue::CSS_PC		DontDelete|ReadOnly
  CSS_DEG       	DOM::CSSPrimitiveValue::CSS_DEG		DontDelete|ReadOnly
  CSS_RAD       	DOM::CSSPrimitiveValue::CSS_RAD		DontDelete|ReadOnly
  CSS_GRAD      	DOM::CSSPrimitiveValue::CSS_GRAD	DontDelete|ReadOnly
  CSS_MS        	DOM::CSSPrimitiveValue::CSS_MS		DontDelete|ReadOnly
  CSS_S			DOM::CSSPrimitiveValue::CSS_S		DontDelete|ReadOnly
  CSS_HZ        	DOM::CSSPrimitiveValue::CSS_HZ		DontDelete|ReadOnly
  CSS_KHZ       	DOM::CSSPrimitiveValue::CSS_KHZ		DontDelete|ReadOnly
  CSS_DIMENSION 	DOM::CSSPrimitiveValue::CSS_DIMENSION	DontDelete|ReadOnly
  CSS_STRING    	DOM::CSSPrimitiveValue::CSS_STRING	DontDelete|ReadOnly
  CSS_URI       	DOM::CSSPrimitiveValue::CSS_URI		DontDelete|ReadOnly
  CSS_IDENT     	DOM::CSSPrimitiveValue::CSS_IDENT	DontDelete|ReadOnly
  CSS_ATTR      	DOM::CSSPrimitiveValue::CSS_ATTR	DontDelete|ReadOnly
  CSS_COUNTER   	DOM::CSSPrimitiveValue::CSS_COUNTER	DontDelete|ReadOnly
  CSS_RECT      	DOM::CSSPrimitiveValue::CSS_RECT	DontDelete|ReadOnly
  CSS_RGBCOLOR  	DOM::CSSPrimitiveValue::CSS_RGBCOLOR	DontDelete|ReadOnly
@end
*/

Value CSSPrimitiveValueConstructor::tryGet(ExecState *exec, const UString &p) const
{
  return DOMObjectLookupGetValue<CSSPrimitiveValueConstructor,CSSValueConstructor>(exec,p,&CSSPrimitiveValueConstructorTable,this);
}

Value CSSPrimitiveValueConstructor::getValue(ExecState *, int token) const
{
  // We use the token as the value to return directly
  return Number(token);
}

Value KJS::getCSSPrimitiveValueConstructor(ExecState *exec)
{
  return cacheGlobalObject<CSSPrimitiveValueConstructor>( exec, "[[cssPrimitiveValue.constructor]]" );
}

// -------------------------------------------------------------------------

const ClassInfo DOMCSSValueList::info = { "CSSValueList", 0, &DOMCSSValueListTable, 0 };

/*
@begin DOMCSSValueListTable 3
  length		DOMCSSValueList::Length		DontDelete|ReadOnly
  item			DOMCSSValueList::Item		DontDelete|Function 1
@end
*/
IMPLEMENT_PROTOFUNC(DOMCSSValueListFunc) // not really a proto, but doesn't matter

DOMCSSValueList::DOMCSSValueList(ExecState *exec, DOM::CSSValueList v)
  : DOMCSSValue(exec, v) { }

Value DOMCSSValueList::tryGet(ExecState *exec, const UString &p) const
{
  Value result;
  DOM::CSSValueList valueList = static_cast<DOM::CSSValueList>(cssValue);

  if (p == "length")
    return Number(valueList.length());
  else if (p == "item")
    return lookupOrCreateFunction<DOMCSSValueListFunc>(exec,p,this,DOMCSSValueList::Item,1,DontDelete|Function);

  bool ok;
  long unsigned int u = p.toULong(&ok);
  if (ok)
    return getDOMCSSValue(exec,valueList.item(u));

  return DOMCSSValue::tryGet(exec,p);
}

Value DOMCSSValueListFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  DOM::CSSValueList valueList = static_cast<DOMCSSValueList *>(thisObj.imp())->toValueList();
  switch (id) {
    case DOMCSSValueList::Item:
      return getDOMCSSValue(exec,valueList.item(args[0].toInteger(exec)));
    default:
      return Undefined();
  }
}

// -------------------------------------------------------------------------

const ClassInfo DOMRGBColor::info = { "RGBColor", 0, &DOMRGBColorTable, 0 };

/*
@begin DOMRGBColorTable 3
  red	DOMRGBColor::Red	DontDelete|ReadOnly
  green	DOMRGBColor::Green	DontDelete|ReadOnly
  blue	DOMRGBColor::Blue	DontDelete|ReadOnly
@end
*/
DOMRGBColor::~DOMRGBColor()
{
  //rgbColors.remove(rgbColor.handle());
}

Value DOMRGBColor::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "red")
    return getDOMCSSValue(exec,rgbColor.red());
  if (p == "green")
    return getDOMCSSValue(exec,rgbColor.green());
  if (p == "blue")
    return getDOMCSSValue(exec,rgbColor.blue());
  return DOMObject::tryGet(exec,p);
}

Value KJS::getDOMRGBColor(ExecState *, DOM::RGBColor c)
{
  // ### implement equals for RGBColor since they're not refcounted objects
  return new DOMRGBColor(c);
}

// -------------------------------------------------------------------------

const ClassInfo DOMRect::info = { "Rect", 0, &DOMRectTable, 0 };
/*
@begin DOMRectTable 4
  top	DOMRect::Top	DontDelete|ReadOnly
  right	DOMRect::Right	DontDelete|ReadOnly
  bottom DOMRect::Bottom DontDelete|ReadOnly
  left	DOMRect::Left	DontDelete|ReadOnly
@end
*/
DOMRect::~DOMRect()
{
  rects.remove(rect.handle());
}

Value DOMRect::tryGet(ExecState *exec, const UString &p) const
{
  if (p == "top")
    return getDOMCSSValue(exec,rect.top());
  if (p == "right")
    return getDOMCSSValue(exec,rect.right());
  if (p == "bottom")
    return getDOMCSSValue(exec,rect.bottom());
  if (p == "left")
    return getDOMCSSValue(exec,rect.left());
  return DOMObject::tryGet(exec,p);
}

Value KJS::getDOMRect(ExecState *, DOM::Rect r)
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

const ClassInfo DOMCounter::info = { "Counter", 0, &DOMCounterTable, 0 };
/*
@begin DOMCounterTable 3
  identifier	DOMCounter::Identifier	DontDelete|ReadOnly
  listStyle	DOMCounter::ListStyle	DontDelete|ReadOnly
  separator	DOMCounter::Separator	DontDelete|ReadOnly
@end
*/
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

Value KJS::getDOMCounter(ExecState *, DOM::Counter c)
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




