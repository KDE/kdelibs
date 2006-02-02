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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "kjs_css.h"
#include "kjs_css.lut.h"

#include "html/html_headimpl.h" // for HTMLStyleElementImpl

#include "dom/css_value.h"
#include "dom/css_rule.h"

#include "css/css_base.h"
#include "css/css_ruleimpl.h"
#include "css/css_renderstyledeclarationimpl.h"
#include "css/css_stylesheetimpl.h"
#include "css/css_valueimpl.h"

#include "misc/htmltags.h"

#include "kjs_dom.h"

using DOM::CSSCharsetRuleImpl;
using DOM::CSSFontFaceRuleImpl;
using DOM::CSSImportRuleImpl;
using DOM::CSSMediaRuleImpl;
using DOM::CSSPageRuleImpl;
using DOM::CSSPrimitiveValue;
using DOM::CSSPrimitiveValueImpl;
using DOM::CSSRule;
using DOM::CSSRuleImpl;
using DOM::CSSRuleListImpl;
using DOM::CSSStyleDeclarationImpl;
using DOM::CSSStyleRuleImpl;
using DOM::CSSStyleSheetImpl;
using DOM::CSSValue;
using DOM::CSSValueImpl;
using DOM::CSSValueListImpl;
using DOM::CounterImpl;
using DOM::DocumentImpl;
using DOM::DOMString;
using DOM::ElementImpl;
using DOM::HTMLStyleElementImpl;
using DOM::MediaListImpl;
using DOM::RectImpl;
using DOM::StyleSheetImpl;
using DOM::StyleSheetListImpl;

#include <kdebug.h>
#include <QList>

namespace KJS {

static QString cssPropertyName( const Identifier &p, bool& hadPixelPrefix )
{
    QString prop = p.qstring();
    int i = prop.length();
    while ( --i ) {
        char c = prop[i].latin1();
        if ( c >= 'A' && c <= 'Z' )
            prop.insert( i, '-' );
    }

    prop = prop.toLower();
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

static int cssPropertyId( const QString& p ) {
  return DOM::getPropertyID(p.latin1(), p.length());
}

static int cssPropertyId( const DOM::DOMString& name ) {
  return cssPropertyId(name.string());
}

static bool isCSSPropertyName(const Identifier &JSPropertyName)
{
    bool dummy;
    QString p = cssPropertyName(JSPropertyName, dummy);
    return cssPropertyId(p);
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
KJS_DEFINE_PROTOTYPE(DOMCSSStyleDeclarationProto)
KJS_IMPLEMENT_PROTOFUNC(DOMCSSStyleDeclarationProtoFunc)
KJS_IMPLEMENT_PROTOTYPE("DOMCSSStyleDeclaration", DOMCSSStyleDeclarationProto, DOMCSSStyleDeclarationProtoFunc)

const ClassInfo DOMCSSStyleDeclaration::info = { "CSSStyleDeclaration", 0, &DOMCSSStyleDeclarationTable, 0 };

DOMCSSStyleDeclaration::DOMCSSStyleDeclaration(ExecState *exec, DOM::CSSStyleDeclarationImpl* s)
  : DOMObject(), m_impl(s)
{
  setPrototype(DOMCSSStyleDeclarationProto::self(exec));
}

DOMCSSStyleDeclaration::~DOMCSSStyleDeclaration()
{
  ScriptInterpreter::forgetDOMObject(m_impl.get());
}

ValueImp* DOMCSSStyleDeclaration::getValueProperty(ExecState *exec, int token)
{
    //### null decls?
    switch (token) {
    case CssText:
        return String(m_impl->cssText());
    case Length:
        return jsNumber(m_impl->length());
    case ParentRule:
        return getDOMCSSRule(exec, m_impl->parentRule());
    }

    assert(0);
    return Undefined();
}

ValueImp *DOMCSSStyleDeclaration::indexGetter(ExecState* , unsigned index)
{
  return String(m_impl->item(index));
}

ValueImp *DOMCSSStyleDeclaration::cssPropertyGetter(ExecState *exec, JSObject*, const Identifier& propertyName, const PropertySlot& slot)
{
  DOMCSSStyleDeclaration *thisObj = static_cast<DOMCSSStyleDeclaration *>(slot.slotBase());

  // Set up pixelOrPos boolean to handle the fact that
  // pixelTop returns "CSS Top" as number value in unit pixels
  // posTop returns "CSS top" as number value in unit pixels _if_ its a
  // positioned element. if it is not a positioned element, return 0
  // from MSIE documentation ### IMPLEMENT THAT (Dirk)
  bool asNumber;
  DOMString p   = cssPropertyName(propertyName, asNumber);
  
  if (asNumber) {
    CSSValueImpl *v = thisObj->m_impl->getPropertyCSSValue(p);
    if (v->cssValueType() == DOM::CSSValue::CSS_PRIMITIVE_VALUE)
      //### FIXME: should this not set exception when type is wrong, or convert?
      return Number(static_cast<CSSPrimitiveValueImpl*>(v)->floatValue(DOM::CSSPrimitiveValue::CSS_PX));
  }

  DOM::DOMString str = thisObj->m_impl->getPropertyValue(p);
  if (!str.isNull())
    return String(str);

  //we know this css property, but nothing set, return empty then
  return String("");
}


bool DOMCSSStyleDeclaration::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
#ifdef KJS_VERBOSE
  kDebug(6070) << "DOMCSSStyleDeclaration::getOwnPropertySlot " << propertyName.qstring() << endl;
#endif

  if (getStaticOwnValueSlot(&DOMCSSStyleDeclarationTable, this, propertyName, slot))
    return true;

  //Check whether it's an index
  if (getIndexSlot(this, propertyName, slot))
    return true;

  if (isCSSPropertyName(propertyName)) {
    slot.setCustom(this, cssPropertyGetter);
    return true;
  }

  return DOMObject::getOwnPropertySlot(exec, propertyName, slot);
}


void DOMCSSStyleDeclaration::put(ExecState *exec, const Identifier &propertyName, ValueImp *value, int attr )
{
#ifdef KJS_VERBOSE
  kDebug(6070) << "DOMCSSStyleDeclaration::put " << propertyName.qstring() << endl;
#endif
  DOMExceptionTranslator exception(exec);
  CSSStyleDeclarationImpl &styleDecl = *m_impl;

  if (propertyName == "cssText") {
    styleDecl.setCssText(value->toString(exec).domString());
  }
  else {
    bool pxSuffix;
    QString prop = cssPropertyName(propertyName, pxSuffix);
    QString propvalue = value->toString(exec).qstring();

    if (pxSuffix)
      propvalue += "px";
#ifdef KJS_VERBOSE
    kDebug(6070) << "DOMCSSStyleDeclaration: prop=" << prop << " propvalue=" << propvalue << endl;
#endif
    // Look whether the property is known.d In that case add it as a CSS property.
    if (int pId = cssPropertyId(prop)) {
      if (propvalue.isEmpty())
        styleDecl.removeProperty(pId);
      else
        styleDecl.setProperty(pId,DOM::DOMString(propvalue),"", exception); // ### is "" ok for priority?
    }
    else
      // otherwise add it as a JS property
      DOMObject::put( exec, propertyName, value, attr );
  }
}

ValueImp *DOMCSSStyleDeclarationProtoFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMCSSStyleDeclaration, thisObj );
  CSSStyleDeclarationImpl& styleDecl = *static_cast<DOMCSSStyleDeclaration*>(thisObj)->impl();

  DOM::DOMString s = args[0]->toString(exec).domString();

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
      styleDecl.setProperty(args[0]->toString(exec).domString(),
                            args[1]->toString(exec).domString(),
                            args[2]->toString(exec).domString());
      return Undefined();
    case DOMCSSStyleDeclaration::Item:
      return String(styleDecl.item(args[0]->toInteger(exec)));
    default:
      return Undefined();
  }
}

ValueImp *getDOMCSSStyleDeclaration(ExecState *exec, CSSStyleDeclarationImpl *s)
{
  return cacheDOMObject<CSSStyleDeclarationImpl, DOMCSSStyleDeclaration>(exec, s);
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

DOMStyleSheet::DOMStyleSheet(ExecState* exec, DOM::StyleSheetImpl* ss)
  : m_impl(ss)
{
  setPrototype(exec->lexicalInterpreter()->builtinObjectPrototype());
}

DOMStyleSheet::~DOMStyleSheet()
{
  ScriptInterpreter::forgetDOMObject(m_impl.get());
}

bool DOMStyleSheet::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  return getStaticValueSlot<DOMStyleSheet, DOMObject>(exec, &DOMStyleSheetTable, this, propertyName, slot);
}

ValueImp *DOMStyleSheet::getValueProperty(ExecState *exec, int token) const
{
  StyleSheetImpl &styleSheet = *m_impl;
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
  return 0;
}

void DOMStyleSheet::put(ExecState *exec, const Identifier &propertyName, ValueImp *value, int attr)
{
  StyleSheetImpl &styleSheet = *m_impl;
  if (propertyName == "disabled") {
    styleSheet.setDisabled(value->toBoolean(exec));
  }
  else
    DOMObject::put(exec, propertyName, value, attr);
}

ValueImp *getDOMStyleSheet(ExecState *exec, DOM::StyleSheetImpl* ss)
{
  if (!ss)
    return Null();

  DOMObject *ret;
  ScriptInterpreter* interp = static_cast<ScriptInterpreter *>(exec->interpreter());
  if ((ret = interp->getDOMObject(ss)))
    return ret;
  else {
    if (ss->isCSSStyleSheet()) {
      CSSStyleSheetImpl* cs = static_cast<CSSStyleSheetImpl*>(ss);
      ret = new DOMCSSStyleSheet(exec,cs);
    }
    else
      ret = new DOMStyleSheet(exec,ss);
    interp->putDOMObject(ss,ret);
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
KJS_IMPLEMENT_PROTOFUNC(DOMStyleSheetListFunc) // not really a proto, but doesn't matter

DOMStyleSheetList::DOMStyleSheetList(ExecState *exec, DOM::StyleSheetListImpl* ssl, DOM::DocumentImpl* doc)
  : m_impl(ssl), m_doc(doc)
{
  setPrototype(exec->lexicalInterpreter()->builtinObjectPrototype());
}

DOMStyleSheetList::~DOMStyleSheetList()
{
  ScriptInterpreter::forgetDOMObject(m_impl.get());
}

ValueImp *DOMStyleSheetList::getValueProperty(ExecState *exec, int token) const
{
    switch(token) {
    case Length:
      return Number(m_impl->length());
    default:
      assert(0);
      return Undefined();
    }
}

ValueImp *DOMStyleSheetList::indexGetter(ExecState *exec, unsigned index)
{
  return getDOMStyleSheet(exec, m_impl->item(index));
}

ValueImp *DOMStyleSheetList::nameGetter(ExecState *exec, JSObject*, const Identifier& propertyName, const PropertySlot& slot)
{
  DOMStyleSheetList *thisObj = static_cast<DOMStyleSheetList *>(slot.slotBase());
  ElementImpl *element = thisObj->m_doc->getElementById(propertyName.domString());
  assert(element->id() == ID_STYLE); //Should be from existence check
  return getDOMStyleSheet(exec, static_cast<HTMLStyleElementImpl *>(element)->sheet());
}

bool DOMStyleSheetList::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
#ifdef KJS_VERBOSE
  kDebug(6070) << "DOMStyleSheetList::getOwnPropertySlot " << propertyName.qstring() << endl;
#endif
  if (getStaticOwnPropertySlot<DOMStyleSheetListFunc, DOMStyleSheetList>(&DOMStyleSheetTable, this, propertyName, slot))
    return true;

  StyleSheetListImpl &styleSheetList = *m_impl;

  // Retrieve stylesheet by index
  if (getIndexSlot(this, styleSheetList, propertyName, slot))
    return true;

  // IE also supports retrieving a stylesheet by name, using the name/id of the <style> tag
  // (this is consistent with all the other collections)
  // ### Bad implementation because returns a single element (are IDs always unique?)
  // and doesn't look for name attribute (see implementation above).
  // But unicity of stylesheet ids is good practice anyway ;)
  ElementImpl *element = m_doc->getElementById(propertyName.domString());
  if (element && element->id() == ID_STYLE) {
    slot.setCustom(this, nameGetter);
    return true;
  }

  return DOMObject::getOwnPropertySlot(exec, propertyName, slot);
}

ValueImp *DOMStyleSheetList::callAsFunction(ExecState *exec, ObjectImp * /*thisObj*/, const List &args)
{
  if (args.size() == 1) {
    // support for styleSheets(<index>) and styleSheets(<name>)
    return get( exec, Identifier(args[0]->toString(exec)) );
  }
  return Undefined();
}

ValueImp *getDOMStyleSheetList(ExecState *exec, DOM::StyleSheetListImpl* ssl, DOM::DocumentImpl* doc)
{
  // Can't use the cacheDOMObject macro because of the doc argument
  DOMObject *ret;
  if (!ssl)
    return Null();
  ScriptInterpreter* interp = static_cast<ScriptInterpreter *>(exec->interpreter());
  if ((ret = interp->getDOMObject(ssl)))
    return ret;
  else {
    ret = new DOMStyleSheetList(exec, ssl, doc);
    interp->putDOMObject(ssl,ret);
    return ret;
  }
}

ValueImp *DOMStyleSheetListFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMStyleSheetList, thisObj );
  DOM::StyleSheetListImpl* styleSheetList = static_cast<DOMStyleSheetList *>(thisObj)->impl();
  if (id == DOMStyleSheetList::Item)
    return getDOMStyleSheet(exec, styleSheetList->item(args[0]->toInteger(exec)));
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
KJS_DEFINE_PROTOTYPE(DOMMediaListProto)
KJS_IMPLEMENT_PROTOFUNC(DOMMediaListProtoFunc)
KJS_IMPLEMENT_PROTOTYPE("DOMMediaList", DOMMediaListProto, DOMMediaListProtoFunc)

DOMMediaList::DOMMediaList(ExecState *exec, DOM::MediaListImpl* ml)
  : m_impl(ml)
{
  setPrototype(DOMMediaListProto::self(exec));
}

DOMMediaList::~DOMMediaList()
{
  ScriptInterpreter::forgetDOMObject(m_impl.get());
}

ValueImp* DOMMediaList::getValueProperty(ExecState *exec, int token) const
{
  const MediaListImpl& mediaList = *m_impl;
  switch (token)
  {
  case MediaText:
    return String(mediaList.mediaText());
  case Length:
    return Number(mediaList.length());
  default:
    assert(0);
    return Undefined();
  }
}

ValueImp *DOMMediaList::indexGetter(ExecState* exec, unsigned index)
{
  return String(m_impl->item(index));
}

bool DOMMediaList::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  if (getStaticOwnValueSlot(&DOMMediaListTable, this, propertyName, slot))
    return true;

  if (getIndexSlot(this, *m_impl, propertyName, slot))
    return true;

  return DOMObject::getOwnPropertySlot(exec, propertyName, slot);
}

void DOMMediaList::put(ExecState *exec, const Identifier &propertyName, ValueImp *value, int attr)
{
  if (propertyName == "mediaText")
    m_impl->setMediaText(value->toString(exec).domString());
  else
    DOMObject::put(exec, propertyName, value, attr);
}

ValueImp *getDOMMediaList(ExecState *exec, DOM::MediaListImpl* ml)
{
  return cacheDOMObject<DOM::MediaListImpl, KJS::DOMMediaList>(exec, ml);
}

ValueImp *DOMMediaListProtoFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMMediaList, thisObj );
  DOM::MediaListImpl& mediaList = *static_cast<DOMMediaList *>(thisObj)->impl();
  switch (id) {
    case DOMMediaList::Item:
      return String(mediaList.item(args[0]->toInteger(exec)));
    case DOMMediaList::DeleteMedium:
      mediaList.deleteMedium(args[0]->toString(exec).domString());
      return Undefined();
    case DOMMediaList::AppendMedium:
      mediaList.appendMedium(args[0]->toString(exec).domString());
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
KJS_DEFINE_PROTOTYPE(DOMCSSStyleSheetProto)
KJS_IMPLEMENT_PROTOFUNC(DOMCSSStyleSheetProtoFunc)
KJS_IMPLEMENT_PROTOTYPE("DOMCSSStyleSheet",DOMCSSStyleSheetProto,DOMCSSStyleSheetProtoFunc) // warning, use _WITH_PARENT if DOMStyleSheet gets a proto

DOMCSSStyleSheet::DOMCSSStyleSheet(ExecState *exec, DOM::CSSStyleSheetImpl* ss): DOMStyleSheet(exec, ss)
{
  setPrototype(DOMCSSStyleSheetProto::self(exec));
}

DOMCSSStyleSheet::~DOMCSSStyleSheet()
{}

ValueImp* DOMCSSStyleSheet::getValueProperty(ExecState *exec, int token)
{
  CSSStyleSheetImpl& cssStyleSheet = *impl();
  switch (token) {
    case OwnerRule:
      return getDOMCSSRule(exec,cssStyleSheet.ownerRule());
    case CssRules:
    case Rules: {
      //### this is a bit odd -- why is the impl returning the wrapper?
      DOM::CSSRuleList rules = cssStyleSheet.cssRules();
      SharedPtr<CSSRuleListImpl> ri = static_cast<CSSRuleListImpl*>(rules.handle());
      return getDOMCSSRuleList(exec, ri.get());
    }
    default:
      assert(0);
      return Undefined();
  }
}


bool DOMCSSStyleSheet::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  return getStaticValueSlot<DOMCSSStyleSheet, DOMStyleSheet>(exec, &DOMCSSStyleSheetTable, this, propertyName, slot);
}

ValueImp *DOMCSSStyleSheetProtoFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMCSSStyleSheet, thisObj );
  DOM::CSSStyleSheetImpl& styleSheet = *static_cast<DOMCSSStyleSheet *>(thisObj)->impl();
  DOMExceptionTranslator exception(exec);

  switch (id) {
    case DOMCSSStyleSheet::InsertRule:
      return Number(styleSheet.insertRule(args[0]->toString(exec).domString(),(long unsigned int)args[1]->toInteger(exec), exception));
    case DOMCSSStyleSheet::DeleteRule:
      styleSheet.deleteRule(args[0]->toInteger(exec), exception);
      return Undefined();
    // IE extensions
    case DOMCSSStyleSheet::AddRule: {
      DOM::DOMString str = args[0]->toString(exec).domString() + " { " + args[1]->toString(exec).domString() + " } ";
      return Number(styleSheet.insertRule(str,(long unsigned int)args[2]->toInteger(exec), exception));
    }
    case DOMCSSStyleSheet::RemoveRule: {
      int index = args.size() > 0 ? args[0]->toInteger(exec) : 0 /*first one*/;
      styleSheet.deleteRule(index, exception);
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
KJS_IMPLEMENT_PROTOFUNC(DOMCSSRuleListFunc) // not really a proto, but doesn't matter

DOMCSSRuleList::DOMCSSRuleList(ExecState* exec, DOM::CSSRuleListImpl* rl)
  : m_impl(rl)
{
  setPrototype(exec->lexicalInterpreter()->builtinObjectPrototype());
}

DOMCSSRuleList::~DOMCSSRuleList()
{
  ScriptInterpreter::forgetDOMObject(m_impl.get());
}

ValueImp *DOMCSSRuleList::getValueProperty(ExecState *exec, int token) const
{
  switch (token) {
  case Length:
    return Number(m_impl->length());
  default:
    assert(0);
    return Undefined();
  }
}

ValueImp *DOMCSSRuleList::indexGetter(ExecState* exec, unsigned index)
{
  return getDOMCSSRule(exec, m_impl->item(index));
}

bool DOMCSSRuleList::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  if (getStaticOwnPropertySlot<DOMCSSRuleListFunc, DOMCSSRuleList>(&DOMCSSRuleListTable, this, propertyName, slot))
    return true;

  //Check whether it's an index
  CSSRuleListImpl &cssRuleList = *m_impl;

  if (getIndexSlot(this, *m_impl, propertyName, slot))
    return true;

  return DOMObject::getOwnPropertySlot(exec, propertyName, slot);
}

ValueImp *DOMCSSRuleListFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMCSSRuleList, thisObj );
  DOM::CSSRuleListImpl& cssRuleList = *static_cast<DOMCSSRuleList *>(thisObj)->impl();
  switch (id) {
    case DOMCSSRuleList::Item:
      return getDOMCSSRule(exec,cssRuleList.item(args[0]->toInteger(exec)));
    default:
      return Undefined();
  }
}

ValueImp *getDOMCSSRuleList(ExecState *exec, DOM::CSSRuleListImpl* rl)
{
  return cacheDOMObject<DOM::CSSRuleListImpl, KJS::DOMCSSRuleList>(exec, rl);
}

// -------------------------------------------------------------------------

KJS_IMPLEMENT_PROTOFUNC(DOMCSSRuleFunc) // Not a proto, but doesn't matter

DOMCSSRule::DOMCSSRule(ExecState* exec, DOM::CSSRuleImpl* r)
  : m_impl(r)
{
  setPrototype(exec->lexicalInterpreter()->builtinObjectPrototype());
}

DOMCSSRule::~DOMCSSRule()
{
  ScriptInterpreter::forgetDOMObject(m_impl.get());
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
  switch (m_impl->type()) {
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
bool DOMCSSRule::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
#ifdef KJS_VERBOSE
  kDebug(6070) << "DOMCSSRule::tryGet " << propertyName.qstring() << endl;
#endif
  //First do the rule-type-specific stuff
  const HashTable* table = classInfo()->propHashTable; // get the right hashtable
  if (getStaticOwnPropertySlot<DOMCSSRuleFunc, DOMCSSRule>(table, this, propertyName, slot))
    return true;

  //Now do generic stuff
  return getStaticPropertySlot<DOMCSSRuleFunc, DOMCSSRule, DOMObject>(exec, &DOMCSSRuleTable, this, propertyName, slot);
}

ValueImp *DOMCSSRule::getValueProperty(ExecState *exec, int token) const
{
  CSSRuleImpl &cssRule = *m_impl;
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
    return String(static_cast<CSSStyleRuleImpl *>(m_impl.get())->selectorText());
  case Style_Style:
    return getDOMCSSStyleDeclaration(exec, static_cast<CSSStyleRuleImpl *>(m_impl.get())->style());

  // for DOM::CSSRule::MEDIA_RULE:
  case Media_Media:
    return getDOMMediaList(exec, static_cast<CSSMediaRuleImpl *>(m_impl.get())->media());
  case Media_CssRules:
    return getDOMCSSRuleList(exec, static_cast<CSSMediaRuleImpl *>(m_impl.get())->cssRules());

  // for DOM::CSSRule::FONT_FACE_RULE:
  case FontFace_Style:
    return getDOMCSSStyleDeclaration(exec, static_cast<CSSFontFaceRuleImpl *>(m_impl.get())->style());

  // for DOM::CSSRule::PAGE_RULE:
  case Page_SelectorText:
    return String(static_cast<CSSPageRuleImpl *>(m_impl.get())->selectorText());
  case Page_Style:
    return getDOMCSSStyleDeclaration(exec, static_cast<CSSPageRuleImpl *>(m_impl.get())->style());

  // for DOM::CSSRule::IMPORT_RULE:
  case Import_Href:
    return String(static_cast<CSSImportRuleImpl *>(m_impl.get())->href());
  case Import_Media:
    return getDOMMediaList(exec, static_cast<CSSImportRuleImpl *>(m_impl.get())->media());
  case Import_StyleSheet:
    return getDOMStyleSheet(exec, static_cast<CSSImportRuleImpl *>(m_impl.get())->styleSheet());

  // for DOM::CSSRule::CHARSET_RULE:
  case Charset_Encoding:
    return String(static_cast<CSSCharsetRuleImpl *>(m_impl.get())->encoding());

  default:
    assert(0);
  }
  return Undefined();
}

void DOMCSSRule::put(ExecState *exec, const Identifier &propertyName, ValueImp *value, int attr)
{
  const HashTable* table = classInfo()->propHashTable; // get the right hashtable
  const HashEntry* entry = Lookup::findEntry(table, propertyName);
  if (entry) {
    if (entry->attr & Function) // function: put as override property
    {
      ObjectImp::put(exec, propertyName, value, attr);
      return;
    }
    else if ((entry->attr & ReadOnly) == 0) // let lookupPut print the warning if not
    {
      putValueProperty(exec, entry->value, value, attr);
      return;
    }
  }
  lookupPut<DOMCSSRule, DOMObject>(exec, propertyName, value, attr, &DOMCSSRuleTable, this);
}

void DOMCSSRule::putValueProperty(ExecState *exec, int token, ValueImp *value, int)
{
  switch (token) {
  // for DOM::CSSRule::STYLE_RULE:
  case Style_SelectorText:
    static_cast<CSSStyleRuleImpl *>(m_impl.get())->setSelectorText(value->toString(exec).domString());
    return;

  // for DOM::CSSRule::PAGE_RULE:
  case Page_SelectorText:
    static_cast<CSSPageRuleImpl *>(m_impl.get())->setSelectorText(value->toString(exec).domString());
    return;

  // for DOM::CSSRule::CHARSET_RULE:
  case Charset_Encoding:
    static_cast<CSSCharsetRuleImpl *>(m_impl.get())->setEncoding(value->toString(exec).domString());
    return;

  default:
    kDebug(6070) << "DOMCSSRule::putValueProperty unhandled token " << token << endl;
  }
}

ValueImp *DOMCSSRuleFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMCSSRule, thisObj );
  DOM::CSSRuleImpl& cssRule = *static_cast<DOMCSSRule *>(thisObj)->impl();

  if (cssRule.type() == DOM::CSSRule::MEDIA_RULE) {
    DOM::CSSMediaRuleImpl& rule = static_cast<DOM::CSSMediaRuleImpl&>(cssRule);
    if (id == DOMCSSRule::Media_InsertRule)
      return Number(rule.insertRule(args[0]->toString(exec).domString(),args[1]->toInteger(exec)));
    else if (id == DOMCSSRule::Media_DeleteRule)
      rule.deleteRule(args[0]->toInteger(exec));
  }

  return Undefined();
}

ValueImp *getDOMCSSRule(ExecState *exec, DOM::CSSRuleImpl* r)
{
  return cacheDOMObject<DOM::CSSRuleImpl, KJS::DOMCSSRule>(exec, r);
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
{
  setPrototype(exec->lexicalInterpreter()->builtinObjectPrototype());
}

bool CSSRuleConstructor::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  return getStaticValueSlot<CSSRuleConstructor, DOMObject>(exec, &CSSRuleConstructorTable, this, propertyName, slot);
}

ValueImp *CSSRuleConstructor::getValueProperty(ExecState *, int token) const
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
  return 0;
}

ValueImp *getCSSRuleConstructor(ExecState *exec)
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

DOMCSSValue::DOMCSSValue(ExecState* exec, DOM::CSSValueImpl* val)
  : m_impl(val)
{
  setPrototype(exec->lexicalInterpreter()->builtinObjectPrototype());
}

DOMCSSValue::~DOMCSSValue()
{
  ScriptInterpreter::forgetDOMObject(m_impl.get());
}

ValueImp *DOMCSSValue::getValueProperty(ExecState *exec, int token) const
{
  CSSValueImpl &cssValue = *m_impl;
  switch (token) {
  case CssText:
    return String(cssValue.cssText());
  case CssValueType:
    return Number(cssValue.cssValueType());
  default:
    assert(0);
    return Undefined();
  }
}

bool DOMCSSValue::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  return getStaticValueSlot<DOMCSSValue, DOMObject>(exec, &DOMCSSValueTable, this, propertyName, slot);
}

void DOMCSSValue::put(ExecState *exec, const Identifier &propertyName, ValueImp *value, int attr)
{
  CSSValueImpl &cssValue = *m_impl;
  if (propertyName == "cssText") {
#warning "FIXME, we need cssText, currently b0rken"
    //cssValue.setCssText(value->toString(exec).domString());
  } else
    DOMObject::put(exec, propertyName, value, attr);
}

ValueImp *getDOMCSSValue(ExecState *exec, DOM::CSSValueImpl* v)
{
  DOMObject *ret;
  if (!v)
    return Null();
  ScriptInterpreter* interp = static_cast<ScriptInterpreter *>(exec->interpreter());
  if ((ret = interp->getDOMObject(v)))
    return ret;
  else {
    if (v->isValueList())
      ret = new DOMCSSValueList(exec, static_cast<CSSValueListImpl *>(v));
    else if (v->isPrimitiveValue())
      ret = new DOMCSSPrimitiveValue(exec, static_cast<CSSPrimitiveValueImpl *>(v));
    else
      ret = new DOMCSSValue(exec,v);
    interp->putDOMObject(v,ret);
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

CSSValueConstructor::CSSValueConstructor(ExecState *exec)
{
  setPrototype(exec->lexicalInterpreter()->builtinObjectPrototype());
}

bool CSSValueConstructor::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  return getStaticValueSlot<CSSValueConstructor, DOMObject>(exec, &CSSValueConstructorTable, this, propertyName, slot);
}

ValueImp *CSSValueConstructor::getValueProperty(ExecState *, int token) const
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
  return 0;
}

ValueImp *getCSSValueConstructor(ExecState *exec)
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
KJS_DEFINE_PROTOTYPE(DOMCSSPrimitiveValueProto)
KJS_IMPLEMENT_PROTOFUNC(DOMCSSPrimitiveValueProtoFunc)
KJS_IMPLEMENT_PROTOTYPE("DOMCSSPrimitiveValue",DOMCSSPrimitiveValueProto,DOMCSSPrimitiveValueProtoFunc)

DOMCSSPrimitiveValue::DOMCSSPrimitiveValue(ExecState *exec, DOM::CSSPrimitiveValueImpl* v)
  : DOMCSSValue(exec, v) {
  setPrototype(DOMCSSPrimitiveValueProto::self(exec));
}

ValueImp *DOMCSSPrimitiveValue::getValueProperty(ExecState *exec, int token)
{
  assert(token == PrimitiveType);
  return Number(static_cast<CSSPrimitiveValueImpl *>(impl())->primitiveType());
}

bool DOMCSSPrimitiveValue::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  return getStaticValueSlot<DOMCSSPrimitiveValue, DOMCSSValue>(exec, &DOMCSSPrimitiveValueTable, this, propertyName, slot);
}

ValueImp *DOMCSSPrimitiveValueProtoFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMCSSPrimitiveValue, thisObj );
  CSSPrimitiveValueImpl &val = *static_cast<CSSPrimitiveValueImpl *>(static_cast<DOMCSSPrimitiveValue *>(thisObj)->impl());
  DOMExceptionTranslator exception(exec);
  switch (id) {
    case DOMCSSPrimitiveValue::SetFloatValue:
      val.setFloatValue(args[0]->toInteger(exec),args[1]->toNumber(exec), exception);
      return Undefined();
    case DOMCSSPrimitiveValue::GetFloatValue:
      //### FIXME: exception?
      return Number(val.floatValue(args[0]->toInteger(exec)));
    case DOMCSSPrimitiveValue::SetStringValue:
      val.setStringValue(args[0]->toInteger(exec),args[1]->toString(exec).domString(), exception);
      return Undefined();
    case DOMCSSPrimitiveValue::GetStringValue:
      return String(DOM::DOMString(val.getStringValue()));
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

bool CSSPrimitiveValueConstructor::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot) {
  return getStaticValueSlot<CSSPrimitiveValueConstructor, DOMObject>(exec, &CSSPrimitiveValueConstructorTable, this, propertyName, slot);
}

ValueImp *CSSPrimitiveValueConstructor::getValueProperty(ExecState *, int token) const
{
  // We use the token as the value to return directly
  return Number(token);
}

ValueImp *getCSSPrimitiveValueConstructor(ExecState *exec)
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
KJS_IMPLEMENT_PROTOFUNC(DOMCSSValueListFunc) // not really a proto, but doesn't matter

DOMCSSValueList::DOMCSSValueList(ExecState *exec, DOM::CSSValueListImpl* v)
  : DOMCSSValue(exec, v) { }


ValueImp *DOMCSSValueList::indexGetter(ExecState *exec, unsigned index)
{
  return getDOMCSSValue(exec, impl()->item(index));
}

bool DOMCSSValueList::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  if (getStaticOwnPropertySlot<DOMCSSValueListFunc, DOMCSSValueList>(
        &DOMCSSValueListTable, this, propertyName, slot))
    return true;

  CSSValueListImpl &valueList = *static_cast<CSSValueListImpl *>(impl());
  if (getIndexSlot(this, valueList, propertyName, slot))
    return true;

  return DOMCSSValue::getOwnPropertySlot(exec, propertyName, slot);
}

ValueImp *DOMCSSValueListFunc::callAsFunction(ExecState *exec, ObjectImp *thisObj, const List &args)
{
  KJS_CHECK_THIS( KJS::DOMCSSValueList, thisObj );
  CSSValueListImpl &valueList = *static_cast<CSSValueListImpl *>(static_cast<DOMCSSValueList *>(thisObj)->impl());
  switch (id) {
    case DOMCSSValueList::Item:
      return getDOMCSSValue(exec,valueList.item(args[0]->toInteger(exec)));
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

DOMRGBColor::DOMRGBColor(ExecState* exec, QRgb c)
  : m_color(c)
{
  setPrototype(exec->lexicalInterpreter()->builtinObjectPrototype());
}

bool DOMRGBColor::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot) {
  return getStaticValueSlot<DOMRGBColor, DOMObject>(exec, &DOMRGBColorTable, this, propertyName, slot);
}

ValueImp *DOMRGBColor::getValueProperty(ExecState *exec, int token) const
{
  int color;
  switch (token) {
  case Red:
    color = qRed(m_color); break;
  case Green:
    color = qGreen(m_color); break;
  case Blue:
    color = qBlue(m_color); break;
  default:
    assert(0);
    return Undefined();
  }

  return new DOMCSSPrimitiveValue(exec, new CSSPrimitiveValueImpl(color, CSSPrimitiveValue::CSS_NUMBER));
}

ValueImp *getDOMRGBColor(ExecState *exec, unsigned color)
{
  // ### implement equals for RGBColor since they're not refcounted objects
  return new DOMRGBColor(exec, color);
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

DOMRect::DOMRect(ExecState *exec, DOM::RectImpl* r)
  : m_impl(r)
{
  setPrototype(exec->lexicalInterpreter()->builtinObjectPrototype());
}

DOMRect::~DOMRect()
{
  ScriptInterpreter::forgetDOMObject(m_impl.get());
}

bool DOMRect::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  return getStaticValueSlot<DOMRect, DOMObject>(exec, &DOMRectTable, this, propertyName, slot);
}

ValueImp *DOMRect::getValueProperty(ExecState *exec, int token) const
{
  DOM::RectImpl& rect = *m_impl;
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
    return 0;
  }
}

ValueImp *getDOMRect(ExecState *exec, DOM::RectImpl* r)
{
  return cacheDOMObject<DOM::RectImpl, KJS::DOMRect>(exec, r);
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
DOMCounter::DOMCounter(ExecState *exec, DOM::CounterImpl* c)
  : m_impl(c)
{
  setPrototype(exec->lexicalInterpreter()->builtinObjectPrototype());
}

DOMCounter::~DOMCounter()
{
  ScriptInterpreter::forgetDOMObject(m_impl.get());
}

bool DOMCounter::getOwnPropertySlot(ExecState *exec, const Identifier& propertyName, PropertySlot& slot)
{
  return getStaticValueSlot<DOMCounter, DOMObject>(exec, &DOMCounterTable, this, propertyName, slot);
}

ValueImp *DOMCounter::getValueProperty(ExecState *, int token) const
{
  CounterImpl &counter = *m_impl;
  switch (token) {
  case identifier:
    return String(counter.identifier());
  case listStyle:
    return String(khtml::stringForListStyleType((khtml::EListStyleType)counter.listStyle()));
  case separator:
    return String(counter.separator());
  default:
    return 0;
  }
}

ValueImp *getDOMCounter(ExecState *exec, DOM::CounterImpl* c)
{
  return cacheDOMObject<DOM::CounterImpl, KJS::DOMCounter>(exec, c);
}

}
