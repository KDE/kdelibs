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

#include <khtml_part.h>
#include <html_element.h>
#include <html_head.h>
#include <html_inline.h>
#include <html_image.h>
#include <dom_string.h>

#include "kjs.h"
#include "operations.h"
#include "kjs_dom.h"
#include "kjs_html.h"
#include "kjs_window.h"
#include "kjs_navigator.h"

#include <htmltags.h>

using namespace KJS;

extern "C" {
  // initialize HTML module
  KJSProxy *kjs_html_init(KHTMLPart *khtml)
  {
    KJScript *script = new KJScript();
    script->setCurrent(script);

    KJS::KJSGlobal *global = KJScript::global();
    DOM::HTMLDocument doc;
    doc = khtml->htmlDocument();
    global->put("document", zeroRef(new KJS::HTMLDocument(doc)));
    global->put("window", zeroRef(new KJS::Window(0L)));
    global->put("navigator", zeroRef(new Navigator()));
    global->put("Image", zeroRef(new ImageObject(global)));

    script->setCurrent(0L);
    // this is somewhat ugly. But the only way I found to control the
    // dlopen'ed interpreter (*no* linking!) were callback functions.
    return new KJSProxy(script, &kjs_eval, &kjs_clear);
  }
  // evaluate code
  bool kjs_eval(KJScript *script, const QChar *c, unsigned int len)
  {
    return script->evaluate((const KJS::UChar*)c, len);
  }
  // clear resources allocated by the interpreter
  void kjs_clear(KJScript *script)
  {
    delete script;
  }
};

UString::UString(const DOM::DOMString &d)
{
  unsigned int len = d.length();
  UChar *dat = new UChar[len];
  memcpy(dat, d.unicode(), len * sizeof(UChar));
  rep = new UStringData(dat, len);
}

DOM::DOMString UString::string() const
{
  return DOM::DOMString((QChar*) data(), size());
}

QString UString::qstring() const
{
  return QString((QChar*) data(), size());
}

KJSO *KJS::HTMLDocFunction::get(const UString &p) const
{
  Ptr tmp;
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
    return new KJSUndefined();
  }

  tmp = new KJS::HTMLCollection(coll);

  return tmp->get(p);
}

KJSO *KJS::HTMLDocFunction::execute(KJSContext *context)
{
  KJSO *result;
  Ptr v, n;
  DOM::HTMLElement element;
  DOM::HTMLCollection coll;

  v = context->activation->get("0");

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
  case Write:
    n = toString(v);
    doc.write(n->sVal().string());
    result = new KJSUndefined();
    break;
  default:
    assert((result = 0L));
  }

  // retrieve n'th element of collection
  if (id == Images || id == Applets || id == Links ||
      id == Forms || id == Anchors) {
    n = toNumber(v);
    element = coll.item((unsigned long)n->dVal());
    result = new HTMLElement(element);
  }

  return new KJSCompletion(Normal, result);
}

KJSO *KJS::HTMLDocument::get(const UString &p) const
{
  KJSO *result;

  if (p == "title")
    result = new KJSString(doc.title());
  else if (p == "referrer")
    result = new KJSString(doc.referrer());
  else if (p == "domain")
    result = new KJSString(doc.domain());
  else if (p == "URL")
    result = new KJSString(doc.URL());
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
  else if (p == "write")
    result = new HTMLDocFunction(doc, HTMLDocFunction::Write);
  else if (p == "cookie")
    result = new KJSString(doc.cookie());
  else {
    // look in base class (Document)
    Ptr tmp = new DOMDocument(doc);
    result = tmp->get(p);

    if (result->isA(Undefined)) {
      DOM::HTMLElement element;
      DOM::HTMLCollection coll = doc.images(); /* TODO: all() */
      DOM::Node node = coll.namedItem(p.string());
      element = node;
      result = new HTMLElement(element);
    }
  }

  return result;
}

void KJS::HTMLDocument::put(const UString &p, KJSO *v, int)
{
  Ptr s;
  if (p == "title") {
    s = toString(v);
    doc.setTitle(s->sVal().string());
  } else if (p == "cookie") {
    s = toString(v);
    doc.setCookie(s->sVal().string());
  }
}

KJSO *KJS::HTMLElement::get(const UString &p) const
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
      return new KJSString(html.version());
    break;
  case ID_LINK:
    link = element;
    if (p == "disabled")
      return new KJSBoolean(link.disabled());
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
    return new KJSString(str);
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
    return new KJSString(str);
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
      return new KJSNumber((unsigned long)anchor.tabIndex()); /* ??? */
    else if (p == "target")
      str = anchor.target();
    else if (p == "type")
      str = anchor.type();
    else
      break;
    return new KJSString(str);
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
      return new KJSBoolean(image.isMap());
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
    return new KJSString(str);
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
    Ptr tmp = new DOMElement(element);
    return tmp->get(p);
  }

  return new KJSString(str);
}

void KJS::HTMLElement::put(const UString &p, KJSO *v, int)
{
  DOM::HTMLHtmlElement html;
  DOM::HTMLLinkElement link;
  DOM::HTMLBodyElement body;
  DOM::HTMLAnchorElement anchor;
  DOM::HTMLImageElement image;

  Ptr s = toString(v);
  DOM::DOMString str = s->sVal().string();
  Ptr b = toBoolean(v);
  Ptr n = toNumber(v);

  switch (element.elementId()) {
  case ID_HTML:
    html = element;
    html.setVersion(str);
    return;
  case ID_LINK:
    link = element;
    if (p == "disabled")
      link.setDisabled(b->bVal());
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
      anchor.setTabIndex((long)n->dVal());
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
      image.setIsMap(b->bVal());
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

KJSO *KJS::HTMLCollection::get(const UString &p) const
{
  KJSO *result;

  if (p == "length")
    result = new KJSNumber(collection.length());
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

KJSO *KJS::HTMLCollectionFunc::execute(KJSContext *context)
{
  KJSO *result;
  Ptr v, n;

  assert(id == Item || id == NamedItem);

  v = context->activation->get("0");
  if (id == Item) {
    n = toNumber(v);
    result = new DOMNode(coll.item((unsigned long)n->dVal()));
  } else {
    n = toString(v);
    result = new DOMNode(coll.namedItem(n->sVal().string()));
  }

  return new KJSCompletion(Normal, result);
}

////////////////////// Image Object ////////////////////////

ImageObject::ImageObject(KJSGlobal *global)
{
  KJSConstructor *ctor = new ImageConstructor(global);
  setConstructor(ctor);
  setPrototype(global->objProto);
  ctor->deref();

  put("length", zeroRef(new KJSNumber(2)), DontEnum);
}

KJSO* ImageObject::execute(KJSContext *)
{
  return new KJSCompletion(Normal, zeroRef(new KJSUndefined()));
}

ImageConstructor::ImageConstructor(KJSGlobal *glob)
  : global(glob)
{
  setPrototype(glob->funcProto);
}

KJSObject* ImageConstructor::construct(KJSList *)
{
  /* TODO: fetch optional height & width from arguments */

  KJSObject *result = (KJSObject*) new Image();
  /* TODO: do we need a prototype ? */

  return result;
}

KJSO *Image::get(const UString &p) const
{
  KJSO *result;

  if (p == "src")
    result = new KJSString(src);
  else
    result = new KJSUndefined();

  return result;
}

void Image::put(const UString &p, KJSO *v, int)
{
  if (p == "src") {
    KJSO *str = toString(v);
    src = str->sVal();
  }
}
