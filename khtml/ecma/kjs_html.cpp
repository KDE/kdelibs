/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
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

#include <stdio.h>

#include <html_element.h>
#include <html_head.h>
#include <html_inline.h>
#include <html_image.h>
#include <dom_string.h>

#include <kjs/operations.h>
#include "kjs_dom.h"
#include "kjs_html.h"

#include <htmltags.h>

using namespace KJS;

KJSO KJS::HTMLDocFunction::get(const UString &p) const
{
  DOM::HTMLCollection coll;

  switch (id) {
  case Images:
    coll = doc.images();
    break;
  case Applets:
    coll = doc.applets();
    break;
  case Links:
    coll = doc.links();
    break;
  case Forms:
    coll = doc.forms();
    break;
  case Anchors:
    coll = doc.anchors();
    break;
  default:
    return Undefined();
  }

  KJSO tmp(new KJS::HTMLCollection(coll));

  return tmp.get(p);
}

Completion KJS::HTMLDocFunction::execute(const List &args)
{
  KJSO result;
  String s;
  DOM::HTMLElement element;
  DOM::HTMLCollection coll;

  KJSO v = args[0];

  switch (id) {
  case Images:
    coll = doc.images();
    break;
  case Applets:
    coll = doc.applets();
    break;
  case Links:
    coll = doc.links();
    break;
  case Forms:
    coll = doc.forms();
    break;
  case Anchors:
    coll = doc.anchors();
    break;
  case Open:
    doc.open();
    result = Undefined();
    break;
  case Close:
    doc.close();
    result = Undefined();
    break;
  case Write:
    s = v.toString();
    doc.write(s.value().string());
    result = Undefined();
    break;
  case WriteLn:
    s = v.toString();
    doc.write((s.value() + "\n").string());
    result = Undefined();
    break;
  case GetElementById:
    s = v.toString();
    result = new DOMElement(doc.getElementById(s.value().string()));
    break;
  case GetElementsByName:
    s = v.toString();
    result = new DOMNodeList(doc.getElementsByName(s.value().string()));
    break;
  }

  // retrieve n'th element of collection
  if (id == Images || id == Applets || id == Links ||
      id == Forms || id == Anchors) {
    element = coll.item((unsigned long)v.toNumber().value());
    result = new HTMLElement(element);
  }

  return Completion(Normal, result);
}

const TypeInfo KJS::HTMLDocument::info = { "HTMLDocument", HostType,
					   &DOMDocument::info, 0, 0 };

KJSO KJS::HTMLDocument::get(const UString &p) const
{
  KJSO result;

  if (p == "title")
    result = String(doc.title());
  else if (p == "referrer")
    result = String(doc.referrer());
  else if (p == "domain")
    result = String(doc.domain());
  else if (p == "URL")
    result = String(doc.URL());
  else if (p == "body")
    result = new HTMLElement(doc.body());
  else if (p == "images")
    result = new HTMLDocFunction(doc, HTMLDocFunction::Images);
  else if (p == "applets")
    result = new HTMLDocFunction(doc, HTMLDocFunction::Applets);
  else if (p == "links")
    result = new HTMLDocFunction(doc, HTMLDocFunction::Links);
  else if (p == "forms")
    result = new HTMLDocFunction(doc, HTMLDocFunction::Forms);
  else if (p == "anchors")
    result = new HTMLDocFunction(doc, HTMLDocFunction::Anchors);
  else if (p == "cookie")
    result = String(doc.cookie());
  else if (p == "open")
    result = new HTMLDocFunction(doc, HTMLDocFunction::Open);
  else if (p == "close")
    result = new HTMLDocFunction(doc, HTMLDocFunction::Close);
  else if (p == "write")
    result = new HTMLDocFunction(doc, HTMLDocFunction::Write);
  else if (p == "writeln")
    result = new HTMLDocFunction(doc, HTMLDocFunction::WriteLn);
  else if (p == "getElementById")
    result = new HTMLDocFunction(doc, HTMLDocFunction::GetElementById);
  else if (p == "getElementsByName")
    result = new HTMLDocFunction(doc, HTMLDocFunction::GetElementsByName);
  else {
    // look in base class (Document)
    KJSO tmp(new DOMDocument(doc));
    result = tmp.get(p);

    if (result.isA(UndefinedType)) {
      DOM::HTMLElement element;
      DOM::HTMLCollection coll = doc.images(); /* TODO: all() */
      DOM::Node node = coll.namedItem(p.string());
      element = node;
      result = new HTMLElement(element);
    }
  }

  return result;
}

void KJS::HTMLDocument::put(const UString &p, const KJSO& v)
{
  String s;
  if (p == "title") {
    s = v.toString();
    doc.setTitle(s.value().string());
  } else if (p == "cookie") {
    s = v.toString();
    doc.setCookie(s.value().string());
  }
}

const TypeInfo KJS::HTMLElement::info = { "HTMLElement", HostType,
					  &DOMElement::info, 0, 0 };

KJSO KJS::HTMLElement::get(const UString &p) const
{
  DOM::DOMString str;
  DOM::HTMLHtmlElement html;
  DOM::HTMLBodyElement body;
  DOM::HTMLLinkElement link;
  DOM::HTMLAnchorElement anchor;
  DOM::HTMLImageElement image;

  switch (element.elementId()) {
  case ID_HTML:
    html = element;
    if (p == "version")
      return String(html.version());
    break;
  case ID_LINK:
    link = element;
    if (p == "disabled")
      return Boolean(link.disabled());
    else if (p == "charset")
      str = link.charset();
    else if (p == "href")
      str = link.href();
    else if (p == "hreflang")
      str = link.hreflang();
    else if (p == "media")
      str = link.media();
    else if (p == "rel")
      str = link.rel();
    else if (p == "rev")
      str = link.rev();
    else if (p == "target")
      str = link.target();
    else if (p == "type")
      str = link.type();
    else
      break;
    return String(str);
    break;
  case ID_BODY:
    body = element;
    if (p == "aLink")
      str = body.aLink();
    else if (p == "background")
      str = body.background();
    else if (p == "bgColor")
      str = body.bgColor();
    else if (p == "link")
      str = body.link();
    else if (p == "text")
      str = body.text();
    else if (p == "vLink")
      str = body.vLink();
    else
      break;
    return String(str);
  case ID_A:
    anchor = element;
    if (p == "accessKey")
      str = anchor.accessKey();
    else if (p == "charset")
      str = anchor.charset();
    else if (p == "coords")
      str = anchor.coords();
    else if (p == "href")
      str = anchor.href();
    else if (p == "hreflang")
      str = anchor.hreflang();
    else if (p == "name")
      str = anchor.name();
    else if (p == "rel")
      str = anchor.rel();
    else if (p == "rev")
      str = anchor.rev();
    else if (p == "shape")
      str = anchor.shape();
    else if (p == "tabIndex")
      return Number((unsigned long)anchor.tabIndex()); /* ??? */
    else if (p == "target")
      str = anchor.target();
    else if (p == "type")
      str = anchor.type();
    else
      break;
    return String(str);
  case ID_IMG:
    image = element;
    if (p == "lowSrc")
      str = image.lowSrc();
    else if (p == "name")
      str = image.name();
    else if (p == "align")
      str = image.align();
    else if (p == "alt")
      str = image.alt();
    else if (p == "border")
      str = image.border();
    else if (p == "height")
      str = image.height();
    else if (p == "hspace")
      str = image.hspace();
    else if (p == "isMap")
      return Boolean(image.isMap());
    else if (p == "longDesc")
      str = image.longDesc();
    else if (p == "src")
      str = image.src();
    else if (p == "useMap")
      str = image.useMap();
    else if (p == "vspace")
      str = image.vspace();
    else if (p == "width")
      str = image.width();
    else
      break;
    return String(str);
  }

  // generic properties
  if (p == "id")
    str = element.id();
  else if (p == "title")
    str = element.title();
  else if (p == "lang")
    str = element.lang();
  else if (p == "dir")
    str = element.dir();
  else if (p == "className")
    str = element.className();
  else {
    KJSO tmp(new DOMElement(element));
    return tmp.get(p);
  }

  return String(str);
}

void KJS::HTMLElement::put(const UString &p, const KJSO& v)
{
  DOM::HTMLHtmlElement html;
  DOM::HTMLLinkElement link;
  DOM::HTMLBodyElement body;
  DOM::HTMLAnchorElement anchor;
  DOM::HTMLImageElement image;

  String s = v.toString();
  DOM::DOMString str = s.value().string();
  Boolean b = v.toBoolean();
  Number n = v.toNumber();

  switch (element.elementId()) {
  case ID_HTML:
    html = element;
    html.setVersion(str);
    return;
  case ID_LINK:
    link = element;
    if (p == "disabled")
      link.setDisabled(b.value());
    else if (p == "charset")
      link.setCharset(str);
    else if (p == "href")
      link.setHref(str);
    else if (p == "hreflang")
      link.setHreflang(str);
    else if (p == "media")
      link.setMedia(str);
    else if (p == "rel")
      link.setRel(str);
    else if (p == "rev")
      link.setRev(str);
    else if (p == "target")
      link.setTarget(str);
    else if (p == "type")
      link.setType(str);
    else
      break;
    return;
  case ID_BODY:
    body = element;
    if (p == "aLink")
      body.setALink(str);
    else if (p == "background")
      body.setBackground(str);
    else if (p == "bgColor")
      body.setBgColor(str);
    else if (p == "link")
      body.setLink(str);
    else if (p == "text")
      body.setText(str);
    else if (p == "vLink")
      body.setVLink(str);
    else
      break;
    return;
  case ID_A:
    anchor = element;
    if (p == "accessKey")
      anchor.setAccessKey(str);
    else if (p == "charset")
      anchor.setCharset(str);
    else if (p == "coords")
      anchor.setCoords(str);
    else if (p == "href")
      anchor.setHref(str);
    else if (p == "hreflang")
      anchor.setHreflang(str);
    else if (p == "name")
      anchor.setName(str);
    else if (p == "rel")
      anchor.setRel(str);
    else if (p == "rev")
      anchor.setRev(str);
    else if (p == "shape")
      anchor.setShape(str);
    else if (p == "tabIndex")
      anchor.setTabIndex((long)n.value());
    else if (p == "target")
      anchor.setTarget(str);
    else if (p == "type")
      anchor.setType(str);
    else
      break;
    return;
  case ID_IMG:
    image = element;
    if (p == "lowSrc")
      image.setLowSrc(str);
    else if (p == "name")
      image.setName(str);
    else if (p == "align")
      image.setAlign(str);
    else if (p == "alt")
      image.setAlt(str);
    else if (p == "border")
      image.setBorder(str);
    else if (p == "height")
      image.setHeight(str);
    else if (p == "hspace")
      image.setHspace(str);
    else if (p == "isMap")
      image.setIsMap(b.value());
    else if (p == "longDesc")
      image.setLongDesc(str);
    else if (p == "src")
      image.setSrc(str);
    else if (p == "useMap")
      image.setUseMap(str);
    else if (p == "vspace")
      image.setVspace(str);
    else if (p == "width")
      image.setWidth(str);
    else
      break;
    return;
  }

  // generic properties
  if (p == "id")
    element.setId(str);
  else if (p == "title")
    element.setTitle(str);
  else if (p == "lang")
    element.setLang(str);
  else if (p == "dir")
    element.setDir(str);
  else if (p == "className")
    element.setClassName(str);
}

KJSO KJS::HTMLCollection::get(const UString &p) const
{
  KJSO result;

  if (p == "length")
    result = Number(collection.length());
  else if (p == "item")
    result = new HTMLCollectionFunc(collection, HTMLCollectionFunc::Item);
  else if (p == "namedItem")
    result = new HTMLCollectionFunc(collection, HTMLCollectionFunc::NamedItem);
  else {
    DOM::Node node;
    DOM::HTMLElement element;
    unsigned long u;

    // name or index ?
    int ret = sscanf(p.cstring().c_str(), "%lu", &u);
    if (ret)
      node = collection.item(u);
    else
      node = collection.namedItem(p.string());

    element = node;
    result = new HTMLElement(element);
  }

  return result;
}

Completion KJS::HTMLCollectionFunc::execute(const List &args)
{
  KJSO result;

  assert(id == Item || id == NamedItem);

  KJSO v = args[0];
  if (id == Item) {
    Number n = v.toNumber();
    result = new DOMNode(coll.item((unsigned long)n.value()));
  } else {
    String s = v.toString();
    result = new DOMNode(coll.namedItem(s.value().string()));
  }

  return Completion(Normal, result);
}

////////////////////// Image Object ////////////////////////

ImageObject::ImageObject(const Global& global)
{
  Constructor ctor(new ImageConstructor(global));
  setConstructor(ctor);
  setPrototype(global.objectPrototype());

  put("length", Number(2), DontEnum);
}

Completion ImageObject::execute(const List &)
{
  return Completion(Normal, Undefined());
}

ImageConstructor::ImageConstructor(const Global& glob)
  : global(glob)
{
  setPrototype(global.functionPrototype());
}

Object ImageConstructor::construct(const List &)
{
  /* TODO: fetch optional height & width from arguments */

  Object result(new Image());
  /* TODO: do we need a prototype ? */

  return result;
}

KJSO Image::get(const UString &p) const
{
  KJSO result;

  if (p == "src")
    result = String(src);
  else
    result = Undefined();

  return result;
}

void Image::put(const UString &p, const KJSO& v)
{
  if (p == "src") {
    String str = v.toString();
    src = str.value();
  }
}
