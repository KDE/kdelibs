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

#include <stdio.h>

#include "khtml.h"
#include "html_document.h"
#include "html_element.h"
#include "html_misc.h"
#include "dom_string.h"

#include "kjs.h"
#include "operations.h"
#include "html_object.h"

// initialize HTML module
extern "C" {
  int kjs_html_init(void *arg)
  {
    KJS::KJSGlobal *global = KJScript::global();
    KHTMLWidget *w = (KHTMLWidget*) arg;
    global->put("document", zeroRef(new KJS::HTMLDocument(w->htmlDocument())));

    return 0;
  }
};

using namespace KJS;

/* TODO: make these a shallow copies */
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
  else
    result = new KJSUndefined();

  return result;
}

void HTMLDocument::put(const CString &p, KJSO *v, int)
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
  if (p == "length")
    return new KJSNumber(collection.length());

  return new KJSUndefined();
}
