/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999 Harri Porten (porten@kde.org)
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

#include <qstring.h>
#include <html_element.h>
#include <dom_string.h>

#include "kjs.h"
#include "operations.h"
#include "kjs_dom.h"
#include "kjs_html.h"
#include "kjs_window.h"

#include <htmltags.h>

using namespace KJS;

extern "C" {
  // initialize HTML module
  KJSProxy *kjs_html_init(DOM::HTMLDocument doc)
  {
    KJScript *script = new KJScript();
    script->setCurrent(script);

    KJS::KJSGlobal *global = KJScript::global();
    global->put("document", zeroRef(new KJS::HTMLDocument(doc)));
    global->put("window", zeroRef(new KJS::Window(0L)));

    script->setCurrent(0L);
    // this is somewhat ugly. But the only way I found to control the
    // dlopen'ed interpreter (*no* linking!) were callback functions.
    return new KJSProxy(script, &kjs_eval, &kjs_clear);
  }
  // evaluate code
  bool kjs_eval(KJScript *script, const QChar *c, unsigned int len)
  {
    return script->evaluate((const KJS::UnicodeChar*)c, len);
  }
  // clear resources allocated by the interpreter
  void kjs_clear(KJScript *script)
  {
    delete script;
  }
};

UString::UString(const DOM::DOMString &d)
{
  l = d.length();
  s = new UnicodeChar[l];
  memcpy(s, d.unicode(), l * sizeof(UnicodeChar));
}

DOM::DOMString UString::string() const
{
  return DOM::DOMString((QChar*) s, l);
}

QString UString::qstring() const
{
  return QString((QChar*) s, l);
}

KJSO *KJS::HTMLDocFunction::execute(KJSContext *context)
{
  KJSO *result;
  Ptr v, n;
  
  switch (id) {
  case IDDocWrite:
    v = context->activation->get("0");
    n = toString(v);
    doc.write(n->sVal().string());
    result = new KJSUndefined();
    break;
  default:
    assert((result = 0L));
  }

  return new KJSCompletion(Normal, result);
}

KJSO *KJS::HTMLDocument::get(const CString &p) const
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
    result = new HTMLCollection(doc.images());
  else if (p == "links")
    result = new HTMLCollection(doc.links());
  else if (p == "write")
    result = new HTMLDocFunction(doc, IDDocWrite);
  else if (p == "cookie")
    result = new KJSString(doc.cookie());
  else {
    // look in base class (Document)
    Ptr tmp = new DOMDocument(doc);
    result = tmp->get(p);
  }

  return result;
}

void KJS::HTMLDocument::put(const CString &p, KJSO *v, int)
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

KJSO *KJS::HTMLElement::get(const CString &p) const
{
  DOM::DOMString str;

  switch (element.elementId()) {
  case ID_BODY:
    // HTMLBodyElement
    DOM::HTMLBodyElement body;
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
  else
    return new KJSUndefined();

  return new KJSString(str);
}

void KJS::HTMLElement::put(const CString &p, KJSO *v, int)
{
  Ptr s = toString(v);
  DOM::DOMString str = s->sVal().string();

  switch (element.elementId()) {
  case ID_BODY:
    // HTMLBodyElement
    DOM::HTMLBodyElement body;
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

KJSO *KJS::HTMLCollection::get(const CString &p) const
{
  KJSO *result;

  if (p == "length")
    result = new KJSNumber(collection.length());
  else if (p == "item")
    result = new HTMLCollectionFunc(collection, HTMLCollectionFunc::Item);
  else if (p == "namedItem")
    result = new HTMLCollectionFunc(collection, HTMLCollectionFunc::NamedItem);
  else
    result = new KJSUndefined();

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

