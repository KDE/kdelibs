// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003 Apple Computer, Inc.
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

#include "kjs_css.h"
#include "kjs_css.lut.h"

#include <dom/html_head.h> // for HTMLStyleElement

#include <css/css_base.h>
#include "kjs_dom.h"

using namespace KJS;
#include <kdebug.h>

static QString cssPropertyName( const Identifier &p, bool& hadPixelPrefix )
{
    QString prop = p.qstring();
    int i = prop.length();
    while ( --i ) {
        char c = prop[i].latin1();
        if ( c >= 'A' && c <= 'Z' )
            prop.insert( i, '-' );
    }

    prop = prop.lower();
    hadPixelPrefix = false;

    if (prop.startsWith("css-")) {
        prop = prop.mid(4);
    } else if (prop.startsWith("pixel-")) {
        prop = prop.mid(6);
        hadPixelPrefix = true;
    } else if (prop.startsWith("pos-")) {
        prop = prop.mid(4);
        hadPixelPrefix = true;
    }

    return prop;
}

/*
@begin DOMCSSStyleDeclarationProtoTable 7
  getPropertyValue	DOMCSSStyleDeclaration::GetPropertyValue	DontDelete|Function 1
  getPropertyCSSValue	DOMCSSStyleDeclaration::GetPropertyCSSValue	DontDelete|Function 1
  removeProperty	DOMCSSStyleDeclaration::RemoveProperty		DontDelete|Function 1
  getPropertyPriority	DOMCSSStyleDeclaration::GetPropertyPriority	DontDelete|Function 1
  setProperty		DOMCSSStyleDeclaration::SetProperty		DontDelete|Function 3
  item			DOMCSSStyleDeclaration::Item			DontDelete|Function 1
# IE names for it (#36063)
  getAttribute          DOMCSSStyleDeclaration::GetPropertyValue	DontDelete|Function 1
  removeAttribute       DOMCSSStyleDeclaration::RemoveProperty		DontDelete|Function 1
  setAttribute		DOMCSSStyleDeclaration::SetProperty		DontDelete|Function 3
@end
@begin DOMCSSStyleDeclarationTable 3
  cssText		DOMCSSStyleDeclaration::CssText		DontDelete
  length		DOMCSSStyleDeclaration::Length		DontDelete|ReadOnly
  parentRule		DOMCSSStyleDeclaration::ParentRule	DontDelete|ReadOnly
@end
*/
DEFINE_PROTOTYPE("DOMCSSStyleDeclaration", DOMCSSStyleDeclarationProto)
IMPLEMENT_PROTOFUNC_DOM(DOMCSSStyleDeclarationProtoFunc)
IMPLEMENT_PROTOTYPE(DOMCSSStyleDeclarationProto, DOMCSSStyleDeclarationProtoFunc)

const ClassInfo DOMCSSStyleDeclaration::info = { "CSSStyleDeclaration", 0, &DOMCSSStyleDeclarationTable, 0 };

DOMCSSStyleDeclaration::DOMCSSStyleDeclaration(ExecState *exec, const DOM::CSSStyleDeclaration& s)
  : DOMObject(DOMCSSStyleDeclarationProto::self(exec)), styleDecl(s)
{ }

DOMCSSStyleDeclaration::~DOMCSSStyleDeclaration()
{
  ScriptInterpreter::forgetDOMObject(styleDecl.handle());
}

bool DOMCSSStyleDeclaration::hasProperty(ExecState *exec, const Identifier &p) const
{
  bool hadPixelPrefix;
  QString cssprop = cssPropertyName(p, hadPixelPrefix);
  if (DOM::getPropertyID(cssprop.latin1(), cssprop.length()))
      return true;

  return ObjectImp::hasProperty(exec, p);
}

Value DOMCSSStyleDeclaration::tryGet(ExecState *exec, const Identifier &propertyName) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMCSSStyleDeclaration::tryGet " << propertyName.qstring() << endl;
#endif
  const HashEntry* entry = Lookup::findEntry(&DOMCSSStyleDeclarationTable, propertyName);
  if (entry)
    switch (entry->value) {
    case CssText:
      return String(styleDecl.cssText());
    case Length:
      return Number(styleDecl.length());
    case ParentRule:
      return getDOMCSSRule(exec,styleDecl.parentRule());
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
    return String(DOM::CSSStyleDeclaration(styleDecl).item(u));

  // pixelTop returns "CSS Top" as number value in unit pixels
  // posTop returns "CSS top" as number value in unit pixels _if_ its a
  // positioned element. if it is not a positioned element, return 0
  // from MSIE documentation ### IMPLEMENT THAT (Dirk)
  bool asNumber;
  QString p = cssPropertyName(propertyName, asNumber);

#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMCSSStyleDeclaration: converting to css property name: " << p << ( asNumber ? "px" : "" ) << endl;
#endif

  if (asNumber) {
    DOM::CSSValue v = styleDecl.getPropertyCSSValue(p);
    if ( !v.isNull() && v.cssValueType() == DOM::CSSValue::CSS_PRIMITIVE_VALUE)
      return Number(static_cast<DOM::CSSPrimitiveValue>(v).getFloatValue(DOM::CSSPrimitiveValue::CSS_PX));
  }

  DOM::DOMString str = const_cast<DOM::CSSStyleDeclaration &>( styleDecl ).getPropertyValue(p);
  if (!str.isNull())
    return String(str);

  // see if we know this css property, return empty then
  if (DOM::getPropertyID(p.latin1(), p.length()))
      return String(DOM::DOMString(""));

  return DOMObject::tryGet(exec, propertyName);
}


void DOMCSSStyleDeclaration::tryPut(ExecState *exec, const Identifier &propertyName, const Value& value, int attr )
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMCSSStyleDeclaration::tryPut " << propertyName.qstring() << endl;
#endif
  if (propertyName == "cssText") {
    styleDecl.setCssText(value.toString(exec).string());
  }
  else {
    bool pxSuffix;
    QString prop = cssPropertyName(propertyName, pxSuffix);
    QString propvalue = value.toString(exec).qstring();

    if (pxSuffix)
      propvalue += "px";
#ifdef KJS_VERBOSE
    kdDebug(6070) << "DOMCSSStyleDeclaration: prop=" << prop << " propvalue=" << propvalue << endl;
#endif
    // Look whether the property is known.d In that case add it as a CSS property.
    if (DOM::getPropertyID(prop.latin1(), prop.length())) {
      if (propvalue.isEmpty())
        styleDecl.removeProperty(prop);
      else
        styleDecl.setProperty(prop,DOM::DOMString(propvalue),""); // ### is "" ok for priority?
    }
    else
      // otherwise add it as a JS property
      DOMObject::tryPut( exec, propertyName, value, attr );
  }
}

Value DOMCSSStyleDeclarationProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMCSSStyleDeclaration, thisObj );
  DOM::CSSStyleDeclaration styleDecl = static_cast<DOMCSSStyleDeclaration *>(thisObj.imp())->toStyleDecl();
  String str = args[0].toString(exec);
  DOM::DOMString s = str.value().string();

  switch (id) {
    case DOMCSSStyleDeclaration::GetPropertyValue:
      return String(styleDecl.getPropertyValue(s));
    case DOMCSSStyleDeclaration::GetPropertyCSSValue:
      return getDOMCSSValue(exec,styleDecl.getPropertyCSSValue(s));
    case DOMCSSStyleDeclaration::RemoveProperty:
      return String(styleDecl.removeProperty(s));
    case DOMCSSStyleDeclaration::GetPropertyPriority:
      return String(styleDecl.getPropertyPriority(s));
    case DOMCSSStyleDeclaration::SetProperty:
      styleDecl.setProperty(args[0].toString(exec).string(),
                            args[1].toString(exec).string(),
                            args[2].toString(exec).string());
      return Undefined();
    case DOMCSSStyleDeclaration::Item:
      return String(styleDecl.item(args[0].toInteger(exec)));
    default:
      return Undefined();
  }
}

Value KJS::getDOMCSSStyleDeclaration(ExecState *exec, const DOM::CSSStyleDeclaration& s)
{
  return cacheDOMObject<DOM::CSSStyleDeclaration, KJS::DOMCSSStyleDeclaration>(exec, s);
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

DOMStyleSheet::DOMStyleSheet(ExecState* exec, const DOM::StyleSheet& ss)
  : DOMObject(exec->interpreter()->builtinObjectPrototype()), styleSheet(ss)
{
}

DOMStyleSheet::~DOMStyleSheet()
{
  ScriptInterpreter::forgetDOMObject(styleSheet.handle());
}

Value DOMStyleSheet::tryGet(ExecState *exec, const Identifier &propertyName) const
{
  return DOMObjectLookupGetValue<DOMStyleSheet,DOMObject>(exec,propertyName,&DOMStyleSheetTable,this);
}

Value DOMStyleSheet::getValueProperty(ExecState *exec, int token) const
{
  switch (token) {
  case Type:
    return String(styleSheet.type());
  case Disabled:
    return Boolean(styleSheet.disabled());
  case OwnerNode:
    return getDOMNode(exec,styleSheet.ownerNode());
  case ParentStyleSheet:
    return getDOMStyleSheet(exec,styleSheet.parentStyleSheet());
  case Href:
    return String(styleSheet.href());
  case Title:
    return String(styleSheet.title());
  case Media:
    return getDOMMediaList(exec, styleSheet.media());
  }
  return Value();
}

void DOMStyleSheet::tryPut(ExecState *exec, const Identifier &propertyName, const Value& value, int attr)
{
  if (propertyName == "disabled") {
    styleSheet.setDisabled(value.toBoolean(exec));
  }
  else
    DOMObject::tryPut(exec, propertyName, value, attr);
}

Value KJS::getDOMStyleSheet(ExecState *exec, const DOM::StyleSheet& ss)
{
  DOMObject *ret;
  if (ss.isNull())
    return Null();
  ScriptInterpreter* interp = static_cast<ScriptInterpreter *>(exec->interpreter());
  if ((ret = interp->getDOMObject(ss.handle())))
    return Value(ret);
  else {
    if (ss.isCSSStyleSheet()) {
      DOM::CSSStyleSheet cs;
      cs = ss;
      ret = new DOMCSSStyleSheet(exec,cs);
    }
    else
      ret = new DOMStyleSheet(exec,ss);
    interp->putDOMObject(ss.handle(),ret);
    return Value(ret);
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
IMPLEMENT_PROTOFUNC_DOM(DOMStyleSheetListFunc) // not really a proto, but doesn't matter

DOMStyleSheetList::DOMStyleSheetList(ExecState *exec, const DOM::StyleSheetList& ssl, const DOM::Document& doc)
  : DOMObject(exec->interpreter()->builtinObjectPrototype()), styleSheetList(ssl), m_doc(doc)
{
}

DOMStyleSheetList::~DOMStyleSheetList()
{
  ScriptInterpreter::forgetDOMObject(styleSheetList.handle());
}

Value DOMStyleSheetList::tryGet(ExecState *exec, const Identifier &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMStyleSheetList::tryGet " << p.qstring() << endl;
#endif
  if (p == lengthPropertyName)
    return Number(styleSheetList.length());
  else if (p == "item")
    return lookupOrCreateFunction<DOMStyleSheetListFunc>(exec,p,this,DOMStyleSheetList::Item,1,DontDelete|Function);

  // Retrieve stylesheet by index
  bool ok;
  long unsigned int u = p.toULong(&ok);
  if (ok)
    return getDOMStyleSheet(exec, DOM::StyleSheetList(styleSheetList).item(u));

  // IE also supports retrieving a stylesheet by name, using the name/id of the <style> tag
  // (this is consistent with all the other collections)
#if 0
  // Bad implementation because DOM::StyleSheet doesn't inherit DOM::Node
  // so we can't use DOMNamedNodesCollection.....
  // We could duplicate it for stylesheets though - worth it ?
  // Other problem of this implementation: it doesn't look for the ID attribute!
  DOM::NameNodeListImpl namedList( m_doc.documentElement().handle(), p.string() );
  int len = namedList.length();
  if ( len ) {
    QValueList<DOM::Node> styleSheets;
    for ( int i = 0 ; i < len ; ++i ) {
      DOM::HTMLStyleElement elem = DOM::Node(namedList.item(i));
      if (!elem.isNull())
        styleSheets.append(elem.sheet());
    }
    if ( styleSheets.count() == 1 ) // single result
      return getDOMStyleSheet(exec, styleSheets[0]);
    else if ( styleSheets.count() > 1 ) {
      return new DOMNamedItemsCollection(exec,styleSheets);
    }
  }
#endif
  // ### Bad implementation because returns a single element (are IDs always unique?)
  // and doesn't look for name attribute (see implementation above).
  // But unicity of stylesheet ids is good practice anyway ;)
  DOM::DOMString pstr = p.string();
  DOM::HTMLStyleElement styleElem = m_doc.getElementById( pstr );
  if (!styleElem.isNull())
    return getDOMStyleSheet(exec, styleElem.sheet());

  return DOMObject::tryGet(exec, p);
}

Value KJS::DOMStyleSheetList::call(ExecState *exec, Object &thisObj, const List &args)
{
  // This code duplication is necessary, DOMStyleSheetList isn't a DOMFunction
  Value val;
  try {
    val = tryCall(exec, thisObj, args);
  }
  // pity there's no way to distinguish between these in JS code
  catch (...) {
    Object err = Error::create(exec, GeneralError, "Exception from DOMStyleSheetList");
    exec->setException(err);
  }
  return val;
}

Value DOMStyleSheetList::tryCall(ExecState *exec, Object & /*thisObj*/, const List &args)
{
  if (args.size() == 1) {
    // support for styleSheets(<index>) and styleSheets(<name>)
    return tryGet( exec, Identifier(args[0].toString(exec)) );
  }
  return Undefined();
}

Value KJS::getDOMStyleSheetList(ExecState *exec, const DOM::StyleSheetList& ssl, const DOM::Document& doc)
{
  // Can't use the cacheDOMObject macro because of the doc argument
  DOMObject *ret;
  if (ssl.isNull())
    return Null();
  ScriptInterpreter* interp = static_cast<ScriptInterpreter *>(exec->interpreter());
  if ((ret = interp->getDOMObject(ssl.handle())))
    return Value(ret);
  else {
    ret = new DOMStyleSheetList(exec, ssl, doc);
    interp->putDOMObject(ssl.handle(),ret);
    return Value(ret);
  }
}

Value DOMStyleSheetListFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMStyleSheetList, thisObj );
  DOM::StyleSheetList styleSheetList = static_cast<DOMStyleSheetList *>(thisObj.imp())->toStyleSheetList();
  if (id == DOMStyleSheetList::Item)
    return getDOMStyleSheet(exec, styleSheetList.item(args[0].toInteger(exec)));
  return Undefined();
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
IMPLEMENT_PROTOFUNC_DOM(DOMMediaListProtoFunc)
IMPLEMENT_PROTOTYPE(DOMMediaListProto, DOMMediaListProtoFunc)

DOMMediaList::DOMMediaList(ExecState *exec, const DOM::MediaList& ml)
  : DOMObject(DOMMediaListProto::self(exec)), mediaList(ml) { }

DOMMediaList::~DOMMediaList()
{
  ScriptInterpreter::forgetDOMObject(mediaList.handle());
}

Value DOMMediaList::tryGet(ExecState *exec, const Identifier &p) const
{
  if (p == "mediaText")
    return String(mediaList.mediaText());
  else if (p == lengthPropertyName)
    return Number(mediaList.length());

  bool ok;
  long unsigned int u = p.toULong(&ok);
  if (ok)
    return String(mediaList.item(u));

  return DOMObject::tryGet(exec, p);
}

void DOMMediaList::tryPut(ExecState *exec, const Identifier &propertyName, const Value& value, int attr)
{
  if (propertyName == "mediaText")
    mediaList.setMediaText(value.toString(exec).string());
  else
    DOMObject::tryPut(exec, propertyName, value, attr);
}

Value KJS::getDOMMediaList(ExecState *exec, const DOM::MediaList& ml)
{
  return cacheDOMObject<DOM::MediaList, KJS::DOMMediaList>(exec, ml);
}

Value KJS::DOMMediaListProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMMediaList, thisObj );
  DOM::MediaList mediaList = static_cast<DOMMediaList *>(thisObj.imp())->toMediaList();
  switch (id) {
    case DOMMediaList::Item:
      return String(mediaList.item(args[0].toInteger(exec)));
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
# MSIE extension
  rules		DOMCSSStyleSheet::Rules		DontDelete|ReadOnly
@end
@begin DOMCSSStyleSheetProtoTable 2
  insertRule	DOMCSSStyleSheet::InsertRule	DontDelete|Function 2
  deleteRule	DOMCSSStyleSheet::DeleteRule	DontDelete|Function 1
# IE extensions
  addRule	DOMCSSStyleSheet::AddRule	DontDelete|Function 3
  removeRule	DOMCSSStyleSheet::RemoveRule	DontDelete|Function 1
@end
*/
DEFINE_PROTOTYPE("DOMCSSStyleSheet",DOMCSSStyleSheetProto)
IMPLEMENT_PROTOFUNC_DOM(DOMCSSStyleSheetProtoFunc)
IMPLEMENT_PROTOTYPE(DOMCSSStyleSheetProto,DOMCSSStyleSheetProtoFunc) // warning, use _WITH_PARENT if DOMStyleSheet gets a proto

DOMCSSStyleSheet::DOMCSSStyleSheet(ExecState *exec, const DOM::CSSStyleSheet& ss)
  : DOMStyleSheet(DOMCSSStyleSheetProto::self(exec),ss) { }

DOMCSSStyleSheet::~DOMCSSStyleSheet()
{
}

Value DOMCSSStyleSheet::tryGet(ExecState *exec, const Identifier &p) const
{
  DOM::CSSStyleSheet cssStyleSheet = static_cast<DOM::CSSStyleSheet>(styleSheet);
  if (p == "ownerRule")
    return getDOMCSSRule(exec,cssStyleSheet.ownerRule());
  else if (p == "cssRules" || p == "rules" /* MSIE extension */)
    return getDOMCSSRuleList(exec,cssStyleSheet.cssRules());
  return DOMStyleSheet::tryGet(exec,p);
}

Value DOMCSSStyleSheetProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMCSSStyleSheet, thisObj );
  DOM::CSSStyleSheet styleSheet = static_cast<DOMCSSStyleSheet *>(thisObj.imp())->toCSSStyleSheet();

  switch (id) {
    case DOMCSSStyleSheet::InsertRule:
      return Number(styleSheet.insertRule(args[0].toString(exec).string(),(long unsigned int)args[1].toInteger(exec)));
    case DOMCSSStyleSheet::DeleteRule:
      styleSheet.deleteRule(args[0].toInteger(exec));
      return Undefined();
    // IE extensions
    case DOMCSSStyleSheet::AddRule: {
      DOM::DOMString str = args[0].toString(exec).string() + " { " + args[1].toString(exec).string() + " } ";
      return Number(styleSheet.insertRule(str,(long unsigned int)args[2].toInteger(exec)));
    }
    case DOMCSSStyleSheet::RemoveRule: {
      int index = args.size() > 0 ? args[0].toInteger(exec) : 0 /*first one*/;
      styleSheet.deleteRule(index);
      return Undefined();
    }
    default:
      return Undefined();
  }
}

// -------------------------------------------------------------------------

const ClassInfo DOMCSSRuleList::info = { "CSSRuleList", 0, &DOMCSSRuleListTable, 0 };
/*
@begin DOMCSSRuleListTable 3
  length		DOMCSSRuleList::Length		DontDelete|ReadOnly
  item			DOMCSSRuleList::Item		DontDelete|Function 1
@end
*/
IMPLEMENT_PROTOFUNC_DOM(DOMCSSRuleListFunc) // not really a proto, but doesn't matter

DOMCSSRuleList::DOMCSSRuleList(ExecState* exec, const DOM::CSSRuleList& rl)
  : DOMObject(exec->interpreter()->builtinObjectPrototype()), cssRuleList(rl)
{
}

DOMCSSRuleList::~DOMCSSRuleList()
{
  ScriptInterpreter::forgetDOMObject(cssRuleList.handle());
}

Value DOMCSSRuleList::tryGet(ExecState *exec, const Identifier &p) const
{
  Value result;
  if (p == lengthPropertyName)
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
  KJS_CHECK_THIS( KJS::DOMCSSRuleList, thisObj );
  DOM::CSSRuleList cssRuleList = static_cast<DOMCSSRuleList *>(thisObj.imp())->toCSSRuleList();
  switch (id) {
    case DOMCSSRuleList::Item:
      return getDOMCSSRule(exec,cssRuleList.item(args[0].toInteger(exec)));
    default:
      return Undefined();
  }
}

Value KJS::getDOMCSSRuleList(ExecState *exec, const DOM::CSSRuleList& rl)
{
  return cacheDOMObject<DOM::CSSRuleList, KJS::DOMCSSRuleList>(exec, rl);
}

// -------------------------------------------------------------------------

IMPLEMENT_PROTOFUNC_DOM(DOMCSSRuleFunc) // Not a proto, but doesn't matter

DOMCSSRule::DOMCSSRule(ExecState* exec, const DOM::CSSRule& r)
  : DOMObject(exec->interpreter()->builtinObjectPrototype()), cssRule(r)
{
}

DOMCSSRule::~DOMCSSRule()
{
  ScriptInterpreter::forgetDOMObject(cssRule.handle());
}

const ClassInfo DOMCSSRule::info = { "CSSRule", 0, &DOMCSSRuleTable, 0 };
const ClassInfo DOMCSSRule::style_info = { "CSSStyleRule", &DOMCSSRule::info, &DOMCSSStyleRuleTable, 0 };
const ClassInfo DOMCSSRule::media_info = { "CSSMediaRule", &DOMCSSRule::info, &DOMCSSMediaRuleTable, 0 };
const ClassInfo DOMCSSRule::fontface_info = { "CSSFontFaceRule", &DOMCSSRule::info, &DOMCSSFontFaceRuleTable, 0 };
const ClassInfo DOMCSSRule::page_info = { "CSSPageRule", &DOMCSSRule::info, &DOMCSSPageRuleTable, 0 };
const ClassInfo DOMCSSRule::import_info = { "CSSImportRule", &DOMCSSRule::info, &DOMCSSImportRuleTable, 0 };
const ClassInfo DOMCSSRule::charset_info = { "CSSCharsetRule", &DOMCSSRule::info, &DOMCSSCharsetRuleTable, 0 };

const ClassInfo* DOMCSSRule::classInfo() const
{
  switch (cssRule.type()) {
  case DOM::CSSRule::STYLE_RULE:
    return &style_info;
  case DOM::CSSRule::MEDIA_RULE:
    return &media_info;
  case DOM::CSSRule::FONT_FACE_RULE:
    return &fontface_info;
  case DOM::CSSRule::PAGE_RULE:
    return &page_info;
  case DOM::CSSRule::IMPORT_RULE:
    return &import_info;
  case DOM::CSSRule::CHARSET_RULE:
    return &charset_info;
  case DOM::CSSRule::UNKNOWN_RULE:
  default:
    return &info;
  }
}
/*
@begin DOMCSSRuleTable 4
  type			DOMCSSRule::Type	DontDelete|ReadOnly
  cssText		DOMCSSRule::CssText	DontDelete|ReadOnly
  parentStyleSheet	DOMCSSRule::ParentStyleSheet	DontDelete|ReadOnly
  parentRule		DOMCSSRule::ParentRule	DontDelete|ReadOnly
@end
@begin DOMCSSStyleRuleTable 2
  selectorText		DOMCSSRule::Style_SelectorText	DontDelete
  style			DOMCSSRule::Style_Style		DontDelete|ReadOnly
@end
@begin DOMCSSMediaRuleTable 4
  media			DOMCSSRule::Media_Media		DontDelete|ReadOnly
  cssRules		DOMCSSRule::Media_CssRules	DontDelete|ReadOnly
  insertRule		DOMCSSRule::Media_InsertRule	DontDelete|Function 2
  deleteRule		DOMCSSRule::Media_DeleteRule	DontDelete|Function 1
@end
@begin DOMCSSFontFaceRuleTable 1
  style			DOMCSSRule::FontFace_Style	DontDelete|ReadOnly
@end
@begin DOMCSSPageRuleTable 2
  selectorText		DOMCSSRule::Page_SelectorText	DontDelete
  style			DOMCSSRule::Page_Style		DontDelete|ReadOnly
@end
@begin DOMCSSImportRuleTable 3
  href			DOMCSSRule::Import_Href		DontDelete|ReadOnly
  media			DOMCSSRule::Import_Media	DontDelete|ReadOnly
  styleSheet		DOMCSSRule::Import_StyleSheet	DontDelete|ReadOnly
@end
@begin DOMCSSCharsetRuleTable 1
  encoding		DOMCSSRule::Charset_Encoding	DontDelete
@end
*/
Value DOMCSSRule::tryGet(ExecState *exec, const Identifier &propertyName) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "DOMCSSRule::tryGet " << propertyName.qstring() << endl;
#endif
  const HashTable* table = classInfo()->propHashTable; // get the right hashtable
  const HashEntry* entry = Lookup::findEntry(table, propertyName);
  if (entry) {
    if (entry->attr & Function)
      return lookupOrCreateFunction<DOMCSSRuleFunc>(exec, propertyName, this, entry->value, entry->params, entry->attr);
    return getValueProperty(exec, entry->value);
  }

  // Base CSSRule stuff or parent class forward, as usual
  return DOMObjectLookupGet<DOMCSSRuleFunc, DOMCSSRule, DOMObject>(exec, propertyName, &DOMCSSRuleTable, this);
}

Value DOMCSSRule::getValueProperty(ExecState *exec, int token) const
{
  switch (token) {
  case Type:
    return Number(cssRule.type());
  case CssText:
    return String(cssRule.cssText());
  case ParentStyleSheet:
    return getDOMStyleSheet(exec,cssRule.parentStyleSheet());
  case ParentRule:
    return getDOMCSSRule(exec,cssRule.parentRule());

  // for DOM::CSSRule::STYLE_RULE:
  case Style_SelectorText:
    return String(static_cast<DOM::CSSStyleRule>(cssRule).selectorText());
  case Style_Style:
    return getDOMCSSStyleDeclaration(exec,static_cast<DOM::CSSStyleRule>(cssRule).style());

  // for DOM::CSSRule::MEDIA_RULE:
  case Media_Media:
    return getDOMMediaList(exec,static_cast<DOM::CSSMediaRule>(cssRule).media());
  case Media_CssRules:
    return getDOMCSSRuleList(exec,static_cast<DOM::CSSMediaRule>(cssRule).cssRules());

  // for DOM::CSSRule::FONT_FACE_RULE:
  case FontFace_Style:
    return getDOMCSSStyleDeclaration(exec,static_cast<DOM::CSSFontFaceRule>(cssRule).style());

  // for DOM::CSSRule::PAGE_RULE:
  case Page_SelectorText:
    return String(static_cast<DOM::CSSPageRule>(cssRule).selectorText());
  case Page_Style:
    return getDOMCSSStyleDeclaration(exec,static_cast<DOM::CSSPageRule>(cssRule).style());

  // for DOM::CSSRule::IMPORT_RULE:
  case Import_Href:
    return String(static_cast<DOM::CSSImportRule>(cssRule).href());
  case Import_Media:
    return getDOMMediaList(exec,static_cast<DOM::CSSImportRule>(cssRule).media());
  case Import_StyleSheet:
    return getDOMStyleSheet(exec,static_cast<DOM::CSSImportRule>(cssRule).styleSheet());

  // for DOM::CSSRule::CHARSET_RULE:
  case Charset_Encoding:
    return String(static_cast<DOM::CSSCharsetRule>(cssRule).encoding());

  default:
    kdDebug(6070) << "WARNING: DOMCSSRule::getValueProperty unhandled token " << token << endl;
  }
  return Undefined();
}

void DOMCSSRule::tryPut(ExecState *exec, const Identifier &propertyName, const Value& value, int attr)
{
  const HashTable* table = classInfo()->propHashTable; // get the right hashtable
  const HashEntry* entry = Lookup::findEntry(table, propertyName);
  if (entry) {
    if (entry->attr & Function) // function: put as override property
    {
      ObjectImp::put(exec, propertyName, value, attr);
      return;
    }
    else if ((entry->attr & ReadOnly) == 0) // let DOMObjectLookupPut print the warning if not
    {
      putValueProperty(exec, entry->value, value, attr);
      return;
    }
  }
  DOMObjectLookupPut<DOMCSSRule, DOMObject>(exec, propertyName, value, attr, &DOMCSSRuleTable, this);
}

void DOMCSSRule::putValueProperty(ExecState *exec, int token, const Value& value, int)
{
  switch (token) {
  // for DOM::CSSRule::STYLE_RULE:
  case Style_SelectorText:
    static_cast<DOM::CSSStyleRule>(cssRule).setSelectorText(value.toString(exec).string());
    return;

  // for DOM::CSSRule::PAGE_RULE:
  case Page_SelectorText:
    static_cast<DOM::CSSPageRule>(cssRule).setSelectorText(value.toString(exec).string());
    return;

  // for DOM::CSSRule::CHARSET_RULE:
  case Charset_Encoding:
    static_cast<DOM::CSSCharsetRule>(cssRule).setEncoding(value.toString(exec).string());
    return;

  default:
    kdDebug(6070) << "WARNING: DOMCSSRule::putValueProperty unhandled token " << token << endl;
  }
}

Value DOMCSSRuleFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMCSSRule, thisObj );
  DOM::CSSRule cssRule = static_cast<DOMCSSRule *>(thisObj.imp())->toCSSRule();

  if (cssRule.type() == DOM::CSSRule::MEDIA_RULE) {
    DOM::CSSMediaRule rule = static_cast<DOM::CSSMediaRule>(cssRule);
    if (id == DOMCSSRule::Media_InsertRule)
      return Number(rule.insertRule(args[0].toString(exec).string(),args[1].toInteger(exec)));
    else if (id == DOMCSSRule::Media_DeleteRule)
      rule.deleteRule(args[0].toInteger(exec));
  }

  return Undefined();
}

Value KJS::getDOMCSSRule(ExecState *exec, const DOM::CSSRule& r)
{
  return cacheDOMObject<DOM::CSSRule, KJS::DOMCSSRule>(exec, r);
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

CSSRuleConstructor::CSSRuleConstructor(ExecState *exec)
  : DOMObject(exec->interpreter()->builtinObjectPrototype())
{
}

Value CSSRuleConstructor::tryGet(ExecState *exec, const Identifier &p) const
{
  return DOMObjectLookupGetValue<CSSRuleConstructor,DOMObject>(exec,p,&CSSRuleConstructorTable,this);
}

Value CSSRuleConstructor::getValueProperty(ExecState *, int token) const
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

DOMCSSValue::DOMCSSValue(ExecState* exec, const DOM::CSSValue& val)
  : DOMObject(exec->interpreter()->builtinObjectPrototype()), cssValue(val)
{
}

DOMCSSValue::~DOMCSSValue()
{
  ScriptInterpreter::forgetDOMObject(cssValue.handle());
}

Value DOMCSSValue::tryGet(ExecState *exec, const Identifier &p) const
{
  if (p == "cssText")
    return String(cssValue.cssText());
  else if (p == "cssValueType")
    return Number(cssValue.cssValueType());
  return DOMObject::tryGet(exec,p);
}

void DOMCSSValue::tryPut(ExecState *exec, const Identifier &propertyName, const Value& value, int attr)
{
  if (propertyName == "cssText")
    cssValue.setCssText(value.toString(exec).string());
  else
    DOMObject::tryPut(exec, propertyName, value, attr);
}

Value KJS::getDOMCSSValue(ExecState *exec, const DOM::CSSValue& v)
{
  DOMObject *ret;
  if (v.isNull())
    return Null();
  ScriptInterpreter* interp = static_cast<ScriptInterpreter *>(exec->interpreter());
  if ((ret = interp->getDOMObject(v.handle())))
    return Value(ret);
  else {
    if (v.isCSSValueList())
      ret = new DOMCSSValueList(exec,v);
    else if (v.isCSSPrimitiveValue())
      ret = new DOMCSSPrimitiveValue(exec,v);
    else
      ret = new DOMCSSValue(exec,v);
    interp->putDOMObject(v.handle(),ret);
    return Value(ret);
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

CSSValueConstructor::CSSValueConstructor(ExecState *exec)
  : DOMObject(exec->interpreter()->builtinObjectPrototype())
{
}

Value CSSValueConstructor::tryGet(ExecState *exec, const Identifier &p) const
{
  return DOMObjectLookupGetValue<CSSValueConstructor,DOMObject>(exec,p,&CSSValueConstructorTable,this);
}

Value CSSValueConstructor::getValueProperty(ExecState *, int token) const
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
IMPLEMENT_PROTOFUNC_DOM(DOMCSSPrimitiveValueProtoFunc)
IMPLEMENT_PROTOTYPE(DOMCSSPrimitiveValueProto,DOMCSSPrimitiveValueProtoFunc)

DOMCSSPrimitiveValue::DOMCSSPrimitiveValue(ExecState *exec, const DOM::CSSPrimitiveValue& v)
  : DOMCSSValue(DOMCSSPrimitiveValueProto::self(exec), v) { }

Value DOMCSSPrimitiveValue::tryGet(ExecState *exec, const Identifier &p) const
{
  if (p=="primitiveType")
    return Number(static_cast<DOM::CSSPrimitiveValue>(cssValue).primitiveType());
  return DOMObject::tryGet(exec,p);
}

Value DOMCSSPrimitiveValueProtoFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMCSSPrimitiveValue, thisObj );
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
      return String(val.getStringValue());
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

Value CSSPrimitiveValueConstructor::tryGet(ExecState *exec, const Identifier &p) const
{
  return DOMObjectLookupGetValue<CSSPrimitiveValueConstructor,CSSValueConstructor>(exec,p,&CSSPrimitiveValueConstructorTable,this);
}

Value CSSPrimitiveValueConstructor::getValueProperty(ExecState *, int token) const
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
IMPLEMENT_PROTOFUNC_DOM(DOMCSSValueListFunc) // not really a proto, but doesn't matter

DOMCSSValueList::DOMCSSValueList(ExecState *exec, const DOM::CSSValueList& v)
  : DOMCSSValue(exec, v) { }

Value DOMCSSValueList::tryGet(ExecState *exec, const Identifier &p) const
{
  Value result;
  DOM::CSSValueList valueList = static_cast<DOM::CSSValueList>(cssValue);

  if (p == lengthPropertyName)
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
  KJS_CHECK_THIS( KJS::DOMCSSValueList, thisObj );
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

DOMRGBColor::DOMRGBColor(ExecState* exec, const DOM::RGBColor& c)
  : DOMObject(exec->interpreter()->builtinObjectPrototype()), rgbColor(c)
{
}

DOMRGBColor::~DOMRGBColor()
{
  //rgbColors.remove(rgbColor.handle());
}

Value DOMRGBColor::tryGet(ExecState *exec, const Identifier &p) const
{
  return DOMObjectLookupGetValue<DOMRGBColor,DOMObject>(exec, p,
						       &DOMRGBColorTable,
						       this);
}

Value DOMRGBColor::getValueProperty(ExecState *exec, int token) const
{
  switch (token) {
  case Red:
    return getDOMCSSValue(exec, rgbColor.red());
  case Green:
    return getDOMCSSValue(exec, rgbColor.green());
  case Blue:
    return getDOMCSSValue(exec, rgbColor.blue());
  default:
    return Value();
  }
}

Value KJS::getDOMRGBColor(ExecState *exec, const DOM::RGBColor& c)
{
  // ### implement equals for RGBColor since they're not refcounted objects
  return Value(new DOMRGBColor(exec, c));
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

DOMRect::DOMRect(ExecState *exec, const DOM::Rect& r)
  : DOMObject(exec->interpreter()->builtinObjectPrototype()), rect(r)
{
}

DOMRect::~DOMRect()
{
  ScriptInterpreter::forgetDOMObject(rect.handle());
}

Value DOMRect::tryGet(ExecState *exec, const Identifier &p) const
{
  return DOMObjectLookupGetValue<DOMRect,DOMObject>(exec, p,
						    &DOMRectTable, this);
}

Value DOMRect::getValueProperty(ExecState *exec, int token) const
{
  switch (token) {
  case Top:
    return getDOMCSSValue(exec, rect.top());
  case Right:
    return getDOMCSSValue(exec, rect.right());
  case Bottom:
    return getDOMCSSValue(exec, rect.bottom());
  case Left:
    return getDOMCSSValue(exec, rect.left());
  default:
    return Value();
  }
}

Value KJS::getDOMRect(ExecState *exec, const DOM::Rect& r)
{
  return cacheDOMObject<DOM::Rect, KJS::DOMRect>(exec, r);
}

// -------------------------------------------------------------------------

const ClassInfo DOMCounter::info = { "Counter", 0, &DOMCounterTable, 0 };
/*
@begin DOMCounterTable 3
  identifier	DOMCounter::identifier	DontDelete|ReadOnly
  listStyle	DOMCounter::listStyle	DontDelete|ReadOnly
  separator	DOMCounter::separator	DontDelete|ReadOnly
@end
*/
DOMCounter::DOMCounter(ExecState *exec, const DOM::Counter& c)
  : DOMObject(exec->interpreter()->builtinObjectPrototype()), counter(c)
{
}

DOMCounter::~DOMCounter()
{
  ScriptInterpreter::forgetDOMObject(counter.handle());
}

Value DOMCounter::tryGet(ExecState *exec, const Identifier &p) const
{
  return DOMObjectLookupGetValue<DOMCounter,DOMObject>(exec, p,
						       &DOMCounterTable, this);
}

Value DOMCounter::getValueProperty(ExecState *, int token) const
{
  switch (token) {
  case identifier:
    return String(counter.identifier());
  case listStyle:
    return String(counter.listStyle());
  case separator:
    return String(counter.separator());
  default:
    return Value();
  }
}

Value KJS::getDOMCounter(ExecState *exec, const DOM::Counter& c)
{
  return cacheDOMObject<DOM::Counter, KJS::DOMCounter>(exec, c);
}
